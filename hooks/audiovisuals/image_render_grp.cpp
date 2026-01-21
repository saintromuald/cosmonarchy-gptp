#include "image_render.h"

static u8* pcx_imageBuffer;
static u8* tex_imageBuffer;
static u8* image_draw_palette_index;
static int grp_width;
static int current_draw_left;
static int current_draw_right;
static int current_draw_bottom;
static u8 cloak_threshold;

namespace drawFunc {
    void __fastcall texture_to_remapping(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, u8* colorTable) {
        pcx_imageBuffer = colorTable;
        tex_imageBuffer = (u8*)warpTextureBuffer;
        Bitmap* surface = (Bitmap*)*g_current_surface;
        image_draw_palette_index = (u8*)pFrame->data_offset;
        u8* img_drw_pal_idx = &image_draw_palette_index[2 * grpRect->top];
        s32 rect_right = grpRect->right;
        u8* surf_buff = &surface->buffer[screenX + screenY * surface->width];
        grp_width = surface->width - rect_right;
        current_draw_left = grpRect->left;
        current_draw_right = rect_right;
        current_draw_bottom = grpRect->bottom;
        u32 i_0;

        while (--current_draw_bottom >= 0) {
            u8* grp_cur = &image_draw_palette_index[*(u16*)img_drw_pal_idx];
            img_drw_pal_idx += 2;
            s32 cur_drw_right = current_draw_right;
            s32 cur_drw_left = current_draw_left;

            while (cur_drw_left) {
                i_0 = *grp_cur++;

                if (i_0 & 0x80) {
                    if (!(cur_drw_left -= (i_0 & 0x7F)))
                        break;

                    if (cur_drw_left < 0) {
                        surf_buff -= cur_drw_left;
                        cur_drw_right += cur_drw_left;
                        break;
                    }
                } else if (i_0 & 0x40) {
                    ++grp_cur;

                    if (!(cur_drw_left -= (i_0 &= 0xBF)))
                        break;

                    if (cur_drw_left < 0) {
                        i_0 = -cur_drw_left;
                        --grp_cur;
                        goto decode_rle;
                    }
                } else {
                    grp_cur += i_0;

                    if (!(cur_drw_left -= i_0))
                        break;

                    if (cur_drw_left < 0) {
                        i_0 = -cur_drw_left;
                        grp_cur -= i_0;
                        goto decode_static;
                    }
                }
            }

            while (cur_drw_right > 0) {
                i_0 = *grp_cur++;

                if (i_0 & 0x80) {
                    if ((cur_drw_right -= (i_0 &= 0x7F)) < 0) {
                        i_0 += cur_drw_right;
                    }

                    surf_buff += i_0;
                    tex_imageBuffer += i_0;
                } else if (i_0 & 0x40) {
                    i_0 &= 0xBF;

                decode_rle:
                    if ((cur_drw_right -= i_0) < 0) {
                        i_0 += cur_drw_right;
                    }

                    ++grp_cur;
                    u8* tex_cur = tex_imageBuffer;

                    do {
                        u32 cur = *tex_cur++;
                        cur = *surf_buff | (cur << 8);
                        cur = pcx_imageBuffer[cur];
                        *surf_buff = cur;
                        ++surf_buff;
                    } while (--i_0);

                    tex_imageBuffer = tex_cur;
                } else {
                decode_static:
                    if ((cur_drw_right -= i_0) < 0) {
                        i_0 += cur_drw_right;
                    }

                    grp_cur += i_0;
                    u8* tex_cur = tex_imageBuffer;

                    do {
                        u32 cur = *tex_cur++;
                        cur = *surf_buff | (cur << 8);
                        cur = pcx_imageBuffer[cur];
                        *surf_buff = cur;
                        ++surf_buff;
                    } while (--i_0);

                    tex_imageBuffer = tex_cur;
                }
            }

            surf_buff += grp_width;
        }
    }

