/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Macros/PlayerMacros.hpp"

#include <array>
#include <cstdint>
#include <cstring>

namespace AscEmu::Packets
{
    struct ActionButtonEntry
    {
        uint32_t action = 0;
        uint32_t type = 0;
        uint8_t misc = 0; // not sent to Mop clients
    };

    class SmsgUpdateActionButtons : public ManagedPacket
    {
    public:
        std::array<ActionButtonEntry, PLAYER_ACTION_BUTTON_COUNT> buttons{};
        uint8_t action = 0;

        SmsgUpdateActionButtons() : SmsgUpdateActionButtons({}, 0)
        {
        }

        SmsgUpdateActionButtons(std::array<ActionButtonEntry, PLAYER_ACTION_BUTTON_COUNT> buttons, uint8_t action) :
            ManagedPacket(SMSG_UPDATE_ACTION_BUTTONS, MIN_PLAYER_ACTION_BUTTON_PACKET_SIZE),
            buttons(std::move(buttons)),
            action(action)
        {
        }

    protected:
        size_t expectedSize() const override { return getPacketSize(); }

        bool internalSerialise(WorldPacket& packet) override
        {
            const uint8_t wireCount = getWireCount();

            if (m_protocol.isMainlineAtLeast(WoW::Expansion::_WoD))
            {
                // every button packed: action in the low 56 bits, type in the high byte
                for (uint8_t i = 0; i < wireCount; ++i)
                {
                    packet << uint64_t((uint64_t(buttons[i].action) & 0x00FFFFFFFFFFFFFFULL) |
                        (uint64_t(buttons[i].type & 0xFF) << 56));
                }
                packet << action;
                return true;
            }

            if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                // every button is sent as 8 bytes (action, type), split into bit and byte streams
                uint8_t rawButtons[PLAYER_ACTION_BUTTON_COUNT][8] = {};

                for (uint8_t i = 0; i < wireCount; ++i)
                {
                    std::memcpy(&rawButtons[i][0], &buttons[i].action, sizeof(uint32_t));
                    std::memcpy(&rawButtons[i][4], &buttons[i].type, sizeof(uint32_t));
                }

                // Bits
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeBit(rawButtons[i][4]);
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeBit(rawButtons[i][5]);
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeBit(rawButtons[i][3]);
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeBit(rawButtons[i][1]);
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeBit(rawButtons[i][6]);
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeBit(rawButtons[i][7]);
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeBit(rawButtons[i][0]);
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeBit(rawButtons[i][2]);

                // Data
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeByteSeq(rawButtons[i][0]);
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeByteSeq(rawButtons[i][1]);
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeByteSeq(rawButtons[i][4]);
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeByteSeq(rawButtons[i][6]);
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeByteSeq(rawButtons[i][7]);
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeByteSeq(rawButtons[i][2]);
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeByteSeq(rawButtons[i][5]);
                for (uint8_t i = 0; i < wireCount; ++i) packet.writeByteSeq(rawButtons[i][3]);

                packet << action;

                return true;
            }

            // WotLK: Action header before buttons
            if (m_protocol.expansion == WoW::Expansion::_WotLK)
                packet << action;

            // WotLK & Cata: 32-bit packed (24 bits action, 8 bits type) per button
            if (m_protocol.isMainlineAtLeast(WoW::Expansion::_WotLK))
            {
                for (uint8_t i = 0; i < wireCount; ++i)
                {
                    uint32_t packed = (buttons[i].action & 0x00FFFFFF) | ((uint32_t(buttons[i].type) & 0xFF) << 24);
                    packet << packed;
                }
            }
            else
            {
                // Classic & TBC: 16-Bit action + 8-Bit type + 8-Bit misc
                for (uint8_t i = 0; i < wireCount; ++i)
                {
                    packet << uint16_t(buttons[i].action);
                    packet << uint8_t(buttons[i].type);
                    packet << uint8_t(buttons[i].misc);
                }
            }

            // Cata: Action footer after all buttons, not before them like in Wotlk and Mop
            if (m_protocol.expansion == WoW::Expansion::_Cata)
                packet << action;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }

    private:
        [[nodiscard]] uint8_t getWireCount() const noexcept
        {
            if (m_protocol.isMainlineAtLeast(WoW::Expansion::_Dragonflight))
                return 180;

            if (m_protocol.isMainlineAtLeast(WoW::Expansion::_Mop))
                return 132;

            if (m_protocol.expansion == WoW::Expansion::_Cata)
                return 144;

            if (m_protocol.expansion == WoW::Expansion::_WotLK)
                return 136;

            if (m_protocol.expansion == WoW::Expansion::_TBC)
                return 132;

            return 120; // Classic
        }

        [[nodiscard]] size_t getPacketSize() const noexcept
        {
            const uint8_t count = getWireCount();

            if (m_protocol.isMainlineAtLeast(WoW::Expansion::_WoD))
                return (count * 8) + 1;

            if (m_protocol.expansion == WoW::Expansion::_Mop)
                return (count * 9) + 1;

            if (m_protocol.isMainlineAtLeast(WoW::Expansion::_WotLK))
                return (count * 4) + 1;

            return count * 4; // Classic & TBC
        }
    };
}
