#include "image_render.h"
#include <math.h>
#include "BW/extender/iscript_extender.h"

u8 threshold_map[256];

UpdateFunc sgUpdateFuncs[] = {
	{ PaletteType::Normal,				updateFunc_None },
	{ PaletteType::NoCloak,				updateFunc_None },
	{ PaletteType::HiddenCloaking,		drawFunc::Cloaking_update },
	{ PaletteType::HiddenCloaked,		updateFunc_None },
	{ PaletteType::HiddenDecloaking,	drawFunc::Decloaking_update },
	{ PaletteType::VisibleCloaking,		drawFunc::Cloaking_update },
	{ PaletteType::VisibleCloaked,		updateFunc_None },
	{ PaletteType::VisibleDecloaking,	drawFunc::Decloaking_update },
	{ PaletteType::EMP,					updateFunc_None },
	{ PaletteType::UseRemapping,		updateFunc_None },
	{ PaletteType::Shadow,				updateFunc_None },
	{ PaletteType::HPBar,				updateFunc_None },
	{ PaletteType::WarpTexture,			updateFunc_None },
	{ PaletteType::RLE_OUTLINE,			updateFunc_None },
	{ PaletteType::RLE_PLAYER_SIDE,		updateFunc_None },
	{ PaletteType::ImageBounds,			updateFunc_None },
	{ PaletteType::Hallucination,		updateFunc_None },
	{ PaletteType::WarpFlash,			drawFunc::warp_flash_update },
	{ PaletteType::WarpAnchor,			updateFunc_None },
	{ PaletteType::ShadowCloak,			drawFunc::ShadowCloak_update },
	{ PaletteType::HiddenCloaking_WA,	drawFunc::Cloaking_update },
	{ PaletteType::HiddenCloaked_WA,	updateFunc_None },
	{ PaletteType::HiddenDecloaking_WA,	drawFunc::Decloaking_update },
	{ PaletteType::VisibleCloaking_WA,	drawFunc::Cloaking_update },
	{ PaletteType::VisibleCloaked_WA,	updateFunc_None },
	{ PaletteType::VisibleDecloaking_WA,drawFunc::Decloaking_update },
	{ PaletteType::HiddenCloaking_WT,	drawFunc::Cloaking_update },
	{ PaletteType::HiddenCloaked_WT,	updateFunc_None },
	{ PaletteType::HiddenDecloaking_WT,	drawFunc::Decloaking_update },
	{ PaletteType::VisibleCloaking_WT,	drawFunc::Cloaking_update },
	{ PaletteType::VisibleCloaked_WT,	updateFunc_None },
	{ PaletteType::VisibleDecloaking_WT,drawFunc::Decloaking_update },
	{ PaletteType::HiddenCloaking_WF,	drawFunc::WarpFlash_Cloaking_update },
	{ PaletteType::HiddenCloaked_WF,	drawFunc::warp_flash_update },
	{ PaletteType::HiddenDecloaking_WF,	drawFunc::WarpFlash_Decloaking_update },
	{ PaletteType::VisibleCloaking_WF,	drawFunc::WarpFlash_Cloaking_update },
	{ PaletteType::VisibleCloaked_WF,	drawFunc::warp_flash_update },
	{ PaletteType::VisibleDecloaking_WF,drawFunc::WarpFlash_Decloaking_update }
};

