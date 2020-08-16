//
#include "hooks/misc/hydra_specific.h"

namespace hooks {
    const u32 jmp_00494FC3 = 0x00494FC3;
    const u32 jmp_00494FAC = 0x00494FAC;
    void __declspec(naked) getFlingyHaltDistance_94FA2_JmpWrapper() {
        //patched 2 instructions before 94fa3 to avoid aise extender conflict
        static u32 current_speed;
        static u32 flingy_id;
        static CFlingy* flingy;
        __asm {
            MOVZX EDX,[ECX+0x24]
            PUSH ESI
            MOV current_speed,EAX
            MOV flingy_id,EDX
            MOV flingy,ECX
            PUSHAD
        }
        if (hooks::realTopSpeed(flingy,flingy_id) != current_speed) {
            __asm {
                POPAD
                JMP jmp_00494FC3
            }
        }
        else {
            __asm {
                POPAD
                JMP jmp_00494FAC
            }
        }
    }
/*
0049637E  |.  8B0C85 F89E6C>MOV ECX,DWORD PTR DS:[EAX*4+6C9EF8]
00496385  |.  894E 34       MOV DWORD PTR DS:[ESI+34],ECX
00496388  |.  66:8B0C45 789>MOV CX,WORD PTR DS:[EAX*2+6C9C78]
*/
    void injectHydraSpecific() {
        jmpPatch(getFlingyHaltDistance_94FA2_JmpWrapper, 0x00494F9E);
        
    }
}

//496360
/*
// 3
CSPRITE *__userpurge InitializeFlingy@<eax>(int a1@<eax>, __int16 a2@<dx>, int a3@<ecx>, CBULLET *a4, char owner, char a6)
{
  int v6; // edi
  POINTS v7; // edx
  CSPRITE *result; // eax
  int v9; // ST00_4
  unsigned __int8 v10; // di
  CIMAGE *i; // esi

  a4->type = a1;
  a1 = (unsigned __int16)a1;
  a4->movementFlags = 0;
  a4->field_3C = 0;
  a4->flingyTopSpeed = Flingy_TopSpeed[(unsigned __int16)a1];
  a4->flingyAcceleration = Flingy_Acceleration[(unsigned __int16)a1];
  a4->flingyTurnRadius = Flingy_TurnRadius[(unsigned __int16)a1];
  a4->flingyMoveControl = Flingy_MoveControl[(unsigned __int16)a1];
  a3 = (signed __int16)a3;
  a4->position.x = a3;
  a4->xHalt = (signed __int16)a3 << 8;
  v6 = a2;
  a4->position.y = a2;
  a4->yHalt = a2 << 8;
  if ( (signed __int16)a3 != a4->field_10.x || a2 != a4->field_10.y )
  {
    a4->field_10.x = a3;
    a4->field_10.y = v6;
    v7 = a4->field_10;
    a4->field_14 = 0;
    a4->movementFlags = 1;
    a4->targetPosition1 = v7;
  }
  if ( a3 != a4->targetPosition2.x || v6 != a4->targetPosition2.y )
  {
    a4->targetPosition2.x = a3;
    a4->targetPosition2.y = v6;
  }
  a4->direction1 = a6;
  a4->direction2 = a6;
  v9 = Flingy_SpriteID[a1];
  a4->field_8 = 1;
  result = createSprite(v9, a3, owner);
  a4->sprite = result;
  if ( result )
  {
    v10 = a4->direction1;
    for ( i = result->overlay; i; i = i->next )
      setImageDirection(i, v10);
    result = (CSPRITE *)1;
  }
  return result;
}
*/