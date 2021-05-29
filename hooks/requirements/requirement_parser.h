#pragma once
#include <SCBW/structures.h>

namespace customRequirements
{
	u16* getOrderRequirementOverrides();
	u16* getOrderRequirementOpcodes();
	u16* getResearchRequirementOverrides();
	u16* getResearchRequirementOpcodes();
	u16* getTechRequirementOverrides();
	u16* getTechRequirementOpcodes();
	u16* getUnitRequirementOverrides();
	u16* getUnitRequirementOpcodes();
	u16* getUpgradeRequirementOverrides();
	u16* getUpgradeRequirementOpcodes();

	void parseRequirementOverrides();
}