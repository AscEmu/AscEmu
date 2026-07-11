/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

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
            ManagedPacket(MSG_CORPSE_QUERY, 0),
            isFound(isFound),
            mapId(mapId),
            position(position),
            corpseMapId(corpseMapId),
            unknown(unknown),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.isMop())
                return isFound ? static_cast<size_t>(1 + 8 + 4 + 4 + 4 + 4 + 4) : static_cast<size_t>(9 + (5 * 4));
            if (m_protocol.expansion > WoW::Expansion::_TBC)
                return isFound ? static_cast<size_t>(1 + 4 + 4 + 4 + 4 + 4 + 4) : 1;
            return isFound ? static_cast<size_t>(1 + 4 + 4 + 4 + 4 + 4) : 1;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
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

                    packet.writeByteSeq(guid[5]);
                    packet << position.z;
                    packet.writeByteSeq(guid[1]);
                    packet << corpseMapId;
                    packet.writeByteSeq(guid[6]);
                    packet.writeByteSeq(guid[4]);
                    packet << position.x;
                    packet.writeByteSeq(guid[3]);
                    packet.writeByteSeq(guid[7]);
                    packet.writeByteSeq(guid[2]);
                    packet.writeByteSeq(guid[0]);
                    packet << mapId;
                    packet << position.y;
                }
                else
                {
                    packet.writeBits(0, 9);
                    for (int i = 0; i < 5; ++i)
                        packet << uint32_t(0);
                }
            }
            else if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                if (isFound)
                    packet << isFound << mapId << position << corpseMapId << unknown;
                else
                    packet << isFound;
            }
            else
            {
                if (isFound)
                    packet << isFound << mapId << position << corpseMapId;
                else
                    packet << isFound;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
