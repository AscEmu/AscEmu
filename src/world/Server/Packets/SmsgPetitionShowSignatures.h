/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/ObjectMgr.hpp"
#include <cstdint>
#include <vector>
#include "WoWGuid.hpp"

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
        std::vector<WoWGuid> signatures;

        SmsgPetitionShowSignatures() : SmsgPetitionShowSignatures(0, 0, 0, 0, 0, { 0 })
        {
        }

        SmsgPetitionShowSignatures(uint64_t itemGuid, const WoWGuid& leaderGuid, uint32_t chartId, uint8_t signatureCount, uint32_t petitionSlots, std::vector<WoWGuid> signatures) :
            ManagedPacket(SMSG_PETITION_SHOW_SIGNATURES, 0),
            itemGuid(itemGuid),
            leaderGuid(leaderGuid.getRawGuid()),
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
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // item, owner with account, petition, the signatures with their choice
                uint32_t ownerAccount = 0;
                if (const auto characterInfo = sObjectMgr.getCachedCharacterInfo(WoWGuid::getLowGuidFromRaw(leaderGuid)))
                    ownerAccount = characterInfo->acct;

                uint32_t count = 0;
                for (auto const& signature : signatures)
                {
                    if (signature.getRawGuid() != 0)
                        ++count;
                }

                packet << WoWGuid(itemGuid).toGuid128(m_protocol.realmId, 0);
                packet << WoWGuid(leaderGuid).toGuid128(m_protocol.realmId, 0);
                packet << uint32_t(ownerAccount);
                packet << int32_t(chartId);
                packet << uint32_t(count);
                for (auto const& signature : signatures)
                {
                    if (signature.getRawGuid() == 0)
                        continue;

                    packet << signature.toGuid128(m_protocol.realmId, 0);
                    packet << int32_t(0);                               // choice
                }
                return true;
            }

            packet << itemGuid;
            packet << leaderGuid;
            packet << chartId;
            packet << signatureCount;
            for (auto const signature : signatures)
            {
                if (signature.getRawGuid() == 0)
                    continue;

                packet << signature.getLowGuid();
                packet << uint32_t(1);
            }
            packet << uint8_t(0);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
