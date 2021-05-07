#include "globals.h"
#include "hook_tools.h"
#include "hooks/hydraFinder.h"
#include "hooks/limits/attack_extender.h"
#include "hooks/weapons/weapon_damage.h"
#include <algorithm>
#include <unordered_set>
#include <Utils/captivating_claws.h>
Globals globals;

void Globals::refreshDebuggingInfo()
{
	/*	unitSpeedDbg = 0;
		weaponDamageDbg = 0;
		gameHookDbg = 0;
		updateUnitStateDbg = 0;
		updateStatusEffectsDbg = 0;*/
}

//SaveState Globals::aise_save() {


void Globals::aise_save()
{
	output.clear();
	std::ostringstream stream(std::stringstream::out | std::stringstream::binary);
	stream.write(reinterpret_cast<const char*>(&extended_unit_table), sizeof(extended_unit_table));
	stream.write(reinterpret_cast<const char*>(&extended_sfx), sizeof(extended_sfx));
	stream.write(reinterpret_cast<const char*>(&tbl_extender), sizeof(tbl_extender));
	stream.write(reinterpret_cast<const char*>(&unitid_extender), sizeof(unitid_extender));

	u32 impactsSize = persistImpacts.size();
	stream.write(reinterpret_cast<const char*>(&impactsSize), sizeof(impactsSize));
	for (s32 i = 0; i < impactsSize; i++)
	{
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].position.x), sizeof(persistImpacts[i].position.x));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].position.y), sizeof(persistImpacts[i].position.y));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].elevation), sizeof(persistImpacts[i].elevation));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].currentTime), sizeof(persistImpacts[i].currentTime));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].currentTime), sizeof(persistImpacts[i].currentTime));

		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].position.x), sizeof(persistImpacts[i].position.x));
		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].position.y), sizeof(persistImpacts[i].position.y));
		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].elevation), sizeof(persistImpacts[i].elevation));
		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].period), sizeof(persistImpacts[i].period));
		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].time), sizeof(persistImpacts[i].time));
		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].impactId), sizeof(persistImpacts[i].impactId));
		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].spriteId), sizeof(persistImpacts[i].spriteId));
		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].radius), sizeof(persistImpacts[i].radius));
	}

	u32 pariahHitCount = pariahHits.size();
	for (s32 i = 0; i < pariahHitCount; i++)
	{
		stream.write(reinterpret_cast<const char*>(&pariahHits[i].source), sizeof(pariahHits[i].source));
		stream.write(reinterpret_cast<const char*>(&pariahHits[i].target), sizeof(pariahHits[i].target));
	}

	u32 apostleCount = apostleAgents.size();
	for (s32 i = 0; i < apostleCount; i++)
	{
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].creator), sizeof(apostleAgents[i].creator));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].energy), sizeof(apostleAgents[i].energy));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].playerId), sizeof(apostleAgents[i].playerId));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].position.x), sizeof(apostleAgents[i].position.x));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].position.y), sizeof(apostleAgents[i].position.y));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].timer), sizeof(apostleAgents[i].timer));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].unitId), sizeof(apostleAgents[i].unitId));
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
			CUnit* hit = bull.already_hit[j];
			stream.write(reinterpret_cast<char*>(&hit), sizeof(hit));
		}
	}

	u32 lurkerMapSize = lurkerSpineHits.size();
	for (auto a : lurkerSpineHits)
	{
		CUnit* key = a.first;
		stream.write(reinterpret_cast<char*>(&key), sizeof(key));
		u32 valueCount = a.second.size();
		stream.write(reinterpret_cast<char*>(&valueCount), sizeof(valueCount));
		for (s32 b = 0; b < valueCount; b++)
		{
			CUnit* value = a.second[b];
			stream.write(reinterpret_cast<char*>(&value), sizeof(value));
		}
	}

	//
	output = stream.rdbuf()->str();
	if (output.length() == 0)
	{
		//		return state;
		return;
		//error
	}

	saveState.ptr = output.c_str();
	saveState.len = output.size();
	return;
	//	return state;
}
void Globals::aise_load(const char* ptr, u32 len)
{
	if (len == 0 || ptr == 0)
	{
		return;//error
	}

	//scbw::print("Load....");
	std::stringstream stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	std::string str(ptr, len);
	stream << str;
	this->reset();
	stream.read(reinterpret_cast<char*>(&extended_unit_table), sizeof(extended_unit_table));
	stream.read(reinterpret_cast<char*>(&extended_sfx), sizeof(extended_sfx));
	stream.read(reinterpret_cast<char*>(&tbl_extender), sizeof(tbl_extender));
	stream.read(reinterpret_cast<char*>(&unitid_extender), sizeof(unitid_extender));

	auto impactsSize = 0;
	stream.read(reinterpret_cast<char*>(&impactsSize), sizeof(impactsSize));
	for (s32 i = 0; i < impactsSize; i++)
	{
		// this code was left here only to keep the proper stream offset, TODO: adjust aise to handle the new struct, or port aise side of this feature (if there still is any)
		PersistImpact impact;
		stream.read(reinterpret_cast<char*>(&impact.position.x), sizeof(impact.position.x));
		stream.read(reinterpret_cast<char*>(&impact.position.y), sizeof(impact.position.y));
		stream.read(reinterpret_cast<char*>(&impact.elevation), sizeof(impact.elevation));
		stream.read(reinterpret_cast<char*>(&impact.currentTime), sizeof(impact.currentTime));
		stream.read(reinterpret_cast<char*>(&impact.currentTime), sizeof(impact.currentTime));

		//stream.read(reinterpret_cast<char*>(&impact.position.x), sizeof(impact.position.x));
		//stream.read(reinterpret_cast<char*>(&impact.position.y), sizeof(impact.position.y));
		//stream.read(reinterpret_cast<char*>(&impact.elevation), sizeof(impact.elevation));
		//stream.read(reinterpret_cast<char*>(&impact.period), sizeof(impact.period));
		//stream.read(reinterpret_cast<char*>(&impact.time), sizeof(impact.time));
		//stream.read(reinterpret_cast<char*>(&impact.impactId), sizeof(impact.impactId));
		//stream.read(reinterpret_cast<char*>(&impact.spriteId), sizeof(impact.spriteId));
		//stream.read(reinterpret_cast<char*>(&impact.radius), sizeof(impact.radius));
		//persistImpacts.push_back(impact);
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
		ApostleAgent agent;
		stream.read(reinterpret_cast<char*>(&agent.creator), sizeof(agent.creator));
		stream.read(reinterpret_cast<char*>(&agent.energy), sizeof(agent.energy));
		stream.read(reinterpret_cast<char*>(&agent.playerId), sizeof(agent.playerId));
		stream.read(reinterpret_cast<char*>(&agent.position.x), sizeof(agent.position.x));
		stream.read(reinterpret_cast<char*>(&agent.position.y), sizeof(agent.position.y));
		stream.read(reinterpret_cast<char*>(&agent.timer), sizeof(agent.timer));
		stream.read(reinterpret_cast<char*>(&agent.unitId), sizeof(agent.unitId));
		apostleAgents.push_back(agent);
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
			CUnit* hit;
			stream.read(reinterpret_cast<char*>(&hit), sizeof(hit));
			bull.already_hit.push_back(hit);
		}
		multihitBullets.push_back(bull);
	}
	u32 lurkerMapSize = 0;
	stream.read(reinterpret_cast<char*>(&lurkerMapSize), sizeof(lurkerMapSize));
	for (s32 i = 0; i < lurkerMapSize; i++) {
		CUnit* key = nullptr;
		stream.read(reinterpret_cast<char*>(&key), sizeof(key));
		u32 valueCount = 0;
		stream.read(reinterpret_cast<char*>(&valueCount), sizeof(valueCount));
		for (s32 j = 0; j < valueCount; j++) {
			CUnit* value = nullptr;
			stream.read(reinterpret_cast<char*>(&value), sizeof(value));
			lurkerSpineHits[key].push_back(value);
		}
	}*/
}

