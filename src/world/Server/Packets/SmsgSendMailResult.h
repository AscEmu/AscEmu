/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

    protected:

        size_t expectedSize() const override { return 12; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << messageId << result << error;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            return false;
        }
    };
}}
