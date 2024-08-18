#include "cm_ids.h"
#include "BW\api.h"
#include "hooks\weapons\weapon_fire.h"
#include "trigger_system/trigger_system.h"
#include "data_ext.h"

bool unit_id_is_capped_gas_structure(u16 unit_id) {
	return unit_id == UnitId::TerranReservoir
		|| unit_id == UnitId::TerranWellbore
		|| unit_id == UnitId::ProtossAquifer
		|| unit_id == UnitId::ZergExcisant
		|| unit_id == UnitId::ZergEzigrisant;
}

bool unit_id_is_gas_structure(u16 unit_id) {
	return unit_id_is_capped_gas_structure(unit_id)
		||	   unit_id == UnitId::ResourceVespeneRidge
		||	   unit_id == UnitId::ResourceVespeneGeyser;
}

bool unit_id_is_uncapped_gas_node(u16 unit_id)
{
	return unit_id == UnitId::ResourceVespeneRidge
		|| unit_id == UnitId::ResourceVespeneGeyser;
}

bool unit_id_is_smaller_building(u16 unit_id) {
	return (units_dat::base_property[unit_id] & UnitProperty::Addon && unit_id != UnitId::TerranHadron)
		|| unit_id == UnitId::TerranSupplyDepot
		|| unit_id == UnitId::TerranWatchdog
		|| unit_id == UnitId::TerranAnchor
		|| unit_id == UnitId::TerranSentinel
		|| unit_id == UnitId::ProtossPylon
		|| unit_id == UnitId::ProtossWarden
		|| unit_id == UnitId::ProtossMatrix
		|| unit_id == UnitId::ZergCagrantCesiant
		|| unit_id == UnitId::ZergSurkithCesiant
		|| unit_id == UnitId::ZergSpraithCesiant
		|| unit_id == UnitId::ZergLarvoskCesiant
		|| unit_id == UnitId::ZergSkortrithCesiant
		|| unit_id == UnitId::ZergSpirtithCesiant
		|| unit_id == UnitId::ZergKazitrithCesiant
		|| unit_id == UnitId::ZergNydusCesiant
		|| unit_id == UnitId::ZergOcsalCesiant;
}

b32 unit_id_is_tech(u32 unit_id, bool allow_addons) {
	if (units_dat::base_property[unit_id] & UnitProperty::Addon && allow_addons) {
		switch (unit_id) {
			case UnitId::TerranComsatStation:
			case UnitId::TerranAnvil:
			case UnitId::TerranQuarry:
				return false;
		}
		return true;
	}

	switch (unit_id) {
		// Terran
		case UnitId::TerranAtlas:
		case UnitId::TerranDaedala:

			// Zerg
		case UnitId::ZergIrolIris:
		case UnitId::ZergIrulentIvoa:
		case UnitId::ZergOthstolOviform:
		case UnitId::ZergQuazethPool:
		case UnitId::ZergKalkiirLake:
		case UnitId::ZergMutathSpire:
		case UnitId::ZergTosvrolHaunt:
		case UnitId::ZergAxtothAxis:
		case UnitId::ZergBegrozithBarrow:
		case UnitId::ZergHydrithDen:
		case UnitId::ZergTethvithSwamp:
		case UnitId::ZergAlkagIteth:
		case UnitId::ZergChokralChazsal:
		case UnitId::ZergGeszkathGrotto:
		case UnitId::ZergAlkajChasm:
		case UnitId::ZergZorkizShroud:
		case UnitId::ZergUltravCavern:
		case UnitId::ZergMatravilNest:
		case UnitId::ZergAlmakisAntre:
		case UnitId::ZergElthisthMound:
		case UnitId::ZergRakzogthRoost:
			return true;
	}
	return false;
}

b32 unit_id_is_tech_tier_structure(u32 unit_id) {
	switch (unit_id) {
		// Terran
		case UnitId::TerranAtlas:
		case UnitId::TerranDaedala:

		// Zerg
		case UnitId::ZergIrolIris:
		case UnitId::ZergOthstolOviform:
			return true;
	}
	return false;
}

bool unit_id_is_transport(u32 unit_id) {
	return unit_id == UnitId::TerranTrojan
		|| unit_id == UnitId::TerranQilin
		|| unit_id == UnitId::TerranPythean
		|| unit_id == UnitId::ProtossEnvoy
		|| unit_id == UnitId::ZergOvileth
		|| unit_id == UnitId::ZergIroleth
		|| unit_id == UnitId::ZergOthstoleth;
}

bool unit_id_race_check(u32 unit_id, u32 race) {
	switch (race) {
		case RaceId::Terran: {
			if (units_dat::group_flags[unit_id].is_terran) {
				return true;
			}
			switch (unit_id) {
				case UnitId::TerranCohort:
				case UnitId::TerranAspirant:
				case UnitId::TerranTarasque:
				case UnitId::TerranLuminary: {
					return true;
				}
			}
		} break;
		case RaceId::Protoss: {
			if (units_dat::group_flags[unit_id].is_protoss) {
				return true;
			}
		} break;
		case RaceId::Zerg: {
			if (units_dat::group_flags[unit_id].is_zerg) {
				return true;
			}
		} break;
	}
	return false;
}

bool unit_id_is_protoss_warp_factory(u32 unit_id) {
	return unit_id == UnitId::ProtossGateway
		|| unit_id == UnitId::ProtossCenotaph
		|| unit_id == UnitId::ProtossGrandLibrary
		|| unit_id == UnitId::ProtossRogueGallery
		|| unit_id == UnitId::ProtossProstrationStage
		|| unit_id == UnitId::ProtossSanctumOfSorrow
		|| unit_id == UnitId::ProtossAncestralArchives
		|| unit_id == UnitId::ProtossHallOfHeretics
		|| unit_id == UnitId::ProtossStargate
		|| unit_id == UnitId::ProtossArgosy
		|| unit_id == UnitId::ProtossFleetBeacon
		|| unit_id == UnitId::ProtossMonumentOfSin;
}

bool unit_id_has_flash_shielding(u32 unit_id) {
	return unit_id != UnitId::ProtossVagrantShade
		&& unit_id != UnitId::TerranAspirant
		&& unit_id != UnitId::TerranLuminary;
}

