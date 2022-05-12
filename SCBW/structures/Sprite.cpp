#include "Sprite.h"
#include "Image.h"
#include "../scbwdata.h"
#include <algorithm>
#include <cassert>

// Functionally identical to playSpriteIscript() (offset 0x00499D00)
void Sprite::playIscriptAnim(IscriptAnimation::Enum animation, bool bReplaceCurrentScript)
{
	assert(this);

	if (bReplaceCurrentScript || !(this->flags & SpriteFlags::IscriptCode))
	{
		for (Image* img = this->images.head; img != nullptr; img = img->link.next)
		{
			img->playIscriptAnim(animation);
		}
	}
}

// Equivalent to SpriteDestructor @ 0x00497B40
void Sprite::free()
{
	assert(this);

	Image* image = this->images.head;
	s32 sprite_row_position;

	while (image != nullptr)
	{
		Image* nextImage = image->link.next;
		image->free();
		image = nextImage;
	}

	if (((s32)this->position.y) < 0)
	{
		sprite_row_position = (this->position.y + 31) / 32;
	}
	else
	{
		sprite_row_position = (this->position.y) / 32;
	}

	if (sprite_row_position < 0)
	{
		sprite_row_position = 0;
	}
	else if (sprite_row_position >= mapTileSize->height)
	{
		sprite_row_position = mapTileSize->height - 1;
	}

	// remove the sprite from the spritesOnTileRow list
	if (spritesOnTileRow->heads[sprite_row_position] == this)
	{
		spritesOnTileRow->heads[sprite_row_position] = this->link.next;
	}

	if (spritesOnTileRow->tails[sprite_row_position] == this)
	{
		spritesOnTileRow->tails[sprite_row_position] = this->link.prev;
	}

	//untie the sprite from the previous and next one
	if (this->link.prev != nullptr)
	{
		(this->link.prev)->link.next = this->link.next;
	}

	if (this->link.next != nullptr)
	{
		(this->link.next)->link.prev = this->link.prev;
	}

	this->link.prev = nullptr;
	this->link.next = nullptr;

	// perform identically to original code
	unusedSprites->insertAfterHead<&Sprite::link>(this);
}


void Sprite::setPosition(u16 x, u16 y)
{
	assert(this);
	if (this->position.x == x && this->position.y == y)
	{
		return;
	}

	const s32 oldTileY = CLAMP(this->position.y / 32, 0, mapTileSize->height - 1);
	const s32 newTileY = CLAMP(y / 32, 0, mapTileSize->height - 1);

	this->position.x = x;
	this->position.y = y;

	if (oldTileY != newTileY)
	{
		const ListExtern<Sprite, &Sprite::link>
			spritesOnOldTileRow(spritesOnTileRow->heads[oldTileY],
				spritesOnTileRow->tails[oldTileY]),
			spritesOnNewTileRow(spritesOnTileRow->heads[newTileY],
				spritesOnTileRow->tails[newTileY]);

		spritesOnOldTileRow.unlink(this);
		spritesOnNewTileRow.insertAfterHead(this);
	}

	for (Image* i = this->images.head; i; i = i->link.next)
	{
		i->flags |= ImageFlags::Redraw;
	}
}

//-------- Create overlay --------//

void createUpdateImageSomething(Image* image)
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

void updateImageDirection(Image* image, u32 direction)
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

// Identical to function @ 0x00498E00
Image* Sprite::createOverlay(u32 imageId, s8 offsetX, s8 offsetY, u32 direction)
{
	assert(this);
	if (unusedImages.head == nullptr)
	{
		return nullptr;
	}
	Image* overlay = unusedImages.popHead();

	if (overlay)
	{
		if (this->images.head)
		{
			this->images.insertBefore<&Image::link>(overlay, this->mainGraphic);
		}
		else
		{
			this->mainGraphic = overlay;
			this->images.insertAfterHead<&Image::link>(overlay);
		}

		overlay->initializeData(this, imageId, offsetX, offsetY);
		createUpdateImageSomething(overlay);
		updateImageDirection(overlay, direction);
	}
	return overlay;
}

Image* Sprite::createUnderlay(u32 imageId, s8 offsetX, s8 offsetY, u32 direction)
{
	assert(this);
	if (unusedImages.head == nullptr)
	{
		return nullptr;
	}
	Image* underlay = unusedImages.popHead();

	if (underlay)
	{
		if (this->images.head)
		{
			this->images.insertAfter<&Image::link>(underlay, this->mainGraphic);
		}
		else
		{
			this->mainGraphic = underlay;
			this->images.insertBeforeHead<&Image::link>(underlay);
		}

		underlay->initializeData(this, imageId, offsetX, offsetY);
		createUpdateImageSomething(underlay);
		updateImageDirection(underlay, direction);
	}
	return underlay;
}

// Identical to function @ 0x00498EA0
Image* Sprite::createTopOverlay(u32 imageId, s8 offsetX, s8 offsetY, u32 direction)
{
	assert(this);
	if (unusedImages.head == nullptr)
	{
		return nullptr;
	}
	Image* overlay = unusedImages.popHead();

	if (overlay)
	{
		if (this->images.head)
		{
			this->images.insertBeforeHead<&Image::link>(overlay);
		}
		else
		{
			this->mainGraphic = overlay;
			this->images.insertAfterHead<&Image::link>(overlay);
		}

		overlay->initializeData(this, imageId, offsetX, offsetY);
		createUpdateImageSomething(overlay);
		updateImageDirection(overlay, direction);
	}
	return overlay;
}

Image* Sprite::createTopUnderlay(u32 imageId, s8 offsetX, s8 offsetY, u32 direction)
{
	assert(this);
	if (unusedImages.head == nullptr)
	{
		return nullptr;
	}
	Image* underlay = unusedImages.popHead();

	if (underlay)
	{
		if (this->images.head)
		{
			this->images.insertAfterHead<&Image::link>(underlay);
		}
		else
		{
			this->mainGraphic = underlay;
			this->images.insertBeforeHead<&Image::link>(underlay);
		}

		underlay->initializeData(this, imageId, offsetX, offsetY);
		createUpdateImageSomething(underlay);
		updateImageDirection(underlay, direction);
	}
	return underlay;
}

//-------- Remove overlay --------//

//Based on function @ 0x004E5CF0
void Sprite::removeOverlay(u32 imageIdStart, u32 imageIdEnd)
{
	assert(this);
	assert(imageIdStart <= imageIdEnd);

	for (Image* image = this->images.head; image; image = image->link.next)
	{
		if (imageIdStart <= image->id && image->id <= imageIdEnd)
		{
			image->free();
			return;
		}
	}
}

void Sprite::removeOverlay(u32 imageId)
{
	assert(this);

	for (Image* image = this->images.head; image; image = image->link.next)
	{
		if (image->id == imageId)
		{
			image->free();
			return;
		}
	}
}

void Sprite::playForOverlay(u32 imageId, u8 anim)
{
	assert(this);

	for (Image* image = this->images.head; image; image = image->link.next)
	{
		if (image->id == imageId)
		{
			image->playIscriptAnim(IscriptAnimation::Death);
			return;
		}
	}
}

//-------- Get overlay --------//

//Identical to function @ 0x00497460
Image* Sprite::getOverlay(u16 imageId) const
{
	assert(this);

	for (Image* image = this->images.head; image; image = image->link.next)
	{
		if (image->id == imageId)
		{
			return image;
		}
	}

	return nullptr;
}

//-------- Other --------//

bool Sprite::isVisibleTo(u8 playerId) const
{
	assert(this);
	return (this->visibilityFlags & (1 << playerId)) != 0;
}