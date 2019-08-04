/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgBattlemasterJoin : public ManagedPacket
    {
    public:
        ObjectGuid guid;
        uint32_t bgType;
        uint32_t instanceId;
        uint8_t asGroup;

        CmsgBattlemasterJoin() : CmsgBattlemasterJoin(0, 0, 0, 0)
        {
        }

        CmsgBattlemasterJoin(uint64_t guid, uint32_t bgType, uint32_t instanceId, uint8_t asGroup) :
            ManagedPacket(CMSG_BATTLEMASTER_JOIN, 8),
            guid(guid),
            bgType(bgType),
            instanceId(instanceId),
            asGroup(asGroup)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= WotLK
            packet >> guid >> bgType >> instanceId >> asGroup;
#endif
#if VERSION_STRING == Cata
            packet >> instanceId;

            guid[2] = packet.readBit();
            guid[0] = packet.readBit();
            guid[3] = packet.readBit();
            guid[1] = packet.readBit();
            guid[5] = packet.readBit();

            asGroup = packet.readBit();

            guid[4] = packet.readBit();
            guid[6] = packet.readBit();
            guid[7] = packet.readBit();

            packet.ReadByteSeq(guid[2]);
            packet.ReadByteSeq(guid[6]);
            packet.ReadByteSeq(guid[4]);
            packet.ReadByteSeq(guid[3]);
            packet.ReadByteSeq(guid[7]);
            packet.ReadByteSeq(guid[0]);
            packet.ReadByteSeq(guid[5]);
            packet.ReadByteSeq(guid[1]);

            bgType = guid.getCounter();
#endif

            return true;
        }
    };
}
