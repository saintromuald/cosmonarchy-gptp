#include "captivating_claws.h"

namespace captivating_claws {
	int captivatingClawSources[4] = {ValueId::CaptivatingClawsSource1, ValueId::CaptivatingClawsSource2, ValueId::CaptivatingClawsSource3, ValueId::CaptivatingClawsSource4};
	u32 getCaptivatingClawsStacks(CUnit* target) {
		return scbw::get_aise_value(target, NULL, AiseId::GenericValue, ValueId::CaptivatingClawsStacks, 0);
	}

	bool canApplyCaptivatingClaws(CUnit* source, CUnit* target) {
		if (scbw::get_aise_value(target, NULL, AiseId::GenericValue, getCaptivatingClawsStacks(target), 0) >= 4) return false;
		for (int i = 0; i < 4; i++) {
			if (scbw::get_generic_unit(target, captivatingClawSources[i]) == source) return false;
		}
		return true;
	}

	void applyCaptivatingClaws(CUnit* source, CUnit* target) {
		int index = (scbw::get_aise_value(target, NULL, AiseId::GenericValue, ValueId::CaptivatingClawsIndex, 0) + 1) % 4;
		scbw::set_generic_value(target, ValueId::CaptivatingClawsIndex, index);

		if (canApplyCaptivatingClaws(source, target)) {
			scbw::add_generic_value(target, ValueId::CaptivatingClawsStacks, 1);
		}
		scbw::set_generic_unit(target, captivatingClawSources[index], source);
		scbw::set_generic_timer(target, ValueId::CaptivatingClawsTimer, (24 * 1) + 1);
	}

	void clearCaptivatingClaws(CUnit* target) {
		scbw::set_generic_value(target, ValueId::CaptivatingClawsStacks, 0);
		for (int i = 0; i < 4; i++) {
			scbw::set_generic_unit(target, captivatingClawSources[i], NULL);
		}
	}
}