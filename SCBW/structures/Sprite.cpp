#include "Sprite.h"
#include "Image.h"
#include "../scbwdata.h"
#include <algorithm>
#include <cassert>
#include "data_ext.h"

// Functionally identical to play_sprite_iscript() (offset 0x00499D00)
void Sprite::play_iscript_animation(IscriptAnimation::Enum animation, bool bReplaceCurrentScript)
{
	assert(this);

	if (bReplaceCurrentScript || !(this->flags & SpriteFlags::IscriptCode))
	{
		for (Image* img = this->images.head; img != nullptr; img = img->link.next)
		{
			img->play_iscript_animation(animation);
		}
	}
}

// Equivalent to SpriteDestructor @ 0x00497B40
void Sprite::free()
{
	assert(this);

	reset_sprite_ext(this);

	Image* image = this->images.head;
	s32 sprite_row_position;
	while (image)
	{
		Image* next_image = image->link.next;
		image->free();
		image = next_image;
	}

	if (this->position.y < 0)
	{
		sprite_row_position = this->position.y + 31 / 32;
	} else {
		sprite_row_position = this->position.y / 32;
	}

	if (sprite_row_position < 0)
	{
		sprite_row_position = 0;
	} else if (sprite_row_position >= g_map_tile_size->height)
	{
		sprite_row_position = g_map_tile_size->height - 1;
	}

	// remove the sprite from the g_sprites_on_tile_row list
	if (g_sprites_on_tile_row->heads[sprite_row_position] == this)
	{
		g_sprites_on_tile_row->heads[sprite_row_position] = this->link.next;
	}

	if (g_sprites_on_tile_row->tails[sprite_row_position] == this)
	{
		g_sprites_on_tile_row->tails[sprite_row_position] = this->link.prev;
	}

	// untie the sprite from the previous and next one
	if (this->link.prev)
	{
		(this->link.prev)->link.next = this->link.next;
	}

	if (this->link.next)
	{
		(this->link.next)->link.prev = this->link.prev;
	}

	this->link.prev = 0;
	this->link.next = 0;

	// perform identically to original code
	g_free_sprites->insertAfterHead<&Sprite::link>(this);
}


void Sprite::set_position(u16 x, u16 y)
{
	assert(this);
	if (this->position.x == x && this->position.y == y)
	{
		return;
	}

	const s32 old_tile_y = CLAMP(this->position.y / 32, 0, g_map_tile_size->height - 1);
	const s32 new_tile_y = CLAMP(y / 32, 0, g_map_tile_size->height - 1);

	this->position.x = x;
	this->position.y = y;

	if (old_tile_y != new_tile_y)
	{
		const ListExtern<Sprite, &Sprite::link>
			spritesOnOldTileRow(g_sprites_on_tile_row->heads[old_tile_y],
				g_sprites_on_tile_row->tails[old_tile_y]),
			spritesOnNewTileRow(g_sprites_on_tile_row->heads[new_tile_y],
				g_sprites_on_tile_row->tails[new_tile_y]);

		spritesOnOldTileRow.unlink(this);
		spritesOnNewTileRow.insertAfterHead(this);
	}

	for (Image* i = this->images.head; i; i = i->link.next)
	{
		i->flags |= ImageFlags::Redraw;
	}
}

//-------- Create overlay --------//

void create_update_image_something(Image* image)
{
	const u32 Func_createUpdateImageSomething = 0x004D66B0;

	__asm
	{
		PUSHAD
		MOV EAX, image
		CALL Func_createUpdateImageSomething
		POPAD
	}
}

void update_image_direction(Image* image, u32 direction)
{
	const u32 Func_updateImageDirection = 0x004D5EA0;

	__asm
	{
		PUSHAD
		PUSH direction
		MOV EAX, image
		CALL Func_updateImageDirection
		POPAD
	}
}

//-------- Create overlay --------//

static void inherit_image_palette(Unit* unit, Image* image, Image* main_image) {
	if (!image->palette_type && unit) {
		if (unit->status & UnitStatus::IsHallucination
		&& (unit->player == *g_local_nation_id || *g_is_in_replay))
		{
			image->palette_type = PaletteType::RLE_SHIFT;
			image->update_fxn = g_sg_update_funcs[PaletteType::RLE_SHIFT].function;
			image->render_fxn = image->flags & 2 ? g_sg_draw_funcs[PaletteType::RLE_SHIFT].function_2 : g_sg_draw_funcs[PaletteType::RLE_SHIFT].function_1;
			image->flags |= 1;
			image->standard.draw_func_param = 0;
		}
		if (unit->status & (UnitStatus::RequiresDetection | UnitStatus::Cloaked)) {
			helper::copy_image_palette_type(image, main_image);
		}
	}
}

// Identical to function @ 0x00498E00
Image* Sprite::create_overlay(u32 imageId, s8 offsetX, s8 offsetY, u32 direction, Unit* palette_unit) {
	assert(this);
	if (!unusedImages.head) {
		return 0;
	}

	Image* overlay = unusedImages.popHead();
	if (overlay) {
		if (this->images.head) {
			this->images.insertBefore<&Image::link>(overlay, this->main_graphic);
		} else {
			this->main_graphic = overlay;
			this->images.insertAfterHead<&Image::link>(overlay);
		}

		overlay->initialize_data(this, imageId, offsetX, offsetY);
		create_update_image_something(overlay);
		update_image_direction(overlay, direction);
		if (palette_unit) {
			inherit_image_palette(palette_unit, overlay, this->main_graphic);
		}
	}
	return overlay;
}

