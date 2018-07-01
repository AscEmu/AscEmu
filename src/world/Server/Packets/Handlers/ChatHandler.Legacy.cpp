/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "Management/WordFilter.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Management/Battleground/Battleground.h"
#include "Map/MapMgr.h"
#include "Units/Creatures/Pet.h"
#include "Chat/ChatDefines.hpp"
#include "Server/Script/ScriptMgr.h"
#include "Chat/ChatHandler.hpp"
#include "Units/Players/Player.h"
#include "Objects/ObjectMgr.h"
#include "Server/Packets/CmsgMessageChat.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Server/Packets/SmsgChatPlayerNotFound.h"
#include "Server/Packets/CmsgTextEmote.h"
#include "Server/Packets/SmsgTextEmote.h"
#include "Server/Packets/SmsgEmote.h"
#include "Server/Packets/CmsgEmote.h"

using namespace AscEmu::Packets;

extern std::string LogFileName;
extern bool bLogChat;

#if VERSION_STRING != Cata
static const uint32 LanguageSkills[NUM_LANGUAGES] =
{
    0,                // UNIVERSAL        0x00
    109,            // ORCISH            0x01
    113,            // DARNASSIAN        0x02
    115,            // TAURAHE            0x03
    0,                // -                0x04
    0,                // -                0x05
    111,            // DWARVISH            0x06
    98,                // COMMON            0x07
    139,            // DEMON TONGUE        0x08
    140,            // TITAN            0x09
    137,            // THALSSIAN        0x0A
    138,            // DRACONIC            0x0B
    0,                // KALIMAG            0x0C
    313,            // GNOMISH            0x0D
    315,            // TROLL            0x0E
    0,                // -                0x0F
    0,                // -                0x10
    0,                // -                0x11
    0,                // -                0x12
    0,                // -                0x13
    0,                // -                0x14
    0,                // -                0x15
    0,                // -                0x16
    0,                // -                0x17
    0,                // -                0x18
    0,                // -                0x19
    0,                // -                0x1A
    0,                // -                0x1B
    0,                // -                0x1C
    0,                // -                0x1D
    0,                // -                0x1E
    0,                // -                0x1F
    0,                // -                0x20
    673,            // -                0x21
    0,                // -                0x22
    759,            // -                0x23
};

