#include "update_status_effects.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include "hooks/weapons/weapon_spellhit.h"
#include "hooks/weapons/weapon_damage.h"
#include "globals.h"

void irradiateProc(Unit* unit, Unit* target)
{
	if (!unit)
	{
		return;
	}

	//Irradiate splash damage does not affect burrowed units
	if (target != unit && target->status & UnitStatus::Burrowed)
	{
		return;
	}

	//Check if the unit is within distance, or is inside the same transport
	if (unit->status & UnitStatus::InTransport
		|| unit->getDistanceToTarget(target) <= 32)
	{
		// const s32 damage = weapons_dat::DamageAmount[WeaponId::Irradiate] * 256 / weapons_dat::Cooldown[WeaponId::Irradiate];
		s32 damage = (125 * 256) / 24;
		if (unit != nullptr)
		{
			if (unit->irradiateTimer == 0)
			{
				damage += (125 * 256) % 24;
				//scbw::print("Add additional damage");
			}
		}
		target->damageWithCustom(50.0 * 256.0 / 15.5, DamageType::Concussive, unit->irradiatedBy, unit->irradiatePlayerId, 0, 1, true);
	}
}
	
namespace hooks
{
	void doIrradiateDamage(Unit* irradiatedUnit)
	{
		//No splash if burrowed
		if (irradiatedUnit->status & UnitStatus::Burrowed)
		{
			irradiateProc(irradiatedUnit, irradiatedUnit);
		}

		//If inside a transport, damage all units loaded within
		else if (irradiatedUnit->status & UnitStatus::InTransport)
		{

			Unit* transport = irradiatedUnit->connectedUnit;

			if (transport != nullptr)
			{
				for (s32 i = 0; i < units_dat::SpaceProvided[transport->id]; ++i)
				{
					Unit* loadedUnit = transport->getLoadedUnit(i);
					if (loadedUnit)
						irradiateProc(irradiatedUnit, loadedUnit);
				}
			}

		}
		//Find and iterate nearby units
		else
		{
			scbw::UnitFinder(
				irradiatedUnit->getX() - 160,
				irradiatedUnit->getY() - 160,
				irradiatedUnit->getX() + 160,
				irradiatedUnit->getY() + 160
			).forEach([&](Unit* target)
			{
				irradiateProc(irradiatedUnit, target);
			});
		}
	}

	static bool safetyUnburrowCheck(Unit* unit, Unit* target)
	{
		if (!(target->status & UnitStatus::GroundedBuilding))
		{
			return false;
		}
		if (target == unit)
		{
			return false;
		}
		if (!helper::func_dounitscollide(unit, target))
		{
			return false;
		}
		return true;
	}

	void updateStatusEffects(Unit* unit)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		if (unit->subunit != nullptr)
		{
			UnitExt* subunitExt = globals.GetUnitExt(unit->subunit);
			subunitExt->units[UnitTurretParentLink] = unit;
		}

		// Decrement Stasis Field timer (and remove effect if 0)
		if (unit->stasisTimer != 0)
		{
			unit->stasisTimer--;
			if (unit->stasisTimer == 0)
			{
				unit->removeStasisField();
			}
		}

		// Forcibly unburrow units that collide with buildings
		if (unit->status & UnitStatus::Burrowed)
		{
			auto tile = scbw::getActiveTileAt(unit->position.x, unit->position.y);
			if (tile.currentlyOccupied)
			{
				Unit* nearest = scbw::UnitFinder::getNearestTarget(
					unit->getLeft() - 32,
					unit->getTop() - 32,
					unit->getRight() + 32,
					unit->getBottom() + 32,
					unit, [&](Unit* target)
				{
					return safetyUnburrowCheck(unit, target);
				});
				bool canBurrowAfter = false;
				// from Ai_Burrowable
				if (nearest != nullptr)
				{
					helper::unburrow(unit);
				}
			}
		}

		// Quarry, Iroleth, Pennant resource regeneration aura
		if (unit->sprite != nullptr && unit->mainOrderId != OrderId::Die
			&& (unit->id == UnitId::TerranQuarry
				|| unit->id == UnitId::ZergIroleth
				|| unit->id == UnitId::ProtossPennant))
		{
			u32 radius;
			if (unit->id == UnitId::TerranQuarry)
			{
				radius = 320;
			}
			else if (unit->id == UnitId::ZergIroleth)
			{
				radius = 224;
			}
			else if (unit->id == UnitId::ProtossPennant)
			{
				radius = 160;
			}

			scbw::UnitFinder(
				unit->getX() - radius,
				unit->getY() - radius,
				unit->getX() + radius,
				unit->getY() + radius
			).forEach([&](Unit* target)
			{
				if (unit == target)
				{
					return;
				}
				if (!(unit->status & UnitStatus::Completed))
				{
					return;
				}
				if (units_dat::BaseProperty[target->id] & UnitProperty::ResourceContainer)
				{
					unitExt->timers[TimerMineralSpecialRegen] = 24;
				}
			});
		}

		// Sire Ectomorphic Accelerant - egg morph speed aura
		if (unit->id == UnitId::ZergSire)
		{
			u32 radius = 224;
			scbw::UnitFinder(
				unit->getX() - radius,
				unit->getY() - radius,
				unit->getX() + radius,
				unit->getY() + radius
			).forEach([&](Unit* target)
			{
				if (unit == target)
				{
					return;
				}
				if (!(unit->status & UnitStatus::Completed))
				{
					return;
				}
				if (scbw::isAlliedTo(target->playerId, unit->playerId)
					&& (target->id == UnitId::ZergEgg || target->id == UnitId::ZergCocoon))
				{
					UnitExt* targetExt = globals.GetUnitExt(target);
					targetExt->timers[TimerSireEggBoost] = 24;
				}
			});
		}

