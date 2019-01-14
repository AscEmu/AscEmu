/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgEmote : public ManagedPacket
    {
    public:
        uint32_t textEmote;
        uint64_t guid;

        SmsgEmote() : SmsgEmote(0, 0)
        {
        }

        SmsgEmote(uint32_t textEmote, uint64_t guid) :
            ManagedPacket(SMSG_EMOTE, 0),
            textEmote(textEmote),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << textEmote << guid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
