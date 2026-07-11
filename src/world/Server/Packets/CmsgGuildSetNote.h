/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGuildSetNote : public ManagedPacket
    {
    public:
        bool isPublic;
        std::string note;

        WoWGuid guid;

        CmsgGuildSetNote() : CmsgGuildSetNote(false, "")
        {
        }

        CmsgGuildSetNote(bool isPublic, std::string note) :
            ManagedPacket(CMSG_GUILD_SET_NOTE, 8),
            isPublic(isPublic),
            note(note)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                guid[1] = packet.readBit();
                guid[4] = packet.readBit();
                guid[5] = packet.readBit();
                guid[3] = packet.readBit();
                guid[0] = packet.readBit();
                guid[7] = packet.readBit();

                isPublic = packet.readBit();

                guid[6] = packet.readBit();

                const uint32_t noteLength = packet.readBits(8);

                guid[2] = packet.readBit();

                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[7]);

                note = packet.readString(noteLength);

                packet.readByteSeq(guid[2]);
                return true;
            }

            return false;
        }
    };
}
