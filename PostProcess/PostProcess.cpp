#include "PostProcess.h"

PostProcess* PostProcess::instance = nullptr;

PostProcess* PostProcess::GetInstance() {
	if (instance == nullptr) {
		instance = new PostProcess();
	}
	return instance;
}

void PostProcess::DeleteInstance() {
	delete instance;
	instance = nullptr;
}

void PostProcess::Initialize() {
	//インスタンスを取得
	dxCommon_ = DirectXCommon::GetInstance();
	//DxcCompilerの初期化
	PostProcess::InitializeDxcCompiler();
	//二パス目のパイプラインステートの作成
	PostProcess::CreateSecondPassPipelineStateObject();
	//ブラー用のパイプラインステートの作成
	PostProcess::CreateBlurPipelineStateObject();
	//最終パスのパイプラインステートの作成
	PostProcess::CreateFinalPassPipelineStateObject();
	//コマンドリストを取得
	commandList_ = dxCommon_->GetCommandList().Get();

	//頂点の作成
	vertices_.push_back(VertexPosUV{ {-1.0f,-1.0f,1.0,1.0f},{0.0f,1.0f} });
	vertices_.push_back(VertexPosUV{ {-1.0f,1.0f,1.0f,1.0f},{0.0f,0.0f} });
	vertices_.push_back(VertexPosUV{ {1.0f,-1.0f,1.0f,1.0f},{1.0f,1.0f} });
	vertices_.push_back(VertexPosUV{ {-1.0f,1.0f,1.0f,1.0f},{0.0f,0.0f} });
	vertices_.push_back(VertexPosUV{ {1.0f,1.0f,1.0f,1.0f},{1.0f,0.0f} });
	vertices_.push_back(VertexPosUV{ {1.0f,-1.0f,1.0f,1.0f},{1.0f,1.0f} });
	//頂点バッファを作成
	vertexResource_ = dxCommon_->CreateBufferResource(sizeof(VertexPosUV) * vertices_.size());
	//頂点バッファビューを作成
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();//リソースの先頭のアドレスから使う
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexPosUV) * vertices_.size());//使用するリソースのサイズは頂点のサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexPosUV);
	//頂点バッファにデータを書き込む
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, vertices_.data(), sizeof(VertexPosUV) * vertices_.size());
	vertexResource_->Unmap(0, nullptr);

	//ガウシアンぼかし用のリソースの作成
	gaussianBlurResource_ = dxCommon_->CreateBufferResource(sizeof(ConstBufferDataGaussianBlur));
	//ガウシアンぼかし用のリソースに書き込む
	gaussianBlurResource_->Map(0, nullptr, reinterpret_cast<void**>(&gaussianBlurData_));
	gaussianBlurData_->textureWidth = WinApp::GetInstance()->kClientWidth;
	gaussianBlurData_->textureHeight = WinApp::GetInstance()->kClientHeight;
	float total = 0.0f;
	for (int i = 0; i < 8; i++) {
		gaussianBlurData_->weight[i] = expf(-(i * i) / (2 * s_ * s_));
		total += gaussianBlurData_->weight[i];
	}
	total = total * 2.0f - 1.0f;
	//最終的な合計値で重みをわる
	for (int i = 0; i < 8; i++) {
		gaussianBlurData_->weight[i] /= total;
	}
	gaussianBlurResource_->Unmap(0, nullptr);

	//マルチパス用のディスクリプタヒープの作成
	multiPassRTVDescriptorHeap_ = dxCommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 12, false);
	multiPassDSVDescriptorHeap_ = dxCommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
	//マルチパス用の深度バッファの作成
	depthStencilResource_ = dxCommon_->CreateDepthStencilTextureResource(WinApp::GetInstance()->kClientWidth, WinApp::GetInstance()->kClientHeight);
	//マルチパス用のDepthStencilViewの作成
	dxCommon_->CreateDepthStencilView(multiPassDSVDescriptorHeap_, depthStencilResource_);

	//一パス目のリソースの作成
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	firstPassSRVIndex_ = TextureManager::GetInstance()->CreateMultiPassTextureResource(WinApp::GetInstance()->kClientWidth, WinApp::GetInstance()->kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, clearColor);
	firstPassRTVIndex_ = PostProcess::CreateMultiPassRenderTargetView(TextureManager::GetInstance()->GetTextureResource(firstPassSRVIndex_), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	//線形深度のリソースの作成
	float clearDepthColor[] = { 1.0f,0.0f,0.0f,0.0f };
	linearDepthSRVIndex_ = TextureManager::GetInstance()->CreateMultiPassTextureResource(WinApp::GetInstance()->kClientWidth, WinApp::GetInstance()->kClientHeight, DXGI_FORMAT_R32_FLOAT, clearDepthColor);
	linearDepthRTVIndex_ = PostProcess::CreateMultiPassRenderTargetView(TextureManager::GetInstance()->GetTextureResource(linearDepthSRVIndex_), DXGI_FORMAT_R32_FLOAT);
	//二パス目のリソースの作成
	secondPassSRVIndex_ = TextureManager::GetInstance()->CreateMultiPassTextureResource(WinApp::GetInstance()->kClientWidth, WinApp::GetInstance()->kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, clearColor);
	secondPassRTVIndex_ = PostProcess::CreateMultiPassRenderTargetView(TextureManager::GetInstance()->GetTextureResource(secondPassSRVIndex_), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	//高輝度用のリソースの作成
	highIntensitySRVIndex_ = TextureManager::GetInstance()->CreateMultiPassTextureResource(WinApp::GetInstance()->kClientWidth, WinApp::GetInstance()->kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, clearColor);
	highIntensityRTVIndex_ = PostProcess::CreateMultiPassRenderTargetView(TextureManager::GetInstance()->GetTextureResource(highIntensitySRVIndex_), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	//ブラー用のリソースの作成
	for (int i = 0; i < 2; i++) {
		//通常ぼかし
		blurSRVIndex_[i] = TextureManager::GetInstance()->CreateMultiPassTextureResource(WinApp::GetInstance()->kClientWidth, WinApp::GetInstance()->kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, clearColor);
		blurRTVIndex_[i] = PostProcess::CreateMultiPassRenderTargetView(TextureManager::GetInstance()->GetTextureResource(blurSRVIndex_[i]), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		//高輝度ぼかし
		highIntensityBlurSRVIndex_[i] = TextureManager::GetInstance()->CreateMultiPassTextureResource(WinApp::GetInstance()->kClientWidth, WinApp::GetInstance()->kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, clearColor);
		highIntensityBlurRTVIndex_[i] = PostProcess::CreateMultiPassRenderTargetView(TextureManager::GetInstance()->GetTextureResource(highIntensityBlurSRVIndex_[i]), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		//縮小ぼかし
		shrinkBlurSRVIndex_[i] = TextureManager::GetInstance()->CreateMultiPassTextureResource(WinApp::GetInstance()->kClientWidth / 2, WinApp::GetInstance()->kClientHeight / 2, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, clearColor);
		shrinkBlurRTVIndex_[i] = PostProcess::CreateMultiPassRenderTargetView(TextureManager::GetInstance()->GetTextureResource(shrinkBlurSRVIndex_[i]), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		//高輝度縮小ぼかし
		highIntensityShrinkBlurSRVIndex_[i] = TextureManager::GetInstance()->CreateMultiPassTextureResource(WinApp::GetInstance()->kClientWidth / 2, WinApp::GetInstance()->kClientHeight / 2, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, clearColor);
		highIntensityShrinkBlurRTVIndex_[i] = PostProcess::CreateMultiPassRenderTargetView(TextureManager::GetInstance()->GetTextureResource(highIntensityShrinkBlurSRVIndex_[i]), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	}

	//Bloom用のリソースを作成
	bloomResource_ = dxCommon_->CreateBufferResource(sizeof(ConstBufferDataBloom));
	//Bloom用のリソースに書き込む
	bloomResource_->Map(0, nullptr, reinterpret_cast<void**>(&bloomData_));
	bloomData_->enable = false;
	bloomResource_->Unmap(0, nullptr);

	//フォグ用のリソースを作成
	fogResource_ = dxCommon_->CreateBufferResource(sizeof(ConstBufferDataFog));
	//フォグ用のリソースに書き込む
	fogResource_->Map(0, nullptr, reinterpret_cast<void**>(&fogData_));
	fogData_->enable = false;
	fogData_->scale = 0.5f;
	fogData_->attenuationRate = 2.0f;
	fogResource_->Unmap(0, nullptr);

	//Dof用のリソースを作成
	dofResource_ = dxCommon_->CreateBufferResource(sizeof(ConstBufferDataDof));
	//Dof用のリソースに書き込む
	dofResource_->Map(0, nullptr, reinterpret_cast<void**>(&dofData_));
	dofData_->enable = true;
	dofResource_->Unmap(0, nullptr);

	//LensDistortion用のリソースを作成
	lensDistortionResource_ = dxCommon_->CreateBufferResource(sizeof(ConstBufferDataLensDistortion));
	//LensDistortion用のリソースに書き込む
	lensDistortionResource_->Map(0, nullptr, reinterpret_cast<void**>(&lensDistortionData_));
	lensDistortionData_->enable = false;
	lensDistortionData_->tightness = 2.5f;
	lensDistortionData_->strength = -0.1f;
	lensDistortionResource_->Unmap(0, nullptr);

	//ビネット用のリソースの作成
	vignetteResource_ = dxCommon_->CreateBufferResource(sizeof(ConstBufferDataVignette));
	//ビネットのリソースに書き込む
	vignetteResource_->Map(0, nullptr, reinterpret_cast<void**>(&vignetteData_));
	vignetteData_->enable = false;
	vignetteData_->intensity = 1.0f;
	vignetteResource_->Unmap(0, nullptr);
}

void PostProcess::InitializeDxcCompiler() {
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));

	//現時点でincludeはしないが、includeに対応するための設定を行っていく
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

IDxcBlob* PostProcess::CompileShader(const std::wstring& filePath, const wchar_t* profile,
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

void PostProcess::CreateSecondPassPipelineStateObject() {
	//RootSignatureの作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//ディスクリプタレンジ
	D3D12_DESCRIPTOR_RANGE descriptorRange[1]{};
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	//RootParameter作成
	D3D12_ROOT_PARAMETER rootParameters[1]{};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	descriptionRootSignature.pParameters = rootParameters;//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);//配列の長さ
	//Sampler
	D3D12_STATIC_SAMPLER_DESC sampler{};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0~1の範囲外をリピート
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	sampler.MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipMapを使う
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);
	descriptionRootSignature.NumStaticSamplers = 1;
	descriptionRootSignature.pStaticSamplers = &sampler;
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
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&secondPassRootSignature_));
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
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(L"shader/SecondPassVS.hlsl",
		L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(L"shader/SecondPassPS.hlsl",
		L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(pixelShaderBlob != nullptr);

	//グラフィックスパイプラインステートの作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = secondPassRootSignature_.Get();//RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };//PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;//BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 2;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//実際に生成
	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&secondPassPipelineState_));
	assert(SUCCEEDED(hr));
}

void PostProcess::CreateBlurPipelineStateObject() {
	//RootSignatureの作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//ディスクリプタレンジ
	D3D12_DESCRIPTOR_RANGE descriptorRange[2]{};
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[1].BaseShaderRegister = 1;
	descriptorRange[1].NumDescriptors = 1;
	//RootParameter作成
	D3D12_ROOT_PARAMETER rootParameters[3]{};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange[1];
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].Descriptor.ShaderRegister = 0;
	descriptionRootSignature.pParameters = rootParameters;//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);//配列の長さ
	//Sampler
	D3D12_STATIC_SAMPLER_DESC sampler{};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0~1の範囲外をリピート
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	sampler.MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipMapを使う
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);
	descriptionRootSignature.NumStaticSamplers = 1;
	descriptionRootSignature.pStaticSamplers = &sampler;
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
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&blurRootSignature_));
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
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(L"shader/HorizontalBlurVS.hlsl",
		L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(L"shader/HorizontalBlurPS.hlsl",
		L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(pixelShaderBlob != nullptr);

	//グラフィックスパイプラインステートの作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = blurRootSignature_.Get();//RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };//PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;//BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 2;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//実際に生成
	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&horizontalBlurPipelineState_));
	assert(SUCCEEDED(hr));


	//縦ぼかし用のシェーダーをコンパイルする
	vertexShaderBlob = CompileShader(L"shader/VerticalBlurVS.hlsl",
		L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(vertexShaderBlob != nullptr);

	pixelShaderBlob = CompileShader(L"shader/VerticalBlurPS.hlsl",
		L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(pixelShaderBlob != nullptr);

	//グラフィックスパイプラインステートの作成
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };//PixelShader
	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&verticalBlurPipelineState_));
	assert(SUCCEEDED(hr));

}