bool unit_id_has_shield_regeneration(u32 unit_id)
{
	return unit_id == UnitId::TerranAspirant
		|| unit_id == UnitId::TerranLuminary;
}

bool unit_id_is_stackable_building(u32 unit_id) {
	return false; // Disabled to see if it improves AI base layouts. -Pr0nogo
	switch (unit_id)
	{
		case UnitId::TerranWatchdog:
		case UnitId::TerranAnchor:
		case UnitId::TerranSentinel:
		case UnitId::ProtossPylon:
		case UnitId::ProtossWarden:
		case UnitId::ProtossMatrix:
	//	case UnitId::ZergCagrantCesiant: // Disabled to prevent spamming. -Pr0nogo
		case UnitId::ZergSurkithCesiant:
		case UnitId::ZergSpraithCesiant:
		case UnitId::ZergLarvoskCesiant:
		case UnitId::ZergSkortrithCesiant:
		case UnitId::ZergSpirtithCesiant:
		case UnitId::ZergKazitrithCesiant:
		case UnitId::ZergOcsalCesiant:
		case UnitId::ZergIrolIris:
		case UnitId::ZergOthstolOviform:
		{
			return true;
		} break;
	}
	return false;
}

bool unit_id_has_addon(u32 unit_id) {
	switch (unit_id) {
		case UnitId::TerranMinistry:
		case UnitId::TerranTreasury:
		case UnitId::TerranStockade:
		case UnitId::TerranCaptaincy:
		case UnitId::TerranIronCouncil:
		case UnitId::TerranFulcrum:
		case UnitId::TerranIronFoundry:
		case UnitId::TerranStarport:
		case UnitId::TerranNaniteAssembly:
		{
			return true;
		} break;
	}
	return false;
}

u32 unit_id_get_larva_type(u32 unit_id) {
	switch (unit_id) {
		case UnitId::ZergCaphrolosk:
		case UnitId::ZergOcsalCesiant:
		case UnitId::ZergOcsalLarva:
		{
			return UnitId::ZergOcsalLarva;
		} break;
	}
	return UnitId::ZergLarva;
}

u32 unit_id_swarmhold_value(u32 unit_id) {
	switch (unit_id) {
		case UnitId::ZergGathtelosk: return 1; // 1 extra unit per egg
	}
	return 0;
}

bool unit_id_is_vespene_node(u32 unit_id) {
	return unit_id == UnitId::ResourceVespeneRidge
		|| unit_id == UnitId::ResourceVespeneGeyser
		|| unit_id == UnitId::TerranReservoir
		|| unit_id == UnitId::TerranWellbore
		|| unit_id == UnitId::ProtossAquifer
		|| unit_id == UnitId::ZergExcisant
		|| unit_id == UnitId::ZergEzigrisant;
}

bool unit_id_is_ground_garrison(u32 unit_id) {
	return unit_id == UnitId::TerranAnchor
		|| unit_id == UnitId::TerranLodestone
		|| unit_id == UnitId::TerranLodestoneStandalone
		|| unit_id == UnitId::TerranAnchorGrounded
		|| unit_id == UnitId::TerranTencendur;
}

bool unit_id_is_garrison(u32 unit_id) {
	return unit_id_is_ground_garrison(unit_id); // Will add more bunker types later. -Pr0nogo
}

bool unit_id_is_melee_air_unit(u32 unit_id) {
	return unit_id == UnitId::ZergNathrokor
		|| unit_id == UnitId::ZergGorgrokor
		|| unit_id == UnitId::ZergTetcaerokor
		|| unit_id == UnitId::ZergAkistrokor
		|| unit_id == UnitId::ZergZarcavrokor
		|| unit_id == UnitId::ZergSkryrokor
		|| unit_id == UnitId::TerranTalos
		|| unit_id == UnitId::TerranWendigo;
}

bool unit_id_is_morphed_from_double_unit(u32 unit_id) {
	return unit_id == UnitId::ZergVorvrokor
		|| unit_id == UnitId::ZergKalkalisk
		|| unit_id == UnitId::ZergTosgrilisk
		|| unit_id == UnitId::ZergGhitorokor;
}

bool unit_id_is_power_generator(u32 unit_id) {
	return unit_id == UnitId::ProtossPylon
		|| unit_id == UnitId::ProtossCrucible;
}

bool unit_id_is_psi_provider(u16 unit_id) {
	return unit_id == UnitId::ProtossPylon
	    || unit_id == UnitId::ProtossCrucible;
}

bool unit_id_can_do_controlled_demolition(u32 unit_id) {
	return units_dat::base_property[unit_id] & UnitProperty::Addon
		|| unit_id == UnitId::TerranReservoir
		|| unit_id == UnitId::TerranWellbore
		|| unit_id == UnitId::TerranLobotomyMine;
}

UnitId::Enum unit_id_to_siege_mode(UnitId::Enum unit_id) {
	switch (unit_id) {
		case UnitId::TerranShamanSupportMode:     return UnitId::TerranShamanVisionaryMode; 
		case UnitId::TerranMatadorTankMode:       return UnitId::TerranMatadorCinderMode;   
		case UnitId::TerranPhalanxTankMode:       return UnitId::TerranPhalanxSiegeMode;   
		case UnitId::TerranMadrigalImperiosoMode: return UnitId::TerranMadrigalFuriaMode;  
	//	case UnitId::TerranCuirassTankMode:       return UnitId::TerramCuirassTrenchMode;  
	}
	return UnitId::None;
}

UnitId::Enum unit_id_to_tank_mode(UnitId::Enum unit_id) {
	switch (unit_id) {
		case UnitId::TerranShamanVisionaryMode: return UnitId::TerranShamanSupportMode;
		case UnitId::TerranMatadorCinderMode:   return UnitId::TerranMatadorTankMode;
		case UnitId::TerranPhalanxSiegeMode:    return UnitId::TerranPhalanxTankMode;
		case UnitId::TerranMadrigalFuriaMode:   return UnitId::TerranMadrigalImperiosoMode;
	//	case UnitId::TerramCuirassTrenchMode:   return UnitId::TerranCuirassTankMode;
	}
	return UnitId::None;
}

