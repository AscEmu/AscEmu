/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // whether the 10.x client shows the social contract on the glue screen
    class SmsgSocialContractRequestResponse : public ManagedPacket
    {
    public:
        bool showSocialContract;

        SmsgSocialContractRequestResponse() : SmsgSocialContractRequestResponse(false) {}

        explicit SmsgSocialContractRequestResponse(bool showSocialContract) :
            ManagedPacket(SMSG_SOCIAL_CONTRACT_REQUEST_RESPONSE, 1),
            showSocialContract(showSocialContract)
        {}

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isDragonflight())
                return false;

            packet.writeBit(showSocialContract);
            packet.flushBits();
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
