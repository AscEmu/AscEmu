/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AchievementDataWriter.h"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    // the criteria progress of the account wide achievements, 9.x and later
    class SmsgAllAccountCriteria : public ManagedPacket
    {
    public:
        std::vector<CriteriaProgressEntry> progress;
        uint32_t accountId = 0;

        SmsgAllAccountCriteria(std::vector<CriteriaProgressEntry> progress, uint32_t accountId) :
            ManagedPacket(SMSG_ALL_ACCOUNT_CRITERIA, 0),
            progress(std::move(progress)),
            accountId(accountId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Shadowlands)
                return false;

            WoWGuid128 const account = WoWGuid128::global(HighGuid128::BNetAccount, accountId);
            packet << uint32_t(progress.size());
            for (auto const& entry : progress)
                AchievementWire::writeProgress(packet, m_protocol, account, entry);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
