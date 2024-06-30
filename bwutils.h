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
			case SpriteId::Corpse_Ovileth: {
				return true;
			} break;
		}
		return false;
	}
}

namespace WeaponId {
	namespace dat = weapons_dat;

	inline f32 get_tooltip_cooldown_offset(WeaponId::Enum id) {
		f32 offset = 0;
		switch (id) {
			case WeaponId::OptectonSunsplitBeam:
			case WeaponId::ArchitectDispersiveSalvo: {
				offset += 1;
			}
		}
		return offset;
	}

	inline f32 get_tooltip_cooldown(WeaponId::Enum id) {
		return dat::cooldown[id] / 24.0f + get_tooltip_cooldown_offset(id);
	}
}