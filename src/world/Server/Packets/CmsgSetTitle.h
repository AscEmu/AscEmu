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
    class CmsgSetTitle : public ManagedPacket
    {
    public:
        uint32_t titleId;

        CmsgSetTitle() : CmsgSetTitle(0)
        {
        }

        CmsgSetTitle(uint32_t titleId) :
            ManagedPacket(CMSG_SET_TITLE, 4),
            titleId(titleId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> titleId;
            return true;
        }
    };
}}
