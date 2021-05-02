#include "neutral_related.h"
#include <SCBW/api.h>
#include <hook_tools.h>

namespace
{
    void __declspec(naked) unitNotNeutral_Wrapper()
    { // 004CBE20
        static CHKUnit* unit;
        static bool result;

        __asm {
            MOV unit, EAX
            PUSHAD
        }
        result = hooks::unitNotNeutral(unit);
        __asm {
            POPAD
            MOVZX EAX, result
            RETN
        }
    }
}

namespace hooks
{
    void injectNeutralRelatedHooks()
    {
        jmpPatch(unitNotNeutral_Wrapper, 0x004CBE20, 1);
    }
}