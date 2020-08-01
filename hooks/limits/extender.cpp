#include "extender.h"

/*
uplink(get_size_list(), NEW_ARRAY_LENGTH);
uplink(table_start_list(), table_start);
uplink(get_address_list(), index_start);
uplink(table_end_list(), table_end);
*/
void Extender::uplink(std::vector<u32> address_list, u32 data){
	for (auto a : address_list) {
		memoryPatch(a, data);
	}
}