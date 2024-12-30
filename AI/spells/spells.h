#pragma once
#include <BW/structures/Unit.h>

// Many of these are now deprecated! -Pr0nogo
namespace AI {
	Unit* find_best_lockdown_target(Unit* caster, bool is_under_attack);
	Unit* find_best_nuke_launch_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_defensive_matrix_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_emp_shockwave_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_irradiate_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_yamato_gun_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_optical_flare_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_restoration_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_restoration_target_ex(Unit* caster, bool isUnderAttack);

	Unit* find_best_parasite_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_ensnare_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_broodspawn_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_carapace_swarm_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_plague_target(Unit* caster, bool isUnderAttack);

	Unit* find_best_psi_storm_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_hallucination_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_stasis_field_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_recall_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_disruption_field_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_feedback_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_maelstrom_target(Unit* caster, bool isUnderAttack);
	Unit* find_best_tyranny_target(Unit* caster, bool is_under_attack);
}