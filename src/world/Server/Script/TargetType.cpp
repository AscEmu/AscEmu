/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "TargetType.hpp"

TargetType::TargetType(uint32_t pTargetGen, TargetFilter pTargetFilter, uint32_t pMinTargetNumber, uint32_t pMaxTargetNumber)
{
    mTargetGenerator = pTargetGen;
    mTargetFilter = pTargetFilter;
    mTargetNumber[0] = pMinTargetNumber;    // Unused array for now
    mTargetNumber[1] = pMaxTargetNumber;
}