RenderFunc sgRenderFuncs[] = {
	{ PaletteType::Normal,				renderFunc_Normal,						renderFunc_Normal2 },
	{ PaletteType::NoCloak,				renderFunc_Normal,						renderFunc_Normal2 },
	{ PaletteType::HiddenCloaking,		drawFunc::HiddenCloakAnim,				drawFunc::HiddenCloakAnim_mirror },
	{ PaletteType::HiddenCloaked,		renderFunc_HiddenCloak,					renderFunc_HiddenCloak2 },
	{ PaletteType::HiddenDecloaking,	drawFunc::HiddenCloakAnim,				drawFunc::HiddenCloakAnim_mirror },
	{ PaletteType::VisibleCloaking,		drawFunc::VisibleCloakAnim,				drawFunc::VisibleCloakAnim_mirror },
	{ PaletteType::VisibleCloaked,		renderFunc_VisibleCloak,				renderFunc_VisibleCloak2 },
	{ PaletteType::VisibleDecloaking,	drawFunc::VisibleCloakAnim,				drawFunc::VisibleCloakAnim_mirror },
	{ PaletteType::EMP,					renderFunc_EMP,							renderFunc_EMP2 },
	{ PaletteType::UseRemapping,		renderFunc_UseRemapping,				renderFunc_UseRemapping2 },
	{ PaletteType::Shadow,				renderFunc_Shadow,						renderFunc_Shadow2 },
	{ PaletteType::HPBar,				renderFunc_HPBar,						renderFunc_HPBar },
	{ PaletteType::WarpTexture,			renderFunc_WarpTexture,					renderFunc_WarpTexture2 },
	{ PaletteType::RLE_OUTLINE,			renderFunc_SelCircleRemapping,			renderFunc_SelCircleRemapping },
	{ PaletteType::RLE_PLAYER_SIDE,		renderFunc_PreservePColor,				renderFunc_PreservePColor2 },
	{ PaletteType::ImageBounds,			renderFunc_ImageBounds,					renderFunc_ImageBounds },
	{ PaletteType::Hallucination,		renderFunc_Hallucination,				renderFunc_Hallucination2 },
	{ PaletteType::WarpFlash,			drawFunc::WarpFlash_Normal,				drawFunc::WarpFlash_Normal_mirror },
	{ PaletteType::WarpAnchor,			drawFunc::WarpAnchor_Normal,			drawFunc::WarpAnchor_Normal },
	{ PaletteType::ShadowCloak,			drawFunc::ShadowCloak,					drawFunc::ShadowCloak_mirror },
	{ PaletteType::HiddenCloaking_WA,	drawFunc::WarpAnchor_HiddenCloakAnim,	drawFunc::WarpAnchor_HiddenCloakAnim },
	{ PaletteType::HiddenCloaked_WA,	drawFunc::WarpAnchor_HiddenCloak,		drawFunc::WarpAnchor_HiddenCloak },
	{ PaletteType::HiddenDecloaking_WA,	drawFunc::WarpAnchor_HiddenCloakAnim,	drawFunc::WarpAnchor_HiddenCloakAnim },
	{ PaletteType::VisibleCloaking_WA,	drawFunc::WarpAnchor_VisibleCloakAnim,	drawFunc::WarpAnchor_VisibleCloakAnim },
	{ PaletteType::VisibleCloaked_WA,	drawFunc::WarpAnchor_VisibleCloak,		drawFunc::WarpAnchor_VisibleCloak },
	{ PaletteType::VisibleDecloaking_WA,drawFunc::WarpAnchor_VisibleCloakAnim,	drawFunc::WarpAnchor_VisibleCloakAnim },
	{ PaletteType::HiddenCloaking_WT,	drawFunc::WarpTexture_HiddenCloakAnim,	drawFunc::WarpTexture_HiddenCloakAnim },
	{ PaletteType::HiddenCloaked_WT,	renderFunc_HiddenCloak,					renderFunc_HiddenCloak },
	{ PaletteType::HiddenDecloaking_WT,	drawFunc::WarpTexture_HiddenCloakAnim,	drawFunc::WarpTexture_HiddenCloakAnim },
	{ PaletteType::VisibleCloaking_WT,	drawFunc::WarpTexture_VisibleCloakAnim,	drawFunc::WarpTexture_VisibleCloakAnim },
	{ PaletteType::VisibleCloaked_WT,	drawFunc::WarpTexture_VisibleCloak,		drawFunc::WarpTexture_VisibleCloak },
	{ PaletteType::VisibleDecloaking_WT,drawFunc::WarpTexture_VisibleCloakAnim,	drawFunc::WarpTexture_VisibleCloakAnim },
	{ PaletteType::HiddenCloaking_WF,	drawFunc::WarpFlash_HiddenCloakAnim,	drawFunc::WarpFlash_HiddenCloakAnim },
	{ PaletteType::HiddenCloaked_WF,	drawFunc::WarpFlash_HiddenCloak,		drawFunc::WarpFlash_HiddenCloak },
	{ PaletteType::HiddenDecloaking_WF,	drawFunc::WarpFlash_HiddenCloakAnim,	drawFunc::WarpFlash_HiddenCloakAnim },
	{ PaletteType::VisibleCloaking_WF,	drawFunc::WarpFlash_Normal,				drawFunc::WarpFlash_Normal_mirror },
	{ PaletteType::VisibleCloaked_WF,	drawFunc::WarpFlash_Normal,				drawFunc::WarpFlash_Normal_mirror },
	{ PaletteType::VisibleDecloaking_WF,drawFunc::WarpFlash_Normal,				drawFunc::WarpFlash_Normal_mirror },
};

namespace drawFunc {
	void __fastcall HiddenCloakAnim(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		helper::drawFunc(draw_hiddenCloak, screenX, screenY, pFrame, grpRect, 0);
		helper::drawFunc(draw_cloakThreshold, screenX, screenY, pFrame, grpRect, (u8)(colorData >> 8));
	}

	void __fastcall HiddenCloakAnim_mirror(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		helper::drawFunc(draw_hiddenCloak2, screenX, screenY, pFrame, grpRect, 0);
		helper::drawFunc(draw_cloakThreshold2, screenX, screenY, pFrame, grpRect, (u8)(colorData >> 8));
	}

