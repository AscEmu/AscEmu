/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class MsgPetitionRename : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        std::string name;

        MsgPetitionRename() : MsgPetitionRename(0, "")
        {
        }

        MsgPetitionRename(uint64_t itemGuid, std::string name) :
            ManagedPacket(MSG_PETITION_RENAME, 8 + 1),
            itemGuid(itemGuid),
            name(name)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + name.size() + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << itemGuid << name;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> itemGuid >> name;
            return true;
        }
    };
}}
