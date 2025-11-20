struct Particle
{
	float3 pos;
	float life;
	float3 vel;
	float size;
	float4 color;
};


cbuffer cbFrame : register(b0)
{
	float g_deltaTime;
	float3 g_gravity;
	float3 g_targetPos;
	int g_maxParticles;
	int g_randomSeed;
	float3 g_spawnRange;
};

cbuffer cbScene : register(b1)
{
	float3 g_camRight;
	float3 g_camUp;
	float4x4 g_mWorld;
}

struct VSInput
{
	float3 pos : POSITION;
	float size : SIZE;
	float4 color : COLOR;
};

struct VSOutput
{
	float4 Pos : SV_Position;
	float4 Color : COLOR;
	float2 UV : TEXCOORD0;
	float Size : SIZE;
	float3 lPos : TEXCOORD1;
};

struct GSOutput
{
	float4 Pos : SV_Position;
	float4 Color : COLOR;
	float2 UV : TEXCOORD0;
	float Size : SIZE;
	float3 lPos : TEXCOORD1;
};




