#include "captivating_claws.h"

namespace captivating_claws {
	int captivatingClawSources[4] = {ValueId::CaptivatingClawsSource1, ValueId::CaptivatingClawsSource2, ValueId::CaptivatingClawsSource3, ValueId::CaptivatingClawsSource4};
	u32 getCaptivatingClawsStacks(CUnit* target) {
		int n = 0;
		for (int i = 0; i < 4; i++) {
			if (scbw::get_generic_unit(target, captivatingClawSources[i])) i++;
		}
		return n;
	}

	void addCaptivatingClawsSource(CUnit* source, CUnit* target) {
		int index = (scbw::get_aise_value(target, NULL, AiseId::GenericValue, ValueId::CaptivatingClawsIndex, 0) + 1) % 4;
		scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetGenericValue, ValueId::CaptivatingClawsIndex, index, 0);
		scbw::sendGPTP_aise_cmd(target, source, GptpId::SetGenericUnit, captivatingClawSources[index], 0, 0);
		scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetGenericTimer, ValueId::CaptivatingClawsTimer, 25, 0);
	}

	bool canApplyCaptivatingClaws(CUnit* source, CUnit* target) {
		if (scbw::get_aise_value(target, NULL, AiseId::GenericValue, getCaptivatingClawsStacks(target), 0) >= 4) return false;
		for (int i = 0; i < 4; i++) {
			if (scbw::get_generic_unit(target, captivatingClawSources[i]) == source) return false;
		}
		return true;
	}

	void clearCaptivatingClawsSources(CUnit* target) {
		for (int i = 0; i < 4; i++) {
			scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetGenericUnit, captivatingClawSources[i], 0, 0);
		}
	}
}