#include "trap_hooks.h"
#include "hook_tools.h"
#include "SCBW/api.h"

namespace
{
    void __declspec(naked) unitIsInstallationAsset_Wrapper()
    {
        static u32 id;
        static u32 result;
        __asm {
            MOV id, EAX
            PUSHAD
        }
        result = hooks::unitIsInstallationAsset(id);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) unitIsFloorTrap_Wrapper()
    {
        static CUnit* unit;
        static u32 result;
        __asm {
            MOV unit, EAX
            PUSHAD
        }
        result = hooks::unitIsFloorTrap(unit);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) disableTrap_Wrapper()
    {
        static CUnit* unit;
        static u32 result;
        __asm {
            MOV unit, EDI
            PUSHAD
        }
        result = hooks::disableTrap(unit);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) updateUnitStrengthAndApplyDefaultOrders_Wrapper()
    {
        static CUnit* a1;
        static u32 result;
        __asm {
            MOV a1, EAX
            PUSHAD
        }
        result = hooks::updateUnitStrengthAndApplyDefaultOrdersHook(a1);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }
}

namespace hooks
{
    void injectTrapHooks()
    {
        jmpPatch(unitIsInstallationAsset_Wrapper, 0x00401430, 3);
        jmpPatch(unitIsFloorTrap_Wrapper, 0x0047B070, 3);

        //jmpPatch(disableTrap_Wrapper, 0x0047B4B0, 1); 


        /*
         Thg2SpecialDIsableUnit
         0047B556  |.  66:3D CB00    CMP AX,0CB
0047B55A  |.  74 1E         JE SHORT 0047B57A
0047B55C  |.  66:3D D100    CMP AX,0D1
0047B560  |.  74 18         JE SHORT 0047B57A
0047B562  |.  66:3D D200    CMP AX,0D2
0047B566  |.  74 12         JE SHORT 0047B57A
0047B568  |.  66:3D D300    CMP AX,0D3
0047B56C  |.  74 0C         JE SHORT 0047B57A
0047B56E  |.  66:3D D400    CMP AX,0D4
0047B572  |.  74 06         JE SHORT 0047B57A
0047B574  |.  66:3D D500    CMP AX,0D5
        */

        /*
        0047BDC7   .  66:3D CB00    CMP AX,0CB
0047BDCB   .  74 1E         JE SHORT 0047BDEB
0047BDCD   .  66:3D D100    CMP AX,0D1
0047BDD1   .  74 18         JE SHORT 0047BDEB
0047BDD3   .  66:3D CD00    CMP AX,0CD
0047BDD7   .  74 12         JE SHORT 0047BDEB
0047BDD9   .  66:3D CE00    CMP AX,0CE
0047BDDD   .  74 0C         JE SHORT 0047BDEB
0047BDDF   .  66:3D CF00    CMP AX,0CF
0047BDE3   .  74 06         JE SHORT 0047BDEB
0047BDE5   .  66:3D D000    CMP AX,0D0
0047BDE9   .  75 07         JNZ SHORT 0047BDF2
        -enableDoodad order
        */


        /*
        0047C05D   .  66:3D CB00    CMP AX,0CB
0047C061   .  8996 DC000000 MOV DWORD PTR DS:[ESI+DC],EDX
0047C067   .  74 06         JE SHORT 0047C06F
0047C069   .  66:3D D100    CMP AX,0D1
0047C06D   .  75 07         JNZ SHORT 0047C076
- hiddenGun order
        */


        //		jmpPatch(updateUnitStrengthAndApplyDefaultOrders_Wrapper, 0x0049FA40, 1);

                //209 - 213




    }
}