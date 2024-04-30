#include "minimap.h"
#include <BW\api.h>

#define DIV_ROUND_UP(n, d) n / d + (n % d != 0)

u16 box_width;
u16 box_height;

namespace hooks {
	void minimapVisionUpdate_256() {
		u32 pos_y = *g_map_height;

		if (pos_y) {
			u8* surf_buff = g_minimap_surface->buffer;
			u8* surf_buff_fog = g_minimap_surface_fog->buffer;
			ActiveTile* active_tile = *g_active_tile_array;

			do {
				u32 pos_x = *g_map_width;

				do {
					u32 cur_vision = (*g_player_visions & (*active_tile).vision_flags) == 0;
					u32 cur_filter = (*g_player_explored_visions & (*active_tile).vision_flags) == 0;

					if (*g_is_in_replay) {
						cur_filter = 0;

						if (*g_replay_reveal_all) {
							cur_vision = 1;
						} else {
							cur_filter = (u8)*g_replay_visions;
							u32 tile_vision = ~(*active_tile).vision_flags;

							if ((cur_filter << 8) & tile_vision) {
								if ((u8)tile_vision & cur_filter) {
									cur_vision = 1;
									cur_filter = 0;
								} else {
									cur_vision = 0;
									cur_filter = 1;
								}
							} else {
								cur_vision = 0;
								cur_filter = 0;
							}
						}
					}

					*surf_buff_fog++ = g_minimap59C2C0[256 * (cur_filter | 2 * cur_vision) + *surf_buff++];
					active_tile += 2;
				} while (--pos_x);

				active_tile += g_map_tile_size->width;
			} while (--pos_y);
		}
	}

	void minimapVisionUpdate_128() {
		u32 pos = (*g_map_height) * (*g_map_width);

		if (pos) {
			u8* surf_buff = g_minimap_surface->buffer;
			u8* surf_buff_fog = g_minimap_surface_fog->buffer;
			ActiveTile* active_tile = *g_active_tile_array;

			do {
				u32 cur_vision = (*g_player_visions & (*active_tile).vision_flags) == 0;
				u32 cur_filter = (*g_player_explored_visions & (*active_tile).vision_flags) == 0;

				if (*g_is_in_replay) {
					cur_filter = 0;

					if (*g_replay_reveal_all) {
						cur_vision = 1;
					} else {
						cur_filter = (u8)*g_replay_visions;
						u32 tile_vision = ~(*active_tile).vision_flags;

						if ((cur_filter << 8) & tile_vision) {
							if ((u8)tile_vision & cur_filter) {
								cur_vision = 1;
								cur_filter = 0;
							} else {
								cur_vision = 0;
								cur_filter = 1;
							}
						} else {
							cur_vision = 0;
							cur_filter = 0;
						}
					}
				}

				*surf_buff_fog++ = g_minimap59C2C0[256 * (cur_filter | 2 * cur_vision) + *surf_buff++];
				++active_tile;
			} while (--pos);
		}
	}

	void minimapVisionUpdate_64() {
		u32 pos_y = *g_map_height >> 1;

		if (pos_y) {
			u8* surf_buff = g_minimap_surface->buffer;
			u8* surf_buff_fog = g_minimap_surface_fog->buffer;
			u32 map_width = *g_map_width;
			u32 buff_step = (g_minimap_surface_fog->width << 1) - map_width;
			u32 map_width_2 = map_width >> 1;
			ActiveTile* active_tile = *g_active_tile_array;

			do {
				u32 pos_x = map_width_2;

				do {
					u32 cur_vision = (*g_player_visions & (*active_tile).vision_flags) == 0;
					u32 cur_filter = (*g_player_explored_visions & (*active_tile).vision_flags) == 0;

					if (*g_is_in_replay) {
						cur_filter = 0;

						if (*g_replay_reveal_all) {
							cur_vision = 1;
						} else {
							cur_filter = (u8)*g_replay_visions;
							u32 tile_vision = ~(*active_tile).vision_flags;

							if ((cur_filter << 8) & tile_vision) {
								if ((u8)tile_vision & cur_filter) {
									cur_vision = 1;
									cur_filter = 0;
								} else {
									cur_vision = 0;
									cur_filter = 1;
								}
							} else {
								cur_vision = 0;
								cur_filter = 0;
							}
						}
					}

					u32 cur = (cur_filter | 2 * cur_vision) << 8;
					*surf_buff_fog = g_minimap59C2C0[*surf_buff + cur];
					*(surf_buff_fog + 1) = g_minimap59C2C0[*(surf_buff + 1) + cur];
					*(surf_buff_fog + map_width) = g_minimap59C2C0[*(surf_buff + map_width) + cur];
					*(surf_buff_fog + map_width + 1) = g_minimap59C2C0[*(surf_buff + map_width + 1) + cur];
					surf_buff += 2;
					surf_buff_fog += 2;
					++active_tile;
				} while (--pos_x);

				surf_buff += buff_step;
				surf_buff_fog += buff_step;
			} while (--pos_y);
		}
	}

