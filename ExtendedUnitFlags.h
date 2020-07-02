#pragma once
#include <map>
#include <vector>
#include <SCBW/structures/CUnit.h>
#include "logger.h"

struct ExtendedUnitFlags;

struct Timer {
	u32* timer = nullptr;
	ExtendedUnitFlags* flags = nullptr;

	Timer(u32* timer) : timer(timer) {}
	Timer(u32* timer, ExtendedUnitFlags* flags) : timer(timer), flags(flags) {}
	virtual ~Timer() {}

	virtual void init() {}
	virtual void tick() {
		if ((*timer) > 0) (*timer)--;
	}
};

struct SpecialTimer1 : public Timer {
	SpecialTimer1(u32* timer, ExtendedUnitFlags* flags) : Timer(timer, flags) {}

	void init();
	void tick();
};

struct ExtendedUnitFlags {
	static std::map<CUnit*, ExtendedUnitFlags> flags;
	static ExtendedUnitFlags* get(CUnit* unitPtr) {
		if (flags.find(unitPtr) == flags.end()) {
			flags.emplace(unitPtr, ExtendedUnitFlags(unitPtr));
		}
		return &flags[unitPtr];
	}
	static void tick() {
		for (auto& kv : flags) {
			ExtendedUnitFlags* flag = &kv.second;
			flag->tickTimers();
		}
	}
	static void unitDie(CUnit* unitPtr) {
		auto it = flags.find(unitPtr);
		if (it != flags.end()) {
			flags.erase(it);
		}
	}

	std::vector<Timer*> timers;
	CUnit* unit = nullptr;

	u32 timerSpecial1 = 0; // used for DT decloaking, can be also used for other special abilities

	ExtendedUnitFlags() {}
	ExtendedUnitFlags(CUnit* unit) : ExtendedUnitFlags() {
		this->unit = unit;
	}

private:

	void tickTimers() {
		SpecialTimer1(&timerSpecial1, this).tick();
	}
};
