#include "cm_ids.h"
#include "BW\api.h"
#include "hooks\weapons\weapon_fire.h"
#include "trigger_system/trigger_system.h"
#include "data_ext.h"

bool unit_id_is_geyser_refinery(u16 unit_id) {
    return unit_id == UnitId::TerranReservoir
        || unit_id == UnitId::TerranWellbore
        || unit_id == UnitId::ProtossAquifer
        || unit_id == UnitId::ZergExcisant
        || unit_id == UnitId::ZergEzigrisant
        || unit_id == UnitId::AskosiAbutment;
}

bool unit_id_is_capped_gas_node(u16 unit_id) {
    return unit_id_is_geyser_refinery(unit_id) || unit_id_is_ridge_refinery(unit_id);
}

bool unit_id_is_gas_node(u16 unit_id) {
    return unit_id_is_geyser_refinery(unit_id) || unit_id_is_ridge_refinery(unit_id) || unit_id_is_uncapped_gas_node(unit_id);
}

bool unit_id_is_uncapped_gas_node(u16 unit_id) {
    return unit_id == UnitId::ResourceVespeneRidge || unit_id == UnitId::ResourceVespeneGeyser;
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
        case UnitId::ZergKethrajKeep:
        case UnitId::ZergZorkizShroud:
        case UnitId::ZergUlgorgCavern:
        case UnitId::ZergZozarcGorge:
        case UnitId::ZergMatravilNest:
        case UnitId::ZergAlmakisAntre:
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
        || unit_id == UnitId::ZergYblyleth
        || unit_id == UnitId::ZergIroleth
        || unit_id == UnitId::ZergOthstoleth;
}

bool unit_id_is_race(u32 unit_id, u32 race) {
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
        case RaceId::Askosi: {
            if (UnitId::is_askosi(static_cast<UnitId::Enum>(unit_id))) {
                return true;
            }
        } break;
    }
    return false;
}

