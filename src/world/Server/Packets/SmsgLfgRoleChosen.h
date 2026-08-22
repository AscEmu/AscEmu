/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLfgRoleChosen : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint8_t isReady;
        uint32_t roles;

        SmsgLfgRoleChosen() : SmsgLfgRoleChosen(0, 0, 0)
        {
        }

        SmsgLfgRoleChosen(uint64_t guid, uint8_t isReady, uint32_t roles) :
            ManagedPacket(SMSG_LFG_ROLE_CHOSEN, 0),
            guid(guid),
            isReady(isReady),
            roles(roles)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 1 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const WoWGuid wowGuid(guid);

                packet.writeBit(wowGuid[6]);
                packet.writeBit(wowGuid[2]);
                packet.writeBit(wowGuid[1]);
                packet.writeBit(wowGuid[7]);
                packet.writeBit(wowGuid[0]);
                packet.writeBit(roles > 0);
                packet.writeBit(wowGuid[3]);
                packet.writeBit(wowGuid[5]);
                packet.writeBit(wowGuid[4]);
                packet.flushBits();

                packet.writeByteSeq(wowGuid[0]);
                packet.writeByteSeq(wowGuid[3]);
                packet.writeByteSeq(wowGuid[6]);
                packet << roles;
                packet.writeByteSeq(wowGuid[5]);
                packet.writeByteSeq(wowGuid[1]);
                packet.writeByteSeq(wowGuid[4]);
                packet.writeByteSeq(wowGuid[2]);
                packet.writeByteSeq(wowGuid[7]);

                return true;
            }
            else if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                packet << guid << isReady << roles;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
