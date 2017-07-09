/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/WordFilter.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Management/Battleground/Battleground.h"
#include "Map/MapMgr.h"
#include "Units/Creatures/Pet.h"
#include "GameCata/Management/GuildMgr.h"

extern std::string LogFileName;
extern bool bLogChat;

static const uint32 LanguageSkills[NUM_LANGUAGES] =
{
    0,              // UNIVERSAL          0x00
    109,            // ORCISH             0x01
    113,            // DARNASSIAN         0x02
    115,            // TAURAHE            0x03
    0,                // -                0x04
    0,                // -                0x05
    111,            // DWARVISH           0x06
    98,             // COMMON             0x07
    139,            // DEMON TONGUE       0x08
    140,            // TITAN              0x09
    137,            // THALSSIAN          0x0A
    138,            // DRACONIC           0x0B
    0,              // KALIMAG            0x0C
    313,            // GNOMISH            0x0D
    315,            // TROLL              0x0E
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
    673,            // GUTTERSPEAK        0x21
    0,                // -                0x22
    759,            // DRAENEI            0x23
    0,              // ZOMBIE             0x24
    0,              // GNOMISH_BINAR      0x25
    0,              // GOBLIN_BINARY      0x26
    791,            // WORGEN             0x27
    792,            // GOBLIN             0x28
};

