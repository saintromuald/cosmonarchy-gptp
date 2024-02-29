#include "cm_utils.h"
#include "BW\api.h"

Box32 box32_rect(Point16 pos, u32 w, u32 h) {
	Box32 box = {};
	box.left   = pos.x - w / 2;
	box.right  = pos.x + w / 2;
	box.top    = pos.y - h / 2;
	box.bottom = pos.y + h / 2;
	return box;
}

Box32 box32_square(Point16 pos, u32 w) {
	Box32 box = {};
	box.left   = pos.x - w / 2;
	box.right  = pos.x + w / 2;
	box.top    = pos.y - w / 2;
	box.bottom = pos.y + w / 2;
	return box;
}

bool is_hostile_or_neutral(u32 player, u32 other_player) {
	if (other_player == NeutralPlayer) {
		return true;
	}
	return player != other_player && !bw::is_allied_to(player, other_player);
}
