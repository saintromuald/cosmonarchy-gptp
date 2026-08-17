#pragma once

#include "types.h"
#include "common.h"
#include "BW/enumerations.h"
#include "safeunion.h"

#include <type_traits>
#include <algorithm>

#pragma pack(1)

struct Unit;
struct Button;
struct StatusScreenState;
struct GrpHead;
struct PortraitData;
struct Options;

struct DialogEvent {
    DialogUserType  user;                   // 0x0
    u16             param;                  // 0x4
    u16             field_6;                // 0x6 
    u16             virtual_key;            // 0x8
    u16             virtual_key_modifier;   // 0xA
    DialogEventType number_order;           // 0xC
    v2u16	current;                        // 0xE
    u16 field_12;                           // 0x12
};

struct UserUnit {
    Unit* unit;
    u16 unit_id;
};

struct Bitmap {
    u16 width;
    u16 height;
    u8* buffer;
};

struct BinDlg;

class DialogBounds {
public:
    DialogBounds();

    static DialogBounds from_ltrb(s16 left, s16 top, s16 right, s16 bottom);
    static DialogBounds from_xywh(s16 x, s16 y, u16 w, u16 h);

    s16 left() const { return m_left; }
    s16 top() const { return m_top; }
    s16 right() const { return m_right; }
    s16 bottom() const { return m_bottom; }

    s16 x() const { return m_left; }
    s16 y() const { return m_top; }
    s16 w() const { return std::max(0, m_right - m_left + 1); };
    s16 h() const { return std::max(0, m_bottom - m_top + 1); };

    void set_ltrb(s16 left, s16 top, s16 right, s16 bottom);
    void set_xywh(s16 x, s16 y, u16 width, u16 height);

    bool contains(const v2u16 point) const;
    bool contains(u16 x, u16 y) const;
    bool contains_mouse() const;

    bool is_empty() const;
    bool is_valid() const;
    v2u16 center() const;
    v2u16 top_left() const;
    v2u16 top_right() const;
    v2u16 bottom_left() const;
    v2u16 bottom_right() const;

    DialogBounds intersect(const DialogBounds& other) const;
    DialogBounds unite(const DialogBounds& other) const;
    DialogBounds inflate(s16 dx, s16 dy) const;
    DialogBounds offset(s16 dx, s16 dy) const;

    operator Box16() const;
    operator Box32() const;

private:
    s16 m_left;
    s16 m_top;
    s16 m_right;
    s16 m_bottom;
};

struct BinDlg;
using BinDlgProc = u32(__fastcall*)(BinDlg*, DialogEvent*);
using TimerCallback = void(__fastcall*)(BinDlg*, u16 index);

using DialogUpdateFunc = void(__fastcall*)(BinDlg*, DialogEvent*, int, void* draw_ctx);

// BinDlg is defined in https://code.google.com/p/vgce/source/browse/trunk/docs/Blizzard/Starcraft/BIN%20FORMAT.txt as Dialog Structure
// That structure is variable depending on the content

// -------------------------------------------------------------------------------------
// Original BinDlg type
// -------------------------------------------------------------------------------------

// To be removed once all old code is refactored and all variants of the 0x36 and 0x46 enums are mapped out
// Much of this is wrong, bw::Dialog (below) should be seen as the current source of truth
struct BinDlg {
    /* 0x00 */ BinDlg* next;

    union {
        /* 0x04 */ Box16 bounds;        // Some dialogs (buttons, statfluf) use x/y/w/h
        /* 0x04 */ Box16 dialog_bounds; // Safer alternative for fully-hooked functions that we can guarantee are always l/t/r/b
    };

    /* 0x0c */ Bitmap bitmap;

    union {
        /* 0x14 */ const char* text;
        /* 0x14 */ char         (*textbox_buffer)[256]; // Only for DialogControlType::TextInput
    };

    /* 0x18 */ DialogFlags::Enum  flags;        // use BinDlgFlags::Enum
    /* 0x1C */ u32                unk_1c;       // Upper two bytes appear to be delay show value (probably for fly-in animations)
    /* 0x20 */ u16                index;
    /* 0x22 */ DialogControlType  control_type; // use DialogControlType::Enum
    /* 0x24 */ u16                graphic;

    union {
        /* 0x26 */ void* user;
        /* 0x26 */ Unit* unit_user;
        /* 0x26 */ BinDlg* dialog_user;
        /* 0x26 */ Button* button_user;
        /* 0x26 */ StatusScreenState* stat_button_user; // from Neiv's list
        /* 0x26 */ u32                  non_user_value;
        /* 0x26 */ GrpHead* grp_head_user;
        /* 0x26 */ PortraitData* portrait_data;
        /* 0x26 */ UserUnit* user_unit_with_id;
        /* 0x26 */ Options* options;
    };

