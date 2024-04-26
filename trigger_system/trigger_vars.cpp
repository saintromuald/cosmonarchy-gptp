#include "trigger_vars.h"
#include <ranges>
#include <fstream>

VarBank::VarBank(const std::string& name) : m_path{(bw::get_starcraft_path() / "banks" / (name + ".bank"))} {
	load();
}

VarBank::~VarBank() {
	save_if_dirty();
}

s32 VarBank::get(const std::string& name, s32 default_value) {
	const auto it = m_json.find(name);
	if (it != m_json.end()) {
		return *it;
	}
	m_dirty = true;
	return m_json[name] = default_value;
}

void VarBank::set(const std::string& name, s32 value) {
	m_json[name] = value;
	m_dirty = true;
}

void VarBank::clear() {
	m_json.clear();
	m_dirty = true;
}

void VarBank::save_if_dirty() {
	if (m_dirty) {
		save();
	}
}

void VarBank::load() {
	std::ifstream file{m_path, std::ios::binary};
	if (file.is_open()) {
		m_json = json::from_cbor(file, true, false);
	} else {
		m_json.clear();
	}
}

void VarBank::save() {
	std::error_code error;
	fs::create_directories(m_path.parent_path(), error);

	std::ofstream file{m_path, std::ios::binary};
	for (const auto byte : json::to_cbor(m_json)) {
		file << byte;
	}

	m_dirty = false;
}

void TriggerVars::init() {
	m_vars.clear();
}

void TriggerVars::update() {
	for (auto& [key, value] : m_vars) {
		const auto val = value.value();
		if (key.type == TriggerVarType_Timer && val) {
			value.set(val - 1);
		}
	}
	for (auto& [name, bank] : m_banks) {
		bank.save_if_dirty();
	}
}

TriggerVar* TriggerVars::find(TriggerVarType type, std::string_view name) {
	auto it = m_vars.find({std::string{name}, type});
	if (it != m_vars.end()) {
		return &it->second;
	}
	return 0;
}

TriggerVar& TriggerVars::find_or_create(TriggerVarType type, std::string_view name) {
	const auto key = TriggerVarKey{std::string{name}, type};
	if (auto it = m_vars.find(key); it != m_vars.end()) {
		return it->second;
	}

	auto& var = m_vars.emplace(std::move(key), TriggerVar{std::string{name}}).first->second;
	var.set(0);
	return var;
}

void TriggerVars::bank_link_var(const std::string& bank_name, const std::string& name, s32 default_value) {
	auto& bank = get_bank(bank_name);
	auto& var = find_or_create(TriggerVarType_Var, name);
	var.link_bank(&bank, default_value);
}

bool TriggerVars::unset(TriggerVarType type, std::string_view name) {
	return m_vars.erase({std::string{name}, type});
}