bool unit_id_is_combatant(u32 unit_id) {
    if (units_dat::base_property[unit_id] & UnitProperty::Spellcaster && !(units_dat::base_property[unit_id] & UnitProperty::Building)) {
        return true;
    }
    if (units_dat::ground_weapon[unit_id] != WeaponId::None || units_dat::air_weapon[unit_id] != WeaponId::None) {
        return true;
    }
    if (u32 subunit_id = units_dat::subunit[unit_id]; subunit_id != UnitId::None) {
        if (units_dat::ground_weapon[subunit_id] != WeaponId::None || units_dat::air_weapon[subunit_id] != WeaponId::None) {
            return true;
        }
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
        && unit_id != UnitId::TerranHarakanGolden
        && unit_id != UnitId::TerranAspirant
        && unit_id != UnitId::TerranBlackjackGolden
        && unit_id != UnitId::TerranLuminary;
}

bool unit_id_has_shield_regeneration(u32 unit_id) {
    return unit_id == UnitId::TerranHarakanGolden
        || unit_id == UnitId::TerranAspirant
        || unit_id == UnitId::TerranBlackjackGolden
        || unit_id == UnitId::TerranLuminary;
}

bool unit_id_is_stackable_building(u32 unit_id) {
    return false; // Disabled to see if it improves AI base layouts. -Pr0nogo
    switch (unit_id) {
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
        case UnitId::ZergOthstolOviform: {
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
        case UnitId::TerranNaniteAssembly: {
            return true;
        } break;
    }
    return false;
}

u32 unit_id_get_larva_type(u32 unit_id) {
    switch (unit_id) {
        case UnitId::ZergCaphrolosk:
        case UnitId::ZergOcsalCesiant:
        case UnitId::ZergOcsalLarva: {
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
        || unit_id == UnitId::TerranSieve
        || unit_id == UnitId::TerranReservoir
        || unit_id == UnitId::TerranWellbore
        || unit_id == UnitId::ProtossCedent
        || unit_id == UnitId::ProtossAquifer
        || unit_id == UnitId::ZergEskigant
        || unit_id == UnitId::ZergExcisant
        || unit_id == UnitId::ZergEzigrisant
        || unit_id == UnitId::AskosiArch
        || unit_id == UnitId::AskosiAbutment;
}

bool unit_id_is_ground_garrison(u32 unit_id) {
    return unit_id == UnitId::TerranAnchor
        || unit_id == UnitId::TerranLodestone
        || unit_id == UnitId::TerranLodestoneStandalone
        || unit_id == UnitId::TerranTrenchfoot
        || unit_id == UnitId::TerranTencendur
        || unit_id == UnitId::ZergScrylyleth;
}

bool unit_id_is_garrison(u32 unit_id) {
    return unit_id_is_ground_garrison(unit_id)
        || unit_id == UnitId::AskosiGrandstandHoldout
        || unit_id == UnitId::AskosiPetract;
}

bool unit_id_is_melee_air_unit(u32 unit_id) {
    return unit_id == UnitId::ZergNathrokor
        || unit_id == UnitId::ZergGorgrokor
        || unit_id == UnitId::ZergAzanthrokor
        || unit_id == UnitId::ZergTetcaerokor
        || unit_id == UnitId::ZergAkistrokor
        || unit_id == UnitId::ZergZarcavrokor
        || unit_id == UnitId::ZergSkryrokor
        || unit_id == UnitId::TerranWendigo;
}

bool unit_id_is_morphed_from_double_unit(u32 unit_id) {
    return unit_id == UnitId::ZergVorvrokor
        || unit_id == UnitId::ZergGarvrokor
        || unit_id == UnitId::ZergLiiralisk
        || unit_id == UnitId::ZergKalkalisk
        || unit_id == UnitId::ZergGhitorokor
        || unit_id == UnitId::ZergTosgrilisk
        || unit_id == UnitId::ZergBegrothilisk;
}

bool unit_id_is_power_generator(u32 unit_id) {
    return unit_id_is_psi_provider(unit_id); // Not sure why these two are distinct, maybe psi referenced supply or something. Kept them both for now, may merge/delete later. -Pr0nogo
}

bool unit_id_is_psi_provider(u16 unit_id) {
    return unit_id == UnitId::ProtossNexus
        || unit_id == UnitId::ProtossPylon
        || unit_id == UnitId::ProtossCrucible
        || unit_id == UnitId::ProtossXenolith
        || unit_id == UnitId::ExtProtossTemple
        || unit_id == UnitId::ProtossWarpGateTheFall

        || unit_id == UnitId::ProtossAutocort;
}

bool unit_id_can_do_controlled_demolition(u32 unit_id) {
    return units_dat::base_property[unit_id] & UnitProperty::Addon
        || unit_id == UnitId::TerranSupplyDepot
        || unit_id == UnitId::TerranSieve
        || unit_id == UnitId::TerranReservoir
        || unit_id == UnitId::TerranWellbore
        || unit_id == UnitId::TerranLobotomyMine
        || unit_id == UnitId::TerranTartarusMine
        || unit_id == UnitId::TerranGoremast;
}

UnitId::Enum unit_id_to_siege_mode(UnitId::Enum unit_id) {
    switch (unit_id) {
        case UnitId::TerranSavantEntropyGauntlets:       return UnitId::TerranSavantPowerSiphon;
        case UnitId::TerranShamanSupportMode:            return UnitId::TerranShamanVisionaryMode;
        case UnitId::TerranMatadorTankMode:              return UnitId::TerranMatadorCinderMode;
        case UnitId::TerranPhalanxTankMode:              return UnitId::TerranPhalanxSiegeMode;
        case UnitId::TerranMadrigalImperiosoMode:        return UnitId::TerranMadrigalFuriaMode;
    //	case UnitId::TerranCuirassTankMode:              return UnitId::TerramCuirassTrenchMode;
    	case UnitId::TerranOgunWorkerMode:               return UnitId::TerranOgunSmithyMode;
    	case UnitId::TerranMadrigalHecatonImperiosoMode: return UnitId::TerranMadrigalHecatonFuriaMode;

        case UnitId::AskosiGrandstand:                   return UnitId::AskosiGrandstandVision;
        case UnitId::AskosiGrandstandVision:             return UnitId::AskosiGrandstandHoldout;
        case UnitId::AskosiGrandstandHoldout:            return UnitId::AskosiGrandstand;
    }
    return UnitId::None;
}

UnitId::Enum unit_id_to_tank_mode(UnitId::Enum unit_id) {
    switch (unit_id) {
        case UnitId::TerranSavantPowerSiphon:        return UnitId::TerranSavantEntropyGauntlets;
        case UnitId::TerranShamanVisionaryMode:      return UnitId::TerranShamanSupportMode;
        case UnitId::TerranMatadorCinderMode:        return UnitId::TerranMatadorTankMode;
        case UnitId::TerranPhalanxSiegeMode:         return UnitId::TerranPhalanxTankMode;
        case UnitId::TerranMadrigalFuriaMode:        return UnitId::TerranMadrigalImperiosoMode;
    //	case UnitId::TerramCuirassTrenchMode:        return UnitId::TerranCuirassTankMode;
        case UnitId::TerranOgunSmithyMode:           return UnitId::TerranOgunWorkerMode;
        case UnitId::TerranMadrigalHecatonFuriaMode: return UnitId::TerranMadrigalHecatonImperiosoMode;

        case UnitId::AskosiGrandstand:               return UnitId::AskosiGrandstandHoldout;
        case UnitId::AskosiGrandstandHoldout:        return UnitId::AskosiGrandstandVision;
        case UnitId::AskosiGrandstandVision:         return UnitId::AskosiGrandstand;
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
    }
    return unit_id == UnitId::ZergHachirosk
        || unit_id == UnitId::ZergCaphrolosk
        || unit_id == UnitId::ZergGathtelosk
        || unit_id == UnitId::ZergLarvoskCesiant
        || unit_id == UnitId::ZergOcsalCesiant
        || unit_id == UnitId::Omnivale;
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
        || unit_id == UnitId::TerranCannibal
        || unit_id == UnitId::TerranAspirant
        || unit_id == UnitId::TerranSouthpaw
        || unit_id == UnitId::TerranTarasque
        || unit_id == UnitId::TerranPhiloc
        || unit_id == UnitId::TerranValkyrieHecaton
        || unit_id == UnitId::TerranQuadriga
        || unit_id == UnitId::TerranWendigo
        || unit_id == UnitId::TerranQilin
        || unit_id == UnitId::TerranLuminary
        || unit_id == UnitId::TerranAuriel
        || unit_id == UnitId::TerranCyclopsFuturist
        || unit_id == UnitId::TerranHarakanGolden
        || unit_id == UnitId::TerranPietyWard
        || unit_id == UnitId::TerranClericFuturist
        || unit_id == UnitId::TerranBlackjackGolden
        || unit_id == UnitId::TerranEidolonNydus
        || unit_id == UnitId::TerranTiangou
        || unit_id == UnitId::TerranMaverickFuturist
        || unit_id == UnitId::TerranBlackguard
        || unit_id == UnitId::TerranArhaun
        || unit_id == UnitId::TerranMaverickUpstart
        || unit_id == UnitId::TerranVultureUpstart
        || unit_id == UnitId::TerranBroadside
        || unit_id == UnitId::TerranCutlass
        
        || unit_id == UnitId::ProtossPrudence
        || unit_id == UnitId::ProtossAdherent
        || unit_id == UnitId::ProtossExtatus
        || unit_id == UnitId::ProtossZealotStarbound
        || unit_id == UnitId::ProtossEcclesiastKhalendric
        || unit_id == UnitId::ProtossCicatus
        || unit_id == UnitId::ProtossRenesair
        || unit_id == UnitId::ProtossAshguard
        || unit_id == UnitId::ProtossDracadinOrder
        || unit_id == UnitId::ProtossManifoldStarbound

        || unit_id == UnitId::ZergYblyleth
        || unit_id == UnitId::ZergScrylyleth
        || unit_id == UnitId::ZergSovrokor
        || unit_id == UnitId::ZergGarvrokor
        || unit_id == UnitId::ZergZethalor
        || unit_id == UnitId::ZergNathrelisk
        || unit_id == UnitId::ZergIziralisk
        || unit_id == UnitId::ZergInzorthrokor
        || unit_id == UnitId::ZergQuazrokor
        || unit_id == UnitId::ZergLakiziliskZorthos
        || unit_id == UnitId::ZergHydraliskOrkoti
        || unit_id == UnitId::ZergAzanthrokor;
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
        || unit_id == UnitId::ZergAlkajelisk
        || unit_id == UnitId::ZergOkrimnilisk;
}

bool unit_id_is_resource_node(u16 unit_id, bool count_gas_structures) {
    if (count_gas_structures) {
        return unit_id == UnitId::ResourceMineralField
            || unit_id == UnitId::ResourceMineralFieldType2
            || unit_id == UnitId::ResourceMineralFieldType3
            || unit_id == UnitId::ResourceVespeneGeyser
            || unit_id == UnitId::ResourceVespeneRidge
            || unit_id == UnitId::TerranSieve
            || unit_id == UnitId::TerranReservoir
            || unit_id == UnitId::TerranWellbore
            || unit_id == UnitId::ProtossCedent
            || unit_id == UnitId::ProtossAquifer
            || unit_id == UnitId::ZergEskigant
            || unit_id == UnitId::ZergExcisant
            || unit_id == UnitId::ZergEzigrisant
            || unit_id == UnitId::AskosiArch
            || unit_id == UnitId::AskosiAbutment;
    }
    return unit_id == UnitId::ResourceMineralField
        || unit_id == UnitId::ResourceMineralFieldType2
        || unit_id == UnitId::ResourceMineralFieldType3
        || unit_id == UnitId::ResourceVespeneGeyser
        || unit_id == UnitId::ResourceVespeneRidge;
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
    if (unit->is_grounded_building()) {
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
        case WeaponEffect::Unknown_Crash: {
            return true;
        }
    }
    return false;
}

bool weapon_id_bounces_lithe_organelle(u32 weapon_id) {
    return weapon_id == WeaponId::MutaliskGlaveWurm
        || weapon_id == WeaponId::LiiraliskHollowedSpores
        || weapon_id == WeaponId::SpirtithCesiantSpirtilSpores;
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
        || weapon_id == WeaponId::GolemPhaseLances
        || weapon_id == WeaponId::QuazrokorQuazrokSpray
        || weapon_id == WeaponId::MortothrokorWickedIncisors
        || weapon_id == WeaponId::GorgrokorSerratedCleavers
        || weapon_id == WeaponId::ZarcavrokorBisectingBlades
        || weapon_id == WeaponId::TethzorokorSaberArray
        || weapon_id == WeaponId::TetcaerokorCleavingMaw
        || weapon_id == WeaponId::DoroninKorozzetBeastblades;
}

bool weapon_id_cleaves(u32 weapon_id) {
    return weapon_id == WeaponId::TethzorokorSaberArray
        || weapon_id == WeaponId::DoroninKorozzetBeastblades;
}

u32 weapon_id_intercepts(u32 weapon_id) {
    switch (weapon_id) {
        case WeaponId::OptectonSunsplitBeam:
        case WeaponId::AccantorPlasmaShell: {
            return InterceptType_GroundOnly;
        }
        case WeaponId::EmpyreanPlasmaBarrage: {
            return InterceptType_AirOnly;
        }
        case WeaponId::HotshotDispatchShotgun:
        case WeaponId::SirenCharybdalRifle:
        case WeaponId::CataphractLonginiusCannons:
        case WeaponId::EngramKhaydarinPlasmaLaser:
        case WeaponId::AtreusDauntlessDisruptorCannon:
        case WeaponId::CharlatanFerventBlaster:
        case WeaponId::CantavisHallucinationCast:
        case WeaponId::IdolPrismaticBlaster:
        case WeaponId::StarSovereignInterventionMissiles: {
            return InterceptType_Any;
        }
    }
    return InterceptType_None;
}

bool weapon_id_pierces(u32 weapon_id) {
    switch (weapon_id) {
        case WeaponId::CyprianTwinRailguns:
        case WeaponId::SavantDiscoidRailgunEntropy:
        case WeaponId::BlackguardAsoniCarbine:
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
        case WeaponId::ZergBonesofReflection: {
            return true;
        }
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
            case WeaponId::GolemPhaseLances:
            case WeaponId::QuazrokorQuazrokSpray:
            case WeaponId::MortothrokorWickedIncisors:
            case WeaponId::GorgrokorSerratedCleavers:
            case WeaponId::ZarcavrokorBisectingBlades:
            case WeaponId::TethzorokorSaberArray:
            case WeaponId::TetcaerokorCleavingMaw:
            case WeaponId::DoroninKorozzetBeastblades: {
                return true;
            }
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
        && tech_id != TechId::HypnagogueThroughTheLookingGlass
        && tech_id != TechId::AskosiRematerialize
        && tech_id != TechId::TritonLobotomyWarhead
        && tech_id != TechId::DivineDiscipline
        && tech_id != TechId::RejoinderOfService
        && tech_id != TechId::AskosiSalvage
        && tech_id != TechId::AskosiHijack
        && tech_id != TechId::AskosiRapidRendezvous
        && tech_id != TechId::AskosiRemoteHijack;
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
        case WeaponId::StrigaPhaseSaber:				return ImageId::StrigaAttackOverlay;
        case WeaponId::PariahIconoclastBlades:			return ImageId::PhaseLineBlast;
        case WeaponId::CikraliskAcidSalvo:			    return ImageId::CikraliskAttackOverlay;
        case WeaponId::AzanthrokorAzanthrokSpew:	    return ImageId::CikraliskAttackOverlay;
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
    return weapon_id != WeaponId::SpiderMines
        && weapon_id != WeaponId::Irradiate
        && weapon_id != WeaponId::YamatoGun
        && weapon_id != WeaponId::PsiStorm
        && weapon_id != WeaponId::CantavisHallucinationCast
        && weapon_id != WeaponId::MindTyrantMaledictus
        && weapon_id != WeaponId::MindTyrantMaelstrom
        && weapon_id != WeaponId::StasisField
        && weapon_id != WeaponId::MatralethParasite
        && weapon_id != WeaponId::Plague
        && weapon_id != WeaponId::Custom
        && weapon_id != WeaponId::NuclearMissile
        && weapon_id != WeaponId::RemoteHijack;
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
        case WeaponId::SavantDiscoidRailgunEntropy: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueSavantLaunchPos1;
            offsets.count = 1;
            return &offsets;
        } break;
        case WeaponId::SavantDiscoidRailgun: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueSavantLaunchPos2;
            offsets.count = 1;
            return &offsets;
        } break;
        case WeaponId::BlackguardAsoniCarbine: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueBlackguardLaunchPos;
            offsets.count = 1;
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
        case WeaponId::ArhaunPhlegethonCannon: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueArhaunLaunchPos;
            offsets.count = 2;
            return &offsets;
        } break;
        case WeaponId::TiangouMoonboundSalvo: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueTiangouLaunchPos;
            offsets.count = 4;
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
        case WeaponId::GoskaLobotomyGrenade: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueTritonAbilityLaunchPos;
            offsets.count = 1;
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
        case WeaponId::IdolPrismaticBlaster: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueIdolLaunchPos;
            offsets.count = 2;
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
        case WeaponId::UnarAutonomousRay: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueUnarLaunchPos;
            offsets.count = 7;
            return &offsets;
        } break;
        case WeaponId::ArchitectDispersiveSalvo: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueArchitectLaunchPos;
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
        case WeaponId::WardenPrismaticCannon: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueWardenLaunchPos;
            offsets.count = 1;
            return &offsets;
        } break;
        case WeaponId::MatrixKhaydarinRay: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueWardenLaunchPos;
            offsets.count = 4;
            return &offsets;
        } break;
        case WeaponId::SpraithCesiantSeekerSpores: {
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
        case WeaponId::IzirokorEnergizedBioplasma: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueIzirokorLaunchPos;
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
        case WeaponId::KethrostalorStaggeredSpores: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueKethrostalorLaunchPos;
            offsets.count = 5;
            return &offsets;
        } break;
        case WeaponId::OgroszalethAcidBarrage: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueKalkaliskLaunchPos;
            offsets.count = 1;
            return &offsets;
        } break;
        case WeaponId::VicarOspoTwinshard: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueVicarLaunchPos;
            offsets.count = 2;
            return &offsets;
        } break;
        case WeaponId::IntendentErsaDriver: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueIntendentLaunchPos;
            offsets.count = 1;
            return &offsets;
        } break;
        case WeaponId::HelionUskoScrapray: {
            static WeaponOffsets offsets{};
            offsets.value_id = ValueHelionLaunchPos;
            offsets.count = 1;
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
        || unit_id == UnitId::ZergOcsalCesiant
        || unit_id == UnitId::ZergCerebrateTheFall
        || unit_id == UnitId::ZergOvermindWithShellTheFall
        || unit_id == UnitId::ZergOvermindTheFall;
}

