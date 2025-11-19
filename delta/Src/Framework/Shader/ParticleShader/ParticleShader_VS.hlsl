#include "inc_ParticleShader.hlsli"

VSOutput main(
float4 Pos : POSITION,
float3 Vel : VELOCITY,
float Life : LIFE,
float Size : SIZE,
float4 Color : COLOR
)
{
	VSOutput Out;
	Out.Pos = mul(Pos, g_mWorld); // ローカル座標系 -> ワールド座標系へ変換
	Out.wPos = Out.Pos.xyz; // ワールド座標を別途保存
	Out.Color = float4(1,0,0,1);
	Out.Vel = Vel;
	Out.Life =Life;
	Out.Size = Size;
	
	return Out;
}
