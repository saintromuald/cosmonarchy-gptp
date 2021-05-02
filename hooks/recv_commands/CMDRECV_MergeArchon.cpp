#include "CMDRECV_MergeArchon.h"
#include <SCBW/api.h>

namespace hooks
{

	void CMDRECV_MergeDarkArchon()
	{

		CUnit* current_unit;
		CUnit* templar_stored;
		CUnit* templar_merge_partner;
		CUnit* templars_stored[SELECTION_ARRAY_LENGTH];
		CUnit** templars_stored_iterator;
		s32 templars_stored_count;

		*selectionIndexStart = 0;
		current_unit = getActivePlayerNextSelection();

		if (current_unit != nullptr &&
			current_unit->canUseTech(TechId::DarkArchonMeld, *ACTIVE_NATION_ID) == 1)
		{

			for (s32 i = 0; i < SELECTION_ARRAY_LENGTH; i++)
				templars_stored[i] = nullptr;

			templars_stored_count = 0;

			while (current_unit != nullptr)
			{

				if (current_unit->id == UnitId::ProtossCabalist)
				{
					templars_stored[templars_stored_count] = current_unit;
					templars_stored_count++;
				}

				current_unit = getActivePlayerNextSelection();

			}

			templars_stored_iterator = &templars_stored[1];
			templars_stored_count--;

			while (templars_stored_count > 0)
			{

				templar_stored = templars_stored_iterator[-1];

				if (templar_stored != nullptr)
				{

					templar_merge_partner = helper::templarMergePartner(templar_stored, templars_stored_iterator, templars_stored_count);

					if (templar_merge_partner != nullptr)
					{

						templar_stored->userActionFlags |= 1;

						if (templar_stored->mainOrderId != OrderId::Die)
						{

							bool bKeepLooping = true;

							while (bKeepLooping)
							{

								if (templar_stored->orderQueueTail == nullptr)
									bKeepLooping = false;
								else
									if (orders_dat::CanBeInterrupted[templar_stored->orderQueueTail->orderId] ||
										templar_stored->orderQueueTail->orderId == OrderId::WarpingDarkArchon
										)
										helper::removeOrderFromUnitQueue(templar_stored, templar_stored->orderQueueTail);
									else
										bKeepLooping = false;

							}

							templar_stored->performAnotherOrder(
								OrderId::WarpingDarkArchon,
								templar_merge_partner->sprite->position.x,
								templar_merge_partner->sprite->position.y,
								templar_merge_partner,
								UnitId::None,
								0
							);



						}

						prepareForNextOrder(templar_stored);

						templar_merge_partner->userActionFlags |= 1;

						if (templar_merge_partner->mainOrderId != OrderId::Die)
						{

							bool bKeepLooping = true;

							while (bKeepLooping)
							{

								if (templar_merge_partner->orderQueueTail == nullptr)
									bKeepLooping = false;
								else
									if (orders_dat::CanBeInterrupted[templar_merge_partner->orderQueueTail->orderId] ||
										templar_merge_partner->orderQueueTail->orderId == OrderId::WarpingDarkArchon
										)
										helper::removeOrderFromUnitQueue(templar_merge_partner, templar_merge_partner->orderQueueTail);
									else
										bKeepLooping = false;

							}

							templar_merge_partner->performAnotherOrder(
								OrderId::WarpingDarkArchon,
								templar_stored->sprite->position.x,
								templar_stored->sprite->position.y,
								templar_stored,
								UnitId::None,
								0
							);

						}

						prepareForNextOrder(templar_merge_partner);

					}

				}

				templars_stored_iterator++;
				templars_stored_count--;

			}

		}

		scbw::refreshConsole();

	}

	;

	void CMDRECV_MergeArchon()
	{

		CUnit* current_unit;
		CUnit* templar_stored;
		CUnit* templar_merge_partner;
		CUnit* templars_stored[SELECTION_ARRAY_LENGTH];
		CUnit** templars_stored_iterator;
		s32 templars_stored_count;

		*selectionIndexStart = 0;
		current_unit = getActivePlayerNextSelection();

		if (current_unit != nullptr &&
			current_unit->canUseTech(TechId::ArchonWarp, *ACTIVE_NATION_ID) == 1)
		{

			for (s32 i = 0; i < SELECTION_ARRAY_LENGTH; i++)
				templars_stored[i] = nullptr;

			templars_stored_count = 0;

			while (current_unit != nullptr)
			{

				if (current_unit->id == UnitId::ProtossCantavis)
				{
					templars_stored[templars_stored_count] = current_unit;
					templars_stored_count++;
				}

				current_unit = getActivePlayerNextSelection();

			}

			templars_stored_iterator = &templars_stored[1];
			templars_stored_count--;

			while (templars_stored_count > 0)
			{

				templar_stored = templars_stored_iterator[-1];

				if (templar_stored != nullptr)
				{

					templar_merge_partner = helper::templarMergePartner(templar_stored, templars_stored_iterator, templars_stored_count);

					if (templar_merge_partner != nullptr)
					{

						templar_stored->userActionFlags |= 1;

						if (templar_stored->mainOrderId != OrderId::Die)
						{

							bool bKeepLooping = true;

							while (bKeepLooping)
							{

								if (templar_stored->orderQueueTail == nullptr)
									bKeepLooping = false;
								else
									if (orders_dat::CanBeInterrupted[templar_stored->orderQueueTail->orderId] ||
										templar_stored->orderQueueTail->orderId == OrderId::WarpingArchon
										)
										helper::removeOrderFromUnitQueue(templar_stored, templar_stored->orderQueueTail);
									else
										bKeepLooping = false;

							}

							templar_stored->performAnotherOrder(
								OrderId::WarpingArchon,
								templar_merge_partner->sprite->position.x,
								templar_merge_partner->sprite->position.y,
								templar_merge_partner,
								UnitId::None,
								0
							);



						}

						prepareForNextOrder(templar_stored);

						templar_merge_partner->userActionFlags |= 1;

						if (templar_merge_partner->mainOrderId != OrderId::Die)
						{

							bool bKeepLooping = true;

							while (bKeepLooping)
							{

								if (templar_merge_partner->orderQueueTail == nullptr)
									bKeepLooping = false;
								else
									if (orders_dat::CanBeInterrupted[templar_merge_partner->orderQueueTail->orderId] ||
										templar_merge_partner->orderQueueTail->orderId == OrderId::WarpingArchon
										)
										helper::removeOrderFromUnitQueue(templar_merge_partner, templar_merge_partner->orderQueueTail);
									else
										bKeepLooping = false;

							}

							templar_merge_partner->performAnotherOrder(
								OrderId::WarpingArchon,
								templar_stored->sprite->position.x,
								templar_stored->sprite->position.y,
								templar_stored,
								UnitId::None,
								0
							);

						}

						prepareForNextOrder(templar_merge_partner);

					}

				}

				templars_stored_iterator++;
				templars_stored_count--;

			}

		}

		scbw::refreshConsole();

	}
}