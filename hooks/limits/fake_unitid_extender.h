#pragma once
#include "hooks/limits/extender.h"

struct UnitIdExtender /*: public Extender*/ {
/*	virtual std::vector<u32> get_address_list();
	virtual std::vector<u32> get_size_list();*/
public:
	//Don't uses extender framework unless real id extender is implemented
	u32 flingy_ids[UNIT_TYPE_COUNT];

	//
	// add other entries later, no need right now
	//
/*	virtual int identifier(std::string str);
	virtual void extend();
	virtual u32 parse_entry_value(std::string str, int id);
	virtual bool is_string_id(int id);
	virtual void set_value(int entry, int id, std::string str);
	virtual void set_value(int entry, int id, int val);
	virtual void set_extended_data();*/
	void init();
};

extern UnitIdExtender new_unitids;