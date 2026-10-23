/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WoWGuid.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgUpdateAccountDataComplete : public ManagedPacket
    {
    public:
        uint32_t uiId;
        uint32_t unknown;
        uint64_t playerGuid;

        SmsgUpdateAccountDataComplete() : SmsgUpdateAccountDataComplete(0, 0)
        {
        }

        SmsgUpdateAccountDataComplete(uint32_t uiId, uint32_t unknown, uint64_t playerGuid = 0) :
            ManagedPacket(SMSG_UPDATE_ACCOUNT_DATA_COMPLETE, 0),
            uiId(uiId),
            unknown(unknown),
            playerGuid(playerGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return 16 + 4 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // 11.x and later: player, type and result; 6.x to 10.x have no answer
                if (!m_protocol.isTheWarWithinOrLater())
                    return false;

                packet << WoWGuid(playerGuid).toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << int32_t(uiId) << int32_t(unknown);
                return true;
            }

            packet << uiId << unknown;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
