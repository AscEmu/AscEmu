/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgPetitionShowSignatures : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        uint64_t leaderGuid;
        uint32_t chartId;
        uint8_t signatureCount;
        uint32_t petitionSlots;
        std::vector<uint32_t> signatures;

        SmsgPetitionShowSignatures() : SmsgPetitionShowSignatures(0, 0, 0, 0, 0, { 0 })
        {
        }

        SmsgPetitionShowSignatures(uint64_t itemGuid, uint64_t leaderGuid, uint32_t chartId, uint8_t signatureCount, uint32_t petitionSlots, std::vector<uint32_t> signatures) :
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
            for (auto const signature : signatures)
            {
                if (signature == 0)
                    continue;

                packet << signature;
                packet << uint32_t(1);
            }
            packet << uint8_t(0);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
