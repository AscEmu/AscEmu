/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgCharFactionChange : public ManagedPacket
    {
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
            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid >> charCreate.name >> charCreate.gender >> charCreate.skin >>
                    charCreate.hairColor >> charCreate.hairStyle >> charCreate.facialHair >>
                    charCreate.face >> charCreate._race;

                guid.init(unpackedGuid);
                return true;
            }

            return false;
        }
    };
}
