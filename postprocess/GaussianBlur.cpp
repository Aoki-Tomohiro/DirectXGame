#include "GaussianBlur.h"

GaussianBlur::GaussianBlur() {};

GaussianBlur::~GaussianBlur() {
	signatureBlob_->Release();
	if (errorBlob_) {
		errorBlob_->Release();
	}
	pixelShaderBlob_->Release();
	vertexShaderBlob_->Release();
};

void GaussianBlur::Initialize() {
	dxCommon_ = DirectXCommon::GetInstance();
	//DXCCompilerの初期化
	GaussianBlur::InitializeDXCCompiler();
	//パイプラインステートの作成
	GaussianBlur::CreatePipelineStateObject();
	//ビューポートの作成
	GaussianBlur::CreateViewport();
	//シザーレクトの作成
	GaussianBlur::CreateScissorRect();
	//メッシュの作成
	vertices_.push_back(VertexData{ {-1.0f,-1.0f,0.0,1.0f},{0.0f,1.0f},{-1.0f,-1.0f,0.0f} });
	vertices_.push_back(VertexData{ {-1.0f,1.0f,0.0f,1.0f},{0.0f,0.0f},{-1.0f,1.0f,0.0f} });
	vertices_.push_back(VertexData{ {1.0f,-1.0f,0.0f,1.0f},{1.0f,1.0f},{1.0f,-1.0f,0.0f} });
	vertices_.push_back(VertexData{ {-1.0f,1.0f,0.0f,1.0f},{0.0f,0.0f},{-1.0f,1.0f,0.0f} });
	vertices_.push_back(VertexData{ {1.0f,1.0f,0.0f,1.0f},{1.0f,0.0f},{1.0f,1.0f,0.0f} });
	vertices_.push_back(VertexData{ {1.0f,-1.0f,0.0f,1.0f},{1.0f,1.0f},{1.0f,-1.0f,0.0f} });
	vertex_ = std::make_unique<Vertex>();
	vertex_->Create(vertices_);
	//ガウシアンぼかし用のリソースの作成
	gaussianBlurResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(ConstBufferDataGaussianBlur));
	//ガウシアンぼかし用のリソースに書き込む
	ConstBufferDataGaussianBlur gaussianData;
	float total = 0.0f;
	for (int i = 0; i < 8; i++) {
		gaussianData.weight[i] = expf(-(i * i) / (2 * s_ * s_));
		total += gaussianData.weight[i];
	}
	total = total * 2.0f - 1.0f;
	//最終的な合計値で重みをわる
	for (int i = 0; i < 8; i++) {
		gaussianData.weight[i] /= total;
	}
	gaussianBlurResource_->Map(0, nullptr, reinterpret_cast<void**>(&gaussianBlurData_));
	*gaussianBlurData_ = gaussianData;
	gaussianBlurResource_->Unmap(0, nullptr);
}

void GaussianBlur::InitializeDXCCompiler() {
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));

	//現時点でincludeはしないが、includeに対応するための設定を行っていく
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

IDxcBlob* GaussianBlur::CompileShader(const std::wstring& filePath, const wchar_t* profile,
	IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler) {
	//1.hlslファイルを読む
	//これからシェーダーをコンパイルする旨をログに出す
	dxCommon_->GetWinApp()->Log(dxCommon_->GetWinApp()->ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));
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
		dxCommon_->GetWinApp()->Log(shaderError->GetStringPointer());
		//警告・エラーダメゼッタイ
		assert(false);
	}


	//4.Compile結果を受け取って返す
	//コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//成功したログを出す
	dxCommon_->GetWinApp()->Log(dxCommon_->GetWinApp()->ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));
	//もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();
	//実行用のバイナリを返却
	return shaderBlob;
}