    void __fastcall threshold_remapping(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, u32 step, u8* colorTable) {
        pcx_imageBuffer = colorTable;
        cloak_threshold = step;
        Bitmap* surface = (Bitmap*)*g_current_surface;
        image_draw_palette_index = (u8*)pFrame->data_offset;
        u8* img_drw_pal_idx = &image_draw_palette_index[2 * grpRect->top];
        s32 rect_right = grpRect->right;
        u8* surf_buff = &surface->buffer[screenX + screenY * surface->width];
        grp_width = surface->width - rect_right;
        current_draw_left = grpRect->left;
        current_draw_right = rect_right;
        current_draw_bottom = grpRect->bottom;
        u32 i_0;

        while (--current_draw_bottom >= 0) {
            u8* grp_cur = &image_draw_palette_index[*(u16*)img_drw_pal_idx];
            img_drw_pal_idx += 2;
            s32 cur_drw_right = current_draw_right;
            s32 cur_drw_left = current_draw_left;

            while (cur_drw_left) {
                i_0 = *grp_cur++;

                if (i_0 & 0x80) {
                    if (!(cur_drw_left -= (i_0 & 0x7F)))
                        break;

                    if (cur_drw_left < 0) {
                        surf_buff -= cur_drw_left;
                        cur_drw_right += cur_drw_left;
                        break;
                    }
                } else if (i_0 & 0x40) {
                    ++grp_cur;

                    if (!(cur_drw_left -= (i_0 &= 0xBF)))
                        break;

                    if (cur_drw_left < 0) {
                        i_0 = -cur_drw_left;
                        --grp_cur;
                        goto decode_rle;
                    }
                } else {
                    grp_cur += i_0;

                    if (!(cur_drw_left -= i_0))
                        break;

                    if (cur_drw_left < 0) {
                        i_0 = -cur_drw_left;
                        grp_cur -= i_0;
                        goto decode_static;
                    }
                }
            }

            while (cur_drw_right > 0) {
                i_0 = *grp_cur++;

                if (i_0 & 0x80) {
                    if ((cur_drw_right -= (i_0 &= 0x7F)) < 0) {
                        i_0 += cur_drw_right;
                    }

                    surf_buff += i_0;
                } else if (i_0 & 0x40) {
                    i_0 &= 0xBF;

                decode_rle:
                    if ((cur_drw_right -= i_0) < 0) {
                        i_0 += cur_drw_right;
                    }

                    u8 ct = cloak_threshold;
                    u32 idx = (*grp_cur++) << 8;

                    do {
                        u32 cur = *surf_buff | idx;
                        cur = pcx_imageBuffer[cur];

                        if ((u8)threshold_map[cur] > ct) {
                            *surf_buff = cur;
                        }

                        ++surf_buff;
                    } while (--i_0);
                } else {
                decode_static:
                    if ((cur_drw_right -= i_0) < 0) {
                        i_0 += cur_drw_right;
                    }

                    u8 ct = cloak_threshold;

                    do {
                        u32 cur = pcx_imageBuffer[*surf_buff | (*grp_cur++ << 8)];

                        if ((u8)threshold_map[cur] > ct) {
                            *surf_buff = cur;
                        }

                        ++surf_buff;
                    } while (--i_0);
                }
            }

            surf_buff += grp_width;
        }
    }

    void __fastcall threshold_flash(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, u32 step, u8* colorTable) {
        pcx_imageBuffer = colorTable;
        cloak_threshold = step;
        Bitmap* surface = (Bitmap*)*g_current_surface;
        image_draw_palette_index = (u8*)pFrame->data_offset;
        u8* img_drw_pal_idx = &image_draw_palette_index[2 * grpRect->top];
        s32 rect_right = grpRect->right;
        u8* surf_buff = &surface->buffer[screenX + screenY * surface->width];
        grp_width = surface->width - rect_right;
        current_draw_left = grpRect->left;
        current_draw_right = rect_right;
        current_draw_bottom = grpRect->bottom;
        u32 i_0;

        while (--current_draw_bottom >= 0) {
            u8* grp_cur = &image_draw_palette_index[*(u16*)img_drw_pal_idx];
            img_drw_pal_idx += 2;
            s32 cur_drw_right = current_draw_right;
            s32 cur_drw_left = current_draw_left;

            while (cur_drw_left) {
                i_0 = *grp_cur++;

                if (i_0 & 0x80) {
                    if (!(cur_drw_left -= (i_0 & 0x7F)))
                        break;

                    if (cur_drw_left < 0) {
                        surf_buff -= cur_drw_left;
                        cur_drw_right += cur_drw_left;
                        break;
                    }
                } else if (i_0 & 0x40) {
                    ++grp_cur;

                    if (!(cur_drw_left -= (i_0 &= 0xBF)))
                        break;

                    if (cur_drw_left < 0) {
                        i_0 = -cur_drw_left;
                        --grp_cur;
                        goto decode_rle;
                    }
                } else {
                    grp_cur += i_0;

                    if (!(cur_drw_left -= i_0))
                        break;

                    if (cur_drw_left < 0) {
                        i_0 = -cur_drw_left;
                        grp_cur -= i_0;
                        goto decode_static;
                    }
                }
            }

            while (cur_drw_right > 0) {
                i_0 = *grp_cur++;

                if (i_0 & 0x80) {
                    if ((cur_drw_right -= (i_0 &= 0x7F)) < 0) {
                        i_0 += cur_drw_right;
                    }

                    surf_buff += i_0;
                } else if (i_0 & 0x40) {
                    i_0 &= 0xBF;

                decode_rle:
                    if ((cur_drw_right -= i_0) < 0) {
                        i_0 += cur_drw_right;
                    }

                    u8 ct = cloak_threshold;
                    u32 idx = *grp_cur++;

                    do {
                        u32 cur = pcx_imageBuffer[idx];

                        if ((u8)threshold_map[cur] > ct) {
                            *surf_buff = cur;
                        }

                        ++surf_buff;
                    } while (--i_0);
                } else {
                decode_static:
                    if ((cur_drw_right -= i_0) < 0) {
                        i_0 += cur_drw_right;
                    }

                    u8 ct = cloak_threshold;

                    do {
                        u32 cur = pcx_imageBuffer[*grp_cur++];

                        if ((u8)threshold_map[cur] > ct) {
                            *surf_buff = cur;
                        }

                        ++surf_buff;
                    } while (--i_0);
                }
            }

            surf_buff += grp_width;
        }
    }

