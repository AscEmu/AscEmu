/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <algorithm>
#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class SmsgGmResponseReceived : public ManagedPacket
    {
    public:
        uint32_t ticketGuid;
        std::string message;
        std::string comment;

        SmsgGmResponseReceived() : SmsgGmResponseReceived(0, "", "")
        {
        }

        SmsgGmResponseReceived(uint32_t ticketGuid, std::string message, std::string comment) :
            ManagedPacket(SMSG_GMRESPONSE_RECEIVED, 0),
            ticketGuid(ticketGuid),
            message(message),
            comment(comment)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + 4 + message.size() + 1 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet << uint32_t(1);        // unk
                packet << ticketGuid;
                packet << message.c_str();

                size_t commentLength = comment.size();
                char const* commentChars = comment.c_str();

                for (int i = 0; i < 4; ++i)
                {
                    if (commentLength)
                    {
                        size_t writeLen = std::min<size_t>(commentLength, 3999);
                        packet.append(commentChars, writeLen);

                        commentLength -= writeLen;
                        commentChars += writeLen;
                    }

                    packet << uint8_t(0);
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
