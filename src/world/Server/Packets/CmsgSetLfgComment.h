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
    class CmsgSetLfgComment : public ManagedPacket
    {
    public:
        std::string comment;

        CmsgSetLfgComment() : CmsgSetLfgComment("")
        {
        }

        CmsgSetLfgComment(std::string comment) :
            ManagedPacket(CMSG_SET_LFG_COMMENT, 0),
            comment(comment)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> comment;
            return true;
        }
    };
}}
