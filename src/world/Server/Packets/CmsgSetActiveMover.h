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
    class CmsgSetActiveMover : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgSetActiveMover() : CmsgSetActiveMover(0)
        {
        }

        CmsgSetActiveMover(uint64_t guid) :
            ManagedPacket(CMSG_SET_ACTIVE_MOVER, 0),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid.GetOldGuid();
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid;
            guid = WoWGuid(unpacked_guid);
            return true;
        }
    };
}}
