#include "briefing.h"
#include <BW/api.h>
#include <BW/structures.h>
#include <BW/bwdata.h>
#include <BW/ui_scale.h>
#include "storm/storm.h"

#include <algorithm>

#include <windows.h>

namespace hooks{
// Hooked in case we need it later - Pr0nogo
void update_mission_objectives() { // 0046CD30 | doesn't work currently
	BinDlg* ready_room_var = *g_ready_room_dialog;
	if (ready_room_var->control_type != DialogControlType::DialogBoxDC) {
		ready_room_var = (*g_ready_room_dialog)->parent;
	}

	BinDlg* objective_dialog = 0;
	BinDlg* child_dialog = ready_room_var->child_dialog;
	bool dialog_exists = true;
	if (child_dialog) {
		while (child_dialog->index != -11) {
			child_dialog = child_dialog->next;
			if (!child_dialog) {
				dialog_exists = false;
				break;
			}
		}
		if (dialog_exists) {
			objective_dialog = child_dialog;
		}
	}

	const char* objective_text;
	u32 string_count = g_map_string_tbl->get_string_count();
	if (string_count) {
		u16 map_string_id = *((u16*)&g_mission_objectives_string_index + 2 * g_local_nation_id) - 1;
		if (map_string_id) {
			if (map_string_id < string_count) { // This check always fails. Probably something to do with g_map_string_tbl being improperly accessed!
				objective_text = (char*)g_map_string_tbl + *((u16*)g_map_string_tbl + map_string_id + 1);
			} else {
				objective_text = "";
			}
		} else {
			objective_text = 0;
		}
	} else {
		objective_text = 0;
	}

	if (!objective_dialog) {
		return;
	} else {
		objective_dialog->text = (char*)objective_text;
		helper::show_dialog(objective_dialog);
		if (!(objective_dialog->flags & DialogFlags::UpdateNeeded)) {
			objective_dialog->flags |= DialogFlags::UpdateNeeded;
			helper::update_dialog(objective_dialog);
		}
	}
}

// Increase portrait canvas size for briefing playback
static constexpr u16 PortraitCanvasWidth = 120;
static constexpr u16 PortraitCanvasHeight = 112;
void init_briefing_portrait_window(bw::Dialog* window) { // 0044EF00
	const u16 canvas_width = u16(PortraitCanvasWidth * ui_scale::Factor);
	const u16 canvas_height = u16(PortraitCanvasHeight * ui_scale::Factor);

	auto portrait_data = (PortraitData*)SMemAlloc(sizeof(PortraitData));
	*portrait_data = {};
	portrait_data->dims_x = canvas_width;
	portrait_data->dims_y = canvas_height;

	const u32 canvas_area = canvas_width * canvas_height;
	portrait_data->bmp_buffer = SMemAlloc(canvas_area);
	std::fill_n((u8*)portrait_data->bmp_buffer, canvas_area, 0);

	portrait_data->center_offset_x = u8((window->bounds.w() - canvas_width) / 2);
	portrait_data->center_offset_y = u8((window->bounds.h() - canvas_height) / 2);

	window->user = portrait_data;
	window->draw_function = helper::briefing_portrait_draw;
	helper::set_callback_timer(window, 1, 50, helper::briefing_portrait_tick);
	bw::hide_dialog(window->to_bindlg());
}
} // namespace hooks
