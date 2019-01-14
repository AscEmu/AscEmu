/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING != Mop
            packet >> createStruct.name >> createStruct._race >> createStruct._class >>
                createStruct.gender >> createStruct.skin >> createStruct.face >> createStruct.hairStyle >>
                createStruct.hairColor >> createStruct.facialHair >> createStruct.outfitId;
#else
            packet >> createStruct.outfitId >> createStruct.hairStyle >> createStruct._class >>
                createStruct.skin >> createStruct.face >> createStruct._race >> createStruct.facialHair >>
                createStruct.gender >> createStruct.hairColor;

            const auto nameLength = packet.readBits(6);
            uint8_t unknown = packet.readBit();
            createStruct.name = packet.ReadString(nameLength);

            if (unknown)
                packet.read<uint32_t>();

            packet.rpos(0);
#endif

            return true;
        }
    };
}}