    union {
        /* 0x2A */ void* fxn_interact;
        /* 0x2A */ BinDlgProc             interaction_function;
    };

    /* 0x2E */ void* fxn_update;
    /* 0x32 */ BinDlg* parent;

    union {
        struct {
            /* 0x36 */ Bitmap bitmap;
            /* 0x3A */ // FILLER
        } DialogBoxDC_0x36;

        struct {
            /* 0x36 */ s16 width;
            /* 0x38 */ s16 height;
        } dimensions_0x36;                                      // Buttons & statfluf regions just have dimensions here

        struct {
            /* 0x36 */ Box16 response_area;
            /* 0x3E */ BinDlg* unk_3e;
        } initialVersion0x36;

        struct {
            /* 0x36 */ s16  response_area_width;         // guess
            /* 0x38 */ s16  response_area_height;        // guess
            /* 0x3A */ u32* response_area_bit_array;     // set with a memory allocation function, bit array is a guess
            /* 0x3E */ u32* unk3e;                       // unverified filler
        } onInitDialog0x36;

        struct {
            /* 0x36 */ void* unk_func_36;                 // should be a function
            /* 0x3A */ u16   scrollMinValue;              // unconfirmed
            /* 0x3C */ u16   scrollMaxValue;              // unconfirmed
            /* 0x3E */ u16   scrollCurrentValue;          // unconfirmed
            /* 0x40 */ u16   unk40;                       // unverified filler
        } scrollDialog0x36; // probably for scrollbar involving dialogs?

        struct {
            /* 0x36 */ BinDlg* unkDlg36;
            /* 0x3A */ u32     unk3a;                       // unverified filler
            /* 0x3E */ u32     unk3e;                       // unverified filler
        } dialogOn0x36; // for when 0x36 is clearly BinDlg* but the rest is unknown

        struct {
            /* 0x36 */ b32 is_selected;    // based on the value only being compared to 0
            /* 0x3A */ u32 unk3a;          // unverified filler
            /* 0x3E */ u32 unk3e;          // unverified filler
        } checkBox0x36;// not completely sure

        struct {
            /* 0x36 */ b32 is_selected;
            /* 0x3A */ u32 unk3a;
            /* 0x3E */ u32 unk3e;
        } radioButton0x36; // not completely sure

        struct {
            /* 0x36 */ u8          unkFlags36;  // "flag" because of possible unkFlags36 |= 1
            /* 0x37 */ unk_t       unk37[3];    // unverified filler
            /* 0x3A */ u8** unk3aArrayCharPtr;  // seen in a different context so may be wrong and belong to another category
            /* 0x3E */ u8          pszTextLen;  // should be the length of psz_text
            /* 0x3F */ unk_t       unk3f[3];    // unverified filler
        } TextBox0x36; // seems to apply to some text related dialogs

        struct {
            /* 0x36 */ u32     time_cost;
            /* 0x3A */ u32* font_handle;
            /* 0x3E */ BinDlg* player_name;
        } statRes;

        struct {
            /* 0x36 */ BinDlg* scrollbar;
            /* 0x3A */ char** strings_array;   // Array of char* to heap-allocated strings
            /* 0x3E */ u8* flags_array;     // Array of u8 flags. 0x16 = copy input string to fresh malloc.
        } ComboBox;

        struct {
            /* 0x36 */ void* value_update_function;       // void __stdcall (BinDlg* dialog)
            /* 0x3A */ u16         current;
            /* 0x3C */ u16         min;
            /* 0x3E */ u16         max;
            /* 0x40 */ u16         cached_pixel_offset;
        } Slider;

        struct {
            /* 0x36 */ Box16       bounds;
            /* 0x3E */ char        unk_3e[2];
            /* 0x40 */ char        center_offset[2];
        } LightUpButton0x36;
    };

    union {
        /* 0x42 */ void* child_smk;
        /* 0x42 */ BinDlg* child_dialog;
        /* 0x42 */ u32* combobox_output_array_u32;
        /* 0x42 */ u8* combobox_output_array_u8;
    };

    union {
        struct {
            /* 0x46 */ v2u16       text_pos;
            /* 0x4A */ u16         response_area_width;
            /* 0x4C */ u16         response_area_height;
            /* 0x4E */ unk_t       unk_4e[8];
        } initial_version_0x46;

        struct {
            /* 0x46 */ v2u16       text_pos;                    // unverified filler
            /* 0x4A */ void* unk_function;
            /* 0x4E */ unk_t       unk_4e[8];                   // unverified filler
        } onInitDialog0x46;

