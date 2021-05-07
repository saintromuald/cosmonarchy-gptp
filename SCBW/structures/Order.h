#pragma once
#include "Target.h"

#pragma pack(1)

//V241 for VS2008

struct Order
{
	/*00*/  Order* prev;
	/*04*/  Order* next;
	/*08*/  u16  orderId;
	/*0A*/  u16  unitId;
	/*0C*/  Target  target;
	/*0C*/			/*target.pt.x*/
	/*0E*/			/*target.pt.y*/
	/*10*/			/*target.unit*/
};

C_ASSERT(sizeof(Order) == 20);
//static_assert(sizeof(Order) == 20, "The size of the Order structure is invalid");

#pragma pack()
