/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    // 11.x: values of client variables the server controls, the client takes only the names it registered as server switches
    class SmsgMirrorVars : public ManagedPacket
    {
    public:
        struct Variable
        {
            std::string name;
            std::string value;
            uint8_t updateType = 0;
        };

        std::vector<Variable> variables;

        SmsgMirrorVars() : SmsgMirrorVars(std::vector<Variable>{}) {}

        SmsgMirrorVars(std::vector<Variable> variables) :
            ManagedPacket(SMSG_MIRROR_VARS, 4),
            variables(std::move(variables))
        {}

    protected:
        size_t expectedSize() const override { return m_minimum_size + variables.size() * 64; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isTheWarWithinOrLater())
                return false;

            packet << static_cast<uint32_t>(variables.size());
            for (const auto& variable : variables)
            {
                // both lengths count the closing zero
                packet.writeBit(variable.updateType);
                packet.writeBits(static_cast<uint32_t>(variable.name.size() + 1), 24);
                packet.writeBits(static_cast<uint32_t>(variable.value.size() + 1), 24);
                packet.flushBits();

                // an empty string is only its length
                if (!variable.name.empty())
                    packet << variable.name;
                if (!variable.value.empty())
                    packet << variable.value;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
