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
    class SmsgTextEmote : public ManagedPacket
    {
    public:
        uint32_t nameLength;
        std::string name;
        uint32_t textEmote;
        uint64_t guid;
        uint32_t unk;

        SmsgTextEmote() : SmsgTextEmote(0, "", 0, 0, 0)
        {
        }

        SmsgTextEmote(uint32_t nameLength, std::string name, uint32_t textEmote, uint64_t guid, uint32_t unk) :
            ManagedPacket(SMSG_TEXT_EMOTE, 28 + nameLength),
            nameLength(nameLength),
            name(name),
            textEmote(textEmote),
            guid(guid),
            unk(unk)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << textEmote << unk << nameLength;
            if (nameLength > 1)
                packet.append(name.c_str(), nameLength);
            else
                packet << uint8_t(0);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
