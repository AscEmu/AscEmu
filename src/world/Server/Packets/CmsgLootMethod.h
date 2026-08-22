/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgLootMethod : public ManagedPacket
    {
    public:
        uint32_t method;
        WoWGuid guid;
        uint32_t threshold;

        CmsgLootMethod() : CmsgLootMethod(0, 0, 0)
        {
        }

        CmsgLootMethod(uint32_t method, uint64_t guid, uint32_t threshold) :
            ManagedPacket(CMSG_LOOT_METHOD, 16),
            method(method),
            guid(guid),
            threshold(threshold)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.readSkip<uint8_t>();

                uint8_t methodByte = 0;
                packet >> methodByte;
                method = methodByte;

                packet >> threshold;

                guid[7] = packet.readBit();
                guid[1] = packet.readBit();
                guid[2] = packet.readBit();
                guid[0] = packet.readBit();
                guid[4] = packet.readBit();
                guid[5] = packet.readBit();
                guid[6] = packet.readBit();
                guid[3] = packet.readBit();

                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[2]);

                return true;
            }
            else
            {
                uint64_t unpackedGuid;
                packet >> method >> unpackedGuid >> threshold;
                guid.init(unpackedGuid);

                return true;
            }
        }
    };
}
