#pragma once
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <float.h>
#include <concepts>

#include <fpm/fixed.hpp>
#include <fpm/math.hpp>
#include <fpm/ios.hpp>
#include <boost/int128/int128.hpp>
#include <boost/int128/limits.hpp>

//Based on http://msdn.microsoft.com/en-us/library/windows/desktop/ms679289.aspx
#define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]

//Include <algorithm> to use this
#define CLAMP(x, minVal, maxVal) (std::min(std::max((x), (minVal)), (maxVal)))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifdef _MSC_VER
typedef double				f64;
typedef float				f32;

typedef unsigned __int64	u64;
typedef unsigned __int32	u32;
typedef unsigned __int16	u16;
typedef unsigned __int8		u8;

typedef __int64				s64;
typedef __int32				s32;
typedef __int16				s16;
typedef __int8				s8;

typedef u64					b64;
typedef u32					b32;
typedef u16					b16;
typedef u8					b8;

typedef u8					unk_t;
typedef unsigned int		UINT;
#endif

// Must be called on all game simulation threads for floats to not desync -Veeq7
inline void init_float_precision() {
    // Disable x87 extended precision
    _controlfp(_PC_53, _MCW_RC);

    // Round to nearest
    _controlfp(_RC_NEAR, _MCW_PC);

    // Mask all FP exceptions
    _controlfp(_MCW_EM, _MCW_EM);
    
    // SSE: flush denormals
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
}

using fp32 = fpm::fixed<s32, s64, 12>;
using fp64 = fpm::fixed<s64, boost::int128::int128_t, 32>;

inline consteval fp32 operator""fp(unsigned long long f) { return fp32{s32(f)}; }
inline consteval fp64 operator""dfp(unsigned long long f) { return fp64{s64(f)}; }
inline consteval fp32 operator""fp(long double f) { return fp32{f}; }
inline consteval fp64 operator""dfp(long double f) { return fp64{f}; }

inline consteval u32 operator""tiles(long double tiles) { return static_cast<u32>(tiles * 32); }
inline consteval u32 operator""tiles(unsigned long long tiles) { return static_cast<u32>(tiles * 32); }

inline consteval u32 operator""secs(long double seconds) { return static_cast<u32>(seconds * 24); }
inline consteval u32 operator""secs(unsigned long long seconds) { return static_cast<u32>(seconds * 24); }

inline consteval u32 operator""mins(long double minutes) { return static_cast<u32>(minutes * 60 * 24); }
inline consteval u32 operator""mins(unsigned long long minutes) { return static_cast<u32>(minutes * 60 * 24); }

inline consteval u32 operator""hp(long double value) { return static_cast<u32>(value * 256); }
inline consteval u32 operator""hp(unsigned long long value) { return static_cast<u32>(value * 256); }

template <typename... Ts>
struct Overloaded : Ts... { using Ts::operator()...; };
