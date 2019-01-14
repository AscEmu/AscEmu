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
#elif VERSION_STRING == Cata
            ObjectGuid unpackedGuid;
            unpackedGuid[2] = packet.readBit();
            unpackedGuid[3] = packet.readBit();
            unpackedGuid[0] = packet.readBit();
            unpackedGuid[6] = packet.readBit();
            unpackedGuid[4] = packet.readBit();
            unpackedGuid[5] = packet.readBit();
            unpackedGuid[1] = packet.readBit();
            unpackedGuid[7] = packet.readBit();

            packet.ReadByteSeq(unpackedGuid[2]);
            packet.ReadByteSeq(unpackedGuid[7]);
            packet.ReadByteSeq(unpackedGuid[0]);
            packet.ReadByteSeq(unpackedGuid[3]);
            packet.ReadByteSeq(unpackedGuid[5]);
            packet.ReadByteSeq(unpackedGuid[6]);
            packet.ReadByteSeq(unpackedGuid[1]);
            packet.ReadByteSeq(unpackedGuid[4]);
#elif VERSION_STRING == Mop
            ObjectGuid unpackedGuid;
            float unknown;
            packet >> unknown;

            unpackedGuid[1] = packet.readBit();
            unpackedGuid[4] = packet.readBit();
            unpackedGuid[7] = packet.readBit();
            unpackedGuid[3] = packet.readBit();
            unpackedGuid[2] = packet.readBit();
            unpackedGuid[6] = packet.readBit();
            unpackedGuid[5] = packet.readBit();
            unpackedGuid[0] = packet.readBit();

            packet.ReadByteSeq(unpackedGuid[5]);
            packet.ReadByteSeq(unpackedGuid[1]);
            packet.ReadByteSeq(unpackedGuid[0]);
            packet.ReadByteSeq(unpackedGuid[6]);
            packet.ReadByteSeq(unpackedGuid[2]);
            packet.ReadByteSeq(unpackedGuid[4]);
            packet.ReadByteSeq(unpackedGuid[7]);
            packet.ReadByteSeq(unpackedGuid[3]);
#endif
            guid.Init(unpackedGuid);
            return true;
        }
    };
}}
