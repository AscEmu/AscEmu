/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <string>

namespace AscEmu::Packets
{
    class SmsgGroupDecline : public ManagedPacket
    {
    public:
        std::string name;

        SmsgGroupDecline(std::string name) :
            ManagedPacket(SMSG_GROUP_DECLINE, 0),
            name(name)
        {
        }

    protected:
        size_t expectedSize() const override { return name.size(); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet << name;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
