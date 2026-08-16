/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgVoidTransferResult : public ManagedPacket
    {
    public:
        uint32_t result;

        SmsgVoidTransferResult() : SmsgVoidTransferResult(0)
        {
        }

        SmsgVoidTransferResult(uint32_t result) :
            ManagedPacket(SMSG_VOID_TRANSFER_RESULT, 4),
            result(result)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet << result;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
