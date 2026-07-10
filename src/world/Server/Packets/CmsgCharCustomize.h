/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgCharCustomize : public ManagedPacket
    {
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
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet >> guid >> createStruct.name >> createStruct.gender >> createStruct.skin >> createStruct.hairColor >>
                    createStruct.hairStyle >> createStruct.facialHair >> createStruct.face;

                return true;
            }

            return false;
        }
    };
}
