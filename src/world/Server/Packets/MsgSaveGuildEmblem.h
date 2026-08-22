/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

class EmblemInfo;

namespace AscEmu::Packets
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
            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet << error;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                guid.init(unpackedGuid);

                emblemInfo.readEmblemInfoFromPacket(packet);
                return true;
            }

            return false;
        }
    };
}
