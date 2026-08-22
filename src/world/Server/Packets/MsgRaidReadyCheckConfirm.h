/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class MsgRaidReadyCheckConfirm : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint8_t isReady;

        MsgRaidReadyCheckConfirm() : MsgRaidReadyCheckConfirm(0, 0)
        {
        }

        MsgRaidReadyCheckConfirm(uint64_t guid, uint8_t isReady) :
            ManagedPacket(MSG_RAID_READY_CHECK_CONFIRM, 9),
            guid(guid),
            isReady(isReady)
        {
        }

    protected:
        size_t expectedSize() const override { return 9; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << guid << isReady;
                return true;
            }

            return false;
        }
    };
}
