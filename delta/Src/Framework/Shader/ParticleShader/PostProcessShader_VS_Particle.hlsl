struct VSInput
{
	float3 pos : POSITION;
	float3 vel : VELOCITY;
	float life : LIFE;
	float size : SIZE;
	float color : COLOR;
};

struct VSOutput
{
	float3 pos : SV_Position;
	float3 vel : VELOCITY;
	float life : LIFE;
	float size : SIZE;
	float4 color : COLOR;
};

float4 main(VSInput In) : SV_POSITION
{
	return In;
}
