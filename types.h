//Contains definitions of basic data types.

#pragma once

//V241 for VS2008

//Based on http://msdn.microsoft.com/en-us/library/windows/desktop/ms679289.aspx
#define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]

//Include <algorithm> to use this
#define CLAMP(x, minVal, maxVal) (std::min(std::max((x), (minVal)), (maxVal)))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifdef _MSC_VER
typedef float             float32;
typedef double            float64;
typedef unsigned __int32  uint32;
typedef unsigned __int16  uint16;
typedef unsigned __int8   uint8;
typedef __int32           int32;
typedef __int16           int16;
typedef __int8            int8;
typedef __int32           bool32;
typedef __int8            bool8;
typedef uint8             unknown_byte;
#endif