	void minimapVisionUpdate_staircase() {
		u32 pos_y = *g_map_height;

		if (pos_y) {
			u8* surf_buff = g_minimap_surface->buffer;
			u8* surf_buff_fog = g_minimap_surface_fog->buffer;
			ActiveTile* active_tile = *g_active_tile_array;
			u32 tile_y = 0;

			do {
				u32 tile_x = 0;
				u32 pos_x = *g_map_width;
				u16 _tile_y = (tile_y >> 5) * g_map_tile_size->width;

				do {
					ActiveTile cur_active_tile = active_tile[(tile_x >> 5) + _tile_y];
					u32 cur_vision = (*g_player_visions & cur_active_tile.vision_flags) == 0;
					u32 cur_filter = (*g_player_explored_visions & cur_active_tile.vision_flags) == 0;

					if (*g_is_in_replay) {
						cur_filter = 0;

						if (*g_replay_reveal_all) {
							cur_vision = 1;
						} else {
							cur_filter = (u8)*g_replay_visions;
							u32 tile_vision = ~cur_active_tile.vision_flags;

							if ((cur_filter << 8) & tile_vision) {
								if ((u8)tile_vision & cur_filter) {
									cur_vision = 1;
									cur_filter = 0;
								} else {
									cur_vision = 0;
									cur_filter = 1;
								}
							} else {
								cur_vision = 0;
								cur_filter = 0;
							}
						}
					}

					*surf_buff_fog++ = g_minimap59C2C0[256 * (cur_filter | 2 * cur_vision) + *surf_buff++];
					tile_x += *g_minimap_zoom_level;
				} while (--pos_x);

				tile_y += *g_minimap_zoom_level;
			} while (--pos_y);
		}
	}

	void minimapSurfaceUpdate_256() {
		u32 pos_y = *g_map_height;

		if (pos_y) {
			u16* map_tileset_handle = (u16*)*megatileArray;
			u8* surf_buff = g_minimap_surface->buffer;
			VX4* vx4_handle = (VX4*)(*vx4FileHandle);
			VR4* vr4_handle = (VR4*)(*vr4FileHandle);

			do {
				u32 pos_x = *g_map_width;

				do {
					u32 i_0 = *map_tileset_handle;
					map_tileset_handle += 2;
					i_0 = vx4_handle->tile[i_0][0].vr4_id;
					i_0 = vr4_handle->minitile[i_0][55];
					*surf_buff++ = g_minimap59CB60[i_0];
				} while (--pos_x);
			
				map_tileset_handle += g_map_tile_size->width;
			} while (--pos_y);
		}
	}

	void minimapSurfaceUpdate_128() {
		u32 buff_size = (*g_map_height) * (*g_map_width);

		if (buff_size) {
			u16* map_tileset_handle = (u16*)*megatileArray;
			u8* surf_buff = ((Bitmap*)g_minimap_surface)->buffer;
			VX4* vx4_handle = (VX4*)(*vx4FileHandle);
			VR4* vr4_handle = (VR4*)(*vr4FileHandle);

			do {
				u32 i_0 = *map_tileset_handle;
				++map_tileset_handle;
				i_0 = vx4_handle->tile[i_0][0].vr4_id;
				i_0 = vr4_handle->minitile[i_0][55];
				*surf_buff++ = g_minimap59CB60[i_0];
			} while (--buff_size);
		}
	}