bool unit_id_is_siege_mode(UnitId::Enum unit_id) {
	return unit_id_to_tank_mode(unit_id) != UnitId::None;
}

bool unit_id_is_tank_mode(UnitId::Enum unit_id) {
	return unit_id_to_siege_mode(unit_id) != UnitId::None;
}

bool unit_id_is_broodspawn(u32 unit_id) {
	return unit_id == UnitId::ZergRilirokor
		|| unit_id == UnitId::ZergRililisk;
}

bool unit_id_is_zerg_production(u16 unit_id, bool town_center_only) {
	if (town_center_only) {
		return unit_id == UnitId::ZergHachirosk
			|| unit_id == UnitId::ZergCaphrolosk
			|| unit_id == UnitId::ZergGathtelosk
			|| unit_id == UnitId::Omnivale;
	} else {
		return unit_id == UnitId::ZergHachirosk
			|| unit_id == UnitId::ZergCaphrolosk
			|| unit_id == UnitId::ZergGathtelosk
			|| unit_id == UnitId::ZergLarvoskCesiant
			|| unit_id == UnitId::ZergOcsalCesiant
			|| unit_id == UnitId::Omnivale;
	}
}

bool unit_id_is_town_center(u16 unit_id) {
	return unit_id == UnitId::TerranMinistry
		|| unit_id == UnitId::ProtossNexus
		|| unit_id == UnitId::ZergHachirosk
		|| unit_id == UnitId::ZergCaphrolosk
		|| unit_id == UnitId::ZergGathtelosk
		|| unit_id == UnitId::Omnivale;
}

bool unit_id_is_factional(u16 unit_id) {
	return unit_id == UnitId::TerranHotshot
		|| unit_id == UnitId::TerranCohort
		|| unit_id == UnitId::TerranAspirant
		|| unit_id == UnitId::TerranSouthpaw
		|| unit_id == UnitId::TerranTarasque
		|| unit_id == UnitId::TerranPhiloc
		|| unit_id == UnitId::TerranWendigo
		|| unit_id == UnitId::TerranLuminary;
}

bool unit_id_uses_xlarge_overlays(u16 unit_id) {
	return unit_id == UnitId::TerranPenumbra
		|| unit_id == UnitId::TerranPhobos
		|| unit_id == UnitId::TerranDiadem
		|| unit_id == UnitId::TerranIronFoundry
		|| unit_id == UnitId::TerranNaniteAssembly
		|| unit_id == UnitId::ProtossOptecton
		|| unit_id == UnitId::ProtossStarSovereign
		|| unit_id == UnitId::ProtossAncestralArchives
		|| unit_id == UnitId::ProtossMonumentOfSin
		|| unit_id == UnitId::ZergOthstoleth
		|| unit_id == UnitId::ZergAlkajelisk;
}

bool unit_id_is_resource_node(u16 unit_id, bool count_gas_structures) {
	if (count_gas_structures) {
		return unit_id == UnitId::ResourceMineralField
			|| unit_id == UnitId::ResourceMineralFieldType2
			|| unit_id == UnitId::ResourceMineralFieldType3
			|| unit_id == UnitId::ResourceVespeneGeyser
			|| unit_id == UnitId::ResourceVespeneRidge
			|| unit_id == UnitId::TerranReservoir
			|| unit_id == UnitId::TerranWellbore
			|| unit_id == UnitId::ProtossAquifer
			|| unit_id == UnitId::ZergExcisant
			|| unit_id == UnitId::ZergEzigrisant;
	} else {
		return unit_id == UnitId::ResourceMineralField
			|| unit_id == UnitId::ResourceMineralFieldType2
			|| unit_id == UnitId::ResourceMineralFieldType3
			|| unit_id == UnitId::ResourceVespeneGeyser
			|| unit_id == UnitId::ResourceVespeneRidge;
	}
}

bool unit_id_is_minerals(u16 unit_id) {
	return unit_id == UnitId::ResourceMineralField
		|| unit_id == UnitId::ResourceMineralFieldType2
		|| unit_id == UnitId::ResourceMineralFieldType3;
}

bool unit_id_is_zerg_supply(u16 unit_id) {
	return (unit_id == UnitId::ZergOvileth
		|| unit_id == UnitId::ZergIroleth
		|| unit_id == UnitId::ZergOthstoleth);
}

bool unit_paths_over_small_units(Unit* unit) {
	// Keeping arg as unit in case we want to add status effects later. -Pr0nogo
	return unit->id == UnitId::TerranPenumbra
		|| unit->id == UnitId::ProtossOptecton;
}

bool unit_collides_with_titanic(Unit* unit) {
	if (unit->status & UnitStatus::GroundedBuilding) {
		return unit->id != UnitId::ProtossPylon && !unit_id_is_minerals(unit->id);
	}
	return unit->id == UnitId::TerranCataphract
		|| unit->id == UnitId::TerranPaladin
		|| unit->id == UnitId::TerranClaymore
		|| unit->id == UnitId::TerranPenumbra
		|| unit->id == UnitId::TerranAion
		|| unit->id == UnitId::ProtossPariah
		|| unit->id == UnitId::ProtossOptecton
		|| unit->id == UnitId::ProtossArchitect
		|| unit->id == UnitId::ProtossDemiurge
		|| unit->id == UnitId::ZergOkrimnilisk
		|| unit->id == UnitId::ZergKeskathalor
		|| unit->id == UnitId::ZergKethrostalor
		|| unit->id == UnitId::ZergUltrokor;
}

bool weapon_id_is_for_harvesting(u32 weapon_id) {
	return weapon_id == WeaponId::MasonFusionCutterHarvest
		|| weapon_id == WeaponId::ScribeParticleBeamHarvest
		|| weapon_id == WeaponId::DrolethSpinesHarvest;
}

bool weapon_id_is_standard_weapon(u32 weapon_id) {
	switch (weapons_dat::explosion_type[weapon_id]) {
		case WeaponEffect::NormalHit:
		case WeaponEffect::SplashRadial:
		case WeaponEffect::SplashEnemy:
		case WeaponEffect::SplashAir:
		case WeaponEffect::Unknown_Crash:
			return true;
	}
	return false;
}