void PostProcess::CreateFinalPassPipelineStateObject() {
	//RootSignatureの作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//ディスクリプタレンジ
	D3D12_DESCRIPTOR_RANGE descriptorRange[5]{};
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 3;
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[1].BaseShaderRegister = 3;
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[2].BaseShaderRegister = 4;
	descriptorRange[2].NumDescriptors = 1;
	descriptorRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[3].BaseShaderRegister = 5;
	descriptorRange[3].NumDescriptors = 1;
	descriptorRange[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[4].BaseShaderRegister = 6;
	descriptorRange[4].NumDescriptors = 1;
	//RootParameter作成
	D3D12_ROOT_PARAMETER rootParameters[10]{};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange[1];
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange[2];
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].DescriptorTable.pDescriptorRanges = &descriptorRange[3];
	rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[4].DescriptorTable.pDescriptorRanges = &descriptorRange[4];
	rootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderを使う
	rootParameters[5].Descriptor.ShaderRegister = 0;//レジスタ番号０とバインド
	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderを使う
	rootParameters[6].Descriptor.ShaderRegister = 1;//レジスタ番号０とバインド
	rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderを使う
	rootParameters[7].Descriptor.ShaderRegister = 2;//レジスタ番号０とバインド
	rootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderを使う
	rootParameters[8].Descriptor.ShaderRegister = 3;//レジスタ番号０とバインド
	rootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderを使う
	rootParameters[9].Descriptor.ShaderRegister = 4;//レジスタ番号０とバインド
	descriptionRootSignature.pParameters = rootParameters;//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);//配列の長さ
	//Sampler
	D3D12_STATIC_SAMPLER_DESC sampler{};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0~1の範囲外をリピート
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	sampler.MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipMapを使う
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);
	descriptionRootSignature.NumStaticSamplers = 1;
	descriptionRootSignature.pStaticSamplers = &sampler;
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
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&finalPassRootSignature_));
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
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(L"shader/FinalPassVS.hlsl",
		L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(L"shader/FinalPassPS.hlsl",
		L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
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
	graphicsPipelineStateDesc.pRootSignature = finalPassRootSignature_.Get();//RootSignature
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
	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&finalPassPipelineState_));
	assert(SUCCEEDED(hr));
}