	void __fastcall VisibleCloakAnim(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		helper::drawFunc(draw_useRemapping, screenX, screenY, pFrame, grpRect, (int)g_color_shift[5].data);
		helper::drawFunc(draw_cloakThreshold, screenX, screenY, pFrame, grpRect, (u8)(colorData >> 8));
	}

	void __fastcall VisibleCloakAnim_mirror(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		helper::drawFunc(draw_useRemapping2, screenX, screenY, pFrame, grpRect, (int)g_color_shift[5].data);
		helper::drawFunc(draw_cloakThreshold2, screenX, screenY, pFrame, grpRect, (u8)(colorData >> 8));
	}

	void __fastcall WarpFlash_Normal(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		helper::drawFunc(draw_warpFlash, screenX, screenY, pFrame, grpRect, (int)g_color_shift[1].data + ((colorData >> 8) & 0xFF00));
	}

	void __fastcall WarpFlash_Normal_mirror(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		helper::drawFunc(draw_warpFlash2, screenX, screenY, pFrame, grpRect, (int)g_color_shift[1].data + ((colorData >> 8) & 0xFF00));
	}

	void __fastcall WarpAnchor_Normal(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		u32 frame = (u8)colorData;
		helper::drawFunc(draw_useRemapping, screenX, screenY, &g_image_grp_graphics[ImageId::WarpAnchorUnderlay]->frames[frame], grpRect, (int)g_color_shift[1].data);
		helper::drawFunc(draw_useRemapping, screenX, screenY, &g_image_grp_graphics[ImageId::WarpAnchor]->frames[frame], grpRect, (int)g_color_shift[5].data);
	}

	void __fastcall WarpAnchor_HiddenCloak(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		u32 frame = (u8)colorData;
		helper::drawFunc(draw_hiddenCloak, screenX, screenY, &g_image_grp_graphics[ImageId::WarpAnchor]->frames[frame], grpRect, 0);
	}

	void __fastcall WarpAnchor_HiddenCloakAnim(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		u32 frame = (u8)colorData;
		GrpFrame* warp_anchor_frame = &g_image_grp_graphics[ImageId::WarpAnchor]->frames[frame];
		helper::drawFunc(draw_hiddenCloak, screenX, screenY, warp_anchor_frame, grpRect, 0);
		u32 cloakL = (u8)(colorData >> 8);
		threshold_remapping(screenX, screenY, &g_image_grp_graphics[ImageId::WarpAnchorUnderlay]->frames[frame], grpRect, cloakL << 1, (u8*)g_color_shift[1].data);
		threshold_remapping(screenX, screenY, warp_anchor_frame, grpRect, cloakL, (u8*)g_color_shift[5].data);
	}

	void __fastcall WarpAnchor_VisibleCloak(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		u32 frame = (u8)colorData;
		helper::drawFunc(draw_useRemapping, screenX, screenY, &g_image_grp_graphics[ImageId::WarpAnchor]->frames[frame], grpRect, (int)g_color_shift[5].data);
	}

	void __fastcall WarpAnchor_VisibleCloakAnim(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		u32 frame = (u8)colorData;
		threshold_remapping(screenX, screenY, &g_image_grp_graphics[ImageId::WarpAnchorUnderlay]->frames[frame], grpRect, (u8)(colorData >> 8), (u8*)g_color_shift[1].data);
		helper::drawFunc(draw_useRemapping, screenX, screenY, &g_image_grp_graphics[ImageId::WarpAnchor]->frames[frame], grpRect, (int)g_color_shift[5].data);
	}/*

	void __fastcall WarpTexture_HiddenCloak(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		helper::drawFunc(draw_hiddenCloak, screenX, screenY, pFrame, grpRect, 0);
	}*/

	void __fastcall WarpTexture_HiddenCloakAnim(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		helper::drawFunc_WarpTexture_colorData(grpRect, (u8)colorData);
		helper::drawFunc(draw_hiddenCloak, screenX, screenY, pFrame, grpRect, 0);
		threshold_texture(screenX, screenY, pFrame, grpRect, (u8)(colorData >> 8));
	}

	void __fastcall WarpTexture_VisibleCloak(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		helper::drawFunc_WarpTexture_colorData(grpRect, (u8)colorData);
		texture_to_remapping(screenX, screenY, pFrame, grpRect, (u8*)g_color_shift[5].data);
	}

	void __fastcall WarpTexture_VisibleCloakAnim(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		helper::drawFunc_WarpTexture_colorData(grpRect, (u8)colorData);
		texture_to_remapping(screenX, screenY, pFrame, grpRect, (u8*)g_color_shift[5].data);
		threshold_texture(screenX, screenY, pFrame, grpRect, (u8)(colorData >> 8));
	}

	void __fastcall WarpFlash_VisibleCloak(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		u32 frame = (u8)colorData;
		helper::drawFunc(draw_useRemapping, screenX, screenY, pFrame, grpRect, (int)g_color_shift[1].data + ((colorData >> 8) & 0xFF00));
	}

