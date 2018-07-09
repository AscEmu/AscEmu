/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

struct PlayerStablePetList
{
    uint32_t petNumber;
    uint32_t entry;
    uint32_t level;
    uint8_t stableState;
    std::string name;
};

namespace AscEmu { namespace Packets
{
    class MsgSaveGuildEmblem : public ManagedPacket
    {
    public:
        uint64_t guid;
        EmblemInfo emblemInfo;

        uint32_t error;

        MsgSaveGuildEmblem() : MsgSaveGuildEmblem(0)
        {
        }

        MsgSaveGuildEmblem(uint32_t error) :
            ManagedPacket(MSG_SAVE_GUILD_EMBLEM, 4),
            error(error)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << error;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid;
            emblemInfo.readEmblemInfoFromPacket(packet);
            return true;
        }
    };
}}
