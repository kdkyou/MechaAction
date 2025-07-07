#include"inc_KdPostProcessShader.hlsli"

Texture2D g_inputTex : register(t0);
SamplerState g_ss : register(s0);

cbuffer cb : register(b0)
{
	float2  g_resolution;	//分割数
	float g_time;			//時間
	float g_frameRate;		//フレームレート相当値
	float g_frequency;		//頻度
	int g_useGrid;			 //グリッド処理をするか
	int g_enable;			//0=無効　1＝有効
	int g_playerHit;		//プレイヤーが受けたかどうか
	float2 g_center;		//範囲の中心
};

//ランダムな値を返す
float rand(float2 co)
{
	return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

//パーリンノイズ
float perlinNoise(float2 st)
{
	float2 p = floor(st);
	float2 f = frac(st);
	float2 u = f * f * (3.0f - 2.0f * f);

	float v00 = rand(p + float2(0, 0));
	float v10 = rand(p + float2(1, 0));
	float v01 = rand(p + float2(0, 1));
	float v11 = rand(p + float2(1, 1));

	return lerp(lerp(v00, v10, u.x), lerp(v01, v11, u.x), u.y);

}


float4 main(VSOutput In) : SV_Target0
{
	float2 uv = In.UV;

	float3 color = 0;

	
	if(g_useGrid == 1)
	{
		// === グリッド演出 ===
		float2 gridSize = g_resolution;
		float2 uvGrid = floor(uv * gridSize) / gridSize;

        // ランダムにグリッド内を点滅させる（ノイズ演出）
		float2 cellID = floor(uv * gridSize);
		float id = dot(cellID, float2(12.9898, 78.233));
		float r = frac(sin(id + g_time) * 43758.5453);

		color += g_inputTex.Sample(g_ss, uvGrid).rgb;
		color.rgb *= r; // チカチカ演出
	}
	else
	{
		//時間ベースのノイズ生成
		float noiseTime1 = perlinNoise(float2(sin(g_time), cos(g_time)) * 10.0f);
		float noiseTime2 = perlinNoise(float2(sin(g_time), cos(g_time)) * 5.0f);

		//ポスタライズ	再度確認
		float posterize1 = floor(frac(noiseTime1 * 10.0f) / (1.0f / g_frameRate)) * (1.0f / g_frameRate);
		float posterize2 = floor(frac(noiseTime2 * 5.0f) / (1.0f / g_frameRate)) * (1.0f / g_frameRate);

		//uv.x方向のノイズ計算　-0.1 < noiseX < 0.1
		float noiseX = (2.0f * rand(float2(posterize1, posterize2)) - 0.5f) * 0.1f;

		//step(t.x)はxがtより大きい場合1を返す
		//float f = step(rand(float2(posterize2, posterize1)), g_frequency);
		float f = step(rand(posterize2), g_frequency);
		noiseX *= f;

		//uv.y方向のノイズ計算　-1 < noiseY < 1
		//float noiseY = (2.0 * rand(float2(posterize1, posterize1))) - 0.5;
		float noiseY = (2.0 * rand(posterize1)) - 0.5;

		//グリッチの高さの補間値計算　どの高さに出現するかは時間変化でランダム
		//float glitchLine1 = step(uv.y - noiseY, rand(float2(noiseY, noiseY)));
		float glitchLine1 = step(uv.y - noiseY, rand(noiseY));
		float glitchLine2 = step(uv.y + noiseY, noiseY);
		float glitch = saturate(glitchLine1 - glitchLine2);

		if(g_playerHit==1)
		{
			
			//指定範囲にマスクをしてグリッチ
			float2 center = g_center;
			float dist = distance(uv, center);
			float effectRadius = 0.6; // 半径50%
			float glitchMask = smoothstep(effectRadius - 0.2, effectRadius + 0.05, dist);
			//glitch *= 1.0f- glitchMask;
			glitch *= glitchMask;

		//速度調整
			uv.x = lerp(uv.x, uv.x + noiseX, glitch);

		//テクスチャのサンプリング
			color += g_inputTex.Sample(g_ss, uv).rgb;

			float glitchOffset = noiseX * glitch;

			// チャンネルごとにずらしてサンプリング
			float r = g_inputTex.Sample(g_ss, uv + float2(glitchOffset, 0)).r;
			float g = g_inputTex.Sample(g_ss, uv).g;
			float b = g_inputTex.Sample(g_ss, uv - float2(glitchOffset, 0)).b;
		
			color.rgb += float3(r, g, b);

			//狂わした色を反転
			//color.rgb = lerp(color.rgb, 1.0 - color.rgb, glitch);

			
			
		}
		else
		{
			//速度調整
			uv.x = lerp(uv.x, uv.x + noiseX, glitch);

			//テクスチャのサンプリング
			color += g_inputTex.Sample(g_ss, uv).rgb;
		}


	}
	
	
	return float4(color, 1);
}
