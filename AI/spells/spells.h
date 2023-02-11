#pragma once
#include <SCBW/structures/Unit.h>

namespace AI {

Unit* findBestLockdownTarget(Unit* caster, bool isUnderAttack);
Unit* find_best_nuke_launch_target(Unit* caster, bool isUnderAttack);
Unit* find_best_defensive_matrix_target(Unit* caster, bool isUnderAttack);
Unit* findBestEmpShockwaveTarget(Unit* caster, bool isUnderAttack);
Unit* find_best_irradiate_target(Unit* caster, bool isUnderAttack);
Unit* find_best_yamato_gun_target(Unit* caster, bool isUnderAttack);
Unit* findBestOpticalFlareTarget(Unit* caster, bool isUnderAttack);
Unit* findBestRestorationTarget(Unit* caster, bool isUnderAttack);
Unit* findBestRestorationTargetSituational(Unit* caster, bool isUnderAttack);

Unit* find_best_parasite_target(Unit* caster, bool isUnderAttack);
Unit* findBestEnsnareTarget(Unit* caster, bool isUnderAttack);
Unit* findBestSpawnBroodlingsTarget(Unit* caster, bool isUnderAttack);
Unit* find_best_carapace_swarm_target(Unit* caster, bool isUnderAttack);
Unit* find_best_plague_target(Unit* caster, bool isUnderAttack);

Unit* find_best_psi_storm_target(Unit* caster, bool isUnderAttack);
Unit* find_best_hallucination_target(Unit* caster, bool isUnderAttack);
Unit* find_best_stasis_field_target(Unit* caster, bool isUnderAttack);
Unit* find_best_recall_target(Unit* caster, bool isUnderAttack);
Unit* findBestDisruptionWebTarget(Unit* caster, bool isUnderAttack);
Unit* findBestFeedbackTarget(Unit* caster, bool isUnderAttack);
Unit* find_best_maelstrom_target(Unit* caster, bool isUnderAttack);
Unit* find_best_tyranny_target(Unit* caster, bool isUnderAttack);

} //AI