    void __fastcall threshold_flash_mirror(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, u32 step, u8* colorTable) {
        pcx_imageBuffer = colorTable;
        cloak_threshold = step;
        Bitmap* surface = (Bitmap*)*g_current_surface;
        image_draw_palette_index = (u8*)pFrame->data_offset;
        u8* img_drw_pal_idx = &image_draw_palette_index[2 * grpRect->top];
        s32 rect_right = grpRect->right;
        u8* surf_buff = &surface->buffer[screenX + screenY * surface->width + rect_right];
        grp_width = surface->width + rect_right;
        current_draw_left = (s32)pFrame->w - rect_right - grpRect->left;
        current_draw_right = rect_right;
        current_draw_bottom = grpRect->bottom;
        u32 i_0;

        while (--current_draw_bottom >= 0) {
            u8* grp_cur = &image_draw_palette_index[*(u16*)img_drw_pal_idx];
            img_drw_pal_idx += 2;
            s32 cur_drw_right = current_draw_right;
            s32 cur_drw_left = current_draw_left;

            while (cur_drw_left) {
                i_0 = *grp_cur++;

                if (i_0 & 0x80) {
                    if (!(cur_drw_left -= (i_0 & 0x7F)))
                        break;

                    if (cur_drw_left < 0) {
                        surf_buff += cur_drw_left;
                        cur_drw_right += cur_drw_left;
                        break;
                    }
                } else if (i_0 & 0x40) {
                    ++grp_cur;

                    if (!(cur_drw_left -= (i_0 &= 0xBF)))
                        break;

                    if (cur_drw_left < 0) {
                        i_0 = -cur_drw_left;
                        --grp_cur;
                        goto decode_rle;
                    }
                } else {
                    grp_cur += i_0;

                    if (!(cur_drw_left -= i_0))
                        break;

                    if (cur_drw_left < 0) {
                        i_0 = -cur_drw_left;
                        grp_cur -= i_0;
                        goto decode_static;
                    }
                }
            }

            while (cur_drw_right > 0) {
                i_0 = *grp_cur++;

                if (i_0 & 0x80) {
                    if ((cur_drw_right -= (i_0 &= 0x7F)) < 0) {
                        i_0 += cur_drw_right;
                    }

                    surf_buff -= i_0;
                } else if (i_0 & 0x40) {
                    i_0 &= 0xBF;

                decode_rle:
                    if ((cur_drw_right -= i_0) < 0) {
                        i_0 += cur_drw_right;
                    }

                    u8 ct = cloak_threshold;
                    u32 idx = *grp_cur++;

                    do {
                        u32 cur = pcx_imageBuffer[idx];

                        if ((u8)threshold_map[cur] > ct) {
                            *surf_buff = cur;
                        }

                        --surf_buff;
                    } while (--i_0);
                } else {
                decode_static:
                    if ((cur_drw_right -= i_0) < 0) {
                        i_0 += cur_drw_right;
                    }

                    u8 ct = cloak_threshold;

                    do {
                        u32 cur = pcx_imageBuffer[*grp_cur++];

                        if ((u8)threshold_map[cur] > ct) {
                            *surf_buff = cur;
                        }

                        --surf_buff;
                    } while (--i_0);
                }
            }

            surf_buff += grp_width;
        }
    }

