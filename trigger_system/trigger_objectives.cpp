#include "trigger_objectives.h"

void Objectives::reset() {
	clear();
}

void Objectives::update() {
	for (auto& [key, objective] : m_objectives) {
		if (objective.timer > 1) { // We treat 1 as elapsed, since we want timer to still show up. 0 means no timer should show.
			objective.timer--;
		}
	}
}

Objective* Objectives::find(s32 key) {
	auto it = m_objectives.find(key);
	if (it != m_objectives.end()) {
		return &it->second;
	}
	return nullptr;
}

Objective& Objectives::find_or_create(s32 key) {
	Objective* found_objective = find(key);
	if (found_objective) {
		return *found_objective;
	}
	return m_objectives[key] = {};
}

void Objectives::remove(s32 key) {
	m_objectives.erase(key);
}

void Objectives::clear() {
	m_objectives.clear();
}