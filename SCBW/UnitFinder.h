#pragma once
#include "scbwdata.h"
#include <algorithm>
#include <functional>
#include <vector>

//V241 for VS2008

namespace scbw {

	using UnitFinderCallbackProcInterface = std::function<void(CUnit*)>;
	using UnitFinderCallbackMatchInterface = std::function<bool(CUnit*)>;
	using UnitFinderCallbackScoreInterface = std::function<s32(CUnit*)>;

	/// The UnitFinder class is used to efficiently search for units in a certain
	/// area using StarCraft's internal data structures.

	class UnitFinder {
	public:
		/// Default constructor.
		UnitFinder();

		/// Constructs and searches for all units within the given bounds.
		UnitFinder(s32 left, s32 top, s32 right, s32 bottom);

		/// Searches for all units within the given bounds, using StarCraft's
		/// internal data structures. This is much more efficient than manually
		/// looping through the entire unit table or linked list.
		void search(s32 left, s32 top, s32 right, s32 bottom);

		//-------- Template member function definitions --------//

		/// Iterates through all units found, calling callback.proc() once for each
		/// unit.
		UnitFinder& filter(UnitFinderCallbackMatchInterface callback);
		void forEach(UnitFinderCallbackProcInterface callback) const;

		template<typename Func>
		void forEachInMatch(UnitFinderCallbackMatchInterface callback, Func f) {
			for (s32 i = 0; i < this->getUnitCount(); ++i) {
				if (callback(this->getUnit(i))) {
					f(this->getUnit(i));
				}
			}
		}

		std::vector<CUnit*> getAllUnits();

		s32 getMatchUnitCount(UnitFinderCallbackMatchInterface callback);
		/// Returns the first unit for which callback.match() returns true.
		/// If there are no matches, returns nullptr.
		CUnit* getFirst(UnitFinderCallbackMatchInterface callback) const;

		/// Returns the unit for which callback.score() returns the highest
		/// nonnegative score. If there are no units, returns nullptr.
		/// Note: If callback.score() returns a negative value, the unit is ignored.
		CUnit* getBest(UnitFinderCallbackScoreInterface callback) const;

		/// Returns the number of units found by UnitFinder::search() call.
		/// If no searches have been conducted, returns 0.
		s32 getUnitCount() const;
		s32 UnitFinder::getUnitCount(UnitFinderCallbackMatchInterface matchInterface) const;

		/// Returns the unit at the given index. Invalid index returns nullptr instead.
		CUnit* getUnit(s32 index) const;

		/// Searches the area given by (@p left, @p top, @p right, @p bottom),
		/// returning the nearest unit to @p sourceUnit for which match(unit)
		/// evaluates to true. If there are no matches, returns nullptr.
		/// This does not use unit collision boxes for calculating distances.
		static CUnit* getNearestTarget(s32 left, s32 top, s32 right, s32 bottom,
			CUnit* sourceUnit, UnitFinderCallbackMatchInterface match);
		static CUnit* getNearestTarget(s32 left, s32 top, s32 right, s32 bottom,
			CBullet* sourceBullet, UnitFinderCallbackMatchInterface match);
		static CUnit* getNearestTarget(s32 left, s32 top, s32 right, s32 bottom,
			Point32 position, UnitFinderCallbackMatchInterface match);
		/// Searches the entire map, returning the nearest unit to @p sourceUnit
		/// for which match(unit) evaluates to true. If there are no matches,
		/// returns nullptr.
		/// This does not use unit collision boxes for calculating distances.
		static CUnit* getNearestTarget(CUnit* sourceUnit, UnitFinderCallbackMatchInterface match);

	private:
		//This function is meant to be used by other getNearest() functions.
		//Do NOT use this function in the game code!
		static CUnit* getNearest(s32 x, s32 y,
			s32 boundsLeft, s32 boundsTop, s32 boundsRight, s32 boundsBottom,
			UnitFinderData* left, UnitFinderData* top,
			UnitFinderData* right, UnitFinderData* bottom,
			UnitFinderCallbackMatchInterface match, CUnit* sourceUnit);
		static CUnit* getNearest(s32 x, s32 y,
			s32 boundsLeft, s32 boundsTop, s32 boundsRight, s32 boundsBottom,
			UnitFinderData* left, UnitFinderData* top,
			UnitFinderData* right, UnitFinderData* bottom,
			UnitFinderCallbackMatchInterface match, CBullet* sourceBullet);
		static CUnit* getNearest(s32 x, s32 y,
			s32 boundsLeft, s32 boundsTop, s32 boundsRight, s32 boundsBottom,
			UnitFinderData* left, UnitFinderData* top,
			UnitFinderData* right, UnitFinderData* bottom,
			UnitFinderCallbackMatchInterface match, Point32 sourcePosition);
		static UnitFinderData* getStartX();
		static UnitFinderData* getStartY();
		static UnitFinderData* getEndX();
		static UnitFinderData* getEndY();

		s32 unitCount;
		CUnit* units[UNIT_ARRAY_LENGTH];
	};

} //scbw
