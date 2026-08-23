/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgBattlefieldPort : public ManagedPacket
    {
    public:
        uint16_t unknown;
        uint32_t bgType;
        uint16_t mapInfo;
        uint8_t action;

        // Cata+ fields. AscEmu's own handler logic only actually consumes `action` (enter
        // vs leave queue) - queueSlot/time/guid are read here purely to stay wire-aligned,
        // matching the reference layout, and are not otherwise used.
        uint32_t queueSlot = 0;
        uint32_t time = 0;
        WoWGuid guid;

        CmsgBattlefieldPort() : CmsgBattlefieldPort(0, 0, 0, 0)
        {
        }

        CmsgBattlefieldPort(uint16_t unknown, uint32_t bgType, uint16_t mapInfo, uint8_t action) :
            ManagedPacket(CMSG_BATTLEFIELD_PORT, 0),
            unknown(unknown),
            bgType(bgType),
            mapInfo(mapInfo),
            action(action)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet >> unknown >> bgType >> mapInfo >> action;

                return true;
            }
            else if (m_protocol.isCata())
            {
                packet >> time;
                packet >> queueSlot;
                packet.readSkip<uint32_t>();   // unk

                guid[0] = packet.readBit();
                guid[1] = packet.readBit();
                guid[5] = packet.readBit();
                guid[6] = packet.readBit();
                guid[7] = packet.readBit();
                guid[4] = packet.readBit();
                guid[3] = packet.readBit();
                guid[2] = packet.readBit();

                action = packet.readBit();

                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[4]);

                return true;
            }
            else if (m_protocol.isMop())
            {
                action = packet.readBit();

                packet >> queueSlot;
                packet.readSkip<uint32_t>();   // id
                packet >> time;

                guid[6] = packet.readBit();
                guid[4] = packet.readBit();
                guid[2] = packet.readBit();
                guid[5] = packet.readBit();
                guid[0] = packet.readBit();
                guid[1] = packet.readBit();
                guid[7] = packet.readBit();
                guid[3] = packet.readBit();

                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[1]);

                return true;
            }

            return false;
        }
    };
}
