#pragma once

class ParticleShader
{
public:
	ParticleShader(){}
	~ParticleShader() {
		Release();
	}

	struct Particle
	{
		Math::Vector3 pos;
		Math::Vector3 vel;
		float life;
		float size;
		uint32_t color;
	};

	struct CBScene
	{
		Math::Vector3	camRight;  //カメラ右
		float dummy1 = 0.0f;
		Math::Vector3	camUp;  //カメラ上
		float dummy2 = 0.0f;
		Math::Matrix mWorld;
	};


	bool Init();
	void Release();

	void BeginParticle();
	void EndParticle();

	void Draw(const std::vector<Particle>& particles, const Math::Vector3& camRight, const Math::Vector3& camUp,const Math::Matrix& mWorld);

private:

	
	ID3D11VertexShader* m_VS = nullptr;
	ID3D11GeometryShader* m_GS = nullptr;
	ID3D11PixelShader* m_PS = nullptr;

	// 頂点入力レイアウト
	ID3D11InputLayout* m_inputLayout = nullptr;

	// バッファ作成
	std::unique_ptr<KdConstantBuffer<CBScene>> m_cbScene;

	bool		m_dirtyCBObj = false;						// 定数バッファのオブジェクトに変更があったかどうか

};