// MIT Start
void WorldSession::handleMessageChatOpcode(WorldPacket& recvData)
{
    CHECK_INWORLD_RETURN

    CmsgMessageChat recv_packet;
    if (!recv_packet.deserialise(recvData))
        return;

    auto language = recv_packet.language;
    auto player_can_speak_language = true;

    if (language != LANG_ADDON)
    {
        if (const auto language_skill = LanguageSkills[language])
            player_can_speak_language = _player->_HasSkillLine(language_skill);
    }

    if (language != LANG_ADDON)
    {
        if (worldConfig.player.isInterfactionChatEnabled)
        {
            language = LANG_UNIVERSAL;
            player_can_speak_language = true;
        }

        if (GetPlayer()->m_modlanguage >= 0)
        {
            language = GetPlayer()->m_modlanguage;
            player_can_speak_language = true;
        }

        // GMs speak universal language
        if (GetPermissionCount() > 0)
        {
            language = LANG_UNIVERSAL;
            player_can_speak_language = true;
        }
    }

    // TODO Verify this - can a player even log in while banned? What does banned mean in this context?
    if (GetPlayer()->IsBanned())
    {
        GetPlayer()->BroadcastMessage("You cannot speak while banned.");
        return;
    }

    if (recv_packet.language != LANG_ADDON && !GetPermissionCount() && worldConfig.chat.linesBeforeProtection != 0)
    {
        if (UNIXTIME >= floodTime)
        {
            floodLines = 0;
            floodTime = UNIXTIME + worldConfig.chat.secondsBeforeProtectionReset;
        }

        if (++floodLines > worldConfig.chat.linesBeforeProtection)
        {
            if (worldConfig.chat.enableSendFloodProtectionMessage)
                _player->BroadcastMessage("Your message has triggered server-side flood protection. You can speak again in %u seconds.", floodTime - UNIXTIME);

            return;
        }
    }

    switch (recv_packet.type)
    {
        case CHAT_MSG_EMOTE:
        case CHAT_MSG_SAY:
        case CHAT_MSG_YELL:
        case CHAT_MSG_WHISPER:
        case CHAT_MSG_CHANNEL:
            if (m_muted >= static_cast<uint32>(UNIXTIME))
            {
                SystemMessage("You are currently muted by a moderator.");
                return;
            }
            break;
        default:
            break;
    }

    if (int(recv_packet.message.find("|T")) > -1)
    {
        //GetPlayer()->BroadcastMessage("Don't even THINK about doing that again");
        return;
    }

    if (!sHookInterface.OnChat(_player, recv_packet.type, recv_packet.language, recv_packet.message.c_str(), recv_packet.destination.c_str()))
        return;


    if (g_chatFilter->isBlockedOrReplaceWord(recv_packet.message))
    {
        SystemMessage("Your chat message was blocked by a server-side filter.");
        return;
    }

    auto is_gm_command = false;
    switch (recv_packet.type)
    {
    case CHAT_MSG_SAY:
    case CHAT_MSG_PARTY:
    case CHAT_MSG_PARTY_LEADER:
    case CHAT_MSG_RAID:
    case CHAT_MSG_RAID_LEADER:
    case CHAT_MSG_RAID_WARNING:
    case CHAT_MSG_GUILD:
    case CHAT_MSG_OFFICER:
    case CHAT_MSG_YELL:
        is_gm_command = sChatHandler.ParseCommands(recv_packet.message.c_str(), this) > 0;
        break;
    }

    switch (recv_packet.type)
    {
    case CHAT_MSG_EMOTE:
        // TODO Verify "strange gestures" for xfaction
        GetPlayer()->SendMessageToSet(SmsgMessageChat(CHAT_MSG_EMOTE, language, GetPlayer()->getGuid(), recv_packet.message, _player->isGMFlagSet()).serialise().get(), true, true);
        LogDetail("[emote] %s: %s", _player->getName().c_str(), recv_packet.message.c_str());
        break;
    case CHAT_MSG_SAY:
        if (is_gm_command || !player_can_speak_language)
            break;

        GetPlayer()->SendMessageToSet(SmsgMessageChat(CHAT_MSG_SAY, language, GetPlayer()->getGuid(), recv_packet.message, _player->isGMFlagSet()).serialise().get(), true);
        break;
    case CHAT_MSG_PARTY:
    case CHAT_MSG_PARTY_LEADER:
    case CHAT_MSG_RAID:
    case CHAT_MSG_RAID_LEADER:
    case CHAT_MSG_RAID_WARNING:
    {
        if (is_gm_command || !player_can_speak_language)
            break;

        const auto send_packet = SmsgMessageChat(recv_packet.type, language, _player->getGuid(), recv_packet.message, _player->isGMFlagSet()).serialise();

        if (const auto group = _player->GetGroup())
        {
            if (recv_packet.type == CHAT_MSG_PARTY || recv_packet.type == CHAT_MSG_PARTY_LEADER
                 && group->isRaid())
            {
                if (const auto subgroup = group->GetSubGroup(_player->GetSubGroup()))
                {
                    group->Lock();
                    for (auto group_member : subgroup->getGroupMembers())
                        if (group_member->m_loggedInPlayer)
                            group_member->m_loggedInPlayer->SendPacket(send_packet.get());
                    group->Unlock();
                }
            }
            else
            {
                for (uint32_t i = 0; i < group->GetSubGroupCount(); ++i)
                {
                    if (const auto sub_group = group->GetSubGroup(i))
                    {
                        group->Lock();
                        for (auto group_member : sub_group->getGroupMembers())
                            if (group_member->m_loggedInPlayer)
                                group_member->m_loggedInPlayer->SendPacket(send_packet.get());
                        group->Unlock();
                    }
                }
            }
            LogDetail("[party] %s: %s", _player->getName().c_str(), recv_packet.message.c_str());
        }
    }
        break;
    case CHAT_MSG_GUILD:
        if (is_gm_command)
            break;

        if (const auto guild = _player->m_playerInfo->guild)
            guild->GuildChat(recv_packet.message.c_str(), this, language);
        break;
    case CHAT_MSG_OFFICER:
        if (is_gm_command)
            break;

        if (const auto guild = _player->m_playerInfo->guild)
            guild->OfficerChat(recv_packet.message.c_str(), this, language);
        break;
        case CHAT_MSG_YELL:
        {
            if (is_gm_command || !player_can_speak_language)
                break;

            auto yell_packet = SmsgMessageChat(CHAT_MSG_YELL, language, _player->getGuid(), recv_packet.message,
                                               _player->isGMFlagSet());
            _player->GetMapMgr()->SendChatMessageToCellPlayers(_player, yell_packet.serialise().get(), 2, 1, language,
                                                               this);
        }
            break;
    case CHAT_MSG_WHISPER:
        if (const auto player_cache = objmgr.GetPlayerCache(recv_packet.destination.c_str(), false))
        {
            const auto target_is_our_faction = _player->GetTeamInitial() == player_cache->GetUInt32Value(CACHE_PLAYER_INITIALTEAM);
            const auto target_is_gm_flagged = player_cache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM);
            const auto we_are_gm_flagged = _player->isGMFlagSet();
            if (target_is_our_faction
                || worldConfig.player.isInterfactionChatEnabled
                || target_is_gm_flagged
                || we_are_gm_flagged)
            {
                const auto target_gm_is_speaking_to_us = player_cache->CountValue64(CACHE_GM_TARGETS, _player->getGuid()) == 0;
                if (!we_are_gm_flagged && target_is_gm_flagged && target_gm_is_speaking_to_us)
                {
                    const auto reply = "SYSTEM: This Game Master does not currently have an open ticket from you and did not receive your whisper. Please submit a new GM Ticket request if you need to speak to a GM. This is an automatic message.";
                    SendPacket(SmsgMessageChat(CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, player_cache->GetGUID(), reply, true).serialise().get());
                    player_cache->DecRef();
                    break;
                }

                const auto we_are_being_ignored = player_cache->CountValue64(CACHE_SOCIAL_IGNORELIST, _player->getGuidLow()) > 0;
                if (we_are_being_ignored)
                {
                    SendPacket(SmsgMessageChat(CHAT_MSG_IGNORED, LANG_UNIVERSAL, player_cache->GetGUID(), recv_packet.message, we_are_gm_flagged).serialise().get());
                    player_cache->DecRef();
                    break;
                }

                player_cache->SendPacket(*SmsgMessageChat(CHAT_MSG_WHISPER, language, _player->getGuid(), recv_packet.message, we_are_gm_flagged).serialise().get());
                if (language != LANG_ADDON)
                    // TODO Verify should this be LANG_UNIVERSAL?
                    SendPacket(SmsgMessageChat(CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, player_cache->GetGUID(), recv_packet.message, we_are_gm_flagged).serialise().get());

                if (player_cache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_AFK))
                {
                    std::string reason;
                    player_cache->GetStringValue(CACHE_AFK_DND_REASON, reason);
                    SendPacket(SmsgMessageChat(CHAT_MSG_AFK, LANG_UNIVERSAL, player_cache->GetGUID(), reason, false).serialise().get());
                }
                else if (player_cache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_DND))
                {
                    std::string reason;
                    player_cache->GetStringValue(CACHE_AFK_DND_REASON, reason);
                    SendPacket(SmsgMessageChat(CHAT_MSG_DND, LANG_UNIVERSAL, player_cache->GetGUID(), reason, false).serialise().get());
                }

                player_cache->DecRef();
            }
        }
        else
        {
            SendPacket(SmsgChatPlayerNotFound(recv_packet.destination).serialise().get());
        }
        break;
    case CHAT_MSG_CHANNEL:
        if (is_gm_command)
            break;

        if (const auto channel = channelmgr.GetChannel(recv_packet.destination.c_str(), GetPlayer()))
            channel->Say(GetPlayer(), recv_packet.message.c_str(), nullptr, false);

        break;
    case CHAT_MSG_AFK:
        GetPlayer()->SetAFKReason(recv_packet.message);
        GetPlayer()->toggleAfk();
        break;
    case CHAT_MSG_DND:
        GetPlayer()->SetAFKReason(recv_packet.message);
        GetPlayer()->toggleDnd();
        break;
    case CHAT_MSG_BATTLEGROUND:
    case CHAT_MSG_BATTLEGROUND_LEADER:
        if (is_gm_command || !player_can_speak_language)
            break;

        if (!GetPlayer()->m_bg)
            break;

        GetPlayer()->m_bg->DistributePacketToTeam(SmsgMessageChat(recv_packet.type, language, GetPlayer()->getGuid(), recv_packet.message, GetPlayer()->isGMFlagSet()).serialise().get(), GetPlayer()->GetTeam());
        break;
    }
}
#endif