	void minimapSurfaceUpdate_64() {
		u32 pos_y = *g_map_height >> 1;

		if (pos_y) {
			u16* map_tileset_handle = (u16*)*megatileArray;
			u8* surf_buff = g_minimap_surface->buffer;
			u32 map_width = *g_map_width;
			u32 buff_step = (g_minimap_surface->width << 1) - map_width;
			u32 map_width_2 = map_width >> 1;
			VX4* vx4_handle = (VX4*)(*vx4FileHandle);
			VR4* vr4_handle = (VR4*)(*vr4FileHandle);

			do {
				u32 pos_x = map_width_2;

				do {
					u32 i_0 = *map_tileset_handle;
					++map_tileset_handle;
					vx4* tile = vx4_handle->tile[i_0];
					i_0 = tile[0].vr4_id;
					i_0 = vr4_handle->minitile[i_0][55];
					*surf_buff = g_minimap59CB60[i_0];
					i_0 = tile[4].vr4_id;
					i_0 = vr4_handle->minitile[i_0][55];
					*(surf_buff + map_width) = g_minimap59CB60[i_0];
					i_0 = tile[1].vr4_id;
					i_0 = vr4_handle->minitile[i_0][55];
					*(surf_buff + 1) = g_minimap59CB60[i_0];
					i_0 = tile[5].vr4_id;
					i_0 = vr4_handle->minitile[i_0][55];
					*(surf_buff + map_width + 1) = g_minimap59CB60[i_0];
					surf_buff += 2;
				} while (--pos_x);

				surf_buff += buff_step;
			} while (--pos_y);
		}
	}

	void minimapSurfaceUpdate_staircase() {
		u32 pos_y = *g_map_height;

		if (pos_y) {
			u16* map_tileset_handle = (u16*)*megatileArray;
			u8* surf_buff = ((Bitmap*)g_minimap_surface)->buffer;
			VX4* vx4_handle = (VX4*)(*vx4FileHandle);
			VR4* vr4_handle = (VR4*)(*vr4FileHandle);
			u32 tile_y = 0;

			do {
				u32 tile_x = 0;
				u32 pos_x = *g_map_width;
				u16 _tile_y = (tile_y >> 5) * g_map_tile_size->width;
				u8 _pos_y = ((tile_y >> 3) & 3) * 4;

				do {
					u32 i_0 = map_tileset_handle[(tile_x >> 5) + _tile_y];
					i_0 = vx4_handle->tile[i_0][((tile_x >> 3) & 3) + _pos_y].vr4_id;
					i_0 = vr4_handle->minitile[i_0][55];
					*surf_buff++ = g_minimap59CB60[i_0];
					tile_x += *g_minimap_zoom_level;
				} while (--pos_x);

				tile_y += *g_minimap_zoom_level;
			} while (--pos_y);
		}
	}

