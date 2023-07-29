#include "cmbw_ids.h"
#include "SCBW\api.h"
#include "hooks\weapons\weapon_fire.h"

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
	return (units_dat::base_property[unit_id] & UnitProperty::Addon &&
		(unit_id != UnitId::TerranFountainhead && unit_id != UnitId::TerranAtelier && unit_id != UnitId::TerranHadron))
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
		|| unit_id == UnitId::ZergOcsalCesiant
		|| unit_id == UnitId::ZergNydusCesiant;
}

b32 unit_id_is_tech(u32 unit_id) {
	if (units_dat::base_property[unit_id] & UnitProperty::Addon) {
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
		case UnitId::TerranFountainhead:
		case UnitId::TerranDaedala:
		case UnitId::TerranAtelier:

			// Zerg
		case UnitId::ZergIrolIris:
		case UnitId::ZergIrulentIvoa:
		case UnitId::ZergOthstolOviform:
		case UnitId::ZergQuazrokPool:
		case UnitId::ZergLiivralPond:
		case UnitId::ZergMuthrokSpire:
		case UnitId::ZergVithrathHaunt:
		case UnitId::ZergAxtothAxis:
		case UnitId::ZergBegrozithBarrow:
		case UnitId::ZergHydrokDen:
		case UnitId::ZergGorgralSwamp:
		case UnitId::ZergIzkagIteth:
		case UnitId::ZergChokralgantChazsal:
		case UnitId::ZergGeszkathGrotto:
		case UnitId::ZergAlkajChasm:
		case UnitId::ZergZorkizShroud:
		case UnitId::ZergUltrokCavern:
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
		case UnitId::TerranFountainhead:
		case UnitId::TerranDaedala:
		case UnitId::TerranAtelier:

			// Zerg
		case UnitId::ZergIrolIris:
		case UnitId::ZergOthstolOviform:
			return true;
	}
	return false;
}

bool unit_id_is_transport(u32 unit_id) {
	return unit_id == UnitId::TerranTrojan
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
		|| unit_id == UnitId::ProtossAstralOmen
		|| unit_id == UnitId::ProtossArgosy
		|| unit_id == UnitId::ProtossFleetBeacon
		|| unit_id == UnitId::ProtossMonumentOfSin;
}

