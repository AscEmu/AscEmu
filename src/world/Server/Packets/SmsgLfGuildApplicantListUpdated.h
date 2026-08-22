/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgLfGuildApplicantListUpdated : public ManagedPacket
    {
    public:
        SmsgLfGuildApplicantListUpdated() : ManagedPacket(SMSG_LF_GUILD_APPLICANT_LIST_UPDATED, 0)
        {
        }

    protected:
        size_t expectedSize() const override { return 0; }

        bool internalSerialise(WorldPacket& /*packet*/) override { return m_protocol.expansion >= WoW::Expansion::_Cata; }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