uint32_t PostProcess::CreateMultiPassRenderTargetView(const Microsoft::WRL::ComPtr<ID3D12Resource>& resource, DXGI_FORMAT format) {
	rtvIndex_++;
	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = format;
	//一パス目用RTVの作成
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dxCommon_->GetCPUDescriptorHandle(multiPassRTVDescriptorHeap_, dxCommon_->descriptorSizeRTV, rtvIndex_);
	dxCommon_->GetDevice()->CreateRenderTargetView(resource.Get(), &rtvDesc, rtvHandle);

	return rtvIndex_;
}

void PostProcess::PreDraw() {
	//barrierを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//通常
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(firstPassSRVIndex_).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier);	
	//線形深度
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(linearDepthSRVIndex_).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier);
	//描画先のRTVとDSVのハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	rtvHandles[0] = dxCommon_->GetCPUDescriptorHandle(multiPassRTVDescriptorHeap_, dxCommon_->descriptorSizeRTV, firstPassRTVIndex_);
	rtvHandles[1] = dxCommon_->GetCPUDescriptorHandle(multiPassRTVDescriptorHeap_, dxCommon_->descriptorSizeRTV, linearDepthRTVIndex_);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxCommon_->GetCPUDescriptorHandle(multiPassDSVDescriptorHeap_, dxCommon_->descriptorSizeDSV, 0);
	//描画先のRTVを設定する
	commandList_->OMSetRenderTargets(2, rtvHandles, false, &dsvHandle);
	//指定した深度で画面全体をクリアする
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	//指定した色で画面をクリアする
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	commandList_->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);
	float clearDepthColor[] = { 1.0f,0.0f,0.0f,0.0f };
	commandList_->ClearRenderTargetView(rtvHandles[1], clearDepthColor, 0, nullptr);
}

