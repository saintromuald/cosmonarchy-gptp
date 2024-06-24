#include "UnitFinder.h"
#include "api.h"
#include <cassert>
#include "hooks/limits/ccmu.h"
#include "globals.h"

namespace bw
{
	UnitFinder::UnitFinder() : unitCount(0) {}

	UnitFinder::UnitFinder(s32 left, s32 top, s32 right, s32 bottom)
		: unitCount(0)
	{
		this->search(left, top, right, bottom);
	}

	s32 UnitFinder::get_unit_count() const
	{
		return this->unitCount;
	}

	s32 UnitFinder::get_unit_count(CallbackMatch callback) const
	{
		s32 count = 0;
		for (Unit* unit : units)
		{
			if (callback(unit)) count++;
		}
		return count;
	}

	Unit* UnitFinder::get_unit(s32 index) const
	{
		if (0 <= index && index <= this->unitCount)
			return this->units[index];
		return nullptr;
	}

	UnitFinderData* UnitFinder::get_start_x()
	{
		return g_unit_ordering_x;
	}

	UnitFinderData* UnitFinder::get_start_y()
	{
		return g_unit_ordering_y;
	}

	UnitFinderData* UnitFinder::get_end_x()
	{
		return g_unit_ordering_x + *g_unit_ordering_count;
	}

	UnitFinderData* UnitFinder::get_end_y()
	{
		return g_unit_ordering_y + *g_unit_ordering_count;
	}

	// The heart and core of StarCraft's unit search engine.
	// Based on BWAPI's Shared/Templates.h
	void UnitFinder::search(s32 left, s32 top, s32 right, s32 bottom)
	{
		// 0: No match, 1: Passed X iteration, 2: Passed Y iteration
		// (Necessary because each UnitFinderData array has 2 entries per unit)
		u8 unitFilter[UNIT_ARRAY_LENGTH + 1] = {0};

		s32 r = right, b = bottom;
		const bool isWidthExtended = right - left - 1 < *g_max_unit_width;
		const bool isHeightExtended = top - bottom - 1 < *g_max_unit_height;

		// Check if the location is smaller than the largest unit
		if (isWidthExtended)
		{
			r += *g_max_unit_width;
		}
		if (isHeightExtended)
		{
			b += *g_max_unit_height;
		}

		// Create UnitFinderData elements for compatibility with stl functions
		UnitFinderData finderVal;

		// Search for the values using built-in binary search algorithm and comparator
		finderVal.position = left;
		UnitFinderData* pLeft = std::lower_bound(get_start_x(), get_end_x(), finderVal);

		finderVal.position = top;
		UnitFinderData* pTop = std::lower_bound(get_start_y(), get_end_y(), finderVal);

		finderVal.position = r;
		UnitFinderData* pRight = std::lower_bound(pLeft, get_end_x(), finderVal);

		finderVal.position = b;
		UnitFinderData* pBottom = std::lower_bound(pTop, get_end_y(), finderVal);

		// Iterate the Y entries of the finder
		for (UnitFinderData* py = pTop; py < pBottom; ++py)
		{
			if (unitFilter[py->unit_index] == 0)
			{
				// If height is small, check unit bounds
				if (!isHeightExtended || Unit::get_unit_from_index(py->unit_index)->get_top() < bottom)
				{
					unitFilter[py->unit_index] = 1;
				}
			}
		}

		// Iterate the X entries of the finder
		this->unitCount = 0;
		for (UnitFinderData* px = pLeft; px < pRight; ++px)
		{
			if (unitFilter[px->unit_index] == 1)
			{
				// If width is small, check unit bounds
				if (!isWidthExtended || Unit::get_unit_from_index(px->unit_index)->get_left() < right)
				{
					Unit* unit = Unit::get_unit_from_index(px->unit_index);
					if (unit)
					{
						this->units[this->unitCount++] = unit;
					}
				}
				unitFilter[px->unit_index] = 0; //Prevent duplicates
			}
		}
	}

	//-------- UnitFinder::getNearest() family --------//

