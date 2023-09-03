#include "Model.h"
#include <fstream>
#include <sstream>

void Model::InitializeDxcCompiler() {
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));

	//現時点でincludeはしないが、includeに対応するための設定を行っていく
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

IDxcBlob* Model::CompileShader(const std::wstring& filePath, const wchar_t* profile,
	IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler) {
	//1.hlslファイルを読む
	//これからシェーダーをコンパイルする旨をログに出す
	WinApp::GetInstance()->Log(WinApp::GetInstance()->ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));
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
		WinApp::GetInstance()->Log(shaderError->GetStringPointer());
		//警告・エラーダメゼッタイ
		assert(false);
	}


	//4.Compile結果を受け取って返す
	//コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//成功したログを出す
	WinApp::GetInstance()->Log(WinApp::GetInstance()->ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));
	//もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();
	//実行用のバイナリを返却
	return shaderBlob;
}

void Model::CreatePipelineStateObject() {
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
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderを使う
	rootParameters[0].Descriptor.ShaderRegister = 0;//レジスタ番号０とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderを使う
	rootParameters[1].Descriptor.ShaderRegister = 0;//レジスタ番号０を使う
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//Tableで利用する数
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;//レジスタ番号１を使う
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
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
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
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(L"Project/Resources/Shader/Object3d.VS.hlsl",
		L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(L"Project/Resources/Shader/Object3d.PS.hlsl",
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
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();//RootSignature
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
	graphicsPipelineStateDesc.NumRenderTargets = 2;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.RTVFormats[1] = DXGI_FORMAT_R32_FLOAT;
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//実際に生成
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState_));
	assert(SUCCEEDED(hr));
}

Model::Model() {};

Model::~Model() {};

void Model::Create(const std::vector<VertexData>& vertices) {
	//DxcCompilerの初期化
	Model::InitializeDxcCompiler();
	//パイプラインステートの作成
	Model::CreatePipelineStateObject();
	//コマンドリストを取得
	commandList_ = DirectXCommon::GetInstance()->GetCommandList().Get();
	//DirectionalLightの作成
	directionalLight_ = std::make_unique<DirectionalLight>();
	directionalLight_->Initialize();
	//メッシュの作成
	vertex_ = std::make_unique<Vertex>();
	vertex_->Create(vertices);
	//マテリアルの作成
	material_ = std::make_unique<Material>();
	material_->Create();
	//行列の作成
	transformationMatrix_ = std::make_unique<TransformationMatrix>();
	transformationMatrix_->Initialize();
}

void Model::CreateSphere() {
	//DxcCompilerの初期化
	Model::InitializeDxcCompiler();
	//パイプラインステートの作成
	Model::CreatePipelineStateObject();
	//コマンドリストを取得
	commandList_ = DirectXCommon::GetInstance()->GetCommandList().Get();
	//DirectionalLightの作成
	directionalLight_ = std::make_unique<DirectionalLight>();
	directionalLight_->Initialize();
	//メッシュの作成
	std::vector<VertexData> vertices{};
	const float pi = 3.14f;
	const uint32_t kSubdivision = 16;
	uint32_t latIndex = 0;
	uint32_t lonIndex = 0;
	//経度分割一つ分の角度φd
	const float kLonEvery = pi * 2.0f / float(kSubdivision);
	//緯度分割一つ分の角度θd
	const float kLatEvery = pi / float(kSubdivision);
	//緯度の方向に分割
	for (latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;//θ
		//経度の方向に分割しながら線を描く
		for (lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;//φ
			//頂点にデータを入力する。基準点a
			vertices.push_back(VertexData{ {std::cos(lat) * std::cos(lon),std::sin(lat),std::cos(lat) * std::sin(lon),1.0f},
				{ float(lonIndex) / float(kSubdivision),1.0f - float(latIndex) / float(kSubdivision)},
				{std::cos(lat) * std::cos(lon),std::sin(lat),std::cos(lat) * std::sin(lon)} });

			//残りの５頂点も順番に計算して入力していく
			vertices.push_back(VertexData{ {std::cos(lat + kLatEvery) * std::cos(lon),std::sin(lat + kLatEvery),std::cos(lat + kLatEvery) * std::sin(lon),1.0f},
				{float(lonIndex) / float(kSubdivision),1.0f - float(latIndex + 1) / float(kSubdivision)},
				{std::cos(lat + kLatEvery) * std::cos(lon),std::sin(lat + kLatEvery),std::cos(lat + kLatEvery) * std::sin(lon)} });

			vertices.push_back(VertexData{ {std::cos(lat) * std::cos(lon + kLonEvery),std::sin(lat),std::cos(lat) * std::sin(lon + kLonEvery),1.0f},
				{float(lonIndex + 1) / float(kSubdivision),1.0f - float(latIndex) / float(kSubdivision)},
				{std::cos(lat) * std::cos(lon + kLonEvery),std::sin(lat),std::cos(lat) * std::sin(lon + kLonEvery)} });

			vertices.push_back(VertexData{ { std::cos(lat) * std::cos(lon + kLonEvery),std::sin(lat),std::cos(lat) * std::sin(lon + kLonEvery),1.0f},
				{float(lonIndex + 1) / float(kSubdivision),1.0f - float(latIndex) / float(kSubdivision)},
				{std::cos(lat) * std::cos(lon + kLonEvery),std::sin(lat),std::cos(lat) * std::sin(lon + kLonEvery)} });

			vertices.push_back(VertexData{ {std::cos(lat + kLatEvery) * std::cos(lon),std::sin(lat + kLatEvery),std::cos(lat + kLatEvery) * std::sin(lon),1.0f},
				{float(lonIndex) / float(kSubdivision),1.0f - float(latIndex + 1) / float(kSubdivision)},
				{std::cos(lat + kLatEvery) * std::cos(lon),std::sin(lat + kLatEvery),std::cos(lat + kLatEvery) * std::sin(lon)} });

			vertices.push_back(VertexData{ {std::cos(lat + kLatEvery) * std::cos(lon + kLonEvery),std::sin(lat + kLatEvery),std::cos(lat + kLatEvery) * std::sin(lon + kLonEvery),1.0f},
				{float(lonIndex + 1) / float(kSubdivision),1.0f - float(latIndex + 1) / float(kSubdivision)},
				{std::cos(lat + kLatEvery) * std::cos(lon + kLonEvery),std::sin(lat + kLatEvery),std::cos(lat + kLatEvery) * std::sin(lon + kLonEvery)} });
		}
	}
	vertex_ = std::make_unique<Vertex>();
	vertex_->Create(vertices);
	//マテリアルの作成
	material_ = std::make_unique<Material>();
	material_->Create();
	//行列の作成
	transformationMatrix_ = std::make_unique<TransformationMatrix>();
	transformationMatrix_->Initialize();
}

