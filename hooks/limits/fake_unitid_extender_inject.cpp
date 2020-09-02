//for 4-byte flingies
#include "SCBW/api.h"
namespace hooks {

	u32* get_flingy_ptr() {
		return reinterpret_cast<u32*>(scbw::get_aise_value(NULL, NULL, AiseId::GetFlingyPtr, 0, 0));
	}
	void unitsDatWrapperPatches() {


	}
}