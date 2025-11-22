#include "inc_ParticleShader.hlsli"

RWStructuredBuffer<Particle> g_Particles : register(u0);

// Wang-hash ベースの高速擬似乱数（uint -> uint）
uint WangHash(uint x)
{
	x = (x ^ 61u) ^ (x >> 16);
	x *= 9u;
	x = x ^ (x >> 4);
	x *= 0x27d4eb2du;
	x = x ^ (x >> 15);
	return x;
}

// uint -> [0,1)
float Rand01(uint seed)
{
	uint h = WangHash(seed);
	// 最大値で割る（4294967295 = 0xFFFFFFFF）
	return (float) h / 4294967295.0f;
}

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint id = DTid.x;

	if (id >= g_maxParticles)
	{
		return;
	}
	  // NOTE: Dispatch の範囲をチェックして必ず id < maxParticles にすること（CPU側）
    g_Particles[id].pos += g_Particles[id].vel * 100.0f * g_deltaTime;
	g_Particles[id].life -= g_deltaTime;
	
	if (g_Particles[id].life <= 0.0f)
	{
			// リスポーン：id と外部シードを組み合わせて個別乱数を生成
		uint seedBase = id + g_randomSeed;
		float rx = (Rand01(seedBase + 17u) - 0.5f) * g_spawnRange.x;
		rx = clamp(rx, -1, 1);
		float ry = (Rand01(seedBase + 31u) - 0.5f) * g_spawnRange.y;
		ry = clamp(ry, -1, 1);
		float rz = (Rand01(seedBase + 47u) - 0.5f) * g_spawnRange.z;
		rz = clamp(rz, -1, 1);

		float size = 1.0f + Rand01(seedBase + 59u) * 5.0f;
		float life = 1.0f + Rand01(seedBase + 31u) * 4.0f;
				
		
		// リスポーン
		g_Particles[id].pos = g_targetPos;
		g_Particles[id].vel = float3(rx + g_gravity.x, ry, rz );
		g_Particles[id].life = 0.15f * life;
		g_Particles[id].size =  0.0021f*size;
		g_Particles[id].color = float4(1.0f, 0.0f, 0.0f, 1.0f);
	}
}
