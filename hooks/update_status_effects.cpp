#include "update_status_effects.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include "hooks/weapons/wpnspellhit.h"
#include "hooks/weapons/weapon_damage.h"
#include "globals.h"

void irradiateProc(CUnit* unit, CUnit* target) {
	//Damage organic units only
	if (!(units_dat::BaseProperty[target->id] & UnitProperty::Organic)) {
		return;
	}

	//Irradiate splash damage does not affect burrowed units
	if (target != unit && target->status & UnitStatus::Burrowed) {
		return;
	}

	//Check if the unit is within distance, or is inside the same transport
	if (unit->status & UnitStatus::InTransport
		|| unit->getDistanceToTarget(target) <= 32) {
		//			const int32 damage = weapons_dat::DamageAmount[WeaponId::Irradiate] * 256 / weapons_dat::Cooldown[WeaponId::Irradiate];
		int32 damage = (125 * 256) / 24;
		if (unit != nullptr) {
			if (unit->irradiateTimer == 0) {
				damage += (125 * 256) % 24;
				//scbw::print("Add additional damage");
			}
		}
		target->damageWith(WeaponId::Irradiate, damage, unit->irradiatedBy, unit->irradiatePlayerId);
	}
}

namespace hooks {

	void doIrradiateDamage(CUnit* irradiatedUnit) {
		//No splash if burrowed
		if (irradiatedUnit->status & UnitStatus::Burrowed) {
			irradiateProc(irradiatedUnit, irradiatedUnit);
		}

		//If inside a transport, damage all units loaded within
		else if (irradiatedUnit->status & UnitStatus::InTransport) {

			CUnit* transport = irradiatedUnit->connectedUnit;

			if (transport != nullptr) {
				for (int i = 0; i < units_dat::SpaceProvided[transport->id]; ++i) {
					CUnit* loadedUnit = transport->getLoadedUnit(i);
					if (loadedUnit)
						irradiateProc(irradiatedUnit, loadedUnit);
				}
			}

		}
		//Find and iterate nearby units
		else {
			scbw::UnitFinder unitFinder(irradiatedUnit->getX() - 160,
				irradiatedUnit->getY() - 160,
				irradiatedUnit->getX() + 160,
				irradiatedUnit->getY() + 160);
			unitFinder.forEach([&](CUnit* target) {
				irradiateProc(irradiatedUnit, target);
			});
		}
	}

	const uint32 Func_CreateThingy = 0x00488210;
	CThingy* createThingyFunc(uint32 spriteId, int16 x, int16 y, uint32 playerId) {
		static CThingy* thingy;
		int32 x_ = x;
		__asm {
			PUSHAD
			PUSH playerId
			MOVSX EDI, y
			PUSH x_
			PUSH spriteId
			CALL Func_CreateThingy
			MOV thingy, EAX
			POPAD
		}
		return thingy;
	}

	const uint32 Func_GetAllUnitsInBounds = 0x0042FF80;
	CUnit** getAllUnitsInBounds_copy(Box16* coords) {
		static CUnit** units_in_bounds;
		__asm {
			PUSHAD
			MOV EAX, coords
			CALL Func_GetAllUnitsInBounds
			MOV units_in_bounds, EAX
			POPAD
		}
		return units_in_bounds;
	}

	//Identical to setAllImageGroupFlagsPal11 @ 0x00497430;
	void setAllImageGroupFlagsPal11(CSprite* sprite) {
		for (
			CImage* current_image = sprite->images.head;
			current_image != nullptr;
			current_image = current_image->link.next
			) {
			if (current_image->paletteType == PaletteType::RLE_HPFLOATDRAW)
				current_image->flags |= CImage_Flags::Redraw;
		}
	}

