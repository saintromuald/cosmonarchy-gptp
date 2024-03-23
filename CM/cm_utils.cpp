#include "cm_utils.h"
#include "BW\api.h"
#include "trigger_system/trigger_system.h"

Box32 box32_rect(Point16 pos, u32 w, u32 h) {
	Box32 box = {};
	box.left   = pos.x - w / 2;
	box.right  = pos.x + w / 2;
	box.top    = pos.y - h / 2;
	box.bottom = pos.y + h / 2;
	return box;
}

Box32 box32_square(Point16 pos, u32 w) {
	Box32 box = {};
	box.left   = pos.x - w / 2;
	box.right  = pos.x + w / 2;
	box.top    = pos.y - w / 2;
	box.bottom = pos.y + w / 2;
	return box;
}

bool is_hostile_or_neutral(u32 player, u32 other_player) {
	if (other_player == NeutralPlayer) {
		return true;
	}
	return player != other_player && !bw::is_allied_to(player, other_player);
}

void replay_witness_event(Unit* unit, SpectatorEvent event_type) {
	if (*g_is_in_replay || globals->players[*g_local_nation_id].witness_mode) {
		bool ping = true;
		Point16 position = {};
		switch (event_type) {
			case SpectatorEvent_Combat:
			{
				position = { unit->position.x, unit->position.y };
			} break;
			case SpectatorEvent_Tech:
			{
				position = { unit->position.x, unit->position.y };
			//	helper::play_sound_global(units_dat::sound_yes_first[unit->id]);
			} break;
		}
		if (ping) {
			helper::minimap_ping(position.x, position.y, 17);
		}
	}
}

u32 get_construction_graphic(Unit* unit)
{
	UnitExt* unit_ext = get_unit_ext(unit);
	switch (unit->id)
	{
		case UnitId::ZergKalkalisk:
		{
			if (unit_ext->values[ValueOriginatedFromOcsalLarva]) {
				return ImageId::KalkaliskBirthEgg;
			} else {
				return ImageId::KalkaliskBirthCocoon;
			}
		} break;
	}
	return units_dat::construction_graphic[unit->id];
}

void init_omnipresence_state() {
	if (bw::get_game_kind() == GameKind_Omnipresence) {
		for (u32 player = 0; player < PLAYABLE_PLAYER_COUNT; player++) {
			if (g_player_table[player].type == PlayerType::Computer || g_player_table[player].type == PlayerType::ComputerSlot) {
				continue;
			}
			if (globals->players[player].omnipresence_state_was_set == false) {
				OmnipresenceState omni_state = OmnipresenceState_None;
				switch (g_player_table[player].race) {
					case RaceId::Terran:
					{
						omni_state = OmnipresenceState_TerranProtoss;
					} break;
					case RaceId::Protoss:
					{
						omni_state = OmnipresenceState_ProtossZerg;
					} break;
					case RaceId::Zerg:
					{
						omni_state = OmnipresenceState_ZergTerran;
					} break;
				}
				globals->players[player].omnipresence_state = omni_state;
				globals->players[player].omnipresence_state_was_set = true;
			}
		}
	}
}

void disconnect_addon_no_event(Unit* unit) {
	Unit* addon = unit->building.addon;
	unit->building.addon = 0;

	UnitExt* addon_ext = get_unit_ext(addon);
	addon_ext->values[ValueAddonInactive] = 1;
	bw::refresh_console();

	Unit* active_unit = *g_active_iscript_unit;
	*g_active_iscript_unit = addon;
	for (Image* i = addon->sprite->images.head; i; i = i->link.next) {
		helper::play_image_iscript(i, IscriptAnimation::LiftOff);
	}
	*g_active_iscript_unit = active_unit;
}

u32 get_bullet_damage(Bullet* bullet, u32 weapon_id)
{
	BulletExt* bullet_ext = get_bullet_ext(bullet);
	if (bullet_ext->damage_override) {
		return bullet_ext->damage_override;
	}
	return weapons_dat::damage_amount[weapon_id];
}