/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"


void WorldSession::HandleReportOpcode(WorldPacket& recv_data)
{
    LOG_DEBUG("WORLD: CMSG_REPORT");

    uint8_t spam_type;                                        // 0 - mail, 1 - chat
    uint64_t spammer_guid;
    uint32_t unk1 = 0;
    uint32_t unk2 = 0;
    uint32_t unk3 = 0;
    uint32_t unk4 = 0;

    std::string description;
    recv_data >> spam_type;                                 // unk 0x01 const, may be spam type (mail/chat)
    recv_data >> spammer_guid;                              // player guid

    switch (spam_type)
    {
        case 0:
        {
            recv_data >> unk1;                              // const 0
            recv_data >> unk2;                              // probably mail id
            recv_data >> unk3;                              // const 0

            LOG_DEBUG("REPORT SPAM: type %u, guid %u, unk1 %u, unk2 %u, unk3 %u", spam_type, Arcemu::Util::GUID_LOPART(spammer_guid), unk1, unk2, unk3);

        } break;
        case 1:
        {
            recv_data >> unk1;                              // probably language
            recv_data >> unk2;                              // message type?
            recv_data >> unk3;                              // probably channel id
            recv_data >> unk4;                              // unk random value
            recv_data >> description;                       // spam description string (messagetype, channel name, player name, message)

            LOG_DEBUG("REPORT SPAM: type %u, guid %u, unk1 %u, unk2 %u, unk3 %u, unk4 %u, message %s", spam_type, Arcemu::Util::GUID_LOPART(spammer_guid), unk1, unk2, unk3, unk4, description.c_str());

        } break;
    }

    // Complaint Received message
    WorldPacket data(SMSG_REPORT_RESULT, 1);
    data << uint8_t(0);     // 1 reset reported player 0 ignore
    data << uint8_t(0);

    SendPacket(&data);
}

void WorldSession::HandleReportPlayerOpcode(WorldPacket& recv_data)
{
    LOG_DEBUG("WORLD: CMSG_REPORT_PLAYER %u", (uint32_t)recv_data.size());

    uint8_t unk3 = 0;   // type
    uint8_t unk4 = 0;   // guid - 1
    uint32_t unk5 = 0;
    uint64_t unk6 = 0;
    uint32_t unk7 = 0;
    uint32_t unk8 = 0;

    std::string message;

    uint32_t length = recv_data.readBits(9);    // length * 2
    recv_data >> unk3;                          // type
    recv_data >> unk4;                          // guid - 1?
    message = recv_data.ReadString(length/2);   // message
    recv_data >> unk5;                          // unk
    recv_data >> unk6;                          // unk
    recv_data >> unk7;                          // unk
    recv_data >> unk8;                          // unk


    //LOG_DEBUG("unk1 %u unk2 %u unk3 %u unk4 %u message: %s unk5 %u, unk6 %u, unk7 %u, unk8 %u", unk1, length, unk3, unk4, message.c_str(), unk5, unk6, unk7, unk8);

    switch (unk3)
    {
        case 0:     // chat spamming
            LOG_DEBUG("Chat spamming report for guid: %u received.", unk4 + 1);
            break;
        case 2:     // cheat
            recv_data >> message;
            LOG_DEBUG("Cheat report for guid: %u received. Message %s", unk4 + 1, message.c_str());
            break;
        case 6:     // char name
            recv_data >> message;
            LOG_DEBUG("char name report for guid: %u received. Message %s", unk4 + 1, message.c_str());
            break;
        case 12:     // guild name
            recv_data >> message;
            LOG_DEBUG("guild name report for guid: %u received. Message %s", unk4 + 1, message.c_str());
            break;
        case 18:     // arena team name
            recv_data >> message;
            LOG_DEBUG("arena team name report for guid: %u received. Message %s", unk4 + 1, message.c_str());
            break;
        case 20:     // chat language
            LOG_DEBUG("Chat language report for guid: %u received.", unk4 + 1);
            break;
        default:
            LOG_DEBUG("type is %u", unk3);
            break;
    }
}
