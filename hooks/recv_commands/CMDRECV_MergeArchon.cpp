#include "CMDRECV_MergeArchon.h"
#include <SCBW/api.h>

namespace hooks
{

	void CMDRECV_MergeDarkArchon()
	{

		Unit* current_unit;
		Unit* templar_stored;
		Unit* templar_merge_partner;
		Unit* templars_stored[SelectionCap];
		Unit** templars_stored_iterator;
		s32 templars_stored_count;

		*unit_iterator_pos = 0;
		current_unit = get_active_player_next_selection();

		if (current_unit != nullptr &&
			current_unit->canUseTech(TechId::DarkArchonMeld, *ACTIVE_NATION_ID) == 1)
		{

			for (s32 i = 0; i < SelectionCap; i++)
				templars_stored[i] = nullptr;

			templars_stored_count = 0;

			while (current_unit != nullptr)
			{

				if (current_unit->id == UnitId::ProtossCabalist)
				{
					templars_stored[templars_stored_count] = current_unit;
					templars_stored_count++;
				}

				current_unit = get_active_player_next_selection();

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

						templar_stored->user_action_flags |= 1;

						if (templar_stored->main_order_id != OrderId::Die)
						{

							bool bKeepLooping = true;

							while (bKeepLooping)
							{

								if (templar_stored->order_queue_tail == nullptr)
									bKeepLooping = false;
								else
									if (orders_dat::CanBeInterrupted[templar_stored->order_queue_tail->order_id] ||
										templar_stored->order_queue_tail->order_id == OrderId::WarpingDarkArchon
										)
										helper::remove_order_from_unit_queue(templar_stored->order_queue_tail, templar_stored);
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

						prepare_for_next_order(templar_stored);

						templar_merge_partner->user_action_flags |= 1;

						if (templar_merge_partner->main_order_id != OrderId::Die)
						{

							bool bKeepLooping = true;

							while (bKeepLooping)
							{

								if (templar_merge_partner->order_queue_tail == nullptr)
									bKeepLooping = false;
								else
									if (orders_dat::CanBeInterrupted[templar_merge_partner->order_queue_tail->order_id] ||
										templar_merge_partner->order_queue_tail->order_id == OrderId::WarpingDarkArchon
										)
										helper::remove_order_from_unit_queue(templar_merge_partner->order_queue_tail, templar_merge_partner);
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

						prepare_for_next_order(templar_merge_partner);

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
		Unit* templars_stored[SelectionCap];
		Unit** templars_stored_iterator;
		s32 templars_stored_count;

		*unit_iterator_pos = 0;
		current_unit = get_active_player_next_selection();

		if (current_unit != nullptr &&
			current_unit->canUseTech(TechId::ArchonWarp, *ACTIVE_NATION_ID) == 1)
		{

			for (s32 i = 0; i < SelectionCap; i++)
				templars_stored[i] = nullptr;

			templars_stored_count = 0;

			while (current_unit != nullptr)
			{

				if (current_unit->id == UnitId::ProtossCantavis)
				{
					templars_stored[templars_stored_count] = current_unit;
					templars_stored_count++;
				}

				current_unit = get_active_player_next_selection();

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

						templar_stored->user_action_flags |= 1;

						if (templar_stored->main_order_id != OrderId::Die)
						{

							bool bKeepLooping = true;

							while (bKeepLooping)
							{

								if (templar_stored->order_queue_tail == nullptr)
									bKeepLooping = false;
								else
									if (orders_dat::CanBeInterrupted[templar_stored->order_queue_tail->order_id] ||
										templar_stored->order_queue_tail->order_id == OrderId::WarpingArchon
										)
										helper::remove_order_from_unit_queue(templar_stored->order_queue_tail, templar_stored);
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

						prepare_for_next_order(templar_stored);

						templar_merge_partner->user_action_flags |= 1;

						if (templar_merge_partner->main_order_id != OrderId::Die)
						{

							bool bKeepLooping = true;

							while (bKeepLooping)
							{

								if (templar_merge_partner->order_queue_tail == nullptr)
									bKeepLooping = false;
								else
									if (orders_dat::CanBeInterrupted[templar_merge_partner->order_queue_tail->order_id] ||
										templar_merge_partner->order_queue_tail->order_id == OrderId::WarpingArchon
										)
										helper::remove_order_from_unit_queue(templar_merge_partner->order_queue_tail, templar_merge_partner);
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

						prepare_for_next_order(templar_merge_partner);

					}

				}

				templars_stored_iterator++;
				templars_stored_count--;

			}

		}

		scbw::refresh_console();

	}
}