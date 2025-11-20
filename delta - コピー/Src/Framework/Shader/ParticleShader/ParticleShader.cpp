#include "ParticleShader.h"

bool ParticleShader::Init(UINT maxParticles)
{

	m_maxParticles = maxParticles;

	auto& dev = KdDirect3D::Instance();


	// StructuredBuffer作成
	D3D11_BUFFER_DESC desc{};
	desc.BindFlags =  D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = sizeof(Particle) * maxParticles;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(Particle);
	desc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA srd{};
	std::vector<Particle> init(maxParticles);
	for (auto& p : init)
	{
		p.pos = { 0,-9999,0 };
		p.vel = { 0,0,0 };
		p.life = 0.0f;
		p.size = 0.1f;
		p.color = { 0,1,1,1 };
	}
	srd.pSysMem = init.data();

	HRESULT hr = dev.WorkDev()->CreateBuffer(&desc, &srd, &m_particleBuffer);
	if (FAILED(hr)) {
		assert(0 && "Buffer作成失敗");
		Release();
		return false;
	}

	CBFrame frame;
	frame.deltaTime = 0.0f;
	frame.gravity = { 0.0f,0.0f,0.0f };
	frame.targetPos = { 0.0f,0.0f,0.0f };
	
	CBScene scene;
	scene.camRight = { 1,0,0 };
	scene.camUp = { 0,1,0 };
	scene.mWorld = Math::Matrix::Identity;

	// バッファ作成
	
	m_cbFrame.Create(&frame);
	m_cbScene.Create(&scene);


	// UAV / SRV作成
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = 0;
	uavDesc.Buffer.NumElements = maxParticles;
	hr=dev.WorkDev()->CreateUnorderedAccessView(m_particleBuffer, &uavDesc, &m_particleUAV);
	if (FAILED(hr))
	{
		assert(0 && "UAV作成失敗");
		Release();
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = maxParticles;
	hr = dev.WorkDev()->CreateShaderResourceView(m_particleBuffer, &srvDesc, &m_particleSRV);
	if (FAILED(hr))
	{
		assert(0 && "SRV作成失敗");
		Release();
		return false;
	}

	{
#include "ParticleShader_VS.shaderInc"

		// 頂点シェーダー作成
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_VS))) {
			assert(0 && "頂点シェーダー作成失敗");
			Release();
			return false;
		}

		// １頂点の詳細な情報
		std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "SIZE"	, 0, DXGI_FORMAT_R32_FLOAT,				0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR"   , 0, DXGI_FORMAT_R8G8B8A8_UNORM,		0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		// 頂点入力レイアウト作成
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateInputLayout(
			&layout[0],
			layout.size(),
			compiledBuffer,
			sizeof(compiledBuffer), &m_inputLayout)))
		{
			assert(0 && "CreateInputLayout失敗");
			Release();
			return false;
		}
	}

	//コンピュートシェーダー
	{
#include "ParticleShader_CS.shaderInc"
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateComputeShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_CS))) {
			assert(0 && "コンピュートシェーダー作成失敗");
			Release();
			return false;
		}
	}


	//ジオメトリシェーダー
	{
#include "ParticleShader_GS.shaderInc"
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateGeometryShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_GS))) {
			assert(0 && "ジオメトリシェーダー作成失敗");
			Release();
			return false;
		}
	}


	//ピクセルシェーダー
	{
#include "ParticleShader_PS.shaderInc"
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS))) {
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();
			return false;
		}
	}


	
	return true;
}

void ParticleShader::Draw()
{
	auto& dev = KdDirect3D::Instance();
	auto ctx = dev.WorkDevContext();
	
	UINT stride = 0;
	UINT offset = 0;
	ID3D11Buffer* nullVB = nullptr;
	ctx->IASetVertexBuffers(0, 1, &nullVB, &stride, &offset);
	ctx->IASetInputLayout(nullptr);

	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// シェーダー設定
	ctx->VSSetShader(m_VS, nullptr, 0);
	ctx->GSSetShader(m_GS, nullptr, 0);
	ctx->PSSetShader(m_PS, nullptr, 0);

	// バッファ書き込み
	if (m_dirtyCBObj)
	{
		m_cbScene.Write();
		m_dirtyCBObj = false;
	}
	ctx->VSSetConstantBuffers(1, 1, m_cbScene.GetAddress());
	ctx->GSSetConstantBuffers(1, 1, m_cbScene.GetAddress());

	// stucturedBufferをSRVとして渡す
	ctx->VSSetShaderResources(0, 1, &m_particleSRV);
	ctx->GSSetShaderResources(0, 1, &m_particleSRV);
	
	// インスタンシング相当のGPUDraw
	ctx->Draw(m_maxParticles, 0);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	ctx->VSSetShaderResources(0, 1, &nullSRV);
	ctx->GSSetShaderResources(0, 1, &nullSRV);
	
}

