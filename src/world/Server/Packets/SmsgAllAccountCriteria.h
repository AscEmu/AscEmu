/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "AchievementDataEntries.h"

#include <vector>

namespace AscEmu::Packets
{
    class SmsgAllAccountCriteria : public ManagedPacket
    {
    public:
        uint32_t accountId;
        std::vector<CriteriaProgressEntry> criteriaProgress;

        SmsgAllAccountCriteria(uint32_t accountId, std::vector<CriteriaProgressEntry> criteriaProgress) :
            ManagedPacket(SMSG_ALL_ACCOUNT_CRITERIA, 4), accountId(accountId), criteriaProgress(std::move(criteriaProgress))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + criteriaProgress.size() * (4 + 8 + 16 + 4 + 4 + 4 + 8 + 8 + 1);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet << uint32_t(criteriaProgress.size());

            const WoWGuid accountGuid = WoWGuid::createModernGlobal(ModernHighGuid::BNetAccount, 0, accountId);
            for (const auto& progress : criteriaProgress)
            {
                packet << uint32_t(progress.criteriaId);
                packet << uint64_t(progress.counter);
                const auto packedPlayer = accountGuid.packModern();
                packet.append(packedPlayer.data(), packedPlayer.size());
                packet << uint32_t(0); // Flags - no special flags for the persisted AscEmu criteria state
                packet << uint32_t(0); // StateFlags - UNKNOWN for Forever, zero for ordinary criteria
                packet.appendPackedTime(progress.date);
                packet << int64_t(0); // TimeFromStart
                packet << int64_t(0); // TimeFromCreate
                packet.writeBit(false); // DynamicID absent
                packet.flushBits();
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
