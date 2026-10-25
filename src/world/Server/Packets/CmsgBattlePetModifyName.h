/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "BattlePetPacketData.hpp"

#include <array>
#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    // a new name for a pet; the declined names of the russian client are read past
    class CmsgBattlePetModifyName : public ManagedPacket
    {
    public:
        uint32_t petId = 0;
        std::string name;

        CmsgBattlePetModifyName() : ManagedPacket(CMSG_BATTLE_PET_MODIFY_NAME, 2)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                petId = BattlePetWire::readGuid(packet);
                const uint32_t nameLength = packet.readBits(7);
                const bool hasDeclined = packet.readBit();

                // 6.x and 12.x send the name before the declined names, 7.x to 11.x after them
                if (!BattlePetWire::bitsBeforeLists(m_protocol) || !hasDeclined)
                {
                    name = packet.readString(nameLength);
                    return true;
                }

                std::array<uint32_t, 5> declinedLengths{};
                for (auto& length : declinedLengths)
                    length = packet.readBits(7);
                for (const uint32_t length : declinedLengths)
                    packet.readString(length);

                name = packet.readString(nameLength);
                return true;
            }

            if (m_protocol.isMop())
            {
                WoWGuid guid;
                guid[5] = packet.readBit();
                guid[7] = packet.readBit();
                guid[3] = packet.readBit();
                guid[0] = packet.readBit();
                guid[6] = packet.readBit();
                const uint32_t nameLength = packet.readBits(7);
                guid[2] = packet.readBit();
                guid[1] = packet.readBit();
                const bool hasDeclined = packet.readBit();
                guid[4] = packet.readBit();

                std::array<uint32_t, 5> declinedLengths{};
                if (hasDeclined)
                    for (auto& length : declinedLengths)
                        length = packet.readBits(7);

                for (const uint8_t byte : std::array<uint8_t, 8>{ 3, 0, 6, 1, 5, 2, 4, 7 })
                    packet.readByteSeq(guid[byte]);

                name = packet.readString(nameLength);
                petId = static_cast<uint32_t>(guid.getRawGuid());
                return true;
            }

            return false;
        }
    };
}
