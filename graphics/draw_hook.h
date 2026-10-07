#pragma once

//V241 for VS2008

namespace hooks {

// Draws screen-space UI shapes (ON_SCREEN, ON_MOUSE, team UI) after world zoom resampling.
// Called by present_frame so UI is not magnified with the game layer.
void draw_deferred_shapes();

void inject_draw_hook();

} //hooks
