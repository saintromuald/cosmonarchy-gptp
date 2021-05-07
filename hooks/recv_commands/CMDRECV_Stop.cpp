#include "CMDRECV_Stop.h"
#include <SCBW/api.h>

namespace hooks
{
	//The functions seems similar but there are some differences,
	//like the u16_ offset, the function to perform the order in
	//CMDRECV_Stop, and the orders id

	void CMDRECV_ReaverStop()
	{

		Unit* current_unit;
		static u32* u32_0066FF60 = (u32*)0x0066FF60;

		*selectionIndexStart = 0;

		current_unit = getActivePlayerNextSelection();

		while (current_unit != nullptr)
		{

			*u32_0066FF60 = 0x00;

			if (orders_dat::RequirementsOffset[OrderId::ReaverStop] == 0xFFFF)
				helper::parseOrdersDatReqs();

			if (current_unit->playerId != *ACTIVE_NATION_ID)
				*u32_0066FF60 = 0x01;
			else
				if (!(current_unit->status & UnitStatus::Completed))
					*u32_0066FF60 = 0x14;
				else
					if (current_unit->isFrozen())
					{
						*u32_0066FF60 = 0x0A;
					}
					else
						if (current_unit->status & UnitStatus::Burrowed &&
							current_unit->id != UnitId::ZergLakizilisk
							)
							*u32_0066FF60 = 0x0B;
						else
							if (current_unit->id == UnitId::TerranMason &&
								current_unit->mainOrderId == OrderId::ConstructingBuilding
								)
								*u32_0066FF60 = 0x08;
							else
								if ((current_unit->id == UnitId::TerranEidolon ||
									current_unit->id == UnitId::Hero_SarahKerrigan ||
									current_unit->id == UnitId::Hero_AlexeiStukov ||
									current_unit->id == UnitId::Hero_SamirDuran ||
									current_unit->id == UnitId::Hero_InfestedDuran) &&
									current_unit->mainOrderId == OrderId::NukeTrack
									)
									*u32_0066FF60 = 0x08;
								else
									if ((current_unit->id == UnitId::ProtossArchon || current_unit->id == UnitId::ProtossAugur) &&
										current_unit->mainOrderId == OrderId::CompletingArchonSummon
										)
										*u32_0066FF60 = 0x08;
									else
										if (current_unit->id == UnitId::ProtossMindTyrant &&
											current_unit->mainOrderId == OrderId::CompletingArchonSummon
											)
											*u32_0066FF60 = 0x08;
										else
											if (orders_dat::RequirementsOffset[OrderId::ReaverStop] == 0)
												*u32_0066FF60 = 0x17;
											else
												if (helper::parseRequirementOpcodes(
													current_unit,
													OrderId::ReaverStop,
													*ACTIVE_NATION_ID,
													0x00514CF8,
													orders_dat::RequirementsOffset[OrderId::ReaverStop]
												) == 1)
												{

													current_unit->userActionFlags |= 1;

													if (current_unit->mainOrderId != OrderId::Die)
													{

														bool bContinueLoop = true;

														while (bContinueLoop)
														{

															if (current_unit->orderQueueTail == nullptr)
																bContinueLoop = false;
															else
																if (orders_dat::CanBeInterrupted[current_unit->orderQueueTail->orderId] ||
																	current_unit->orderQueueTail->orderId == OrderId::ReaverStop
																	)
																	helper::removeOrderFromUnitQueue(current_unit, current_unit->orderQueueTail);
																else
																	bContinueLoop = false;

														}

														current_unit->performAnotherOrder(OrderId::ReaverStop, 0, 0, nullptr, UnitId::None, 0);

													}

													prepareForNextOrder(current_unit);

												}

			current_unit = getActivePlayerNextSelection();

		}

	}

	;

