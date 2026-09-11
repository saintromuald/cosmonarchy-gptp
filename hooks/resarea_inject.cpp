#include "resarea.h"
#include "hook_tools.h"

namespace hooks {
	void inject_resarea_hooks() {
		// CHK_UNIT @ 4CD7A0
		memory_patch(0x004CD864, (u8)0xD3);//Change destination register from EAX to EDX.
		call_patch(encode_start_location, 0x004CD86A);
		// GameInit @ 4EEE00
		call_patch(create_resareas, 0x004EEFB6);
	}
}