void Model::CreateFromOBJ(const std::string& directoryPath, const std::string& filename) {
	//DxcCompilerの初期化
	Model::InitializeDxcCompiler();
	//パイプラインステートの作成
	Model::CreatePipelineStateObject();
	//コマンドリストを取得
	commandList_ = DirectXCommon::GetInstance()->GetCommandList().Get();
	//DirectionalLightの作成
	directionalLight_ = std::make_unique<DirectionalLight>();
	directionalLight_->Initialize();
	//モデルの読み込み
	modelData_ = Model::LoadObjFile(directoryPath, filename);
	//メッシュの作成
	vertex_ = std::make_unique<Vertex>();
	vertex_->Create(modelData_.vertices);
	//マテリアルの作成
	material_ = std::make_unique<Material>();
	material_->Create();
	//行列の作成
	transformationMatrix_ = std::make_unique<TransformationMatrix>();
	transformationMatrix_->Initialize();
	//テクスチャの読み込み
	textureHandle_ = TextureManager::GetInstance()->Load(modelData_.material.textureFilePath);
}

Model::ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;//構築するModelData
	std::vector<Vector4> positions;//位置
	std::vector<Vector3> normals;//法線
	std::vector<Vector2> texcoords;//テクスチャ座標
	std::string line;//ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());//とりあえず開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;//先頭の識別子を読む

		//identifierに応じた処理
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.z *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.z *= -1.0f;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];
			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				//頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');// /区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				//要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				triangle[faceVertex] = { position,texcoord,normal };
			}
			//頂点を逆順で登録することで、回り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			//materialTempalteLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}

Model::MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData materialData;//構築するMaterialData
	std::string line;//ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());//とりあえず開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		//identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}

void Model::Draw(const WorldTransform& worldTransform, const ViewProjection& viewProjection) {
	//行列を計算する
	transformationMatrix_->Update(worldTransform, viewProjection);
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
	//RootSignatureを設定
	commandList_->SetGraphicsRootSignature(rootSignature_.Get());
	//PSOを設定
	commandList_->SetPipelineState(pipelineState_.Get());
	//マテリアルを設定
	material_->SetGraphicsCommand(UINT(RootParameterIndex::Material));
	//transformationMatrixを設定
	transformationMatrix_->SetGraphicsCommand(UINT(RootParameterIndex::TransformationMatrix));
	//ディスクリプタヒープをセット
	TextureManager::GetInstance()->SetGraphicsDescriptorHeap();
	//テクスチャを設定
	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(UINT(RootParameterIndex::Texture), textureHandle_);
	//directionalLightを設定
	directionalLight_->SetGraphicsCommand(UINT(RootParameterIndex::DirectionalLight));
	//描画
	vertex_->Draw();
}

void Model::Draw(const WorldTransform& worldTransform, const ViewProjection& viewProjection, uint32_t textureHandle) {
	//行列を計算する
	transformationMatrix_->Update(worldTransform, viewProjection);
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
	//RootSignatureを設定
	commandList_->SetGraphicsRootSignature(rootSignature_.Get());
	//PSOを設定
	commandList_->SetPipelineState(pipelineState_.Get());
	//マテリアルを設定
	material_->SetGraphicsCommand(UINT(RootParameterIndex::Material));
	//transformationMatrixを設定
	transformationMatrix_->SetGraphicsCommand(UINT(RootParameterIndex::TransformationMatrix));
	//ディスクリプタヒープをセット
	TextureManager::GetInstance()->SetGraphicsDescriptorHeap();
	//テクスチャを設定
	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(UINT(RootParameterIndex::Texture), textureHandle_);
	//directionalLightを設定
	directionalLight_->SetGraphicsCommand(UINT(RootParameterIndex::DirectionalLight));
	//描画
	vertex_->Draw();
}