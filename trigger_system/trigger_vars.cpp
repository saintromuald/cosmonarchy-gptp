#include "trigger_vars.h"
#include <ranges>

void TriggerVars::init() {
	m_vars.clear();
}

void TriggerVars::update() {
	for (auto& [key, value] : m_vars) {
		if (value.value) {
			value.value--;
		}
	}
}

TriggerVar* TriggerVars::find(TriggerVarType type, std::string_view name) {
	auto it = m_vars.find({std::string{name}, type});
	if (it != m_vars.end()) {
		return &it->second;
	}
	return 0;
}

TriggerVar* TriggerVars::find_or_create(TriggerVarType type, std::string_view name) {
	const auto key = TriggerVarKey{std::string{name}, type};
	if (auto it = m_vars.find(key); it != m_vars.end()) {
		return &it->second;
	}

	auto& var = m_vars[key];
	var.value = 0;
	return &var;
}

bool TriggerVars::unset(TriggerVarType type, std::string_view name) {
	return m_vars.erase({std::string{name}, type});
}