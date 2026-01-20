#include "new_requirements.h"
#include <hook_tools.h>
#include "hooks/rally_point.h"
#include "trigger_system/trigger_system.h"

namespace hooks {
    enum {
        TECH_LOCKED = -1,
        TECH_HIDDEN = 0,
        TECH_ACTIVE = 1,
    };

    static bool player_has_unit_id(s32 player_id, u32 unit_id) {
        return Player::number_of_units_of_id(player_id, unit_id);
    }

    static bool player_has_completed_unit_id(s32 player_id, u32 unit_id) {
        return Player::number_of_completed_units_of_id(player_id, unit_id);
    }

    static bool player_has_completed_landed_unit_id(s32 player_id, u32 unit_id) {
        return Player::number_of_completed_landed_units_of_id(player_id, unit_id);
    }

    static bool player_has_completed_powered_unit_id(s32 player_id, u32 unit_id) {
        return Player::number_of_completed_powered_units_of_id(player_id, unit_id);
    }

    static bool player_has_completed_addon_unit_id(s32 player_id, u32 unit_id) {
        return Player::number_of_completed_addon_units_of_id(player_id, unit_id);
    }

    static bool player_has_base_unit_id(s32 player_id, u32 unit_id) {
        return Player::number_of_base_units_of_id(player_id, unit_id);
    }

    static bool player_has_gestated_unit_id(s32 player_id, u32 unit_id) {
        return Player::number_of_gestated_units_of_id(player_id, unit_id);
    }

