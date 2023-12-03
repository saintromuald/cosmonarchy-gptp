#include "Image.h"
#include "Sprite.h"
#include <SCBW/api.h>
#include <SCBW/scbwdata.h>
#include <cassert>
#include "hook_tools.h"
#include <hooks/weapons/weapon_spellhit.h>

void Image::play_iscript_animation(IscriptAnimation::Enum animation)
{
	assert(this);
	u32 animation_ = (u8)animation;

	const u32 Func_PlayIscriptAnim = 0x004D8470;  //AKA play_image_iscript();
	__asm {
		PUSHAD
		PUSH animation_
		MOV ECX, this
		CALL Func_PlayIscriptAnim
		POPAD
	}
}

//should be equivalent to helper::image_destructor @ 0x004D4CE0 with ESI = sprite
void Image::free()
{

	assert(this);

	if (!(g_screen_layers->game.has_been_refreshed))
		scbw::refresh_screen(this->screen_pos.x,
			this->screen_pos.y,
			this->screen_pos.x + this->grp_size.right,
			this->screen_pos.y + this->grp_size.bottom
		);	//should be equivalent to a call to refreshImage @ 0x004970A0 with EAX = this

			//perform identically to original code
	this->parent_sprite->images.unlink<&Image::link>(this);

	this->grp_file = nullptr;

	//perform identically to original code
	unusedImages.insertAfterHead(this);

}

// Loosely based on code at @ 0x004D5A50
void Image::set_remapping(ColorRemapping::Enum remapping)
{
	assert(this);
	this->standard.draw_func_param = g_color_shift[remapping].data;
}

// Identical to function @ 0x004D5A50
void Image::initialize_data(Sprite* parent, u16 image_id, s8 x, s8 y)
{
	assert(this);
	hooks::initialize_image_data(this, parent, image_id, x, y);
}

// Same as function 0x004D5EA0, direction is 0-31
void Image::set_image_direction(u8 direction)
{
	assert(this);
	constexpr u32 Func_setDir = 0x004D5EA0;
	__asm {
		PUSHAD
		MOV EAX, this
		PUSH direction
		CALL Func_setDir
		POPAD
	}
}

GrpFrame* Image::get_current_frame() const
{
	assert(this);
	return &this->grp_file->frames[this->frame_index];
}

namespace hooks
{
	// Hooked to replace calls in unhooked functions. -Pr0nogo
	void initialize_image_data(Image* image, Sprite* parent, u16 image_id, s8 horizontal_offset, s8 vertical_offset) { // 004D5A50
		image->id = image_id;
		image->grp_file = g_image_grp_graphics[image_id];
		image->flags = 0;
		if (images_dat::is_turnable[image_id] & 1) {
			image->flags |= ImageFlags::HasRotationFrames;
		}
		if (images_dat::is_turnable[image_id] & 2) {
			image->flags |= ImageFlags::Omnidirectional; // New
		}
		if (images_dat::is_clickable[image_id] & 1) {
			image->flags |= ImageFlags::Clickable;
		}
		image->frameset = 0;
		image->direction = 0;
		image->frame_index = 0;
		image->parent_sprite = parent;
		image->horizontal_offset = horizontal_offset;
		image->vertical_offset = vertical_offset;
		image->grp_size = Box16();
		image->standard.draw_func_param = 0;

		// Initialize iscript data
		image->iscript_header_offset = 0;
		image->iscript_offset = 0;
		image->return_address = 0;
		image->animation = IscriptAnimation::Init;
		image->wait = 0;

		if (images_dat::rle_function[image_id] == PaletteType::RLE_PLAYER_SIDE) {
			*(u32*)(&image->standard.draw_func_param) = parent->player;
		}
		if (images_dat::rle_function[image_id] == PaletteType::RLE_EFFECT) {
			image->standard.draw_func_param = g_color_shift[images_dat::remapping[image_id]].data;
		}
	}

	// Reimplemented by X405, ported by Pr0nogo, to enable omnidirectional rendering. -Pr0nogo
	void set_image_direction(Image* image, u8 direction) { // 004D5F80
		if (image->flags & ImageFlags::UsesSpecialOffset) {
			helper::update_image_position_offset(image);
		}

		if (image->flags & ImageFlags::HasRotationFrames) {
			u32 new_direction = (direction + 4) / 8;
			u32 new_flip_flag = 0;

			if (new_direction > 16 && !(image->flags & ImageFlags::Omnidirectional)) {
				new_direction = 32 - new_direction;
				new_flip_flag = ImageFlags::Mirrored;
			} else {
				new_direction %= 32;
			}

			if (image->direction != new_direction || (image->flags & ImageFlags::Mirrored) != new_flip_flag) {
				image->direction = new_direction;
				image->flags &= ~ImageFlags::Mirrored;
				image->flags |= new_flip_flag;
				helper::set_image_palette_type(image, image->palette_type);

				if (image->frame_index != image->frameset + image->direction) {
					image->flags |= ImageFlags::Redraw;
					image->frame_index = image->frameset + image->direction;
				}
			}
		}
	}

