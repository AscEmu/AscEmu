/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgCharRename : public ManagedPacket
    {
    public:
        size_t size;
        uint8_t result;
        uint64_t guid;
        std::string name;

        SmsgCharRename() : SmsgCharRename(0, 0, 0, "")
        {
        }

        SmsgCharRename(size_t size, uint8_t result, uint64_t guid, std::string name) :
            ManagedPacket(SMSG_CHAR_RENAME, size),
            size(size),
            result(result),
            guid(guid),
            name(name)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << result << guid << name;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
