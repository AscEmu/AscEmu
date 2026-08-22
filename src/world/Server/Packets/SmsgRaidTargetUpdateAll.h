/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Group.h"

namespace AscEmu::Packets
{
    class SmsgRaidTargetUpdateAll : public ManagedPacket
    {
    public:
        Group const* group;

        static constexpr uint8_t iconCount = 8;

        SmsgRaidTargetUpdateAll() : SmsgRaidTargetUpdateAll(nullptr)
        {
        }

        SmsgRaidTargetUpdateAll(Group const* group) :
            ManagedPacket(SMSG_RAID_TARGET_UPDATE_ALL, static_cast<size_t>(1 + iconCount * 9)),
            group(group)
        {
        }

    protected:
        size_t expectedSize() const override { return static_cast<size_t>(1 + iconCount * 9); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
                return false;

            packet.writeBits(0, 25);
            packet.flushBits();

            if (group != nullptr)
            {
                for (uint8_t i = 0; i < iconCount; ++i)
                {
                    const WoWGuid guid = group->m_targetIcons[i];
                    if (guid.getRawGuid() == 0)
                        continue;

                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[5]);

                    packet.flushBits();

                    packet.writeByteSeq(guid[4]);
                    packet.writeByteSeq(guid[7]);
                    packet.writeByteSeq(guid[1]);
                    packet.writeByteSeq(guid[0]);
                    packet.writeByteSeq(guid[6]);
                    packet.writeByteSeq(guid[5]);
                    packet.writeByteSeq(guid[3]);
                    packet << i;
                    packet.writeByteSeq(guid[2]);
                }
            }

            packet << static_cast<uint8_t>(0);

            return true;
        }
    };
}
