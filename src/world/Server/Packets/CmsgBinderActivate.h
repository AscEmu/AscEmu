/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgBinderActivate : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgBinderActivate() : CmsgBinderActivate(0)
        {
        }

        CmsgBinderActivate(uint64_t guid) :
            ManagedPacket(CMSG_BINDER_ACTIVATE, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                guid.init(unpackedGuid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                WoWGuid npcGuid;
                npcGuid[0] = packet.readBit();
                npcGuid[5] = packet.readBit();
                npcGuid[4] = packet.readBit();
                npcGuid[7] = packet.readBit();
                npcGuid[6] = packet.readBit();
                npcGuid[2] = packet.readBit();
                npcGuid[1] = packet.readBit();
                npcGuid[3] = packet.readBit();

                packet.readByteSeq(npcGuid[0]);
                packet.readByteSeq(npcGuid[4]);
                packet.readByteSeq(npcGuid[2]);
                packet.readByteSeq(npcGuid[3]);
                packet.readByteSeq(npcGuid[7]);
                packet.readByteSeq(npcGuid[1]);
                packet.readByteSeq(npcGuid[5]);
                packet.readByteSeq(npcGuid[6]);

                guid.init(npcGuid);
                return true;
            }

            return false;
        }
    };
}
