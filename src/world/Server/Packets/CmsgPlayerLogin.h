/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgPlayerLogin : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgPlayerLogin() : CmsgPlayerLogin(0)
        {
        }

        CmsgPlayerLogin(uint64_t guid) :
            ManagedPacket(CMSG_PLAYER_LOGIN, 0),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.Init(unpackedGuid);
#elif VERSION_STRING == Cata
            guid[2] = packet.readBit();
            guid[3] = packet.readBit();
            guid[0] = packet.readBit();
            guid[6] = packet.readBit();
            guid[4] = packet.readBit();
            guid[5] = packet.readBit();
            guid[1] = packet.readBit();
            guid[7] = packet.readBit();

            packet.ReadByteSeq(guid[2]);
            packet.ReadByteSeq(guid[7]);
            packet.ReadByteSeq(guid[0]);
            packet.ReadByteSeq(guid[3]);
            packet.ReadByteSeq(guid[5]);
            packet.ReadByteSeq(guid[6]);
            packet.ReadByteSeq(guid[1]);
            packet.ReadByteSeq(guid[4]);
#elif VERSION_STRING == Mop
            float unknown;
            packet >> unknown;

            guid[1] = packet.readBit();
            guid[4] = packet.readBit();
            guid[7] = packet.readBit();
            guid[3] = packet.readBit();
            guid[2] = packet.readBit();
            guid[6] = packet.readBit();
            guid[5] = packet.readBit();
            guid[0] = packet.readBit();

            packet.ReadByteSeq(guid[5]);
            packet.ReadByteSeq(guid[1]);
            packet.ReadByteSeq(guid[0]);
            packet.ReadByteSeq(guid[6]);
            packet.ReadByteSeq(guid[2]);
            packet.ReadByteSeq(guid[4]);
            packet.ReadByteSeq(guid[7]);
            packet.ReadByteSeq(guid[3]);
#endif
            return true;
        }
    };
}