	// Based on function @ 0x004E8320
	Unit* UnitFinder::getNearest(s32 x, s32 y, s32 boundsLeft, s32 boundsTop, s32 boundsRight, s32 boundsBottom,
		UnitFinderData* left, UnitFinderData* top,
		UnitFinderData* right, UnitFinderData* bottom,
		CallbackMatch match, Unit* source_unit)
	{
		s32 bestDistance = bw::distance_between(0, 0, std::max(x - boundsLeft, boundsRight - x), std::max(y - boundsTop, boundsBottom - y));
		Unit* bestUnit = nullptr;
		bool canContinueSearch, hasFoundBestUnit;

		do
		{
			canContinueSearch = false;
			hasFoundBestUnit = false;
			//Search to the left
			if (get_start_x() <= left)
			{
				Unit* unit = Unit::get_unit_from_index(left->unit_index);

				if (boundsLeft <= unit->get_x())
				{
					if (boundsTop <= unit->get_y() && unit->get_y() < boundsBottom)
					{
						if (unit != source_unit && match(unit))
						{
							s32 distance = bw::distance_between(x, y, unit->get_x(), unit->get_y());
							if (distance < bestDistance)
							{
								bestDistance = distance;
								bestUnit = unit;
								hasFoundBestUnit = true;
							}
						}
					}
				}
				else
				{
					left = get_start_x() - 1;
				}
				canContinueSearch = true;
				--left;
			}

			// Search to the right
			if (right < get_end_x())
			{
				Unit* unit = Unit::get_unit_from_index(right->unit_index);
				if (unit->get_x() < boundsRight)
				{
					if (boundsTop <= unit->get_y() && unit->get_y() < boundsBottom)
					{
						if (unit != source_unit && match(unit))
						{
							s32 distance = bw::distance_between(x, y, unit->get_x(), unit->get_y());
							if (distance < bestDistance)
							{
								bestDistance = distance;
								bestUnit = unit;
								hasFoundBestUnit = true;
							}
						}
					}
				}
				else
				{
					right = get_end_x();
				}
				canContinueSearch = true;
				++right;
			}

			// Search upwards
			if (get_start_y() <= top)
			{
				Unit* unit = Unit::get_unit_from_index(top->unit_index);
				if (boundsTop <= unit->get_y())
				{
					if (boundsLeft <= unit->get_x() && unit->get_x() < boundsRight)
					{
						if (unit != source_unit && match(unit))
						{
							s32 distance = bw::distance_between(x, y, unit->get_x(), unit->get_y());
							if (distance < bestDistance)
							{
								bestDistance = distance;
								bestUnit = unit;
								hasFoundBestUnit = true;
							}
						}
					}
				}
				else
				{
					top = get_start_y() - 1;
				}
				canContinueSearch = true;
				--top;
			}

			//Search downwards
			if (bottom < get_end_y())
			{
				Unit* unit = Unit::get_unit_from_index(bottom->unit_index);
				if (unit->get_y() < boundsBottom)
				{
					if (boundsLeft <= unit->get_x() && unit->get_x() < boundsRight)
					{
						if (unit != source_unit && match(unit))
						{
							s32 distance = bw::distance_between(x, y, unit->get_x(), unit->get_y());
							if (distance < bestDistance)
							{
								bestDistance = distance;
								bestUnit = unit;
								hasFoundBestUnit = true;
							}
						}
					}
				}
				else
				{
					bottom = get_end_y();
				}
				canContinueSearch = true;
				++bottom;
			}

			// Reduce the search bounds
			if (hasFoundBestUnit)
			{
				boundsLeft = std::max(boundsLeft, x - bestDistance);
				boundsRight = std::min(boundsRight, x + bestDistance);
				boundsTop = std::max(boundsTop, y - bestDistance);
				boundsBottom = std::max(boundsBottom, y + bestDistance);
			}
		} while (canContinueSearch);
		return bestUnit;
	}

