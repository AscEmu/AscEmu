/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgCharCustomize : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint8_t result;
        uint64_t guid = 0;
        CharCreate charStruct = CharCreate();

        SmsgCharCustomize() : SmsgCharCustomize(0)
        {
        }

        SmsgCharCustomize(uint8_t result) :
            ManagedPacket(SMSG_CHAR_CUSTOMIZE, 1),
            result(result)
        {
        }

        SmsgCharCustomize(uint8_t result, uint64_t guid, CharCreate charStruct) :
            ManagedPacket(SMSG_CHAR_CUSTOMIZE, 9 + charStruct.name.size() + 1 + 7),
            result(result),
            guid(guid),
            charStruct(charStruct)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (result != 0)
            {
                packet << result;
            }
            else
            {
                packet << result << guid << charStruct.name << charStruct.gender << charStruct.skin <<
                    charStruct.face << charStruct.hairStyle << charStruct.hairColor << charStruct.facialHair;
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
#endif
    };
}}
