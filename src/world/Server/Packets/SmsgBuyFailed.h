/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgBuyFailed : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t itemId;
        uint8_t error;

        SmsgBuyFailed() : SmsgBuyFailed(0, 0, 0)
        {
        }

        SmsgBuyFailed(uint64_t guid, uint32_t itemId, uint8_t error) :
            ManagedPacket(SMSG_BUY_FAILED, 13),
            guid(guid),
            itemId(itemId),
            error(error)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << itemId << error;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
