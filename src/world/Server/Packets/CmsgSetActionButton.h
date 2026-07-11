/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSetActionButton : public ManagedPacket
    {
    public:
        uint8_t button;
        uint32_t misc;
        uint32_t type;
        uint32_t action;

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

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint16_t action16;
                uint8_t misc8;
                uint8_t type8;
                packet >> button >> action16 >> misc8 >> type8;
                action = action16;
                misc = misc8;
                type = type8;
            }
            else // Mop
            {
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

                packet.readByteSeq(buttonStream[6]);
                packet.readByteSeq(buttonStream[7]);
                packet.readByteSeq(buttonStream[3]);
                packet.readByteSeq(buttonStream[5]);
                packet.readByteSeq(buttonStream[2]);
                packet.readByteSeq(buttonStream[1]);
                packet.readByteSeq(buttonStream[4]);
                packet.readByteSeq(buttonStream[0]);

                action = buttonStream.getGuidLowPart();
                type = buttonStream.getGuidHighPart();
                misc = 0; // not sent in packet
            }

            return true;
        }
    };
}
