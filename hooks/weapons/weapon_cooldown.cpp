#include "weapon_cooldown.h"
#include "../../SCBW/scbwdata.h"
#include "../../SCBW/enumerations.h"
#include "../../SCBW/api.h"
#include "../../math_utils.h"
#include "globals.h"

namespace hooks
{

	/// Calculates the unit's weapon cooldown, factoring in upgrades and status effects.
	/// @return	the modified cooldown value.
	u32 getModifiedWeaponCooldownHook(Unit* unit, u8 weaponId)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);
		u32 cooldown = weapons_dat::Cooldown[weaponId];

		// Set modifier based on Stim (increase), Ensnare (decrease), and attackspeed upgrades (increase)
		s32 cooldownModifier = (unit->stimTimer ? 1 : 0) - (unit->ensnareTimer ? 1 : 0)
			+ (unit->status & UnitStatus::CooldownUpgrade ? 1 : 0);

		// Crucible Aura
		cooldown /= 1 + 0.05 * unitExt->unitVectors[UnitVectorCrucibles].size();

		// Anticthon - Passive
		cooldown /= 1 + 0.02 * unitExt->values[ValueAnticthonStacks];

		// Vithrilisk - Corrosive Spores: lower attack speed of victim
		cooldown *= 1 + unitExt->values[ValueCorrosiveTouchStacks] * VithriliskCorrosiveTouchCooldownIncrease;

		// Vassal Terminal Surge behavior
		cooldown /= 1.0 + 0.05 * unitExt->values[ValueTerminalSurge];

		// Madcap Maverick Feeders behavior: increase weapon speed while stationary
		if (unit->id == UnitId::TerranMadcap)
		{
			u32 stacks = ceil((f64)unitExt->values[ValueMadcapStacks] / MadcapPassiveDecayTime);
			if (stacks > 0)
			{
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

		// Kaiser Rampage upgrade: gains double attack speed for 1 second on kill
		if (unit->id == UnitId::ZergUltrokor)
		{
			if (unitExt->timers[TimerKaiserRampage] > 0)
			{
				cooldown /= 2;
			}
		}

		// Phobos Argent Capacitors behavior
		if (unit->id == UnitId::TerranPhobos)
		{
			if (unitExt->values[ValuePhobosDischarge] > 0)
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
		if (unitExt->timers[TimerCausticDischargeSpeed] > 0)
		{
			cooldown /= 1.5;
		}

		// Hierophant Signify: Malice
		if (unitExt->values[ValueSignifyStacks] >= 5)
		{
			cooldown /= 0.6666;
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
		u32 adrenalStacks = unitExt->values[ValueAdrenalStacks];
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

	f32 getWeaponCooldownFloat(Unit* unit, u8 weaponId)
	{
		return getModifiedWeaponCooldownHook(unit, weaponId) / 24.0f;
	}

	f32 getCurrentDPS(Unit* unit, u8 weaponId)
	{
		return weapons_dat::DamageAmount[weaponId] / getWeaponCooldownFloat(unit, weaponId);
	}

} //hooks
