#pragma once
#include "common.h"
#include "List.h"
#include <BW/enumerations.h>
#pragma pack(1)

// Additions from bwapi/bwapi/BWMemoryEdit/Enums/RLE.cs
// Additions from bwapi/bwapi/BWAPI/Source/BW/Image.h

struct Sprite;
struct GrpHead;
struct GrpFrame;

/// Used by Image.palette_type
namespace PaletteType {
    enum Enum : u8 {
        Normal				 = 0,
        NoCloak				 = 1,
        HiddenCloaking		 = 2,
        HiddenCloaked		 = 3,
        HiddenDecloaking	 = 4,
        VisibleCloaking		 = 5,
        VisibleCloaked		 = 6,
        VisibleDecloaking	 = 7,
        EMP					 = 8,
        UseRemapping		 = 9,
        Shadow				 = 10,
        HPBar				 = 11,
        WarpTexture			 = 12,
        RLE_OUTLINE			 = 13,
        RLE_PLAYER_SIDE		 = 14,
        ImageBounds			 = 15,
        Hallucination		 = 16,
        WarpFlash			 = 17,
        WarpAnchor			 = 18,
        ShadowCloak			 = 19,
        HiddenCloaking_WA	 = 20,
        HiddenCloaked_WA	 = 21,
        HiddenDecloaking_WA	 = 22,
        VisibleCloaking_WA	 = 23,
        VisibleCloaked_WA	 = 24,
        VisibleDecloaking_WA = 25,
        HiddenCloaking_WT	 = 26,
        HiddenCloaked_WT	 = 27,
        HiddenDecloaking_WT	 = 28,
        VisibleCloaking_WT	 = 29,
        VisibleCloaked_WT	 = 30,
        VisibleDecloaking_WT = 31,
        HiddenCloaking_WF	 = 32,
        HiddenCloaked_WF	 = 33,
        HiddenDecloaking_WF	 = 34,
        VisibleCloaking_WF	 = 35,
        VisibleCloaked_WF	 = 36,
        VisibleDecloaking_WF = 37
    };
}

//note: the flag number is chosen according to the value used in 
//<< flag operations, that's why it start with 0.
namespace ImageFlags {
    enum Enum : u16 {
        Redraw            = 0x0001,	// flag 0   : 0x0001
        Mirrored          = 0x0002,	// flag 1   : 0x0002
        DontUpdate        = 0x0004,	// flag 2   : 0x0004
        HasRotationFrames = 0x0008,	// flag 3   : 0x0008
        UseFullIscript    = 0x0010,	// flag 4   : 0x0010
        Clickable         = 0x0020,	// flag 5   : 0x0020
        Invisible         = 0x0040,	// flag 6   : 0x0040
        UsesSpecialOffset = 0x0080,	// flag 7   : 0x0080
        Omnidirectional   = 0x0100,	// flag 8   : 0x0100 // New!
        Flag09            = 0x0200,	// flag 9   : 0x0200
        Flag10            = 0x0400,	// flag 10  : 0x0400
        Flag11            = 0x0800,	// flag 11  : 0x0800
        Flag12            = 0x1000,	// flag 12  : 0x1000
        Flag13            = 0x2000,	// flag 13  : 0x2000
        Flag14            = 0x4000,	// flag 14  : 0x4000
        Flag15            = 0x8000	// flag 15  : 0x8000
    };
}

struct Image {
    // Makes the image play the specified Iscript animation entry.
    void play_iscript_animation(IscriptAnimation::Enum animation);

    // AKA ImageDestructor() @ 0x004D4CE0
    // Removes the image from the linked list of valid images. This is different from a struct destructor, since Image objects are stored in a static array and never truly destroyed.
    void free();

    // Set the color remapping of this image to @ remapping.
    void set_remapping(ColorRemapping::Enum remapping);

    // Resets most of the data member values.
    void initialize_data(Sprite* parent, u16 image_id, s8 horizontal_offset, s8 vertical_offset);
    void set_image_direction(u8 direction);

    // Return a pointer to the current frame when called
    GrpFrame* get_current_frame() const;
    bool is_mirrored() const;

    void set_iscript(IscriptAnimation::Enum animation);

    ////////////////////////////////////////////////////////////////
    // Actual data structure

    /*0x00*/  Link<Image>	link;
    /*0x08*/  u16			id;
    /*0x0A*/  u8			palette_type;			// Use PaletteType::Enum
    /*0x0B*/  u8			direction;
    /*0x0C*/  u16			flags : 12;				// Use ImageFlags::Enum
              u16           iscript_key : 4;        // For the extender!
    /*0x0E*/  s8			horizontal_offset;
    /*0x0F*/  s8			vertical_offset;
    /*0x10*/  u16			iscript_header_offset;
    /*0x12*/  u16			iscript_offset;
    /*0x14*/  u16			return_address;
    /*0x16*/  u8			animation;				// Use IscriptAnimation::Enum
    /*0x17*/  u8			wait;					// iscript sleep time
    /*0x18*/  u16			frameset;
    /*0x1A*/  u16			frame_index;
    /*0x1C*/  v2u16		map_pos;
    /*0x20*/  v2u16		screen_pos;
    /*0x24*/  Box16			grp_size;				// Bounds for GRP frame, only different from normal when part of graphic is out of bounds.
    /*0x2C*/  GrpHead*		grp_file;
    /*0x30*/  union {
        struct {
    /*0x30*/  void*         draw_func_param;
        } standard;
        struct {
    /*0x30*/  u8            frame;
    /*0x31*/  u8            step0;
    /*0x32*/  u8            step1;
    /*0x33*/  u8            step2;
        } draw_fxn;
    };
    /*0x34*/  void*			render_fxn;
    /*0x38*/  void*			update_fxn;
    /*0x3C*/  Sprite*		parent_sprite;
};

C_ASSERT(sizeof(Image) == 64); /*0x40*/
//static_assert(sizeof(Image) == 64, "The count of the Image structure is invalid");

struct IscriptProgram; // Fuck you visual studio
namespace hooks {
    void initialize_image_data(Image* image, Sprite* parent, u16 image_id, s8 horizontal_offset, s8 vertical_offset);	// 004D5A50
    void set_image_direction(Image* image, u8 direction);																// 004D5F80
    void update_image_direction(Image* image, u8 direction);															// 004D5EA0
    void play_image_iscript(Image* image, u8 animation);																// 004D8470
    void show_cursor_marker(u16 pos_x, u16 pos_y);																		// 00488660
    void play_warp_in_overlay(Image* image, u32 unused);																// 004D8500
//  void image_update_warp_flash(Image* image);																			// 004D8590
    void iscript_opcode_cases(Image* image, IscriptProgram* program_state, u32 noop, u32* distance_moved);				// 004D74C0

    void inject_image_hooks();
}

#pragma pack()