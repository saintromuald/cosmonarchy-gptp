#include "selection_extender.h"
#include "SCBW/api.h"

namespace hooks {
	s32 select_dual_birth_units(Unit* unit) { // 0045D040
		u32 selection_index = 0;
		Unit* selection_array[SelectionCap];
		memcpy(selection_array, active_player_selection, sizeof(selection_array));
		s32 result = 0;
		while (selection_array[result])
		{
			if (++result >= SelectionCap)
			{
				return result;
			}
		}
		if (result < 12)
		{
			selection_index = result + 1;
			selection_array[result] = unit;
			helper::create_new_unit_selections_from_list((Unit**)selection_array, result + 1);
			helper::cmdact_select(selection_array, selection_index);
			result = 1;
			*client_selection_changed = 1;
			scbw::refresh_console();
		}
		return result;
	}

	bool is_selected_unit_group_enabled() // 0049A2C0
	{
		for (u32 i = 0; i < SelectionCap; ++i)
		{
			Unit* unit_from_selection = active_player_selection->unit[i];
			if (!unit_from_selection)
			{
				break;
			}
			if (unit_from_selection->is_frozen())
			{
				return false;
			}
		}
		return true;
	}

	char select_unit_initial() // 0049B690
	{
		char result = 0;
		for (u32 i = 0; i < SelectionCap; ++i)
		{
			Unit* unit_from_selection = active_player_selection->unit[i];
			if (!unit_from_selection)
			{
				break;
			}
			if (*g_is_in_replay || unit_from_selection->player != *g_local_nation_id)
			{
				if (units_dat::base_property[unit_from_selection->id] & UnitProperty::Subunit)
				{
					helper::destroy_unit_selection(unit_from_selection->subunit);
					helper::create_unit_selection(unit_from_selection->subunit, i);
					active_player_selection->unit[i] = unit_from_selection->subunit;
				}
				else
				{
					helper::destroy_unit_selection(unit_from_selection);
					helper::create_unit_selection(unit_from_selection, i);
				}
			}
		}

		if (*g_multiplayer_mode)
		{
			if (game_data->teams_enabled)
			{
				Unit* player_selection = *players_selections->unit[0];
				Unit* player_selection_2 = *players_selections->unit[0];
				u32 force_offset = 0;
				u32 iterations_2 = 0;
				for (u32 force_offset = 0; force_offset < 0x120; force_offset += 36)
				{
					result = *g_local_human_id;
					if (iterations_2 != result)
					{
						if (g_player_table[result].force == (g_player_table[0].force + force_offset))
						{
							for (u32 i_3 = 0; i_3 < SelectionCap; ++i_3)
							{
								Unit* previous_unit = player_selection->link.prev;
								if (!player_selection->link.prev)
								{
									break;
								}
								if (*g_is_in_replay || previous_unit->player != *g_local_nation_id)
								{
									Sprite* previous_sprite = previous_unit->sprite;
									u32 sprite_flags = previous_sprite->flags >> 1;
									if (sprite_flags & (SpriteFlags::DrawSelCircle | SpriteFlags::Flag01))
									{
										u32 sprite_flags_2 = previous_sprite->flags ^ (previous_sprite->flags ^ 2 * (sprite_flags - 1)) & 6;
										previous_sprite->flags = sprite_flags_2;
										if (!(sprite_flags_2 & (SpriteFlags::Flag01 | SpriteFlags::Flag02)))
										{
											Image* underlay = previous_sprite->images.tail;
											if (underlay)
											{
												while (1)
												{
													if (underlay->id >= ImageId::SelectionCircleDashed_22pixels
													&&  underlay->id <= ImageId::SelectionCircleDashed_224pixels)
													{
														break;
													}
													underlay = underlay->link.prev;
													if (!underlay)
													{
														goto LABEL_29;
													}
												}
												helper::remove_selection_circle_image(underlay);
											}
										}
									}
								LABEL_29:
									u32 previous_player = previous_unit->player;
									Sprite* previous_sprite_2 = previous_unit->sprite;
									u32 sprite_flags_3 = previous_sprite_2->flags;
									if (((sprite_flags_3 >> 1) & (SpriteFlags::DrawSelCircle | SpriteFlags::Flag01)) != helper::illegal_team_check(player_selection) - 1)
									{
										u32 sprite_flags_4 = sprite_flags_3 ^ (sprite_flags_3 ^ ((sprite_flags_3 & 0xFE) + 2)) & 6;
										previous_sprite_2->flags = sprite_flags_4;
										if ((sprite_flags_4 & (SpriteFlags::Flag01 | SpriteFlags::Flag02)) <= 2
										&&  !helper::create_selection_circle(previous_sprite_2, g_selection_circle_color[previous_player], ImageId::SelectionCircleDashed_22pixels))
										{
											previous_sprite_2->flags &= 0xF9u;
										}
									}
								}
								result = i_3 + 1;
								player_selection = player_selection->link.next;
							}
						}
					}
					++iterations_2;
				//	player_selection = (Unit*)&(*player_selection_2)[12]; // Needs to be fixed
					++player_selection_2;
				}
			}
		}
		return result;
	}

