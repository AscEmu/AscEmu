/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>
#include <utility>
#include <utility>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgLootMasterList : public ManagedPacket
    {
    public:
        std::vector<uint64_t> onlineGroupMembers;

        SmsgLootMasterList() : SmsgLootMasterList({ 0 })
        {
        }

        SmsgLootMasterList(std::vector<uint64_t> onlineGroupMembers) :
            ManagedPacket(SMSG_LOOT_MASTER_LIST, 0),
            onlineGroupMembers(onlineGroupMembers)
        {
        }

    protected:
        size_t expectedSize() const override { return 330; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << static_cast<uint8_t>(onlineGroupMembers.size());
            for (auto memberGuid : onlineGroupMembers)
                packet << memberGuid;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
