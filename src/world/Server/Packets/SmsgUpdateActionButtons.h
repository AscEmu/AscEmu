/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include <array>
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgUpdateActionButtons : public ManagedPacket
    {
    public:
        std::array<ActionButton, PLAYER_ACTION_BUTTON_COUNT> buttons{};
        uint8_t action = 0;

        SmsgUpdateActionButtons() : SmsgUpdateActionButtons({}, 0)
        {
        }

        SmsgUpdateActionButtons(std::array<ActionButton, PLAYER_ACTION_BUTTON_COUNT> buttons, uint8_t action) :
            ManagedPacket(SMSG_UPDATE_ACTION_BUTTONS, PLAYER_ACTION_BUTTON_SIZE + 1),
            buttons(std::move(buttons)),
            action(action)
        {
        }

    protected:
        size_t expectedSize() const override { return buttons.size() * sizeof(ActionButton) + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            // ActionButton itself has a different member layout for Mop (no Misc field, Type is
            // 4 bytes instead of 1), so the two formats below cannot be unified behind a single
            // runtime check - the Mop branch would not compile against the pre-Mop struct layout
            // and vice versa.
#if VERSION_STRING < Mop
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
                packet << uint16_t(buttons[i].Action);

                if (miscBeforeType)
                {
                    // Since Wotlk misc needs to be sent before type
                    packet << buttons[i].Misc;
                    packet << buttons[i].Type;
                }
                else
                {
                    packet << buttons[i].Type;
                    packet << buttons[i].Misc;
                }
            }

            if (m_protocol.expansion == WoW::Expansion::_Cata)
                packet << action;
#else
            if (!m_protocol.isMop())
                return false;

            static_assert(sizeof(ActionButton) == 8);

            uint8_t rawButtons[PLAYER_ACTION_BUTTON_COUNT][8] = {};
            auto* packedButtons = reinterpret_cast<ActionButton*>(rawButtons);

            for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
            {
                packedButtons[i].Action = static_cast<uint32_t>(buttons[i].Action);
                packedButtons[i].Type = static_cast<uint32_t>(buttons[i].Type);
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
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
