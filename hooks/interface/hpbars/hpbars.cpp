#include "hpbars.h"

namespace hooks
{
	void show_health_bar(Unit* unit)
	{
		if (unit == nullptr) return;
		helper::remove_hp_bar(unit->sprite->images.head);
		_unitTemp = unit;
		helper::create_health_bar(unit->sprite);
		_unitTemp = nullptr;
	}

	void compile_healthbar(Image* image, Sprite* sprite) // 004D6010
	{
		Unit* selection = active_player_selection->unit[sprite->selection_index];
		image->vertical_offset = sprites_dat::HpBarSize[sprite->id + 262] + (g_image_grp_graphic[orders_dat::Unused3[sprite->id + 62] + 561]->frames[0].height >> 1) + 8;

		u8 is_visible = sprites_dat::IsVisible[sprite->id + 390];
		image->grp_file->frames[0].x = 0;
		image->grp_file->frames[0].y = 0;
		image->grp_file->frames[0].width = is_visible - (is_visible - 1) % 3;
		if (image->grp_file->frames[0].width < 0x13u)
		{
			image->grp_file->frames[0].width = 19;
		}
		image->grp_file->frames[0].height = 0;

		if (units_dat::base_property[selection->id] & UnitProperty::Spellcaster && !(selection->status & UnitStatus::IsHallucination)
		||  selection->status & UnitStatus::IsHallucination
		||	selection->id == UnitId::ZergRilirokor
		||	selection->id == UnitId::ZergRililisk) // New
		{
			image->grp_file->frames[0].height += 6;
		}

		GrpHead* grp_file = image->grp_file;
		u8 grp_height = grp_file->frames[0].height;
		u8 grp_height_final = grp_height + 5;
		if (units_dat::ShieldsEnabled[selection->id])
		{
			grp_height_final = grp_height + 7;
		}
		grp_file->frames[0].height = grp_height_final;
		image->grp_file->width = image->grp_file->frames[0].width;
		image->grp_file->height = image->grp_file->frames[0].height;
		image->parent_sprite = sprite;
		image->flags = 0;

		s16* grp_left = &image->grp_size.left; // wtf
		*(u32*)grp_left = 0;
		*((u32*)grp_left + 1) = 0;
		image->draw_func_param = selection;
		helper::update_image_data_draw(image);
		image->flags |= 1u; // was LOBYTE
		helper::play_image_iscript(image, 0);
	}

	Unit* _unitTemp;
}