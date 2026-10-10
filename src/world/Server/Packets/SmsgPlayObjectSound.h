/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPlayObjectSound : public ManagedPacket
    {
    public:
        uint32_t soundId;
        uint64_t objectGuid;

        // 6.x and 7.x clients: the position of the sound
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        SmsgPlayObjectSound() : SmsgPlayObjectSound(0, 0)
        {
        }

        SmsgPlayObjectSound(uint32_t soundId, uint64_t objectGuid) :
            ManagedPacket(SMSG_PLAY_OBJECT_SOUND, 0),
            soundId(soundId),
            objectGuid(objectGuid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 8;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands())
            {
                // sound, source, target, position
                packet << soundId;
                packet << WoWGuid(objectGuid).toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << WoWGuid(objectGuid).toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << x << y << z;
                if (m_protocol.isShadowlands())
                    packet << int32_t(0);               // broadcast text
                return true;
            }

            packet << soundId << objectGuid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
