#pragma once
#include "BWdata.h"
#include <algorithm>
#include <functional>
#include <vector>

//V241 for VS2008

namespace bw {

	/// The UnitFinder struct is used to efficiently search for units in a certain
	/// box using StarCraft's internal data structures.

	struct UnitFinder {
		using CallbackProc = std::function<void(Unit*)>;
		using CallbackMatch = std::function<bool(Unit*)>;
		using CallbackScore = std::function<s32(Unit*)>;

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
		UnitFinder& filter(CallbackMatch callback);
		void for_each(CallbackProc callback) const;

		template<typename Func>
		void for_each_in_match(CallbackMatch callback, Func f) {
			for (s32 i = 0; i < this->get_unit_count(); ++i) {
				if (callback(this->get_unit(i))) {
					f(this->get_unit(i));
				}
			}
		}

		std::vector<Unit*> get_all_units();

		s32 get_matching_unit_count(CallbackMatch callback);
		/// Returns the first unit for which callback.match() returns true.
		/// If there are no matches, returns nullptr.
		Unit* get_first(CallbackMatch callback) const;

		/// Returns the unit for which callback.score() returns the highest
		/// nonnegative score. If there are no units, returns nullptr.
		/// Note: If callback.score() returns a negative value, the unit is ignored.
		Unit* get_best(CallbackScore callback) const;

		/// Returns the number of units found by UnitFinder::search() call.
		/// If no searches have been conducted, returns 0.
		s32 get_unit_count() const;
		s32 get_unit_count(CallbackMatch match) const;

		/// Returns the unit at the given index. Invalid index returns nullptr instead.
		Unit* get_unit(s32 index) const;

		/// Searches the box given by (@p left, @p top, @p right, @p bottom),
		/// returning the nearest unit to @p source_unit for which match(unit)
		/// evaluates to true. If there are no matches, returns nullptr.
		/// This does not use unit collision boxes for calculating distances.
		static Unit* get_nearest_target(s32 left, s32 top, s32 right, s32 bottom,
			Unit* source_unit, CallbackMatch match);
		static Unit* get_nearest_target(s32 left, s32 top, s32 right, s32 bottom,
			Bullet* sourceBullet, CallbackMatch match);
		static Unit* get_nearest_target(s32 left, s32 top, s32 right, s32 bottom,
			v2s32 position, CallbackMatch match);
		/// Searches the entire map, returning the nearest unit to @p source_unit
		/// for which match(unit) evaluates to true. If there are no matches,
		/// returns nullptr.
		/// This does not use unit collision boxes for calculating distances.
		static Unit* get_nearest_target(Unit* source_unit, CallbackMatch match);

			//This function is meant to be used by other getNearest() functions.
		//Do NOT use this function in the game code!
		static Unit* getNearest(s32 x, s32 y,
			s32 boundsLeft, s32 boundsTop, s32 boundsRight, s32 boundsBottom,
			UnitFinderData* left, UnitFinderData* top,
			UnitFinderData* right, UnitFinderData* bottom,
			CallbackMatch match, Unit* source_unit);
		static Unit* getNearest(s32 x, s32 y,
			s32 boundsLeft, s32 boundsTop, s32 boundsRight, s32 boundsBottom,
			UnitFinderData* left, UnitFinderData* top,
			UnitFinderData* right, UnitFinderData* bottom,
			CallbackMatch match, Bullet* sourceBullet);
		static Unit* getNearest(s32 x, s32 y,
			s32 boundsLeft, s32 boundsTop, s32 boundsRight, s32 boundsBottom,
			UnitFinderData* left, UnitFinderData* top,
			UnitFinderData* right, UnitFinderData* bottom,
			CallbackMatch match, v2s32 sourcePosition);
		static UnitFinderData* get_start_x();
		static UnitFinderData* get_start_y();
		static UnitFinderData* get_end_x();
		static UnitFinderData* get_end_y();

		s32 unitCount;
		Unit* units[UNIT_ARRAY_LENGTH];
	};

} //scbw