	void __cdecl setMapSizeConstants() {//TODO: Doesn't handle map dimensions smaller than 64.
		u32 map_width = g_map_tile_size->width;
		u32 map_height = g_map_tile_size->height;
		u32 map_size = map_width <= map_height ? map_height : map_width;

		if (!(map_size & 0x3F)) {//Opting against a switch statement because it generates a large jump table and this code is executed only once on mission start, so I consider it wasteful.
			if (map_size == 64) {
				map_width <<= 1;
				map_height <<= 1;
				*g_minimap_surface_CB = minimapSurfaceUpdate_64;
				*g_minimap_vision_CB = minimapVisionUpdate_64;
			} else if (map_size == 128) {
				*g_minimap_surface_CB = minimapSurfaceUpdate_128;
				*g_minimap_vision_CB = minimapVisionUpdate_128;
			} else if (map_size == 256) {
				map_width >>= 1;
				map_height >>= 1;
				*g_minimap_surface_CB = minimapSurfaceUpdate_256;
				*g_minimap_vision_CB = minimapVisionUpdate_256;
			} else {//map size == 192
				map_width <<= 1;
				map_width = DIV_ROUND_UP(map_width, 3);
				map_height <<= 1;
				map_height = DIV_ROUND_UP(map_height, 3);
				*g_minimap_surface_CB = minimapSurfaceUpdate_staircase;
				*g_minimap_vision_CB = minimapVisionUpdate_staircase;
			}
		} else if (map_size == 96) {
			map_width <<= 2;
			map_width = DIV_ROUND_UP(map_width, 3);
			map_height <<= 2;
			map_height = DIV_ROUND_UP(map_height, 3);
			*g_minimap_surface_CB = minimapSurfaceUpdate_staircase;
			*g_minimap_vision_CB = minimapVisionUpdate_staircase;
		} else {
			float scale = 128.0F / (float)map_size;
			map_width = (u32)ceil(map_width * scale);
			map_height = (u32)ceil(map_height * scale);
			*g_minimap_surface_CB = minimapSurfaceUpdate_staircase;
			*g_minimap_vision_CB = minimapVisionUpdate_staircase;
		}

		u32 zoom_level = DIV_ROUND_UP(map_size, 4);
		*g_minimap_zoom_level = zoom_level;
		u32 width = DIV_ROUND_UP(640, zoom_level);
		box_width = width >= 2 ? width : 2;
		u32 height = DIV_ROUND_UP(416, zoom_level);
		box_height = height >= 1 ? height : 1;
		*g_map_width = map_width;
		*g_map_height = map_height;
		BinDlg* minimap_dlg = *g_minimap_dlg;
		s16 offset = (128 - map_width) >> 1;
		minimap_dlg->bounds.left += offset;
		minimap_dlg->bounds.right -= offset;
		offset = (128 - map_height) >> 1;
		minimap_dlg->bounds.top += offset;
		minimap_dlg->bounds.bottom -= offset;

		//inline of createMinimapSurface @ 0x004A3720
		Bitmap* surf_buff = g_minimap_surface;
		Bitmap* surf_buff_fog = g_minimap_surface_fog;
		surf_buff->width = map_width;
		surf_buff->height = map_height;
		surf_buff_fog->width = map_width;
		surf_buff_fog->height = map_height;
		surf_buff->buffer = (u8*)malloc(map_width * map_height);
		surf_buff_fog->buffer = (u8*)malloc(map_width * map_height);
		//end of createMinimapSurface

		RECT minimap_rect = *g_minimap_rect;
		minimap_rect.left = minimap_dlg->bounds.left;
		minimap_rect.top = minimap_dlg->bounds.top;
		minimap_rect.right = minimap_dlg->bounds.right;
		minimap_rect.bottom = minimap_dlg->bounds.bottom;
	}

	void minimap_ping(s32 posX, s32 posY, u8 color) {
		Ping* ping_array = (*minimap_ping_array).ping_array;

		while (ping_array->isActive) {
			if (!ping_array[1].isActive) {
				++ping_array;
				break;
			}

			if (!ping_array[2].isActive) {
				ping_array += 2;
				break;
			}

			if (!ping_array[3].isActive) {
				ping_array += 3;
				break;
			}

			if ((ping_array += 4) == (*minimap_ping_array).ping_array + 16)
				return;
		}

		ping_array->isActive = true;
		u32 zoom_level = *g_minimap_zoom_level;
		ping_array->x = (posX / zoom_level) - 64;
		ping_array->y = (posY / zoom_level) - 64;
		ping_array->timer = 0;
		ping_array->player = color;
	}

