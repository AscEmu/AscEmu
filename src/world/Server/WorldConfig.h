/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum WorldConfigRates
{
    RATE_HEALTH = 0,    // hp
    RATE_POWER1,        // mp
    RATE_POWER2,        // rage
    RATE_POWER3,        // focus
    RATE_POWER4,        // energy
    //RATE_POWER5,
    //RATE_POWER6,
    RATE_POWER7,        // runic power
    RATE_DROP0,         // quality level
    RATE_DROP1,
    RATE_DROP2,
    RATE_DROP3,
    RATE_DROP4,
    RATE_DROP5,
    RATE_DROP6,
    RATE_MONEY,
    RATE_XP,
    RATE_RESTXP,
    RATE_QUESTXP,
    RATE_EXPLOREXP,
    RATE_HONOR,
    RATE_QUESTREPUTATION,
    RATE_KILLREPUTATION,
    RATE_SKILLCHANCE,
    RATE_SKILLRATE,
    RATE_ARENAPOINTMULTIPLIER2X,
    RATE_ARENAPOINTMULTIPLIER3X,
    RATE_ARENAPOINTMULTIPLIER5X,
    RATE_VEHICLES_POWER_REGEN,
    MAX_RATES
};

enum WorldConfigIntRates
{
    INTRATE_SAVE = 0,
    INTRATE_COMPRESSION,
    INTRATE_PVPTIMER,
    MAX_INTRATES
};

enum WorldConfigRealmTypes
{
    REALM_PVE = 0,
    REALM_PVP = 1
};

class WorldConfig
{
    public:

        WorldConfig();
        ~WorldConfig();
};
