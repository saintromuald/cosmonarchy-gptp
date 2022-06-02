#include "CMDRECV_MergeArchon.h"
#include <SCBW/api.h>

namespace hooks
{

	void CMDRECV_MergeDarkArchon()
	{

		Unit* current_unit;
		Unit* templar_stored;
		Unit* templar_merge_partner;
		Unit* templars_stored[SELECTION_ARRAY_LENGTH];
		Unit** templars_stored_iterator;
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

					templar_merge_partner = helper::templar_merge_partner(templar_stored, templars_stored_iterator, templars_stored_count);

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
										helper::remove_order_from_unit_queue(templar_stored->orderQueueTail, templar_stored);
									else
										bKeepLooping = false;

							}

							templar_stored->perform_another_order(
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
										helper::remove_order_from_unit_queue(templar_merge_partner->orderQueueTail, templar_merge_partner);
									else
										bKeepLooping = false;

							}

							templar_merge_partner->perform_another_order(
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

		scbw::refresh_console();

	}

	;

	void CMDRECV_MergeArchon()
	{

		Unit* current_unit;
		Unit* templar_stored;
		Unit* templar_merge_partner;
		Unit* templars_stored[SELECTION_ARRAY_LENGTH];
		Unit** templars_stored_iterator;
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

					templar_merge_partner = helper::templar_merge_partner(templar_stored, templars_stored_iterator, templars_stored_count);

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
										helper::remove_order_from_unit_queue(templar_stored->orderQueueTail, templar_stored);
									else
										bKeepLooping = false;

							}

							templar_stored->perform_another_order(
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
										helper::remove_order_from_unit_queue(templar_merge_partner->orderQueueTail, templar_merge_partner);
									else
										bKeepLooping = false;

							}

							templar_merge_partner->perform_another_order(
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

		scbw::refresh_console();

	}
}