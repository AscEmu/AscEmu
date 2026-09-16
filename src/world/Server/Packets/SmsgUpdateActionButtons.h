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
        uint8_t misc = 0;                                       // not sent to Mop clients
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
            ManagedPacket(SMSG_UPDATE_ACTION_BUTTONS, PLAYER_ACTION_BUTTON_SIZE + 1),
            buttons(std::move(buttons)),
            action(action)
        {
        }

    protected:
        size_t expectedSize() const override { return buttons.size() * 8 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // every button is sent as 8 bytes (action, type), split into bit and byte streams
                uint8_t rawButtons[PLAYER_ACTION_BUTTON_COUNT][8] = {};

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                {
                    std::memcpy(&rawButtons[i][0], &buttons[i].action, sizeof(uint32_t));
                    std::memcpy(&rawButtons[i][4], &buttons[i].type, sizeof(uint32_t));
                }

                // Bits
                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeBit(rawButtons[i][4]);

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeBit(rawButtons[i][5]);

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeBit(rawButtons[i][3]);

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeBit(rawButtons[i][1]);

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeBit(rawButtons[i][6]);

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeBit(rawButtons[i][7]);

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeBit(rawButtons[i][0]);

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeBit(rawButtons[i][2]);

                // Data
                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeByteSeq(rawButtons[i][0]);

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeByteSeq(rawButtons[i][1]);

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeByteSeq(rawButtons[i][4]);

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeByteSeq(rawButtons[i][6]);

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeByteSeq(rawButtons[i][7]);

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeByteSeq(rawButtons[i][2]);

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeByteSeq(rawButtons[i][5]);

                for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
                    packet.writeByteSeq(rawButtons[i][3]);

                packet << action;

                return true;
            }

            if (m_protocol.expansion == WoW::Expansion::_WotLK)
                packet << action;

            // Misc/Type order flips starting Wotlk; Classic/TBC send Type then Misc.
            const bool miscBeforeType = m_protocol.expansion >= WoW::Expansion::_WotLK;

            for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
            {
                // TODO: this needs investigation
                // action, as in spell id, can be and will be over uint16_t max (65535) on wotlk and cata
                // but if I send action in uint32_t, client ignores the button completely and leaves an empty button slot, or corrupts other slots as well
                // however casting the action to uint16_t seems to somehow work. I tested it with a spell id over 65535.
                // but this is not a solution and can cause undefined behaviour... (previously ActionButton::Action was stored in uint16_t)
                // I believe client accepts at most 4 bytes per button -Appled
                packet << uint16_t(buttons[i].action);

                if (miscBeforeType)
                {
                    // Since Wotlk misc needs to be sent before type
                    packet << uint8_t(buttons[i].misc);
                    packet << uint8_t(buttons[i].type);
                }
                else
                {
                    packet << uint8_t(buttons[i].type);
                    packet << uint8_t(buttons[i].misc);
                }
            }

            if (m_protocol.expansion == WoW::Expansion::_Cata)
                packet << action;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
