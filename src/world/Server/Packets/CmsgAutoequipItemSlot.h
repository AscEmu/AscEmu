/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    class CmsgAutoequipItemSlot : public ManagedPacket
    {
    public:
        WoWGuid itemGuid;
        int8_t destSlot;

        CmsgAutoequipItemSlot() : CmsgAutoequipItemSlot(0, 0)
        {
        }

        CmsgAutoequipItemSlot(uint64_t itemGuid, int8_t destSlot) :
            ManagedPacket(CMSG_AUTOEQUIP_ITEM_SLOT, 9),
            itemGuid(itemGuid),
            destSlot(destSlot)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                destSlot = static_cast<int8_t>(packet.read<uint8_t>());

                itemGuid[6] = packet.readBit();
                itemGuid[3] = packet.readBit();
                itemGuid[0] = packet.readBit();

                const uint32_t trailerCount = packet.readBits(2);
                itemGuid[7] = packet.readBit();

                // Trailing per-entry presence bits of an unknown/unused data block the
                // client always appends here - discarded, matches the reference handler.
                std::vector<std::pair<bool, bool>> trailerPresence;
                for (uint32_t i = 0; i < trailerCount; ++i)
                {
                    const bool hasSecond = packet.readBit();
                    const bool hasFirst = packet.readBit();
                    trailerPresence.emplace_back(hasFirst, hasSecond);
                }

                itemGuid[5] = packet.readBit();
                itemGuid[2] = packet.readBit();
                itemGuid[1] = packet.readBit();
                itemGuid[4] = packet.readBit();

                packet.readByteSeq(itemGuid[6]);
                packet.readByteSeq(itemGuid[3]);
                packet.readByteSeq(itemGuid[1]);
                packet.readByteSeq(itemGuid[7]);
                packet.readByteSeq(itemGuid[2]);
                packet.readByteSeq(itemGuid[0]);
                packet.readByteSeq(itemGuid[4]);
                packet.readByteSeq(itemGuid[5]);

                for (const auto& entry : trailerPresence)
                {
                    if (entry.first)
                        packet.readSkip<uint8_t>();
                    if (entry.second)
                        packet.readSkip<uint8_t>();
                }

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid >> destSlot;
                itemGuid.init(unpackedGuid);

                return true;
            }

            return false;
        }
    };
}
