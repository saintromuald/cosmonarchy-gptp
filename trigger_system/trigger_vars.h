#pragma once
#include "BW/api.h"
#include <unordered_map>

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

struct TriggerVar {
	s32 value;
};

class TriggerVars {
public:
	void init();
	void update();

	TriggerVar* find(TriggerVarType type, std::string_view name);
	TriggerVar* find_or_create(TriggerVarType type, std::string_view name);
	bool unset(TriggerVarType type, std::string_view name);

private:
	std::unordered_map<TriggerVarKey, TriggerVar> m_vars;
};

inline TriggerVars g_trigger_vars;