void GaussianBlur::CreatePipelineStateObject() {
	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;//0から始まる
	descriptorRange[0].NumDescriptors = 1;//数は一つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	//RootParameter作成。複数設定できるので配列。今回は結果一つだけなので長さ一の配列
	D3D12_ROOT_PARAMETER rootParameters[2] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderを使う
	rootParameters[0].Descriptor.ShaderRegister = 0;//レジスタ番号０を使う
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//Tableで利用する数
	descriptionRootSignature.pParameters = rootParameters;//ルートパラメーター配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);//配列の長さ
	//Sampler
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//０～１の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	//シリアライズしてバイナリにする
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(hr)) {
		dxCommon_->GetWinApp()->Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}
	//バイナリを元に生成
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
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
	vertexShaderBlob_ = CompileShader(L"Shader/BlurVS.hlsl",
		L"vs_6_0", dxcUtils_, dxcCompiler_, includeHandler_);
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = CompileShader(L"Shader/HorizontalBlurPS.hlsl",
		L"ps_6_0", dxcUtils_, dxcCompiler_, includeHandler_);
	assert(pixelShaderBlob_ != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();//RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob_->GetBufferPointer(),
	vertexShaderBlob_->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob_->GetBufferPointer(),
	pixelShaderBlob_->GetBufferSize() };//PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;//BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
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
	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&horizontalBlurGraphicsPipelineState_));
	assert(SUCCEEDED(hr));

	//縦ぼかし
	vertexShaderBlob_ = CompileShader(L"Shader/BlurVS.hlsl",
		L"vs_6_0", dxcUtils_, dxcCompiler_, includeHandler_);
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = CompileShader(L"Shader/VerticalBlurPS.hlsl",
		L"ps_6_0", dxcUtils_, dxcCompiler_, includeHandler_);
	assert(pixelShaderBlob_ != nullptr);

	graphicsPipelineStateDesc.VS = { vertexShaderBlob_->GetBufferPointer(),
	vertexShaderBlob_->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob_->GetBufferPointer(),
	pixelShaderBlob_->GetBufferSize() };//PixelShader

	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&verticalBlurGraphicsPipelineState_));
	assert(SUCCEEDED(hr));


	//縮小横ぼかし
	vertexShaderBlob_ = CompileShader(L"Shader/BlurVS.hlsl",
		L"vs_6_0", dxcUtils_, dxcCompiler_, includeHandler_);
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = CompileShader(L"Shader/HorizontalShrinkBlurPS.hlsl",
		L"ps_6_0", dxcUtils_, dxcCompiler_, includeHandler_);
	assert(pixelShaderBlob_ != nullptr);

	graphicsPipelineStateDesc.VS = { vertexShaderBlob_->GetBufferPointer(),
	vertexShaderBlob_->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob_->GetBufferPointer(),
	pixelShaderBlob_->GetBufferSize() };//PixelShader

	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&horizontalShrinkBlurGraphicsPipelineState_));
	assert(SUCCEEDED(hr));


	//縮小縦ぼかし
	vertexShaderBlob_ = CompileShader(L"Shader/BlurVS.hlsl",
		L"vs_6_0", dxcUtils_, dxcCompiler_, includeHandler_);
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = CompileShader(L"Shader/VerticalShrinkBlurPS.hlsl",
		L"ps_6_0", dxcUtils_, dxcCompiler_, includeHandler_);
	assert(pixelShaderBlob_ != nullptr);

	graphicsPipelineStateDesc.VS = { vertexShaderBlob_->GetBufferPointer(),
	vertexShaderBlob_->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob_->GetBufferPointer(),
	pixelShaderBlob_->GetBufferSize() };//PixelShader

	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&verticalShrinkBlurGraphicsPipelineState_));
	assert(SUCCEEDED(hr));
}

void GaussianBlur::CreateViewport() {
	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport_.Width = float(dxCommon_->GetWinApp()->kClientWidth);
	viewport_.Height = float(dxCommon_->GetWinApp()->kClientHeight);
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
}

void GaussianBlur::CreateScissorRect() {
	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect_.left = 0;
	scissorRect_.right = dxCommon_->GetWinApp()->kClientWidth;
	scissorRect_.top = 0;
	scissorRect_.bottom = dxCommon_->GetWinApp()->kClientHeight;
}

