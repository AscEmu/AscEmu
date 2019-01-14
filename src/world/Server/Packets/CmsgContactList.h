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
    class CmsgContactList : public ManagedPacket
    {
    public:
        uint32_t list_flag;

        CmsgContactList() : CmsgContactList(0)
        {
        }

        CmsgContactList(uint32_t list_flag) :
            ManagedPacket(CMSG_CONTACT_LIST, 4),
            list_flag(list_flag)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << list_flag;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> list_flag;
            return true;
        }
    };
}}
