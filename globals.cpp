#include "globals.h"
#include "hook_tools.h"
#include "hooks/hydraFinder.h"
globals Globals;

//SaveState globals::aise_save() {
void globals::aise_save(){
	output.clear();
	std::ostringstream stream(std::stringstream::out|std::stringstream::binary);
	stream.write(reinterpret_cast<const char*>(&extended_unit_table), sizeof(extended_unit_table));
	stream.write(reinterpret_cast<const char*>(&extended_sfx), sizeof(extended_sfx));
	stream.write(reinterpret_cast<const char*>(&tbl_extender), sizeof(tbl_extender));
	stream.write(reinterpret_cast<const char*>(&unitid_extender), sizeof(unitid_extender));

	u32 impactsSize = persistImpacts.size();
	stream.write(reinterpret_cast<const char*>(&impactsSize), sizeof(impactsSize));
	for (int i = 0; i < impactsSize;i++) {
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].position.x), sizeof(persistImpacts[i].position.x));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].position.y), sizeof(persistImpacts[i].position.y));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].elevation), sizeof(persistImpacts[i].elevation));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].repeatsLeft), sizeof(persistImpacts[i].repeatsLeft));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].timer), sizeof(persistImpacts[i].timer));
		stream.write(reinterpret_cast<const char*>(&persistImpacts[i].impactId), sizeof(persistImpacts[i].impactId));
	}
	u32 pariahHitCount = pariahHits.size();
	for (int i = 0; i < pariahHitCount; i++) {
		stream.write(reinterpret_cast<const char*>(&pariahHits[i].source), sizeof(pariahHits[i].source));
		stream.write(reinterpret_cast<const char*>(&pariahHits[i].target), sizeof(pariahHits[i].target));
	}
	u32 apostleCount = apostleAgents.size();
	for (int i = 0; i < apostleCount; i++) {
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].creator), sizeof(apostleAgents[i].creator));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].energy), sizeof(apostleAgents[i].energy));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].playerId), sizeof(apostleAgents[i].playerId));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].position.x), sizeof(apostleAgents[i].position.x));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].position.y), sizeof(apostleAgents[i].position.y));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].timer), sizeof(apostleAgents[i].timer));
		stream.write(reinterpret_cast<const char*>(&apostleAgents[i].unitId), sizeof(apostleAgents[i].unitId));
	}
	u32 multihitCount = multihitBullets.size();
	stream.write(reinterpret_cast<char*>(&multihitCount), sizeof(multihitCount));
	for (int i = 0; i < multihitCount; i++) {
		multihitBullet bull = multihitBullets[i];
		stream.write(reinterpret_cast<char*>(&bull.bullet), sizeof(bull.bullet));
		stream.write(reinterpret_cast<char*>(&bull.divider), sizeof(bull.divider));
		u32 alreadyHitCount = bull.already_hit.size();
		stream.write(reinterpret_cast<char*>(&alreadyHitCount), sizeof(alreadyHitCount));
		for (int j = 0; j < alreadyHitCount; j++) {
			CUnit* hit = bull.already_hit[j];
			stream.write(reinterpret_cast<char*>(&hit), sizeof(hit));
		}
	}
	u32 lurkerMapSize = lurkerSpineHits.size();
	for (auto a : lurkerSpineHits) {
		CUnit* key = a.first;
		stream.write(reinterpret_cast<char*>(&key), sizeof(key));
		u32 valueCount = a.second.size();
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
void globals::aise_load(const char* ptr, u32 len){
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
	u32 pariahHitsCount = 0;
	stream.read(reinterpret_cast<char*>(&pariahHitsCount), sizeof(pariahHitsCount));
	for (int i = 0; i < pariahHitsCount; i++) {
		pariahHit hit;
		stream.read(reinterpret_cast<char*>(&hit.source), sizeof(hit.source));
		stream.read(reinterpret_cast<char*>(&hit.target), sizeof(hit.target));
		pariahHits.push_back(hit);
	}
	u32 apostleCount = 0;
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
	u32 multihitCount = 0;
	stream.read(reinterpret_cast<char*>(&multihitCount), sizeof(multihitCount));
	//scbw::printFormattedText("Hits: %d", multihitCount);
	/*
	for (int i = 0; i < multihitCount; i++) {
		multihitBullet bull;
		stream.read(reinterpret_cast<char*>(&bull.bullet), sizeof(bull.bullet));
		stream.read(reinterpret_cast<char*>(&bull.divider), sizeof(bull.divider));
		u32 alreadyHitCount = 0;
		stream.read(reinterpret_cast<char*>(&alreadyHitCount), sizeof(alreadyHitCount));
		for (int j = 0; j < alreadyHitCount; j++) {
			CUnit* hit;
			stream.read(reinterpret_cast<char*>(&hit), sizeof(hit));
			bull.already_hit.push_back(hit);
		}
		multihitBullets.push_back(bull);
	}
	u32 lurkerMapSize = 0;
	stream.read(reinterpret_cast<char*>(&lurkerMapSize), sizeof(lurkerMapSize));
	for (int i = 0; i < lurkerMapSize; i++) {
		CUnit* key = NULL;
		stream.read(reinterpret_cast<char*>(&key), sizeof(key));
		u32 valueCount = 0;
		stream.read(reinterpret_cast<char*>(&valueCount), sizeof(valueCount));	
		for (int j = 0; j < valueCount; j++) {
			CUnit* value = NULL;
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
	
	for (int i = 0; i < this->apostleAgents.size(); i++) {
		if (apostleAgents[i].creator == unit) {
			apostleAgents[i].creator=NULL;
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
				revived->status |= UnitStatus::Invincible;
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
					auto n = unitRangeCount(NULL, CFlags::NoInvincible | CFlags::Enemy | CFlags::NoAir, {});
					scbw::UnitFinder unitsInSplash(impact.position.x - r, impact.position.y - r,
						impact.position.x + r, impact.position.y + r);
					for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
						auto unit = unitsInSplash.getUnit(i);
						if (n.match(unit)) {
							double damage_factor = unitSizeFactor[units_dat::SizeType[unit->id]];
							double damage = (double)weapons_dat::DamageAmount[WeaponId::IncendiaryPayload] * (damage_factor / 256.0);
							unitsInSplash.getUnit(i)->inflictDamage(damage, NULL, -1, false, true, true);
						}
					}
				}
			}
			else if (impact.impactId == ImpactId::Planetcracker) {
				int r = 8*32;
				auto n = unitRangeCount(NULL, CFlags::NoInvincible | CFlags::NoAir, {});
				scbw::UnitFinder unitsInSplash(impact.position.x - r, impact.position.y - r,
					impact.position.x + r, impact.position.y + r);
				for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
					auto unit = unitsInSplash.getUnit(i);
					if (n.match(unit)) {
						unitsInSplash.getUnit(i)->inflictDamage(1777, NULL, -1, false, true, true);
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
			a.creator = NULL;//agent values themselves must not be deleted to prevent non-working lazarus agent buffs
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
u32 globals::get_multihit_count(CBullet* bullet) {
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
void globals::initLazarusAgent(int x, int y, u8 player, u32 unitId, u32 energy, CUnit* unit) {
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

void globals::createPlanetCracker(int x, int y, u8 player, u8 elevationLevel) {
	persistImpact impact;
	impact.position = Point32{ x,y };
	impact.timer = 360;
	impact.repeatsLeft = 0;
	impact.elevation = elevationLevel;
	impact.impactId = ImpactId::Planetcracker;
	persistImpacts.push_back(impact);
}
void globals::createSalamanderFire(int x, int y, u8 player,u8 elevationLevel, bool addImpactValue) {
	if (addImpactValue) {
		for (auto a : persistImpacts) {
			if (a.impactId==ImpactId::Salamander && scbw::getDistanceFast(x, y, a.position.x, a.position.y) < 12) {
				return;
			}
		}
	}
	CThingy* thingy = scbw::createThingy(SpriteId::SalamanderImpact, x, y, 11);
	if (thingy != NULL) {
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


void globals::createIonField(int x, int y, u8 player, u8 elevationLevel, bool addImpactValue) {
	if (addImpactValue) {
		for (auto a : persistImpacts) {
			if (a.impactId==ImpactId::IonField && scbw::getDistanceFast(x, y, a.position.x, a.position.y) < 12) {
				return;
			}
		}
	}
	CThingy* thingy = scbw::createThingy(SpriteId::IonField, x, y, 11);
	if (thingy != NULL) {
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


void globals::clear_lurker(CUnit* lurker) {
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
	static u32 len;
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
//	const u8 retn = 0xc3;
//	memoryPatch(0x004AAF23 + 5, &retn, 1);
//	memoryPatch(0x004AAE94 + 5, &retn, 1);
}