		// Ghost Somatic Implants behavior
		if (unit->id == UnitId::TerranEidolon)
		{
			if (!(unit->status & (UnitStatus::Cloaked + UnitStatus::RequiresDetection)))
			{
				if (unitExt->values[ValueSomaticImplants] == 1)
				{
					unit->updateSpeed();
					unitExt->values[ValueSomaticImplants] = 0;
				}
			}
		}

		if (unit->stimTimer != 0)
		{
			unit->stimTimer--;
			if (unit->stimTimer == 0)
			{
				unit->updateSpeed();
			}
		}

		// Why are we updating Vorvaling/Ovileth speed every frame?
		if (unit->id == UnitId::ZergVorvrokor || unit->id == UnitId::ZergOvileth)
		{
			unit->updateSpeed();
		}

		// Decrement Ensnare timer (and remove effect if 0)
		if (unit->ensnareTimer != 0)
		{
			unit->ensnareTimer--;
			if (unit->ensnareTimer == 0)
			{
				unit->removeOverlay(ImageId::EnsnareOverlay_Small, ImageId::EnsnareOverlay_Large);
				unit->updateSpeed();
			}
		}

		// Decrement Defensive Matrix timer (and remove effect if 0)
		if (unit->defensiveMatrixTimer != 0)
		{
			unit->defensiveMatrixTimer--;
			if (unit->defensiveMatrixTimer == 0)
			{
				unit->reduceDefensiveMatrixHp(unit->defensiveMatrixHp);
			}
		}

		// Redraw console/UI elements
		if (unit == *activePortraitUnit)
		{
			if (unit->isValidCaster())
			{
				if (!scbw::isAlliedTo(*LOCAL_NATION_ID, unit->playerId))
				{
					scbw::refreshConsole();
				}
			}
		}

		// Irradiate ability
		if (unit->irradiateTimer != 0)
		{
			unit->irradiateTimer--;
			doIrradiateDamage(unit);
			if (unit->irradiateTimer == 0)
			{
				unit->removeOverlay(ImageId::IrradiateNew_Small, ImageId::IrradiateNew_Large);
				unit->irradiatedBy = nullptr;
				unit->irradiatePlayerId = 8;
			}
		}

		if (unit->lockdownTimer != 0)
		{
			unit->lockdownTimer--;
			if (unit->lockdownTimer == 0)
			{
				unit->removeLockdown();
			}
		}

		if (unit->maelstromTimer != 0)
		{
			unit->maelstromTimer--;
			if (unit->maelstromTimer == 0)
			{
				unit->removeMaelstrom();
			}
		}

		if (unit->plagueTimer != 0)
		{
			unit->plagueTimer--;

			if (!(unit->status & UnitStatus::Invincible))
			{
				s32 damage = (weapons_dat::DamageAmount[WeaponId::Plague] * 256) / 30;
				if (unit->plagueTimer == 0)
				{
					damage += (weapons_dat::DamageAmount[WeaponId::Plague] * 256) % 30;
				}
				if (damage > 0)
				{
					unit->damageWith(WeaponId::Plague, damage);
				}
			}

			if (unit->plagueTimer == 0)
			{
				unit->removeOverlay(ImageId::PlagueOverlay_Small, ImageId::PlagueOverlay_Large);
				if (unit->newFlags1 & NewFlags1::Metastasis)
				{
					unit->newFlags1 &= ~NewFlags1::Metastasis;
				}
			}

		}

		if (unit->id == UnitId::ProtossClarion && scbw::get_aise_value(unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
		{
			scbw::placeClarionLinks(unit); //clears, then places all clarion links
		}

		if (unit->isUnderStorm != 0)
		{
			unit->isUnderStorm--;
		}

		// Vithrilisk - Corrosive Touch - update overlays
		if (unitExt->values[ValueCorrosiveTouchStacks] != 0)
		{
			// Calculate the appropriate overlay ID
			u32 acidOverlayId = GetSporesOverlayId(unit);
			if (!unit->getOverlay(acidOverlayId))
			{
				unit->removeOverlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);
				if (unit->subunit != nullptr)
				{
					unit = unit->subunit;
				}
				unit->sprite->createTopOverlay(acidOverlayId);
			}
		}
	}

	void AI_Burrower(Unit* a1)
	{
		if (units_dat::BaseProperty[a1->id] & UnitProperty::Burrowable)
		{
			if (!(a1->status & UnitStatus::Burrowed) && a1->id != UnitId::ZergDroleth)
			{
				if (a1->mainOrderId == OrderId::ComputerAI || a1->mainOrderId == OrderId::Guard)
				{
					if (a1->moveTarget.pt.x == a1->sprite->position.x
						&& a1->moveTarget.pt.y == a1->sprite->position.y
						&& ((a1->status & UnitStatus::Unmovable) != 0) != -1
						// Originally checked for burrow tech here
						)
					{
						bool canBurrow = true;
						auto tile = scbw::getActiveTileAt(a1->position.x, a1->position.y);
						if (tile.currentlyOccupied)
						{
							Unit* nearest = scbw::UnitFinder::getNearestTarget(
								a1->getLeft() - 32,
								a1->getTop() - 32,
								a1->getRight() + 32,
								a1->getBottom() + 32,
								a1,
								[&](Unit* target)
							{
								return safetyUnburrowCheck(a1, target);
							}
							);
							if (nearest != nullptr)
							{
								canBurrow = false;
							}
						}
						if (canBurrow)
						{
							helper::order_computer(a1, OrderId::Burrow);
						}
					}
				}
			}
		}
	}
}
