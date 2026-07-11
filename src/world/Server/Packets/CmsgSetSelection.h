/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSetSelection : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgSetSelection() : CmsgSetSelection(0)
        {
        }

        CmsgSetSelection(uint64_t guid) :
            ManagedPacket(CMSG_SET_SELECTION, 0),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t rawGuid;
                packet >> rawGuid;
                guid.init(rawGuid);
            }
            else
            {
                guid[7] = packet.readBit();
                guid[6] = packet.readBit();
                guid[5] = packet.readBit();
                guid[4] = packet.readBit();
                guid[3] = packet.readBit();
                guid[2] = packet.readBit();
                guid[1] = packet.readBit();
                guid[0] = packet.readBit();
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[2]);
            }
            return true;
        }
    };
}
