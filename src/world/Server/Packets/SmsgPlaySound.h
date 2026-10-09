/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPlaySound : public ManagedPacket
    {
    public:
        uint32_t soundId;
        WoWGuid sourceGuid;     // Mop

        SmsgPlaySound() : SmsgPlaySound(0)
        {
        }

        SmsgPlaySound(uint32_t soundId, WoWGuid sourceGuid = WoWGuid()) :
            ManagedPacket(SMSG_PLAY_SOUND, 0),
            soundId(soundId),
            sourceGuid(sourceGuid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
                return 4;
            else if (m_protocol.isMop())
                return 4 + 9;

            return 0;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << soundId;

                return true;
            }
            else if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA())
            {
                packet << int32_t(soundId);
                packet << sourceGuid.toGuid128(m_protocol.realmId, m_receiverMapId);
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(sourceGuid[2]);
                packet.writeBit(sourceGuid[3]);
                packet.writeBit(sourceGuid[7]);
                packet.writeBit(sourceGuid[6]);
                packet.writeBit(sourceGuid[0]);
                packet.writeBit(sourceGuid[5]);
                packet.writeBit(sourceGuid[4]);
                packet.writeBit(sourceGuid[1]);

                packet << soundId;

                packet.writeByteSeq(sourceGuid[3]);
                packet.writeByteSeq(sourceGuid[2]);
                packet.writeByteSeq(sourceGuid[4]);
                packet.writeByteSeq(sourceGuid[7]);
                packet.writeByteSeq(sourceGuid[5]);
                packet.writeByteSeq(sourceGuid[0]);
                packet.writeByteSeq(sourceGuid[6]);
                packet.writeByteSeq(sourceGuid[1]);

                return true;
            }
            
            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
