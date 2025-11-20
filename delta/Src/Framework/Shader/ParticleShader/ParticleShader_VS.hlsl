#include "inc_ParticleShader.hlsli"
#include "../inc_KdCommon.hlsli"

StructuredBuffer<Particle> g_Particles : register(t0);

VSOutput main(uint vertexID : SV_VertexID)
{
	
	VSOutput Out;
	Particle p = g_Particles[vertexID];

	//ワールド行列
	float4 pos = float4(p.pos, 1);
	pos = mul(pos, g_mView); //ビュー変換 
	pos = mul(pos, g_mProj); // 射影変換
	
	
	Out.Pos = pos;
	Out.Color = p.color;
	Out.UV = float2(0.5f, 0.5f);
	Out.Size = p.size;
	Out.lPos = pos;

	return Out;
}
