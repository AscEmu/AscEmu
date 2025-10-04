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
        DeclinedNamesArray declinedNames;

        CmsgSetPlayerDeclinedNames() : CmsgSetPlayerDeclinedNames(0, "", DeclinedNamesArray())
        {
        }

        CmsgSetPlayerDeclinedNames(uint64_t guid, std::string name, DeclinedNamesArray&& declinedNames) :
            ManagedPacket(CMSG_SET_PLAYER_DECLINED_NAMES, 8 + 1 + 1 + 1 + 1 + 1 + 1),
            guid(guid),
            name(name),
            declinedNames(std::move(declinedNames))
        {
        }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid >> name;
            for (uint8_t i = 0; i < MAX_DECLINED_NAMES; ++i)
                packet >> declinedNames[i];
            return true;
        }
    };
}
