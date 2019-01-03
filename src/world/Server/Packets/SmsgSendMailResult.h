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
    class SmsgSendMailResult : public ManagedPacket
    {
    public:
        uint32_t messageId;
        uint32_t result;
        uint32_t error;

        // MAIL_ERR_BAG_FULL
        uint32_t inventoryError = 0;
        // MAIL_RES_ITEM_TAKEN
        uint32_t itemGuidLow = 0;
        uint32_t itemStackCount = 0;

        SmsgSendMailResult() : SmsgSendMailResult(0, 0, 0)
        {
        }

        SmsgSendMailResult(uint32_t messageId, uint32_t result, uint32_t error) :
            ManagedPacket(SMSG_SEND_MAIL_RESULT, 0),
            messageId(messageId),
            result(result),
            error(error)
        {
        }
        SmsgSendMailResult(uint32_t messageId, uint32_t result, uint32_t error, uint32_t inventoryError) :
            ManagedPacket(SMSG_SEND_MAIL_RESULT, 0),
            messageId(messageId),
            result(result),
            error(error),
            inventoryError(inventoryError)
        {
        }

        SmsgSendMailResult(uint32_t messageId, uint32_t result, uint32_t error, uint32_t itemGuidLow, uint32_t itemStackCount) :
            ManagedPacket(SMSG_SEND_MAIL_RESULT, 0),
            messageId(messageId),
            result(result),
            error(error),
            itemGuidLow(itemGuidLow),
            itemStackCount(itemStackCount)
        {
        }

    protected:

        size_t expectedSize() const override
        {
            if (error == 1)     // MAIL_ERR_BAG_FULL
                return 16;

            if (result == 2)    // MAIL_RES_ITEM_TAKEN
                return 20;

            return 12;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << messageId << result << error;

            if (error == 1)         // MAIL_ERR_BAG_FULL
                packet << inventoryError;
            else if (result == 2)   // MAIL_RES_ITEM_TAKEN
                packet << itemGuidLow << itemStackCount;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
