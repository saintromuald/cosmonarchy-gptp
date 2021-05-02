#pragma once
#pragma pack(1)
#include "SCBW\structures.h"
#include "extender.h"
#include <vector>

struct CCMUExtender : public Extender
{
    virtual std::vector<u32> get_address_list();
    virtual std::vector<u32> get_size_list();
    std::vector<u32> table_start_list();
    std::vector<u32> table_end_list();
    std::vector<u32> sprite_offset_list();
    std::vector<u32> orderingXlist();
    std::vector<u32> orderingXlist_off4();
    std::vector<u32> orderingYlist();
    std::vector<u32> orderingYlist_off4();
    std::vector<u32> orderingY_min18endlist();
public:
    CUnitLayout units[UNIT_ARRAY_LENGTH];
    UnitFinderData orderingX[UNIT_ARRAY_LENGTH * 2];
    UnitFinderData orderingY[UNIT_ARRAY_LENGTH * 2];
    CUnit* position_search_units[UNIT_ARRAY_LENGTH + 1];
    u32 position_search_temp[UNIT_ARRAY_LENGTH + 1];
    CUnit* air_splash_results[UNIT_ARRAY_LENGTH];

    virtual void extend();
    virtual s32 identifier(std::string str);
    virtual u32 parse_entry_value(std::string str, s32 id);
    virtual bool is_string_id(s32 id);
    virtual void set_value(s32 entry, s32 id, std::string str);
    virtual void set_value(s32 entry, s32 id, s32 val);
    virtual void set_extended_data();
};

namespace hooks
{
    void injectCCMUHooks();
}

extern CCMUExtender new_unit_table;