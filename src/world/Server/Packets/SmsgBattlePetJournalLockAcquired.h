/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "BattlePetPacketData.hpp"

namespace AscEmu::Packets
{
    // the journal may be changed by this session
    class SmsgBattlePetJournalLockAcquired : public ManagedPacket
    {
    public:
        SmsgBattlePetJournalLockAcquired() : ManagedPacket(SMSG_BATTLE_PET_JOURNAL_LOCK_ACQUIRED, 0)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override { return m_protocol.expansion >= WoW::Expansion::_Mop; }
        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