	// Hooked by X405 as part of the omnidirectional frame display feature addition. -Pr0nogo
	void update_image_direction(Image* image, u8 direction) { // 004D5EA0
		if (image->flags & ImageFlags::HasRotationFrames) {
			u32 new_direction = direction;
			u32 new_flip_flag = 0;

			if (new_direction > 16 && !(image->flags & ImageFlags::Omnidirectional)) {
				new_direction = 32 - new_direction;
				new_flip_flag = ImageFlags::Mirrored;
			} else {
				new_direction %= 32;
			}

			if ((image->flags & ImageFlags::Mirrored) != new_flip_flag || image->direction != new_direction) {
				image->direction = new_direction;
				image->flags &= ~ImageFlags::Mirrored;
				image->flags |= new_flip_flag;
				helper::set_image_palette_type(image, image->palette_type);

				if (image->frame_index != image->direction + image->frameset) {
					image->flags |= ImageFlags::Redraw;
					image->frame_index = image->direction + image->frameset;
				}

				if (image->flags & ImageFlags::UsesSpecialOffset) {
					helper::update_image_position_offset(image);
				}
			}
		}
	}

	// Reimplemented by X405, ported by Pr0nogo, to fix followmaingraphic for omnidirectional sprites. -Pr0nogo
	void iscript_opcode_followmaingraphic(Image* image, IscriptProgram* program_state, u32 noop, u32* distance_moved) // 004D750C
	{
		if (!noop)
		{
			Image* main = image->parent_sprite->main_graphic;
			if (main)
			{
				if (image->frame_index != main->frame_index || ((image->flags ^ main->flags) & ImageFlags::Mirrored))
				{
					image->frameset = main->frameset;
					image->direction = main->direction;
					image->flags ^= ((image->flags ^ main->flags) & ImageFlags::Mirrored);
					Image* paletted_image = helper::set_image_palette_type(image, image->palette_type);
					helper::update_image_frame_index(paletted_image);
				}
			}
		}
	}

	// Hooked to extend iscript.bin. -Pr0nogo
	void play_image_iscript(Image* image, u8 animation) { // 004D8470
		char final_anim = animation;
		if ((animation != IscriptAnimation::Death || image->animation != IscriptAnimation::Death)
		&&  (image->flags & ImageFlags::UseFullIscript || !animation || animation == IscriptAnimation::Death))
		{
			char image_anim = image->animation;
			if (animation != image_anim || animation != IscriptAnimation::Walking && animation != IscriptAnimation::IsWorking) {
				if (animation == IscriptAnimation::GndAttkRpt) {
					if (image_anim != IscriptAnimation::GndAttkRpt && image_anim != IscriptAnimation::GndAttkInit)
					{
						final_anim = IscriptAnimation::GndAttkInit;
					}
				} else if (animation  == IscriptAnimation::AirAttkRpt
				&&		   image_anim != IscriptAnimation::AirAttkRpt
				&&		   image_anim != IscriptAnimation::AirAttkInit)
				{
					final_anim = IscriptAnimation::AirAttkInit;
				}
				u32 offset = image->iscript_header_offset;
				char* data = *g_iscript_data;
				image->animation = final_anim;
				image->iscript_offset = *(u16*)&data[2 * final_anim + 8 + offset];
				image->wait = 0;
				image->return_address = 0;
				helper::iscript_opcode_cases(image, &image->iscript_header_offset, 0, 0);
			}
		}
	}

	// Hooked to extend iscript.bin. -Pr0nogo
	void ground_attack_init(u16 pos_x, u16 pos_y) { // 00488660
		Thingy* thingy = *g_first_used_thingy_1;
		Sprite* thingy_sprite = thingy->sprite;
		*g_can_draw_thingy = 0;
		helper::move_sprite(thingy_sprite, pos_x + 1, pos_y + 1);
		for (Image* i = thingy->sprite->images.head; i; i = i->link.next) {
			if (i->flags & ImageFlags::UseFullIscript) {
				u32 header = i->iscript_header_offset;
				char* data = *g_iscript_data;
				i->animation = 2;
				i->iscript_offset = *(u16*)&data[header + 12];
				i->wait = 0;
				i->return_address = 0;
				helper::iscript_opcode_cases(i, &i->iscript_header_offset, 0, 0);
			}
		}
		*g_can_draw_thingy = 1;
	}

	// Hooked to extend iscript.bin. -Pr0nogo
	void play_warp_in_overlay(Image* image, u32 a2) { // 004D8500
	//	void(__fastcall *function)(u32, u32, GrpFrame*, Rect32*, u32);
		helper::is_valid_script(image, 193);
		u32 offset = image->iscript_header_offset;
		char* data = *g_iscript_data;
		image->animation = 0;
		image->iscript_offset = *(u16*)&data[offset + 8];
		image->wait = 0;
		image->return_address = 0;
		helper::iscript_opcode_cases(image, &image->iscript_header_offset, 0, 0);
		helper::is_valid_script(image, images_dat::iscript_id[image->id]);
		image->palette_type = 12;
		image->update_fxn = g_sg_update_funcs[12].function;
		u16 flags = image->flags;
		void* function = g_sg_draw_funcs[12].function_2;
		if (!(flags & ImageFlags::Mirrored)) {
			function = g_sg_draw_funcs[12].function_1;
		}
		image->render_fxn = function;
		image->flags = flags | ImageFlags::Redraw;
		helper::iscript_opcode_cases(image, &image->iscript_header_offset, 0, 0); // guessed second parameter
	}

	// Hooked to extend iscript.bin. -Pr0nogo
	void image_update_warp_flash(Image* image) { // 004D8590
		u8 param = image->warp.param;
		if (image->warp.param) {
			--image->warp.param;
		} else {
			image->warp.param = 2;
			if (image->warp.timer >= 63) {
				if (image->animation != IscriptAnimation::Death) { // IscriptAnimation::Cuckdeath
					u16 header = image->iscript_header_offset;
					image->animation = IscriptAnimation::Death;
					char* data = *g_iscript_data;
					header = *(u16*)&data[header + 10];
					image->wait = 0;
					image->return_address = 0;
					image->iscript_offset = header;
					helper::iscript_opcode_cases(image, &image->iscript_header_offset, 0, 0);
				}
				(*g_active_iscript_entity)->order_signal |= 1;
				image->flags |= ImageFlags::Redraw;
			} else {
				++image->warp.timer;
				image->flags |= ImageFlags::Redraw;
			}
		}
	}

