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
    class CmsgUpdateAccountData : public ManagedPacket
    {
    public:
        uint32_t uiId;
        uint32_t uiTimestamp;
        uint32_t uiDecompressedSize;
        uint32_t compressedSize = 0;
        size_t dataOffset = 8;

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
            else if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // 6.x to 9.x: player, time, size and the type as bits; 10.x and later: time, size, player, type
                WoWGuid128 player;
                int64_t time = 0;
                if (m_protocol.expansion <= WoW::Expansion::_Shadowlands)
                {
                    packet >> player;
                    if (m_protocol.isShadowlands())
                        packet >> time;
                    else
                        packet >> uiTimestamp;
                    packet >> uiDecompressedSize;
                    uiId = packet.readBits(m_protocol.isShadowlands() ? 4 : 3);
                    packet.resetBitPos();
                }
                else
                {
                    packet >> time;
                    packet >> uiDecompressedSize;
                    packet >> player;
                    packet >> uiId;
                }
                if (time != 0)
                    uiTimestamp = static_cast<uint32_t>(time);

                packet >> compressedSize;
                dataOffset = packet.rpos();
                if (uiDecompressedSize >= 0xFFFF || compressedSize > packet.remaining())
                {
                    packet.rfinish();
                    return false;
                }

                packet.rfinish();
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
