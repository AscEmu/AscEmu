/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgGuildSetNote : public ManagedPacket
    {
#if VERSION_STRING >= Cata
    public:
        bool isPublic;
        std::string note;

        ObjectGuid guid;

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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
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

            packet.ReadByteSeq(guid[4]);
            packet.ReadByteSeq(guid[5]);
            packet.ReadByteSeq(guid[0]);
            packet.ReadByteSeq(guid[3]);
            packet.ReadByteSeq(guid[1]);
            packet.ReadByteSeq(guid[6]);
            packet.ReadByteSeq(guid[7]);

            note = packet.ReadString(noteLength);

            packet.ReadByteSeq(guid[2]);
            return true;
        }
#endif
    };
}}