#if VERSION_STRING != Cata
void WorldSession::handleTextEmoteOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    if (!_player->isAlive())
     return;

    CmsgTextEmote recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    if (m_muted && m_muted >= static_cast<uint32_t>(UNIXTIME))
    {
        SystemMessage("Your voice is currently muted by a moderator.");
        return;
    }

    if (!GetPermissionCount() && worldConfig.chat.linesBeforeProtection)
    {
        if (UNIXTIME >= floodTime)
        {
            floodLines = 0;
            floodTime = UNIXTIME + worldConfig.chat.secondsBeforeProtectionReset;
        }

        if (++floodLines > worldConfig.chat.linesBeforeProtection)
            return;
    }

    uint32_t nameLength = 1;
    std::string unitName;

    auto pUnit = _player->GetMapMgr()->GetUnit(recv_packet.guid);
    if (pUnit)
    {
        if (pUnit->isPlayer())
            unitName = static_cast<Player*>(pUnit)->getName();
        else if (pUnit->isPet())
            unitName = static_cast<Pet*>(pUnit)->GetName();
        else
            unitName = static_cast<Creature*>(pUnit)->GetCreatureProperties()->Name;

        nameLength = static_cast<uint32_t>(unitName.length() + 1);
    }

    auto emoteTextEntry = sEmotesTextStore.LookupEntry(recv_packet.text_emote);
    if (emoteTextEntry)
    {
        sHookInterface.OnEmote(_player, static_cast<EmoteType>(emoteTextEntry->textid), pUnit);
        if (pUnit)
            CALL_SCRIPT_EVENT(pUnit, OnEmote)(_player, static_cast<EmoteType>(emoteTextEntry->textid));

        switch (emoteTextEntry->textid)
        {
            case EMOTE_STATE_SLEEP:
            case EMOTE_STATE_SIT:
            case EMOTE_STATE_KNEEL:
            case EMOTE_STATE_DANCE:
            {
                _player->setEmoteState(emoteTextEntry->textid);
            } break;
            default:
                break;
        }

        GetPlayer()->SendMessageToSet(SmsgEmote(emoteTextEntry->textid, GetPlayer()->getGuid()).serialise().get(), true);

        GetPlayer()->SendMessageToSet(SmsgTextEmote(nameLength, unitName, recv_packet.text_emote, GetPlayer()->getGuid(), recv_packet.unk).serialise().get(), true);

#if VERSION_STRING > TBC
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE, recv_packet.text_emote, 0, 0);
#endif
        sQuestMgr.OnPlayerEmote(_player, recv_packet.text_emote, recv_packet.guid);
    }
}
#endif

