/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Logging/Logger.hpp"

#include "zlib.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgUpdateAccountData : public ManagedPacket
    {
    public:
        uint32_t accountDataId = 0;
        const uint8_t* rawData = nullptr;
        uint32_t rawSize = 0;

        SmsgUpdateAccountData() : SmsgUpdateAccountData(0, nullptr, 0)
        {
        }

        SmsgUpdateAccountData(uint32_t accountDataId, const uint8_t* rawData, uint32_t rawSize) :
            ManagedPacket(SMSG_UPDATE_ACCOUNT_DATA, 0),
            accountDataId(accountDataId), rawData(rawData), rawSize(rawSize)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + rawSize;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << accountDataId;

            if (!rawData || rawSize == 0)
            {
                packet << uint32_t(0);
                return true;
            }

            packet << rawSize;

            if (rawSize > 200)
            {
                packet.resize(rawSize + 800);

                uLongf destSize;
                if (compress(packet.contents() + (sizeof(uint32_t) * 2), &destSize, rawData, rawSize) != Z_OK)
                {
                    sLogger.debug("CMSG_REQUEST_ACCOUNT_DATA: Error while compressing data");
                    return false;
                }

                packet.resize(destSize + 8);
            }
            else
            {
                packet.append(rawData, rawSize);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
