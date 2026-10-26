/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AchievementDataWriter.h"

#include <cstdint>

namespace AscEmu::Packets
{
    // one criteria of the account wide achievements changed, 9.x and later
    class SmsgAccountCriteriaUpdate : public ManagedPacket
    {
    public:
        CriteriaProgressEntry progress;
        uint32_t accountId = 0;

        SmsgAccountCriteriaUpdate(CriteriaProgressEntry progress, uint32_t accountId) :
            ManagedPacket(SMSG_ACCOUNT_CRITERIA_UPDATE, 0),
            progress(progress),
            accountId(accountId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Shadowlands)
                return false;

            AchievementWire::writeProgress(packet, m_protocol, WoWGuid128::global(HighGuid128::BNetAccount, accountId), progress);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
