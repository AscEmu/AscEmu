/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSetPrimaryTalentTree : public ManagedPacket
    {
    public:
        uint32_t specializationTabId = 0;

        CmsgSetPrimaryTalentTree() : ManagedPacket(CMSG_SET_PRIMARY_TALENT_TREE, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
                return false;

            packet >> specializationTabId;
            return true;
        }
    };
}
