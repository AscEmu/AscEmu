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
    class SmsgPetNameQuery : public ManagedPacket
    {
    public:
        uint32_t petNumber;
        std::string name;
        uint32_t timeStamp;
        uint8_t unknown;

        SmsgPetNameQuery() : SmsgPetNameQuery(0, "", 0, 0)
        {
        }

        SmsgPetNameQuery(uint32_t petNumber, std::string name, uint32_t timeStamp, uint8_t unknown) :
            ManagedPacket(SMSG_PET_NAME_QUERY_RESPONSE, 9 + name.size() + 1),
            petNumber(petNumber),
            name(name),
            timeStamp(timeStamp),
            unknown(unknown)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << petNumber << name << timeStamp << unknown;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
