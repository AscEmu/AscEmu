/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgPetRename : public ManagedPacket
    {
    public:
        WoWGuid guid;
        std::string name;
        uint32_t petNumber = 0;      //Mop - identifies the pet instead of guid

        CmsgPetRename() : CmsgPetRename(0, "")
        {
        }

        CmsgPetRename(uint64_t guid, std::string name) :
            ManagedPacket(CMSG_PET_RENAME, 8),
            guid(guid),
            name(name)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpacked_guid;
                packet >> unpacked_guid >> name;
                guid.init(unpacked_guid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> petNumber;

                const bool hasName = !packet.readBit();
                const bool hasDeclinedNames = packet.readBit();

                uint32_t declinedNameLength[5] = {};
                if (hasDeclinedNames)
                {
                    for (uint8_t i = 0; i < 5; ++i)
                        declinedNameLength[i] = static_cast<uint32_t>(packet.readBits(7));
                }

                uint32_t nameLength = 0;
                if (hasName)
                    nameLength = static_cast<uint32_t>(packet.readBits(8));

                if (hasName)
                    name = packet.readString(nameLength);

                // declined (grammatical case) name forms are not supported - consumed only for wire alignment
                if (hasDeclinedNames)
                {
                    for (uint8_t i = 0; i < 5; ++i)
                        packet.readString(declinedNameLength[i]);
                }

                return true;
            }

            return false;
        }
    };
}
