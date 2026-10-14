/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Server/MirrorVars.hpp"

#include <utility>
#include <vector>

namespace AscEmu::Packets
{

    class SmsgMirrorVars : public ManagedPacket
    {
    public:
        explicit SmsgMirrorVars(std::vector<AscEmu::MirrorVar> variables = {}) :
            ManagedPacket(SMSG_MIRROR_VARS, 4), variables(std::move(variables)) {}

        std::vector<AscEmu::MirrorVar> variables;

    protected:
        size_t expectedSize() const override
        {
            size_t size = 4;
            for (const AscEmu::MirrorVar& variable : variables)
            {
                size += 7;
                if (!variable.name.empty())
                    size += variable.name.size() + 1;
                if (!variable.value.empty())
                    size += variable.value.size() + 1;
            }
            return size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << uint32_t(variables.size());
            for (const AscEmu::MirrorVar& variable : variables)
            {
                const uint32_t nameLength = variable.name.empty() ? 0U : static_cast<uint32_t>(variable.name.size() + 1U);
                const uint32_t valueLength = variable.value.empty() ? 0U : static_cast<uint32_t>(variable.value.size() + 1U);

                packet.writeBit(variable.update);
                packet.writeBits(nameLength, 24);
                packet.writeBits(valueLength, 24);
                packet.flushBits();

                if (nameLength != 0U)
                    packet << variable.name;
                if (valueLength != 0U)
                    packet << variable.value;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket&) override { return false; }
    };
}