u32 get_worker_construction_distance(u32 structure_id) {
    switch (structure_id) {
        case UnitId::ProtossAquifer:
        case UnitId::ProtossArdentAuthority:
        case UnitId::AskosiLandlock:
        case UnitId::AskosiBounty:
        case UnitId::AskosiRadix:
        case UnitId::AskosiFunction:
        case UnitId::AskosiCosmoscope: {
            return 88;
        }
        case UnitId::ProtossGrandLibrary:
        case UnitId::ProtossProstrationStage:
        case UnitId::ProtossSyntheticSynod:
        case UnitId::ProtossArgosy: {
            return 99;
        }
        case UnitId::AskosiGrandstand:
        case UnitId::AskosiChantry:
        case UnitId::AskosiCipher:
        case UnitId::AskosiGauntlet: {
            return 48;
        }
    }
    return 70;
}

bool unit_id_ignored_on_minimap(u32 unit_id) {
    return unit_id_is_spell(unit_id) || units_dat::base_property[unit_id] & UnitProperty::Subunit;
}

bool unit_is_combatant(Unit* unit) {
    switch (unit->id) {
        // TODO: add combat casters here
        case UnitId::ZergLakizilisk: {
            return true;
        }
    }
    if (unit->subunit && unit->subunit->is_subunit()) {
        return unit->subunit->has_weapon();
    }
    return unit->has_weapon();
}

bool unit_id_can_repair(u32 unit_id) {
    return unit_id == UnitId::TerranMason
        || unit_id == UnitId::TerranTalos
        || unit_id == UnitId::DanielsMason
        || unit_id == UnitId::TerranOgunWorkerMode;
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

bool unit_id_is_zerg_egg(u32 unit_id) {
    return unit_id == UnitId::ZergEgg || unit_id == UnitId::ZergCocoon;
}

u32 get_mutate_image_id(u32 unit_id) {
    switch (unit_id) {
        case UnitId::ZergYblyleth:		return ImageId::EvigriliskMutate;
        case UnitId::ZergQuazilisk:		return ImageId::QuaziliskMutate;
        case UnitId::ZergQuazrokor:		return ImageId::QuaziliskMutate;
        case UnitId::ZergNathrokor:		return ImageId::NathrokorMutate;
        case UnitId::ZergCikralisk:		return ImageId::CikraliskMutate;
        case UnitId::ZergProtathalor:	return ImageId::ProtathalorMutate;
        case UnitId::ZergIzirokor:		return ImageId::IzirokorMutate;
        case UnitId::ZergIziralisk:		return ImageId::IziraliskMutate;
        case UnitId::ZergSkithrokor:	return ImageId::SkithrokorMutate;
        case UnitId::ZergKagralisk:		return ImageId::KagraliskMutate;
        case UnitId::ZergEvigrilisk:	return ImageId::EvigriliskMutate;
        case UnitId::ZergAlmaksalisk:	return ImageId::AlmaksaliskMutate;
        case UnitId::ZergZoryusthaleth:	return ImageId::ZoryusthalethMutate;
        case UnitId::ZergLakizilisk:	return ImageId::LakiziliskMutate;
        case UnitId::ZergGorgrokor:		return ImageId::GorgrokorMutate;
        case UnitId::ZergAzanthrokor:	return ImageId::AzanthrokorMutate;
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
    }
    if (unit->ext().timers[TimerEnsnaringBrood] > 0) {
        return true;
    }
    return false;
}

bool unit_can_be_ensnared(Unit* unit) {
    if (units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding || unit_is_tower(unit)) {
        return true;
    }
    if (unit->is_building()) {
        return false;
    }
    return true;
}

bool weapon_id_is_line_attack(u32 weapon_id) {
    return weapon_id == WeaponId::StrigaPhaseSaber
        || weapon_id == WeaponId::PariahIconoclastBlades
        || weapon_id == WeaponId::CikraliskAcidSalvo
        || weapon_id == WeaponId::AzanthrokorAzanthrokSpew
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
        case UnitId::ZergAgrileth:		return SoundId::Zerg_Ultra_ZUlDth00_WAV;
        case UnitId::ZergGosvileth:		return SoundId::Zerg_DRONE_ZDrDth00_WAV;
        case UnitId::ZergIroleth:		return SoundId::Zerg_OVERLORD_ZOvDth00_WAV;
        case UnitId::ZergVorvrokor:		return SoundId::VorvrokorDeath01;
        case UnitId::ZergGarvrokor:		return SoundId::VorvrokorDeath01;
        case UnitId::ZergLiiralisk:		return SoundId::Zerg_Hydra_ZHyDth00_WAV;
        case UnitId::ZergKalkalisk:		return SoundId::Zerg_Mutalid_ZMuDth00_WAV;
        case UnitId::ZergGhitorokor:	return SoundId::Zerg_Hydra_ZHyDth00_WAV;
        case UnitId::ZergTosgrilisk:	return SoundId::Zerg_Mutalid_ZMuDth00_WAV;
        case UnitId::ZergBactalisk:		return SoundId::BactaliskDeath01;
        case UnitId::ZergTethzorokor:	return SoundId::Zerg_Ultra_ZUlDth00_WAV;
        case UnitId::ZergVithrilisk:	return SoundId::Zerg_Devourer_ZDvDth00_WAV;
        case UnitId::ZergAlkajelisk:	return SoundId::Zerg_Guardian_ZGuDth00_WAV;
        case UnitId::ZergUltrokor:		return SoundId::Zerg_Ultra_ZUlDth00_WAV;
        case UnitId::ZergZarcavrokor:	return bw::rand_between(SoundId::GorgrokorDeath01, SoundId::GorgrokorDeath02);
        case UnitId::ZergTetcaerokor:	return SoundId::Zerg_Hydra_ZHyDth00_WAV;
        case UnitId::ZergAkistrokor:	return SoundId::Zerg_Queen_ZQuDth00_WAV;

        default:						return SoundId::None;
    }
}

