#include"inc_KdPostProcessShader.hlsli"

Texture2D g_inputTex : register(t0);
SamplerState g_ss : register(s0);

cbuffer cb : register(b0)
{
	half3 g_sampleCountParams;
	half g_intensity;
	half2 g_radialCenter;
	bool g_useDither;
};

float InterleavedGradientNoise(float2 pixCoord, int frameCount)
{
	const float3 magic = float3(0.06711056f, 0.00583715f, 52.9829189f);
	float2 frameMagicScale = float2(2.083f, 4.867f);
	pixCoord += frameCount * frameMagicScale;
	return frac(magic.z * frac(dot(pixCoord, magic.xy)));
}

float4 main(VSOutput input) : SV_Target0
{
	//出力結果
	half4 output = 0;
	//現在のUV座標から中心までの方向ベクトル算出
	const float2 uv = input.UV - g_radialCenter;
	//サンプリング回数
	const half sampleCount = g_sampleCountParams.x;
	//サンプリング回数の逆数
	const half rcpSampleCount = g_sampleCountParams.y;

	half dither = 0;
	
	// z サンプリング回数-1の逆数
	const half rcpSampleCountMinusOne = g_sampleCountParams.z;

	
	if (g_useDither == true)	//ディザリングっぽくぼかす場合
	{
		dither = InterleavedGradientNoise(input.Pos.xy, 0);
	}

	//サンプリング回数分
	for (int i = 0; i < sampleCount;i++)
	{
		float t = 0.0f;

		//ディザリングが有効時
		if (g_useDither == true)
		{
			t = (i + dither) * rcpSampleCount;

		}
		else
		{
			t = i * rcpSampleCountMinusOne;
		}

		//サンプリング回数分中心点に向かって位置を変えながらテクスチャをサンプリング
		output += g_inputTex.Sample(g_ss, uv * lerp(1, 1 - g_intensity, t) + g_radialCenter);

	}
	output *= rcpSampleCount;
	
		return output;
}


