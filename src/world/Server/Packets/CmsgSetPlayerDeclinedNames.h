/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgSetPlayerDeclinedNames : public ManagedPacket
    {
    public:
        uint64_t guid;
        std::string name;
        std::array<std::string, 5> declinedNames;

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
            for (size_t i = 0; i < declinedNames.size(); i++)
            {
                if (packet.rpos() >= packet.size())
                {
                    declinedNames[i].clear();
                    continue;
                }

                packet >> declinedNames[i];
            }

        return true;
        }
    };
}
