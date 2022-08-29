#include "Image.h"
#include "Sprite.h"
#include <SCBW/api.h>
#include <SCBW/scbwdata.h>
#include <cassert>

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

//Loosely based on code at @ 0x004D5A50
void Image::set_remapping(ColorRemapping::Enum remapping)
{
	assert(this);
	this->draw_func_param = g_color_shift[remapping].data;
}
//Identical to function @ 0x004D5A50
void Image::initialize_data(Sprite* parent, u16 imageId, s8 x, s8 y)
{
	assert(this);
	this->id = imageId;

	this->grp_file = g_image_grp_graphics[imageId];
	this->flags = ((images_dat::is_turnable[imageId] & 1) << 3) | ((images_dat::is_clickable[imageId] & 1) << 5);
	this->frameset = 0;
	this->direction = 0;
	this->frame_index = 0;
	this->parent_sprite = parent;
	this->horizontal_offset = x;
	this->vertical_offset = y;
	this->grp_size = Box16();
	this->draw_func_param = 0;

	//Initialize iscript data
	this->iscript_header_offset = 0;
	this->iscript_offset = 0;
	this->unknown14 = 0;
	this->animation = IscriptAnimation::Init;
	this->wait = 0;
	/*
	if (images_dat::rle_function[imageId] == PaletteType::RLE_PLAYER_SIDE)
		*(u32*)(&this->draw_func) = parent->player;
	if (images_dat::rle_function[imageId] == PaletteType::RLE_EFFECT)
		this->draw_func = g_color_shift[images_dat::remapping[imageId]].data;*/

	if (images_dat::rle_function[imageId] == PaletteType::RLE_PLAYER_SIDE)
	{
		*(u32*)(&this->draw_func_param) = parent->player;
	}
	if (images_dat::rle_function[imageId] == PaletteType::RLE_EFFECT)
	{
		this->draw_func_param = g_color_shift[images_dat::remapping[imageId]].data;
	}
}


///same as function 0x004D5EA0, direction is 0-31
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
