/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgJoinChannel : public ManagedPacket
    {
    public:
        std::string channel_name;
        std::string password;
        uint32_t dbc_id;
        uint16_t unk;

        CmsgJoinChannel() : CmsgJoinChannel(0, 0)
        {
        }

        CmsgJoinChannel(uint16_t unk, uint32_t dbc_id, std::string channel_name = "", std::string password = "") :
            ManagedPacket(CMSG_JOIN_CHANNEL, 0),
            unk(unk),
            dbc_id(dbc_id),
            channel_name(channel_name),
            password(password)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << dbc_id << unk << channel_name << password;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> dbc_id >> unk >> channel_name >> password;
            return true;
        }
    };
}}
