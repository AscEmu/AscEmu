/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class CmsgRequestHotfix : public ManagedPacket
    {
    public:
        uint32_t type {0};
        uint32_t count {0};
        uint32_t entry {0};

        std::vector<uint32_t> entries;

        CmsgRequestHotfix() : ManagedPacket(CMSG_REQUEST_HOTFIX, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet >> type;
                count = packet.readBits(23);

                auto guids = std::make_unique<WoWGuid[]>(count);
                for (uint32_t i = 0; i < count; ++i)
                {
                    guids[i][0] = packet.readBit();
                    guids[i][4] = packet.readBit();
                    guids[i][7] = packet.readBit();
                    guids[i][2] = packet.readBit();
                    guids[i][5] = packet.readBit();
                    guids[i][3] = packet.readBit();
                    guids[i][6] = packet.readBit();
                    guids[i][1] = packet.readBit();
                }

                for (uint32_t i = 0; i < count; ++i)
                {
                    packet.readByteSeq(guids[i][5]);
                    packet.readByteSeq(guids[i][6]);
                    packet.readByteSeq(guids[i][7]);
                    packet.readByteSeq(guids[i][0]);
                    packet.readByteSeq(guids[i][1]);
                    packet.readByteSeq(guids[i][3]);
                    packet.readByteSeq(guids[i][4]);

                    packet >> entry;

                    packet.readByteSeq(guids[i][2]);
                }

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> type;
                count = packet.readBits(21);

                entries.reserve(count);

                auto guids = std::make_unique<WoWGuid[]>(count);
                for (uint32_t i = 0; i < count; ++i)
                {
                    guids[i][6] = packet.readBit();
                    guids[i][3] = packet.readBit();
                    guids[i][0] = packet.readBit();
                    guids[i][1] = packet.readBit();
                    guids[i][4] = packet.readBit();
                    guids[i][5] = packet.readBit();
                    guids[i][7] = packet.readBit();
                    guids[i][2] = packet.readBit();

                    packet.readByteSeq(guids[i][1]);

                    packet >> entry;

                    entries.push_back(entry);

                    packet.readByteSeq(guids[i][0]);
                    packet.readByteSeq(guids[i][5]);
                    packet.readByteSeq(guids[i][6]);
                    packet.readByteSeq(guids[i][4]);
                    packet.readByteSeq(guids[i][7]);
                    packet.readByteSeq(guids[i][2]);
                    packet.readByteSeq(guids[i][3]);
                }

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WoD && m_protocol.expansion <= WoW::Expansion::_Legion)
            {
                // bulk query: hash of the client data table and the requested records
                packet >> type;
                count = packet.readBits(13);

                entries.reserve(count);
                for (uint32_t i = 0; i < count; ++i)
                {
                    WoWGuid128 guid;
                    packet >> guid;
                    packet >> entry;

                    entries.push_back(entry);
                }

                return !packet.hadReadFailure();
            }
            else if (m_protocol.isBfA() || m_protocol.isShadowlands())
            {
                // bulk query: hash of the client data table and the requested record ids
                packet >> type;
                count = packet.readBits(13);

                entries.reserve(count);
                for (uint32_t i = 0; i < count; ++i)
                {
                    packet >> entry;
                    entries.push_back(entry);
                }

                return !packet.hadReadFailure();
            }

            return false;
        }
    };
}
