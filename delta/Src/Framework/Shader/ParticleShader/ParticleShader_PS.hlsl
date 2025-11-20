#include "inc_ParticleShader.hlsli"

SamplerState g_ss : register(s0);

StructuredBuffer<Particle> g_Particles : register(t0);

float4 main(GSOutput In) : SV_Target0
{
	float4 OutColor = In.Color;
	return OutColor;
}
