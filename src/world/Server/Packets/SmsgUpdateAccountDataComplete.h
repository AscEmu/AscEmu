/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgUpdateAccountDataComplete : public ManagedPacket
    {
    public:
        uint32_t uiId;
        uint32_t unknown;

        SmsgUpdateAccountDataComplete() : SmsgUpdateAccountDataComplete(0, 0)
        {
        }

        SmsgUpdateAccountDataComplete(uint32_t uiId, uint32_t unknown) :
            ManagedPacket(SMSG_UPDATE_ACCOUNT_DATA_COMPLETE, 0),
            uiId(uiId),
            unknown(unknown)
        {
        }

    protected:
        size_t expectedSize() const override { return m_protocol.isForever() ? size_t(10) : size_t(8); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                // Observed Forever 69913 completion layout: packed empty
                // ObjectGuid, data type, result.
                packet << uint16_t(0);
                packet << uiId << unknown;
                return true;
            }

            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            packet << uiId << unknown;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
