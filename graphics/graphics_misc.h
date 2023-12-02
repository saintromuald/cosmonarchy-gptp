#pragma once
#include <string>
#include "../types.h"

namespace graphics {
	const std::string& get_string_from_index(s32 stringIndex);
	//const std::string& get_statres_string_from_index(s32 stringIndex);

	s32 draw_all_shapes();
	s32 draw_stat_res_shapes();
}