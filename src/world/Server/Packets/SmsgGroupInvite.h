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
        WoWGuid invitedGuid;
        std::string realmName;
        bool inGroup = false;

        SmsgGroupInvite() : SmsgGroupInvite(0, "", 0)
        {
        }

        SmsgGroupInvite(uint8_t failed, std::string name, uint64_t inviterGuid, uint64_t invitedGuid = 0, std::string realmName = "", bool inGroup = false) :
            ManagedPacket(SMSG_GROUP_INVITE, PACKET_SIZE),
            failed(failed),
            name(name),
            inviterGuid(inviterGuid),
            invitedGuid(invitedGuid),
            realmName(realmName),
            inGroup(inGroup)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.writeBits(static_cast<uint32_t>(realmName.size()), 8);
                packet.writeBits(0, 8);
                packet.writeBit(invitedGuid[2]);
                packet.writeBit(0);
                packet.writeBits(static_cast<uint32_t>(name.size()), 6);
                packet.writeBit(invitedGuid[7]);
                packet.writeBit(invitedGuid[5]);
                packet.writeBit(!inGroup);
                packet.writeBit(0);
                packet.writeBit(invitedGuid[1]);
                packet.writeBit(1);
                packet.writeBit(1);
                packet.writeBits(0, 22);
                packet.writeBit(invitedGuid[3]);
                packet.writeBit(invitedGuid[0]);
                packet.writeBit(invitedGuid[4]);
                packet.writeBit(invitedGuid[6]);
                packet.flushBits();

                packet.writeByteSeq(invitedGuid[6]);
                packet.writeString(realmName);
                packet.writeByteSeq(invitedGuid[7]);
                packet.writeByteSeq(invitedGuid[2]);
                packet.writeByteSeq(invitedGuid[0]);
                packet << uint64_t(0);
                packet << uint32_t(0);
                packet << uint32_t(0);
                packet.writeByteSeq(invitedGuid[1]);
                packet.writeByteSeq(invitedGuid[5]);
                packet.writeByteSeq(invitedGuid[4]);
                packet << int32_t(0);
                packet.writeString(name);
                packet.writeByteSeq(invitedGuid[3]);
                packet << uint32_t(0);

                return true;
            }
            else if (m_protocol.isCata())
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

                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << failed << name;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return true; }
    };
}
