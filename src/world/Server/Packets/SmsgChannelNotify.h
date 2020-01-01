/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgChannelNotify : public ManagedPacket
    {
    public:
        uint8_t flag;
        std::string channelName;
        uint64_t guid;

        uint8_t extraFlag;
        uint32_t channelId;
        uint8_t extraFlags2;

        SmsgChannelNotify() : SmsgChannelNotify(0, "", 0, 0, 0)
        {
        }

        SmsgChannelNotify(uint8_t flag, std::string channelName, uint64_t guid = 0, uint8_t extraFlag = 0, uint32_t channelId = 0, uint8_t extraFlags2 = 0) :
            ManagedPacket(SMSG_CHANNEL_NOTIFY, 0),
            flag(flag), channelName(std::move(channelName)), guid(guid), extraFlag(extraFlag), channelId(channelId), extraFlags2(extraFlags2)
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
                case 0:     //joined
                case 1:     //left
                case 7:     //setpass
                case 8:     //owner
                case 9:     //noton2
                case 11:    //whoowner
                case 13:    //enableannounce
                case 14:    //disableannounce
                case 15:    //moderated
                case 16:    //unmoderated
                case 18:    //kicked
                case 20:    //banned
                case 21:    //unbanned
                case 23:    //alreadyon
                case 24:    //invited
                case 29:    //youinvited
                    packet << guid;
                break;
                case 2:     //youjoined
                    packet << extraFlag << channelId << uint32_t(0);
                break;
                case 3:     //youleft
                    packet << channelId << uint32_t(0) << uint8_t(0);
                break;
                case 12:    //mode
                    packet << guid << extraFlag << extraFlags2;
                break;
                default:
                break;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
