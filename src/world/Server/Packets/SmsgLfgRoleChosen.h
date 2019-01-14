/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgLfgRoleChosen : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint64_t guid;
        uint8_t isReady;
        uint32_t roles;

        SmsgLfgRoleChosen() : SmsgLfgRoleChosen(0, 0, 0)
        {
        }

        SmsgLfgRoleChosen(uint64_t guid, uint8_t isReady, uint32_t roles) :
            ManagedPacket(SMSG_LFG_ROLE_CHOSEN, 0),
            guid(guid),
            isReady(isReady),
            roles(roles)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 1 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << isReady << roles;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}}
