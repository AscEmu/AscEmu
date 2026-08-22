/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class CmsgGroupUninviteGuid : public ManagedPacket
    {
    public:
        WoWGuid guid;
        std::string reason;

        CmsgGroupUninviteGuid() : CmsgGroupUninviteGuid(0)
        {
        }

        CmsgGroupUninviteGuid(uint64_t guid) :
            ManagedPacket(CMSG_GROUP_UNINVITE_GUID, 8),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.readSkip<uint8_t>();

                guid[6] = packet.readBit();
                guid[4] = packet.readBit();
                guid[3] = packet.readBit();
                guid[2] = packet.readBit();
                guid[0] = packet.readBit();
                guid[1] = packet.readBit();
                guid[7] = packet.readBit();
                guid[5] = packet.readBit();

                const uint8_t reasonLength = static_cast<uint8_t>(packet.readBits(8));
                reason = packet.readString(reasonLength);

                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[0]);

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid >> reason;
                guid.init(unpackedGuid);

                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_WotLK)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                guid.init(unpackedGuid);

                return true;
            }

            return false;
        }
    };
}