void PostProcess::PostDraw() {
	//barrierを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//通常
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(firstPassSRVIndex_).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList_->ResourceBarrier(1, &barrier);
	//線形深度
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(linearDepthSRVIndex_).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList_->ResourceBarrier(1, &barrier);

	//二パス目の描画前処理
	PostProcess::PreSecondPassDraw();
	//二パス目の描画処理
	PostProcess::SecondPassDraw();
	//二パス目の描画後処理
	PostProcess::PostSecondPassDraw();

	//横ぼかし前処理
	PostProcess::PreHorizontalBlur();
	//横ぼかし処理
	PostProcess::Blur(horizontalBlurPipelineState_, secondPassSRVIndex_, highIntensitySRVIndex_);
	//横ぼかし後処理
	PostProcess::PostHorizontalBlur();

	//縦ぼかし前処理
	PostProcess::PreVerticalBlur();
	//縦ぼかし処理
	PostProcess::Blur(verticalBlurPipelineState_, blurSRVIndex_[0], highIntensityBlurSRVIndex_[0]);
	//縦ぼかし後処理
	PostProcess::PostVerticalBlur();


	//横縮小ぼかし前処理
	PostProcess::PreHorizontalShrinkBlur();
	//横縮小ぼかし処理
	PostProcess::Blur(horizontalBlurPipelineState_, blurSRVIndex_[1], highIntensityBlurSRVIndex_[1]);
	//横縮小ぼかし後処理
	PostProcess::PostHorizontalShrinkBlur();

	//縦縮小ぼかし前処理
	PostProcess::PreVerticalShrinkBlur();
	//縦縮小ぼかし処理
	PostProcess::Blur(verticalBlurPipelineState_, shrinkBlurSRVIndex_[0], highIntensityShrinkBlurSRVIndex_[0]);
	//縦縮小ぼかし後処理
	PostProcess::PostVerticalShrinkBlur();
}

