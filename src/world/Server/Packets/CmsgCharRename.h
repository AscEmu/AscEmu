/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgCharRename : public ManagedPacket
    {
    public:
        WoWGuid guid;
        std::string name;

        size_t size = 0;

        CmsgCharRename() : CmsgCharRename(0, "")
        {
        }

        CmsgCharRename(uint64_t guid, std::string name) :
            ManagedPacket(CMSG_CHAR_RENAME, 0),
            guid(guid),
            name(name)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            size = packet.size();

            uint64_t unpackedGuid;
            packet >> unpackedGuid >> name;

            guid.init(unpackedGuid);
            return true;
        }
    };
}
