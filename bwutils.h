#pragma once
#include "BW\bwdata.h"

namespace TechId {
    namespace dat = techdata_dat;

    inline u32 energy_cost(Enum id) {
        return dat::energy_cost[id] * 256;
    }
}

namespace SpriteId {
    namespace dat = sprites_dat;

    inline bool is_corpse(Enum id) {
        switch (id) {
            case SpriteId::Corpse_Mason:
            case SpriteId::Corpse_Maverick:
            case SpriteId::Corpse_MaverickFuturist:
            case SpriteId::Corpse_Harakan:
            case SpriteId::Corpse_Cyprian:
            case SpriteId::Corpse_Cleric:
            case SpriteId::Corpse_Shaman:
            case SpriteId::Corpse_Eidolon:
            case SpriteId::Corpse_Apostle:
            case SpriteId::Corpse_Vulture:
            case SpriteId::Corpse_Cyclops:
            case SpriteId::Corpse_Goliath:
            case SpriteId::Corpse_Blackjack:
            case SpriteId::Corpse_Wraith:
            case SpriteId::Corpse_Scribe:
            case SpriteId::Corpse_Zealot:
            case SpriteId::Corpse_Dracadin:
            case SpriteId::Corpse_Ecclesiast:
            case SpriteId::Corpse_Legionnaire:
            case SpriteId::Corpse_Lanifect:
            case SpriteId::Corpse_Larva:
            case SpriteId::Corpse_Egg:
            case SpriteId::Corpse_Droleth:
            case SpriteId::Corpse_Zethrokor:
            case SpriteId::Corpse_Quazilisk:
            case SpriteId::Corpse_Mortothrokor:
            case SpriteId::Corpse_Hydralisk:
            case SpriteId::Corpse_Izirokor:
            case SpriteId::Corpse_Bactalisk:
            case SpriteId::Corpse_Kagralisk:
            case SpriteId::Corpse_Evigrilisk:
            case SpriteId::Corpse_Zoryusthaleth:
            case SpriteId::Corpse_Lakizilisk:
            case SpriteId::Corpse_Konvilisk:
            case SpriteId::Corpse_Rilirokor:
            case SpriteId::Corpse_Ultrokor:
            case SpriteId::Corpse_Isthrathaleth:
            case SpriteId::Corpse_TerranBuildingLarge:
            case SpriteId::Corpse_TerranBuildingSmall:
            case SpriteId::Corpse_ProtossBuildingLarge:
            case SpriteId::Corpse_ProtossBuildingSmall:
            case SpriteId::Corpse_ZergBuildingLarge:
            case SpriteId::Corpse_ZergBuildingSmall:
            case SpriteId::Corpse_Aspirant:
            case SpriteId::Corpse_TerranUnitMedium:
            case SpriteId::Corpse_TerranUnitSmall:
            case SpriteId::Corpse_ProtossUnitMedium:
            case SpriteId::Corpse_ProtossUnitSmall:
            case SpriteId::Corpse_ZergUnitMedium:
            case SpriteId::Corpse_ZergUnitSmall:
            case SpriteId::Corpse_ZergUnitTiny:
            case SpriteId::Corpse_Dilettante:
            case SpriteId::Corpse_Ancile:
            case SpriteId::Corpse_Trojan:
            case SpriteId::Corpse_Envoy:
            case SpriteId::Corpse_Ovileth:
            case SpriteId::Corpse_PhalanxTank:
            case SpriteId::Corpse_PhalanxSiege:
            case SpriteId::Corpse_Hierophant:
            case SpriteId::Corpse_Simulacrum: 
            case SpriteId::Corpse_Vassal: 
            case SpriteId::Corpse_Nathrokor:
            case SpriteId::Corpse_Vorvrokor:
            case SpriteId::Corpse_Manifold:
            case SpriteId::Corpse_Golem:
            case SpriteId::Corpse_Idol:
            case SpriteId::Corpse_Artisan:
            case SpriteId::Corpse_Witness:
            case SpriteId::Corpse_Savant:
            case SpriteId::Corpse_Hypnagogue:
            case SpriteId::Corpse_Iziralisk:
            case SpriteId::Corpse_Gorgrokor:
            case SpriteId::Corpse_MadrigalFuria:
            case SpriteId::Corpse_Renesair:
            case SpriteId::Corpse_DracadinOrder:
            case SpriteId::Corpse_HydraliskOrkoti:
            case SpriteId::Corpse_Inzorthrokor:
            case SpriteId::Corpse_Agrileth:
            case SpriteId::Corpse_AskosiBuildingLarge:
            case SpriteId::Corpse_AskosiBuildingSmall:
            case SpriteId::Corpse_AskosiBuildingTiny:
            case SpriteId::Corpse_Cardinal:
            case SpriteId::Corpse_Vicar:
            case SpriteId::Corpse_Missionary:
            case SpriteId::Corpse_Proselyte:
            case SpriteId::Corpse_Malkothrokor:
            case SpriteId::Corpse_Zethalor:
            case SpriteId::Corpse_Thaumalogue:
            case SpriteId::Corpse_MaverickUpstart:
            case SpriteId::Corpse_Silversoul:
            case SpriteId::Corpse_Civilian:
            case SpriteId::Corpse_Pilgrim:
            case SpriteId::Corpse_Rozirokor: {
                return true;
            } break;
        }
        return false;
    }
}

namespace WeaponId {
    namespace dat = weapons_dat;

    inline f32 get_tooltip_cooldown_offset(Enum id) {
        f32 offset = 0;
        switch (id) {
            case WeaponId::OptectonSunsplitBeam: {
                offset += 1.6;
            } break;
            case WeaponId::OscilliantRotatingRay:
            case WeaponId::UnarAutonomousRay:
            case WeaponId::ArchitectDispersiveSalvo: {
                offset += 1;
            } break;
            case WeaponId::PaladinCrusaderMissiles: {
                offset += 0.9;
            } break;
            case WeaponId::WyvernJacksonCannon:
            case WeaponId::EpigraphLitanyCannons:
            case WeaponId::EmpyreanPlasmaBarrage: {
                offset += 0.5;
            } break;
        }
        return offset;
    }

    inline f32 get_tooltip_cooldown(Enum id) {
        return dat::cooldown[id] / 24.0f + get_tooltip_cooldown_offset(id);
    }
}