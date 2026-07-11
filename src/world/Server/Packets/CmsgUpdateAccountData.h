/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgUpdateAccountData : public ManagedPacket
    {
    public:
        uint32_t uiId;
        uint32_t uiTimestamp;
        uint32_t uiDecompressedSize;

        CmsgUpdateAccountData() : CmsgUpdateAccountData(0, 0, 0)
        {
        }

        CmsgUpdateAccountData(uint32_t uiId, uint32_t uiTimestamp, uint32_t uiDecompressedSize) :
            ManagedPacket(CMSG_UPDATE_ACCOUNT_DATA, 4 + 4),
            uiId(uiId),
            uiTimestamp(uiTimestamp),
            uiDecompressedSize(uiDecompressedSize)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> uiId;
                if (m_protocol.expansion == WoW::Expansion::_Cata)
                {
                    packet >> uiTimestamp;
                }
                packet >> uiDecompressedSize;

                if (uiDecompressedSize >= 0xFFFF)
                {
                    packet.rfinish();
                    return false;
                }
            }
            else // Mop
            {
                packet >> uiDecompressedSize;
                packet >> uiTimestamp;
                packet >> uiDecompressedSize;
                uiId = packet.readBits(3);

                if (uiDecompressedSize >= 0xFFFF)
                {
                    packet.rfinish();
                    return false;
                }

                packet.rfinish();
            }

            return true;
        }
    };
}
