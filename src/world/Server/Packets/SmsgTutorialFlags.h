/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgTutorialFlags : public ManagedPacket
    {
    public:
        std::vector<uint32_t> tutorials;

        SmsgTutorialFlags() : SmsgTutorialFlags(std::vector<uint32_t>())
        {
        }

        SmsgTutorialFlags(std::vector<uint32_t> tutorials) :
            ManagedPacket(SMSG_TUTORIAL_FLAGS, 0),
            tutorials(std::move(tutorials))
        {
        }

    protected:
        size_t expectedSize() const override { return tutorials.size(); }

        bool internalSerialise(WorldPacket& packet) override
        {
            // All versions share same implementation
            for (auto tutorial_id : tutorials)
                packet << tutorial_id;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            while (packet.remaining() >= sizeof(uint32_t))
            {
                uint32_t tutorial_id;
                packet >> tutorial_id;
                tutorials.push_back(tutorial_id);
            }

            return true;
        }
    };
}}