u32 weapon_id_is_automatic(u32 weapon_id) {
    return weapon_id == WeaponId::DurendalPhilistineSalvo
        || weapon_id == WeaponId::TarasqueAcceleratedPlasmid;
}

bool unit_id_has_custom_air_weapon(u32 unit_id) {
    return unit_id == UnitId::TerranDurendal
        || unit_id == UnitId::ZergOkrimnilisk
        || unit_id == UnitId::ZergZobriolisk;
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
        || unit_id == UnitId::TerranEidolonNydus
        || unit_id == UnitId::TerranWraith
        || unit_id == UnitId::TerranSundog
        || unit_id == UnitId::Goska;
}

bool unit_id_is_ridge_refinery(u32 unit_id) {
    return unit_id == UnitId::TerranSieve
        || unit_id == UnitId::ProtossCedent
        || unit_id == UnitId::ZergEskigant
        || unit_id == UnitId::AskosiArch;
}

bool skip_military_tracking_by_unit_id(u32 unit_id) {
    return unit_id == UnitId::TerranMorningstar
        || unit_id == UnitId::ProtossSteward
        || unit_id_is_trap(unit_id);
}

bool unit_id_is_trap(u32 unit_id) {
    return unit_id == UnitId::TerranSpiderMine
        || unit_id == UnitId::TerranLobotomyMine
        || unit_id == UnitId::TerranTartarusMine
        || unit_id == UnitId::TerranGoremast
        || unit_id == UnitId::AskosiEyelet;
}

bool unit_building_can_act(Unit* unit) {
    if (unit->is_in_air() || unit->is_hallucination() || !unit->is_completed()) {
        return false;
    }
    return true;
}

bool unit_could_spawn_custom_larva(Unit* unit) {
    if (unit_id_spawns_larva_by_order(unit->id)) {
        return false;
    }
    switch (unit->id) {
        case UnitId::Omnivale: {
            if (unit->player < PlayerLimit && g_globals->players[unit->player].game_mode_race != GameModeRace_TerranProtoss) {
                return true;
            }
        } break;
    }
    return false;
}

bool unit_spawns_custom_larva(Unit* unit) {
    if (unit->is_in_air() || !unit->is_completed()) {
        return false;
    }
    return unit_could_spawn_custom_larva(unit);
}

bool unit_could_spread_custom_creep(Unit* unit) {
    if (helper::spreads_creep(unit->id, true)) {
        return false;
    }
    if (UnitId::spreads_cagra_while_burrowed(static_cast<UnitId::Enum>(unit->id)) && unit->is_burrowed()) {
        return true;
    }
    auto& unit_ext = unit->ext();
    switch (unit->id) {
        case UnitId::Omnivale: {
            if (unit->player < PlayerLimit && g_globals->players[unit->player].game_mode_race != GameModeRace_TerranProtoss) {
                return true;
            }
        } break;
    }
    if (unit_ext.values[ValueInfested] && unit->is_grounded_building()) {
        return true;
    }
    if (unit_ext.values[ValueUprisingSpawner] && unit->get_race() == RaceId::Zerg) {
        return true;
    }
    if (unit->id == UnitId::ZergYblyleth) {
        return true;
    }
    return false;
}

bool unit_spreads_custom_creep(Unit* unit) {
    if (unit->is_in_air() || !unit->is_completed()) {
        return false;
    }
    if (unit->id == UnitId::ZergYblyleth && !unit->is_burrowed()) {
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
        || unit_id == UnitId::TerranHarakanGolden
        || unit_id == UnitId::TerranStriga
        || unit_id == UnitId::TerranMatadorTankMode
        || unit_id == UnitId::TerranMatadorTurretTankMode
        || unit_id == UnitId::TerranMatadorCinderMode
        || unit_id == UnitId::TerranMatadorTurretCinderMode
        || unit_id == UnitId::TerranDurendal
        || unit_id == UnitId::ProtossPariah
        || unit_id == UnitId::ZergCikralisk
        || unit_id == UnitId::ZergAzanthrokor
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
        || portrait_id == PortraitId::AdvisorZerg;
}

u32 get_player_portrait_id_override(u32 portrait_id, u32 player) {
    if (player >= PlayerLimit) {
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
                case Faction::ProtossKhalendricKnights:     return PortraitId::AdvisorKhalendricKnights;
                case Faction::ProtossOrderOfTheSacredSaber: return PortraitId::Patriarch; // Temporary
                case Faction::ProtossStarbound:             return PortraitId::AdvisorStarbound;
                case Faction::ProtossAstralOmen:            return PortraitId::Demiurge;
            }
        } break;
        case PortraitId::AdvisorZerg: {
            switch (faction) {
                case Faction::ZergImicrenTendril:			return PortraitId::AdvisorImicren;
                case Faction::ZergOrkotiTendril:			return PortraitId::AdvisorOrkoti;
                case Faction::ZergZorthosTendril:			return PortraitId::AdvisorZorthos;
            }
        } break;
    }
    return portrait_id;
}

bool unit_id_is_tiny_unit(u32 unit_id) {
    return unit_id == UnitId::TerranPietyWard
        || unit_id == UnitId::TerranLobotomyMine
        || unit_id == UnitId::TerranTartarusMine
        || unit_id == UnitId::TerranGoremast
        || unit_id == UnitId::ZergRilirokor
        || unit_id == UnitId::AskosiEyelet;
}

bool unit_id_is_spell(u32 unit_id) {
    return unit_id == UnitId::Spell_CarapaceSwarm
        || unit_id == UnitId::Spell_CarapaceSwarmSmall
        || unit_id == UnitId::Spell_ScannerSweep;
}

bool unit_id_has_cleric_heal(u32 unit_id) {
    return unit_id == UnitId::TerranCleric
        || unit_id == UnitId::TerranClericFuturist;
}

bool unit_id_is_valid_for_most_abilities(u32 unit_id) {
    return unit_id != UnitId::ZergLarva
        && unit_id != UnitId::ZergOcsalLarva
        && unit_id != UnitId::ZergEgg
        && unit_id != UnitId::ZergCocoon
        && unit_id != UnitId::ProtossSteward;
}

bool unit_id_is_hero(u32 unit_id) {
    return unit_id == UnitId::Goska
        || unit_id == UnitId::Stroluum
        || unit_id == UnitId::Ackmed;
}

bool weapon_id_can_self_splash(u32 weapon_id) {
    return weapon_id == WeaponId::CyclopsBrontesPayload
        || weapon_id == WeaponId::PsiStorm;
}

bool unit_id_is_mine_layer(UnitId::Enum unit_id) {
    return unit_id == UnitId::TerranVulture
        || unit_id == UnitId::TerranVultureHecaton
        || unit_id == UnitId::TerranVultureUpstart
        || unit_id == UnitId::TerranClericFuturist
        || unit_id == UnitId::AskosiProselyte;
}

bool unit_id_can_construct(UnitId::Enum unit_id) {
    return unit_id == UnitId::TerranMason
        || unit_id == UnitId::TerranTalos
        || unit_id == UnitId::DanielsMason
        || unit_id == UnitId::TerranOgunWorkerMode
        || unit_id == UnitId::ProtossScribe
        || unit_id == UnitId::ProtossPrudence
        || unit_id == UnitId::ProtossAutocort
        || unit_id == UnitId::ZergDroleth
        || unit_id == UnitId::ZergAgrileth
        || unit_id == UnitId::ZergGosvileth
        || unit_id == UnitId::AskosiPilgrim
        || unit_id == UnitId::AskosiDyad
        || unit_id == UnitId::AskosiConstituent;
}

