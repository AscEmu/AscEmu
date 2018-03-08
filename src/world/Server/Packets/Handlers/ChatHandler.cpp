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
        LogDetail("[emote] %s: %s", _player->GetName(), recv_packet.message.c_str());
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
            LogDetail("[party] %s: %s", _player->GetName(), recv_packet.message.c_str());
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

                player_cache->SendPacket(SmsgMessageChat(CHAT_MSG_WHISPER, language, _player->getGuid(), recv_packet.message, we_are_gm_flagged).serialise().get());
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

// MIT End

//void WorldSession::HandleMessagechatOpcode(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN
//
//    WorldPacket* data = nullptr;
//
//    uint32 type;
//    int32 lang;
//
//    const char* pMisc = NULL;
//    const char* pMsg = NULL;
//
//    recv_data >> type;
//    recv_data >> lang;
//
//    if (lang >= NUM_LANGUAGES)
//        return;
//
//    if (GetPlayer()->IsBanned())
//    {
//        GetPlayer()->BroadcastMessage("You cannot do that when banned.");
//        return;
//    }
//
//    // Flood protection
//    if (lang != -1 && !GetPermissionCount() && worldConfig.chat.linesBeforeProtection != 0)
//    {
//        /* flood detection, wheeee! */
//        if (UNIXTIME >= floodTime)
//        {
//            floodLines = 0;
//            floodTime = UNIXTIME + worldConfig.chat.secondsBeforeProtectionReset;
//        }
//
//        if ((++floodLines) > worldConfig.chat.linesBeforeProtection)
//        {
//            if (worldConfig.chat.enableSendFloodProtectionMessage)
//            {
//                _player->BroadcastMessage("Your message has triggered serverside flood protection. You can speak again in %u seconds.", floodTime - UNIXTIME);
//            }
//
//            return;
//        }
//    }
//
//    switch (type)
//    {
//        case CHAT_MSG_EMOTE:
//        case CHAT_MSG_SAY:
//        case CHAT_MSG_YELL:
//        case CHAT_MSG_WHISPER:
//        case CHAT_MSG_CHANNEL:
//        {
//            if (m_muted && m_muted >= (uint32)UNIXTIME)
//            {
//                SystemMessage("Your voice is currently muted by a moderator.");
//                return;
//            }
//        }
//        break;
//    }
//
//    std::string msg, to = "", channel = "", tmp;
//    msg.reserve(256);
//
//    // Process packet
//    switch (type)
//    {
//        case CHAT_MSG_SAY:
//        case CHAT_MSG_EMOTE:
//        case CHAT_MSG_PARTY:
//        case CHAT_MSG_PARTY_LEADER:
//        case CHAT_MSG_RAID:
//        case CHAT_MSG_RAID_LEADER:
//        case CHAT_MSG_RAID_WARNING:
//        case CHAT_MSG_GUILD:
//        case CHAT_MSG_OFFICER:
//        case CHAT_MSG_YELL:
//            recv_data >> msg;
//            pMsg = msg.c_str();
//            //g_chatFilter->ParseEscapeCodes((char*)pMsg,true);
//            pMisc = 0;
//            break;
//        case CHAT_MSG_WHISPER:
//            recv_data >> to >> msg;
//            pMsg = msg.c_str();
//            pMisc = to.c_str();
//            break;
//        case CHAT_MSG_CHANNEL:
//            recv_data >> channel;
//            recv_data >> msg;
//            pMsg = msg.c_str();
//            pMisc = channel.c_str();
//            break;
//        case CHAT_MSG_AFK:
//        case CHAT_MSG_DND:
//            break;
//        case CHAT_MSG_BATTLEGROUND:
//        case CHAT_MSG_BATTLEGROUND_LEADER:
//            recv_data >> msg;
//            pMsg = msg.c_str();
//            break;
//        default:
//            LOG_ERROR("CHAT: unknown msg type %u, lang: %u", type, lang);
//    }
//
//
//    if (int(msg.find("|T")) > -1)
//    {
//        GetPlayer()->BroadcastMessage("Don't even THINK about doing that again");
//        return;
//    }
//
//    // HookInterface OnChat event
//    if (pMsg && !sHookInterface.OnChat(_player, type, lang, pMsg, pMisc))
//        return;
//
//    Channel* chn = NULL;
//    // Main chat message processing
//    switch (type)
//    {
//        case CHAT_MSG_EMOTE:
//        {
//            if (worldConfig.player.isInterfactionChatEnabled && lang > 0)
//                lang = 0;
//
//            if (g_chatFilter->isBlockedOrReplaceWord(msg))
//            {
//                SystemMessage("Your chat message was blocked by a server-side filter.");
//                return;
//            }
//
//            if (GetPlayer()->m_modlanguage >= 0)
//                data = sChatHandler.FillMessageData(CHAT_MSG_EMOTE, GetPlayer()->m_modlanguage, msg.c_str(), _player->getGuid(), _player->isGMFlagSet() ? 4 : 0);
//            else if (lang == 0 && worldConfig.player.isInterfactionChatEnabled)
//                data = sChatHandler.FillMessageData(CHAT_MSG_EMOTE, CanUseCommand('0') ? LANG_UNIVERSAL : lang, msg.c_str(), _player->getGuid(), _player->isGMFlagSet() ? 4 : 0);
//            else
//                data = sChatHandler.FillMessageData(CHAT_MSG_EMOTE, CanUseCommand('c') ? LANG_UNIVERSAL : lang, msg.c_str(), _player->getGuid(), _player->isGMFlagSet() ? 4 : 0);
//
//            GetPlayer()->SendMessageToSet(data, true, !worldConfig.player.isInterfactionChatEnabled);
//
//            LogDetail("[emote] %s: %s", _player->GetName(), msg.c_str());
//            delete data;
//
//        }
//        break;
//        case CHAT_MSG_SAY:
//        {
//            if (worldConfig.player.isInterfactionChatEnabled && lang > 0)
//                lang = 0;
//
//            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
//                break;
//
//            if (g_chatFilter->isBlockedOrReplaceWord(msg))
//            {
//                SystemMessage("Your chat message was blocked by a server-side filter.");
//                return;
//            }
//
//            if (GetPlayer()->m_modlanguage >= 0)
//            {
//                data = sChatHandler.FillMessageData(CHAT_MSG_SAY, GetPlayer()->m_modlanguage, msg.c_str(), _player->getGuid(), _player->isGMFlagSet() ? 4 : 0);
//                GetPlayer()->SendMessageToSet(data, true);
//            }
//            else
//            {
//                if (lang > 0 && LanguageSkills[lang] && !_player->_HasSkillLine(LanguageSkills[lang]))
//                    return;
//
//                if (lang == 0 && !CanUseCommand('c') && !worldConfig.player.isInterfactionChatEnabled)
//                    return;
//
//                data = sChatHandler.FillMessageData(CHAT_MSG_SAY, lang, msg.c_str(), _player->getGuid(), _player->isGMFlagSet() ? 4 : 0);
//
//                GetPlayer()->SendMessageToSet(data, true);
//            }
//            delete data;
//
//        }
//        break;
//        case CHAT_MSG_PARTY:
//        case CHAT_MSG_PARTY_LEADER:
//        case CHAT_MSG_RAID:
//        case CHAT_MSG_RAID_LEADER:
//        case CHAT_MSG_RAID_WARNING:
//        {
//            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
//                break;
//
//            if (worldConfig.player.isInterfactionChatEnabled && lang > 0)
//                lang = 0;
//
//            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
//            {
//                SystemMessage("Your chat message was blocked by a server-side filter.");
//                return;
//            }
//
//            Group* pGroup = _player->GetGroup();
//            if (pGroup == NULL) break;
//
//            if (GetPlayer()->m_modlanguage >= 0)
//                data = sChatHandler.FillMessageData(type, GetPlayer()->m_modlanguage, msg.c_str(), _player->getGuid(), _player->isGMFlagSet() ? 4 : 0);
//            else if (lang == 0 && worldConfig.player.isInterfactionChatEnabled)
//                data = sChatHandler.FillMessageData(type, (CanUseCommand('0') && lang != -1) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->getGuid(), _player->isGMFlagSet() ? 4 : 0);
//            else
//                data = sChatHandler.FillMessageData(type, (CanUseCommand('c') && lang != -1) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->getGuid(), _player->isGMFlagSet() ? 4 : 0);
//            if (type == CHAT_MSG_PARTY && pGroup->getGroupType() == GROUP_TYPE_RAID)
//            {
//                // only send to that subgroup
//                SubGroup* sgr = _player->GetGroup() ?
//                    _player->GetGroup()->GetSubGroup(_player->GetSubGroup()) : 0;
//
//                if (sgr)
//                {
//                    _player->GetGroup()->Lock();
//                    for (GroupMembersSet::iterator itr = sgr->GetGroupMembersBegin(); itr != sgr->GetGroupMembersEnd(); ++itr)
//                    {
//                        if ((*itr)->m_loggedInPlayer)
//                        {
//                            (*itr)->m_loggedInPlayer->GetSession()->SendChatPacket(data, 1, lang, this);
//                        }
//                    }
//                    _player->GetGroup()->Unlock();
//                }
//            }
//            else
//            {
//                SubGroup* sgr;
//                for (uint32 i = 0; i < _player->GetGroup()->GetSubGroupCount(); ++i)
//                {
//                    sgr = _player->GetGroup()->GetSubGroup(i);
//                    _player->GetGroup()->Lock();
//                    for (GroupMembersSet::iterator itr = sgr->GetGroupMembersBegin(); itr != sgr->GetGroupMembersEnd(); ++itr)
//                    {
//                        if ((*itr)->m_loggedInPlayer)
//                        {
//                            (*itr)->m_loggedInPlayer->GetSession()->SendChatPacket(data, 1, lang, this);
//                        }
//                    }
//                    _player->GetGroup()->Unlock();
//
//                }
//            }
//            LogDetail("[party] %s: %s", _player->GetName(), msg.c_str());
//            delete data;
//        }
//        break;
//        case CHAT_MSG_GUILD:
//        {
//            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
//            {
//                break;
//            }
//
//            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
//            {
//                SystemMessage("Your chat message was blocked by a server-side filter.");
//                return;
//            }
//
//            if (_player->m_playerInfo->guild)
//                _player->m_playerInfo->guild->GuildChat(msg.c_str(), this, lang);
//
//        }
//        break;
//        case CHAT_MSG_OFFICER:
//        {
//            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
//            {
//                break;
//            }
//
//            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
//            {
//                SystemMessage("Your chat message was blocked by a server-side filter.");
//                return;
//            }
//
//            if (_player->m_playerInfo->guild)
//            {
//                _player->m_playerInfo->guild->OfficerChat(msg.c_str(), this, lang);
//            }
//        }
//        break;
//        case CHAT_MSG_YELL:
//        {
//            if (worldConfig.player.isInterfactionChatEnabled && lang > 0)
//            {
//                lang = 0;
//            }
//
//            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
//            {
//                break;
//            }
//
//            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
//            {
//                SystemMessage("Your chat message was blocked by a server-side filter.");
//                return;
//            }
//            if (lang > 0 && LanguageSkills[lang] && _player->_HasSkillLine(LanguageSkills[lang]) == false)
//                return;
//
//            if (lang == 0 && worldConfig.player.isInterfactionChatEnabled)
//            {
//                data = sChatHandler.FillMessageData(CHAT_MSG_YELL, (CanUseCommand('0') && lang != -1) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->getGuid(), _player->isGMFlagSet() ? 4 : 0);
//            }
//            else if (GetPlayer()->m_modlanguage >= 0)
//            {
//                data = sChatHandler.FillMessageData(CHAT_MSG_YELL, GetPlayer()->m_modlanguage, msg.c_str(), _player->getGuid(), _player->isGMFlagSet() ? 4 : 0);
//            }
//            else
//            {
//                data = sChatHandler.FillMessageData(CHAT_MSG_YELL, (CanUseCommand('c') && lang != -1) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->getGuid(), _player->isGMFlagSet() ? 4 : 0);
//            }
//
//            _player->GetMapMgr()->SendChatMessageToCellPlayers(_player, data, 2, 1, lang, this);
//            delete data;
//        }
//        break;
//        case CHAT_MSG_WHISPER:
//        {
//            if (lang != -1)
//            {
//                lang = LANG_UNIVERSAL; //All whispers are universal
//            }
//
//            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
//            {
//                SystemMessage("Your chat message was blocked by a server-side filter.");
//                return;
//            }
//
//            PlayerCache* playercache = objmgr.GetPlayerCache(to.c_str(), false);
//            if (playercache == nullptr)
//            {
//                data = new WorldPacket(SMSG_CHAT_PLAYER_NOT_FOUND, to.length() + 1);
//                *data << to;
//                SendPacket(data);
//                delete data;
//                break;
//            }
//
//            if (_player->GetTeamInitial() != playercache->GetUInt32Value(CACHE_PLAYER_INITIALTEAM) && !worldConfig.player.isInterfactionChatEnabled && !playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM) && !_player->isGMFlagSet())
//            {
//                WorldPacket response(SMSG_CHAT_PLAYER_NOT_FOUND, to.length() + 1);
//                response << to;
//                SendPacket(&response);
//                playercache->DecRef();
//                break;
//            }
//
//            // Check that the player isn't a gm with his status on
//            ///\todo Game Master's on retail are able to have block whispers after they close the ticket with the current packet.
//            // When a Game Master is visible to your player it says "This player is unavailable for whisper" I need to figure out how this done.
//            if (!HasPermissions() && playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM) && playercache->CountValue64(CACHE_GM_TARGETS, _player->getGuid()) == 0)
//            {
//                // Build automated reply
//                std::string Reply = "SYSTEM: This Game Master does not currently have an open ticket from you and did not receive your whisper. Please submit a new GM Ticket request if you need to speak to a GM. This is an automatic message.";
//                data = sChatHandler.FillMessageData(CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, Reply.c_str(), playercache->getGuid(), 4);
//                SendPacket(data);
//                delete data;
//                playercache->DecRef();
//                break;
//            }
//
//            if (playercache->CountValue64(CACHE_SOCIAL_IGNORELIST, _player->getGuidLow()) > 0)
//            {
//                data = sChatHandler.FillMessageData(CHAT_MSG_IGNORED, LANG_UNIVERSAL, msg.c_str(), playercache->getGuid(), _player->isGMFlagSet() ? 4 : 0);
//                SendPacket(data);
//                delete data;
//                playercache->DecRef();
//                break;
//            }
//            else
//            {
//                data = sChatHandler.FillMessageData(CHAT_MSG_WHISPER, lang, msg.c_str(), _player->getGuid(), _player->isGMFlagSet() ? 4 : 0);
//                playercache->SendPacket(data);
//            }
//
//
//            //Sent the to Users id as the channel, this should be fine as it's not used for whisper
//            if (lang != -1) //DO NOT SEND if its an addon message!
//            {
//                data = sChatHandler.FillMessageData(CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, msg.c_str(), playercache->getGuid(), playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
//                SendPacket(data);
//                delete data;
//            }
//
//            if (playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_AFK))
//            {
//                // Has AFK flag, autorespond.
//                std::string reason;
//                playercache->GetStringValue(CACHE_AFK_DND_REASON, reason);
//
//                data = sChatHandler.FillMessageData(CHAT_MSG_AFK, LANG_UNIVERSAL, reason.c_str(), playercache->getGuid(), _player->isGMFlagSet() ? 4 : 0);
//                SendPacket(data);
//                delete data;
//            }
//            else if (playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_DND))
//            {
//                // Has DND flag, autorespond.
//                std::string reason;
//                playercache->GetStringValue(CACHE_AFK_DND_REASON, reason);
//                data = sChatHandler.FillMessageData(CHAT_MSG_DND, LANG_UNIVERSAL, reason.c_str(), playercache->getGuid(), playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
//                SendPacket(data);
//                delete data;
//            }
//
//            playercache->DecRef();
//
//        }
//        break;
//        case CHAT_MSG_CHANNEL:
//        {
//            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
//            {
//                SystemMessage("Your chat message was blocked by a server-side filter.");
//                return;
//            }
//
//            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
//            {
//                break;
//            }
//
//            chn = channelmgr.GetChannel(channel.c_str(), GetPlayer());
//            if (chn)
//            {
//                //g_chatFilter->ParseEscapeCodes((char*)pMsg, (chn->m_flags & CHANNEL_PACKET_ALLOWLINKS)>0);
//                chn->Say(GetPlayer(), msg.c_str(), NULL, false);
//            }
//        }
//        break;
//        case CHAT_MSG_AFK:
//        {
//            std::string reason = "";
//            recv_data >> reason;
//
//            GetPlayer()->SetAFKReason(reason);
//
//            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
//            {
//                SystemMessage("Your chat message was blocked by a server-side filter.");
//                return;
//            }
//
//            /* WorldPacket *data, WorldSession* session, uint32 type, uint32 language, const char *channelName, const char *message*/
//            if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK))
//            {
//                GetPlayer()->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK);
//                if (worldConfig.getKickAFKPlayerTime())
//                {
//                    sEventMgr.RemoveEvents(GetPlayer(), EVENT_PLAYER_SOFT_DISCONNECT);
//                }
//            }
//            else
//            {
//                GetPlayer()->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK);
//
//                if (GetPlayer()->m_bg)
//                {
//                    GetPlayer()->m_bg->RemovePlayer(GetPlayer(), false);
//                }
//
//                if (worldConfig.getKickAFKPlayerTime())
//                {
//                    sEventMgr.AddEvent(GetPlayer(), &Player::SoftDisconnect, EVENT_PLAYER_SOFT_DISCONNECT, worldConfig.getKickAFKPlayerTime(), 1, 0);
//                }
//            }
//        }
//        break;
//        case CHAT_MSG_DND:
//        {
//            std::string reason;
//            recv_data >> reason;
//            GetPlayer()->SetAFKReason(reason);
//
//            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
//            {
//                SystemMessage("Your chat message was blocked by a server-side filter.");
//                return;
//            }
//
//            if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DND))
//            {
//                GetPlayer()->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DND);
//            }
//            else
//            {
//                GetPlayer()->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_DND);
//            }
//        }
//        break;
//
//        case CHAT_MSG_BATTLEGROUND:
//        case CHAT_MSG_BATTLEGROUND_LEADER:
//        {
//            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
//            {
//                break;
//            }
//
//            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
//            {
//                SystemMessage("Your chat message was blocked by a server-side filter.");
//                return;
//            }
//
//            if (_player->m_bg != NULL)
//            {
//                data = sChatHandler.FillMessageData(type, lang, msg.c_str(), _player->getGuid());
//                _player->m_bg->DistributePacketToTeam(data, _player->GetTeam());
//                delete data;
//            }
//        }
//        break;
//    }
//}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleEmoteOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recv_data, 4);

    if (!_player->isAlive())
    {
        return;
    }

    uint32 emote;
    recv_data >> emote;
    _player->Emote((EmoteType)emote);
