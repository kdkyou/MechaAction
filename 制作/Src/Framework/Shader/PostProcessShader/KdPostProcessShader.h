#pragma once

class KdPostProcessShader
{
public:
	KdPostProcessShader() {}
	~KdPostProcessShader()
	{
		Release();
	}

	void SetNearClippingDistance(float distance) { m_cb0_DoFInfo.Work().NearClippingDistance = distance; }
	void SetFarClippingDistance(float distance) { m_cb0_DoFInfo.Work().FarClippingDistance = distance; }
	void SetFocusDistance(float distance) { m_cb0_DoFInfo.Work().FocusDistance = distance; }
	void SetFocusRange(float fore, float back) { m_cb0_DoFInfo.Work().FocusForeRange = fore; m_cb0_DoFInfo.Work().FocusBackRange = back; }

	void SetBrightThreshold(float threshold) { m_cb0_BrightInfo.Work().Threshold = threshold; }

	//放射ブラー用
	//サンプリング回数　長さ 中心 反映しない範囲 ディザリング 回転角度
	void SetRadialBlurInfo(int samlingSize, float strength, const Math::Vector2& center, float mask, int dither, float vor);
	void UndoRadialBlur();

	//グリッチ用
	//	グリッド	発生時間	フレームレート絶対0.0にしない	頻度	グリッドするか
	//				↑動く時間	↑滑らかに↓粗く			↑発生しやすい↓たまに
	void SetGlitch(const Math::Vector2& resolu, float time, float frameRate, float frequency, int useGrid, int playerHit, const Math::Vector2& center);
	void UndoGlitch();

	enum TextureKind
	{
		Normal,
		RadialBlur,
		Glitch,
		Add,
	};
	void SetCombine(UINT kind) { m_cb0_CombineInfo.Work().Switch = kind; }

	struct Vertex
	{
		Math::Vector3 Pos;
		Math::Vector2 UV;
	};

	bool Init();

	void Release();

	void Draw();

	void BeginBright();
	void EndBright();

	void PostEffectProcess();

	void GenerateBlurTexture(std::shared_ptr<KdTexture>& spSrcTex, std::shared_ptr<KdTexture>& spDstTex, D3D11_VIEWPORT& VP, int blurRadius);

	//放射ブラー用
	void GenerateRadialBlurTexture(std::shared_ptr<KdTexture>& spSrcTex, std::shared_ptr<KdTexture>& spDstTex, D3D11_VIEWPORT& VP);

	//グリッチ用
	void GenerateGlitchTexture(std::shared_ptr<KdTexture>& spSrcTex, std::shared_ptr<KdTexture>& spDstTex, D3D11_VIEWPORT& VP);
	//合成用
	void GenerateCombineTexture(std::shared_ptr<KdTexture>& spSrcTex, std::shared_ptr<KdTexture>& spDstTex, D3D11_VIEWPORT& VP);

private:

	void BlurProcess();

	//放射ブラー用
	void RadialBlurProcess();
	//グリッチ用
	void GlicthProcess();
	//合成用
	void CombineProcess();

	void LightBloomProcess();
	void DepthOfFieldProcess();


	void CreateBlurOffsetList(std::vector<Math::Vector3>& dstInfo, const std::shared_ptr<KdTexture>& spSrcTex, int samplingSize, const Math::Vector2& dir);

	void DrawTexture(std::shared_ptr<KdTexture>* spSrcTex, int srcTexSize, std::shared_ptr<KdTexture> spDstTex, D3D11_VIEWPORT* pVP);

	void SetBlurInfo(const std::shared_ptr<KdTexture>& spSrcTex, int samplingSize, const Math::Vector2& dir);
	void SetBlurInfo(const std::vector<Math::Vector3>& srcInfo);

	void SetBlurToDevice();

	//放射ブラー用
	void SetRadialBlurToDevice();

	//グリッチ用
	void SetGlitchToDevice();
	//合成用
	void SetCombineToDevice();

	void SetDoFToDevice();
	void SetBrightToDevice();

	ID3D11VertexShader* m_VS = nullptr;
	ID3D11InputLayout* m_inputLayout = nullptr;

	ID3D11PixelShader* m_PS_Blur = nullptr;
	ID3D11PixelShader* m_PS_RBlur = nullptr;		//放射ブラー用
	ID3D11PixelShader* m_PS_Glitch = nullptr;		//グリッチ用
	ID3D11PixelShader* m_PS_Combine = nullptr;		//合成用
	ID3D11PixelShader* m_PS_DoF = nullptr;
	ID3D11PixelShader* m_PS_Bright = nullptr;

	static const int kBlurSamplingRadius = 8;
	static const int kLightBloomSamplingRadius = 4;

	static const int kMaxSampling = 31;
	struct cbBlur
	{
		Math::Vector4 Info[kMaxSampling];

		int SamplingNum = 0;
		int _blank[3] = { 0, 0 ,0 };
	};
	KdConstantBuffer<cbBlur>	m_cb0_BlurInfo;

	//放射ブラー用
	struct cbRadialBlur
	{
		int samples = 0;
		float strength = 0.0f;
		Math::Vector2 center = { 0.5f,0.5f };

		float mask = 0.0f;
		int  dither = 1;
		float vortex = 0.0f;
		int _blank = 0;
	};
	KdConstantBuffer<cbRadialBlur> m_cb0_RadialBlurInfo;

	//グリッチ用
	struct cbGlitch
	{
		Math::Vector2 resolution = { 0.0f,0.0f };
		float time = 0.0f;
		float frameRate = 0.0f;

		float frequency = 0.0f;
		int useGrid = 0;
		int enable = 0;
		int playerHit = 0;

		Math::Vector2 center = { 0.0f,0.0f };
		int _blank2[2] = { 0,0 };
	};
	KdConstantBuffer<cbGlitch> m_cb0_GlitchInfo;

	//合成用
	struct cbCombine
	{
		int Switch = 0;
		int _blank[3] = { 0,0,0 };
	};
	KdConstantBuffer<cbCombine>	m_cb0_CombineInfo;



	struct cbDepthOfField
	{
		float NearClippingDistance = 0.0f;
		float FarClippingDistance = 1000.0f;

		float FocusDistance = 0.0f;
		float FocusForeRange = 0.0f;
		float FocusBackRange = 1000.0f;
		int   _blank[3] = { 0, 0, 0 };
	};
	KdConstantBuffer<cbDepthOfField>	m_cb0_DoFInfo;

	struct cbBrightFilter
	{
		float Threshold = 0.0f;
		int _blank[3] = { 0, 0, 0 };
	};
	KdConstantBuffer<cbBrightFilter>	m_cb0_BrightInfo;

	KdRenderTargetPack	m_postEffectRTPack;

	KdRenderTargetPack	m_blurRTPack;
	KdRenderTargetPack	m_strongBlurRTPack;

	//放射ブラー用
	KdRenderTargetPack m_radialBlurRTPack;
	//グリッチ用
	KdRenderTargetPack m_glitchRTPack;
	//合成用
	KdRenderTargetPack m_combineRTPack;


	KdRenderTargetPack	m_depthOfFieldRTPack;

	KdRenderTargetPack	m_brightEffectRTPack;
	static const int	kLightBloomNum = 4;
	KdRenderTargetPack	m_lightBloomRTPack[kLightBloomNum];

	KdRenderTargetChanger m_postEffectRTChanger;
	KdRenderTargetChanger m_brightRTChanger;

	Vertex m_screenVert[4];
};
