#include "CSprite.h"
#include "CImage.h"
#include "../scbwdata.h"
#include <algorithm>
#include <cassert>

//Functionally identical to playSpriteIscript() (offset 0x00499D00)
void CSprite::playIscriptAnim(IscriptAnimation::Enum animation, bool bReplaceCurrentScript) {

	assert(this);

	if (bReplaceCurrentScript || !(this->flags & CSprite_Flags::IscriptCode))
		for (CImage* img = this->images.head; img != nullptr; img = img->link.next)
			img->playIscriptAnim(animation);

}

//Equivalent to SpriteDestructor @ 0x00497B40
void CSprite::free() {

	assert(this);

	CImage* image = this->images.head;
	int32 sprite_row_position;

	while (image != nullptr) {
		CImage* nextImage = image->link.next;
		image->free();
		image = nextImage;
	}

	if (((int32)this->position.y) < 0)
		sprite_row_position = (this->position.y + 31) / 32;
	else
		sprite_row_position = (this->position.y) / 32;

	if (sprite_row_position < 0)
		sprite_row_position = 0;
	else
		if (sprite_row_position >= mapTileSize->height)
			sprite_row_position = mapTileSize->height - 1;

	//remove the sprite from the spritesOnTileRow list
	if (spritesOnTileRow->heads[sprite_row_position] == this)
		spritesOnTileRow->heads[sprite_row_position] = this->link.next;

	if (spritesOnTileRow->tails[sprite_row_position] == this)
		spritesOnTileRow->tails[sprite_row_position] = this->link.prev;

	//untie the sprite from the previous and next one
	if (this->link.prev != nullptr)
		(this->link.prev)->link.next = this->link.next;

	if (this->link.next != nullptr)
		(this->link.next)->link.prev = this->link.prev;

	this->link.prev = nullptr;
	this->link.next = nullptr;

	//perform identically to original code
	unusedSprites->insertAfterHead<&CSprite::link>(this);

}


void CSprite::setPosition(uint16 x, uint16 y) {
	assert(this);
	if (this->position.x == x && this->position.y == y) return;

	const int oldTileY = CLAMP(this->position.y / 32, 0, mapTileSize->height - 1);
	const int newTileY = CLAMP(y / 32, 0, mapTileSize->height - 1);

	this->position.x = x;
	this->position.y = y;

	if (oldTileY != newTileY) {
		const CListExtern<CSprite, &CSprite::link>
			spritesOnOldTileRow(spritesOnTileRow->heads[oldTileY],
				spritesOnTileRow->tails[oldTileY]),
			spritesOnNewTileRow(spritesOnTileRow->heads[newTileY],
				spritesOnTileRow->tails[newTileY]);

		spritesOnOldTileRow.unlink(this);
		spritesOnNewTileRow.insertAfterHead(this);
	}

	for (CImage* i = this->images.head; i; i = i->link.next)
		i->flags |= CImage_Flags::Redraw;
}


//-------- Create overlay --------//

void createUpdateImageSomething(CImage* image) {
	const uint32 Func_createUpdateImageSomething = 0x004D66B0;

	__asm {
		PUSHAD
		MOV EAX, image
		CALL Func_createUpdateImageSomething
		POPAD
	}
}

void updateImageDirection(CImage* image, uint32 direction) {
	const uint32 Func_updateImageDirection = 0x004D5EA0;

	__asm {
		PUSHAD
		PUSH direction
		MOV EAX, image
		CALL Func_updateImageDirection
		POPAD
	}
}

//-------- Create overlay --------//

//Identical to function @ 0x00498E00
CImage* CSprite::createOverlay(uint32 imageId, int8 x, int8 y, uint32 direction) {
	assert(this);
	CImage* overlay = unusedImages.popHead();

	if (overlay) {
		if (this->images.head)
			this->images.insertBefore<&CImage::link>(overlay, this->mainGraphic);
		else {
			this->mainGraphic = overlay;
			this->images.insertAfterHead<&CImage::link>(overlay);
		}

		overlay->initializeData(this, imageId, x, y);
		createUpdateImageSomething(overlay);
		updateImageDirection(overlay, direction);
	}
	return overlay;
}

CImage* CSprite::createUnderlay(uint32 imageId, int8 x, int8 y, uint32 direction) {
	assert(this);
	CImage* underlay = unusedImages.popHead();

	if (underlay) {
		if (this->images.head)
			this->images.insertAfter<&CImage::link>(underlay, this->mainGraphic);
		else {
			this->mainGraphic = underlay;
			this->images.insertBeforeHead<&CImage::link>(underlay);
		}

		underlay->initializeData(this, imageId, x, y);
		createUpdateImageSomething(underlay);
		updateImageDirection(underlay, direction);
	}
	return underlay;
}

//Identical to function @ 0x00498EA0
CImage* CSprite::createTopOverlay(uint32 imageId, int8 x, int8 y, uint32 direction) {
	assert(this);
	CImage* overlay = unusedImages.popHead();

	if (overlay) {
		if (this->images.head)
			this->images.insertBeforeHead<&CImage::link>(overlay);
		else {
			this->mainGraphic = overlay;
			this->images.insertAfterHead<&CImage::link>(overlay);
		}

		overlay->initializeData(this, imageId, x, y);
		createUpdateImageSomething(overlay);
		updateImageDirection(overlay, direction);
	}
	return overlay;
}
CImage* CSprite::createTopUnderlay(uint32 imageId, int8 x, int8 y, uint32 direction) {
	assert(this);
	CImage* underlay = unusedImages.popHead();

	if (underlay) {
		if (this->images.head)
			this->images.insertAfterHead<&CImage::link>(underlay);
		else {
			this->mainGraphic = underlay;
			this->images.insertBeforeHead<&CImage::link>(underlay);
		}

		underlay->initializeData(this, imageId, x, y);
		createUpdateImageSomething(underlay);
		updateImageDirection(underlay, direction);
	}
	return underlay;
}


//-------- Remove overlay --------//

//Based on function @ 0x004E5CF0
void CSprite::removeOverlay(uint32 imageIdStart, uint32 imageIdEnd) {
	assert(this);
	assert(imageIdStart <= imageIdEnd);

	for (CImage* image = this->images.head; image; image = image->link.next) {
		if (imageIdStart <= image->id && image->id <= imageIdEnd) {
			image->free();
			return;
		}
	}
}

void CSprite::removeOverlay(uint32 imageId) {
	assert(this);

	for (CImage* image = this->images.head; image; image = image->link.next) {
		if (image->id == imageId) {
			image->free();
			return;
		}
	}
}
void CSprite::playForOverlay(uint32 imageId, uint8 anim) {
	assert(this);

	for (CImage* image = this->images.head; image; image = image->link.next) {
		if (image->id == imageId) {
			image->playIscriptAnim(IscriptAnimation::Death);
			return;
		}
	}
}
//-------- Get overlay --------//

//Identical to function @ 0x00497460
CImage* CSprite::getOverlay(uint16 imageId) const {
	assert(this);

	for (CImage* image = this->images.head; image; image = image->link.next)
		if (image->id == imageId)
			return image;

	return nullptr;
}

//-------- Other --------//

bool CSprite::isVisibleTo(uint8 playerId) const {
	assert(this);
	return (this->visibilityFlags & (1 << playerId)) != 0;
}
