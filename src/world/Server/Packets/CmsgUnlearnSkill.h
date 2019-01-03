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
    class CmsgUnlearnSkill : public ManagedPacket
    {
    public:
        uint32_t skillLineId;

        CmsgUnlearnSkill() : CmsgUnlearnSkill(0)
        {
        }

        CmsgUnlearnSkill(uint32_t skillLineId) :
            ManagedPacket(CMSG_UNLEARN_SKILL, 4),
            skillLineId(skillLineId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> skillLineId;
            return true;
        }
    };
}}
