#pragma once

struct KdEase
{
	inline float InSine(float progress);

	inline float OutSine(float progress){ return sin((progress * DirectX::XM_PI) * 0.5f);}
	
	inline float InOutSine(float progress);

	inline float OutBounce(float progress);
};