	// for bullets
	Unit* UnitFinder::getNearest(s32 x, s32 y, s32 boundsLeft, s32 boundsTop, s32 boundsRight, s32 boundsBottom,
		UnitFinderData* left, UnitFinderData* top,
		UnitFinderData* right, UnitFinderData* bottom,
		CallbackMatch match, Bullet* sourceBullet)
	{
		s32 bestDistance = bw::distance_between(0, 0, std::max(x - boundsLeft, boundsRight - x), std::max(y - boundsTop, boundsBottom - y));
		Unit* bestUnit = nullptr;
		bool canContinueSearch, hasFoundBestUnit;

		do
		{
			canContinueSearch = false;
			hasFoundBestUnit = false;
			//Search to the left
			if (get_start_x() <= left)
			{
				Unit* unit = Unit::get_unit_from_index(left->unit_index);
				if (boundsLeft <= unit->get_x())
				{
					if (boundsTop <= unit->get_y() && unit->get_y() < boundsBottom)
					{
						if (match(unit))
						{
							s32 distance = bw::distance_between(x, y, unit->get_x(), unit->get_y());
							if (distance < bestDistance)
							{
								bestDistance = distance;
								bestUnit = unit;
								hasFoundBestUnit = true;
							}
						}
					}
				}
				else
				{
					left = get_start_x() - 1;
				}
				canContinueSearch = true;
				--left;
			}

			// Search to the right
			if (right < get_end_x())
			{
				Unit* unit = Unit::get_unit_from_index(right->unit_index);
				if (unit->get_x() < boundsRight)
				{
					if (boundsTop <= unit->get_y() && unit->get_y() < boundsBottom)
					{
						if (match(unit))
						{
							s32 distance = bw::distance_between(x, y, unit->get_x(), unit->get_y());
							if (distance < bestDistance)
							{
								bestDistance = distance;
								bestUnit = unit;
								hasFoundBestUnit = true;
							}
						}
					}
				}
				else
				{
					right = get_end_x();
				}
				canContinueSearch = true;
				++right;
			}

			// Search upwards
			if (get_start_y() <= top)
			{
				Unit* unit = Unit::get_unit_from_index(top->unit_index);
				if (boundsTop <= unit->get_y())
				{
					if (boundsLeft <= unit->get_x() && unit->get_x() < boundsRight)
					{
						if (match(unit))
						{
							s32 distance = bw::distance_between(x, y, unit->get_x(), unit->get_y());
							if (distance < bestDistance)
							{
								bestDistance = distance;
								bestUnit = unit;
								hasFoundBestUnit = true;
							}
						}
					}
				}
				else
				{
					top = get_start_y() - 1;
				}
				canContinueSearch = true;
				--top;
			}

			// Search downwards
			if (bottom < get_end_y())
			{
				Unit* unit = Unit::get_unit_from_index(bottom->unit_index);
				if (unit->get_y() < boundsBottom)
				{
					if (boundsLeft <= unit->get_x() && unit->get_x() < boundsRight)
					{
						if (match(unit))
						{
							s32 distance = bw::distance_between(x, y, unit->get_x(), unit->get_y());
							if (distance < bestDistance)
							{
								bestDistance = distance;
								bestUnit = unit;
								hasFoundBestUnit = true;
							}
						}
					}
				}
				else
				{
					bottom = get_end_y();
				}
				canContinueSearch = true;
				++bottom;
			}
			// Reduce the search bounds
			if (hasFoundBestUnit)
			{
				boundsLeft = std::max(boundsLeft, x - bestDistance);
				boundsRight = std::min(boundsRight, x + bestDistance);
				boundsTop = std::max(boundsTop, y - bestDistance);
				boundsBottom = std::max(boundsBottom, y + bestDistance);
			}
		}
		while (canContinueSearch);

		return bestUnit;
	}

