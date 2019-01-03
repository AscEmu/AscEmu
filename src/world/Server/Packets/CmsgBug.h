/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgBug : public ManagedPacket
    {
    public:
        uint32_t suggestion;
        uint32_t contentLenght;
        std::string content;
        uint32_t typeLenght;
        std::string type;

        CmsgBug() : CmsgBug(0, 0, "", 0, "")
        {
        }

        CmsgBug(uint32_t suggestion, uint32_t contentLenght, std::string content, uint32_t typeLenght, std::string type) :
            ManagedPacket(CMSG_BUG, 4 + 4 + 1 + 4 + 1),
            suggestion(suggestion),
            contentLenght(contentLenght),
            content(content),
            typeLenght(typeLenght),
            type(type)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> suggestion >> contentLenght >> content >> typeLenght >> type;
            return true;
        }
    };
}}