	void __fastcall WarpFlash_HiddenCloak(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		return;
	}

	void __fastcall WarpFlash_HiddenCloakAnim(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		threshold_remapping(screenX, screenY, pFrame, grpRect, (u8)(colorData >> 8), (u8*)((int)g_color_shift[1].data + ((colorData >> 8) & 0xFF00)));
	}

	void __fastcall Decloaking_update(Image* image) {
		if (!(image->draw_fxn.step0 -= image->draw_fxn.step0 != 0)) {
			u8 type = image->palette_type;
			if (type < PaletteType::HiddenCloaking_WT) {
				if (type < PaletteType::WarpAnchor) {
					type = PaletteType::Normal;
				} else {
					type = PaletteType::WarpAnchor;
				}
			} else {
				type = PaletteType::WarpTexture;
			}
			image->palette_type = type;
			image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[type].fxn_mirror : sgRenderFuncs[type].fxn;
			image->update_fxn = sgUpdateFuncs[type].fxn;
			(*g_active_iscript_unit)->status &= ~(UnitStatus::RequiresDetection | UnitStatus::Cloaked);
		}
		image->flags |= ImageFlags::Redraw;
	}

	void __fastcall Cloaking_update(Image* image) {
		if (++image->draw_fxn.step0 >= Step0) {
			u8 type = ++image->palette_type;
			image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[type].fxn_mirror : sgRenderFuncs[type].fxn;
			image->update_fxn = sgUpdateFuncs[type].fxn;
			(*g_active_iscript_unit)->status |= (UnitStatus::RequiresDetection | UnitStatus::Cloaked);
		}
		image->flags |= ImageFlags::Redraw;
	}

	void __fastcall warp_flash_update(Image* image) { // 004D8590
		bool redraw = false;
		if (!(image->draw_fxn.step2 -= image->draw_fxn.step2 != 0)) {
			image->draw_fxn.step2 = 2;
			redraw = true;
			if (++image->draw_fxn.step1 >= 63) {
				if (image->animation != IscriptAnimation::Death) {
					image->set_iscript(IscriptAnimation::Death);
					helper::iscript_opcode_cases(image, &image->iscript_header_offset, 0, 0);
				}
				(*g_active_iscript_entity)->order_signal |= 1;
			}
		}
		image->flags |= (u8)redraw;
	}

	void __fastcall WarpFlash_Decloaking_update(Image* image) {
		bool redraw = false;
		if (!(image->draw_fxn.step0 -= image->draw_fxn.step0 != 0)) {
			redraw = true;
			u8 type = image->palette_type;
			image->palette_type = PaletteType::WarpFlash;
			image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[type].fxn_mirror : sgRenderFuncs[type].fxn;
			image->update_fxn = sgUpdateFuncs[type].fxn;
			(*g_active_iscript_entity)->status &= ~(UnitStatus::RequiresDetection | UnitStatus::Cloaked);
		}
		if (!(image->draw_fxn.step2 -= image->draw_fxn.step2 != 0)) {
			image->draw_fxn.step2 = 2;
			redraw = true;
			if (++image->draw_fxn.step1 >= 63) {
				if (image->animation != IscriptAnimation::Death) {
					image->set_iscript(IscriptAnimation::Death);
					helper::iscript_opcode_cases(image, &image->iscript_header_offset, 0, 0);
				}
				(*g_active_iscript_entity)->order_signal |= 1;
			}
		}
		image->flags |= (u8)redraw;
	}

	void __fastcall ShadowCloak_update(Image* image) {
		Image* main_image = image->parent_sprite->main_graphic;
		u8 step = main_image->draw_fxn.step0 >> 1;
		if (step > 13) {
			step = 13;
		}
		image->draw_fxn.step0 = step;
		u8 type = main_image->palette_type;
		if (type == PaletteType::HiddenCloaked || type == PaletteType::VisibleCloaked) {
			image->flags |= ImageFlags::Invisible;
		} else if (!(image->parent_sprite->flags & SpriteFlags::Burrowed)) {
			if (type < PaletteType::HiddenCloaking || type > PaletteType::VisibleDecloaking) {
				image->palette_type = PaletteType::Shadow;
				image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[PaletteType::Shadow].fxn_mirror : sgRenderFuncs[PaletteType::Shadow].fxn;
				image->update_fxn = sgUpdateFuncs[PaletteType::Shadow].fxn;
			}
			image->flags &= ~ImageFlags::Invisible;
		}
		image->flags |= ImageFlags::Redraw;
	}

