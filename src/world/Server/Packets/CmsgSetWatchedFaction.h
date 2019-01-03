/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgSetWatchedFaction : public ManagedPacket
    {
    public:
        uint32_t factionId;

        CmsgSetWatchedFaction() : CmsgSetWatchedFaction(0)
        {
        }

        CmsgSetWatchedFaction(uint32_t factionId) :
            ManagedPacket(CMSG_SET_WATCHED_FACTION, 0),
            factionId(factionId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << factionId;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> factionId;
            return true;
        }
    };
}}
