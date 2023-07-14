//Source file for the Transfer Tech & Upgrades hook module.
//This controls how technologies and upgrades are transferred when units are
//passed from one player to another, either by triggers or casting Mind Control.
#include "transfer_tech_upgrades.h"
#include "apply_upgrade_flags.h"
#include <SCBW/api.h>

namespace {
	struct TechTransferData {
		u16 unit_id;
		u16 relatedTech[5]; //Maximum number of tech (use TechId::None to mark a premature end-of-list)
	};

	const TechTransferData tech_transfer_data[] = {
		// Terran
		{UnitId::TerranMaverick, TechId::Stimpacks, TechId::None},
		{UnitId::TerranHarakan, TechId::Stimpacks, TechId::None},
		{UnitId::TerranEidolon, TechId::Lockdown, TechId::PersonnelCloaking, TechId::None},
		{UnitId::TerranCleric, TechId::Healing, TechId::Restoration, TechId::None},
		{UnitId::TerranVulture, TechId::SpiderMines, TechId::None},
		{UnitId::TerranPhalanxTankMode, TechId::TankSiegeMode, TechId::None},
		{UnitId::TerranPhalanxSiegeMode, TechId::TankSiegeMode, TechId::None},
		{UnitId::TerranWraith, TechId::CloakingField, TechId::None},
		{UnitId::TerranSeraph, TechId::DefensiveMatrix, TechId::Irradiate, TechId::EMPShockwave, TechId::None},
		{UnitId::TerranMinotaur, TechId::YamatoGun, TechId::None},

		// Zerg
		{UnitId::ZergMatraleth, TechId::Infestation, TechId::MatralethParasite, TechId::SpawnBroodlings, TechId::Ensnare, TechId::None},
		{UnitId::ZergIsthrathaleth, TechId::Burrowing, TechId::CarapaceSwarm, TechId::Plague, TechId::Consume, TechId::None},
		{UnitId::ZergDroleth, TechId::Burrowing, TechId::None},
		{UnitId::ZergZethrokor, TechId::Burrowing, TechId::None},
		{UnitId::ZergHydralisk, TechId::Burrowing, TechId::LurkerAspect, TechId::None},
		{UnitId::ZergLakizilisk, TechId::LurkerAspect, TechId::None},

		// Protoss
		{UnitId::ProtossCantavis, TechId::ArchonWarp, TechId::PsionicStorm, TechId::Hallucination, TechId::None},
		{UnitId::ProtossDidact, TechId::DimensionalRecall, TechId::StasisField, TechId::None},
		{UnitId::ProtossLanifect, TechId::LanifectDisruptionField, TechId::None},
		{UnitId::ProtossCabalist, TechId::DarkArchonMeld, TechId::None},
		{UnitId::ProtossMindTyrant, TechId::Tyranny, TechId::Feedback, TechId::MindTyrantMaelstrom, TechId::None},
	};

	struct UpgradeTransferData {
		u16 unit_id;
		u8 related_upgrades[4]; //Maximum number of tech (use UpgradeId::None to mark a premature end-of-list)
	};

	const UpgradeTransferData upgrade_transfer_data[] = {
		// Terran
		{UnitId::TerranMaverick, UpgradeId::U238Shells, UpgradeId::None},
		{UnitId::TerranEidolon, UpgradeId::OcularImplants, UpgradeId::MoebiusReactor, UpgradeId::None},
		{UnitId::TerranCleric, UpgradeId::CaduceusReactor, UpgradeId::None},
		{UnitId::TerranVulture, UpgradeId::IonThrusters, UpgradeId::None},
		{UnitId::TerranGoliath, UpgradeId::CharonBooster, UpgradeId::None},
		{UnitId::TerranSeraph, UpgradeId::TitanReactor, UpgradeId::None},
		{UnitId::TerranWraith, UpgradeId::ApolloReactor, UpgradeId::None},
		{UnitId::TerranMinotaur, UpgradeId::ColossusReactor, UpgradeId::None},

		// Zerg
		{UnitId::ZergOvileth, UpgradeId::VentralSacs, UpgradeId::Antennae, UpgradeId::None},
		{UnitId::ZergZethrokor, UpgradeId::MetabolicBoost, UpgradeId::AdrenalGlands, UpgradeId::None},
		{UnitId::ZergHydralisk, UpgradeId::MuscularAugments, UpgradeId::GroovedSpines, UpgradeId::None},
		{UnitId::ZergMatraleth, UpgradeId::GameteMeiosis, UpgradeId::None},
		{UnitId::ZergIsthrathaleth, UpgradeId::MetasynapticNode, UpgradeId::None},
		{UnitId::ZergUltrokor, UpgradeId::AnabolicSynthesis, UpgradeId::ChitinousPlating, UpgradeId::None},

		// Protoss
		{UnitId::ProtossZealot, UpgradeId::LegEnhancements, UpgradeId::None},
		{UnitId::ProtossDracadin, UpgradeId::SingularityCharge, UpgradeId::None},
		{UnitId::ProtossCantavis, UpgradeId::KhaydarinAmulet, UpgradeId::None},
		{UnitId::ProtossMindTyrant, UpgradeId::ArgusTalisman, UpgradeId::None},
		{UnitId::ProtossAccantorOld, UpgradeId::ReinforcedScarabs, UpgradeId::ReaverCapacity, UpgradeId::None},
		{UnitId::ProtossEnvoy, UpgradeId::GraviticDrive, UpgradeId::None},
		{UnitId::ProtossWitness, UpgradeId::SensorArray, UpgradeId::GraviticBoosters, UpgradeId::None},
		{UnitId::ProtossPanoptus, UpgradeId::ApialSensors, UpgradeId::GraviticThrusters, UpgradeId::None},
		{UnitId::ProtossSolarion, UpgradeId::AuxilliaryHangars, UpgradeId::None},
		{UnitId::ProtossDidact, UpgradeId::KhaydarinCore, UpgradeId::None},
		{UnitId::ProtossLanifect, UpgradeId::ArgusJewel, UpgradeId::None},
	};
}