void GaussianBlur::Update() {
	ConstBufferDataGaussianBlur gaussianData;
	float total = 0.0f;
	for (int i = 0; i < 8; i++) {
		gaussianData.weight[i] = expf(-(i * i) / (2 * s_ * s_));
		total += gaussianData.weight[i];
	}
	total = total * 2.0f - 1.0f;
	//最終的な合計値で重みをわる
	for (int i = 0; i < 8; i++) {
		gaussianData.weight[i] /= total;
	}
	gaussianBlurResource_->Map(0, nullptr, reinterpret_cast<void**>(&gaussianBlurData_));
	*gaussianBlurData_ = gaussianData;
	gaussianBlurResource_->Unmap(0, nullptr);
}

void GaussianBlur::HorizontalBlur() {
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = dxCommon_->GetGPUDescriptorHandle(dxCommon_->GetSRVDescriptorHeap(), dxCommon_->descriptorSizeSRV, 1);
	//ビューポートを設定
	dxCommon_->GetCommandList()->RSSetViewports(1, &viewport_);
	//シザーレクトを設定
	dxCommon_->GetCommandList()->RSSetScissorRects(1, &scissorRect_);
	//RootSignatureを設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSOを設定
	dxCommon_->GetCommandList()->SetPipelineState(horizontalBlurGraphicsPipelineState_.Get());
	//ぼかしの重さを設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, gaussianBlurResource_->GetGPUVirtualAddress());
	//通常テクスチャを設定
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvHandle);
	//描画
	vertex_->Draw();
}

void GaussianBlur::VerticalBlur() {
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = dxCommon_->GetGPUDescriptorHandle(dxCommon_->GetSRVDescriptorHeap(), dxCommon_->descriptorSizeSRV, 2);
	//ビューポートを設定
	dxCommon_->GetCommandList()->RSSetViewports(1, &viewport_);
	//シザーレクトを設定
	dxCommon_->GetCommandList()->RSSetScissorRects(1, &scissorRect_);
	//RootSignatureを設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSOを設定
	dxCommon_->GetCommandList()->SetPipelineState(verticalBlurGraphicsPipelineState_.Get());
	//ぼかしの重さを設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, gaussianBlurResource_->GetGPUVirtualAddress());
	//横にぼかしたテクスチャを設定
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvHandle);
	//描画
	vertex_->Draw();
}

void GaussianBlur::HorizontalShrinkBlur() {
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = dxCommon_->GetGPUDescriptorHandle(dxCommon_->GetSRVDescriptorHeap(), dxCommon_->descriptorSizeSRV, 1);
	//ビューポートを設定
	D3D12_VIEWPORT viewport{};
	viewport.Height = float(dxCommon_->GetWinApp()->kClientHeight) / 2;
	viewport.Width = float(dxCommon_->GetWinApp()->kClientWidth) / 2;
	dxCommon_->GetCommandList()->RSSetViewports(1, &viewport);
	//シザーレクトを設定
	dxCommon_->GetCommandList()->RSSetScissorRects(1, &scissorRect_);
	//RootSignatureを設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSOを設定
	dxCommon_->GetCommandList()->SetPipelineState(horizontalShrinkBlurGraphicsPipelineState_.Get());
	//ぼかしの重さを設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, gaussianBlurResource_->GetGPUVirtualAddress());
	//通常テクスチャを設定
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvHandle);
	//描画
	vertex_->Draw();
}

void GaussianBlur::VerticalShrinkBlur() {
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = dxCommon_->GetGPUDescriptorHandle(dxCommon_->GetSRVDescriptorHeap(), dxCommon_->descriptorSizeSRV, 3);
	//ビューポートを設定
	D3D12_VIEWPORT viewport{};
	viewport.Height = float(dxCommon_->GetWinApp()->kClientHeight) / 2;
	viewport.Width = float(dxCommon_->GetWinApp()->kClientWidth) / 2;
	dxCommon_->GetCommandList()->RSSetViewports(1, &viewport);
	//Scissorを設定
	dxCommon_->GetCommandList()->RSSetScissorRects(1, &scissorRect_);
	//RootSignatureを設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSOを設定
	dxCommon_->GetCommandList()->SetPipelineState(verticalShrinkBlurGraphicsPipelineState_.Get());
	//ぼかしの重さを設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, gaussianBlurResource_->GetGPUVirtualAddress());
	//横にぼかしたテクスチャを設定
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvHandle);
	//描画
	vertex_->Draw();
}