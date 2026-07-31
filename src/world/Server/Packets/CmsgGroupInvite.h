/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgGroupInvite : public ManagedPacket
    {
    public:
        std::string name;

        CmsgGroupInvite() : CmsgGroupInvite("")
        {
        }

        CmsgGroupInvite(std::string name) :
            ManagedPacket(CMSG_GROUP_INVITE, 1),
            name(name)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                WoWGuid unk_guid;

                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();

                unk_guid[2] = packet.readBit();
                unk_guid[7] = packet.readBit();

                uint8_t realm_name_length = static_cast<uint8_t>(packet.readBits(9));

                unk_guid[3] = packet.readBit();

                uint8_t member_name_length = static_cast<uint8_t>(packet.readBits(10));

                unk_guid[5] = packet.readBit();
                unk_guid[4] = packet.readBit();
                unk_guid[6] = packet.readBit();
                unk_guid[0] = packet.readBit();
                unk_guid[1] = packet.readBit();

                packet.readByteSeq(unk_guid[4]);
                packet.readByteSeq(unk_guid[7]);
                packet.readByteSeq(unk_guid[6]);

                name = packet.readString(member_name_length);
                std::string realm_name = packet.readString(realm_name_length);

                packet.readByteSeq(unk_guid[1]);
                packet.readByteSeq(unk_guid[0]);
                packet.readByteSeq(unk_guid[5]);
                packet.readByteSeq(unk_guid[3]);
                packet.readByteSeq(unk_guid[2]);
            }
            else
            {
                packet >> name;
            }
            return true;
        }
    };
}