    s32 create_unit_id_allowed(u16 unit_id, s32 player_id, bool ignore_creator) {
        *g_last_internal_error = InternalErrors::UnknownError0;
        if (unit_id == UnitId::None) {
            return false;
        }
        if (!g_unit_availability->available[player_id][unit_id]) {
            return false;
        }

        enum {
            Tier1 = 0,
            Tier2,
            Tier3,
            Tier4,
        };
        bool TierUnlocked[3][5]{};

        TierUnlocked[RaceId::Zerg][Tier1] = true;
        TierUnlocked[RaceId::Zerg][Tier2] = player_has_base_unit_id(player_id, UnitId::ZergIrolIris) || player_has_gestated_unit_id(player_id, UnitId::ZergOthstolOviform);
        TierUnlocked[RaceId::Zerg][Tier3] = player_has_completed_unit_id(player_id, UnitId::ZergOthstolOviform);

        TierUnlocked[RaceId::Terran][Tier1] = true;
        TierUnlocked[RaceId::Terran][Tier2] = player_has_completed_landed_unit_id(player_id, UnitId::TerranAtlas);
        TierUnlocked[RaceId::Terran][Tier3] = player_has_completed_landed_unit_id(player_id, UnitId::TerranDaedala);

        TierUnlocked[RaceId::Protoss][Tier1] = true;
        TierUnlocked[RaceId::Protoss][Tier2] = player_has_completed_powered_unit_id(player_id, UnitId::ProtossEmbassy)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossGateway)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossLattice);
        TierUnlocked[RaceId::Protoss][Tier3] = player_has_completed_powered_unit_id(player_id, UnitId::ProtossCrucible)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossGrandLibrary)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossRogueGallery)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossArdentAuthority)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossStargate);
        TierUnlocked[RaceId::Protoss][Tier4] = player_has_completed_powered_unit_id(player_id, UnitId::ProtossProstrationStage)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossUnitaryUnion)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossArgosy);

        Faction faction = g_globals->players[player_id].faction;

        if (bw::get_game_kind() == GameKind::DarkAges && bw::unit_id_blocked_in_dark_ages(static_cast<UnitId::Enum>(unit_id))) {
            return false;
        }

        // TERRAN
        // @Todo: Consider making AI check for landed structures as a safety, e.g. player_has_completed_landed_unit_id(player_id, UnitId::TerranMinistry)
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranMason) || player_has_completed_unit_id(player_id, UnitId::TerranTalos)
            || player_has_completed_unit_id(player_id, UnitId::TerranOgunWorkerMode)) {
            switch (unit_id) {
                case UnitId::TerranMinistry:
                case UnitId::TerranWatchdog:
                case UnitId::TerranAnchor:
                case UnitId::TerranTreasury:
                case UnitId::TerranSieve:
                case UnitId::TerranCestus:
                case UnitId::TerranAtlas:
                case UnitId::TerranStockade:
                case UnitId::TerranFulcrum:
                case UnitId::TerranStarpad: {
                    return true;
                } break;
                case UnitId::TerranReservoir:
                case UnitId::TerranFacade:
                case UnitId::TerranSentinel:
                case UnitId::TerranTencendur:
                case UnitId::TerranDaedala:
                case UnitId::TerranFountainhead:
                case UnitId::TerranCaptaincy:
                case UnitId::TerranMantle:
                case UnitId::TerranStarport: {
                    if (TierUnlocked[RaceId::Terran][Tier2]) {
                        return true;
                    }
                } break;
                case UnitId::TerranDiadem:
                case UnitId::TerranWellbore:
                case UnitId::TerranApothecary:
                case UnitId::TerranIronFoundry:
                case UnitId::TerranNaniteAssembly: {
                    if (TierUnlocked[RaceId::Terran][Tier3]) {
                        return true;
                    }
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranMinistry)) {
            switch (unit_id) {
                case UnitId::TerranMason:
                case UnitId::TerranLodestone:
                case UnitId::TerranQuarry:
                case UnitId::TerranAnvil: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranParthenon)) {
            switch (unit_id) {
                case UnitId::TerranArmistice: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranAnvil)) {
            switch (unit_id) {
                case UnitId::TerranMorningstar: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || (player_has_completed_unit_id(player_id, UnitId::TerranTreasury) && player_has_completed_addon_unit_id(player_id, UnitId::TerranQuarry))) {
            switch (unit_id) {
                case UnitId::TerranTalos:
                case UnitId::TerranOgunWorkerMode: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranMinistry) || player_has_completed_unit_id(player_id, UnitId::TerranTreasury)) {
            switch (unit_id) {
                case UnitId::TerranLodestone:
                case UnitId::TerranQuarry:
                case UnitId::TerranAnvil: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranAtlas)) {
            switch (unit_id) {
                case UnitId::TerranFountainhead: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranStockade)) {
            switch (unit_id) {
                case UnitId::TerranVestry:
                case UnitId::TerranCovenant: {
                    return true;
                } break;
                case UnitId::TerranMaverick: {
                    if (faction != Faction::TerranHecatonCoalition && faction != Faction::TerranFuturistFederation) {
                        return true;
                    }
                } break;
                case UnitId::TerranHotshot: {
                    if (faction == Faction::TerranHecatonCoalition) {
                        return true;
                    }
                } break;
                case UnitId::TerranMaverickFuturist: {
                    if (faction == Faction::TerranFuturistFederation) {
                        return true;
                    }
                } break;
                case UnitId::TerranHarakan: {
                    if (faction != Faction::TerranGoldenRepublic) {
                        return true;
                    }
                } break;
                case UnitId::TerranCyprian: {
                    if (faction != Faction::TerranNydusTrinity && faction != Faction::TerranUpstart) {
                        return true;
                    }
                } break;
                case UnitId::TerranHarakanGolden: {
                    if (faction == Faction::TerranGoldenRepublic) {
                        return true;
                    }
                } break;
                case UnitId::TerranCohort: {
                    if (faction == Faction::TerranNydusTrinity) {
                        return true;
                    }
                } break;
                case UnitId::TerranCannibal: {
                    if (faction == Faction::TerranUpstart) {
                        return true;
                    }
                } break;
                case UnitId::TerranCleric: {
                    if (player_has_completed_addon_unit_id(player_id, UnitId::TerranVestry) && faction != Faction::TerranFuturistFederation) {
                        return true;
                    }
                } break;
                case UnitId::TerranClericFuturist: {
                    if (player_has_completed_addon_unit_id(player_id, UnitId::TerranVestry) && faction == Faction::TerranFuturistFederation) {
                        return true;
                    }
                } break;
                case UnitId::TerranShamanSupportMode: {
                    if (player_has_completed_addon_unit_id(player_id, UnitId::TerranVestry)) {
                        return true;
                    }
                } break;
                case UnitId::TerranEidolon: {
                    if (player_has_completed_addon_unit_id(player_id, UnitId::TerranCovenant) && faction != Faction::TerranNydusTrinity) {
                        return true;
                    }
                } break;
                case UnitId::TerranEidolonNydus: {
                    if (player_has_completed_addon_unit_id(player_id, UnitId::TerranCovenant) && faction == Faction::TerranNydusTrinity) {
                        return true;
                    }
                } break;
                case UnitId::TerranSavantEntropyGauntlets: {
                    if (player_has_completed_addon_unit_id(player_id, UnitId::TerranCovenant)) {
                        return true;
                    }
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranCaptaincy)) {
            switch (unit_id) {
                case UnitId::TerranMadcap: {
                    if (faction != Faction::TerranFuturistFederation) {
                        return true;
                    }
                } break;
                case UnitId::TerranBlackguard: {
                    if (faction == Faction::TerranFuturistFederation) {
                        return true;
                    }
                } break;
                case UnitId::TerranHeracles:
                case UnitId::TerranTinkerersTower:
                case UnitId::TerranGuildhall: {
                    return true;
                } break;
                case UnitId::TerranApostle: {
                    if (faction != Faction::TerranHecatonCoalition) {
                        return true;
                    }
                } break;
                case UnitId::TerranAuriel: {
                    if (faction == Faction::TerranHecatonCoalition) {
                        return true;
                    }
                } break;
                case UnitId::TerranSiren:
                case UnitId::TerranStriga: {
                    if (player_has_completed_addon_unit_id(player_id, UnitId::TerranTinkerersTower)) {
                        return true;
                    }
                } break;
                case UnitId::TerranOlympian:
                case UnitId::TerranAutocrat: {
                    if (player_has_completed_addon_unit_id(player_id, UnitId::TerranGuildhall)) {
                        return true;
                    }
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranApothecary)) {
            switch (unit_id) {
                case UnitId::TerranDilettante:
                case UnitId::TerranSevenstep:
                case UnitId::TerranSilvertongue:
                case UnitId::TerranHypnagogue: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranFulcrum)) {
            switch (unit_id) {
                case UnitId::TerranScrapyard:
                case UnitId::TerranPalladium: {
                    return true;
                } break;
                case UnitId::TerranVulture:         return faction != Faction::TerranHecatonCoalition;
                case UnitId::TerranVultureHecaton:  return faction == Faction::TerranHecatonCoalition;
                case UnitId::TerranCyclops:         return faction != Faction::TerranFuturistFederation;
                case UnitId::TerranCyclopsFuturist: return faction == Faction::TerranFuturistFederation;
                case UnitId::TerranGoliath:         return faction != Faction::TerranHecatonCoalition && faction != Faction::TerranGoldenRepublic && faction != Faction::TerranUpstart;
                case UnitId::TerranArhaun:          return faction == Faction::TerranHecatonCoalition;
                case UnitId::TerranAspirant:        return faction == Faction::TerranGoldenRepublic;
                case UnitId::TerranSouthpaw:        return faction == Faction::TerranUpstart;
                case UnitId::TerranBlackjack:       return faction != Faction::TerranFuturistFederation && faction != Faction::TerranGoldenRepublic;
                case UnitId::TerranBlackjackGolden: return faction == Faction::TerranGoldenRepublic;
                case UnitId::TerranTiangou:         return faction == Faction::TerranFuturistFederation;
                case UnitId::TerranMatadorTankMode: {
                    if (player_has_completed_addon_unit_id(player_id, UnitId::TerranScrapyard)) {
                        return true;
                    }
                } break;
                case UnitId::TerranPhalanxTankMode: {
                    if (faction != Faction::TerranNydusTrinity) {
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranPalladium)) {
                            return true;
                        }
                    }
                } break;
                case UnitId::TerranTarasque: {
                    if (faction == Faction::TerranNydusTrinity) {
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranPalladium)) {
                            return true;
                        }
                    }
                } break;
                case UnitId::TerranMadrigalImperiosoMode: {
                    if (faction != Faction::TerranHecatonCoalition) {
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranPalladium)) {
                            return true;
                        }
                    }
                } break;
                case UnitId::TerranMadrigalHecatonImperiosoMode: {
                    if (faction == Faction::TerranHecatonCoalition) {
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranPalladium)) {
                            return true;
                        }
                    }
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranMantle)) {
            switch (unit_id) {
                case UnitId::TerranRamesses:
                case UnitId::TerranDurendal:
                case UnitId::TerranCataphract: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranIronFoundry)) {
            switch (unit_id) {
                case UnitId::TerranPaladin:
                case UnitId::TerranCuirassTankMode:
                case UnitId::TerranPazuzu:
                case UnitId::TerranBioticBastion:
                case UnitId::TerranFutureStation: {
                    return true;
                } break;
                case UnitId::TerranClaymore:
                case UnitId::TerranPenumbra: {
                    if (player_has_completed_addon_unit_id(player_id, UnitId::TerranFutureStation))
                    {
                        return true;
                    }
                } break;
                case UnitId::TerranAnticthon:
                case UnitId::TerranAion: {
                    if (player_has_completed_addon_unit_id(player_id, UnitId::TerranBioticBastion))
                    {
                        return true;
                    }
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranStarpad)) {
            switch (unit_id) {
                case UnitId::TerranWraith: {
                    if (faction != Faction::TerranFuturistFederation) {
                        return true;
                    }
                    return false;
                }
                case UnitId::TerranValkyrie: {
                    return true;
                }
                case UnitId::TerranAncile: {
                    if (faction != Faction::TerranUpstart) {
                        return true;
                    }
                    return false;
                }
                case UnitId::TerranTrojan: {
                    if (faction != Faction::TerranHecatonCoalition) {
                        return true;
                    }
                    return false;
                }
                case UnitId::TerranPhiloc: {
                    if (faction == Faction::TerranFuturistFederation) {
                        return true;
                    }
                    return false;
                }
                case UnitId::TerranWendigo: {
                    if (faction == Faction::TerranUpstart) {
                        return true;
                    }
                    return false;
                } break;
                case UnitId::TerranQilin: {
                    if (faction == Faction::TerranHecatonCoalition) {
                        return true;
                    }
                    return false;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranStarport)) {
            switch (unit_id) {
                case UnitId::TerranGorgon:
                case UnitId::TerranWyvern:
                case UnitId::TerranPythean:
                case UnitId::TerranRotary:
                case UnitId::TerranCommandment: {
                    return true;
                } break;
                case UnitId::TerranSundog:
                case UnitId::TerranSalamander: {
                    if (player_has_completed_addon_unit_id(player_id, UnitId::TerranRotary)) {
                        return true;
                    }
                } break;
                case UnitId::TerranSeraph: {
                    if (faction != Faction::TerranGoldenRepublic) {
                        if (ignore_creator || player_has_completed_addon_unit_id(player_id, UnitId::TerranCommandment)) {
                            return true;
                        }
                    }
                } break;
                case UnitId::TerranAzazel: {
                    if (ignore_creator || player_has_completed_addon_unit_id(player_id, UnitId::TerranCommandment)) {
                        return true;
                    }
                } break;
                case UnitId::TerranLuminary: {
                    if (faction == Faction::TerranGoldenRepublic) {
                        if (ignore_creator || player_has_completed_addon_unit_id(player_id, UnitId::TerranCommandment)) {
                            return true;
                        }
                    }
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranNaniteAssembly)) {
            switch (unit_id) {
                case UnitId::TerranHadron:
                case UnitId::TerranCentaur:
                case UnitId::TerranMinotaur:
                case UnitId::TerranMagnetar: {
                    return true;
                } break;
                case UnitId::TerranHippogriff:
                case UnitId::TerranPhobos: {
                    if (player_has_completed_addon_unit_id(player_id, UnitId::TerranHadron)) {
                        return true;
                    }
                } break;
            }
        }

        // PROTOSS
        // Units
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossNexus)) {
            if (unit_id == UnitId::ProtossScribe) {
                return true;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossEmbassy)) {
            switch (unit_id) {
                case UnitId::ProtossArtisan:
                case UnitId::ProtossEnvoy:
                case UnitId::ProtossWitness:
                case UnitId::ProtossAutocort: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossStridentStratum)) {
            switch (unit_id) {
                case UnitId::ProtossMonogram: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossGateway)) {
            switch (unit_id) {
                case UnitId::ProtossZealot: {
                    if (faction != Faction::ProtossStarbound) {
                        return true;
                    }
                }
                case UnitId::ProtossZealotStarbound: {
                    if (faction == Faction::ProtossStarbound) {
                        return true;
                    }
                }
                case UnitId::ProtossDracadin: {
                    if (faction != Faction::ProtossOrderOfTheSacredSaber) {
                        return true;
                    }
                } break;
                case UnitId::ProtossDracadinOrder: {
                    if (faction == Faction::ProtossKhalendricKnights) {
                        return true;
                    }
                }
                case UnitId::ProtossEcclesiast: {
                    if (faction != Faction::ProtossKhalendricKnights) {
                        return true;
                    }
                }
                case UnitId::ProtossEcclesiastKhalendric: {
                    if (faction == Faction::ProtossKhalendricKnights) {
                        return true;
                    }
                }
                case UnitId::ProtossLegionnaire: {
                    if (faction != Faction::ProtossOrderOfTheSacredSaber && faction != Faction::ProtossStarbound) {
                        return true;
                    }
                }
                case UnitId::ProtossAdherent: {
                    if (faction == Faction::ProtossOrderOfTheSacredSaber) {
                        return true;
                    }
                }
                case UnitId::ProtossCicatus: {
                    if (faction == Faction::ProtossStarbound) {
                        return true;
                    }
                }
                case UnitId::ProtossHierophant: {
                	if (faction != Faction::ProtossKhalendricKnights) {
                        return true;
                	}
                }
                case UnitId::ProtossAshguard: {
                    if (faction == Faction::ProtossKhalendricKnights) {
                    	return true;
                    }
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossGrandLibrary)) {
            switch (unit_id) {
                case UnitId::ProtossAmaranth:
                case UnitId::ProtossHerald:
                case UnitId::ProtossAtreus:
                case UnitId::ProtossCantavis: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossRogueGallery)) {
            switch (unit_id) {
                case UnitId::ProtossVagrant:
                case UnitId::ProtossCabalist:
                case UnitId::ProtossCharlatan:
                case UnitId::ProtossBarghest: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossProstrationStage)) {
            switch (unit_id) {
                case UnitId::ProtossPariah:
                case UnitId::ProtossPotentate:
                case UnitId::ProtossSycophant:
                case UnitId::ProtossMindTyrant: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossAncestralArchives)) {
            switch (unit_id) {
                case UnitId::ProtossArchon:
                case UnitId::ProtossOptecton:
                case UnitId::ProtossPatriarch: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossHallOfHeretics)) {
            switch (unit_id) {
                case UnitId::ProtossApostate:
                case UnitId::ProtossAugur: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossLattice)) {
            switch (unit_id) {
                case UnitId::ProtossVassal: {
                    if (faction != Faction::ProtossOrderOfTheSacredSaber) {
                        return true;
                    }
                } break;
                case UnitId::ProtossRenesair: {
                    if (faction == Faction::ProtossOrderOfTheSacredSaber) {
                        return true;
                    }
                } break;
                case UnitId::ProtossSimulacrum: {
                    return true;
                } break;
                case UnitId::ProtossManifold: {
                    if (faction != Faction::ProtossStarbound) {
                        return true;
                    }
                } break;
                case UnitId::ProtossManifoldStarbound: {
                    if (faction == Faction::ProtossStarbound) {
                        return true;
                    }
                } break;
                case UnitId::ProtossGolem:
                case UnitId::ProtossIdol: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossArdentAuthority)) {
            switch (unit_id) {
                case UnitId::ProtossServitor:
                case UnitId::ProtossPositron:
                case UnitId::ProtossAccantor:
                case UnitId::ProtossClarion: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossAuxiliaryOasis)) {
            switch (unit_id) {
                case UnitId::ProtossOscilliant:
                case UnitId::ProtossUnar:
                case UnitId::ProtossArchitect:
                case UnitId::ProtossVibrance: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossSyntheticSynod)) {
            switch (unit_id) {
                case UnitId::ProtossAnalogue:
                case UnitId::ProtossDemiurge:
                case UnitId::ProtossEmpress: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossStargate)) {
            switch (unit_id) {
                case UnitId::ProtossAurora: {
                    if (faction != Faction::ProtossOrderOfTheSacredSaber) {
                        return true;
                    }
                } break;
                case UnitId::ProtossExtatus: {
                    if (faction == Faction::ProtossOrderOfTheSacredSaber) {
                        return true;
                    }
                } break;
                case UnitId::ProtossPanoptus:
                case UnitId::ProtossLanifect:
                case UnitId::ProtossGladius:
                case UnitId::ProtossExemplar: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossArgosy)) {
            switch (unit_id) {
                case UnitId::ProtossMagister:
                case UnitId::ProtossEpigraph:
                case UnitId::ProtossDidact:
                case UnitId::ProtossEmpyrean:
                case UnitId::ProtossSolarion: {
                    return true;
                } break;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossMonumentOfSin)) {
            switch (unit_id) {
                case UnitId::ProtossAnthelion:
                case UnitId::ProtossStarSovereign: {
                    return true;
                } break;
            }
        }

        // Hangar units
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossAccantorOld)) {
            if (unit_id == UnitId::ProtossPlasmaShell) {
                return true;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossSolarion)) {
            if (unit_id == UnitId::ProtossSteward) {
                return true;
            }
        }

        // Structures
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossScribe) || player_has_completed_unit_id(player_id, UnitId::ProtossAutocort)) {
            switch (unit_id) {
                case UnitId::ProtossNexus:
                case UnitId::ProtossPylon:
                case UnitId::ProtossCedent:
                case UnitId::ProtossBulwark:
                case UnitId::ProtossWarden:
                case UnitId::ProtossEmbassy:
                case UnitId::ProtossGateway:
                case UnitId::ProtossLattice: {
                    return true;
                } break;
                case UnitId::ProtossCrucible:
                case UnitId::ProtossAquifer:
                case UnitId::ProtossMatrix:
                case UnitId::ProtossGrandLibrary:
                case UnitId::ProtossRogueGallery:
                case UnitId::ProtossArdentAuthority:
                case UnitId::ProtossStargate: {
                    if (TierUnlocked[RaceId::Protoss][Tier2]) {
                        return true;
                    }
                } break;
                case UnitId::ProtossEngram:
                case UnitId::ProtossProstrationStage:
                case UnitId::ProtossUnitaryUnion:
                case UnitId::ProtossArgosy: {
                    if (TierUnlocked[RaceId::Protoss][Tier3]) {
                        return true;
                    }
                } break;
                case UnitId::ProtossXenolith:
                case UnitId::ProtossAncestralArchives:
                case UnitId::ProtossSyntheticSynod:
                case UnitId::ProtossMonumentOfSin: {
                    if (TierUnlocked[RaceId::Protoss][Tier4]) {
                        return true;
                    }
                } break;
            }
        }

        // ZERG
        // Units
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergLarva)) {
            switch (unit_id) {
                case UnitId::ZergDroleth:
                case UnitId::ZergRilirokor: {
                    return true;
                } break;
                case UnitId::ZergAgrileth: {
                    if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_gestated_unit_id(player_id, UnitId::ZergVornathPond) || player_has_base_unit_id(player_id, UnitId::ZergQuazethPool)) {
                        return true;
                    }
                } break;
                case UnitId::ZergOvileth: {
                    if (faction != Faction::ZergZorthosTendril) {
                        return true;
                    }
                } break;
                case UnitId::ZergYblyleth: {
                    if (faction == Faction::ZergZorthosTendril) {
                        return true;
                    }
                } break;
                case UnitId::ZergOthstoleth: {
                    if (TierUnlocked[RaceId::Zerg][Tier3]) {
                        return true;
                    }
                } break;
                case UnitId::ZergZethrokor: {
                    if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_gestated_unit_id(player_id, UnitId::ZergVornathPond) || player_has_base_unit_id(player_id, UnitId::ZergQuazethPool)) {
                        return true;
                    }
                } break;
                case UnitId::ZergQuazilisk: {
                    if (faction != Faction::ZergImicrenTendril) {
                        if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_gestated_unit_id(player_id, UnitId::ZergVornathPond) || player_has_base_unit_id(player_id, UnitId::ZergQuazethPool)) {
                            return true;
                        }
                    }
                } break;
                case UnitId::ZergQuazrokor: {
                    if (faction == Faction::ZergImicrenTendril) {
                        if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_gestated_unit_id(player_id, UnitId::ZergVornathPond) || player_has_base_unit_id(player_id, UnitId::ZergQuazethPool)) {
                            return true;
                        }
                    }
                } break;
                case UnitId::ZergNathrokor: {
                    if (faction != Faction::ZergZorthosTendril) {
                        if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_base_unit_id(player_id, UnitId::ZergVornathPond)) {
                            return true;
                        }
                    }
                } break;
                case UnitId::ZergNathrelisk: {
                    if (faction == Faction::ZergZorthosTendril) {
                        if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_base_unit_id(player_id, UnitId::ZergVornathPond)) {
                            return true;
                        }
                    }
                } break;
                case UnitId::ZergMutalisk:
                case UnitId::ZergProtathalor:
                case UnitId::ZergCikralisk: {
                    if (player_has_unit_id(player_id, UnitId::ZergTosvrolHaunt) || player_has_base_unit_id(player_id, UnitId::ZergMutathSpire)) {
                        return true;
                    }
                } break;
                case UnitId::ZergMortothrokor:
                case UnitId::ZergAxitrilisk:
                case UnitId::ZergSasyrvoleth: {
                    if (player_has_unit_id(player_id, UnitId::ZergBegrozithBarrow) || player_has_base_unit_id(player_id, UnitId::ZergAxtothAxis)) {
                        return true;
                    }
                } break;
                case UnitId::ZergHydralisk: {
                    if (faction != Faction::ZergOrkotiTendril) {
                        if (player_has_unit_id(player_id, UnitId::ZergTethvithSwamp) || player_has_gestated_unit_id(player_id, UnitId::ZergSkibactScab) || player_has_base_unit_id(player_id, UnitId::ZergHydrithDen)) {
                            return true;
                        }
                    }
                } break;
                case UnitId::ZergHydraliskOrkoti: {
                    if (faction == Faction::ZergOrkotiTendril) {
                        if (player_has_unit_id(player_id, UnitId::ZergTethvithSwamp) || player_has_gestated_unit_id(player_id, UnitId::ZergSkibactScab) || player_has_base_unit_id(player_id, UnitId::ZergHydrithDen)) {
                            return true;
                        }
                    }
                } break;
                case UnitId::ZergIzirokor: {
                    if (faction != Faction::ZergZorthosTendril) {
                        if (player_has_unit_id(player_id, UnitId::ZergTethvithSwamp) || player_has_gestated_unit_id(player_id, UnitId::ZergSkibactScab) || player_has_base_unit_id(player_id, UnitId::ZergHydrithDen)) {
                            return true;
                        }
                    }
                } break;
                case UnitId::ZergIziralisk: {
                    if (faction == Faction::ZergZorthosTendril) {
                        if (player_has_unit_id(player_id, UnitId::ZergTethvithSwamp) || player_has_gestated_unit_id(player_id, UnitId::ZergSkibactScab) || player_has_base_unit_id(player_id, UnitId::ZergHydrithDen)) {
                            return true;
                        }
                    }
                } break;
                case UnitId::ZergSkithrokor: {
                    if (player_has_unit_id(player_id, UnitId::ZergTethvithSwamp) || player_has_base_unit_id(player_id, UnitId::ZergSkibactScab)) {
                        return true;
                    }
                } break;
                case UnitId::ZergKagralisk:
                case UnitId::ZergEvigrilisk:
                case UnitId::ZergAlmaksalisk: {
                    if (player_has_unit_id(player_id, UnitId::ZergChokralChazsal) || player_has_base_unit_id(player_id, UnitId::ZergAlkagIteth)) {
                        return true;
                    }
                } break;
                case UnitId::ZergZoryusthaleth: {
                    if (faction != Faction::ZergImicrenTendril) {
                        if (player_has_unit_id(player_id, UnitId::ZergZozarcGorge) || player_has_gestated_unit_id(player_id, UnitId::ZergUlgorgCavern) || player_has_base_unit_id(player_id, UnitId::ZergZorkizShroud)) {
                            return true;
                        }
                    }
                } break;
                case UnitId::ZergInzorthrokor: {
                    if (faction == Faction::ZergImicrenTendril) {
                        if (player_has_unit_id(player_id, UnitId::ZergZozarcGorge) || player_has_gestated_unit_id(player_id, UnitId::ZergUlgorgCavern) || player_has_base_unit_id(player_id, UnitId::ZergZorkizShroud)) {
                            return true;
                        }
                    }
                } break;
                case UnitId::ZergLakizilisk: {
                    if (player_has_unit_id(player_id, UnitId::ZergZozarcGorge) || player_has_gestated_unit_id(player_id, UnitId::ZergUlgorgCavern) || player_has_base_unit_id(player_id, UnitId::ZergZorkizShroud)) {
                        return true;
                    }
                } break;
                case UnitId::ZergGorgrokor: {
                    if (player_has_unit_id(player_id, UnitId::ZergZozarcGorge) || player_has_base_unit_id(player_id, UnitId::ZergUlgorgCavern)) {
                        return true;
                    }
                } break;
                case UnitId::ZergKonvilisk:
                case UnitId::ZergVilgorokor:
                case UnitId::ZergMatraleth: {
                    if (player_has_unit_id(player_id, UnitId::ZergAlmakisAntre) || player_has_base_unit_id(player_id, UnitId::ZergMatravilNest)) {
                        return true;
                    }
                } break;
                case UnitId::ZergKeskathalor:
                case UnitId::ZergGeszithalor: {
                    if (player_has_unit_id(player_id, UnitId::ZergKethrajKeep) || player_has_base_unit_id(player_id, UnitId::ZergGeszkathGrotto)) {
                        return true;
                    }
                } break;
            }
        }

        // Morphs
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergDroleth) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergGosvileth) {
                if (TierUnlocked[RaceId::Zerg][Tier2]) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)
        ||  player_has_completed_unit_id(player_id, UnitId::ZergOvileth) || player_has_completed_unit_id(player_id, UnitId::ZergYblyleth)) {
            if (unit_id == UnitId::ZergIroleth) {
                if (TierUnlocked[RaceId::Zerg][Tier2]) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergZethrokor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergVorvrokor) {
                if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_base_unit_id(player_id, UnitId::ZergVornathPond)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)
        ||  player_has_completed_unit_id(player_id, UnitId::ZergQuazilisk) || player_has_completed_unit_id(player_id, UnitId::ZergQuazrokor)) {
            if (unit_id == UnitId::ZergLiiralisk) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergKalkiirLake)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)
        ||  player_has_completed_unit_id(player_id, UnitId::ZergNathrokor) || player_has_completed_unit_id(player_id, UnitId::ZergNathrelisk)) {
            if (unit_id == UnitId::ZergKalkalisk) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergKalkiirLake)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergCikralisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergGhitorokor) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergTosvrolHaunt)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergProtathalor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergSovroleth) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergTosvrolHaunt)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergMutalisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergTosgrilisk) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergTosvrolHaunt)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergAxitrilisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergBegrothilisk) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergBegrozithBarrow)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergMortothrokor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergOztukrokor) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergBegrozithBarrow)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)
        ||  player_has_completed_unit_id(player_id, UnitId::ZergHydralisk) || player_has_completed_unit_id(player_id, UnitId::ZergHydraliskOrkoti)) {
            if (unit_id == UnitId::ZergBactalisk) {
                if (player_has_unit_id(player_id, UnitId::ZergTethvithSwamp) || player_has_base_unit_id(player_id, UnitId::ZergSkibactScab)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)
        ||  player_has_completed_unit_id(player_id, UnitId::ZergIzirokor) || player_has_completed_unit_id(player_id, UnitId::ZergIziralisk)) {
            if (unit_id == UnitId::ZergTethzorokor) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergTethvithSwamp)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergSkithrokor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergVithrilisk) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergTethvithSwamp)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergKagralisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergCherbrathalor) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergChokralChazsal)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergEvigrilisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergOkrimnilisk) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergChokralChazsal)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergAlmaksalisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergAlgoztalisk) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergChokralChazsal)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergGeszithalor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergAlkajelisk) {
                if (player_has_base_unit_id(player_id, UnitId::ZergGeszkathGrotto) || player_has_completed_unit_id(player_id, UnitId::ZergKethrajKeep)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)
        ||  player_has_completed_unit_id(player_id, UnitId::ZergZoryusthaleth) || player_has_completed_unit_id(player_id, UnitId::ZergInzorthrokor)) {
            if (unit_id == UnitId::ZergUltrokor) {
                if (ignore_creator || player_has_base_unit_id(player_id, UnitId::ZergUlgorgCavern || player_has_unit_id(player_id, UnitId::ZergZozarcGorge))) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergLakizilisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergZobriolisk) {
                if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergZozarcGorge)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergGorgrokor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergZarcavrokor) {
                if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergZozarcGorge)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergKonvilisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergTetcaerokor) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergVilgorokor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergIsthrathaleth) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre)) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergMatraleth) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva)) {
            if (unit_id == UnitId::ZergAkistrokor) {
                if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre)) {
                    return true;
                }
            }
        }

        // Structures
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergDroleth) || player_has_completed_unit_id(player_id, UnitId::ZergGosvileth)
            || player_has_completed_unit_id(player_id, UnitId::ZergAgrileth)) {
            switch (unit_id) {
                case UnitId::ZergHachirosk:
                case UnitId::ZergCagrantCesiant:
                case UnitId::ZergEskigant:
                case UnitId::ZergQuazethPool:
                case UnitId::ZergHydrithDen:
                case UnitId::ZergZorkizShroud:
                case UnitId::ZergIrolIris: {
                    return true;
                } break;
                case UnitId::ZergExcisant:
                case UnitId::ZergMutathSpire:
                case UnitId::ZergAlkagIteth:
                case UnitId::ZergMatravilNest: {
                    if (TierUnlocked[RaceId::Zerg][Tier2]) {
                        return true;
                    }
                } break;
                case UnitId::ZergAxtothAxis:
                case UnitId::ZergGeszkathGrotto:
                case UnitId::ZergElthisthMound: {
                    if (TierUnlocked[RaceId::Zerg][Tier3]) {
                        return true;
                    }
                } break;
            }
        }

        // Structure morphs
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergCagrantCesiant)) {
            if (unit_id == UnitId::ZergSurkithCesiant
            ||  unit_id == UnitId::ZergSpraithCesiant
            ||	unit_id == UnitId::ZergLarvoskCesiant
            ||	unit_id == UnitId::ZergKazitrithCesiant) {
                return true;
            }
            if (unit_id == UnitId::ZergNydusCesiant) {
                if (TierUnlocked[RaceId::Zerg][Tier2]) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergSurkithCesiant)) {
            if (unit_id == UnitId::ZergSkortrithCesiant) {
                if (TierUnlocked[RaceId::Zerg][Tier2]) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergSpraithCesiant)) {
            if (unit_id == UnitId::ZergSpirtithCesiant) {
                if (TierUnlocked[RaceId::Zerg][Tier2]) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergLarvoskCesiant)) {
            if (unit_id == UnitId::ZergOcsalCesiant) {
                if (TierUnlocked[RaceId::Zerg][Tier2]) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergHachirosk)) {
            if (unit_id == UnitId::ZergCaphrolosk) {
                if (TierUnlocked[RaceId::Zerg][Tier2]) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergExcisant)) {
            if (unit_id == UnitId::ZergEzigrisant) {
                if (TierUnlocked[RaceId::Zerg][Tier3]) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergQuazethPool)) {
            if (unit_id == UnitId::ZergVornathPond) {
                return true;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergVornathPond)) {
            if (unit_id == UnitId::ZergKalkiirLake) {
                return true;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergHydrithDen)) {
            if (unit_id == UnitId::ZergSkibactScab) {
                return true;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergSkibactScab)) {
            if (unit_id == UnitId::ZergTethvithSwamp) {
                return true;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergZorkizShroud)) {
            if (unit_id == UnitId::ZergUlgorgCavern) {
                return true;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergUlgorgCavern)) {
            if (unit_id == UnitId::ZergZozarcGorge) {
                return true;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergMutathSpire)) {
            if (unit_id == UnitId::ZergTosvrolHaunt) {
                return true;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergIrolIris)) {
            if (unit_id == UnitId::ZergOthstolOviform) {
                return true;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergHachirosk)) {
            if (unit_id == UnitId::ZergGathtelosk) {
                if (TierUnlocked[RaceId::Zerg][Tier3]) {
                    return true;
                }
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergMatravilNest)) {
            if (unit_id == UnitId::ZergAlmakisAntre) {
                return true;
            }
        }
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergAlkagIteth)) {
            if (unit_id == UnitId::ZergChokralChazsal) {
                return true;
            }
        }

        // ASKOSI
        // Units
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::AskosiLandlock)) {
            if (unit_id == UnitId::AskosiPilgrim) {
                // Proselyte, Dyad
                return true;
            }
        }

        // Structures
        if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::AskosiPilgrim)) { // And Dyad
            switch (unit_id) {
                case UnitId::AskosiLandlock:
                case UnitId::AskosiArch:
                case UnitId::AskosiGrandstand:
                case UnitId::AskosiChantry:
                case UnitId::AskosiRadix:
                case UnitId::AskosiCipher: {
                    return true;
                }
                case UnitId::AskosiFunction:
                case UnitId::AskosiGauntlet: {
                    if (player_has_completed_unit_id(player_id, UnitId::AskosiRadix) || player_has_completed_unit_id(player_id, UnitId::AskosiFunction)) {
                        return true;
                    }
                }
                case UnitId::AskosiCosmoscope: {
                    if (player_has_completed_unit_id(player_id, UnitId::AskosiCipher) || player_has_completed_unit_id(player_id, UnitId::AskosiGauntlet)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    s32 create_unit_allowed_partial(u16 unit_id, Unit* unit, s32 player_id) {
        *g_last_internal_error = InternalErrors::UnknownError0;
        if (unit_id == UnitId::None) {
            *g_last_internal_error = InternalErrors::UnknownError16;
            return TECH_HIDDEN;
        }
        if (player_id != unit->player) {
            *g_last_internal_error = InternalErrors::UnknownError1;
            return TECH_HIDDEN;
        }
        if (!(unit->status & UnitStatus::Completed)) {
            *g_last_internal_error = InternalErrors::UnknownError20;
            return TECH_HIDDEN;
        }
        if (unit->is_frozen()) {
            *g_last_internal_error = InternalErrors::UnitFrozen;
            return TECH_HIDDEN;
        }
        if (!g_unit_availability->available[unit->player][unit_id]) {
            *g_last_internal_error = InternalErrors::UnknownError2;
            return TECH_HIDDEN;
        }
        if (unit->is_queue_slot_active(4)) {
            *g_last_internal_error = InternalErrors::UnknownError3;
            return TECH_HIDDEN;
        }
        if (unit->status & UnitStatus::IsHallucination) {
            *g_last_internal_error = InternalErrors::UnknownError22;
            return TECH_HIDDEN;
        }
        if (bw::get_game_kind() == GameKind::DarkAges && bw::unit_id_blocked_in_dark_ages(static_cast<UnitId::Enum>(unit->id))) {
            return false;
        }


        // New hidden cases
        if (helper::is_constructing_addon(unit)) {
            return TECH_HIDDEN;
        }
        if (units_dat::base_property[unit->id] & UnitProperty::Addon) {
            UnitExt* unit_ext = get_unit_ext(unit);
            if (unit_ext->values[ValueAddonInactive]) {
                return TECH_HIDDEN;
            }
        }

        enum {
            Tier1 = 0,
            Tier2,
            Tier3,
            Tier4,
        };

        bool TierUnlocked[3][4]{};
        TierUnlocked[RaceId::Zerg][Tier1] = true;
        TierUnlocked[RaceId::Zerg][Tier2] = player_has_base_unit_id(player_id, UnitId::ZergIrolIris)
                                        ||  player_has_gestated_unit_id(player_id, UnitId::ZergOthstolOviform);
        TierUnlocked[RaceId::Zerg][Tier3] = player_has_completed_unit_id(player_id, UnitId::ZergOthstolOviform);

        TierUnlocked[RaceId::Terran][Tier1] = true;
        TierUnlocked[RaceId::Terran][Tier2] = player_has_completed_landed_unit_id(player_id, UnitId::TerranAtlas);
        TierUnlocked[RaceId::Terran][Tier3] = player_has_completed_landed_unit_id(player_id, UnitId::TerranDaedala);

        TierUnlocked[RaceId::Protoss][Tier1] = true;
        TierUnlocked[RaceId::Protoss][Tier2] = player_has_completed_powered_unit_id(player_id, UnitId::ProtossEmbassy)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossGateway)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossLattice);
        TierUnlocked[RaceId::Protoss][Tier3] = player_has_completed_powered_unit_id(player_id, UnitId::ProtossCrucible)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossGrandLibrary)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossRogueGallery)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossArdentAuthority)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossStargate);
        TierUnlocked[RaceId::Protoss][Tier4] = player_has_completed_powered_unit_id(player_id, UnitId::ProtossProstrationStage)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossUnitaryUnion)
            || player_has_completed_powered_unit_id(player_id, UnitId::ProtossArgosy);

        Faction faction = g_globals->players[player_id].faction;

        bool has_addon = false;
        if (unit->building.addon) {
            if (!(unit->building.addon >= g_unit_table)) { // This is a crash case when Vultures are group-selected with buildings! -Pr0nogo
                return TECH_HIDDEN;
            }
            has_addon = true;
        }
        bool in_air = unit->status & UnitStatus::InAir;
        bool is_training = unit->build_queue[unit->build_queue_slot] != UnitId::None;
        bool is_adding_on = units_dat::base_property[unit->build_queue[unit->build_queue_slot]] & UnitProperty::Addon;

        switch (unit->id) {
            // TERRAN
            case UnitId::TerranMason:
            case UnitId::TerranTalos:
            case UnitId::TerranOgunWorkerMode: {
                if (unit->main_order_id == OrderId::ConstructingBuilding) {
                    return TECH_HIDDEN;
                }

                switch (unit_id) {
                    case UnitId::TerranMinistry:
                    case UnitId::TerranWatchdog:
                    case UnitId::TerranAnchor:
                    case UnitId::TerranTreasury:
                    case UnitId::TerranSieve:
                    case UnitId::TerranCestus:
                    case UnitId::TerranAtlas:
                    case UnitId::TerranStockade:
                    case UnitId::TerranFulcrum:
                    case UnitId::TerranStarpad: {
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranReservoir:
                    case UnitId::TerranFacade:
                    case UnitId::TerranSentinel:
                    case UnitId::TerranTencendur:
                    case UnitId::TerranDaedala:
                    case UnitId::TerranCaptaincy:
                    case UnitId::TerranMantle:
                    case UnitId::TerranStarport: {
                        if (TierUnlocked[RaceId::Terran][Tier2]) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::TerranDiadem:
                    case UnitId::TerranWellbore:
                    case UnitId::TerranApothecary:
                    case UnitId::TerranIronFoundry:
                    case UnitId::TerranNaniteAssembly: {
                        if (TierUnlocked[RaceId::Terran][Tier3]) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                }
            } break;
            case UnitId::TerranMinistry: {
                switch (unit_id) {
                    case UnitId::TerranMason: {
                        if (in_air) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranTalos: {
                    case UnitId::TerranOgunWorkerMode:
                        if (in_air) {
                            return TECH_HIDDEN;
                        }
                        if (!player_has_completed_addon_unit_id(player_id, UnitId::TerranQuarry)) {
                            return TECH_LOCKED;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranLodestone:
                    case UnitId::TerranQuarry:
                    case UnitId::TerranAnvil: {
                        if (has_addon || is_training) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                }
            } break;
            case UnitId::TerranAnvil: {
                switch (unit_id) {
                    case UnitId::TerranMorningstar: {
                        if (!unit->building.silo.nuke && unit->build_queue[unit->build_queue_slot] == UnitId::None) {
                            return TECH_ACTIVE;
                        }
                    }
                }
            } break;
            case UnitId::TerranTreasury: {
                switch (unit_id) {
                    case UnitId::TerranTalos: {
                    case UnitId::TerranOgunWorkerMode:
                        if (in_air) {
                            return TECH_HIDDEN;
                        }
                        if (!player_has_completed_addon_unit_id(player_id, UnitId::TerranQuarry)) {
                            return TECH_LOCKED;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranLodestone:
                    case UnitId::TerranQuarry:
                    case UnitId::TerranAnvil: {
                        if (has_addon || is_training) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                }
            } break;
            case UnitId::TerranStockade: {
                switch (unit_id) {
                    case UnitId::TerranVestry:
                    case UnitId::TerranCovenant: {
                        if (has_addon || is_training) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranMaverick: {
                        if (faction != Faction::TerranHecatonCoalition && faction != Faction::TerranFuturistFederation) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranHotshot: {
                        if (faction == Faction::TerranHecatonCoalition) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranMaverickFuturist: {
                        if (faction == Faction::TerranFuturistFederation) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranHarakan: {
                        if (faction != Faction::TerranGoldenRepublic) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranCyprian: {
                        if (faction != Faction::TerranNydusTrinity && faction != Faction::TerranUpstart) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranCleric: {
                        if (in_air) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranVestry) && faction != Faction::TerranFuturistFederation) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranClericFuturist: {
                        if (in_air) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranVestry) && faction == Faction::TerranFuturistFederation) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranShamanSupportMode: {
                        if (in_air) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranVestry)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranEidolon: {
                        if (faction != Faction::TerranNydusTrinity) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            if (player_has_completed_addon_unit_id(player_id, UnitId::TerranCovenant)) {
                                return TECH_ACTIVE;
                            }
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranEidolonNydus: {
                        if (faction == Faction::TerranNydusTrinity) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            if (player_has_completed_addon_unit_id(player_id, UnitId::TerranCovenant)) {
                                return TECH_ACTIVE;
                            }
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranSavantEntropyGauntlets: {
                        if (in_air) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranCovenant)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranHarakanGolden: {
                        if (faction == Faction::TerranGoldenRepublic) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranCohort: {
                        if (faction == Faction::TerranNydusTrinity) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranCannibal: {
                        if (faction == Faction::TerranUpstart) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                }
                return TECH_HIDDEN;
            } break;
            case UnitId::TerranCaptaincy: {
                switch (unit_id) {
                    case UnitId::TerranTinkerersTower:
                    case UnitId::TerranGuildhall: {
                        if (has_addon || is_training) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranMadcap: {
                        if (faction != Faction::TerranFuturistFederation) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranBlackguard: {
                        if (faction == Faction::TerranFuturistFederation) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranHeracles: {
                        if (in_air) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranApostle: {
                        if (faction != Faction::TerranHecatonCoalition) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranAuriel: {
                        if (faction == Faction::TerranHecatonCoalition) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranSiren:
                    case UnitId::TerranStriga: {
                        if (in_air) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranTinkerersTower)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranOlympian:
                    case UnitId::TerranAutocrat: {
                        if (in_air) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranGuildhall)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                }
            } break;
            case UnitId::TerranApothecary: {
                switch (unit_id) {
                    case UnitId::TerranDilettante:
                    case UnitId::TerranSevenstep:
                    case UnitId::TerranSilvertongue:
                    case UnitId::TerranHypnagogue: {
                        if (in_air) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                }
            } break;
            case UnitId::TerranFulcrum: {
                switch (unit_id) {
                    case UnitId::TerranScrapyard:
                    case UnitId::TerranPalladium: {
                        if (has_addon || is_training) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranVulture: {
                        if (faction != Faction::TerranHecatonCoalition) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranVultureHecaton: {
                        if (faction == Faction::TerranHecatonCoalition) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranCyclops: {
                        if (faction != Faction::TerranFuturistFederation) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranCyclopsFuturist: {
                        if (faction == Faction::TerranFuturistFederation) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranGoliath: {
                        if (faction != Faction::TerranHecatonCoalition && faction != Faction::TerranGoldenRepublic && faction != Faction::TerranUpstart) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranBlackjack: {
                        if (faction != Faction::TerranFuturistFederation && faction != Faction::TerranGoldenRepublic) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            if (player_has_completed_addon_unit_id(player_id, UnitId::TerranScrapyard)) {
                                return TECH_ACTIVE;
                            }
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranBlackjackGolden: {
                        if (faction == Faction::TerranGoldenRepublic) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            if (player_has_completed_addon_unit_id(player_id, UnitId::TerranScrapyard)) {
                                return TECH_ACTIVE;
                            }
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranTiangou: {
                        if (faction == Faction::TerranFuturistFederation) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            if (player_has_completed_addon_unit_id(player_id, UnitId::TerranScrapyard)) {
                                return TECH_ACTIVE;
                            }
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranMatadorTankMode: {
                        if (in_air || is_adding_on) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranScrapyard)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranPhalanxTankMode: {
                        if (faction != Faction::TerranNydusTrinity) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            if (player_has_completed_addon_unit_id(player_id, UnitId::TerranPalladium)) {
                                return TECH_ACTIVE;
                            }
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranMadrigalImperiosoMode: {
                        if (faction != Faction::TerranHecatonCoalition) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            if (player_has_completed_addon_unit_id(player_id, UnitId::TerranPalladium)) {
                                return TECH_ACTIVE;
                            }
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranMadrigalHecatonImperiosoMode: {
                        if (faction == Faction::TerranHecatonCoalition) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            if (player_has_completed_addon_unit_id(player_id, UnitId::TerranPalladium)) {
                                return TECH_ACTIVE;
                            }
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranArhaun: {
                        if (faction == Faction::TerranHecatonCoalition) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranArhaun: {
                        if (faction == Faction::TerranHecatonCoalition) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranAspirant: {
                        if (faction == Faction::TerranGoldenRepublic) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranSouthpaw: {
                        if (faction == Faction::TerranUpstart) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranTarasque: {
                        if (faction == Faction::TerranNydusTrinity) {
                            if (in_air || is_adding_on) {
                                return TECH_HIDDEN;
                            }
                            if (player_has_completed_addon_unit_id(player_id, UnitId::TerranPalladium)) {
                                return TECH_ACTIVE;
                            }
                        }
                        return TECH_HIDDEN;
                    }
                }
            } break;
            case UnitId::TerranMantle: {
                switch (unit_id) {
                    case UnitId::TerranRamesses:
                    case UnitId::TerranDurendal:
                    case UnitId::TerranCataphract: {
                        if (in_air) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                }
            } break;
            case UnitId::TerranIronFoundry: {
                switch (unit_id) {
                    case UnitId::TerranBioticBastion:
                    case UnitId::TerranFutureStation: {
                        if (has_addon || is_training) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranPaladin:
                    case UnitId::TerranCuirassTankMode:
                    case UnitId::TerranPazuzu: {
                        if (in_air || is_adding_on) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranClaymore:
                    case UnitId::TerranPenumbra: {
                        if (in_air || is_adding_on) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranFutureStation)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranAnticthon:
                    case UnitId::TerranAion: {
                        if (in_air || is_adding_on) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranBioticBastion)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                }
            } break;
            case UnitId::TerranStarpad: {
                switch (unit_id) {
                    case UnitId::TerranWraith: {
                        if (faction != Faction::TerranFuturistFederation) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranValkyrie: {
                        if (in_air) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranTrojan: {
                        if (faction != Faction::TerranHecatonCoalition) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranAncile: {
                        if (faction != Faction::TerranUpstart) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranPhiloc: {
                        if (faction == Faction::TerranFuturistFederation) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranWendigo: {
                        if (faction == Faction::TerranUpstart) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranQilin: {
                        if (faction == Faction::TerranHecatonCoalition) {
                            if (in_air) {
                                return TECH_HIDDEN;
                            }
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                }
            } break;
            case UnitId::TerranStarport: {
                switch (unit_id) {
                    case UnitId::TerranRotary:
                    case UnitId::TerranCommandment: {
                        if (has_addon || is_training) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranGorgon:
                    case UnitId::TerranWyvern:
                    case UnitId::TerranPythean: {
                        if (in_air || is_adding_on) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranSundog:
                    case UnitId::TerranSalamander: {
                        if (in_air || is_adding_on) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranRotary)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranAzazel: {
                        if (in_air || is_adding_on) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranCommandment)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranSeraph: {
                        if (in_air || is_adding_on || faction == Faction::TerranGoldenRepublic) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranCommandment)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::TerranLuminary: {
                        if (in_air || is_adding_on || faction != Faction::TerranGoldenRepublic) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranCommandment)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                }
            } break;
            case UnitId::TerranNaniteAssembly: {
                switch (unit_id) {
                    case UnitId::TerranHadron: {
                        if (has_addon || is_training) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranCentaur:
                    case UnitId::TerranMinotaur:
                    case UnitId::TerranMagnetar: {
                        if (in_air || is_adding_on) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::TerranHippogriff:
                    case UnitId::TerranPhobos: {
                        if (in_air || is_adding_on) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_completed_addon_unit_id(player_id, UnitId::TerranHadron)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                }
            } break;
            
            // PROTOSS
            // Units
            case UnitId::ProtossNexus: {
                if (unit_id == UnitId::ProtossScribe) {
                    return TECH_ACTIVE;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ProtossEmbassy: {
                switch (unit_id) {
                    case UnitId::ProtossArtisan:
                    case UnitId::ProtossEnvoy:
                    case UnitId::ProtossWitness:
                    case UnitId::ProtossAutocort: {
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ProtossStridentStratum: {
                switch (unit_id) {
                    case UnitId::ProtossMonogram: {
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ProtossGateway: {
                switch (unit_id) {
                    case UnitId::ProtossZealot: {
                        if (faction == Faction::ProtossStarbound) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::ProtossZealotStarbound: {
                        if (faction != Faction::ProtossStarbound) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::ProtossDracadin: {
                        if (faction == Faction::ProtossOrderOfTheSacredSaber) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::ProtossDracadinOrder: {
                        if (faction != Faction::ProtossOrderOfTheSacredSaber) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::ProtossEcclesiast: {
                        if (faction == Faction::ProtossKhalendricKnights) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::ProtossEcclesiastKhalendric: {
                        if (faction != Faction::ProtossKhalendricKnights) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::ProtossLegionnaire: {
                        if (faction == Faction::ProtossOrderOfTheSacredSaber || faction == Faction::ProtossStarbound) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::ProtossAdherent: {
                        if (faction != Faction::ProtossOrderOfTheSacredSaber) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::ProtossCicatus: {
                        if (faction != Faction::ProtossStarbound) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::ProtossHierophant: {
                        if (faction == Faction::ProtossKhalendricKnights) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    case UnitId::ProtossAshguard: {
                        if (faction != Faction::ProtossKhalendricKnights) {
                            return TECH_HIDDEN;
                        }
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
            } break;
            case UnitId::ProtossGrandLibrary: {
                switch (unit_id) {
                    case UnitId::ProtossAmaranth:
                    case UnitId::ProtossHerald:
                    case UnitId::ProtossAtreus:
                    case UnitId::ProtossCantavis: {
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
            } break;
            case UnitId::ProtossRogueGallery: {
                switch (unit_id) {
                    case UnitId::ProtossVagrant:
                    case UnitId::ProtossCabalist:
                    case UnitId::ProtossCharlatan:
                    case UnitId::ProtossBarghest: {
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
            } break;
            case UnitId::ProtossProstrationStage: {
                switch (unit_id) {
                    case UnitId::ProtossPariah:
                    case UnitId::ProtossPotentate:
                    case UnitId::ProtossSycophant:
                    case UnitId::ProtossMindTyrant:
                    {
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
            } break;
            case UnitId::ProtossAncestralArchives: {
                switch (unit_id) {
                    case UnitId::ProtossArchon:
                    case UnitId::ProtossOptecton:
                    case UnitId::ProtossPatriarch:
                    {
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
            } break;
            case UnitId::ProtossHallOfHeretics: {
                switch (unit_id) {
                    case UnitId::ProtossApostate:
                    case UnitId::ProtossAugur: {
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
            } break;
            case UnitId::ProtossLattice: {
                switch (unit_id) {
                    case UnitId::ProtossVassal: {
                        if (faction != Faction::ProtossOrderOfTheSacredSaber) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::ProtossRenesair: {
                        if (faction == Faction::ProtossOrderOfTheSacredSaber) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::ProtossSimulacrum: {
                        return TECH_ACTIVE;
                    }
                    case UnitId::ProtossManifold: {
                        if (faction != Faction::ProtossStarbound) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::ProtossManifoldStarbound: {
                        if (faction == Faction::ProtossStarbound) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::ProtossGolem:
                    case UnitId::ProtossIdol: {
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
            } break;
            case UnitId::ProtossArdentAuthority: {
                switch (unit_id) {
                    case UnitId::ProtossServitor:
                    case UnitId::ProtossPositron:
                    case UnitId::ProtossAccantor:
                    case UnitId::ProtossClarion: {
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
            } break;
            case UnitId::ProtossUnitaryUnion: {
                switch (unit_id) {
                    case UnitId::ProtossOscilliant:
                    case UnitId::ProtossUnar:
                    case UnitId::ProtossArchitect:
                    case UnitId::ProtossVibrance: {
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
            } break;
            case UnitId::ProtossSyntheticSynod: {
                switch (unit_id) {
                    case UnitId::ProtossAnalogue:
                    case UnitId::ProtossDemiurge:
                    case UnitId::ProtossEmpress: {
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
            } break;
            case UnitId::ProtossStargate: {
                switch (unit_id) {
                    case UnitId::ProtossAurora: {
                        if (faction != Faction::ProtossOrderOfTheSacredSaber) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::ProtossExtatus: {
                        if (faction == Faction::ProtossOrderOfTheSacredSaber) {
                            return TECH_ACTIVE;
                        }
                        return TECH_HIDDEN;
                    }
                    case UnitId::ProtossPanoptus:
                    case UnitId::ProtossLanifect:
                    case UnitId::ProtossGladius:
                    case UnitId::ProtossExemplar: {
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
            } break;
            case UnitId::ProtossArgosy: {
                switch (unit_id) {
                    case UnitId::ProtossMagister:
                    case UnitId::ProtossEpigraph:
                    case UnitId::ProtossDidact:
                    case UnitId::ProtossEmpyrean:
                    case UnitId::ProtossSolarion: {
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
            } break;
            case UnitId::ProtossMonumentOfSin: {
                switch (unit_id) {
                    case UnitId::ProtossAnthelion:
                    case UnitId::ProtossStarSovereign: {
                        return TECH_ACTIVE;
                    }
                    return TECH_HIDDEN;
                }
            } break;

            // Hangar units
            case UnitId::ProtossSolarion: {
                if (unit_id == UnitId::ProtossSteward) {
                    u32 steward_count = unit->unit_with_hangar.in_hangar_count + unit->unit_with_hangar.out_hangar_count;
                    for (u32 i = 0; i < 5; ++i) {
                        steward_count += unit->build_queue[i] != UnitId::None;
                    }
                    if (steward_count >= 12) {
                        return TECH_HIDDEN;
                    }
                    return TECH_ACTIVE;
                }
                return TECH_HIDDEN;
            }

            // Structures
            case UnitId::ProtossScribe:
            case UnitId::ProtossPrudence:
            case UnitId::ProtossAutocort: {
                switch (unit_id) {
                    case UnitId::ProtossNexus:
                    case UnitId::ProtossPylon:
                    case UnitId::ProtossCedent:
                    case UnitId::ProtossBulwark:
                    case UnitId::ProtossWarden:
                    case UnitId::ProtossEmbassy:
                    case UnitId::ProtossGateway:
                    case UnitId::ProtossLattice: {
                        return TECH_ACTIVE;
                    }
                    case UnitId::ProtossCrucible:
                    case UnitId::ProtossAquifer:
                    case UnitId::ProtossMatrix:
                    case UnitId::ProtossGrandLibrary:
                    case UnitId::ProtossRogueGallery:
                    case UnitId::ProtossArdentAuthority:
                    case UnitId::ProtossStargate: {
                        if (TierUnlocked[RaceId::Protoss][Tier2]) {
                            return TECH_ACTIVE;
                        }
                    }
                    case UnitId::ProtossEngram:
                    case UnitId::ProtossProstrationStage:
                    case UnitId::ProtossUnitaryUnion:
                    case UnitId::ProtossArgosy: {
                        if (TierUnlocked[RaceId::Protoss][Tier3]) {
                            return TECH_ACTIVE;
                        }
                    }
                    case UnitId::ProtossXenolith:
                    case UnitId::ProtossAncestralArchives:
                    case UnitId::ProtossSyntheticSynod:
                    case UnitId::ProtossMonumentOfSin: {
                        if (TierUnlocked[RaceId::Protoss][Tier4]) {
                            return TECH_ACTIVE;
                        }
                    }
                    return TECH_HIDDEN;
                }
            } break;
            
            // ZERG
            // Units
            case UnitId::ZergLarva: {
                switch (unit_id) {
                    case UnitId::ZergDroleth:
                    case UnitId::ZergRilirokor: {
                        return TECH_ACTIVE;
                    }
                    case UnitId::ZergAgrileth: {
                        if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_gestated_unit_id(player_id, UnitId::ZergVornathPond) || player_has_base_unit_id(player_id, UnitId::ZergQuazethPool)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergOvileth: {
                        if (faction != Faction::ZergZorthosTendril) {
                            return TECH_ACTIVE;
                        }
                    } 
                    case UnitId::ZergYblyleth: {
                        if (faction == Faction::ZergZorthosTendril) {
                            return TECH_ACTIVE;
                        }
                    } 
                    case UnitId::ZergOthstoleth: {
                        if (TierUnlocked[RaceId::Zerg][Tier3]) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergZethrokor: {
                        if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_gestated_unit_id(player_id, UnitId::ZergVornathPond) || player_has_base_unit_id(player_id, UnitId::ZergQuazethPool)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergQuazilisk: {
                        if (faction != Faction::ZergImicrenTendril) {
                            if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_gestated_unit_id(player_id, UnitId::ZergVornathPond) || player_has_base_unit_id(player_id, UnitId::ZergQuazethPool)) {
                                return TECH_ACTIVE;
                            }
                            return TECH_LOCKED;
                        }
                    }
                    case UnitId::ZergQuazrokor: {
                        if (faction == Faction::ZergImicrenTendril) {
                            if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_gestated_unit_id(player_id, UnitId::ZergVornathPond) || player_has_base_unit_id(player_id, UnitId::ZergQuazethPool)) {
                                return TECH_ACTIVE;
                            }
                            return TECH_LOCKED;
                        }
                    }
                    case UnitId::ZergNathrokor: {
                        if (faction == Faction::ZergZorthosTendril) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_base_unit_id(player_id, UnitId::ZergVornathPond)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergNathrelisk: {
                        if (faction != Faction::ZergZorthosTendril) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_base_unit_id(player_id, UnitId::ZergVornathPond)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergHydralisk:{
                        if (faction == Faction::ZergOrkotiTendril) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_unit_id(player_id, UnitId::ZergTethvithSwamp) || player_has_gestated_unit_id(player_id, UnitId::ZergSkibactScab) || player_has_base_unit_id(player_id, UnitId::ZergHydrithDen)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergHydraliskOrkoti:{
                        if (faction != Faction::ZergOrkotiTendril) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_unit_id(player_id, UnitId::ZergTethvithSwamp) || player_has_gestated_unit_id(player_id, UnitId::ZergSkibactScab) || player_has_base_unit_id(player_id, UnitId::ZergHydrithDen)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergIzirokor: {
                        if (faction == Faction::ZergZorthosTendril) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_unit_id(player_id, UnitId::ZergTethvithSwamp) || player_has_gestated_unit_id(player_id, UnitId::ZergSkibactScab) || player_has_base_unit_id(player_id, UnitId::ZergHydrithDen)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergIziralisk: {
                        if (faction != Faction::ZergZorthosTendril) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_unit_id(player_id, UnitId::ZergTethvithSwamp) || player_has_gestated_unit_id(player_id, UnitId::ZergSkibactScab) || player_has_base_unit_id(player_id, UnitId::ZergHydrithDen)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergSkithrokor: {
                        if (player_has_unit_id(player_id, UnitId::ZergTethvithSwamp) || player_has_base_unit_id(player_id, UnitId::ZergSkibactScab)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergZoryusthaleth: {
                        if (faction == Faction::ZergImicrenTendril) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_unit_id(player_id, UnitId::ZergZozarcGorge) || player_has_gestated_unit_id(player_id, UnitId::ZergUlgorgCavern) || player_has_base_unit_id(player_id, UnitId::ZergZorkizShroud)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergInzorthrokor: {
                        if (faction != Faction::ZergImicrenTendril) {
                            return TECH_HIDDEN;
                        }
                        if (player_has_unit_id(player_id, UnitId::ZergZozarcGorge) || player_has_gestated_unit_id(player_id, UnitId::ZergUlgorgCavern) || player_has_base_unit_id(player_id, UnitId::ZergZorkizShroud)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergLakizilisk:
                    case UnitId::ZergGorgrokor: {
                        if (player_has_unit_id(player_id, UnitId::ZergZozarcGorge) || player_has_gestated_unit_id(player_id, UnitId::ZergUlgorgCavern) || player_has_base_unit_id(player_id, UnitId::ZergZorkizShroud)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergKagralisk:
                    case UnitId::ZergEvigrilisk:
                    case UnitId::ZergAlmaksalisk: {
                        if (player_has_unit_id(player_id, UnitId::ZergChokralChazsal) || player_has_base_unit_id(player_id, UnitId::ZergAlkagIteth)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergMutalisk:
                    case UnitId::ZergProtathalor:
                    case UnitId::ZergCikralisk: {
                        if (player_has_unit_id(player_id, UnitId::ZergTosvrolHaunt) || player_has_base_unit_id(player_id, UnitId::ZergMutathSpire)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergMortothrokor:
                    case UnitId::ZergAxitrilisk:
                    case UnitId::ZergSasyrvoleth: {
                        if (player_has_unit_id(player_id, UnitId::ZergBegrozithBarrow) || player_has_base_unit_id(player_id, UnitId::ZergAxtothAxis)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergKonvilisk:
                    case UnitId::ZergVilgorokor:
                    case UnitId::ZergMatraleth: {
                        if (player_has_unit_id(player_id, UnitId::ZergAlmakisAntre) || player_has_base_unit_id(player_id, UnitId::ZergMatravilNest)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergKeskathalor:
                    case UnitId::ZergGeszithalor: {
                        if (player_has_unit_id(player_id, UnitId::ZergKethrajKeep) || player_has_base_unit_id(player_id, UnitId::ZergGeszkathGrotto)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                }
                return TECH_HIDDEN;
            }
            
            // Morphs
            case UnitId::ZergOvileth:
            case UnitId::ZergYblyleth: {
                if (unit_id == UnitId::ZergIroleth) {
                    if (unit->is_burrowed()) {
                        return TECH_HIDDEN;
                    }
                    if (TierUnlocked[RaceId::Zerg][Tier2] && bw::transport_is_empty(unit)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergZethrokor: {
                if (unit_id == UnitId::ZergVorvrokor) {
                    if (unit->is_burrowed()) {
                        return TECH_HIDDEN;
                    }
                    if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_base_unit_id(player_id, UnitId::ZergVornathPond)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_ACTIVE;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergQuazilisk:
            case UnitId::ZergQuazrokor: {
                if (unit_id == UnitId::ZergLiiralisk) {
                    if (unit->is_burrowed()) {
                        return TECH_HIDDEN;
                    }
                    if (player_has_completed_unit_id(player_id, UnitId::ZergKalkiirLake)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergNathrokor:
            case UnitId::ZergNathrelisk: {
                if (unit_id == UnitId::ZergKalkalisk) {
                    if (player_has_completed_unit_id(player_id, UnitId::ZergKalkiirLake)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergCikralisk: {
                if (unit_id == UnitId::ZergGhitorokor) {
                    if (player_has_completed_unit_id(player_id, UnitId::ZergTosvrolHaunt)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergProtathalor: {
                if (unit_id == UnitId::ZergSovroleth) {
                    if (player_has_completed_unit_id(player_id, UnitId::ZergTosvrolHaunt)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergMutalisk: {
                if (unit_id == UnitId::ZergTosgrilisk) {
                    if (player_has_completed_unit_id(player_id, UnitId::ZergTosvrolHaunt)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergAxitrilisk: {
                if (unit_id == UnitId::ZergBegrothilisk) {
                    if (player_has_completed_unit_id(player_id, UnitId::ZergBegrozithBarrow)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergMortothrokor: {
                if (unit_id == UnitId::ZergOztukrokor) {
                    if (player_has_completed_unit_id(player_id, UnitId::ZergBegrozithBarrow)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergHydralisk:
            case UnitId::ZergHydraliskOrkoti: {
                if (unit_id == UnitId::ZergBactalisk) {
                    if (player_has_unit_id(player_id, UnitId::ZergTethvithSwamp) || player_has_base_unit_id(player_id, UnitId::ZergSkibactScab)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergIzirokor:
            case UnitId::ZergIziralisk: {
                if (unit_id == UnitId::ZergTethzorokor) {
                    if (unit->is_burrowed()) {
                        return TECH_HIDDEN;
                    }
                    if (player_has_completed_unit_id(player_id, UnitId::ZergTethvithSwamp)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergSkithrokor: {
                if (unit_id == UnitId::ZergVithrilisk) {
                    if (player_has_completed_unit_id(player_id, UnitId::ZergTethvithSwamp)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergKagralisk: {
                if (unit_id == UnitId::ZergCherbrathalor) {
                    if (player_has_completed_unit_id(player_id, UnitId::ZergChokralChazsal)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergEvigrilisk: {
                if (unit_id == UnitId::ZergOkrimnilisk) {
                    if (player_has_completed_unit_id(player_id, UnitId::ZergChokralChazsal)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergAlmaksalisk: {
                if (unit_id == UnitId::ZergAlgoztalisk) {
                    if (player_has_completed_unit_id(player_id, UnitId::ZergChokralChazsal)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergZoryusthaleth:
            case UnitId::ZergInzorthrokor: {
                if (unit_id == UnitId::ZergUltrokor) {
                    if (unit->is_burrowed()) {
                        return TECH_HIDDEN;
                    }
                    if (player_has_base_unit_id(player_id, UnitId::ZergUlgorgCavern) || player_has_unit_id(player_id, UnitId::ZergZozarcGorge)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergLakizilisk: {
                if (unit_id == UnitId::ZergZobriolisk) {
                    if (unit->is_burrowed()) {
                        return TECH_HIDDEN;
                    }
                    if (player_has_completed_unit_id(player_id, UnitId::ZergZozarcGorge)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergGorgrokor: {
                if (unit_id == UnitId::ZergZarcavrokor) {
                    if (player_has_completed_unit_id(player_id, UnitId::ZergZozarcGorge)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergKonvilisk: {
                if (unit_id == UnitId::ZergTetcaerokor) {
                    if (unit->is_burrowed()) {
                        return TECH_HIDDEN;
                    }
                    if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergVilgorokor: {
                if (unit_id == UnitId::ZergIsthrathaleth) {
                    if (unit->is_burrowed()) {
                        return TECH_HIDDEN;
                    }
                    if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergMatraleth: {
                if (unit_id == UnitId::ZergAkistrokor) {
                    if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergKeskathalor: {
                if (unit_id == UnitId::ZergKethrostalor) {
                    if (unit->is_burrowed()) {
                        return TECH_HIDDEN;
                    }
                    if (player_has_completed_unit_id(player_id, UnitId::ZergKethrajKeep)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergGeszithalor: {
                if (unit_id == UnitId::ZergAlkajelisk) {
                    if (player_has_base_unit_id(player_id, UnitId::ZergGeszkathGrotto) || player_has_completed_unit_id(player_id, UnitId::ZergKethrajKeep)) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergOcsalLarva: {
                switch (unit_id) {
                    case UnitId::ZergGosvileth: {
                        if (TierUnlocked[RaceId::Zerg][Tier2]) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergIroleth: {
                        if (TierUnlocked[RaceId::Zerg][Tier2]) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergVorvrokor: {
                        if (player_has_unit_id(player_id, UnitId::ZergKalkiirLake) || player_has_base_unit_id(player_id, UnitId::ZergVornathPond)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergLiiralisk:
                    case UnitId::ZergKalkalisk: {
                        if (player_has_completed_unit_id(player_id, UnitId::ZergKalkiirLake)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergGhitorokor:
                    case UnitId::ZergSovroleth:
                    case UnitId::ZergTosgrilisk: {
                        if (player_has_completed_unit_id(player_id, UnitId::ZergTosvrolHaunt)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergBegrothilisk:
                    case UnitId::ZergOztukrokor: {
                        if (player_has_completed_unit_id(player_id, UnitId::ZergBegrozithBarrow)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergBactalisk: {
                        if (player_has_base_unit_id(player_id, UnitId::ZergSkibactScab) || player_has_unit_id(player_id, UnitId::ZergTethvithSwamp)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergTethzorokor:
                    case UnitId::ZergVithrilisk: {
                        if (player_has_completed_unit_id(player_id, UnitId::ZergTethvithSwamp)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergCherbrathalor:
                    case UnitId::ZergOkrimnilisk:
                    case UnitId::ZergAlgoztalisk: {
                        if (player_has_completed_unit_id(player_id, UnitId::ZergChokralChazsal)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergAlkajelisk: {
                        return TECH_ACTIVE;
                    }
                    case UnitId::ZergUltrokor: {
                        if (player_has_base_unit_id(player_id, UnitId::ZergUlgorgCavern) || player_has_unit_id(player_id, UnitId::ZergZozarcGorge)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergZobriolisk:
                    case UnitId::ZergZarcavrokor: {
                        if (player_has_completed_unit_id(player_id, UnitId::ZergZozarcGorge)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergTetcaerokor:
                    case UnitId::ZergIsthrathaleth:
                    case UnitId::ZergAkistrokor: {
                        if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                }
                return TECH_HIDDEN;
            }

            // Structures
            case UnitId::ZergDroleth: {
                switch (unit_id) {
                    case UnitId::ZergHachirosk:
                    case UnitId::ZergCagrantCesiant:
                    case UnitId::ZergEskigant:
                    case UnitId::ZergQuazethPool:
                    case UnitId::ZergHydrithDen:
                    case UnitId::ZergZorkizShroud:
                    case UnitId::ZergIrolIris: {
                        return TECH_ACTIVE;
                    }
                    case UnitId::ZergExcisant:
                    case UnitId::ZergMutathSpire:
                    case UnitId::ZergAlkagIteth:
                    case UnitId::ZergMatravilNest: {
                        if (TierUnlocked[RaceId::Zerg][Tier2]) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergAxtothAxis:
                    case UnitId::ZergGeszkathGrotto:
                    case UnitId::ZergElthisthMound: {
                        if (TierUnlocked[RaceId::Zerg][Tier3]) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergGosvileth: {
                        if (unit->is_burrowed()) {
                            return TECH_HIDDEN;
                        }
                        if (TierUnlocked[RaceId::Zerg][Tier2]) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergAgrileth: {
                switch (unit_id) {
                    case UnitId::ZergHachirosk:
                    case UnitId::ZergEskigant:
                    case UnitId::ZergCagrantCesiant:
                    case UnitId::ZergQuazethPool:
                    case UnitId::ZergHydrithDen:
                    case UnitId::ZergZorkizShroud:
                    case UnitId::ZergIrolIris: {
                        return TECH_ACTIVE;
                    }
                    case UnitId::ZergExcisant:
                    case UnitId::ZergMutathSpire:
                    case UnitId::ZergAlkagIteth:
                    case UnitId::ZergMatravilNest: {
                        if (TierUnlocked[RaceId::Zerg][Tier2]) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergAxtothAxis:
                    case UnitId::ZergGeszkathGrotto: {
                        if (TierUnlocked[RaceId::Zerg][Tier3]) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergGosvileth: {
                switch (unit_id) {
                    case UnitId::ZergLarvoskCesiant:
                    case UnitId::ZergSurkithCesiant:
                    case UnitId::ZergSpraithCesiant:
                    case UnitId::ZergKazitrithCesiant:
                    case UnitId::ZergOthstolOviform:
                    case UnitId::ZergVornathPond:
                    case UnitId::ZergSkibactScab:
                    case UnitId::ZergUlgorgCavern: {
                        return TECH_ACTIVE;
                    }
                    case UnitId::ZergCaphrolosk:
                    case UnitId::ZergNydusCesiant:
                    case UnitId::ZergTosvrolHaunt:
                    case UnitId::ZergChokralChazsal:
                    case UnitId::ZergAlmakisAntre: {
                        if (TierUnlocked[RaceId::Zerg][Tier2]) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::ZergGathtelosk:
                    case UnitId::ZergEzigrisant:
                    case UnitId::ZergBegrozithBarrow:
                    case UnitId::ZergKethrajKeep: {
                        if (TierUnlocked[RaceId::Zerg][Tier3]) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                }
                return TECH_HIDDEN;
            }
            
            // Structure morphs
            case UnitId::ZergCagrantCesiant: {
                if (unit_id == UnitId::ZergSurkithCesiant
                ||  unit_id == UnitId::ZergSpraithCesiant
                ||  unit_id == UnitId::ZergLarvoskCesiant
                ||  unit_id == UnitId::ZergKazitrithCesiant) {
                    return TECH_ACTIVE;
                }
                if (unit_id == UnitId::ZergNydusCesiant) {
                    if (TierUnlocked[RaceId::Zerg][Tier2]) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergSurkithCesiant: {
                if (unit_id == UnitId::ZergSkortrithCesiant) {
                    if (TierUnlocked[RaceId::Zerg][Tier2]) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergSpraithCesiant: {
                if (unit_id == UnitId::ZergSpirtithCesiant) {
                    if (TierUnlocked[RaceId::Zerg][Tier2]) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergLarvoskCesiant: {
                if (unit_id == UnitId::ZergOcsalCesiant) {
                    if (TierUnlocked[RaceId::Zerg][Tier2]) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergHachirosk: {
                if (unit_id == UnitId::ZergCaphrolosk) {
                    if (TierUnlocked[RaceId::Zerg][Tier2]) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                if (unit_id == UnitId::ZergGathtelosk) {
                    if (TierUnlocked[RaceId::Zerg][Tier3]) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergExcisant: {
                if (unit_id == UnitId::ZergEzigrisant) {
                    if (TierUnlocked[RaceId::Zerg][Tier3]) {
                        return TECH_ACTIVE;
                    }
                    return TECH_LOCKED;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergQuazethPool: {
                if (unit_id == UnitId::ZergVornathPond) {
                    return TECH_ACTIVE;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergVornathPond: {
                if (unit_id == UnitId::ZergKalkiirLake) {
                    return TECH_ACTIVE;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergHydrithDen: {
                if (unit_id == UnitId::ZergSkibactScab) {
                    return TECH_ACTIVE;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergSkibactScab: {
                if (unit_id == UnitId::ZergTethvithSwamp) {
                    return TECH_ACTIVE;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergMutathSpire: {
                if (unit_id == UnitId::ZergTosvrolHaunt) {
                    return TECH_ACTIVE;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergIrolIris: {
                if (unit_id == UnitId::ZergOthstolOviform) {
                    return TECH_ACTIVE;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergZorkizShroud: {
                if (unit_id == UnitId::ZergUlgorgCavern) {
                    return TECH_ACTIVE;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergUlgorgCavern: {
                if (unit_id == UnitId::ZergZozarcGorge) {
                    return TECH_ACTIVE;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergMatravilNest: {
                if (unit_id == UnitId::ZergAlmakisAntre) {
                    return TECH_ACTIVE;
                }
                return TECH_HIDDEN;
            }
            case UnitId::ZergAlkagIteth: {
                if (unit_id == UnitId::ZergChokralChazsal) {
                    return TECH_ACTIVE;
                }
                return TECH_HIDDEN;
            }

            // ASKOSI
            // Units
            case UnitId::AskosiLandlock: {
                switch (unit_id) {
                    case UnitId::AskosiPilgrim: {
                        return TECH_ACTIVE;
                    }
                    case UnitId::AskosiProselyte: {
                        if (player_has_completed_unit_id(player_id, UnitId::AskosiRadix) || player_has_completed_unit_id(player_id, UnitId::AskosiFunction)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::AskosiDyad: {
                        if (player_has_completed_unit_id(player_id, UnitId::AskosiCipher) || player_has_completed_unit_id(player_id, UnitId::AskosiGauntlet)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                }
                return TECH_HIDDEN;
            }
            case UnitId::AskosiRadix: {
                switch (unit_id) {
                    case UnitId::AskosiMissionary:
                    case UnitId::AskosiCardinal: {
                        return TECH_ACTIVE;
                    }
                    case UnitId::AskosiMimic: {
                        if (player_has_completed_unit_id(player_id, UnitId::AskosiCipher)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::AskosiVicar: {
                        if (player_has_completed_unit_id(player_id, UnitId::AskosiGauntlet)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                }
                return TECH_HIDDEN;
            }
            case UnitId::AskosiFunction: {
                switch (unit_id) {
                    case UnitId::AskosiRove:
                    case UnitId::AskosiObject: {
                        return TECH_ACTIVE;
                    }
                    case UnitId::AskosiScepter: {
                        if (player_has_completed_unit_id(player_id, UnitId::AskosiCipher)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::AskosiIntendent: {
                        if (player_has_completed_unit_id(player_id, UnitId::AskosiGauntlet)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                }
                return TECH_HIDDEN;
            }
            case UnitId::AskosiCosmoscope: {
                switch (unit_id) {
                    case UnitId::AskosiHelion:
                    case UnitId::AskosiEmissary: {
                        return TECH_ACTIVE;
                    }
                    case UnitId::AskosiInkblot: {
                        if (player_has_completed_unit_id(player_id, UnitId::AskosiCipher)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::AskosiTirade: {
                        if (player_has_completed_unit_id(player_id, UnitId::AskosiGauntlet)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                }
                return TECH_HIDDEN;
            }

            // Structures
            case UnitId::AskosiPilgrim: {
                switch (unit_id) {
                    case UnitId::AskosiLandlock:
                    case UnitId::AskosiArch:
                    case UnitId::AskosiGrandstand:
                    case UnitId::AskosiChantry:
                    case UnitId::AskosiRadix:
                    case UnitId::AskosiFunction: {
                        return TECH_ACTIVE;
                    }
                    case UnitId::AskosiCipher:
                    case UnitId::AskosiGauntlet: {
                        if (player_has_completed_unit_id(player_id, UnitId::AskosiRadix) || player_has_completed_unit_id(player_id, UnitId::AskosiFunction)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                    case UnitId::AskosiCosmoscope: {
                        if (player_has_completed_unit_id(player_id, UnitId::AskosiCipher) || player_has_completed_unit_id(player_id, UnitId::AskosiGauntlet)) {
                            return TECH_ACTIVE;
                        }
                        return TECH_LOCKED;
                    }
                }
            } break;

            // Gamemodes \\ 

            case UnitId::Omnivale: {
                if (unit_id == UnitId::TerranMason
                ||	unit_id == UnitId::ProtossScribe) {
                    return TECH_ACTIVE;
                }
                return TECH_HIDDEN;
            }
        }
        return TECH_HIDDEN;
    }

    bool can_set_rally_opcode(Unit* unit) {
        return hooks::unit_can_rally(unit);
    }

    u16 addon_datreq_fix(u32 dat_req_offset, u16 unit_id) { // Hooked to fix Treasuries being unable to claim addons. -DarkenedFantasies
        while (requirements::units[dat_req_offset] == RequirementOpcodes::CurrentUnitIs) {
            if (requirements::units[++dat_req_offset] == unit_id) {
                return unit_id;
            }

            if (requirements::units[++dat_req_offset] != RequirementOpcodes::Or) {
                break;
            }

            ++dat_req_offset;
        }

        return UnitId::None;
    }
}