bool weapon_id_bounces_lithe_organelle(u32 weapon_id) {
	return weapon_id == WeaponId::MutaliskGlaveWurm
		|| weapon_id == WeaponId::LiiraliskHollowedSpores
		|| weapon_id == WeaponId::SpirtithColonySpirtilSpores;
}

bool weapon_id_bounces_crushing_tide(u32 weapon_id) {
	return weapon_id == WeaponId::IzirokorEnergizedBioplasma
		|| weapon_id == WeaponId::TosgriliskExplosiveSpores
		|| weapon_id == WeaponId::KonviliskSowingSpines
		|| weapon_id == WeaponId::CherbrathalorVenemousBursts;
}

bool weapon_id_accelerates(u32 weapon_id) {
	return weapon_id == WeaponId::EmpyreanPlasmaBarrage;
}

bool weapon_id_is_melee_aoe(u32 weapon_id) {
	return weapon_id == WeaponId::SycophantSparringSaber
		|| weapon_id == WeaponId::GolemBloodlessAssault
		|| weapon_id == WeaponId::MortothrokorWickedIncisors
		|| weapon_id == WeaponId::GorgrokorSerratedCleavers
		|| weapon_id == WeaponId::ZarcavrokorBisectingBlades
		|| weapon_id == WeaponId::TethzorokorSaberArray
		|| weapon_id == WeaponId::TetcaerokorCleavingMaw;
}

bool weapon_id_cleaves(u32 weapon_id) {
	return weapon_id == WeaponId::TethzorokorSaberArray;
}

u32 weapon_id_intercepts(u32 weapon_id) {
	switch (weapon_id) {
		case WeaponId::OptectonSunsplitBeam:
		case WeaponId::AccantorPlasmaShell:
			return InterceptType_GroundOnly;

		case WeaponId::EmpyreanPlasmaBarrage:
			return InterceptType_AirOnly;

		case WeaponId::HotshotDispatchShotgun:
		case WeaponId::SirenCharybdalRifle:
		case WeaponId::EngramKhaydarinPlasmaLaser:
		case WeaponId::AtreusDauntlessDisruptorCannon:
		case WeaponId::CharlatanFerventBlaster:
		case WeaponId::CantavisHallucinationCast:
		case WeaponId::PatriarchIronhandWeaver:
		case WeaponId::IdolPrismaticBlaster:
		case WeaponId::StarSovereignInterventionMissiles:
			return InterceptType_Any;
	}
	return InterceptType_None;
}

bool weapon_id_pierces(u32 weapon_id) {
	switch (weapon_id) {
		case WeaponId::CyprianTwinRailguns:
		case WeaponId::SavantDiscoidRailgunPiercing:
		case WeaponId::AutocratEminenceRifle:
		case WeaponId::SevenstepGeometricRiflePiercing:
		case WeaponId::RamessesPharoahCannons:
		case WeaponId::ClaymoreHarpoonRailgun:
		case WeaponId::PenumbraErebolShockCannon:
		case WeaponId::WyvernJacksonCannon:
		case WeaponId::MagnetarNewtonianShockCannon:
		case WeaponId::HippogriffChimeraCannons:
		case WeaponId::SentinelCerberusRailgun:
		case WeaponId::KalkaliskViscousAcid:
		case WeaponId::SkithrokorSkitteringSpores:
		case WeaponId::ZarcavrokorBonesofReflection:
			return true;
	}
	return false;
}

bool weapon_id_piercing_stops_on_armor(u32 weapon_id) {
	if (weapon_id_is_incremental(weapon_id)) {
		return false;
	}
	return true;
}

bool weapon_is_melee_with_aoe(Bullet* bullet, u32 weapon_id) {
	if (bullet && bullet->source_unit) {
		switch (weapon_id) {
			case WeaponId::SycophantSparringSaber:
			case WeaponId::GolemBloodlessAssault:
			case WeaponId::MortothrokorWickedIncisors:
			case WeaponId::GorgrokorSerratedCleavers:
			case WeaponId::ZarcavrokorBisectingBlades:
			case WeaponId::TethzorokorSaberArray:
			case WeaponId::TetcaerokorCleavingMaw:
				return true;
		}
	}
	return false;
}

bool order_id_is_harvest_related(u32 order_id) {
	return order_id == OrderId::Harvest1
		|| order_id == OrderId::Harvest5
		|| order_id == OrderId::MoveToMinerals
		|| order_id == OrderId::HarvestGas1
		|| order_id == OrderId::HarvestGas3
		|| order_id == OrderId::HarvestFinish
		|| order_id == OrderId::ReturnGas
		|| order_id == OrderId::ReturnMinerals
		|| order_id == OrderId::MiningMinerals
		|| order_id == OrderId::CanHarvestOre
		|| order_id == OrderId::CanEnterGasMine
		|| order_id == OrderId::ResetCollision2
		|| order_id == OrderId::MoveToHarvestObscured;
}

bool tech_id_has_custom_targeting(u32 tech_id) {
	return tech_id != TechId::Hallucination
		&& tech_id != TechId::LuminaryAuralAssault
		&& tech_id != TechId::HypnagogueThroughTheLookingGlass;
}

bool image_id_is_powerup(u32 image_id) {
	return image_id == ImageId::MineralChunkType1
		|| image_id == ImageId::MineralChunkType2
		|| image_id == ImageId::GasRaw
		|| image_id == ImageId::GasRaw2
		|| image_id == ImageId::TerranGasTankType1
		|| image_id == ImageId::TerranGasTankType2
		|| image_id == ImageId::ProtossGasOrbType1
		|| image_id == ImageId::ProtossGasOrbType2
		|| image_id == ImageId::ZergGasSacType1
		|| image_id == ImageId::ZergGasSacType2
		|| image_id == ImageId::Flag
		|| image_id == ImageId::KhaydarinCrystal
		|| image_id == ImageId::PsiEmitter
		|| image_id == ImageId::DataDisc;
}

