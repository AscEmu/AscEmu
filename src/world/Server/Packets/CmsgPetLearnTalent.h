/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgPetLearnTalent : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid guid;
        uint32_t talentId;
        uint32_t talentCol;

        CmsgPetLearnTalent() : CmsgPetLearnTalent(0, 0, 0)
        {
        }

        CmsgPetLearnTalent(uint64_t guid, uint32_t talentId, uint32_t talentCol) :
            ManagedPacket(CMSG_PET_LEARN_TALENT, 16),
            guid(guid),
            talentId(talentId),
            talentCol(talentCol)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid >> talentId >> talentCol;
            guid.Init(unpacked_guid);
            return true;
        }
#endif
    };
}}
