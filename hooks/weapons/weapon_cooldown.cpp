#include "weapon_cooldown.h"
#include "../../SCBW/scbwdata.h"
#include "../../SCBW/enumerations.h"
#include "../../SCBW/api.h"
#include "../../MathUtils.h"
#include "globals.h"

namespace hooks
{

	/// Calculates the unit's weapon cooldown, factoring in upgrades and status effects.
	///
	/// @return		The modified cooldown value.
	u32 getModifiedWeaponCooldownHook(CUnit* unit, u8 weaponId)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		u32 cooldown = weapons_dat::Cooldown[weaponId];

		// Devourer - Corrosive Spores: lower attack speed of victim
		if (unitExt->CorrosiveTouchStacks != 0)
		{
			cooldown *= 1 + unitExt->CorrosiveTouchStacks * VithriliskCorrosiveTouchCooldownIncrease;
		}

		// Set modifier based on Stim (increase), Ensnare (decrease), and attackspeed upgrades (increase)
		s32 cooldownModifier = (unit->stimTimer ? 1 : 0) - (unit->ensnareTimer ? 1 : 0)
			+ (unit->status & UnitStatus::CooldownUpgrade ? 1 : 0);

		// Vassal Terminal Surge behavior
		if (units_dat::BaseProperty[unit->id] & UnitProperty::RoboticUnit)
		{
			f64 stacks = unitExt->TerminalSurge;
			if (stacks > 0)
			{
				cooldown /= 1.0 + 0.05 * stacks;
			}
		}

		// Madcap Maverick Feeders behavior: increase weapon speed while stationary
		if (unit->id == UnitId::TerranMadcap)
		{
			u32 stacks = ceil((f64)unitExt->MadcapStacks / MadcapPassiveDecayTime);
			if (stacks > 0)
			{
				//scbw::printString(std::to_string(stacks) + ", " + std::to_string(scbw::get_generic_value(unit, ValueId::MadcapStacks)));
				f64 newCooldown = (f64)cooldown * (1.0 - stacks * MadcapAttackSpeedPerStack);
				cooldown = newCooldown;
			}
		}

		// Blackjack Wear and Tear: reduce weapon speed based on missing health
		else if (unit->id == UnitId::TerranBlackjack)
		{
			const u32 health = unit->hitPoints / 256;
			const u32 maxHealth = unit->getMaxHpInGame();
			const f64 multiplier = MathUtils::map(health, (u32)0, maxHealth, 0.66, 1.0, true); // nice casting tbh
			cooldown /= multiplier;
		}

		// Kaiser Rampage upgrade: gains f64 attack speed for 1 second on kill
		if (unit->id == UnitId::ZergUltrakor)
		{
			if (unitExt->KaiserRampage > 0)
			{
				cooldown /= 2;
			}
		}

		// Phobos Argent Capacitors behavior
		if (unit->id == UnitId::TerranPhobos)
		{
			if (unitExt->PhobosDischarge > 0)
			{
				cooldown /= 2;
			}
		}

		// Atreus Stalwart Stance behavior: gains 125% attack speed while shields remain intact
		if (unit->id == UnitId::ProtossAtreus
			&& unit->getCurrentShieldsInGame() > 1)
		{ // Leaving it at 1 so incidental regen doesn't give the bonus
			cooldown /= 1.25;
		}

		// Ion Cannon Ion Field: Attack Speed Buff
		if (unitExt->CausticDischargeSpeed > 0)
		{
			cooldown /= 1.5;
		}

		if (cooldownModifier > 0)
		{	// Lame hardcode for attackspeed upgrades
			cooldown /= 2;
		}
		else
		{
			if (cooldownModifier < 0)
			{	// Presumably, lame hardcode for some slow
				cooldown += cooldown / 4;

			}
		} if (unit->stimTimer)
		{ // Fixes aice-related Stimpack issue
			cooldown += 2;
		}
		u32 adrenalStacks = unitExt->AdrenalStacks;
		if (adrenalStacks)
		{
			cooldown -= adrenalStacks / 2;
		}
		if (cooldown > 250)
		{ // Max cutoff
			cooldown = 250;
		}
		else if (cooldown < 3)
		{ // Min cutoff (previously 5)
			cooldown = 3;
		}

		return cooldown;
	}

} //hooks
