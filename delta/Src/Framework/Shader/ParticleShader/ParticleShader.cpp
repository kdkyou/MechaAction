#include "ParticleShader.h"

bool ParticleShader::Init()
{
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
			{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "LIFE"	, 0, DXGI_FORMAT_R32_FLOAT,				0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "SIZE"	, 0, DXGI_FORMAT_R32_FLOAT,				0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR"   , 0, DXGI_FORMAT_R8G8B8A8_UNORM,		0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

	
	// 定数バッファ作成
	m_cbScene = std::make_unique<KdConstantBuffer<CBScene>>();
	m_cbScene->Create();

	return true;
}

void ParticleShader::Draw(const std::vector<Particle>& particles, const Math::Vector3& camRight, const Math::Vector3& camUp, const Math::Matrix& mWorld)
{
	if (particles.empty())return;

	auto& dev = KdDirect3D::Instance();
	auto ctx = dev.WorkDevContext();

	// 頂点バッファの作成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Particle) * (UINT)particles.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = particles.data();

	ID3D11Buffer* vb = nullptr;
	HRESULT hr = dev.WorkDev()->CreateBuffer(&bd, &initData, &vb);
	if (FAILED(hr) || !vb) {
		KdSafeRelease(vb);
		return;
		}
	// バッファの設定
	UINT stride = sizeof(Particle);
	UINT offset = 0;
	ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	ctx->IASetInputLayout(m_inputLayout);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	m_cbScene->Work().camRight = camRight;
	m_cbScene->Work().camUp = camUp;
	m_cbScene->Work().mWorld = mWorld;

	UINT cbSize = sizeof(CBScene);

	m_cbScene->Write();
	m_cbScene->SetVS(0);
	m_cbScene->SetGS(0);
	m_cbScene->SetPS(0);

	// 定数バッファセット
	ctx->VSSetConstantBuffers(0, 1, m_cbScene->GetAddress());
	ctx->GSSetConstantBuffers(0, 1, m_cbScene->GetAddress());
	ctx->PSSetConstantBuffers(0, 1, m_cbScene->GetAddress());

	
	ctx->Draw((UINT)particles.size(), 0);


	KdSafeRelease(vb);
	
}

void ParticleShader::Release()
{
	KdSafeRelease(m_VS);
	KdSafeRelease(m_GS);
	KdSafeRelease(m_PS);

	KdSafeRelease(m_inputLayout);

	m_cbScene->Release();
}

void ParticleShader::BeginParticle()
{
	// 頂点シェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetVertexShader(m_VS))
	{
		KdShaderManager::Instance().SetInputLayout(m_inputLayout);
		KdShaderManager::Instance().SetVSConstantBuffer(0, m_cbScene->GetAddress());
	}

	{
		ID3D11GeometryShader* pNowGS = nullptr;
		KdDirect3D::Instance().WorkDevContext()->GSGetShader(&pNowGS, nullptr, nullptr);

		
		KdDirect3D::Instance().WorkDevContext()->GSSetShader(m_GS, nullptr, 0);

		KdSafeRelease(pNowGS);

	}

	// ピクセルシェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetPixelShader(m_PS))
	{
	}

	KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Anisotropic_Wrap);
	KdShaderManager::Instance().ChangeDepthStencilState(KdDepthStencilState::ZDisable); 
	KdShaderManager::Instance().ChangeRasterizerState(KdRasterizerState::CullNone); 
}

void ParticleShader::EndParticle()
{

	auto& dev = KdDirect3D::Instance();
	auto ctx = dev.WorkDevContext();

	ctx->GSSetShader(nullptr, nullptr, 0);
	// ピクセル／頂点シェーダーも解除（安全のため）
	ctx->VSSetShader(nullptr, nullptr, 0);
	ctx->PSSetShader(nullptr, nullptr, 0);
	ctx->IASetInputLayout(nullptr);

	KdShaderManager::Instance().UndoSamplerState();
	KdShaderManager::Instance().UndoDepthStencilState();
	KdShaderManager::Instance().UndoRasterizerState();
}
