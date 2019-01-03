/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgTextEmote : public ManagedPacket
    {
    public:
        uint32_t text_emote;
        uint32_t unk; //cata: numEmote
        uint64_t guid;

        CmsgTextEmote() : CmsgTextEmote(0, 0, 0)
        {
        }

        CmsgTextEmote(uint32_t text_emote, uint32_t unk, uint64_t guid) :
            ManagedPacket(CMSG_TEXT_EMOTE, 16),
            text_emote(text_emote),
            unk(unk),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << text_emote << unk << guid;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> text_emote >> unk >> guid;
            return true;
        }
    };
}}
