#include "hooks\tactical_ai.h"
#include "SCBW/api.h"
#include "SCBW/scbwdata.h"
#include "unit_constants.h"
#include "math_utils.h"
#include "globals.h"
#include "hooks/weapons/weapon_cooldown.h"
#include "hooks/weapons/weapon_damage.h"

/* DONE
Apostle Lazarus Agent: If 5 or more organic allies (that don't have the buff) are nearby,
cast the spell. **DONE**
AI should also try to position Apostles near organics.

Matraleth Parasite: When 5 or more supply's worth of units (of any owner) are within 128px of each other,
target the group. **DONE**

Star Sovereign Grief of All Gods: When at least 2000 HP+shields worth of enemy units are within 192px of the Star Sovereign,
and there are no allied town halls and less than 2000 HP+shields worth of allied units within the same radius,
and the Star Sovereign has more than 50% of HP+shields combined,
channel the ability. **DONE**
Attempt to protect the Star Sovereign with low-cost/low-HP units.

Lanifect Disruption Field:
When there are at least 3 allies within 128px, and there are at least 2 enemies within 128px,
and there are no activated Disruption Fields within 96px,
and the Lanifect is above 50% total HP and shields,
move towards the most significant enemy threat within 128px (in terms of enemy unit count).
Recheck conditions to disable the behavior (besides HP/shield checks). **DONE**
*/

/* TODO
Eidolon Lobotomy Mine: I think all mines should be reworked in terms of how the AI places them,
I would suggest dropping them in positions where enemy ground units
have been spotted/attacked/killed within the last few minutes.
That way it's like they're responding to ground paths taken by enemies.
Vulture Spider Mines could be done in similar ways.

Madrigal Tempo Change: When 3 or more hostiles within 4 weapon range,
switch to Tempo Up (Furia);
else, switch to Tempo Down (Imperioso)

Paladin Skyfall: If 6 or more enemies are within 64px of each other while being within range,
target terrain between the Paladin and the target group, 64px away from the group.

Science Vessel Observance: If 3 or more enemies are within 96px of each other, and the Vessel has more than 50% HP, target
the group and stay within range while attempting to avoid damage.

Azazel Sublime Shepherd: Toggle while 2 or more allied units are within 96px of the Azazel, and stay within 96px of as many
allies as possible.

Didact Recall: "Offensive" cast when the Didact is added to an attack and a group of 6 or more units are within 160px of
one another. Attempt to group as many units as possible during the channel.
*/

#define ENABLE_KITE_DEBUG_MESSAGES 0

#if ENABLE_KITE_DEBUG_MESSAGES
#define KITE_DEBUG(...) scbw::printf(__VA_ARGS__)
#else
#define KITE_DEBUG(...)
#endif

struct KitePositions
{
	bool enemies_found;
	f32 biases[9];
	u32 forward_index;
	u32 backward_index;
	u32 strafe_index;
	Point16 forward_pos;
	Point16 backward_pos;
	Point16 strafe_pos;
};

const u32 MoveRadius = 4 * 32;
const u32 LocalGroupRadius = 4 * 32;
const u32 CrowdCalcRadius = 2 * 32;

const u32 KiteZoneCount = 9;
const Vector32f KiteOffsets[KiteZoneCount] =
{
	{-1, -1}, {0, -1}, {+1, -1},
	{-1,  0}, {0,  0}, {+1,  0},	
	{-1, +1}, {0, +1}, {+1, +1},
};
const Vector32f BiasOrigins[KiteZoneCount] =
{
	{-0.7, -0.7}, { 0, -1.0}, {+0.7, -0.7},
	{-1.0,    0}, { 0,    0}, {+1.0,    0},	
	{-0.7, +0.7}, { 0, +1.0}, {+0.7, +0.7},
};

static u32 get_max_range(u32 unitId)
{
	if (unitId == UnitId::ProtossSolarion)
	{
		return 8 * 32;
	}
	if (unitId == UnitId::ProtossAccantor)
	{
		return 8 * 32;
	}

	u32 ground_weapon = units_dat::GroundWeapon[unitId];
	u32 ground_range = ground_weapon != WeaponId::None ? weapons_dat::MaxRange[ground_weapon] : 0;

	u32 air_weapon = units_dat::AirWeapon[unitId];
	u32 air_range = air_weapon != WeaponId::None ? weapons_dat::MaxRange[air_weapon] : 0;

	return std::max(ground_range, air_range);
}

static bool is_melee_attacker(u32 unitId)
{
	return get_max_range(unitId) <= 32;
}

