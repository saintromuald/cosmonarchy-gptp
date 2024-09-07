#pragma once

#define Min(a, b) ((a) < (b) ? a : b)
#define Max(a, b) ((a) > (b) ? a : b)
#define Clamp(x, min, max) ((x) < (min) ? (min) : (x) > (max) ? (max) : (x))

inline namespace MathUtils {

constexpr f64 Normalize(f64 value, f64 min, f64 max, bool do_clamp = false) {
	f64 result = ((f64)value - (f64)min) / ((f64)max - (f64)min);
	if (do_clamp) {
		result = Clamp(result, 0.0, 1.0);
	}
	return result;
}

constexpr f64 Lerp(f64 fraction, f64 min, f64 max, bool do_clamp = false) {
	f64 result = (fraction * (max - min)) + min;
	if (do_clamp) {
		result = Clamp(result, min, max);
	}
	return result;
}

constexpr f64 Map(f64 value, f64 smin, f64 smax, f64 tmin, f64 tmax, bool do_clamp = false) {
	return Lerp(Normalize(value, smin, smax, do_clamp), tmin, tmax, false);
}

};