/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgSetActionButton : public ManagedPacket
    {
    public:
        uint8_t button;
        uint8_t misc;
        uint8_t type;
        uint16_t action;

        CmsgSetActionButton() : CmsgSetActionButton(0, 0, 0, 0)
        {
        }

        CmsgSetActionButton(uint8_t button, uint8_t misc, uint8_t type, uint16_t action) :
            ManagedPacket(CMSG_SET_ACTION_BUTTON, 0),
            button(button),
            misc(misc),
            type(type),
            action(action)
        {
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << button << action << misc << type;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> button >> action >> misc >> type;
            return true;
        }
    };
}}