static KitePositions get_kite_positions(Unit* unit, u32 hostile_group_radius)
{
	u32 zone_value[KiteZoneCount] = {0};
	KitePositions result = {};
	result.enemies_found = false;

	for (int i = 0; i < KiteZoneCount; i++)
	{
		Point16 search_pos = unit->position;
		search_pos.x += KiteOffsets[i].x * hostile_group_radius;
		search_pos.y += KiteOffsets[i].y * hostile_group_radius;

		scbw::UnitFinder(
			search_pos.x - hostile_group_radius,
			search_pos.y - hostile_group_radius,
			search_pos.x + hostile_group_radius,
			search_pos.y + hostile_group_radius
		).forEachInMatch(cosmonarchyFinderMatch(unit, Flags::Enemy | Flags::CanAttackOrCast, {}),
			[&](Unit* hostileUnit)
		{
			u32 strength = 0;
			if (hostileUnit->canAttackTarget(unit, true)) {
				if (unit->status & UnitStatus::InAir) {
					strength = hostileUnit->airStrength;
				} else {
					strength = hostileUnit->groundStrength;
				}
			}

			if (strength && !(units_dat::BaseProperty[hostileUnit->id] & UnitProperty::Building))
			{
				zone_value[i] += strength;
				result.enemies_found = true;
			}
		});
	}

	if (result.enemies_found)
	{
		Vector32f biasVectors[KiteZoneCount] =
		{
			{-0.7, -0.7}, { 0, -1.1}, {+0.7, -0.7},
			{-1.1,    0}, { 0,    0}, {+1.1,    0},	
			{-0.7, +0.7}, { 0, +1.1}, {+0.7, +0.7},
		};
		for (int i = 0; i < KiteZoneCount; i++)
		{
			result.biases[i] = 0;
			biasVectors[i].x *= zone_value[i];
			biasVectors[i].y *= zone_value[i];
		}
		for (int i = 0; i < KiteZoneCount; i++)
		{
			for (int j = 0; j < KiteZoneCount; j++)
			{
				result.biases[i] += scbw::getDotProduct(BiasOrigins[i], biasVectors[j]);
			}
			result.biases[i] /= KiteZoneCount;
		}

		result.forward_index = KiteZoneCount / 2;
		result.backward_index = KiteZoneCount / 2;
		result.strafe_index = 0;
		for (int i = 0; i < KiteZoneCount; i++)
		{
			if (i == KiteZoneCount / 2)
			{
				continue;
			}

			if (result.biases[i] > result.biases[result.forward_index])
			{
				result.forward_index = i;
			}

			if (result.biases[i] < result.biases[result.backward_index])
			{
				result.backward_index = i;
			}

			if (fabs(result.biases[i]) < fabs(result.biases[result.strafe_index]))
			{
				result.strafe_index = i;
			}
		}

		//KITE_DEBUG("%f %f %f", biases[0], biases[1], biases[2]);
		//KITE_DEBUG("%f %f %f", biases[3], biases[4], biases[5]);
		//KITE_DEBUG("%f %f %f", biases[6], biases[7], biases[8]);
		//KITE_DEBUG("\n");

		result.forward_pos = unit->position;
		result.forward_pos.x += KiteOffsets[result.forward_index].x * MoveRadius;
		result.forward_pos.y += KiteOffsets[result.forward_index].y * MoveRadius;
		result.forward_pos = scbw::clampPoint(result.forward_pos);

		result.backward_pos = unit->position;
		result.backward_pos.x += KiteOffsets[result.backward_index].x * MoveRadius;
		result.backward_pos.y += KiteOffsets[result.backward_index].y * MoveRadius;
		result.backward_pos = scbw::clampPoint(result.backward_pos);

		result.strafe_pos = unit->position;
		result.strafe_pos.x += KiteOffsets[result.strafe_index].x * MoveRadius;
		result.strafe_pos.y += KiteOffsets[result.strafe_index].y * MoveRadius;
		result.strafe_pos = scbw::clampPoint(result.strafe_pos);
	}
	return result;
}

static u8 get_current_weapon_cooldown(Unit* unit)
{
	if (unit->id == UnitId::ProtossSolarion || unit->id == UnitId::ProtossAccantor)
	{
		if (unit->mainOrderTimer == 0 && (unit->carrier.inHangarCount || (*elapsedTimeFrames) % 24 == 0))
		{
			return 0;
		}
		return 2;
	}

	return std::max(unit->groundWeaponCooldown, unit->airWeaponCooldown);
}

static u8 get_max_weapon_cooldown(Unit* unit)
{
	if (unit->id == UnitId::ProtossSolarion || unit->id == UnitId::ProtossAccantor)
	{
		return 1;
	}

	u32 ground_weapon = unit->getGroundWeapon();
	u32 ground_cooldown = 0;
	if (ground_weapon != WeaponId::None)
	{
		ground_cooldown = hooks::getModifiedWeaponCooldownHook(unit, ground_weapon);
	}

	u32 air_weapon = unit->getAirWeapon();
	u32 air_cooldown = 0;
	if (air_weapon != WeaponId::None)
	{
		air_cooldown = hooks::getModifiedWeaponCooldownHook(unit, air_weapon);
	}

	return std::max(ground_cooldown, air_cooldown);
}

static u32 get_distance_to_target(Unit* source)
{
	Unit* target = source->orderTarget.unit;
	if (!target)
	{
		return get_max_range(source->id);
	}

	return scbw::get_distance_fast(source->position, target->position);
}

static bool is_channeling(Unit* unit) {
	if (unit->mainOrderId == OrderId::NukeTrack) {
		return true;
	}
	if (unit->mainOrderId == OrderId::Plague && unit->sprite->mainGraphic->animation == IscriptAnimation::Unused2) {
		return true;
	}
	if (unit->mainOrderId == OrderId::FireYamatoGun1 && unit->sprite->mainGraphic->animation == IscriptAnimation::CastSpell) {
		return true;
	}
	if (unit->mainOrderId == OrderId::PaladinSkyfall && unit->sprite->mainGraphic->animation == IscriptAnimation::CastSpell) {
		return true;
	}
	if (unit->id == UnitId::ProtossStarSovereign && unit->mainOrderId == OrderId::CTFCOP2 && unit->sprite->mainGraphic->animation == IscriptAnimation::CastSpell) {
		return true;
	}
	if (unit->id == UnitId::ProtossAnthelion && unit->mainOrderId == OrderId::Move && unit->sprite->mainGraphic->animation == IscriptAnimation::Walking) {
		return true;
	}
	return false;
}

