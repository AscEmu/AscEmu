/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgBinderConfirm : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t zoneId;

        SmsgBinderConfirm() : SmsgBinderConfirm(0, 0)
        {
        }

        SmsgBinderConfirm(uint64_t guid, uint32_t zoneId) :
            ManagedPacket(SMSG_BINDER_CONFIRM, 12),
            guid(guid),
            zoneId(zoneId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << zoneId;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