void PostProcess::PreSecondPassDraw() {
	//barrierを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//通常テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(secondPassSRVIndex_).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier);
	//高輝度テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(highIntensitySRVIndex_).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier);
	//描画先のRTVとハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	rtvHandles[0] = dxCommon_->GetCPUDescriptorHandle(multiPassRTVDescriptorHeap_, dxCommon_->descriptorSizeRTV, secondPassRTVIndex_);
	rtvHandles[1] = dxCommon_->GetCPUDescriptorHandle(multiPassRTVDescriptorHeap_, dxCommon_->descriptorSizeRTV, highIntensityRTVIndex_);
	//描画先のRTVを設定する
	commandList_->OMSetRenderTargets(2, rtvHandles, false, nullptr);
	//指定した色で画面をクリアする
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	commandList_->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);
	commandList_->ClearRenderTargetView(rtvHandles[1], clearColor, 0, nullptr);
	//クライアント領域のサイズと一緒にして画面全体に表示
	D3D12_VIEWPORT viewport;
	viewport.Width = float(WinApp::GetInstance()->kClientWidth);
	viewport.Height = float(WinApp::GetInstance()->kClientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//viewportを設定
	commandList_->RSSetViewports(1, &viewport);
	//基本的にビューポートと同じ矩形が構成されるようにする
	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = LONG(WinApp::GetInstance()->kClientWidth);
	scissorRect.top = 0;
	scissorRect.bottom = LONG(WinApp::GetInstance()->kClientHeight);
	//ScissorRectを設定
	commandList_->RSSetScissorRects(1, &scissorRect);
}

