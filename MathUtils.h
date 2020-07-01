#pragma once

static struct MathUtils {
	template<typename T>
	static T clamp(T value, T min, T max);

	template<typename T>
	static double normalize(T value, T min, T max, bool doClamp = false);

	template<typename T>
	static T lerp(double fraction, T min, T max, bool doClamp = false);

	template<typename sourceType, typename targetType>
	static targetType map(sourceType value, sourceType sourceMin, sourceType soureMax, targetType targetMin, targetType targetMax, bool doClamp = false);
};

template<typename T>
T MathUtils::clamp(T value, T min, T max) {
	return value < min ? min : value > max ? max : value;
}

template<typename T>
double MathUtils::normalize(T value, T min, T max, bool doClamp) {
	double returnValue = ((value - min) / (max - min));
	if (doClamp) returnValue = MathUtils::clamp(returnValue, 0.0, 1.0);
	return returnValue;
}

template<typename T>
T MathUtils::lerp(double fraction, T min, T max, bool doClamp) {
	T returnValue = (fraction * (max - min)) + min;
	if (doClamp) returnValue = MathUtils::clamp(returnValue, min, max);
	return returnValue;
}

template<typename sourceType, typename targetType>
targetType MathUtils::map(sourceType value, sourceType sourceMin, sourceType sourceMax, targetType targetMin, targetType targetMax, bool doClamp) {
	return (targetType)MathUtils::lerp(MathUtils::normalize(value, sourceMin, sourceMax, doClamp), targetMin, targetMax);
}

