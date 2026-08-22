/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgGroupRaidConvert : public ManagedPacket
    {
    public:
        bool toRaid = true;

        CmsgGroupRaidConvert() : ManagedPacket(CMSG_GROUP_RAID_CONVERT, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                uint8_t toRaidByte = 0;
                packet >> toRaidByte;
                toRaid = toRaidByte != 0;

                return true;
            }
            else
            {
                toRaid = true;

                return true;
            }
        }
    };
}
