#pragma once
#include <BW/api.h>

typedef u16 REQ_SCRIPT;

extern REQ_SCRIPT* units_req;
extern REQ_SCRIPT* upgrades_req;
extern REQ_SCRIPT* research_req;
extern REQ_SCRIPT* tech_req;
extern REQ_SCRIPT* orders_req;

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
