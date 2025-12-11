#include"inc_KdPostProcessShader.hlsli"

Texture2D g_inputTex : register(t0);
SamplerState g_ss : register(s0);

cbuffer cb : register(b0)
{
	int g_samples;
	float g_strength;
	float2 g_center;
	float g_mask;
	int  g_dither;
	float g_vortex;
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
	float3 color = 0;

	if(g_samples<2)
	{
		color += g_inputTex.Sample(g_ss, input.UV).rgb;
		return float4(color,1);
	}
	
	//中心を基準に
	float2 pos = input.UV - g_center;
	//中心からの距離
	float dist = length(pos);
	//中心からの角度
	float angle = atan2(pos.y,pos.x);
	
	float factor = g_strength / float(g_samples) * dist;

	//ディザリング
	float dither = 0.0f;
	
	if(g_dither)
	{
		dither = InterleavedGradientNoise(input.Pos.xy, 0);
	}


	//ブラーが適用されない範囲を計算。0.1の範囲をぼかす
	factor *= smoothstep(g_mask - 0.1, g_mask, dist);

	//サンプリング数の逆数
	float srcSampleCount = 1.0f / g_samples;
	
	//サンプリング回数分
	for (int i = 0; i < g_samples;i++)
	{
		float uvOffset = 0.0f;
		//ディザリングが有効時
		if (g_dither)
		{
			uvOffset = (i + dither) * srcSampleCount ;
			//サンプリング回数分中心点に向かって位置を変えながらテクスチャをサンプリング
			color += g_inputTex.Sample(g_ss, pos * lerp(1,1-g_strength,uvOffset) + g_center).rgb;
		}
		else
		{
			uvOffset = 1.0 - factor * float(i);
			float2 ofs = float2(dist * cos(angle + g_vortex * uvOffset), dist * sin(angle + g_vortex * uvOffset));
			color += g_inputTex.Sample(g_ss, ofs * uvOffset + g_center).rgb;
		}
	}
		color /= float(g_samples);
	
	return float4(color, 1);
}