	void CMDRECV_CarrierStop()
	{

		Unit* current_unit;
		static u32* u32_0066FF60 = (u32*)0x0066FF60;

		*selectionIndexStart = 0;

		current_unit = getActivePlayerNextSelection();

		while (current_unit != nullptr)
		{

			*u32_0066FF60 = 0x00;

			if (orders_dat::RequirementsOffset[OrderId::CarrierStop] == 0xFFFF)
				helper::parseOrdersDatReqs();

			if (current_unit->playerId != *ACTIVE_NATION_ID)
				*u32_0066FF60 = 0x01;
			else
				if (!(current_unit->status & UnitStatus::Completed))
					*u32_0066FF60 = 0x14;
				else
					if (current_unit->isFrozen())
					{
						*u32_0066FF60 = 0x0A;
					}
					else
						if (current_unit->status & UnitStatus::Burrowed &&
							current_unit->id != UnitId::ZergLakizilisk
							)
							*u32_0066FF60 = 0x0B;
						else
							if (current_unit->id == UnitId::TerranMason &&
								current_unit->mainOrderId == OrderId::ConstructingBuilding
								)
								*u32_0066FF60 = 0x08;
							else
								if ((current_unit->id == UnitId::TerranEidolon ||
									current_unit->id == UnitId::Hero_SarahKerrigan ||
									current_unit->id == UnitId::Hero_AlexeiStukov ||
									current_unit->id == UnitId::Hero_SamirDuran ||
									current_unit->id == UnitId::Hero_InfestedDuran) &&
									current_unit->mainOrderId == OrderId::NukeTrack
									)
									*u32_0066FF60 = 0x08;
								else
									if ((current_unit->id == UnitId::ProtossArchon || current_unit->id == UnitId::ProtossAugur) &&
										current_unit->mainOrderId == OrderId::CompletingArchonSummon
										)
										*u32_0066FF60 = 0x08;
									else
										if (current_unit->id == UnitId::ProtossMindTyrant &&
											current_unit->mainOrderId == OrderId::CompletingArchonSummon
											)
											*u32_0066FF60 = 0x08;
										else
											if (orders_dat::RequirementsOffset[OrderId::CarrierStop] == 0)
												*u32_0066FF60 = 0x17;
											else
												if (helper::parseRequirementOpcodes(
													current_unit,
													OrderId::CarrierStop,
													*ACTIVE_NATION_ID,
													0x00514CF8,
													orders_dat::RequirementsOffset[OrderId::CarrierStop]
												) == 1)
												{

													current_unit->userActionFlags |= 1;

													if (current_unit->mainOrderId != OrderId::Die)
													{

														bool bContinueLoop = true;

														while (bContinueLoop)
														{

															if (current_unit->orderQueueTail == nullptr)
																bContinueLoop = false;
															else
																if (orders_dat::CanBeInterrupted[current_unit->orderQueueTail->orderId] ||
																	current_unit->orderQueueTail->orderId == OrderId::CarrierStop
																	)
																	helper::removeOrderFromUnitQueue(current_unit, current_unit->orderQueueTail);
																else
																	bContinueLoop = false;

														}

														current_unit->performAnotherOrder(OrderId::CarrierStop, 0, 0, nullptr, UnitId::None, 0);

													}

													prepareForNextOrder(current_unit);

												}

			current_unit = getActivePlayerNextSelection();

		}

	}

	;

	void CMDRECV_Stop(u8 bCommandType)
	{

		Unit* current_unit;
		static u32* u32_0066FF60 = (u32*)0x0066FF60;

		*selectionIndexStart = 0;

		current_unit = getActivePlayerNextSelection();

		while (current_unit != nullptr)
		{

			*u32_0066FF60 = 0x00;

			if (orders_dat::RequirementsOffset[OrderId::Stop] == 0xFFFF)
				helper::parseOrdersDatReqs();

			if (current_unit->playerId != *ACTIVE_NATION_ID)
				*u32_0066FF60 = 0x01;
			else
				if (!(current_unit->status & UnitStatus::Completed))
					*u32_0066FF60 = 0x14;
				else
					if (current_unit->isFrozen())
					{
						*u32_0066FF60 = 0x0A;
					}
					else
						if (current_unit->status & UnitStatus::Burrowed &&
							current_unit->id != UnitId::ZergLakizilisk
							)
							*u32_0066FF60 = 0x0B;
						else
							if ((current_unit->id == UnitId::TerranEidolon ||
								current_unit->id == UnitId::Hero_SarahKerrigan ||
								current_unit->id == UnitId::Hero_AlexeiStukov ||
								current_unit->id == UnitId::Hero_SamirDuran ||
								current_unit->id == UnitId::Hero_InfestedDuran) &&
								current_unit->mainOrderId == OrderId::NukeTrack
								)
								*u32_0066FF60 = 0x08;
							else
								if ((current_unit->id == UnitId::ProtossArchon || current_unit->id == UnitId::ProtossAugur) &&
									current_unit->mainOrderId == OrderId::CompletingArchonSummon
									)
									*u32_0066FF60 = 0x08;
								else
									if (current_unit->id == UnitId::ProtossMindTyrant &&
										current_unit->mainOrderId == OrderId::CompletingArchonSummon
										)
										*u32_0066FF60 = 0x08;
									else
										if (orders_dat::RequirementsOffset[OrderId::Stop] == 0)
											*u32_0066FF60 = 0x17;
										else
											if (helper::parseRequirementOpcodes(
												current_unit,
												OrderId::Stop,
												*ACTIVE_NATION_ID,
												0x00514CF8,
												orders_dat::RequirementsOffset[OrderId::Stop]
											) == 1)
												helper::function_004754F0(
													current_unit,
													UnitId::None,
													*(u32*)(0x006D5C24),
													*(u32*)(0x006D5C20),
													OrderId::Stop,
													0,
													bCommandType != 0, //true (1) if Queued Order
													1
												);
			current_unit = getActivePlayerNextSelection();
		}
	}
}