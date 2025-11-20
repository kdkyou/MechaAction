#pragma once

class ParticleShader
{
public:
	ParticleShader() {}
	~ParticleShader() {
		Release();
	}

	struct Particle
	{
		Math::Vector3 pos = {};
		float life = 0.0f;
		Math::Vector3 vel = {};
		float size = 10.0f;
		Math::Vector4 color = { 5,0,0,1 };
	};

	struct CBScene
	{
		Math::Vector3	camRight;  //カメラ右
		float dummy1 = 0.0f;
		Math::Vector3	camUp;  //カメラ上
		float dummy2 = 0.0f;
		Math::Matrix mWorld;	// ワールド行列
	};

	struct CBFrame
	{
		float deltaTime = 0.0f;
		Math::Vector3 gravity = {};
		Math::Vector3 targetPos = {};
		int maxParticles = 0;

		int randomSeed = 0;      // 追加: CPU 側シード
		Math::Vector3 spawnRange = { 0.0f, 0.0f, 0.0f }; // 追加: 振れ幅
	};

	bool Init(UINT maxParticles);
	void Release();

	void SetCamRightUp(const Math::Vector3& right, const Math::Vector3& up, const Math::Matrix& mWorld);

	void BeginParticle();
	void EndParticle();

	void UpdateGPU(float deltaTme, const Math::Vector3& targetPos, const Math::Vector3& Vec);

	void Draw(const std::vector<Particle>& particles, const Math::Vector3& camRight, const Math::Vector3& camUp, const Math::Matrix& mWorld);
	void Draw();

private:

	// GPU上のバッファ
	ID3D11Buffer* m_particleBuffer = nullptr;		 // StructuredBuffer<Particle>
	KdConstantBuffer<CBFrame>		m_cbFrame;		 // StructuredBuffer<Frame>
	KdConstantBuffer<CBScene>		m_cbScene;		 // StructuredBuffer<Scene>
	ID3D11UnorderedAccessView* m_particleUAV = nullptr; // Compute書き込み用
	ID3D11ShaderResourceView* m_particleSRV = nullptr;	 // 描画読み込み用

	ID3D11ComputeShader* m_CS = nullptr;
	ID3D11GeometryShader* m_GS = nullptr;
	ID3D11VertexShader* m_VS = nullptr;
	ID3D11PixelShader* m_PS = nullptr;
	ID3D11InputLayout* m_inputLayout = nullptr;

	UINT m_maxParticles = 0;
	bool		m_dirtyCBObj = false;						// 定数バッファのオブジェクトに変更があったかどうか

};