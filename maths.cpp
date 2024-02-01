#include "maths.h"

f32 clamp(f32 value, f32 min, f32 max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

f32 lerp(f32 t, f32 a, f32 b) {
	return a * (1 - t) + b * t;
}

f32 ease_in_out(f32 t) {
	f32 ease_in = t * t;
	f32 ease_out = 1 - (1 - t) * (1 - t);
	return lerp(t, ease_in, ease_out);
}

f32 smoothstep(f32 t, f32 a, f32 b) {
	return lerp(ease_in_out(t), a, b);
}

