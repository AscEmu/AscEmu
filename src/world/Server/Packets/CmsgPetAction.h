/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

//\brief: This packet is wrong.
namespace AscEmu { namespace Packets
{
    class CmsgPetAction : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint16_t misc;
        uint16_t action;
        uint64_t targetguid;

        CmsgPetAction() : CmsgPetAction(0, 0, 0, 0)
        {
        }

        CmsgPetAction(uint64_t guid, uint16_t misc, uint16_t action, uint64_t targetguid) :
            ManagedPacket(CMSG_PET_ACTION, 20),
            guid(guid),
            misc(misc),
            action(action),
            targetguid(targetguid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid >> misc >> action >> targetguid;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}}
