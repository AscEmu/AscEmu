/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "ForeverSpellPacketUtils.hpp"

namespace AscEmu::Packets
{
    class SmsgSpellPrepare : public ManagedPacket
    {
    public:
        WoWGuid clientCastId;
        WoWGuid serverCastId;

        SmsgSpellPrepare(WoWGuid clientCastId, WoWGuid serverCastId) :
            ManagedPacket(SMSG_SPELL_PREPARE, 0),
            clientCastId(clientCastId),
            serverCastId(serverCastId)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 36;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            ForeverSpellPacket::writePackedGuid(packet, clientCastId);
            ForeverSpellPacket::writePackedGuid(packet, serverCastId);
            return true;
        }
    };
}
