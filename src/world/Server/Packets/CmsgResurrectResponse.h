/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WoWGuid.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgResurrectResponse : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t status;

        CmsgResurrectResponse() : CmsgResurrectResponse(0, 0)
        {
        }

        CmsgResurrectResponse(uint64_t guid, uint8_t status) :
            ManagedPacket(CMSG_RESURRECT_RESPONSE, 9),
            guid(guid),
            status(status)
        {
        }

        bool deserialise(WorldPacket& packet) override
        {
            if (packet.remaining() < expectedSize())
                return false;

            return internalDeserialise(packet);
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
                return m_minimum_size;
            else if (m_protocol.isMop())
                return 2; // uint8 status + packed guid mask byte
            return 6; // 6.x and later: the two mask bytes of the packed 128 bit guid and the 32 bit response
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                WoWGuid128 resurrecter;
                packet >> resurrecter;
                guid = WoWGuid::fromGuid128(resurrecter);
                uint32_t response = 0;
                packet >> response;
                status = static_cast<uint8_t>(response);
                return !packet.hadReadFailure();
            }

            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid >> status;
                guid.init(unpackedGuid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> status;

                guid[3] = packet.readBit();
                guid[0] = packet.readBit();
                guid[6] = packet.readBit();
                guid[4] = packet.readBit();
                guid[5] = packet.readBit();
                guid[2] = packet.readBit();
                guid[1] = packet.readBit();
                guid[7] = packet.readBit();

                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[5]);
                return true;
            }

            return false;
        }
    };
}
