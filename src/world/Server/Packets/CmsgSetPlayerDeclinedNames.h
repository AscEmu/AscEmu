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
    class CmsgSetPlayerDeclinedNames : public ManagedPacket
    {
    public:
        uint64_t guid;
        std::string name;

        CmsgSetPlayerDeclinedNames() : CmsgSetPlayerDeclinedNames(0, "")
        {
        }

        CmsgSetPlayerDeclinedNames(uint64_t guid, std::string name) :
            ManagedPacket(CMSG_SET_PLAYER_DECLINED_NAMES, 8),
            guid(guid),
            name(name)
        {
        }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid >> name;
            return true;
        }
    };
}}
