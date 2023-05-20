#include "globals.h"
#include "hook_tools.h"
#include "hooks/cosmonarchy_finder.h"
#include "hooks/weapons/weapon_damage.h"
#include "hooks/weapons/weapon_fire.h"
#include <algorithm>
#include <unordered_set>
#include <Utils/captivating_claws.h>
IquareGlobals iquare_globals;

void IquareGlobals::refresh_debugging_info()
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

void IquareGlobals::aise_save()
{
	scbw::print("WARNING: Saving doesn't work properly in Cosmonarchy.\n\t\t\tAny plugin specific data will be lost.", GameTextColor::Yellow);
	return;

	output.clear();
	std::ostringstream stream(std::stringstream::out | std::stringstream::binary);
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

	u32 pariahHitCount = line_attack_hits.size();
	for (s32 i = 0; i < pariahHitCount; i++)
	{
		stream.write(reinterpret_cast<const char*>(&line_attack_hits[i].source), sizeof(line_attack_hits[i].source));
		stream.write(reinterpret_cast<const char*>(&line_attack_hits[i].target), sizeof(line_attack_hits[i].target));
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
		stream.write(reinterpret_cast<char*>(&unit_ext_array[i].damage_taken_timer), sizeof(unit_ext_array[i].damage_taken_timer));
		stream.write(reinterpret_cast<char*>(&unit_ext_array[i].values), sizeof(unit_ext_array[i].values));
		stream.write(reinterpret_cast<char*>(&unit_ext_array[i].timers), sizeof(unit_ext_array[i].timers));
		stream.write(reinterpret_cast<char*>(&unit_ext_array[i].units), sizeof(unit_ext_array[i].units));

		for (auto vec : unit_ext_array[i].value_vectors)
		{
			auto vecSize = vec.size();
			stream.write(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.write(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}

		for (auto vec : unit_ext_array[i].timer_vectors)
		{
			auto vecSize = vec.size();
			stream.write(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.write(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}

		for (auto vec : unit_ext_array[i].unit_vectors)
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

void IquareGlobals::aise_load(const char* ptr, u32 len)
{
	this->reset();
	return;

	if (len == 0 || ptr == 0)
	{
		return; // error
	}

	std::stringstream stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	std::string str(ptr, len);
	stream << str;
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
		line_attack_hits.push_back(hit);
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
		stream.read(reinterpret_cast<char*>(&unit_ext_array[i].damage_taken_timer), sizeof(unit_ext_array[i].damage_taken_timer));
		stream.read(reinterpret_cast<char*>(&unit_ext_array[i].values), sizeof(unit_ext_array[i].values));
		stream.read(reinterpret_cast<char*>(&unit_ext_array[i].timers), sizeof(unit_ext_array[i].timers));
		stream.read(reinterpret_cast<char*>(&unit_ext_array[i].units), sizeof(unit_ext_array[i].units));

		for (auto vec : unit_ext_array[i].value_vectors)
		{
			auto vecSize = vec.size();
			stream.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.read(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}

		for (auto vec : unit_ext_array[i].timer_vectors)
		{
			auto vecSize = vec.size();
			stream.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
			for (auto elem : vec)
			{
				stream.read(reinterpret_cast<char*>(&elem), sizeof(elem));
			}
		}

		for (auto vec : unit_ext_array[i].unit_vectors)
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

void IquareGlobals::reset() {
	effects.clear();
	line_attack_hits.clear();
	multihit_bullets.clear();
	lurker_spine_hits.clear();
}

void IquareGlobals::unit_death(Unit* unit) {
	for (s32 i = 0; i < this->effects.size(); i++) {
		Effect* effect = &effects[i];
		if (effect->source == unit) {
			effect->source = nullptr;
		}
		if (effect->origin == unit) {
			effect->origin = nullptr;
		}
	}

	lurker_spine_hits[unit].clear();
	for (auto& a : lurker_spine_hits) {
		for (s32 b = 0; b < a.second.size(); b++) {
			if (a.second[b] == unit) {
				a.second.erase(a.second.begin() + b);
				b--;
			}
		}
	}
}

void apply_irradiate(Unit* unit, u8 player, u32 time) {
	if (time == 0) {
		return;
	}
	// if not already irradiated and not burrowed, add the irradiate overlay
	if (unit->irradiate_timer == 0 && !(unit->status & UnitStatus::Burrowed)) {
		u32 irradiate_image_id = ImageId::IrradiateNew_Small + scbw::get_overlay_offset(unit->id);
		Unit* overlay_unit = unit->subunit ? unit->subunit : unit;
		overlay_unit->sprite->create_top_overlay(irradiate_image_id, 0, 0, 0);
	}
	unit->irradiate_caster = 0;
	unit->irradiate_player_id = player;
	unit->irradiate_timer = time;
}

void tick_effect(Effect* effect) {
	const s32 effect_period = 6;
	if (effect->current_time % effect_period == 0) {
		const f64 damage_multiplier = 256.0 * (f64)effect_period / effect->max_time;
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
					unit->damage_with_custom(2500 * damage_multiplier, -1, effect->source, effect->player);
				});
			} break;
			case EffectId_SalamanderFire: {
				EntryArray unit_array = {};
				unit_search_radial(&unit_array, {(u16)effect->position.x, (u16)effect->position.y}, effect->radius);
				unit_search_filter(&unit_array, nullptr, FilterNoAir);

				for (u32 j = 0; j < unit_array.count; j++) {
					Unit* target = unit_array.units[j];
					target->damage_with_custom(30 * damage_multiplier, 3, effect->source, effect->player);
				}
			} break;
			case EffectId_AspirantCriticality: {
				scbw::UnitFinder(
					effect->position.x - effect->radius,
					effect->position.y - effect->radius,
					effect->position.x + effect->radius,
					effect->position.y + effect->radius
				).for_each_in_match(cosmonarchy_finder_match(nullptr, Flags::NoAir, {}), [&](Unit* unit) {
					if (scbw::get_distance_fast(effect->position.x, effect->position.y, unit->get_x(), unit->get_y()) > effect->radius) {
						return;
					}
					unit->heal_shields_then_hp(4 * 256 / 24 * effect_period, effect->source, true);
				});
			} break;
			case EffectId_KeskathalorCausticDischarge: {
				scbw::UnitFinder(
					effect->position.x - effect->radius,
					effect->position.y - effect->radius,
					effect->position.x + effect->radius,
					effect->position.y + effect->radius
				).for_each_in_match(cosmonarchy_finder_match(nullptr, Flags::NoAir, {}), [&](Unit* unit) {
					if (scbw::get_distance_fast(effect->position.x, effect->position.y, unit->get_x(), unit->get_y()) > effect->radius) {
						return;
					}
					UnitExt* unit_ext = get_unit_ext(unit);
					unit->damage_with_custom(30 * damage_multiplier, -1, effect->source, effect->player);
					unit_ext->timers[TimerCausticDischargeSpeed] = 5 * 24 + 1;
				});
			} break;
			case EffectId_IonCannonIonField: {
				scbw::UnitFinder(
					effect->position.x - effect->radius,
					effect->position.y - effect->radius,
					effect->position.x + effect->radius,
					effect->position.y + effect->radius
				).for_each_in_match(cosmonarchy_finder_match(nullptr, Flags::NoAir, {}), [&](Unit* unit) {
					if (scbw::get_distance_fast(effect->position.x, effect->position.y, unit->get_x(), unit->get_y()) > effect->radius) {
						return;
					}
					UnitExt* unit_ext = get_unit_ext(unit);
					unit->damage_with_custom(100 * damage_multiplier, -1, effect->source, effect->player);
					unit_ext->timers[TimerIonFieldAttackSpeed] = 2 * 24 + 1;
				});
			} break;
			case EffectId_HallucinatingField: {
				scbw::UnitFinder(
					effect->position.x - effect->radius,
					effect->position.y - effect->radius,
					effect->position.x + effect->radius,
					effect->position.y + effect->radius
				).for_each_in_match(cosmonarchy_finder_match(nullptr, 0, {}), [&](Unit* unit) {
					if (scbw::get_distance_fast(effect->position.x, effect->position.y, unit->get_x(), unit->get_y()) > effect->radius) {
						return;
					}
					scbw::apply_hallucinating(unit, effect->player, 3 * 24 + 1);
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
					apply_irradiate(unit, effect->player, 15);
				});
			} break;
			case EffectId_NuclearMissileGroundZero: {
				const u32 delta = effect->max_time - effect->current_time;
				const u32 stage = delta / effect_period;

				switch (stage) {
					case 0: {
						iquare_globals.create_effect(EffectId_Irradiate, 20 * 24 - delta, effect->position, 1.5 * 32, SpriteId::IrradiateNew_Large, 5, effect->player, effect->source);
					} break;
					case 1:
					case 2: {
						f32 distance = stage * 2 * 32;
						u32 count = stage * 8;

						Point16 new_pos = effect->position;
						new_pos.x += distance;
						for (u32 j = 0; j < count; ++j) {
							new_pos = scbw::rotate_point(effect->position, new_pos, (360.0 / count));
							iquare_globals.create_effect(EffectId_Irradiate, 20 * 24 - delta, new_pos, 1.5 * 32, SpriteId::IrradiateNew_Large, 5, effect->player, effect->source);
						}
					} break;
				}
			} break;
			case EffectId_LanifectDisruptionField: {
				if (!effect->origin) {
					effect->current_time = 0;
					break;
				}

				UnitExt* parent_ext = get_unit_ext(effect->origin);
				if (!parent_ext->values[ValueDisruptionFieldOn]) {
					effect->current_time = 0;
					break;
				}

				scbw::UnitFinder(
					effect->position.x - 60,
					effect->position.y - 40,
					effect->position.x + 60,
					effect->position.y + 40
				).for_each_in_match(cosmonarchy_finder_match(nullptr, 0, {}), [&](Unit* unit) {
					UnitExt* unit_ext = get_unit_ext(unit);
					unit_ext->timers[TimerUnderDisruptionField] = 8;
					unit->status |= UnitStatus::CanNotAttack;
					if (unit->subunit) {
						unit->subunit->status |= UnitStatus::CanNotAttack;
					}
				});
			} break;
			case EffectId_IsthrathalethPlagueField: {
				scbw::UnitFinder(
					effect->position.x - 1.5 * 32,
					effect->position.y - 1 * 32,
					effect->position.x + 1.5 * 32,
					effect->position.y + 1 * 32
				).for_each([&](Unit* unit) {
					helper::plague_proc(unit, 0);
				});
			} break;
			case EffectId_AionReclamationField: {
				EntryArray corpses = {};
				corpse_search_radial(&corpses, effect->position, effect->radius);
				corpse_search_sort_by_distance(&corpses, effect->position);
				for (u32 i = 0; i < corpses.count; ++i) {
					Thingy* corpse = corpses.corpses[i];
					scbw::reclaim_corpse(corpse, effect->player);
					effect->current_time = 0;
					break;
				}
			} break;
		}
	}
}

void IquareGlobals::create_effect(u8 id, u32 time, Point16 position, u32 radius, s32 sprite_id, u8 elevation, u8 player, Unit* source, Unit* parent) {
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
	tick_effect(&effect);
	effects.push_back(effect);
}

void IquareGlobals::frame_hook() {
	for (s32 i = 0; i < effects.size(); i++) {
		Effect* effect = &effects[i];
		effect->current_time--;
		if (effect->origin) {
			effect->position = {effect->origin->position.x, effect->origin->position.y};
			if (effect->thingy)
			{
				effect->thingy->sprite->set_position(effect->origin->position.x, effect->origin->position.y);
			}
		}
		tick_effect(effect);
		if (effect->current_time == 0) {
			if (effect->thingy && effect->thingy->sprite) {
				effect->thingy->sprite->flags |= SpriteFlags::RemoveAfterDeath;
				effect->thingy->sprite->play_iscript_animation(IscriptAnimation::Death);
			}

			effects.erase(effects.begin() + i);
			i--;
		}
	}
}

void IquareGlobals::remove_unit_from_line_attack_table(Unit* unit, bool source_only)
{
	for (s32 i = 0; i < line_attack_hits.size(); i++)
	{
		if (line_attack_hits[i].source == unit || (line_attack_hits[i].target == unit && !source_only))
		{
			line_attack_hits.erase(line_attack_hits.begin() + i);
			i--;
		}
	}
}

void IquareGlobals::remove_multihit_bullet(Bullet* bullet)
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

bool IquareGlobals::was_hit_by_line_attack(Unit* source, Unit* target)
{
	for (auto a : line_attack_hits)
	{
		if (source == a.source && target == a.target)
		{
			return true;
		}
	}
	return false;
}

void IquareGlobals::register_line_attack_hit(Unit* source, Unit* target)
{
	PariahHit hit;
	hit.source = source;
	hit.target = target;
	line_attack_hits.push_back(hit);
}

bool IquareGlobals::was_already_hit_by_multihit_bullet(Bullet* bullet, Unit* target) {
	for (auto& a : multihit_bullets) {
		if (bullet == a.bullet) {
			for (auto b : a.already_hit) {
				if (b == target) {
					return true;
				}
			}
		}
	}
	return false;
}

bool IquareGlobals::multihit_bullet_is_set(Bullet* bullet) {
	for (auto& a : multihit_bullets) {
		if (bullet == a.bullet) {
			return true;
		}
	}
	return false;
}

void IquareGlobals::add_bullet_multihit(Bullet* bullet, Unit* target) {
	for (auto& a : multihit_bullets) {
		if (a.bullet == bullet) {
			a.already_hit.push_back(target);
			return;
		}
	}
	MultihitBullet gb(bullet, {target}, 0);
	multihit_bullets.push_back(gb);
}

void IquareGlobals::add_incremental_bullet_multihit(Bullet* bullet, Unit* target)
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

s32 IquareGlobals::get_divider(Bullet* bullet) {
	for (auto& a : multihit_bullets) {
		if (a.bullet == bullet && a.divider != 0) {
			return a.divider;
		}
	}
	return 1;
}

u32 IquareGlobals::get_multihit_count(Bullet* bullet) {
	for (auto& a : multihit_bullets) {
		if (a.bullet == bullet) {
			return a.already_hit.size();
		}
	}
	return 0;
}

void IquareGlobals::add_multihit_empty_bullet(Bullet* bullet, s32 divider) {
	for (auto& a : multihit_bullets) {
		if (a.bullet == bullet) {
			a.divider = divider;
			return;
		}
	}
	MultihitBullet gb(bullet, {}, divider);
	multihit_bullets.push_back(gb);
}

void IquareGlobals::clear_lakizilisk(Unit* lakizilisk) {
	lurker_spine_hits[lakizilisk].clear();
}

void IquareGlobals::add_lakizilisk_spine_pair(Unit* lakizilisk, Unit* target) {
	lurker_spine_hits[lakizilisk].push_back(target);
}

bool IquareGlobals::spine_pair_exists(Unit* lakizilisk, Unit* target) {
	for (auto a : lurker_spine_hits) {
		for (auto b : a.second) {
			if (a.first == lakizilisk && b == target) {
				return true;
			}
		}
	}
	return false;
}

void __declspec(naked) globals_save_state_wrapper()
{
	static SaveState* state;
	__asm {
		PUSHAD
	}
//	state = &globals.aise_save();
	iquare_globals.aise_save();
	state = &saveState;
	__asm {
		POPAD
		MOV EAX, state
		RETN
	}
}

void __declspec(naked) globals_load_state_wrapper()
{
	static const char* pointer;
	static u32 len;
	__asm {
		MOV len, ECX
		MOV pointer, EAX
		PUSHAD
	}
	iquare_globals.aise_load(pointer, len);
	__asm {
		POPAD
		RETN
	}
}

void inject_globals_hooks()
{
	jump_patch(globals_save_state_wrapper, 0x004AAF23);
	jump_patch(globals_load_state_wrapper, 0x004AAE94);
	// const u8 retn = 0xc3;
	// memoryPatch(0x004AAF23 + 5, &retn, 1);
	// memoryPatch(0x004AAE94 + 5, &retn, 1);
}

namespace scbw
{
	void refresh_debuffs(Unit* unit)
	{
		UnitExt* unit_ext = get_unit_ext(unit);

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
		if (unit_ext->timers[TimerObservanceDebuff])
		{
			unit_ext->timers[TimerObservanceDebuff] = SeraphObservanceDebuffTimer;
		}

		// Refresh Hallucination
		if (unit_ext->timers[TimerHallucinating])
		{
			unit_ext->timers[TimerHallucinating] = 3 * 24 + 1;
		}

		// Refresh Captivating ZethrokorClaws
		if (unit_ext->timers[TimerCaptivatingClaws])
		{
			unit_ext->timers[TimerCaptivatingClaws] = VorvrokorCaptivatingClawsTimer;
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
		if (unit_ext->timers[TimerCorrosiveTouch])
		{
			unit_ext->timers[TimerCorrosiveTouch] = VithriliskCorrosiveTouchTimer;
		}

		// Refresh Irradiate
		if (unit->irradiate_timer) {
			unit->irradiate_timer = SeraphIrradiateTimer;
		}

		// Refresh Dirge
		if (unit_ext->timers[TimerDirgeFuse])
		{
			unit_ext->timers[TimerDirgeFuse] = MadrigalDirgeTimer;
		}

		// Refresh Hierophant Signify
		if (unit_ext->timers[TimerSignify])
		{
			unit_ext->timers[TimerSignify] = 3 * 24 + 1;
		}

		//was hardcoded in original code
		scbw::refresh_console();
	}

	void inherit_debuffs(Unit* parent, Unit* child)
	{
		UnitExt* parent_ext = get_unit_ext(parent);
		UnitExt* child_ext = get_unit_ext(child);

		// NOTE: classic period formula is (([real seconds / 0.042) / 8)
		// e.g. (([15] / 0.042) / 8) = 44.642, rounded up to 45

		// Get overlay info
		s32 overlay_offset = get_overlay_offset(child->id);

		// Inherit Ensnare
		if (parent->ensnare_timer != 0)
		{
			child->ensnare_timer = parent->ensnare_timer;
			if (!(child->sprite->get_overlay(ImageId::EnsnareOverlay_Small + overlay_offset)))
			{
				child->sprite->create_overlay(ImageId::EnsnareOverlay_Small + overlay_offset);
			}
			child->update_speed();
		}

		// Inherit Plague
		if (parent->plague_timer != 0)
		{
			child->plague_timer = parent->plague_timer;
			if (!(child->sprite->get_overlay(ImageId::PlagueOverlay_Small + overlay_offset)))
			{
				child->sprite->create_overlay(ImageId::PlagueOverlay_Small + overlay_offset);
			}
		}

		// Inherit Incubation
		if (child_ext->timers[TimerIncubationArm]) {
			child_ext->timers[TimerIncubationArm] = parent_ext->timers[TimerIncubationArm];
			child_ext->values[ValueIncubationPlayer] = parent_ext->values[ValueIncubationPlayer];
			child->sprite->create_overlay(ImageId::ParasiteOverlay);
		}

		// Inherit Observance
		if (parent_ext->timers[TimerObservanceDebuff] > 0)
		{
			child_ext->timers[TimerObservanceDebuff] = parent_ext->timers[TimerObservanceDebuff];
			child_ext->timers[TimerObservancePeriod] = parent_ext->timers[TimerObservancePeriod];
			child_ext->values[ValueObservanceRend] = parent_ext->values[ValueObservanceRend];
		}

		// Inherit Knockout Drivers
		if (parent_ext->timers[TimerPetrification] > 0)
		{
			child_ext->timers[TimerPetrification] = parent_ext->timers[TimerPetrification];
		}

		// Inherit Hallucination
		if (parent_ext->timers[TimerHallucinating]  > 0)
		{
			child_ext->timers[TimerHallucinating] = parent_ext->timers[TimerHallucinating];
		}

		// Inherit Captivating ZethrokorClaws
		if (auto timer = parent_ext->timers[TimerCaptivatingClaws])
		{
			child_ext->timers[TimerCaptivatingClaws] = timer;
			parent_ext->unit_vectors[UnitVectorCaptivatingClaws] = child_ext->unit_vectors[UnitVectorCaptivatingClaws];
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
		if (parent_ext->timers[TimerCorrosiveTouch])
		{
			child_ext->values[ValueCorrosiveTouchStacks] = parent_ext->values[ValueCorrosiveTouchStacks];
			child_ext->timers[TimerCorrosiveTouch] = parent_ext->timers[TimerCorrosiveTouch];
		}

		// Inherit Irradiate
		if (parent->irradiate_timer) {
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
		if (parent_ext->values[ValueDeconstruction])
		{
			child_ext->values[ValueDeconstruction] = parent_ext->values[ValueDeconstruction];
			child_ext->timers[TimerDeconstruction] = parent_ext->timers[TimerDeconstruction];
			Sprite* sprite = child->sprite;
			if (child->subunit != nullptr)
			{
				UnitExt* subunit_ext = get_unit_ext(child->subunit);
				subunit_ext->values[ValueDeconstruction] = parent_ext->values[ValueDeconstruction];
				subunit_ext->timers[TimerDeconstruction] = parent_ext->timers[TimerDeconstruction];
				sprite = child->subunit->sprite;
			}
			if (!sprite->get_overlay(ImageId::DeconstructionSmall + overlay_offset))
			{
				sprite->create_overlay(ImageId::DeconstructionSmall + overlay_offset);
			}
		}

		// Inherit Terminal Surge
		if (parent_ext->values[ValueTerminalSurge])
		{
			child_ext->values[ValueTerminalSurge] = parent_ext->values[ValueTerminalSurge];
		}

		// Inherit Stellar Enforcement
		if (parent_ext->timers[TimerStellarEnforcement])
		{
			child_ext->timers[TimerStellarEnforcement] = parent_ext->timers[TimerStellarEnforcement];
		}

		// Inherit Swarming Omen
		if (parent_ext->values[ValueSwarmingOmenPlayer])
		{
			child_ext->values[ValueSwarmingOmenPlayer] = parent_ext->values[ValueSwarmingOmenPlayer];
		}

		// Inherit Kaiser Rampage
		if (parent_ext->timers[TimerKaiserRampage])
		{
			child_ext->timers[TimerKaiserRampage] = parent_ext->timers[TimerKaiserRampage];
		}

		// Inherit Kelvin Munitions
		if (parent_ext->timers[TimerKelvinMunitions])
		{
			child_ext->timers[TimerKelvinMunitions] = parent_ext->timers[TimerKelvinMunitions];
		}

		// Inherit Dirge
		if (parent_ext->timers[TimerDirgeFuse])
		{
			child_ext->timers[TimerDirgeFuse] = parent_ext->timers[TimerDirgeFuse];
			child_ext->values[ValueDirgeStacks] = parent_ext->values[ValueDirgeStacks];
		}

		// Inherit Empress Passive Stacks
		if (parent_ext->values[ValueEmpressPassiveStacks])
		{
			child_ext->values[ValueEmpressPassiveStacks] = parent_ext->values[ValueEmpressPassiveStacks];
			child_ext->values[ValueEmpressSourcePlayer] = parent_ext->values[ValueEmpressSourcePlayer];
		}

		// Inherit Hierophant Signify
		child_ext->timers[TimerSignify] = parent_ext->timers[TimerSignify];
		child_ext->values[ValueSignifyStacks] = parent_ext->values[ValueSignifyStacks];
		child_ext->value_vectors[ValueVectorSignifySourcePlayer] = parent_ext->value_vectors[ValueVectorSignifySourcePlayer];
		scbw::update_signify_status(child, child_ext);

		scbw::refresh_console();
	}

	// TODO: update
	bool is_debuffed(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		bool has_parasite = false;
		for (u32 i = 0; i < PlayerLimit; ++i) {
			if (!scbw::is_allied_to(i, unit->player)) {
				has_parasite = true;
			}
		}

		return unit_ext->timers[TimerPetrification]
			|| unit_ext->timers[TimerObservancePeriod]
			|| unit_ext->timers[TimerCorrosiveTouch]
			|| unit_ext->values[ValueDeconstruction]
			|| unit_ext->values[ValueEntropyGauntlets]
			|| unit_ext->timers[TimerKelvinMunitions] >= 1
			|| unit_ext->values[ValueSignifyStacks]
			|| captivating_claws_get_timer(unit) > 0
			|| unit->irradiate_timer != 0
			|| unit->ensnare_timer != 0
			|| unit->lockdown_timer != 0
			|| unit->maelstrom_timer != 0
			|| unit->plague_timer != 0
			|| unit->is_blind
			|| has_parasite;
	}

	void restore_func(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);

		Unit* overlayTargetUnit = unit;
		if (unit->subunit != nullptr)
		{
			overlayTargetUnit = unit->subunit;
		}
		u32 overlayImageId = ImageId::RestorationHit_Small + scbw::get_overlay_offset(unit->id);;
		(overlayTargetUnit->sprite)->create_top_overlay(overlayImageId, 0, 0, 0);

		// remove status effects and corresponding overlays

		// Remove Incubation
		if (!scbw::is_allied_to(unit_ext->values[ValueIncubationPlayer], unit->player) && unit_ext->timers[TimerIncubationArm]) {
			unit_ext->timers[TimerIncubationArm] = 0;
			unit->remove_overlay(ImageId::ParasiteOverlay);
		}

		// Remove default flags
		unit->parasite_flags = 0;
		unit->is_blind = 0;

		// Remove Blind
		if (unit_ext->timers[TimerAuralAssaultBlind]) {
			unit_ext->timers[TimerAuralAssaultBlind] = 0;

			u32 overlay = ImageId::OpticalFlareHit_Small + scbw::get_overlay_offset(unit->id);
			Unit* overlay_unit = unit->subunit ? unit->subunit : unit;
			overlay_unit->remove_overlay(overlay);
			unit->status &= ~UnitStatus::CanNotReceiveOrders;
		}

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
		if (unit->irradiate_timer != 0) {
			unit->remove_overlay(ImageId::IrradiateNew_Small, ImageId::IrradiateNew_Large);
			unit->irradiate_timer = 0;

			// specific updates following irradiate removal
			unit->irradiate_caster = 0;
			unit->irradiate_player_id = 8;
		}

		// Remove Observance
		if (unit_ext->timers[TimerObservanceDebuff])
		{
			unit_ext->timers[TimerObservanceDebuff] = 1;
		}

		// Remove Knockout Drivers
		if (unit_ext->timers[TimerPetrification])
		{
			unit_ext->timers[TimerPetrification] = 1;
		}

		// Remove Hallucination
		if (unit_ext->timers[TimerHallucinating])
		{
			unit_ext->timers[TimerHallucinating] = 1;
		}

		// Remove Captivating Claws
		if (unit_ext->timers[TimerCaptivatingClaws])
		{
			unit_ext->timers[TimerCaptivatingClaws] = 1;
			unit_ext->unit_vectors[UnitVectorCaptivatingClaws] = {};
		}

		// Remove Controlled Opposition
		unit_ext->values[ValueControlledOpposition] = 0;

		// Remove Empress stacks
		unit_ext->values[ValueEmpressPassiveStacks] = 0;

		// Remove Lockdown
		unit->remove_lockdown();

		// Remove Maelstrom
		unit->remove_maelstrom();

		// Remove Devourer Corrosive Spores
		unit->remove_acid_spores();

		// Remove Deconstruction
		unit_ext->values[ValueDeconstruction] = 0;
		unit_ext->timers[TimerDeconstruction] = 0;
		unit->remove_overlay(ImageId::DeconstructionSmall, ImageId::DeconstructionLarge);
		if (unit->subunit)
		{
			UnitExt* subunit_ext = get_unit_ext(unit->subunit);
			subunit_ext->values[ValueDeconstruction] = 0;
			subunit_ext->timers[TimerDeconstruction] = 0;
			unit->subunit->remove_overlay(ImageId::DeconstructionSmall, ImageId::DeconstructionLarge);
		}

		// Remove Kelvin Munitions
		if (unit_ext->timers[TimerKelvinMunitions])
		{
			unit_ext->timers[TimerKelvinMunitions] = 0;
		}

		// Remove Dirge
		if (unit_ext->timers[TimerDirgeFuse])
		{
			unit_ext->timers[TimerDirgeFuse] = 0;
			unit_ext->values[ValueDirgeStacks] = 0;
		}

		// Remove Hierophant Signify
		unit_ext->timers[TimerSignify] = 0;
		unit_ext->values[ValueSignifyStacks] = 0;
		unit_ext->value_vectors[ValueVectorSignifySourcePlayer] = {};
		scbw::update_signify_status(unit, unit_ext);

		// Remove Stellar Enforcement
		unit_ext->timers[TimerStellarEnforcement] = 0;

		scbw::refresh_console();
	}

	bool is_slowed(Unit* unit)
	{
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit->ensnare_timer)
		{
			return true;
		}
		if (unit_ext->timers[TimerSignify])
		{
			return true;
		}
		if (unit_ext->timers[TimerPetrification])
		{
			return true;
		}
		if (unit_ext->timers[TimerCaptivatingClaws])
		{
			return true;
		}
		if (unit_ext->timers[TimerKelvinMunitions] >= 1)
		{
			return true;
		}
		return false;
	}

	void do_deconstruction_damage(Unit* unit, u16 weapon_id, bool play_sound)
	{
		// Deconstruction - Pazuzu passive
		UnitExt* unit_ext = get_unit_ext(unit);
		f64 self_damage = (unit_ext->values[ValueDeconstruction] * 0.05 * (f64)(weapons_dat::damage_amount[weapon_id] * 256.0));
		Unit* deconstruction_target = unit;
		if (units_dat::base_property[unit->id] & UnitProperty::Subunit)
		{
			deconstruction_target = unit_ext->units[UnitTurretParentLink]; // TODO: port as unit
			if (!deconstruction_target)
			{
				return;
			}
		}
		hooks::weapon_damage(self_damage, deconstruction_target, weapon_id, deconstruction_target, deconstruction_target->player, 0, 1);
		if (play_sound)
		{
			helper::play_sound_from_unit(SoundId::PazuzuDeconstructionDamage, unit, 1, 0);
		}
	}

	void remove_overlays_2(u32 from, u32 to, Unit* unit)
	{
		const u32 Func_RemoveOverlays2 = 0x00492370;
		__asm {
			PUSHAD
			MOV EAX, from
			MOV EBX, unit
			PUSH to
			CALL Func_RemoveOverlays2
			POPAD
		}
	}

	void clear_clarion_links(Unit* unit)
	{
		UnitExt* unitExt = get_unit_ext(unit);

		auto& vec = unitExt->unit_vectors[UnitVectorPhaseLink];
		for (Unit* vecUnit : vec)
		{
			UnitExt* vecUnitExt = get_unit_ext(vecUnit);
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

	void place_clarion_links(Unit* unit)
	{
		UnitExt* unitExt = get_unit_ext(unit);
		auto& link = unitExt->unit_vectors[UnitVectorPhaseLink];

		clear_clarion_links(unit);
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
			if (units_dat::base_property[target->id] & UnitProperty::Building)
			{
				return false;
			}
			if (!target->shields_enabled())
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

			UnitExt* targetExt = get_unit_ext(target);
			auto& vec = targetExt->unit_vectors[UnitVectorPhaseLinkClarions];
			it = std::find(vec.begin(), vec.end(), unit);
			if (it == vec.end()) {
				vec.push_back(unit);
			}
		});
	}

	bool has_clarion_links(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		return unit_ext->unit_vectors[UnitVectorPhaseLinkClarions].size();
	}

	void link_damage(Unit* unit, s32 damage, hooks::WeaponDamageData& data) {
		hooks::WeaponDamageData new_data = data;
		new_data.damage = damage;
		new_data.shield_reduce_amount = 0;
		new_data.target = unit;
		new_data.dont_proc_link_damage = true;
		hooks::weapon_damage_root(new_data);
	}

	void spread_clarion_damage(hooks::WeaponDamageData& data) {
		Unit* unit = data.target;
		auto links = get_clarion_links(unit); // has to be copy
		links.erase(std::remove(links.begin(), links.end(), unit), links.end());
		s32 damage = data.damage;
		if (links.size() == 0 || unit->shields < 256) {
			link_damage(unit, damage, data);
		} else {
			links.erase(std::remove_if(links.begin(), links.end(), [](Unit* linkUnit) {
				return linkUnit->shields < 256;
			}), links.end());
			if (links.size()) {
				s32 divided = damage / links.size();
				auto it = std::find(links.begin(), links.end(), unit);
				if (it != links.end()) {
					links.erase(it);
				}
				Unit* damage_origin = data.attacker;
				if (!damage_origin) {
					damage_origin = unit;
				}
				s8 old_direction = data.direction;
				for (Unit* linked_unit : links) {
					s32 unit_damage = std::min(divided, linked_unit->shields);
					damage -= unit_damage;
					data.direction = scbw::get_angle(damage_origin->get_x(), damage_origin->get_y(), linked_unit->get_x(), linked_unit->get_y());
					link_damage(linked_unit, unit_damage, data);
				}
				data.direction = old_direction;
			}
			link_damage(unit, damage, data);
		}
	}

	std::vector<Unit*> get_clarion_links(Unit* unit) {
		UnitExt* unitExt = get_unit_ext(unit);

		std::unordered_set<Unit*> links = {};
		auto& clarions = unitExt->unit_vectors[UnitVectorPhaseLinkClarions];
		for (Unit* clarion : clarions) {
			if (!clarion) {
				continue;
			}
			UnitExt* clarionExt = get_unit_ext(clarion);
			auto link = clarionExt->unit_vectors[UnitVectorPhaseLink];
			links.insert(link.begin(), link.end());
		}
		return std::vector<Unit*>(links.begin(), links.end());
	}
}