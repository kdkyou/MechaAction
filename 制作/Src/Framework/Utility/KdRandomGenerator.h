#pragma once

#include <random>

class KdRandomGenerator
{
private:
	std::mt19937_64 rnd;

public:

	KdRandomGenerator()
	{
		std::random_device rd;
		rnd = std::mt19937_64(rd());
	}

	int		GetInt(int minValue, int maxValue);
	float	GetFloat(float minValue, float maxValue);
};