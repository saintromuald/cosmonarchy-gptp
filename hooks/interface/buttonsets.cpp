#include "buttonsets.h"
#include "BW/api.h"
#include "globals.h"
#include "hooks/game_hooks.h"
#include "hooks/grafting/button_functions.h"
#include "config.h"
#include "trigger_system/trigger_system.h"
#include "req_table.h"
#include "Game.h"

// Example on how to create a custom button set.
// This makes a copy of the Marine buttonset.
// Note that the function would have to be duplicated for each custom buttonset
ButtonSet* get_custom_buttonset() {
	// Use static variables to avoid memory allocation trouble
	static ButtonSet custom_buttonset;
	static Button custom_buttons_array[50];

	// Since it's a copy of the marine set, it would have the same amount of buttons
	custom_buttonset.buttons_in_set = g_buttonset_table[UnitId::TerranMaverick].buttons_in_set;

	// connected_unit should usually be init at 0xFFFF.Almost unused (maybe not fully implemented).
	custom_buttonset.connected_unit = 0xFFFF;

	// This should always be like this since the array is static
	custom_buttonset.first_button = &custom_buttons_array[0];

	// For each button in the array until reaching the amount of buttons_in_set, copy the data for each member of the button structure from the marine to the custom set.
	for (u32 i = 0; i < custom_buttonset.buttons_in_set; i++) {
		custom_buttons_array[i].action_func = g_buttonset_table[UnitId::TerranMaverick].first_button[i].action_func;
		custom_buttons_array[i].action_string_id = g_buttonset_table[UnitId::TerranMaverick].first_button[i].action_string_id;
		custom_buttons_array[i].action_var = g_buttonset_table[UnitId::TerranMaverick].first_button[i].action_var;
		custom_buttons_array[i].icon_id = g_buttonset_table[UnitId::TerranMaverick].first_button[i].icon_id;
		custom_buttons_array[i].position = g_buttonset_table[UnitId::TerranMaverick].first_button[i].position;
		custom_buttons_array[i].requirement_func = g_buttonset_table[UnitId::TerranMaverick].first_button[i].requirement_func;
		custom_buttons_array[i].requirement_string_id = g_buttonset_table[UnitId::TerranMaverick].first_button[i].requirement_string_id;
		custom_buttons_array[i].requirement_var = g_buttonset_table[UnitId::TerranMaverick].first_button[i].requirement_var;
	}
	return &custom_buttonset;
}

ButtonsetContext buttonset_context;

namespace hooks {
	static Unit* get_selected_unit() {
		Unit* active = *g_active_portrait_unit;
		for (u32 i = 0; i < SelectionCap; ++i) {
			Unit* unit = g_client_selection_group->unit[i];
			if (unit && unit->status & UnitStatus::Completed) {
				return unit;
			}
		}
		return active;
	}

	// 00458BC0
	// Called (with a jump) as last instruction of update_buttonset
	// Will update the buttonset for the currently unit active type, or handle special cases of mixed selections
	void update_buttonset_ex() {
		u8 count_workers = 0;
		u8 count_burrowables = 0;
		u8 count_larva = 0;
		u8 count_ocsal_larva = 0;
		u8 count_eggs  = 0;
		u8 count_zerg_production_base = 0;
		u8 count_zerg_production_ocsal = 0;
		u8 count_zergs = 0;
		u8 count_buildings = 0;
		u8 count_attacking_buildings = 0;
		u8 count_lifted_buildings = 0;
		u8 count_repairmen = 0;

		u8 count_nexuses = 0;
		u8 count_ministries = 0;

		u16 active_button_set_id;
		u16 selected_button_set_id;
		u32 selected_properties;
		Unit* selected;

		Unit* active = get_selected_unit();
		active_button_set_id = active->current_buttonset;

		for (u32 i = 0; i < SelectionCap; ++i) { // count units to know if will be using special group buttonset (workers, burrowers, etc)
			Unit* selected = g_client_selection_group->unit[i];
			if (selected) {
				selected_button_set_id = selected->current_buttonset;

				selected_properties = units_dat::base_property[selected->id];
				if (selected_properties & UnitProperty::Worker) {
					count_workers++;
				}

				if (selected_properties & UnitProperty::Building) {
					count_buildings++;
					if (selected->get_ground_weapon() != WeaponId::None || selected->get_air_weapon() != WeaponId::None) {
						count_attacking_buildings++;
					}
					if (selected->status & UnitStatus::InAir) {
						count_lifted_buildings++;
					}
				}

				if (selected_properties & UnitProperty::Burrowable) {
					count_burrowables++;
				}

				if (units_dat::group_flags[selected->id].is_zerg) {
					count_zergs++;
				}

				if (selected->id == UnitId::ZergLarva) {
					count_larva++;
				}

				if (selected->id == UnitId::ZergOcsalLarva) {
					count_ocsal_larva++;
				}

				if (selected->id == UnitId::ZergEgg) {
					count_eggs++;
				}

				if (unit_id_can_repair(selected->id)) {
					count_repairmen++;
				}

				switch (selected->id) {
					case UnitId::TerranMinistry: {
						count_ministries++;
					} break;
					case UnitId::ProtossNexus: {
						count_nexuses++;
					} break;
					case UnitId::ZergHachirosk:
					case UnitId::ZergGathtelosk:
					case UnitId::ZergLarvoskCesiant: {
						count_zerg_production_base++;
					} break;
					case UnitId::ZergCaphrolosk:
					case UnitId::ZergOcsalCesiant: {
						count_zerg_production_ocsal++;
					}
				}
			}
		}

		for (u32 i = 0; i < SelectionCap; ++i) {
			Unit* selected = g_client_selection_group->unit[i];
			if (selected) {
				// if you use a custom buttonset, and are reusing the current_buttonset property of an active differently, change these conditions to take it in account
				if (selected->status & UnitStatus::Completed && selected_button_set_id != active_button_set_id) {
					ButtonSet* selected_button_set = hooks::get_buttonset(selected_button_set_id);
					ButtonSet* active_button_set = hooks::get_buttonset(active_button_set_id);
					if (active_button_set->first_button != selected_button_set->first_button) {
						if (selected_button_set->connected_unit != active_button_set_id) {
							// Kerrigan (Ghost) and Raynor (Vulture) have the corresponding normal active as "connected active"
							if (active_button_set->connected_unit == selected_button_set_id) {
								active_button_set_id = selected_button_set_id;
								*g_current_buttonset_id = selected_button_set_id;
							} else {
								*g_current_buttonset_id = UnitId::Buttons_GroupMixed;
							}
						}
					}
				}
			}
		}
		if (count_larva && count_larva + count_eggs == g_client_selection_count) {
			*g_current_buttonset_id = UnitId::ZergLarva;
		}
		if (count_ocsal_larva && count_ocsal_larva + count_eggs == g_client_selection_count) {
			*g_current_buttonset_id = UnitId::ZergOcsalLarva;
		}
		if (*g_current_buttonset_id != UnitId::Buttons_Blank) {
			if (count_repairmen == g_client_selection_count) {
				*g_current_buttonset_id = UnitId::Buttons_MixedRepairmen;
			} else if (count_workers == g_client_selection_count) {
				*g_current_buttonset_id = UnitId::Buttons_GroupWorkers;
			} else if (count_burrowables) {
				*g_current_buttonset_id = UnitId::Buttons_GroupBurrower;
			} else if (count_ministries == g_client_selection_count) {
				*g_current_buttonset_id = UnitId::TerranMinistry;
			} else if (count_nexuses == g_client_selection_count) {
				*g_current_buttonset_id = UnitId::ProtossNexus;
			} else if (count_zerg_production_base == g_client_selection_count) {
				*g_current_buttonset_id = UnitId::ZergHachirosk;
			} else if (count_zerg_production_ocsal == g_client_selection_count) {
				*g_current_buttonset_id = UnitId::ZergCaphrolosk;
			} else if (count_lifted_buildings) {
				*g_current_buttonset_id = UnitId::Buttons_GroupMixed; // TODO: Replace with Buttons_LiftedMixed or something
			} else if (count_attacking_buildings) {
				*g_current_buttonset_id = UnitId::ProtossWarden;
			} else if (count_buildings) {
				*g_current_buttonset_id = UnitId::TerranGoliathTurret;
			}
		}
	}

	// 00458D50
	// Not calling others converted to C functions
	// Probably making it so the button corresponding to current order has light on
	// Last function called within updateCurrentButtonSet
	void highlight_button() {
		u8 current_unit_main_order_id = *g_buttonset_unit_order_id;
		u16 current_unit_button_icon = orders_dat::button_icon[current_unit_main_order_id];
		BinDlg* current_button_dialog = (*g_buttonset_dialog)->child_dialog;
		u8 current_subunit_main_order_id = *g_buttonset_subunit_order_id;

		// if the subunit holds the weapon, get the icon from it
		u16 current_subunit_button_icon = 0xFFFF;
		if (current_subunit_main_order_id != 0xFF) {
			current_subunit_button_icon = orders_dat::button_icon[current_subunit_main_order_id];
		}

		u16 used_button_icon;
		if (current_unit_button_icon != 0xFFFF) {
			if (current_subunit_button_icon == 0xFFFF || current_unit_button_icon != IconId::Stop) {
				used_button_icon = current_unit_button_icon;
			} else {
				used_button_icon = current_subunit_button_icon;
			}
		} else {
			used_button_icon = current_subunit_button_icon;
		}

		if (current_button_dialog) {
			bool stop_looping = false;
			do {
				// look for the button corresponding to the one used for the current main_order
				if (current_button_dialog->flags & BinDlgFlags::Visible) {
					if (current_button_dialog->graphic != used_button_icon) {
						current_button_dialog = current_button_dialog->next;
					} else {
						stop_looping = true;
					}
				} else {
					current_button_dialog = current_button_dialog->next;
				}
			} while (!stop_looping && current_button_dialog);
		}
		if (!current_button_dialog) {
			*g_highlighted_icon = 0xFFFF;
		} else {
			if (!(current_button_dialog->flags & BinDlgFlags::Highlighted)) {
				current_button_dialog->flags |= BinDlgFlags::Highlighted;
				bw::update_dialog(current_button_dialog);
			}
			*g_highlighted_icon = used_button_icon;
		}
	}

	// 00458DE0
	// Case when buttonset would use g_buttonset_special_buttonset_id
	// Use update_buttonset_ex() for individual units and groups
	// Called by update_current_buttonset and updateSelectedUnitData (004C38B0)
	void update_buttonset() {
		*g_buttonset_special_buttonset_id = UnitId::Buttons_Blank;
		*g_active_portrait_buttonset_id = UnitId::Buttons_Blank;
		if (bw::in_replay()) {
			if (*g_is_replay_paused) {
				*g_buttonset_special_buttonset_id = UnitId::Buttons_ReplayPaused;
			} else {
				*g_buttonset_special_buttonset_id = UnitId::Buttons_ReplayPlaying;
			}
			*g_current_buttonset_id = UnitId::Buttons_Blank;
			return;
		}
		if (g_client_selection_count > 0) {
			if (g_is_targeting) {
				*g_buttonset_special_buttonset_id = UnitId::Buttons_Cancel;
			} else if (g_is_placing_building) {
				*g_buttonset_special_buttonset_id = UnitId::Buttons_CancelPlaceBuilding;
			}
		}
		*g_current_buttonset_id = UnitId::Buttons_Blank;
		if (g_client_selection_count > 1) {
			update_buttonset_ex();
		}
	}

	// Add active/unit units checks here and you can change the buttonset to any you created here in GPTP using something based on get_custom_buttonset method or a more customized way.
	// Note: if you use update_command_card, you can disable the injection of this function in the _inject.cpp file.
	ButtonSet* get_buttonset(s32 index) {
		return &(g_buttonset_table[index]);
	}

