#pragma once
#include <memory>

#include "hooks/weapons/weapon_damage.h"
#include "Utils.h"

class IGameMode {
public:
	virtual ~IGameMode() {}

	virtual void on_init() {}
	virtual void on_tick() {}

	virtual void on_unit_init(Unit* unit) {}
	virtual void on_unit_tick(Unit* unit) {}
	virtual void on_unit_damage(const hooks::WeaponDamageData& data) {}
	virtual void on_unit_damage_modifiers(hooks::WeaponDamageData& data) {}
	virtual void on_unit_damage_lifesteal(const hooks::WeaponDamageData& data, fp32& hp_factor, fp32& shields_factor) {}
	virtual bool on_unit_damage_fatal(const hooks::WeaponDamageData& data) { return true; }
	virtual void on_unit_death(Unit* unit) {}
	// TODO: add more major hook callbacks, like stats e.g. OnUnitSpeedUpdate

	// TODO: add bullet events

	// TODO: add corpse events
	
	// TODO: add effect events?
};

inline std::unique_ptr<IGameMode> g_active_game_mode;
