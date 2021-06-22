#pragma once
#include "logger.h"

// TODO: replace those with non template versions, also remove the namespace, no need for it

static struct MathUtils
{
	template<typename T>
	static T clamp(T value, T min, T max);

	template<typename T>
	static f64 normalize(T value, T min, T max, bool doClamp = false);

	template<typename T>
	static T lerp(f64 fraction, T min, T max, bool doClamp = false);

	template<typename T, typename Y>
	static Y map(T value, T sourceMin, T sourceMax, Y targetMin, Y targetMax, bool doClamp = false);
};

template<typename T>
T MathUtils::clamp(T value, T min, T max)
{
	if (min > max)
	{
		return MathUtils::clamp(value, max, min);
	}
	return value < min ? min : value > max ? max : value;
}

template<typename T>
f64 MathUtils::normalize(T value, T min, T max, bool doClamp)
{
	f64 returnValue = ((f64)value - (f64)min) / ((f64)max - (f64)min);
	if (doClamp)
	{
		returnValue = MathUtils::clamp(returnValue, 0.0, 1.0);
	}
	return returnValue;
}

template<typename T>
T MathUtils::lerp(f64 fraction, T min, T max, bool doClamp)
{
	T returnValue = (fraction * (max - min)) + min;
	if (doClamp)
	{
		returnValue = MathUtils::clamp(returnValue, min, max);
	}
	return returnValue;
}

template<typename T, typename Y>
Y MathUtils::map(T value, T sourceMin, T sourceMax, Y targetMin, Y targetMax, bool doClamp)
{
	return (Y)MathUtils::lerp(MathUtils::normalize(value, sourceMin, sourceMax, doClamp), targetMin, targetMax);
}