    void __fastcall threshold_texture(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, u32 step) {
        tex_imageBuffer = (u8*)warpTextureBuffer;
        cloak_threshold = step;
        Bitmap* surface = (Bitmap*)*g_current_surface;
        image_draw_palette_index = (u8*)pFrame->data_offset;
        u8* img_drw_pal_idx = &image_draw_palette_index[2 * grpRect->top];
        s32 rect_right = grpRect->right;
        u8* surf_buff = &surface->buffer[screenX + screenY * surface->width];
        grp_width = surface->width - rect_right;
        current_draw_left = grpRect->left;
        current_draw_right = rect_right;
        current_draw_bottom = grpRect->bottom;
        u32 i_0;

        while (--current_draw_bottom >= 0) {
            u8* grp_cur = &image_draw_palette_index[*(u16*)img_drw_pal_idx];
            img_drw_pal_idx += 2;
            s32 cur_drw_right = current_draw_right;
            s32 cur_drw_left = current_draw_left;

            while (cur_drw_left) {
                i_0 = *grp_cur++;

                if (i_0 & 0x80) {
                    if (!(cur_drw_left -= (i_0 & 0x7F)))
                        break;

                    if (cur_drw_left < 0) {
                        surf_buff -= cur_drw_left;
                        cur_drw_right += cur_drw_left;
                        break;
                    }
                } else if (i_0 & 0x40) {
                    ++grp_cur;

                    if (!(cur_drw_left -= (i_0 &= 0xBF)))
                        break;

                    if (cur_drw_left < 0) {
                        i_0 = -cur_drw_left;
                        --grp_cur;
                        goto decode_rle;
                    }
                } else {
                    grp_cur += i_0;

                    if (!(cur_drw_left -= i_0))
                        break;

                    if (cur_drw_left < 0) {
                        i_0 = -cur_drw_left;
                        grp_cur -= i_0;
                        goto decode_static;
                    }
                }
            }

            while (cur_drw_right > 0) {
                i_0 = *grp_cur++;

                if (i_0 & 0x80) {
                    if ((cur_drw_right -= (i_0 &= 0x7F)) < 0) {
                        i_0 += cur_drw_right;
                    }

                    surf_buff += i_0;
                    tex_imageBuffer += i_0;
                } else if (i_0 & 0x40) {
                    i_0 &= 0xBF;

                decode_rle:
                    if ((cur_drw_right -= i_0) < 0) {
                        i_0 += cur_drw_right;
                    }

                    grp_cur++;
                    u8 ct = cloak_threshold;
                    u8* tex_cur = tex_imageBuffer;

                    do {
                        u32 cur = *tex_cur++;

                        if ((u8)threshold_map[(u8)g_palette_remap[cur]] > ct) {
                            *surf_buff = cur;
                        }

                        ++surf_buff;
                    } while (--i_0);

                    tex_imageBuffer = tex_cur;
                } else {
                decode_static:
                    if ((cur_drw_right -= i_0) < 0) {
                        i_0 += cur_drw_right;
                    }

                    grp_cur += i_0;
                    u8 ct = cloak_threshold;
                    u8* tex_cur = tex_imageBuffer;

                    do {
                        u32 cur = *tex_cur++;

                        if ((u8)threshold_map[(u8)g_palette_remap[cur]] > ct) {
                            *surf_buff = cur;
                        }

                        ++surf_buff;
                    } while (--i_0);

                    tex_imageBuffer = tex_cur;
                }
            }

            surf_buff += grp_width;
        }
    }

