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
    class CmsgLfgSetBootVote : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        bool voteFor;

        CmsgLfgSetBootVote() : CmsgLfgSetBootVote(false)
        {
        }

        CmsgLfgSetBootVote(bool voteFor) :
            ManagedPacket(CMSG_LFG_SET_BOOT_VOTE, 1),
            voteFor(voteFor)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> voteFor;
            return true;
        }
#endif
    };
}}
