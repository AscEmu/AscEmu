/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum FactionFlags
{
    FACTION_FLAG_VISIBLE            = 0x01,
    FACTION_FLAG_AT_WAR             = 0x02,
    FACTION_FLAG_HIDDEN             = 0x04,
    FACTION_FLAG_FORCED_INVISIBLE   = 0x08, // if both ACTION_FLAG_VISIBLE and FACTION_FLAG_FORCED_INVISIBLE are set, client crashes!
    FACTION_FLAG_DISABLE_ATWAR      = 0x10, // disables AtWar button for client, but you can be in war with the faction
    FACTION_FLAG_INACTIVE           = 0x20,
    FACTION_FLAG_RIVAL              = 0x40  // only Scryers and Aldor have this flag
};
