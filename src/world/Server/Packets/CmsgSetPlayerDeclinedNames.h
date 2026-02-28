/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"
#include "WoWGuid.hpp"

namespace AscEmu::Packets
{
    class CmsgSetPlayerDeclinedNames : public ManagedPacket
    {
    public:
        uint64_t guid;
        std::vector<std::string> declinedNames;

        CmsgSetPlayerDeclinedNames() : CmsgSetPlayerDeclinedNames(0)
        {
        }

        CmsgSetPlayerDeclinedNames(uint64_t guid) :
            ManagedPacket(CMSG_SET_PLAYER_DECLINED_NAMES, 8),
            guid(guid)
        {
        }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            declinedNames.clear();
#if VERSION_STRING == Mop
            // MoP: from sgl-panda-core — 8 bits GUID mask, 5×7 bits lengths, 5 strings, then 8 bytes GUID (strings before GUID bytes).
            WoWGuid unpackedGuid;
            unpackedGuid[0] = packet.readBit();
            unpackedGuid[2] = packet.readBit();
            unpackedGuid[1] = packet.readBit();
            unpackedGuid[7] = packet.readBit();
            unpackedGuid[5] = packet.readBit();
            unpackedGuid[6] = packet.readBit();
            unpackedGuid[4] = packet.readBit();
            unpackedGuid[3] = packet.readBit();

            uint32_t nameLength[5];
            for (int i = 0; i < 5; ++i)
                nameLength[i] = packet.readBits(7);

            for (int i = 0; i < 5; ++i)
                declinedNames.push_back(nameLength[i] ? packet.ReadString(nameLength[i]) : std::string());

            packet.ReadByteSeq(unpackedGuid[0]);
            packet.ReadByteSeq(unpackedGuid[7]);
            packet.ReadByteSeq(unpackedGuid[3]);
            packet.ReadByteSeq(unpackedGuid[6]);
            packet.ReadByteSeq(unpackedGuid[4]);
            packet.ReadByteSeq(unpackedGuid[2]);
            packet.ReadByteSeq(unpackedGuid[1]);
            packet.ReadByteSeq(unpackedGuid[5]);

            guid = unpackedGuid.getRawGuid();
            return true;
#else
            packet >> guid;
            for (int i = 0; i < 5; ++i)
            {
                std::string name;
                packet >> name;
                declinedNames.push_back(name);
            }
            return true;
#endif
        }
    };
}
