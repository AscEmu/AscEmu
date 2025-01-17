/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include <cstdint>

enum TargetFilter : uint32_t;

enum TargetGenerator
{
    // Self
    TargetGen_Self,                         // Target self (Note: doesn't always mean self, also means the spell can choose various target)

    // Current
    TargetGen_Current,                      // Current highest aggro (attacking target)
    TargetGen_Destination,                  // Target is a destination coordinates (X, Y, Z)

    // Second most hated
    TargetGen_SecondMostHated,              // Second highest aggro

    // Predefined target
    TargetGen_Predefined,                   // Pre-defined target unit

    // Random Unit
    TargetGen_RandomUnit,                   // Random target unit (players, totems, pets, etc.)
    TargetGen_RandomUnitDestination,        // Random destination coordinates (X, Y, Z)
    TargetGen_RandomUnitApplyAura,          // Random target unit to self cast aura

    // Random Player
    TargetGen_RandomPlayer,                 // Random target player
    TargetGen_RandomPlayerDestination,      // Random player destination coordinates (X, Y, Z)
    TargetGen_RandomPlayerApplyAura         // Random target player to self cast aura
};

class SERVER_DECL TargetType
{
public:
    TargetType(uint32_t pTargetGen = TargetGen_Self, TargetFilter pTargetFilter = static_cast<TargetFilter>(0) /*TargetFilter_None*/, uint32_t pMinTargetNumber = 0, uint32_t pMaxTargetNumber = 0);
    ~TargetType() = default;

    uint32_t mTargetGenerator;              // Defines what kind of target should we try to find
    TargetFilter mTargetFilter;             // Defines filter of target
    uint32_t mTargetNumber[2];              // 0: Defines min. number of creature on hatelist (0 - any, 1 - the most hated etc.)
    // 1: Defines max. number of creature on hatelist (0 - any, HateList.size + 1 - the least hated etc.)
};
