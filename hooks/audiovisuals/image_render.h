#pragma once
#include "BW/structures/Image.h"
#include <graphics/Bitmap.h>
#include "BW/bwdata.h"

#define Step0 33	//originally 8
//#define Step1 0		//originally 3
#define updateFunc_None					(update_fxn*)0x004D55F0
//#define updateFunc_Decloaking			(update_fxn*)0x004D5D10
//#define updateFunc_Cloaking			(update_fxn*)0x004D5D70
//#define updateFunc_WarpFlash			(update_fxn*)0x004D8590
#define renderFunc_Normal				(render_fxn*)0x0040ABBE
#define renderFunc_Normal2				(render_fxn*)0x0040BF60
#define renderFunc_HiddenCloak			(render_fxn*)0x0040B155
#define renderFunc_HiddenCloak2			(render_fxn*)0x0040B9A9
//#define renderFunc_HiddenCloakAnim		(render_fxn*)0x004D55B0
//#define renderFunc_HiddenCloakAnim2		(render_fxn*)0x004D5570
#define renderFunc_VisibleCloak			(render_fxn*)0x004D54D0
#define renderFunc_VisibleCloak2		(render_fxn*)0x004D54B0
//#define renderFunc_VisibleCloakAnim		(render_fxn*)0x004D5530
//#define renderFunc_VisibleCloakAnim2	(render_fxn*)0x004D54F0
#define renderFunc_EMP					(render_fxn*)0x0040B2D3
#define renderFunc_EMP2					(render_fxn*)0x0040BB2E
#define renderFunc_UseRemapping			(render_fxn*)0x0040B596
#define renderFunc_UseRemapping2		(render_fxn*)0x0040BCA3
#define renderFunc_Shadow				(render_fxn*)0x0040B6F0
#define renderFunc_Shadow2				(render_fxn*)0x0040C0AE
#define renderFunc_HPBar				(render_fxn*)0x0047A820
#define renderFunc_WarpTexture			(render_fxn*)0x004D5CD0
#define renderFunc_WarpTexture2			(render_fxn*)0x004D5C90
#define renderFunc_SelCircleRemapping	(render_fxn*)0x004D5390
#define renderFunc_PreservePColor		(render_fxn*)0x004D61A0
#define renderFunc_PreservePColor2		(render_fxn*)0x004D6120
#define renderFunc_ImageBounds			(render_fxn*)0x004D5280
#define renderFunc_Hallucination		(render_fxn*)0x004D5210
#define renderFunc_Hallucination2		(render_fxn*)0x004D51A0
//#define renderFunc_WarpFlash			(render_fxn*)0x004D5480
//#define renderFunc_WarpFlash2			(render_fxn*)0x004D5450
#define draw_warpTexture				0x0040AD04
#define draw_warpTexture2				0x0040AE63
#define draw_cloakThreshold				0x0040AFD5
#define draw_hiddenCloak				0x0040B155
#define	draw_warpFlash					0x0040B441
#define draw_useRemapping				0x0040B596
#define draw_cloakThreshold2			0x0040B824
#define draw_hiddenCloak2				0x0040B9A9
#define draw_useRemapping2				0x0040BCA3
#define	draw_warpFlash2					0x0040BE04
#define warpTextureBuffer				0x005254C0

typedef void __fastcall update_fxn(Image* image);

struct UpdateFunc {
	u32 id;
	update_fxn* fxn;
};

typedef void __fastcall render_fxn(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);

struct RenderFunc {
	u32 id;
	render_fxn* fxn;
	render_fxn* fxn_mirror;
};

extern u8 threshold_map[256];
extern UpdateFunc sgUpdateFuncs[];
extern RenderFunc sgRenderFuncs[];

namespace drawFunc {
	void __fastcall HiddenCloakAnim(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);				//0x004D55B0
	void __fastcall HiddenCloakAnim_mirror(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);		//0x004D5570
	void __fastcall VisibleCloakAnim(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);			//0x004D5530
	void __fastcall VisibleCloakAnim_mirror(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);		//0x004D54F0
	void __fastcall WarpFlash_Normal(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);			//0x004D5480
	void __fastcall WarpFlash_Normal_mirror(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);		//0x004D5450
	void __fastcall WarpAnchor_Normal(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
	void __fastcall WarpAnchor_HiddenCloak(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
	void __fastcall WarpAnchor_HiddenCloakAnim(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
	void __fastcall WarpAnchor_VisibleCloak(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
	void __fastcall WarpAnchor_VisibleCloakAnim(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
	//void __fastcall WarpTexture_HiddenCloak(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
	void __fastcall WarpTexture_HiddenCloakAnim(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
	void __fastcall WarpTexture_VisibleCloak(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
	void __fastcall WarpTexture_VisibleCloakAnim(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
	void __fastcall WarpFlash_VisibleCloak(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
	void __fastcall WarpFlash_HiddenCloak(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
	void __fastcall WarpFlash_HiddenCloakAnim(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
	void __fastcall Decloaking_update(Image* image);																		//0x004D5D10
	void __fastcall Cloaking_update(Image* image);																			//0x004D5D70
	void __fastcall WarpFlash_update(Image* image);																			//0x004D8590
	void __fastcall WarpFlash_Decloaking_update(Image* image);
	void __fastcall WarpFlash_Cloaking_update(Image* image);
	void __fastcall ShadowCloak_update(Image* image);

	void __fastcall texture_to_remapping(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, u8* colorTable);
	void __fastcall threshold_remapping(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, u32 step, u8* colorTable);
	void __fastcall threshold_texture(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, u32 step);
	void __fastcall ShadowCloak(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
	void __fastcall ShadowCloak_mirror(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
}

namespace hooks {
	u8 get_type_offset(u8 type);
	void updateSpriteFlagOnPalette(Sprite* sprite);								//0x00497E80
	void make_sprite_visible(Sprite* sprite);									//0x00497ED0
	void make_cloaked(u8 cloakH, Sprite* sprite, u8 cloakedStatus, u8 cloakL);	//0x00497FD0
	void sub_4980F0(Sprite* sprite);											//0x004980F0
	void showImagesHiddenByCloak(Sprite* sprite);								//0x00498170
	void make_sprite_invisible(Sprite* sprite);									//0x00498A10
	void UpdateSpriteImagesForCloak(Sprite* sprite);							//0x00498A90
	void sub_498B50(Sprite* sprite, bool flipped);								//0x00498B50
	Sprite* __stdcall duplicate_sprite(Sprite* sprite);							//0x00499210
	void sub_4BCD70(u8* paletteEntry);											//0x004BCD70

	void inject_image_render_hooks();
}

namespace helper {
	void drawFunc(int function, s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);
	void drawFunc_WarpTexture_colorData(Box32* grpRect, u32 frame);				//0x004D53F0
}