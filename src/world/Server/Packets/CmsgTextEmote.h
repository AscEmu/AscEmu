/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgTextEmote : public ManagedPacket
    {
    public:
        uint32_t text_emote;
        uint32_t numEmote;
        WoWGuid guid;

        CmsgTextEmote() : CmsgTextEmote(0, 0, 0)
        {
        }

        CmsgTextEmote(uint32_t text_emote, uint32_t unk, uint64_t guid) :
            ManagedPacket(CMSG_TEXT_EMOTE, 0),
            text_emote(text_emote),
            numEmote(unk),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t rawGuid;
                packet >> text_emote >> numEmote >> rawGuid;
                guid.init(rawGuid);
            }
            else // Mop
            {
                packet >> text_emote;
                packet >> numEmote;

                guid[6] = packet.readBit();
                guid[7] = packet.readBit();
                guid[3] = packet.readBit();
                guid[2] = packet.readBit();
                guid[0] = packet.readBit();
                guid[5] = packet.readBit();
                guid[1] = packet.readBit();
                guid[4] = packet.readBit();

                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[6]);
            }
            return true;
        }
    };
}
