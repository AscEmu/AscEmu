/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgPlayerLogin : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgPlayerLogin() : CmsgPlayerLogin(0)
        {
        }

        CmsgPlayerLogin(uint64_t guid) :
            ManagedPacket(CMSG_PLAYER_LOGIN, 0),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                guid.init(unpackedGuid);
            }
            else if (m_protocol.expansion == WoW::Expansion::_Cata)
            {
                WoWGuid unpackedGuid;
                unpackedGuid[2] = packet.readBit();
                unpackedGuid[3] = packet.readBit();
                unpackedGuid[0] = packet.readBit();
                unpackedGuid[6] = packet.readBit();
                unpackedGuid[4] = packet.readBit();
                unpackedGuid[5] = packet.readBit();
                unpackedGuid[1] = packet.readBit();
                unpackedGuid[7] = packet.readBit();

                packet.readByteSeq(unpackedGuid[2]);
                packet.readByteSeq(unpackedGuid[7]);
                packet.readByteSeq(unpackedGuid[0]);
                packet.readByteSeq(unpackedGuid[3]);
                packet.readByteSeq(unpackedGuid[5]);
                packet.readByteSeq(unpackedGuid[6]);
                packet.readByteSeq(unpackedGuid[1]);
                packet.readByteSeq(unpackedGuid[4]);
                guid.init(unpackedGuid);
            }
            else if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                WoWGuid unpackedGuid;
                float unknown;
                packet >> unknown;

                unpackedGuid[1] = packet.readBit();
                unpackedGuid[4] = packet.readBit();
                unpackedGuid[7] = packet.readBit();
                unpackedGuid[3] = packet.readBit();
                unpackedGuid[2] = packet.readBit();
                unpackedGuid[6] = packet.readBit();
                unpackedGuid[5] = packet.readBit();
                unpackedGuid[0] = packet.readBit();

                packet.readByteSeq(unpackedGuid[5]);
                packet.readByteSeq(unpackedGuid[1]);
                packet.readByteSeq(unpackedGuid[0]);
                packet.readByteSeq(unpackedGuid[6]);
                packet.readByteSeq(unpackedGuid[2]);
                packet.readByteSeq(unpackedGuid[4]);
                packet.readByteSeq(unpackedGuid[7]);
                packet.readByteSeq(unpackedGuid[3]);
                guid.init(unpackedGuid);
            }
            
            return true;
        }
    };
}
