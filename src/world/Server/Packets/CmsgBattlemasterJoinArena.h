/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgBattlemasterJoinArena : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t category;
        uint8_t asGroup;
        uint8_t ratedMatch;

        CmsgBattlemasterJoinArena() : CmsgBattlemasterJoinArena(0, 0, 0, 0)
        {
        }

        CmsgBattlemasterJoinArena(uint64_t guid, uint8_t category, uint8_t asGroup, uint8_t ratedMatch) :
            ManagedPacket(CMSG_BATTLEMASTER_JOIN_ARENA, 0),
            guid(guid),
            category(category),
            asGroup(asGroup),
            ratedMatch(ratedMatch)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid >> category >> asGroup >> ratedMatch;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}}
