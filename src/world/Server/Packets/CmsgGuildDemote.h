/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgGuildDemote : public ManagedPacket
    {
    public:
#if VERSION_STRING < Cata
        std::string name;

        CmsgGuildDemote() : CmsgGuildDemote("")
        {
        }

        CmsgGuildDemote(std::string name) :
            ManagedPacket(CMSG_GUILD_DEMOTE, 1),
            name(name)
        {
        }
#else
        ObjectGuid guid;

        CmsgGuildDemote() : CmsgGuildDemote(0)
        {
        }

        CmsgGuildDemote(ObjectGuid guid) :
            ManagedPacket(CMSG_GUILD_DEMOTE, 8),
            guid(guid)
        {
        }
#endif

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet >> name;
#else
            guid[7] = packet.readBit();
            guid[1] = packet.readBit();
            guid[5] = packet.readBit();
            guid[6] = packet.readBit();
            guid[2] = packet.readBit();
            guid[3] = packet.readBit();
            guid[0] = packet.readBit();
            guid[4] = packet.readBit();

            packet.ReadByteSeq(guid[1]);
            packet.ReadByteSeq(guid[2]);
            packet.ReadByteSeq(guid[7]);
            packet.ReadByteSeq(guid[5]);
            packet.ReadByteSeq(guid[6]);
            packet.ReadByteSeq(guid[0]);
            packet.ReadByteSeq(guid[4]);
            packet.ReadByteSeq(guid[3]);
#endif
            return true;
        }
    };
}}
