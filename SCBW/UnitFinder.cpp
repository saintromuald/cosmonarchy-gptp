#include "UnitFinder.h"
#include "api.h"
#include <cassert>
#include "hooks/limits/ccmu.h"
#include "globals.h"
//V241 for VS2008

namespace scbw
{

	UnitFinder::UnitFinder() : unitCount(0) {}

	UnitFinder::UnitFinder(s32 left, s32 top, s32 right, s32 bottom)
		: unitCount(0)
	{
		this->search(left, top, right, bottom);
	}

	s32 UnitFinder::getUnitCount() const
	{
		return this->unitCount;
	}

	s32 UnitFinder::getUnitCount(UnitFinderCallbackMatchInterface callback) const
	{
		s32 count = 0;
		for (Unit* unit : units)
		{
			if (callback(unit)) count++;
		}
		return count;
	}

	Unit* UnitFinder::getUnit(s32 index) const
	{
		if (0 <= index && index <= this->unitCount)
			return this->units[index];
		return nullptr;
	}

	UnitFinderData* UnitFinder::getStartX()
	{
		if (globals.extended_unit_table)
		{
			return new_unit_table.orderingX;
		}
		return unitOrderingX;
	}

	UnitFinderData* UnitFinder::getStartY()
	{
		if (globals.extended_unit_table)
		{
			return new_unit_table.orderingY;
		}
		return unitOrderingY;
	}

	UnitFinderData* UnitFinder::getEndX()
	{
		if (globals.extended_unit_table)
		{
			return new_unit_table.orderingX + *unitOrderingCount;
		}
		return unitOrderingX + *unitOrderingCount;
	}

	UnitFinderData* UnitFinder::getEndY()
	{
		if (globals.extended_unit_table)
		{
			return new_unit_table.orderingY + *unitOrderingCount;
		}
		return unitOrderingY + *unitOrderingCount;
	}

	// The heart and core of StarCraft's unit search engine.
	// Based on BWAPI's Shared/Templates.h
	void UnitFinder::search(s32 left, s32 top, s32 right, s32 bottom)
	{
		//0: No match, 1: Passed X iteration, 2: Passed Y iteration
		//(Necessary because each UnitFinderData array has 2 entries per unit)
		u8 unitSearchFlags[UNIT_ARRAY_LENGTH + 1] = {0};

		s32 r = right, b = bottom;
		const bool isWidthExtended = right - left - 1 < *MAX_UNIT_WIDTH;
		const bool isHeightExtended = top - bottom - 1 < *MAX_UNIT_HEIGHT;

		// Check if the location is smaller than the largest unit
		if (isWidthExtended)
			r += *MAX_UNIT_WIDTH;
		if (isHeightExtended)
			b += *MAX_UNIT_HEIGHT;

		// Create UnitFinderData elements for compatibility with stl functions
		UnitFinderData finderVal;

		// Search for the values using built-in binary search algorithm and comparator
		finderVal.position = left;
		UnitFinderData* pLeft = std::lower_bound(getStartX(), getEndX(), finderVal);

		finderVal.position = top;
		UnitFinderData* pTop = std::lower_bound(getStartY(), getEndY(), finderVal);

		finderVal.position = r;
		UnitFinderData* pRight = std::lower_bound(pLeft, getEndX(), finderVal);

		finderVal.position = b;
		UnitFinderData* pBottom = std::lower_bound(pTop, getEndY(), finderVal);

		// Iterate the Y entries of the finder
		for (UnitFinderData* py = pTop; py < pBottom; ++py)
		{
			if (unitSearchFlags[py->unitIndex] == 0)
			{
				// If height is small, check unit bounds
				if (!isHeightExtended
					|| Unit::getFromIndex(py->unitIndex)->getTop() < bottom)
					unitSearchFlags[py->unitIndex] = 1;
			}
		}

		// Iterate the X entries of the finder
		this->unitCount = 0;
		for (UnitFinderData* px = pLeft; px < pRight; ++px)
		{
			if (unitSearchFlags[px->unitIndex] == 1)
			{
				// If width is small, check unit bounds
				if (!isWidthExtended
					|| Unit::getFromIndex(px->unitIndex)->getLeft() < right)
				{
					Unit* unit = Unit::getFromIndex(px->unitIndex);
					if (unit)
						this->units[this->unitCount++] = unit;
				}
				unitSearchFlags[px->unitIndex] = 0; //Prevent duplicates
			}
		}
	}

	//-------- UnitFinder::getNearest() family --------//