	void __fastcall WarpFlash_Cloaking_update(Image* image) {
		bool redraw = false;
		if (++image->draw_fxn.step0 >= Step0) {
			redraw = true;
			u8 type = ++image->palette_type;
			image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[type].fxn_mirror : sgRenderFuncs[type].fxn;
			image->update_fxn = sgUpdateFuncs[type].fxn;
			(*g_active_iscript_unit)->status |= (UnitStatus::RequiresDetection | UnitStatus::Cloaked);
		}
		if (!(image->draw_fxn.step2 -= image->draw_fxn.step2 != 0)) {
			image->draw_fxn.step2 = 2;
			redraw = true;
			if (++image->draw_fxn.step1 >= 63) {
				if (image->animation != IscriptAnimation::Death) {
					image->set_iscript(IscriptAnimation::Death);
					helper::iscript_opcode_cases(image, &image->iscript_header_offset, 0, 0);
				}
				(*g_active_iscript_entity)->order_signal |= 1;
			}
		}
		image->flags |= (u8)redraw;
	}
}

namespace hooks {
	u8 get_type_offset(u8 type) {
		switch (type) {
			case PaletteType::WarpTexture: {
				return 12;
			}
			case PaletteType::WarpFlash: {
				return 17;
			}
			case PaletteType::WarpAnchor:
			case PaletteType::HiddenCloaking_WA:
			case PaletteType::HiddenCloaked_WA:
			case PaletteType::HiddenDecloaking_WA:
			case PaletteType::VisibleCloaking_WA:
			case PaletteType::VisibleCloaked_WA:
			case PaletteType::VisibleDecloaking_WA: {
				return 18;
			}
			case PaletteType::HiddenCloaking_WT:
			case PaletteType::HiddenCloaked_WT:
			case PaletteType::HiddenDecloaking_WT:
			case PaletteType::VisibleCloaking_WT:
			case PaletteType::VisibleCloaked_WT:
			case PaletteType::VisibleDecloaking_WT: {
				return 24;
			}
			case PaletteType::HiddenCloaking_WF:
			case PaletteType::HiddenCloaked_WF:
			case PaletteType::HiddenDecloaking_WF:
			case PaletteType::VisibleCloaking_WF:
			case PaletteType::VisibleCloaked_WF:
			case PaletteType::VisibleDecloaking_WF: {
				return 30;
			}
			default: {
				return 0;
			}
		}
	}

	u8 get_new_type(u8 type_offset) {
		if (type_offset == PaletteType::WarpTexture) {
			return 24;
		}
		if (type_offset == PaletteType::WarpFlash) {
			return 30;
		}
		return type_offset;
	}

	u8 get_new_type(u8 type, u8 type_offset) {
		if (!type) {
			if (type_offset == PaletteType::WarpTexture) {
				return 24;
			}
			if (type_offset == PaletteType::WarpFlash) {
				return 30;
			}
			return type_offset;
		} else {
			switch (type_offset) {
				case PaletteType::HiddenCloaking_WA:
				case PaletteType::HiddenCloaked_WA:
				case PaletteType::HiddenDecloaking_WA:
				case PaletteType::VisibleCloaking_WA:
				case PaletteType::VisibleCloaked_WA:
				case PaletteType::VisibleDecloaking_WA: {
					return 18;
				}
				case PaletteType::HiddenCloaking_WT:
				case PaletteType::HiddenCloaked_WT:
				case PaletteType::HiddenDecloaking_WT:
				case PaletteType::VisibleCloaking_WT:
				case PaletteType::VisibleCloaked_WT:
				case PaletteType::VisibleDecloaking_WT: {
					return 24;
				}
				case PaletteType::HiddenCloaking_WF:
				case PaletteType::HiddenCloaked_WF:
				case PaletteType::HiddenDecloaking_WF:
				case PaletteType::VisibleCloaking_WF:
				case PaletteType::VisibleCloaked_WF:
				case PaletteType::VisibleDecloaking_WF: {
					return 30;
				}
				default: {
					return 0;
				}
			}
		}
	}

	void update_sprite_flag_on_palette(Sprite* sprite) {
		for (Image* image = sprite->images.head; image; image = image->link.next) {
			u8 type = image->palette_type;
			u8 type_offset = get_type_offset(type);
			type -= type_offset;

			if (type >= PaletteType::HiddenCloaking && type <= PaletteType::VisibleDecloaking) {
				image->palette_type = type_offset;
				image->flags |= ImageFlags::Redraw;
				image->draw_fxn.step0 = 0;
				image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[type_offset].fxn_mirror : sgRenderFuncs[type_offset].fxn;
				image->update_fxn = sgUpdateFuncs[type_offset].fxn;
			} else {
				u16 flags = image->flags;

				if (type == PaletteType::Shadow || (type == PaletteType::ShadowCloak && (flags |= ImageFlags::Invisible))) {
					flags |= ImageFlags::Redraw;
					image->palette_type = PaletteType::Shadow;
					image->flags = flags;
					image->draw_fxn.step0 = 0;
					image->render_fxn = flags & ImageFlags::Mirrored ? sgRenderFuncs[PaletteType::Shadow].fxn_mirror : sgRenderFuncs[PaletteType::Shadow].fxn;
					image->update_fxn = sgUpdateFuncs[PaletteType::Shadow].fxn;
				}
			}
		}
	}

