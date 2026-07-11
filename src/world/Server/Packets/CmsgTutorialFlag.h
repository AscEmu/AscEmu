/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgTutorialFlag : public ManagedPacket
    {
    public:
        uint32_t flag;

        CmsgTutorialFlag() : CmsgTutorialFlag(0)
        {
        }

        CmsgTutorialFlag(uint32_t flag) :
            ManagedPacket(CMSG_TUTORIAL_FLAG, 1),
            flag(flag)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> flag;
            return true;
        }
    };
}
