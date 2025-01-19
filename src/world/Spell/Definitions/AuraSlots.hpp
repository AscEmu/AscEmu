/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"

#include <cstdint>

namespace AuraSlots
{
    enum AuraSlotEnum : uint16_t
    {
        PASSIVE_SLOT_START      = 0,
#if VERSION_STRING == Classic
        // 100 passive aura slots for each unit
        PASSIVE_SLOT_END        = PASSIVE_SLOT_START + 100,
        // 32 helpful aura slots for each unit
        POSITIVE_SLOT_START     = PASSIVE_SLOT_END,
        POSITIVE_SLOT_END       = POSITIVE_SLOT_START + 32,
        // 16 harmful aura slots for each unit
        NEGATIVE_SLOT_START     = POSITIVE_SLOT_END,
        NEGATIVE_SLOT_END       = NEGATIVE_SLOT_START + 16,
#elif VERSION_STRING == TBC
        // 170 passive aura slots for each unit
        PASSIVE_SLOT_END        = PASSIVE_SLOT_START + 170,
        // 40 helpful aura slots for each unit
        POSITIVE_SLOT_START     = PASSIVE_SLOT_END,
        POSITIVE_SLOT_END       = POSITIVE_SLOT_START + 40,
        // 40 harmful aura slots for each unit
        NEGATIVE_SLOT_START     = POSITIVE_SLOT_END,
        NEGATIVE_SLOT_END       = NEGATIVE_SLOT_START + 40,
#else
        // 180 passive aura slots for each unit
        PASSIVE_SLOT_END        = PASSIVE_SLOT_START + 180,
        // 80 helpful aura slots for each unit
        POSITIVE_SLOT_START     = PASSIVE_SLOT_END,
        POSITIVE_SLOT_END       = POSITIVE_SLOT_START + 80,
        // 100 harmful aura slots for each unit
        NEGATIVE_SLOT_START     = POSITIVE_SLOT_END,
        NEGATIVE_SLOT_END       = NEGATIVE_SLOT_START + 100,
#endif

        // Helpers

        REMOVABLE_SLOT_START = POSITIVE_SLOT_START,
        REMOVABLE_SLOT_END = NEGATIVE_SLOT_END,

        TOTAL_SLOT_START = PASSIVE_SLOT_START,
        TOTAL_SLOT_END = NEGATIVE_SLOT_END,
    };

    // These refer to visual slots (= auras visible on client)
    enum VisualAuraSlots : uint8_t
    {
        POSITIVE_VISUAL_SLOT_START  = 0,
#if VERSION_STRING == Classic
        POSITIVE_VISUAL_SLOT_END    = POSITIVE_VISUAL_SLOT_START + 32,
#else
        POSITIVE_VISUAL_SLOT_END    = POSITIVE_VISUAL_SLOT_START + 40,
#endif
        NEGATIVE_VISUAL_SLOT_START  = POSITIVE_VISUAL_SLOT_END,
        NEGATIVE_VISUAL_SLOT_END    = NEGATIVE_VISUAL_SLOT_START + 16,
    };
};
