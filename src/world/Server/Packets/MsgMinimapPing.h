/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class MsgMinimapPing : public ManagedPacket
    {
    public:
        uint64_t guid;
        float posX;
        float posY;

        MsgMinimapPing() : MsgMinimapPing(0, 0, 0)
        {
        }

        MsgMinimapPing(uint64_t guid, float posX, float posY) :
            ManagedPacket(MSG_MINIMAP_PING, 8),
            guid(guid),
            posX(posX),
            posY(posY)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << posX << posY;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> posX >> posY;
            return true;
        }
    };
}}
