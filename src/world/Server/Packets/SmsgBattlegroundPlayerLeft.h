/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgBattlegroundPlayerLeft : public ManagedPacket
    {
    public:
        WoWGuid guid;

        SmsgBattlegroundPlayerLeft() : SmsgBattlegroundPlayerLeft(WoWGuid())
        {
        }

        SmsgBattlegroundPlayerLeft(WoWGuid guid) :
            ManagedPacket(SMSG_BATTLEGROUND_PLAYER_LEFT, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= WotLK
            packet << guid;
#else
            ObjectGuid bitGuid = guid;

#if VERSION_STRING == Cata

            packet.writeBit(bitGuid[7]);
            packet.writeBit(bitGuid[6]);
            packet.writeBit(bitGuid[2]);
            packet.writeBit(bitGuid[4]);
            packet.writeBit(bitGuid[5]);
            packet.writeBit(bitGuid[1]);
            packet.writeBit(bitGuid[3]);
            packet.writeBit(bitGuid[0]);

            packet.WriteByteSeq(bitGuid[4]);
            packet.WriteByteSeq(bitGuid[2]);
            packet.WriteByteSeq(bitGuid[5]);
            packet.WriteByteSeq(bitGuid[7]);
            packet.WriteByteSeq(bitGuid[0]);
            packet.WriteByteSeq(bitGuid[6]);
            packet.WriteByteSeq(bitGuid[1]);
            packet.WriteByteSeq(bitGuid[3]);

#elif VERSION_STRING == Mop

            packet.writeBit(bitGuid[3]);
            packet.writeBit(bitGuid[5]);
            packet.writeBit(bitGuid[6]);
            packet.writeBit(bitGuid[0]);
            packet.writeBit(bitGuid[1]);
            packet.writeBit(bitGuid[2]);
            packet.writeBit(bitGuid[7]);
            packet.writeBit(bitGuid[4]);

            packet.WriteByteSeq(bitGuid[0]);
            packet.WriteByteSeq(bitGuid[6]);
            packet.WriteByteSeq(bitGuid[5]);
            packet.WriteByteSeq(bitGuid[7]);
            packet.WriteByteSeq(bitGuid[2]);
            packet.WriteByteSeq(bitGuid[1]);
            packet.WriteByteSeq(bitGuid[3]);
            packet.WriteByteSeq(bitGuid[4]);

#endif
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
