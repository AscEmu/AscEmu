/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Utilities/utf8String.hpp"
#include "Utilities/Util.hpp"

#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class SmsgGroupInvite : public ManagedPacket
    {
        static const size_t PACKET_SIZE = sizeof(uint8_t) + 96;
    public:
        uint8_t failed;
        utf8_string name;
        WoWGuid inviterGuid;

        SmsgGroupInvite() : SmsgGroupInvite(0, "", 0)
        {
        }

        SmsgGroupInvite(uint8_t failed, std::string name, uint64_t inviterGuid) :
            ManagedPacket(SMSG_GROUP_INVITE, PACKET_SIZE),
            failed(failed),
            name(name),
            inviterGuid(inviterGuid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet.writeBit(0);

                packet.writeBit(inviterGuid[0]);
                packet.writeBit(inviterGuid[3]);
                packet.writeBit(inviterGuid[2]);

                packet.writeBit(failed);                   //not in group

                packet.writeBit(inviterGuid[6]);
                packet.writeBit(inviterGuid[5]);

                packet.writeBits(0, 9);

                packet.writeBit(inviterGuid[4]);

                packet.writeBits(strlen(name.c_str()), 7);

                packet.writeBits(0, 24);
                packet.writeBit(0);

                packet.writeBit(inviterGuid[1]);
                packet.writeBit(inviterGuid[7]);

                packet.flushBits();

                packet.writeByteSeq(inviterGuid[1]);
                packet.writeByteSeq(inviterGuid[4]);

                packet << int32_t(::Util::getMSTime());
                packet << int32_t(0);
                packet << int32_t(0);

                packet.writeByteSeq(inviterGuid[6]);
                packet.writeByteSeq(inviterGuid[0]);
                packet.writeByteSeq(inviterGuid[2]);
                packet.writeByteSeq(inviterGuid[3]);
                packet.writeByteSeq(inviterGuid[5]);
                packet.writeByteSeq(inviterGuid[7]);

                packet.writeString(name);

                packet << int32_t(0);
            }
            else
            {
                packet << failed << name;
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return true; }
    };
}
