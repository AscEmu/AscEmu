/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgSetActionButton : public ManagedPacket
    {
    public:
        uint8_t button;
#if VERSION_STRING <= Cata
        uint8_t misc;
        uint8_t type;
        uint16_t action;
#else
        uint32_t misc;
        uint32_t type;
        uint32_t action;
#endif

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
#if VERSION_STRING <= Cata
            packet >> button >> action >> misc >> type;
#else

            packet >> button;
            WoWGuid buttonStream;
	
            buttonStream[7] = packet.readBit();
            buttonStream[0] = packet.readBit();
            buttonStream[5] = packet.readBit();
            buttonStream[2] = packet.readBit();
            buttonStream[1] = packet.readBit();
            buttonStream[6] = packet.readBit();
            buttonStream[3] = packet.readBit();
            buttonStream[4] = packet.readBit();

            packet.ReadByteSeq(buttonStream[6]);
            packet.ReadByteSeq(buttonStream[7]);
            packet.ReadByteSeq(buttonStream[3]);
            packet.ReadByteSeq(buttonStream[5]);
            packet.ReadByteSeq(buttonStream[2]);
            packet.ReadByteSeq(buttonStream[1]);
            packet.ReadByteSeq(buttonStream[4]);
            packet.ReadByteSeq(buttonStream[0]);

            action = buttonStream.getGuidLowPart();
            type = buttonStream.getGuidHighPart();
            misc = 0; // not sent in packet

#endif
            return true;
        }
    };
}
