/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class MsgCorspeQuery : public ManagedPacket
    {
    public:
        uint8_t isFound;
        uint32_t mapId;
        LocationVector position;
        uint32_t corpseMapId;
        uint32_t unknown;
        WoWGuid guid;

        MsgCorspeQuery() : MsgCorspeQuery(0)
        {
        }

        MsgCorspeQuery(uint8_t isFound, uint32_t mapId = 0, LocationVector position = {}, uint32_t corpseMapId = 0, uint32_t unknown = 0, WoWGuid guid = uint64_t(0)) :
            ManagedPacket(MSG_CORPSE_QUERY, isFound ? 1 + 5 * 4 : 1),
            isFound(isFound),
            mapId(mapId),
            position(position),
            corpseMapId(corpseMapId),
            unknown(unknown),
            guid(guid)
        {
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING > TBC
            if (isFound)
                packet << isFound << mapId << position << corpseMapId << unknown;
            else
                packet << isFound;

#elif VERSION_STRING < Mop
            if (isFound)
                packet << isFound << mapId << position << corpseMapId;
            else
                packet << isFound;

#else
            if (isFound)
            {
                packet.writeBit(guid[0]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[2]);
                packet.writeBit(isFound);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[6]);

                packet.WriteByteSeq(guid[5]);
                packet << position.z;
                packet.WriteByteSeq(guid[1]);
                packet << corpseMapId;
                packet.WriteByteSeq(guid[6]);
                packet.WriteByteSeq(guid[4]);
                packet << position.x;
                packet.WriteByteSeq(guid[3]);
                packet.WriteByteSeq(guid[7]);
                packet.WriteByteSeq(guid[2]);
                packet.WriteByteSeq(guid[0]);
                packet << mapId;
                packet << position.y;
            }
            else
            {
                packet.writeBits(0, 9);
                for (int i = 0; i < 5; ++i)
                    packet << uint32_t(0);
            }
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
