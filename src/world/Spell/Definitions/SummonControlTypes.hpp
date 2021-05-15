/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum SummonControlTypes
{
    SUMMON_CONTROL_TYPE_WILD      = 0,
    SUMMON_CONTROL_TYPE_GUARDIAN  = 1,
    SUMMON_CONTROL_TYPE_PET       = 2,
    SUMMON_CONTROL_TYPE_POSSESSED = 3,
    SUMMON_CONTROL_TYPE_VEHICLE   = 4,
    SUMMON_CATEGORY_UNK = 5  // as of patch 3.3.5a only Bone Spike in Icecrown Citadel
                                 // uses this category
};
