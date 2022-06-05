#include "globals.h"
#include "hook_tools.h"
#include "hooks/cosmonarchy_finder.h"
#include "hooks/limits/attack_extender.h"
#include "hooks/weapons/weapon_damage.h"
#include <algorithm>
#include <unordered_set>
#include <Utils/captivating_claws.h>
Globals globals;

void Globals::refreshDebuggingInfo()
{
	/*	
	unitSpeedDbg = 0;
	weaponDamageDbg = 0;
	gameHookDbg = 0;
	updateUnitStateDbg = 0;
	updateStatusEffectsDbg = 0;
	*/
}

static SaveState saveState;

void Globals::aise_save()
{
	scbw::print("WARNING: Saving doesn't work properly in Cosmonarchy.\n\t\t\tAny plugin specific data will be lost.", GameTextColor::Yellow);
	return;

	output.clear();
	std::ostringstream stream(std::stringstream::out | std::stringstream::binary);
	stream.write(reinterpret_cast<const char*>(&extended_unit_table), sizeof(extended_unit_table));
	stream.write(reinterpret_cast<const char*>(&extended_sfx), sizeof(extended_sfx));
	stream.write(reinterpret_cast<const char*>(&tbl_extender), sizeof(tbl_extender));
	stream.write(reinterpret_cast<const char*>(&unitid_extender), sizeof(unitid_extender));

	u32 effectsSize = effects.size();
	stream.write(reinterpret_cast<const char*>(&effectsSize), sizeof(effectsSize));
	for (s32 i = 0; i < effectsSize; i++)
	{
		Effect* effect = &effects[i];
		stream.write(reinterpret_cast<const char*>(&effect->id), sizeof(effect->id));
		stream.write(reinterpret_cast<const char*>(&effect->position.x), sizeof(effect->position.x));
		stream.write(reinterpret_cast<const char*>(&effect->position.y), sizeof(effect->position.y));
		stream.write(reinterpret_cast<const char*>(&effect->radius), sizeof(effect->radius));
		stream.write(reinterpret_cast<const char*>(&effect->current_time), sizeof(effect->current_time));
		stream.write(reinterpret_cast<const char*>(&effect->max_time), sizeof(effect->max_time));;
		stream.write(reinterpret_cast<const char*>(&effect->elevation), sizeof(effect->elevation));
		stream.write(reinterpret_cast<const char*>(&effect->sprite_id), sizeof(effect->sprite_id));
		stream.write(reinterpret_cast<const char*>(&effect->player), sizeof(effect->player));
		// should find a way on how to serialize thingy, since pointers won't match after save / load
		//stream.write(reinterpret_cast<const char*>(&effect->thingy), sizeof(effect->thingy));
	}

	u32 pariahHitCount = pariah_hits.size();
	for (s32 i = 0; i < pariahHitCount; i++)
	{
		stream.write(reinterpret_cast<const char*>(&pariah_hits[i].source), sizeof(pariah_hits[i].source));
		stream.write(reinterpret_cast<const char*>(&pariah_hits[i].target), sizeof(pariah_hits[i].target));
	}

	u32 apostleCount = reviving_units.size();
	for (s32 i = 0; i < apostleCount; i++)
	{
		stream.write(reinterpret_cast<const char*>(&reviving_units[i].energy), sizeof(reviving_units[i].energy));
		stream.write(reinterpret_cast<const char*>(&reviving_units[i].playerId), sizeof(reviving_units[i].playerId));
		stream.write(reinterpret_cast<const char*>(&reviving_units[i].position.x), sizeof(reviving_units[i].position.x));
		stream.write(reinterpret_cast<const char*>(&reviving_units[i].position.y), sizeof(reviving_units[i].position.y));
		stream.write(reinterpret_cast<const char*>(&reviving_units[i].timer), sizeof(reviving_units[i].timer));
		stream.write(reinterpret_cast<const char*>(&reviving_units[i].unit_id), sizeof(reviving_units[i].unit_id));
	}

	for (s32 i = 0; i < PLAYABLE_PLAYER_COUNT; i++)
	{
		for (s32 j = 0; j < ATTACK_LIMIT; i++)
		{
			stream.write(reinterpret_cast<char*>(&attackers.attacks[i][j]), sizeof(u16));
		}
	}

	u32 multihitCount = multihit_bullets.size();
	stream.write(reinterpret_cast<char*>(&multihitCount), sizeof(multihitCount));
	for (s32 i = 0; i < multihitCount; i++)
	{
		MultihitBullet bull = multihit_bullets[i];
		stream.write(reinterpret_cast<char*>(&bull.bullet), sizeof(bull.bullet));
		stream.write(reinterpret_cast<char*>(&bull.divider), sizeof(bull.divider));
		u32 alreadyHitCount = bull.already_hit.size();
		stream.write(reinterpret_cast<char*>(&alreadyHitCount), sizeof(alreadyHitCount));
		for (s32 j = 0; j < alreadyHitCount; j++)
		{
			Unit* hit = bull.already_hit[j];
			stream.write(reinterpret_cast<char*>(&hit), sizeof(hit));
		}
	}

	u32 lurkerMapSize = lurker_spine_hits.size();
	for (auto a : lurker_spine_hits)
	{
		Unit* key = a.first;
		stream.write(reinterpret_cast<char*>(&key), sizeof(key));
		u32 valueCount = a.second.size();
		stream.write(reinterpret_cast<char*>(&valueCount), sizeof(valueCount));
		for (s32 b = 0; b < valueCount; b++)
		{
			Unit* value = a.second[b];
			stream.write(reinterpret_cast<char*>(&value), sizeof(value));
		}
	}

	for (u32 i = 0; i < UNIT_ARRAY_LENGTH; ++i)
	{ 
		stream.write(reinterpret_cast<char*>(&unitExt[i].damage_receive_timer), sizeof(unitExt[i].damage_receive_timer));
		stream.write(reinterpret_cast<char*>(&unitExt[i].values), sizeof(unitExt[i].values));
		stream.write(reinterpret_cast<char*>(&unitExt[i].timers), sizeof(unitExt[i].timers));
		stream.write(reinterpret_cast<char*>(&unitExt[i].units), sizeof(unitExt[i].units));

		for (auto vec : unitExt[i].value_vectors)
		{
			auto vecSize = vec.size();
			stream.write(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.write(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}

		for (auto vec : unitExt[i].timer_vectors)
		{
			auto vecSize = vec.size();
			stream.write(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.write(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}

		for (auto vec : unitExt[i].unit_vectors)
		{
			auto vecSize = vec.size();
			stream.write(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.write(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}
	}

	output = stream.rdbuf()->str();
	if (output.length() == 0)
	{
		// return state;
		return;
		// error
	}

	saveState.ptr = output.c_str();
	saveState.len = output.size();
	return;
	// return state;
}

void Globals::aise_load(const char* ptr, u32 len)
{
	this->reset();
	return;

	if (len == 0 || ptr == 0)
	{
		return; // error
	}

	//scbw::print("Load....");
	std::stringstream stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	std::string str(ptr, len);
	stream << str;
	stream.read(reinterpret_cast<char*>(&extended_unit_table), sizeof(extended_unit_table));
	stream.read(reinterpret_cast<char*>(&extended_sfx), sizeof(extended_sfx));
	stream.read(reinterpret_cast<char*>(&tbl_extender), sizeof(tbl_extender));
	stream.read(reinterpret_cast<char*>(&unitid_extender), sizeof(unitid_extender));

	auto impactsSize = 0;
	stream.read(reinterpret_cast<char*>(&impactsSize), sizeof(impactsSize));
	for (s32 i = 0; i < impactsSize; i++)
	{
		Effect effect = {};
		stream.read(reinterpret_cast<char*>(&effect.id), sizeof(effect.id));
		stream.read(reinterpret_cast<char*>(&effect.position.x), sizeof(effect.position.x));
		stream.read(reinterpret_cast<char*>(&effect.position.y), sizeof(effect.position.y));
		stream.read(reinterpret_cast<char*>(&effect.radius), sizeof(effect.radius));
		stream.read(reinterpret_cast<char*>(&effect.current_time), sizeof(effect.current_time));
		stream.read(reinterpret_cast<char*>(&effect.max_time), sizeof(effect.max_time));
		stream.read(reinterpret_cast<char*>(&effect.elevation), sizeof(effect.elevation));
		stream.read(reinterpret_cast<char*>(&effect.sprite_id), sizeof(effect.sprite_id));
		stream.read(reinterpret_cast<char*>(&effect.player), sizeof(effect.player));
		// should find a way on how to serialize thingy, since pointers won't match after save / load
		//stream.read(reinterpret_cast<char*>(&effect.thingy), sizeof(effect.thingy));
		effects.push_back(effect);
	}

	u32 pariahHitsCount = 0;
	stream.read(reinterpret_cast<char*>(&pariahHitsCount), sizeof(pariahHitsCount));
	for (s32 i = 0; i < pariahHitsCount; i++)
	{
		PariahHit hit;
		stream.read(reinterpret_cast<char*>(&hit.source), sizeof(hit.source));
		stream.read(reinterpret_cast<char*>(&hit.target), sizeof(hit.target));
		pariah_hits.push_back(hit);
	}

	u32 apostleCount = 0;
	stream.read(reinterpret_cast<char*>(&apostleCount), sizeof(apostleCount));
	for (s32 i = 0; i < pariahHitsCount; i++)
	{
		RevivingUnit agent;
		stream.read(reinterpret_cast<char*>(&agent.energy), sizeof(agent.energy));
		stream.read(reinterpret_cast<char*>(&agent.playerId), sizeof(agent.playerId));
		stream.read(reinterpret_cast<char*>(&agent.position.x), sizeof(agent.position.x));
		stream.read(reinterpret_cast<char*>(&agent.position.y), sizeof(agent.position.y));
		stream.read(reinterpret_cast<char*>(&agent.timer), sizeof(agent.timer));
		stream.read(reinterpret_cast<char*>(&agent.unit_id), sizeof(agent.unit_id));
		reviving_units.push_back(agent);
	}

	for (s32 i = 0; i < PLAYABLE_PLAYER_COUNT; i++)
	{
		for (s32 j = 0; j < ATTACK_LIMIT; i++)
		{
			stream.write(reinterpret_cast<char*>(&attackers.attacks[i][j]), sizeof(u16));
		}
	}

	//u32 multihitCount = 0;
	//stream.read(reinterpret_cast<char*>(&multihitCount), sizeof(multihitCount));
	//scbw::printf("Hits: %d", multihitCount);
	/*
	for (s32 i = 0; i < multihitCount; i++) {
		MultihitBullet bull;
		stream.read(reinterpret_cast<char*>(&bull.bullet), sizeof(bull.bullet));
		stream.read(reinterpret_cast<char*>(&bull.divider), sizeof(bull.divider));
		u32 alreadyHitCount = 0;
		stream.read(reinterpret_cast<char*>(&alreadyHitCount), sizeof(alreadyHitCount));
		for (s32 j = 0; j < alreadyHitCount; j++) {
			Unit* hit;
			stream.read(reinterpret_cast<char*>(&hit), sizeof(hit));
			bull.already_hit.push_back(hit);
		}
		multihit_bullets.push_back(bull);
	}

	u32 lurkerMapSize = 0;
	stream.read(reinterpret_cast<char*>(&lurkerMapSize), sizeof(lurkerMapSize));
	for (s32 i = 0; i < lurkerMapSize; i++) {
		Unit* key = nullptr;
		stream.read(reinterpret_cast<char*>(&key), sizeof(key));
		u32 valueCount = 0;
		stream.read(reinterpret_cast<char*>(&valueCount), sizeof(valueCount));
		for (s32 j = 0; j < valueCount; j++) {
			Unit* value = nullptr;
			stream.read(reinterpret_cast<char*>(&value), sizeof(value));
			lurker_spine_hits[key].push_back(value);
		}
	}*/

	for (u32 i = 0; i < UNIT_ARRAY_LENGTH; ++i)
	{ 
		stream.read(reinterpret_cast<char*>(&unitExt[i].damage_receive_timer), sizeof(unitExt[i].damage_receive_timer));
		stream.read(reinterpret_cast<char*>(&unitExt[i].values), sizeof(unitExt[i].values));
		stream.read(reinterpret_cast<char*>(&unitExt[i].timers), sizeof(unitExt[i].timers));
		stream.read(reinterpret_cast<char*>(&unitExt[i].units), sizeof(unitExt[i].units));

		for (auto vec : unitExt[i].value_vectors)
		{
			auto vecSize = vec.size();
			stream.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.read(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}

		for (auto vec : unitExt[i].timer_vectors)
		{
			auto vecSize = vec.size();
			stream.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.read(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}

		for (auto vec : unitExt[i].unit_vectors)
		{
			auto vecSize = vec.size();
			stream.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.read(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}
	}
}

static void ClearUnitExt(int index)
{
	globals.unitExt[index] = {};
	// this should set combat timer to highest u32,
	// meaning every unit will start being out of combat
	globals.unitExt[index].damage_receive_timer = -1;
	globals.unitExt[index].damage_dealt_timer = -1;
}

void Globals::reset()
{
	effects.clear();
	pariah_hits.clear();
	multihit_bullets.clear();
	reviving_units.clear();
	lurker_spine_hits.clear();
	for (u32 i = 0; i < UNIT_ARRAY_LENGTH; ++i)
	{ 
		ClearUnitExt(i);
	}
}

void Globals::unitDeathHook(Unit* unit)
{
	for (u32 j = 0; j < UNIT_ARRAY_LENGTH; ++j)
	{ // set dead unit pointers to nullptr and erase them from unit vectors
		UnitExt* ext = &unitExt[j];

		for (u32 i = 0; i < UnitCount; ++i)
		{
			if (ext->units[i] == unit)
			{
				ext->units[i] = nullptr;
			}
		}

		for (u32 i = 0; i < UnitVectorCount; ++i)
		{
			auto& vec = ext->unit_vectors[i];
			vec.erase(std::remove_if(vec.begin(), vec.end(), [&](Unit* vecUnit)
			{
				return vecUnit == unit;
			}), vec.end());
		}
	}
	ClearUnitExt(unit->get_unit_index());

	for (s32 i = 0; i < this->effects.size(); i++)
	{
		Effect* effect = &effects[i];
		if (effect->source == unit)
		{
			effect->source = nullptr;
		}
		if (effect->origin == unit)
		{
			effect->origin = nullptr;
		}
	}

	lurker_spine_hits[unit].clear();
	for (auto& a : lurker_spine_hits)
	{
		for (s32 b = 0; b < a.second.size(); b++)
		{
			if (a.second[b] == unit)
			{
				a.second.erase(a.second.begin() + b);
				b--;
			}
		}
	}
}

void ApplyIrradiate(Unit* unit, u8 player, u32 time)
{
	if (time == 0)
	{
		return;
	}
	//if not already irradiated and not burrowed, add the irradiate overlay
	if (unit->irradiate_timer == 0 && !(unit->status & UnitStatus::Burrowed))
	{
		u32 irradiateImageId = ImageId::IrradiateNew_Small + scbw::get_overlay_offset(unit->id);
		Unit* unitWithOverlay;
		if (unit->subunit != nullptr)
		{
			unitWithOverlay = unit->subunit;
		}
		else
		{
			unitWithOverlay = unit;
		}
		unitWithOverlay->sprite->create_top_overlay(irradiateImageId, 0, 0, 0);
	}
	unit->irradiate_caster = nullptr;
	unit->irradiate_player_id = player;
	unit->irradiate_timer = time;
}

void Globals::create_effect(u8 id, u32 time, Vector32 position, u32 radius, s32 sprite_id, u8 elevation, u8 player, Unit* source, Unit* parent)
{
	Effect effect = {};
	effect.id = id;
	effect.position = position;
	effect.radius = radius;
	effect.current_time = time;
	effect.max_time = time;
	effect.sprite_id = sprite_id;
	effect.elevation = elevation;
	effect.player = player;
	effect.thingy = nullptr;
	effect.source = source;
	effect.origin = parent;

	if (effect.sprite_id >= 0) {
		Thingy* thingy = scbw::create_and_init_thingy(effect.sprite_id, effect.position.x, effect.position.y, player);
		if (!thingy) {
			return;
		}
		effect.thingy = thingy;
	}
	effects.push_back(effect);
}

void Globals::frame_hook() {
	for (u32 j = 0; j < UNIT_ARRAY_LENGTH; ++j) {
		UnitExt* ext = &unitExt[j];
		if (ext->damage_receive_timer != -1) { // don't increment past max value of u32
			++ext->damage_receive_timer;
		}
		if (ext->damage_dealt_timer != -1) { // don't increment past max value of u32
			++ext->damage_dealt_timer;
		}
		for (u32 i = 0; i < PlayerLimit; ++i) {
			if (ext->ai_timer_avoid_targeting[i]) {
				ext->ai_timer_avoid_targeting[i] -= 1;
			}
		}
		for (u32 i = 0; i < TimerCount; ++i) {
			if (ext->timers[i] > 0) {
				ext->timers[i]--;
			}
		}
		for (u32 i = 0; i < TimerVectorCount; ++i) {
			for (auto& timer : ext->timer_vectors[i]) {
				if (timer > 0) {
					timer--;
				}
			}
			ext->timer_vectors[i].erase(std::remove_if(ext->timer_vectors[i].begin(), ext->timer_vectors[i].end(), [&](u32& timer) {
				return timer == 0;
			}), ext->timer_vectors[i].end());
		}
	}

	// Lazarus Agent and Counter-Proposal
	for (s32 i = 0; i < reviving_units.size(); ++i) {
		auto& revivingUnit = reviving_units[i];
		if (revivingUnit.timer > 0) {
			revivingUnit.timer--;
		} else {
			if (playerTable[revivingUnit.playerId].type != PlayerType::Computer || revivingUnit.town || revivingUnit.ignore_town) {
				auto revived = scbw::create_unit_in_game(revivingUnit.unit_id, revivingUnit.playerId, revivingUnit.position.x, revivingUnit.position.y, nullptr, false);
				if (revived) {
					UnitExt* revivedExt = globals.get_unit_ext(revived);
					revived->hitpoints = 1;
					revived->shields = 0;
					revived->healOverflow(revivingUnit.hp);
					revived->energy = revivingUnit.energy;
					revived->status |= UnitStatus::Invincible | UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack;
					revived->play_iscript_animation(IscriptAnimation::Unused1);
					revivedExt->values[ValueReviveStarted] = 1;
					revivedExt->values[ValueReviveTownPointer] = (u32)revivingUnit.town;
					revivedExt->timers[TimerReviveFlagClearMin] = 3;
					revivedExt->timers[TimerReviveFlagClearMax] = 14;
				}
			}
			reviving_units.erase(reviving_units.begin() + i);
			i--;
		}
	}

	for (s32 i = 0; i < effects.size(); i++) {
		Effect* effect = &effects[i];
		effect->current_time--;
		if (effect->origin) {
			effect->position = {effect->origin->position.x, effect->origin->position.y};
			if (effect->thingy)
			{
				effect->thingy->sprite->setPosition(effect->origin->position.x, effect->origin->position.y);
			}
		}
		const s32 framePeriod = 6;
		if (effect->current_time % framePeriod == 1) {
			const f64 damageMultiplier = 256.0 * (f64)framePeriod / effect->max_time;
			switch (effect->id) {
				case EffectId_StarSovereignTheGriefOfAllGods: {
					if (!effect->origin) {
						effect->current_time = 0;
						return;
					}

					scbw::UnitFinder(
						effect->position.x - effect->radius,
						effect->position.y - effect->radius,
						effect->position.x + effect->radius,
						effect->position.y + effect->radius
					).for_each_in_match(cosmonarchy_finder_match(nullptr, Flags::NoAir, {}), [&](Unit* unit) {
						unit->damage_with_custom(2500 * damageMultiplier, DamageType::Chronal, effect->source, effect->player, 0, 1, true);
					});
				} break;
				case EffectId_SalamanderFire: {
					EntryArray unit_array = unit_search_radial({(u16)effect->position.x, (u16)effect->position.y}, effect->radius);
					unit_search_filter(&unit_array, nullptr, FilterNoAir);

					for (u32 j = 0; j < unit_array.size; j++) {
						Unit* target = unit_array.units[j];
						target->damage_with_custom(20 * damageMultiplier, DamageType::Concussive, effect->source, effect->player, 0, 1, true);
					}
				} break;
				case EffectId_Criticality: {
					scbw::UnitFinder(
						effect->position.x - effect->radius,
						effect->position.y - effect->radius,
						effect->position.x + effect->radius,
						effect->position.y + effect->radius
					).for_each_in_match(cosmonarchy_finder_match(nullptr, Flags::NoAir, {}), [&](Unit* unit) {
						if (scbw::get_distance_fast(effect->position.x, effect->position.y, unit->get_x(), unit->get_y()) > effect->radius)
						{
							return;
						}
						unit->healUnderflow(2 * 256 / framePeriod, effect->source, true); // this math seems wrong
					});
				} break;
				case EffectId_CausticDischarge: {
					scbw::UnitFinder(
						effect->position.x - effect->radius,
						effect->position.y - effect->radius,
						effect->position.x + effect->radius,
						effect->position.y + effect->radius
					).for_each_in_match(cosmonarchy_finder_match(nullptr, Flags::NoAir, {}), [&](Unit* unit) {
						if (scbw::get_distance_fast(effect->position.x, effect->position.y, unit->get_x(), unit->get_y()) > effect->radius) {
							return;
						}
						UnitExt* unitExt = globals.get_unit_ext(unit);
						unit->damage_with_custom(30 * damageMultiplier, DamageType::Impact, effect->source, effect->player, 0, 1, true);
						unitExt->timers[TimerCausticDischargeSpeed] = 5 * 24 + 1;
					});
				} break;
				case EffectId_IonField: {
					scbw::UnitFinder(
						effect->position.x - effect->radius,
						effect->position.y - effect->radius,
						effect->position.x + effect->radius,
						effect->position.y + effect->radius
					).for_each_in_match(cosmonarchy_finder_match(nullptr, Flags::NoAir, {}), [&](Unit* unit) {
						if (scbw::get_distance_fast(effect->position.x, effect->position.y, unit->get_x(), unit->get_y()) > effect->radius) {
							return;
						}
						UnitExt* unitExt = globals.get_unit_ext(unit);
						unit->damage_with_custom(100 * damageMultiplier, DamageType::Chronal, effect->source, effect->player, 0, 1, true);
						unitExt->timers[TimerIonFieldSpeed] = 2 * 24 + 1;
					});
				} break;
				case EffectId_Hallucination: {
					scbw::UnitFinder(
						effect->position.x - effect->radius,
						effect->position.y - effect->radius,
						effect->position.x + effect->radius,
						effect->position.y + effect->radius
					).for_each_in_match(cosmonarchy_finder_match(nullptr, 0, {}), [&](Unit* unit) {
						if (scbw::get_distance_fast(effect->position.x, effect->position.y, unit->get_x(), unit->get_y()) > effect->radius) {
							return;
						}
						UnitExt* unitExt = globals.get_unit_ext(unit);
						if (!unitExt->timers[TimerHallucinating]) {
							scbw::apply_new_hallucination(unit, effect->player, effect->current_time);
						} else {
							unitExt->timers[TimerHallucinating] = effect->current_time;
						}
					});
				} break;
				case EffectId_Irradiate: {
					scbw::UnitFinder(
						effect->position.x - effect->radius,
						effect->position.y - effect->radius,
						effect->position.x + effect->radius,
						effect->position.y + effect->radius
					).for_each_in_match(cosmonarchy_finder_match(nullptr, 0, {}), [&](Unit* unit) {
						if (scbw::get_distance_fast(effect->position.x, effect->position.y, unit->get_x(), unit->get_y()) > effect->radius) {
							return;
						}
						ApplyIrradiate(unit, effect->player, 15);
					});
				} break;
				case EffectId_GroundZero: {
					const u32 delta = effect->max_time - effect->current_time;
					const u32 stage = delta / framePeriod;

					switch (stage) {
						case 0: {
							globals.create_effect(EffectId_Irradiate, 20 * 24 - delta, effect->position, 1.5 * 32, SpriteId::IrradiateNew_Large, 5, effect->player, effect->source);
						} break;
						case 1:
						case 2: {
							f32 d = stage * 2 * 32;
							u32 count = stage * 8;

							Vector32 newPos = effect->position;
							newPos.x += d;
							for (u32 j = 0; j < count; ++j) {
								newPos = scbw::rotate_point(effect->position, newPos, (360.0 / count));
								globals.create_effect(EffectId_Irradiate, 20 * 24 - delta, newPos, 1.5 * 32, SpriteId::IrradiateNew_Large, 5, effect->player, effect->source);
							}
						} break;
					}
				} break;
				case EffectId_DisruptionWeb: {
					if (!effect->origin) {
						effect->current_time = 0;
						break;
					}

					UnitExt* parentExt = globals.get_unit_ext(effect->origin);
					if (!parentExt->values[ValueDisruptionFieldOn]) {
						effect->current_time = 0;
						break;
					}

					scbw::UnitFinder(
						effect->position.x - 60,
						effect->position.y - 40,
						effect->position.x + 60,
						effect->position.y + 40
					).for_each_in_match(cosmonarchy_finder_match(nullptr, 0, {}), [&](Unit* unit) {
						UnitExt* unitExt = globals.get_unit_ext(unit);
						unitExt->timers[TimerUnderDisruptionWeb] = 8;
						unit->status |= UnitStatus::CanNotAttack;
					});
				} break;
				case EffectId_PlagueField: {
					scbw::UnitFinder(
						effect->position.x - 1.5 * 32,
						effect->position.y - 1 * 32,
						effect->position.x + 1.5 * 32,
						effect->position.y + 1 * 32
					).for_each([&](Unit* unit) {
						helper::PlagueProc(unit, 0);
					});
				} break;
			}
		}
		if (effect->current_time == 0) {
			if (effect->thingy && effect->thingy->sprite) {
				effect->thingy->sprite->play_iscript_animation(IscriptAnimation::Death);
			}

			effects.erase(effects.begin() + i);
			i--;
		}
	}
}

void Globals::remove_unit_from_pariah_table(Unit* unit, bool sourceOnly)
{
	for (s32 i = 0; i < pariah_hits.size(); i++)
	{
		if (pariah_hits[i].source == unit || (pariah_hits[i].target == unit && !sourceOnly))
		{
			pariah_hits.erase(pariah_hits.begin() + i);
			i--;
		}
	}
}

void Globals::remove_multihit_bullet(Bullet* bullet)
{
	for (s32 i = 0; i < multihit_bullets.size(); i++)
	{
		if (multihit_bullets[i].bullet == bullet)
		{
			multihit_bullets.erase(multihit_bullets.begin() + i);
			i--;
		}
	}
}

bool Globals::was_hit_by_pariah(Unit* source, Unit* target)
{
	for (auto a : pariah_hits)
	{
		if (source == a.source && target == a.target)
		{
			return true;
		}
	}
	return false;
}

void Globals::register_pariah_hit(Unit* source, Unit* target)
{
	PariahHit hit;
	hit.source = source;
	hit.target = target;
	pariah_hits.push_back(hit);
}

bool Globals::was_already_hit_by_multihit_bullet(Bullet* bullet, Unit* target)
{
	for (auto& a : multihit_bullets)
	{
		if (bullet == a.bullet)
		{
			for (auto b : a.already_hit)
			{
				if (b == target)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool Globals::multihit_bullet_is_set(Bullet* bullet)
{
	for (auto& a : multihit_bullets)
	{
		if (bullet == a.bullet)
		{
			return true;
		}
	}
	return false;
}

void Globals::add_bullet_multihit(Bullet* bullet, Unit* target)
{
	for (auto& a : multihit_bullets)
	{
		if (a.bullet == bullet)
		{
			a.already_hit.push_back(target);
			return;
		}
	}
	MultihitBullet gb(bullet, {target}, 0);
	multihit_bullets.push_back(gb);
}

void Globals::add_incremental_bullet_multihit(Bullet* bullet, Unit* target)
{
	for (auto& a : multihit_bullets)
	{
		if (a.bullet == bullet)
		{
			a.already_hit.push_back(target);
			return;
		}
	}
	MultihitBullet gb(bullet, {target}, 1);
	multihit_bullets.push_back(gb);
}

s32 Globals::get_divider(Bullet* bullet)
{
	for (auto& a : multihit_bullets)
	{
		if (a.bullet == bullet && a.divider != 0)
		{
			return a.divider;
		}
	}
	return 1;
}

u32 Globals::get_multihit_count(Bullet* bullet)
{
	for (auto& a : multihit_bullets)
	{
		if (a.bullet == bullet)
		{
			return a.already_hit.size();
		}
	}
	return 0;
}

void Globals::add_multihit_empty_bullet(Bullet* bullet, s32 divider)
{
	for (auto& a : multihit_bullets)
	{
		if (a.bullet == bullet)
		{
			a.divider = divider;
			return;
		}
	}
	MultihitBullet gb(bullet, {}, divider);
	multihit_bullets.push_back(gb);
}

void Globals::initRevive(s32 x, s32 y, u8 player, u32 unit_id, u32 energy, u32 hp, AiTown* town)
{
	RevivingUnit revivingUnit = {};
	revivingUnit.playerId = player;
	revivingUnit.position.x = x;
	revivingUnit.position.y = y;
	revivingUnit.timer = 48;
	revivingUnit.unit_id = unit_id;
	revivingUnit.energy = energy;
	revivingUnit.hp = hp;
	if (units_dat::BaseProperty[unit_id] & (UnitProperty::Worker | UnitProperty::Building) || unit_id == UnitId::ZergOvileth)
	{
		revivingUnit.town = town;
	}
	else
	{
		revivingUnit.ignore_town = true;
	}
	reviving_units.push_back(revivingUnit);
}

void Globals::clear_lakizilisk(Unit* lurker)
{
	lurker_spine_hits[lurker].clear();
}

void Globals::add_lurker_spine_pair(Unit* lurker, Unit* target)
{
	lurker_spine_hits[lurker].push_back(target);
}

bool Globals::spine_pair_exists(Unit* lurker, Unit* target)
{
	for (auto a : lurker_spine_hits)
	{
		for (auto b : a.second)
		{
			if (a.first == lurker && b == target)
			{
				return true;
			}
		}
	}
	return false;
}

void __declspec(naked) Globals_SaveStateWrapper()
{
	static SaveState* state;
	__asm {
		PUSHAD
	}
	//	state = &globals.aise_save();
	globals.aise_save();
	state = &saveState;
	__asm {
		POPAD
		MOV EAX, state
		RETN
	}
}

void __declspec(naked) Globals_LoadStateWrapper()
{
	static const char* pointer;
	static u32 len;
	__asm {
		MOV len, ECX
		MOV pointer, EAX
		PUSHAD
	}
	globals.aise_load(pointer, len);
	__asm {
		POPAD
		RETN
	}
}

void injectGlobalsHooks()
{
	jump_patch(Globals_SaveStateWrapper, 0x004AAF23);
	jump_patch(Globals_LoadStateWrapper, 0x004AAE94);
	// const u8 retn = 0xc3;
	// memoryPatch(0x004AAF23 + 5, &retn, 1);
	// memoryPatch(0x004AAE94 + 5, &retn, 1);
}

namespace scbw
{
	void refreshDebuffs(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		// NOTE: classic period formula is (([real seconds / 0.042) / 8)
		// e.g. (([15] / 0.042) / 8) = 44.642, rounded up to 45

		// Refresh Ensnare
		if (unit->ensnare_timer)
		{
			unit->ensnare_timer = MatralethEnsnareTimer;
			// specific update following ensnare removal
			unit->update_speed();
		}

		// Refresh Plague
		if (unit->plague_timer)
		{
			unit->plague_timer = IsthrathalethPlagueTimer;
		}

		// Refresh Observance
		if (unitExt->timers[TimerObservanceDebuff])
		{
			unitExt->timers[TimerObservanceDebuff] = SeraphObservanceDebuffTimer;
		}

		// Refresh Hallucination
		if (unitExt->timers[TimerHallucinating])
		{
			unitExt->timers[TimerHallucinating] = CantavisHallucinationTimer;
		}

		// Refresh Captivating Claws
		if (unitExt->timers[TimerCaptivatingClaws])
		{
			unitExt->timers[TimerCaptivatingClaws] = VorvrokorCaptivatingClawsTimer;
		}

		// Refresh Lockdown
		if (unit->lockdown_timer)
		{
			unit->lockdown_timer = LobotomyMineLockdownTimer;
		}

		// Refresh Maelstrom
		if (unit->maelstrom_timer)
		{
			unit->maelstrom_timer = MindTyrantMaelstromTimer;
		}

		// Refresh Devourer Corrosive Spores
		if (unitExt->timers[TimerCorrosiveTouch])
		{
			unitExt->timers[TimerCorrosiveTouch] = VithriliskCorrosiveTouchTimer;
		}

		// Refresh Irradiate
		if (unit->irradiate_timer)
		{
			unit->irradiate_timer = SeraphIrradiateTimer;
		}

		// Refresh Dirge
		if (unitExt->timers[TimerDirgeFuse])
		{
			unitExt->timers[TimerDirgeFuse] = MadrigalDirgeTimer;
		}

		// Refresh Hierophant Signify
		if (unitExt->timers[TimerSignify])
		{
			unitExt->timers[TimerSignify] = 3 * 24 + 1;
		}

		//was hardcoded in original code
		scbw::refresh_console();
	}

	void inherit_debuffs(Unit* parent, Unit* child)
	{
		UnitExt* parentExt = globals.get_unit_ext(parent);
		UnitExt* childExt = globals.get_unit_ext(child);

		// NOTE: classic period formula is (([real seconds / 0.042) / 8)
		// e.g. (([15] / 0.042) / 8) = 44.642, rounded up to 45

		// Get overlay info
		s32 overlayOffset = get_overlay_offset(child->id);

		// Inherit Ensnare
		if (parent->ensnare_timer != 0)
		{
			child->ensnare_timer = parent->ensnare_timer;
			if (!(child->sprite->get_overlay(ImageId::EnsnareOverlay_Small + overlayOffset)))
			{
				child->sprite->create_overlay(ImageId::EnsnareOverlay_Small + overlayOffset);
			}
			child->update_speed();
		}

		// Inherit Plague
		if (parent->plague_timer != 0)
		{
			child->plague_timer = parent->plague_timer;
			if (!(child->sprite->get_overlay(ImageId::PlagueOverlay_Small + overlayOffset)))
			{
				child->sprite->create_overlay(ImageId::PlagueOverlay_Small + overlayOffset);
			}
		}

		// Inherit Parasite
		if (parentExt->units[UnitParasiteSource] != nullptr)
		{
			childExt->units[UnitParasiteSource] = parentExt->units[UnitParasiteSource];
		}

		// Inherit armor rend (generic)
		if (parentExt->values[ValueArmorReduce] > 0)
		{
			childExt->values[ValueArmorReduce] = parentExt->values[ValueArmorReduce];
		}

		// Inherit Observance
		if (parentExt->timers[TimerObservanceDebuff] > 0)
		{
			childExt->timers[TimerObservanceDebuff] = parentExt->timers[TimerObservanceDebuff];
			childExt->timers[TimerObservancePeriod] = parentExt->timers[TimerObservancePeriod];
			childExt->values[ValueObservanceRend] = parentExt->values[ValueObservanceRend];
		}

		// Inherit Knockout Drivers
		if (parentExt->timers[TimerPetrification] > 0)
		{
			childExt->timers[TimerPetrification] = parentExt->timers[TimerPetrification];
		}

		// Inherit Hallucination
		if (parentExt->timers[TimerHallucinating]  > 0)
		{
			childExt->timers[TimerHallucinating] = parentExt->timers[TimerHallucinating];
		}

		// Inherit Captivating Claws
		if (auto timer = parentExt->timers[TimerCaptivatingClaws])
		{
			childExt->timers[TimerCaptivatingClaws] = timer;
			parentExt->unit_vectors[UnitVectorCaptivatingClaws] = childExt->unit_vectors[UnitVectorCaptivatingClaws];
		}

		// Inherit Lobotomy Mine slow
		if (parentExt->timers[TimerLobotomySlow])
		{
			childExt->timers[TimerLobotomySlow] = parentExt->timers[TimerLobotomySlow];
		}

		// Inherit Lockdown
		if (parent->lockdown_timer != 0)
		{
			child->lockdown_timer = parent->lockdown_timer;
		}

		// Inherit Maelstrom
		if (parent->maelstrom_timer != 0)
		{
			child->maelstrom_timer = parent->maelstrom_timer;
		}

		// Inherit Devourer Corrosive Spores
		if (parentExt->timers[TimerCorrosiveTouch])
		{
			childExt->values[ValueCorrosiveTouchStacks] = parentExt->values[ValueCorrosiveTouchStacks];
			childExt->timers[TimerCorrosiveTouch] = parentExt->timers[TimerCorrosiveTouch];
		}

		// Inherit Irradiate
		if (parent->irradiate_timer != 0)
		{
			child->irradiate_timer = parent->irradiate_timer;
		}

		// Inherit Defensive Matrix
		if (parent->defensive_matrix_timer > 0)
		{
			child->defensive_matrix_hp = parent->defensive_matrix_hp;
			child->defensive_matrix_timer = parent->defensive_matrix_timer;
		}

		// Inherit Stasis Field
		if (parent->stasis_timer > 0)
		{
			child->stasis_timer = parent->stasis_timer;
		}

		// Inherit Deconstruction
		if (parentExt->values[ValueDeconstruction])
		{
			childExt->values[ValueDeconstruction] = parentExt->values[ValueDeconstruction];
			Sprite* sprite = child->sprite;
			if (child->subunit != nullptr)
			{
				UnitExt* subunitExt = globals.get_unit_ext(child->subunit);
				subunitExt->values[ValueDeconstruction] = parentExt->values[ValueDeconstruction];
				sprite = child->subunit->sprite;
			}
			if (!sprite->get_overlay(ImageId::DeconstructionSmall + overlayOffset))
			{
				sprite->create_overlay(ImageId::DeconstructionSmall + overlayOffset);
			}
		}

		// Inherit Entropy Gauntlets
		if (parentExt->values[ValueEntropyGauntlets])
		{
			childExt->values[ValueEntropyGauntlets] = parentExt->values[ValueEntropyGauntlets];
			if (!(child->sprite->get_overlay(ImageId::EntropyGauntletsOverlaySmall + overlayOffset)))
			{
				child->sprite->create_overlay(ImageId::EntropyGauntletsOverlaySmall + overlayOffset);
			}
		}

		// Inherit Sublime Shepherd
		if (parentExt->values[ValueSublimeShepherd])
		{
			childExt->values[ValueSublimeShepherd] = parentExt->values[ValueSublimeShepherd];
		}

		// Inherit Terminal Surge
		if (parentExt->values[ValueTerminalSurge])
		{
			childExt->values[ValueTerminalSurge] = parentExt->values[ValueTerminalSurge];
		}

		// Inherit Stellar Enforcement
		if (parentExt->values[ValueStellarEnforcement])
		{
			childExt->values[ValueStellarEnforcement] = parentExt->values[ValueStellarEnforcement];
		}

		// Inherit Adrenal Frenzy
		if (parentExt->timers[TimerNathrokorBuff])
		{
			childExt->timers[TimerNathrokorBuff] = parentExt->timers[TimerNathrokorBuff];
			childExt->values[ValueNathrokorBuffStacks] = parentExt->values[ValueNathrokorBuffStacks];
		}

		// Inherit Swarming Omen
		if (parentExt->timers[TimerSwarmingOmen])
		{
			childExt->values[ValueSwarmingOmenStatus] = parentExt->values[ValueSwarmingOmenStatus];
			childExt->timers[TimerSwarmingOmen] = parentExt->timers[TimerSwarmingOmen];
		}

		// Inherit Kaiser Rampage
		if (parentExt->timers[TimerKaiserRampage])
		{
			childExt->timers[TimerKaiserRampage] = parentExt->timers[TimerKaiserRampage];
		}

		// Inherit Kelvin Munitions
		if (parentExt->values[ValueKelvinMunitionsSlow])
		{
			childExt->values[ValueKelvinMunitionsSlow] = parentExt->values[ValueKelvinMunitionsSlow];
		}

		// Inherit Dirge
		if (parentExt->timers[TimerDirgeFuse])
		{
			childExt->timers[TimerDirgeFuse] = parentExt->timers[TimerDirgeFuse];
			childExt->values[ValueDirgeStacks] = parentExt->values[ValueDirgeStacks];
		}

		// Inherit Lazarus Agent
		if (parentExt->values[ValueLazarusAgent])
		{
			childExt->values[ValueLazarusAgent] = parentExt->values[ValueLazarusAgent];
		}

		// Inherit Empress Passive Stacks
		if (parentExt->values[ValueEmpressPassiveStacks])
		{
			childExt->values[ValueEmpressPassiveStacks] = parentExt->values[ValueEmpressPassiveStacks];
			childExt->values[ValueEmpressSourcePlayer] = parentExt->values[ValueEmpressSourcePlayer];
		}

		// Inherit Anticthon
		childExt->values[ValueAnticthonStacks] = parentExt->values[ValueAnticthonStacks];

		// Inherit Hierophant Signify
		childExt->timers[TimerSignify] = parentExt->timers[TimerSignify];
		childExt->values[ValueSignifyStacks] = parentExt->values[ValueSignifyStacks];
		childExt->value_vectors[ValueVectorSignifySourcePlayer] = parentExt->value_vectors[ValueVectorSignifySourcePlayer];
		scbw::update_signify_status(child, childExt);

		scbw::refresh_console();
	}

	// TODO: update
	bool isDebuffed(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);
		return unitExt->timers[TimerMaledictionSlow]
			|| unitExt->timers[TimerPetrification]
			|| unitExt->timers[TimerLobotomySlow]
			|| unitExt->timers[TimerObservancePeriod]
			|| unitExt->timers[TimerCorrosiveTouch]
			|| unitExt->values[ValueDeconstruction]
			|| unitExt->values[ValueEntropyGauntlets]
			|| unitExt->values[ValueArmorReduce]
			|| unitExt->values[ValueKelvinMunitionsSlow]
			|| unitExt->values[ValueSignifyStacks]
			|| captivating_claws_get_timer(unit) > 0
			|| unit->irradiate_timer != 0
			|| unit->ensnare_timer != 0
			|| unit->lockdown_timer != 0
			|| unit->maelstrom_timer != 0
			|| unit->plague_timer != 0
			|| unit->parasite_flags
			|| unit->is_blind;
	}

	void restore_func(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		Unit* overlayTargetUnit = unit;
		if (unit->subunit != nullptr)
		{
			overlayTargetUnit = unit->subunit;
		}
		u32 overlayImageId = ImageId::RestorationHit_Small + scbw::get_overlay_offset(unit->id);;
		(overlayTargetUnit->sprite)->create_top_overlay(overlayImageId, 0, 0, 0);

		// remove status effects and corresponding overlays

		// Remove Parasite
		if (unitExt->timers[TimerParasiteArm])
		{
			unitExt->timers[TimerParasiteArm] = 0;
			unit->remove_overlay(ImageId::ParasiteOverlay);
		}
		unitExt->units[UnitParasiteSource] = nullptr;

		// Remove default flags
		unit->parasite_flags = 0;
		unit->is_blind = 0;

		// Remove Ensnare
		if (unit->ensnare_timer != 0)
		{
			unit->remove_overlay(ImageId::EnsnareOverlay_Small, ImageId::EnsnareOverlay_Large);
			unit->ensnare_timer = 0;
			//specific update following ensnare removal
			unit->update_speed();
		}

		// Remove Plague
		if (unit->plague_timer != 0)
		{
			unit->remove_overlay(ImageId::PlagueOverlay_Small, ImageId::PlagueOverlay_Large);
			unit->plague_timer = 0;
		}

		// Remove Irradiate
		if (unit->irradiate_timer != 0)
		{
			unit->remove_overlay(ImageId::IrradiateNew_Small, ImageId::IrradiateNew_Large);
			unit->irradiate_timer = 0;

			//specific updates following irradiate removal
			unit->irradiate_caster = nullptr;
			unit->irradiate_player_id = 8;
		}

		// Remove Malediction
		if (unitExt->timers[TimerMaledictionSlow])
		{
			unitExt->timers[TimerMaledictionSlow] = 1;
		}

		// Remove Observance
		if (unitExt->timers[TimerObservanceDebuff])
		{
			unitExt->timers[TimerObservanceDebuff] = 1;
		}

		// Remove Knockout Drivers
		if (unitExt->timers[TimerPetrification])
		{
			unitExt->timers[TimerPetrification] = 1;
		}

		// Remove Hallucination
		if (unitExt->timers[TimerHallucinating])
		{
			unitExt->timers[TimerHallucinating] = 1;
		}

		// Remove Captivating Claws
		if (unitExt->timers[TimerCaptivatingClaws])
		{
			unitExt->timers[TimerCaptivatingClaws] = 1;
			unitExt->unit_vectors[UnitVectorCaptivatingClaws] = {};
		}

		// Remove Lobotomy Mine slow
		if (unitExt->timers[TimerLobotomySlow])
		{
			unitExt->timers[TimerLobotomySlow] = 1;
		}

		// Remove Entropy Gauntlets
		if (unitExt->values[ValueEntropyGauntlets])
		{
			unitExt->values[ValueEntropyGauntlets] = 0;
			u32 overlayOffset = scbw::get_overlay_offset(unit->id);
			u32 overlayImageId = ImageId::EntropyGauntletsOverlaySmall - overlayOffset;
			if (unit->subunit != nullptr)
			{
				unit->subunit->sprite->remove_overlay(overlayImageId);
			}
			else
			{
				unit->sprite->remove_overlay(overlayImageId);
			}
		}

		// Remove armor rend (generic)
		unitExt->values[ValueArmorReduce] = 0;

		// Remove Controlled Opposition
		unitExt->values[ValueControlledOpposition] = 0;

		// Remove Empress stacks
		unitExt->values[ValueEmpressPassiveStacks] = 0;

		// Remove Lockdown
		unit->remove_lockdown();

		// Remove Maelstrom
		unit->remove_maelstrom();

		// Remove Devourer Corrosive Spores
		unit->removeAcidSpores();

		// Remove Deconstruction
		unitExt->values[ValueDeconstruction] = 0;
		unit->remove_overlay(ImageId::DeconstructionSmall, ImageId::DeconstructionLarge);
		if (unit->subunit)
		{
			UnitExt* subunitExt = globals.get_unit_ext(unit->subunit);
			subunitExt->values[ValueDeconstruction] = 0;
			unit->subunit->remove_overlay(ImageId::DeconstructionSmall, ImageId::DeconstructionLarge);
		}

		// Remove Kelvin Munitions
		if (unitExt->values[ValueKelvinMunitionsSlow])
		{
			unitExt->values[ValueKelvinMunitionsSlow] = 0;
		}

		// Remove Dirge
		if (unitExt->timers[TimerDirgeFuse])
		{
			unitExt->timers[TimerDirgeFuse] = 0;
			unitExt->values[ValueDirgeStacks] = 0;
		}

		// Remove Hierophant Signify
		unitExt->timers[TimerSignify] = 0;
		unitExt->values[ValueSignifyStacks] = 0;
		unitExt->value_vectors[ValueVectorSignifySourcePlayer] = {};
		scbw::update_signify_status(unit, unitExt);

		// Remove Nathrokor attack stacks
		for (u32 i = 0; i < 8; i++)
		{
			unitExt->nathrokor_target_stacks[i] = 0;
		}

		scbw::refresh_console();
	}

	void applyParasite(Unit* unit, u8 playerID, Unit* queen)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		if (!scbw::can_be_parasited(unit))
		{
			return;
		}
		s32 playerFlag = 1 << playerID;
		if (unit->parasite_flags != playerFlag)
		{ // <- Prevent restarting arm period
			unitExt->timers[TimerParasiteArm] = DefaultArmParasiteTimer;
			unit->sprite->create_overlay(ImageId::ParasiteOverlay, 0, 0, 0);
		}
		unitExt->units[UnitParasiteSource] = queen; // TODO: remove?
		unit->parasite_flags = playerFlag; // Parasite now overwrites other player's parasite
		scbw::refresh_console();
	}

	bool isSlowed(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		if (unit->ensnare_timer)
		{
			return true;
		}

		if (unitExt->timers[TimerSignify])
		{
			return true;
		}

		if (unitExt->timers[TimerPetrification])
		{
			return true;
		}

		if (unitExt->timers[TimerCaptivatingClaws])
		{
			return true;
		}

		if (unitExt->values[ValueKelvinMunitionsSlow])
		{
			return true;
		}

		return false;
	}

	Unit** getSourceQueens(Unit* unit) // TODO: remove?
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		Unit* queens[2] = {nullptr};
		if (unit == nullptr)
		{
			return queens; // <- prevents crash from incubators, but does not fix the issue
		}
		queens[0] = unitExt->units[UnitBroodlingMother];
		queens[1] = unitExt->units[UnitParasiteSource];
		return queens;
	}

	void doDeconstructionDamage(Unit* unit, u16 weaponId, bool playSound)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		// Deconstruction - Pazuzu passive
		f64 selfDamage = (unitExt->values[ValueDeconstruction] * 0.05 * (f64)(weapons_dat::DamageAmount[weaponId] * 256.0));
		auto deconstructionTarget = unit;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Subunit)
		{
			deconstructionTarget = unitExt->units[UnitTurretParentLink]; // TODO: port as unit
			if (deconstructionTarget == nullptr)
			{
				return;
			}
		}
		hooks::weaponDamageHook(selfDamage, deconstructionTarget, weaponId, deconstructionTarget, deconstructionTarget->player, 0, 1);
		if (playSound)
		{
			helper::play_sound_from_unit(1372, unit, 1, 0);
		}
	}

	Bullet* setMissileSourcePos(Unit* unit, u32 weaponId, u32 valueId, s32 rotateCount, u32 overlayType, u32 frameset_offset, u32 fake_amount)
	{
		if (unit->sprite == nullptr || unit->order_target.unit == nullptr)
		{
			return nullptr;
		}
		UnitExt* unitExt = globals.get_unit_ext(unit);
		auto rotate = unitExt->values[valueId];
		const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::get_image_data(overlayType, unit->sprite->main_graphic->id));
		u8 frameDirection = unit->sprite->main_graphic->direction;
		Point8 offset = loFile->get_offset(frameDirection + (17 * frameset_offset), rotate);
		if (unit->sprite->main_graphic->flags & ImageFlags::Mirrored)
		{
			offset.x = -offset.x;
		}
		rotate++;
		if (rotate > rotateCount - 1)
		{
			rotate = 0;
		}
		unitExt->values[valueId] = rotate;
		auto X = unit->get_x() + offset.x;
		auto Y = unit->get_y() + offset.y;
		Bullet* bullet = helper::create_bullet(weaponId, unit, X, Y, unit->player, scbw::get_angle(X, Y, unit->order_target.unit->position.x, unit->order_target.unit->position.y));
		if (bullet && unitExt->values[valueId] < fake_amount)
		{
			bullet->hitFlags |= 0x1;
		}
		return bullet;
	}

	const u32 Func_RemoveOverlays2 = 0x00492370;
	void removeoverlays2(u32 from, u32 to, Unit* unit)
	{
		__asm {
			PUSHAD
			MOV EAX, from
			MOV EBX, unit
			PUSH to
			CALL Func_RemoveOverlays2
			POPAD
		}
	}

	void clearClarionLinks(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		auto& vec = unitExt->unit_vectors[UnitVectorPhaseLink];
		for (Unit* vecUnit : vec)
		{
			UnitExt* vecUnitExt = globals.get_unit_ext(vecUnit);
			auto& clarions = vecUnitExt->unit_vectors[UnitVectorPhaseLinkClarions];
			clarions.erase(std::remove_if(clarions.begin(), clarions.end(),
			[&](Unit* target)
			{
				return target == unit;
			}), clarions.end());
		}
		vec.clear();
		unitExt->unit_vectors[UnitVectorPhaseLinkClarions].clear();
	}

	void placeClarionLinks(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);
		auto& link = unitExt->unit_vectors[UnitVectorPhaseLink];

		clearClarionLinks(unit);
		const s32 radius = 3 * 32;
		scbw::UnitFinder(
			unit->get_x() - radius,
			unit->get_y() - radius,
			unit->get_x() + radius,
			unit->get_y() + radius
		).filter([&](Unit* target)
		{
			if (!scbw::is_allied_to(unit->player, target->player))
			{
				return false;
			}
			if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
			{
				return false;
			}
			if (!target->shieldsEnabled())
			{
				return false;
			}
			if (target->shields < 256)
			{
				return false;
			}
			if (scbw::get_distance_fast(target->get_x(), target->get_y(), unit->get_x(), unit->get_y()) > radius)
			{
				return false;
			}
			return true;
		}).for_each([&](Unit* target)
		{
			auto it = std::find(link.begin(), link.end(), target);
			if (it == link.end()) {
				link.push_back(target);
			}

			UnitExt* targetExt = globals.get_unit_ext(target);
			auto& vec = targetExt->unit_vectors[UnitVectorPhaseLinkClarions];
			it = std::find(vec.begin(), vec.end(), unit);
			if (it == vec.end()) {
				vec.push_back(unit);
			}
		});
	}

	bool hasClarionLinks(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);
		return unitExt->unit_vectors[UnitVectorPhaseLinkClarions].size();
	}

	void linkDamage(Unit* unit, s32 damage, hooks::WeaponDamageData& data)
	{
		hooks::WeaponDamageData newData = {
			damage,
			damage,
			0,
			data.damageType,
			data.damageDivisor,
			data.weaponId,
			data.direction,
			unit,
			data.attacker,
			data.attackingPlayerId,
			true,
			data.ignoreFlatArmor
		};
		hooks::weaponDamage_root(newData);
	}

	void spreadClarionDamage(hooks::WeaponDamageData& data)
	{
		Unit* unit = data.target;
		auto links = getClarionLinks(unit); // has to be copy
		links.erase(std::remove(links.begin(), links.end(), unit), links.end());
		s32 damage = data.finalDamage;
		if (links.size() == 0 || unit->shields < 256)
		{
			linkDamage(unit, damage, data);
		}
		else
		{
			links.erase(std::remove_if(links.begin(), links.end(), [](Unit* linkUnit)
			{
				return linkUnit->shields < 256;
			}), links.end());
			if (links.size())
			{
				s32 divided = damage / links.size();
				auto it = std::find(links.begin(), links.end(), unit);
				if (it != links.end())
				{
					links.erase(it);
				}
				Unit* damageOriginUnit = data.attacker;
				if (!damageOriginUnit)
				{
					damageOriginUnit = unit;
				}
				s8 oldDirection = data.direction;
				for (Unit* linkUnit : links)
				{
					s32 unitDamage = std::min(divided, linkUnit->shields);
					damage -= unitDamage;
					data.direction = scbw::get_angle(damageOriginUnit->get_x(), damageOriginUnit->get_y(), linkUnit->get_x(), linkUnit->get_y());
					linkDamage(linkUnit, unitDamage, data);
				}
				data.direction = oldDirection;
			}
			linkDamage(unit, damage, data);
		}
	}

	std::vector<Unit*> getClarionLinks(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		std::unordered_set<Unit*> links = {};
		auto& clarions = unitExt->unit_vectors[UnitVectorPhaseLinkClarions];
		for (Unit* clarion : clarions)
		{
			if (!clarion)
			{
				continue;
			}
			UnitExt* clarionExt = globals.get_unit_ext(clarion);
			auto link = clarionExt->unit_vectors[UnitVectorPhaseLink];
			links.insert(link.begin(), link.end());
		}
		return std::vector<Unit*>(links.begin(), links.end());
	}

	void spreadSovrolethDamage(hooks::WeaponDamageData& data)
	{
		Unit* unit = data.target;
		EntryArray sovroleths = scbw::get_writhing_web_sovroleths(unit);
		s32 damage = data.finalDamage / (sovroleths.size + 1);
		for (u32 i = 0; i < sovroleths.size; ++i) {
			linkDamage(sovroleths.units[i], damage, data);
		}
		linkDamage(unit, damage, data);
	}
}