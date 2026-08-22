/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgLfgSetBootVote : public ManagedPacket
    {
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
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_WotLK)
                return false;

            packet >> voteFor;
            return true;
        }
    };
}
