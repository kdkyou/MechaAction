#include"inc_KdPostProcessShader.hlsli"

Texture2D g_inputTex : register(t0);
SamplerState g_ss : register(s0);

cbuffer cb : register(b0)
{
	int g_samples;
	float g_strength;
	float g_mask;
	bool  g_dither;
	float2 g_center;
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
	//リターンさせる色
	float4 color = { 0.0f, 0.0f, 0.0f, 0.0f };

	
	//中心を基準に
	float2 pos = input.UV - g_center;
	//中心からの距離
	float dist = length(pos);
	float factor = g_strength / float(g_samples) * dist;

	//ディザリング
	float dither = 0.0f;
	if(g_dither==true)
	{
		dither = InterleavedGradientNoise(input.Pos.xy, 0);
	}

	//ブラーが適用されない範囲を計算。0.1の範囲をぼかす
	factor *= smoothstep(g_mask - 0.1, g_mask, dist);

	//サンプリング数の逆数
	float srcSampleCount = 1 / g_samples;
	
	//サンプリング回数分
	for (int i = 0; i < g_samples;i++)
	{
		float uvOffset = 0.0f;
		//ディザリングが有効時
		if (g_dither == true)
		{
			uvOffset = (i + dither) * srcSampleCount ;
			//サンプリング回数分中心点に向かって位置を変えながらテクスチャをサンプリング
			color += g_inputTex.Sample(g_ss, pos * lerp(1,1-g_strength,uvOffset) + g_center);
		}
		else
		{
			uvOffset = 1.0 - factor * float(i);
			color += g_inputTex.Sample(g_ss, pos * uvOffset + g_center);
		}

		

	}
		color /= float(g_samples);
	
		return color;
}

//cbuffer cb : register(b0)
//{
//	float3 g_sampleCountParams;
//	float g_intensity;
//	float2 g_radialCenter;
//	bool g_useDither;
//};


//float4 main(VSOutput input) : SV_Target0
//{
//	//出力結果
//	float4	output = 0;
//	//現在のUV座標から中心までの方向ベクトル算出
//	const float2 uv = input.UV - g_radialCenter;
//	//サンプリング回数
//	const float sampleCount = g_sampleCountParams.x;
//	//サンプリング回数の逆数
//	const float rcpSampleCount = g_sampleCountParams.y;

//	float dither = 0;
	
//	// z サンプリング回数-1の逆数
//	const float rcpSampleCountMinusOne = g_sampleCountParams.z;

	
//	if (g_useDither == true)	//ディザリングっぽくぼかす場合
//	{
//		dither = InterleavedGradientNoise(input.Pos.xy, 0);
//	}

//	//サンプリング回数分
//	for (int i = 0; i < sampleCount; i++)
//	{
//		float t = 0.0f;

//		//ディザリングが有効時
//		if (g_useDither == true)
//		{
//			t = (i + dither) * rcpSampleCount;

//		}
//		else
//		{
//			t = i * rcpSampleCountMinusOne;
//		}

//		//サンプリング回数分中心点に向かって位置を変えながらテクスチャをサンプリング
//		output += g_inputTex.Sample(g_ss, uv * lerp(1, 1 - g_intensity, t) + g_radialCenter);

//	}
//	output *= rcpSampleCount;
	
//	return output;
//}


