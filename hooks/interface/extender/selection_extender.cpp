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
		return false;
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

		if (*multiplayer_mode)
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
														&& underlay->id <= ImageId::SelectionCircleDashed_224pixels)
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
											&& !helper::create_selection_circle(previous_sprite_2, g_selection_circle_color[previous_player], ImageId::SelectionCircleDashed_22pixels))
										{
											previous_sprite_2->flags &= 0xF9u;
										}
									}
								}
								result = i_3 + 1;
								player_selection = (player_selection + 4);
							}
						}
					}
					++iterations_2;
				//	player_selection = &(*player_selection_2)[12]; // Obviously wrong
					++player_selection_2;
				}
			}
		}
		return result;
	}
}