#include "inc_ParticleShader.hlsli"
#include "../inc_KdCommon.hlsli"



[maxvertexcount(4)]
void main(
	point VSOutput In[1], 
	inout TriangleStream< GSOutput > stream
)
{
	// 中心点の取得
	float3 right = g_camRight * 0.5f;
	float3 up = g_camUp * 0.5f;
	float3 c = In[0].Pos;

	// 四隅の座標作成
	float3 corner[4] =
	{
		c + (-right + up),
        c + (right + up),
        c + (-right - up),
        c + (right - up)
	};

	float2 uv[4] =
	{
		float2(0, 0),
		float2(1, 0),
		float2(0, 1),
		float2(1, 1)
	};

	float4x4 mViewProj = mul(g_mView, g_mProj);

	[unroll]
	for (int i = 0;i < 4;i++)
	{
		 GSOutput Out;
		Out.Pos = mul(float4(corner[i], 1), mViewProj);
		Out.UV = uv[i];
		Out.Color = float4(0,1,0,1);
		stream.Append(Out);
	}

	
}
