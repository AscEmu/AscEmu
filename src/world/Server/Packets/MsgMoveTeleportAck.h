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
    class MsgMoveTeleportAck : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t flags;
        uint32_t time;

        MsgMoveTeleportAck() : MsgMoveTeleportAck(0, 0)
        {
        }

        MsgMoveTeleportAck(uint32_t flags, uint32_t time) :
            ManagedPacket(MSG_MOVE_TELEPORT_ACK, 4 + 4 + 8),
            flags(flags),
            time(time)
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
            packet >> guid >> flags >> time;
#else
            packet >> flags >> time;

            ObjectGuid cataGuid;
            cataGuid[5] = packet.readBit();
            cataGuid[0] = packet.readBit();
            cataGuid[1] = packet.readBit();
            cataGuid[6] = packet.readBit();
            cataGuid[3] = packet.readBit();
            cataGuid[7] = packet.readBit();
            cataGuid[2] = packet.readBit();
            cataGuid[4] = packet.readBit();

            packet.ReadByteSeq(cataGuid[4]);
            packet.ReadByteSeq(cataGuid[2]);
            packet.ReadByteSeq(cataGuid[7]);
            packet.ReadByteSeq(cataGuid[6]);
            packet.ReadByteSeq(cataGuid[5]);
            packet.ReadByteSeq(cataGuid[1]);
            packet.ReadByteSeq(cataGuid[3]);
            packet.ReadByteSeq(cataGuid[0]);

            guid.Init(cataGuid);
#endif
            return true;
        }
    };
}}