void WorldSession::handleEmoteOpcode(WorldPacket& recv_data)
{
    CmsgEmote recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    if (!_player->isAlive())
        return;

    _player->Emote(static_cast<EmoteType>(recv_packet.emote));

#if VERSION_STRING > TBC
    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE, recv_packet.emote, 0, 0);
#endif

    uint64_t guid = _player->getGuid();
    sQuestMgr.OnPlayerEmote(_player, recv_packet.emote, guid);
}

// MIT End

#if VERSION_STRING != Cata
///\todo remove these unk unk unk nighrmare!
void WorldSession::HandleReportSpamOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recv_data, 1 + 8);
    LOG_DEBUG("WORLD: CMSG_REPORT_SPAM");

    uint8 spam_type;                                        // 0 - mail, 1 - chat
    uint64 spammer_guid;
    uint32 unk1 = 0;
    uint32 unk2 = 0;
    uint32 unk3 = 0;
    uint32 unk4 = 0;

    std::string description = "";
    recv_data >> spam_type;                                 // unk 0x01 const, may be spam type (mail/chat)
    recv_data >> spammer_guid;                              // player guid
    switch (spam_type)
    {
        case 0:
        {
            CHECK_PACKET_SIZE(recv_data, recv_data.rpos() + 4 + 4 + 4);
            recv_data >> unk1;                              // const 0
            recv_data >> unk2;                              // probably mail id
            recv_data >> unk3;                              // const 0
        } break;
        case 1:
        {
            CHECK_PACKET_SIZE(recv_data, recv_data.rpos() + 4 + 4 + 4 + 4 + 1);
            recv_data >> unk1;                              // probably language
            recv_data >> unk2;                              // message type?
            recv_data >> unk3;                              // probably channel id
            recv_data >> unk4;                              // unk random value
            recv_data >> description;                       // spam description string (messagetype, channel name, player name, message)
        } break;
    }
    // NOTE: all chat messages from this spammer automatically ignored by spam reporter until logout in case chat spam.
    // if it's mail spam - ALL mails from this spammer automatically removed by client

    // Complaint Received message
    WorldPacket data(SMSG_COMPLAIN_RESULT, 1);
    data << uint8(0);
    SendPacket(&data);

    LOG_DEBUG("REPORT SPAM: type %u, guid %u, unk1 %u, unk2 %u, unk3 %u, unk4 %u, message %s", spam_type, Arcemu::Util::GUID_LOPART(spammer_guid), unk1, unk2, unk3, unk4, description.c_str());
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleChatIgnoredOpcode(WorldPacket & recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 8 + 1);

    uint64 iguid;
    uint8 unk;

    recvPacket >> iguid;
    recvPacket >> unk; // probably related to spam reporting

    Player* player = objmgr.GetPlayer(uint32(iguid));
    if (!player || !player->GetSession())
    {
        return;
    }

    WorldPacket* data = sChatHandler.FillMessageData(CHAT_MSG_IGNORED, LANG_UNIVERSAL, _player->getName().c_str(), _player->getGuid());
    player->GetSession()->SendPacket(data);
    delete data;
}
#else
//MIT
void WorldSession::HandleChatIgnoredOpcode(WorldPacket& recvPacket)
{
    uint8_t unk;
    recvPacket >> unk;

    ObjectGuid playerGuid;

    playerGuid[5] = recvPacket.readBit();
    playerGuid[2] = recvPacket.readBit();
    playerGuid[6] = recvPacket.readBit();
    playerGuid[4] = recvPacket.readBit();
    playerGuid[7] = recvPacket.readBit();
    playerGuid[0] = recvPacket.readBit();
    playerGuid[1] = recvPacket.readBit();
    playerGuid[3] = recvPacket.readBit();

    recvPacket.ReadByteSeq(playerGuid[0]);
    recvPacket.ReadByteSeq(playerGuid[6]);
    recvPacket.ReadByteSeq(playerGuid[5]);
    recvPacket.ReadByteSeq(playerGuid[1]);
    recvPacket.ReadByteSeq(playerGuid[4]);
    recvPacket.ReadByteSeq(playerGuid[3]);
    recvPacket.ReadByteSeq(playerGuid[7]);
    recvPacket.ReadByteSeq(playerGuid[2]);

    Player* player = objmgr.GetPlayer((uint32)playerGuid);
    if (player == nullptr || player->GetSession() == nullptr)
    {
        return;
    }

    WorldPacket* data = sChatHandler.FillMessageData(CHAT_MSG_IGNORED, LANG_UNIVERSAL, _player->getName().c_str(), _player->getGuid());
    player->GetSession()->SendPacket(data);
    delete data;
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleChatChannelWatchOpcode(WorldPacket& recvPacket)
{
    std::string channelName;
    recvPacket >> channelName;

    LogDebugFlag(LF_OPCODE, "Unhandled... Player %s watch channel: %s", _player->getName().c_str(), channelName.c_str());
}
#endif
