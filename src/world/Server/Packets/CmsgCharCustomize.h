/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgCharCustomize : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid guid;
        CharCreate createStruct;

        CmsgCharCustomize() : CmsgCharCustomize(0, CharCreate())
        {
        }

        CmsgCharCustomize(uint64_t guid, CharCreate createStruct) :
            ManagedPacket(CMSG_CHAR_CUSTOMIZE, 10),
            guid(guid),
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
            packet >> guid >> createStruct.name >> createStruct.gender >> createStruct.skin >> createStruct.hairColor >> 
                createStruct.hairStyle >> createStruct.facialHair >> createStruct.face;

            return true;
        }
#endif
    };
}}
