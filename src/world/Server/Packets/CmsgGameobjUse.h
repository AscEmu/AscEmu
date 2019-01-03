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
    class CmsgGameobjUse : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgGameobjUse() : CmsgGameobjUse(0)
        {
        }

        CmsgGameobjUse(uint64_t guid) :
            ManagedPacket(CMSG_GAMEOBJ_USE, 0),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.Init(unpackedGuid);
            return true;
        }
    };
}}
