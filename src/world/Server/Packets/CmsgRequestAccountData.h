/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgRequestAccountData : public ManagedPacket
    {
    public:
        uint32_t accountDataId = 0;

        CmsgRequestAccountData() : CmsgRequestAccountData(0)
        {
        }

        CmsgRequestAccountData(uint32_t accountDataId) :
            ManagedPacket(CMSG_REQUEST_ACCOUNT_DATA, 4),
            accountDataId(accountDataId)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet >> accountDataId;
                return true;
            }
            else if (m_protocol.isMop())
            {
                accountDataId = static_cast<uint32_t>(packet.readBits(3));
                return true;
            }

            return false;
        }
    };
}
