/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <array>
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAuthChallenge : public ManagedPacket
    {
    public:
        uint32_t seed;

        // 6.2.4 and 7.3.5 clients: server challenge and the two encryption seeds of the instance connection
        std::array<uint8_t, 16> challenge{};
        std::array<uint8_t, 32> dosChallenge{};

        SmsgAuthChallenge() : SmsgAuthChallenge(0)
        {
        }

        SmsgAuthChallenge(uint32_t seed) :
            ManagedPacket(SMSG_AUTH_CHALLENGE, 37),
            seed(seed)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_WotLK)
            {
                packet << seed;

            }
            else if (m_protocol.expansion == WoW::Expansion::_WotLK)
            {
                packet << uint32_t(1) << seed << uint32_t(0xC0FFEEEE) << uint32_t(0x00BABE00) << uint32_t(0xDF1697E5) << uint32_t(0x1234ABCD);

            }
            else if (m_protocol.expansion == WoW::Expansion::_Cata)
            {
                for (int i = 0; i < 8; ++i)
                    packet << uint32_t(0);
    
                packet << seed << uint8_t(1);

            }
            else if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                packet << uint16_t(0);

                for (int i = 0; i < 8; ++i)
                    packet << uint32_t(0);

                packet << uint8_t(1) << seed;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                packet.append(dosChallenge.data(), dosChallenge.size());
                packet.append(challenge.data(), challenge.size());
                packet << uint8_t(1);                   // dos zero bits
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