	void make_sprite_visible(Sprite* sprite) {
		for (Image* image = sprite->images.head; image; image = image->link.next) {
			u8 type = image->palette_type;
			u8 type_offset = get_type_offset(type);
			type -= type_offset;

			if (type >= PaletteType::HiddenCloaking && type <= PaletteType::HiddenDecloaking) {
				type += type_offset + 3;
				image->palette_type = type;
				image->flags |= ImageFlags::Redraw;
				image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[type].fxn_mirror : sgRenderFuncs[type].fxn;
				image->update_fxn = sgUpdateFuncs[type].fxn;
			} else if (!type) {
				type = get_new_type(type_offset) + PaletteType::VisibleCloaked;
				image->palette_type = type;
				image->flags |= ImageFlags::Redraw;
				image->draw_fxn.step0 = 33;
				image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[type].fxn_mirror : sgRenderFuncs[type].fxn;
				image->update_fxn = sgUpdateFuncs[type].fxn;
			} else if (type == PaletteType::Shadow) {
				image->palette_type = PaletteType::ShadowCloak;
				image->flags |= ImageFlags::Redraw;
				image->draw_fxn.step0 = 13;
				image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[PaletteType::ShadowCloak].fxn_mirror : sgRenderFuncs[PaletteType::ShadowCloak].fxn;
				image->update_fxn = sgUpdateFuncs[PaletteType::ShadowCloak].fxn;
			}
		}
	}

	//Parameter "cloakH" is now deprecated here.
	void make_cloaked(u8 cloakH, Sprite* sprite, u8 cloakedStatus, u8 cloakL) {
		helper::refresh_all_visible_sprites(sprite);

		if (cloakedStatus == 1) {
			for (Image* image = sprite->images.head; image; image = image->link.next) {
				u8 type = image->palette_type;
				u8 type_offset = get_type_offset(type);
				type -= type_offset;

				if (!type) {
					type = get_new_type(type_offset) + PaletteType::HiddenCloaking;
					image->palette_type = type;
					image->flags |= ImageFlags::Redraw;
					image->draw_fxn.step0 = cloakL;
					image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[type].fxn_mirror : sgRenderFuncs[type].fxn;
					image->update_fxn = sgUpdateFuncs[type].fxn;
				} else if (type == PaletteType::Shadow) {
					image->palette_type = PaletteType::ShadowCloak;
					image->flags |= ImageFlags::Redraw;
					image->draw_fxn.step0 = cloakL;
					image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[PaletteType::ShadowCloak].fxn_mirror : sgRenderFuncs[PaletteType::ShadowCloak].fxn;
					image->update_fxn = sgUpdateFuncs[PaletteType::ShadowCloak].fxn;
				}
			}
		} else if (cloakedStatus == 2) {
			for (Image* image = sprite->images.head; image; image = image->link.next) {
				u8 type = image->palette_type;
				u8 type_offset = get_type_offset(type);
				type -= type_offset;

				if (!type) {
					type = get_new_type(type_offset) + PaletteType::HiddenDecloaking;
					image->palette_type = type;
					image->flags |= ImageFlags::Redraw;
					image->draw_fxn.step0 = cloakL;
					image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[type].fxn_mirror : sgRenderFuncs[type].fxn;
					image->update_fxn = sgUpdateFuncs[type].fxn;
				} else if (type == PaletteType::Shadow) {
					image->palette_type = PaletteType::ShadowCloak;
					image->flags |= ImageFlags::Redraw;
					image->draw_fxn.step0 = cloakL;
					image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[PaletteType::ShadowCloak].fxn_mirror : sgRenderFuncs[PaletteType::ShadowCloak].fxn;
					image->update_fxn = sgUpdateFuncs[PaletteType::ShadowCloak].fxn;
				}
			}
		} else if (cloakedStatus == 3) {
			for (Image* image = sprite->images.head; image; image = image->link.next) {
				u8 type = image->palette_type;
				u8 type_offset = get_type_offset(type);
				type -= type_offset;

				if (!type || type == PaletteType::HiddenCloaking || type == PaletteType::HiddenDecloaking) {
					type = get_new_type(type, type_offset) + PaletteType::HiddenCloaked;
					image->palette_type = type;
					image->flags |= ImageFlags::Redraw;
					image->draw_fxn.step0 = cloakL;
					image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[type].fxn_mirror : sgRenderFuncs[type].fxn;
					image->update_fxn = sgUpdateFuncs[type].fxn;
				} else if (type == PaletteType::Shadow || type == PaletteType::ShadowCloak) {
					image->palette_type = PaletteType::Shadow;
					image->flags |= (ImageFlags::Redraw | ImageFlags::Invisible);
					image->draw_fxn.step0 = cloakL;
					image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[PaletteType::Shadow].fxn_mirror : sgRenderFuncs[PaletteType::Shadow].fxn;
					image->update_fxn = sgUpdateFuncs[PaletteType::Shadow].fxn;
				}
			}
		}
	}

