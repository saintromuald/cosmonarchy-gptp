#pragma once
#include <memory>

#include "BW/api.h"
#include "hooks/weapons/weapon_damage.h"
#include "Utils.h"

class IGameMode {
public:
	virtual ~IGameMode() {}

	virtual void OnInit() {}
	virtual void OnTick() {}

	virtual void OnUnitInit(Unit* unit) {}
	virtual void OnUnitTick(Unit* unit) {}
	virtual void OnUnitDamage(const hooks::WeaponDamageData& data) {}
	virtual void OnUnitDamageModifiers(hooks::WeaponDamageData& data) {}
	virtual void OnUnitDamageLifesteal(const hooks::WeaponDamageData& data, f32& hp_factor, f32& shields_factor) {}
	virtual bool OnUnitDamageFatal(const hooks::WeaponDamageData& data) { return true; }
	virtual void OnUnitDeath(Unit* unit) {}
	// TODO: add more major hook callbacks, like stats e.g. OnUnitSpeedUpdate

	// TODO: add bullet events

	// TODO: add corpse events
	
	// TODO: add effect events?
};

inline std::unique_ptr<IGameMode> g_active_game_mode;
