/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgSetActiveMover : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgSetActiveMover() : CmsgSetActiveMover(0)
        {
        }

        CmsgSetActiveMover(uint64_t guid) :
            ManagedPacket(CMSG_SET_ACTIVE_MOVER, 0),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid.getRawGuid();
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            sLogger.info("DEBUG: CmsgSetActiveMover deserialise. Size: {}", packet.size());
#if VERSION_STRING == Cata
            try {
                guid[7] = packet.readBit();
                guid[2] = packet.readBit();
                guid[1] = packet.readBit();
                guid[0] = packet.readBit();
                guid[4] = packet.readBit();
                guid[5] = packet.readBit();
                guid[6] = packet.readBit();
                guid[3] = packet.readBit();

                packet.ReadByteSeq(guid[3]);
                packet.ReadByteSeq(guid[2]);
                packet.ReadByteSeq(guid[4]);
                packet.ReadByteSeq(guid[0]);
                packet.ReadByteSeq(guid[5]);
                packet.ReadByteSeq(guid[1]);
                packet.ReadByteSeq(guid[6]);
                packet.ReadByteSeq(guid[7]);
                
                sLogger.info("DEBUG: CmsgSetActiveMover GUID parsed: 0x{:X}", guid.getRawGuid());
            } catch (...) {
                sLogger.failure("DEBUG: CmsgSetActiveMover exception");
                return false;
            }
#elif VERSION_STRING == Mop

            packet.readBit(); // unk

            guid[3] = packet.readBit();
            guid[0] = packet.readBit();
            guid[2] = packet.readBit();
            guid[1] = packet.readBit();
            guid[5] = packet.readBit();
            guid[4] = packet.readBit();
            guid[7] = packet.readBit();
            guid[6] = packet.readBit();

            packet.ReadByteSeq(guid[3]);
            packet.ReadByteSeq(guid[4]);
            packet.ReadByteSeq(guid[5]);
            packet.ReadByteSeq(guid[2]);
            packet.ReadByteSeq(guid[7]);
            packet.ReadByteSeq(guid[0]);
            packet.ReadByteSeq(guid[1]);
            packet.ReadByteSeq(guid[6]);
#else
            uint64_t unpacked_guid;
            packet >> unpacked_guid;
            guid = WoWGuid(unpacked_guid);
#endif
            return true;
        }
    };
}
