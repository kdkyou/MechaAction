#include "inc_ParticleShader.hlsli"

RWStructuredBuffer<Particle> g_Particles : register(u0);

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint id = DTid.x ;

	  // NOTE: Dispatch の範囲をチェックして必ず id < maxParticles にすること（CPU側）
    // ここでは簡潔のため省略（CPU側で ceil を使って dispatch 数を決める）	
	g_Particles[id].pos += g_Particles[id].vel * g_deltaTime;
	g_Particles[id].life -= g_deltaTime;
	
	if (g_Particles[id].life <= 0.0f)
	{
		// リスポーン
		g_Particles[id].pos = g_targetPos;
		g_Particles[id].vel = g_gravity;
		g_Particles[id].life = 5.0f;
		g_Particles[id].size = 60.0f;
		g_Particles[id].color = float4(0.0f, 5.0f, 0.0f, 1.0f);	
	}
}
