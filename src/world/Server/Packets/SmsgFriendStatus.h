/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "Units/Players/PlayerDefines.hpp"

//\todo send friend note based on status.
namespace AscEmu { namespace Packets
{
    class SmsgFriendStatus : public ManagedPacket
    {
    public:
        uint8_t status;
        uint64_t guid;
        std::string note;
        uint8_t online;
        uint32_t areaId;
        uint32_t level;
        uint32_t _class;

        SmsgFriendStatus() : SmsgFriendStatus(0, 0, "", 0, 0, 0, 0)
        {
        }

        SmsgFriendStatus(uint8_t status, uint64_t guid = 0, std::string note = "", uint8_t online = 0, uint32_t areaId = 0,
            uint32_t level = 0, uint32_t _class = 0) :
            ManagedPacket(SMSG_FRIEND_STATUS, 22 + note.size() + 1),
            status(status),
            guid(guid),
            note(note),
            online(online),
            areaId(areaId),
            level(level),
            _class(_class)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << status;

            if (guid != 0)
                packet << guid;

            if (status == FRIEND_ADDED_OFFLINE || status == FRIEND_ADDED_ONLINE)
                packet << note;

            if (status == FRIEND_ONLINE || status == FRIEND_ADDED_ONLINE)
                packet << online << areaId << level << _class;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