ImageId::Enum analogue_image_id_overwrite(u32 weapon_id) {
	switch (weapon_id) {
		case WeaponId::HarakanFlamethrower:				return ImageId::HarakanFlamethrower;
		case WeaponId::MatadorTwinstrikeFlamethrowers:	return ImageId::HarakanFlamethrower;
		case WeaponId::DurendalAbaddonFlamethrowers:	return ImageId::HarakanFlamethrower;
		case WeaponId::StrigaPhaseSaber:				return ImageId::PhaseLineBlast;
		case WeaponId::PariahIconoclastBlades:			return ImageId::PhaseLineBlast;
		case WeaponId::AckmedMickenMelter:				return ImageId::HarakanFlamethrower;
	}
	return ImageId::None;
}

bool weapon_id_can_proc_onhits(u32 weapon_id) {
	// A value of false makes the weapon ignore the following:
	// * Ancile Providence
	// * Clarion Phase Link
	// * Analogue Empathy Core
	// * Custom trigger damage overrides

	return weapon_id != WeaponId::HypnagogueThroughTheLookingGlass
		&& weapon_id != WeaponId::SpiderMines
		&& weapon_id != WeaponId::LobotomyMineExplosion
		&& weapon_id != WeaponId::Irradiate
		&& weapon_id != WeaponId::YamatoGun
		&& weapon_id != WeaponId::PsiStorm
		&& weapon_id != WeaponId::CantavisHallucinationCast
		&& weapon_id != WeaponId::MindTyrantTyranny
		&& weapon_id != WeaponId::MindTyrantMaelstrom
		&& weapon_id != WeaponId::StasisField
		&& weapon_id != WeaponId::MatralethParasite
		&& weapon_id != WeaponId::Plague
		&& weapon_id != WeaponId::Custom
		&& weapon_id != WeaponId::NuclearMissile;
}

WeaponOffsets* get_weapon_offsets(u32 weapon_id) {
	// NOTE: using per case static so we can return a pointer to valid memory, since stack memory is invalidated after return
	// TODO: rework to using a weapon_id WeaponOffsets table, with some sort of optionality check (maybe a bool?)
	switch (weapon_id) {
		case WeaponId::CyprianTwinRailguns: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueCyprianLaunchPos;
			offsets.count = 2;
			return &offsets;
		} break;
		case WeaponId::VultureFragmentationGrenade: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueVultureLaunchPos;
			offsets.count = 2;
			return &offsets;
		} break;
		case WeaponId::GoliathContenderMissiles: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueGoliathLaunchPos;
			offsets.count = 2;
			return &offsets;
		} break;
		case WeaponId::PhalanxArcliteShockCannon: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValuePhalanxSiegeModeLaunchPos;
			offsets.count = 1;
			offsets.use_special_overlay = true;
			return &offsets;
		} break;
		case WeaponId::RamessesPharoahCannons: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueRamessesLaunchPos;
			offsets.count = 2;
			return &offsets;
		} break;
		case WeaponId::CataphractLonginiusCannons: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueCataphractLaunchPos;
			offsets.count = 4;
			return &offsets;
		} break;
		case WeaponId::PaladinCrusaderMissiles: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValuePaladinLaunchPos;
			offsets.count = 5;
			return &offsets;
		} break;
		case WeaponId::ClaymoreHarpoonRailgun: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueClaymoreLaunchPos;
			offsets.count = 1;
			return &offsets;
		} break;
		case WeaponId::PenumbraErebolShockCannon: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValuePenumbraLaunchPos;
			offsets.count = 1;
			return &offsets;
		} break;
		case WeaponId::WraithBurstLasers: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueWraithLaunchPos;
			offsets.count = 2;
			return &offsets;
		} break;
		case WeaponId::SundogBlindsideRockets: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueSundogLaunchPos;
			offsets.count = 2;
			return &offsets;
		} break;
		case WeaponId::CentaurSunchaserMissiles: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueCentaurLaunchPos;
			offsets.count = 4;
			return &offsets;
		} break;
		case WeaponId::MagnetarNewtonianShockCannon: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueMagnetarLaunchPos;
			offsets.count = 4;
			return &offsets;
		} break;
		case WeaponId::PhobosArgosLaserBatteries: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValuePhobosLaunchPos;
			offsets.count = 4;
			return &offsets;
		} break;
		case WeaponId::HippogriffChimeraCannons: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueHippogriffLaunchPos;
			offsets.count = 4;
			return &offsets;
		} break;
		case WeaponId::HippogriffPegasusMissiles: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueHippogriffLaunchPos2;
			offsets.count = 6;
			offsets.use_special_overlay = true;
			return &offsets;
		} break;
		case WeaponId::SentinelCerberusRailgun: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueSentinelLaunchPos;
			offsets.count = 1;
			return &offsets;
		} break;
		case WeaponId::DiademIonCannon:
		case WeaponId::DiademHammerfallBeacon: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueIonCannonLaunchPos;
			offsets.count = 1;
			offsets.use_special_overlay = true;
			return &offsets;
		} break;
		case WeaponId::WardenPrismaticCannon: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueWardenLaunchPos;
			offsets.count = 1;
			return &offsets;
		} break;
		case WeaponId::DracadinPhaseDisruptor: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueDracadinLaunchPos;
			offsets.count = 1;
			return &offsets;
		} break;
		case WeaponId::CharlatanFerventBlaster: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueCharlatanLaunchPos;
			offsets.count = 1;
			return &offsets;
		} break;
		case WeaponId::OptectonSunsplitBeam: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueOptectonLaunchPos1;
			offsets.count = 1;
			return &offsets;
		} break;
		case WeaponId::PositronAntimatterMissile: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValuePositronLaunchPos;
			offsets.count = 1;
			return &offsets;
		} break;
		case WeaponId::AccantorPlasmaShell: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueAccantorLaunchPos;
			offsets.count = 1;
			return &offsets;
		} break;
		case WeaponId::PanoptusPlasmaMissiles: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValuePanoptusLaunchPos;
			offsets.count = 2;
			return &offsets;
		} break;
		case WeaponId::ExemplarPlasmaTorpedo: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueExemplarLaunchPos;
			offsets.count = 1;
			return &offsets;
		} break;
		case WeaponId::EpigraphLitanyCannons: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueEpigraphLaunchPos;
			offsets.count = 4;
			return &offsets;
		} break;
		case WeaponId::EmpyreanPlasmaBarrage: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueEmpyreanLaunchPos;
			offsets.count = 4;
			return &offsets;
		} break;
		case WeaponId::StarSovereignCelestialCannon: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueSovereignLaunchPos;
			offsets.count = 2;
			return &offsets;
		} break;
		case WeaponId::MatrixKhaydarinRay: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueWardenLaunchPos;
			offsets.count = 4;
			return &offsets;
		} break;
		case WeaponId::SpraithColonySeekerSpores: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueSpraithColonyLaunchPos;
			offsets.count = 1;
			return &offsets;
		} break;
		case WeaponId::LiiraliskHollowedSpores: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueLiiraliskLaunchPos;
			offsets.count = 2;
			return &offsets;
		} break;
		case WeaponId::KalkaliskViscousAcid: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueKalkaliskLaunchPos;
			offsets.count = 1;
			return &offsets;
		} break;
		case WeaponId::AxitriliskSearingVenom: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueAxitriliskLaunchPos;
			offsets.count = 1;
			return &offsets;
		} break;
		case WeaponId::SkithrokorSkitteringSpores: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueSkithrokorLaunchPos;
			offsets.count = 1;
			return &offsets;
		} break;
		case WeaponId::BactaliskCorrosiveAcid: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueBactaliskLaunchPos;
			offsets.count = 2;
			return &offsets;
		} break;
		case WeaponId::CherbrathalorVenemousBursts: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueCherbrathalorLaunchPos;
			offsets.count = 4;
			return &offsets;
		} break;
		case WeaponId::KeskathalorScreamingVenom: {
			static WeaponOffsets offsets{};
			offsets.value_id = ValueKeskathalorLaunchPos;
			offsets.count = 2;
			offsets.fake_count = 1;
			return &offsets;
		} break;
	}
	return nullptr;
}

