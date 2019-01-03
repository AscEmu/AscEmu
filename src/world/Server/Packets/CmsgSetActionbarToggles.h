/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgSetActionbarToggles : public ManagedPacket
    {
    public:
        uint8_t actionbarId;

        CmsgSetActionbarToggles() : CmsgSetActionbarToggles(0)
        {
        }

        CmsgSetActionbarToggles(uint8_t actionbarId) :
            ManagedPacket(CMSG_SET_ACTIONBAR_TOGGLES, 0),
            actionbarId(actionbarId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << actionbarId;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> actionbarId;
            return true;
        }
    };
}}
