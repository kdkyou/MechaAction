#include "KdRandomGenerator.h"

int KdRandomGenerator::GetInt(int minValue, int maxValue)
{
	std::uniform_int_distribution<int> range(minValue, maxValue);
	return range(rnd);
}

float KdRandomGenerator::GetFloat(float minValue, float maxValue)
{
	std::uniform_real_distribution<float> range(minValue, maxValue);
	return range(rnd);
}