bool unit_id_spawns_larva_by_order(u32 unit_id) {
	return unit_id == UnitId::ZergHachirosk
		|| unit_id == UnitId::ZergCaphrolosk
		|| unit_id == UnitId::ZergGathtelosk
		|| unit_id == UnitId::ZergLarvoskCesiant
		|| unit_id == UnitId::ZergOcsalCesiant;
}

u32 get_scribe_warp_distance(u32 unit_id) {
	switch (unit_id) {
		case UnitId::ProtossAquifer:
		case UnitId::ProtossArdentAuthority: {
			return 88;
		}
		case UnitId::ProtossGrandLibrary:
		case UnitId::ProtossProstrationStage:
		case UnitId::ProtossSyntheticSynod:
		case UnitId::ProtossArgosy: {
			return 99;
		}
		default: {
			return 70;
		}
	}
}

bool unit_id_ignored_on_minimap(u32 unit_id) {
	return unit_id_is_spell(unit_id) || units_dat::base_property[unit_id] & UnitProperty::Subunit;
}

bool unit_is_combatant(Unit* unit) {
	switch (unit->id) {
		// TODO: add combat casters here
		case UnitId::ZergLakizilisk:
			return true;
	}
	if (unit->subunit && unit->subunit->is_subunit()) {
		return unit->subunit->has_weapon();
	} else {
		return unit->has_weapon();
	}
}

bool unit_id_can_repair(u32 unit_id) {
	return unit_id == UnitId::TerranMason
		|| unit_id == UnitId::TerranTalos
		|| unit_id == UnitId::DanielsMason;
}

bool weapon_id_is_flamethrower(u32 weapon_id) {
	return weapon_id == WeaponId::HarakanFlamethrower
		|| weapon_id == WeaponId::MatadorTwinstrikeFlamethrowers
		|| weapon_id == WeaponId::DurendalAbaddonFlamethrowers
		|| weapon_id == WeaponId::AckmedMickenMelter;
}

bool unit_id_is_larva(u32 unit_id) {
	return unit_id == UnitId::ZergLarva || unit_id == UnitId::ZergOcsalLarva;
}

u32 get_mutate_image_id(u32 unit_id) {
	switch (unit_id) {
		case UnitId::ZergQuazilisk:		return ImageId::QuaziliskMutate;
		case UnitId::ZergNathrokor:		return ImageId::NathrokorMutate;
		case UnitId::ZergCikralisk:		return ImageId::CikraliskMutate;
		case UnitId::ZergProtathalor:	return ImageId::ProtathalorMutate;
		case UnitId::ZergIzirokor:		return ImageId::IzirokorMutate;
		case UnitId::ZergSkithrokor:	return ImageId::SkithrokorMutate;
		case UnitId::ZergKagralisk:		return ImageId::KagraliskMutate;
		case UnitId::ZergEvigrilisk:	return ImageId::EvigriliskMutate;
		case UnitId::ZergAlmaksalisk:	return ImageId::AlmaksaliskMutate;
		case UnitId::ZergGorgrokor:		return ImageId::GorgrokorMutate;
		case UnitId::ZergVilgorokor:	return ImageId::VilgorokorMutate;
		case UnitId::ZergGeszithalor:	return ImageId::GeszithalorMutate;
	}
	return ImageId::None;
}

bool unit_is_tower(Unit* unit) {
	if (units_dat::base_property[unit->id] & UnitProperty::Building && unit_is_combatant(unit)) {
		return true;
	}
	return false;
}

bool unit_applies_ensnare(Unit* unit) {
	switch (unit->id) {
		case UnitId::ZergVorvrokor:
		case UnitId::ZergAkistrokor: {
			return true;
		} break;
		default: {
			UnitExt* unit_ext = get_unit_ext(unit);
			if (unit_ext->timers[TimerEnsnaringBrood] > 0) {
				return true;
			}
		} break;
	}
	return false;
}

bool unit_can_be_ensnared(Unit* unit) {
	if (units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding || unit_is_tower(unit)) {
		return true;
	}
	if (units_dat::base_property[unit->id] & UnitProperty::Building) {
		return false;
	}
	return true;
}

