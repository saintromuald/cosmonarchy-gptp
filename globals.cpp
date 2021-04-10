#include "globals.h"
#include "hook_tools.h"
#include "hooks/hydraFinder.h"
#include "hooks/limits/attack_extender.h"
#include "hooks/weapons/weapon_damage.h"
#include <algorithm>
#include <Utils/captivating_claws.h>
Globals globals;

void Globals::refreshDebuggingInfo() {
	/*	unitSpeedDbg = 0;
		weaponDamageDbg = 0;
		gameHookDbg = 0;
		updateUnitStateDbg = 0;
		updateStatusEffectsDbg = 0;*/
}

//SaveState Globals::aise_save() {


void Globals::aise_save() {
	output.clear();
	std::ostringstream stream(std::stringstream::out | std::stringstream::binary);
	stream.write(reinterpret_cast<const char*>(&extended_unit_table), sizeof(extended_unit_table));
	stream.write(reinterpret_cast<const char*>(&extended_sfx), sizeof(extended_sfx));
	stream.write(reinterpret_cast<const char*>(&tbl_extender), sizeof(tbl_extender));
	stream.write(reinterpret_cast<const char*>(&unitid_extender), sizeof(unitid_extender));

	uint32 impactsSize = persistImpacts.size();
	stream.write(reinterpret_cast<const char*>(&impactsSize), sizeof(impactsSize));
	for (int i = 0; i < impactsSize; i++) {
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].position.x), sizeof(persistImpacts[i].position.x));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].position.y), sizeof(persistImpacts[i].position.y));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].elevation), sizeof(persistImpacts[i].elevation));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].time), sizeof(persistImpacts[i].time));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].time), sizeof(persistImpacts[i].time));

		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].position.x), sizeof(persistImpacts[i].position.x));
		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].position.y), sizeof(persistImpacts[i].position.y));
		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].elevation), sizeof(persistImpacts[i].elevation));
		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].period), sizeof(persistImpacts[i].period));
		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].time), sizeof(persistImpacts[i].time));
		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].impactId), sizeof(persistImpacts[i].impactId));
		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].spriteId), sizeof(persistImpacts[i].spriteId));
		//stream.write(reinterpret_cast<const char*>(&persistImpacts[i].mergeRadius), sizeof(persistImpacts[i].mergeRadius));
	}

	uint32 pariahHitCount = pariahHits.size();
	for (int i = 0; i < pariahHitCount; i++) {
		stream.write(reinterpret_cast<const char*>(&pariahHits[i].source), sizeof(pariahHits[i].source));
		stream.write(reinterpret_cast<const char*>(&pariahHits[i].target), sizeof(pariahHits[i].target));
	}

	uint32 apostleCount = apostleAgents.size();
	for (int i = 0; i < apostleCount; i++) {
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].creator), sizeof(apostleAgents[i].creator));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].energy), sizeof(apostleAgents[i].energy));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].playerId), sizeof(apostleAgents[i].playerId));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].position.x), sizeof(apostleAgents[i].position.x));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].position.y), sizeof(apostleAgents[i].position.y));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].timer), sizeof(apostleAgents[i].timer));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].unitId), sizeof(apostleAgents[i].unitId));
	}

	for (int i = 0; i < PLAYABLE_PLAYER_COUNT; i++) {
		for (int j = 0; j < ATTACK_LIMIT; i++) {
			stream.write(reinterpret_cast<char*>(&attackers.attacks[i][j]), sizeof(uint16));
		}
	}

	uint32 multihitCount = multihitBullets.size();
	stream.write(reinterpret_cast<char*>(&multihitCount), sizeof(multihitCount));
	for (int i = 0; i < multihitCount; i++) {
		MultihitBullet bull = multihitBullets[i];
		stream.write(reinterpret_cast<char*>(&bull.bullet), sizeof(bull.bullet));
		stream.write(reinterpret_cast<char*>(&bull.divider), sizeof(bull.divider));
		uint32 alreadyHitCount = bull.already_hit.size();
		stream.write(reinterpret_cast<char*>(&alreadyHitCount), sizeof(alreadyHitCount));
		for (int j = 0; j < alreadyHitCount; j++) {
			CUnit* hit = bull.already_hit[j];
			stream.write(reinterpret_cast<char*>(&hit), sizeof(hit));
		}
	}

	uint32 lurkerMapSize = lurkerSpineHits.size();
	for (auto a : lurkerSpineHits) {
		CUnit* key = a.first;
		stream.write(reinterpret_cast<char*>(&key), sizeof(key));
		uint32 valueCount = a.second.size();
		stream.write(reinterpret_cast<char*>(&valueCount), sizeof(valueCount));
		for (int b = 0; b < valueCount; b++) {
			CUnit* value = a.second[b];
			stream.write(reinterpret_cast<char*>(&value), sizeof(value));
		}
	}

	//
	output = stream.rdbuf()->str();
	if (output.length() == 0) {
		//		return state;
		return;
		//error
	}

	saveState.ptr = output.c_str();
	saveState.len = output.size();
	return;
	//	return state;
}
void Globals::aise_load(const char* ptr, uint32 len) {
	if (len == 0 || ptr == 0) {
		return;//error
	}

	//scbw::printText("Load....");
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
	for (int i = 0; i < impactsSize; i++) {
		// this code was left here only to keep the proper stream offset, TODO: adjust aise to handle the new struct, or port aise side of this feature (if there still is any)
		PersistImpact impact;
		stream.read(reinterpret_cast<char*>(&impact.position.x), sizeof(impact.position.x));
		stream.read(reinterpret_cast<char*>(&impact.position.y), sizeof(impact.position.y));
		stream.read(reinterpret_cast<char*>(&impact.elevation), sizeof(impact.elevation));
		stream.read(reinterpret_cast<char*>(&impact.time), sizeof(impact.time));
		stream.read(reinterpret_cast<char*>(&impact.time), sizeof(impact.time));

		//stream.read(reinterpret_cast<char*>(&impact.position.x), sizeof(impact.position.x));
		//stream.read(reinterpret_cast<char*>(&impact.position.y), sizeof(impact.position.y));
		//stream.read(reinterpret_cast<char*>(&impact.elevation), sizeof(impact.elevation));
		//stream.read(reinterpret_cast<char*>(&impact.period), sizeof(impact.period));
		//stream.read(reinterpret_cast<char*>(&impact.time), sizeof(impact.time));
		//stream.read(reinterpret_cast<char*>(&impact.impactId), sizeof(impact.impactId));
		//stream.read(reinterpret_cast<char*>(&impact.spriteId), sizeof(impact.spriteId));
		//stream.read(reinterpret_cast<char*>(&impact.mergeRadius), sizeof(impact.mergeRadius));
		//persistImpacts.push_back(impact);
	}

	uint32 pariahHitsCount = 0;
	stream.read(reinterpret_cast<char*>(&pariahHitsCount), sizeof(pariahHitsCount));
	for (int i = 0; i < pariahHitsCount; i++) {
		PariahHit hit;
		stream.read(reinterpret_cast<char*>(&hit.source), sizeof(hit.source));
		stream.read(reinterpret_cast<char*>(&hit.target), sizeof(hit.target));
		pariahHits.push_back(hit);
	}

	uint32 apostleCount = 0;
	stream.read(reinterpret_cast<char*>(&apostleCount), sizeof(apostleCount));
	for (int i = 0; i < pariahHitsCount; i++) {
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

	for (int i = 0; i < PLAYABLE_PLAYER_COUNT; i++) {
		for (int j = 0; j < ATTACK_LIMIT; i++) {
			stream.write(reinterpret_cast<char*>(&attackers.attacks[i][j]), sizeof(uint16));
		}
	}

	//uint32 multihitCount = 0;
	//stream.read(reinterpret_cast<char*>(&multihitCount), sizeof(multihitCount));
	//scbw::printFormattedText("Hits: %d", multihitCount);
	/*
	for (int i = 0; i < multihitCount; i++) {
		MultihitBullet bull;
		stream.read(reinterpret_cast<char*>(&bull.bullet), sizeof(bull.bullet));
		stream.read(reinterpret_cast<char*>(&bull.divider), sizeof(bull.divider));
		uint32 alreadyHitCount = 0;
		stream.read(reinterpret_cast<char*>(&alreadyHitCount), sizeof(alreadyHitCount));
		for (int j = 0; j < alreadyHitCount; j++) {
			CUnit* hit;
			stream.read(reinterpret_cast<char*>(&hit), sizeof(hit));
			bull.already_hit.push_back(hit);
		}
		multihitBullets.push_back(bull);
	}
	uint32 lurkerMapSize = 0;
	stream.read(reinterpret_cast<char*>(&lurkerMapSize), sizeof(lurkerMapSize));
	for (int i = 0; i < lurkerMapSize; i++) {
		CUnit* key = nullptr;
		stream.read(reinterpret_cast<char*>(&key), sizeof(key));
		uint32 valueCount = 0;
		stream.read(reinterpret_cast<char*>(&valueCount), sizeof(valueCount));
		for (int j = 0; j < valueCount; j++) {
			CUnit* value = nullptr;
			stream.read(reinterpret_cast<char*>(&value), sizeof(value));
			lurkerSpineHits[key].push_back(value);
		}
	}*/
}

void Globals::reset() {
	persistImpacts.clear();
	pariahHits.clear();
	multihitBullets.clear();
	apostleAgents.clear();
	lurkerSpineHits.clear();
	unitStructs.clear();
}

void Globals::unitFrameHook(CUnit* unit) {
	if (unit->id == UnitId::ProtossPariah && unit->orderSignal & 0x20) {
		unit->orderSignal &= ~0x20;
		remove_unit_from_pariah_table(unit, true);
	}
}

void Globals::unitDeathHook(CUnit* unit) {
	unitStructs.erase(unit);
	for (auto& a : unitStructs) { // set dead unit pointers to nullptr and erase them from unit vectors
		for (auto& unitPair : a.second.genericUnits) {
			if (unitPair.second == unit) {
				unitPair.second = nullptr;
			}
		}

		for (auto& unitVecPair : a.second.genericUnitVectors) {
			auto& vec = unitVecPair.second;
			vec.erase(std::remove_if(vec.begin(), vec.end(), [&](CUnit* vecUnit) {
				return vecUnit == unit;
			}), vec.end());
		}

		for (auto& unitMapPair : a.second.genericUnitTimerMap) {
			auto& map = unitMapPair.second;
			map.erase(unit);
		}
	}

	for (int i = 0; i < this->apostleAgents.size(); i++) {
		if (apostleAgents[i].creator == unit) {
			apostleAgents[i].creator = nullptr;
		}
	}

	lurkerSpineHits[unit].clear();
	for (auto& a : lurkerSpineHits) {
		for (int b = 0; b < a.second.size(); b++) {
			if (a.second[b] == unit) {
				a.second.erase(a.second.begin() + b);
				b--;
			}
		}
	}
}

void Globals::frameHook() {
	for (auto& unitStruct : unitStructs) {
		for (auto& timerPair : unitStruct.second.genericTimers) {
			if (timerPair.second > 0) {
				timerPair.second--;
			}
		}
		for (auto& timerVecPair : unitStruct.second.genericTimerVectors) {
			for (auto& timer : timerVecPair.second) {
				if (timer > 0) {
					timer--;
				}
			}
			timerVecPair.second.erase(std::remove_if(timerVecPair.second.begin(), timerVecPair.second.end(), [&](int& timer) {
				if (timer == 0) return true;
			}), timerVecPair.second.end());
		}
		for (auto& timerVecPair : unitStruct.second.genericUnitTimerMap) {
			for (auto& timer : timerVecPair.second) {
				if (timer.second > 0) {
					timer.second--;
				} else {
					timerVecPair.second.erase(timer.first);
				}
			}
		}
	}

	for (int i = 0; i < apostleAgents.size(); i++) {
		auto& agent = apostleAgents[i];
		if (agent.timer > 0) {
			agent.timer--;
		} else {
			// scbw::printFormattedText("Agent Creator: 0x%X", agent.creator);
			auto revived = scbw::createUnitIngame(agent.unitId, agent.playerId, agent.position.x, agent.position.y, agent.creator);
			if (revived) {
				revived->setHp(units_dat::MaxHitPoints[revived->id] / 2);
				revived->energy = agent.energy;
				revived->playIscriptAnim(IscriptAnimation::Unused1);
				scbw::set_generic_value(revived, ValueId::LazarusNoHeal, 1);
				// revived->status |= UnitStatus::Invincible;
				scbw::set_generic_value(revived, ValueId::IsReviving, 1);
			}
			apostleAgents.erase(apostleAgents.begin() + i);
			i--;
		}
	}

	for (int i = 0; i < persistImpacts.size(); i++) {
		auto& impact = persistImpacts[i];
		impact.time--;
		const int framePeriod = 6;
		if (impact.time % framePeriod == 1) {
			const double damageMultiplier = 256.0 * (double)framePeriod / impact.internalTimeMax;
			switch (impact.impactId) {
				case ImpactId::SalamanderFire:
				{
					const int radius = 0.5 * 32;
					scbw::UnitFinder(
						impact.position.x - radius,
						impact.position.y - radius,
						impact.position.x + radius,
						impact.position.y + radius
					).forEachInMatch(unitRangeCount(nullptr, CFlags::Enemy | CFlags::NoAir, {}), [&](CUnit* unit) {
						unit->damageWithIgnoreFlatArmor(WeaponId::SalamanderIncendiaryPayload, weapons_dat::DamageAmount[WeaponId::SalamanderIncendiaryPayload] * damageMultiplier);
					});
					break;
				}
				case ImpactId::GriefOfAllGods:
				{
					const int radius = 8 * 32;
					scbw::UnitFinder(
						impact.position.x - radius,
						impact.position.y - radius,
						impact.position.x + radius,
						impact.position.y + radius
					).forEachInMatch(unitRangeCount(nullptr, CFlags::NoAir, {}), [&](CUnit* unit) {
						unit->damageWithIgnoreFlatArmor(WeaponId::StarSovereignGriefOfAllGods, weapons_dat::DamageAmount[WeaponId::StarSovereignGriefOfAllGods] * damageMultiplier);
					});
					break;
				}
				case ImpactId::Criticality:
				{
					const int radius = 1.25 * 32;
					scbw::UnitFinder(
						impact.position.x - radius,
						impact.position.y - radius,
						impact.position.x + radius,
						impact.position.y + radius
					).forEachInMatch(unitRangeCount(nullptr, CFlags::NoAir, {}), [&](CUnit* unit) {
						unit->damageWithIgnoreFlatArmor(WeaponId::AspirantCriticality, weapons_dat::DamageAmount[WeaponId::AspirantCriticality] * damageMultiplier);
					});
					break;
				}
				case ImpactId::CausticDischarge:
				{
					const int radius = 1.5 * 32;
					scbw::UnitFinder(
						impact.position.x - radius,
						impact.position.y - radius,
						impact.position.x + radius,
						impact.position.y + radius
					).forEachInMatch(unitRangeCount(nullptr, CFlags::NoAir, {}), [&](CUnit* unit) {
						unit->damageWithIgnoreFlatArmor(WeaponId::KeskathalorCausticDischarge, weapons_dat::DamageAmount[WeaponId::KeskathalorCausticDischarge] * damageMultiplier);
						scbw::set_generic_timer(unit, ValueId::CausticDischargeSpeed, 5 * 24 + 1);
					});
					break;
				}
				case ImpactId::IonField:
				{
					const int radius = 3 * 32;
					scbw::UnitFinder(
						impact.position.x - radius,
						impact.position.y - radius,
						impact.position.x + radius,
						impact.position.y + radius
					).forEachInMatch(unitRangeCount(nullptr, CFlags::NoAir, {}), [&](CUnit* unit) {
						unit->damageWithIgnoreFlatArmor(WeaponId::IonField, weapons_dat::DamageAmount[WeaponId::IonField] * damageMultiplier);
						scbw::set_generic_timer(unit, ValueId::IonFieldSpeed, 2 * 24 + 1);
					});
					break;
				}
			}
		}
		if (impact.time <= 0) {
			persistImpacts.erase(persistImpacts.begin() + i);
			i--;
		}
	}
}

void Globals::remove_unit_from_pariah_table(CUnit* unit, bool sourceOnly) {
	for (int i = 0; i < pariahHits.size(); i++) {
		if (pariahHits[i].source == unit || (pariahHits[i].target == unit && !sourceOnly)) {
			pariahHits.erase(pariahHits.begin() + i);
			i--;
		}
	}
}

void Globals::remove_multihit_bullet(CBullet* bullet) {
	for (int i = 0; i < multihitBullets.size(); i++) {
		if (multihitBullets[i].bullet == bullet) {
			multihitBullets.erase(multihitBullets.begin() + i);
			i--;
		}
	}
}

bool Globals::was_hit_by_pariah(CUnit* source, CUnit* target) {
	for (auto a : pariahHits) {
		if (source == a.source && target == a.target) {
			return true;
		}
	}
	return false;
}

void Globals::register_pariah_hit(CUnit* source, CUnit* target) {
	PariahHit hit;
	hit.source = source;
	hit.target = target;
	pariahHits.push_back(hit);
}

void Globals::clear_ptr_in_apostle_table(CUnit* ptr) {
	for (auto& a : apostleAgents) {
		if (a.creator == ptr) {
			a.creator = nullptr; // agent values themselves must not be deleted to prevent non-working lazarus agent buffs
		}
	}
}

bool Globals::was_already_hit_by_multihit_bullet(CBullet* bullet, CUnit* target) {
	for (auto& a : multihitBullets) {
		if (bullet == a.bullet) {
			for (auto b : a.already_hit) {
				if (b == target) {
					return true;
				}
			}
		}
	}
	return false;
}

bool Globals::multihit_bullet_is_set(CBullet* bullet) {
	for (auto& a : multihitBullets) {
		if (bullet == a.bullet) {
			return true;
		}
	}
	return false;
}

void Globals::add_bullet_multihit(CBullet* bullet, CUnit* target) {
	for (auto& a : multihitBullets) {
		if (a.bullet == bullet) {
			a.already_hit.push_back(target);
			return;
		}
	}
	MultihitBullet gb(bullet, {target}, 0);
	multihitBullets.push_back(gb);
}

void Globals::add_incremental_bullet_multihit(CBullet* bullet, CUnit* target) {
	for (auto& a : multihitBullets) {
		if (a.bullet == bullet) {
			a.already_hit.push_back(target);
			return;
		}
	}
	MultihitBullet gb(bullet, {target}, 1);
	multihitBullets.push_back(gb);
}

int Globals::get_divider(CBullet* bullet) {
	for (auto& a : multihitBullets) {
		if (a.bullet == bullet && a.divider != 0) {
			return a.divider;
		}
	}
	return 1;
}

uint32 Globals::get_multihit_count(CBullet* bullet) {
	for (auto& a : multihitBullets) {
		if (a.bullet == bullet) {
			return a.already_hit.size();
		}
	}
	return 0;
}

void Globals::add_multihit_empty_bullet(CBullet* bullet, int divider) {
	for (auto& a : multihitBullets) {
		if (a.bullet == bullet) {
			a.divider = divider;
			return;
		}
	}
	MultihitBullet gb(bullet, {}, divider);
	multihitBullets.push_back(gb);
}

void Globals::initLazarusAgent(int x, int y, uint8 player, uint32 unitId, uint32 energy, CUnit* unit) {
	ApostleAgent a;
	a.playerId = player;
	a.position.x = x;
	a.position.y = y;
	a.timer = 48;
	a.unitId = unitId;
	a.energy = energy;
	a.creator = unit;
	//	scbw::printFormattedText("Set creator to 0x%X", i.creator);
	//	scbw::printFormattedText("Try push back %d", apostleAgents.size());
	apostleAgents.push_back(a);
}

void Globals::createGenericPersistImpact(uint8 impactId, Point32 position, uint32 mergeRadius, uint32 time, int32 spriteId, uint8 elevation) {
	PersistImpact impact;
	impact.impactId = impactId;
	impact.position = position;
	impact.mergeRadius = mergeRadius;
	impact.time = time;
	impact.internalTimeMax = time;
	impact.spriteId = spriteId;
	impact.elevation = elevation;

	if (impact.mergeRadius != 0) {
		for (auto& other : persistImpacts) {
			if (other.impactId == impact.impactId && scbw::getDistanceFast(impact.position.x, impact.position.y, other.position.x, other.position.y) < impact.mergeRadius) {
				other.time = impact.time;
				return;
			}
		}
	}

	if (impact.spriteId >= 0) {
		CThingy* thingy = scbw::createThingy(impact.spriteId, impact.position.x, impact.position.y, 11);
		if (thingy == nullptr) {
			return;
		}
		thingy->sprite->elevationLevel = impact.elevation;
		scbw::setThingyVisibilityFlags(thingy);
	}
	persistImpacts.push_back(impact);
}

void Globals::clear_lakizilisk(CUnit* lurker) {
	lurkerSpineHits[lurker].clear();
}

void Globals::add_lurker_spine_pair(CUnit* lurker, CUnit* target) {
	lurkerSpineHits[lurker].push_back(target);
}

bool Globals::spine_pair_exists(CUnit* lurker, CUnit* target) {
	for (auto a : lurkerSpineHits) {
		for (auto b : a.second) {
			if (a.first == lurker && b == target) {
				return true;
			}
		}
	}
	return false;
}

void __declspec(naked) Globals_SaveStateWrapper() {
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

void __declspec(naked) Globals_LoadStateWrapper() {
	static const char* pointer;
	static uint32 len;
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

void injectGlobalsHooks() {
	jmpPatch(Globals_SaveStateWrapper, 0x004AAF23);
	jmpPatch(Globals_LoadStateWrapper, 0x004AAE94);
	// const uint8 retn = 0xc3;
	// memoryPatch(0x004AAF23 + 5, &retn, 1);
	// memoryPatch(0x004AAE94 + 5, &retn, 1);
}

namespace scbw {
	uint32 get_generic_value(CUnit* u, uint32 id) {
		auto find = globals.unitStructs.find(u);
		if (find != globals.unitStructs.end()) {
			return find->second.getGenericValue(id);
		}
		return 0;
	}

	void set_generic_value(CUnit* u, uint32 id, uint32 val) {
		globals.unitStructs[u].setGenericValue(id, val);
	}

	void add_generic_value(CUnit* u, uint32 id, int val) {
		globals.unitStructs[u].addGenericValue(id, val);
	}

	uint32 get_generic_timer(CUnit* u, uint32 id) {
		auto find = globals.unitStructs.find(u);
		if (find != globals.unitStructs.end()) {
			return find->second.getGenericTimer(id);
		}
		return 0;
	}

	void set_generic_timer(CUnit* u, uint32 id, uint32 val) {
		globals.unitStructs[u].setGenericTimer(id, val);
	}

	CUnit* get_generic_unit(CUnit* u, uint32 id) {
		auto find = globals.unitStructs.find(u);
		if (find != globals.unitStructs.end()) {
			return find->second.getGenericUnit(id);
		}
		return nullptr;
	}

	void set_generic_unit(CUnit* u, uint32 id, CUnit* val) {
		globals.unitStructs[u].setGenericUnit(id, val);
	}

	std::vector<int>& get_generic_value_vector(CUnit* u, uint32 id) {
		auto& find = globals.unitStructs.find(u);
		if (find == globals.unitStructs.end()) {
			globals.unitStructs[u] = {};
		}
		return globals.unitStructs[u].getGenericValueVector(id);
	}

	std::vector<int>& get_generic_timer_vector(CUnit* u, uint32 id) {
		auto& find = globals.unitStructs.find(u);
		if (find == globals.unitStructs.end()) {
			globals.unitStructs[u] = {};
		}
		return globals.unitStructs[u].getGenericTimerVector(id);
	}

	std::vector<CUnit*>& get_generic_unit_vector(CUnit* u, uint32 id) {
		auto& find = globals.unitStructs.find(u);
		if (find == globals.unitStructs.end()) {
			globals.unitStructs[u] = {};
		}
		return globals.unitStructs[u].getGenericUnitVector(id);
	}

	std::unordered_map<CUnit*, int>& get_generic_unit_timer_map(CUnit* u, uint32 id) {
		auto& find = globals.unitStructs.find(u);
		if (find == globals.unitStructs.end()) {
			globals.unitStructs[u] = {};
		}
		return globals.unitStructs[u].getGenericUnitTimerMap(id);
	}

	void refreshDebuffs(CUnit* unit) {
		// NOTE: classic period formula is (([real seconds / 0.042) / 8)
		// e.g. (([15] / 0.042) / 8) = 44.642, rounded up to 45

		// Reset Ensnare
		if (unit->ensnareTimer != 0) {
			unit->ensnareTimer = 37;
			//specific update following ensnare removal
			unit->updateSpeed();
		}

		// Refresh Plague
		if (unit->plagueTimer != 0) {
			unit->plagueTimer = 30;
		}

		// Reset Malediction
		if (scbw::get_generic_timer(unit, ValueId::MaledictionSlow) > 0) {
			scbw::set_generic_timer(unit, ValueId::MaledictionSlow, 4 * 24);
		}

		// Reset Observance
		if (scbw::get_generic_timer(unit, ValueId::ObservanceTimerDebuff) > 0) {
			scbw::set_generic_timer(unit, ValueId::ObservanceTimerDebuff, 5 * 24);
		}

		// Reset Knockout Drivers
		if (scbw::get_generic_timer(unit, ValueId::KnockoutDriversTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::KnockoutDriversTimer, 2 * 24);
		}

		// Reset Hallucination
		if (scbw::get_generic_timer(unit, ValueId::HallucinatingTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::HallucinatingTimer, 20 * 24);
		}

		// Reset Captivating Claws
		if (get_generic_timer(unit, ValueId::CaptivatingClawsTimer) > 0) {
			set_generic_timer(unit, ValueId::CaptivatingClawsTimer, 1 * 24);
		}

		// Reset Lobotomy Mine slow
		if (scbw::get_generic_timer(unit, ValueId::LobotomySlow) > 0) {
			scbw::set_generic_timer(unit, ValueId::LobotomySlow, 6 * 24);
		}

		// Reset Lockdown
		if (unit->lockdownTimer != 0)
			unit->lockdownTimer = 18;

		// Reset Maelstrom
		if (unit->maelstromTimer != 0)
			unit->maelstromTimer = 12;

		// Reset Devourer Corrosive Spores
		if (scbw::get_generic_timer(unit, ValueId::CorrosiveTouchTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::CorrosiveTouchTimer, VithriliskCorrosiveTouchTimer);
		}

		// Reset Irradiate
		if (unit->irradiateTimer != 0)
			unit->irradiateTimer = 24;

		//was hardcoded in original code
		scbw::refreshConsole();
	}

	void inheritDebuffs(CUnit* parent, CUnit* child) {
		// NOTE: classic period formula is (([real seconds / 0.042) / 8)
		// e.g. (([15] / 0.042) / 8) = 44.642, rounded up to 45

		// Get overlay info
		int32 overlayOffset = getOverlayOffset(child->id);

		// Inherit Ensnare
		if (parent->ensnareTimer != 0) {
			child->ensnareTimer = parent->ensnareTimer;
			if (!(child->sprite->getOverlay(ImageId::EnsnareOverlay_Small + overlayOffset))) {
				child->sprite->createOverlay(ImageId::EnsnareOverlay_Small + overlayOffset);
			}
			child->updateSpeed();
		}

		// Inherit Plague
		if (parent->plagueTimer != 0) {
			child->plagueTimer = parent->plagueTimer;
			if (!(child->sprite->getOverlay(ImageId::PlagueOverlay_Small + overlayOffset))) {
				child->sprite->createOverlay(ImageId::PlagueOverlay_Small + overlayOffset);
			}
		}

		// Inherit Parasite
		if (scbw::get_generic_unit(parent, ValueId::ParasiteSource) != nullptr) {
			scbw::set_generic_unit(child, ValueId::ParasiteSource, scbw::get_generic_unit(parent, ValueId::ParasiteSource));
		}

		// Inherit armor rend (generic)
		if (scbw::get_generic_value(parent, ValueId::ArmorReduce) > 0) {
			scbw::set_generic_value(child, ValueId::ArmorReduce, scbw::get_generic_value(parent, ValueId::ArmorReduce));
		}

		// Inherit Malediction
		if (scbw::get_generic_timer(parent, ValueId::MaledictionSlow) > 0) {
			scbw::set_generic_timer(child, ValueId::MaledictionSlow, scbw::get_generic_timer(parent, ValueId::MaledictionSlow));
		}

		// Inherit Observance
		if (scbw::get_generic_timer(parent, ValueId::ObservanceTimerDebuff) > 0) {
			scbw::set_generic_timer(child, ValueId::ObservanceTimerDebuff, scbw::get_generic_timer(parent, ValueId::ObservanceTimerDebuff));
			scbw::set_generic_timer(child, ValueId::ObservanceTimerPeriod, scbw::get_generic_timer(parent, ValueId::ObservanceTimerPeriod));
			scbw::set_generic_timer(child, ValueId::ObservanceRend, scbw::get_generic_timer(parent, ValueId::ObservanceRend));
		}

		// Inherit Knockout Drivers
		if (scbw::get_generic_timer(parent, ValueId::KnockoutDriversTimer) > 0) {
			scbw::set_generic_timer(child, ValueId::KnockoutDriversTimer, scbw::get_generic_timer(parent, ValueId::KnockoutDriversTimer));
		}

		// Inherit Hallucination
		if (scbw::get_generic_timer(parent, ValueId::HallucinatingTimer) > 0) {
			scbw::set_generic_timer(child, ValueId::HallucinatingTimer, scbw::get_generic_timer(parent, ValueId::HallucinatingTimer));
		}

		// Inherit Captivating Claws
		if (auto timer = scbw::get_generic_timer(parent, ValueId::CaptivatingClawsTimer)) {
			scbw::set_generic_timer(child, ValueId::CaptivatingClawsTimer, timer);
			auto& parentVec = scbw::get_generic_unit_vector(parent, ValueId::CaptivatingClawsUnitVector);
			auto& childVec = scbw::get_generic_unit_vector(child, ValueId::CaptivatingClawsUnitVector);
			childVec = parentVec;
		}

		// Inherit Lobotomy Mine slow
		if (scbw::get_generic_timer(parent, ValueId::LobotomySlow) > 0) {
			scbw::set_generic_timer(child, ValueId::LobotomySlow, scbw::get_generic_timer(parent, ValueId::LobotomySlow));
		}

		// Inherit Lockdown
		if (parent->lockdownTimer != 0)
			child->lockdownTimer = parent->lockdownTimer;

		// Inherit Maelstrom
		if (parent->maelstromTimer != 0)
			child->maelstromTimer = parent->maelstromTimer;

		// Inherit Devourer Corrosive Spores
		if (scbw::get_generic_timer(parent, ValueId::CorrosiveTouchTimer) != 0) {
			scbw::set_generic_value(child, ValueId::CorrosiveTouchStacks, scbw::get_generic_value(parent, ValueId::CorrosiveTouchStacks));
			scbw::set_generic_timer(child, ValueId::CorrosiveTouchTimer, scbw::get_generic_timer(parent, ValueId::CorrosiveTouchTimer));
		}

		// Inherit Irradiate
		if (parent->irradiateTimer != 0)
			child->irradiateTimer = parent->irradiateTimer;

		// Inherit Defensive Matrix
		if (parent->defensiveMatrixTimer > 0) {
			child->defensiveMatrixHp = parent->defensiveMatrixHp;
			child->defensiveMatrixTimer = parent->defensiveMatrixTimer;
		}

		// Inherit Stasis Field
		if (parent->stasisTimer > 0) {
			child->stasisTimer = parent->stasisTimer;
		}

		// Inherit Deconstruction
		if (scbw::get_generic_value(parent, ValueId::Deconstruction) > 0) {
			scbw::set_generic_value(child, ValueId::Deconstruction, scbw::get_generic_value(parent, ValueId::Deconstruction));
			if (child->subunit != nullptr) {
				scbw::set_generic_value(child->subunit, ValueId::Deconstruction, scbw::get_generic_value(parent, ValueId::Deconstruction));
			}
			if (!(child->sprite->getOverlay(ImageId::DeconstructionSmall + overlayOffset))) {
				child->sprite->createOverlay(ImageId::DeconstructionSmall + overlayOffset);
			}
		}

		// Inherit Entropy Gauntlets
		if (scbw::get_generic_timer(parent, ValueId::EntropyGauntlets) > 0) {
			scbw::set_generic_timer(child, ValueId::EntropyGauntlets, scbw::get_generic_timer(parent, ValueId::EntropyGauntlets));
			if (!(child->sprite->getOverlay(ImageId::EntropyGauntletsOverlaySmall + overlayOffset))) {
				child->sprite->createOverlay(ImageId::EntropyGauntletsOverlaySmall + overlayOffset);
			}
		}

		// Inherit Sublime Shepherd
		if (scbw::get_generic_value(parent, ValueId::SublimeShepherd) > 0) {
			scbw::set_generic_value(child, ValueId::SublimeShepherd, scbw::get_generic_value(parent, ValueId::SublimeShepherd));
		}

		// Inherit Cowardice

		// Inherit Malice

		// Inherit Terminal Surge
		if (scbw::get_generic_value(parent, ValueId::TerminalSurge) > 0) {
			scbw::set_generic_value(child, ValueId::TerminalSurge, scbw::get_generic_value(parent, ValueId::TerminalSurge));
		}

		// Inherit Astral Blessing
		if (scbw::get_generic_timer(parent, ValueId::EcclesiastTimer) > 0) {
			scbw::set_generic_value(child, ValueId::EcclesiastStack, scbw::get_generic_value(parent, ValueId::EcclesiastStack));
			scbw::set_generic_value(child, ValueId::EcclesiastPlayer, scbw::get_generic_value(parent, ValueId::EcclesiastPlayer));
			scbw::set_generic_timer(child, ValueId::EcclesiastTimer, scbw::get_generic_timer(parent, ValueId::EcclesiastTimer));
		}

		// Inherit Stellar Enforcement
		if (scbw::get_generic_value(parent, ValueId::StellarEnforcement) > 0) {
			scbw::set_generic_value(child, ValueId::StellarEnforcement, get_generic_value(parent, ValueId::StellarEnforcement));
		}

		// Inherit Adrenal Frenzy
		if (scbw::get_generic_timer(parent, ValueId::NathrokorTimer) > 0) {
			scbw::set_generic_timer(child, ValueId::NathrokorTimer, scbw::get_generic_timer(parent, ValueId::NathrokorTimer));
		}

		// Inherit Swarming Omen
		if (scbw::get_generic_timer(parent, ValueId::SwarmingOmenTimer) > 0) {
			scbw::set_generic_value(child, ValueId::SwarmingOmenStatus, scbw::get_generic_value(parent, ValueId::SwarmingOmenStatus));
			scbw::set_generic_timer(child, ValueId::SwarmingOmenTimer, scbw::get_generic_timer(parent, ValueId::SwarmingOmenTimer));
		}

		// Inherit Kaiser Rampage


		//was hardcoded in original code
		scbw::refreshConsole();
	}

	bool isDebuffed(CUnit* unit) {
		return scbw::get_generic_timer(unit, ValueId::MaledictionSlow) > 0
			|| scbw::get_generic_timer(unit, ValueId::KnockoutDriversTimer) > 0
			|| scbw::get_generic_timer(unit, ValueId::LobotomySlow) > 0
			|| scbw::get_generic_timer(unit, ValueId::ObservanceTimerPeriod) > 0
			|| scbw::get_generic_timer(unit, ValueId::CorrosiveTouchTimer) > 0
			|| scbw::get_generic_value(unit, ValueId::EntropyGauntlets) > 0
			|| scbw::get_generic_value(unit, ValueId::ArmorReduce) > 0
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

	void restoreFunc(CUnit* unit) {
		CUnit* overlayTargetUnit;
		if (unit->subunit != nullptr)
			overlayTargetUnit = unit->subunit;
		else
			overlayTargetUnit = unit;
		uint32 overlayImageId = ImageId::RestorationHit_Small + scbw::getOverlayOffset(unit->id);;
		(overlayTargetUnit->sprite)->createTopOverlay(overlayImageId, 0, 0, 0);

		//remove status effects and corresponding overlays
		//timers set to 0 twice reflect the original code

		// Remove Parasite
		if (scbw::get_generic_timer(unit, ValueId::ParasiteArmTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::ParasiteArmTimer, 0);
			unit->removeOverlay(ImageId::ParasiteOverlay);
		}
		scbw::set_generic_unit(unit, ValueId::ParasiteSource, nullptr);

		// Remove default flags
		unit->parasiteFlags = 0;
		unit->isBlind = 0;

		// Remove Ensnare
		if (unit->ensnareTimer != 0) {
			unit->removeOverlay(ImageId::EnsnareOverlay_Small, ImageId::EnsnareOverlay_Large);
			unit->ensnareTimer = 0;
			//specific update following ensnare removal
			unit->updateSpeed();
		}

		// Remove Plague
		if (unit->plagueTimer != 0) {
			unit->removeOverlay(ImageId::PlagueOverlay_Small, ImageId::PlagueOverlay_Large);
			unit->plagueTimer = 0;
		}

		// Remove Irradiate
		if (unit->irradiateTimer != 0) {
			unit->removeOverlay(ImageId::Irradiate_Small, ImageId::Irradiate_Large);
			unit->irradiateTimer = 0;

			//specific updates following irradiate removal
			unit->irradiatedBy = nullptr;
			unit->irradiatePlayerId = 8;
		}

		// Remove armor rend (generic)
		scbw::set_generic_value(unit, ValueId::ArmorReduce, 0);

		// Remove Entropy Gauntlets
		if (scbw::get_generic_timer(unit, ValueId::EntropyGauntlets) > 0) {
			scbw::set_generic_timer(unit, ValueId::EntropyGauntlets, 1);
		}

		// Remove Malediction
		if (scbw::get_generic_timer(unit, ValueId::MaledictionSlow) > 0) {
			scbw::set_generic_timer(unit, ValueId::MaledictionSlow, 1);
		}

		// Remove Observance
		if (scbw::get_generic_timer(unit, ValueId::ObservanceTimerDebuff) > 0) {
			scbw::set_generic_timer(unit, ValueId::ObservanceTimerDebuff, 1);
		}

		// Remove Knockout Drivers
		if (scbw::get_generic_timer(unit, ValueId::KnockoutDriversTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::KnockoutDriversTimer, 1);
		}

		// Remove Hallucination
		if (scbw::get_generic_timer(unit, ValueId::HallucinatingTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::HallucinatingTimer, 1);
		}

		// Remove Captivating Claws
		if (scbw::get_generic_timer(unit, ValueId::CaptivatingClawsTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::CaptivatingClawsTimer, 0);
			scbw::get_generic_unit_vector(unit, ValueId::CaptivatingClawsUnitVector) = {};
		}

		// Remove Lobotomy Mine slow
		if (scbw::get_generic_timer(unit, ValueId::LobotomySlow) > 0) {
			scbw::set_generic_timer(unit, ValueId::LobotomySlow, 1);
		}

		// Remove Lockdown
		unit->removeLockdown();

		// Remove Maelstrom
		unit->removeMaelstrom();

		// Remove Devourer Corrosive Spores
		unit->removeAcidSpores();

		// Remove Deconstruction
		scbw::set_generic_value(unit, ValueId::Deconstruction, 0);
		unit->removeOverlay(ImageId::DeconstructionSmall, ImageId::DeconstructionLarge);
		if (unit->subunit) {
			scbw::set_generic_value(unit->subunit, ValueId::Deconstruction, 0);
			unit->subunit->removeOverlay(ImageId::DeconstructionSmall, ImageId::DeconstructionLarge);
		}

		//was hardcoded in original code
		scbw::refreshConsole();
	}
	void applyParasite(CUnit* unit, uint8 playerID, CUnit* queen) {
		if (!scbw::canBeParasited(unit)) return;
		int playerFlag = 1 << playerID;
		if (unit->parasiteFlags != playerFlag) { // <- Prevent restarting arm period
			scbw::set_generic_timer(unit, ValueId::ParasiteArmTimer, DefaultArmParasiteTimer);
			unit->sprite->createOverlay(ImageId::ParasiteOverlay, 0, 0, 0);
		}
		scbw::set_generic_unit(unit, ValueId::ParasiteSource, queen);
		unit->parasiteFlags = playerFlag; // Parasite now overwrites other player's parasite
		scbw::refreshConsole();
	}
	CBullet* setMissileSourcePosRet(CUnit* unit, uint32 weaponId, uint32 valueId, int32 rotateCount, uint32 overlayType, uint32 frameset_offset) {
		if (unit->sprite == nullptr || unit->orderTarget.unit == nullptr) {
			return nullptr;
		}
		auto rotate = scbw::get_generic_value(unit, valueId);
		const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::getImageData(overlayType,
			unit->sprite->mainGraphic->id));
		uint8 frameDirection = unit->sprite->mainGraphic->direction;
		Point8 offset = loFile->getOffset(frameDirection + (17 * frameset_offset), rotate);
		if (unit->sprite->mainGraphic->flags & CImage_Flags::Mirrored) {
			offset.x = -offset.x;
		}
		rotate++;
		if (rotate > rotateCount - 1) {
			rotate = 0;
		}
		scbw::set_generic_value(unit, valueId, rotate);
		auto X = unit->getX() + offset.x;
		auto Y = unit->getY() + offset.y;
		auto bullet = createBulletRet(weaponId, unit, X, Y, unit->playerId, scbw::getAngle(X, Y,
			unit->orderTarget.unit->position.x, unit->orderTarget.unit->position.y));
		return bullet;
	}

	bool isSlowed(CUnit* unit) {
		if (unit->ensnareTimer || unit->unusedTimer) {
			return true;
		}
		if (get_generic_timer(unit, ValueId::MaledictionSlow) > 1) {
			return true;
		}
		if (get_generic_timer(unit, ValueId::LobotomySlow) > 1) {
			return true;
		}
		if (unit->id == UnitId::TerranWyvern && get_generic_rust_value(unit, ValueId::ReverseThrust) == 1) {
			return true;
		}
		if (unit->id == UnitId::TerranCyprian) {
			if (scbw::get_generic_value(unit, AiseId::SendCyprianValue) == 1) {
				return true;
			}
		}
		if (unit->id == UnitId::ProtossExemplar && scbw::get_generic_rust_value(unit, ValueId::ExemplarSwitch)) {
			return true;
		}
		return false;
	}

	CUnit** getSourceQueens(CUnit* unit) {
		CUnit* queens[2] = {nullptr};
		if (unit == nullptr) return queens; // <- prevents crash from incubators, but does not fix the issue
		queens[0] = scbw::get_generic_unit(unit, ValueId::BroodlingMother);
		queens[1] = scbw::get_generic_unit(unit, ValueId::ParasiteSource);
		return queens;
	}

	void doDeconstructionDamage(CUnit* unit, uint16 weaponId, bool playSound) {
		// Deconstruction - Pazuzu passive
		double selfDamage = (0.25 * (double)(weapons_dat::DamageAmount[weaponId] * 256.0));
		auto deconstructionTarget = unit;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Subunit) {
			deconstructionTarget = (CUnit*)scbw::get_generic_value(unit, ValueId::TurretParentLink);
			if (deconstructionTarget == nullptr) {
				return;
			}
		}
		hooks::weaponDamageHook(selfDamage, deconstructionTarget, 0, deconstructionTarget, deconstructionTarget->playerId, deconstructionTarget->playerId, 1);
		if (playSound) {
			scbw::playSound(1372, unit);
		}
	}

	void setMissileSourcePos(CUnit* unit, uint32 weaponId, uint32 valueId, int32 rotateCount, uint32 overlayType, uint32 frameset_offset) {
		if (unit->sprite == nullptr || unit->orderTarget.unit == nullptr) {
			return;
		}
		auto rotate = scbw::get_generic_value(unit, valueId);
		const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::getImageData(overlayType,
			unit->sprite->mainGraphic->id));
		uint8 frameDirection = unit->sprite->mainGraphic->direction;
		Point8 offset = loFile->getOffset(frameDirection + (17 * frameset_offset), rotate);
		if (unit->sprite->mainGraphic->flags & CImage_Flags::Mirrored) {
			offset.x = -offset.x;
		}
		rotate++;
		if (rotate > rotateCount - 1) {
			rotate = 0;
		}
		scbw::set_generic_value(unit, valueId, rotate);
		auto X = unit->getX() + offset.x;
		auto Y = unit->getY() + offset.y;
		createBullet(weaponId, unit, X, Y, unit->playerId, scbw::getAngle(X, Y,
			unit->orderTarget.unit->position.x, unit->orderTarget.unit->position.y));
	}

	const uint32 Func_RemoveOverlays2 = 0x00492370;
	void removeoverlays2(uint32 from, uint32 to, CUnit* unit) {
		__asm {
			PUSHAD
			MOV EAX, from
			MOV EBX, unit
			PUSH to
			CALL Func_RemoveOverlays2
			POPAD
		}
	}

	void clearClarionLinks(CUnit* unit) {
		auto& vec = scbw::get_generic_unit_vector(unit, ValueId::ClarionLinkFeedback);
		for (CUnit* vecUnit : vec) {
			if (scbw::get_generic_unit(vecUnit, ValueId::ClarionLinkUnit) == unit) {
				scbw::set_generic_unit(vecUnit, ValueId::ClarionLinkUnit, nullptr);
				//removeoverlays2(1102, 1104, vecUnit);
			}
		}
		vec.clear();
	}

	void placeClarionLinks(CUnit* unit) {
		bool isClarion = unit->id == UnitId::ProtossClarion;
		auto& vec = scbw::get_generic_unit_vector(unit, ValueId::ClarionLinkFeedback);

		clearClarionLinks(unit);
		scbw::UnitFinder(
			unit->getX() - 128,
			unit->getY() - 128,
			unit->getX() + 128,
			unit->getY() + 128
		).filter([&](CUnit* target) {
			if (!scbw::isAlliedTo(unit->playerId, target->playerId)) {
				return false;
			}
			if (units_dat::BaseProperty[target->id] & UnitProperty::Building) {
				return false;
			}
			if (!target->shieldsEnabled()) {
				return false;
			}
			if (target->shields <= 0) {
				return false;
			}
			if (scbw::getDistanceFast(target->getX(), target->getY(), unit->getX(), unit->getY()) > 128) {
				return false;
			}
			return true;
		}).forEach([&](CUnit* target) {
			vec.push_back(target);
			target->sprite->createOverlay(ImageId::PhaseLinkSmall + scbw::getOverlayOffset(target->id), 0, 0, 0);
			scbw::set_generic_unit(target, ValueId::ClarionLinkUnit, unit);
		});
	}

	bool hasClarionLinks(CUnit* unit) {
		return scbw::get_generic_unit(unit, ValueId::ClarionLinkUnit);
	}

	void phaseLinkDamage(CUnit* unit, int32 damage, hooks::WeaponDamageData& data) {
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
			false
		};
		hooks::weaponDamage_root(newData);
	}

	void spreadClarionDamage(hooks::WeaponDamageData& data) {
		CUnit* unit = data.target;
		auto& vec = getClarionLinks(unit);
		if (vec.size() == 0) {
			phaseLinkDamage(unit, data.shieldReduceAmount, data);
		} else {
			int divided = data.shieldReduceAmount / vec.size();
			for (CUnit* vecUnit : vec) {
				phaseLinkDamage(vecUnit, divided, data);
			}
		}
	}

	std::vector<CUnit*> getClarionLinks(CUnit* unit) {
		if (unit->id == UnitId::ProtossClarion) {
			return scbw::get_generic_unit_vector(unit, ValueId::ClarionLinkFeedback);
		}
		auto clarion = scbw::get_generic_unit(unit, ValueId::ClarionLinkUnit);
		return scbw::get_generic_unit_vector(clarion, ValueId::ClarionLinkFeedback);
	}
}