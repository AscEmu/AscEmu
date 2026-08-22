/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGroupChangeSubGroup : public ManagedPacket
    {
    public:
        std::string name;
        uint8_t subGroup;
        WoWGuid guid;

        CmsgGroupChangeSubGroup() : CmsgGroupChangeSubGroup("", 0)
        {
        }

        CmsgGroupChangeSubGroup(std::string name, uint8_t subGroup) :
            ManagedPacket(CMSG_GROUP_CHANGE_SUB_GROUP, 0),
            name(name),
            subGroup(subGroup)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.readSkip<uint8_t>();
                packet >> subGroup;

                guid[1] = packet.readBit();
                guid[4] = packet.readBit();
                guid[6] = packet.readBit();
                guid[3] = packet.readBit();
                guid[7] = packet.readBit();
                guid[2] = packet.readBit();
                guid[0] = packet.readBit();
                guid[5] = packet.readBit();

                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[7]);

                return true;
            }
            else
            {
                packet >> name >> subGroup;

                return true;
            }
        }
    };
}
