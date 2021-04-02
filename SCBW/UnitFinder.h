#pragma once
#include "scbwdata.h"
#include <algorithm>
#include <functional>

//V241 for VS2008

namespace scbw {

using UnitFinderCallbackProcInterface  = std::function<void(CUnit*)>;
using UnitFinderCallbackMatchInterface = std::function<bool(CUnit*)>;
using UnitFinderCallbackScoreInterface = std::function<int(CUnit*)>;

/// The UnitFinder class is used to efficiently search for units in a certain
/// area using StarCraft's internal data structures.

class UnitFinder {
  public:
    /// Default constructor.
    UnitFinder();

    /// Constructs and searches for all units within the given bounds.
    UnitFinder(int left, int top, int right, int bottom);

    /// Searches for all units within the given bounds, using StarCraft's
    /// internal data structures. This is much more efficient than manually
    /// looping through the entire unit table or linked list.
    void search(int left, int top, int right, int bottom);

	//-------- Template member function definitions --------//

    /// Iterates through all units found, calling callback.proc() once for each
    /// unit.
    void forEach(UnitFinderCallbackProcInterface callback) const;

    template<typename Func>
    void forEachInMatch(UnitFinderCallbackMatchInterface callback, Func f) {
        for (int i = 0; i < this->getUnitCount(); ++i) {
            if (callback(this->getUnit(i))) {
                f(this->getUnit(i));
            }
        }
    }
    int getMatchUnitCount(UnitFinderCallbackMatchInterface callback);
    /// Returns the first unit for which callback.match() returns true.
    /// If there are no matches, returns nullptr.
    CUnit* getFirst(UnitFinderCallbackMatchInterface callback) const;

    /// Returns the unit for which callback.score() returns the highest
    /// nonnegative score. If there are no units, returns nullptr.
    /// Note: If callback.score() returns a negative value, the unit is ignored.
    CUnit* getBest(UnitFinderCallbackScoreInterface callback) const;

    /// Returns the number of units found by UnitFinder::search() call.
    /// If no searches have been conducted, returns 0.
    int getUnitCount() const;
    int UnitFinder::getUnitCount(UnitFinderCallbackMatchInterface matchInterface) const;

    /// Returns the unit at the given index. Invalid index returns nullptr instead.
    CUnit* getUnit(int index) const;
   
    /// Searches the area given by (@p left, @p top, @p right, @p bottom),
    /// returning the nearest unit to @p sourceUnit for which match(unit)
    /// evaluates to true. If there are no matches, returns nullptr.
    /// This does not use unit collision boxes for calculating distances.
    static CUnit* getNearestTarget(int left, int top, int right, int bottom,
                                   CUnit* sourceUnit, UnitFinderCallbackMatchInterface match);
    static CUnit* getNearestTarget(int left, int top, int right, int bottom,
        CBullet* sourceBullet, UnitFinderCallbackMatchInterface match);
    static CUnit* getNearestTarget(int left, int top, int right, int bottom,
        Point32 position, UnitFinderCallbackMatchInterface match);
    /// Searches the entire map, returning the nearest unit to @p sourceUnit
    /// for which match(unit) evaluates to true. If there are no matches,
    /// returns nullptr.
    /// This does not use unit collision boxes for calculating distances.
    static CUnit* getNearestTarget(CUnit* sourceUnit, UnitFinderCallbackMatchInterface match);

  private:
    //This function is meant to be used by other getNearest() functions.
    //Do NOT use this function in the game code!
    static CUnit* getNearest(int x, int y,
      int boundsLeft, int boundsTop, int boundsRight, int boundsBottom,
      UnitFinderData* left,  UnitFinderData* top,
      UnitFinderData* right, UnitFinderData* bottom,
      UnitFinderCallbackMatchInterface match, CUnit* sourceUnit);
    static CUnit* getNearest(int x, int y,
        int boundsLeft, int boundsTop, int boundsRight, int boundsBottom,
        UnitFinderData* left, UnitFinderData* top,
        UnitFinderData* right, UnitFinderData* bottom,
        UnitFinderCallbackMatchInterface match, CBullet* sourceBullet);
    static CUnit* getNearest(int x, int y,
        int boundsLeft, int boundsTop, int boundsRight, int boundsBottom,
        UnitFinderData* left, UnitFinderData* top,
        UnitFinderData* right, UnitFinderData* bottom,
        UnitFinderCallbackMatchInterface match, Point32 sourcePosition);
    static UnitFinderData* getStartX();
    static UnitFinderData* getStartY();
    static UnitFinderData* getEndX();
    static UnitFinderData* getEndY();

    int unitCount;
    CUnit* units[UNIT_ARRAY_LENGTH];
};

} //scbw