namespace hooks {
	// Summon a function passed as parameter, having the same variables layout for both upgrade and tech related functions.
	bool research_helper(u32 researchTechFunc, Unit* source_unit, u32 techId, u32 relativeVariable, u32 destPlayerId) {
		static u32 result;
		__asm {
			PUSHAD
			PUSH destPlayerId		// [EBP+0x14]
			PUSH relativeVariable	// [EBP+0x10] // this variable has different meaning depending on the caller (boolean or number) 
			PUSH techId				// [EBP+0x0C]
			PUSH source_unit			// [EBP+0x08]
			CALL researchTechFunc
			MOV result, EAX
			POPAD
		}
		return (result != 0);

	}

	// Transfers all tech related to the @p source unit to @p target_player.
	// Originally researchAllTechsFromUnit @ 004E2C00
	void transfer_unit_tech_to_player_hook(Unit* source_unit, u8 target_player, u32 research_tech_func) {
		return;

		const TechTransferData* tech_data_array_pos_begin = &tech_transfer_data[0];								// original was (TechTransferData*) 0x005011CA;
		const TechTransferData* tech_data_array_pos_end = &tech_transfer_data[ARRAY_SIZE(tech_transfer_data)];	// original was (TechTransferData*) 0x005012DE;
		const s32 related_tech_array_size = 5;

		// Stop if the source unit does not exist
		if (!research_tech_func || !source_unit) {
			helper::SErrSetLastError(87);
		} else {
			bool break_main_loop = false;
			bool break_sub_loop;

			// Browse through TechTransferData structures
			for (TechTransferData* data = (TechTransferData*)tech_data_array_pos_begin; !break_main_loop && (data != tech_data_array_pos_end); ++data) {
				if (source_unit->id == data->unit_id) {
					break_sub_loop = false;

					// Browse through techs for current unit
					for (s32 tech_index = 0; !break_sub_loop && tech_index < related_tech_array_size; tech_index++) {
						u16 tech_id = data->relatedTech[tech_index];
						// Forced end of array, stop checking techs, but still check others TechTransferData
						// (in case the unit is represented several times?)
						if (tech_id == TechId::None) {
							break_sub_loop = true;
						} else {
							// Check if tech is researched by unit for transfer
							b8 is_researched = false;
							if (tech_id >= TechId::Restoration) {
								is_researched = TechBw->is_researched[source_unit->player][tech_id - TechId::Restoration];
							} else {
								is_researched = TechSc->is_researched[source_unit->player][tech_id];\
							}

							// Probably research the tech of unit if not already researched by player
							// if return false, stop checking techs and TechTransferData
							if (!research_helper(research_tech_func, source_unit, tech_id, is_researched, target_player)) {
								break_sub_loop = true;
								break_main_loop = true;
							}
						}
					}
				}
			}
		}
	}

	// Transfers all upgrades related to the @p source unit to @p target_player.
	// Originally upgradeAllUpgradesFromUnit @ 004E2B50
	void transfer_unit_upgrades_to_player_hook(Unit* source_unit, u8 target_player, u32 research_upgrades_func) {
		return;

		const UpgradeTransferData* upgrade_data_array_pos_begin = &upgrade_transfer_data[0];								// original was (UpgradeTransferData*) 0x005012E2;
		const UpgradeTransferData* upgrade_data_array_pos_end = &upgrade_transfer_data[ARRAY_SIZE(upgrade_transfer_data)];	// original was (UpgradeTransferData*) 0x005013DC;
		const s32 related_upgrade_array_size = 4;

		//Stop if the source unit does not exist
		if (!research_upgrades_func || !source_unit) {
			helper::SErrSetLastError(87); // 0x57
		} else { 
			bool break_main_loop = false;
			bool break_sub_loop = false;

			// Browse through UpgradeTransferData structures
			for (UpgradeTransferData* data = (UpgradeTransferData*)upgrade_data_array_pos_begin; !break_main_loop && (data != upgrade_data_array_pos_end); ++data) { 
				if (source_unit->id == data->unit_id) {
					break_sub_loop = false;

					// Browse through upgrades for current unit
					for (s32 upgrade_index = 0; !break_sub_loop && upgrade_index < related_upgrade_array_size; upgrade_index++) {
						u16 upgrade_id = data->related_upgrades[upgrade_index];
						// Forced end of array, stop checking upgrades, but still check others UpgradeTransferData
						// (in case the unit is represented several times?)
						if (upgrade_id == UpgradeId::None) {
							break_sub_loop = true;
						} else {
							u8 source_upgrade_level = 0;
							// check level of upgrade of by unit for transfer
							if (upgrade_id >= UpgradeId::PrismaticShield) {
								source_upgrade_level = UpgradesBw->current_level[source_unit->player][upgrade_id - UpgradeId::PrismaticShield];
							} else {
								source_upgrade_level = UpgradesSc->current_level[source_unit->player][upgrade_id];
							}

							// Probably increase the upgrade level of player until reaching same point as source_unit
							// if return false, stop checking upgrades and UpgradeTransferData
							if (!research_helper(research_upgrades_func, source_unit, upgrade_id, source_upgrade_level, target_player)) {
								break_sub_loop = true;
								break_main_loop = true;
							}
						}
					}
				}
			}
		}
	}
}
