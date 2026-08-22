/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgInventoryChangeFailure : public ManagedPacket
    {
    public:
        uint8_t error;
        WoWGuid srcGuid;
        WoWGuid destGuid;
        uint32_t extraData;

        bool sendExtraData;

        SmsgInventoryChangeFailure() : SmsgInventoryChangeFailure(0, 0, 0, 0, false)
        {
        }

        SmsgInventoryChangeFailure(uint8_t error, uint64_t srcGuid, uint64_t destGuid, uint32_t extraData, bool sendExtraData) :
            ManagedPacket(SMSG_INVENTORY_CHANGE_FAILURE, 0),
            error(error),
            srcGuid(srcGuid),
            destGuid(destGuid),
            extraData(extraData),
            sendExtraData(sendExtraData)
        {
        }

    protected:

        size_t expectedSize() const override { return 22; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                if (error == 0)
                    return true;

                packet.writeBit(destGuid[4]);
                packet.writeBit(srcGuid[3]);
                packet.writeBit(destGuid[6]);
                packet.writeBit(destGuid[2]);
                packet.writeBit(srcGuid[4]);
                packet.writeBit(destGuid[5]);
                packet.writeBit(srcGuid[1]);
                packet.writeBit(srcGuid[6]);
                packet.writeBit(destGuid[0]);
                packet.writeBit(destGuid[3]);
                packet.writeBit(destGuid[1]);
                packet.writeBit(srcGuid[2]);
                packet.writeBit(srcGuid[0]);
                packet.writeBit(srcGuid[5]);
                packet.writeBit(srcGuid[7]);
                packet.writeBit(destGuid[7]);
                packet.flushBits();

                packet.writeByteSeq(destGuid[0]);
                packet << uint8_t(0);    // bag type subclass
                packet.writeByteSeq(destGuid[6]);
                packet.writeByteSeq(srcGuid[4]);
                packet.writeByteSeq(srcGuid[0]);
                packet.writeByteSeq(srcGuid[7]);
                packet.writeByteSeq(srcGuid[3]);
                packet.writeByteSeq(destGuid[1]);
                packet.writeByteSeq(destGuid[5]);
                packet.writeByteSeq(srcGuid[5]);
                packet.writeByteSeq(destGuid[7]);
                packet.writeByteSeq(destGuid[2]);
                packet.writeByteSeq(srcGuid[1]);
                packet.writeByteSeq(srcGuid[6]);
                packet.writeByteSeq(srcGuid[2]);
                packet.writeByteSeq(destGuid[3]);
                packet.writeByteSeq(destGuid[4]);
                packet << error;

                if (sendExtraData)
                    packet << extraData;

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << error << srcGuid.getRawGuid() << destGuid.getRawGuid() << uint8_t(0);
                if (sendExtraData)
                    packet << extraData;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
