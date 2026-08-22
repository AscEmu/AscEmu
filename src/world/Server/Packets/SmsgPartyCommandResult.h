/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class SmsgPartyCommandResult : public ManagedPacket
    {
    public:
        uint32_t unknown;
        std::string memberName;
        uint32_t error;
        uint32_t val;

        SmsgPartyCommandResult() : SmsgPartyCommandResult(0, "", 0, 0)
        {
        }

        SmsgPartyCommandResult(uint32_t unknown, std::string memberName, uint32_t error, uint32_t val = 0) :
            ManagedPacket(SMSG_PARTY_COMMAND_RESULT, 12),
            unknown(unknown),
            memberName(memberName),
            error(error),
            val(val)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << unknown;
            if (memberName.empty())
                packet << uint8_t(0);
            else
                packet << memberName.c_str();

            packet << error;

            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet << val;
            }

            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet << uint64_t(0);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
