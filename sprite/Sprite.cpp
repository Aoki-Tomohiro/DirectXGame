#include "Sprite.h"

Microsoft::WRL::ComPtr<IDxcUtils> Sprite::sDxcUtils_;
Microsoft::WRL::ComPtr<IDxcCompiler3> Sprite::sDxcCompiler_;
Microsoft::WRL::ComPtr<IDxcIncludeHandler> Sprite::sIncludeHandler_;
Microsoft::WRL::ComPtr<ID3D12RootSignature>  Sprite::sRootSignature_;
Microsoft::WRL::ComPtr<ID3D12PipelineState>  Sprite::sPipelineState_;
ID3D12GraphicsCommandList* Sprite::sCommandList_;
Matrix4x4 Sprite::sMatProjection_;

void Sprite::Initialize() {
	//DXCCompilerの初期化
	Sprite::InitializeDXCCompiler();
	//パイプラインステートの作成
	Sprite::CreatePipelineStateObject();
	//コマンドリストを取得
	sCommandList_ = DirectXCommon::GetInstance()->GetCommandList().Get();
	//平行投影行列の作成
	sMatProjection_ = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::GetInstance()->kClientWidth), float(WinApp::GetInstance()->kClientHeight), 0.0f, 100.0f);
}

void Sprite::InitializeDXCCompiler() {
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&sDxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&sDxcCompiler_));
	assert(SUCCEEDED(hr));

	//現時点でincludeはしないが、includeに対応するための設定を行っていく
	hr = sDxcUtils_->CreateDefaultIncludeHandler(&sIncludeHandler_);
	assert(SUCCEEDED(hr));
}

IDxcBlob* Sprite::CompileShader(const std::wstring& filePath, const wchar_t* profile,
	IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler) {
	//WinAppのインスタンスを取得
	WinApp* winApp = WinApp::GetInstance();
	//1.hlslファイルを読む
	//これからシェーダーをコンパイルする旨をログに出す
	winApp->Log(winApp->ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));
	//hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	//読めなかったら止める
	assert(SUCCEEDED(hr));
	//読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF8の文字コードであることを通知


	//2.Compileする
	LPCWSTR arguments[] = {
		filePath.c_str(),//コンパイル対象のhlslファイル名
		L"-E",L"main",//エントリーポイントの指定。基本的にmain以外にはしない
		L"-T",profile,//ShaderProfileの設定
		L"-Zi",L"-Qembed_debug",//デバッグ用の情報を埋め込む
		L"-Od",//最適化を外しておく
		L"-Zpr",//メモリレイアウトは行優先
	};
	//実際にShaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,//読み込んだファイル
		arguments,//コンパイルオプション
		_countof(arguments),//コンパイルオプションの数
		includeHandler,//includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult)//コンパイル結果
	);
	//コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));


	//3.警告・エラーが出ていないか確認する
	//警告・エラーが出てたらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		winApp->Log(shaderError->GetStringPointer());
		//警告・エラーダメゼッタイ
		assert(false);
	}


	//4.Compile結果を受け取って返す
	//コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//成功したログを出す
	winApp->Log(winApp->ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));
	//もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();
	//実行用のバイナリを返却
	return shaderBlob;
}

void Sprite::CreatePipelineStateObject() {
	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;//0から始まる
	descriptorRange[0].NumDescriptors = 1;//数は一つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//Offsetを自動計算
	//RootParameter作成。複数設定できるので配列。今回は結果一つだけなので長さ１の配列
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderを使う
	rootParameters[0].Descriptor.ShaderRegister = 0;//レジスタ番号０とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderを使う
	rootParameters[1].Descriptor.ShaderRegister = 0;//レジスタ番号０を使う
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//Tableで利用する数
	descriptionRootSignature.pParameters = rootParameters;//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);//配列の長さ
	//Sampler
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0;//レジスタ番号０を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		WinApp::GetInstance()->Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリを元に生成
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&sRootSignature_));
	assert(SUCCEEDED(hr));

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//BlendStateの設定
	//すべての色要素を書き込む
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasterizerStateの設定を行う
	//裏面(時計回り)を表示しない
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(L"shader/SpriteVS.hlsl",
		L"vs_6_0", sDxcUtils_.Get(), sDxcCompiler_.Get(), sIncludeHandler_.Get());
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(L"shader/SpritePS.hlsl",
		L"ps_6_0", sDxcUtils_.Get(), sDxcCompiler_.Get(), sIncludeHandler_.Get());
	assert(pixelShaderBlob != nullptr);

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//グラフィックスパイプラインステートの作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = sRootSignature_.Get();//RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };//PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;//BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	//graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//実際に生成
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&sPipelineState_));
	assert(SUCCEEDED(hr));
}

