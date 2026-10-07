#include "frame_overlay.h"

void FrameOverlay::set_procs(DrawProc draw, SkipProc skip) {
    m_draw = draw;
    m_skip = skip;
}

bool FrameOverlay::registered() const {
    return m_draw != nullptr;
}

bool FrameOverlay::skip() const {
    return m_skip && m_skip();
}

void FrameOverlay::draw(u8* pixels, u32 width, u32 height, s32 pitch) const {
    if (m_draw) {
        m_draw(pixels, width, height, pitch);
    }
}
