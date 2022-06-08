#pragma once
#include <SCBW/structures.h>

namespace customRequirements
{
	u16* get_order_requirement_overrides();
	u16* get_order_requirement_opcodes();
	u16* get_research_requirement_overrides();
	u16* get_research_requirement_opcodes();
	u16* get_tech_requirement_overrides();
	u16* get_tech_requirement_opcodes();
	u16* getUnitRequirementOverrides();
	u16* getUnitRequirementOpcodes();
	u16* get_upgrade_requirement_overrides();
	u16* get_upgrade_requirement_opcodes();

	void parseRequirementOverrides();
}