        struct {
            /* 0x46 */ u8          item_count;
            /* 0x47 */ u8          unk47;                       // unverified filler
            /* 0x48 */ u8          unk48;
            /* 0x49 */ u8          unk49;
            /* 0x4A */ u8          unk4a;
            /* 0x4B */ u8          unk4b;
            /* 0x4C */ b8          unk4c;
            /* 0x4D */ u8          unk4d;
            /* 0x4E */ u8          unk4e;
            /* 0x4F */ unk_t       unk_4f[7];                   // unverified filler
        } alternateVersion0x46;

        struct {
            /* 0x46 */ u8          item_count;
            /* 0x47 */ u8          malloc_size;
            /* 0x48 */ s8          selection_index;
            /* 0x49 */ u8          element_height;
            /* 0x4A */ u8          _unk2;
            /* 0x4B */ u8          _unk3;
            /* 0x4C */ u8          flag;
            /* 0x4D */ u8          selection_index_2;
            /* 0x4E */ u8          listbox_selection_index;
            /* 0x4F */ u8          _unk5;
            /* 0x50 */ u16         combobox_saved_top_y;
            /* 0x52 */ void* unknown_function;
        } ComboBox0x46;

        struct {
            /* 0x46 */ Box16       bounds;
            /* 0x4E */ unk_t       unk_4e[8];                   // 35,0,0,0,0,0,0,0
        } LightUpButton0x46;
    };
};

C_ASSERT(sizeof(BinDlg) == 86); //0x56
//static_assert(sizeof(BinDlg) == 86, "The count of the BinDlg structure is invalid");

namespace bw {
struct Dialog;

typedef u32 __fastcall DialogInteraction(Dialog*, DialogEvent*);
typedef u32 __fastcall DialogDraw(Dialog*, DialogEvent*, u32 user, void* draw_context);

// -------------------------------------------------------------------------------------
// Data Union (0x36) members. This union is generally used for data used in interactions
// -------------------------------------------------------------------------------------
struct MenuData {
    Bitmap  backup_surface;  // Used as a temporary draw serface
    Dialog* focused_control; // For keyboard focus system, disabled by DialogFlags::UseAlternateStyle (on by default in glues)
};
static_assert(sizeof(MenuData) == 12);

struct SelectionButtonData { // Used for both OptionButton and Checkbox
    b32 selected;
    u8  padding[8];
};
static_assert(sizeof(SelectionButtonData) == 12);

typedef u32 __fastcall SliderCallback(Dialog*);
struct SliderData {
    SliderCallback* callback; // Called whenever value changes
    u16   current;
    u16   min;
    u16   max;
    u16   cached_pixel_offset;
};
static_assert(sizeof(SliderData) == 12);

struct ListOrComboData {
    Dialog* scrollbar;    // Child control for listbox when needed
    char** strings_array;
    u8* flags_array;
};
static_assert(sizeof(ListOrComboData) == 12);

struct LightUpButtonData {
    DialogBounds response_bounds;   // Local-space bounds. Adjusted to screenspace in interaction function -> init children
    s16 text_render_param;          // passed to 0x004e1020
    s16 center_offset_scaled;       // Possibly used for slide-in animation
};
static_assert(sizeof(LightUpButtonData) == 12);

struct TextInputData {
    u8 flags;
    u8 scroll_offset;
    u8 horizontal_margin;
    u8 vertical_offset;
    u16 visible_width;
    bool click_notified_slots; // Have we notified active-widget slots this click?
    u8 unknown_7;
    u8 cursor_position;
    u8 unknown_9[3];
};
static_assert(sizeof(TextInputData) == 12);

// -------------------------------------------------------------------------------------
// Child Union (0x42) members (Always provided as a pointer)
// -------------------------------------------------------------------------------------
struct LightUpButtonAnimation { // For SMK animated buttons
    LightUpButtonAnimation* next;
    u32 flags; // Bit4=SmkResource, Bit8=OnState, Bit16=NotFirst(runtime)
    u16 unkown_8;
    const char* filename;
    void* smk_handle;
    v2u16 render_pos; // Offset from center
    u16 smk_width;
    u16 smk_height;
    u8* frame_buffer;
};
static_assert(sizeof(LightUpButtonAnimation) == 0x1E);

// -------------------------------------------------------------------------------------
// Extra Union (0x46) members. This union is generally used for data used in drawing
// -------------------------------------------------------------------------------------
typedef u32 __fastcall ListboxRowDraw(Box16* row, s32 mode, s32 flags);
struct ListOrComboExtra {
    u8 item_count;              // Read this for current item count
    u8 malloc_size;             // Always 254
    s8 selection_index;         // Read this for current selection
    u8 element_height;
    u8 visible_row_count;       // rows that fit; from init: (h+1) / element_height
    u8 _unk4b;
    bool combo_open_upward;     // Calculated automatically, not available for manual control
    u8 selection_index_2;       // Temporary selection storage in click/release handler
    u8 clicked_index;           // Set by lbuttondown, consumed by lbuttonup
    u8 _unk5;
    u16 combobox_saved_top;     // Cached location for drawing combobox label when open
    void* (__fastcall* item_renderer)(Box16* row, int mode, int flags);
};
static_assert(sizeof(ListOrComboExtra) == 16);

struct LightUpButtonExtra {
    DialogBounds bounds; // Adjusted to screenspace in fixup / init function
    u8 style;            // (flags >> 20) & 0xFF, set by InitChildren
    char padding[7];
};
static_assert(sizeof(LightUpButtonExtra) == 16);

// ---------------------------------------------------------------------------------------
// Dialog Definition - Identical layout to BinDlg* and designed as a type-safe replacement
// ---------------------------------------------------------------------------------------
struct Dialog {
    Dialog* next;       // For the child dialog linked-list
    DialogBounds bounds;
    Bitmap bitmap;      // Used for top level dialogs (menu background) as well as images
    const char* text;
    DialogFlags::Enum flags;
    u16 delay_timer;    // Probably for hiding widgets until fly-in animation is complete
    u16 unknown_1e;
    u16 index;
    DialogControlType type;
    u16 graphic;        // Used to index graphics from grp files e.g. for cursor animation
    void* user;
    DialogInteraction* interaction_function;
    DialogDraw* draw_function;
    Dialog* parent;

