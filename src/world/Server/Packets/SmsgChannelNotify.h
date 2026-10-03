/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Chat/ChannelDefines.hpp"

#include <cstdint>
#include <string>
#include <utility>

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

        bool serialiseLegion(WorldPacket& packet)
        {
            // joining and leaving a channel have their own packets
            if (flag == CHANNEL_NOTIFY_FLAG_YOUJOINED)
            {
                packet.initialize(SMSG_CHANNEL_NOTIFY_JOINED, 2 + 4 + 4 + 8 + channelName.size());
                packet.writeBits(static_cast<uint32_t>(channelName.length()), 7);
                packet.writeBits(0, 10);                                    // welcome message
                packet << uint32_t(extraFlag);
                packet << uint32_t(channelId);
                packet << uint64_t(0);                                      // instance
                packet.writeString(channelName);
                return true;
            }

            if (flag == CHANNEL_NOTIFY_FLAG_YOULEFT)
            {
                packet.initialize(SMSG_CHANNEL_NOTIFY_LEFT, 1 + 4 + channelName.size());
                packet.writeBits(static_cast<uint32_t>(channelName.length()), 7);
                packet.writeBit(false);                                     // suspended
                packet << uint32_t(channelId);
                packet.writeString(channelName);
                return true;
            }

            // player names are resolved by the client, the sender of a notice is the player it is about
            const WoWGuid128 senderGuid = WoWGuid(guid).toGuid128(m_protocol.realmId, 0);
            const WoWGuid128 targetGuid = WoWGuid(sourceGuid).toGuid128(m_protocol.realmId, 0);

            packet.writeBits(flag, 6);
            packet.writeBits(static_cast<uint32_t>(channelName.length()), 7);
            packet.writeBits(static_cast<uint32_t>(playerName.length()), 6);
            packet << senderGuid;
            packet << WoWGuid128();                                         // account of the sender
            packet << uint32_t(m_protocol.getVirtualRealmAddress());
            packet << targetGuid;
            packet << uint32_t(m_protocol.getVirtualRealmAddress());
            packet << uint32_t(channelId);

            if (flag == CHANNEL_NOTIFY_FLAG_MODE_CHG)
            {
                packet << uint8_t(extraFlag);
                packet << uint8_t(extraFlags2);
            }

            packet.writeString(channelName);
            packet.writeString(playerName);
            return true;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isLegion())
                return serialiseLegion(packet);

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
                    if (m_protocol.expansion == WoW::Expansion::_Classic)
                    {
                        packet << extraFlag << uint32_t(0);
                    }
                    else
                    {
                        packet << extraFlag << channelId << uint32_t(0);
                    }
                break;
                case CHANNEL_NOTIFY_FLAG_YOULEFT:
                    if (m_protocol.expansion >= WoW::Expansion::_TBC)
                    {
                        packet << channelId << uint8_t(channelId != 0);
                    }
                    break;
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
