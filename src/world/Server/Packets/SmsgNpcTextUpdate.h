/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    struct NpcTextGossipEmote
    {
        uint32_t delay = 0;
        uint32_t emote = 0;
    };

    // One of the 8 gossip "pages" the client can pick from at random, weighted by probability.
    // text0/text1 already have the locale/empty-string fallback applied (whichever of the pair is
    // non-empty). language and emotes are only used pre-Mop.
    struct NpcTextGossipEntry
    {
        float probability = 1.0f;
        std::string text0;
        std::string text1;
        uint32_t language = 0;
        std::vector<NpcTextGossipEmote> emotes;
    };

    // Populated by WorldSession::handleNpcTextQueryOpcode, which owns the DB lookup
    // (sMySQLStore.getNpcGossipText/getLocalizedNpcGossipText) and the locale fallback rules.
    struct NpcTextUpdateInput
    {
        uint32_t textId = 0;
        bool found = false;           // whether a DB gossip entry existed for textId
        std::string fallbackGreeting; // localized "Hey there..." text, only used when !found
        std::vector<NpcTextGossipEntry> pages; // 8 entries, only used when found
    };

    class SmsgNpcTextUpdate : public ManagedPacket
    {
    public:
        NpcTextUpdateInput input;

        SmsgNpcTextUpdate() : SmsgNpcTextUpdate(NpcTextUpdateInput{})
        {
        }

        explicit SmsgNpcTextUpdate(NpcTextUpdateInput input) :
            ManagedPacket(SMSG_NPC_TEXT_UPDATE, 0),
            input(std::move(input))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 100;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << input.textId;

            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                if (input.found)
                {
                    for (const auto& page : input.pages)
                    {
                        packet << float(page.probability);
                        packet << page.text0;
                        packet << page.text1;
                        packet << page.language;

                        for (const auto& emote : page.emotes)
                        {
                            packet << uint32_t(emote.delay);
                            packet << uint32_t(emote.emote);
                        }
                    }
                }
                else
                {
                    for (uint8_t i = 0; i < 8; ++i)
                    {
                        packet << float(1.0f);              // Prob
                        packet << input.fallbackGreeting;
                        packet << input.fallbackGreeting;
                        packet << uint32_t(0x00);           // Language

                        // GOSSIP_EMOTE_COUNT fallback emotes, all zero
                        for (uint8_t e = 0; e < gossipEmoteCount; e++)
                        {
                            packet << uint32_t(0x00);       // Emote delay
                            packet << uint32_t(0x00);       // Emote
                        }
                    }
                }

                return true;
            }
            else if (m_protocol.isMop())
            {
                ByteBuffer buffer;
                if (input.found)
                {
                    for (const auto& page : input.pages)
                        buffer << float(page.probability); // probability

                    for (uint8_t i = 0; i < 8; ++i)
                        buffer << uint32_t(input.textId); // broadcast text id

                    for (const auto& page : input.pages)
                        buffer << page.text0;
                }
                else
                {
                    buffer << uint32_t(1); // unk

                    for (uint8_t i = 0; i < 7; ++i)
                        buffer << uint32_t(0); // probability

                    buffer << uint32_t(1);  //unk

                    for (uint8_t i = 0; i < 7; ++i)
                        buffer << uint32_t(0); // broadcast text id

                    buffer << input.fallbackGreeting;
                }

                packet << uint32_t(buffer.size());
                packet.append(buffer);
                packet.writeBit(1); // write cache?
                packet.flushBits();

                return true;
            }

            return false;
        }

    private:
        // Mirrors GOSSIP_EMOTE_COUNT (CreatureMacros.hpp) - the fixed shape of the synthetic
        // "no gossip text configured" fallback response, independent of any specific NPC's data.
        static constexpr uint8_t gossipEmoteCount = 3;

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