#if VERSION_STRING > TBC
    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE, emote, 0, 0);
#endif
    uint64 guid = _player->getGuid();
    sQuestMgr.OnPlayerEmote(_player, emote, guid);
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleTextEmoteOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recv_data, 16);
    if (!_player->isAlive())
    {
        return;
    }

    uint64 guid;
    uint32 text_emote;
    uint32 unk;
    uint32 namelen = 1;
    const char* name = " ";

    recv_data >> text_emote;
    recv_data >> unk;
    recv_data >> guid;
    if (m_muted && m_muted >= (uint32)UNIXTIME)
    {
        SystemMessage("Your voice is currently muted by a moderator.");
        return;
    }

    if (!GetPermissionCount() && worldConfig.chat.linesBeforeProtection)
    {
        /* flood detection, wheeee! */
        if (UNIXTIME >= floodTime)
        {
            floodLines = 0;
            floodTime = UNIXTIME + worldConfig.chat.secondsBeforeProtectionReset;
        }

        if ((++floodLines) > worldConfig.chat.linesBeforeProtection)
        {
            return;
        }
    }
    Unit* pUnit = _player->GetMapMgr()->GetUnit(guid);
    if (pUnit)
    {
        if (pUnit->IsPlayer())
        {
            name = static_cast< Player* >(pUnit)->GetName();
            namelen = (uint32)strlen(name) + 1;
        }
        else if (pUnit->IsPet())
        {
            name = static_cast< Pet* >(pUnit)->GetName().c_str();
            namelen = (uint32)strlen(name) + 1;
        }
        else
        {
            Creature* p = static_cast< Creature* >(pUnit);
            name = p->GetCreatureProperties()->Name.c_str();
            namelen = (uint32)strlen(name) + 1;
        }
    }

    auto emote_text_entry = sEmotesTextStore.LookupEntry(text_emote);
    if (emote_text_entry)
    {
        WorldPacket data(SMSG_EMOTE, 28 + namelen);

        sHookInterface.OnEmote(_player, (EmoteType)emote_text_entry->textid, pUnit);
        if (pUnit)
        {
            CALL_SCRIPT_EVENT(pUnit, OnEmote)(_player, (EmoteType)emote_text_entry->textid);
        }

        switch (emote_text_entry->textid)
        {
            case EMOTE_STATE_SLEEP:
            case EMOTE_STATE_SIT:
            case EMOTE_STATE_KNEEL:
            case EMOTE_STATE_DANCE:
            {
                _player->SetEmoteState(emote_text_entry->textid);
            } break;
            default:
                break;
        }

        data << uint32(emote_text_entry->textid);
        data << uint64(GetPlayer()->getGuid());
        GetPlayer()->SendMessageToSet(&data, true); //If player receives his own emote, his animation stops.

        data.Initialize(SMSG_TEXT_EMOTE);
        data << uint64(GetPlayer()->getGuid());
        data << uint32(text_emote);
        data << unk;
        data << uint32(namelen);
        if (namelen > 1)
        {
            data.append(name, namelen);
        }
        else
        {
            data << uint8(0x00);
        }

        GetPlayer()->SendMessageToSet(&data, true);
#if VERSION_STRING > TBC
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE, text_emote, 0, 0);
#endif
        sQuestMgr.OnPlayerEmote(_player, text_emote, guid);
    }
}
#endif

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

    WorldPacket* data = sChatHandler.FillMessageData(CHAT_MSG_IGNORED, LANG_UNIVERSAL, _player->GetName(), _player->getGuid());
    player->GetSession()->SendPacket(data);
    delete data;
}

void WorldSession::HandleChatChannelWatchOpcode(WorldPacket& recvPacket)
{
    std::string channelName;
    recvPacket >> channelName;

    LogDebugFlag(LF_OPCODE, "Unhandled... Player %s watch channel: %s", _player->GetName(), channelName.c_str());
}
#endif