bool unit_id_is_hangar_unit(UnitId::Enum unit_id) {
    return unit_id == UnitId::ProtossSteward || unit_id == UnitId::ProtossPlasmaShell;
}

bool unit_id_is_air_spawner(UnitId::Enum unit_id) {
    return unit_id == UnitId::TerranStarpad
        || unit_id == UnitId::TerranStarport
        || unit_id == UnitId::TerranNaniteAssembly
        || unit_id == UnitId::ProtossStargate
        || unit_id == UnitId::ProtossArgosy
        || unit_id == UnitId::ProtossMonumentOfSin
        || unit_id == UnitId::ZergMutathSpire
        || unit_id == UnitId::ZergTosvrolHaunt;
}

bool unit_id_is_flyer_or_air_spawner(UnitId::Enum unit_id) {
    return units_dat::base_property[unit_id] & UnitProperty::Flyer
        || unit_id_is_air_spawner(unit_id);
}

bool weapon_id_deals_no_damage(WeaponId::Enum weapon_id) {
    return weapon_id == WeaponId::MindTyrantMaledictus; // More will be added later. -Pr0nogo
}

bool unit_id_blocks_construction(UnitId::Enum unit_id) {
    return unit_id == UnitId::VolatileEgg;
}

UnitId::Enum get_racial_worker_id(u32 race_id) {
    switch (race_id) {
        case RaceId::Terran:  return UnitId::TerranMason;
        case RaceId::Protoss: return UnitId::ProtossScribe;
        case RaceId::Zerg:    return UnitId::ZergDroleth;
        case RaceId::Askosi:  return UnitId::AskosiPilgrim;
    }
    return UnitId::None;
}

UnitId::Enum get_racial_town_center_id(u32 race_id) {
    switch (race_id) {
        case RaceId::Terran:  return UnitId::TerranMinistry;
        case RaceId::Protoss: return UnitId::ProtossNexus;
        case RaceId::Zerg:    return UnitId::ZergHachirosk;
        case RaceId::Askosi:  return UnitId::AskosiLandlock;
    }
    return UnitId::None;
}

