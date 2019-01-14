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
    class CmsgSetContactNotes : public ManagedPacket
    {
    public:
        uint64_t guid;
        std::string note;

        CmsgSetContactNotes() : CmsgSetContactNotes(0, "")
        {
        }

        CmsgSetContactNotes(uint64_t guid, std::string note) :
            ManagedPacket(CMSG_SET_CONTACT_NOTES, 4),
            guid(guid),
            note(note)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << note;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid >> note;
            return true;
        }
    };
}}
