#pragma once
#include "BW/api.h"

#include <map>

struct Objective {
	bool show;
	s32 score;
	u32 max_score;
	u32 timer;
	std::string text;
};

class Objectives {
public:
	void reset();
	void update();

	Objective* find(s32 key);
	Objective& find_or_create(s32 key);
	void remove(s32 key);
	void clear();

	std::map<s32, Objective>::iterator begin() { return m_objectives.begin(); }
	std::map<s32, Objective>::iterator end()   { return m_objectives.end(); }

private:
	std::map<s32, Objective> m_objectives;
};

inline Objectives g_objectives;