bool weapon_id_is_line_attack(u32 weapon_id) {
	return weapon_id == WeaponId::StrigaPhaseSaber
		|| weapon_id == WeaponId::PariahIconoclastBlades
		|| weapon_id_is_flamethrower(weapon_id);
}

u32 get_splash_width_from_weapon_id(u32 weapon_id) {
	switch (weapon_id) {
		case WeaponId::PsiStorm: {
			return 5 * 32;
		}
		case WeaponId::NuclearMissile: {
			return 8 * 32;
		}
		default:  {
			return 5 * 32;
		//	return weapons_dat::outer_splash_radius[weapon_id];
		}
	}
}

u32 get_splash_height_from_weapon_id(u32 weapon_id) {
	switch (weapon_id) {
		case WeaponId::PsiStorm: {
			return 4 * 32;
		}
		case WeaponId::NuclearMissile: {
			return 7 * 32;
		}
		default:  {
			return 5 * 32;
		//	return weapons_dat::outer_splash_radius[weapon_id];
		}
	}
}

bool weapon_id_bounces_caustic_fission(u32 weapon_id) {
	return weapon_id == WeaponId::BactaliskCorrosiveAcid
		|| weapon_id == WeaponId::AlkajeliskPlasmidSpores;
}

u32 get_death_sfx_id(u32 unit_id) {
	switch (unit_id) {
		case UnitId::ZergGosvileth:		return SoundId::Zerg_DRONE_ZDrDth00_WAV;
		case UnitId::ZergIroleth:		return SoundId::Zerg_OVERLORD_ZOvDth00_WAV;
		case UnitId::ZergVorvrokor:		return SoundId::VorvrokorDeath01;
		case UnitId::ZergLiiralisk:		return SoundId::Zerg_Hydra_ZHyDth00_WAV;
		case UnitId::ZergKalkalisk:		return SoundId::Zerg_Mutalid_ZMuDth00_WAV;
		case UnitId::ZergGhitorokor:	return SoundId::Zerg_Hydra_ZHyDth00_WAV;
		case UnitId::ZergTosgrilisk:	return SoundId::Zerg_Mutalid_ZMuDth00_WAV;
		case UnitId::ZergBactalisk:		return SoundId::BactaliskDeath01;
		case UnitId::ZergTethzorokor:	return SoundId::Zerg_Ultra_ZUlDth00_WAV;
		case UnitId::ZergVithrilisk:	return SoundId::Zerg_Devourer_ZDvDth00_WAV;
		case UnitId::ZergAlkajelisk:	return SoundId::Zerg_Guardian_ZGuDth00_WAV;
		case UnitId::ZergUltrokor:		return SoundId::Zerg_Ultra_ZUlDth00_WAV;
		case UnitId::ZergZarcavrokor:	return SoundId::Zerg_Guardian_ZGuDth00_WAV;
		case UnitId::ZergTetcaerokor:	return SoundId::Zerg_Hydra_ZHyDth00_WAV;
		case UnitId::ZergAkistrokor:	return SoundId::Zerg_Queen_ZQuDth00_WAV;

		default:						return SoundId::None;
	}
}

u32 weapon_id_is_secondary(u32 weapon_id) {
	return weapon_id == WeaponId::DurendalPhilistineAutogun
		|| weapon_id == WeaponId::TarasqueAcceleratedPlasmid
		|| weapon_id == WeaponId::OptectonSkybondRay;
}

bool unit_id_has_custom_air_weapon(u32 unit_id) {
	return unit_id == UnitId::TerranDurendal
		|| unit_id == UnitId::ZergOkrimnilisk;
}

bool weapon_id_returns_to_attacker(u32 weapon_id) {
	return weapon_id == WeaponId::SkithrokorSkitteringSpores
		|| weapon_id == WeaponId::LakiziliskSubterraneanSpines;
}

bool weapon_id_is_incremental(u32 weapon_id) {
	return weapon_id == WeaponId::KalkaliskViscousAcid
		|| weapon_id == WeaponId::SkithrokorSkitteringSpores;
}

bool unit_id_has_proximity_cloak(u32 unit_id) {
	return unit_id == UnitId::TerranEidolon
		|| unit_id == UnitId::TerranWraith
		|| unit_id == UnitId::TerranSundog
		|| unit_id == UnitId::Goska;
}

bool unit_id_is_ridge_refinery(u32 unit_id) {
	return false; // Will add new IDs later. -Pr0nogo
}

bool skip_military_tracking_by_unit_id(u32 unit_id) {
	return unit_id == UnitId::TerranMorningstar
		|| unit_id == UnitId::ProtossSteward
		|| unit_id_is_trap(unit_id);
}

bool unit_id_is_trap(u32 unit_id) {
	return unit_id == UnitId::TerranSpiderMine
		|| unit_id == UnitId::TerranLobotomyMine;
}

bool unit_building_can_act(Unit* unit) {
	if (unit->status & UnitStatus::InAir ||
		unit->status & UnitStatus::IsHallucination ||
		!(unit->status & UnitStatus::Completed)) {
		return false;
	}
	return true;
}

bool unit_could_spawn_custom_larva(Unit* unit) {
	if (unit_id_spawns_larva_by_order(unit->id)) {
		return false;
	}
	UnitExt* unit_ext = get_unit_ext(unit);
	switch (unit->id) {
		case UnitId::Omnivale: {
			if (g_globals->players[unit->player].game_mode_race != GameModeRace_TerranProtoss) {
				return true;
			}
		} break;
	}
	return false;
}

bool unit_spawns_custom_larva(Unit* unit) {
	if (unit->status & UnitStatus::InAir || !(unit->status & UnitStatus::Completed)) {
		return false;
	}
	return unit_could_spawn_custom_larva(unit);
}

