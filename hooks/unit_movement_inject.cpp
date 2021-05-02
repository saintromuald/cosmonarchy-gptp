#include "unit_movement.h"
#include <hook_tools.h>

namespace
{
    void __declspec(naked) unitMovementDormant_Wrapper()
    { // 0046A8D0
        static CUnit* unit;
        static s32 result;

        __asm {
            MOV unit, EAX
            PUSHAD
        }
        result = hooks::unitMovementDormant(unit);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }
}

namespace hooks
{
    void injectUnitMovementHooks()
    {
        jmpPatch(unitMovementDormant_Wrapper, 0x0046A8D0, 1);
    }
}