void PostProcess::PostSecondPassDraw() {
	//barrierを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//通常テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(secondPassSRVIndex_).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList_->ResourceBarrier(1, &barrier);
	//高輝度テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(highIntensitySRVIndex_).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList_->ResourceBarrier(1, &barrier);
}

void PostProcess::SecondPassDraw() {
	//ルートシグネチャを設定
	commandList_->SetGraphicsRootSignature(secondPassRootSignature_.Get());
	//パイプラインステートを設定
	commandList_->SetPipelineState(secondPassPipelineState_.Get());
	//VertexBufferViewを設定
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	//ディスクリプタヒープをセット
	TextureManager::GetInstance()->SetGraphicsDescriptorHeap();
	//テクスチャを設定
	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(0, firstPassSRVIndex_);
	//形状を設定
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//描画
	commandList_->DrawInstanced(UINT(vertices_.size()), 1, 0, 0);
}

void PostProcess::PreHorizontalBlur() {
	//barrierを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//通常テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(blurSRVIndex_[0]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier);
	//高輝度テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(highIntensityBlurSRVIndex_[0]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier);
	//描画先のRTVとハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	rtvHandles[0] = dxCommon_->GetCPUDescriptorHandle(multiPassRTVDescriptorHeap_, dxCommon_->descriptorSizeRTV, blurRTVIndex_[0]);
	rtvHandles[1] = dxCommon_->GetCPUDescriptorHandle(multiPassRTVDescriptorHeap_, dxCommon_->descriptorSizeRTV, highIntensityBlurRTVIndex_[0]);
	//描画先のRTVを設定する
	commandList_->OMSetRenderTargets(2, rtvHandles, false, nullptr);
	//指定した色で画面をクリアする
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	commandList_->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);
	commandList_->ClearRenderTargetView(rtvHandles[1], clearColor, 0, nullptr);
	//クライアント領域のサイズと一緒にして画面全体に表示
	D3D12_VIEWPORT viewport;
	viewport.Width = float(WinApp::GetInstance()->kClientWidth);
	viewport.Height = float(WinApp::GetInstance()->kClientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//viewportを設定
	commandList_->RSSetViewports(1, &viewport);
	//基本的にビューポートと同じ矩形が構成されるようにする
	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = LONG(WinApp::GetInstance()->kClientWidth);
	scissorRect.top = 0;
	scissorRect.bottom = LONG(WinApp::GetInstance()->kClientHeight);
	//ScissorRectを設定
	commandList_->RSSetScissorRects(1, &scissorRect);
}

void PostProcess::PostHorizontalBlur() {
	//barrierを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//通常テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(blurSRVIndex_[0]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList_->ResourceBarrier(1, &barrier);
	//高輝度テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(highIntensityBlurSRVIndex_[0]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList_->ResourceBarrier(1, &barrier);
}

void PostProcess::PreVerticalBlur() {
	//barrierを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//通常テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(blurSRVIndex_[1]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier);
	//高輝度テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(highIntensityBlurSRVIndex_[1]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier);
	//描画先のRTVとハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	rtvHandles[0] = dxCommon_->GetCPUDescriptorHandle(multiPassRTVDescriptorHeap_, dxCommon_->descriptorSizeRTV, blurRTVIndex_[1]);
	rtvHandles[1] = dxCommon_->GetCPUDescriptorHandle(multiPassRTVDescriptorHeap_, dxCommon_->descriptorSizeRTV, highIntensityBlurRTVIndex_[1]);
	//描画先のRTVを設定する
	commandList_->OMSetRenderTargets(2, rtvHandles, false, nullptr);
	//指定した色で画面をクリアする
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	commandList_->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);
	commandList_->ClearRenderTargetView(rtvHandles[1], clearColor, 0, nullptr);
	//クライアント領域のサイズと一緒にして画面全体に表示
	D3D12_VIEWPORT viewport;
	viewport.Width = float(WinApp::GetInstance()->kClientWidth);
	viewport.Height = float(WinApp::GetInstance()->kClientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//viewportを設定
	commandList_->RSSetViewports(1, &viewport);
	//基本的にビューポートと同じ矩形が構成されるようにする
	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = LONG(WinApp::GetInstance()->kClientWidth);
	scissorRect.top = 0;
	scissorRect.bottom = LONG(WinApp::GetInstance()->kClientHeight);
	//ScissorRectを設定
	commandList_->RSSetScissorRects(1, &scissorRect);
}

