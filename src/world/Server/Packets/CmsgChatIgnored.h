/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgChatIgnored : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t unk1;

        CmsgChatIgnored() : CmsgChatIgnored(0, 0)
        {
        }

        CmsgChatIgnored(uint64_t guid, uint8_t unk1) :
            ManagedPacket(CMSG_CHAT_IGNORED, 0),
            guid(guid),
            unk1(unk1)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                uint64_t unpacked_guid;
                packet >> unpacked_guid >> unk1;
                guid.init(unpacked_guid);
            }
            else
            {
                WoWGuid playerGuid;

                if (m_protocol.expansion == WoW::Expansion::_Cata)
                {
                    packet >> unk1;

                    playerGuid[5] = packet.readBit();
                    playerGuid[2] = packet.readBit();
                    playerGuid[6] = packet.readBit();
                    playerGuid[4] = packet.readBit();
                    playerGuid[7] = packet.readBit();
                    playerGuid[0] = packet.readBit();
                    playerGuid[1] = packet.readBit();
                    playerGuid[3] = packet.readBit();

                    packet.readByteSeq(playerGuid[0]);
                    packet.readByteSeq(playerGuid[6]);
                    packet.readByteSeq(playerGuid[5]);
                    packet.readByteSeq(playerGuid[1]);
                    packet.readByteSeq(playerGuid[4]);
                    packet.readByteSeq(playerGuid[3]);
                    packet.readByteSeq(playerGuid[7]);
                    packet.readByteSeq(playerGuid[2]);
                }
                else if (m_protocol.expansion == WoW::Expansion::_Mop)
                {
                    playerGuid[5] = packet.readBit();

                    packet >> unk1;

                    playerGuid[0] = packet.readBit();
                    playerGuid[1] = packet.readBit();
                    playerGuid[3] = packet.readBit();
                    playerGuid[6] = packet.readBit();
                    playerGuid[7] = packet.readBit();
                    playerGuid[4] = packet.readBit();
                    playerGuid[2] = packet.readBit();

                    packet.readByteSeq(playerGuid[2]);
                    packet.readByteSeq(playerGuid[0]);
                    packet.readByteSeq(playerGuid[3]);
                    packet.readByteSeq(playerGuid[4]);
                    packet.readByteSeq(playerGuid[7]);
                    packet.readByteSeq(playerGuid[6]);
                    packet.readByteSeq(playerGuid[0]);
                    packet.readByteSeq(playerGuid[5]);
                }

                guid.init(playerGuid);
            }
            return true;
        }
    };
}
