#pragma once
#include "../../types.h"

// Interface for debug plugins (e.g. aise_debug.qdp) to hook frame presentation:
class FrameOverlay {
public:
    // (pixels, width, height, pitch): renders overlay onto screen buffer before presentation.
    using DrawProc = void(__cdecl*)(u8*, u32, u32, s32);

    // Non-zero drops this frame entirely (e.g. fast forward).
    using SkipProc = u32(__cdecl*)();

    void set_procs(DrawProc draw, SkipProc skip);

    bool registered() const;
    bool skip() const;
    void draw(u8* pixels, u32 width, u32 height, s32 pitch) const;

private:
    DrawProc m_draw = nullptr;
    SkipProc m_skip = nullptr;
};

inline FrameOverlay g_frame_overlay;