static s32 get_target_cost(Unit* source, Unit* target)
{
	// Value attacking wounded units first
	s32 cost = target->get_current_life_ingame() / 10;

	// Value attacking more threatning units first
	if (units_dat::BaseProperty[target->id] & UnitProperty::Spellcaster
		&& target->id != UnitId::TerranComsatStation
		&& target->id != UnitId::ProtossShieldBattery)
	{
		cost -= (target->energy / 256 / 50) * 20;
	}

	// Value channeling units more
	if (is_channeling(target)) {
		cost -= 50;
	}

	// Value big boys more
	switch (target->id) {
		case UnitId::ProtossAnthelion: {
			cost -= 50;
		} break;
		case UnitId::SharedAnticthon:
		case UnitId::SharedEmpress:
		case UnitId::ProtossDemiurge:
		case UnitId::ProtossClarion: {
			cost -= 25;
		} break;
	}

	// Value splitting simulacra more
	UnitExt* target_ext = globals.get_unit_ext(target);
	if (target_ext->timers[TimerSimulacrumClone]) {
		cost -= 5;
	}

	if (target->id == UnitId::ProtossAccantor)
	{
		cost -= (weapons_dat::DamageAmount[WeaponId::Scarab] /* / INSERT ACCANTOR WEAPON COOLDOWN HERE / 24.0f*/)
			* 1.5; // aoe multiplier
	}
	else if (target->id == UnitId::ProtossSolarion)
	{
		cost -= (weapons_dat::DamageAmount[WeaponId::PulseCannon] /* / INSERT STEWARD WEAPON COOLDOWN HERE / 24.0f*/)
			* (target->carrier.inHangarCount + target->carrier.outHangarCount)
			* 1.5; // additional bias to encourage targeting solarions more
	}
	else
	{
		u32 weapon_id = WeaponId::None;
		if (source->status & UnitStatus::InAir)
		{
			weapon_id = target->getAirWeapon();
		}
		else
		{
			weapon_id = target->getGroundWeapon();
		}
		f32 multiplier = weapons_dat::InnerSplashRadius[weapon_id] ? 1.5 : 1;
		cost -= hooks::getCurrentDPS(target, weapon_id) * multiplier;
	}

	// Value attacking units with smaller pathing cost more
	s32 dist = (s32)scbw::get_distance_fast(source->position, target->position) - get_max_range(source->id);
	if (dist > 0)
	{
		cost += dist / 32.0;
	}

	// Value attacking strategically important targets
	if (target->id == UnitId::ZergNydusCanal
		|| (units_dat::BaseProperty[target->id] & UnitProperty::Worker
			&& target->mainOrderId == OrderId::ConstructingBuilding)
		|| (units_dat::BaseProperty[target->id] & UnitProperty::ResourceDepot
			&& !(target->status & UnitStatus::Completed)))
	{
		cost -= 50;
	}
	else if (target->id == UnitId::ProtossPylon
		|| units_dat::BaseProperty[target->id] & UnitProperty::ResourceDepot
		|| units_dat::BaseProperty[target->id] & UnitProperty::Worker
		|| units_dat::GroupFlags[target->id].isFactory
		|| scbw::unit_id_is_tech(target->id))
	{
		cost -= 25;
	}
	
	{
		u32 weapon_id = 0;
		if (target->status & UnitStatus::InAir)
		{
			weapon_id = source->getAirWeapon();
		}
		else
		{
			weapon_id = source->getGroundWeapon();
		}

		// Value dark swarm immune units way less
		if (darkswarm_blocks_damage(target, weapon_id, 1))
		{
			cost += 1000;
		}
	}

	return cost;
}

enum KiterType
{
	KiterType_TargetingOnly,
	KiterType_Melee,
	KiterType_Ranged,
	KiterType_Artillery,
	KiterType_Flagship,

	KiterType_Count,
};

static KiterType get_kiter_type(u32 unit_id)
{
	if (units_dat::BaseProperty[unit_id] & UnitProperty::Building) {
		// No kiting; Just smart targeting
		return KiterType_TargetingOnly;
	}

	switch(unit_id)
	{
		case UnitId::ZergLakizilisk:
		case UnitId::TerranPhalanxSiegeMode:
		{
			// No kiting; Just smart targeting
			return KiterType_TargetingOnly;
		}

		case UnitId::ProtossSolarion:
		case UnitId::ProtossArchitect:
		case UnitId::ProtossAccantor:
		case UnitId::ZergKeskathalor:
		case UnitId::ZergGeszithalor:
		case UnitId::ZergBactalisk:
		case UnitId::TerranPaladin:
		{
			// Stay back
			return KiterType_Artillery;
		}

		case UnitId::TerranMinotaur:
		case UnitId::TerranCentaur:
		case UnitId::TerranPhobos:
		case UnitId::ProtossEmpyrean:
		case UnitId::ProtossStarSovereign:
		{
			// Advantage / Disadvantage kiting
			return KiterType_Flagship;
		}
	}

	if (is_melee_attacker(unit_id))
	{
		// Minor adjustments, while in melee range
		return KiterType_Melee;
	}
	// Default kiting
	return KiterType_Ranged;
}

static bool unit_could_kite(Unit* unit)
{
	UnitExt* unit_ext = globals.get_unit_ext(unit);
	bool in_combat = unit_ext->damageReceiveTimer < 4 * 24
		|| unit_ext->damageDealtTimer < 4 * 24
		|| unit->mainOrderId == OrderId::Patrol
		|| unit->mainOrderId == OrderId::AttackMove
		|| unit->mainOrderId == OrderId::AttackMoveComputer
		|| unit->mainOrderId == OrderId::AttackUnit
		|| unit->mainOrderId == OrderId::CarrierFight
		|| unit->mainOrderId == OrderId::ReaverFight
		|| unit->mainOrderId == OrderId::Move
		|| unit->mainOrderId == OrderId::MoveToMinerals
		|| unit->mainOrderId == OrderId::HarvestGas1
		|| unit->mainOrderId == OrderId::ComputerAI
		|| unit->mainOrderId == OrderId::ComputerReturn;

	if (unit->id == UnitId::ZergLakizilisk) {
		if (!(unit->status & UnitStatus::Burrowed)) {
			return false;
		}
	} else {
		if (unit->status & UnitStatus::Burrowed) {
			return false;
		}
	}

	return in_combat
		&& unit->hasWeapon()
		&& !unit->isFrozen()
		&& unit->status & UnitStatus::Completed
		&& !(unit->movementFlags & MovementFlags::StartingAttack)	
		// Special units
		&& unit->id != UnitId::ProtossSteward
		&& unit->id != UnitId::ProtossPlasmaShell
		//&& unit->id != UnitId::ProtossAccantor // currently disabled
		&& unit->id != UnitId::TerranNuclearMissile
		&& unit->id != UnitId::ProtossAnthelion
		// Other rules
		&& unit->mainOrderId != OrderId::BuildTerran
		&& unit->mainOrderId != OrderId::ConstructingBuilding
		&& unit->mainOrderId != OrderId::BuildProtoss1
		&& unit->mainOrderId != OrderId::BuildProtoss2
		&& unit->mainOrderId != OrderId::DroneLand
		&& unit->mainOrderId != OrderId::DroneStartBuild
		&& unit->mainOrderId != OrderId::DroneBuild
		&& !unit_ext->values[ValueDisruptionFieldOn];
}

