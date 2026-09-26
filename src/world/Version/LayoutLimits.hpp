/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Largest value count of each struct over all client versions, sizes the embedded object value storage.

#pragma once

#include <cstdint>

// Largest uint32 value count of each struct over all client versions, sizes the object value storage
namespace Version
{
    inline constexpr uint16_t kMaxObjectValues = 12;
    inline constexpr uint16_t kMaxUnitValues = 234;
    inline constexpr uint16_t kMaxPlayerValues = 4625;
    inline constexpr uint16_t kMaxItemValues = 85;
    inline constexpr uint16_t kMaxContainerValues = 230;
    inline constexpr uint16_t kMaxGameObjectValues = 33;
    inline constexpr uint16_t kMaxDynamicObjectValues = 21;
    inline constexpr uint16_t kMaxCorpseValues = 46;
    inline constexpr uint16_t kMaxAreaTriggerValues = 43;
}
