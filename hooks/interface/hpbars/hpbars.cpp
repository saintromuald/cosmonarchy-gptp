#include "hpbars.h"

namespace hooks
{
	void show_health_bar(Unit* unit)
	{
		if (!unit)
		{
			return;
		}
		helper::remove_hp_bar(unit->sprite->images.head);
		temp_unit = unit;
		helper::create_healthbar(unit->sprite);
		temp_unit = 0;
	}

	void compile_healthbar(Image* image, Sprite* sprite) // 004D6010
	{
		Unit* selection = active_player_selection->unit[sprite->selection_index];
		image->vertical_offset = sprites_dat::hp_bar_size[sprite->id + 262] + (g_image_grp_graphics[orders_dat::Unused3[sprite->id + 62] + 561]->frames[0].height >> 1) + 8;

		u8 is_visible = sprites_dat::is_visible[sprite->id + 390];
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
		if (units_dat::shields_enabled[selection->id])
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

	Unit* temp_unit; // ? -Veeq7

	// Hooked to allow showing all HP bars. -Pr0nogo
	void remove_healthbar(Image* image) // 004D5030
	{
		if (!(*g_game_layer).has_been_refreshed)
		{
			helper::refresh_image(image);
		}
		Sprite* sprite = image->parent_sprite;
		if (sprite->images.head == image)
		{
			sprite->images.head = image->link.next;
		}
		if (sprite->images.tail == image)
		{
			sprite->images.tail = image->link.prev;
		}
		if (image->link.prev)
		{
			image->link.prev->link.next = image->link.next;
		}
		Image* next_image = image->link.next;
		if (next_image)
		{
			next_image->link.prev = image->link.prev;
		}
		image->link.prev = 0;
		image->link.next = 0;
		Image* last_free_healthbar = *g_last_free_healthbar;
		if (*g_last_free_healthbar)
		{
			if (*g_first_free_healthbar == *g_last_free_healthbar)
			{
				*g_first_free_healthbar = image;
			}
			image->link.prev = *g_last_free_healthbar;
			image->link.next = last_free_healthbar->link.next;
			Image* next_free_healthbar = last_free_healthbar->link.next;
			if (next_free_healthbar)
			{
				next_free_healthbar->link.prev = image;
			}
			last_free_healthbar->link.next = image;
		}
		else
		{
			*g_first_free_healthbar = image;
			*g_last_free_healthbar = image;
		}
	}

	// Hooked to allow showing all HP bars. -Pr0nogo
	Image* create_healthbar(u32 unused, Sprite* sprite) // 004D6420
	{
		Image* last_free_healthbar = *g_last_free_healthbar;
		if (*g_last_free_healthbar == 0)
		{
			return 0;
		}

		Image* next_free_healthbar = (*g_last_free_healthbar)->link.next;
		*g_last_free_healthbar = (*g_last_free_healthbar)->link.next;
		if (*g_first_free_healthbar == last_free_healthbar)
		{
			*g_first_free_healthbar = last_free_healthbar->link.prev;
		}

		if (last_free_healthbar->link.prev)
		{
			last_free_healthbar->link.prev->link.next = next_free_healthbar;
		}
		if (next_free_healthbar)
		{
			next_free_healthbar->link.prev = last_free_healthbar->link.prev;
		}

		last_free_healthbar->link.prev = 0;
		next_free_healthbar = 0;
		Image* image = sprite->images.head;
		sprite->images.head = last_free_healthbar;
		last_free_healthbar->link.prev = image->link.prev;
		next_free_healthbar = image;
		if (image->link.prev)
		{
			image->link.prev->link.next = last_free_healthbar;
		}

		image->link.prev = last_free_healthbar;
		helper::compile_healthbar(last_free_healthbar, sprite);
		return last_free_healthbar;
	}
}