	// for points
	Unit* UnitFinder::getNearest(s32 x, s32 y, 	s32 boundsLeft, s32 boundsTop, s32 boundsRight, s32 boundsBottom,
		UnitFinderData* left, UnitFinderData* top,
		UnitFinderData* right, UnitFinderData* bottom,
		CallbackMatch match, Vec32 sourcePosition)
	{
		s32 bestDistance = bw::distance_between(0, 0, std::max(x - boundsLeft, boundsRight - x), std::max(y - boundsTop, boundsBottom - y));
		Unit* bestUnit = nullptr;
		bool canContinueSearch, hasFoundBestUnit;

		do
		{
			canContinueSearch = false;
			hasFoundBestUnit = false;
			// Search to the left
			if (get_start_x() <= left)
			{
				Unit* unit = Unit::get_unit_from_index(left->unit_index);
				if (boundsLeft <= unit->get_x())
				{
					if (boundsTop <= unit->get_y() && unit->get_y() < boundsBottom)
					{
						if (match(unit))
						{
							s32 distance = bw::distance_between(x, y, unit->get_x(), unit->get_y());
							if (distance < bestDistance)
							{
								bestDistance = distance;
								bestUnit = unit;
								hasFoundBestUnit = true;
							}
						}
					}
				}
				else
				{
					left = get_start_x() - 1;
				}
				canContinueSearch = true;
				--left;
			}

			// Search to the right
			if (right < get_end_x())
			{
				Unit* unit = Unit::get_unit_from_index(right->unit_index);
				if (unit->get_x() < boundsRight)
				{
					if (boundsTop <= unit->get_y() && unit->get_y() < boundsBottom)
					{
						if (match(unit))
						{
							s32 distance = bw::distance_between(x, y, unit->get_x(), unit->get_y());
							if (distance < bestDistance)
							{
								bestDistance = distance;
								bestUnit = unit;
								hasFoundBestUnit = true;
							}
						}
					}
				}
				else
				{
					right = get_end_x();
				}
				canContinueSearch = true;
				++right;
			}

			// Search upwards
			if (get_start_y() <= top)
			{
				Unit* unit = Unit::get_unit_from_index(top->unit_index);
				if (boundsTop <= unit->get_y())
				{
					if (boundsLeft <= unit->get_x() && unit->get_x() < boundsRight)
					{
						if (match(unit))
						{
							s32 distance = bw::distance_between(x, y, unit->get_x(), unit->get_y());
							if (distance < bestDistance)
							{
								bestDistance = distance;
								bestUnit = unit;
								hasFoundBestUnit = true;
							}
						}
					}
				}
				else
				{
					top = get_start_y() - 1;
				}
				canContinueSearch = true;
				--top;
			}

			// Search downwards
			if (bottom < get_end_y())
			{
				Unit* unit = Unit::get_unit_from_index(bottom->unit_index);
				if (unit->get_y() < boundsBottom)
				{
					if (boundsLeft <= unit->get_x() && unit->get_x() < boundsRight)
					{
						if (match(unit))
						{
							s32 distance = bw::distance_between(x, y, unit->get_x(), unit->get_y());
							if (distance < bestDistance)
							{
								bestDistance = distance;
								bestUnit = unit;
								hasFoundBestUnit = true;
							}
						}
					}
				}
				else
				{
					bottom = get_end_y();
				}
				canContinueSearch = true;
				++bottom;
			}
			// Reduce the search bounds
			if (hasFoundBestUnit)
			{
				boundsLeft = std::max(boundsLeft, x - bestDistance);
				boundsRight = std::min(boundsRight, x + bestDistance);
				boundsTop = std::max(boundsTop, y - bestDistance);
				boundsBottom = std::max(boundsBottom, y + bestDistance);
			}
		} while (canContinueSearch);
		return bestUnit;
	}


	Unit* UnitFinder::get_nearest_target(s32 left, s32 top, s32 right, s32 bottom, Unit* source_unit, CallbackMatch match)
	{
		UnitFinderData* searchLeft, * searchTop, * searchRight, * searchBottom;

		// If the unit sprite is hidden
		if (source_unit->sprite->flags & SpriteFlags::Hidden)
		{
			UnitFinderData temp;
			temp.position = source_unit->get_x();
			searchRight = std::lower_bound(get_start_x(), get_end_x(), temp);
			searchLeft = searchRight - 1;
			temp.position = source_unit->get_y();
			searchBottom = std::lower_bound(get_start_y(), get_end_y(), temp);
			searchTop = searchBottom - 1;
		} else {
			searchLeft = get_start_x() + source_unit->finder_index.right - 1;
			searchRight = get_start_x() + source_unit->finder_index.left + 1;
			searchTop = get_start_y() + source_unit->finder_index.bottom - 1;
			searchBottom = get_start_y() + source_unit->finder_index.top + 1;
		}

		return getNearest(source_unit->get_x(), source_unit->get_y(),
			left, top, right, bottom,
			searchLeft, searchTop, searchRight, searchBottom,
			match, source_unit);
	}

