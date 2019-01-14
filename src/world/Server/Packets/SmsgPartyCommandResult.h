/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgPartyCommandResult : public ManagedPacket
    {
    public:
        uint32_t unknown;
        std::string memberName;
        uint32_t error;

        SmsgPartyCommandResult() : SmsgPartyCommandResult(0, "", 0)
        {
        }

        SmsgPartyCommandResult(uint32_t unknown, std::string memberName, uint32_t error) :
            ManagedPacket(SMSG_PARTY_COMMAND_RESULT, 12),
            unknown(unknown),
            memberName(memberName),
            error(error)
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

#if VERSION_STRING >= Cata
            packet << uint32_t(0) << uint32_t(0);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
