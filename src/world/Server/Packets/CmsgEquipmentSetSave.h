/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/ItemDefines.hpp"
#include <array>
#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class CmsgEquipmentSetSave : public ManagedPacket
    {
    public:
        WoWGuid setGuid;
        uint32_t index = 0;
        std::array<WoWGuid, EQUIPMENT_SLOT_END> itemGuid{};
        std::string setName;
        std::string iconName;

        CmsgEquipmentSetSave() :
            ManagedPacket(CMSG_EQUIPMENT_SET_SAVE, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet >> index;

                for (uint32_t i = 0; i < EQUIPMENT_SLOT_END; ++i)
                {
                    itemGuid[i][5] = packet.readBit();
                    itemGuid[i][0] = packet.readBit();
                    itemGuid[i][1] = packet.readBit();
                    itemGuid[i][4] = packet.readBit();
                    itemGuid[i][6] = packet.readBit();
                    itemGuid[i][3] = packet.readBit();
                    itemGuid[i][7] = packet.readBit();
                    itemGuid[i][2] = packet.readBit();
                }

                setGuid[7] = packet.readBit();
                setGuid[1] = packet.readBit();
                setGuid[5] = packet.readBit();
                setGuid[2] = packet.readBit();
                setGuid[3] = packet.readBit();
                setGuid[0] = packet.readBit();

                uint32_t setNameLen = static_cast<uint32_t>(packet.readBits(8));
                setGuid[6] = packet.readBit();
                uint32_t iconNameLen = static_cast<uint32_t>(packet.readBits(8));

                setGuid[4] = packet.readBit();

                if (packet.readBit())
                    ++iconNameLen;

                packet.readByteSeq(setGuid[4]);
                packet.readByteSeq(setGuid[0]);

                for (uint32_t i = 0; i < EQUIPMENT_SLOT_END; ++i)
                {
                    packet.readByteSeq(itemGuid[i][1]);
                    packet.readByteSeq(itemGuid[i][0]);
                    packet.readByteSeq(itemGuid[i][7]);
                    packet.readByteSeq(itemGuid[i][3]);
                    packet.readByteSeq(itemGuid[i][6]);
                    packet.readByteSeq(itemGuid[i][2]);
                    packet.readByteSeq(itemGuid[i][4]);
                    packet.readByteSeq(itemGuid[i][5]);
                }

                iconName = packet.readString(iconNameLen);

                packet.readByteSeq(setGuid[7]);
                packet.readByteSeq(setGuid[2]);

                setName = packet.readString(setNameLen);

                packet.readByteSeq(setGuid[6]);
                packet.readByteSeq(setGuid[1]);
                packet.readByteSeq(setGuid[5]);
                packet.readByteSeq(setGuid[3]);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                setGuid.init(unpackedGuid);

                packet >> index;
                packet >> setName;
                packet >> iconName;

                for (uint32_t i = 0; i < EQUIPMENT_SLOT_END; ++i)
                {
                    uint64_t unpackedItemGuid;
                    packet >> unpackedItemGuid;
                    itemGuid[i].init(unpackedItemGuid);
                }

                return true;
            }

            return false;
        }
    };
}