	// STUPID
	template<typename T>
	T* take_iscript_data(IscriptProgram* program_state, u32 count)
	{
		BYTE* data = (BYTE*)*g_iscript_data + program_state->program_counter;
		program_state->program_counter += sizeof(T) * count;
		return (T*)data;
	}

	template<typename T>
	T take_iscript_datum(IscriptProgram* program_state)
	{
		return *take_iscript_data<T>(program_state, 1);
	}

	// Reimplemented by X405, ported by Pr0nogo, to extend iscript.bin and add functionality. -Pr0nogo
	// Local only.
	void iscript_playfram(Image* image, u32 frame) // 004D5E70
	{
		if (image->frameset != frame)
		{
			image->frameset = frame;
			if (image->frame_index != image->frameset + image->direction)
			{
				image->flags |= ImageFlags::Redraw;
				image->frame_index = image->frameset + image->direction;
			}
		}
	}

	// Reimplemented by X405, ported by Pr0nogo, to extend iscript.bin and add functionality. -Pr0nogo
	// Local only.
	void iscript_set_pos(Image* image, char x, char y) // 004D5900
	{
		if (image->horizontal_offset != x || image->vertical_offset != y)
		{
			image->flags |= ImageFlags::Redraw;
			image->horizontal_offset = x;
			image->vertical_offset = y;
		}
	}

	// Reimplemented by X405, ported by Pr0nogo, to extend iscript.bin and add functionality. -Pr0nogo
	// Local only.
	Thingy* iscript_create_sprite(Image* image, u16 sprite_id, u32 x, u32 y, char elevation_level) // 004D7120
	{
		Thingy* result = helper::create_thingy(sprite_id, x + image->horizontal_offset + (u16)image->parent_sprite->position.x, y + image->vertical_offset + image->parent_sprite->position.y, 0);
		if (result)
		{
			result->sprite->elevation = elevation_level;
			helper::set_thingy_visibility_flags(result);
		}
		return result;
	}

	// Reimplemented by X405, ported by Pr0nogo, to extend iscript.bin and add functionality. -Pr0nogo
	void iscript_fireweapon(Unit* unit, u8 weapon_id) // 00479D40
	{
		helper::fire_unit_weapon(unit, weapon_id);
		if (weapons_dat::damage_factor[weapon_id] == 2)
		{
			helper::fire_unit_weapon(unit, weapon_id);
		}
	}

	// Reimplemented by X405, ported by Pr0nogo, to extend iscript.bin and add functionality. -Pr0nogo
	void iscript_attackwith(Unit* attacker, u8 is_ground_weapon) // 00479D60
	{
		if (attacker->order_target.unit || attacker->id == UnitId::TerranDiadem)
		{
			u32 weapon_id;
			if (is_ground_weapon != 1)
			{
				weapon_id = units_dat::air_weapon[attacker->id];
			} else if (attacker->id != UnitId::ZergLakizilisk || (attacker->status & UnitStatus::Burrowed))
			{
				weapon_id = units_dat::ground_weapon[attacker->id];
			} else {
				weapon_id = WeaponId::None;
			}
			helper::iscript_fireweapon(attacker, weapon_id);
		}
	}

	// Reimplemented by X405, ported by Pr0nogo, to extend iscript.bin and add functionality. -Pr0nogo
	void iscript_useweapon(Unit* unit, u8 weapon_id) // 00479C30
	{
		if (weapons_dat::flingy_id[weapon_id])
		{
			helper::create_bullet(weapon_id, unit, unit->order_target.pt.x, unit->order_target.pt.y, unit->player, unit->current_direction_1);
			if (weapons_dat::damage_factor[weapon_id] == 2)
			{
				helper::create_bullet(weapon_id, unit, unit->order_target.pt.x, unit->order_target.pt.y, unit->player, unit->current_direction_1);
			}
		}
	}

	// Reimplemented by X405, ported by Pr0nogo, to extend iscript.bin and add functionality. -Pr0nogo
	void iscript_nobrkcodeend(Unit* unit) // 004753E0
	{
		unit->status &= ~UnitStatus::NoBrkCodeStart;
		unit->sprite->flags &= ~SpriteFlags::IscriptCode;
		if (unit->order_queue_head && (unit->user_action_flags & 1))
		{
			if (unit->id == UnitId::TerranDiadem) {
				return;
			}
			helper::ignore_iscript_go_to_idle(unit);
			helper::do_next_queued_order(unit);
		}
	}

