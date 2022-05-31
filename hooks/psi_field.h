/// Support for adding/removing psi fields. To use, you will also have to modify the following hooks:
///   hooks/unit_destructor_special.cpp
///   hooks/unit_selection_graphics.cpp

// NOT FULLY REVALIDATED BY UNDEADSTAR

#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {

/// Edit these functions in psi_field.cpp.
bool isReadyToMakePsiField(Unit* unit);
bool canMakePsiField(u16 unitId);

/// This function must be called once per frame in next_frame().
/// This uses canMakePsiField() and isReadyToMakePsiField() internally.
void update_psi_field_providers();

//Call this in initialize.h.
void injectPsiFieldHooks();

}