	void sub_4980F0(Sprite* sprite) {
		helper::refresh_all_visible_sprites(sprite);

		for (Image* image = sprite->images.head; image; image = image->link.next) {
			u8 type = image->palette_type;
			u8 type_offset = get_type_offset(type);
			type -= type_offset;

			if (!type || type == PaletteType::HiddenDecloaking || type == PaletteType::VisibleDecloaking) {
				type = get_new_type(type, type_offset) + PaletteType::HiddenCloaking;
				image->palette_type = type;
				image->flags |= ImageFlags::Redraw;
				image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[type].fxn_mirror : sgRenderFuncs[type].fxn;
				image->update_fxn = sgUpdateFuncs[type].fxn;
			} else if (type == PaletteType::Shadow || type == PaletteType::ShadowCloak) {
				image->palette_type = PaletteType::ShadowCloak;
				image->flags |= ImageFlags::Redraw;
				image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[PaletteType::ShadowCloak].fxn_mirror : sgRenderFuncs[PaletteType::ShadowCloak].fxn;
				image->update_fxn = sgUpdateFuncs[PaletteType::ShadowCloak].fxn;
			}
		}
	}

	void showImagesHiddenByCloak(Sprite* sprite) {
		for (Image* image = sprite->images.head; image; image = image->link.next) {
			if ((!images_dat::draw_if_cloaked[image->id] || image->palette_type == PaletteType::Shadow) && image->flags & ImageFlags::Invisible) {
				image->flags &= ~ImageFlags::Invisible;
				image->flags |= ImageFlags::Redraw;
			}
		}
	}

	void make_sprite_invisible(Sprite* sprite) {
		if (sprite->flags & SpriteFlags::Burrowed) {
			make_cloaked(0, sprite, 3, 0);
			return;
		}

		for (Image* image = sprite->images.head; image; image = image->link.next) {
			u8 type = image->palette_type;
			u8 type_offset = get_type_offset(type);
			type -= type_offset;

			if (type >= PaletteType::VisibleCloaking && type <= PaletteType::VisibleDecloaking) {
				type += type_offset - 3;
				image->palette_type = type;
				image->flags |= ImageFlags::Redraw;
				image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[type].fxn_mirror : sgRenderFuncs[type].fxn;
				image->update_fxn = sgUpdateFuncs[type].fxn;
			}
		}
	}

	void UpdateSpriteImagesForCloak(Sprite* sprite) {
		for (Image* image = sprite->images.head; image; image = image->link.next) {
			u8 type = image->palette_type;
			u8 type_offset = get_type_offset(type);
			type -= type_offset;

			if (type == PaletteType::HiddenCloaking || type == PaletteType::HiddenCloaked
			|| type == PaletteType::VisibleCloaking || type == PaletteType::VisibleCloaked) {
				type = type < PaletteType::HiddenDecloaking ? PaletteType::HiddenDecloaking : PaletteType::VisibleDecloaking;
				type += type_offset;
				image->palette_type = type;
				image->flags |= ImageFlags::Redraw;
				image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[type].fxn_mirror : sgRenderFuncs[type].fxn;
				image->update_fxn = sgUpdateFuncs[type].fxn;
			} else if (type == PaletteType::Shadow || type == PaletteType::ShadowCloak) {
				image->palette_type = PaletteType::ShadowCloak;
				image->flags |= ImageFlags::Redraw;
				image->render_fxn = image->flags & ImageFlags::Mirrored ? sgRenderFuncs[PaletteType::ShadowCloak].fxn_mirror : sgRenderFuncs[PaletteType::ShadowCloak].fxn;
				image->update_fxn = sgUpdateFuncs[PaletteType::ShadowCloak].fxn;
			}

			//inline of showImagesHiddenByCloak @ 0x00498170 (extracted from loop)
			if (!images_dat::draw_if_cloaked[image->id] && image->flags & ImageFlags::Invisible) {
				image->flags &= ~ImageFlags::Invisible;
				image->flags |= ImageFlags::Redraw;
			}
		}
	}

