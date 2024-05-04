#pragma once

namespace graphics {

// Simple error handling for drawing graphics
enum GraphicsErrorId {
    ERR_UNKNOWN_SHAPE       = 0x1,
    ERR_UNKNOWN_COORD_TYPE  = 0x2,
};

// Sets an error flag.
void set_error(GraphicsErrorId error);

// Clears all error flags.
void clear_errors();

// Checks current error flags and draws appropriate error messages to the screen.
// If NDEBUG is defined (i.e. release builds), this function does nothing.
void draw_error_messages();

}