static bool unit_can_kite(Unit* unit) {
	// ComputerReturn was patched away, still keeping the unreliable hardcode just in case
	UnitExt* unit_ext = globals.get_unit_ext(unit);
	return get_kiter_type(unit->id) != KiterType_TargetingOnly && (unit_ext->timers[TimerAI_KitingCooldown] == 0 || unit->mainOrderId == OrderId::ComputerReturn);
}

static bool unit_kiting_can_attack(Unit* unit)
{
	if (unit->id == UnitId::ProtossSolarion || unit->id == UnitId::ProtossAccantor)
	{
		return get_current_weapon_cooldown(unit) <= get_max_weapon_cooldown(unit);
	}
	if (unit->movementFlags & MovementFlags::Accelerating || unit->flingyTurnSpeed < 35)
	{
		return get_current_weapon_cooldown(unit) <= get_max_weapon_cooldown(unit);
	}
	return get_current_weapon_cooldown(unit) <= get_max_weapon_cooldown(unit) * 0.5;
}

void kiting_ai(Unit* unit, UnitExt* unit_ext)
{
	// Kiting AI
	if (unit_could_kite(unit) && unit_ext->timers[TimerAI_KitingLeash] == 1)
	{
		unit_ext->timers[TimerAI_KitingCooldown] = 1 * 24;
		unit->orderTo(OrderId::Move, unit_ext->aiKitingOrigin.x, unit_ext->aiKitingOrigin.y);
		unit->issueQueuedOrderTarget(OrderId::ComputerAI, 0, false);
	}

	if (unit_could_kite(unit))
	{
		u32 hostile_group_radius = 6 * 32;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
		{
			hostile_group_radius = 2 * 32;
		}

		scbw::UnitFinder local_group(
			unit->getX() - LocalGroupRadius,
			unit->getY() - LocalGroupRadius,
			unit->getX() + LocalGroupRadius,
			unit->getY() + LocalGroupRadius
		);
		local_group.filter(cosmonarchyFinderMatch(unit, Flags::IncludingSelf | Flags::Own | Flags::CanAttackOrCast, {unit->id}));
		local_group.filter([&](Unit* local_unit){ return unit_could_kite(local_unit); });

		if (unit_kiting_can_attack(unit))
		{
			local_group.forEach([&](Unit* local_unit)
			{
				if (!unit_kiting_can_attack(local_unit))
				{
					return;
				}

				UnitExt* local_unit_ext = globals.get_unit_ext(local_unit);
				if (local_unit_ext->values[ValueAI_PreventTargeting])
				{
					return;
				}

				if (get_kiter_type(local_unit->id) != KiterType_TargetingOnly)
				{
					if (units_dat::BaseProperty[local_unit->id] & UnitProperty::Worker)
					{
						local_unit_ext->timers[TimerAI_KitingLeash] = 4 * 24;
					}
					else
					{
						local_unit_ext->timers[TimerAI_KitingLeash] = 24 * 24;
					}
					local_unit_ext->aiKitingOrigin = local_unit->position;
				}

				auto target_match = cosmonarchyFinderMatch(local_unit, Flags::Enemy, {});
				Unit* best_target = 0;
				s32 best_cost = INT_MAX;
				scbw::UnitFinder(
					local_unit->getX() - hostile_group_radius * 2,
					local_unit->getY() - hostile_group_radius * 2,
					local_unit->getX() + hostile_group_radius * 2,
					local_unit->getY() + hostile_group_radius * 2
				).forEach([&](Unit* target) {
					if (!target_match(target) || !local_unit->canAttackTarget(target, true)) {
						return;
					}

					UnitExt* target_ext = globals.get_unit_ext(target);
					if (!target_ext->timers[TimerAI_AvoidTargeting] && unit->canAttackTarget(target, true) && target->isVisibleTo(unit->player)) {
						s32 cost = get_target_cost(local_unit, target);
						if (cost < best_cost)
						{
							best_target = target;
							best_cost = cost;
						}
					}
				});

				if (best_target)
				{
					local_unit_ext->timers[TimerAI_KitingCooldown] = 2;
					KITE_DEBUG("%s: attack", local_unit->getName());
					if (local_unit->id == UnitId::ProtossSolarion)
					{
						local_unit->orderTo(OrderId::CarrierFight, best_target);
					}
					else if (local_unit->id == UnitId::ProtossAccantor)
					{	
						local_unit->orderTo(OrderId::ReaverFight, best_target);
					}
					else
					{
						local_unit->orderTo(OrderId::AttackUnit, best_target);
						local_unit_ext->values[ValueAI_PreventTargeting] = 1;
					}
					local_unit->issueQueuedOrderTarget(OrderId::ComputerAI, 0, false);
				}
				else
				{
					local_unit_ext->timers[TimerAI_KitingCooldown] = 8;
					local_unit_ext->timers[TimerAI_KitingLeash] = 0;
					local_unit_ext->values[ValueAI_KitingSignal] = 0;
					local_unit_ext->values[ValueAI_PreventTargeting] = 0;
				}
			});
		}
		else if (unit_can_kite(unit))
		{
			u32 sync = 0;
			if (unit->id != UnitId::ProtossSolarion && unit->id != UnitId::ProtossAccantor)
			{
				local_group.forEach([&](Unit* local_unit)
				{
					UnitExt* local_unit_ext = globals.get_unit_ext(local_unit);
					if (local_unit_ext->values[ValueAI_KitingSignal] >= 256) {
						sync += 1;
					}
				});
			}

			if (sync >= local_group.unitCount * 1
				|| unit->id == UnitId::ProtossSolarion
				|| unit->id == UnitId::ProtossAccantor)
			{
				KitePositions result = get_kite_positions(unit, hostile_group_radius);
				if (!result.enemies_found)
				{			
					local_group.forEach([&](Unit* local_unit)
					{
						UnitExt* local_unit_ext = globals.get_unit_ext(local_unit);
						local_unit_ext->timers[TimerAI_KitingCooldown] = 8;
						local_unit_ext->timers[TimerAI_KitingLeash] = 0;
						local_unit_ext->values[ValueAI_KitingSignal] = 0;
						local_unit_ext->values[ValueAI_PreventTargeting] = 0;
					});
					return;
				}

				u32 crowd_score = 0;
				if (!(unit->status & UnitStatus::InAir))
				{
					crowd_score = cosmonarchyUnitCount(unit, unit->position, CrowdCalcRadius, Flags::Ally | Flags::NotSelf | Flags::NoAir, {});
				}

				local_group.forEach([&](Unit* local_unit)
				{
					UnitExt* local_unit_ext = globals.get_unit_ext(local_unit);

					if (local_unit_ext->values[ValueAI_KitingSignal] < 256
						&& local_unit->id != UnitId::ProtossAccantor
						&& local_unit->id != UnitId::ProtossSolarion)
					{
						return;
					}

					if (!unit_can_kite(local_unit)) {
						return;
					}

					if (unit->id == UnitId::ProtossSolarion || unit->id == UnitId::ProtossAccantor)
					{
						local_unit_ext->timers[TimerAI_KitingCooldown] = 5 * 24;
					}
					else if (unit->movementFlags & MovementFlags::Accelerating || unit->flingyTurnSpeed < 35)
					{
						local_unit_ext->timers[TimerAI_KitingCooldown] = 2;
					}
					else
					{
						local_unit_ext->timers[TimerAI_KitingCooldown] = 6;
					}
					local_unit_ext->timers[TimerAI_KitingLeash] = 0;
					local_unit_ext->values[ValueAI_KitingSignal] = 0;
					local_unit_ext->values[ValueAI_PreventTargeting] = 0;

					bool target_has_min_range = false;
					if (local_unit->orderTarget.unit)
					{
						if (local_unit->orderTarget.unit->id == UnitId::ZergGeszithalor
							|| local_unit->orderTarget.unit->id == UnitId::ZergBactalisk
							|| local_unit->orderTarget.unit->id == UnitId::ProtossArchitect)
						{
							target_has_min_range = true;
						}
						else
						{
							Unit* context = local_unit->orderTarget.unit;
							if (context->subunit)
							{
								context = context->subunit;
							}
							u32 weapon_id = WeaponId::None;
							if (local_unit->status & UnitStatus::InAir)
							{
								weapon_id = context->getAirWeapon();
							}
							else
							{
								weapon_id = context->getGroundWeapon();
							}
							target_has_min_range = weapons_dat::MinRange[weapon_id]
								|| weapons_dat::ExplosionType[weapon_id] == WeaponEffect::SplashRadial;
						}
					}

					u32 local_group_value = 0;
					local_group.forEach([&](Unit* local_unit)
					{
						local_group_value += std::max(local_unit->groundStrength, local_unit->airStrength);
					});

					const f32 confidence_factor = 8;
					const u32 distance = get_distance_to_target(local_unit);
					const u32 max_range = get_max_range(local_unit->id);
					switch (get_kiter_type(local_unit->id))
					{
						case KiterType_Melee:
						{
							if (local_unit->hitPoints < local_unit->get_total_hp_plus_shields() * 0.2)
							{
								local_unit_ext->timers[TimerAI_KitingCooldown] = 24;
								local_unit->orderTo(OrderId::Move, result.backward_pos.x, result.backward_pos.y);
								KITE_DEBUG("%s: fall back", local_unit->getName());
							}
							else if (distance <= max_range * 2)
							{
								local_unit_ext->timers[TimerAI_KitingCooldown] = 24;
								local_unit->orderTo(OrderId::Move, result.forward_pos.x, result.forward_pos.y);
								KITE_DEBUG("%s: forward", local_unit->getName());
							}
							else if (crowd_score > 9 && distance <= max_range * 4)
							{
								local_unit_ext->timers[TimerAI_KitingCooldown] = 24;
								local_unit->orderTo(OrderId::Move, result.strafe_pos.x, result.strafe_pos.y);
								KITE_DEBUG("%s: strafe", local_unit->getName());
							}
							local_unit->issueQueuedOrderTarget(OrderId::ComputerAI, 0, false);
						} break;
						case KiterType_Ranged:
						{
							if ((local_unit->hitPoints < local_unit->get_total_hp_plus_shields() * 0.2
								|| local_group_value * confidence_factor < result.biases[result.forward_index]
								|| distance <= max_range * 0.5)
								&& !target_has_min_range)
							{
								local_unit->orderTo(OrderId::Move, result.backward_pos.x, result.backward_pos.y);
								KITE_DEBUG("%s: fall back", local_unit->getName());
							}
							else if (distance > max_range * 1.5
								|| (distance > max_range * 1 && crowd_score > 9)
								|| (distance > max_range * 0.75 && local_group_value > result.biases[result.forward_index] * confidence_factor)
								|| target_has_min_range)
							{
								local_unit->orderTo(OrderId::Move, result.forward_pos.x, result.forward_pos.y);
								KITE_DEBUG("%s: forward", local_unit->getName());
							}
							else if (crowd_score > 9)
							{
								local_unit->orderTo(OrderId::Move, result.strafe_pos.x, result.strafe_pos.y);
								KITE_DEBUG("%s: strafe", local_unit->getName());
							}
							else if (local_group_value <= result.biases[result.forward_index] * confidence_factor)
							{
								local_unit->orderTo(OrderId::Move, result.backward_pos.x, result.backward_pos.y);
								KITE_DEBUG("%s: backward", local_unit->getName());
							}
							local_unit->issueQueuedOrderTarget(OrderId::ComputerAI, 0, false);
						} break;
						case KiterType_Artillery:
						{
							if (distance > max_range * 1.5)
							{
								local_unit->orderTo(OrderId::Move, result.forward_pos.x, result.forward_pos.y);
								KITE_DEBUG("%s: forward", local_unit->getName());
							}
							else if (distance < max_range * 1.25)
							{
								local_unit->orderTo(OrderId::Move, result.backward_pos.x, result.backward_pos.y);
								KITE_DEBUG("%s: backward", local_unit->getName());
							}
							local_unit->issueQueuedOrderTarget(OrderId::ComputerAI, 0, false);
						} break;
						case KiterType_Flagship:
						{
							if (local_unit->hitPoints < local_unit->get_total_hp_plus_shields() * 0.2
								|| local_group_value * confidence_factor < result.biases[result.forward_index])
							{
								local_unit->orderTo(OrderId::Move, result.backward_pos.x, result.backward_pos.y);
								KITE_DEBUG("%s: fall back", local_unit->getName());
							}
							else if (distance > max_range * 1.5
								|| (distance > max_range * 0.75 && local_group_value > result.biases[result.forward_index] * confidence_factor))
							{
								local_unit->orderTo(OrderId::Move, result.forward_pos.x, result.forward_pos.y);
								KITE_DEBUG("%s: forward", local_unit->getName());
							}
							local_unit->issueQueuedOrderTarget(OrderId::ComputerAI, 0, false);
						} break;
					}
				});
			}
		}
	}
}

