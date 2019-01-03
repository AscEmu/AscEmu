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
    class MsgSaveGuildEmblem : public ManagedPacket
    {
    public:
        WoWGuid guid;
        EmblemInfo emblemInfo;

        uint32_t error;

        MsgSaveGuildEmblem() : MsgSaveGuildEmblem(0)
        {
        }

        MsgSaveGuildEmblem(uint32_t error) :
            ManagedPacket(MSG_SAVE_GUILD_EMBLEM, 8 + 4 + 4 + 4 + 4 + 4),
            error(error)
        {
        }

    protected:

        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << error;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.Init(unpackedGuid);

            emblemInfo.readEmblemInfoFromPacket(packet);
            return true;
        }
    };
}}
