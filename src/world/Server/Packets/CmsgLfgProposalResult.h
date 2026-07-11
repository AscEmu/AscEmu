/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgLfgProposalResult : public ManagedPacket
    {
    public:
        uint32_t lfgGroupId;
        bool accept;

        CmsgLfgProposalResult() : CmsgLfgProposalResult(0, false)
        {
        }

        CmsgLfgProposalResult(uint32_t lfgGroupId, bool accept) :
            ManagedPacket(CMSG_LFG_PROPOSAL_RESULT, 5),
            lfgGroupId(lfgGroupId),
            accept(accept)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> lfgGroupId >> accept;
            return true;
        }
    };
}
