/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgCharFactionChange : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid guid;
        CharCreate charCreate;

        CmsgCharFactionChange() : CmsgCharFactionChange(0, CharCreate())
        {
        }

        CmsgCharFactionChange(uint64_t guid, CharCreate charCreate) :
            ManagedPacket(CMSG_CHAR_FACTION_CHANGE, 16),
            guid(guid),
            charCreate(charCreate)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid >> charCreate.name >> charCreate.gender >> charCreate.skin >>
                charCreate.hairColor >> charCreate.hairStyle >> charCreate.facialHair >>
                charCreate.face >> charCreate._race;

            guid.Init(unpackedGuid);
            return true;
        }
#endif
    };
}}
