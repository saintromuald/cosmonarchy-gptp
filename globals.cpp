#include "globals.h"
#include "hook_tools.h"
#include "hooks/cosmonarchy_finder.h"
#include "hooks/limits/attack_extender.h"
#include "hooks/weapons/weapon_damage.h"
#include <algorithm>
#include <unordered_set>
#include <Utils/captivating_claws.h>
Globals globals;

void Globals::refreshDebuggingInfo()
{
	/*	
	unitSpeedDbg = 0;
	weaponDamageDbg = 0;
	gameHookDbg = 0;
	updateUnitStateDbg = 0;
	updateStatusEffectsDbg = 0;
	*/
}

static SaveState saveState;

void Globals::aise_save()
{
	scbw::print("WARNING: Saving doesn't work properly in Cosmonarchy.\n\t\t\tAny plugin specific data will be lost.", GameTextColor::Yellow);
	return;

	output.clear();
	std::ostringstream stream(std::stringstream::out | std::stringstream::binary);
	stream.write(reinterpret_cast<const char*>(&extended_unit_table), sizeof(extended_unit_table));
	stream.write(reinterpret_cast<const char*>(&extended_sfx), sizeof(extended_sfx));
	stream.write(reinterpret_cast<const char*>(&tbl_extender), sizeof(tbl_extender));
	stream.write(reinterpret_cast<const char*>(&unitid_extender), sizeof(unitid_extender));

	u32 effectsSize = effects.size();
	stream.write(reinterpret_cast<const char*>(&effectsSize), sizeof(effectsSize));
	for (s32 i = 0; i < effectsSize; i++)
	{
		Effect* effect = &effects[i];
		stream.write(reinterpret_cast<const char*>(&effect->effectId), sizeof(effect->effectId));
		stream.write(reinterpret_cast<const char*>(&effect->position.x), sizeof(effect->position.x));
		stream.write(reinterpret_cast<const char*>(&effect->position.y), sizeof(effect->position.y));
		stream.write(reinterpret_cast<const char*>(&effect->radius), sizeof(effect->radius));
		stream.write(reinterpret_cast<const char*>(&effect->currentTime), sizeof(effect->currentTime));
		stream.write(reinterpret_cast<const char*>(&effect->maxTime), sizeof(effect->maxTime));;
		stream.write(reinterpret_cast<const char*>(&effect->elevation), sizeof(effect->elevation));
		stream.write(reinterpret_cast<const char*>(&effect->spriteId), sizeof(effect->spriteId));
		stream.write(reinterpret_cast<const char*>(&effect->player), sizeof(effect->player));
		// should find a way on how to serialize thingy, since pointers won't match after save / load
		//stream.write(reinterpret_cast<const char*>(&effect->thingy), sizeof(effect->thingy));
	}

	u32 pariahHitCount = pariahHits.size();
	for (s32 i = 0; i < pariahHitCount; i++)
	{
		stream.write(reinterpret_cast<const char*>(&pariahHits[i].source), sizeof(pariahHits[i].source));
		stream.write(reinterpret_cast<const char*>(&pariahHits[i].target), sizeof(pariahHits[i].target));
	}

	u32 apostleCount = revivingUnits.size();
	for (s32 i = 0; i < apostleCount; i++)
	{
		stream.write(reinterpret_cast<const char*>(&revivingUnits[i].energy), sizeof(revivingUnits[i].energy));
		stream.write(reinterpret_cast<const char*>(&revivingUnits[i].playerId), sizeof(revivingUnits[i].playerId));
		stream.write(reinterpret_cast<const char*>(&revivingUnits[i].position.x), sizeof(revivingUnits[i].position.x));
		stream.write(reinterpret_cast<const char*>(&revivingUnits[i].position.y), sizeof(revivingUnits[i].position.y));
		stream.write(reinterpret_cast<const char*>(&revivingUnits[i].timer), sizeof(revivingUnits[i].timer));
		stream.write(reinterpret_cast<const char*>(&revivingUnits[i].unitId), sizeof(revivingUnits[i].unitId));
	}

	for (s32 i = 0; i < PLAYABLE_PLAYER_COUNT; i++)
	{
		for (s32 j = 0; j < ATTACK_LIMIT; i++)
		{
			stream.write(reinterpret_cast<char*>(&attackers.attacks[i][j]), sizeof(u16));
		}
	}

	u32 multihitCount = multihitBullets.size();
	stream.write(reinterpret_cast<char*>(&multihitCount), sizeof(multihitCount));
	for (s32 i = 0; i < multihitCount; i++)
	{
		MultihitBullet bull = multihitBullets[i];
		stream.write(reinterpret_cast<char*>(&bull.bullet), sizeof(bull.bullet));
		stream.write(reinterpret_cast<char*>(&bull.divider), sizeof(bull.divider));
		u32 alreadyHitCount = bull.already_hit.size();
		stream.write(reinterpret_cast<char*>(&alreadyHitCount), sizeof(alreadyHitCount));
		for (s32 j = 0; j < alreadyHitCount; j++)
		{
			Unit* hit = bull.already_hit[j];
			stream.write(reinterpret_cast<char*>(&hit), sizeof(hit));
		}
	}

	u32 lurkerMapSize = lurkerSpineHits.size();
	for (auto a : lurkerSpineHits)
	{
		Unit* key = a.first;
		stream.write(reinterpret_cast<char*>(&key), sizeof(key));
		u32 valueCount = a.second.size();
		stream.write(reinterpret_cast<char*>(&valueCount), sizeof(valueCount));
		for (s32 b = 0; b < valueCount; b++)
		{
			Unit* value = a.second[b];
			stream.write(reinterpret_cast<char*>(&value), sizeof(value));
		}
	}

	for (u32 i = 0; i < UNIT_ARRAY_LENGTH; ++i)
	{ 
		stream.write(reinterpret_cast<char*>(&unitExt[i].damageReceiveTimer), sizeof(unitExt[i].damageReceiveTimer));
		stream.write(reinterpret_cast<char*>(&unitExt[i].values), sizeof(unitExt[i].values));
		stream.write(reinterpret_cast<char*>(&unitExt[i].timers), sizeof(unitExt[i].timers));
		stream.write(reinterpret_cast<char*>(&unitExt[i].units), sizeof(unitExt[i].units));

		for (auto vec : unitExt[i].valueVectors)
		{
			auto vecSize = vec.size();
			stream.write(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.write(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}

		for (auto vec : unitExt[i].timerVectors)
		{
			auto vecSize = vec.size();
			stream.write(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.write(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}

		for (auto vec : unitExt[i].unitVectors)
		{
			auto vecSize = vec.size();
			stream.write(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.write(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}
	}

	output = stream.rdbuf()->str();
	if (output.length() == 0)
	{
		// return state;
		return;
		// error
	}

	saveState.ptr = output.c_str();
	saveState.len = output.size();
	return;
	// return state;
}

void Globals::aise_load(const char* ptr, u32 len)
{
	this->reset();
	return;

	if (len == 0 || ptr == 0)
	{
		return; // error
	}

	//scbw::print("Load....");
	std::stringstream stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	std::string str(ptr, len);
	stream << str;
	stream.read(reinterpret_cast<char*>(&extended_unit_table), sizeof(extended_unit_table));
	stream.read(reinterpret_cast<char*>(&extended_sfx), sizeof(extended_sfx));
	stream.read(reinterpret_cast<char*>(&tbl_extender), sizeof(tbl_extender));
	stream.read(reinterpret_cast<char*>(&unitid_extender), sizeof(unitid_extender));

	auto impactsSize = 0;
	stream.read(reinterpret_cast<char*>(&impactsSize), sizeof(impactsSize));
	for (s32 i = 0; i < impactsSize; i++)
	{
		Effect effect = {};
		stream.read(reinterpret_cast<char*>(&effect.effectId), sizeof(effect.effectId));
		stream.read(reinterpret_cast<char*>(&effect.position.x), sizeof(effect.position.x));
		stream.read(reinterpret_cast<char*>(&effect.position.y), sizeof(effect.position.y));
		stream.read(reinterpret_cast<char*>(&effect.radius), sizeof(effect.radius));
		stream.read(reinterpret_cast<char*>(&effect.currentTime), sizeof(effect.currentTime));
		stream.read(reinterpret_cast<char*>(&effect.maxTime), sizeof(effect.maxTime));
		stream.read(reinterpret_cast<char*>(&effect.elevation), sizeof(effect.elevation));
		stream.read(reinterpret_cast<char*>(&effect.spriteId), sizeof(effect.spriteId));
		stream.read(reinterpret_cast<char*>(&effect.player), sizeof(effect.player));
		// should find a way on how to serialize thingy, since pointers won't match after save / load
		//stream.read(reinterpret_cast<char*>(&effect.thingy), sizeof(effect.thingy));
		effects.push_back(effect);
	}

	u32 pariahHitsCount = 0;
	stream.read(reinterpret_cast<char*>(&pariahHitsCount), sizeof(pariahHitsCount));
	for (s32 i = 0; i < pariahHitsCount; i++)
	{
		PariahHit hit;
		stream.read(reinterpret_cast<char*>(&hit.source), sizeof(hit.source));
		stream.read(reinterpret_cast<char*>(&hit.target), sizeof(hit.target));
		pariahHits.push_back(hit);
	}

	u32 apostleCount = 0;
	stream.read(reinterpret_cast<char*>(&apostleCount), sizeof(apostleCount));
	for (s32 i = 0; i < pariahHitsCount; i++)
	{
		RevivingUnit agent;
		stream.read(reinterpret_cast<char*>(&agent.energy), sizeof(agent.energy));
		stream.read(reinterpret_cast<char*>(&agent.playerId), sizeof(agent.playerId));
		stream.read(reinterpret_cast<char*>(&agent.position.x), sizeof(agent.position.x));
		stream.read(reinterpret_cast<char*>(&agent.position.y), sizeof(agent.position.y));
		stream.read(reinterpret_cast<char*>(&agent.timer), sizeof(agent.timer));
		stream.read(reinterpret_cast<char*>(&agent.unitId), sizeof(agent.unitId));
		revivingUnits.push_back(agent);
	}

	for (s32 i = 0; i < PLAYABLE_PLAYER_COUNT; i++)
	{
		for (s32 j = 0; j < ATTACK_LIMIT; i++)
		{
			stream.write(reinterpret_cast<char*>(&attackers.attacks[i][j]), sizeof(u16));
		}
	}

	//u32 multihitCount = 0;
	//stream.read(reinterpret_cast<char*>(&multihitCount), sizeof(multihitCount));
	//scbw::printf("Hits: %d", multihitCount);
	/*
	for (s32 i = 0; i < multihitCount; i++) {
		MultihitBullet bull;
		stream.read(reinterpret_cast<char*>(&bull.bullet), sizeof(bull.bullet));
		stream.read(reinterpret_cast<char*>(&bull.divider), sizeof(bull.divider));
		u32 alreadyHitCount = 0;
		stream.read(reinterpret_cast<char*>(&alreadyHitCount), sizeof(alreadyHitCount));
		for (s32 j = 0; j < alreadyHitCount; j++) {
			Unit* hit;
			stream.read(reinterpret_cast<char*>(&hit), sizeof(hit));
			bull.already_hit.push_back(hit);
		}
		multihitBullets.push_back(bull);
	}

	u32 lurkerMapSize = 0;
	stream.read(reinterpret_cast<char*>(&lurkerMapSize), sizeof(lurkerMapSize));
	for (s32 i = 0; i < lurkerMapSize; i++) {
		Unit* key = nullptr;
		stream.read(reinterpret_cast<char*>(&key), sizeof(key));
		u32 valueCount = 0;
		stream.read(reinterpret_cast<char*>(&valueCount), sizeof(valueCount));
		for (s32 j = 0; j < valueCount; j++) {
			Unit* value = nullptr;
			stream.read(reinterpret_cast<char*>(&value), sizeof(value));
			lurkerSpineHits[key].push_back(value);
		}
	}*/

	for (u32 i = 0; i < UNIT_ARRAY_LENGTH; ++i)
	{ 
		stream.read(reinterpret_cast<char*>(&unitExt[i].damageReceiveTimer), sizeof(unitExt[i].damageReceiveTimer));
		stream.read(reinterpret_cast<char*>(&unitExt[i].values), sizeof(unitExt[i].values));
		stream.read(reinterpret_cast<char*>(&unitExt[i].timers), sizeof(unitExt[i].timers));
		stream.read(reinterpret_cast<char*>(&unitExt[i].units), sizeof(unitExt[i].units));

		for (auto vec : unitExt[i].valueVectors)
		{
			auto vecSize = vec.size();
			stream.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.read(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}

		for (auto vec : unitExt[i].timerVectors)
		{
			auto vecSize = vec.size();
			stream.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.read(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}

		for (auto vec : unitExt[i].unitVectors)
		{
			auto vecSize = vec.size();
			stream.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.read(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}
	}
}

static void ClearUnitExt(int index)
{
	globals.unitExt[index] = {};
	// this should set combat timer to highest u32,
	// meaning every unit will start being out of combat
	globals.unitExt[index].damageReceiveTimer = -1;
	globals.unitExt[index].damageDealtTimer = -1;
}

void Globals::reset()
{
	effects.clear();
	pariahHits.clear();
	multihitBullets.clear();
	revivingUnits.clear();
	lurkerSpineHits.clear();
	for (u32 i = 0; i < UNIT_ARRAY_LENGTH; ++i)
	{ 
		ClearUnitExt(i);
	}
}

void Globals::unitDeathHook(Unit* unit)
{
	for (u32 j = 0; j < UNIT_ARRAY_LENGTH; ++j)
	{ // set dead unit pointers to nullptr and erase them from unit vectors
		UnitExt* ext = &unitExt[j];

		for (u32 i = 0; i < UnitCount; ++i)
		{
			if (ext->units[i] == unit)
			{
				ext->units[i] = nullptr;
			}
		}

		for (u32 i = 0; i < UnitVectorCount; ++i)
		{
			auto& vec = ext->unitVectors[i];
			vec.erase(std::remove_if(vec.begin(), vec.end(), [&](Unit* vecUnit)
			{
				return vecUnit == unit;
			}), vec.end());
		}
	}
	ClearUnitExt(unit->getIndex());

	for (s32 i = 0; i < this->effects.size(); i++)
	{
		Effect* effect = &effects[i];
		if (effect->source == unit)
		{
			effect->source = nullptr;
		}
		if (effect->origin == unit)
		{
			effect->origin = nullptr;
		}
	}

	lurkerSpineHits[unit].clear();
	for (auto& a : lurkerSpineHits)
	{
		for (s32 b = 0; b < a.second.size(); b++)
		{
			if (a.second[b] == unit)
			{
				a.second.erase(a.second.begin() + b);
				b--;
			}
		}
	}
}

void ApplyIrradiate(Unit* unit, u8 player, u32 time)
{
	if (time == 0)
	{
		return;
	}
	//if not already irradiated and not burrowed, add the irradiate overlay
	if (unit->irradiateTimer == 0 && !(unit->status & UnitStatus::Burrowed))
	{
		u32 irradiateImageId = ImageId::IrradiateNew_Small + scbw::getOverlayOffset(unit->id);
		Unit* unitWithOverlay;
		if (unit->subunit != nullptr)
		{
			unitWithOverlay = unit->subunit;
		}
		else
		{
			unitWithOverlay = unit;
		}
		unitWithOverlay->sprite->createTopOverlay(irradiateImageId, 0, 0, 0);
	}
	unit->irradiatedBy = nullptr;
	unit->irradiatePlayerId = player;
	unit->irradiateTimer = time;
}

void Globals::createEffect(u8 effectId, u32 time, Vector32 position, u32 radius, s32 spriteId, u8 elevation, u8 player, Unit* source, Unit* parent)
{
	Effect effect = {};
	effect.effectId = effectId;
	effect.position = position;
	effect.radius = radius;
	effect.currentTime = time;
	effect.maxTime = time;
	effect.spriteId = spriteId;
	effect.elevation = elevation;
	effect.player = player;
	effect.thingy = nullptr;
	effect.source = source;
	effect.origin = parent;

	if (effect.spriteId >= 0)
	{
		Thingy* thingy = scbw::createAndInitThingy(effect.spriteId, effect.position.x, effect.position.y, player);
		if (!thingy)
		{
			return;
		}
		effect.thingy = thingy;
	}
	effects.push_back(effect);
}

void Globals::frameHook()
{
	for (u32 j = 0; j < UNIT_ARRAY_LENGTH; ++j)
	{
		UnitExt* ext = &unitExt[j];
		if (ext->damageReceiveTimer != -1) // don't increment past max value of u32
		{
			++ext->damageReceiveTimer;
		}
		if (ext->damageDealtTimer != -1) // don't increment past max value of u32
		{
			++ext->damageDealtTimer;
		}
		for (u32 i = 0; i < TimerCount; ++i)
		{
			if (ext->timers[i] > 0)
			{
				ext->timers[i]--;
			}
		}
		for (u32 i = 0; i < TimerVectorCount; ++i)
		{
			for (auto& timer : ext->timerVectors[i])
			{
				if (timer > 0)
				{
					timer--;
				}
			}
			ext->timerVectors[i].erase(std::remove_if(ext->timerVectors[i].begin(), ext->timerVectors[i].end(), [&](u32& timer)
			{
				return timer == 0;
			}), ext->timerVectors[i].end());
		}
	}

	// Lazarus Agent and Counter-Proposal
	for (s32 i = 0; i < revivingUnits.size(); ++i)
	{
		auto& revivingUnit = revivingUnits[i];
		if (revivingUnit.timer > 0)
		{
			revivingUnit.timer--;
		}
		else
		{
			if (playerTable[revivingUnit.playerId].type != PlayerType::Computer || revivingUnit.town || revivingUnit.ignore_town)
			{
				auto revived = scbw::create_unit_ingame(revivingUnit.unitId, revivingUnit.playerId, revivingUnit.position.x, revivingUnit.position.y, nullptr, false);
				if (revived)
				{
					UnitExt* revivedExt = globals.get_unit_ext(revived);
					revived->hitPoints = 1;
					revived->shields = 0;
					revived->healOverflow(revivingUnit.hp);
					revived->energy = revivingUnit.energy;
					revived->status |= UnitStatus::Invincible | UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack;
					revived->play_iscript_animation(IscriptAnimation::Unused1);
					revivedExt->values[ValueReviveStarted] = 1;
					revivedExt->values[ValueReviveTownPointer] = (u32)revivingUnit.town;
					revivedExt->timers[TimerReviveFlagClearMin] = 3;
					revivedExt->timers[TimerReviveFlagClearMax] = 14;
				}
			}
			revivingUnits.erase(revivingUnits.begin() + i);
			i--;
		}
	}

	for (s32 i = 0; i < effects.size(); i++)
	{
		Effect* effect = &effects[i];
		effect->currentTime--;
		if (effect->origin)
		{
			effect->position = {effect->origin->position.x, effect->origin->position.y};
			if (effect->thingy)
			{
				effect->thingy->sprite->setPosition(effect->origin->position.x, effect->origin->position.y);
			}
		}
		const s32 framePeriod = 6;
		if (effect->currentTime % framePeriod == 1)
		{
			const f64 damageMultiplier = 256.0 * (f64)framePeriod / effect->maxTime;
			switch (effect->effectId)
			{
				case EffectId::StarSovereignTheGriefOfAllGods:
				{
					if (!effect->origin)
					{
						effect->currentTime = 0;
						return;
					}

					scbw::UnitFinder(
						effect->position.x - effect->radius,
						effect->position.y - effect->radius,
						effect->position.x + effect->radius,
						effect->position.y + effect->radius
					).forEachInMatch(cosmonarchyFinderMatch(nullptr, Flags::NoAir, {}), [&](Unit* unit)
					{
						unit->damageWithCustom(2500 * damageMultiplier, DamageType::Chronal, effect->source, effect->player, 0, 1, true);
					});
				} break;
				case EffectId::SalamanderFire:
				{
					UnitArray unit_array = UnitSearchRadial({(u16)effect->position.x, (u16)effect->position.y}, effect->radius);
					UnitArrayFilter(&unit_array, nullptr, FilterNoAir);

					for (u32 j = 0; j < unit_array.size; j++)
					{
						Unit* target = unit_array.units[j];
						target->damageWithCustom(20 * damageMultiplier, DamageType::Concussive, effect->source, effect->player, 0, 1, true);
					}
				} break;
				case EffectId::Criticality:
				{
					scbw::UnitFinder(
						effect->position.x - effect->radius,
						effect->position.y - effect->radius,
						effect->position.x + effect->radius,
						effect->position.y + effect->radius
					).forEachInMatch(cosmonarchyFinderMatch(nullptr, Flags::NoAir, {}), [&](Unit* unit)
					{
						if (scbw::getDistanceFast(effect->position.x, effect->position.y, unit->getX(), unit->getY()) > effect->radius)
						{
							return;
						}
						unit->healUnderflow(2 * 256 / framePeriod, effect->source, true); // this math seems wrong
					});
				} break;
				case EffectId::CausticDischarge:
				{
					scbw::UnitFinder(
						effect->position.x - effect->radius,
						effect->position.y - effect->radius,
						effect->position.x + effect->radius,
						effect->position.y + effect->radius
					).forEachInMatch(cosmonarchyFinderMatch(nullptr, Flags::NoAir, {}), [&](Unit* unit)
					{
						if (scbw::getDistanceFast(effect->position.x, effect->position.y, unit->getX(), unit->getY()) > effect->radius)
						{
							return;
						}
						UnitExt* unitExt = globals.get_unit_ext(unit);
						unit->damageWithCustom(30 * damageMultiplier, DamageType::Impact, effect->source, effect->player, 0, 1, true);
						unitExt->timers[TimerCausticDischargeSpeed] = 5 * 24 + 1;
					});
				} break;
				case EffectId::IonField:
				{
					scbw::UnitFinder(
						effect->position.x - effect->radius,
						effect->position.y - effect->radius,
						effect->position.x + effect->radius,
						effect->position.y + effect->radius
					).forEachInMatch(cosmonarchyFinderMatch(nullptr, Flags::NoAir, {}), [&](Unit* unit)
					{
						if (scbw::getDistanceFast(effect->position.x, effect->position.y, unit->getX(), unit->getY()) > effect->radius)
						{
							return;
						}
						UnitExt* unitExt = globals.get_unit_ext(unit);
						unit->damageWithCustom(100 * damageMultiplier, DamageType::Chronal, effect->source, effect->player, 0, 1, true);
						unitExt->timers[TimerIonFieldSpeed] = 2 * 24 + 1;
					});
				} break;
				case EffectId::Hallucination:
				{
					scbw::UnitFinder(
						effect->position.x - effect->radius,
						effect->position.y - effect->radius,
						effect->position.x + effect->radius,
						effect->position.y + effect->radius
					).forEachInMatch(cosmonarchyFinderMatch(nullptr, 0, {}), [&](Unit* unit)
					{
						if (scbw::getDistanceFast(effect->position.x, effect->position.y, unit->getX(), unit->getY()) > effect->radius)
						{
							return;
						}
						UnitExt* unitExt = globals.get_unit_ext(unit);
						if (!unitExt->timers[TimerHallucinating])
						{
							scbw::applyNewHallucination(unit, effect->player, effect->currentTime);
						}
						else
						{
							unitExt->timers[TimerHallucinating] = effect->currentTime;
						}
					});
				} break;
				case EffectId::Irradiate:
				{
					scbw::UnitFinder(
						effect->position.x - effect->radius,
						effect->position.y - effect->radius,
						effect->position.x + effect->radius,
						effect->position.y + effect->radius
					).forEachInMatch(cosmonarchyFinderMatch(nullptr, 0, {}), [&](Unit* unit)
					{
						if (scbw::getDistanceFast(effect->position.x, effect->position.y, unit->getX(), unit->getY()) > effect->radius)
						{
							return;
						}
						ApplyIrradiate(unit, effect->player, 15);
					});
				} break;
				case EffectId::GroundZero:
				{
					const u32 delta = effect->maxTime - effect->currentTime;
					const u32 stage = delta / framePeriod;

					switch (stage)
					{
						case 0:
						{
							globals.createEffect(EffectId::Irradiate, 20 * 24 - delta, effect->position, 1.5 * 32, SpriteId::IrradiateNew_Large, 5, effect->player, effect->source);
						} break;
						case 1:
						case 2:
						{
							f32 d = stage * 2 * 32;
							u32 count = stage * 8;

							Vector32 newPos = effect->position;
							newPos.x += d;
							for (u32 j = 0; j < count; ++j)
							{
								newPos = scbw::rotate_point(effect->position, newPos, (360.0 / count));
								globals.createEffect(EffectId::Irradiate, 20 * 24 - delta, newPos, 1.5 * 32, SpriteId::IrradiateNew_Large, 5, effect->player, effect->source);
							}
						} break;
					}
				} break;
				case EffectId::DisruptionWeb:
				{
					if (!effect->origin)
					{
						effect->currentTime = 0;
						break;
					}

					UnitExt* parentExt = globals.get_unit_ext(effect->origin);
					if (!parentExt->values[ValueDisruptionFieldOn])
					{
						effect->currentTime = 0;
						break;
					}

					scbw::UnitFinder(
						effect->position.x - 60,
						effect->position.y - 40,
						effect->position.x + 60,
						effect->position.y + 40
					).forEachInMatch(cosmonarchyFinderMatch(nullptr, 0, {}), [&](Unit* unit)
					{
						UnitExt* unitExt = globals.get_unit_ext(unit);
						unitExt->timers[TimerUnderDisruptionWeb] = 8;
						unit->status |= UnitStatus::CanNotAttack;
					});
				} break;
				case EffectId::PlagueField:
				{
					scbw::UnitFinder(
						effect->position.x - 1.5 * 32,
						effect->position.y - 1 * 32,
						effect->position.x + 1.5 * 32,
						effect->position.y + 1 * 32
					).forEach([&](Unit* unit) {
						helper::PlagueProc(unit, 0);
					});
				} break;
			}
		}
		if (effect->currentTime == 0)
		{
			if (effect->thingy && effect->thingy->sprite)
			{
				effect->thingy->sprite->play_iscript_animation(IscriptAnimation::Death);
			}

			effects.erase(effects.begin() + i);
			i--;
		}
	}
}

void Globals::remove_unit_from_pariah_table(Unit* unit, bool sourceOnly)
{
	for (s32 i = 0; i < pariahHits.size(); i++)
	{
		if (pariahHits[i].source == unit || (pariahHits[i].target == unit && !sourceOnly))
		{
			pariahHits.erase(pariahHits.begin() + i);
			i--;
		}
	}
}

void Globals::remove_multihit_bullet(Bullet* bullet)
{
	for (s32 i = 0; i < multihitBullets.size(); i++)
	{
		if (multihitBullets[i].bullet == bullet)
		{
			multihitBullets.erase(multihitBullets.begin() + i);
			i--;
		}
	}
}

bool Globals::was_hit_by_pariah(Unit* source, Unit* target)
{
	for (auto a : pariahHits)
	{
		if (source == a.source && target == a.target)
		{
			return true;
		}
	}
	return false;
}

void Globals::register_pariah_hit(Unit* source, Unit* target)
{
	PariahHit hit;
	hit.source = source;
	hit.target = target;
	pariahHits.push_back(hit);
}

bool Globals::was_already_hit_by_multihit_bullet(Bullet* bullet, Unit* target)
{
	for (auto& a : multihitBullets)
	{
		if (bullet == a.bullet)
		{
			for (auto b : a.already_hit)
			{
				if (b == target)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool Globals::multihit_bullet_is_set(Bullet* bullet)
{
	for (auto& a : multihitBullets)
	{
		if (bullet == a.bullet)
		{
			return true;
		}
	}
	return false;
}

void Globals::add_bullet_multihit(Bullet* bullet, Unit* target)
{
	for (auto& a : multihitBullets)
	{
		if (a.bullet == bullet)
		{
			a.already_hit.push_back(target);
			return;
		}
	}
	MultihitBullet gb(bullet, {target}, 0);
	multihitBullets.push_back(gb);
}

void Globals::add_incremental_bullet_multihit(Bullet* bullet, Unit* target)
{
	for (auto& a : multihitBullets)
	{
		if (a.bullet == bullet)
		{
			a.already_hit.push_back(target);
			return;
		}
	}
	MultihitBullet gb(bullet, {target}, 1);
	multihitBullets.push_back(gb);
}

s32 Globals::get_divider(Bullet* bullet)
{
	for (auto& a : multihitBullets)
	{
		if (a.bullet == bullet && a.divider != 0)
		{
			return a.divider;
		}
	}
	return 1;
}

u32 Globals::get_multihit_count(Bullet* bullet)
{
	for (auto& a : multihitBullets)
	{
		if (a.bullet == bullet)
		{
			return a.already_hit.size();
		}
	}
	return 0;
}

void Globals::add_multihit_empty_bullet(Bullet* bullet, s32 divider)
{
	for (auto& a : multihitBullets)
	{
		if (a.bullet == bullet)
		{
			a.divider = divider;
			return;
		}
	}
	MultihitBullet gb(bullet, {}, divider);
	multihitBullets.push_back(gb);
}

void Globals::initRevive(s32 x, s32 y, u8 player, u32 unitId, u32 energy, u32 hp, AiTown* town)
{
	RevivingUnit revivingUnit = {};
	revivingUnit.playerId = player;
	revivingUnit.position.x = x;
	revivingUnit.position.y = y;
	revivingUnit.timer = 48;
	revivingUnit.unitId = unitId;
	revivingUnit.energy = energy;
	revivingUnit.hp = hp;
	if (units_dat::BaseProperty[unitId] & (UnitProperty::Worker | UnitProperty::Building) || unitId == UnitId::ZergOvileth)
	{
		revivingUnit.town = town;
	}
	else
	{
		revivingUnit.ignore_town = true;
	}
	revivingUnits.push_back(revivingUnit);
}

void Globals::clear_lakizilisk(Unit* lurker)
{
	lurkerSpineHits[lurker].clear();
}

void Globals::add_lurker_spine_pair(Unit* lurker, Unit* target)
{
	lurkerSpineHits[lurker].push_back(target);
}

bool Globals::spine_pair_exists(Unit* lurker, Unit* target)
{
	for (auto a : lurkerSpineHits)
	{
		for (auto b : a.second)
		{
			if (a.first == lurker && b == target)
			{
				return true;
			}
		}
	}
	return false;
}

void __declspec(naked) Globals_SaveStateWrapper()
{
	static SaveState* state;
	__asm {
		PUSHAD
	}
	//	state = &globals.aise_save();
	globals.aise_save();
	state = &saveState;
	__asm {
		POPAD
		MOV EAX, state
		RETN
	}
}

void __declspec(naked) Globals_LoadStateWrapper()
{
	static const char* pointer;
	static u32 len;
	__asm {
		MOV len, ECX
		MOV pointer, EAX
		PUSHAD
	}
	globals.aise_load(pointer, len);
	__asm {
		POPAD
		RETN
	}
}

void injectGlobalsHooks()
{
	jmpPatch(Globals_SaveStateWrapper, 0x004AAF23);
	jmpPatch(Globals_LoadStateWrapper, 0x004AAE94);
	// const u8 retn = 0xc3;
	// memoryPatch(0x004AAF23 + 5, &retn, 1);
	// memoryPatch(0x004AAE94 + 5, &retn, 1);
}

namespace scbw
{
	void refreshDebuffs(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		// NOTE: classic period formula is (([real seconds / 0.042) / 8)
		// e.g. (([15] / 0.042) / 8) = 44.642, rounded up to 45

		// Refresh Ensnare
		if (unit->ensnareTimer)
		{
			unit->ensnareTimer = MatralethEnsnareTimer;
			// specific update following ensnare removal
			unit->updateSpeed();
		}

		// Refresh Plague
		if (unit->plagueTimer)
		{
			unit->plagueTimer = IsthrathalethPlagueTimer;
		}

		// Refresh Observance
		if (unitExt->timers[TimerObservanceDebuff])
		{
			unitExt->timers[TimerObservanceDebuff] = SeraphObservanceDebuffTimer;
		}

		// Refresh Hallucination
		if (unitExt->timers[TimerHallucinating])
		{
			unitExt->timers[TimerHallucinating] = CantavisHallucinationTimer;
		}

		// Refresh Captivating Claws
		if (unitExt->timers[TimerCaptivatingClaws])
		{
			unitExt->timers[TimerCaptivatingClaws] = VorvrokorCaptivatingClawsTimer;
		}

		// Refresh Lockdown
		if (unit->lockdownTimer)
		{
			unit->lockdownTimer = LobotomyMineLockdownTimer;
		}

		// Refresh Maelstrom
		if (unit->maelstromTimer)
		{
			unit->maelstromTimer = MindTyrantMaelstromTimer;
		}

		// Refresh Devourer Corrosive Spores
		if (unitExt->timers[TimerCorrosiveTouch])
		{
			unitExt->timers[TimerCorrosiveTouch] = VithriliskCorrosiveTouchTimer;
		}

		// Refresh Irradiate
		if (unit->irradiateTimer)
		{
			unit->irradiateTimer = SeraphIrradiateTimer;
		}

		// Refresh Dirge
		if (unitExt->timers[TimerDirgeFuse])
		{
			unitExt->timers[TimerDirgeFuse] = MadrigalDirgeTimer;
		}

		// Refresh Hierophant Signify
		if (unitExt->timers[TimerSignify])
		{
			unitExt->timers[TimerSignify] = 3 * 24 + 1;
		}

		//was hardcoded in original code
		scbw::refresh_console();
	}

	void inheritDebuffs(Unit* parent, Unit* child)
	{
		UnitExt* parentExt = globals.get_unit_ext(parent);
		UnitExt* childExt = globals.get_unit_ext(child);

		// NOTE: classic period formula is (([real seconds / 0.042) / 8)
		// e.g. (([15] / 0.042) / 8) = 44.642, rounded up to 45

		// Get overlay info
		s32 overlayOffset = getOverlayOffset(child->id);

		// Inherit Ensnare
		if (parent->ensnareTimer != 0)
		{
			child->ensnareTimer = parent->ensnareTimer;
			if (!(child->sprite->getOverlay(ImageId::EnsnareOverlay_Small + overlayOffset)))
			{
				child->sprite->createOverlay(ImageId::EnsnareOverlay_Small + overlayOffset);
			}
			child->updateSpeed();
		}

		// Inherit Plague
		if (parent->plagueTimer != 0)
		{
			child->plagueTimer = parent->plagueTimer;
			if (!(child->sprite->getOverlay(ImageId::PlagueOverlay_Small + overlayOffset)))
			{
				child->sprite->createOverlay(ImageId::PlagueOverlay_Small + overlayOffset);
			}
		}

		// Inherit Parasite
		if (parentExt->units[UnitParasiteSource] != nullptr)
		{
			childExt->units[UnitParasiteSource] = parentExt->units[UnitParasiteSource];
		}

		// Inherit armor rend (generic)
		if (parentExt->values[ValueArmorReduce] > 0)
		{
			childExt->values[ValueArmorReduce] = parentExt->values[ValueArmorReduce];
		}

		// Inherit Observance
		if (parentExt->timers[TimerObservanceDebuff] > 0)
		{
			childExt->timers[TimerObservanceDebuff] = parentExt->timers[TimerObservanceDebuff];
			childExt->timers[TimerObservancePeriod] = parentExt->timers[TimerObservancePeriod];
			childExt->values[ValueObservanceRend] = parentExt->values[ValueObservanceRend];
		}

		// Inherit Knockout Drivers
		if (parentExt->timers[TimerPetrification] > 0)
		{
			childExt->timers[TimerPetrification] = parentExt->timers[TimerPetrification];
		}

		// Inherit Hallucination
		if (parentExt->timers[TimerHallucinating]  > 0)
		{
			childExt->timers[TimerHallucinating] = parentExt->timers[TimerHallucinating];
		}

		// Inherit Captivating Claws
		if (auto timer = parentExt->timers[TimerCaptivatingClaws])
		{
			childExt->timers[TimerCaptivatingClaws] = timer;
			parentExt->unitVectors[UnitVectorCaptivatingClaws] = childExt->unitVectors[UnitVectorCaptivatingClaws];
		}

		// Inherit Lobotomy Mine slow
		if (parentExt->timers[TimerLobotomySlow])
		{
			childExt->timers[TimerLobotomySlow] = parentExt->timers[TimerLobotomySlow];
		}

		// Inherit Lockdown
		if (parent->lockdownTimer != 0)
		{
			child->lockdownTimer = parent->lockdownTimer;
		}

		// Inherit Maelstrom
		if (parent->maelstromTimer != 0)
		{
			child->maelstromTimer = parent->maelstromTimer;
		}

		// Inherit Devourer Corrosive Spores
		if (parentExt->timers[TimerCorrosiveTouch])
		{
			childExt->values[ValueCorrosiveTouchStacks] = parentExt->values[ValueCorrosiveTouchStacks];
			childExt->timers[TimerCorrosiveTouch] = parentExt->timers[TimerCorrosiveTouch];
		}

		// Inherit Irradiate
		if (parent->irradiateTimer != 0)
		{
			child->irradiateTimer = parent->irradiateTimer;
		}

		// Inherit Defensive Matrix
		if (parent->defensiveMatrixTimer > 0)
		{
			child->defensiveMatrixHp = parent->defensiveMatrixHp;
			child->defensiveMatrixTimer = parent->defensiveMatrixTimer;
		}

		// Inherit Stasis Field
		if (parent->stasisTimer > 0)
		{
			child->stasisTimer = parent->stasisTimer;
		}

		// Inherit Deconstruction
		if (parentExt->values[ValueDeconstruction])
		{
			childExt->values[ValueDeconstruction] = parentExt->values[ValueDeconstruction];
			Sprite* sprite = child->sprite;
			if (child->subunit != nullptr)
			{
				UnitExt* subunitExt = globals.get_unit_ext(child->subunit);
				subunitExt->values[ValueDeconstruction] = parentExt->values[ValueDeconstruction];
				sprite = child->subunit->sprite;
			}
			if (!sprite->getOverlay(ImageId::DeconstructionSmall + overlayOffset))
			{
				sprite->createOverlay(ImageId::DeconstructionSmall + overlayOffset);
			}
		}

		// Inherit Entropy Gauntlets
		if (parentExt->values[ValueEntropyGauntlets])
		{
			childExt->values[ValueEntropyGauntlets] = parentExt->values[ValueEntropyGauntlets];
			if (!(child->sprite->getOverlay(ImageId::EntropyGauntletsOverlaySmall + overlayOffset)))
			{
				child->sprite->createOverlay(ImageId::EntropyGauntletsOverlaySmall + overlayOffset);
			}
		}

		// Inherit Sublime Shepherd
		if (parentExt->values[ValueSublimeShepherd])
		{
			childExt->values[ValueSublimeShepherd] = parentExt->values[ValueSublimeShepherd];
		}

		// Inherit Terminal Surge
		if (parentExt->values[ValueTerminalSurge])
		{
			childExt->values[ValueTerminalSurge] = parentExt->values[ValueTerminalSurge];
		}

		// Inherit Stellar Enforcement
		if (parentExt->values[ValueStellarEnforcement])
		{
			childExt->values[ValueStellarEnforcement] = parentExt->values[ValueStellarEnforcement];
		}

		// Inherit Adrenal Frenzy
		if (parentExt->timers[TimerNathrokorBuff])
		{
			childExt->timers[TimerNathrokorBuff] = parentExt->timers[TimerNathrokorBuff];
			childExt->values[ValueNathrokorBuffStacks] = parentExt->values[ValueNathrokorBuffStacks];
		}

		// Inherit Swarming Omen
		if (parentExt->timers[TimerSwarmingOmen])
		{
			childExt->values[ValueSwarmingOmenStatus] = parentExt->values[ValueSwarmingOmenStatus];
			childExt->timers[TimerSwarmingOmen] = parentExt->timers[TimerSwarmingOmen];
		}

		// Inherit Kaiser Rampage
		if (parentExt->timers[TimerKaiserRampage])
		{
			childExt->timers[TimerKaiserRampage] = parentExt->timers[TimerKaiserRampage];
		}

		// Inherit Kelvin Munitions
		if (parentExt->values[ValueKelvinMunitionsSlow])
		{
			childExt->values[ValueKelvinMunitionsSlow] = parentExt->values[ValueKelvinMunitionsSlow];
		}

		// Inherit Dirge
		if (parentExt->timers[TimerDirgeFuse])
		{
			childExt->timers[TimerDirgeFuse] = parentExt->timers[TimerDirgeFuse];
			childExt->values[ValueDirgeStacks] = parentExt->values[ValueDirgeStacks];
		}

		// Inherit Lazarus Agent
		if (parentExt->values[ValueLazarusAgent])
		{
			childExt->values[ValueLazarusAgent] = parentExt->values[ValueLazarusAgent];
			childExt->values[ValueLazarusAgentApostleSrc] = parentExt->values[ValueLazarusAgentApostleSrc];
		}

		// Inherit Empress Passive Stacks
		if (parentExt->values[ValueEmpressPassiveStacks])
		{
			childExt->values[ValueEmpressPassiveStacks] = parentExt->values[ValueEmpressPassiveStacks];
			childExt->values[ValueEmpressSourcePlayer] = parentExt->values[ValueEmpressSourcePlayer];
		}

		// Inherit Anticthon
		childExt->values[ValueAnticthonStacks] = parentExt->values[ValueAnticthonStacks];

		// Inherit Hierophant Signify
		childExt->timers[TimerSignify] = parentExt->timers[TimerSignify];
		childExt->values[ValueSignifyStacks] = parentExt->values[ValueSignifyStacks];
		childExt->valueVectors[ValueVectorSignifySourcePlayer] = parentExt->valueVectors[ValueVectorSignifySourcePlayer];
		scbw::updateSignifyStatus(child, childExt);

		scbw::refresh_console();
	}

	// TODO: update
	bool isDebuffed(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);
		return unitExt->timers[TimerMaledictionSlow]
			|| unitExt->timers[TimerPetrification]
			|| unitExt->timers[TimerLobotomySlow]
			|| unitExt->timers[TimerObservancePeriod]
			|| unitExt->timers[TimerCorrosiveTouch]
			|| unitExt->values[ValueDeconstruction]
			|| unitExt->values[ValueEntropyGauntlets]
			|| unitExt->values[ValueArmorReduce]
			|| unitExt->values[ValueKelvinMunitionsSlow]
			|| unitExt->values[ValueSignifyStacks]
			|| captivatingClawsGetTimer(unit) > 0
			|| unit->irradiateTimer != 0
			|| unit->ensnareTimer != 0
			|| unit->lockdownTimer != 0
			|| unit->maelstromTimer != 0
			|| unit->plagueTimer != 0
			|| unit->parasiteFlags
			|| unit->isBlind;
	}

	void restoreFunc(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		Unit* overlayTargetUnit = unit;
		if (unit->subunit != nullptr)
		{
			overlayTargetUnit = unit->subunit;
		}
		u32 overlayImageId = ImageId::RestorationHit_Small + scbw::getOverlayOffset(unit->id);;
		(overlayTargetUnit->sprite)->createTopOverlay(overlayImageId, 0, 0, 0);

		// remove status effects and corresponding overlays

		// Remove Parasite
		if (unitExt->timers[TimerParasiteArm])
		{
			unitExt->timers[TimerParasiteArm] = 0;
			unit->removeOverlay(ImageId::ParasiteOverlay);
		}
		unitExt->units[UnitParasiteSource] = nullptr;

		// Remove default flags
		unit->parasiteFlags = 0;
		unit->isBlind = 0;

		// Remove Ensnare
		if (unit->ensnareTimer != 0)
		{
			unit->removeOverlay(ImageId::EnsnareOverlay_Small, ImageId::EnsnareOverlay_Large);
			unit->ensnareTimer = 0;
			//specific update following ensnare removal
			unit->updateSpeed();
		}

		// Remove Plague
		if (unit->plagueTimer != 0)
		{
			unit->removeOverlay(ImageId::PlagueOverlay_Small, ImageId::PlagueOverlay_Large);
			unit->plagueTimer = 0;
		}

		// Remove Irradiate
		if (unit->irradiateTimer != 0)
		{
			unit->removeOverlay(ImageId::IrradiateNew_Small, ImageId::IrradiateNew_Large);
			unit->irradiateTimer = 0;

			//specific updates following irradiate removal
			unit->irradiatedBy = nullptr;
			unit->irradiatePlayerId = 8;
		}

		// Remove Malediction
		if (unitExt->timers[TimerMaledictionSlow])
		{
			unitExt->timers[TimerMaledictionSlow] = 1;
		}

		// Remove Observance
		if (unitExt->timers[TimerObservanceDebuff])
		{
			unitExt->timers[TimerObservanceDebuff] = 1;
		}

		// Remove Knockout Drivers
		if (unitExt->timers[TimerPetrification])
		{
			unitExt->timers[TimerPetrification] = 1;
		}

		// Remove Hallucination
		if (unitExt->timers[TimerHallucinating])
		{
			unitExt->timers[TimerHallucinating] = 1;
		}

		// Remove Captivating Claws
		if (unitExt->timers[TimerCaptivatingClaws])
		{
			unitExt->timers[TimerCaptivatingClaws] = 1;
			unitExt->unitVectors[UnitVectorCaptivatingClaws] = {};
		}

		// Remove Lobotomy Mine slow
		if (unitExt->timers[TimerLobotomySlow])
		{
			unitExt->timers[TimerLobotomySlow] = 1;
		}

		// Remove Entropy Gauntlets
		if (unitExt->values[ValueEntropyGauntlets])
		{
			unitExt->values[ValueEntropyGauntlets] = 0;
			u32 overlayOffset = scbw::getOverlayOffset(unit->id);
			u32 overlayImageId = ImageId::EntropyGauntletsOverlaySmall - overlayOffset;
			if (unit->subunit != nullptr)
			{
				unit->subunit->sprite->removeOverlay(overlayImageId);
			}
			else
			{
				unit->sprite->removeOverlay(overlayImageId);
			}
		}

		// Remove armor rend (generic)
		unitExt->values[ValueArmorReduce] = 0;

		// Remove Controlled Opposition
		unitExt->values[ValueControlledOpposition] = 0;

		// Remove Empress stacks
		unitExt->values[ValueEmpressPassiveStacks] = 0;

		// Remove Lockdown
		unit->removeLockdown();

		// Remove Maelstrom
		unit->removeMaelstrom();

		// Remove Devourer Corrosive Spores
		unit->removeAcidSpores();

		// Remove Deconstruction
		unitExt->values[ValueDeconstruction] = 0;
		unit->removeOverlay(ImageId::DeconstructionSmall, ImageId::DeconstructionLarge);
		if (unit->subunit)
		{
			UnitExt* subunitExt = globals.get_unit_ext(unit->subunit);
			subunitExt->values[ValueDeconstruction] = 0;
			unit->subunit->removeOverlay(ImageId::DeconstructionSmall, ImageId::DeconstructionLarge);
		}

		// Remove Kelvin Munitions
		if (unitExt->values[ValueKelvinMunitionsSlow])
		{
			unitExt->values[ValueKelvinMunitionsSlow] = 0;
		}

		// Remove Dirge
		if (unitExt->timers[TimerDirgeFuse])
		{
			unitExt->timers[TimerDirgeFuse] = 0;
			unitExt->values[ValueDirgeStacks] = 0;
		}

		// Remove Hierophant Signify
		unitExt->timers[TimerSignify] = 0;
		unitExt->values[ValueSignifyStacks] = 0;
		unitExt->valueVectors[ValueVectorSignifySourcePlayer] = {};
		scbw::updateSignifyStatus(unit, unitExt);

		// Remove Nathrokor attack stacks
		for (u32 i = 0; i < 8; i++)
		{
			unitExt->nathrokorTargetStacks[i] = 0;
		}

		scbw::refresh_console();
	}

	void applyParasite(Unit* unit, u8 playerID, Unit* queen)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		if (!scbw::canBeParasited(unit))
		{
			return;
		}
		s32 playerFlag = 1 << playerID;
		if (unit->parasiteFlags != playerFlag)
		{ // <- Prevent restarting arm period
			unitExt->timers[TimerParasiteArm] = DefaultArmParasiteTimer;
			unit->sprite->createOverlay(ImageId::ParasiteOverlay, 0, 0, 0);
		}
		unitExt->units[UnitParasiteSource] = queen; // TODO: remove?
		unit->parasiteFlags = playerFlag; // Parasite now overwrites other player's parasite
		scbw::refresh_console();
	}

	bool isSlowed(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		if (unit->ensnareTimer)
		{
			return true;
		}

		if (unitExt->timers[TimerSignify])
		{
			return true;
		}

		if (unitExt->timers[TimerPetrification])
		{
			return true;
		}

		if (unitExt->timers[TimerCaptivatingClaws])
		{
			return true;
		}

		if (unitExt->values[ValueKelvinMunitionsSlow])
		{
			return true;
		}

		return false;
	}

	Unit** getSourceQueens(Unit* unit) // TODO: remove?
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		Unit* queens[2] = {nullptr};
		if (unit == nullptr)
		{
			return queens; // <- prevents crash from incubators, but does not fix the issue
		}
		queens[0] = unitExt->units[UnitBroodlingMother];
		queens[1] = unitExt->units[UnitParasiteSource];
		return queens;
	}

	void doDeconstructionDamage(Unit* unit, u16 weaponId, bool playSound)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		// Deconstruction - Pazuzu passive
		f64 selfDamage = (unitExt->values[ValueDeconstruction] * 0.05 * (f64)(weapons_dat::DamageAmount[weaponId] * 256.0));
		auto deconstructionTarget = unit;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Subunit)
		{
			deconstructionTarget = unitExt->units[UnitTurretParentLink]; // TODO: port as unit
			if (deconstructionTarget == nullptr)
			{
				return;
			}
		}
		hooks::weaponDamageHook(selfDamage, deconstructionTarget, weaponId, deconstructionTarget, deconstructionTarget->playerId, 0, 1);
		if (playSound)
		{
			helper::playSoundFromSource(1372, unit, 1, 0);
		}
	}

	Bullet* setMissileSourcePos(Unit* unit, u32 weaponId, u32 valueId, s32 rotateCount, u32 overlayType, u32 frameset_offset, u32 fake_amount)
	{
		if (unit->sprite == nullptr || unit->orderTarget.unit == nullptr)
		{
			return nullptr;
		}
		UnitExt* unitExt = globals.get_unit_ext(unit);
		auto rotate = unitExt->values[valueId];
		const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::getImageData(overlayType, unit->sprite->mainGraphic->id));
		u8 frameDirection = unit->sprite->mainGraphic->direction;
		Point8 offset = loFile->getOffset(frameDirection + (17 * frameset_offset), rotate);
		if (unit->sprite->mainGraphic->flags & ImageFlags::Mirrored)
		{
			offset.x = -offset.x;
		}
		rotate++;
		if (rotate > rotateCount - 1)
		{
			rotate = 0;
		}
		unitExt->values[valueId] = rotate;
		auto X = unit->getX() + offset.x;
		auto Y = unit->getY() + offset.y;
		Bullet* bullet = helper::createBullet(weaponId, unit, X, Y, unit->playerId, scbw::getAngle(X, Y, unit->orderTarget.unit->position.x, unit->orderTarget.unit->position.y));
		if (bullet && unitExt->values[valueId] < fake_amount)
		{
			bullet->hitFlags |= 0x1;
		}
		return bullet;
	}

	const u32 Func_RemoveOverlays2 = 0x00492370;
	void removeoverlays2(u32 from, u32 to, Unit* unit)
	{
		__asm {
			PUSHAD
			MOV EAX, from
			MOV EBX, unit
			PUSH to
			CALL Func_RemoveOverlays2
			POPAD
		}
	}

	void clearClarionLinks(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		auto& vec = unitExt->unitVectors[UnitVectorPhaseLink];
		for (Unit* vecUnit : vec)
		{
			UnitExt* vecUnitExt = globals.get_unit_ext(vecUnit);
			auto& clarions = vecUnitExt->unitVectors[UnitVectorPhaseLinkClarions];
			clarions.erase(std::remove_if(clarions.begin(), clarions.end(),
			[&](Unit* target)
			{
				return target == unit;
			}), clarions.end());
		}
		vec.clear();
		unitExt->unitVectors[UnitVectorPhaseLinkClarions].clear();
	}

	void placeClarionLinks(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);
		auto& link = unitExt->unitVectors[UnitVectorPhaseLink];

		clearClarionLinks(unit);
		const s32 radius = 3 * 32;
		scbw::UnitFinder(
			unit->getX() - radius,
			unit->getY() - radius,
			unit->getX() + radius,
			unit->getY() + radius
		).filter([&](Unit* target)
		{
			if (!scbw::isAlliedTo(unit->playerId, target->playerId))
			{
				return false;
			}
			if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
			{
				return false;
			}
			if (!target->shieldsEnabled())
			{
				return false;
			}
			if (target->shields < 256)
			{
				return false;
			}
			if (scbw::getDistanceFast(target->getX(), target->getY(), unit->getX(), unit->getY()) > radius)
			{
				return false;
			}
			return true;
		}).forEach([&](Unit* target)
		{
			auto it = std::find(link.begin(), link.end(), target);
			if (it == link.end()) {
				link.push_back(target);
			}

			UnitExt* targetExt = globals.get_unit_ext(target);
			auto& vec = targetExt->unitVectors[UnitVectorPhaseLinkClarions];
			it = std::find(vec.begin(), vec.end(), unit);
			if (it == vec.end()) {
				vec.push_back(unit);
			}
		});
	}

	bool hasClarionLinks(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);
		return unitExt->unitVectors[UnitVectorPhaseLinkClarions].size();
	}

	void linkDamage(Unit* unit, s32 damage, hooks::WeaponDamageData& data)
	{
		hooks::WeaponDamageData newData = {
			damage,
			damage,
			0,
			data.damageType,
			data.damageDivisor,
			data.weaponId,
			data.direction,
			unit,
			data.attacker,
			data.attackingPlayerId,
			true,
			data.ignoreFlatArmor
		};
		hooks::weaponDamage_root(newData);
	}

	void spreadClarionDamage(hooks::WeaponDamageData& data)
	{
		Unit* unit = data.target;
		auto links = getClarionLinks(unit); // has to be copy
		links.erase(std::remove(links.begin(), links.end(), unit), links.end());
		s32 damage = data.finalDamage;
		if (links.size() == 0 || unit->shields < 256)
		{
			linkDamage(unit, damage, data);
		}
		else
		{
			links.erase(std::remove_if(links.begin(), links.end(), [](Unit* linkUnit)
			{
				return linkUnit->shields < 256;
			}), links.end());
			if (links.size())
			{
				s32 divided = damage / links.size();
				auto it = std::find(links.begin(), links.end(), unit);
				if (it != links.end())
				{
					links.erase(it);
				}
				Unit* damageOriginUnit = data.attacker;
				if (!damageOriginUnit)
				{
					damageOriginUnit = unit;
				}
				s8 oldDirection = data.direction;
				for (Unit* linkUnit : links)
				{
					s32 unitDamage = std::min(divided, linkUnit->shields);
					damage -= unitDamage;
					data.direction = scbw::getAngle(damageOriginUnit->getX(), damageOriginUnit->getY(), linkUnit->getX(), linkUnit->getY());
					linkDamage(linkUnit, unitDamage, data);
				}
				data.direction = oldDirection;
			}
			linkDamage(unit, damage, data);
		}
	}

	std::vector<Unit*> getClarionLinks(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		std::unordered_set<Unit*> links = {};
		auto& clarions = unitExt->unitVectors[UnitVectorPhaseLinkClarions];
		for (Unit* clarion : clarions)
		{
			if (!clarion)
			{
				continue;
			}
			UnitExt* clarionExt = globals.get_unit_ext(clarion);
			auto link = clarionExt->unitVectors[UnitVectorPhaseLink];
			links.insert(link.begin(), link.end());
		}
		return std::vector<Unit*>(links.begin(), links.end());
	}

	void spreadSovrolethDamage(hooks::WeaponDamageData& data)
	{
		Unit* unit = data.target;
		UnitArray sovroleths = scbw::get_writhing_web_sovroleths(unit);
		s32 damage = data.finalDamage / (sovroleths.size + 1);
		for (u32 i = 0; i < sovroleths.size; ++i) {
			linkDamage(sovroleths.units[i], damage, data);
		}
		linkDamage(unit, damage, data);
	}
}