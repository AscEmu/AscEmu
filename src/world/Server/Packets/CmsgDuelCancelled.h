/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgDuelCancelled : public ManagedPacket
    {
    public:
        // Guid of the duel arbiter (the duel flag gameobject). Not currently used for
        // validation - AscEmu's own m_duelPlayer/m_duelState tracking already guards this
        // handler - but the client always sends it, so it must be consumed off the wire.
        WoWGuid arbiterGuid;

        CmsgDuelCancelled() : ManagedPacket(CMSG_DUEL_CANCELLED, 8)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                arbiterGuid = WoWGuid(unpackedGuid);

                return true;
            }

            return false;
        }
    };
}