	// Reimplemented by X405, ported by Pr0nogo, to extend iscript.bin and add functionality. -Pr0nogo
	void iscript_opcode_cases(Image* image, IscriptProgram* program_state, u32 noop, u32* distance_moved) // 004D74C0 | arg2 iscript_data unused, omitted
	{
		if (program_state->wait)
		{
			program_state->wait -= 1;
			return;
		}
		while (2)
		{
			IscriptOpcodes::Enum opcode = take_iscript_datum<IscriptOpcodes::Enum>(program_state);
			switch (opcode)
			{
				case IscriptOpcodes::opc_playfram:
				{
					u16 arg = take_iscript_datum<u16>(program_state);
					if (noop)
					{
						break;
					}
					iscript_playfram(image, arg);
					break;
				}
				case IscriptOpcodes::opc_playframtile:
				{
					u16 arg = take_iscript_datum<u16>(program_state);
					if (noop)
					{
						break;
					}
					u16 offset = *g_current_tileset + arg;
					if (offset < (u16)(image->grp_file->frame_count & 0x7FFF))
					{
						iscript_playfram(image, offset);
					}
					break;
				}
				case IscriptOpcodes::opc_sethorpos:
				{
					s8 x = take_iscript_datum<s8>(program_state);
					if (noop)
					{
						break;
					}
					if (image->horizontal_offset != x)
					{
						image->flags |= ImageFlags::Redraw;
						image->horizontal_offset = x;
					}
					break;
				}
				case IscriptOpcodes::opc_setvertpos:
				{
					s8 y = take_iscript_datum<s8>(program_state);
					if (noop)
					{
						break;
					}
					if ((*g_active_iscript_unit == nullptr || ((*g_active_iscript_unit)->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection)) == 0) && image->vertical_offset != y)
					{
						image->flags |= ImageFlags::Redraw;
						image->vertical_offset = y;
					}
					break;
				}
				case IscriptOpcodes::opc_setpos:
				{
					char x = take_iscript_datum<char>(program_state);
					char y = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					iscript_set_pos(image, x, y);
					break;
				}
				case IscriptOpcodes::opc_wait:
				{
					char arg = take_iscript_datum<char>(program_state);
					program_state->wait = arg - 1;
					return;
				}
				case IscriptOpcodes::opc_waitrand:
				{
					u8 min = take_iscript_datum<unsigned char>(program_state);
					u8 max = take_iscript_datum<unsigned char>(program_state);
					if (noop)
					{
						break;
					}
					u8 v16 = helper::randomize_short(3);
					program_state->wait = min + v16 % (max - min + 1) - 1;
					return;
				}
				case IscriptOpcodes::opc_goto:
				{
					u16 new_pc = take_iscript_datum<u16>(program_state);
					program_state->program_counter = new_pc;
					break;
				}
				case IscriptOpcodes::opc_imgol:
				{
					u16 image_id = take_iscript_datum<u16>(program_state);
					char x = take_iscript_datum<char>(program_state);
					char y = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					helper::iscript_create_image(image, image_id, x + image->horizontal_offset, (u8)(image->vertical_offset + y), 1);
					break;
				}
				case IscriptOpcodes::opc_imgul:
				{
					u16 image_id = take_iscript_datum<u16>(program_state);
					char x = take_iscript_datum<char>(program_state);
					char y = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					helper::iscript_create_image(image, image_id, x + image->horizontal_offset, (u8)(image->vertical_offset + y), 0);
					break;
				}
				case IscriptOpcodes::opc_imgolorig:
				{
					u16 v24 = take_iscript_datum<u16>(program_state);
					if (noop)
					{
						break;
					}
					if (Image* new_image = helper::iscript_create_image(image, v24, 0, 0, 1))
					{
						if ((new_image->flags & ImageFlags::UsesSpecialOffset) == 0)
						{
							new_image->flags |= ImageFlags::UsesSpecialOffset;
							helper::update_image_position_offset(new_image);
						}
					}
					break;
				}
				case IscriptOpcodes::opc_switchul:
				{
					u16 v27 = take_iscript_datum<u16>(program_state);
					if (noop)
					{
						break;
					}
					if (Image* new_image = helper::iscript_create_image(image, v27, 0, 0, 0))
					{
						if ((new_image->flags & ImageFlags::UsesSpecialOffset) == 0)
						{
							new_image->flags |= ImageFlags::UsesSpecialOffset;
							helper::update_image_position_offset(new_image);
						}
					}
					break;
				}
				case IscriptOpcodes::opc_imgoluselo:
				{
					u16 image_id = take_iscript_datum<u16>(program_state);
					u8 v30 = take_iscript_datum<u8>(program_state);
					char v29 = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					Point16 pt;
					helper::iscript_use_lo_file(&pt, image, v30, v29);
					helper::iscript_create_image(image, image_id, LOBYTE(pt.x) + image->horizontal_offset, (u8)(LOBYTE(pt.y) + image->vertical_offset), 1);
					break;
				}
				case IscriptOpcodes::opc_imguluselo:
				{
					u16 image_id = take_iscript_datum<u16>(program_state);
					u8 v34 = take_iscript_datum<u8>(program_state);
					char v33 = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					Point16 pt;
					helper::iscript_use_lo_file(&pt, image, v34, v33);
					helper::iscript_create_image(image, image_id, LOBYTE(pt.x) + image->horizontal_offset, (u8)(LOBYTE(pt.y) + image->vertical_offset), 0);
					break;
				}
				case IscriptOpcodes::opc_sprol:
				{
					u16 sprite_id = take_iscript_datum<u16>(program_state);
					char x = take_iscript_datum<char>(program_state);
					char y = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					iscript_create_sprite(image, sprite_id, x, y, image->parent_sprite->elevation + 1);
					break;
				}
				case IscriptOpcodes::opc_highsprol:
				{
					u16 sprite_id = take_iscript_datum<u16>(program_state);
					char x = take_iscript_datum<char>(program_state);
					char y = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					iscript_create_sprite(image, sprite_id, x, y, image->parent_sprite->elevation - 1);
					break;
				}
				case IscriptOpcodes::opc_lowsprul:
				{
					u16 sprite_id = take_iscript_datum<u16>(program_state);
					char x = take_iscript_datum<char>(program_state);
					char y = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					iscript_create_sprite(image, sprite_id, x, y, 1);
					break;
				}
				case IscriptOpcodes::opc_uflunstable:
				{
					u16 flingy_id = take_iscript_datum<u16>(program_state);
					if (noop)
					{
						break;
					}
					if (Flingy* flingy = helper::create_flingy(0, image->parent_sprite->position.y + image->vertical_offset, (u16)image->parent_sprite->position.x + image->horizontal_offset, flingy_id))
					{
						Thingy* thingy = (Thingy*)helper::ufl_unstable_randomize(flingy);
						helper::set_thingy_visibility_flags(thingy);
					}
					break;
				}
				case IscriptOpcodes::opc_spruluselo:
				{
					u16 sprite_id = take_iscript_datum<u16>(program_state);
					char x = take_iscript_datum<char>(program_state);
					char y = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					if (*g_active_iscript_unit && ((*g_active_iscript_unit)->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection)) && !images_dat::draw_if_cloaked[sprites_dat::image_id[sprite_id]])
					{
						break;
					}
					if (Thingy* thingy = iscript_create_sprite(image, sprite_id, x, y, image->parent_sprite->elevation))
					{
						helper::set_all_overlay_directions_generic((Unit*)thingy, (image->flags & ImageFlags::Mirrored) ? 32 - image->direction : image->direction);
					}
					break;
				}
				case IscriptOpcodes::opc_sprul:
				{
					u16 sprite_id = take_iscript_datum<u16>(program_state);
					char x = take_iscript_datum<char>(program_state);
					char y = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					if (*g_active_iscript_unit && ((*g_active_iscript_unit)->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection)) && !images_dat::draw_if_cloaked[sprites_dat::image_id[sprite_id]])
					{
						break;
					}
					if (Thingy* thingy = iscript_create_sprite(image, sprite_id, x, y, image->parent_sprite->elevation - 1))
					{
						helper::set_all_overlay_directions_generic((Unit*)thingy, (image->flags & ImageFlags::Mirrored) ? 32 - image->direction : image->direction);
					}
					break;
				}
				case IscriptOpcodes::opc_sproluselo:
				{
					u16 sprite_id = take_iscript_datum<u16>(program_state);
					char arg3 = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					Point16 pt;
					helper::iscript_use_lo_file(&pt, image, arg3, 0);
					if (Thingy* thingy = iscript_create_sprite(image, sprite_id, pt.x, pt.y, image->parent_sprite->elevation + 1))
					{
						helper::set_all_overlay_directions_generic((Unit*)thingy, (image->flags & ImageFlags::Mirrored) ? 32 - image->direction : image->direction);
					}
					break;
				}
				case IscriptOpcodes::opc_end:
					if (noop)
					{
						break;
					}
					helper::image_destructor(image);
					return;
				case IscriptOpcodes::opc_setflipstate:
				{
					char arg = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					helper::flip_image(image, arg);
					break;
				}
				case IscriptOpcodes::opc_playsnd:
				{
					u16 sfx = take_iscript_datum<u16>(program_state);
					if (noop)
					{
						break;
					}
					helper::iscript_play_sound(image, sfx);
					break;
				}
				case IscriptOpcodes::opc_playsndbtwn:
				{
					u16 min = take_iscript_datum<u16>(program_state);
					u16 max = take_iscript_datum<u16>(program_state);
					if (noop)
					{
						break;
					}
					helper::iscript_play_sound(image, (min + helper::randomize_short(5) % ((u32)max - min + 1)));
					break;
				}
				case IscriptOpcodes::opc_domissiledmg:
					if (noop)
					{
						break;
					}
				//	CBullet_Damage(iscript_bullet); // This is a fatal error call, according to IDA. Not present in our IDA decompilation of this opcode. -Pr0nogo
					break;
				case IscriptOpcodes::opc_attackmelee:
					if (noop)
					{
						break;
					}
					hooks::iscript_attackmelee(*g_active_iscript_unit);
					// Fallthrough intended
				case IscriptOpcodes::opc_playsndrand:
				{
					u8 arg_count = take_iscript_datum<char>(program_state);
					u16* args = take_iscript_data<u16>(program_state, arg_count);
					if (noop)
					{
						break;
					}
					helper::iscript_play_sound(image, args[helper::randomize_short(4) % arg_count]);
					break;
				}
				case IscriptOpcodes::opc_followmaingraphic:
				{
					if (noop)
					{
						break;
					}
					if (Image* main_image = image->parent_sprite->main_graphic)
					{
						if (image->frame_index != main_image->frame_index || ((image->flags ^ main_image->flags) & ImageFlags::Mirrored))
						{
							image->frameset = main_image->frameset;
							image->direction = main_image->direction;
							image->flags ^= ((image->flags ^ main_image->flags) & ImageFlags::Mirrored);
							Image* paletted_image = helper::set_image_palette_type(image, image->palette_type);
							helper::update_image_frame_index(paletted_image);
						}
					}
					break;
				}
				case IscriptOpcodes::opc_randcondjmp:
				{
					u8 chance = take_iscript_datum<char>(program_state);
					u16 new_pc = take_iscript_datum<unsigned short>(program_state);
					if ((u8)helper::randomize_short(7) <= (u32)chance)
					{
						program_state->program_counter = new_pc;
					}
					break;
				}
				case IscriptOpcodes::opc_turnccwise:
				{
					char arg = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					helper::turn_unit_left(*g_active_iscript_unit, arg);
					break;
				}
				case IscriptOpcodes::opc_turncwise:
				{
					char arg = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					helper::turn_unit(*g_active_iscript_unit, (*g_active_iscript_unit)->current_direction_1 + 8 * arg);
					break;
				}
				case IscriptOpcodes::opc_turn1cwise:
					if (noop)
					{
						break;
					}
					if ((*g_active_iscript_unit)->order_target.unit == nullptr)
					{
						helper::turn_unit(*g_active_iscript_unit, (*g_active_iscript_unit)->current_direction_1 + 8);
					}
					break;
				case IscriptOpcodes::opc_turnrand:
				{
					char arg = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					if ((helper::randomize_short(6) & 3) == 1)
					{
						helper::turn_unit_left(*g_active_iscript_unit, arg);
					}
					else
					{
						helper::turn_unit(*g_active_iscript_unit, (*g_active_iscript_unit)->current_direction_1 + 8 * arg);
					}
					break;
				}
				case IscriptOpcodes::opc_setspawnframe:
				{
					char arg = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					helper::set_unit_retreat_point(*g_active_iscript_unit, arg);
					break;
				}
				case IscriptOpcodes::opc_sigorder:
				{
					char arg = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					(*g_active_iscript_entity)->order_signal |= arg;
					break;
				}
				case IscriptOpcodes::opc_attackwith:
				{
					char arg = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					iscript_attackwith(*g_active_iscript_unit, arg);
					break;
				}
				case IscriptOpcodes::opc_attack:
					if (noop)
					{
						break;
					}
					if ((*g_active_iscript_unit)->order_target.unit && ((*g_active_iscript_unit)->order_target.unit->status & UnitStatus::InAir))
					{
						helper::iscript_fireweapon(*g_active_iscript_unit, units_dat::air_weapon[(*g_active_iscript_unit)->id]);
					}
					else
					{
						iscript_attackwith(*g_active_iscript_unit, 1);
					}
					break;
				case IscriptOpcodes::opc_castspell:
					if (noop)
					{
						break;
					}
					if (orders_dat::order_weapon_id[(*g_active_iscript_unit)->main_order_id] != WeaponId::None && !helper::can_cast_spell(*g_active_iscript_unit))
					{
						helper::iscript_fireweapon(*g_active_iscript_unit, orders_dat::order_weapon_id[(*g_active_iscript_unit)->main_order_id]);
					}
					break;
				case IscriptOpcodes::opc_useweapon:
				{
					char weapon_id = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					helper::iscript_useweapon(*g_active_iscript_unit, weapon_id);
					break;
				}
				case IscriptOpcodes::opc_move:
				{
					char arg = take_iscript_datum<char>(program_state);
					u32 speed = arg << 8;
					if (distance_moved)
					{
						*distance_moved = helper::get_modified_unit_speed(speed, *g_active_iscript_unit);
					}
					if (noop)
					{
						break;
					}
					u32 final_speed = helper::get_modified_unit_speed(speed, *g_active_iscript_unit);
					helper::set_unit_movement_speed(*g_active_iscript_unit, final_speed);
					break;
				}
				case IscriptOpcodes::opc_gotorepeatattk:
					if (noop)
					{
						break;
					}
					if (*g_active_iscript_unit)
					{
						(*g_active_iscript_unit)->movement_flags &= ~8;
					}
					break;
				case IscriptOpcodes::opc_engframe:
				{
					u8 frame = take_iscript_datum<u8>(program_state);
					if (noop)
					{
						break;
					}
					image->frameset = frame;
					Image* main_image = image->parent_sprite->main_graphic;
					image->direction = main_image->direction;
					image->flags ^= ((image->flags ^ main_image->flags) & ImageFlags::Mirrored);
					Image* paletted_image = helper::set_image_palette_type(image, image->palette_type);
					helper::update_image_frame_index(paletted_image);
					break;
				}
				case IscriptOpcodes::opc_engset:
				{
					u8 frame = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					Image* main_graphic = image->parent_sprite->main_graphic;
					image->direction = main_graphic->direction;
					image->frameset = main_graphic->frameset + frame * (main_graphic->grp_file->frame_count & 0x7FFF);
					image->flags ^= ((image->flags ^ main_graphic->flags) & ImageFlags::Mirrored);
					Image* palette_type = helper::set_image_palette_type(image, image->palette_type);
					helper::update_image_frame_index(palette_type);
					break;
				}
				case IscriptOpcodes::opc___2d:
					if (noop)
					{
						break;
					}
					*g_can_draw_thingy = 0;
					break;
				case IscriptOpcodes::opc_nobrkcodestart:
					if (noop)
					{
						break;
					}
					(*g_active_iscript_unit)->status |= UnitStatus::NoBrkCodeStart;
					(*g_active_iscript_unit)->sprite->flags |= 0x80u;
					break;
				case IscriptOpcodes::opc_nobrkcodeend:
					if (noop)
					{
						break;
					}
					if (*g_active_iscript_unit)
					{
						u16 counter = program_state->program_counter;
						iscript_nobrkcodeend(*g_active_iscript_unit);
						program_state->program_counter = counter;
					}
					break;
				case IscriptOpcodes::opc_ignorerest:
				{
					if (noop)
					{
						break;
					}
					if ((*g_active_iscript_unit)->order_target.unit)
					{
						program_state->program_counter -= 1;
						program_state->wait = 10;
						return;
					}
					u16 counter = program_state->program_counter;
					helper::ignore_iscript_go_to_idle(*g_active_iscript_unit);
					program_state->program_counter = counter;
					break;
				}
				case IscriptOpcodes::opc_attkshiftproj:
				{
					char amount = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					weapons_dat::forward_offset[(*g_active_iscript_unit)->get_ground_weapon()] = amount;
					iscript_attackwith(*g_active_iscript_unit, 1);
					break;
				}
				case IscriptOpcodes::opc_tmprmgraphicstart:
					if (noop)
					{
						break;
					}
					helper::hide_image(image);
					break;
				case IscriptOpcodes::opc_tmprmgraphicend:
					if (noop)
					{
						break;
					}
					helper::show_image(image);
					break;
				case IscriptOpcodes::opc_setfldirect:
				{
					char direction = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					helper::turn_unit(*g_active_iscript_unit, 8 * direction);
					break;
				}
				case IscriptOpcodes::opc_call:
				{
					u16 new_pc = take_iscript_datum<u16>(program_state);
					program_state->unsigned4 = program_state->program_counter;
					program_state->program_counter = new_pc;
					break;
				}
				case IscriptOpcodes::opc_return:
					program_state->program_counter = program_state->unsigned4;
					break;
				case IscriptOpcodes::opc_setflspeed:
				{
					u16 speed = take_iscript_datum<u16>(program_state);
					if (noop)
					{
						break;
					}
					(*g_active_iscript_unit)->flingy_top_speed = speed;
					break;
				}
				case IscriptOpcodes::opc_creategasoverlays:
				{
					char offset = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					Point16 pt;
					helper::iscript_use_lo_file(&pt, image, 2, offset);
					u32 gas_id = offset + ((*g_active_iscript_unit)->building.resource.resource_amount != 0 ? 430 : 435);
					if (Image* new_image = helper::get_empty_image())
					{
						helper::images_insert(new_image, image->parent_sprite->images.head, image);
						helper::some_play_image_crap_that_crashes(LOBYTE(pt.y) + image->vertical_offset, LOBYTE(pt.x) + image->horizontal_offset, new_image, image->parent_sprite, (u16)gas_id);
					}
					break;
				}
				case IscriptOpcodes::opc_pwrupcondjmp:
				{
					u16 new_pc = take_iscript_datum<u16>(program_state);
					if (image->parent_sprite && image->parent_sprite->main_graphic != image)
					{
						program_state->program_counter = new_pc;
					}
					break;
				}
				case IscriptOpcodes::opc_trgtrangecondjmp:
				{
					u16 distance = take_iscript_datum<u16>(program_state);
					u16 jump_pos = take_iscript_datum<unsigned short>(program_state);
					if (noop)
					{
						break;
					}
					if ((*g_active_iscript_unit)->order_target.unit)
					{
						u32 final_y;
						u32 final_x;
						helper::get_closest_point_of_target(*g_active_iscript_unit, (u32)&final_x, (u32)&final_y);
						if (helper::is_distance_greater_than_halt_distance(*g_active_iscript_unit, final_y, final_x, distance))
						{
							program_state->program_counter = jump_pos;
						}
					}
					break;
				}
				case IscriptOpcodes::opc_trgtarccondjmp:
				{
					u16 target_angle = take_iscript_datum<u16>(program_state);
					u16 test_angle = take_iscript_datum<unsigned short>(program_state);
					u16 new_pc = take_iscript_datum<u16>(program_state);
					if (noop)
					{
						break;
					}
					if (Unit* target = (*g_active_iscript_unit)->order_target.unit)
					{
						u32 target_x = (u16)target->sprite->position.x;
						u32 target_y = (u16)target->sprite->position.y;
						u32 unit_x = (u16)(*g_active_iscript_unit)->sprite->position.x;
						u32 unit_y = (u16)(*g_active_iscript_unit)->sprite->position.y;
						char unit_angle = helper::get_direction_from_points(unit_x, unit_y, target_x, target_y);
						if (helper::get_difference_between_points((u32)target_angle, unit_angle) < test_angle)
						{
							program_state->program_counter = new_pc;
						}
					}
					break;
				}
				case IscriptOpcodes::opc_curdirectcondjmp:
				{
					u16 target_angle = take_iscript_datum<u16>(program_state);
					u16 test_angle = take_iscript_datum<unsigned short>(program_state);
					u16 new_pc = take_iscript_datum<u16>(program_state);
					if (noop)
					{
						break;
					}
					if (helper::get_difference_between_points((u32)target_angle, (*g_active_iscript_unit)->current_direction_1) < test_angle)
					{
						program_state->program_counter = new_pc;
					}
					break;
				}
				case IscriptOpcodes::opc_imgulnextid:
				{
					char x = take_iscript_datum<char>(program_state);
					char y = take_iscript_datum<char>(program_state);
					if (noop)
					{
						break;
					}
					helper::iscript_create_image(image, image->id + 1, x + image->horizontal_offset, (u8)(image->vertical_offset + y), 0);
					break;
				}
				case IscriptOpcodes::opc_liftoffcondjmp:
				{
					u16 jump_pos = take_iscript_datum<u16>(program_state);
					if (noop)
					{
						break;
					}
					if ((*g_active_iscript_unit)->status & UnitStatus::InAir)
					{
						program_state->program_counter = jump_pos;
					}
					break;
				}
				case IscriptOpcodes::opc_warpoverlay:
				{
					u16 arg = take_iscript_datum<u16>(program_state);
					if (noop)
					{
						break;
					}
					image->flags |= ImageFlags::Redraw;
					image->standard.draw_func_param = (void*)arg;
					break;
				}
				case IscriptOpcodes::opc_orderdone:
				{
					u8 arg = take_iscript_datum<u8>(program_state);
					if (noop)
					{
						break;
					}
					(*g_active_iscript_entity)->order_signal &= ~arg;
					break;
				}
				case IscriptOpcodes::opc_grdsprol:
				{
					u16 sprite_id = take_iscript_datum<u16>(program_state);
					char x = take_iscript_datum<char>(program_state);
					char y = take_iscript_datum<char>(program_state);

					if (noop)
					{
						break;
					}
					if (helper::can_unit_id_fit_at_xy(x + image->parent_sprite->position.x + image->horizontal_offset, y + image->parent_sprite->position.y + image->vertical_offset, UnitId::TerranMaverick)) // Not sure why Maverick is used here. -Pr0nogo
					{
						iscript_create_sprite(image, sprite_id, x, y, image->parent_sprite->elevation + 1);
					}
					break;
				}
				case IscriptOpcodes::opc_dogrddamage:
					if (noop)
					{
						break;
					}
				//	CBullet_Damage(iscript_bullet);	// This is a fatal error call, according to IDA. Not present in our IDA decompilation of this opcode. -Pr0nogo
					break;
				default:
				{
				} break;
			}
		}
	}

	// HOOKING \\

	void __declspec(naked) initialize_image_data_wrapper() { // 004D5A50
		static Image* image;
		static Sprite* sprite;
		static u32 image_id;
		static s32 horizontal_offset;
		static s32 vertical_offset;
		static u32 direction;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV horizontal_offset, EAX
			MOV EAX, [EBP + 0x0C]
			MOV vertical_offset, EAX
			POP EAX
			MOV image_id, ESI
			MOV sprite, EDI
			MOV image, EAX
			PUSHAD
		}
		hooks::initialize_image_data(image, sprite, image_id, (s8)horizontal_offset, (s8)vertical_offset);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) set_image_direction_wrapper() { // 004D5F80
		static Image* image;
		static u32 direction;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV direction, EAX
			POP EAX
			MOV image, ESI
			PUSHAD
		}
		hooks::set_image_direction(image, (u8)direction);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) update_image_direction_wrapper() { // 004D5EA0
		static Image* image;
		static u32 unused;
		static u32 direction;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV direction, EAX
			POP EAX
			MOV unused, EDX
			MOV image, EAX
			PUSHAD
		}
		hooks::update_image_direction(image, (u8)direction);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) play_image_iscript_wrapper() { // 004D8470
		static Image* image;
		static u32 animation;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV animation, EAX
			POP EAX
			MOV image, ECX
			PUSHAD
		}
		hooks::play_image_iscript(image, (u8)animation);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) ground_attack_init_wrapper() { // 00488660
		static u16 x_pos;
		static u16 y_pos;
		__asm {
			MOV y_pos, AX
			MOV x_pos, CX
			PUSHAD
		}
		hooks::ground_attack_init(x_pos, y_pos);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) play_warp_in_overlay_wrapper() { // 004D8500
		static Image* image;
		static u32 unused;
		__asm {
			MOV image, EAX
			MOV unused, ECX
			PUSHAD
		}
		hooks::play_warp_in_overlay(image, unused);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) image_update_warp_flash_wrapper() { // 004D8590
		static Image* image;
		__asm {
			MOV image, ECX
			PUSHAD
		}
		hooks::image_update_warp_flash(image);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) iscript_opcode_cases_wrapper() { // 004D74C0
		static Image* image;
		static void* data;
		static IscriptProgram* program_state;
		static u32 noop;
		static u32* distance_moved;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV program_state, EAX
			MOV EAX, [EBP + 0x0C]
			MOV noop, EAX
			MOV EAX, [EBP + 0x10]
			MOV distance_moved, EAX
			POP EAX
			MOV data, EDX		// ignored, unused
			MOV image, ECX
			PUSHAD
		}
		hooks::iscript_opcode_cases(image, program_state, noop, distance_moved);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) iscript_fireweapon_wrapper() { // 00479D40
		static Unit* unit;
		static u8 weapon_id;
		__asm {
			MOV weapon_id, BL
			MOV unit, EAX
			PUSHAD
		}
		hooks::iscript_fireweapon(unit, weapon_id);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) iscript_attackwith_wrapper() { // 00479D60
		static Unit* attacker;
		static u8 is_ground_weapon;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV is_ground_weapon, AL
			POP EAX
			MOV attacker, EAX
			PUSHAD
		}
		hooks::iscript_attackwith(attacker, is_ground_weapon);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) iscript_useweapon_wrapper() { // 00479C30
		static Unit* unit;
		static u16 weapon_id;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV weapon_id, AX
			POP EAX
			MOV unit, ESI
			PUSHAD
		}
		hooks::iscript_useweapon(unit, weapon_id);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) iscript_nobrkcodeend_wrapper() { // 004753E0
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::iscript_nobrkcodeend(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void inject_image_hooks()
	{
		jump_patch(initialize_image_data_wrapper,	0x004D5A50, 2);
		jump_patch(set_image_direction_wrapper,		0x004D5F80, 1);
		jump_patch(update_image_direction_wrapper,	0x004D5EA0, 2);
		jump_patch(play_image_iscript_wrapper,		0x004D8470, 1);
		jump_patch(ground_attack_init_wrapper,		0x00488660, 3);
		jump_patch(play_warp_in_overlay_wrapper,	0x004D8500, 0);
		jump_patch(image_update_warp_flash_wrapper,	0x004D8590, 1);
	//	jump_patch(iscript_opcode_cases_wrapper,	0x004D74C0, 1);
		jump_patch(iscript_fireweapon_wrapper,		0x00479D40, 4);
		jump_patch(iscript_attackwith_wrapper,		0x00479D60, 1);
		jump_patch(iscript_useweapon_wrapper,		0x00479C30, 3);
		jump_patch(iscript_nobrkcodeend_wrapper,	0x004753E0, 4);
	}
}