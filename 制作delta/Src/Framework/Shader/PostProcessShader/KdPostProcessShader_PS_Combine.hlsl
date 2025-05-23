#include"inc_KdPostProcessShader.hlsli"

Texture2D g_inputTex : register(t0);
SamplerState g_ss : register(s0);

float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