	//Based on function @ 0x004E8320
	Unit* UnitFinder::getNearest(s32 x, s32 y,
		s32 boundsLeft, s32 boundsTop, s32 boundsRight, s32 boundsBottom,
		UnitFinderData* left, UnitFinderData* top,
		UnitFinderData* right, UnitFinderData* bottom,
		UnitFinderCallbackMatchInterface match, Unit* sourceUnit)
	{

		s32 bestDistance = scbw::getDistanceFast(0, 0,
			std::max(x - boundsLeft, boundsRight - x),
			std::max(y - boundsTop, boundsBottom - y));

		Unit* bestUnit = nullptr;
		bool canContinueSearch, hasFoundBestUnit;

		do
		{
			canContinueSearch = false;
			hasFoundBestUnit = false;

			//Search to the left
			if (getStartX() <= left)
			{
				Unit* unit = Unit::getFromIndex(left->unitIndex);

				if (boundsLeft <= unit->getX())
				{
					if (boundsTop <= unit->getY() && unit->getY() < boundsBottom)
					{
						if (unit != sourceUnit && match(unit))
						{
							s32 distance = scbw::getDistanceFast(x, y, unit->getX(), unit->getY());
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
					left = getStartX() - 1;

				canContinueSearch = true;
				--left;
			}

			//Search to the right
			if (right < getEndX())
			{
				Unit* unit = Unit::getFromIndex(right->unitIndex);

				if (unit->getX() < boundsRight)
				{
					if (boundsTop <= unit->getY() && unit->getY() < boundsBottom)
					{
						if (unit != sourceUnit && match(unit))
						{
							s32 distance = scbw::getDistanceFast(x, y, unit->getX(), unit->getY());
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
					right = getEndX();

				canContinueSearch = true;
				++right;
			}

			//Search upwards
			if (getStartY() <= top)
			{
				Unit* unit = Unit::getFromIndex(top->unitIndex);

				if (boundsTop <= unit->getY())
				{
					if (boundsLeft <= unit->getX() && unit->getX() < boundsRight)
					{
						if (unit != sourceUnit && match(unit))
						{
							s32 distance = scbw::getDistanceFast(x, y, unit->getX(), unit->getY());
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
					top = getStartY() - 1;

				canContinueSearch = true;
				--top;
			}

			//Search downwards
			if (bottom < getEndY())
			{
				Unit* unit = Unit::getFromIndex(bottom->unitIndex);

				if (unit->getY() < boundsBottom)
				{
					if (boundsLeft <= unit->getX() && unit->getX() < boundsRight)
					{
						if (unit != sourceUnit && match(unit))
						{
							s32 distance = scbw::getDistanceFast(x, y, unit->getX(), unit->getY());
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
					bottom = getEndY();

				canContinueSearch = true;
				++bottom;
			}

			//Reduce the search bounds
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

	//
	// for bullets
	//
	Unit* UnitFinder::getNearest(s32 x, s32 y,
		s32 boundsLeft, s32 boundsTop, s32 boundsRight, s32 boundsBottom,
		UnitFinderData* left, UnitFinderData* top,
		UnitFinderData* right, UnitFinderData* bottom,
		UnitFinderCallbackMatchInterface match, Bullet* sourceBullet)
	{

		s32 bestDistance = scbw::getDistanceFast(0, 0,
			std::max(x - boundsLeft, boundsRight - x),
			std::max(y - boundsTop, boundsBottom - y));

		Unit* bestUnit = nullptr;
		bool canContinueSearch, hasFoundBestUnit;

		do
		{
			canContinueSearch = false;
			hasFoundBestUnit = false;

			//Search to the left
			if (getStartX() <= left)
			{
				Unit* unit = Unit::getFromIndex(left->unitIndex);

				if (boundsLeft <= unit->getX())
				{
					if (boundsTop <= unit->getY() && unit->getY() < boundsBottom)
					{
						if (match(unit))
						{
							s32 distance = scbw::getDistanceFast(x, y, unit->getX(), unit->getY());
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
					left = getStartX() - 1;

				canContinueSearch = true;
				--left;
			}

			//Search to the right
			if (right < getEndX())
			{
				Unit* unit = Unit::getFromIndex(right->unitIndex);

				if (unit->getX() < boundsRight)
				{
					if (boundsTop <= unit->getY() && unit->getY() < boundsBottom)
					{
						if (match(unit))
						{
							s32 distance = scbw::getDistanceFast(x, y, unit->getX(), unit->getY());
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
					right = getEndX();

				canContinueSearch = true;
				++right;
			}

			//Search upwards
			if (getStartY() <= top)
			{
				Unit* unit = Unit::getFromIndex(top->unitIndex);

				if (boundsTop <= unit->getY())
				{
					if (boundsLeft <= unit->getX() && unit->getX() < boundsRight)
					{
						if (match(unit))
						{
							s32 distance = scbw::getDistanceFast(x, y, unit->getX(), unit->getY());
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
					top = getStartY() - 1;

				canContinueSearch = true;
				--top;
			}

			//Search downwards
			if (bottom < getEndY())
			{
				Unit* unit = Unit::getFromIndex(bottom->unitIndex);

				if (unit->getY() < boundsBottom)
				{
					if (boundsLeft <= unit->getX() && unit->getX() < boundsRight)
					{
						if (match(unit))
						{
							s32 distance = scbw::getDistanceFast(x, y, unit->getX(), unit->getY());
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
					bottom = getEndY();

				canContinueSearch = true;
				++bottom;
			}
			//Reduce the search bounds
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

	//for points
	Unit* UnitFinder::getNearest(s32 x, s32 y,
		s32 boundsLeft, s32 boundsTop, s32 boundsRight, s32 boundsBottom,
		UnitFinderData* left, UnitFinderData* top,
		UnitFinderData* right, UnitFinderData* bottom,
		UnitFinderCallbackMatchInterface match, Vector32 sourcePosition)
	{

		s32 bestDistance = scbw::getDistanceFast(0, 0,
			std::max(x - boundsLeft, boundsRight - x),
			std::max(y - boundsTop, boundsBottom - y));

		Unit* bestUnit = nullptr;
		bool canContinueSearch, hasFoundBestUnit;

		do
		{
			canContinueSearch = false;
			hasFoundBestUnit = false;

			//Search to the left
			if (getStartX() <= left)
			{
				Unit* unit = Unit::getFromIndex(left->unitIndex);

				if (boundsLeft <= unit->getX())
				{
					if (boundsTop <= unit->getY() && unit->getY() < boundsBottom)
					{
						if (match(unit))
						{
							s32 distance = scbw::getDistanceFast(x, y, unit->getX(), unit->getY());
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
					left = getStartX() - 1;

				canContinueSearch = true;
				--left;
			}

			//Search to the right
			if (right < getEndX())
			{
				Unit* unit = Unit::getFromIndex(right->unitIndex);

				if (unit->getX() < boundsRight)
				{
					if (boundsTop <= unit->getY() && unit->getY() < boundsBottom)
					{
						if (match(unit))
						{
							s32 distance = scbw::getDistanceFast(x, y, unit->getX(), unit->getY());
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
					right = getEndX();

				canContinueSearch = true;
				++right;
			}

			//Search upwards
			if (getStartY() <= top)
			{
				Unit* unit = Unit::getFromIndex(top->unitIndex);

				if (boundsTop <= unit->getY())
				{
					if (boundsLeft <= unit->getX() && unit->getX() < boundsRight)
					{
						if (match(unit))
						{
							s32 distance = scbw::getDistanceFast(x, y, unit->getX(), unit->getY());
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
					top = getStartY() - 1;

				canContinueSearch = true;
				--top;
			}

			//Search downwards
			if (bottom < getEndY())
			{
				Unit* unit = Unit::getFromIndex(bottom->unitIndex);

				if (unit->getY() < boundsBottom)
				{
					if (boundsLeft <= unit->getX() && unit->getX() < boundsRight)
					{
						if (match(unit))
						{
							s32 distance = scbw::getDistanceFast(x, y, unit->getX(), unit->getY());
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
					bottom = getEndY();

				canContinueSearch = true;
				++bottom;
			}
			//Reduce the search bounds
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


	Unit* UnitFinder::getNearestTarget(s32 left, s32 top, s32 right, s32 bottom,
		Unit* sourceUnit, UnitFinderCallbackMatchInterface match)
	{
		UnitFinderData* searchLeft, * searchTop, * searchRight, * searchBottom;

		//If the unit sprite is hidden
		if (sourceUnit->sprite->flags & SpriteFlags::Hidden)
		{
			UnitFinderData temp;
			temp.position = sourceUnit->getX();
			searchRight = std::lower_bound(getStartX(), getEndX(), temp);
			searchLeft = searchRight - 1;
			temp.position = sourceUnit->getY();
			searchBottom = std::lower_bound(getStartY(), getEndY(), temp);
			searchTop = searchBottom - 1;
		}
		else
		{
			searchLeft = getStartX() + sourceUnit->finderIndex.right - 1;
			searchRight = getStartX() + sourceUnit->finderIndex.left + 1;
			searchTop = getStartY() + sourceUnit->finderIndex.bottom - 1;
			searchBottom = getStartY() + sourceUnit->finderIndex.top + 1;
		}

		return getNearest(sourceUnit->getX(), sourceUnit->getY(),
			left, top, right, bottom,
			searchLeft, searchTop, searchRight, searchBottom,
			match, sourceUnit);
	}
	Unit* UnitFinder::getNearestTarget(s32 left, s32 top, s32 right, s32 bottom,
		Bullet* sourceBullet, UnitFinderCallbackMatchInterface match)
	{
		UnitFinderData* searchLeft, * searchTop, * searchRight, * searchBottom;

		//If the unit sprite is hidden
		UnitFinderData temp;
		temp.position = sourceBullet->position.x;
		searchRight = std::lower_bound(getStartX(), getEndX(), temp);
		searchLeft = searchRight - 1;
		temp.position = sourceBullet->position.y;
		searchBottom = std::lower_bound(getStartY(), getEndY(), temp);
		searchTop = searchBottom - 1;


		return getNearest(sourceBullet->position.x, sourceBullet->position.y,
			left, top, right, bottom,
			searchLeft, searchTop, searchRight, searchBottom,
			match, sourceBullet);
	}
	Unit* UnitFinder::getNearestTarget(s32 left, s32 top, s32 right, s32 bottom,
		Vector32 sourcePosition, UnitFinderCallbackMatchInterface match)
	{
		UnitFinderData* searchLeft, * searchTop, * searchRight, * searchBottom;

		//If the unit sprite is hidden
		UnitFinderData temp;
		temp.position = sourcePosition.x;
		searchRight = std::lower_bound(getStartX(), getEndX(), temp);
		searchLeft = searchRight - 1;
		temp.position = sourcePosition.y;
		searchBottom = std::lower_bound(getStartY(), getEndY(), temp);
		searchTop = searchBottom - 1;


		return getNearest(sourcePosition.x, sourcePosition.y,
			left, top, right, bottom,
			searchLeft, searchTop, searchRight, searchBottom,
			match, sourcePosition);
	}
	Unit* UnitFinder::getNearestTarget(Unit* sourceUnit, UnitFinderCallbackMatchInterface match)
	{
		return getNearestTarget(0, 0, mapTileSize->width * 32, mapTileSize->height * 32, sourceUnit, match);
	}

	//-------- Template member function definitions --------//
	s32 UnitFinder::getMatchUnitCount(UnitFinderCallbackMatchInterface callback)
	{
		s32 count = 0;
		for (s32 i = 0; i < this->getUnitCount(); ++i)
		{
			if (callback(this->getUnit(i)))
			{
				count++;
			}
		}
		return count;
	}

	UnitFinder& UnitFinder::filter(scbw::UnitFinderCallbackMatchInterface callback)
	{
		s32 newUnitCount = 0;
		for (s32 i = 0; i < this->getUnitCount(); ++i)
		{
			Unit* unit = this->getUnit(i);
			if (callback(unit))
			{
				units[newUnitCount++] = unit;
			}
		}
		this->unitCount = newUnitCount;
		return *this;
	}

	void UnitFinder::forEach(scbw::UnitFinderCallbackProcInterface callback) const
	{
		for (s32 i = 0; i < this->getUnitCount(); ++i)
		{
			callback(this->getUnit(i));
		}
	}

	std::vector<Unit*> UnitFinder::getAllUnits()
	{
		std::vector<Unit*> vec;
		for (s32 i = 0; i < this->getUnitCount(); ++i)
		{
			vec.push_back(this->getUnit(i));
		}
		return vec;
	}

	Unit* UnitFinder::getFirst(UnitFinderCallbackMatchInterface callback) const
	{

		for (s32 i = 0; i < this->getUnitCount(); ++i)
			if (callback(this->getUnit(i)))
				return this->getUnit(i);

		return nullptr;

	}

	Unit* UnitFinder::getBest(scbw::UnitFinderCallbackScoreInterface callback) const
	{

		s32 bestScore = -1;
		Unit* bestUnit = nullptr;

		for (s32 i = 0; i < this->getUnitCount(); ++i)
		{

			const s32 score = callback(this->getUnit(i));

			if (score > bestScore)
			{
				bestUnit = this->getUnit(i);
				bestScore = score;
			}

		}

		return bestUnit;
	}

} //scbw
