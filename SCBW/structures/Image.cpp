#include "Image.h"
#include "Sprite.h"
#include <SCBW/api.h>
#include <SCBW/scbwdata.h>
#include <cassert>
#include "hook_tools.h"

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

//should be equivalent to ImageDestructor @ 0x004D4CE0 with ESI = sprite
void Image::free()
{

	assert(this);

	if (!(screenLayers->game.hasBeenRefreshed))
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
	this->draw_func_param = g_color_shift[remapping].data;
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
		if (images_dat::is_turnable[image_id] & 1)
		{
			image->flags |= ImageFlags::HasRotationFrames;
		}
		if (images_dat::is_turnable[image_id] & 2)
		{
			image->flags |= ImageFlags::Omnidirectional; // New
		}
		if (images_dat::is_clickable[image_id] & 1)
		{
			image->flags |= ImageFlags::Clickable;
		}
		image->frameset = 0;
		image->direction = 0;
		image->frame_index = 0;
		image->parent_sprite = parent;
		image->horizontal_offset = horizontal_offset;
		image->vertical_offset = vertical_offset;
		image->grp_size = Box16();
		image->draw_func_param = 0;

		// Initialize iscript data
		image->iscript_header_offset = 0;
		image->iscript_offset = 0;
		image->unknown14 = 0;
		image->animation = IscriptAnimation::Init;
		image->wait = 0;

		if (images_dat::rle_function[image_id] == PaletteType::RLE_PLAYER_SIDE) {
			*(u32*)(&image->draw_func_param) = parent->player;
		}
		if (images_dat::rle_function[image_id] == PaletteType::RLE_EFFECT) {
			image->draw_func_param = g_color_shift[images_dat::remapping[image_id]].data;
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
				new_flip_flag = ImageFlags::Mirrored;
				new_direction = 32 - new_direction;
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
			u32 new_flip_flag = 0;
			u32 new_direction = direction;

			if (new_direction > 16 && !(image->flags & ImageFlags::Omnidirectional)) {
				new_direction = 32 - new_direction;
				new_flip_flag = ImageFlags::Mirrored;
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

	// Hooked to extend iscript.bin. -Pr0nogo
	void play_image_iscript(Image* image, u8 animation) { // 004D8470
		char final_anim = animation;
		if ((animation != IscriptAnimation::Death || image->animation != IscriptAnimation::Death)
		&&  (image->flags & ImageFlags::Flag04 || !animation || animation == IscriptAnimation::Death)) {
			char image_anim = image->animation;
			if (animation != image_anim || animation != IscriptAnimation::Walking && animation != IscriptAnimation::IsWorking) {
				if (animation == IscriptAnimation::GndAttkRpt) {
					if (image_anim != IscriptAnimation::GndAttkRpt && image_anim != IscriptAnimation::GndAttkInit) {
						final_anim = IscriptAnimation::GndAttkInit;
					}
				} else if (animation == IscriptAnimation::AirAttkRpt
					&& image_anim != IscriptAnimation::AirAttkRpt
					&& image_anim != IscriptAnimation::AirAttkInit) {
					final_anim = IscriptAnimation::AirAttkInit;
				}
				u32 offset = image->iscript_header_offset;
				char* data = *g_iscript_data;
				image->animation = final_anim;
				image->iscript_offset = *(u16*)&data[2 * final_anim + 8 + offset];
				image->wait = 0;
				image->unknown14 = 0;
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
			if (i->flags & ImageFlags::Flag04) {
				u32 header = i->iscript_header_offset;
				char* data = *g_iscript_data;
				i->animation = 2;
				i->iscript_offset = *(u16*)&data[header + 12];
				i->wait = 0;
				i->unknown14 = 0;
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
		image->unknown14 = 0;
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
		image->flags = flags | 1;
		helper::iscript_opcode_cases(image, &image->iscript_header_offset, 0, 0); // guessed second parameter
	}

	// Hooked to extend iscript.bin. -Pr0nogo
	// TODO: Fix an error where warp in overlay doesn't disappear, when this hook is enabled -Veeq7
	void iscript_something_death(Image* image) { // 004D8590
		u8* param = (u8*)&image->draw_func_param + 1;
		if (*param) {
			*param -= 1;
		} else if (*param >= 63u) {
			*param = 2;
			if (image->animation != IscriptAnimation::Death) { // IscriptAnimation::Cuckdeath
				u16 header = image->iscript_header_offset;
				image->animation = 1; 
				char* data = *g_iscript_data;
				header = *(u16*)&data[header + 10];
				image->wait = 0;
				image->unknown14 = 0;
				image->iscript_offset = header;
				helper::iscript_opcode_cases(image, &image->iscript_header_offset, 0, 0);
			}
			(*g_active_iscript_entity)->order_signal |= 1u;
			image->flags |= ImageFlags::Redraw;
		} else {
			*param += 1;
			image->flags |= ImageFlags::Redraw;
		}
	}

	// Ported from X405's Magnetar hook of iscript_opcode_cases to fix Omnidirectional shadows. -Pr0nogo
	// TODO: Add partial wrapper to make this code run, and add omnidirectional handling
	void iscript_opcode_followmaingraphic(Image* image, void* program_state, u32 noop, u32* distance_moved) {
		Image* main = image->parent_sprite->main_graphic;
		if (!noop && main)
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

	void __declspec(naked) iscript_something_death_wrapper() { // 004D8590
		static Image* image;
		__asm {
			MOV image, ECX
			PUSHAD
		}
		hooks::iscript_something_death(image);
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
		// jump_patch(iscript_something_death_wrapper,	0x004D8590, 1);
	}
}