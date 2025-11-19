
cbuffer cbScene : register(b0)
{
	float3 g_camRight;
	float3 g_camUp;
	float4x4 g_mWorld;
}

struct VSInput
{
	float4 Pos : POSITION;
	float3 Vel : VELOCITY;
	float Life : LIFE;
	float Size : SIZE;
	float4 Color : COLOR;
};

struct VSOutput
{
	float4 Pos : SV_Position;
	float3 wPos  : TEXCOORD0;	// ワールド3D座標
	float3 Vel : TEXCOORD1;
	float Life : TEXCOORD2;
	float Size : TEXCOORD3;
	float4 Color : TEXCOORD4;
};

struct GSOutput
{
	float4 Pos : SV_Position;
	float2 UV : TEXCOORD0;
	float4 Color : TEXCOORD4;
};


