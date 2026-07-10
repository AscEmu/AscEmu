/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgCharCreate : public ManagedPacket
    {
    public:
        CharCreate createStruct;

        CmsgCharCreate() : CmsgCharCreate(CharCreate())
        {
        }

        CmsgCharCreate(CharCreate createStruct) :
            ManagedPacket(CMSG_CHAR_CREATE, 10),
            createStruct(createStruct)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
            {
                packet >> createStruct.name >> createStruct._race >> createStruct._class >>
                    createStruct.gender >> createStruct.skin >> createStruct.face >> createStruct.hairStyle >>
                    createStruct.hairColor >> createStruct.facialHair >> createStruct.outfitId;
            }
            else // Mop
            {
                packet >> createStruct.outfitId >> createStruct.hairStyle >> createStruct._class >>
                    createStruct.skin >> createStruct.face >> createStruct._race >> createStruct.facialHair >>
                    createStruct.gender >> createStruct.hairColor;

                const auto nameLength = packet.readBits(6);
                uint8_t unknown = packet.readBit();
                createStruct.name = packet.readString(nameLength);

                if (unknown)
                    packet.read<uint32_t>();

                packet.rpos(0);
            }

            return true;
        }
    };
}
