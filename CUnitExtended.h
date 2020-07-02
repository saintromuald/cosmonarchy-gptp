#pragma once
#include <map>
#include <vector>
#include <SCBW/structures/CUnit.h>
#include "logger.h"

struct CUnitExtended;

struct Timer {
	u32* timer = nullptr;
	CUnitExtended* flags = nullptr;

	Timer(u32* timer) : timer(timer) {}
	Timer(u32* timer, CUnitExtended* flags) : timer(timer), flags(flags) {}
	virtual ~Timer() {}

	virtual void init() {}
	virtual void tick() {
		if ((*timer) > 0) (*timer)--;
	}
};

struct TimerSpecial1 : public Timer {
	TimerSpecial1(u32* timer, CUnitExtended* flags) : Timer(timer, flags) {}

	void init();
	void tick();
};

struct CUnitExtended {
	static std::map<CUnit*, CUnitExtended*> flags;
	static CUnitExtended* get(CUnit* unitPtr) {
		if (flags.find(unitPtr) == flags.end()) {
			flags.emplace(unitPtr, new CUnitExtended(unitPtr));
		}
		return flags[unitPtr];
	}
	static void unitDie(CUnit* unitPtr) {
		auto it = flags.find(unitPtr);
		if (it != flags.end()) {
			delete it->second;
			flags.erase(it);
		}
	}

	std::vector<Timer*> timers;
	CUnit* unit = nullptr;

	u32 timerSpecial1 = 0; // used for DT decloaking, can be also used for other special abilities

	CUnitExtended() {}
	CUnitExtended(CUnit* unit) : CUnitExtended() {
		this->unit = unit;
		timers.push_back(new TimerSpecial1(&timerSpecial1, this));
	}

	virtual ~CUnitExtended() {
		for (Timer* timer : timers) {
			delete timer;
		}
		timers.clear();
	}

	void tick() {
		for (Timer* timer : timers) {
			timer->tick();
		}
	}
};