bool unit_could_spread_custom_creep(Unit* unit) {
	if (helper::spreads_creep(unit->id, true)) {
		return false;
	}
	UnitExt* unit_ext = get_unit_ext(unit);
	switch (unit->id) {
		case UnitId::Omnivale: {
			if (g_globals->players[unit->player].game_mode_race != GameModeRace_TerranProtoss) {
				return true;
			}
		} break;
	}
	if (unit_ext->values[ValueInfested] && unit->status & UnitStatus::GroundedBuilding) {
		return true;
	}
	if (unit_ext->values[ValueUprisingSpawner] && unit->get_race() == RaceId::Zerg) {
		return true;
	}
	return false;
}

bool unit_spreads_custom_creep(Unit* unit) {
	if (unit->status & UnitStatus::InAir || !(unit->status & UnitStatus::Completed)) {
		return false;
	}
	return unit_could_spread_custom_creep(unit);
}

bool unit_id_should_give_gas_in_massacre_mode(u32 unit_id) {
	return   unit_id != UnitId::ProtossSteward
		&&   unit_id != UnitId::ZergLarva
		&&   unit_id != UnitId::ZergOcsalLarva
		&&   unit_id != UnitId::ZergEgg
		&&   unit_id != UnitId::ZergCocoon
		&&   unit_id != UnitId::Special_MapRevealer
		&&  !unit_id_is_spell(unit_id)
		&& !(units_dat::base_property[unit_id] & UnitProperty::Subunit)
		&& !unit_id_is_resource_node(unit_id, false);
}

bool rapid_rendezvous_can_target_unit_id(u32 unit_id) { // Does not prohibit air units, must be done separately
	return !unit_id_is_resource_node(unit_id, false)
		&&	unit_id != UnitId::ZergLarva
		&&	unit_id != UnitId::ZergOcsalLarva
		&&	unit_id != UnitId::ZergEgg;
}

bool unit_id_ignores_ground_collision(u32 unit_id) {
	return unit_id_is_larva(unit_id) || unit_id == UnitId::ZergRilirokor;
}

bool unit_id_has_projection_attack(u32 unit_id) {
	return unit_id == UnitId::TerranHarakan
		|| unit_id == UnitId::TerranStriga
		|| unit_id == UnitId::TerranMatadorCinderMode
		|| unit_id == UnitId::TerranMatadorTurretCinderMode
		|| unit_id == UnitId::TerranDurendal
		|| unit_id == UnitId::ProtossPariah
		|| unit_id == UnitId::Ackmed;
}

bool player_id_is_extended(u32 player_id) {
	if (player_id > PlayerLimit) {
		return true;
	}
	return false;
}

bool order_id_is_build_order(u32 order_id) {
	return	order_id == OrderId::BuildTerran
	||		order_id == OrderId::BuildProtoss1
	||		order_id == OrderId::DroneLand
	||		order_id == OrderId::DroneBuild
	||		order_id == OrderId::DroneStartBuild;
}

bool unit_id_has_custom_weapon_handling(u32 unit_id) { // TODO: better name 
	return unit_id == UnitId::TerranWyvern
		|| unit_id == UnitId::TerranWyvernTurret
		|| unit_id == UnitId::ProtossAnthelion;
}

bool unit_id_is_burrowed_attacker(u32 unit_id) {
	return unit_id == UnitId::ZergLakizilisk;
}

bool buttonset_id_is_faction_selection(u32 buttonset_id) {
	return buttonset_id == UnitId::Buttons_FactionSelect
		|| buttonset_id == UnitId::Buttons_FactionSelectProtoss
		|| buttonset_id == UnitId::Buttons_FactionSelectZerg;
}

bool portrait_id_is_default_advisor(u32 portrait_id) {
	return portrait_id == PortraitId::AdvisorOrionImperium
		|| portrait_id == PortraitId::ProtossAdvisor
		|| portrait_id == PortraitId::AdvisorAtaltreth;
}

u32 get_player_portrait_id_override(u32 portrait_id, u32 player) {
	if (player >= PLAYABLE_PLAYER_COUNT) {
		return portrait_id;
	}
	const auto faction = g_globals->players[player].faction;
	switch (portrait_id) {
		case PortraitId::AdvisorOrionImperium: {
			switch (faction) {
				case Faction::TerranHecatonCoalition:		return PortraitId::HADRAL; // Temporary
				case Faction::TerranFuturistFederation:		return PortraitId::AdvisorFuturistFederation;
				case Faction::TerranGoldenRepublic:			return PortraitId::AdvisorGoldenRepublic;
				case Faction::TerranNydusTrinity:			return PortraitId::AdvisorNydusTrinity;
				case Faction::TerranUpstart:				return PortraitId::AdvisorUpstartTerran;
			}
		} break;
		case PortraitId::ProtossAdvisor: {
			switch (faction) {
				case Faction::ProtossOrderOfTheSacredSaber: return PortraitId::Patriarch; // Temporary
				case Faction::ProtossStarbound:             return PortraitId::AdvisorStarbound;
			}
		} break;
		case PortraitId::AdvisorAtaltreth: {
			switch (faction) {
				case Faction::ZergImicrenTendril:			return PortraitId::AdvisorImicren;
				case Faction::ZergZorthosTendril:			return PortraitId::AdvisorZorthos;
			}
		} break;
	}
	return portrait_id;
}

bool unit_id_is_tiny_unit(u32 unit_id) {
	return unit_id == UnitId::TerranLobotomyMine
		|| unit_id == UnitId::ZergRilirokor;
}

bool unit_id_is_spell(u32 unit_id) {
	return unit_id == UnitId::Spell_CarapaceSwarm
		|| unit_id == UnitId::Spell_CarapaceSwarmSmall
		|| unit_id == UnitId::Spell_ScannerSweep;
}

bool unit_id_has_auto_healing(u32 unit_id) {
	return unit_id == UnitId::TerranCleric; // In case we want to add more later. -Pr0nogo
}

bool unit_id_is_valid_for_most_abilities(u32 unit_id) {
	return unit_id != UnitId::ZergLarva
		&& unit_id != UnitId::ZergOcsalLarva
		&& unit_id != UnitId::ZergEgg
		&& unit_id != UnitId::ZergCocoon
		&& unit_id != UnitId::ProtossSteward;
}

bool unit_id_is_hero(u32 unit_id) {
	return unit_id == UnitId::Goska || unit_id == UnitId::Stroluum;
}