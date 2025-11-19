
cbuffer cbScene : register(b0)
{
	float4x4 g_mViewProj;
	float3 g_camRight;
	float3 g_camUp;
}

struct VSOutput
{
	float3 pos : POSITION;
	float3 vel : VELOCITY;
	float life : LIFE;
	float size : SIZE;
	float4 color : COLOR;
};


// 頂点1つから4つの頂点を出力してクワッドを描画
[maxvertexcount(4)]
void main(point VSOutput In[1], inout TriangleStream<GSOutput> triStream)
{
	float3 right = g_camRight * In[0].size;
	float3 up = g_camUp * In[0].size;

	float3 center = In[0].pos;
	float lifeFade = saturate(In[0].life);

	float3 corners[4] =
	{
		center + (-right + up), // 左上
        center + (right + up), // 右上
        center + (-right - up), // 左下
        center + (right - up) // 右下
	};

	float2 uvs[4] =
	{
		float2(0, 0),
        float2(1, 0),
        float2(0, 1),
        float2(1, 1)
	};

    // カラー補正（寿命によるフェード）
	float4 color = In[0].color;
	color.a *= lifeFade;

    [unroll]
	for (int i = 0; i < 4; i++)
	{
		GSOutput o;
		o.pos = mul(float4(corners[i], 1), g_mViewProj);
		o.uv = uvs[i];
		o.color = color;
		triStream.Append(o);
	}
}
