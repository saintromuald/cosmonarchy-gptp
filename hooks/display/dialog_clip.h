#pragma once

// Dialog system clip boxes (default 640x480):
// - 0051A16C: Exclusive clip box for dialog refresh regions (clamped in 0041C200, drawn in 0041CB50).
// - 0051A15C: Inclusive clip box for mouse refresh rectangles (clamped in 0041CA00).
namespace hooks {

void inject_dialog_clip_hooks();

} // hooks