void ParticleShader::Draw(const std::vector<Particle>& particles, const Math::Vector3& camRight, const Math::Vector3& camUp, const Math::Matrix& mWorld)
{
	if (particles.empty())return;

	/*auto& dev = KdDirect3D::Instance();
	auto ctx = dev.WorkDevContext();*/

	//// 頂点バッファの作成
	//D3D11_BUFFER_DESC bd = {};
	//bd.Usage = D3D11_USAGE_DYNAMIC;
	//bd.ByteWidth = sizeof(Particle) * (UINT)particles.size();
	//bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//D3D11_SUBRESOURCE_DATA initData = {};
	//initData.pSysMem = particles.data();

	//ID3D11Buffer* vb = nullptr;
	//HRESULT hr = dev.WorkDev()->CreateBuffer(&bd, &initData, &vb);
	//if (FAILED(hr) || !vb) {
	//	KdSafeRelease(vb);
	//	return;
	//	}
	//// バッファの設定
	//UINT stride = sizeof(Particle);
	//UINT offset = 0;
	//ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	//ctx->IASetInputLayout(m_inputLayout);
	//ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	//m_cbScene->Work().camRight = camRight;
	//m_cbScene->Work().camUp = camUp;
	//m_cbScene->Work().mWorld = mWorld;

	//UINT cbSize = sizeof(CBScene);

	//m_cbScene->SetVS(0);
	//m_cbScene->SetGS(0);
	//m_cbScene->SetPS(0);
	//m_cbScene->Write();

	//// 定数バッファセット
	//ctx->VSSetConstantBuffers(0, 1, m_cbScene->GetAddress());
	//ctx->GSSetConstantBuffers(0, 1, m_cbScene->GetAddress());
	//ctx->PSSetConstantBuffers(0, 1, m_cbScene->GetAddress());

	//
	//ctx->Draw((UINT)particles.size(), 0);


}

void ParticleShader::Release()
{
	KdSafeRelease(m_VS);
	KdSafeRelease(m_CS);
	KdSafeRelease(m_GS);
	KdSafeRelease(m_PS);

	KdSafeRelease(m_inputLayout);
	
	KdSafeRelease(m_particleBuffer);
	KdSafeRelease(m_particleSRV);
	KdSafeRelease(m_particleUAV);
	m_cbFrame.Release();
	m_cbScene.Release();
}

void ParticleShader::SetCamRightUp(const Math::Vector3& right, const Math::Vector3& up, const Math::Matrix& mWorld)
{
	m_cbScene.Work().camRight = right;
	m_cbScene.Work().camUp = up;
	m_cbScene.Work().mWorld = mWorld;
	m_dirtyCBObj = true;
}

void ParticleShader::BeginParticle()
{
	// 頂点シェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetVertexShader(m_VS))
	{
	}

	// ジオメトリシェーダーのパイプライン変更
	{
		ID3D11GeometryShader* pNowGS = nullptr;
		KdDirect3D::Instance().WorkDevContext()->GSGetShader(&pNowGS, nullptr, nullptr);
		// セットしようとしているシェーダーが現行と同じならキャンセル
		bool needChange = pNowGS != m_GS;

		if (needChange)
		{
			KdDirect3D::Instance().WorkDevContext()->GSSetShader(m_GS, nullptr, 0);
		}
		KdSafeRelease(pNowGS);
	}

	// ピクセルシェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetPixelShader(m_PS))
	{
	}

	KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Anisotropic_Wrap);
	KdShaderManager::Instance().ChangeDepthStencilState(KdDepthStencilState::ZDisable); 
	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);
	KdShaderManager::Instance().ChangeRasterizerState(KdRasterizerState::CullNone); 
}

void ParticleShader::EndParticle()
{

	auto& dev = KdDirect3D::Instance();
	auto ctx = dev.WorkDevContext();

	// ピクセル／頂点シェーダーも解除（安全のため）
	ctx->VSSetShader(nullptr, nullptr, 0);
	ctx->GSSetShader(nullptr, nullptr, 0);
	ctx->PSSetShader(nullptr, nullptr, 0);
	ctx->IASetInputLayout(nullptr);

	KdShaderManager::Instance().UndoSamplerState();
	KdShaderManager::Instance().UndoDepthStencilState();
	KdShaderManager::Instance().UndoBlendState();
	KdShaderManager::Instance().UndoRasterizerState();
}

void ParticleShader::UpdateGPU(float deltaTme, const Math::Vector3& targetPos, const Math::Vector3& Vec)
{
	auto ctx = KdDirect3D::Instance().WorkDevContext();


	// 定数バッファ転送
	m_cbFrame.Work().deltaTime = deltaTme;
	m_cbFrame.Work().gravity = Vec;
	m_cbFrame.Work().targetPos = targetPos;
	m_cbFrame.Work().maxParticles = m_maxParticles;

	// 追加: シードと振れ幅を設定（振れ幅は任意に調整）
	m_cbFrame.Work().randomSeed = (rand() % 3); // or frame counter
	m_cbFrame.Work().spawnRange = { 2.0f, 2.0f, 1.0f }; // 例: X,Z 範囲 ±1、Y ±0.5
	m_cbFrame.Write();

	
	ctx->CSSetConstantBuffers(0, 1,m_cbFrame.GetAddress());
	ctx->CSSetUnorderedAccessViews(0, 1, &m_particleUAV, nullptr);
	
	ctx->CSSetShader(m_CS, nullptr, 0);

	UINT disPatchX = (m_maxParticles + 255) / 256;
	ctx->Dispatch(disPatchX, 1, 1);

	ID3D11UnorderedAccessView* nullUAV = nullptr;
	ctx->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	ctx->CSSetShader(nullptr, nullptr, 0);
}
