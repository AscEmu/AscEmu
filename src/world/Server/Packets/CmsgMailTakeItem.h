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
    class CmsgMailTakeItem : public ManagedPacket
    {
    public:
        uint64_t gobjGuid;
        uint32_t messageId;
        uint32_t lowGuid;

        CmsgMailTakeItem() : CmsgMailTakeItem(0, 0, 0)
        {
        }

        CmsgMailTakeItem(uint64_t gobjGuid, uint32_t messageId, uint32_t lowGuid) :
            ManagedPacket(CMSG_MAIL_TAKE_ITEM, 16),
            gobjGuid(gobjGuid),
            messageId(messageId),
            lowGuid(lowGuid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> gobjGuid >> messageId >> lowGuid;
            return true;
        }
    };
}}
