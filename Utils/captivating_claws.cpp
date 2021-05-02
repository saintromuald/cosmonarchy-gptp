#include "captivating_claws.h"
#include "globals.h"
#include <algorithm>

u32 captivatingClawsGetStacks(CUnit* target)
{
    return std::min(scbw::get_generic_unit_vector(target, ValueId::CaptivatingClawsUnitVector).size(), (u32)4);
}

u32 captivatingClawsGetTimer(CUnit* target)
{
    return scbw::get_generic_timer(target, ValueId::CaptivatingClawsTimer);
}

void captivatingClawsApply(CUnit* source, CUnit* target)
{
    scbw::set_generic_timer(target, ValueId::CaptivatingClawsTimer, 1 * 24 + 1);
    auto& vec = scbw::get_generic_unit_vector(target, ValueId::CaptivatingClawsUnitVector);
    auto find = std::find(vec.begin(), vec.end(), source);
    if (find == vec.end())
    {
        vec.push_back(source);
    }
}