	// Main update_buttonset function; calls update_buttonset() and others
	void update_current_buttonset() { // 004599A0
		// Quick targeting post-handling
		if (g_game->m_quick_casting.signal) {
			g_game->m_quick_casting.signal = false;
			*g_buttonset_special_buttonset_id = g_game->m_quick_casting.command_card;
			g_minimap_cursor = 0;
		}

		u16 buttonset_id = *g_buttonset_special_buttonset_id;
		b32 refresh = g_force_button_refresh;
		Unit* active_unit = *g_active_portrait_unit;
		if (g_force_button_refresh && (*g_buttonset_special_buttonset_id == UnitId::None || g_is_in_replay)) {
			helper::update_buttonset();
			buttonset_id = *g_buttonset_special_buttonset_id;
		}
		if (buttonset_id == UnitId::None) {
			buttonset_id = *g_current_buttonset_id;
			if (*g_current_buttonset_id == UnitId::None) {
				if (!active_unit) {
					goto BLOCK_highlight;
				}
				buttonset_id = active_unit->current_buttonset;
			}
		}
		if (*g_active_portrait_buttonset_id != buttonset_id) {
			*g_active_portrait_buttonset_id = buttonset_id;
			g_force_button_refresh = true;
		BLOCK_update_then_highlight:
			helper::update_command_card();
			helper::refresh_button_tooltip();
			if (active_unit) {
				Unit* subunit = active_unit->subunit;
				u8 unit_order_id = active_unit->main_order_id;
				u8 subunit_order_id;
				if (subunit) {
					subunit_order_id = subunit->main_order_id;
				} else {
					subunit_order_id = -1;
				}
				highlight_button();
				*g_buttonset_unit_order_id = unit_order_id;
				*g_buttonset_subunit_order_id = subunit_order_id;
				highlight_button();
			}
			if (g_is_in_replay) {
				helper::replay_stat_buttons(*g_buttonset_dialog);
			}
			g_force_button_refresh = false;
			return;
		}
	BLOCK_highlight:
		if (refresh) {
			goto BLOCK_update_then_highlight;
		}
		if (active_unit) {
			Unit* subunit_2 = active_unit->subunit;
			u8 unit_order_id_2 = active_unit->main_order_id;
			u8 subunit_order_id_2 = subunit_2 ? subunit_2->main_order_id : -1;
			if (*g_buttonset_unit_order_id != active_unit->main_order_id || *g_buttonset_subunit_order_id != subunit_order_id_2) {
				highlight_button();
				*g_buttonset_unit_order_id = unit_order_id_2;
				*g_buttonset_subunit_order_id = subunit_order_id_2;
				highlight_button();
			}
		}
	}

	s32 compare_button(const void* a, const void* b) {
		Button* button_a = (Button*)a;
		Button* button_b = (Button*)b;

		s32 pos_diff = button_a->position - button_b->position;
		if (pos_diff) return button_a->position - button_b->position;

		return button_a->icon_id - button_b->icon_id;
	}

	static char* find_hotkey_char(char* label) {
		for (char* c = label; *c; ++c) {
			if (c[0] == 1 && c[1] == '(' && c[2] == 03 && c[3] && c[4] == 1 && c[5] == ')') {
				return c + 3;
			}
		}
		return 0;
	}

