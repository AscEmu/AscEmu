/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSocketGems : public ManagedPacket
    {
    public:
        WoWGuid itemGuid;
        uint64_t gemGuid[3];

        CmsgSocketGems() : CmsgSocketGems(0)
        {
        }

        CmsgSocketGems(uint64_t itemGuid) :
            ManagedPacket(CMSG_SOCKET_GEMS, 0),
            itemGuid(itemGuid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                itemGuid.init(unpackedGuid);

                for (auto i = 0; i < 3; ++i)
                    packet >> gemGuid[i];

                return true;
            }
            else if (m_protocol.isMop())
            {
                WoWGuid gemGuids[3];

                for (int i = 0; i < 3; ++i)
                    gemGuids[i][4] = packet.readBit();
                for (int i = 0; i < 3; ++i)
                    gemGuids[i][0] = packet.readBit();
                for (int i = 0; i < 3; ++i)
                    gemGuids[i][6] = packet.readBit();
                for (int i = 0; i < 3; ++i)
                    gemGuids[i][2] = packet.readBit();
                for (int i = 0; i < 3; ++i)
                    gemGuids[i][1] = packet.readBit();
                for (int i = 0; i < 3; ++i)
                    gemGuids[i][7] = packet.readBit();
                for (int i = 0; i < 3; ++i)
                    gemGuids[i][3] = packet.readBit();
                for (int i = 0; i < 3; ++i)
                    gemGuids[i][5] = packet.readBit();

                itemGuid[5] = packet.readBit();
                itemGuid[0] = packet.readBit();
                itemGuid[6] = packet.readBit();
                itemGuid[2] = packet.readBit();
                itemGuid[3] = packet.readBit();
                itemGuid[4] = packet.readBit();
                itemGuid[7] = packet.readBit();
                itemGuid[1] = packet.readBit();

                packet.readByteSeq(itemGuid[7]);
                packet.readByteSeq(itemGuid[2]);
                packet.readByteSeq(itemGuid[6]);

                for (int i = 0; i < 3; ++i)
                    packet.readByteSeq(gemGuids[i][6]);
                for (int i = 0; i < 3; ++i)
                    packet.readByteSeq(gemGuids[i][4]);
                for (int i = 0; i < 3; ++i)
                    packet.readByteSeq(gemGuids[i][3]);
                for (int i = 0; i < 3; ++i)
                    packet.readByteSeq(gemGuids[i][2]);
                for (int i = 0; i < 3; ++i)
                    packet.readByteSeq(gemGuids[i][0]);
                for (int i = 0; i < 3; ++i)
                    packet.readByteSeq(gemGuids[i][1]);
                for (int i = 0; i < 3; ++i)
                    packet.readByteSeq(gemGuids[i][7]);
                for (int i = 0; i < 3; ++i)
                    packet.readByteSeq(gemGuids[i][5]);

                packet.readByteSeq(itemGuid[4]);
                packet.readByteSeq(itemGuid[3]);
                packet.readByteSeq(itemGuid[1]);
                packet.readByteSeq(itemGuid[5]);
                packet.readByteSeq(itemGuid[0]);

                for (int i = 0; i < 3; ++i)
                    gemGuid[i] = uint64_t(gemGuids[i]);

                return true;
            }

            return false;
        }
    };
}