void PostProcess::PostVerticalBlur() {
	//barrierを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//通常テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(blurSRVIndex_[1]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList_->ResourceBarrier(1, &barrier);
	//高輝度テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(highIntensityBlurSRVIndex_[1]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList_->ResourceBarrier(1, &barrier);
}

void PostProcess::PreHorizontalShrinkBlur(){
	//barrierを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//通常テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(shrinkBlurSRVIndex_[0]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier);
	//高輝度テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(highIntensityShrinkBlurSRVIndex_[0]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier);
	//描画先のRTVとハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	rtvHandles[0] = dxCommon_->GetCPUDescriptorHandle(multiPassRTVDescriptorHeap_, dxCommon_->descriptorSizeRTV, shrinkBlurRTVIndex_[0]);
	rtvHandles[1] = dxCommon_->GetCPUDescriptorHandle(multiPassRTVDescriptorHeap_, dxCommon_->descriptorSizeRTV, highIntensityShrinkBlurRTVIndex_[0]);
	//描画先のRTVを設定する
	commandList_->OMSetRenderTargets(2, rtvHandles, false, nullptr);
	//指定した色で画面をクリアする
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	commandList_->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);
	commandList_->ClearRenderTargetView(rtvHandles[1], clearColor, 0, nullptr);
	//縮小バッファのサイズと一緒にして画面全体に表示
	D3D12_VIEWPORT viewport;
	viewport.Width = float(WinApp::GetInstance()->kClientWidth) / 2;
	viewport.Height = float(WinApp::GetInstance()->kClientHeight) / 2;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//viewportを設定
	commandList_->RSSetViewports(1, &viewport);
	//基本的にビューポートと同じ矩形が構成されるようにする
	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = LONG(WinApp::GetInstance()->kClientWidth) / 2;
	scissorRect.top = 0;
	scissorRect.bottom = LONG(WinApp::GetInstance()->kClientHeight) / 2;
	//ScissorRectを設定
	commandList_->RSSetScissorRects(1, &scissorRect);
}

void PostProcess::PostHorizontalShrinkBlur() {
	//barrierを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//通常テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(shrinkBlurSRVIndex_[0]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList_->ResourceBarrier(1, &barrier);
	//高輝度テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(highIntensityShrinkBlurSRVIndex_[0]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList_->ResourceBarrier(1, &barrier);
}

void PostProcess::PreVerticalShrinkBlur() {
	//barrierを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//通常テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(shrinkBlurSRVIndex_[1]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier);
	//高輝度テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(highIntensityShrinkBlurSRVIndex_[1]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier);
	//描画先のRTVとハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	rtvHandles[0] = dxCommon_->GetCPUDescriptorHandle(multiPassRTVDescriptorHeap_, dxCommon_->descriptorSizeRTV, shrinkBlurRTVIndex_[1]);
	rtvHandles[1] = dxCommon_->GetCPUDescriptorHandle(multiPassRTVDescriptorHeap_, dxCommon_->descriptorSizeRTV, highIntensityShrinkBlurRTVIndex_[1]);
	//描画先のRTVを設定する
	commandList_->OMSetRenderTargets(2, rtvHandles, false, nullptr);
	//指定した色で画面をクリアする
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	commandList_->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);
	commandList_->ClearRenderTargetView(rtvHandles[1], clearColor, 0, nullptr);
	//縮小バッファのサイズと一緒にして画面全体に表示
	D3D12_VIEWPORT viewport;
	viewport.Width = float(WinApp::GetInstance()->kClientWidth) / 2;
	viewport.Height = float(WinApp::GetInstance()->kClientHeight) / 2;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//viewportを設定
	commandList_->RSSetViewports(1, &viewport);
	//基本的にビューポートと同じ矩形が構成されるようにする
	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = LONG(WinApp::GetInstance()->kClientWidth) / 2;
	scissorRect.top = 0;
	scissorRect.bottom = LONG(WinApp::GetInstance()->kClientHeight) / 2;
	//ScissorRectを設定
	commandList_->RSSetScissorRects(1, &scissorRect);
}

