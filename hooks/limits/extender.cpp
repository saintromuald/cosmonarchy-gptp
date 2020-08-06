#include "extender.h"

void Extender::uplink(std::vector<u32> address_list, u32 data){
	for (auto a : address_list) {
		memoryPatch(a, data);
	}
}