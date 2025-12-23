//Contains definitions of basic data types.

#pragma once

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

#include <xmmintrin.h>
#include <float.h>

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