    void __fastcall ShadowCloak(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
        pcx_imageBuffer = (u8*)&dark_pcx_image_map->index[18][colorData];
        Bitmap* surface = (Bitmap*)*g_current_surface;
        image_draw_palette_index = (u8*)pFrame->data_offset;
        u8* img_drw_pal_idx = &image_draw_palette_index[2 * grpRect->top];
        s32 rect_right = grpRect->right;
        u8* surf_buff = &surface->buffer[screenX + screenY * surface->width];
        grp_width = surface->width - rect_right;
        current_draw_left = grpRect->left;
        current_draw_right = rect_right;
        current_draw_bottom = grpRect->bottom;
        u32 i_0;

        while (--current_draw_bottom >= 0) {
            u8* grp_cur = &image_draw_palette_index[*(u16*)img_drw_pal_idx];
            img_drw_pal_idx += 2;
            s32 cur_drw_right = current_draw_right;
            s32 cur_drw_left = current_draw_left;

            while (cur_drw_left) {
                i_0 = *grp_cur++;

                if (i_0 & 0x80) {
                    if (!(cur_drw_left -= (i_0 & 0x7F)))
                        break;

                    if (cur_drw_left < 0) {
                        surf_buff -= cur_drw_left;
                        cur_drw_right += cur_drw_left;
                        break;
                    }
                } else if (i_0 & 0x40) {
                    ++grp_cur;

                    if (!(cur_drw_left -= (i_0 &= 0xBF)))
                        break;

                    if (cur_drw_left < 0) {
                        i_0 = -cur_drw_left;
                        --grp_cur;
                        goto decode_rle;
                    }
                } else {
                    grp_cur += i_0;

                    if (!(cur_drw_left -= i_0))
                        break;

                    if (cur_drw_left < 0) {
                        i_0 = -cur_drw_left;
                        grp_cur -= i_0;
                        goto decode_static;
                    }
                }
            }

            while (cur_drw_right > 0) {
                i_0 = *grp_cur++;

                if (i_0 & 0x80) {
                    if ((cur_drw_right -= (i_0 &= 0x7F)) < 0) {
                        i_0 += cur_drw_right;
                    }

                    surf_buff += i_0;
                    continue;
                } else if (i_0 & 0x40) {
                    i_0 &= 0xBF;

                decode_rle:
                    if ((cur_drw_right -= i_0) < 0) {
                        i_0 += cur_drw_right;
                    }

                    ++grp_cur;
                } else {
                decode_static:
                    if ((cur_drw_right -= i_0) < 0) {
                        i_0 += cur_drw_right;
                    }

                    grp_cur += i_0;
                }

                do {
                    u32 i_2 = *surf_buff;
                    *surf_buff = pcx_imageBuffer[i_2];
                    ++surf_buff;
                } while (--i_0);
            }

            surf_buff += grp_width;
        }
    }

    void __fastcall ShadowCloak_mirror(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) {
        pcx_imageBuffer = &dark_pcx_image_map->index[18][colorData];
        Bitmap* surface = (Bitmap*)*g_current_surface;
        image_draw_palette_index = (u8*)pFrame->data_offset;
        u8* img_drw_pal_idx = &image_draw_palette_index[2 * grpRect->top];
        s32 rect_right = grpRect->right;
        u8* surf_buff = &surface->buffer[screenX + screenY * surface->width + rect_right];
        grp_width = surface->width + rect_right;
        current_draw_left = (s32)pFrame->w - rect_right - grpRect->left;
        current_draw_right = rect_right;
        current_draw_bottom = grpRect->bottom;
        u32 i_0;

        while (--current_draw_bottom >= 0) {
            u8* grp_cur = &image_draw_palette_index[*(u16*)img_drw_pal_idx];
            img_drw_pal_idx += 2;
            s32 cur_drw_right = current_draw_right;
            s32 cur_drw_left = current_draw_left;

            while (cur_drw_left) {
                i_0 = *grp_cur++;

                if (i_0 & 0x80) {
                    if (!(cur_drw_left -= (i_0 & 0x7F)))
                        break;

                    if (cur_drw_left < 0) {
                        surf_buff += cur_drw_left;
                        cur_drw_right += cur_drw_left;
                        break;
                    }
                } else if (i_0 & 0x40) {
                    ++grp_cur;

                    if (!(cur_drw_left -= (i_0 &= 0xBF)))
                        break;

                    if (cur_drw_left < 0) {
                        i_0 = -cur_drw_left;
                        --grp_cur;
                        goto decode_rle;
                    }
                } else {
                    grp_cur += i_0;

                    if (!(cur_drw_left -= i_0))
                        break;

                    if (cur_drw_left < 0) {
                        i_0 = -cur_drw_left;
                        grp_cur -= i_0;
                        goto decode_static;
                    }
                }
            }

            while (cur_drw_right > 0) {
                i_0 = *grp_cur++;

                if (i_0 & 0x80) {
                    if ((cur_drw_right -= (i_0 &= 0x7F)) < 0) {
                        i_0 += cur_drw_right;
                    }

                    surf_buff -= i_0;
                    continue;
                } else if (i_0 & 0x40) {
                    i_0 &= 0xBF;

                decode_rle:
                    if ((cur_drw_right -= i_0) < 0) {
                        i_0 += cur_drw_right;
                    }

                    ++grp_cur;
                } else {
                decode_static:
                    if ((cur_drw_right -= i_0) < 0) {
                        i_0 += cur_drw_right;
                    }

                    grp_cur += i_0;
                }

                do {
                    u32 i_2 = *(--surf_buff);
                    *surf_buff = pcx_imageBuffer[i_2];
                } while (--i_0);
            }

            surf_buff += grp_width;
        }
    }
}