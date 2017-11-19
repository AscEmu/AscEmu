/*
Copyryight (c) 2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>
#include "Units/Unit.h"

// TODO: Move this stuff into a class so it's not global scope
#define SPELL_GROUP_FOREACH(X) \
    uint32_t intbit = 0; \
    uint32_t groupnum = 0; \
    for (uint32_t bit = 0; bit < SPELL_GROUPS; ++bit, ++intbit) { \
        if (intbit == 32) { \
            ++groupnum; \
            intbit = 0; \
        } \
        if ((1 << intbit) & group[groupnum]) \
            X; \
    }

namespace ascemu { namespace World { namespace Spell { namespace Helpers
{
    inline uint32_t decimalToMask(uint32_t dec) { return (static_cast<uint32_t>(1) << (dec - 1)); }

    inline void spellModFlatFloatValue(int* m, float* v, uint32_t* group)
    {
        if (m == nullptr)
            return;

        SPELL_GROUP_FOREACH(*v += m[bit]);
    }

    inline void spellModFlatIntValue(int* m, int* v, uint32_t* group)
    {
        if (m == nullptr)
            return;

        SPELL_GROUP_FOREACH(*v += m[bit]);
    }

    inline void spellModPercentageFloatValue(int* m, float* v, uint32_t* group)
    {
        if (m == nullptr)
            return;

        SPELL_GROUP_FOREACH(*v += ((*v) * m[bit]) / 100.0f);
    }

    inline void spellModPercentageIntValue(int* m, int* v, uint32_t* group)
    {
        if (m == nullptr)
            return;

        SPELL_GROUP_FOREACH(*v += ((*v) * m[bit]) / 100);
    }
}}}}

#undef SPELL_GROUP_FOREACH