static f32 get_blissful_embrace_target_cost(Unit* source, Unit* target) {
	f32 cost = 0;

	// Value attacking units with higher dps
	u32 weapon_id = 0;
	if (source->status & UnitStatus::InAir)
	{
		weapon_id = target->getAirWeapon();
	}
	else
	{
		weapon_id = target->getGroundWeapon();
	}
	f32 multiplier = weapons_dat::InnerSplashRadius[weapon_id] ? 1.5 : 1;
	cost -= hooks::getCurrentDPS(target, weapon_id) * multiplier;

	// Value attacking units with smaller pathing cost more
	s32 dist = (s32)scbw::get_distance_fast(source->position, target->position) - get_max_range(source->id);
	if (dist > 0)
	{
		cost += dist / 32.0;
	}

	// Value attacking strategically important targets
	if (units_dat::GroupFlags[target->id].isFactory)
	{
		cost -= 50;
	}
	if (units_dat::BaseProperty[target->id] & UnitProperty::ResourceDepot)
	{
		cost -= 25;
	}
	if (scbw::unit_id_is_tech(target->id))
	{
		cost -= 25;
	}
	if (target->id == UnitId::ProtossPylon)
	{
		cost -= 25;
	}
	if (target->id == UnitId::TerranBunker && target->hasLoadedUnit())
	{
		cost -= 15;
	}

	return cost;
}