Image* Sprite::create_underlay(u32 imageId, s8 offsetX, s8 offsetY, u32 direction, Unit* palette_unit) {
	assert(this);
	if (!unusedImages.head) {
		return 0;
	}

	Image* underlay = unusedImages.popHead();
	if (underlay) {
		if (this->images.head) {
			this->images.insertAfter<&Image::link>(underlay, this->main_graphic);
		} else {
			this->main_graphic = underlay;
			this->images.insertBeforeHead<&Image::link>(underlay);
		}

		underlay->initialize_data(this, imageId, offsetX, offsetY);
		create_update_image_something(underlay);
		update_image_direction(underlay, direction);
		if (palette_unit) {
			inherit_image_palette(palette_unit, underlay, this->main_graphic);
		}
	}
	return underlay;
}

// Identical to function @ 0x00498EA0
Image* Sprite::create_top_overlay(u32 imageId, s8 offsetX, s8 offsetY, u32 direction, Unit* palette_unit) {
	assert(this);
	if (!unusedImages.head) {
		return 0;
	}

	Image* overlay = unusedImages.popHead();
	if (overlay) {
		if (this->images.head) {
			this->images.insertBeforeHead<&Image::link>(overlay);
		} else {
			this->main_graphic = overlay;
			this->images.insertAfterHead<&Image::link>(overlay);
		}

		overlay->initialize_data(this, imageId, offsetX, offsetY);
		create_update_image_something(overlay);
		update_image_direction(overlay, direction);
		if (palette_unit) {
			inherit_image_palette(palette_unit, overlay, this->main_graphic);
		}
	}
	return overlay;
}

Image* Sprite::create_top_underlay(u32 imageId, s8 offsetX, s8 offsetY, u32 direction, Unit* palette_unit) {
	assert(this);
	if (!unusedImages.head) {
		return 0;
	}

	Image* underlay = unusedImages.popHead();
	if (underlay) {
		if (this->images.head) {
			this->images.insertAfterHead<&Image::link>(underlay);
		} else {
			this->main_graphic = underlay;
			this->images.insertBeforeHead<&Image::link>(underlay);
		}

		underlay->initialize_data(this, imageId, offsetX, offsetY);
		create_update_image_something(underlay);
		update_image_direction(underlay, direction);
		if (palette_unit) {
			inherit_image_palette(palette_unit, underlay, this->main_graphic);
		}
	}
	return underlay;
}

//-------- Remove overlay --------//

//Based on function @ 0x004E5CF0
void Sprite::remove_overlay(u32 image_id_start, u32 image_id_end)
{
	assert(this);
	assert(image_id_start <= image_id_end);
	for (Image* image = this->images.head; image; image = image->link.next)
	{
		if (image_id_start <= image->id && image->id <= image_id_end)
		{
			image->free();
			return;
		}
	}
}

void Sprite::remove_overlay(u32 image_id)
{
	assert(this);
	for (Image* image = this->images.head; image; image = image->link.next)
	{
		if (image->id == image_id)
		{
			image->free();
			return;
		}
	}
}

void Sprite::play_for_overlay(u32 image_id, u8 anim)
{
	assert(this);
	for (Image* image = this->images.head; image; image = image->link.next)
	{
		if (image->id == image_id)
		{
			image->play_iscript_animation((IscriptAnimation::Enum)anim);
			return;
		}
	}
}

//-------- Get overlay --------//

//Identical to function @ 0x00497460
Image* Sprite::get_overlay(u16 image_id) const
{
	assert(this);
	for (Image* image = this->images.head; image; image = image->link.next)
	{
		if (image->id == image_id)
		{
			return image;
		}
	}
	return 0;
}

//-------- Other --------//

bool Sprite::is_visible_to(u8 player) const
{
	assert(this);
	return (this->visibility_status & (1 << player)) != 0;
}

namespace hooks {
	// Hooked to investigate crashes and desyncs. -Pr0nogo
	void update_visibility_hash(Sprite* sprite) // 00497C30
	{
		memset(g_current_visibility_hash, 0, sizeof(g_current_visibility_hash));
		u32 sprite_hash = *g_current_visibility_hash;
		u32 hor_line = 0;
		do
		{
			for (Sprite* i = g_horizontal_sprite_lines_begin[hor_line]; i; i = i->link.next)
			{
				if (g_is_valid_sprite[(unsigned __int16)i->id])
				{
					sprite_hash ^= i->visibility_status;
				}
			}
			++hor_line;
			++sprite_hash;
		} while (hor_line < 256);

		u32 elevation = sprite->elevation;
		if (sprite->elevation >= 256)
		{
			elevation = 255;
		}

		u32 filter = 0;
		for (u32 j = (u32)sprite; j <= elevation; ++j)
		{
			filter = ((g_region_vision_filter[j] ^ filter) >> 31) | 2 * (g_region_vision_filter[j] ^ filter);
		}
		s32 vision = (filter >> 16) ^ filter;
		*g_region_vision_hash = vision ^ vision + 1;
		*g_current_hash_offset = (u32)sprite;
	}
}