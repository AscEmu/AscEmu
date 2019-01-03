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
    class CmsgPetRename : public ManagedPacket
    {
    public:
        WoWGuid guid;
        std::string name;

        CmsgPetRename() : CmsgPetRename(0, "")
        {
        }

        CmsgPetRename(uint64_t guid, std::string name) :
            ManagedPacket(CMSG_PET_RENAME, 8),
            guid(guid),
            name(name)
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
            packet >> unpacked_guid >> name;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}}
