Texture2D g_particleTex : register(t0);
SamplerState g_ss : register(s0);

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float4 color : COLOR;
};

float4 main(GSOutput In) : SV_TARGET
{
	float4 texCol = g_particleTex.Sample(g_ss, In.uv);
	return texCol * In.color;
}
