#include "psi_field.h"
#include "../SCBW/scbwdata.h"
#include "SCBW/api.h"

namespace hooks
{
	// Check if the given unit id can generate psi fields
	bool can_make_psi_field(u16 unit_id) {
		if (unit_id == UnitId::ProtossPylon) {
			return true;
		}
		return false;
	}

	// Actual state check whether a unit can generate a psi field
	bool is_ready_to_make_psi_field(Unit* unit) {
		return can_make_psi_field(unit->id);
	}

    // Conditions on which the power field should be showed
    bool unit_can_show_power_for(Unit* unit, u32 player) {
        if (!scbw::is_allied_to(unit->player, player)) {
            return false;
        }
        return true;
    }

    // Hooked to allow allies to benefit from power -Veeq7
    bool unit_has_power(u32 unit_id, s32 x, s32 y, u32 player) {
        if (!(units_dat::BaseProperty[unit_id] & UnitProperty::RequiredPsi)) {
            return true;
        }

        u32 w = *psi_field_grp_width;
        u32 h = *psi_field_grp_height;
        for (Unit* unit = *first_psi_field_provider; unit; unit = unit->psi_link.next) {
            if (!unit_can_show_power_for(unit, player)) {
                continue;
            }
            if (fabs(unit->sprite->position.x - x) >= w || fabs(unit->sprite->position.y - y) >= h) {
                continue;
            }
            if ((*psi_field_mask)[2 * w / 32 * ((y + h - unit->sprite->position.y) / 32) + (x + w - unit->sprite->position.x) / 32]) {
                return true;
            }
        }
        return false;
    }

    // Hooked to show allied power fields -Veeq7
    b32 show_power_sprite(Unit* unit) {
        Sprite* sprite = helper::create_sprite(SpriteId::PsiFieldRightUpper, unit->sprite->position.x, unit->sprite->position.y, unit->player); // Originally player was 0
        unit->building.pylon_aura = sprite;
        if (!sprite) {
            return false;
        }
        sprite->elevationLevel = 1;
        if (*IS_IN_REPLAY || !unit_can_show_power_for(unit, *LOCAL_NATION_ID)) {
            unit->building.pylon_aura->flags |= SpriteFlags::Hidden;
            helper::refresh_all_visible_images_at_screen_position(unit->building.pylon_aura, 0);
            return true;
        }
        if (*psi_unknown) {
            helper::refresh_all_visible_images_at_screen_position(unit->building.pylon_aura, *psi_vision_filter);
            return true;
        }
        if (!(unit->sprite->flags & SpriteFlags::Selected) ) {
            unit->building.pylon_aura->flags |= SpriteFlags::Hidden;
            helper::refresh_all_visible_images_at_screen_position(unit->building.pylon_aura, 0);
            return true;
        }
        helper::create_pylon_aura();
        return true;
    }

    // Hooked to show allied power fields -Veeq7
    void __cdecl create_pylon_aura() {
        for (Unit* unit = *first_psi_field_provider; unit; unit = unit->psi_link.next) {
            if (!*IS_IN_REPLAY && unit_can_show_power_for(unit, *LOCAL_NATION_ID) && (unit->building.pylon_aura || show_power_sprite(unit))) {
                unit->building.pylon_aura->flags &= ~SpriteFlags::Hidden;
                helper::refresh_all_visible_images_at_screen_position(unit->building.pylon_aura, *psi_vision_filter);
            }
        }
    }
}