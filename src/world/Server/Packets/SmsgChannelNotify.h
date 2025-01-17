/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"
#include "Chat/ChannelDefines.hpp"

namespace AscEmu::Packets
{
    class SmsgChannelNotify : public ManagedPacket
    {
    public:
        uint8_t flag;
        utf8_string channelName;
        uint64_t guid;

        uint8_t extraFlag;
        uint32_t channelId;
        uint8_t extraFlags2;

        std::string playerName;
        uint64_t sourceGuid;

        SmsgChannelNotify() : SmsgChannelNotify(0, "", 0, 0, 0)
        {
        }

        SmsgChannelNotify(uint8_t flag, std::string channelName, uint64_t guid = 0, uint8_t extraFlag = 0, uint32_t channelId = 0, uint8_t extraFlags2 = 0, std::string playerName = std::string(), uint64_t sourceGuid = 0) :
            ManagedPacket(SMSG_CHANNEL_NOTIFY, 0),
            flag(flag), channelName(std::move(channelName)), guid(guid), extraFlag(extraFlag), channelId(channelId), extraFlags2(extraFlags2),
            playerName(std::move(playerName)), sourceGuid(sourceGuid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 1 + channelName.size() + 1 + 8;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << flag;
            packet << channelName;

            switch (flag)
            {
                case CHANNEL_NOTIFY_FLAG_JOINED:
                case CHANNEL_NOTIFY_FLAG_LEFT:
                case CHANNEL_NOTIFY_FLAG_SETPASS:
                case CHANNEL_NOTIFY_FLAG_CHGOWNER:
                case CHANNEL_NOTIFY_FLAG_ENABLE_ANN:
                case CHANNEL_NOTIFY_FLAG_DISABLE_ANN:
                case CHANNEL_NOTIFY_FLAG_MODERATED:
                case CHANNEL_NOTIFY_FLAG_UNMODERATED:
                case CHANNEL_NOTIFY_FLAG_ALREADY_ON:
                case CHANNEL_NOTIFY_FLAG_INVITED:
                    packet << guid;
                break;
                case CHANNEL_NOTIFY_FLAG_NOT_ON_2:
                case CHANNEL_NOTIFY_FLAG_WHO_OWNER:
                case CHANNEL_NOTIFY_FLAG_NOT_BANNED:
                case CHANNEL_NOTIFY_FLAG_YOU_INVITED:
                case CHANNEL_NOTIFY_FLAG_INVITED_BANNED:
                    packet << playerName;
                break;
                case CHANNEL_NOTIFY_FLAG_YOUJOINED:
#if VERSION_STRING == Classic
                    packet << extraFlag << uint32_t(0);
#else
                    packet << extraFlag << channelId << uint32_t(0);
#endif
                break;
#if VERSION_STRING >= TBC
                case CHANNEL_NOTIFY_FLAG_YOULEFT:
                    packet << channelId << uint8_t(channelId != 0);
                break;
#endif
                case CHANNEL_NOTIFY_FLAG_MODE_CHG:
                    packet << guid << extraFlag << extraFlags2;
                break;
                case CHANNEL_NOTIFY_FLAG_KICKED:
                case CHANNEL_NOTIFY_FLAG_BANNED:
                case CHANNEL_NOTIFY_FLAG_UNBANNED:
                    packet << guid << sourceGuid;
                break;
                default:
                break;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
