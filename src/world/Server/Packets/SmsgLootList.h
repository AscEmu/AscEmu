/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // Used by both Player::sendLooter (Objects/Units/Players/Player.cpp - always solo,
    // hasGroupLooter left false) and Group::sendLooter (Management/Group.cpp - optionally
    // appends the group's master/round-robin looter guid in place of the trailing byte).
    class SmsgLootList : public ManagedPacket
    {
    public:
        WoWGuid creatureGuid;
        bool hasGroupLooter = false;
        WoWGuid groupLooterGuid = 0;
        WoWGuid looterGuid = 0;

        SmsgLootList() : SmsgLootList(0)
        {
        }

        explicit SmsgLootList(WoWGuid creatureGuid) :
            ManagedPacket(SMSG_LOOT_LIST, 8 + 1 + 1),
            creatureGuid(creatureGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 1 + (hasGroupLooter ? 8 : 1); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // 6.x and later: the owner of the loot (the creature here), from 7.x on also the looted object, then
                // the optional master looter and round robin winner
                const auto creature = WoWGuid(creatureGuid.getRawGuid()).toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << creature;
                if (!m_protocol.isWoD())
                    packet << creature;

                packet.writeBit(hasGroupLooter);        // master looter
                packet.writeBit(looterGuid != uint64_t(0));   // round robin winner
                packet.flushBits();

                if (hasGroupLooter)
                    packet << WoWGuid(groupLooterGuid.getRawGuid()).toGuid128(m_protocol.realmId, m_receiverMapId);
                if (looterGuid)
                    packet << WoWGuid(looterGuid.getRawGuid()).toGuid128(m_protocol.realmId, m_receiverMapId);

                return true;
            }

            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << creatureGuid;
                packet << uint8_t(hasGroupLooter);

                if (hasGroupLooter)
                    packet << groupLooterGuid;
                else
                    packet << uint8_t(0);

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(creatureGuid[5]);
                packet.writeBit(hasGroupLooter);

                if (hasGroupLooter)
                {
                    packet.writeBit(groupLooterGuid[4]);
                    packet.writeBit(groupLooterGuid[6]);
                    packet.writeBit(groupLooterGuid[0]);
                    packet.writeBit(groupLooterGuid[7]);
                    packet.writeBit(groupLooterGuid[5]);
                    packet.writeBit(groupLooterGuid[2]);
                    packet.writeBit(groupLooterGuid[3]);
                    packet.writeBit(groupLooterGuid[1]);
                }

                packet.writeBit(creatureGuid[1]);
                packet.writeBit(looterGuid != uint64_t(0));
                packet.writeBit(creatureGuid[4]);
                packet.writeBit(creatureGuid[3]);
                packet.writeBit(creatureGuid[2]);

                if (looterGuid)
                {
                    packet.writeBit(looterGuid[2]);
                    packet.writeBit(looterGuid[3]);
                    packet.writeBit(looterGuid[4]);
                    packet.writeBit(looterGuid[5]);
                    packet.writeBit(looterGuid[6]);
                    packet.writeBit(looterGuid[0]);
                    packet.writeBit(looterGuid[1]);
                    packet.writeBit(looterGuid[7]);
                }

                packet.writeBit(creatureGuid[7]);
                packet.writeBit(creatureGuid[0]);
                packet.writeBit(creatureGuid[6]);
                packet.flushBits();

                if (looterGuid)
                {
                    packet.writeByteSeq(looterGuid[7]);
                    packet.writeByteSeq(looterGuid[1]);
                    packet.writeByteSeq(looterGuid[0]);
                    packet.writeByteSeq(looterGuid[6]);
                    packet.writeByteSeq(looterGuid[5]);
                    packet.writeByteSeq(looterGuid[3]);
                    packet.writeByteSeq(looterGuid[4]);
                    packet.writeByteSeq(looterGuid[2]);
                }

                if (hasGroupLooter)
                {
                    packet.writeByteSeq(groupLooterGuid[4]);
                    packet.writeByteSeq(groupLooterGuid[5]);
                    packet.writeByteSeq(groupLooterGuid[6]);
                    packet.writeByteSeq(groupLooterGuid[3]);
                    packet.writeByteSeq(groupLooterGuid[2]);
                    packet.writeByteSeq(groupLooterGuid[7]);
                    packet.writeByteSeq(groupLooterGuid[0]);
                    packet.writeByteSeq(groupLooterGuid[1]);
                }

                packet.writeByteSeq(creatureGuid[5]);
                packet.writeByteSeq(creatureGuid[1]);
                packet.writeByteSeq(creatureGuid[6]);
                packet.writeByteSeq(creatureGuid[2]);
                packet.writeByteSeq(creatureGuid[3]);
                packet.writeByteSeq(creatureGuid[0]);
                packet.writeByteSeq(creatureGuid[7]);
                packet.writeByteSeq(creatureGuid[4]);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
