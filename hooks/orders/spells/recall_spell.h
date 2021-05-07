#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{

	void spells_Recall_ActionOnValidTargetForRecall(Unit* target, Unit* caster);	//004942D0  sub_4942D0
	void spells_Recall_ActionOnTargetInRange(Unit* target, Unit* caster);			//00494400  sub_494400
	void orders_Recall(Unit* unit);												//00494470

	void injectRecallSpellHooks();

} //hooks