void PostProcess::PostVerticalShrinkBlur() {
	//barrierを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//通常テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(shrinkBlurSRVIndex_[1]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList_->ResourceBarrier(1, &barrier);
	//高輝度テクスチャ
	barrier.Transition.pResource = TextureManager::GetInstance()->GetTextureResource(highIntensityShrinkBlurSRVIndex_[1]).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList_->ResourceBarrier(1, &barrier);
	//バックバッファをセット
	dxCommon_->SetBackBuffer();
}

void PostProcess::Blur(const Microsoft::WRL::ComPtr<ID3D12PipelineState>& pipelineState, uint32_t blurSRVIndex, uint32_t highIntensityBlurSRVIndex) {
	//ルートシグネチャを設定
	commandList_->SetGraphicsRootSignature(blurRootSignature_.Get());
	//パイプラインステートを設定
	commandList_->SetPipelineState(pipelineState.Get());
	//VertexBufferViewを設定
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	//ディスクリプタヒープをセット
	TextureManager::GetInstance()->SetGraphicsDescriptorHeap();
	//テクスチャを設定
	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(0, blurSRVIndex);
	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(1, highIntensityBlurSRVIndex);
	//CBVをセット
	commandList_->SetGraphicsRootConstantBufferView(2, gaussianBlurResource_->GetGPUVirtualAddress());
	//形状を設定
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//描画
	commandList_->DrawInstanced(UINT(vertices_.size()), 1, 0, 0);
}

void PostProcess::Draw() {
	//クライアント領域のサイズと一緒にして画面全体に表示
	D3D12_VIEWPORT viewport;
	viewport.Width = float(WinApp::GetInstance()->kClientWidth);
	viewport.Height = float(WinApp::GetInstance()->kClientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//viewportを設定
	commandList_->RSSetViewports(1, &viewport);
	//基本的にビューポートと同じ矩形が構成されるようにする
	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = LONG(WinApp::GetInstance()->kClientWidth);
	scissorRect.top = 0;
	scissorRect.bottom = LONG(WinApp::GetInstance()->kClientHeight);
	//ScissorRectを設定
	commandList_->RSSetScissorRects(1, &scissorRect);
	//ルートシグネチャを設定
	commandList_->SetGraphicsRootSignature(finalPassRootSignature_.Get());
	//パイプラインステートを設定
	commandList_->SetPipelineState(finalPassPipelineState_.Get());
	//VertexBufferViewを設定
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	//ディスクリプタヒープをセット
	TextureManager::GetInstance()->SetGraphicsDescriptorHeap();
	//テクスチャを設定
	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(0, linearDepthSRVIndex_);
	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(1, blurSRVIndex_[1]);
	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(2, highIntensityBlurSRVIndex_[1]);
	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(3, shrinkBlurSRVIndex_[1]);
	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(4, highIntensityShrinkBlurSRVIndex_[1]);
	//CBVを設定
	commandList_->SetGraphicsRootConstantBufferView(5, bloomResource_->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(6, fogResource_->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(7, dofResource_->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(8, lensDistortionResource_->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(9, vignetteResource_->GetGPUVirtualAddress());
	//形状を設定
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//描画
	commandList_->DrawInstanced(UINT(vertices_.size()), 1, 0, 0);
}