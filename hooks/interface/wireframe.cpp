#include "wireframe.h"
#include "../../BW/api.h"

//Not const because the first two bytes are written to.
//Could be changed to a handle for a pcx color table, for easier implementation of new colors.
u8 grpwire_remappings[] = {
	0, 0, 14, 14, 161, 161, 163, 163, 164, 164, // energy indicator
	41, 42, 41, 160, 165,						// normal
	41, 160, 160, 162, 51,
	182, 183, 182, 183, 185,					// parasite
	182, 183, 183, 184, 186,
	140, 141, 140, 71, 255,						// hero
	140, 142, 142, 150, 255,
	160, 162, 43, 196, 127,						// hallucination
	160, 194, 194, 198, 57
};

namespace hooks {
	// Was initially in building_zerg. The rest of the function this fragment is from use too many unknowns, thus this is only a fragment.
	// This allows to change the wireframe displayed for the unit, the special case being that zerg morphing buildings show the wireframe of the new building before construction is complete.
	// -iquare
	u16 stat_data_unit_wireframe_update_fragment(Unit* unit) {
		u16 unit_id = UnitId::None;
		bool use_current_id = true;
		if (!unit->is_completed()) {
			unit_id = unit->build_queue[unit->build_queue_slot];
			if (bw::is_zerg_building_morph_result(unit_id)) {
				use_current_id = false;
			}
		}
		if (use_current_id) {
			unit_id = unit->id;
		}
		return unit_id;
	}

	u8 wireframe_shields(Unit* unit) {
		auto eax = unit->id;
		u8 al = 0;
		if (units_dat::shields_enabled[unit->id]) {
			auto lastSlot = bw::get_last_queue_slot_id(unit);
			u32 maxShields = units_dat::max_shield_points[lastSlot];
			if (lastSlot == unit->id) {
				maxShields = unit->max_shields(); // for stasis, later
			}
			if (maxShields) {
				u32 shields = (u32)unit->shields >> 8;
				shields *= 7;
				shields /= maxShields;
				if (shields * 2 + 1 >= 40) {
					return al; // safety check
				}
				auto wireConst = g_wireframe_constants[shields * 2];
				auto wireConstShift = g_wireframe_constants[(shields * 2) + 1];
				auto buf1 = g_char_buffer[wireConst];//CL
				al = g_char_buffer[wireConstShift];//AL
				*off_50CE81 = buf1;
				*off_50CE82 = al;
			}
		} else {
			al = *g_byte_shield_string;
			*off_50CE81 = al;
			*off_50CE82 = al;
		}
		return al;
	}

	void __fastcall remap_grpwire_button(u16 graphic, BinDlg* dialog) {
		u32 _ebx;
		__asm MOV _ebx, EBX;
		UserUnit* user = dialog->user_unit_with_id;
		if ((u32)user != _ebx) {
			return;
		}

		char* default_grp_remap = &g_palette_remap[0xD0];
		u32 idx = (u8)(((dialog->flags & BinDlgFlags::NoClickSound) >> 30) + (graphic >> 7));
		u32 clr = *(u32*)&grpwire_remappings[idx + idx * 4 + 10];
		*(u32*)default_grp_remap = clr;
		((u8*)&clr)[1] = ((u8*)&clr)[0];
		*(u16*)grpwire_remappings = clr;
		default_grp_remap[8] = grpwire_remappings[idx + idx * 4 + 14];

		if (idx = units_dat::base_property[user->unit_id] & UnitProperty::Spellcaster) {
			Unit* unit = user->unit;
			u32 max_energy = unit->get_max_energy();
			u32 cur_energy = unit->energy;
			idx = cur_energy / (max_energy >> 8) >> 5;
		}
		*(u16*)&default_grp_remap[9] = *(u16*)&grpwire_remappings[idx];
	}
}