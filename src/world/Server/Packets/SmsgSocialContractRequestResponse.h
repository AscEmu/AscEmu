/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once
#include "ManagedPacket.h"
namespace AscEmu::Packets
{
    class SmsgSocialContractRequestResponse : public ManagedPacket
    {
    public:
        explicit SmsgSocialContractRequestResponse(bool showSocialContract = false) : ManagedPacket(SMSG_SOCIAL_CONTRACT_REQUEST_RESPONSE, 1), showSocialContract(showSocialContract) {}
    protected:
        size_t expectedSize() const override { return 1; }
        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;
            packet.writeBit(showSocialContract);
            packet.flushBits();
            return true;
        }
        bool internalDeserialise(WorldPacket&) override { return false; }
    private:
        bool showSocialContract;
    };
}
