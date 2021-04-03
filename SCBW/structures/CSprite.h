//Based on BWAPI's BW/CSprite.h

#pragma once
#include "common.h"
#include "CList.h"
#include "../enumerations.h"
#pragma pack(1)

//V241 for VS2008

struct CImage;

//note: the flag number is chosen according to the value used in 
//<< flag operations, that's why it start with 0.
namespace CSprite_Flags {
	enum Enum : uint8 {
		DrawSelCircle = 0x01,		//flag 0: 0x01, Draw selection circle.
		Flag01 = 0x02,		//flag 1: 0x02
		Flag02 = 0x04,		//flag 2: 0x04
		Selected = 0x08,		//flag 3: 0x08, Selected.
		Flag04 = 0x10,		//flag 4: 0x10
		Hidden = 0x20,		//flag 5: 0x20, Hidden
		Burrowed = 0x40,		//flag 6: 0x40, Burrowed
		IscriptCode = 0x80,		//flag 7: 0x80, Iscript unbreakable code section.
	};
}

struct CSprite {
	//Utility method definitions added by pastelmind

	  /// Makes the sprite play the specified Iscript animation entry.
	void playIscriptAnim(IscriptAnimation::Enum animation, bool bReplaceCurrentScript = true);

	/// AKA SpriteDestructor() @ 0x00497B40
	/// Removes the sprite from the linked list of valid images. This is different
	/// from a class destructor, since CSprite objects are stored in a static
	/// array and never truly destroyed.
	void free();

	/// AKA fixSpritePosition @ 0x00497A10
	/// Moves the sprite to the new position.
	void setPosition(uint16 x, uint16 y);

	/// Creates an image overlay over the main image of this sprite.
	CImage* createOverlay(uint32 imageId, int8 x = 0, int8 y = 0, uint32 direction = 0);
	CImage* createUnderlay(uint32 imageId, int8 x = 0, int8 y = 0, uint32 direction = 0);

	/// Creates an image overlay on top of all other images of this sprite.
	CImage* createTopOverlay(uint32 imageId, int8 x = 0, int8 y = 0, uint32 direction = 0);
	CImage* createTopUnderlay(uint32 imageId, int8 x = 0, int8 y = 0, uint32 direction = 0);

	void playForOverlay(uint32 imageId, uint8 anim);
	/// Removes the first image overlay with an ID value between @p imageIdStart
	/// and @p imageIdEnd.
	void removeOverlay(uint32 imageIdStart, uint32 imageIdEnd);

	/// Removes the first image overlay with the given image ID.
	void removeOverlay(uint32 imageId);

	/// Returns the overlay image of this sprite that has the given @p imageId.
	/// If the image cannot be found, returns nullptr.
	CImage* getOverlay(uint16 imageId) const;

	/// Checks whether this sprite is visible to @p playerId (i.e. not covered by
	/// the fog of war). This does NOT check detectability.
	bool isVisibleTo(uint8 playerId) const;

	////////////////////////////////////////////////////////////////
	//Actual data structure

	/*0x00*/ CLink<CSprite> link;
	/*0x08*/ uint16       spriteId;
	/*0x0A*/ uint8        playerId;          // officially "creator"
	/*0x0B*/ uint8        selectionIndex;    // 0 <= selectionIndex <= 11. Index in the selection area at bottom of screen.
	/*0x0C*/ uint8        visibilityFlags;   // Determines whether the sprite is visible (not hidden by the fog-of-war).
	/*0x0D*/ uint8        elevationLevel;
	/*0x0E*/ uint8        flags;				// Use CSprite_Flags
	/*0x0F*/ uint8        selectionTimer;
	/*0x10*/ uint16       index;				//uint32->uint16 based on BWAPI/Source/BW/CSprite.h
	/*0x12*/ uint8		   width;		//based on BWAPI/Source/BW/CSprite.h
	/*0x13*/ uint8		   height;		//based on BWAPI/Source/BW/CSprite.h
	/*0x14*/ Point16   position;
	/*0x18*/ CImage* mainGraphic;    // officially "pImagePrimary"
	/*0x1C*/ CList<CImage> images;      // officially "pImageHead" and "pImageTail"

	////////////////////////////////////////////////////////////////////
	// Official Broodwar methods (from beta), ignore these
	/*
	bool  Dead() const;
	bool  HasDamageFrames() const;
	*/
};

C_ASSERT(sizeof(CSprite) == 36);
//static_assert(sizeof(CSprite) == 36, "The size of the CSprite structure is invalid");

#pragma pack()