	const uint32 Func_DoUnitsCollide = 0x00469B60;
	bool func_dounitscollide(CUnit* first, CUnit* second) {
		static uint32 result = 0;
		__asm {
			PUSHAD
			MOV EAX, first
			MOV ECX, second
			CALL Func_DoUnitsCollide
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_Sub_4E97C0 = 0x004E97C0;
	void unburrow(CUnit* unit) {
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub_4E97C0
			POPAD
		}
	}

	static bool safetyUnburrowCheck(CUnit* unit, CUnit* target) {
		if (!(target->status & UnitStatus::GroundedBuilding)) {
			return false;
		}
		if (target->id == UnitId::BasiliskCreeper) {
			return false;
		}
		if (target == unit) {
			return false;
		}
		if (!func_dounitscollide(unit, target)) {
			return false;
		}
		return true;
	}

	void updateStatusEffects(CUnit* unit) {
		if (unit->subunit != nullptr) {
			scbw::set_generic_value(unit->subunit, ValueId::TurretParentLink, (uint32)unit);
		}

		// Decrement Stasis Field timer (and remove effect if 0)
		if (unit->stasisTimer != 0) {
			unit->stasisTimer--;
			if (unit->stasisTimer == 0) {
				unit->removeStasisField();
			}
		}

		//// Update speed based on behaviors or status effects

		// This switch statement is for units that can change speeds at will (e.g. on toggle)
		switch (unit->id) {
			case UnitId::ProtossExemplar:
			{
				unit->updateSpeed();
				break;
			}
		}

		// Forcibly unburrow units that collide with buildings
		if (unit->status & UnitStatus::Burrowed) {
			auto tile = scbw::getActiveTileAt(unit->position.x, unit->position.y);
			if (tile.currentlyOccupied) {
				CUnit* nearest = scbw::UnitFinder::getNearestTarget(
					unit->getLeft() - 32,
					unit->getTop() - 32,
					unit->getRight() + 32,
					unit->getBottom() + 32,
					unit, [&](CUnit* target) {
					return safetyUnburrowCheck(unit, target);
				});
				bool canBurrowAfter = false;
				//from Ai_Burrowable
				//
				if (nearest != nullptr) {
					unburrow(unit);
				}
			}

		}

		// Azazel - Sublime Shepherd ability
		if (unit->id == UnitId::TerranAzazel) {
			if (scbw::get_generic_rust_value(unit, ValueId::SublimeShepherd) == 1) {
				scbw::UnitFinder(
					unit->getX() - 96,
					unit->getY() - 96,
					unit->getX() + 96,
					unit->getY() + 96
				).forEach([&](CUnit* target) {
					if (target == unit) {
						return;
					}
					if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
						return;
					scbw::sendGPTP_aise_cmd(unit, target, GptpId::SetSublimeShepherd, 1, 0, 0);
				});
			}
		}

		// Quarry, Iroleth, Othstoleth, Pennant resource regeneration aura
		if (unit->sprite != nullptr && unit->mainOrderId != OrderId::Die
			&& (unit->id == UnitId::TerranQuarry
				|| unit->id == UnitId::ZergIroleth
				|| unit->id == UnitId::ZergOthstoleth
				|| unit->id == UnitId::ProtossPennant)) {

			uint32 radius;
			if (unit->id == UnitId::TerranQuarry) {
				radius = 320;
			} else if (unit->id == UnitId::ZergIroleth || unit->id == UnitId::ZergOthstoleth) {
				radius = 224;
			} else if (unit->id == UnitId::ProtossPennant) {
				radius = 160;
			}

			scbw::UnitFinder(
				unit->getX() - radius,
				unit->getY() - radius,
				unit->getX() + radius,
				unit->getY() + radius
			).forEach([&](CUnit* target) {
				if (unit == target) {
					return;
				}
				if (!(unit->status & UnitStatus::Completed)) {
					return;
				}
				if (units_dat::BaseProperty[target->id] & UnitProperty::ResourceContainer) {
					scbw::set_generic_timer(target, ValueId::MineralSpecialRegenTimer, 24);
				}
			});
		}

		// Othstoleth morph speed increase aura
		if (unit->id == UnitId::ZergOthstoleth) {
			uint32 radius = 224;
			scbw::UnitFinder(
				unit->getX() - radius,
				unit->getY() - radius,
				unit->getX() + radius,
				unit->getY() + radius
			).forEach([&](CUnit* target) {
				if (unit == target) {
					return;
				}
				if (!(unit->status & UnitStatus::Completed)) {
					return;
				}
				if (scbw::isAlliedTo(target->playerId, unit->playerId) && scbw::isZergEggNew(target, true, false)) {
					scbw::set_generic_timer(target, ValueId::EggBoostTimer, 24);
				}
			});
		}

		// Ghost Somatic Implants behavior
		if (unit->id == UnitId::TerranEidolon) {
			if (
				!(unit->status & (UnitStatus::Cloaked + UnitStatus::RequiresDetection))) {
				if (scbw::get_generic_value(unit, ValueId::SomaticImplants) == 1) {
					unit->updateSpeed();
					scbw::set_generic_value(unit, ValueId::SomaticImplants, 0);
				}
			}
		}

		if (unit->stimTimer != 0) {

			unit->stimTimer--;

			if (unit->stimTimer == 0)
				unit->updateSpeed();
		}

		if (unit->id == UnitId::ZergVorvaling || unit->id == UnitId::TerranShaman || unit->id == UnitId::ZergOvileth) {
			unit->updateSpeed();
		}

		// Decrement Ensnare timer (and remove effect if 0)
		if (unit->ensnareTimer != 0) {

			unit->ensnareTimer--;

			if (unit->ensnareTimer == 0) {
				unit->removeOverlay(ImageId::EnsnareOverlay_Small, ImageId::EnsnareOverlay_Large);
				unit->updateSpeed();
			}

		}

		// Shaman - Overcharge upgrade
		if (unit->unusedTimer != 0) {
			//overcharge timer
			unit->unusedTimer--;
			if (unit->unusedTimer == 0) {
				unit->updateSpeed();
			}

		}

		// Decrement Defensive Matrix timer (and remove effect if 0)
		if (unit->defensiveMatrixTimer != 0) {

			unit->defensiveMatrixTimer--;

			if (unit->defensiveMatrixTimer == 0)
				unit->reduceDefensiveMatrixHp(unit->defensiveMatrixHp);

		}

		// Redraw console/UI elements
		if (unit == *activePortraitUnit) {
			if (unit->isValidCaster()) {
				if (!scbw::isAlliedTo(*LOCAL_NATION_ID, unit->playerId)) {
					scbw::refreshConsole();
				}
			}
		}

		// Irradiate ability
		if (unit->irradiateTimer != 0) {
			unit->irradiateTimer--;

			doIrradiateDamage(unit);
			if (unit->irradiateTimer == 0) {
				unit->removeOverlay(ImageId::IrradiateNew_Small, ImageId::IrradiateNew_Large);
				unit->irradiatedBy = nullptr;
				unit->irradiatePlayerId = 8;
			}
		}

		if (unit->lockdownTimer != 0) {

			unit->lockdownTimer--;

			if (unit->lockdownTimer == 0)
				unit->removeLockdown();

		}

		if (unit->maelstromTimer != 0) {
			unit->maelstromTimer--;
			if (unit->maelstromTimer == 0) {
				unit->removeMaelstrom();
				if (scbw::get_generic_value(unit, ValueId::Malediction)) {
					scbw::set_generic_value(unit, ValueId::Malediction, 0);
					scbw::set_generic_timer(unit, ValueId::MaledictionSlow, (24 * 4) + 1);
					uint32 overlayImageId;
					if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
						overlayImageId = ImageId::Malediction_Medium;
					else
						if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
							overlayImageId = ImageId::Malediction_Large;
						else
							overlayImageId = ImageId::Malediction_Small;
					unit->sprite->createTopOverlay(overlayImageId,
						0, 0, 0);
				}
				unit->updateSpeed();
			}
		}

		if (unit->plagueTimer != 0) {
			unit->plagueTimer--;
			if (unit->plagueTimer <= 45) {
				unit->plagueTimer = 0;
			}

			if (!(unit->status & UnitStatus::Invincible)) {
				int32 damage = (weapons_dat::DamageAmount[WeaponId::Plague] * 256) / 30;

				if (unit->plagueTimer == 0) {
					damage += (weapons_dat::DamageAmount[WeaponId::Plague] * 256) % 30;
				}

				if (damage > 0) {
					unit->damageWith(WeaponId::Plague, damage);
				}
			}

			if (unit->plagueTimer == 0) {
				unit->removeOverlay(ImageId::PlagueOverlay_Small, ImageId::PlagueOverlay_Large);
				if (unit->newFlags1 & NewFlags1::Metastasis) {
					unit->newFlags1 &= ~NewFlags1::Metastasis;
				}
			}

		}

		if (unit->id == UnitId::ProtossClarion && scbw::get_aise_value(unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1) {
			scbw::placeClarionLinks(unit); //clears, then places all clarion links
		}

		if (unit->isUnderStorm != 0) {
			unit->isUnderStorm--;
		}

		// Vithrilisk - Corrosive Touch - update overlays
		if (scbw::get_generic_value(unit, ValueId::CorrosiveTouchStacks) != 0) {

			//Calculate the appropriate overlay ID
			uint32 acidOverlayId = GetSporesOverlayId(unit);

			if (!unit->getOverlay(acidOverlayId)) {

				unit->removeOverlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);

				if (unit->subunit != nullptr)
					unit = unit->subunit;

				unit->sprite->createTopOverlay(acidOverlayId);

			}
		}
	}

	const uint32 Func_orderComputerClear = 0x00475310;
	void orderComputerClear(CUnit* a1, uint8 a2) {
		__asm {
			PUSHAD
			MOV ESI, a1
			MOVZX CL, a2
			CALL Func_orderComputerClear
			POPAD
		}
	}
	void AI_Burrower(CUnit* a1) {
		if (units_dat::BaseProperty[a1->id] & UnitProperty::Burrowable) {
			if (!(a1->status & UnitStatus::Burrowed) && a1->id != UnitId::ZergDroleth) {
				if (a1->mainOrderId == OrderId::ComputerAI || a1->mainOrderId == OrderId::Guard) {
					if (a1->moveTarget.pt.x == a1->sprite->position.x
						&& a1->moveTarget.pt.y == a1->sprite->position.y
						&& ((a1->status & UnitStatus::Unmovable) != 0) != -1
						//&& techUseAllowed(a1, TechId::Burrowing, (uint8)a1->playerId) == 1
						) {
						bool canBurrow = true;
						auto tile = scbw::getActiveTileAt(a1->position.x, a1->position.y);
						if (tile.currentlyOccupied) {
							CUnit* nearest = scbw::UnitFinder::getNearestTarget(
								a1->getLeft() - 32,
								a1->getTop() - 32,
								a1->getRight() + 32,
								a1->getBottom() + 32,
								a1,
								[&](CUnit* target) {
								return safetyUnburrowCheck(a1, target);
							}
							);
							if (nearest != nullptr) {
								canBurrow = false;
							}
						}
						if (canBurrow) {
							orderComputerClear(a1, OrderId::Burrow);
						}
					}
				}
			}
		}
	}

} //hooks
