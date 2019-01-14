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
    class CmsgPlayedTime : public ManagedPacket
    {
    public:
        uint8_t displayInUi;

        CmsgPlayedTime() : CmsgPlayedTime(0)
        {
        }

        CmsgPlayedTime(uint8_t displayInUi) :
            ManagedPacket(CMSG_PLAYED_TIME, 0),
            displayInUi(displayInUi)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << displayInUi;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING > TBC
            packet >> displayInUi;
#endif
            return true;
        }
    };
}}
