#include "RandomNumberGenerator.hpp"

#include <stdlib.h>

int RandomNumberGenerator::RollRandomIntLessThan(int maxNotInclusive)
{
	return static_cast<int>(RollRandomFloatZeroToOneExclusive() * static_cast<float>(maxNotInclusive));
}

int RandomNumberGenerator::RollRandomIntInRange(int minInclusive, int maxInclusive)
{
	return static_cast<int>(static_cast<float>(minInclusive) + RollRandomFloatZeroToOneExclusive() * static_cast<float>(maxInclusive + 1 - minInclusive));
}

float RandomNumberGenerator::RollRandomFloatZeroToOne()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX); // [0, 1]
}

float RandomNumberGenerator::RollRandomFloatInRange(float minInclusive, float maxInclusive)
{
	return minInclusive + RollRandomFloatZeroToOne() * (maxInclusive - minInclusive);
}

float RandomNumberGenerator::RollRandomFloatZeroToOneExclusive()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX + 1); // [0, 1)
}

bool RandomNumberGenerator::RollRandomBool()
{
	return rand() < RAND_MAX / 2;
}
