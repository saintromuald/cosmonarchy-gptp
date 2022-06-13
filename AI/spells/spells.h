#pragma once
#include <SCBW/structures/Unit.h>

namespace AI {

Unit* findBestLockdownTarget(Unit* caster, bool isUnderAttack);
Unit* find_best_nuke_launch_target(Unit* caster, bool isUnderAttack);
Unit* findBestDefensiveMatrixTarget(Unit* caster, bool isUnderAttack);
Unit* findBestEmpShockwaveTarget(Unit* caster, bool isUnderAttack);
Unit* findBestIrradiateTarget(Unit* caster, bool isUnderAttack);
Unit* findBestYamatoGunTarget(Unit* caster, bool isUnderAttack);
Unit* findBestOpticalFlareTarget(Unit* caster, bool isUnderAttack);
Unit* findBestRestorationTarget(Unit* caster, bool isUnderAttack);
Unit* findBestRestorationTargetSituational(Unit* caster, bool isUnderAttack);

Unit* findBestParasiteTarget(Unit* caster, bool isUnderAttack);
Unit* findBestEnsnareTarget(Unit* caster, bool isUnderAttack);
Unit* findBestSpawnBroodlingsTarget(Unit* caster, bool isUnderAttack);
Unit* findBestDarkSwarmTarget(Unit* caster, bool isUnderAttack);
Unit* findBestPlagueTarget(Unit* caster, bool isUnderAttack);

Unit* find_best_psi_storm_target(Unit* caster, bool isUnderAttack);
Unit* findBestHallucinationTarget(Unit* caster, bool isUnderAttack);
Unit* findBestStasisFieldTarget(Unit* caster, bool isUnderAttack);
Unit* findBestRecallTarget(Unit* caster, bool isUnderAttack);
Unit* findBestDisruptionWebTarget(Unit* caster, bool isUnderAttack);
Unit* findBestFeedbackTarget(Unit* caster, bool isUnderAttack);
Unit* findBestMaelstromTarget(Unit* caster, bool isUnderAttack);
Unit* findBestMindControlTarget(Unit* caster, bool isUnderAttack);

} //AI