	Unit* UnitFinder::get_nearest_target(s32 left, s32 top, s32 right, s32 bottom, Bullet* sourceBullet, CallbackMatch match)
	{
		UnitFinderData* searchLeft, * searchTop, * searchRight, * searchBottom;

		//If the unit sprite is hidden
		UnitFinderData temp;
		temp.position = sourceBullet->position.x;
		searchRight = std::lower_bound(get_start_x(), get_end_x(), temp);
		searchLeft = searchRight - 1;
		temp.position = sourceBullet->position.y;
		searchBottom = std::lower_bound(get_start_y(), get_end_y(), temp);
		searchTop = searchBottom - 1;


		return getNearest(sourceBullet->position.x, sourceBullet->position.y,
			left, top, right, bottom,
			searchLeft, searchTop, searchRight, searchBottom,
			match, sourceBullet);
	}

	Unit* UnitFinder::get_nearest_target(s32 left, s32 top, s32 right, s32 bottom, Vec32 sourcePosition, CallbackMatch match)
	{
		UnitFinderData* searchLeft, * searchTop, * searchRight, * searchBottom;

		// If the unit sprite is hidden
		UnitFinderData temp;
		temp.position = sourcePosition.x;
		searchRight = std::lower_bound(get_start_x(), get_end_x(), temp);
		searchLeft = searchRight - 1;
		temp.position = sourcePosition.y;
		searchBottom = std::lower_bound(get_start_y(), get_end_y(), temp);
		searchTop = searchBottom - 1;
		return getNearest(sourcePosition.x, sourcePosition.y,
			left, top, right, bottom,
			searchLeft, searchTop, searchRight, searchBottom,
			match, sourcePosition);
	}

	Unit* UnitFinder::get_nearest_target(Unit* source_unit, CallbackMatch match)
	{
		return get_nearest_target(0, 0, g_map_tile_size->width * 32, g_map_tile_size->height * 32, source_unit, match);
	}

	//-------- Template member function definitions --------//
	s32 UnitFinder::get_matching_unit_count(CallbackMatch callback)
	{
		s32 count = 0;
		for (s32 i = 0; i < this->get_unit_count(); ++i)
		{
			if (callback(this->get_unit(i)))
			{
				count++;
			}
		}
		return count;
	}

	UnitFinder& UnitFinder::filter(CallbackMatch callback)
	{
		s32 newUnitCount = 0;
		for (s32 i = 0; i < this->get_unit_count(); ++i)
		{
			Unit* unit = this->get_unit(i);
			if (callback(unit))
			{
				units[newUnitCount++] = unit;
			}
		}
		this->unitCount = newUnitCount;
		return *this;
	}

	void UnitFinder::for_each(CallbackProc callback) const
	{
		for (s32 i = 0; i < this->get_unit_count(); ++i)
		{
			callback(this->get_unit(i));
		}
	}

	std::vector<Unit*> UnitFinder::get_all_units()
	{
		std::vector<Unit*> vec;
		for (s32 i = 0; i < this->get_unit_count(); ++i)
		{
			vec.push_back(this->get_unit(i));
		}
		return vec;
	}

	Unit* UnitFinder::get_first(CallbackMatch callback) const
	{
		for (s32 i = 0; i < this->get_unit_count(); ++i)
		{
			if (callback(this->get_unit(i)))
			{
				return this->get_unit(i);
			}
		}
		return nullptr;
	}

	Unit* UnitFinder::get_best(CallbackScore callback) const
	{
		s32 bestScore = -1;
		Unit* bestUnit = nullptr;
		for (s32 i = 0; i < this->get_unit_count(); ++i)
		{
			const s32 score = callback(this->get_unit(i));
			if (score > bestScore)
			{
				bestUnit = this->get_unit(i);
				bestScore = score;
			}
		}
		return bestUnit;
	}
}