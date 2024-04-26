#pragma once
#include "BW/api.h"
#include <map>
#include <unordered_map>

#include "json.hpp"
using nlohmann::json;

enum TriggerVarType {
	TriggerVarType_Var,
	TriggerVarType_Timer,

	TriggerVarType_Count,
};

struct TriggerVarKey {
	std::string name;
	TriggerVarType type;
	
	bool operator==(const TriggerVarKey&) const = default;
};

template <>
struct std::hash<TriggerVarKey> {
	size_t operator()(const TriggerVarKey& key) const {
		return std::hash<std::string>{}(key.name) ^ (std::hash<u32>{}(key.type) << 1);
	}
};

class VarBank {
public:
	explicit VarBank(const std::string& name);

	s32 get(const std::string& name);
	void set(const std::string& name, s32 value);
	void clear();

private:
	void load();
	void save();

private:
	fs::path m_path;
	json m_json;
};

class TriggerVar {
public:
	TriggerVar(std::string name, s32 value = 0) : m_name(std::move(name)), m_value(value) {}

	s32 value() const { return m_value; }

	void set(s32 value) {
		if (m_bank) {
			m_bank->set(m_name, value);
		}
		m_value = value;
	}

	void link_bank(VarBank* bank) {
		m_bank = bank;
		if (bank) {
			m_value = bank->get(m_name);
		} else {
			m_value = 0;
		}
	}

private:
	std::string m_name;
	s32 m_value;
	VarBank* m_bank = nullptr;
};

class TriggerVars {
public:
	void init();
	void update();

	TriggerVar* find(TriggerVarType type, std::string_view name);
	TriggerVar& find_or_create(TriggerVarType type, std::string_view name);
	void bank_link_var(const std::string& bank_name, const std::string& name);
	bool unset(TriggerVarType type, std::string_view name);

	VarBank& get_bank(const std::string& name) {
		if (const auto it = m_banks.find(name); it != m_banks.end()) {
			return it->second;
		}
		auto& bank = m_banks.emplace(name, name).first->second;
		return bank;
	}

private:
	std::map<std::string, VarBank> m_banks; // Note: we need std::map's stable addresses for trigger vars to keep track of the banks
	std::unordered_map<TriggerVarKey, TriggerVar> m_vars;
};

inline TriggerVars g_trigger_vars;

