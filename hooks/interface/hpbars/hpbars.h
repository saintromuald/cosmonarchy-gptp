#pragma once
#include "SCBW/scbwdata.h"

namespace hooks
{
    void ShowHealthBar(CUnit* unit);
    void compileHealthBar(CImage* a1, CSprite* a2);

    void injectHpBarsHooks();

    extern CUnit* _unitTemp;
}