	char create_all_selection_circles() // 00499A60
	{
		for (s32 i = 0; i < SelectionCap; ++i) {
			Unit* unit_from_selection = active_player_selection->unit[i];
			if (!unit_from_selection)
			{
				break;
			}
			if (units_dat::base_property[unit_from_selection->id] & UnitProperty::Subunit)
			{
				helper::create_unit_selection(unit_from_selection->subunit, i);
				active_player_selection->unit[i] = unit_from_selection->subunit;
			}
			else
			{
				helper::create_unit_selection(unit_from_selection, i);
			}
		}

		u32 player_zeroed = 0;
		if (*g_multiplayer_mode)
		{
			if (game_data->teams_enabled)
			{
				u32 local_player = *g_local_human_id;
				u32 local_force = g_player_table[*g_local_human_id].force;
				u32 player_iterator = 8;
				char* game_result_text_var = *game_result_text; // Actually a unit var
				u8 last_player_force = g_player_table[8].force;
				while (1)
				{
					Player* player_from_force = 0;						// Avoids compiler errors, obviously not final
				//	Player* player_from_force = last_player_force - 36; // Some struct nonsense
					Player* player_from_force_2 = player_from_force;
					Unit** selection = players_selections->unit[0];
					Unit** selection_2 = selection;
					player_zeroed = player_iterator - 1;
					if (player_zeroed != local_player)
					{
						player_zeroed = player_from_force->id;
						bool iterations_2 = 0;
						bool iterations_3 = 0;
						if (player_zeroed == local_force)
						{
							/*
							for (u32 i = 0; iterations_2 ^ iterations_3; ++i) {

							}
							*/

							bool iterations = 0;
							do
							{
								Unit* unit_from_selection = *selection;
								if (!*selection)
								{
									break;
								}
								Sprite* sprite = unit_from_selection->sprite;
								u8 sprite_flags = sprite->flags;
								u8 color = g_selection_circle_color[unit_from_selection->player];
								if (((sprite_flags >> 1) & 3) != helper::illegal_team_check(*selection) - 1)
								{
									u8 sprite_flags_2 = sprite_flags ^ (sprite_flags ^ ((sprite_flags & 0xFE) + 2)) & 6; // xd
									sprite->flags = sprite_flags_2;
									if ((sprite_flags_2 & 6u) <= 2
										&& !helper::create_selection_circle(sprite, color, ImageId::SelectionCircleDashed_22pixels))
									{
										sprite->flags &= 0xF9u;
									}
								}
								player_zeroed = iterations + 1;
								++selection;
								iterations_3 = helper::OFSUB(iterations + 1, 12);
								iterations_2 = iterations++ - 11 < 0;
							} while (iterations_2 ^ iterations_3);
						}
					}
					/*
					last_player_force = player_from_force_2;
					if (player_from_force_2 == g_player_table[0].force)
					{
						break;
					}
					local_player = *g_local_human_id;
					game_result_text_var = selection_2;
					player_iterator = player_zeroed;
					*/
				}
			}
		}
		return player_zeroed;
	}

	// Ported from GPTP. -Pr0nogo
	void update_selected_unit_data() { // 004C38B0
		Unit** current_unit_offset = (Unit**)g_client_selection_group;
		Unit* unit_at_selection_top = 0;
		for (u32 i = 0; i < SelectionCap; i++)
		{
			g_client_selection_group->unit[i] = active_player_selection->unit[i];
		}

		*g_client_selection_count = 0;
		*g_active_portrait_unit = 0;
		while (*current_unit_offset < g_client_selection_group_end) {
			Unit* current_unit = *current_unit_offset;

			if (current_unit) {
				if (helper::compare_unit_rank(current_unit, unit_at_selection_top) != 0)
				{
					unit_at_selection_top = current_unit;
				}
				(*g_client_selection_count)++;
			}
			current_unit_offset++;
		}

		*g_active_portrait_unit = unit_at_selection_top;
		if (*g_client_selection_count == 1)
		{
			g_client_selection_group->unit[0] = *g_active_portrait_unit;
			for (u32 i = 1; i < SelectionCap; i++)
			{
				g_client_selection_group->unit[i] = 0;
			}
		}
		helper::update_buttonset();
	}
}