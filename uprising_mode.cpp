#include "uprising_mode.h"
#include "BW\api.h"
#include "trigger_system/trigger_system.h"
#include "hooks/limits/sfx_extender.h"
#include "ai_layouts.h"

// I copied and edited Threshold code for this. -Pr0nogo

// TODO: 
	/*
	* GENERAL
		* Move these files into CM directory, or maybe some other directory.
	* FIXES
		* Upstarts can't see anyone because they're an extended player, just like Turned (zombies)
		* AI expands into Upstart camps
	* NEW FEATURES
		* When 10 upstarts are near the spawner, they should go on an attack
		* Terran spawners should add addons
		* Zerg spawners should mutate into higher tiers
		* Protoss spawners should warp into successor options
		* All camps should set up defensive structures over time
	*/


#define UpstartSpawnCap 100
struct UpstartSpawnLocation {
	Point16 pos;
	u32 resarea;
	bool inited;
};

struct UpstartModeState {
	bool init;

	u32 spawn_count;
	UpstartSpawnLocation spawns[UpstartSpawnCap];
};
static UpstartModeState upstart_mode;

void init_upstart_mode() {
	memset(&upstart_mode, 0, sizeof(upstart_mode));

	if (!upstart_mode.init) {
		upstart_mode.init = true;
	}
}

void end_upstart_mode() {
	memset(&upstart_mode, 0, sizeof(upstart_mode));
}

void init_upstart_spawn(CHKUnit* unit, u32 flags) {
	if (bw::get_game_kind() != GameKind_Uprising || unit->player != 11) {
		return;
	}

	Point16 position = { unit->x, unit->y };
	switch (unit->unit_id) {
		case UnitId::TownCenterMarker:
		{
			if (!(flags & ChkUnitFlags_Cloaked)) {
				UpstartSpawnLocation spawn = {};
				spawn.pos = position;
				spawn.resarea = bw::get_closest_resarea(position);
				spawn.inited = true;

				if (upstart_mode.spawn_count < UpstartSpawnCap) {
					upstart_mode.spawns[upstart_mode.spawn_count++] = spawn;
				} else {
					bw::printf("Spawn cap reached (" STR(UpstartSpawnCap) ")");
				}

				// Ignore active start locations
				for (u32 player = 0; player < PLAYABLE_PLAYER_COUNT; player++) {
					if (bw::player_is_active(player) && spawn.resarea == helper::get_closest_base_from_startloc(player, spawn.pos.x, spawn.pos.y)) {
						spawn.inited = false;
					}
				}
			}
		} break;
	}
}

u32 upstart_spawner_basic(u32 index) {
	switch (index) {
		case 1: return UnitId::TerranStockade;
		case 2: return UnitId::ProtossGateway;
		case 3: return UnitId::ZergQuazethPool;
		case 4: return UnitId::TerranFulcrum;
		case 5: return UnitId::ProtossLattice;
		case 6: return UnitId::ZergHydrithDen;
	}
}

u32 upstart_spawn_basic(u32 spawner_id) {
	u32 max_options = 0;
	switch (spawner_id) {
		case UnitId::TerranStockade:	max_options = 3; break;
		case UnitId::TerranFulcrum:		max_options = 3; break;
		case UnitId::ProtossGateway:	max_options = 5; break;
		case UnitId::ProtossLattice:	max_options = 5; break;
		case UnitId::ZergQuazethPool:	max_options = 2; break;
		case UnitId::ZergHydrithDen:	max_options = 2; break;
	}
	u32 random_index = bw::rand_between(1, max_options);
	switch (spawner_id) {
		case UnitId::TerranStockade:
		{
			switch (random_index) {
				case 1: return UnitId::TerranMaverick;
				case 2: return UnitId::TerranHarakan;
				case 3: return UnitId::TerranCyprian;
			}
		} break;
		case UnitId::TerranFulcrum:
		{
			switch (random_index) {
				case 1: return UnitId::TerranVulture;
				case 2: return UnitId::TerranCyclops;
				case 3: return UnitId::TerranGoliath;
			}
		} break;
		case UnitId::ProtossGateway:
		{
			switch (random_index) {
				case 1: return UnitId::ProtossZealot;
				case 2: return UnitId::ProtossDracadin;
				case 3: return UnitId::ProtossEcclesiast;
				case 4: return UnitId::ProtossLegionnaire;
				case 5: return UnitId::ProtossHierophant;
			}
		} break;
		case UnitId::ProtossLattice:
		{
			switch (random_index) {
				case 1: return UnitId::ProtossSimulacrum;
				case 2: return UnitId::ProtossManifold;
				case 3: return UnitId::ProtossVassal;
				case 4: return UnitId::ProtossIdol;
				case 5: return UnitId::ProtossGolem;
			}
		} break;
		case UnitId::ZergQuazethPool:
		{
			switch (random_index) {
				case 1: return UnitId::ZergZethrokor;
				case 2: return UnitId::ZergQuazilisk;
			}
		} break;
		case UnitId::ZergHydrithDen:
		{
			switch (random_index) {
				case 1: return UnitId::ZergHydralisk;
				case 2: return UnitId::ZergIzirokor;
			}
		} break;
	}
}

void spawn_upstarts() {
	for (u32 i = 0; i < upstart_mode.spawn_count; ++i) {
		UpstartSpawnLocation spawn = upstart_mode.spawns[i];
		if (spawn.inited) {
			u32 spawner_id = upstart_spawner_basic(bw::rand_between(1, 6));
			Unit* spawner = bw::create_unit_in_game(spawner_id, 10, spawn.pos.x, spawn.pos.y);
			UnitExt* spawner_ext = get_unit_ext(spawner);
			spawner_ext->values[ValueUprisingSpawner] = 1;
			spawner_ext->timers[TimerUprisingSpawn] = 3;
			if (spawner->get_race() == RaceId::Zerg) {
				bw::spawn_custom_creep(spawner);
			}
		}
	}
}