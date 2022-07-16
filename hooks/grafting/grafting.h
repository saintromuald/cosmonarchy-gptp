#pragma once
#include <SCBW/api.h>

typedef u16 REQ_SCRIPT;

namespace GraftType {
  enum Enum : u32 {
    units_req,
    upgrades_req,
    research_req,
    tech_req,
    orders_req,
    buttons,
    statInfo,
  };
};

namespace hooks {

  void load_req_grafts();
  void load_button_grafts();

  void inject_grafting_data(void* graftData, u32 type);
};
