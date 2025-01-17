/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgBankerActivate : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgBankerActivate() : CmsgBankerActivate(0)
        {
        }

        CmsgBankerActivate(uint64_t guid) :
            ManagedPacket(CMSG_BANKER_ACTIVATE, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.Init(unpackedGuid);
            return true;

            // uint64_t guid;

            // guid[4] = recvData.readBit();
            // guid[5] = recvData.readBit();
            // guid[0] = recvData.readBit();
            // guid[6] = recvData.readBit();
            // guid[1] = recvData.readBit();
            // guid[2] = recvData.readBit();
            // guid[7] = recvData.readBit();
            // guid[3] = recvData.readBit();

            // recvData.ReadByteSeq(guid[1]);
            // recvData.ReadByteSeq(guid[7]);
            // recvData.ReadByteSeq(guid[2]);
            // recvData.ReadByteSeq(guid[5]);
            // recvData.ReadByteSeq(guid[6]);
            // recvData.ReadByteSeq(guid[3]);
            // recvData.ReadByteSeq(guid[0]);
            // recvData.ReadByteSeq(guid[4]);
        }
    };
}
