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
    class CmsgGroupChangeSubGroup : public ManagedPacket
    {
    public:
        std::string name;
        uint8_t subGroup;

        CmsgGroupChangeSubGroup() : CmsgGroupChangeSubGroup("", 0)
        {
        }

        CmsgGroupChangeSubGroup(std::string name, uint8_t subGroup) :
            ManagedPacket(CMSG_GROUP_CHANGE_SUB_GROUP, 0),
            name(name),
            subGroup(subGroup)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name >> subGroup;
            return true;
        }
    };
}}
