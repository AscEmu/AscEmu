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
    class SmsgPetActionSound : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t sound;

        SmsgPetActionSound() : SmsgPetActionSound(0, 0)
        {
        }

        SmsgPetActionSound(uint64_t guid, uint32_t sound) :
            ManagedPacket(SMSG_PET_ACTION_SOUND, 12),
            guid(guid),
            sound(sound)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << sound;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