static void ClearUnitExt(int index)
{
	globals.unitExt[index] = {};
	// this should set combat timer to highest u32,
	// meaning every unit will start being out of combat
	globals.unitExt[index].combatTimer = -1;
}

void Globals::reset()
{
	persistImpacts.clear();
	pariahHits.clear();
	multihitBullets.clear();
	apostleAgents.clear();
	lurkerSpineHits.clear();
	for (u32 j = 0; j < UNIT_ARRAY_LENGTH; ++j)
	{ 
		ClearUnitExt(j);
	}
}

void Globals::unitDeathHook(CUnit* unit)
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
			vec.erase(std::remove_if(vec.begin(), vec.end(), [&](CUnit* vecUnit)
			{
				return vecUnit == unit;
			}), vec.end());
		}
	}
	ClearUnitExt(unit->getIndex());

	for (s32 i = 0; i < this->apostleAgents.size(); i++)
	{
		if (apostleAgents[i].creator == unit)
		{
			apostleAgents[i].creator = nullptr;
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

void ApplyIrradiate(CUnit* unit, u8 player, u32 time)
{
	if (time == 0)
	{
		return;
	}
	//if not already irradiated and not burrowed, add the irradiate overlay
	if (unit->irradiateTimer == 0 && !(unit->status & UnitStatus::Burrowed))
	{
		u32 irradiateImageId = ImageId::IrradiateNew_Small + scbw::getOverlayOffset(unit->id);
		CUnit* unitWithOverlay;
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

void Globals::createGenericPersistImpact(u8 impactId, u32 time, Point32 position, u32 radius, s32 spriteId, u8 elevation, u8 player)
{
	PersistImpact impact;
	impact.impactId = impactId;
	impact.position = position;
	impact.radius = radius;
	impact.currentTime = time;
	impact.maxTime = time;
	impact.spriteId = spriteId;
	impact.elevation = elevation;
	impact.player = player;
	impact.thingy = nullptr;

	if (impact.spriteId >= 0)
	{
		CThingy* thingy = scbw::createAndInitThingy(impact.spriteId, impact.position.x, impact.position.y);
		if (thingy == nullptr)
		{
			return;
		}
		impact.thingy = thingy;
	}
	persistImpacts.push_back(impact);
}

void Globals::frameHook()
{
	for (u32 j = 0; j < UNIT_ARRAY_LENGTH; ++j)
	{
		UnitExt* ext = &unitExt[j];
		if (ext->combatTimer != -1) // don't increment past max value of u32
		{
			++ext->combatTimer;
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
				if (timer == 0) return true;
			}), ext->timerVectors[i].end());
		}
	}

	for (s32 i = 0; i < apostleAgents.size(); ++i)
	{
		auto& agent = apostleAgents[i];
		if (agent.timer > 0)
		{
			agent.timer--;
		}
		else
		{
			auto revived = scbw::createUnitIngame(agent.unitId, agent.playerId, agent.position.x, agent.position.y, agent.creator);
			if (revived)
			{
				UnitExt* revivedExt = globals.GetUnitExt(revived);
				revived->setHp(units_dat::MaxHitPoints[revived->id] / 2);
				revived->energy = agent.energy;
				revived->playIscriptAnim(IscriptAnimation::Unused1);
				revivedExt->values[ValueLazarusNoHeal] = 1;
				// TODO: Make reviving units invincible
//				revived->status |= UnitStatus::Invincible;
				revivedExt->values[ValueIsReviving] = 1;
			}
			apostleAgents.erase(apostleAgents.begin() + i);
			i--;
		}
	}

	for (s32 i = 0; i < persistImpacts.size(); i++)
	{
		persistImpacts[i].currentTime--;
		const s32 framePeriod = 6;
		if (persistImpacts[i].currentTime % framePeriod == 1)
		{
			const f64 damageMultiplier = 256.0 * (f64)framePeriod / persistImpacts[i].maxTime;
			switch (persistImpacts[i].impactId)
			{
				case ImpactId::SalamanderFire:
				{
					scbw::UnitFinder(
						persistImpacts[i].position.x - persistImpacts[i].radius,
						persistImpacts[i].position.y - persistImpacts[i].radius,
						persistImpacts[i].position.x + persistImpacts[i].radius,
						persistImpacts[i].position.y + persistImpacts[i].radius
					).forEachInMatch(unitRangeCount(nullptr, CFlags::NoAir, {}), [&](CUnit* unit)
					{
						unit->damageWithIgnoreFlatArmor(WeaponId::SalamanderIncendiaryPayload, weapons_dat::DamageAmount[WeaponId::SalamanderIncendiaryPayload] * damageMultiplier);
					});
					break;
				}
				case ImpactId::GriefOfAllGods:
				{
					scbw::UnitFinder(
						persistImpacts[i].position.x - persistImpacts[i].radius,
						persistImpacts[i].position.y - persistImpacts[i].radius,
						persistImpacts[i].position.x + persistImpacts[i].radius,
						persistImpacts[i].position.y + persistImpacts[i].radius
					).forEachInMatch(unitRangeCount(nullptr, CFlags::NoAir, {}), [&](CUnit* unit)
					{
						unit->damageWithIgnoreFlatArmor(WeaponId::StarSovereignGriefOfAllGods, weapons_dat::DamageAmount[WeaponId::StarSovereignGriefOfAllGods] * damageMultiplier);
					});
					break;
				}
				case ImpactId::Criticality:
				{
					scbw::UnitFinder(
						persistImpacts[i].position.x - persistImpacts[i].radius,
						persistImpacts[i].position.y - persistImpacts[i].radius,
						persistImpacts[i].position.x + persistImpacts[i].radius,
						persistImpacts[i].position.y + persistImpacts[i].radius
					).forEachInMatch(unitRangeCount(nullptr, CFlags::NoAir, {}), [&](CUnit* unit)
					{
						unit->damageWithIgnoreFlatArmor(WeaponId::AspirantCriticality, weapons_dat::DamageAmount[WeaponId::AspirantCriticality] * damageMultiplier);
					});
					break;
				}
				case ImpactId::CausticDischarge:
				{
					scbw::UnitFinder(
						persistImpacts[i].position.x - persistImpacts[i].radius,
						persistImpacts[i].position.y - persistImpacts[i].radius,
						persistImpacts[i].position.x + persistImpacts[i].radius,
						persistImpacts[i].position.y + persistImpacts[i].radius
					).forEachInMatch(unitRangeCount(nullptr, CFlags::NoAir, {}), [&](CUnit* unit)
					{
						UnitExt* unitExt = globals.GetUnitExt(unit);
						unit->damageWithIgnoreFlatArmor(WeaponId::KeskathalorCausticDischarge, weapons_dat::DamageAmount[WeaponId::KeskathalorCausticDischarge] * damageMultiplier);
						unitExt->timers[TimerCausticDischargeSpeed] = 5 * 24 + 1;
					});
					break;
				}
				case ImpactId::IonField:
				{
					scbw::UnitFinder(
						persistImpacts[i].position.x - persistImpacts[i].radius,
						persistImpacts[i].position.y - persistImpacts[i].radius,
						persistImpacts[i].position.x + persistImpacts[i].radius,
						persistImpacts[i].position.y + persistImpacts[i].radius
					).forEachInMatch(unitRangeCount(nullptr, CFlags::NoAir, {}), [&](CUnit* unit)
					{
						UnitExt* unitExt = globals.GetUnitExt(unit);
						unit->damageWithIgnoreFlatArmor(WeaponId::IonField, weapons_dat::DamageAmount[WeaponId::IonField] * damageMultiplier);
						unitExt->timers[TimerIonFieldSpeed] = 2 * 24 + 1;
					});
					break;
				}
				case ImpactId::Hallucination:
				{
					scbw::UnitFinder(
						persistImpacts[i].position.x - persistImpacts[i].radius,
						persistImpacts[i].position.y - persistImpacts[i].radius,
						persistImpacts[i].position.x + persistImpacts[i].radius,
						persistImpacts[i].position.y + persistImpacts[i].radius
					).forEachInMatch(unitRangeCount(nullptr, CFlags::Enemy, {}), [&](CUnit* unit)
					{
						UnitExt* unitExt = globals.GetUnitExt(unit);
						if (unitExt->timers[TimerHallucinating] < persistImpacts[i].currentTime)
						{
							unitExt->values[ValueHallucinatingSourcePlayer] = persistImpacts[i].player;
							unitExt->timers[TimerHallucinating] = persistImpacts[i].currentTime;

							scbw::playSound(SoundId::Protoss_TEMPLAR_PTeHal00_WAV, unit);
							if (unit->subunit != nullptr)
							{
								unit->subunit->sprite->createOverlay(ImageId::HallucinationHit, 0, 0, 0);
							}
							else
							{
								unit->sprite->createOverlay(ImageId::HallucinationHit, 0, 0, 0);
							}
						}
					});
					break;
				}
				case ImpactId::Irradiate:
				{
					scbw::UnitFinder(
						persistImpacts[i].position.x - persistImpacts[i].radius,
						persistImpacts[i].position.y - persistImpacts[i].radius,
						persistImpacts[i].position.x + persistImpacts[i].radius,
						persistImpacts[i].position.y + persistImpacts[i].radius
					).forEachInMatch(unitRangeCount(nullptr, 0, {}), [&](CUnit* unit)
					{
						ApplyIrradiate(unit, persistImpacts[i].player, 15);
					});
					break;
				}
				case ImpactId::GroundZero:
				{
					u32 stage = (persistImpacts[i].maxTime - persistImpacts[i].currentTime) / framePeriod;

					switch (stage)
					{
						case 0:
						{
							globals.createGenericPersistImpact(ImpactId::Irradiate, 30 * 24, persistImpacts[i].position, 1.5 * 32, SpriteId::IrradiateNew_Large, 5, persistImpacts[i].player);
						} break;
						case 1:
						case 2:
						{
							f32 d = stage * 2 * 32;
							u32 count = stage * 8;

							Point32 newPos = persistImpacts[i].position;
							newPos.x += d;
							for (u32 j = 0; j < count; ++j)
							{
								newPos = scbw::rotate_point(persistImpacts[i].position, newPos, (360.0 / count));
								globals.createGenericPersistImpact(ImpactId::Irradiate, 30 * 24, newPos, 1.5 * 32, SpriteId::IrradiateNew_Large, 5, persistImpacts[i].player);
							}
						} break;
					}
				}
			}
		}
		if (persistImpacts[i].currentTime <= 0)
		{
			if (persistImpacts[i].thingy && persistImpacts[i].thingy->sprite)
			{
				persistImpacts[i].thingy->sprite->playIscriptAnim(IscriptAnimation::Death);
			}

			persistImpacts.erase(persistImpacts.begin() + i);
			i--;
		}
	}
}

void Globals::remove_unit_from_pariah_table(CUnit* unit, bool sourceOnly)
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

void Globals::remove_multihit_bullet(CBullet* bullet)
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

bool Globals::was_hit_by_pariah(CUnit* source, CUnit* target)
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

void Globals::register_pariah_hit(CUnit* source, CUnit* target)
{
	PariahHit hit;
	hit.source = source;
	hit.target = target;
	pariahHits.push_back(hit);
}

void Globals::clear_ptr_in_apostle_table(CUnit* ptr)
{
	for (auto& a : apostleAgents)
	{
		if (a.creator == ptr)
		{
			a.creator = nullptr; // agent values themselves must not be deleted to prevent non-working lazarus agent buffs
		}
	}
}

bool Globals::was_already_hit_by_multihit_bullet(CBullet* bullet, CUnit* target)
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

bool Globals::multihit_bullet_is_set(CBullet* bullet)
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

void Globals::add_bullet_multihit(CBullet* bullet, CUnit* target)
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

void Globals::add_incremental_bullet_multihit(CBullet* bullet, CUnit* target)
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

s32 Globals::get_divider(CBullet* bullet)
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

u32 Globals::get_multihit_count(CBullet* bullet)
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

void Globals::add_multihit_empty_bullet(CBullet* bullet, s32 divider)
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

void Globals::initLazarusAgent(s32 x, s32 y, u8 player, u32 unitId, u32 energy, CUnit* unit)
{
	ApostleAgent a;
	a.playerId = player;
	a.position.x = x;
	a.position.y = y;
	a.timer = 48;
	a.unitId = unitId;
	a.energy = energy;
	a.creator = unit;
	apostleAgents.push_back(a);
}

void Globals::clear_lakizilisk(CUnit* lurker)
{
	lurkerSpineHits[lurker].clear();
}

void Globals::add_lurker_spine_pair(CUnit* lurker, CUnit* target)
{
	lurkerSpineHits[lurker].push_back(target);
}

bool Globals::spine_pair_exists(CUnit* lurker, CUnit* target)
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
	void refreshDebuffs(CUnit* unit)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		// NOTE: classic period formula is (([real seconds / 0.042) / 8)
		// e.g. (([15] / 0.042) / 8) = 44.642, rounded up to 45

		// Reset Ensnare
		if (unit->ensnareTimer != 0)
		{
			unit->ensnareTimer = 37;
			//specific update following ensnare removal
			unit->updateSpeed();
		}

		// Refresh Plague
		if (unit->plagueTimer != 0)
		{
			unit->plagueTimer = 30;
		}

		// Reset Malediction
		if (unitExt->timers[TimerMaledictionSlow] > 0)
		{
			unitExt->timers[TimerMaledictionSlow] = 4 * 24;
		}

		// Reset Observance
		if (unitExt->timers[TimerObservanceDebuff] > 0)
		{
			unitExt->timers[TimerObservanceDebuff] = 5 * 24;
		}

		// Reset Knockout Drivers
		if (unitExt->timers[TimerKnockoutDrivers] > 0)
		{
			unitExt->timers[TimerKnockoutDrivers] = 2 * 24;
		}

		// Reset Hallucination
		if (unitExt->timers[TimerHallucinating] > 0)
		{
			unitExt->timers[TimerHallucinating] = 20 * 24;
		}

		// Reset Captivating Claws
		if (unitExt->timers[TimerCaptivatingClaws] > 0)
		{
			unitExt->timers[TimerCaptivatingClaws] = 1 * 24;
		}

		// Reset Lobotomy Mine slow
		if (unitExt->timers[TimerLobotomySlow] > 0)
		{
			unitExt->timers[TimerLobotomySlow] = 4 * 24;
		}

		// Reset Lockdown
		if (unit->lockdownTimer != 0)
		{
			unit->lockdownTimer = 18;
		}

		// Reset Maelstrom
		if (unit->maelstromTimer != 0)
		{
			unit->maelstromTimer = 18;
		}

		// Reset Devourer Corrosive Spores
		if (unitExt->timers[TimerCorrosiveTouch] > 0)
		{
			unitExt->timers[TimerCorrosiveTouch] = VithriliskCorrosiveTouchTimer;
		}

		// Reset Irradiate
		if (unit->irradiateTimer != 0)
		{
			unit->irradiateTimer = 10;
		}

		//was hardcoded in original code
		scbw::refreshConsole();
	}

	void inheritDebuffs(CUnit* parent, CUnit* child)
	{
		UnitExt* parentExt = globals.GetUnitExt(parent);
		UnitExt* childExt = globals.GetUnitExt(child);

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

		// Inherit Malediction
		if (parentExt->timers[TimerMaledictionSlow] > 0)
		{
			childExt->timers[TimerMaledictionSlow] = parentExt->timers[TimerMaledictionSlow];
		}

		// Inherit Observance
		if (parentExt->timers[TimerObservanceDebuff] > 0)
		{
			childExt->timers[TimerObservanceDebuff] = parentExt->timers[TimerObservanceDebuff];
			childExt->timers[TimerObservancePeriod] = parentExt->timers[TimerObservancePeriod];
			childExt->values[ValueObservanceRend] = parentExt->values[ValueObservanceRend];
		}

		// Inherit Knockout Drivers
		if (parentExt->timers[TimerKnockoutDrivers] > 0)
		{
			childExt->timers[TimerKnockoutDrivers] = parentExt->timers[TimerKnockoutDrivers];
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
			auto& parentVec = parentExt->unitVectors[UnitVectorCaptivatingClaws];
			auto& childVec = childExt->unitVectors[UnitVectorCaptivatingClaws];
			childVec = parentVec;
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
			CSprite* sprite = child->sprite;
			if (child->subunit != nullptr)
			{
				UnitExt* subunitExt = globals.GetUnitExt(child->subunit);
				subunitExt->values[ValueDeconstruction] = parentExt->values[ValueDeconstruction];
				sprite = child->subunit->sprite;
			}
			if (!(sprite->getOverlay(ImageId::DeconstructionSmall + overlayOffset)))
			{
				sprite->createOverlay(ImageId::DeconstructionSmall + overlayOffset);
			}
		}

		// Inherit Entropy Gauntlets
		if (parentExt->timers[TimerEntropyGauntlets])
		{
			childExt->timers[TimerEntropyGauntlets] = parentExt->timers[TimerEntropyGauntlets];
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

		// TODO: Implement
		// Inherit Cowardice
		// Inherit Malice

		// Inherit Terminal Surge
		if (parentExt->values[ValueTerminalSurge])
		{
			childExt->values[ValueTerminalSurge] = parentExt->values[ValueTerminalSurge];
		}

		// Inherit Astral Blessing
		if (parentExt->timers[TimerEcclesiast])
		{
			childExt->values[ValueEcclesiastStack] = parentExt->values[ValueEcclesiastStack];
			childExt->values[ValueEcclesiastPlayer] = parentExt->values[ValueEcclesiastPlayer];
			childExt->timers[TimerEcclesiast] = parentExt->timers[TimerEcclesiast];
		}

		// Inherit Stellar Enforcement
		if (parentExt->values[ValueStellarEnforcement])
		{
			childExt->values[ValueStellarEnforcement] = parentExt->values[ValueStellarEnforcement];
		}

		// Inherit Adrenal Frenzy
		if (parentExt->timers[TimerNathrokor])
		{
			childExt->timers[TimerNathrokor] = parentExt->timers[TimerNathrokor];
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

		//was hardcoded in original code
		scbw::refreshConsole();
	}

	bool isDebuffed(CUnit* unit)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);
		return unitExt->timers[TimerMaledictionSlow]
			|| unitExt->timers[TimerKnockoutDrivers]
			|| unitExt->timers[TimerLobotomySlow]
			|| unitExt->timers[TimerObservancePeriod]
			|| unitExt->timers[TimerCorrosiveTouch]
			|| unitExt->timers[TimerEntropyGauntlets]
			|| unitExt->values[ValueArmorReduce]
			|| unitExt->values[ValueKelvinMunitionsSlow]
			|| captivatingClawsGetTimer(unit) > 0
			|| unit->irradiateTimer != 0
			|| unit->ensnareTimer != 0
			|| unit->lockdownTimer != 0
			|| unit->maelstromTimer != 0
			|| unit->plagueTimer != 0
			|| unit->parasiteFlags
			|| unit->isBlind;
		// TODO: cowardice, malice + also add them to status tooltip
	}

	void restoreFunc(CUnit* unit)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		CUnit* overlayTargetUnit = unit;
		if (unit->subunit != nullptr)
		{
			overlayTargetUnit = unit->subunit;
		}
		u32 overlayImageId = ImageId::RestorationHit_Small + scbw::getOverlayOffset(unit->id);;
		(overlayTargetUnit->sprite)->createTopOverlay(overlayImageId, 0, 0, 0);

		//remove status effects and corresponding overlays
		//timers set to 0 twice reflect the original code

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

		// Remove armor rend (generic)
		unitExt->values[ValueArmorReduce] = 0;

		// Remove Entropy Gauntlets
		if (unitExt->timers[TimerEntropyGauntlets])
		{
			unitExt->timers[TimerEntropyGauntlets] = 1;
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
		if (unitExt->timers[TimerKnockoutDrivers])
		{
			unitExt->timers[TimerKnockoutDrivers] = 1;
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
			UnitExt* subunitExt = globals.GetUnitExt(unit->subunit);
			subunitExt->values[ValueDeconstruction] = 0;
			unit->subunit->removeOverlay(ImageId::DeconstructionSmall, ImageId::DeconstructionLarge);
		}

		// Remove Kelvin Munitions
		if (unitExt->values[ValueKelvinMunitionsSlow])
		{
			unitExt->values[ValueKelvinMunitionsSlow] = 0;
		}

		//was hardcoded in original code
		scbw::refreshConsole();
	}

	void applyParasite(CUnit* unit, u8 playerID, CUnit* queen)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

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
		scbw::refreshConsole();
	}

	bool isSlowed(CUnit* unit)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		if (unit->ensnareTimer || unit->unusedTimer)
		{
			return true;
		}
		if (unit->id == UnitId::TerranWyvern && get_generic_rust_value(unit, RustId::ReverseThrust) == 1)
		{
			return true;
		}
		if (unitExt->timers[TimerMaledictionSlow] > 1)
		{
			return true;
		}
		if (unitExt->timers[TimerLobotomySlow] > 1)
		{
			return true;
		}
		if (unitExt->values[ValueKelvinMunitionsSlow])
		{
			return true;
		}
		return false;
	}

	CUnit** getSourceQueens(CUnit* unit) // TODO: remove?
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		CUnit* queens[2] = {nullptr};
		if (unit == nullptr) return queens; // <- prevents crash from incubators, but does not fix the issue
		queens[0] = unitExt->units[UnitBroodlingMother];
		queens[1] = unitExt->units[UnitParasiteSource];
		return queens;
	}

	void doDeconstructionDamage(CUnit* unit, u16 weaponId, bool playSound)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		// Deconstruction - Pazuzu passive
		f64 selfDamage = (0.25 * (f64)(weapons_dat::DamageAmount[weaponId] * 256.0));
		auto deconstructionTarget = unit;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Subunit)
		{
			deconstructionTarget = (CUnit*)unitExt->values[ValueTurretParentLink]; // TODO: port as unit
			if (deconstructionTarget == nullptr)
			{
				return;
			}
		}
		hooks::weaponDamageHook(selfDamage, deconstructionTarget, 0, deconstructionTarget, deconstructionTarget->playerId, deconstructionTarget->playerId, 1);
		if (playSound)
		{
			scbw::playSound(1372, unit);
		}
	}

	CBullet* setMissileSourcePos(CUnit* unit, u32 weaponId, u32 valueId, s32 rotateCount, u32 overlayType, u32 frameset_offset)
	{
		if (unit->sprite == nullptr || unit->orderTarget.unit == nullptr)
		{
			return nullptr;
		}
		UnitExt* unitExt = globals.GetUnitExt(unit);
		auto rotate = unitExt->values[valueId];
		const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::getImageData(overlayType,
			unit->sprite->mainGraphic->id));
		u8 frameDirection = unit->sprite->mainGraphic->direction;
		Point8 offset = loFile->getOffset(frameDirection + (17 * frameset_offset), rotate);
		if (unit->sprite->mainGraphic->flags & CImage_Flags::Mirrored)
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
		return helper::createBullet(weaponId, unit, X, Y, unit->playerId, scbw::getAngle(X, Y,
			unit->orderTarget.unit->position.x, unit->orderTarget.unit->position.y));
	}

	const u32 Func_RemoveOverlays2 = 0x00492370;
	void removeoverlays2(u32 from, u32 to, CUnit* unit)
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

	void clearClarionLinks(CUnit* unit)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		auto& vec = unitExt->unitVectors[UnitVectorPhaseLink];
		for (CUnit* vecUnit : vec)
		{
			auto& clarionVec = unitExt->unitVectors[UnitVectorPhaseLinkClarions];
			auto it = std::find(clarionVec.begin(), clarionVec.end(), unit);
			if (it != clarionVec.end())
			{
				clarionVec.erase(it);
			}
		}
		vec.clear();
	}

	void placeClarionLinks(CUnit* unit)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		bool isClarion = unit->id == UnitId::ProtossClarion;
		auto& link = unitExt->unitVectors[UnitVectorPhaseLink];

		clearClarionLinks(unit);
		const s32 radius = 128;
		scbw::UnitFinder(
			unit->getX() - radius,
			unit->getY() - radius,
			unit->getX() + radius,
			unit->getY() + radius
		).filter([&](CUnit* target)
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
		}).forEach([&](CUnit* target)
		{
			link.push_back(target);
			target->sprite->createOverlay(ImageId::PhaseLinkSmall + scbw::getOverlayOffset(target->id), 0, 0, 0);
			UnitExt* targetExt = globals.GetUnitExt(target);
			targetExt->unitVectors[UnitVectorPhaseLinkClarions].push_back(unit);
		});
	}

	bool hasClarionLinks(CUnit* unit)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);
		return unitExt->unitVectors[UnitVectorPhaseLinkClarions].size();
	}

	void phaseLinkDamage(CUnit* unit, s32 damage, hooks::WeaponDamageData& data)
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
		CUnit* unit = data.target;
		auto links = getClarionLinks(unit); // has to be copy
		s32 damage = data.finalDamage;
		if (links.size() == 0 || unit->shields < 256)
		{
			phaseLinkDamage(unit, damage, data);
		}
		else
		{
			links.erase(std::remove_if(links.begin(), links.end(), [](CUnit* linkUnit)
			{
				return linkUnit->shields < 256;
			}), links.end());
			s32 divided = damage / links.size();
			links.erase(std::find(links.begin(), links.end(), unit));
			for (CUnit* linkUnit : links)
			{
				s32 unitDamage = std::min(divided, linkUnit->shields);
				damage -= unitDamage;
				phaseLinkDamage(linkUnit, unitDamage, data);
			}
			phaseLinkDamage(unit, damage, data);
		}
	}

	std::vector<CUnit*> getClarionLinks(CUnit* unit)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		std::unordered_set<CUnit*> links = {};
		if (unit->id == UnitId::ProtossClarion)
		{
			auto link = unitExt->unitVectors[UnitVectorPhaseLink];
			links.insert(link.begin(), link.end());
		}
		auto& clarions = unitExt->unitVectors[UnitVectorPhaseLinkClarions];
		for (CUnit* clarion : clarions)
		{
			if (!clarion)
			{
				continue;
			}
			UnitExt* clarionExt = globals.GetUnitExt(clarion);
			auto link = clarionExt->unitVectors[UnitVectorPhaseLink];
			links.insert(link.begin(), link.end());
		}
		return std::vector<CUnit*>(links.begin(), links.end());
	}
}