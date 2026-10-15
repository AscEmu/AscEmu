/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // the answer to a Battle.net rpc call of the world connection: status, method, object id 1, token, no data
    class SmsgBattlenetResponse : public ManagedPacket
    {
    public:
        // rpc call not implemented by this service
        static constexpr uint32_t StatusNotImplemented = 0xBC7;

        uint32_t status;
        uint64_t methodType;
        uint32_t token;

        SmsgBattlenetResponse() : SmsgBattlenetResponse(StatusNotImplemented, 0, 0) {}

        SmsgBattlenetResponse(uint32_t status, uint64_t methodType, uint32_t token) :
            ManagedPacket(SMSG_BATTLENET_RESPONSE, 28),
            status(status),
            methodType(methodType),
            token(token)
        {}

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isTheWarWithinOrLater())
                return false;

            packet << status;
            packet << methodType;
            packet << uint64_t(1);                  // object id
            packet << token;
            packet << uint32_t(0);                  // data size
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
