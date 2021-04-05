#include "globals.h"
#include "hook_tools.h"
#include "hooks/hydraFinder.h"
#include "hooks/limits/attack_extender.h"
#include <hooks/weapons/weapon_damage.h>
globals Globals;

void globals::refreshDebuggingInfo() {
/*	unitSpeedDbg = 0;
	weaponDamageDbg = 0;
	gameHookDbg = 0;
	updateUnitStateDbg = 0;
	updateStatusEffectsDbg = 0;*/
}

//SaveState globals::aise_save() {


void globals::aise_save(){
	output.clear();
	std::ostringstream stream(std::stringstream::out|std::stringstream::binary);
	stream.write(reinterpret_cast<const char*>(&extended_unit_table), sizeof(extended_unit_table));
	stream.write(reinterpret_cast<const char*>(&extended_sfx), sizeof(extended_sfx));
	stream.write(reinterpret_cast<const char*>(&tbl_extender), sizeof(tbl_extender));
	stream.write(reinterpret_cast<const char*>(&unitid_extender), sizeof(unitid_extender));

	uint32 impactsSize = persistImpacts.size();
	stream.write(reinterpret_cast<const char*>(&impactsSize), sizeof(impactsSize));
	for (int i = 0; i < impactsSize;i++) {
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].position.x), sizeof(persistImpacts[i].position.x));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].position.y), sizeof(persistImpacts[i].position.y));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].elevation), sizeof(persistImpacts[i].elevation));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].repeatsLeft), sizeof(persistImpacts[i].repeatsLeft));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].timer), sizeof(persistImpacts[i].timer));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].impactId), sizeof(persistImpacts[i].impactId));
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
		multihitBullet bull = multihitBullets[i];
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
void globals::aise_load(const char* ptr, uint32 len){
	if (len == 0 || ptr==0) {
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
		persistImpact impact;
		stream.read(reinterpret_cast<char*>(&impact.position.x), sizeof(impact.position.x));
		stream.read(reinterpret_cast<char*>(&impact.position.y), sizeof(impact.position.y));
		stream.read(reinterpret_cast<char*>(&impact.elevation), sizeof(impact.elevation));
		stream.read(reinterpret_cast<char*>(&impact.repeatsLeft), sizeof(impact.repeatsLeft));
		stream.read(reinterpret_cast<char*>(&impact.timer), sizeof(impact.timer));
		stream.read(reinterpret_cast<char*>(&impact.impactId), sizeof(impact.impactId));
		persistImpacts.push_back(impact);
	}
	uint32 pariahHitsCount = 0;
	stream.read(reinterpret_cast<char*>(&pariahHitsCount), sizeof(pariahHitsCount));
	for (int i = 0; i < pariahHitsCount; i++) {
		pariahHit hit;
		stream.read(reinterpret_cast<char*>(&hit.source), sizeof(hit.source));
		stream.read(reinterpret_cast<char*>(&hit.target), sizeof(hit.target));
		pariahHits.push_back(hit);
	}
	uint32 apostleCount = 0;
	stream.read(reinterpret_cast<char*>(&apostleCount), sizeof(apostleCount));
	for (int i = 0; i < pariahHitsCount; i++) {
		apostleAgent agent;
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
		multihitBullet bull;
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
void globals::reset() {
	persistImpacts.clear();
	pariahHits.clear();
	multihitBullets.clear();
	apostleAgents.clear();
	lurkerSpineHits.clear();
}
void globals::unitFrameHook(CUnit* unit) {
	if (unit->id == UnitId::ProtossPariah && unit->orderSignal & 0x20) {
		unit->orderSignal &= ~0x20;
		remove_unit_from_pariah_table(unit, true);
	}
}
void globals::unitDeathHook(CUnit* unit) {
	unitStructs.erase(unit);
	for (int i = 0; i < this->apostleAgents.size(); i++) {
		if (apostleAgents[i].creator == unit) {
			apostleAgents[i].creator=nullptr;
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
void globals::frameHook() {
	for (auto & a : unitStructs) {
		for (auto& timerPair : a.second.genericTimers) {
			if (timerPair.second > 0) {
//				scbw::printFormattedText("Timer decrease: %d %d", i, a.second.genericTimers[i]);
				timerPair.second--;
			}
		}
	}
	for (int a = 0; a < apostleAgents.size(); a++) {
		auto& agent = apostleAgents[a];
		if (agent.timer > 0) {
			agent.timer--;
		}
		else {
			//revive unit
//			scbw::printFormattedText("Agent Creator: 0x%X", agent.creator);
			auto revived = scbw::createUnitIngame(agent.unitId, agent.playerId, agent.position.x, agent.position.y, agent.creator);
			if (revived) {
				revived->setHp(units_dat::MaxHitPoints[revived->id] / 2);
				revived->energy = agent.energy;
				revived->playIscriptAnim(IscriptAnimation::Unused1);
				scbw::set_generic_value(revived, ValueId::LazarusNoHeal, 1);
			//	revived->status |= UnitStatus::Invincible;
				scbw::set_generic_value(revived, ValueId::IsReviving, 1);

				
			}
			apostleAgents.erase(apostleAgents.begin() + a);
			a--;
		}
	}
	for (int a = 0; a < persistImpacts.size();a++) {
		auto& impact = persistImpacts[a];
		if (impact.timer > 0) {
			//deals 20 concussive damage per 5 seconds
			//deals 4 (1024) concussive damage per second
			//deals [128] concussive damage per 1/3 seconds (8 frames)
			if (impact.impactId == ImpactId::Salamander) {
				if (impact.timer % 3 == 0) {
					constexpr int unitSizeFactor[5] = { 2, 0, 256, 192, 128 };//Concussive damage
					auto count = 0;
					int r = 16;
					auto n = unitRangeCount(nullptr, CFlags::NoInvincible | CFlags::Enemy | CFlags::NoAir, {});
					scbw::UnitFinder unitsInSplash(impact.position.x - r, impact.position.y - r,
						impact.position.x + r, impact.position.y + r);
					for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
						auto unit = unitsInSplash.getUnit(i);
						if (n(unit)) {
							double damage_factor = unitSizeFactor[units_dat::SizeType[unit->id]];
							double damage = (double)(weapons_dat::DamageAmount[WeaponId::SalamanderIncendiaryPayload] + weapons_dat::DamageUpgrade[WeaponId::SalamanderIncendiaryPayload]) * (damage_factor / 256.0);
							unitsInSplash.getUnit(i)->inflictDamage(damage, nullptr, -1, false, true, true);
						}
					}
				}
			}
			else if (impact.impactId == ImpactId::Planetcracker) {
				int r = 8*32;
				auto n = unitRangeCount(nullptr, CFlags::NoInvincible | CFlags::NoAir, {});
				scbw::UnitFinder unitsInSplash(impact.position.x - r, impact.position.y - r,
					impact.position.x + r, impact.position.y + r);
				for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
					auto unit = unitsInSplash.getUnit(i);
					if (n(unit)) {
						unitsInSplash.getUnit(i)->inflictDamage(1777, nullptr, -1, false, true, true);
						//1777 is [ 2500dmg*256 / 15sec*24 ]
					}
				}
			}
			impact.timer--;
		}
		else {
			if (impact.repeatsLeft > 0) {
				impact.repeatsLeft--;
				impact.timer = 24;
				createSalamanderFire(impact.position.x, impact.position.y,11, impact.elevation, false);
			}
			else {
				persistImpacts.erase(persistImpacts.begin() + a);
				a--;
			}
		}
	}
}

void globals::remove_unit_from_pariah_table(CUnit* unit, bool sourceOnly){
	for (int i = 0; i < pariahHits.size(); i++) {
		if (pariahHits[i].source == unit || (pariahHits[i].target == unit && !sourceOnly)) {
			pariahHits.erase(pariahHits.begin() + i);
			i--;
		}
	}
}
void globals::remove_multihit_bullet(CBullet* bullet) {
	for (int i = 0; i < multihitBullets.size(); i++) {
		if (multihitBullets[i].bullet == bullet) {
			multihitBullets.erase(multihitBullets.begin() + i);
			i--;
		}
	}
}
bool globals::was_hit_by_pariah(CUnit* source, CUnit* target){
	for (auto a : pariahHits) {
		if (source == a.source && target == a.target) {
			return true;
		}
	}
	return false;
}
void globals::register_pariah_hit(CUnit* source, CUnit* target){
	pariahHit hit;
	hit.source = source;
	hit.target = target;
	pariahHits.push_back(hit);
}
void globals::clear_ptr_in_apostle_table(CUnit* ptr) {
	for (auto& a : apostleAgents) {
		if (a.creator == ptr) {
			a.creator = nullptr;//agent values themselves must not be deleted to prevent non-working lazarus agent buffs
		}
	}
}

bool globals::was_already_hit_by_multihit_bullet(CBullet* bullet, CUnit* target) {
	for (auto &a : multihitBullets) {
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
bool globals::multihit_bullet_is_set(CBullet* bullet) {
	for (auto& a : multihitBullets) {
		if (bullet == a.bullet) {
			return true;
		}
	}
	return false;
}
void globals::add_bullet_multihit(CBullet* bullet, CUnit* target) {
	for (auto &a : multihitBullets) {
		if (a.bullet == bullet) {
			a.already_hit.push_back(target);
			return;
		}
	}
	multihitBullet gb(bullet, { target },0);
	multihitBullets.push_back(gb);
}
void globals::add_incremental_bullet_multihit(CBullet* bullet, CUnit* target) {
	for (auto& a : multihitBullets) {
		if (a.bullet == bullet) {
			a.already_hit.push_back(target);
			return;
		}
	}
	multihitBullet gb(bullet, { target }, 1);
	multihitBullets.push_back(gb);
}
int globals::get_divider(CBullet* bullet) {
	for (auto& a : multihitBullets) {
		if (a.bullet == bullet && a.divider != 0) {
			return a.divider;
		}
	}
	return 1;
}
uint32 globals::get_multihit_count(CBullet* bullet) {
	for (auto& a : multihitBullets) {
		if (a.bullet == bullet) {
			return a.already_hit.size();
		}
	}
	return 0;
}
void globals::add_multihit_empty_bullet(CBullet* bullet, int divider) {
	for (auto& a : multihitBullets) {
		if (a.bullet == bullet) {
			a.divider=divider;
			return;
		}
	}
	multihitBullet gb(bullet, {}, divider);
	multihitBullets.push_back(gb);
}
void globals::initLazarusAgent(int x, int y, uint8 player, uint32 unitId, uint32 energy, CUnit* unit) {
	apostleAgent a;
	a.playerId = player;
	a.position.x = x;
	a.position.y = y;
	a.timer = 48;
	a.unitId = unitId;
	a.energy = energy;
	a.creator = unit;
//	scbw::printFormattedText("Set creator to 0x%X", a.creator);
//	scbw::printFormattedText("Try push back %d", apostleAgents.size());
	apostleAgents.push_back(a);
}

void globals::createPlanetCracker(int x, int y, uint8 player, uint8 elevationLevel) {
	persistImpact impact;
	impact.position = Point32{ x,y };
	impact.timer = 360;
	impact.repeatsLeft = 0;
	impact.elevation = elevationLevel;
	impact.impactId = ImpactId::Planetcracker;
	persistImpacts.push_back(impact);
}
void globals::createSalamanderFire(int x, int y, uint8 player,uint8 elevationLevel, bool addImpactValue) {
	if (addImpactValue) {
		for (auto a : persistImpacts) {
			if (a.impactId==ImpactId::Salamander && scbw::getDistanceFast(x, y, a.position.x, a.position.y) < 12) {
				return;
			}
		}
	}
	CThingy* thingy = scbw::createThingy(SpriteId::SalamanderImpact, x, y, 11);
	if (thingy != nullptr) {
		thingy->sprite->elevationLevel = elevationLevel;
		scbw::setThingyVisibilityFlags(thingy);
		if (addImpactValue) {
			persistImpact impact;
			impact.position = Point32{ x,y };
			impact.timer = 24;
			impact.repeatsLeft = 5 - 1;
			impact.elevation = elevationLevel;
			impact.impactId = ImpactId::Salamander;
			persistImpacts.push_back(impact);
		}
	}
}


void globals::createIonField(int x, int y, uint8 player, uint8 elevationLevel, bool addImpactValue) {
	if (addImpactValue) {
		for (auto a : persistImpacts) {
			if (a.impactId==ImpactId::IonField && scbw::getDistanceFast(x, y, a.position.x, a.position.y) < 12) {
				return;
			}
		}
	}
	CThingy* thingy = scbw::createThingy(SpriteId::IonField, x, y, 11);
	if (thingy != nullptr) {
		thingy->sprite->elevationLevel = elevationLevel;
		scbw::setThingyVisibilityFlags(thingy);
		if (addImpactValue) {
			persistImpact impact;
			impact.position = Point32{ x,y };
			impact.timer = 24;
			impact.repeatsLeft = 5 - 1;
			impact.elevation = elevationLevel;
			impact.impactId = ImpactId::IonField;
			persistImpacts.push_back(impact);
		}
	}
}


void globals::clear_lakizilisk(CUnit* lurker) {
	lurkerSpineHits[lurker].clear();

}
void globals::add_lurker_spine_pair(CUnit* lurker, CUnit* target) {
	lurkerSpineHits[lurker].push_back(target);
}
bool globals::spine_pair_exists(CUnit* lurker, CUnit* target) {
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
//	state = &Globals.aise_save();
	Globals.aise_save();
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
		MOV len,ECX
		MOV pointer,EAX
		PUSHAD
	}
	Globals.aise_load(pointer, len);
	__asm {
		POPAD
		RETN
	}
}

void injectGlobalsHooks() {
	jmpPatch(Globals_SaveStateWrapper, 0x004AAF23);
	jmpPatch(Globals_LoadStateWrapper, 0x004AAE94);
//	const uint8 retn = 0xc3;
//	memoryPatch(0x004AAF23 + 5, &retn, 1);
//	memoryPatch(0x004AAE94 + 5, &retn, 1);
}

namespace scbw {
	CUnit* get_generic_unit(CUnit* unit, uint32 id) {
		auto find = Globals.unitStructs.find(unit);
		if (find != Globals.unitStructs.end()) {
			return find->second.getGenericUnit(id);
		}
		return nullptr;
	}
	uint32 get_generic_value(CUnit* u, uint32 val) {
		auto find = Globals.unitStructs.find(u);
		if (find != Globals.unitStructs.end()) {
			return find->second.getGenericValue(val);
		}
		return 0;
	}
	uint32 get_generic_timer(CUnit* u, uint32 val) {
		auto find = Globals.unitStructs.find(u);
		if (find != Globals.unitStructs.end()) {
			return find->second.getGenericTimer(val);
		}
		return 0;
	}
	void set_generic_unit(CUnit* unit, uint32 id, CUnit* targ) {
		Globals.unitStructs[unit].setGenericUnit(id, targ);
	}
	void set_generic_value(CUnit* u, uint32 id, uint32 val) {
		Globals.unitStructs[u].setGenericValue(id, val);
	}
	void set_generic_timer(CUnit* u, uint32 id, uint32 val) {
		Globals.unitStructs[u].setGenericTimer(id, val);
	}
	void add_generic_value(CUnit* u, uint32 id, int val) {
		Globals.unitStructs[u].addGenericValue(id, val);
	}

	void refreshDebuffs(CUnit* unit) {
		// NOTE: classic timer formula is (([real seconds / 0.042) / 8)
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
		if (scbw::get_generic_timer(unit, ValueId::CaptivatingClawsTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::CaptivatingClawsTimer, 1 * 24);
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
		if (unit->corrosiveSporeCount != 0)
			for (int i = 0; i < unit->corrosiveSporeCount; i++)
				unit->acidSporeTime[i] = 30;

		// Reset Irradiate
		if (unit->irradiateTimer != 0)
			unit->irradiateTimer = 24;

		//was hardcoded in original code
		scbw::refreshConsole();
	}

	void inheritDebuffs(CUnit* parent, CUnit* child) {
		// NOTE: classic timer formula is (([real seconds / 0.042) / 8)
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
		if (scbw::get_generic_timer(parent, ValueId::CaptivatingClawsTimer) > 0) {
			scbw::set_generic_timer(child, ValueId::CaptivatingClawsTimer, scbw::get_generic_timer(parent, ValueId::CaptivatingClawsTimer));
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
		if (parent->corrosiveSporeCount != 0) {
			child->corrosiveSporeCount = parent->corrosiveSporeCount;
			for (int i = 0; i < parent->corrosiveSporeCount; i++)
				child->acidSporeTime[i] = parent->acidSporeTime[i];
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
			if (!(child->sprite->getOverlay(1332 + overlayOffset))) {
				child->sprite->createOverlay(1332 + overlayOffset);
			}
		}

		// Inherit Entropy Gauntlets
		if (scbw::get_generic_timer(parent, ValueId::EntropyGauntlets) > 0) {
			scbw::set_generic_timer(child, ValueId::EntropyGauntlets, scbw::get_generic_timer(parent, ValueId::EntropyGauntlets));
			if (!(child->sprite->getOverlay(1107 + overlayOffset))) {
				child->sprite->createOverlay(1107 + overlayOffset);
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
			|| scbw::get_generic_timer(unit, ValueId::CaptivatingClawsTimer) > 0
			|| scbw::get_generic_timer(unit, ValueId::LobotomySlow) > 0
			|| scbw::get_generic_timer(unit, ValueId::ObservanceTimerDebuff) > 0
			|| scbw::get_generic_value(unit, ValueId::EntropyGauntlets) > 0
			|| scbw::get_generic_value(unit, ValueId::ArmorReduce) > 0
			|| unit->irradiateTimer != 0
			|| unit->ensnareTimer != 0
			|| unit->lockdownTimer != 0
			|| unit->maelstromTimer != 0
			|| unit->corrosiveSporeCount != 0
			|| unit->plagueTimer != 0
			|| unit->parasiteFlags
			|| unit->isBlind;
		// TODO: cowardice, malice + also add them to status tooltip
	}
	void restoreFunc(CUnit* unit) {
		uint32 overlayImageId;
		CUnit* overlayTargetUnit;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
			overlayImageId = ImageId::RestorationHit_Medium;
		else if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
			overlayImageId = ImageId::RestorationHit_Large;
		else
			overlayImageId = ImageId::RestorationHit_Small;
		if (unit->subunit != nullptr)
			overlayTargetUnit = unit->subunit;
		else
			overlayTargetUnit = unit;
		(overlayTargetUnit->sprite)->createTopOverlay(overlayImageId, 0, 0, 0);

		//remove status effects and corresponding overlays
		//timers set to 0 twice reflect the original code

		// Remove Parasite
		if (scbw::get_generic_timer(unit, ValueId::ParasiteArmTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::ParasiteArmTimer, 0);
			unit->removeOverlay(1031);
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
			scbw::set_generic_timer(unit, ValueId::CaptivatingClawsTimer, 1);
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
		unit->removeOverlay(1332);
		if (unit->subunit) {
			scbw::set_generic_value(unit->subunit, ValueId::Deconstruction, 0);
			unit->subunit->removeOverlay(1332);
		}

		//was hardcoded in original code
		scbw::refreshConsole();
	}
	void applyParasite(CUnit* unit, uint8 playerID, CUnit* queen) {
		if (!scbw::canBeParasited(unit)) return;
		int playerFlag = 1 << playerID;
		if (unit->parasiteFlags != playerFlag) { // <- Prevent restarting arm timer
			scbw::set_generic_timer(unit, ValueId::ParasiteArmTimer, DefaultArmParasiteTimer);
			unit->sprite->createOverlay(1031, 0, 0, 0);
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
		CUnit* queens[2] = { nullptr };
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

}