#pragma once
#include "common.h"
#include "List.h"
#include "../enumerations.h"
#pragma pack(1)

struct Image;

// Note: the flag number is chosen according to the value used in << flag operations, that's why it start with 0.
namespace SpriteFlags
{
	enum Enum : u8
	{
		DrawSelCircle = 0x01,	// flag 0: Draw selection circle
		Flag01 = 0x02,			// flag 1:
		Flag02 = 0x04,			// flag 2:
		Selected = 0x08,		// flag 3: Selected
		Flag04 = 0x10,			// flag 4:
		Hidden = 0x20,			// flag 5: Hidden
		Burrowed = 0x40,		// flag 6: Burrowed
		IscriptCode = 0x80,		// flag 7: Iscript unbreakable code section
	};
}

struct Sprite
{
	// Makes the sprite play the specified Iscript animation entry.
	void play_iscript_animation(IscriptAnimation::Enum animation, bool bReplaceCurrentScript = true);

	// AKA SpriteDestructor() @ 0x00497B40
	// Removes the sprite from the linked list of valid images. This is different from a struct destructor, since Sprite objects are stored in a static array and never truly destroyed.
	void free();

	// AKA fixSpritePosition @ 0x00497A10
	// Moves the sprite to the new position.
	void set_position(u16 x, u16 y);

	Image* create_overlay(u32 imageId, s8 offsetX = 0, s8 offsetY = 0, u32 direction = 0); // Creates an image overlay over the main image of this sprite.
	Image* create_underlay(u32 imageId, s8 offsetX = 0, s8 offsetY = 0, u32 direction = 0);

	Image* create_top_overlay(u32 imageId, s8 offsetX = 0, s8 offsetY = 0, u32 direction = 0); // Creates an image overlay on top of all other images of this sprite.
	Image* create_top_underlay(u32 imageId, s8 offsetX = 0, s8 offsetY = 0, u32 direction = 0);

	void play_for_overlay(u32 imageId, u8 anim);	
	void remove_overlay(u32 imageIdStart, u32 imageIdEnd); // Removes the first image overlay with an ID value between @p imageIdStart and @p imageIdEnd.
	void remove_overlay(u32 imageId); // Removes the first image overlay with the given image ID.
	Image* get_overlay(u16 imageId) const; // Returns the overlay image of this sprite that has the given @p imageId. If the image cannot be found, returns nullptr.

	bool is_visible_to(u8 player) const; // Checks whether this sprite is visible to @p player (i.e. not covered by the fog of war). This does NOT check detectability.

	////////////////////////////////////////////////////////////////
	//Actual data structure

	/*0x00*/ Link<Sprite>	link;
	/*0x08*/ u16			id;
	/*0x0A*/ u8				player;				// officially "creator"
	/*0x0B*/ u8				selection_index;	// 0 <= selection_index <= 11. Index in the selection box at bottom of screen.
	/*0x0C*/ u8				visibility_status;	// Determines whether the sprite is visible (not hidden by the fog-of-war).
	/*0x0D*/ u8				elevation_level;
	/*0x0E*/ u8				flags;				// Use Sprite_Flags
	/*0x0F*/ u8				selection_timer;
	/*0x10*/ u16			index;				// u32->u16 based on BWAPI/Source/BW/Sprite.h
	/*0x12*/ u8				width;				// based on BWAPI/Source/BW/Sprite.h
	/*0x13*/ u8				height;				// based on BWAPI/Source/BW/Sprite.h
	/*0x14*/ Point16		position;
	/*0x18*/ Image*			main_graphic;		// officially "pImagePrimary"
	/*0x1C*/ List<Image>	images;				// officially "pImageHead" and "pImageTail"

	////////////////////////////////////////////////////////////////////
	// Official Broodwar methods (from beta), ignore these
	/*
	bool  dead() const;
	bool  has_damage_frames() const;
	*/
};

C_ASSERT(sizeof(Sprite) == 36);
//static_assert(sizeof(Sprite) == 36, "The count of the Sprite structure is invalid");

#pragma pack()