Sprite::Sprite() {};

Sprite::~Sprite() {};

void Sprite::Create(uint32_t textureHandle, Vector2 position) {
	//テクスチャハンドルを取得
	textureHandle_ = textureHandle;
	//リソースの設定を取得
	resourceDesc_ = TextureManager::GetInstance()->GetResourceDesc(textureHandle_);
	//リソースの設定からサイズを取得
	Vector2 size = { float(resourceDesc_.Width), float(resourceDesc_.Height) };
	//頂点の作成
	vertices_.push_back(VertexPosUV{ { position.x,position.y + size.y,0.0f,1.0f},{ 0.0f,1.0f } });//左下
	vertices_.push_back(VertexPosUV{ { position.x,position.y,0.0f,1.0f},{ 0.0f,0.0f } });//左上
	vertices_.push_back(VertexPosUV{ { position.x + size.x,position.y + size.y,0.0f,1.0f } ,{ 1.0f,1.0f } });//右下
	vertices_.push_back(VertexPosUV{ { position.x,position.y,0.0f,1.0f},{ 0.0f,0.0f } });//左上
	vertices_.push_back(VertexPosUV{ { position.x + size.x,position.y,0.0f,1.0f } ,{ 1.0f,0.0f } });//右上
	vertices_.push_back(VertexPosUV{ { position.x + size.x,position.y + size.y,0.0f,1.0f } ,{ 1.0f,1.0f } });//右下
	//頂点バッファを作成
	vertexResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(VertexPosUV) * vertices_.size());
	//頂点バッファビューを作成
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();//リソースの先頭のアドレスから使う
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexPosUV) * vertices_.size());//使用するリソースのサイズは頂点のサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexPosUV);
	//頂点バッファにデータを書き込む
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, vertices_.data(), sizeof(VertexPosUV) * vertices_.size());
	vertexResource_->Unmap(0, nullptr);
	//WVP用のリソースの作成
	WVPResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ConstBufferDataWVP));
	//マテリアル用のリソースの作成
	MaterialResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ConstBufferDataMaterial));
}

void Sprite::Map() {
	//WVP用のリソースにデータを書き込む
	Matrix4x4 worldMatrix = MakeAffineMatrix({ scale_.x,scale_.y,1.0f }, { 0.0f,0.0f,rotation_ }, { translation_.x,translation_.y,0.0f });
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, sMatProjection_));
	WVPResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	wvpData_->WVP = worldViewProjectionMatrix;
	WVPResource_->Unmap(0, nullptr);

	//マテリアル用のリソースにデータを書き込む
	Transform uvTransform{ {uvScale_.x,uvScale_.y,1.0f},{0.0f,0.0f,uvRotation_},{uvTranslation_.x,uvTranslation_.y,0.0f} };
	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransform.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransform.translate));
	MaterialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = color_;
	materialData_->uvTransform = uvTransformMatrix;
	MaterialResource_->Unmap(0, nullptr);
}

void Sprite::Draw() {
	//マッピングする
	Sprite::Map();
	//クライアント領域のサイズと一緒にして画面全体に表示
	D3D12_VIEWPORT viewport;
	viewport.Width = float(WinApp::GetInstance()->kClientWidth);
	viewport.Height = float(WinApp::GetInstance()->kClientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//viewportを設定
	sCommandList_->RSSetViewports(1, &viewport);
	//基本的にビューポートと同じ矩形が構成されるようにする
	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = LONG(WinApp::GetInstance()->kClientWidth);
	scissorRect.top = 0;
	scissorRect.bottom = LONG(WinApp::GetInstance()->kClientHeight);
	//ScissorRectを設定
	sCommandList_->RSSetScissorRects(1, &scissorRect);
	//ルートシグネチャを設定
	sCommandList_->SetGraphicsRootSignature(sRootSignature_.Get());
	//パイプラインステートを設定
	sCommandList_->SetPipelineState(sPipelineState_.Get());
	//VertexBufferViewを設定
	sCommandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	//WVPResourceを設定
	sCommandList_->SetGraphicsRootConstantBufferView(0, WVPResource_->GetGPUVirtualAddress());
	//MaterialResourceを設定
	sCommandList_->SetGraphicsRootConstantBufferView(1, MaterialResource_->GetGPUVirtualAddress());
	//テクスチャを設定
	TextureManager::GetInstance()->SetGraphicsCommand(textureHandle_);
	//形状を設定
	sCommandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//描画
	sCommandList_->DrawInstanced(UINT(vertices_.size()), 1, 0, 0);
}