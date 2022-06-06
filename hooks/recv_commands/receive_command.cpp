#include "receive_command.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks
{
	//this function receives the Attack/Attack Move commands and
	//the spells commands, maybe more
	void receive_command(s32 x, s32 y, Unit* unitParam, u32 wUnitType, u32 bActionOrder, u32 bCommandType)
	{

		u8 array_of_data[0x30];
		u16* saved_x = (u16*)&array_of_data[0];					//[EBP-0x30]
		u16* saved_y = (u16*)&array_of_data[2];					//[EBP-0x2E]
		u32* saved_unknown_1 = (u32*)&array_of_data[0];			//[EBP-0x30] 
		u32* saved_unknown_2 = (u32*)&array_of_data[4];			//[EBP-0x2C] 
		u32* saved_bCommandType = (u32*)&array_of_data[0x1C];	//[EBP-0x14]
		u32* saved_order = (u32*)&array_of_data[0x20];			//[EBP-0x10]
		u32* saved_order_2 = (u32*)&array_of_data[0x24];		//[EBP-0x0C]
		u32* saved_order_3 = (u32*)&array_of_data[0x28];		//[EBP-0x08]
		Unit** saved_unit = (Unit**)&array_of_data[0x2C];		//[EBP-0x04]

		u8* switch_value = (u8*)0x0049ADA0;

		*saved_x = x;
		*saved_y = y;
		*saved_order = bActionOrder;

		helper::function_0049A8C0((u8*)&array_of_data[0], orders_dat::CanBeObstructed[bActionOrder]);

		if (bActionOrder > OrderId::CTFCOP2 || switch_value[bActionOrder] == 1)
		{

			Unit* current_unit;
			u32 current_order = bActionOrder;	//EBX

			*unit_iterator_pos = 0;
			current_unit = getActivePlayerNextSelection();

			*saved_unit = current_unit;

			while (current_unit != nullptr)
			{

				bool jump_to_9AD7E = false;
				bool bOrderAllowed;

				if (orders_dat::TechUsed[current_order] != TechId::None)
					bOrderAllowed = (current_unit->canUseTech(orders_dat::TechUsed[current_order], *ACTIVE_NATION_ID) == 1);
				else
					bOrderAllowed = (helper::order_is_allowed(current_unit, current_order, *ACTIVE_NATION_ID) == 1);

				if (bOrderAllowed)
				{

					if (current_unit == unitParam)
					{ //target is the caster/ordered unit
						if (!helper::CanTargetSelf(current_unit, bActionOrder))
							jump_to_9AD7E = true;
					}

				}
				else
					jump_to_9AD7E = true;

				if (!jump_to_9AD7E)
				{ //9ABC4:

					if (helper::function_0049A410(current_unit, bActionOrder) == 0)
						jump_to_9AD7E = true;
					else
						if (current_unit->main_order_id != OrderId::NukeLaunch ||
							bActionOrder == OrderId::Die
							)
						{ //9ABE7

							bool jump_to_9ac4e = false;
							bool jump_to_9aca9 = false;
							bool jump_to_9ad09 = false;
							bool jump_to_9ad74 = false;
							bool jump_to_9ad77 = false;
							bool loop_to_9ac0d = true;

							*saved_order_2 = OrderId::Nothing2;
							*saved_order_3 = bActionOrder;

							if (current_unit->id == UnitId::TerranCleric)
							{

								if (bActionOrder == OrderId::AttackMove ||
									bActionOrder == OrderId::Attack1
									)
								{ //9AC09

									*saved_order_3 = OrderId::HealMove;

								}
								else
									jump_to_9aca9 = true;

							}
							else
								jump_to_9ac4e = true;

							while (loop_to_9ac0d)
							{

								if (!jump_to_9ac4e && !jump_to_9aca9)
								{

									if (unitParam == nullptr)
									{
										jump_to_9ad09 = true;
										loop_to_9ac0d = false;
									}
									else
									{

										helper::function_004756E0(current_unit, unitParam, *saved_order_3, bCommandType);

										if (*saved_order_2 == OrderId::Nothing2)
										{
											jump_to_9ad74 = true;
											loop_to_9ac0d = false;
										}
										else
										{
											helper::function_004756E0(current_unit->subunit, unitParam, *saved_order_2, bCommandType);
											jump_to_9ad74 = true;
											loop_to_9ac0d = false;
										}

									}

								}

								if (jump_to_9ac4e)
								{

									jump_to_9ac4e = false;

									if (bActionOrder != OrderId::Attack1)
										jump_to_9aca9 = true;
									else
									{

										//Attack command conversion

										if (unitParam != nullptr)
											*saved_order_3 = units_dat::AttackUnitOrder[current_unit->id];
										else
											*saved_order_3 = units_dat::AttackMoveOrder[current_unit->id];

										if (*saved_order_3 == OrderId::Nothing2)
										{
											jump_to_9AD7E = true;
											loop_to_9ac0d = false;
										}
										else
											if (current_unit->subunit->is_subunit())
											{

												if (unitParam != nullptr)
												{
													*saved_order_2 = units_dat::AttackUnitOrder[current_unit->subunit->id];
													jump_to_9aca9 = true;
												}
												else
												{
													*saved_order_2 = units_dat::AttackMoveOrder[current_unit->subunit->id];
													jump_to_9aca9 = true;
												}

											}
											else
												jump_to_9aca9 = true;

									}

								}

								//9aca9:
								if (jump_to_9aca9)
								{

									jump_to_9aca9 = false;

									if (*saved_order_3 == OrderId::RallyPoint1)
									{

										Unit* target = unitParam;

										if (target == nullptr)
											target = current_unit;

										UnitExt* current_unitExt = globals.get_unit_ext(current_unit);
										current_unit->rally.unit = target;
										current_unit->rally.pt.x = target->sprite->position.x;
										current_unit->rally.pt.y = target->sprite->position.y;

										jump_to_9ad77 = true;
										loop_to_9ac0d = false;

									}

									//9acdf
									if (*saved_order_2 == OrderId::RallyPoint2)
									{

										loop_to_9ac0d = false;

										UnitExt* current_unitExt = globals.get_unit_ext(current_unit);
										current_unit->rally.unit = nullptr;
										current_unit->rally.pt.x = x;
										current_unit->rally.pt.y = y;

										jump_to_9ad77 = true;

									}

								}

							}

							//9ad09
							if (jump_to_9ad09)
							{

								jump_to_9ad09 = false;

								*saved_x = x;
								*saved_y = y;

								if (wUnitType == UnitId::None)
									helper::function_0049A500(current_unit, (u8*)&array_of_data[0]);
								else
									*saved_unknown_2 = *saved_unknown_1;

								helper::function_004756B0(
									current_unit,
									*saved_order_3,
									*saved_unknown_2 / 65536,
									*saved_unknown_2,
									bCommandType
								);

								if (*saved_order_2 == OrderId::Nothing2)
									jump_to_9ad74 = true;
								else
								{
									helper::function_004756B0(
										current_unit->subunit,
										*saved_order_2,
										*saved_unknown_2 / 65536,
										*saved_unknown_2,
										bCommandType
									);
									jump_to_9ad74 = true;
								}

							}

							//9ad74
							if (jump_to_9ad74 == true)
							{
								jump_to_9ad74 = false;
								jump_to_9ad77 = true;
							}

							//9ad77
							if (jump_to_9ad77 == true)
							{
								jump_to_9ad77 = false;
								current_unit->user_action_flags |= 2;
								jump_to_9AD7E = true;
							}
						}
						else
							jump_to_9AD7E = true;

				}

				//9AD7E (use next unit in selection):
				if (jump_to_9AD7E)
				{
					jump_to_9AD7E = false;
					*saved_unit = getActivePlayerNextSelection();
				}

				//9AB62:
				current_unit = *saved_unit;
				current_order = *saved_order;
			}
		}
	}
}