void WorldSession::HandleMessagechatOpcode(WorldPacket& recv_data)
{
    WorldPacket* data = nullptr;

    uint32 type;
    int32 lang;

    const char* pMisc = NULL;
    const char* pMsg = NULL;

    switch (recv_data.GetOpcode())
    {
        case CMSG_MESSAGECHAT_SAY:
            type = CHAT_MSG_SAY;
            break;
        case CMSG_MESSAGECHAT_YELL:
            type = CHAT_MSG_YELL;
            break;
        case CMSG_MESSAGECHAT_CHANNEL:
            type = CHAT_MSG_CHANNEL;
            break;
        case CMSG_MESSAGECHAT_WHISPER:
            type = CHAT_MSG_WHISPER;
            break;
        case CMSG_MESSAGECHAT_GUILD:
            type = CHAT_MSG_GUILD;
            break;
        case CMSG_MESSAGECHAT_OFFICER:
            type = CHAT_MSG_OFFICER;
            break;
        case CMSG_MESSAGECHAT_AFK:
            type = CHAT_MSG_AFK;
            break;
        case CMSG_MESSAGECHAT_DND:
            type = CHAT_MSG_DND;
            break;
        case CMSG_MESSAGECHAT_EMOTE:
            type = CHAT_MSG_EMOTE;
            break;
        case CMSG_MESSAGECHAT_PARTY:
            type = CHAT_MSG_PARTY;
            break;
        case CMSG_MESSAGECHAT_RAID:
            type = CHAT_MSG_RAID;
            break;
        case CMSG_MESSAGECHAT_BATTLEGROUND:
            type = CHAT_MSG_BATTLEGROUND;
            break;
        case CMSG_MESSAGECHAT_RAID_WARNING:
            type = CHAT_MSG_RAID_WARNING;
            break;
        default:
            LogError("HandleMessagechatOpcode : Unknown chat opcode (0x%X)", recv_data.GetOpcode());
            recv_data.clear();
            return;
    }

    if (type != CHAT_MSG_EMOTE && type != CHAT_MSG_AFK && type != CHAT_MSG_DND)
    {
        recv_data >> lang;
    }
    else
    {
        lang = LANG_UNIVERSAL;
    }

    LogDebug("ChatHandler : player mod language %u and lang is %u \n", GetPlayer()->m_modlanguage, lang);

    if (lang >= NUM_LANGUAGES)
        return;

    if (GetPlayer()->IsBanned())
    {
        GetPlayer()->BroadcastMessage("You cannot do that when banned.");
        return;
    }

    // Flood protection
    if (lang != -1 && !GetPermissionCount() && worldConfig.chat.linesBeforeProtection != 0)
    {
        /* flood detection, wheeee! */
        if (UNIXTIME >= floodTime)
        {
            floodLines = 0;
            floodTime = UNIXTIME + worldConfig.chat.secondsBeforeProtectionReset;
        }

        if ((++floodLines) > worldConfig.chat.linesBeforeProtection)
        {
            if (worldConfig.chat.enableSendFloodProtectionMessage)
            {
                _player->BroadcastMessage("Your message has triggered serverside flood protection. You can speak again in %u seconds.", floodTime - UNIXTIME);
            }

            return;
        }
    }

    switch (type)
    {
        case CHAT_MSG_EMOTE:
        case CHAT_MSG_SAY:
        case CHAT_MSG_YELL:
        case CHAT_MSG_WHISPER:
        case CHAT_MSG_CHANNEL:
        case CHAT_MSG_PARTY:
        case CHAT_MSG_PARTY_LEADER:
        case CHAT_MSG_BATTLEGROUND:
        case CHAT_MSG_BATTLEGROUND_LEADER:
        case CHAT_MSG_RAID:
        case CHAT_MSG_RAID_WARNING:
        case CHAT_MSG_RAID_LEADER:
        case CHAT_MSG_GUILD:
        case CHAT_MSG_OFFICER:
        {
            if (m_muted && m_muted >= (uint32)UNIXTIME)
            {
                SystemMessage("Your voice is currently muted by a moderator.");
                return;
            }
        }
        break;
    }

    std::string msg, to = "", channel = "", tmp;
    msg.reserve(256);

    // Process packet
    switch (type)
    {
        case CHAT_MSG_SAY:
        case CHAT_MSG_PARTY:
        case CHAT_MSG_PARTY_LEADER:
        case CHAT_MSG_BATTLEGROUND_LEADER:
        case CHAT_MSG_GUILD:
        case CHAT_MSG_OFFICER:
        case CHAT_MSG_RAID:
        case CHAT_MSG_RAID_LEADER:
        case CHAT_MSG_RAID_WARNING:
        case CHAT_MSG_BATTLEGROUND:
        case CHAT_MSG_AFK:
        case CHAT_MSG_DND:
            msg = recv_data.ReadString(recv_data.readBits(9));
            break;
        case CHAT_MSG_WHISPER:
        {
            uint32 toLength, msgLength;
            toLength = recv_data.readBits(10);
            msgLength = recv_data.readBits(9);
            to = recv_data.ReadString(toLength);
            msg = recv_data.ReadString(msgLength);
        }
        break;
        default:
            LOG_ERROR("CHAT: unknown msg type %u, lang: %u", type, lang);
    }


    if (int(msg.find("|T")) > -1)
    {
        GetPlayer()->BroadcastMessage("Don't even THINK about doing that again");
        return;
    }

    // HookInterface OnChat event
    if (pMsg && !sHookInterface.OnChat(_player, type, lang, pMsg, pMisc))
        return;

    if (GetPlayer()->getAuraWithAuraEffect(SPELL_AURA_COMPREHEND_LANG))
    {
        LOG_ERROR("Player has no aura with effect SPELL_AURA_COMPREHEND_LANG... returning now!");
        return;
    }

    Channel* chn = NULL;
    // Main chat message processing
    switch (type)
    {
        case CHAT_MSG_EMOTE:
        {
            if (worldConfig.player.isInterfactionChatEnabled && lang > 0)
                lang = 0;

            if (g_chatFilter->isBlockedOrReplaceWord(msg))
            {
                SystemMessage("Your chat message was blocked by a server-side filter.");
                return;
            }

            if (GetPlayer()->m_modlanguage >= 0)
                data = sChatHandler.FillMessageData(CHAT_MSG_EMOTE, GetPlayer()->m_modlanguage, msg.c_str(), _player->GetGUID(), _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
            else if (lang == 0 && worldConfig.player.isInterfactionChatEnabled)
                data = sChatHandler.FillMessageData(CHAT_MSG_EMOTE, CanUseCommand('0') ? LANG_UNIVERSAL : lang, msg.c_str(), _player->GetGUID(), _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
            else
                data = sChatHandler.FillMessageData(CHAT_MSG_EMOTE, CanUseCommand('c') ? LANG_UNIVERSAL : lang, msg.c_str(), _player->GetGUID(), _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);

            GetPlayer()->SendMessageToSet(data, true, !worldConfig.player.isInterfactionChatEnabled);

            LogDetail("[emote] %s: %s", _player->GetName(), msg.c_str());
            delete data;

        }
        break;
        case CHAT_MSG_SAY:
        {
            if (worldConfig.player.isInterfactionChatEnabled && lang > 0)
                lang = 0;

            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
                break;

            if (g_chatFilter->isBlockedOrReplaceWord(msg))
            {
                SystemMessage("Your chat message was blocked by a server-side filter.");
                return;
            }

            if (GetPlayer()->m_modlanguage >= 0)
            {
                data = sChatHandler.FillMessageData(CHAT_MSG_SAY, GetPlayer()->m_modlanguage, msg.c_str(), _player->GetGUID(), _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
                GetPlayer()->SendMessageToSet(data, true);
            }
            else
            {
                if (lang > 0 && LanguageSkills[lang] && !_player->_HasSkillLine(LanguageSkills[lang]))
                    return;

                if (lang == 0 && !CanUseCommand('c') && !worldConfig.player.isInterfactionChatEnabled)
                    return;

                data = sChatHandler.FillMessageData(CHAT_MSG_SAY, lang, msg.c_str(), _player->GetGUID(), _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);

                GetPlayer()->SendMessageToSet(data, true);
            }
            delete data;

        }
        break;
        case CHAT_MSG_PARTY:
        case CHAT_MSG_PARTY_LEADER:
        case CHAT_MSG_RAID:
        case CHAT_MSG_RAID_LEADER:
        case CHAT_MSG_RAID_WARNING:
        {
            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
                break;

            if (worldConfig.player.isInterfactionChatEnabled && lang > 0)
                lang = 0;

            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
            {
                SystemMessage("Your chat message was blocked by a server-side filter.");
                return;
            }

            Group* pGroup = _player->GetGroup();
            if (pGroup == NULL) break;

            if (GetPlayer()->m_modlanguage >= 0)
                data = sChatHandler.FillMessageData(type, GetPlayer()->m_modlanguage, msg.c_str(), _player->GetGUID(), _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
            else if (lang == 0 && worldConfig.player.isInterfactionChatEnabled)
                data = sChatHandler.FillMessageData(type, (CanUseCommand('0') && lang != -1) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->GetGUID(), _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
            else
                data = sChatHandler.FillMessageData(type, (CanUseCommand('c') && lang != -1) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->GetGUID(), _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
            if (type == CHAT_MSG_PARTY && pGroup->GetGroupType() == GROUP_TYPE_RAID)
            {
                // only send to that subgroup
                SubGroup* sgr = _player->GetGroup() ?
                    _player->GetGroup()->GetSubGroup(_player->GetSubGroup()) : 0;

                if (sgr)
                {
                    _player->GetGroup()->Lock();
                    for (GroupMembersSet::iterator itr = sgr->GetGroupMembersBegin(); itr != sgr->GetGroupMembersEnd(); ++itr)
                    {
                        if ((*itr)->m_loggedInPlayer)
                        {
                            (*itr)->m_loggedInPlayer->GetSession()->SendChatPacket(data, 1, lang, this);
                        }
                    }
                    _player->GetGroup()->Unlock();
                }
            }
            else
            {
                SubGroup* sgr;
                for (uint32 i = 0; i < _player->GetGroup()->GetSubGroupCount(); ++i)
                {
                    sgr = _player->GetGroup()->GetSubGroup(i);
                    _player->GetGroup()->Lock();
                    for (GroupMembersSet::iterator itr = sgr->GetGroupMembersBegin(); itr != sgr->GetGroupMembersEnd(); ++itr)
                    {
                        if ((*itr)->m_loggedInPlayer)
                        {
                            (*itr)->m_loggedInPlayer->GetSession()->SendChatPacket(data, 1, lang, this);
                        }
                    }
                    _player->GetGroup()->Unlock();

                }
            }
            LogDetail("[party] %s: %s", _player->GetName(), msg.c_str());
            delete data;
        }
        break;
        case CHAT_MSG_GUILD:
        {
            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
            {
                break;
            }

            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
            {
                SystemMessage("Your chat message was blocked by a server-side filter.");
                return;
            }

            if (_player->m_playerInfo->m_guild)
                sGuildMgr.getGuildById(_player->m_playerInfo->m_guild)->broadcastToGuild(this, false, msg.c_str(), lang);
        }
        break;
        case CHAT_MSG_OFFICER:
        {
            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
            {
                break;
            }

            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
            {
                SystemMessage("Your chat message was blocked by a server-side filter.");
                return;
            }

            if (_player->m_playerInfo->m_guild)
                sGuildMgr.getGuildById(_player->m_playerInfo->m_guild)->broadcastToGuild(this, true, msg.c_str(), lang);
        }
        break;
        case CHAT_MSG_YELL:
        {
            if (worldConfig.player.isInterfactionChatEnabled && lang > 0)
            {
                lang = 0;
            }

            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
            {
                break;
            }

            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
            {
                SystemMessage("Your chat message was blocked by a server-side filter.");
                return;
            }
            if (lang > 0 && LanguageSkills[lang] && _player->_HasSkillLine(LanguageSkills[lang]) == false)
                return;

            if (lang == 0 && worldConfig.player.isInterfactionChatEnabled)
            {
                data = sChatHandler.FillMessageData(CHAT_MSG_YELL, (CanUseCommand('0') && lang != -1) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->GetGUID(), _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
            }
            else if (GetPlayer()->m_modlanguage >= 0)
            {
                data = sChatHandler.FillMessageData(CHAT_MSG_YELL, GetPlayer()->m_modlanguage, msg.c_str(), _player->GetGUID(), _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
            }
            else
            {
                data = sChatHandler.FillMessageData(CHAT_MSG_YELL, (CanUseCommand('c') && lang != -1) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->GetGUID(), _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
            }

            _player->GetMapMgr()->SendChatMessageToCellPlayers(_player, data, 2, 1, lang, this);
            delete data;
        }
        break;
        case CHAT_MSG_WHISPER:
        {
            if (lang != -1)
            {
                lang = LANG_UNIVERSAL; //All whispers are universal
            }

            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
            {
                SystemMessage("Your chat message was blocked by a server-side filter.");
                return;
            }

            PlayerCache* playercache = objmgr.GetPlayerCache(to.c_str(), false);
            if (playercache == nullptr)
            {
                data = new WorldPacket(SMSG_CHAT_PLAYER_NOT_FOUND, to.length() + 1);
                *data << to;
                SendPacket(data);
                delete data;
                break;
            }

            if (_player->GetTeamInitial() != playercache->GetUInt32Value(CACHE_PLAYER_INITIALTEAM) && !worldConfig.player.isInterfactionChatEnabled && !playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM) && !_player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM))
            {
                WorldPacket response(SMSG_CHAT_PLAYER_NOT_FOUND, to.length() + 1);
                response << to;
                SendPacket(&response);
                playercache->DecRef();
                break;
            }

            // Check that the player isn't a gm with his status on
            ///\todo Game Master's on retail are able to have block whispers after they close the ticket with the current packet.
            // When a Game Master is visible to your player it says "This player is unavailable for whisper" I need to figure out how this done.
            if (!HasPermissions() && playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM) && playercache->CountValue64(CACHE_GM_TARGETS, _player->GetGUID()) == 0)
            {
                // Build automated reply
                std::string Reply = "SYSTEM: This Game Master does not currently have an open ticket from you and did not receive your whisper. Please submit a new GM Ticket request if you need to speak to a GM. This is an automatic message.";
                data = sChatHandler.FillMessageData(CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, Reply.c_str(), playercache->GetGUID(), 4);
                SendPacket(data);
                delete data;
                playercache->DecRef();
                break;
            }

            if (playercache->CountValue64(CACHE_SOCIAL_IGNORELIST, _player->GetLowGUID()) > 0)
            {
                data = sChatHandler.FillMessageData(CHAT_MSG_IGNORED, LANG_UNIVERSAL, msg.c_str(), playercache->GetGUID(), _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
                SendPacket(data);
                delete data;
                playercache->DecRef();
                break;
            }
            else
            {
                data = sChatHandler.FillMessageData(CHAT_MSG_WHISPER, lang, msg.c_str(), _player->GetGUID(), _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
                playercache->SendPacket(data);
            }


            //Sent the to Users id as the channel, this should be fine as it's not used for whisper
            if (lang != -1) //DO NOT SEND if its an addon message!
            {
                data = sChatHandler.FillMessageData(CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, msg.c_str(), playercache->GetGUID(), playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
                SendPacket(data);
                delete data;
            }

            if (playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_AFK))
            {
                // Has AFK flag, autorespond.
                std::string reason;
                playercache->GetStringValue(CACHE_AFK_DND_REASON, reason);

                data = sChatHandler.FillMessageData(CHAT_MSG_AFK, LANG_UNIVERSAL, reason.c_str(), playercache->GetGUID(), _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
                SendPacket(data);
                delete data;
            }
            else if (playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_DND))
            {
                // Has DND flag, autorespond.
                std::string reason;
                playercache->GetStringValue(CACHE_AFK_DND_REASON, reason);
                data = sChatHandler.FillMessageData(CHAT_MSG_DND, LANG_UNIVERSAL, reason.c_str(), playercache->GetGUID(), playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
                SendPacket(data);
                delete data;
            }

            playercache->DecRef();

        }
        break;
        case CHAT_MSG_CHANNEL:
        {
            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
            {
                SystemMessage("Your chat message was blocked by a server-side filter.");
                return;
            }

            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
            {
                break;
            }

            chn = channelmgr.GetChannel(channel.c_str(), GetPlayer());
            if (chn)
            {
                //g_chatFilter->ParseEscapeCodes((char*)pMsg, (chn->m_flags & CHANNEL_PACKET_ALLOWLINKS)>0);
                chn->Say(GetPlayer(), msg.c_str(), NULL, false);
            }
        }
        break;
        case CHAT_MSG_AFK:
        {
            std::string reason = "";
            recv_data >> reason;

            GetPlayer()->SetAFKReason(reason);

            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
            {
                SystemMessage("Your chat message was blocked by a server-side filter.");
                return;
            }

            /* WorldPacket *data, WorldSession* session, uint32 type, uint32 language, const char *channelName, const char *message*/
            if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK))
            {
                GetPlayer()->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK);
                if (worldConfig.getKickAFKPlayerTime())
                {
                    sEventMgr.RemoveEvents(GetPlayer(), EVENT_PLAYER_SOFT_DISCONNECT);
                }
            }
            else
            {
                GetPlayer()->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK);

                if (GetPlayer()->m_bg)
                {
                    GetPlayer()->m_bg->RemovePlayer(GetPlayer(), false);
                }

                if (worldConfig.getKickAFKPlayerTime())
                {
                    sEventMgr.AddEvent(GetPlayer(), &Player::SoftDisconnect, EVENT_PLAYER_SOFT_DISCONNECT, worldConfig.getKickAFKPlayerTime(), 1, 0);
                }
            }
        }
        break;
        case CHAT_MSG_DND:
        {
            std::string reason;
            recv_data >> reason;
            GetPlayer()->SetAFKReason(reason);

            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
            {
                SystemMessage("Your chat message was blocked by a server-side filter.");
                return;
            }

            if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DND))
            {
                GetPlayer()->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DND);
            }
            else
            {
                GetPlayer()->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_DND);
            }
        }
        break;

        case CHAT_MSG_BATTLEGROUND:
        case CHAT_MSG_BATTLEGROUND_LEADER:
        {
            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
            {
                break;
            }

            if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
            {
                SystemMessage("Your chat message was blocked by a server-side filter.");
                return;
            }

            if (_player->m_bg != NULL)
            {
                data = sChatHandler.FillMessageData(type, lang, msg.c_str(), _player->GetGUID());
                _player->m_bg->DistributePacketToTeam(data, _player->GetTeam());
                delete data;
            }
        }
        break;
    }
}