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
    class CmsgEmote : public ManagedPacket
    {
    public:
        uint32_t emote;

        CmsgEmote() : CmsgEmote(0)
        {
        }

        CmsgEmote(uint32_t emote) :
            ManagedPacket(CMSG_EMOTE, 4),
            emote(emote)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& /*packet*/) override { return false; }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> emote;
            return false;
        }
    };
}}
