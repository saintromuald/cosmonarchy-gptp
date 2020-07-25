#include "hooks\economic_units.h"
namespace hooks {
	u32 get_order_from_unit_id(u32 unit_id) {
		u32 order = 0;
		switch (unit_id) {
		case UnitId::scv://0x7
			order = OrderId::BuildTerran;
			break;
		case UnitId::drone://0x29
			order = OrderId::DroneStartBuild;
			break;
		case UnitId::probe://0x40
			order = OrderId::BuildProtoss1;
			break;
		default:
			break;
		};
		return order;
	}
}