	void __fastcall MinimapImageUpdate(BinDlg* minimap, int dummy0, int dummy1, int dummy2) {
		Bitmap* surface = g_minimap_surface_fog;
		Bounds drawbounds = { 0, 0, surface->width - 1, surface->height - 1, surface->width, surface->height };
		helper::blit_surface(&drawbounds, surface, minimap->bounds.left, minimap->bounds.top);

		if (!*g_minimap_draw_neutrals) {
			u32 zoom_level = *g_minimap_zoom_level;
			u32 box_y = (u32)minimap->bounds.top + *g_move_to_y / zoom_level;
			u32 box_x = (u32)minimap->bounds.left + *g_move_to_x / zoom_level;
			
			//inline and simplification of DrawRectangle @ 0x004A37A0
			u32 surf_w = ((Bitmap*)(*g_current_surface))->width;
			u8* surf_buff = ((Bitmap*)(*g_current_surface))->buffer + surf_w * box_y + box_x;
			u32 height = box_height - 1;
			u32 bounds_bottom = (u32)minimap->bounds.bottom;

			if (box_y + height >= bounds_bottom) {
				height = bounds_bottom - box_y;
			}

			u8 remainder = box_width & 3;
			u32 _surf_w = surf_w * height;
			u32 width = box_width >> 2;
			u32 color = g_palette_index_reference[0];
			color |= (color << 24) | (color << 16) | (color << 8);
			s32 i_0 = 1;

			do {
				u8* dest = &surf_buff[i_0 * _surf_w];

				if (remainder) {
					u32 _is_odd = width * 4 + remainder;

					do {
						dest[--_is_odd] = (u8)color;
					} while (_is_odd & 3);
				}

				u32 rep = width;

				do {
					(u32&)dest[--rep * 4] = color;
				} while (rep);
			} while (--i_0 >= 0);

			//similar to DrawVerticalLine @ 0x0041D7D0, but draws both lines simultaneously
			width = box_width;
			i_0 = height - 1;

			do {
				u8* dest = &surf_buff[i_0 * surf_w];
				dest[0] = (u8)color;
				dest[width - 1] = (u8)color;
			} while (--i_0);
		}
	}

	void draw_unit_box(s32 posX, u8 color, s32 posY, u32 sizeX, u32 sizeY, bool isBuilding) {
		++*g_minimap_unit_box_count;
		s32 size_x;
		s32 size_y;
		u32 zoom_level = *g_minimap_zoom_level;

		if (isBuilding) {
			size_x = sizeX >> 1;
			size_y = sizeY >> 1;
			sizeX = DIV_ROUND_UP(sizeX, zoom_level);
			sizeY = DIV_ROUND_UP(sizeY, zoom_level);

			if (sizeX < 2) {
				sizeX = 2;
			}

			if (sizeY < 2) {
				sizeY = 2;
			}
		} else {
			size_x = 32;
			size_y = 32;
			sizeX = 2;
			sizeY = 2;
		}

		posX -= size_x;
		if (posX < 0) {
			posX = 0;
		}
		posY -= size_y;
		if (posY < 0) {
			posY = 0;
		}

		u32 pos_x = DIV_ROUND_UP(posX, zoom_level);
		u32 pos_y = DIV_ROUND_UP(posY, zoom_level);

		if (pos_x + sizeX >= *g_map_width) {
			pos_x = *g_map_width - sizeX;
		}

		if (pos_y + sizeY >= *g_map_height) {
			pos_y = *g_map_height - sizeY;
		}

		//inline and simplification of DrawBox @ 0x004E1D20
		u32 surf_w = ((Bitmap*)(*g_current_surface))->width;
		u8* surf_buff = ((Bitmap*)(*g_current_surface))->buffer + surf_w * pos_y + pos_x;
		u8 remainder = sizeX & 1;
		u32 width = sizeX >> 1;
		u16 _color = (color << 8) | color;
		s32 i_0 = sizeY - 1;

		do {
			u8* dest = &surf_buff[i_0 * surf_w];

			if (remainder) {
				dest[width * 2] = (u8)_color;
			}

			u32 rep = width;

			do {
				(u16&)dest[--rep * 2] = _color;
			} while (rep);
		} while (--i_0 >= 0);
	}

	void minimapGameMouseUpdate(BinDlg* minimap) {
		//Truncated.
		u16 pos_x = (*g_mouse).x - (box_width >> 1);
		u16 pos_y = (*g_mouse).y - (box_height >> 1);
		helper::get_minimap_cursor_pos(&pos_x, &pos_y);

		if ((pos_x & -8) != *g_move_to_x || (pos_y & -8) != *g_move_to_y) {
			helper::move_screen({pos_x, pos_y});
		}

		if (!(minimap->flags & BinDlgFlags::Highlighted)) {
			minimap->flags |= 1;
			helper::update_dialog(minimap);
		}
	}
}