	void update_command_card() {
		init_req_table();

		struct SelectFactionCommand {
			u8 type;
			u8 subtype;
			u8 player;
			Faction faction;
			u8 padding[8];
		};
		static_assert(sizeof(SelectFactionCommand) == 12);

		SelectFactionCommand command = {};
		command.type = GameCommand_Custom;
		command.subtype = NewGameCommand::SelectFaction;
		command.player = *g_local_human_id;
		command.faction = Faction::None; // Default case
		auto buttonset_id = *g_active_portrait_buttonset_id;
		u32 local_player = *g_local_human_id;
		auto& cm_player = g_globals->players[local_player];
		if (bw::get_game_kind() == GameKind::FactionWars && !cm_player.faction_was_set) {
			if (buttonset_id == UnitId::None) {
				switch (g_player_table[local_player].race) {
					case RaceId::Terran: {
						buttonset_id = UnitId::Buttons_FactionSelect;
					} break;
					case RaceId::Protoss: {
						buttonset_id = UnitId::Buttons_FactionSelectProtoss;
					} break;
					case RaceId::Zerg: {
						buttonset_id = UnitId::Buttons_FactionSelectZerg;
					} break;
					case RaceId::Askosi: {
						// TODO: Add Askosi case
					} break;
				}
			} else if (buttonset_id == UnitId::Buttons_FactionSelect) {
				switch (g_player_table[local_player].race) {
					case RaceId::Terran: {
						// TODO: Race-specific buttonset code
					} break;
					case RaceId::Protoss: {
						// TODO: Race-specific buttonset code
					} break;
					case RaceId::Zerg: {
						// TODO: Race-specific buttonset code
					} break;
					case RaceId::Askosi: {
						// TODO: Add Askosi case
					} break;
				}
			} else if (buttonset_id == UnitId::Buttons_FactionSelect1) {
                switch (g_player_table[local_player].race) {
                    case RaceId::Terran: {
                        command.faction = Faction::TerranOrionImperium;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                    case RaceId::Protoss: {
                        command.faction = Faction::ProtossDivinePrimacy;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                    case RaceId::Zerg: {
                        command.faction = Faction::ZergAtaltrethTendril;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                }
                cm_player.faction_was_set = true;
            } else if (buttonset_id == UnitId::Buttons_FactionSelect2) {
                switch (g_player_table[local_player].race) {
                    case RaceId::Terran: {
                        command.faction = Faction::TerranHecatonCoalition;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                    case RaceId::Protoss: {
                        command.faction = Faction::ProtossOrderOfTheSacredSaber;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                    case RaceId::Zerg: {
                        command.faction = Faction::ZergImicrenTendril;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                }
                cm_player.faction_was_set = true;
            } else if (buttonset_id == UnitId::Buttons_FactionSelect3) {
                switch (g_player_table[local_player].race) {
                    case RaceId::Terran: {
                        command.faction = Faction::TerranFuturistFederation;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                    case RaceId::Protoss: {
                        command.faction = Faction::ProtossKhalendricKnights;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                    case RaceId::Zerg: {
                        command.faction = Faction::ZergOrkotiTendril;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                }
                cm_player.faction_was_set = true;
            } else if (buttonset_id == UnitId::Buttons_FactionSelect4) {
                switch (g_player_table[local_player].race) {
                    case RaceId::Terran: {
                        command.faction = Faction::TerranGoldenRepublic;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                    case RaceId::Protoss: {
                        command.faction = Faction::ProtossStarbound;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                    case RaceId::Zerg: {
                        command.faction = Faction::ZergGoszirithTendril;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                }
                cm_player.faction_was_set = true;
            } else if (buttonset_id == UnitId::Buttons_FactionSelect5) {
                switch (g_player_table[local_player].race) {
                    case RaceId::Terran: {
                        command.faction = Faction::TerranNydusTrinity;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                //	case RaceId::Protoss: {
                //		command.faction = Faction::Protoss5;
                //		helper::queue_game_command((u8*)&command, sizeof(command));
                //	} break;
                    case RaceId::Zerg: {
                        command.faction = Faction::ZergZorthosTendril;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                }
                cm_player.faction_was_set = true;
            } else if (buttonset_id == UnitId::Buttons_FactionSelect6) {
                switch (g_player_table[local_player].race) {
                    case RaceId::Terran: {
                        command.faction = Faction::TerranUpstart;
                        helper::queue_game_command((u8*)&command, sizeof(command));
                    } break;
                //	case RaceId::Protoss: {
                //		command.faction = Faction::Protoss6;
                //		helper::queue_game_command((u8*)&command, sizeof(command));
                //	} break;
                //	case RaceId::Zerg: {
                //		command.faction = Faction::Zerg6;
                //		helper::queue_game_command((u8*)&command, sizeof(command));
                //	} break;
                }
                cm_player.faction_was_set = true;
            }
		}

		// Custom buttonset overrides
		if (buttonset_id == UnitId::TerranTreasury) {
			buttonset_id = UnitId::Special_StartLocation;
		}

		bool has_larva = false;
		bool has_ocsal_larva = false;
		bool has_egg = false;
		for (u32 i = 0; i < SelectionCap; ++i) {
			Unit* unit = g_client_selection_group->unit[i];
			if (!unit) {
				break;
			}
			if (unit->id == UnitId::ZergLarva) {
				has_larva = true;
			}
			if (unit->id == UnitId::ZergOcsalLarva) {
				has_ocsal_larva = true;
			}
			if (unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon) {
				has_egg = true;
			}
		}

		// Fix larva category buttons not clearing for eggs
		if (!has_larva && !has_ocsal_larva && has_egg) {
			if (buttonset_id == UnitId::Buttons_LarvaSwarm
			||  buttonset_id == UnitId::Buttons_LarvaAssault
			||  buttonset_id == UnitId::Buttons_LarvaControl
			||  buttonset_id == UnitId::Buttons_OcsalLarvaSwarm
			||  buttonset_id == UnitId::Buttons_OcsalLarvaAssault
			||  buttonset_id == UnitId::Buttons_OcsalLarvaControl) {
				helper::BTNSACT_ChangeButtons(UnitId::ZergEgg);
				return;
			}
		}
		if (has_larva && !has_ocsal_larva && !has_egg && buttonset_id == UnitId::ZergEgg) {
			helper::BTNSACT_ChangeButtons(UnitId::ZergLarva);
			return;
		}
		if (has_ocsal_larva && !has_larva && !has_egg && buttonset_id == UnitId::ZergEgg) {
			helper::BTNSACT_ChangeButtons(UnitId::ZergLarva);
			return;
		}

		ButtonSet* const current_buttonset = get_buttonset(buttonset_id);
		bool stop_there = false;
		bool jumpto59336 = false;
		bool jumpto5933C = false;
		bool jumpto5934E = false;
		s32 current_button_state;
		Button* current_button;
		u16 buttons_count = 0;
		current_button = current_buttonset->first_button;
		current_button_state = Button_State::Invisible;

		Unit* unit = get_selected_unit();
		bool changed = false;
		for (u32 i = 0; i < current_buttonset->buttons_in_set; ++i) {
			Button* button = &current_buttonset->first_button[i];
			if (button->icon_id == IconId::Cancel) {
				if (unit_id_is_larva(buttonset_id) || buttonset_id == UnitId::ZergEgg || buttonset_id == UnitId::ZergCocoon
					|| units_dat::group_flags[unit->id].is_zerg && units_dat::group_flags[unit->id].is_factory) {
					button->position = 6;
				} else if (buttonset_id == UnitId::ProtossLanifect || buttonset_id == UnitId::ProtossDidact) {
					button->position = 7;
				} else if (buttonset_id == UnitId::TerranApostle) {
					button->position = 8;
				} else {
					button->position = 9;
				}
				changed = true;
			}

			if (g_config.flip_attack_and_patrol) {
				switch (button->icon_id) {
					case IconId::Attack: {
						button->position = 4;
                        changed = true;
                    } break;
					case IconId::Patrol:
					case IconId::Gather: {
						button->position = 3;
                        changed = true;
                    } break;
				}
			}
		}
		if (changed) {
			qsort(current_buttonset->first_button, current_buttonset->buttons_in_set, sizeof(Button), compare_button);
		}

		BinDlg* current_dialog = *g_buttonset_dialog;
		if (current_dialog->control_type != DialogControlTypes::DialogBoxDC) {
			current_dialog = current_dialog->parent;
		}
		current_dialog = current_dialog->child_dialog;
		while (current_dialog && current_dialog->index != 1) {
			current_dialog = current_dialog->next;
		}
		if (!current_dialog || current_dialog->index != 1) {
			stop_there = true;
		}

		// Overwrite mnemonic hotkey text with grid hotkeys, if applicable
		if (!stop_there && g_config.grid_hotkeys) {
			for (BinDlg* dialog = current_dialog; dialog; dialog = dialog->next) {
				if (dialog->index > 0 && dialog->index <= 9
					&& dialog->control_type == DialogControlTypes::Button
					&& dialog->button_user && dialog->button_user->requirement_string_id) {
					// we normally want const string to avoid accidental overwrites, but here we intend to overwrite the hotkey character, that's why we cast to char*
					char* label = (char*)helper::get_stat_txt_string(dialog->button_user->requirement_string_id); 
					char* hotkey_char = find_hotkey_char(label + 2);
					if (hotkey_char) {
						u32 index = dialog->index - 1;
						*hotkey_char = toupper(g_config.grid_layout[index / 3].at(index % 3));
					}
				}
			}
		}

		UnitExt* unit_ext = unit ? get_unit_ext(unit) : 0;
		while (!stop_there) {
			// 00459230
			if (current_dialog->index < 1) {
				stop_there = true;
			} else if (current_dialog->index > 9) {
				jumpto5934E = true;
			} else { // loop to ignore the first unused/disabled buttons
				current_button_state = Button_State::Invisible;
				while (current_button_state == Button_State::Invisible && (buttons_count < current_buttonset->buttons_in_set)) {
					current_button_state = helper::req_check((u32)current_button->requirement_func, current_button->requirement_var, g_local_nation_id, unit);
					// Fight that lardcode, tony!
					if (current_button->action_func == (ActFunc*)ButtonActions::Change_Displayed_Buttons) {
						if (buttonset_id == UnitId::ZergLarva || buttonset_id == UnitId::ZergOcsalLarva || buttonset_id_is_faction_selection(buttonset_id)) {
							current_button_state = Button_State::Enabled;
						}
						if ((buttonset_id == UnitId::TerranMason || buttonset_id == UnitId::TerranTalos) && (unit->main_order_id != OrderId::ConstructingBuilding)) {
							if (!(unit->main_order_id == OrderId::BuildTerran && unit->order_target.unit) && g_client_selection_count == 1)
							{
								current_button_state = Button_State::Enabled;
							}
						}
					}

					if (current_button->requirement_func == (ReqFunc*)ButtonRequirements::Spell_Researched) {
						UnitExt* unit_ext = get_unit_ext(unit);
						// Terran structure buttonset handling - Controlled Demolition / Emergency Detonation
						switch (current_button->requirement_var) {
							case TechId::ControlledDemolition: {
								if (unit_ext->timers[TimerControlledDemolition] || unit_ext->timers[TimerEmergencyDetonation]) {
									current_button_state = Button_State::Invisible;
								} else {
									current_button_state = Button_State::Enabled;
								}
							} break;
							case TechId::EmergencyDetonation: {
								if (!unit_ext->timers[TimerControlledDemolition] || unit_ext->timers[TimerEmergencyDetonation]) {
									current_button_state = Button_State::Invisible;
								} else {
									current_button_state = Button_State::Enabled;
								}
							} break;
							case TechId::HypnagogueThroughTheLookingGlass: {
								current_button_state = Button_State::Enabled;
							} break;
							case TechId::LarvalSubmenuSelectSwarm:
							case TechId::LarvalSubmenuSelectAssault:
							case TechId::LarvalSubmenuSelectControl: {
								if ((!units_dat::group_flags[unit->id].is_zerg || !units_dat::group_flags[unit->id].is_factory || !unit_id_is_larva(unit->id))) {
									current_button_state = Button_State::Invisible;
									break;
								}
								if (unit->id != UnitId::Omnivale || g_globals->players[unit->player].game_mode_race == GameModeRace_TerranProtoss) {
									current_button_state = Button_State::Invisible;
									break;
								}
								current_button_state = Button_State::Enabled;
							} break;
							case TechId::GoskaLobotomyGrenade: {
								if (unit->id == UnitId::Goska && bw::client_selection_count() == 1) {
									current_button_state = Button_State::Enabled;
								}
							} break;
							case TechId::StroluumSkryspawn: {
								if (unit_ext->timers[TimerSkryspawn]) {
									current_button_state = Button_State::Disabled;
								} else if (unit->status & UnitStatus::Burrowed) {
									current_button_state = Button_State::Invisible;
								} else {
									current_button_state = Button_State::Enabled;
								}
							} break;
						}

						if (current_button->requirement_var == TechId::ControlledDemolition) {
							if (unit_ext->timers[TimerControlledDemolition] || unit_ext->timers[TimerEmergencyDetonation]) {
								current_button_state = Button_State::Invisible;
							} else {
								current_button_state = Button_State::Enabled;
							}
						} else if (current_button->requirement_var == TechId::EmergencyDetonation) {
							if (!unit_ext->timers[TimerControlledDemolition] || unit_ext->timers[TimerEmergencyDetonation]) {
								current_button_state = Button_State::Invisible;
							} else {
								current_button_state = Button_State::Enabled;
							}
						} else if (current_button->requirement_var == TechId::HypnagogueThroughTheLookingGlass) {
							current_button_state = Button_State::Enabled;
						} else if (current_button->requirement_var == TechId::LarvalSubmenuSelectSwarm
							||	   current_button->requirement_var == TechId::LarvalSubmenuSelectAssault
							||	   current_button->requirement_var == TechId::LarvalSubmenuSelectControl)
						{
							if ((units_dat::group_flags[unit->id].is_zerg && units_dat::group_flags[unit->id].is_factory)
								|| unit_id_is_larva(unit->id)
								|| (unit->id == UnitId::Omnivale && g_globals->players[unit->player].game_mode_race != GameModeRace_TerranProtoss)) {
								current_button_state = Button_State::Enabled;
							} else {
								current_button_state = Button_State::Invisible;
							}
						}
					}

					if (current_button->icon_id == IconId::Liftoff) {
						bool hide = true;
						for (u32 i = 0; i < g_client_selection_count; ++i) {
							Unit* unit = g_client_selection_group->unit[i];
							UnitExt* unit_ext = get_unit_ext(unit);
							if (!unit_ext->values[ValueIsNeutralBunker]) {
								hide = false;
								break;
							}
						}
						if (hide) {
							current_button_state = Button_State::Invisible;
						}
					}
					
					if (current_button->icon_id == IconId::SetRallyPoint) {
						if ((unit_id_is_larva(unit->id) || unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon) && has_egg) {
							current_button_state = Button_State::Enabled;
						}
						if (unit->id == UnitId::ZergCagrantCesiant) {
							current_button_state = Button_State::Enabled;
						}
					}

					if (current_button->icon_id == IconId::Burrow || current_button->icon_id == IconId::LakiziliskBurrow) {
						bool has_burrowed = false;
						bool has_unburrowed = false;

						for (u32 i = 0; i < g_client_selection_count; ++i) {
							Unit* unit = g_client_selection_group->unit[i];
							if (unit->status & UnitStatus::Burrowed) {
								has_burrowed = true;
							} else {
								has_unburrowed = true;
							}
						}

						if (has_unburrowed && !has_burrowed) {
							current_button_state = Button_State::Enabled;
						} else {
							current_button_state = Button_State::Invisible;
						}
					}

					if (current_button->icon_id == IconId::Unburrow || current_button->icon_id == IconId::LakiziliskUnburrow) {
						bool has_burrowed = false;
						bool has_unburrowed = false;

						for (u32 i = 0; i < g_client_selection_count; ++i) {
							Unit* unit = g_client_selection_group->unit[i];
							if (unit->status & UnitStatus::Burrowed) {
								has_burrowed = true;
							} else {
								has_unburrowed = true;
							}
						}

						if (has_burrowed) {
							current_button_state = Button_State::Enabled;
						} else {
							current_button_state = Button_State::Invisible;
						}
					}

					if (current_button->icon_id == IconId::Larva) {
						if ((units_dat::group_flags[unit->id].is_zerg && units_dat::group_flags[unit->id].is_factory)
							|| unit_id_is_larva(unit->id)
							|| (unit->id == UnitId::Omnivale && g_globals->players[unit->player].game_mode_race != GameModeRace_TerranProtoss)) {
							current_button_state = Button_State::Enabled;
						} else {
							current_button_state = Button_State::Invisible;
						}
					}

					if (buttonset_id == UnitId::ZergLarva && !has_egg && current_button->icon_id == IconId::Cancel) {
						current_button_state = Button_State::Invisible;
					}

					if (buttonset_id == UnitId::ZergOcsalLarva && !has_egg && current_button->icon_id == IconId::Cancel) {
						current_button_state = Button_State::Invisible;
					}

					if (current_button->action_func == (ActFunc*)ButtonActions::Create_AddOn && g_client_selection_count > 1) {
						current_button_state = Button_State::Invisible;
					}

					if (current_button->action_func == (ActFunc*)ButtonActions::Attack_Building && unit->status & UnitStatus::InAir) {
						current_button_state = Button_State::Invisible;
					}

					if (current_button->requirement_func == (ReqFunc*)ButtonRequirements::Attacking_Building && unit->status & UnitStatus::InAir) {
						current_button_state = Button_State::Invisible;
					}

					if (current_button->icon_id == IconId::Land && g_client_selection_count > 1) {
						current_button_state = Button_State::Invisible;
					}

					if (current_button->requirement_func == (ReqFunc*)ButtonRequirements::Tank_Is_In_Siege_Mode) {
						if (unit_id_is_siege_mode((UnitId::Enum)unit->id)) {
							current_button_state = Button_State::Enabled;
						} else {
							current_button_state = Button_State::Invisible;
						}
					}

					if (current_button->requirement_func == (ReqFunc*)ButtonRequirements::Tank_Is_In_Tank_Mode) {
						if (unit_id_is_tank_mode((UnitId::Enum)unit->id)) {
							current_button_state = Button_State::Enabled;
						} else {
							current_button_state = Button_State::Invisible;
						}
					}

					enum {
						Tier1 = 0,
						Tier2,
						Tier3,
						Tier4,
					};

					bool TierUnlocked[3][4]{};
					TierUnlocked[RaceId::Protoss][Tier1] = true;
					TierUnlocked[RaceId::Protoss][Tier2] = false;
					TierUnlocked[RaceId::Protoss][Tier3] = false;
					TierUnlocked[RaceId::Protoss][Tier4] = false;

					bool has_addon = 0;
					bool in_air = 0;
					bool is_training = 0;
					bool is_burrowed = 0;
					bool is_adding_on = 0;
					Faction faction = Faction::None;
					if (unit) {
						has_addon = units_dat::base_property[unit->id] & UnitProperty::Building && unit->building.addon;
						in_air = unit->status & UnitStatus::InAir;
						is_training = unit->build_queue[unit->build_queue_slot] != UnitId::None;
						is_burrowed = unit->status & UnitStatus::Burrowed;
						is_adding_on = unit->current_build_unit && units_dat::base_property[unit->current_build_unit->id] & UnitProperty::Addon;
						if (unit->player <= PLAYABLE_PLAYER_COUNT) {
							faction = g_globals->players[unit->player].faction;
						}

						TierUnlocked[RaceId::Protoss][Tier2] = player_has_completed_powered_unit_id(unit->player, UnitId::ProtossEmbassy)
							|| player_has_completed_powered_unit_id(unit->player, UnitId::ProtossGateway)
							|| player_has_completed_powered_unit_id(unit->player, UnitId::ProtossLattice);
						TierUnlocked[RaceId::Protoss][Tier3] = player_has_completed_powered_unit_id(unit->player, UnitId::ProtossGrandLibrary)
							|| player_has_completed_powered_unit_id(unit->player, UnitId::ProtossRogueGallery)
							|| player_has_completed_powered_unit_id(unit->player, UnitId::ProtossArdentAuthority)
							|| player_has_completed_powered_unit_id(unit->player, UnitId::ProtossStargate);
						TierUnlocked[RaceId::Protoss][Tier4] = player_has_completed_powered_unit_id(unit->player, UnitId::ProtossProstrationStage)
							|| player_has_completed_powered_unit_id(unit->player, UnitId::ProtossUnitaryUnion)
							|| player_has_completed_powered_unit_id(unit->player, UnitId::ProtossArgosy);
					}

					if (current_button->requirement_func == (ReqFunc*)ButtonRequirements::Can_Create_UnitorBuilding) {
						switch (unit->id) {
							case UnitId::TerranStockade: {
								switch (current_button->requirement_var) {
									case UnitId::TerranHarakan: {
										if (bw::unit_queue_full(unit) || is_adding_on || in_air || faction == Faction::TerranGoldenRepublic) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;

									case UnitId::TerranCleric:
									case UnitId::TerranShamanSupportMode: {
										if (!bw::all_selected_units_have_addon(UnitId::TerranVestry) || bw::unit_queue_full(unit)) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::TerranEidolon:
									case UnitId::TerranSavant: {
										if (!bw::all_selected_units_have_addon(UnitId::TerranCovenant) || bw::unit_queue_full(unit)) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;

									case UnitId::TerranCyprian: {
										if (faction == Faction::TerranNydusTrinity) {
											current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::TerranHarakanGolden: {
										if (bw::unit_queue_full(unit) || is_adding_on || in_air || faction != Faction::TerranGoldenRepublic) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::TerranCohort: {
										if (bw::unit_queue_full(unit) || is_adding_on || in_air || faction != Faction::TerranNydusTrinity) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
								}
							} break;
							case UnitId::TerranCaptaincy: {
								switch (current_button->requirement_var) {
									case UnitId::TerranApostle: {
										if (faction != Faction::TerranHecatonCoalition) {
											if (bw::unit_queue_full(unit) || is_adding_on || in_air) {
												current_button_state = Button_State::Invisible;
											} else {
												current_button_state = Button_State::Enabled;
											}
										} else {
											current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::TerranAuriel: {
										if (faction == Faction::TerranHecatonCoalition) {
											if (bw::unit_queue_full(unit) || is_adding_on || in_air) {
												current_button_state = Button_State::Invisible;
											} else {
												current_button_state = Button_State::Enabled;
											}
										} else {
											current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::TerranSiren:
									case UnitId::TerranStriga: {
										if (!bw::all_selected_units_have_addon(UnitId::TerranTinkerersTower) || bw::unit_queue_full(unit)) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::TerranOlympian:
									case UnitId::TerranAutocrat: {
										if (!bw::all_selected_units_have_addon(UnitId::TerranGuildhall) || bw::unit_queue_full(unit)) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
								}
							} break;
							case UnitId::TerranFulcrum: {
								switch (current_button->requirement_var) {
									case UnitId::TerranBlackjack:
									case UnitId::TerranMatadorTankMode: {
										if (!bw::all_selected_units_have_addon(UnitId::TerranScrapyard) || bw::unit_queue_full(unit)) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::TerranMadrigalImperiosoMode: {
										if (!bw::all_selected_units_have_addon(UnitId::TerranPalladium) || bw::unit_queue_full(unit)) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;

									case UnitId::TerranCyclops: {
										if (faction == Faction::TerranFuturistFederation) {
											current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::TerranCyclopsFuturist: {
										if (faction != Faction::TerranFuturistFederation) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;

									case UnitId::TerranGoliath: {
										if (faction == Faction::TerranGoldenRepublic
										||	faction == Faction::TerranUpstart) {
											current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::TerranSouthpaw: {
										if (bw::unit_queue_full(unit) || is_adding_on || in_air || faction != Faction::TerranUpstart) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::TerranAspirant: {
										if (bw::unit_queue_full(unit) || is_adding_on || in_air || faction != Faction::TerranGoldenRepublic) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;

									case UnitId::TerranPhalanxTankMode: {
										if (!bw::all_selected_units_have_addon(UnitId::TerranPalladium) || bw::unit_queue_full(unit)
										||	 faction == Faction::TerranNydusTrinity) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::TerranTarasque: {
										if (!bw::all_selected_units_have_addon(UnitId::TerranPalladium) || bw::unit_queue_full(unit)
										||  faction != Faction::TerranNydusTrinity) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
								}
							} break;
							case UnitId::TerranIronFoundry: {
								switch (current_button->requirement_var) {
									case UnitId::TerranFutureStation:
									case UnitId::TerranBioticBastion: {
										if (!has_addon && !is_training && !is_adding_on) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::TerranClaymore:
									case UnitId::TerranPenumbra: {
										if (!bw::all_selected_units_have_addon(UnitId::TerranFutureStation) || bw::unit_queue_full(unit) || is_adding_on) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::TerranAnticthon:
									case UnitId::TerranAion: {
										if (!bw::all_selected_units_have_addon(UnitId::TerranBioticBastion) || bw::unit_queue_full(unit) || is_adding_on) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
								}
							} break;
							case UnitId::TerranStarpad: {
								switch (current_button->requirement_var) {
									case UnitId::TerranWraith: {
										if (faction == Faction::TerranFuturistFederation) {
											current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::TerranAncile: {
										if (faction == Faction::TerranUpstart) {
											current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::TerranTrojan: {
										if (faction == Faction::TerranHecatonCoalition) {
											current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::TerranQilin: {
										if (faction == Faction::TerranHecatonCoalition) {
											if (!in_air) {
												current_button_state = Button_State::Enabled;
											}
										}
									} break;
								}
							} break;
							case UnitId::TerranStarport: {
								switch (current_button->requirement_var) {
									case UnitId::TerranSalamander:
									case UnitId::TerranSundog: {
										if (!bw::all_selected_units_have_addon(UnitId::TerranRotary) || bw::unit_queue_full(unit)) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::TerranAzazel: {
										if (!bw::all_selected_units_have_addon(UnitId::TerranCommandment) || bw::unit_queue_full(unit)) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::TerranSeraph: {
										if (!bw::all_selected_units_have_addon(UnitId::TerranCommandment) || bw::unit_queue_full(unit)
										||	 faction == Faction::TerranGoldenRepublic) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::TerranLuminary: {
										if (!bw::all_selected_units_have_addon(UnitId::TerranCommandment) || bw::unit_queue_full(unit)
										|| 	 faction != Faction::TerranGoldenRepublic) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
								}
							} break;
							case UnitId::TerranNaniteAssembly: {
								switch (current_button->requirement_var) {
									case UnitId::TerranHadron: {
										if (!has_addon && !is_training && !is_adding_on) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::TerranCentaur:
									case UnitId::TerranMinotaur:
									case UnitId::TerranMagnetar:
									case UnitId::TerranPythean: {
										if (bw::unit_queue_full(unit) || is_adding_on || in_air) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::TerranHippogriff:
									case UnitId::TerranPhobos: {
										if (bw::unit_queue_full(unit) || is_adding_on || in_air) {
											current_button_state = Button_State::Invisible;
										} else if (!bw::all_selected_units_have_addon(UnitId::TerranHadron)) {
											current_button_state = Button_State::Disabled;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
								}
							} break;
						}

						// As of late 2022, we must force train/build icons for extended unit ids to show up using this very dumb method that
						// inlines requirements in yet another position. Not sure how to fix! -Pr0nogo
						switch (unit->id) {
							case UnitId::Omnivale: {
								bool is_omnipresence = bw::get_game_kind() == GameKind::Omnipresence;
								GameModeRace duopoly_state = g_globals->players[unit->player].game_mode_race;
								switch (current_button->requirement_var) {
									case UnitId::TerranMason: {
										if (is_omnipresence || duopoly_state != GameModeRace_ProtossZerg) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::ProtossScribe: {
										if (is_omnipresence || duopoly_state != GameModeRace_ZergTerran) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::ZergDroleth: {
										if (is_omnipresence || duopoly_state != GameModeRace_TerranProtoss) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Invisible;
										}
									} break;
								}
							} break;

							case UnitId::TerranMason:
							case UnitId::TerranTalos: {
								if (unit->main_order_id != OrderId::BuildTerran && unit->main_order_id != OrderId::ConstructingBuilding) {
									switch (current_button->requirement_var) {
										case UnitId::TerranMinistry:
										case UnitId::TerranWatchdog:
										case UnitId::TerranAnchor:
										case UnitId::TerranTreasury:
										case UnitId::TerranAtlas:
										case UnitId::TerranStockade:
										case UnitId::TerranFulcrum:
										case UnitId::TerranStarpad: {
											current_button_state = Button_State::Enabled;
										} break;
										case UnitId::TerranReservoir:
										case UnitId::TerranSentinel:
										case UnitId::TerranDaedala:
										case UnitId::TerranCaptaincy:
										case UnitId::TerranMantle:
										case UnitId::TerranStarport: {
											if (player_has_completed_landed_unit_id(unit->player, UnitId::TerranAtlas)) {
												current_button_state = Button_State::Enabled;
											} else {
												current_button_state = Button_State::Disabled;
											}
										} break;
										case UnitId::TerranDiadem:
										case UnitId::TerranWellbore:
										case UnitId::TerranApothecary:
										case UnitId::TerranIronFoundry:
										case UnitId::TerranNaniteAssembly: {
											if (player_has_completed_landed_unit_id(unit->player, UnitId::TerranDaedala)) {
												current_button_state = Button_State::Enabled;
											} else {
												current_button_state = Button_State::Disabled;
											}
										} break;
										default: {
											current_button_state = Button_State::Invisible;
										} break;
									}
								}
							} break;
							case UnitId::TerranMinistry: {
								switch (current_button->requirement_var) {
									case UnitId::TerranMason: {
										if (unit->status & UnitStatus::GroundedBuilding && !is_adding_on) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::TerranLodestone:
									case UnitId::TerranQuarry:
									case UnitId::TerranAnvil: {
										if (has_addon || is_training || is_adding_on) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
								}
							} break;
							case UnitId::TerranTreasury: {
								switch (current_button->requirement_var) {
									case UnitId::TerranLodestone:
									case UnitId::TerranQuarry:
									case UnitId::TerranAnvil: {
										if (has_addon || is_training || is_adding_on) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Enabled;
										}
									} break;
								}
							} break;
							case UnitId::TerranQuarry: {
								switch (current_button->requirement_var) {
									case UnitId::TerranTalos: {
										if (!unit_ext->values[ValueAddonInactive]) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Invisible;
										}
									} break;
								}
							} break;
							case UnitId::TerranAnvil: {
								switch (current_button->requirement_var) {
									case UnitId::TerranMorningstar: {
										if (!unit_ext->values[ValueAddonInactive] && !unit->building.silo.nuke && !is_training) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Invisible;
										}
									} break;
								}
							} break;
							case UnitId::TerranMantle: {
								switch (current_button->requirement_var) {
									case UnitId::TerranRamesses:
									case UnitId::TerranDurendal:
									case UnitId::TerranCataphract: {
										if (unit->status & UnitStatus::GroundedBuilding) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Invisible;
										}
									} break;
								}
							} break;
							case UnitId::TerranApothecary: {
								switch (current_button->requirement_var) {
									case UnitId::TerranDilettante:
									case UnitId::TerranSevenstep:
									case UnitId::TerranHypnagogue:
									case UnitId::TerranSilvertongue: {
										if (unit->status & UnitStatus::GroundedBuilding) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Invisible;
										}
									} break;
								}
							} break;
							case UnitId::ProtossScribe:
							case UnitId::ProtossPrudence: {
								switch (current_button->requirement_var) {
									case UnitId::ProtossNexus:
									case UnitId::ProtossPylon:
									case UnitId::ProtossAquifer:
									case UnitId::ProtossWarden:
									case UnitId::ProtossEmbassy:
									case UnitId::ProtossCrucible:
									case UnitId::ProtossMatrix:
									case UnitId::ProtossEngram:
									case UnitId::ProtossGateway:
									case UnitId::ProtossLattice: {
										current_button_state = Button_State::Enabled;
									} break;
									case UnitId::ProtossGrandLibrary:
									case UnitId::ProtossRogueGallery:
									case UnitId::ProtossArdentAuthority:
									case UnitId::ProtossStargate: {
										if (TierUnlocked[RaceId::Protoss][Tier2]) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ProtossProstrationStage:
									case UnitId::ProtossUnitaryUnion:
									case UnitId::ProtossArgosy: {
										if (TierUnlocked[RaceId::Protoss][Tier3]) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ProtossAncestralArchives:
									case UnitId::ProtossSyntheticSynod:
									case UnitId::ProtossMonumentOfSin: {
										if (TierUnlocked[RaceId::Protoss][Tier4]) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
								}
							} break;
							case UnitId::ProtossEmbassy: {
								if (current_button->requirement_var == UnitId::ProtossArtisan
								||  current_button->requirement_var == UnitId::ProtossEnvoy
								||  current_button->requirement_var == UnitId::ProtossWitness) {
									current_button_state = Button_State::Enabled;
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossStridentStratum: {
								if (current_button->requirement_var == UnitId::ProtossMonogram) {
									current_button_state = Button_State::Enabled;
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossGateway: {
								switch (current_button->requirement_var) {
									case UnitId::ProtossZealot: {
										if (faction != Faction::ProtossStarbound) {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::ProtossZealotStarbound: {
										if (faction == Faction::ProtossStarbound) {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::ProtossDracadin:
									case UnitId::ProtossEcclesiast:
									case UnitId::ProtossHierophant: {
										current_button_state = Button_State::Enabled;
									} break;
									case UnitId::ProtossLegionnaire: {
										if (faction != Faction::ProtossOrderOfTheSacredSaber && faction != Faction::ProtossStarbound) {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::ProtossAdherent: {
										if (faction == Faction::ProtossOrderOfTheSacredSaber) {
											current_button_state = Button_State::Enabled;
										}
									} break;
									case UnitId::ProtossCicatus: {
										if (faction == Faction::ProtossStarbound) {
											current_button_state = Button_State::Enabled;
										}
									} break;
								}
							} break;
							case UnitId::ProtossGrandLibrary: {
								if (current_button->requirement_var == UnitId::ProtossAmaranth
								||  current_button->requirement_var == UnitId::ProtossHerald
								||  current_button->requirement_var == UnitId::ProtossAtreus
								||  current_button->requirement_var == UnitId::ProtossCantavis) {
									current_button_state = Button_State::Enabled;
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossProstrationStage: {
								if (current_button->requirement_var == UnitId::ProtossPariah
								||  current_button->requirement_var == UnitId::ProtossPotentate
								||  current_button->requirement_var == UnitId::ProtossSycophant
								||  current_button->requirement_var == UnitId::ProtossMindTyrant) {
									current_button_state = Button_State::Enabled;
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossAncestralArchives: {
								if (current_button->requirement_var == UnitId::ProtossArchon
								||  current_button->requirement_var == UnitId::ProtossOptecton
								||  current_button->requirement_var == UnitId::ProtossPatriarch) {
									current_button_state = Button_State::Enabled;
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossHallOfHeretics: {
								if (current_button->requirement_var == UnitId::ProtossApostate
								||  current_button->requirement_var == UnitId::ProtossAugur) {
									current_button_state = Button_State::Enabled;
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossLattice: {
								if (current_button->requirement_var == UnitId::ProtossManifold
								||  current_button->requirement_var == UnitId::ProtossSimulacrum
								||	current_button->requirement_var == UnitId::ProtossVassal
								||	current_button->requirement_var == UnitId::ProtossGolem
								||  current_button->requirement_var == UnitId::ProtossIdol) {
									current_button_state = Button_State::Enabled;
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossArdentAuthority: {
								if (current_button->requirement_var == UnitId::ProtossServitor
								||  current_button->requirement_var == UnitId::ProtossPositron
								||  current_button->requirement_var == UnitId::ProtossAnalogue
								||	current_button->requirement_var == UnitId::ProtossAccantor
								||  current_button->requirement_var == UnitId::ProtossArchitect) {
									current_button_state = Button_State::Enabled;
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossAuxiliaryOasis: {
								if (current_button->requirement_var == UnitId::ProtossMainstay
								||  current_button->requirement_var == UnitId::ProtossMaven
								||  current_button->requirement_var == UnitId::ProtossCloudform
								||	current_button->requirement_var == UnitId::ProtossVibrance
								||  current_button->requirement_var == UnitId::ProtossOscilliant) {
									current_button_state = Button_State::Enabled;
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossStargate: {
								if (current_button->requirement_var == UnitId::ProtossAurora
								||  current_button->requirement_var == UnitId::ProtossLanifect
								||  current_button->requirement_var == UnitId::ProtossPanoptus
								||	current_button->requirement_var == UnitId::ProtossExemplar
								||  current_button->requirement_var == UnitId::ProtossGladius) {
									current_button_state = Button_State::Enabled;
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossArgosy: {
								if (current_button->requirement_var == UnitId::ProtossMagister
								||  current_button->requirement_var == UnitId::ProtossEpigraph
								||  current_button->requirement_var == UnitId::ProtossDidact
								||	current_button->requirement_var == UnitId::ProtossEmpyrean
								||  current_button->requirement_var == UnitId::ProtossSolarion) {
									current_button_state = Button_State::Enabled;
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossMonumentOfSin: {
								if (current_button->requirement_var == UnitId::ProtossStarSovereign
								||  current_button->requirement_var == UnitId::ProtossAnthelion) {
									current_button_state = Button_State::Enabled;
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergLarva: {
								switch (current_button->requirement_var) {
									case UnitId::ZergDroleth:
									case UnitId::ZergOvileth: {
										current_button_state = Button_State::Enabled;
									} break;
									case UnitId::ZergOthstoleth:
									case UnitId::ZergAgrileth: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergOthstolOviform)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergZethrokor:
									case UnitId::ZergQuazilisk: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergQuazethPool)
										||  player_has_unit_id(unit->player, UnitId::ZergVornathPond)
										||  player_has_unit_id(unit->player, UnitId::ZergKalkiirLake)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergNathrokor: {
										if (faction != Faction::ZergZorthosTendril) {	
											if (player_has_completed_unit_id(unit->player, UnitId::ZergVornathPond)
											||  player_has_unit_id(unit->player, UnitId::ZergKalkiirLake)) {
												current_button_state = Button_State::Enabled;
											} else {
												current_button_state = Button_State::Disabled;
											}
										} else {
												current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::ZergNathrelisk: {
										if (faction == Faction::ZergZorthosTendril) {	
											if ((player_has_completed_unit_id(unit->player, UnitId::ZergVornathPond)
											||  player_has_unit_id(unit->player, UnitId::ZergKalkiirLake)) && faction == Faction::ZergZorthosTendril) {
												current_button_state = Button_State::Enabled;
											} else {
												current_button_state = Button_State::Disabled;
											}
										} else {
												current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::ZergHydralisk: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergHydrithDen)
										||  player_has_unit_id(unit->player, UnitId::ZergSkibactScab)
										||  player_has_unit_id(unit->player, UnitId::ZergTethvithSwamp)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergIzirokor: {
										if (faction != Faction::ZergZorthosTendril) {
											if (player_has_completed_unit_id(unit->player, UnitId::ZergHydrithDen)
												|| player_has_unit_id(unit->player, UnitId::ZergSkibactScab)
												|| player_has_unit_id(unit->player, UnitId::ZergTethvithSwamp)) {
												current_button_state = Button_State::Enabled;
											} else {
												current_button_state = Button_State::Disabled;
											}
										} else {
												current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::ZergIziralisk: {
										if (faction == Faction::ZergZorthosTendril) {
											if (player_has_completed_unit_id(unit->player, UnitId::ZergHydrithDen)
												|| player_has_unit_id(unit->player, UnitId::ZergSkibactScab)
												|| player_has_unit_id(unit->player, UnitId::ZergTethvithSwamp)) {
												current_button_state = Button_State::Enabled;
											} else {
												current_button_state = Button_State::Disabled;
											}
										} else {
												current_button_state = Button_State::Invisible;
										}
									} break;
									case UnitId::ZergSkithrokor: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergSkibactScab)
										||  player_has_unit_id(unit->player, UnitId::ZergTethvithSwamp)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergZoryusthaleth:
									case UnitId::ZergLakizilisk:
									case UnitId::ZergGorgrokor: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergZorkizShroud)
										||  player_has_unit_id(unit->player, UnitId::ZergUltravCavern)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergMutalisk:
									case UnitId::ZergProtathalor:
									case UnitId::ZergCikralisk: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergMutathSpire)
										||  player_has_unit_id(unit->player, UnitId::ZergTosvrolHaunt)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergKagralisk:
									case UnitId::ZergEvigrilisk:
									case UnitId::ZergAlmaksalisk: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergAlkagIteth)
										||  player_has_unit_id(unit->player, UnitId::ZergChokralChazsal)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergKonvilisk:
									case UnitId::ZergVilgorokor:
									case UnitId::ZergMatraleth: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergMatravilNest)
										||  player_has_unit_id(unit->player, UnitId::ZergAlmakisAntre)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergMortothrokor:
									case UnitId::ZergAxitrilisk:
									case UnitId::ZergSasyrvoleth: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergAxtothAxis)
										||  player_has_unit_id(unit->player, UnitId::ZergBegrozithBarrow)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergKeskathalor:
									case UnitId::ZergGeszithalor: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergGeszkathGrotto)
										||  player_has_unit_id(unit->player, UnitId::ZergAlkajChasm)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergIsthrathaleth: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergElthisthMound)
										||  player_has_unit_id(unit->player, UnitId::ZergRakzogthRoost)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									default: {
										current_button_state = Button_State::Invisible;
									} break;
								}
							} break;
							case UnitId::ZergOcsalLarva: {
								switch (current_button->requirement_var) {
									case UnitId::ZergGosvileth: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergIrolIris)
										||	player_has_unit_id(unit->player, UnitId::ZergOthstolOviform)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergIroleth: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergIrolIris)
										||  player_has_unit_id(unit->player, UnitId::ZergOthstolOviform)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergVorvrokor: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergVornathPond)
										||  player_has_unit_id(unit->player, UnitId::ZergKalkiirLake)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergLiiralisk:
									case UnitId::ZergKalkalisk: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergKalkiirLake)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergBactalisk: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergSkibactScab)
										|| player_has_unit_id(unit->player, UnitId::ZergTethvithSwamp)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
								} break;
									case UnitId::ZergTethzorokor:
									case UnitId::ZergVithrilisk: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergTethvithSwamp)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergUltrokor:
									case UnitId::ZergZobriolisk:
									case UnitId::ZergZarcavrokor: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergUltravCavern)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergGhitorokor:
									case UnitId::ZergSovroleth:
									case UnitId::ZergTosgrilisk: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergTosvrolHaunt)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergCherbrathalor:
									case UnitId::ZergOkrimnilisk:
									case UnitId::ZergAlgoztalisk: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergChokralChazsal)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergTetcaerokor:
									case UnitId::ZergIsthrathaleth:
									case UnitId::ZergAkistrokor: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergAlmakisAntre)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergKethrostalor:
									case UnitId::ZergAlkajelisk: {
										if (player_has_completed_unit_id(unit->player, UnitId::ZergGeszkathGrotto)) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} break;
									default: {
										current_button_state = Button_State::Invisible;
									} break;
								}
							} break;
							case UnitId::ZergZethrokor: {
								if (current_button->requirement_var == UnitId::ZergVorvrokor) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergVornathPond)
									||  player_has_unit_id(unit->player, UnitId::ZergKalkiirLake)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								} break;
							} break;
							case UnitId::ZergMutalisk: {
								if (current_button->requirement_var == UnitId::ZergTosgrilisk) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergTosvrolHaunt)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergCikralisk: {
								if (!is_burrowed && current_button->requirement_var == UnitId::ZergGhitorokor) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergTosvrolHaunt)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergIzirokor:
							case UnitId::ZergIziralisk: {
								if (!is_burrowed && current_button->requirement_var == UnitId::ZergTethzorokor) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergTethvithSwamp)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergKagralisk: {
								if (current_button->requirement_var == UnitId::ZergCherbrathalor) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergChokralChazsal)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergEvigrilisk: {
								if (current_button->requirement_var == UnitId::ZergOkrimnilisk) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergChokralChazsal)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergAlmaksalisk: {
								if (current_button->requirement_var == UnitId::ZergAlgoztalisk) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergChokralChazsal)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergLakizilisk: {
								if (!is_burrowed && current_button->requirement_var == UnitId::ZergZobriolisk) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergUltravCavern)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergGorgrokor: {
								if (current_button->requirement_var == UnitId::ZergZarcavrokor) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergUltravCavern)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergKonvilisk: {
								if (!is_burrowed && current_button->requirement_var == UnitId::ZergTetcaerokor) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergAlmakisAntre)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergSurkithCesiant: {
								if (current_button->requirement_var == UnitId::ZergSkortrithCesiant) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergIrolIris)
									||  player_has_unit_id(unit->player, UnitId::ZergOthstolOviform)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergSpraithCesiant: {
								if (current_button->requirement_var == UnitId::ZergSpirtithCesiant) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergIrolIris)
									||  player_has_unit_id(unit->player, UnitId::ZergOthstolOviform)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergExcisant: {
								if (current_button->requirement_var == UnitId::ZergEzigrisant) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergOthstolOviform)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergQuazethPool: {
								if (current_button->requirement_var == UnitId::ZergVornathPond) {
									current_button_state = Button_State::Enabled;
								} else if (current_button->requirement_var == UnitId::ZergKalkiirLake) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergIrolIris)
									||  player_has_unit_id(unit->player, UnitId::ZergOthstolOviform)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergHydrithDen: {
								if (current_button->requirement_var == UnitId::ZergSkibactScab) {
									current_button_state = Button_State::Enabled;
								} else if (current_button->requirement_var == UnitId::ZergTethvithSwamp) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergIrolIris)
									||  player_has_unit_id(unit->player, UnitId::ZergOthstolOviform)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergAlkagIteth: {
								if (current_button->requirement_var == UnitId::ZergChokralChazsal) {
									if (player_has_completed_unit_id(unit->player, UnitId::ZergOthstolOviform)) {
										current_button_state = Button_State::Enabled;
									} else {
										current_button_state = Button_State::Disabled;
									}
								} else {
									current_button_state = Button_State::Invisible;
								}
							} break;
						}
					}

					if (current_button->requirement_func == (ReqFunc*)ButtonRequirements::Spell_Researched) {
						// Conditional hiding of custom ability icons
						if (current_button->icon_id == IconId::MasonFusionCutter
						&&  unit->main_order_id == OrderId::ConstructingBuilding) { // Talos Fusion Sealers
							current_button_state = Button_State::Invisible;
						}

						/* // Enable this when buttonsets.cpp doesn't have the selection order bug. -Pr0nogo
						if (current_button->requirement_var == TechId::Stimpacks) {
							if (unit_ext->timers[TimerStimpackDelay] > 1) {
								current_button_state = Button_State::Disabled;
							} else {
							//	current_button_state = Button_State::Enabled;
							}
						}
						*/

						// Passive icons (yes, it's quite ugly)
						if (current_button->requirement_var == TechId::Healing && current_button->icon_id != IconId::Heal) {
							current_button_state = Button_State::Disabled;
							if (bw::client_selection_unit_id_count(unit->id) != g_client_selection_count) {
								current_button_state = Button_State::Invisible;
							}
							switch (unit->id) {
								// GLOBAL \\
								
								case UnitId::TerranEidolon:
								case UnitId::TerranWraith:
								case UnitId::TerranSundog: {
									if (current_button->icon_id == IconId::SensorArray) { // Proximity Cloak
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossWitness: {
									if (current_button->icon_id == IconId::DidactPassiveShroudOfAdun) { // Steady Shroud
										current_button_state = Button_State::Enabled;
									}
								} break;

								// HEROES \\

								case UnitId::Goska: {
									if (current_button->icon_id == IconId::U238_Shells		// Fossilizer Rounds
									||  current_button->icon_id == IconId::SensorArray) {	// Proximity Cloak
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::InfestedKerriganExt: {
									if (current_button->icon_id == IconId::Ensnare) { // Ensnaring Brood
										current_button_state = Button_State::Enabled;
									}
								} break;

								// FACTIONAL \\

								case UnitId::TerranCohort: {
									if (current_button->icon_id == IconId::TerranInfantryArmor) { // Servile Vigor
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranSouthpaw: {
									if (current_button->icon_id == IconId::StewardPulseCannon) { // Offset: Linear
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranAspirant: {
									if (current_button->icon_id == IconId::ScarabDamage) { // Criticality
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranTarasque: {
									if (current_button->icon_id == IconId::GeszithalorAcidSpore) { // Fighting Flight
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranWendigo: {
									if (current_button->icon_id == IconId::ParticleBeamHarvest) { // Shattering Winds
										current_button_state = Button_State::Enabled;
									}
								} break;

								// TERRAN \\

								case UnitId::TerranTalos: {
									if (current_button->icon_id == IconId::MasonFusionCutter) { // Fusion Sealers
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranHarakan:
								case UnitId::Ackmed: {
									if (current_button->icon_id == IconId::FlyerAttack) { // Apollyoid Adhesive
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranCyprian: {
									if (current_button->icon_id == IconId::AtreusDauntlessDisruptionCannon) { // Kinetic Penetration
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranShamanSupportMode:
								case UnitId::TerranShamanVisionaryMode: {
									if (current_button->icon_id == IconId::Restoration) { // Nanite Field
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranSavant: {
									if (current_button->icon_id == IconId::DidactPhaseDisruptorCannon && bw::get_generic_rust_value(unit, RustId::ToggleSavantOnhit) == 0) { // Entropy Gauntlets
										current_button_state = Button_State::Enabled;
									} else if (current_button->icon_id == IconId::IonThrusters && bw::get_generic_rust_value(unit, RustId::ToggleSavantOnhit) > 0) { // Power Siphon
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranMadcap: {
									if (current_button->icon_id == IconId::GoliathTwinAutocannons) { // Maverick Feeders
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranHeracles: {
									if (current_button->icon_id == IconId::FusionCutterHarvest) { // Kelvin Munitions
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranApostle: {
									if (current_button->icon_id == IconId::RechargeShields	// Lazarus Agent
									||  current_button->icon_id == IconId::Reclamation) {	// Reclamation
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranAuriel: {
									if (current_button->icon_id == IconId::FlyerAttack) { // Incineration
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranHotshot:
								case UnitId::TerranSiren:
								case UnitId::TerranCataphract: {
									if (current_button->icon_id == IconId::AtreusDauntlessDisruptionCannon		// Prolonged Arms
									||  current_button->icon_id == IconId::PsiAssault) {						// Offset: Cone
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranStriga: {
									if (current_button->icon_id == IconId::DracadinPhaseDisruptor	// Projection
									||	current_button->icon_id == IconId::PsiAssault				// Fleeting Focus
									||	current_button->icon_id == IconId::FlyerAttack) {			// Apollyoid Adhesive
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranOlympian: {
									if (current_button->icon_id == IconId::OcularImplants) { // Dishonorable Discharge
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranAutocrat: {
									if (current_button->icon_id == IconId::RechargeShields					// Reclamation Fields
									||  current_button->icon_id == IconId::AtreusDauntlessDisruptionCannon	// Kinetic Penetration
									||  current_button->icon_id == IconId::Reclamation) {					// Reclamation
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranSevenstep: {
									if (current_button->icon_id == IconId::DefensiveMatrix) { // Curtain Call
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranLobotomyMine: {
									if (current_button->icon_id == IconId::UseSpiderMines) { // Dazing Detonation
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranCyclops:
								case UnitId::TerranCyclopsFuturist: {
									if (current_button->icon_id == IconId::VultureFragmentationGrenade) { // Recursion Rounds
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranBlackjack: {
									if (current_button->icon_id == IconId::ProtossAirPlating) { // Wear and Tear
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranMatadorTankMode: {
									if (current_button->icon_id == IconId::FlyerAttack) { // Apollyoid Adhesive
										current_button_state = Button_State::Disabled;
									}
								} break;
								case UnitId::TerranMatadorCinderMode: {
									if (current_button->icon_id == IconId::FlyerAttack) { // Apollyoid Adhesive
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranPhalanxTankMode: {
									if (current_button->icon_id == IconId::TerranVehiclePlating) { // Babylon Plating
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranPhalanxSiegeMode: {
									if (current_button->icon_id == IconId::TerranVehiclePlating) { // Babylon Plating
										current_button_state = Button_State::Disabled;
									}
								} break;
								case UnitId::TerranMadrigalImperiosoMode: {
									if (current_button->icon_id == IconId::FusionCutterHarvest) { // Dirge
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranMadrigalFuriaMode: {
									if (current_button->icon_id == IconId::GeminiMissiles) { // Furia Salvo
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranRamesses: {
									if (current_button->icon_id == IconId::AtreusDauntlessDisruptionCannon) { // Kinetic Penetration
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranDurendal: {
									if (current_button->icon_id == IconId::TerranShipWeapons // Forward March
									||  current_button->icon_id == IconId::FlyerAttack) {	 // Apollyoid Adhesive
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranPaladin: {
									if (current_button->icon_id == IconId::StewardPulseCannon) { // Offset: Linear
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranCuirassTankMode: {
									if (current_button->icon_id == IconId::MoebiusReactor) { // Stonewall Protocol
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranPazuzu: {
									if (current_button->icon_id == IconId::ScarabDamage) { // Deconstruction
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranPenumbra: {
									if (current_button->icon_id == IconId::AtreusDauntlessDisruptionCannon	// Kinetic Penetration
									||	current_button->icon_id == IconId::LegEnhancements)	{				// Phantasmal Passage
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranClaymore: {
									if (current_button->icon_id == IconId::AtreusDauntlessDisruptionCannon  // Kinetic Penetration
									||	current_button->icon_id == IconId::FusionCutterHarvest) {			// Kelvin Munitions
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranAnticthon: {
									if (current_button->icon_id == IconId::RilirokorToxicSpores // Counter-Proposal
									||	current_button->icon_id == IconId::ApialSensors) {		// Multi-Attack V
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranAion: {
									if (current_button->icon_id == IconId::RechargeShields		// Reclamation Fields
									||	current_button->icon_id == IconId::StewardPulseCannon	// Offset: Linear
									||	current_button->icon_id == IconId::Reclamation) {		// Reclamation
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranTrojan: {
									if (current_button->icon_id == IconId::Land) { // Hotdrop
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranQilin: {
									if (current_button->icon_id == IconId::ColossusReactor) { // Nanite Chambers
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranAncile: {
									if (current_button->icon_id == IconId::DefensiveMatrix) { // Providence
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranGorgon: {
									if (current_button->icon_id == IconId::ArgusJewel) { // Petrification
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranWyvern: {
									if (current_button->icon_id == IconId::AfterburnersOnUnused				  // Reverse Thrust
									||	current_button->icon_id == IconId::AtreusDauntlessDisruptionCannon)	{ // Kinetic Penetration
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranValkyrie: {
									if (current_button->icon_id == IconId::PsiAssault) { // Offset: Cone
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranSalamander: {
									if (current_button->icon_id == IconId::HarakanFlamethrower	 // Incendiary Payload
									||	current_button->icon_id == IconId::StewardPulseCannon) { // Offset: Linear
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranAzazel: {
									if (current_button->icon_id == IconId::ApialSensors) { // Sublime Shepherd
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranSeraph: {
									if (current_button->icon_id == IconId::OcularImplants) { // Observance
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranPhobos: {
									if (current_button->icon_id == IconId::KhaydarinCore) { // Argent Capacitors
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranCentaur: {
									if (current_button->icon_id == IconId::GeminiMissiles) { // Sunchaser Salvo
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranMinotaur: {
									if (current_button->icon_id == IconId::TerranShipPlating    // Fore Castle
									||  current_button->icon_id == IconId::SingularityCharge) { // Monoceros Targeting
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranMagnetar: {
									if (current_button->icon_id == IconId::ArchonPsionicShockwave				// Law of Attraction
									||  current_button->icon_id == IconId::AtreusDauntlessDisruptionCannon) {	// Kinetic Penetration
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranHippogriff: {
									if (current_button->icon_id == IconId::AtreusDauntlessDisruptionCannon) { // Kinetic Penetration
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranAnvil: {
									if (current_button->icon_id == IconId::RadioactiveUnused2) { // Scorched Earth
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranSentinel: {
									if (current_button->icon_id == IconId::AtreusDauntlessDisruptionCannon) { // Kinetic Penetration
										if (unit->status & UnitStatus::GroundedBuilding) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Invisible;
										}
									}
								} break;
								case UnitId::TerranDiadem: {
									if (current_button->icon_id == IconId::OpticalFlare					// Ion Field
									|| current_button->icon_id == IconId::DidactPassiveShroudOfAdun) {	// Artillery targeting
										if (unit->status & UnitStatus::GroundedBuilding) {
											current_button_state = Button_State::Enabled;
										} else {
											current_button_state = Button_State::Invisible;
										}
									}
								} break;
								case UnitId::TerranStockade:
								case UnitId::TerranCaptaincy:
								case UnitId::TerranFulcrum:
								case UnitId::TerranIronFoundry:
								case UnitId::TerranStarport: {
									bool is_adding_on = units_dat::base_property[unit->build_queue[unit->build_queue_slot]] & UnitProperty::Addon;
									if (!unit->building.addon && !is_adding_on != OrderId::BuildAddon && !(unit->status & UnitStatus::InAir) && !bw::unit_queue_full(unit)) {
										current_button_state = Button_State::Disabled; // Still shows even when they are building an addon!
									} else {
										current_button_state = Button_State::Invisible;
									}
								} break;

								// PROTOSS \\

								case UnitId::ProtossArtisan: {
									if (current_button->icon_id == IconId::KhaydarinCore) { // Radiant Rifts
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossClarion: {
									if (current_button->icon_id == IconId::RechargeShields) { // Phase Link
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossZealot: {
									if (current_button->icon_id == IconId::PlasmaShields) { // Imperious March
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossEcclesiast: {
									if (current_button->icon_id == IconId::ProtossGroundWeapons	// Astral Blessing
									||	current_button->icon_id == IconId::LegEnhancements) {	// Vengeance
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossLegionnaire: {
									if (current_button->icon_id == IconId::PsiAssault) { // Blade Vortex
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossAdherent: {
									if (current_button->icon_id == IconId::DidactPhaseDisruptorCannon	// Eternal Service
									||	current_button->icon_id == IconId::LegEnhancements) {			// Deathless Procession
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossHierophant:
								case UnitId::ProtossEpigraph: {
									if (current_button->icon_id == IconId::PhotonCannonWeapon			// Signify: Cowardice
									||  current_button->icon_id == IconId::DidactPhaseDisruptorCannon	// Signify: Malice
									||  current_button->icon_id == IconId::ProtossGroundWeapons) {		// Ascension
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossCicatus:
								case UnitId::ProtossVagrant: {
									if (current_button->icon_id == IconId::MindControlUnused) { // Nihilism
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossAmaranth: {
									if (current_button->icon_id == IconId::DidactPhaseDisruptorCannon) { // Eternal Service
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossHerald: {
									if (current_button->icon_id == IconId::RechargeShields	// Triumphant Rise
									||	current_button->icon_id == IconId::KhaydarinCore) {	// Transcendent Fall
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossCabalist: {
									if (current_button->icon_id == IconId::DidactPassiveShroudOfAdun	// Steady Shroud
									||	current_button->icon_id == IconId::ProtossGroundWeapons) {		// Silent Strides
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossBarghest: {
									if (current_button->icon_id == IconId::DidactPassiveShroudOfAdun	// Steady Shroud
									||	current_button->icon_id == IconId::ApialSensors) {				// Repentant Chorus
										current_button_state = Button_State::Enabled;
									}
									if (current_button->icon_id == IconId::ApialSensors) {
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossCharlatan: {
									if (current_button->icon_id == IconId::AntimatterMissiles			// Interception
									||  current_button->icon_id == IconId::DemiurgePassiveWorship) {	// Leading the Blind
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossPariah: {
									if (current_button->icon_id == IconId::DracadinPhaseDisruptor	// Projection
									||	current_button->icon_id == IconId::PsiAssault) {			// Heretical Bedlam
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossPotentate: {
									if (current_button->icon_id == IconId::RechargeShields) { // Transcendent Tide
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossSycophant: {
									if (current_button->icon_id == IconId::RechargeShields) { // Idolatry
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossArchon: {
									if (current_button->icon_id == IconId::RechargeShields	  // Astral Aegis
									||	current_button->icon_id == IconId::LegEnhancements)	{ // Vengeance
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossOptecton: {
									if (current_button->icon_id == IconId::AntimatterMissiles // Interception
									||	current_button->icon_id == IconId::LegEnhancements) { // Titanic Stride
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossPatriarch: {
									if (current_button->icon_id == IconId::ProtossGroundWeapons  // Astral Rule
									||	current_button->icon_id == IconId::AntimatterMissiles) { // Interception
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossSimulacrum: {
									if (current_button->icon_id == IconId::ApialSensors) { // Autovitality
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossZealotStarbound:
								case UnitId::ProtossVassal:
								case UnitId::ProtossServitor: {
									if (current_button->icon_id == IconId::LegEnhancements) { // Vengeance
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossManifold: {
									if (current_button->icon_id == IconId::DidactPhaseDisruptorCannon) { // Concurring Chorus
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossGolem: {
									if (current_button->icon_id == IconId::PlasmaShields) { // Tranquility Array
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossAtreus:
								case UnitId::ProtossIdol:
								case UnitId::ProtossAccantor:
								case UnitId::ProtossEngram: {
									if (current_button->icon_id == IconId::AntimatterMissiles) { // Interception
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossPositron: {
									if (current_button->icon_id == IconId::ScarabDamage) { // Particle Annihilation
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossAnalogue: {
									if (current_button->icon_id == IconId::DidactPhaseDisruptorCannon) { // Empathy Core
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossArchitect: {
									if (current_button->icon_id == IconId::AntimatterMissiles) { // Reactive Payload
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossVibrance: {
									if (current_button->icon_id == IconId::DidactPassiveShroudOfAdun	// Steady Shroud
									||  current_button->icon_id == IconId::ProtossGroundWeapons) {		// Astral Blessing
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossDemiurge: {
									if (current_button->icon_id == IconId::DemiurgePassiveWorship) { // Worship
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossEmpress: {
									if (current_button->icon_id == IconId::ParticleBeamHarvest) { // Anodyne Annex
										current_button_state = Button_State::Enabled;
									}
								}
								break;
								case UnitId::ProtossAurora: {
									if (current_button->icon_id == IconId::MojoScout) { // Last Orders
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossGladius: {
									if (current_button->icon_id == IconId::DracadinPhaseDisruptor) { // Binding Plasma
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossMagister: {
									if (current_button->icon_id == IconId::DidactPhaseDisruptorCannon) { // Stellar Enforcement
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossDidact: {
									if (current_button->icon_id == IconId::DidactPassiveShroudOfAdun) { // Shroud of Adun
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossEmpyrean: {
									if (current_button->icon_id == IconId::StewardPulseCannon) { // Heavenly Passage
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossSolarion: {
									if (current_button->icon_id == IconId::Interceptor) { // Robotic Fleet
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossSteward: {
									if (current_button->icon_id == IconId::AntimatterMissiles) { // Evasive Maneuvers
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossStarSovereign: {
									if (current_button->icon_id == IconId::AntimatterMissiles) { // Intervention
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossAnthelion: {
									if (current_button->icon_id == IconId::DimensionalRecall		// False Light
									||	current_button->icon_id == IconId::ArgusJewel) {         	// Gravity Well
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossMatrix: {
									if (current_button->icon_id == IconId::DidactPhaseDisruptorCannon) { // Astral Tether
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossCrucible: {
									if (current_button->icon_id == IconId::KhaydarinCore) { // Khaydarin Charge
										current_button_state = Button_State::Enabled;
									}
								} break;

								// ZERG \\
								
								case UnitId::ZergGosvileth: {
									if (current_button->icon_id == IconId::VentralSacs) { // Plethoral Gestation
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergNathrokor: {
									if (current_button->icon_id == IconId::AdrenalGlands	// Adrenal Frenzy
									||	current_button->icon_id == IconId::RoarUnused) {	// Hidebreaker
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergNathrelisk: {
									if (current_button->icon_id == IconId::GeszithalorAcidSpore) { // Mitosal Fog
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergVorvrokor: {
									if (current_button->icon_id == IconId::Ensnare			// Captivating Claws
									||	current_button->icon_id == IconId::RoarUnused) {	// Hidebreaker
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergLiiralisk:
								case UnitId::ZergMutalisk:
								case UnitId::ZergSpirtithCesiant: {
									if (current_button->icon_id == IconId::GeszithalorAcidSpore) { // Lithe Organelle
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergKalkalisk: {
									if (current_button->icon_id == IconId::GeszithalorAcidSpore) { // Ablative Spores
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergGhitorokor: {
									if (current_button->icon_id == IconId::SuicideScourge			// Relentless Aggression
									||  current_button->icon_id == IconId::UltrokorKaiserBlades) {  // Saber Siphon
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergSovroleth: {
									if (current_button->icon_id == IconId::Ensnare								// Writhing Web
									||	current_button->icon_id == IconId::SunkenColonySubterraneanTentacle) {	// Corpse Cocoon
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergTosgrilisk:
								case UnitId::ZergIzirokor:
								case UnitId::ZergCherbrathalor: {
									if (current_button->icon_id == IconId::GeszithalorAcidSpore) { // Crushing Tide
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergMortothrokor: {
									if (current_button->icon_id == IconId::PneumatizedCaparace) { // Whetted Appetite
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergAxitrilisk:
								case UnitId::ZergAxitriliskWeaponless: {
									if (current_button->icon_id == IconId::RoarUnused) { // Perpetual Wind
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergSasyrvoleth: {
									if (current_button->icon_id == IconId::GeszithalorAcidSpore) { // Lucilleri Pheremones
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergIziralisk: {
									if (current_button->icon_id == IconId::VenomUnused) {			// Corrosive Touch
										current_button_state = Button_State::Enabled;
										}
								} break;
								case UnitId::ZergSkithrokor: {
									if (current_button->icon_id == IconId::GeszithalorAcidSpore) { // Ablative Return
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergBactalisk:
								case UnitId::ZergAlkajelisk: {
									if (current_button->icon_id == IconId::GeszithalorAcidSpore) { // Caustic Fission
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergTethzorokor: {
									if (current_button->icon_id == IconId::UltrokorKaiserBlades) { // Saber Siphon
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergVithrilisk: {
									if (current_button->icon_id == IconId::VenomUnused				// Corrosive Touch
									||  current_button->icon_id == IconId::SuicideInfestedTerran) { // Critical Mass
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergEvigrilisk: {
									if (current_button->icon_id == IconId::RilirokorToxicSpores) { // Lashing Limbs
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergOkrimnilisk: {
									if (current_button->icon_id == IconId::Antennae) { // Endless Limbs
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergAlgoztalisk: {
									if (current_button->icon_id == IconId::ChitinousPlating) { // Escape Velocity
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergGeszithalor: {
									if (current_button->icon_id == IconId::ZergFlyerAttacks) { // Searing Acid
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergKeskathalor: {
									if (current_button->icon_id == IconId::VenomUnused) { // Caustic Discharge
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergZoryusthaleth: {
									if (current_button->icon_id == IconId::Rilirokor			// Mitosis
									||	current_button->icon_id == IconId::MetabolicBoost) {	// Swarming Omen
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergRilirokor:
								case UnitId::ZergRililisk: {
									if (current_button->icon_id == IconId::RoarUnused) { // Death Cycle
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergLakizilisk: {
									bool is_mixed = false;
									for (u32 i = 0; i < g_client_selection_count; ++i) {
										Unit* selected = g_client_selection_group->unit[i];
										if (selected->id != unit->id) {
											is_mixed = true;
										}
									}
									if (current_button->icon_id == IconId::Attack) { // Burrowed attacker
										if (is_mixed) {
											current_button_state = Button_State::Enabled;
										} else if (unit->status & UnitStatus::Burrowed && unit->main_order_id != OrderId::Unburrow) {
											current_button_state = Button_State::Invisible;
										} else {
											current_button_state = Button_State::Disabled;
										}
									} else if (current_button->icon_id == IconId::LakiziliskSubterraneanSpines) { // Seismic Spines
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergKonvilisk: {
									if (current_button->icon_id == IconId::SunkenColonySubterraneanTentacle // Fertile Spines
									||	current_button->icon_id == IconId::GeszithalorAcidSpore) {			// Crushing Tide
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergVilgorokor: {
									if (current_button->icon_id == IconId::Dark_Swarm				// Reconstitution
									||  current_button->icon_id == IconId::Radioactive_Zerg_Egg) {	// Autophagy
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergVilgoleth: {
									if (current_button->icon_id == IconId::PneumatizedCaparace)	{ // Corpse Autophagy
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergMatraleth: {
									if (current_button->icon_id == IconId::Ensnare) { // Ensnaring Brood
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergUltrokor: {
									if (current_button->icon_id == IconId::UltrokorKaiserBlades // Saber Siphon
									||  current_button->icon_id == IconId::AnabolicSynthesis) {	// Kaiser Rampage
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergZarcavrokor: {
									if (current_button->icon_id == IconId::RoarUnused) { // Fan of Teeth
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergTetcaerokor: {
									if (current_button->icon_id == IconId::RoarUnused) { // Virile Coating
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergAkistrokor: {
									if (current_button->icon_id == IconId::Ensnare				// Captivating Claws
									||	current_button->icon_id == IconId::SuicideScourge) {	// Devouring Instinct
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergHachirosk:
								case UnitId::ZergLarvoskCesiant: {
									if (current_button->icon_id == IconId::MetasynapticNode) { // Larval Gestation
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergCaphrolosk:
								case UnitId::ZergGathtelosk:
								case UnitId::ZergOcsalCesiant: {
									if (current_button->icon_id == IconId::SuicideScourge) { // Ocsal Gestation / Swarmhold
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergSkortrithCesiant: {
									if (current_button->icon_id == IconId::SunkenColonySubterraneanTentacle) { // Tendril Amitosis
										current_button_state = Button_State::Enabled;
									}
								} break;
							}

							// Passive icons: END \\

						}
					}

					// Attack overrides
					if (current_button->action_func == (ActFunc*)ButtonActions::Attack_Building) {
						if (unit_id_is_ground_garrison(unit->id)) {
							u32 loaded = 0;
							for (u32 i = 0; i < 8; ++i) {								
								Unit* bunker_unit = unit->get_loaded_unit(i);
								if (bunker_unit && bw::unit_is_ranged_attacker(bunker_unit)) {
									loaded++;
								}
							}
							if (loaded) {
								current_button_state = Button_State::Enabled;
							} else {
								current_button_state = Button_State::Disabled;
							}
							current_button_state = Button_State::Invisible; // Hiding the button for now, since it does nothing.
						}
					}

					// Form switch, mode change, toggles, and related overrides
					if (current_button->action_func == (ActFunc*)ButtonActions::Use_Technology) {
						if (unit->id != UnitId::ProtossMindTyrant) {
							for (s32 i = 0; i < generictoggles; i++) {
								if (current_button->action_var == toggle_abilities[i]) {
									if (bw::get_generic_rust_value(unit, toggle_values[i]) == 1) {
										current_button->icon_id = button_toggle_off[i];
										current_button->requirement_string_id = string_toggle_off[i];
										current_button->action_string_id = string_disabled_off[i];
									} else {
										current_button->icon_id = button_toggle_on[i];
										current_button->requirement_string_id = string_toggle_on[i];
										current_button->action_string_id = string_disabled_on[i];
									}
								}
							}
						}

						if (unit->id == UnitId::TerranSavant) {
							if (unit_ext->timers[TimerSavantFormSwitchCooldown] > 1) {
								current_button_state = Button_State::Disabled;
							} else {
								current_button_state = Button_State::Enabled;
							}
						}

						// Hide Recall, since cancel is the same slot
						if (unit->id == UnitId::ProtossDidact && current_button->action_var == TechId::DimensionalRecall) {
							if (unit_ext->values[ValueDidactRecalling]) {
								current_button_state = Button_State::Invisible;
							} else {
								current_button_state = Button_State::Enabled;
							}
						}

						// Cancel new abilities!
						if (current_button->action_var == TechId::CancelNewAbility) {
							bool can_cancel = false;
							switch (unit->id) {
								case UnitId::TerranApostle: {
									if (bw::get_generic_rust_value(unit, RustId::ToggleLazarusAgent) == 1) {
										can_cancel = true;
									}
								} break;
								case UnitId::TerranWyvern: {
									if (unit_ext->values[ValueWyvernLandGrabOn]) {
										can_cancel = true;
									}
								} break;
								case UnitId::ProtossDidact: {
									if (unit_ext->values[ValueDidactRecalling]) {
										can_cancel = true;
									}
								} break;
								case UnitId::ProtossCantavis: {
									if (g_iquare_globals->get_effect(unit_ext->current_storm)) {
										can_cancel = true;
									}
								} break;
							}
							
							if (unit_ext->timers[TimerControlledDemolition] && !unit_ext->timers[TimerEmergencyDetonation]) {
								can_cancel = true;
							}

							if (can_cancel) {
								current_button_state = Button_State::Enabled;
							} else {
								current_button_state = Button_State::Invisible;
							}
						}
					}

					// Crig overwrites
					const auto disable_button = [&] {
						if (current_button_state == Button_State::Invisible) return false;
						current_button_state = Button_State::Disabled;
						return true;
					};
                    const auto disable_buttons_if = [&](bool condition, auto... icon_ids) {
						if (!condition) return;
						((current_button->icon_id == icon_ids && disable_button()), ...);
					};
					const auto disable_buttons_unit_ids = [&](UnitIdBitset unit_id_bitset) {
						if (current_button->requirement_func != (ReqFunc*)ButtonRequirements::Can_Create_UnitorBuilding) return;
						if (!unit_id_bitset[current_button->requirement_var]) return;
                        disable_button();
					};
					const auto dark_ages_hide_flyers = [&] {
                        if (bw::get_game_kind() != GameKind::DarkAges) return;
                        if (current_button->requirement_func != (ReqFunc*)ButtonRequirements::Can_Create_UnitorBuilding) return;
						if (!bw::unit_id_blocked_in_dark_ages(static_cast<UnitId::Enum>(current_button->requirement_var))) return;
						current_button_state = Button_State::Invisible;
					};
					disable_buttons_if(cm_player.disable_liftoff, IconId::Liftoff, IconId::Land);
					disable_buttons_if(cm_player.disable_burrow, IconId::Burrow, IconId::Unburrow);
					disable_buttons_unit_ids(cm_player.disabled_units);
					dark_ages_hide_flyers();

					// TODO: Reenable and test
					//if (unit->player != *g_local_nation_id && current_button_state == Button_State::Enabled) {
					//	current_button_state = Button_State::Disabled;
					//}

					if (current_button_state == Button_State::Invisible) {
						buttons_count++;
						current_button++; // select next button (advance by sizeof(Button))
					}
				}
			}

			// 00459278
			if (!stop_there && !jumpto5934E) {
				jumpto5933C = (!g_is_in_replay) && (*g_active_portrait_unit != nullptr) && ((*g_active_portrait_unit)->player != g_local_nation_id);
				// TODO: Reenable and test
				// jumpto5933C = false;
				if (!jumpto5933C) {
					// 0045929A
					if (buttons_count >= current_buttonset->buttons_in_set || current_dialog->index < current_button->position) {
						jumpto5933C = true;
					} else {
						// 004592B5
						current_dialog->button_user = current_button;
						if (current_dialog->graphic != current_button->icon_id) {
							// 004592C2
							struct
							{
								u32 unknown_value_1;	//[EBP-1C]	[00]
								u32 unknown_value_2;	//[EBP-18]	[04]
								u32 not_allocated;		//[EBP-14]	[08]
								u16 unknown_value_3;	//[EBP-10]	[0C]
								s16 mouseX;				//[EBP-0E]	[0E]
								s16 mouseY;				//[EBP-0C]	[10]
								u16 not_allocated_2;	//[EBP-0A]	[12]
								u32 buttonState;		//[EBP-08]	[14]
								ButtonSet* buttonset;	//[EBP-04]	[18]
							} stack_placeholder;
							current_dialog->graphic = current_button->icon_id;
							if (!(current_dialog->flags & BinDlgFlags::Highlighted)) {
								current_dialog->flags |= BinDlgFlags::Highlighted;
								bw::update_dialog(current_dialog);
							}
							stack_placeholder.not_allocated = 0;
							stack_placeholder.not_allocated_2 = 0;
							stack_placeholder.unknown_value_1 = 0x00000006;
							stack_placeholder.unknown_value_2 = 0x00000000;
							stack_placeholder.unknown_value_3 = 0x000E;
							stack_placeholder.mouseY = g_mouse->y;
							stack_placeholder.mouseX = g_mouse->x;
							stack_placeholder.buttonState = current_button_state;
							stack_placeholder.buttonset = current_buttonset;
							helper::fxn_interact(current_dialog, (u32)&stack_placeholder);
							current_button_state = stack_placeholder.buttonState;
						}
						// 0045930B
						bw::show_dialog(current_dialog);
						if (current_button_state == Button_State::Disabled) {
							bw::disable_dialog(current_dialog);
							// 0045931C
							if (*g_last_internal_error == 0x15) {
								current_button->action_string_id = 0x02FA; // Text: Not Implemented.
								current_button++;	// select next button (advance by sizeof(Button)
								buttons_count++;
								jumpto5934E = true;
							} else {
								jumpto59336 = true;
							}
						}
						// 00459331
						if (!jumpto59336 && !jumpto5934E) {
							helper::enable_control(current_dialog);
						}
						if (!jumpto5934E) {
							// 00459336
							current_button++;	// select next button (advance by sizeof(Button)
							buttons_count++;
							jumpto5934E = true;
						}
					}
				}
			}
			if (!stop_there && !jumpto5934E) {
				// 0045933C
				bw::hide_dialog(current_dialog);
				current_dialog->graphic = 0xFFFF;
				current_dialog->button_user = nullptr;
			}
			// 0045934E
			current_dialog = current_dialog->next;
			if (current_dialog == nullptr) {
				stop_there = true;
			}
			jumpto59336 = false;
			jumpto5933C = false;
			jumpto5934E = false;
		}
	}

	// 00459AD0
	// Identical to array at 0x0051AD30
	// Size is 40 = 4 * 10 = 0x28
	// 0x004598D0 is statbtn_Btn_Interact
	// 0x00459890 is statbtn_BIN_ReplayProgressbar
	const u32 Btn_fxnInteractFuncs[] = {
		0x004598D0, 0x004598D0, 0x004598D0,
		0x004598D0, 0x004598D0, 0x004598D0,
		0x004598D0, 0x004598D0, 0x004598D0,
		0x00459890
	};

	// Using a local array instead of the original may allow to go past the 9 buttons limitations
	// May need a special case using the original array adress and count when in replay if modified
	void statbtn_bin_custom_ctrl_id(BinDlg* dialog) {
		helper::register_user_dialog_action(dialog, (u32)&Btn_fxnInteractFuncs, sizeof(Btn_fxnInteractFuncs));
		helper::bindlg_blit_surface(dialog);
	}

	// Eventually, hook the following to make stat_txt.tbl a .txt.
	//		0x004575A0 draw_cancel_upgrade_context
	//		0x00457B10 draw_cancel_unit_context
	// -Pr0nogo
}