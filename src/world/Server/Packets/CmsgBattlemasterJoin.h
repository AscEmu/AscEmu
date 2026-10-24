/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgBattlemasterJoin : public ManagedPacket
    {
    public:
        uint64_t guid;

        uint32_t bgType;
        uint32_t instanceId;
        uint8_t asGroup;

        // Mop-only: an optional LFG-style role mask, sent only when hasRoleMask is set on
        // the wire. Not present at all pre-Mop.
        uint8_t roleMask = 0;

        CmsgBattlemasterJoin() : CmsgBattlemasterJoin(0, 0, 0, 0)
        {
        }

        CmsgBattlemasterJoin(uint64_t guid, uint32_t bgType, uint32_t instanceId, uint8_t asGroup) :
            ManagedPacket(CMSG_BATTLEMASTER_JOIN, 0),
            guid(guid),
            bgType(bgType),
            instanceId(instanceId),
            asGroup(asGroup)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet >> guid >> bgType >> instanceId >> asGroup;

                return true;
            }
            else if (m_protocol.expansion == WoW::Expansion::_Cata)
            {
                WoWGuid guidCount;
                packet >> instanceId;

                guidCount[2] = packet.readBit();
                guidCount[0] = packet.readBit();
                guidCount[3] = packet.readBit();
                guidCount[1] = packet.readBit();
                guidCount[5] = packet.readBit();

                asGroup = packet.readBit();

                guidCount[4] = packet.readBit();
                guidCount[6] = packet.readBit();
                guidCount[7] = packet.readBit();

                packet.readByteSeq(guidCount[2]);
                packet.readByteSeq(guidCount[6]);
                packet.readByteSeq(guidCount[4]);
                packet.readByteSeq(guidCount[3]);
                packet.readByteSeq(guidCount[7]);
                packet.readByteSeq(guidCount[0]);
                packet.readByteSeq(guidCount[5]);
                packet.readByteSeq(guidCount[1]);

                bgType = guidCount.getCounter();

                return true;
            }
            else if (m_protocol.isMop())
            {
                // No instanceId on the wire in Mop.
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();

                WoWGuid guidCount;

                guidCount[1] = packet.readBit();
                guidCount[7] = packet.readBit();
                guidCount[0] = packet.readBit();
                guidCount[3] = packet.readBit();

                asGroup = packet.readBit();

                guidCount[4] = packet.readBit();

                const bool hasRoleMask = !packet.readBit();

                guidCount[6] = packet.readBit();
                guidCount[2] = packet.readBit();
                guidCount[5] = packet.readBit();

                packet.readByteSeq(guidCount[7]);
                packet.readByteSeq(guidCount[2]);
                packet.readByteSeq(guidCount[4]);
                packet.readByteSeq(guidCount[5]);
                packet.readByteSeq(guidCount[0]);
                packet.readByteSeq(guidCount[6]);
                packet.readByteSeq(guidCount[3]);
                packet.readByteSeq(guidCount[1]);

                if (hasRoleMask)
                    packet >> roleMask;

                bgType = guidCount.getCounter();

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // 6.x+: the queue id names the battlemaster list, 8.x+ sends a list of them and no group flag
                uint64_t queueId = 0;
                if (m_protocol.expansion >= WoW::Expansion::_BfA)
                {
                    uint32_t count = 0;
                    packet >> count >> roleMask;
                    packet.readSkip<int32_t>();     // blacklist maps
                    packet.readSkip<int32_t>();
                    if (count == 0)
                        return false;
                    packet >> queueId;
                }
                else
                {
                    packet >> queueId >> roleMask;
                    packet.readSkip<int32_t>();     // blacklist maps
                    packet.readSkip<int32_t>();
                    asGroup = packet.readBit();
                }
                bgType = static_cast<uint32_t>(queueId & 0xFFFF);
                instanceId = 0;
                return true;
            }

            return false;
        }
    };
}
