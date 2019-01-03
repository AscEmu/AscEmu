/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgGossipPoi : public ManagedPacket
    {
    public:
        uint32_t flags;
        float posX;
        float posY;
        uint32_t icon;
        uint32_t data;
        std::string name;

        SmsgGossipPoi() : SmsgGossipPoi(0, 0, 0, 0, 0, "")
        {
        }

        SmsgGossipPoi(uint32_t flags, float posX, float posY, uint32_t icon, uint32_t data, std::string name) :
            ManagedPacket(SMSG_GOSSIP_POI, 0),
            flags(flags),
            posX(posX),
            posY(posY),
            icon(icon),
            data(data),
            name(std::move(name))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 4 + 4 + 4 + 4 + name.length();
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << flags << posX << posY << icon << data;
            if (name.length())
                packet.append(reinterpret_cast<const uint8_t*>(name.c_str()), name.length() + 1);
            else
                packet << uint8_t(0);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
