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
    class CmsgUpdateAccountData : public ManagedPacket
    {
    public:
        uint32_t uiId;
        uint32_t uiTimestamp;
        uint32_t uiDecompressedSize;

        CmsgUpdateAccountData() : CmsgUpdateAccountData(0, 0, 0)
        {
        }

        CmsgUpdateAccountData(uint32_t uiId, uint32_t uiTimestamp, uint32_t uiDecompressedSize) :
            ManagedPacket(CMSG_UPDATE_ACCOUNT_DATA, 4 + 4),
            uiId(uiId),
            uiTimestamp(uiTimestamp),
            uiDecompressedSize(uiDecompressedSize)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> uiId;
#if VERSION_STRING >= Cata
            packet >> uiTimestamp;
#endif
            packet >> uiDecompressedSize;
            if (uiDecompressedSize >= 0xFFFF)
            {
                packet.rfinish();
                return false;
            }

            return true;
        }
    };
}}
