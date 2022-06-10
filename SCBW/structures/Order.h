#pragma once
#include "Target.h"

#pragma pack(1)

struct Order
{
	/*00*/  Order*	prev;
	/*04*/  Order*	next;
	/*08*/  u16		order_id;
	/*0A*/  u16		unit_id;
	/*0C*/  Target  target;
	/*0C*/			/*target.pt.x*/
	/*0E*/			/*target.pt.y*/
	/*10*/			/*target.unit*/
};

C_ASSERT(sizeof(Order) == 20);

#pragma pack()