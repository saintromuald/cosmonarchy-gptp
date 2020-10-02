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
	auto impactsSize = salamanderImpacts.size();
	stream.write(reinterpret_cast<const char*>(&impactsSize), sizeof(impactsSize));
	for (int i = 0; i < impactsSize;i++) {
		stream.write(reinterpret_cast<const char*>(&salamanderImpacts[i].position.x), sizeof(salamanderImpacts[i].position.x));
		stream.write(reinterpret_cast<const char*>(&salamanderImpacts[i].position.y), sizeof(salamanderImpacts[i].position.y));
		stream.write(reinterpret_cast<const char*>(&salamanderImpacts[i].elevation), sizeof(salamanderImpacts[i].elevation));
		stream.write(reinterpret_cast<const char*>(&salamanderImpacts[i].repeatsLeft), sizeof(salamanderImpacts[i].repeatsLeft));
		stream.write(reinterpret_cast<const char*>(&salamanderImpacts[i].timer), sizeof(salamanderImpacts[i].timer));
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
	auto impactsSize = 0;
	salamanderImpacts.clear();
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
}
void globals::reset() {
	salamanderImpacts.clear();
}
void globals::frameHook() {
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
				auto n = unitRangeCount(NULL, CFlags::NoInvincible | CFlags::NoBuilding | CFlags::Enemy | CFlags::NoAir, {});
				scbw::UnitFinder unitsInSplash(impact.position.x - r, impact.position.y - r, 
											impact.position.x + r, impact.position.y + r);
				for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
					auto unit = unitsInSplash.getUnit(i);
					if (n.match(unit)) {
						auto damage_factor = unitSizeFactor[units_dat::SizeType[unit->id]];
						double damage = 128.0 * (damage_factor / 256.0);
						unitsInSplash.getUnit(i)->inflictDamage(damage, NULL, -1, false, true);
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
