/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>
#include <utility>

#include "ManagedPacket.h"
#include "Management/Guild/GuildDefinitions.h"

namespace AscEmu { namespace Packets
{
    class SmsgPetitionSignResult : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        uint64_t playerGuid;
        uint32_t result;

        SmsgPetitionSignResult() : SmsgPetitionSignResult(0, 0, 0)
        {
        }

        SmsgPetitionSignResult(uint64_t itemGuid, uint64_t playerGuid, uint32_t result) :
            ManagedPacket(SMSG_PETITION_SIGN_RESULTS, 20),
            itemGuid(itemGuid),
            playerGuid(playerGuid),
            result(result)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << itemGuid << playerGuid << result;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
