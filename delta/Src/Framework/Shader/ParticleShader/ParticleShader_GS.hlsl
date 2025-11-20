#include "inc_ParticleShader.hlsli"
#include "../inc_KdCommon.hlsli"

StructuredBuffer<Particle> g_Particles : register(t0);

[maxvertexcount(4)]
void main(
	point VSOutput In[1],
	inout LineStream<GSOutput> stream
)
{
	// 中心点の取得
	float4 c = In[0].Pos;
	float size = In[0].Size * 0.5f;

	float4 right = float4(g_camRight * size, 1);
	float4 up = float4(g_camUp * size, 1);

	// float4x4 mViewProj = mul(g_mView, g_mProj);

	
	// 四隅の座標作成
	float4 corner[4] =
	{
		c - right + up,
        c + right + up,
        c - right - up,
        c + right - up
	};

	float2 uv[4] =
	{
		float2(0, 0),
		float2(1, 0),
		float2(0, 1),
		float2(1, 1)
	};

	
	for (int i = 0; i < 4; i++)
	{
		GSOutput Out = In[0];
		float4 clip = corner[i];
	
		Out.Pos = clip;
		Out.UV = uv[i];
		stream.Append(Out);
	}
	stream.RestartStrip();

}