void tactical_ai(Unit* unit, UnitExt* unitExt)
{
	if (unit->id == UnitId::TerranBunker
		&& !unitExt->timers[TimerAI_GenericAbilityCooldown])
	{
		const u32 bunker_capacity = units_dat::SpaceProvided[unit->id];

		UnitArray allies = UnitSearchSquare(unit->position, unit->getSightRange(false) * 32);
		UnitArraySortByDistance(&allies, unit->position);
		UnitArray enemies = allies;
		UnitArrayFilter(&allies, unit, FilterAlly);
		UnitArrayFilter(&enemies, unit, unit->player == 11 ? 0 : FilterEnemy);

		if (enemies.size)
		{
			u32 counter = 0;
			for (u32 i = 0; i < bunker_capacity; ++i)
			{
				Unit* target = unit->getLoadedUnit(i);
				if (target)
				{
					counter += units_dat::SpaceRequired[target->id];
				}
			}
			for (u32 i = 0; i < allies.size && counter <= bunker_capacity; ++i)
			{
				Unit* target = allies.units[i];

				if (target->unit_ai
					&& helper::canEnterTransport(unit, target)
					&& !is_melee_attacker(target->id)
					&& !(units_dat::BaseProperty[unit->id] & UnitProperty::Worker))
				{
					target->orderTo(OrderId::EnterTransport, unit);
					counter += units_dat::SpaceRequired[target->id];
				}
			}
			unitExt->timers[TimerAI_GenericAbilityCooldown] = 2 * 24;
			unitExt->values[ValueAI_BunkerCustom] = 1;
		}
		else if (!unitExt->values[ValueIsNeutralBunker]
			&& unitExt->values[ValueAI_BunkerCustom])
		{
			for (u32 i = 0; i < bunker_capacity; ++i)
			{
				Unit* target = unit->getLoadedUnit(i);
				if (target && target->unit_ai)
				{
					unit->orderTo(OrderId::Unload);
				}
			}
			unitExt->timers[TimerAI_GenericAbilityCooldown] = 1 * 24;
			unitExt->values[ValueAI_BunkerCustom] = 0;
		}
	}

	if (unit->unit_ai && !helper::is_unit_loaded(unit) &&
		playerTable[unit->player].type != PlayerType::Human &&
		playerTable[unit->player].type != PlayerType::HumanDefeated)
	{
		kiting_ai(unit, unitExt);

		if (unit->id == UnitId::TerranApostle)
		{
			if (unit->hasEnergy(techdata_dat::EnergyCost[TechId::ApostleLazarusAgent] * 256) && unitExt->timers[TimerAI_Lazarus] == 0)			
			{
				s32 count = 0;
				cosmonarchyFinderLoop(0, unit->position, 128, Flags::NoBuilding | Flags::Ally, {}, [&](Unit* u)
				{
					UnitExt* uExt = globals.get_unit_ext(u);
					if (uExt->values[ValueLazarusAgent] != 1)
					{
						count++;
					}
				});
				if (count >= 5)
				{
					scbw::set_generic_rust_value(unit, RustId::CastLazarusAgent, 1);
					unitExt->timers[TimerAI_Lazarus] = ApostleAILastBreathTimer;
				}
			}
		}
		else if (unit->id == UnitId::TerranEidolon)
		{
			// mines
		}
		else if (unit->id == UnitId::TerranSavant)
		{
			Unit* target = unit->orderTarget.unit;
			if (target && target->energy > 5 * 256 && unit->energy < 190 * 256) {
				scbw::set_generic_rust_value(unit, RustId::ToggleSavantOnhit, 1);
			} else {
				scbw::set_generic_rust_value(unit, RustId::ToggleSavantOnhit, 0);
			}
		}
		else if (unit->id == UnitId::TerranMadrigal)
		{
			f32 furiaDamagePotential = hooks::getCurrentDPS(unit, WeaponId::MadrigalFuriaMissiles);
			furiaDamagePotential *= cosmonarchyUnitCount(unit, unit->position, 5 * 32, Flags::Enemy, {});
			if (unit->orderTarget.unit)
			{
				UnitExt* target_ext = globals.get_unit_ext(unit->orderTarget.unit);
				furiaDamagePotential += (f32)target_ext->values[ValueDirgeStacks] * MadrigalDirgeDamage;
			}
			f32 dirgeDamagePotential = (weapons_dat::DamageAmount[WeaponId::MadrigalImperiosoMissiles] + MadrigalDirgeDamage) / hooks::getWeaponCooldownFloat(unit, WeaponId::MadrigalImperiosoMissiles);

			if (furiaDamagePotential > dirgeDamagePotential)
			{
				scbw::set_generic_rust_value(unit, RustId::ToggleTempoChange, 0);
			}
			else
			{
				scbw::set_generic_rust_value(unit, RustId::ToggleTempoChange, 1);
			}
		}
		else if (unit->id == UnitId::ZergMatraleth)
		{
			if (unit->mainOrderId != OrderId::CastParasite && unit->energy >= techdata_dat::EnergyCost[TechId::Parasite] * 256 && unitExt->timers[TimerAI_Parasite] == 0)
			{
				cosmonarchyFinderLoop(unit, unit->sprite->position, weapons_dat::MaxRange[WeaponId::Parasite],
					Flags::NotSelf, {}, [&](Unit* u)
				{
					std::vector<std::pair<Unit*, u32>> group;
					s32 supply = 0;
					cosmonarchyFinderLoop(u, u->position, 128, 0, {}, [&](Unit* u2)
					{
						u32 priority = units_dat::SupplyRequired[u->id] * 100;
						if (u->status & UnitStatus::InAir)
						{
							priority /= 4;
						}
						auto weapon = u->getGroundWeapon();
						if (weapon != WeaponId::None)
						{
							if (weapons_dat::MaxRange[weapon] >= 64)
							{
								priority /= 2;
							}
						}
						group.push_back(std::make_pair(u, priority));
						supply += units_dat::SupplyRequired[u->id];
					});
					std::sort(group.begin(), group.end(), [](auto& left, auto& right)
					{
						return left.second > right.second;
					});
					supply /= 2;
					if (supply >= 5)
					{
						unit->orderTo(OrderId::CastParasite, group[0].first);
						unitExt->timers[TimerAI_Parasite] = MatralethAIParasiteTimer;
						return;
					}
				});
			}

			if (unit->hasEnergy(techdata_dat::EnergyCost[TechId::MatralethEnsnaringBrood] * 256) && unitExt->timers[TimerAI_EnsnaringBrood] == 0)
			{
				s32 enemy_count = cosmonarchyUnitCount(0, unit->position, 256, Flags::NoBuilding | Flags::Enemy | Flags::CanAttackOrCast, {});
				if (enemy_count >= 2)
				{
					s32 count = 0;
					cosmonarchyFinderLoop(0, unit->position, 128, Flags::NoBuilding | Flags::Ally | Flags::CanAttackOrCast, {}, [&](Unit* u)
					{
						UnitExt* uExt = globals.get_unit_ext(u);
						if (!uExt->values[TimerEnsnaringBrood])
						{
							count++;
						}
					});
					if (count >= 5)
					{
						scbw::set_generic_rust_value(unit, RustId::CastEnsnaringBrood, 1);
						unitExt->timers[TimerAI_EnsnaringBrood] = MatralethAIEnsnaringBroodTimer;
					}
				}
			}
		}
		else if (unit->id == UnitId::ProtossLanifect)
		{
			u32 web_state = scbw::get_generic_rust_value(unit, RustId::ToggleDisruptionField);
			u32 lanifects = 0;
			u32 webs = 0;
			cosmonarchyFinderLoop(unit, unit->position, AILanifectSeekRange, Flags::Own, {}, [&](Unit* target) {
				if (target->id == UnitId::ProtossLanifect) {
					lanifects += 1;
					if (scbw::get_generic_rust_value(target, RustId::ToggleDisruptionField)) {
						webs += 1;
					}
				}
			});

			u32 allies = cosmonarchyUnitCount(unit, unit->position, AILanifectSeekRange, Flags::IncludingSelf | Flags::Ally, {});
			u32 enemies = cosmonarchyUnitCount(unit, unit->position, AILanifectSeekRange, Flags::Enemy, {});
			u32 threshold = std::min(std::min(ceilf(enemies / 4), ceilf(lanifects / 4)), ceilf(allies / 2));
			if (web_state || webs < threshold)
			{
				Unit* best_target = 0;
				s32 max_score = INT_MIN;
				cosmonarchyFinderLoop(unit, unit->position, AILanifectSeekRange, Flags::Enemy, {}, [&](Unit* target) {
					bool valid = true;
					cosmonarchyFinderLoop(unit, target->position, AILanifectCheckRange / 2, Flags::Own, {}, [&](Unit* dweb) {
						if (scbw::get_generic_rust_value(dweb, RustId::ToggleDisruptionField)) {
							valid = false;
						}
					});
					if (valid) {
						s32 score = cosmonarchyUnitCount(unit, target->position, AILanifectCheckRange, Flags::Enemy, {});
						score -= cosmonarchyUnitCount(unit, target->position, AILanifectCheckRange, Flags::Ally, {});
						score -= scbw::get_distance_fast(unit->position, target->position) / 128;
						if (score > max_score) {
							max_score = score;
							best_target = target;
						}
					}
				});
				if (best_target) {
					unit->orderTo(OrderId::Move, best_target->position.x, best_target->position.y);
					scbw::set_generic_rust_value(unit, RustId::ToggleDisruptionField, 1);
					unitExt->timers[TimerAI_DisruptionField] = AIWebTimer;
					unitExt->timers[TimerAI_KitingCooldown] = AIWebTimer;
				}
			}

			if (web_state && webs > threshold)
			{
				scbw::set_generic_rust_value(unit, RustId::ToggleDisruptionField, 0);
				if (unit->mainOrderId == OrderId::Move)
				{
					unit->orderToIdle();
				}
				unitExt->timers[TimerAI_KitingCooldown] = 0;
			}
		}
		else if (unit->id == UnitId::ProtossStarSovereign && unitExt->timers[TimerAI_GriefOfAllGods] == 0)
		{
			auto enemyUnitsHealth = 0;
			auto alliedUnitsHealth = 0;
			bool alliedTownHalls = false;
			cosmonarchyFinderLoop(unit, unit->sprite->position, AIStarsovereignAbilSeekRange,
				Flags::NotSelf | Flags::NoAir, {}, [&](Unit* u)
			{
				if (!scbw::is_allied_to(unit->player, u->player))
				{
					enemyUnitsHealth += (u->hitPoints + u->shields) / 256;
				}
				if (scbw::is_allied_to(unit->player, u->player))
				{
					if (units_dat::BaseProperty[u->id] & UnitProperty::ResourceDepot)
					{
						alliedTownHalls = true;
						return;
					}
					alliedUnitsHealth += (u->hitPoints + u->shields) / 256;
				}
			});
			if (!alliedTownHalls && enemyUnitsHealth >= 2000
				&& alliedUnitsHealth < 2000
				&& (unit->hitPoints + unit->shields) * 2 >=	(units_dat::MaxHitPoints[unit->id] + units_dat::MaxShieldPoints[unit->id]))
			{
				unitExt->timers[TimerAI_GriefOfAllGods] = 612;
				unitExt->timers[TimerAI_KitingCooldown] = 5 * 24;
				unit->order_computer(OrderId::KhaydarinEclipse);
				unit->orderTarget.pt = unit->sprite->position;
				unit->mainOrderState = 0;
			}
		}
		else if (unit->id == UnitId::ProtossDemiurge && unitExt->timers[TimerAI_GenericAbilityCooldown] == 0)
		{

			if (unit->energy >= 50 * 256)
			{
				auto& vec = unitExt->unitVectors[UnitVectorWorshipersDemiurge];

				u32 totalCount = vec.size();
				u32 inCombatCount = 0;
				for (Unit* worshiper : vec)
				{
					UnitExt* worshiperExt = globals.get_unit_ext(worshiper);
					if (worshiperExt->damageDealtTimer < 4 * 24
						|| worshiperExt->damageReceiveTimer < 4 * 24)
					{
						inCombatCount++;
					}
				}

				if (totalCount > 3 && inCombatCount > totalCount - inCombatCount)
				{
					scbw::set_generic_rust_value(unit, RustId::CastGodsMadeFlesh, 1);
					unitExt->timers[TimerAI_GenericAbilityCooldown] = 10 * 24;
				}
				else
				{
					unitExt->timers[TimerAI_GenericAbilityCooldown] = 1 * 24;
				}
			}
			else
			{
				unitExt->timers[TimerAI_GenericAbilityCooldown] = 1 * 24;
			}
		}
		else if (unit->id == UnitId::ZergAkistrokor && unit->mainOrderId != OrderId::BlissfulEmbrace) {
			UnitArray targets = UnitSearchRadial(unit->position, 512 * 32);
			UnitArrayFilter(&targets, unit, FilterEnemy | FilterBuilding);

			f32 min_cost = FLT_MAX;
			Unit* best_target = 0;
			for (u32 i = 0; i < targets.size; ++i) {
				Unit* target = targets.units[i];
				if (target->hitPoints <= target->getMaxHpRaw() / 2 && target->status & UnitStatus::Completed
					&& units_dat::BaseProperty[target->id] & UnitProperty::Building && !(units_dat::BaseProperty[target->id] & UnitProperty::Addon)) {
					f32 cost = get_blissful_embrace_target_cost(unit, target);
					if (cost < min_cost) {
						min_cost = cost;
						best_target = target;
					}
				}
			}

			if (best_target) {
				unit->orderTo(OrderId::BlissfulEmbrace, best_target);
			}
		}

		/*	
		else if (unit->id == UnitId::TerranPaladin) {
			if (unit->mainOrderId != OrderId::Skyfall && scbw::get_generic_timer(unit,RustId::AI_PaladinTimer)) {
				bool skyfallCondition = false;
				cosmonarchyFinderLoop(unit, unit->sprite->position, weapons_dat::MaxRange[WeaponId::Skyfall],
					CFlags::Enemy, {}, [&](Unit* u) {

					// scbw::print("Loop");
					Unit* tryTarget = u;
					s32 innerCount = 0;
					cosmonarchyFinderLoop(u, u->position, weapons_dat::MaxRange[WeaponId::Skyfall] , CFlags::Ally, {},
						[&](Unit* internalUnit) {
						innerCount++;
					});
					// scbw::printf("Count is %d", innerCount);
					if (innerCount >= 6) {
						Point16 source = unit->position;
						Point16 target = u->position;

						auto distance = scbw::get_distance_fast(source.x, source.y, target.x, target.y);
						if (distance >= 64) {
							f64 fraction = (distance - 64) / distance;
							u16 resultX, resultY;
							resultX = MathUtils::lerp(fraction, source.x, target.x, false);
							resultY = MathUtils::lerp(fraction, source.y, target.y, false);
							unit->orderTo(OrderId::Skyfall, u->position.x, u->position.y);
							scbw::set_generic_timer(unit, RustId::AI_PaladinTimer, 50);
						}
					}
				});
			}
		}
		*/
	}
}
