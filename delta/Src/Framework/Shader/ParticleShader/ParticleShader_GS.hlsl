#include "inc_ParticleShader.hlsli"
#include "../inc_KdCommon.hlsli"

StructuredBuffer<Particle> g_Particles : register(t0);

[maxvertexcount(6)]
void main(
	point VSOutput In[1],
	inout TriangleStream<GSOutput> stream
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
	//float4 corner[4] =
	//{
	//	c - right + up,
 //       c + right - up,
	//	c - right - up,
 //       c + right + up
	//};

	float2 uv[4] =
	{
		float2(0, 0),
		float2(1, 0),
		float2(0, 1),
		float2(1, 1)
	};

	for (int i = 0; i < 3; i++)
	{
		GSOutput Out = In[0];
	
		Out.Pos = corner[i];
		Out.UV = uv[i];
	//	Out.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
		stream.Append(Out);
	}
	stream.RestartStrip();

	for(int j = 1; j< 4; j++)
	{
		GSOutput Out = In[0];
	
		Out.Pos = corner[j];
		Out.UV = uv[j];
	//	Out.Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
		stream.Append(Out);
	}
	stream.RestartStrip();

}
