#pragma once
#include "BW/api.h"

#include <map>

struct Objective {
	bool show = false;
	s32 score = 0;
	u32 max_score = 0;
	u32 timer = 0;
	std::string text = "Unnamed Objective";
};

class Objectives {
public:
	void reset();
	void update();

	Objective& get(s32 key);
	void remove(s32 key);
	void clear();

	std::map<s32, Objective>::iterator begin() { return m_objectives.begin(); }
	std::map<s32, Objective>::iterator end()   { return m_objectives.end(); }

private:
	std::map<s32, Objective> m_objectives;
};

inline Objectives g_objectives;
