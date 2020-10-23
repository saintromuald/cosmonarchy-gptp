#include "globals.h"
#include "hook_tools.h"
#include "hooks/hydraFinder.h"
globals Globals;

SaveState globals::aise_save() {
	static SaveState state;
	output.clear();
	std::ostringstream stream(std::stringstream::out|std::stringstream::binary);
	stream.write(reinterpret_cast<const char*>(&extended_unit_table), sizeof(extended_unit_table));
	stream.write(reinterpret_cast<const char*>(&extended_sfx), sizeof(extended_sfx));
	stream.write(reinterpret_cast<const char*>(&tbl_extender), sizeof(tbl_extender));
	stream.write(reinterpret_cast<const char*>(&unitid_extender), sizeof(unitid_extender));
	u32 impactsSize = salamanderImpacts.size();
	stream.write(reinterpret_cast<const char*>(&impactsSize), sizeof(impactsSize));
	for (int i = 0; i < impactsSize;i++) {
		stream.write(reinterpret_cast<const char*>(&salamanderImpacts[i].position.x), sizeof(salamanderImpacts[i].position.x));
		stream.write(reinterpret_cast<const char*>(&salamanderImpacts[i].position.y), sizeof(salamanderImpacts[i].position.y));
		stream.write(reinterpret_cast<const char*>(&salamanderImpacts[i].elevation), sizeof(salamanderImpacts[i].elevation));
		stream.write(reinterpret_cast<const char*>(&salamanderImpacts[i].repeatsLeft), sizeof(salamanderImpacts[i].repeatsLeft));
		stream.write(reinterpret_cast<const char*>(&salamanderImpacts[i].timer), sizeof(salamanderImpacts[i].timer));
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
	output = stream.rdbuf()->str();
	if (output.length() == 0) {
		return state;
		//error
	}
	state.ptr = output.c_str();
	state.len = output.size();
	return state;
}
void globals::aise_load(const char* ptr, u32 len){
	if (len == 0 || ptr==0) {
		return;//error
	}
	std::stringstream stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	std::string str(ptr, len);
	stream << str;
	stream.read(reinterpret_cast<char*>(&extended_unit_table), sizeof(extended_unit_table));
	stream.read(reinterpret_cast<char*>(&extended_sfx), sizeof(extended_sfx));
	stream.read(reinterpret_cast<char*>(&tbl_extender), sizeof(tbl_extender));
	stream.read(reinterpret_cast<char*>(&unitid_extender), sizeof(unitid_extender));
	
	pariahHits.clear();
	salamanderImpacts.clear();
	auto impactsSize = 0;
	stream.read(reinterpret_cast<char*>(&impactsSize), sizeof(impactsSize));
	for (int i = 0; i < impactsSize; i++) {
		salamanderImpact impact;
		stream.read(reinterpret_cast<char*>(&impact.position.x), sizeof(impact.position.x));
		stream.read(reinterpret_cast<char*>(&impact.position.y), sizeof(impact.position.y));
		stream.read(reinterpret_cast<char*>(&impact.elevation), sizeof(impact.elevation));
		stream.read(reinterpret_cast<char*>(&impact.repeatsLeft), sizeof(impact.repeatsLeft));
		stream.read(reinterpret_cast<char*>(&impact.timer), sizeof(impact.timer));
		salamanderImpacts.push_back(impact);
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
}
void globals::reset() {
	salamanderImpacts.clear();
}
void globals::unitFrameHook(CUnit* unit) {
	if (unit->id == UnitId::ProtossPariah && unit->orderSignal & 0x20) {
		unit->orderSignal &= ~0x20;
		remove_unit_from_pariah_table(unit, true);
	}
}
void globals::unitDeathHook(CUnit* unit) {
	if (scbw::get_generic_value(unit, ValueId::LazarusAgent) == 1) {
		initLazarusAgent(unit->position.x, unit->position.y, unit->playerId, unit->id, unit->energy, NULL);
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
			auto revived = scbw::createUnitIngame(agent.unitId, agent.playerId, agent.position.x, agent.position.y, agent.creator);
			if (revived) {
				revived->setHp(units_dat::MaxHitPoints[revived->id] / 2);
				revived->energy = agent.energy;
				revived->playIscriptAnim(IscriptAnimation::Unused1);
				scbw::set_generic_value(revived, ValueId::LazarusNoHeal, 1);
			}
			apostleAgents.erase(apostleAgents.begin() + a);
			a--;
		}
	}
	for (int a = 0; a < salamanderImpacts.size();a++) {
		auto& impact = salamanderImpacts[a];
		if (impact.timer > 0) {
			//deals 20 concussive damage per 5 seconds
			//deals 4 (1024) concussive damage per second
			//deals [128] concussive damage per 1/3 seconds (8 frames)
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
						unitsInSplash.getUnit(i)->inflictDamage(damage, NULL, -1, false, true,true);
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
				salamanderImpacts.erase(salamanderImpacts.begin() + a);
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

void globals::add_bullet_multihit(CBullet* bullet, CUnit* target) {
	for (auto &a : multihitBullets) {
		if (a.bullet == bullet) {
			a.already_hit.push_back(target);
			return;
		}
	}
	multihitBullet gb(bullet, { target });
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
//	scbw::printFormattedText("Try push back %d", apostleAgents.size());
	apostleAgents.push_back(a);
}
void globals::createSalamanderFire(int x, int y, u8 player,u8 elevationLevel, bool addImpactValue) {
	if (addImpactValue) {
		for (auto a : salamanderImpacts) {
			if (scbw::getDistanceFast(x, y, a.position.x, a.position.y) < 12) {
				return;
			}
		}
	}
	CThingy* thingy = scbw::createThingy(SpriteId::SalamanderImpact, x, y, 11);
	if (thingy != NULL) {
		thingy->sprite->elevationLevel = elevationLevel;
		scbw::setThingyVisibilityFlags(thingy);
		if (addImpactValue) {
			salamanderImpact impact;
			impact.position = Point32{ x,y };
			impact.timer = 24;
			impact.repeatsLeft = 5 - 1;
			impact.elevation = elevationLevel;
			salamanderImpacts.push_back(impact);
		}
	}

}
void __declspec(naked) Globals_SaveStateWrapper() {
	static SaveState* state;
	__asm {
		PUSHAD
	}
	state = &Globals.aise_save();
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
}
