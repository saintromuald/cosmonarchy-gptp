#pragma once
#include <vector>
#include "SCBW\api.h"
#include "hook_tools.h"
class Extender {
public:
	virtual std::vector<u32> get_address_list() = 0;
	virtual std::vector<u32> get_size_list() = 0;
	virtual void extend() = 0;
	void uplink(std::vector<u32> address_list, u32 address);
//	virtual void uplink() - define here
//	identifier
//	get
//  parse entry value
//  parse - define here
//	parse-set (?)
//	file entry count
};

