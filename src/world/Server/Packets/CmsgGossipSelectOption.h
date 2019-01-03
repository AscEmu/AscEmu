/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgGossipSelectOption : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t gossip_id;
        uint32_t option;
        std::string input;

        CmsgGossipSelectOption() : CmsgGossipSelectOption(0, 0, 0, "")
        {
        }

        CmsgGossipSelectOption(uint64_t guid, uint32_t gossip_id, uint32_t option, std::string input) :
            ManagedPacket(CMSG_GOSSIP_SELECT_OPTION, 0),
            guid(guid),
            gossip_id(gossip_id),
            option(option),
            input(input)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << gossip_id << option;

            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid >> gossip_id >> option;
            guid.Init(unpackedGuid);

            if (packet.rpos() != packet.wpos())
                packet >> input;
            return true;
        }
    };
}}
