#include"inc_KdPostProcessShader.hlsli"

Texture2D g_inputTex : register(t0);	//Combine
Texture2D g_radialTex : register(t1);	//ReaialBlur
Texture2D g_glitchTex : register(t2);	//Glitch

SamplerState g_ss : register(s0);

cbuffer cb : register(b0)
{
	int g_switch;	//描画項目の変更フラグ
	
};

float4 main(VSOutput In) : SV_TARGET
{

	float3 color = 0;

	
	//無し
	if(g_switch ==  0)
	{
		color += g_inputTex.Sample(g_ss, In.UV).rgb;
		
	}
	//RadialBlurのみ
	else if (g_switch == 1)
	{
		color += g_radialTex.Sample(g_ss, In.UV).rgb;
	}
	//Glitchのみ
	else if (g_switch == 2)
	{
		color  += g_glitchTex.Sample(g_ss, In.UV).rgb;
	}
	//加算合成
	else if(g_switch == 3)
	{
		float3 color1 = g_radialTex.Sample(g_ss, In.UV).rgb;
		float3 color2 = g_glitchTex.Sample(g_ss, In.UV).rgb;
		
		color  += saturate(color1 + color2);
	}
	//半合成
	else if(g_switch == 4)
	{
		
	}

	return float4(color,1);
	
}