bool unit_id_can_regenerate_shields(u32 unit_id) {
	return unit_id != UnitId::ProtossVagrantShade;
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

bool unit_id_is_ground_bunker(u32 unit_id) {
	return unit_id == UnitId::TerranAnchor;
	//	|| unit_id == UnitId::TerranTencendur;
}

bool unit_id_is_bunker(u32 unit_id) {
	return unit_id_is_ground_bunker(unit_id); // Will add more bunker types later. -Pr0nogo
}

bool unit_id_is_melee_air_unit(u32 unit_id) {
	return unit_id == UnitId::ZergNathrokor
		|| unit_id == UnitId::ZergGorgrokor
		|| unit_id == UnitId::ZergTetcaerokor
		|| unit_id == UnitId::ZergAkistrokor
		|| unit_id == UnitId::ZergZarcavrokor;
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
		|| unit_id == UnitId::TerranWellbore;
}

bool unit_id_is_siege_mode(u32 unit_id) {
	return unit_id == UnitId::TerranShamanVisionaryMode
		|| unit_id == UnitId::TerranMatadorCinderMode
		|| unit_id == UnitId::TerranPhalanxSiegeMode
		|| unit_id == UnitId::TerranMadrigalFuriaMode
		|| unit_id == UnitId::TerramCuirassTrenchMode;
}

bool unit_id_is_tank_mode(u32 unit_id) {
	return unit_id == UnitId::TerranShamanSupportMode
		|| unit_id == UnitId::TerranMatadorTankMode
		|| unit_id == UnitId::TerranPhalanxTankMode
		|| unit_id == UnitId::TerranMadrigalImperiosoMode
		|| unit_id == UnitId::TerranCuirassTankMode;
}

bool unit_id_is_broodspawn(u32 unit_id) {
	return unit_id == UnitId::ZergRilirokor
		|| unit_id == UnitId::ZergRililisk;
}

bool unit_id_is_zerg_production(u16 unit_id, bool town_center_only) {
	if (town_center_only) {
		return unit_id == UnitId::ZergHachirosk
			|| unit_id == UnitId::ZergCaphrolosk
			|| unit_id == UnitId::ZergGathtelosk;
	} else {
		return unit_id == UnitId::ZergHachirosk
			|| unit_id == UnitId::ZergCaphrolosk
			|| unit_id == UnitId::ZergGathtelosk
			|| unit_id == UnitId::ZergLarvoskCesiant
			|| unit_id == UnitId::ZergOcsalCesiant;
	}
}

bool unit_id_is_town_center(u16 unit_id) {
	return unit_id == UnitId::TerranMinistry
		|| unit_id == UnitId::ProtossNexus
		|| unit_id == UnitId::ZergHachirosk
		|| unit_id == UnitId::ZergCaphrolosk
		|| unit_id == UnitId::ZergGathtelosk;
}

bool unit_id_is_factional(u16 unit_id) {
	return unit_id == UnitId::TerranAspirant
		|| unit_id == UnitId::TerranCohort
		|| unit_id == UnitId::TerranLuminary
		|| unit_id == UnitId::TerranTarasque;
}

bool unit_id_uses_xlarge_overlays(u16 unit_id) {
	return unit_id == UnitId::TerranPenumbra
		|| unit_id == UnitId::TerranPhobos
		|| unit_id == UnitId::ProtossStarSovereign
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

bool unit_regens_in_combat(Unit* unit) {
	return false; // Can update later with status checks and etc. -Pr0nogo
}
bool unit_paths_over_small_units(Unit* unit) {
	// Keeping arg as unit in case we want to add status effects later. -Pr0nogo
	return unit->id == UnitId::TerranPenumbra;
}

bool unit_collides_with_titanic(Unit* unit) {
	if (unit->status & UnitStatus::GroundedBuilding) {
		return unit->id != UnitId::ProtossPylon && !unit_id_is_minerals(unit->id);
	}
	return unit->id == UnitId::TerranClaymore
		|| unit->id == UnitId::TerranPenumbra
		|| unit->id == UnitId::TerranAion
		|| unit->id == UnitId::ProtossOptecton
		|| unit->id == UnitId::ProtossDemiurge
		|| unit->id == UnitId::ZergKeskathalor
		|| unit->id == UnitId::ZergKethrostalor
		|| unit->id == UnitId::ZergUltrokor;
}

bool unit_has_fore_castle(Unit* unit) {
	switch (unit->id) {
		case UnitId::TerranMinotaur: {
			return true;
		}
	}
	UnitExt* unit_ext = get_unit_ext(unit);
	if (unit_ext->values[ValueHasSublimeShepherd]) {
		return true;
	}
	return false;
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
		|| weapon_id == WeaponId::TosgriliskExplosiveSpores
		|| weapon_id == WeaponId::SpirtithColonySpirtilSpores;
}

bool weapon_id_accelerates(u32 weapon_id) {
	return weapon_id == WeaponId::EmpyreanPlasmaBarrage;
}

bool weapon_id_is_melee_aoe(u32 weapon_id) {
	return weapon_id == WeaponId::NathrokorPlasmidBreath
		|| weapon_id == WeaponId::GorgrokorSerratedCleavers
		|| weapon_id == WeaponId::ZarcavrokorBisectingBlades
		|| weapon_id == WeaponId::TethzorokorSaberArray
		|| weapon_id == WeaponId::TetcaerokorCleavingMaw;
}

u32 weapon_id_intercepts(u32 weapon_id) {
	switch (weapon_id) {
		case WeaponId::OptectonSunsplitBeam:
		case WeaponId::AccantorPlasmaShell:
			return InterceptType_GroundOnly;

		case WeaponId::ValkyrieHaloRockets:
		case WeaponId::EmpyreanPlasmaBarrage:
			return InterceptType_AirOnly;

		case WeaponId::SirenCharybdalRifle:
		case WeaponId::PatriarchIronhandWeaver:
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
		case WeaponId::MadrigalImperiosoMissiles:
		case WeaponId::RamessesPharoahCannons:
		case WeaponId::ClaymoreHarpoonRailgun:
		case WeaponId::PenumbraErebolShockCannon:
		case WeaponId::MagnetarNewtonianShockCannon:
		case WeaponId::HippogriffChimeraCannons:
		case WeaponId::SentinelCerberusRailgun:
		case WeaponId::KalkaliskViscousAcid:
		case WeaponId::ZarcavrokorBonesofReflection:
			return true;
	}
	return false;
}

bool weapon_id_piercing_stops_on_armor(u32 weapon_id) {
	switch (weapon_id) {
		case WeaponId::PenumbraErebolShockCannon:
			return false;
	}
	return true;
}

bool weapon_is_melee_with_aoe(Bullet* bullet, u32 weapon_id) {
	if (bullet && bullet->source_unit) {
		switch (weapon_id) {
			case WeaponId::GolemBloodlessAssault:
			case WeaponId::NathrokorPlasmidBreath:
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

bool tech_id_needs_target(u32 tech_id) {
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
		|| image_id == ImageId::YoungChrysalis
		|| image_id == ImageId::DataDisc
		|| image_id == ImageId::Uraj
		|| image_id == ImageId::Khalis;
}

ImageId::Enum analogue_image_id_overwrite(u32 weapon_id) {
	switch (weapon_id) {
		case WeaponId::HarakanFlamethrower:				return ImageId::HarakanFlamethrower;
		case WeaponId::MatadorTwinstrikeFlamethrowers:	return ImageId::HarakanFlamethrower;
		case WeaponId::DurendalAbaddonFlamethrowers:	return ImageId::HarakanFlamethrower;
		case WeaponId::StrigaPhaseSaber:				return ImageId::PhaseLineBlast;
		case WeaponId::PariahIconoclastBlades:			return ImageId::PhaseLineBlast;
	}
	return ImageId::None;
}

bool weapon_id_procs_onhits(u32 weapon_id) {
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
		&& weapon_id != WeaponId::Custom;
}

WeaponOffsets get_weapon_offsets(u32 weapon_id)
{
	WeaponOffsets offsets = {};
	switch (weapon_id)
	{
		case WeaponId::CyprianTwinRailguns:
		{
			offsets.value_id = ValueCyprianLaunchPos;
			offsets.count = 2;
		} break;
		case WeaponId::GoliathContenderMissiles:
		{
			offsets.value_id = ValueGoliathLaunchPos;
			offsets.count = 2;
		} break;
		case WeaponId::PhalanxArcliteShockCannon:
		{
			offsets.value_id = ValuePhalanxSiegeModeLaunchPos;
			offsets.count = 1;
			offsets.use_special_overlay = true;
		} break;
		case WeaponId::RamessesPharoahCannons:
		{
			offsets.value_id = ValueRamessesLaunchPos;
			offsets.count = 2;
		} break;
		case WeaponId::ClaymoreHarpoonRailgun:
		{
			offsets.value_id = ValueClaymoreLaunchPos;
			offsets.count = 1;
		} break;
		case WeaponId::PenumbraErebolShockCannon:
		{
			offsets.value_id = ValuePenumbraLaunchPos;
			offsets.count = 1;
		} break;
		case WeaponId::WraithBurstLasers:
		{
			offsets.value_id = ValueWraithLaunchPos;
			offsets.count = 2;
			offsets.fake_count = 1;
		} break;
		case WeaponId::SundogBlindsideRockets:
		{
			offsets.value_id = ValueSundogLaunchPos;
			offsets.count = 4;
		} break;
		case WeaponId::CentaurSunchaserMissiles:
		{
			offsets.value_id = ValueCentaurLaunchPos;
			offsets.count = 4;
		} break;
		case WeaponId::PhobosArgosLaserBatteries:
		{
			offsets.value_id = ValuePhobosLaunchPos;
			offsets.count = 4;
		} break;
		case WeaponId::SentinelCerberusRailgun:
		{
			offsets.value_id = ValueSentinelLaunchPos;
			offsets.count = 1;
		} break;
		case WeaponId::DiademIonCannon:
		case WeaponId::DiademHammerfallBeacon:
		{
			offsets.value_id = ValueIonCannonLaunchPos;
			offsets.count = 1;
			offsets.use_special_overlay = true;
		} break;
		case WeaponId::WardenPrismaticCannon:
		{
			offsets.value_id = ValueWardenLaunchPos;
			offsets.count = 1;
		} break;
		case WeaponId::CharlatanFerventBlaster:
		{
			offsets.value_id = ValueCharlatanLaunchPos;
			offsets.count = 1;
		} break;
		case WeaponId::OptectonSunsplitBeam:
		{
			offsets.value_id = ValueOptectonLaunchPos1;
			offsets.count = 1;
		} break;
		case WeaponId::PositronAntimatterMissile:
		{
			offsets.value_id = ValuePositronLaunchPos;
			offsets.count = 1;
		} break;
		case WeaponId::AccantorPlasmaShell:
		{
			offsets.value_id = ValueAccantorLaunchPos;
			offsets.count = 1;
		} break;
		case WeaponId::PanoptusPlasmaMissiles:
		{
			offsets.value_id = ValuePanoptusLaunchPos;
			offsets.count = 2;
		} break;
		case WeaponId::ExemplarPlasmaTorpedo:
		{
			offsets.value_id = ValueExemplarLaunchPos;
			offsets.count = 1;
		} break;
		case WeaponId::EpigraphLitanyCannons:
		{
			offsets.value_id = ValueEpigraphLaunchPos;
			offsets.count = 4;
		} break;
		case WeaponId::EmpyreanPlasmaBarrage:
		{
			offsets.value_id = ValueEmpyreanLaunchPos;
			offsets.count = 4;
		} break;
		case WeaponId::StarSovereignCelestialCannon:
		{
			offsets.value_id = ValueSovereignLaunchPos;
			offsets.count = 2;
		} break;
		case WeaponId::SpraithColonySeekerSpores:
		{
			offsets.value_id = ValueSpraithColonyLaunchPos;
			offsets.count = 1;
		} break;
		case WeaponId::LiiraliskHollowedSpores:
		{
			offsets.value_id = ValueLiiraliskLaunchPos;
			offsets.count = 2;
		} break;
		case WeaponId::KalkaliskViscousAcid:
		{
			offsets.value_id = ValueKalkaliskLaunchPos;
			offsets.count = 1;
		} break;
		case WeaponId::AxitriliskSearingVenom:
		{
			offsets.value_id = ValueAxitriliskLaunchPos;
			offsets.count = 1;
		} break;
		case WeaponId::SkithrokorSkitteringSpores:
		{
			offsets.value_id = ValueSkithrokorLaunchPos;
			offsets.count = 1;
		} break;
		case WeaponId::BactaliskCorrosiveAcid:
		{
			offsets.value_id = ValueBactaliskLaunchPos;
			offsets.count = 2;
		} break;
		case WeaponId::KeskathalorScreamingVenom:
		{
			offsets.value_id = ValueKeskathalorLaunchPos;
			offsets.count = 2;
			offsets.fake_count = 1;
		} break;
		case WeaponId::CataphractLonginiusCannons:
		{
			offsets.value_id = ValueCataphractLaunchPos;
			offsets.count = 4;
		} break;
		case WeaponId::PaladinCrusaderMissiles:
		{
			offsets.value_id = ValuePaladinLaunchPos;
			offsets.count = 5;
		} break;
		case WeaponId::MatrixKhaydarinRay:
		{
			offsets.value_id = ValueWardenLaunchPos;
			offsets.count = 4;
		} break;
		default:
		{
			return {};
		}
	}
	return offsets;
}

bool unit_id_spawns_larva(u32 unit_id)
{
	return unit_id == UnitId::ZergHachirosk
		|| unit_id == UnitId::ZergCaphrolosk
		|| unit_id == UnitId::ZergGathtelosk
		|| unit_id == UnitId::ZergLarvoskCesiant
		|| unit_id == UnitId::ZergOcsalCesiant;
}

u32 get_scribe_warp_distance(u32 unit_id) {
	switch (unit_id)
	{
		case UnitId::ProtossAquifer:
		case UnitId::ProtossArdentAuthority:
		{
			return 88;
		}
		case UnitId::ProtossGrandLibrary:
		case UnitId::ProtossProstrationStage:
		case UnitId::ProtossSyntheticSynod:
		case UnitId::ProtossArgosy:
		{
			return 99;
		}
		default:
		{
			return 70;
		}
	}
}

bool unit_id_ignored_on_minimap(u32 unit_id) {
	switch (unit_id) {
		case UnitId::Spell_DarkSwarm:
		{
			return true;
		}
	}
	return units_dat::base_property[unit_id] & UnitProperty::Subunit;
}

bool unit_is_combatant(Unit* unit) {
	switch (unit->id) {
		// TODO: add combat casters here
		case UnitId::ZergLakizilisk:
			return true;
	}
	return unit->has_weapon();
}

bool unit_id_can_repair(u32 unit_id) {
	return unit_id == UnitId::TerranMason
		|| unit_id == UnitId::TerranTalos
		|| unit_id == UnitId::DanielsMason;
}

bool weapon_id_is_flamethrower(u32 weapon_id) {
	return weapon_id == WeaponId::HarakanFlamethrower
		|| weapon_id == WeaponId::MatadorTwinstrikeFlamethrowers
		|| weapon_id == WeaponId::DurendalAbaddonFlamethrowers;
}

bool unit_id_is_larva(u32 unit_id) {
	return unit_id == UnitId::ZergLarva || unit_id == UnitId::ZergOcsalLarva;
}

u32 get_mutate_image_id(u32 unit_id) {
	switch (unit_id)
	{
		case UnitId::ZergQuazilisk:
		{
			return ImageId::QuaziliskMutate;
		}
		default:
		{
			return 0;
		}
	}
}