	void sub_498B50(Sprite* sprite, bool flipped) {
		for (Image* image = sprite->images.head; image; image = image->link.next) {
			u16 image_flags = image->flags;
			u8 flip_state = (image_flags & ImageFlags::Mirrored) >> 1;

			if (flip_state != (u8)flipped) {
				image->flags = (image_flags & ~ImageFlags::Mirrored) | (flipped << 1) | ImageFlags::Redraw;
				u8 type = image->palette_type;
				image->render_fxn = flipped ? sgRenderFuncs[type].fxn_mirror : sgRenderFuncs[type].fxn;

				if (type == PaletteType::WarpFlash) {
					image->draw_fxn.step1 = 48;
					image->draw_fxn.step2 = 2;
				}

				if (image_flags & ImageFlags::UsesSpecialOffset) {
					//inline of update_image_position_offset @ 0x004D5A00
					Image* main_image = image->parent_sprite->main_graphic;
					GrpHead* overlay_grp = (GrpHead*)images_dat::lo_special[main_image->id];
					GrpFrame* frame = &overlay_grp->frames[main_image->frame_index];
					s8 h_offset = frame->x;

					if (main_image->flags & ImageFlags::Mirrored) {
						h_offset = -h_offset;
					}

					s8 v_offset = frame->y;

					if (image->horizontal_offset != h_offset || image->vertical_offset != v_offset) {
						image->horizontal_offset = h_offset;
						image->vertical_offset = v_offset;
					}
				}
			}
		}
	}

	Sprite* /*__stdcall*/ duplicate_sprite(Sprite* sprite) {
		Sprite* new_sprite = helper::create_sprite(sprite->id, sprite->position.x, sprite->position.y, sprite->player);
		if (!new_sprite) {
			return nullptr;
		}

		Image* image = new_sprite->images.head;
		if (image) {
			Image* next_image;
			do {
				next_image = image->link.next;
				image->free();
				image = next_image;
			} while (next_image);
		}

		new_sprite->main_graphic = nullptr;
		for (image = sprite->images.tail; image; image = image->link.prev) {
			u8 type = image->palette_type;
			if (type == PaletteType::HPBar || type == PaletteType::RLE_OUTLINE) {
				continue;
			}

			Image* new_image = helper::create_image(new_sprite, image->id, image->horizontal_offset, image->vertical_offset, image->direction);
			if (!new_image) {
				new_sprite->free();
				return nullptr;
			}

			new_image->frameset = image->frameset;
			new_image->frame_index = image->frame_index;
			new_image->draw_fxn = image->draw_fxn;
			u16 new_flags = new_image->flags;
			u8 flip_state = image->flags & ImageFlags::Mirrored;
			if ((new_flags & ImageFlags::Mirrored) != flip_state) {//I don't know why this entire block exists, everything within it is already handled by create_image.
				type = new_image->palette_type;
				new_flags = (new_flags & ~ImageFlags::Mirrored) | flip_state | ImageFlags::Redraw;
				new_image->flags = new_flags;
				new_image->render_fxn = flip_state ? sgRenderFuncs[type].fxn_mirror : sgRenderFuncs[type].fxn;

				if (new_flags & ImageFlags::UsesSpecialOffset) {
					//inline of update_image_position_offset @ 0x004D5A00
					Image* main_image = image->parent_sprite->main_graphic;
					GrpHead* overlay_grp = (GrpHead*)images_dat::lo_special[main_image->id];
					GrpFrame* frame = &overlay_grp->frames[main_image->frame_index];
					
					s8 h_offset = frame->x;
					if (main_image->flags & ImageFlags::Mirrored) {
						h_offset = -h_offset;
					}
					s8 v_offset = frame->y;
					if (image->horizontal_offset != h_offset || image->vertical_offset != v_offset) {
						image->horizontal_offset = h_offset;
						image->vertical_offset = v_offset;
					}
				}
			}
			helper::update_image_draw_data(new_image);
			if (image == sprite->main_graphic) {
				new_sprite->main_graphic = new_image;
			}
		}
		return new_sprite;
	}

	void sub_4BCD70(u8* paletteEntry) {
		s32 i_0 = 255;
		paletteEntry += 1022;

		do {
			float blue = *paletteEntry;
			float green = *(paletteEntry - 1);
			float red = *(paletteEntry - 2);
			paletteEntry -= 4;
			blue = blue * blue * 0.068F;
			green = green * green * 0.691F;
			red = red * red * 0.241F;
			u8 RGB = (u32)(sqrtf(red + green + blue) * 1.0352942F) >> 3;
			threshold_map[i_0] = RGB;
			UnkRGBMashup[i_0] = RGB >> 2;
		} while (--i_0 >= 0);
	}
}

//Placing this here instead of bwhelper because it's very specific to image_render.
namespace helper {
	void drawFunc(int function, s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
		__asm {
			PUSHAD
			MOV		EAX, colorData
			PUSH	EAX
			MOV		EAX, grpRect
			PUSH	EAX
			MOV		EAX, pFrame
			PUSH	EAX
			MOV		ECX, screenX
			MOV		EDX, screenY
			MOV		EAX, function
			CALL	EAX
			POPAD
		}
	}

	void drawFunc_WarpTexture_colorData(Box32* grpRect, u32 frame) {
		const u32 func_address = 0x004D53F0;
		__asm {
			PUSHAD
			PUSH	warpTextureBuffer
			MOV		EAX, frame
			PUSH	EAX
			MOV		EAX, grpRect
			CALL	func_address
			POPAD
		}
	}
}