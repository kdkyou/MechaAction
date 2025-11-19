#include "inc_ParticleShader.hlsli"

Texture2D g_inputTex : register(t0);
SamplerState g_ss : register(s0);

float4 main(GSOutput In) : SV_Target0
{
	float4 outColor = In.Color;
	outColor *= In.Color;

	return outColor;

}
