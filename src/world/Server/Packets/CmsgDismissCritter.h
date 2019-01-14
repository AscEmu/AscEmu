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
    class CmsgDismissCritter : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid guid;

        CmsgDismissCritter() : CmsgDismissCritter(0)
        {
        }

        CmsgDismissCritter(uint64_t guid) :
            ManagedPacket(CMSG_DISMISS_CRITTER, 8),
            guid(guid)
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
            packet >> unpacked_guid;
            guid.Init(unpacked_guid);
            return true;
        }
#endif
    };
}}