namespace UnitId {
bool can_cast_recall(UnitId::Enum unit_id) {
    return unit_id == UnitId::ProtossXenolith
        || unit_id == UnitId::ProtossDidact;
}

bool has_manual_weapon_fire_sfx(UnitId::Enum unit_id) {
    return unit_id == UnitId::TerranSavantEntropyGauntlets
        || unit_id == UnitId::TerranSavantPowerSiphon

        // @TODO: Extend weapons and remove these
        || unit_id == UnitId::ProtossZealot
        || unit_id == UnitId::ProtossZealotStarbound
        || unit_id == UnitId::ProtossEcclesiast
        || unit_id == UnitId::ProtossEcclesiastKhalendric
        || unit_id == UnitId::ProtossCicatus
        || unit_id == UnitId::ProtossManifold
        || unit_id == UnitId::ProtossManifoldStarbound
        || unit_id == UnitId::AskosiAvatar
        || unit_id == UnitId::AskosiAgonist
        || unit_id == UnitId::AskosiPhontis
        || unit_id == UnitId::AskosiAkreno
        || unit_id == UnitId::AskosiCrematorium;
}

bool is_atg_regression(UnitId::Enum old_unit_id, UnitId::Enum new_unit_id) {
    return units_dat::base_property[old_unit_id] & UnitProperty::Flyer
       && !units_dat::base_property[new_unit_id] & UnitProperty::Flyer;
}

u32 get_overlay_offset(UnitId::Enum unit_id) {
    if (unit_id_uses_xlarge_overlays(unit_id)) {
        return 3;
    }
    if (units_dat::base_property[unit_id] & UnitProperty::LargeOverlay) {
        return 2;
    }
    if (units_dat::base_property[unit_id] & UnitProperty::MediumOverlay) {
        return 1;
    }
    return 0;
}

SpriteId::Enum get_corpse_sprite_id(UnitId::Enum unit_id) {
    // Specific
    switch (unit_id) {
        case UnitId::TerranMason:                        return SpriteId::Corpse_Mason;
        case UnitId::TerranMaverick:                     return SpriteId::Corpse_Maverick;
        case UnitId::TerranHarakan:                      return SpriteId::Corpse_Harakan;
        case UnitId::TerranHarakanGolden:                return SpriteId::Corpse_Harakan;
        case UnitId::TerranCyprian:                      return SpriteId::Corpse_Cyprian;
        case UnitId::TerranCleric:                       return SpriteId::Corpse_Cleric;
        case UnitId::TerranShamanSupportMode:            return SpriteId::Corpse_Shaman;
        case UnitId::TerranShamanVisionaryMode:          return SpriteId::Corpse_Shaman;
        case UnitId::TerranEidolon:                      return SpriteId::Corpse_Eidolon;
        case UnitId::TerranEidolonNydus:                 return SpriteId::Corpse_Eidolon;
        case UnitId::TerranSavantEntropyGauntlets:       return SpriteId::Corpse_Savant;
        case UnitId::TerranSavantPowerSiphon:            return SpriteId::Corpse_Savant;
        case UnitId::TerranApostle:                      return SpriteId::Corpse_Apostle;
        case UnitId::TerranDilettante:                   return SpriteId::Corpse_Dilettante;
        case UnitId::TerranHypnagogue:                   return SpriteId::Corpse_Hypnagogue;
        case UnitId::TerranVulture:                      return SpriteId::Corpse_Vulture;
        case UnitId::TerranVultureHecaton:               return SpriteId::Corpse_Vulture;
        case UnitId::TerranVultureUpstart:               return SpriteId::Corpse_Vulture;
        case UnitId::TerranCyclops:                      return SpriteId::Corpse_Cyclops;
        case UnitId::TerranCyclopsFuturist:              return SpriteId::Corpse_Cyclops;
        case UnitId::TerranGoliath:                      return SpriteId::Corpse_Goliath;
        case UnitId::TerranAspirant:                     return SpriteId::Corpse_Aspirant;
        case UnitId::TerranBlackjack:                    return SpriteId::Corpse_Blackjack;
        case UnitId::TerranBlackjackGolden:              return SpriteId::Corpse_Blackjack;
        case UnitId::TerranPhalanxTankMode:              return SpriteId::Corpse_PhalanxTank;
        case UnitId::TerranPhalanxSiegeMode:             return SpriteId::Corpse_PhalanxSiege;
        case UnitId::TerranMadrigalImperiosoMode:        return SpriteId::Corpse_MadrigalFuria;
        case UnitId::TerranMadrigalHecatonImperiosoMode: return SpriteId::Corpse_MadrigalFuria;
        case UnitId::TerranMadrigalFuriaMode:            return SpriteId::Corpse_MadrigalFuria;
        case UnitId::TerranTriton:                       return SpriteId::Corpse_TerranUnitMedium;
        case UnitId::TerranWraith:                       return SpriteId::Corpse_Wraith;
        case UnitId::TerranAncile:                       return SpriteId::Corpse_Ancile;
        case UnitId::TerranTrojan:                       return SpriteId::Corpse_Trojan;
                                                   
        case UnitId::ProtossScribe:                return SpriteId::Corpse_Scribe;
        case UnitId::ProtossEnvoy:                 return SpriteId::Corpse_Envoy;
        case UnitId::ProtossWitness:               return SpriteId::Corpse_Witness;
        case UnitId::ProtossArtisan:               return SpriteId::Corpse_Artisan;
        case UnitId::ProtossZealot:                return SpriteId::Corpse_Zealot;
        case UnitId::ProtossZealotStarbound:       return SpriteId::Corpse_Zealot;
        case UnitId::ProtossDracadin:              return SpriteId::Corpse_Dracadin;
        case UnitId::ProtossDracadinOrder:         return SpriteId::Corpse_DracadinOrder;
        case UnitId::ProtossEcclesiast:            return SpriteId::Corpse_Ecclesiast;
        case UnitId::ProtossLegionnaire:           return SpriteId::Corpse_Legionnaire;
        case UnitId::ProtossHierophant:            return SpriteId::Corpse_Hierophant;
        case UnitId::ProtossSimulacrum:            return SpriteId::Corpse_Simulacrum;
        case UnitId::ProtossManifold:              return SpriteId::Corpse_Manifold;
        case UnitId::ProtossVassal:                return SpriteId::Corpse_Vassal;
        case UnitId::ProtossIdol:                  return SpriteId::Corpse_Idol;
        case UnitId::ProtossGolem:                 return SpriteId::Corpse_Golem;
        case UnitId::ProtossRenesair:              return SpriteId::Corpse_Renesair;
        case UnitId::ProtossLanifect:              return SpriteId::Corpse_Lanifect;

        case UnitId::ZergLarva:                    return SpriteId::Corpse_Larva;
        case UnitId::ZergOcsalLarva:               return SpriteId::Corpse_Larva;
        case UnitId::ZergEgg:                      return SpriteId::Corpse_Egg;
        case UnitId::ZergDroleth:                  return SpriteId::Corpse_Droleth;
        case UnitId::ZergAgrileth:                 return SpriteId::Corpse_Agrileth;
        case UnitId::ZergOvileth:                  return SpriteId::Corpse_Ovileth;
        case UnitId::ZergZethrokor:                return SpriteId::Corpse_Zethrokor;
        case UnitId::ZergSovrokor:                 return SpriteId::Corpse_Zethrokor;
        case UnitId::ZergQuazilisk:                return SpriteId::Corpse_Quazilisk;
        case UnitId::ZergQuazrokor:                return SpriteId::Corpse_Quazilisk;
        case UnitId::ZergVorvrokor:                return SpriteId::Corpse_Vorvrokor;
        case UnitId::ZergGarvrokor:                return SpriteId::Corpse_Vorvrokor;
        case UnitId::ZergNathrokor:                return SpriteId::Corpse_Nathrokor;
        case UnitId::ZergLiiralisk:                return SpriteId::Corpse_ZergUnitSmall;
        case UnitId::ZergKalkalisk:                return SpriteId::Corpse_ZergUnitSmall;
        case UnitId::ZergGhitorokor:               return SpriteId::Corpse_ZergUnitMedium;
        case UnitId::ZergSovroleth:                return SpriteId::Corpse_ZergUnitSmall;
        case UnitId::ZergTosgrilisk:               return SpriteId::Corpse_ZergUnitMedium;
        case UnitId::ZergMortothrokor:             return SpriteId::Corpse_Mortothrokor;
        case UnitId::ZergHydralisk:                return SpriteId::Corpse_Hydralisk;
        case UnitId::ZergHydraliskOrkoti:          return SpriteId::Corpse_HydraliskOrkoti;
        case UnitId::ZergIzirokor:                 return SpriteId::Corpse_Izirokor;
        case UnitId::ZergIziralisk:                return SpriteId::Corpse_Izirokor;
        case UnitId::ZergBactalisk:                return SpriteId::Corpse_Bactalisk;
        case UnitId::ZergKagralisk:                return SpriteId::Corpse_Kagralisk;
        case UnitId::ZergEvigrilisk:               return SpriteId::Corpse_Evigrilisk;
        case UnitId::ZergAlkajelisk:               return SpriteId::Corpse_ZergBuildingLarge;
        case UnitId::ZergZoryusthaleth:            return SpriteId::Corpse_Zoryusthaleth;
        case UnitId::ZergInzorthrokor:             return SpriteId::Corpse_Inzorthrokor;
        case UnitId::ZergLakizilisk:               return SpriteId::Corpse_Lakizilisk;
        case UnitId::ZergLakiziliskZorthos:        return SpriteId::Corpse_Lakizilisk;
        case UnitId::ZergGorgrokor:                return SpriteId::Corpse_Gorgrokor;
        case UnitId::ZergAzanthrokor:              return SpriteId::Corpse_ZergUnitMedium;
        case UnitId::ZergKonvilisk:                return SpriteId::Corpse_Konvilisk;
        case UnitId::ZergRilirokor:                return SpriteId::Corpse_Rilirokor;
        case UnitId::ZergUltrokor:                 return SpriteId::Corpse_Ultrokor;
        case UnitId::ZergIsthrathaleth:            return SpriteId::Corpse_Isthrathaleth;

        case UnitId::AskosiMissionary:             return SpriteId::Corpse_Missionary;
        case UnitId::AskosiCardinal:               return SpriteId::Corpse_Cardinal;
        case UnitId::AskosiVicar:                  return SpriteId::Corpse_Vicar;
    }

    // Generic
    u32 overlay_size = UnitId::get_overlay_offset(unit_id);
    bool is_unit = !(units_dat::base_property[unit_id] & UnitProperty::Building);
    switch (Unit::get_race(unit_id)) {
        case RaceId::Terran:
        case RaceId::Askosi: {
            switch (overlay_size) {
                case 0: return !is_unit ? SpriteId::Corpse_TerranUnitSmall  : SpriteId::Corpse_TerranBuildingSmall;
                case 1: return !is_unit ? SpriteId::Corpse_TerranUnitMedium : SpriteId::Corpse_TerranBuildingSmall;
                case 2: return SpriteId::Corpse_TerranBuildingSmall;
                case 3: return SpriteId::Corpse_TerranBuildingLarge;
            }
            return SpriteId::Corpse_TerranUnitSmall;
        }
        case RaceId::Protoss: {
            switch (overlay_size) {
                case 0: return !is_unit ? SpriteId::Corpse_ProtossUnitSmall  : SpriteId::Corpse_ProtossBuildingSmall;
                case 1: return !is_unit ? SpriteId::Corpse_ProtossUnitMedium : SpriteId::Corpse_ProtossBuildingSmall;
                case 2: return SpriteId::Corpse_ProtossBuildingSmall;
                case 3: return SpriteId::Corpse_ProtossBuildingLarge;
            }
            return SpriteId::Corpse_ProtossUnitSmall;
        }
        case RaceId::Zerg: {
            switch (overlay_size) {
                case 0: return !is_unit ? SpriteId::Corpse_ZergUnitSmall  : SpriteId::Corpse_ZergBuildingSmall;
                case 1: return !is_unit ? SpriteId::Corpse_ZergUnitMedium : SpriteId::Corpse_ZergBuildingSmall;
                case 2: return SpriteId::Corpse_ZergBuildingSmall;
                case 3: return SpriteId::Corpse_ZergBuildingLarge;
            }
            return SpriteId::Corpse_ZergUnitSmall;
        }
    }
}

bool is_askosi(UnitId::Enum unit_id) {
    return unit_id == UnitId::AskosiPilgrim
        || unit_id == UnitId::AskosiProselyte
        || unit_id == UnitId::AskosiDyad
        || unit_id == UnitId::AskosiMissionary
        || unit_id == UnitId::AskosiCardinal
        || unit_id == UnitId::AskosiMimic
        || unit_id == UnitId::AskosiVicar
        || unit_id == UnitId::AskosiRove
        || unit_id == UnitId::AskosiObject
        || unit_id == UnitId::AskosiScepter
        || unit_id == UnitId::AskosiIntendent
        || unit_id == UnitId::AskosiHelion
        || unit_id == UnitId::AskosiEmissary
        || unit_id == UnitId::AskosiInkblot
        || unit_id == UnitId::AskosiTirade
        || unit_id == UnitId::AskosiConstituent
        || unit_id == UnitId::AskosiAphorist
        || unit_id == UnitId::AskosiThaumalogue
        || unit_id == UnitId::AskosiAvatar
        || unit_id == UnitId::AskosiDoronin
        || unit_id == UnitId::AskosiAgonist
        || unit_id == UnitId::AskosiPetract
        || unit_id == UnitId::AskosiPhontis
        || unit_id == UnitId::AskosiZonguard
        || unit_id == UnitId::AskosiAkreno
        || unit_id == UnitId::AskosiIconoclast
        || unit_id == UnitId::AskosiCrematorium
        || unit_id == UnitId::AskosiParaclete
        || unit_id == UnitId::AskosiAuxine

        || unit_id == UnitId::AskosiLandlock
        || unit_id == UnitId::AskosiArch
        || unit_id == UnitId::AskosiGrandstand
        || unit_id == UnitId::AskosiGrandstandHoldout
        || unit_id == UnitId::AskosiGrandstandVision
        || unit_id == UnitId::AskosiChantry
        || unit_id == UnitId::AskosiRadix
        || unit_id == UnitId::AskosiCipher
        || unit_id == UnitId::AskosiFunction
        || unit_id == UnitId::AskosiGauntlet
        || unit_id == UnitId::AskosiCosmoscope
        || unit_id == UnitId::AskosiBounty
        || unit_id == UnitId::AskosiAbutment
        || unit_id == UnitId::AskosiDorminge
        || unit_id == UnitId::AskosiProscenium
        || unit_id == UnitId::AskosiConduit
        || unit_id == UnitId::AskosiZylode
        || unit_id == UnitId::AskosiBestiary
        || unit_id == UnitId::AskosiAstrolabe
        || unit_id == UnitId::AskosiDomain
        || unit_id == UnitId::AskosiPanoply
        || unit_id == UnitId::AskosiLotiro
        || unit_id == UnitId::AskosiObelisk;
}

bool is_forerunner(UnitId::Enum unit_id) {
    return unit_id == UnitId::ForerunnerFerrum
        || unit_id == UnitId::ForerunnerChalyb
        || unit_id == UnitId::ForerunnerCircitor
        || unit_id == UnitId::ForerunnerExigo;
}

SoundId::Enum get_unit_ready_sound(UnitId::Enum unit_id) {
    if (unit_id == UnitId::ProtossSteward || unit_id == UnitId::ProtossPlasmaShell) {
        return SoundId::None;
    }
    u32 ready_sound = units_dat::sound_ready[unit_id];
    switch (unit_id) {
        case UnitId::TerranMorningstar: {
            ready_sound = bw::get_advisor_sound_from_event_type(g_local_nation_id, AdvisorEvent_MissileArmed);
        } break;
        case UnitId::TerranTalos: {
            ready_sound = helper::get_random_unit_response(SoundId::TalosReady01, SoundId::TalosReady03);
        } break;
        case UnitId::TerranHotshot: {
            ready_sound = helper::get_random_unit_response(SoundId::HotshotReady01, SoundId::HotshotReady03);
        } break;
        case UnitId::TerranCyprian: {
            ready_sound = helper::get_random_unit_response(SoundId::CyprianReady01, SoundId::CyprianReady03);
        } break;
        case UnitId::TerranCannibal: {
            ready_sound = helper::get_random_unit_response(SoundId::CannibalReady01, SoundId::CannibalReady03);
        } break;
        case UnitId::TerranShamanSupportMode: {
            ready_sound = helper::get_random_unit_response(SoundId::ShamanReady01, SoundId::ShamanReady03);
        } break;
        case UnitId::TerranSavantEntropyGauntlets:
        case UnitId::TerranSavantPowerSiphon: {
            ready_sound = helper::get_random_unit_response(SoundId::SavantReady01, SoundId::SavantReady03);
        } break;
        case UnitId::TerranMadcap: {
            ready_sound = helper::get_random_unit_response(SoundId::MadcapReady01, SoundId::MadcapReady03);
        } break;
        case UnitId::TerranHeracles: {
            ready_sound = helper::get_random_unit_response(SoundId::HeraclesReady01, SoundId::HeraclesReady03);
        } break;
        case UnitId::TerranApostle: {
            ready_sound = helper::get_random_unit_response(SoundId::ApostleReady01, SoundId::ApostleReady03);
        } break;
        case UnitId::TerranSiren: {
            ready_sound = helper::get_random_unit_response(SoundId::SirenReady01, SoundId::SirenReady03);
        } break;
        case UnitId::TerranStriga: {
            ready_sound = helper::get_random_unit_response(SoundId::StrigaReady01, SoundId::StrigaReady03);
        } break;
        case UnitId::TerranDilettante: {
            ready_sound = helper::get_random_unit_response(SoundId::DilettanteReady01, SoundId::DilettanteReady03);
        } break;
        case UnitId::TerranSevenstep: {
            ready_sound = helper::get_random_unit_response(SoundId::SevenstepReady01, SoundId::SevenstepReady03);
        } break;
        case UnitId::TerranAspirant: {
            ready_sound = helper::get_random_unit_response(SoundId::AspirantReady01, SoundId::AspirantReady03);
        } break;
        case UnitId::TerranBlackjack:
        case UnitId::TerranBlackjackGolden: {
            ready_sound = helper::get_random_unit_response(SoundId::BlackjackReady01, SoundId::BlackjackReady03);
        } break;
        case UnitId::TerranMatadorTankMode: {
            ready_sound = helper::get_random_unit_response(SoundId::MatadorReady01, SoundId::MatadorReady03);
        } break;
        case UnitId::TerranPhalanxTankMode:
        case UnitId::TerranPhalanxTankModeTurret:
        case UnitId::TerranPhalanxSiegeMode:
        case UnitId::TerranPhalanxSiegeModeTurret: {
            ready_sound = helper::get_random_unit_response(SoundId::PhalanxReady01, SoundId::PhalanxReady02);
        } break;
        case UnitId::TerranRamesses: {
            ready_sound = helper::get_random_unit_response(SoundId::RamessesReady01, SoundId::RamessesReady03);
        } break;
        case UnitId::TerranDurendal: {
            ready_sound = helper::get_random_unit_response(SoundId::DurendalReady01, SoundId::DurendalReady03);
        } break;
        case UnitId::TerranCataphract: {
            ready_sound = helper::get_random_unit_response(SoundId::CataphractReady01, SoundId::CataphractReady03);
        } break;
        case UnitId::TerranTriton: {
            ready_sound = helper::get_random_unit_response(SoundId::TritonReady01, SoundId::TritonReady03);
        } break;
        case UnitId::TerranPaladin: {
            ready_sound = helper::get_random_unit_response(SoundId::PaladinReady01, SoundId::PaladinReady03);
        } break;
        case UnitId::TerranPazuzu: {
            ready_sound = helper::get_random_unit_response(SoundId::PazuzuReady01, SoundId::PazuzuReady03);
        } break;
        case UnitId::TerranMadrigalImperiosoMode:
        case UnitId::TerranMadrigalHecatonImperiosoMode: {
            ready_sound = helper::get_random_unit_response(SoundId::MadrigalReady01, SoundId::MadrigalReady03);
        } break;
        case UnitId::TerranAnticthon: {
            ready_sound = helper::get_random_unit_response(SoundId::AnticthonReady01, SoundId::AnticthonReady03);
        } break;
        case UnitId::TerranAion: {
            ready_sound = helper::get_random_unit_response(SoundId::AionReady01, SoundId::AionReady03);
        } break;
        case UnitId::TerranAncile: {
            ready_sound = helper::get_random_unit_response(SoundId::AncileReady01, SoundId::AncileReady01);
        } break;
        case UnitId::TerranWyvern: {
            ready_sound = helper::get_random_unit_response(SoundId::WyvernReady01, SoundId::WyvernReady03);
        } break;
        case UnitId::TerranSundog: {
            ready_sound = helper::get_random_unit_response(SoundId::SundogReady01, SoundId::SundogReady03);
        } break;
        case UnitId::TerranSalamander: {
            ready_sound = helper::get_random_unit_response(SoundId::SalamanderReady01, SoundId::SalamanderReady03);
        } break;
        case UnitId::TerranAzazel: {
            ready_sound = helper::get_random_unit_response(SoundId::AzazelReady01, SoundId::AzazelReady03);
        } break;
        case UnitId::TerranPhobos: {
            ready_sound = helper::get_random_unit_response(SoundId::PhobosReady01, SoundId::PhobosReady03);
        } break;
        case UnitId::TerranCentaur:
        case UnitId::TerranMagnetar: // TODO: Own lines
        case UnitId::TerranHippogriff: // TODO: Own lines
        {
            ready_sound = helper::get_random_unit_response(SoundId::CentaurReady01, SoundId::CentaurReady03);
        } break;
        case UnitId::ProtossZealot: {
            ready_sound = helper::get_random_unit_response(SoundId::ZealotReady01, SoundId::ZealotReady02);
        } break;
        case UnitId::ProtossEcclesiast: {
            ready_sound = helper::get_random_unit_response(SoundId::EcclesiastReady01, SoundId::EcclesiastReady03);
        } break;
        case UnitId::ProtossDracadin:
        case UnitId::ProtossDracadinOrder: {
            ready_sound = helper::get_random_unit_response(SoundId::DracadinReady01, SoundId::DracadinReady03);
        } break;
        case UnitId::ProtossLegionnaire: {
            ready_sound = helper::get_random_unit_response(SoundId::LegionnaireReady01, SoundId::LegionnaireReady03);
        } break;
        case UnitId::ProtossHierophant: {
            ready_sound = helper::get_random_unit_response(SoundId::HierophantReady01, SoundId::HierophantReady03);
        } break;
        case UnitId::ProtossAmaranth: {
            ready_sound = helper::get_random_unit_response(SoundId::AmaranthReady01, SoundId::AmaranthReady03);
        } break;
        case UnitId::ProtossHerald: {
            ready_sound = helper::get_random_unit_response(SoundId::HeraldReady01, SoundId::HeraldReady03);
        } break;
        case UnitId::ProtossVagrant: {
            ready_sound = helper::get_random_unit_response(SoundId::VagrantReady01, SoundId::VagrantReady03);
        } break;
        case UnitId::ProtossVagrantShade: {
            ready_sound = helper::get_random_unit_response(SoundId::VagrantShadeReady01, SoundId::VagrantShadeReady04);
        } break;
        case UnitId::ProtossPariah: {
            ready_sound = helper::get_random_unit_response(SoundId::PariahReady01, SoundId::PariahReady03);
        } break;
        case UnitId::ProtossPotentate: {
            ready_sound = helper::get_random_unit_response(SoundId::PotentateReady01, SoundId::PotentateReady03);
        } break;
        case UnitId::ProtossSycophant: {
            ready_sound = helper::get_random_unit_response(SoundId::SycophantReady01, SoundId::SycophantReady03);
        } break;
        case UnitId::ProtossOptecton: {
            ready_sound = helper::get_random_unit_response(SoundId::OptectonReady01, SoundId::OptectonReady03);
        } break;
        case UnitId::ProtossPatriarch: {
            ready_sound = helper::get_random_unit_response(SoundId::PatriarchReady01, SoundId::PatriarchReady03);
        } break;
        case UnitId::ProtossGolem: {
            ready_sound = helper::get_random_unit_response(SoundId::GolemReady01, SoundId::GolemReady02);
        } break;
        case UnitId::ProtossEmpress: {
            ready_sound = helper::get_random_unit_response(SoundId::EmpressReady01, SoundId::EmpressReady03);
        } break;
        case UnitId::ProtossMagister: {
            ready_sound = helper::get_random_unit_response(SoundId::MagisterReady01, SoundId::MagisterReady03);
        } break;
        case UnitId::ProtossStarSovereign: {
            ready_sound = helper::get_random_unit_response(SoundId::StarSovereignReady01, SoundId::StarSovereignReady03);
        } break;
        case UnitId::ZergLiiralisk: {
            ready_sound = helper::get_random_unit_response(SoundId::LiiraliskReady01, SoundId::LiiraliskReady02);
        } break;
        case UnitId::AskosiPilgrim: {
            ready_sound = helper::get_random_unit_response(SoundId::PilgrimReady01, SoundId::PilgrimReady03);
        } break;
        case UnitId::AskosiProselyte: {
            ready_sound = helper::get_random_unit_response(SoundId::ProselyteReady01, SoundId::ProselyteReady03);
        } break;
        case UnitId::AskosiMissionary: {
            ready_sound = helper::get_random_unit_response(SoundId::MissionaryReady01, SoundId::MissionaryReady03);
        } break;
        case UnitId::AskosiRove: {
            ready_sound = helper::get_random_unit_response(SoundId::RoveReady01, SoundId::RoveReady03);
        } break;
        case UnitId::AskosiHelion: {
            ready_sound = helper::get_random_unit_response(SoundId::HelionReady01, SoundId::HelionReady03);
        } break;
    }
    return (SoundId::Enum)ready_sound;
}

bool has_autovitality(UnitId::Enum unit_id) {
    return unit_id == UnitId::ProtossSimulacrum || unit_id == UnitId::ProtossRenesair || unit_id == UnitId::ProtossSimulacrumAstral;
}

bool turns_to_face_target(UnitId::Enum unit_id) {
    return unit_id == UnitId::TerranGoliath
        || unit_id == UnitId::TerranTriton
        || unit_id == UnitId::TerranTiangou
        || unit_id == UnitId::TerranArhaun;
}

bool has_automandate(UnitId::Enum unit_id) {
    return unit_id == UnitId::ProtossSimulacrumAstral;
}

bool has_interruptable_windup(UnitId::Enum unit_id) {
    return unit_id == UnitId::TerranCyprian
        || unit_id == UnitId::TerranEidolon
        || unit_id == UnitId::TerranGoliath;
}

SoundId::Enum get_windup_sfx(UnitId::Enum unit_id) {
    switch (unit_id) {
    case UnitId::TerranCyprian: return bw::SoundSet{ SoundId::CyprianWindup01, SoundId::CyprianWindup03 }.rand();
    case UnitId::TerranEidolon: return bw::SoundSet{ SoundId::EidolonWindup01, SoundId::EidolonWindup03 }.rand();
    case UnitId::TerranGoliath: return bw::SoundSet{ SoundId::GoliathTwinAutocannonWindup01, SoundId::GoliathTwinAutocannonWindup03 }.rand();
    }
    return SoundId::None;
}

bool is_terran_worker(UnitId::Enum unit_id) {
    return unit_id == UnitId::TerranMason
        || unit_id == UnitId::TerranTalos
        || unit_id == UnitId::TerranOgunWorkerMode;
}

bool is_zerg_worker(UnitId::Enum unit_id) {
    return unit_id == UnitId::ZergDroleth
        || unit_id == UnitId::ZergAgrileth
        || unit_id == UnitId::ZergGosvileth;
}

bool is_beacon(UnitId::Enum unit_id) {
    return unit_id == UnitId::ExtBeaconTerran
        || unit_id == UnitId::ExtBeaconProtoss
        || unit_id == UnitId::ExtBeaconZerg;
}

bool has_fusion_sealers(Unit* unit) {
    return unit->id == UnitId::TerranOgunSmithyMode;
}

bool match_id_to_base(UnitId::Enum match_id, UnitId::Enum base_id) {
    switch (base_id) {
        case UnitId::ZergIrolIris:       return match_id == UnitId::ZergOthstolOviform;
        case UnitId::ZergQuazethPool:    return match_id == UnitId::ZergVornathPond;
        case UnitId::ZergVornathPond:    return match_id == UnitId::ZergKalkiirLake;
        case UnitId::ZergHydrithDen:     return match_id == UnitId::ZergSkibactScab;
        case UnitId::ZergSkibactScab:    return match_id == UnitId::ZergTethvithSwamp;
        case UnitId::ZergZorkizShroud:   return match_id == UnitId::ZergUlgorgCavern;
        case UnitId::ZergUlgorgCavern:   return match_id == UnitId::ZergZozarcGorge;
        case UnitId::ZergMutathSpire:    return match_id == UnitId::ZergTosvrolHaunt;
        case UnitId::ZergAlkagIteth:     return match_id == UnitId::ZergChokralChazsal;
        case UnitId::ZergMatravilNest:   return match_id == UnitId::ZergAlmakisAntre;
        case UnitId::ZergAxtothAxis:     return match_id == UnitId::ZergBegrozithBarrow;
        case UnitId::ZergGeszkathGrotto: return match_id == UnitId::ZergKethrajKeep;
    }
    return false;
}

bool is_triple_birth(UnitId::Enum unit_id) {
    return unit_id == UnitId::ZergSovrokor;
}

bool has_forward_march(UnitId::Enum unit_id) {
    return unit_id == UnitId::TerranDurendal
        || unit_id == UnitId::AskosiIntendent
        || unit_id == UnitId::AskosiZonguard;
}

// units_dat: make building placement box divisible by 32! -Pr0nogo
bool spreads_cagra_while_burrowed(UnitId::Enum unit_id) {
    return unit_id == UnitId::ZergAgrileth
        || unit_id == UnitId::ZergYblyleth
        || unit_id == UnitId::ZergScrylyleth
        || unit_id == UnitId::ZergMalkothrokor;
}

bool has_swarming_spite(UnitId::Enum unit_id) {
    return unit_id == UnitId::ZergSkryrokor;
}

} // namespace UnitId

namespace WeaponId {
u32 get_custom_melee_factor(WeaponId::Enum weapon_id) {
    return 1;
}

bool weapon_splits(WeaponId::Enum weapon_id) {
    return weapon_id == WeaponId::ArchitectDispersiveSalvo;
}

bool bounces_binding_plasma(WeaponId::Enum weapon_id) {
    return weapon_id == WeaponId::OscilliantRotatingRay
        || weapon_id == WeaponId::GladiusPlasmaFlurry;
}

bool can_attack_target(WeaponId::Enum weapon_id, Unit* target) {
    if (!target) {
        return false;
    }
    if (weapons_dat::target_flags[weapon_id].air && target->is_in_air()) {
        return true;
    }
    if (weapons_dat::target_flags[weapon_id].ground && !target->is_in_air()) {
        return true;
    }
    return false;
}

bool bounces_power_breach(WeaponId::Enum weapon_id) {
    return weapon_id == WeaponId::IntendentErsaDriver;
}
} // namespace WeaponId