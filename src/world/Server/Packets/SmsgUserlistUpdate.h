/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgUserlistUpdate : public ManagedPacket
    {
    public:
        uint32_t channelId;
        uint8_t channelFlags;
        uint8_t memberFlags;
        std::string channelName;
        WoWGuid guid;

        SmsgUserlistUpdate() : SmsgUserlistUpdate(0, 0, 0, "", 0)
        {
        }

        SmsgUserlistUpdate(uint32_t channelId, uint8_t channelFlags, uint8_t memberFlags, std::string channelName, uint64_t guid) :
            ManagedPacket(SMSG_USERLIST_UPDATE, 0),
            channelId(channelId), channelFlags(channelFlags), memberFlags(memberFlags), channelName(std::move(channelName)), guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 1 + 1 + 4 + channelName.size();
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.writeBit(guid[2]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[0]);

                packet.writeBits(channelName.size(), 7);

                packet.writeBit(guid[4]);

                packet.flushBits();

                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[5]);

                packet << uint8_t(channelFlags);

                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[3]);

                packet << uint32_t(channelId);
                packet << channelName;

                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[4]);

                packet << uint8_t(memberFlags);

                return true;
            }
            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
