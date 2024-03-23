#include "hooks/limits/sfx_extender.h"
#include "globals.h"

namespace sfx_dat {
	u8* volume;
	u8* sound_flags;
	u16* portrait_duration;
	u8* type;
	char** sound_file;
	SoundData* sound_data;
}

namespace hooks
{
	void sfx_extender_get_count() {
		iquare_globals.extended_sfx = 1;
	}

	void new_sfx_extender() {
		NEW_SOUND_TYPE_COUNT = SoundIdLimit; // Define your length here (change later to custom func reading sfxdata)
		sfx_dat::volume = new u8[NEW_SOUND_TYPE_COUNT];
		sfx_dat::sound_flags = new u8[NEW_SOUND_TYPE_COUNT];
		sfx_dat::portrait_duration = new u16[NEW_SOUND_TYPE_COUNT];
		sfx_dat::type = new u8[NEW_SOUND_TYPE_COUNT];
		sfx_dat::sound_file = new char*[NEW_SOUND_TYPE_COUNT];
		sfx_dat::sound_data = new SoundData[NEW_SOUND_TYPE_COUNT];
		memset(sfx_dat::sound_data, 0, sizeof(SoundData) * NEW_SOUND_TYPE_COUNT);
		memory_patch(0x0045F042, NEW_SOUND_TYPE_COUNT);
		memory_patch(0x004BBC2D, NEW_SOUND_TYPE_COUNT);
		memory_patch(0x004BBCA5, NEW_SOUND_TYPE_COUNT - 1);
		memory_patch(0x004BBEB7, NEW_SOUND_TYPE_COUNT - 1);
		memory_patch(0x004BCA98, NEW_SOUND_TYPE_COUNT);
		memory_patch(0x004BCD2A, NEW_SOUND_TYPE_COUNT);
		memory_patch(0x004DB0FA, NEW_SOUND_TYPE_COUNT);
		memory_patch(0x005154A0, NEW_SOUND_TYPE_COUNT);
		memory_patch(0x005154AC, NEW_SOUND_TYPE_COUNT);
		memory_patch(0x005154B8, NEW_SOUND_TYPE_COUNT);
		memory_patch(0x005154C4, NEW_SOUND_TYPE_COUNT);
		memory_patch(0x005154D0, NEW_SOUND_TYPE_COUNT);
		memory_patch(0x00455BDC, sfx_dat::volume);
		memory_patch(0x0048D0F0, ((u32)sfx_dat::volume) + 0x17);
		memory_patch(0x0048D16C, ((u32)sfx_dat::volume) + 2);
		memory_patch(0x0048D409, ((u32)sfx_dat::volume) + 0x17);
		memory_patch(0x0048ECED, sfx_dat::volume);
		memory_patch(0x0048EDD7, sfx_dat::volume);
		memory_patch(0x0048EE5C, sfx_dat::volume);
		memory_patch(0x0048EFA5, sfx_dat::volume);
		memory_patch(0x0048F028, sfx_dat::volume);
		memory_patch(0x0048F0CA, sfx_dat::volume);
		memory_patch(0x0048F1A7, sfx_dat::volume);
		memory_patch(0x004E3EB5, sfx_dat::volume);
		memory_patch(0x004E3F55, sfx_dat::volume);
		memory_patch(0x005154C8, sfx_dat::volume);
		memory_patch(0x004BB972, sfx_dat::sound_flags);
		memory_patch(0x004BBC54, sfx_dat::sound_flags);
		memory_patch(0x004BBC67, sfx_dat::sound_flags);
		memory_patch(0x004BBF17, sfx_dat::sound_flags);
		memory_patch(0x004BBF20, sfx_dat::sound_flags);
		memory_patch(0x004BBFF3, sfx_dat::sound_flags);
		memory_patch(0x004BC06C, sfx_dat::sound_flags);
		memory_patch(0x004BC0C0, sfx_dat::sound_flags);
		memory_patch(0x004BC7AE, sfx_dat::sound_flags);
		memory_patch(0x004BC80A, sfx_dat::sound_flags);
		memory_patch(0x004BC846, sfx_dat::sound_flags);
		memory_patch(0x004BCAD6, sfx_dat::sound_flags);
		memory_patch(0x004BCAF3, sfx_dat::sound_flags);
		memory_patch(0x004BCB0B, sfx_dat::sound_flags);
		memory_patch(0x004BCD44, sfx_dat::sound_flags);
		memory_patch(0x005154B0, sfx_dat::sound_flags);
		memory_patch(0x0048F127, sfx_dat::portrait_duration);
		memory_patch(0x0048F204, sfx_dat::portrait_duration);
		memory_patch(0x0048F2EB, sfx_dat::portrait_duration);
		memory_patch(0x0048F376, ((u32)sfx_dat::portrait_duration) + 0x10E);
		memory_patch(0x0048F3D2, sfx_dat::portrait_duration);
		memory_patch(0x0048F41B, ((u32)sfx_dat::portrait_duration) + 0x10E);
		memory_patch(0x0048F4A0, sfx_dat::portrait_duration);
		memory_patch(0x0048F572, sfx_dat::portrait_duration);
		memory_patch(0x0048F607, sfx_dat::portrait_duration);
		memory_patch(0x0048F697, sfx_dat::portrait_duration);
		memory_patch(0x0048F732, sfx_dat::portrait_duration);
		memory_patch(0x0048F822, sfx_dat::portrait_duration);
		memory_patch(0x0048F8E3, ((u32)sfx_dat::portrait_duration) + 0x104);
		memory_patch(0x0048FAEB, sfx_dat::portrait_duration);
		memory_patch(0x005154BC, sfx_dat::portrait_duration);
		memory_patch(0x004BBEC3, sfx_dat::type);
		memory_patch(0x004BC0D3, sfx_dat::type);
		memory_patch(0x004BC813, sfx_dat::type);
		memory_patch(0x005154A4, sfx_dat::type);
		memory_patch(0x0046102D, ((u32)sfx_dat::sound_file) + 0x58);
		memory_patch(0x0046116D, ((u32)sfx_dat::sound_file) + 0x54);
		memory_patch(0x004BC690, sfx_dat::sound_file);
		memory_patch(0x004BCA3C, sfx_dat::sound_file);
		memory_patch(0x004BCB2A, ((u32)sfx_dat::sound_file) + 0x3C);
		memory_patch(0x004DB0E6, sfx_dat::sound_file);
		memory_patch(0x00515498, sfx_dat::sound_file);
		memory_patch(0x004BB8E8, sfx_dat::sound_data);
		memory_patch(0x004BBC47, sfx_dat::sound_data);
		memory_patch(0x004BC077, sfx_dat::sound_data);
		memory_patch(0x004BC674, sfx_dat::sound_data);
		memory_patch(0x004BCAA6, sfx_dat::sound_data);
		memory_patch(0x0045EF66, ((u32)sfx_dat::sound_data) + 8);
		memory_patch(0x0048F11D, ((u32)sfx_dat::sound_data) + 8);
		memory_patch(0x0048F1FA, ((u32)sfx_dat::sound_data) + 8);
		memory_patch(0x0048F2DE, ((u32)sfx_dat::sound_data) + 8);
		memory_patch(0x0048F3CA, ((u32)sfx_dat::sound_data) + 8);
		memory_patch(0x0048F498, ((u32)sfx_dat::sound_data) + 8);
		memory_patch(0x0048F56A, ((u32)sfx_dat::sound_data) + 8);
		memory_patch(0x0048F5FF, ((u32)sfx_dat::sound_data) + 8);
		memory_patch(0x0048F68F, ((u32)sfx_dat::sound_data) + 8);
		memory_patch(0x0048F72A, ((u32)sfx_dat::sound_data) + 8);
		memory_patch(0x0048F81A, ((u32)sfx_dat::sound_data) + 8);
		memory_patch(0x0048FAE3, ((u32)sfx_dat::sound_data) + 8);
		memory_patch(0x0048F8DA, ((u32)sfx_dat::sound_data) + 0x828);
		memory_patch(0x0048F36F, ((u32)sfx_dat::sound_data) + 0x878);
		memory_patch(0x0048F412, ((u32)sfx_dat::sound_data) + 0x878);
	}
}