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
    class SmsgPetitionShowSignatures : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        uint64_t leaderGuid;
        uint32_t chartId;
        uint8_t signatureCount;
        uint32_t petitionSlots;
        uint32_t* signatures;

        SmsgPetitionShowSignatures() : SmsgPetitionShowSignatures(0, 0, 0, 0, 0, { 0 })
        {
        }

        SmsgPetitionShowSignatures(uint64_t itemGuid, uint64_t leaderGuid, uint32_t chartId, uint8_t signatureCount, uint32_t petitionSlots, uint32_t* signatures) :
            ManagedPacket(SMSG_PETITION_SHOW_SIGNATURES, 0),
            itemGuid(itemGuid),
            leaderGuid(leaderGuid),
            chartId(chartId),
            signatureCount(signatureCount),
            petitionSlots(petitionSlots),
            signatures(signatures)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 8 + 4 + 4 + petitionSlots * 8 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << itemGuid;
            packet << leaderGuid;
            packet << chartId;
            packet << signatureCount;
            for (uint32_t j = 0; j < petitionSlots; ++j)
            {
                if (signatures[j] == 0)
                    continue;

                packet << signatures[j];
                packet << uint32_t(1);
            }
            packet << uint8_t(0);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
