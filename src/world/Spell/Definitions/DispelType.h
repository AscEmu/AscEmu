/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum DispelType : uint8_t
{
    DISPEL_NONE            = 0,
    DISPEL_MAGIC           = 1,
    DISPEL_CURSE           = 2,
    DISPEL_DISEASE         = 3,
    DISPEL_POISON          = 4,
    DISPEL_STEALTH         = 5,
    DISPEL_INVISIBILTY     = 6,
    DISPEL_ALL             = 7,
    DISPEL_SPECIAL_NPCONLY = 8,
    DISPEL_FRENZY          = 9, // enrage/frenzy
    DISPEL_UNK10           = 10, // only spell 45362 has this on 3.3.5a
    DISPEL_UNK11           = 11, // only spell 66006 (Divine Storm) has this on 3.3.5a
    TOTAL_DISPEL_TYPES
};
