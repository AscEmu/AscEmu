/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace MySQLStructure
{
    struct Broadcast
    {
        uint32_t id;
        uint32_t interval;
        uint32_t random_interval;
        uint32_t next_update;
        std::string text;
    };

    struct GossipMenuOption
    {
        uint32_t id;
        std::string text;
    };

    struct ItemPage
    {
        uint32_t id;
        std::string text;
        uint32_t nextPage;
    };

}
