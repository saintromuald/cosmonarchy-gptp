#pragma once
#include <BW/api.h>

// I am experimenting with using classes for encapsulation
// Using std::vector, std::find, and std::sort because we don't have any custom alternatives,
// and I don't want to fuck with manual array handling and etc
//																						-Veeq7
class ButtonsetContext {
public:
	// Called on game start
	void reset() {
		index = 0;
		buttonsets.clear();
	}

	void update() { // TODO: call when user (on client side) selects a new group of units
		reset();

		for (u32 i = 0; i < g_client_selection_count; i++) {
			Unit* unit = g_client_selection_group->unit[i];
			if (std::find(buttonsets.begin(), buttonsets.end(), unit->id) != buttonsets.end()) {
				buttonsets.push_back(unit->id);
			}
		}

		std::sort(buttonsets.begin(), buttonsets.end(), [](u32 a, u32 b) {
			// TODO: define sorting
			// I think we want different sorting for buildings and for units, technically we can't select buildings and units,
			// but I would still sort buildings before units for the sake of handling that case
			// 
			// For buildings:
			// 1. townhall and townhall like buildings (e.g. embassy) 
			// 2. military production (low tier prod first, e.g. low mineral -> high minerals -> low gas -> high gas)
			// 3. the rest
			// 
			// We will also need to find a way to treat production with addons as seperate case,
			// maybe instead of just returning u32 from this class we would also return some custom enum like Addon
			// 
			// For units:
			// 1. combat units with most amount of active spells
			// 2. combat units with most amount of passive spells
			// 3. workers
			// 4. pacifists 
			return 0;
		});
	}

	void cycle() { // TODO: call when user uses tab
		size_t count = buttonsets.size();
		if (count) {
			index = (index + 1) % count;
		} else {
			index = 0;
		}
	}

	u32 get_buttonset() { // TODO: call when updating buttonsets for groups (ex)
		return buttonsets[index];
	}

	// TODO: 

private:
	u32 index = 0;
	std::vector<u32> buttonsets;
};

extern ButtonsetContext buttonset_context;

namespace hooks {
	void update_buttonset_ex();							//	00458BC0
	void highlight_button();							//	00458D50 sub_458D50
	void update_buttonset();							//	00458DE0
	void update_command_card();							//	004591D0
	void update_current_buttonset();					//	004599A0
	void statbtn_bin_custom_ctrl_id(BinDlg* dialog);	//	00459AD0
	ButtonSet* get_buttonset(s32 index);

	void inject_buttonset_hooks();
}