    // Data Union 0x36
    safeunion::Union<
        safeunion::Entry<DialogControlType::DialogBoxDC, MenuData>,
        safeunion::Entry<DialogControlType::OptionButton, SelectionButtonData>,
        safeunion::Entry<DialogControlType::CheckBox, SelectionButtonData>,
        safeunion::Entry<DialogControlType::HorizontalScrollBar, SliderData>,
        safeunion::Entry<DialogControlType::VerticalScrollBar, SliderData>,
        safeunion::Entry<DialogControlType::TextInput, TextInputData>,
        safeunion::Entry<DialogControlType::ListBox, ListOrComboData>,
        safeunion::Entry<DialogControlType::ComboBox, ListOrComboData>,
        safeunion::Entry<DialogControlType::LightUpButton, LightUpButtonData>
    > union_data;

    // Child Union 0x42
    safeunion::Union<
        safeunion::Entry<DialogControlType::DialogBoxDC, Dialog*>,
        safeunion::Entry<DialogControlType::ListBox, u32*>,
        safeunion::Entry<DialogControlType::LightUpButton, LightUpButtonAnimation*>
    > union_child;

    // Extra union 0x46
    safeunion::Union<
        safeunion::Entry<DialogControlType::ListBox, ListOrComboExtra>,
        safeunion::Entry<DialogControlType::ComboBox, ListOrComboExtra>,
        safeunion::Entry<DialogControlType::LightUpButton, LightUpButtonExtra>
    > union_extra;

    template <typename T> auto data() { return union_data.cast<T>(type); }
    template <typename T> auto data() const { return union_data.cast<T>(type); }
    template <typename T> auto child() { return union_child.cast<T>(type); }
    template <typename T> auto child() const { return union_child.cast<T>(type); }
    template <typename T> auto extra() { return union_extra.cast<T>(type); }
    template <typename T> auto extra() const { return union_extra.cast<T>(type); }

    BinDlg* to_bindlg() { return reinterpret_cast<BinDlg*>(this); }
    const BinDlg* to_bindlg() const { return reinterpret_cast<const BinDlg*>(this); }
};

// Natvis / debug-info anchor.
//
// Forces MSVC to output debug symbols for these types
// Debug builds only; these have zero runtime effect
#ifdef _DEBUG
namespace _natvis_anchor {
inline void _pin_types() {
    MenuData               _1;
    SliderData             _2;
    TextInputData          _3;
    ListOrComboData        _4;
    ListOrComboExtra       _5;
    LightUpButtonData      _6;
    LightUpButtonAnimation _7;
    LightUpButtonExtra     _8;
    ListOrComboExtra       _9;
    LightUpButtonAnimation _10;

    // volatile reads keep the optimizer from discarding the locals
    // (and their layout info) as dead.
    (void)reinterpret_cast<volatile char&>(_1);
    (void)reinterpret_cast<volatile char&>(_2);
    (void)reinterpret_cast<volatile char&>(_3);
    (void)reinterpret_cast<volatile char&>(_4);
    (void)reinterpret_cast<volatile char&>(_5);
    (void)reinterpret_cast<volatile char&>(_6);
    (void)reinterpret_cast<volatile char&>(_7);
    (void)reinterpret_cast<volatile char&>(_8);
    (void)reinterpret_cast<volatile char&>(_9);
    (void)reinterpret_cast<volatile char&>(_10);
}
inline auto _keep_alive = &_pin_types;
} // namespace _natvis_anchor
#endif

}

#pragma pack()
