/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace ScarletMonastery
{
    enum CreatureEntry
    {
        CN_COMMANDER_MOGRAINE = 3976,
        CN_VISHAS = 3983,
        CN_THALNOS = 4543,
        CN_DOAN = 6487,
        CN_HEROD = 3975,
        CN_WHITEMANE = 3977,
    };

    enum CreatureSpells
    {
        SP_THALNOS_SHADOW_BOLT = 9613,
        SP_THALNOS_FLAME_SPIKE = 8814,
        SP_DOAN_SHIELD = 9438,
        SP_DOAN_NOVA = 9435,
        SP_HEROD_ENRAGESPELL = 8269,
        SP_WHITEMANE_SLEEP = 9256,
        SP_WHITEMANE_RESURRECTION = 25435
    };

    enum CreatureSay
    {
        // Scarlet Commander Mograine
        SAY_MORGRAINE_01 = 2101,     // Infidels!They must be purified!
        SAY_MORGRAINE_02 = 2102,     // Unworthy!
        SAY_MORGRAINE_03 = 2103,     // At your side, milady!

        // High Inquisitor Whitemane
        SAY_SOUND_RESTALK2 = 5840,
        SAY_WHITEMANE_01 = 2104,     // Mograine has fallen!You shall pay for this treachery!Arise, my champion!Arise!
        SAY_WHITEMANE_02 = 2105,     // The Light has spoken!
        SAY_WHITEMANE_03 = 2106,     // Arise, my champion!
    };

    enum GameObjectEntry
    {
        GO_INQUISITORS_DOOR = 104600,
        GO_SCARLET_SECRET_DOOR = 97700,
        GO_SCARLET_TORCH = 97701,
        GO_ARMORY_DOOR = 101851,
        GO_ARMORY_LEVER = 101852,
        GO_CATHEDRAL_DOOR = 101850,
        GO_CATHEDRAL_LEVER = 101853
    };
}
