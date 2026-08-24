/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgAttackSwing : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgAttackSwing() : CmsgAttackSwing(0)
        {
        }

        CmsgAttackSwing(uint64_t guid) :
            ManagedPacket(CMSG_ATTACKSWING, 0),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
            {
                uint64_t unpacked_guid;
                packet >> unpacked_guid;

                guid.init(unpacked_guid);
                return true;
            }
            else
            {
                WoWGuid unpacked_guid;
                unpacked_guid[6] = packet.readBit();
                unpacked_guid[5] = packet.readBit();
                unpacked_guid[7] = packet.readBit();
                unpacked_guid[0] = packet.readBit();
                unpacked_guid[3] = packet.readBit();
                unpacked_guid[1] = packet.readBit();
                unpacked_guid[4] = packet.readBit();
                unpacked_guid[2] = packet.readBit();

                packet.readByteSeq(unpacked_guid[6]);
                packet.readByteSeq(unpacked_guid[7]);
                packet.readByteSeq(unpacked_guid[1]);
                packet.readByteSeq(unpacked_guid[3]);
                packet.readByteSeq(unpacked_guid[2]);
                packet.readByteSeq(unpacked_guid[0]);
                packet.readByteSeq(unpacked_guid[4]);
                packet.readByteSeq(unpacked_guid[5]);
                guid.init(unpacked_guid);

                return true;
            }
        }
    };
}
