/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/LFG/LFG.hpp"
#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    // Cata already consolidates the old SMSG_LFG_UPDATE_PLAYER / SMSG_LFG_UPDATE_PARTY into a
    // single opcode (SMSG_LFG_UPDATE_STATUS); Mop keeps the same opcode symbol but renames it
    // SMSG_LFD_UPDATE_STATUS on the wire and reworks the layout again (adds RequestedRoles,
    // drops the separate LfgJoined bit). Ticket id/time and dungeon category aren't tracked by
    // AscEmu's LFGMgr yet, so they're sent as 0 - purely cosmetic client-side.
    class SmsgLfgUpdateStatus : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t updateType = 0;
        bool isParty = false;
        bool joined = false;
        bool queued = false;
        uint32_t roles = 0;
        LfgDungeonSet dungeons;
        std::string comment;

        SmsgLfgUpdateStatus() : ManagedPacket(SMSG_LFG_UPDATE_STATUS, 0)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 20 + comment.length() + dungeons.size() * 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const uint32_t size = static_cast<uint32_t>(dungeons.size());

                packet.writeBits(static_cast<uint32_t>(comment.size()), 8);
                packet.writeBit(isParty);
                packet.writeBit(joined);
                packet.writeBits(size, 22);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[1]);
                packet.writeBit(joined);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[0]);
                packet.writeBit(true);
                packet.writeBit(queued);
                packet.writeBits(0, 24);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[4]);
                packet.flushBits();

                packet.writeByteSeq(guid[3]);
                packet << uint8_t(0);
                packet << uint8_t(0);
                packet << uint8_t(0);

                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[6]);
                packet << updateType;
                packet << roles;
                packet << uint32_t(0);
                packet.writeByteSeq(guid[5]);
                packet.writeString(comment);
                packet.writeByteSeq(guid[2]);

                for (auto dungeonEntry : dungeons)
                    packet << dungeonEntry;

                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[1]);
                packet << uint32_t(0);
                packet << uint8_t(0);
                packet << uint32_t(3);
                packet.writeByteSeq(guid[7]);

                return true;
            }
            else if (m_protocol.isCata())
            {
                packet.writeBit(guid[1]);
                packet.writeBit(isParty);
                packet.writeBits(static_cast<uint32_t>(dungeons.size()), 24);
                packet.writeBit(guid[6]);
                packet.writeBit(queued);
                packet.writeBits(static_cast<uint32_t>(comment.size()), 9);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[2]);
                packet.writeBit(joined);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[5]);
                packet.writeBit(queued);
                packet.flushBits();

                packet << updateType;                     // Reason
                packet.writeString(comment);

                packet << uint32_t(0);                    // Ticket.Id - not tracked
                packet << uint32_t(0);                    // Ticket.Time - not tracked
                packet.writeByteSeq(guid[6]);

                packet << uint8_t(0);
                packet << uint8_t(0);
                packet << uint8_t(0);

                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[0]);
                packet << uint32_t(3);                    // Ticket.Type - RideType::Lfg
                packet.writeByteSeq(guid[7]);

                for (auto dungeonEntry : dungeons)
                    packet << dungeonEntry;

                return true;
            }

            return false;
        }
    };
}
