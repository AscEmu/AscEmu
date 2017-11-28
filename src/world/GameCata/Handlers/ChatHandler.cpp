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
#include "Server/Script/ScriptMgr.h"
#include "Chat/ChatHandler.hpp"
#include "Spell/SpellAuras.h"
#include "Objects/ObjectMgr.h"

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

struct OpcodeToChatType
{
    uint32 opcode;
    uint32 chatType;
};

#define MSG_OPCODE_COUNT 13

OpcodeToChatType opcodeToChatTypeList[MSG_OPCODE_COUNT] =
{
    { CMSG_MESSAGECHAT_SAY, CHAT_MSG_SAY },
    { CMSG_MESSAGECHAT_YELL, CHAT_MSG_YELL },
    { CMSG_MESSAGECHAT_CHANNEL, CHAT_MSG_CHANNEL },
    { CMSG_MESSAGECHAT_WHISPER, CHAT_MSG_WHISPER },
    { CMSG_MESSAGECHAT_GUILD, CHAT_MSG_GUILD },
    { CMSG_MESSAGECHAT_OFFICER, CHAT_MSG_OFFICER },
    { CMSG_MESSAGECHAT_AFK, CHAT_MSG_AFK },
    { CMSG_MESSAGECHAT_DND, CHAT_MSG_DND },
    { CMSG_MESSAGECHAT_EMOTE, CHAT_MSG_EMOTE },
    { CMSG_MESSAGECHAT_PARTY, CHAT_MSG_PARTY },
    { CMSG_MESSAGECHAT_RAID, CHAT_MSG_RAID },
    { CMSG_MESSAGECHAT_BATTLEGROUND, CHAT_MSG_BATTLEGROUND },
    { CMSG_MESSAGECHAT_RAID_WARNING, CHAT_MSG_RAID_WARNING }
};

uint32_t getMessageTypeForOpcode(uint32_t opcode)
{
    for (int i = 0; i < MSG_OPCODE_COUNT; ++i)
    {
        if (opcodeToChatTypeList[i].opcode == opcode)
        {
            return opcodeToChatTypeList[i].chatType;
        }
    }

    return 0xFF;
}

bool WorldSession::isSessionMuted()
{
    if (m_muted && m_muted >= (uint32)UNIXTIME)
    {
        SystemMessage("Your voice is currently muted by a moderator.");
        return true;
    }

    return false;
}

bool WorldSession::isFloodProtectionTriggered()
{
    if (!GetPermissionCount() && worldConfig.chat.linesBeforeProtection)
    {
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
            return true;
        }
    }

    return false;
}

void WorldSession::HandleMessagechatOpcode(WorldPacket& recvData)
{
    WorldPacket* data = nullptr;

    int32 lang;

    const char* pMisc = nullptr;
    const char* pMsg = nullptr;

    uint32 type = getMessageTypeForOpcode(recvData.GetOpcode());
    if (type == 0xFF)
    {
        LogError("HandleMessagechatOpcode : Unknown chat opcode (0x%X)", recvData.GetOpcode());
        recvData.clear();
        return;
    }

    if (type != CHAT_MSG_EMOTE && type != CHAT_MSG_AFK && type != CHAT_MSG_DND)
    {
        recvData >> lang;
    }
    else
    {
        lang = LANG_UNIVERSAL;
    }

    if (lang >= NUM_LANGUAGES)
    {
        LogError("HandleMessagechatOpcode : Player %s chat in unknown language %u!", lang);
        return;
    }

    if (GetPlayer()->IsBanned())
    {
        GetPlayer()->BroadcastMessage("You cannot do that when banned.");
        return;
    }

    // Flood protection
    if (lang != LANG_ADDON && isFloodProtectionTriggered())
    {
        return;
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
            if (isSessionMuted())
            {
                return;
            }
        }
        break;
    }

    std::string msg;
    std::string to;
    std::string channel;
    std::string tmp;

    // Process packet
    uint32_t toLength;
    uint32_t msgLength;
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
            msg = recvData.ReadString(recvData.readBits(9));
            break;
        case CHAT_MSG_WHISPER:
        {
            toLength = recvData.readBits(10);
            msgLength = recvData.readBits(9);
            to = recvData.ReadString(toLength);
            msg = recvData.ReadString(msgLength);
        }
        break;
        case CHAT_MSG_CHANNEL:
        {
            toLength = recvData.readBits(10);
            msgLength = recvData.readBits(9);
            msg = recvData.ReadString(msgLength);
            channel = recvData.ReadString(toLength);
        } break;
        default:
        {
            LOG_ERROR("CHAT: unknown msg type %u, lang: %u", type, lang);
        } break;
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

    if (g_chatFilter->isBlockedOrReplaceWord(msg) == true)
    {
        SystemMessage("Your chat message was blocked by a server-side filter.");
        return;
    }

    Channel* chn = nullptr;

    // set lang to universal if we set it in our config
    Player* playerSender = GetPlayer();
    uint8_t chatTag = 0;

    if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
    {
        LogDebug("Command parsed for player %s, input: '%s'", playerSender->GetName(), msg.c_str());
        return;
    }

    if (playerSender->isGMFlagSet())
    {
        lang = LANG_UNIVERSAL;
        chatTag = 4;
    }
    else
    {
        if (worldConfig.player.isInterfactionChatEnabled)
        {
            lang = LANG_UNIVERSAL;
        }
        else
        {
            switch (type)
            {
                case CHAT_MSG_GUILD:
                case CHAT_MSG_OFFICER:
                {
                    if (worldConfig.player.isInterfactionGuildEnabled)
                        lang = LANG_UNIVERSAL;
                } break;
                case CHAT_MSG_PARTY:
                case CHAT_MSG_PARTY_LEADER:
                case CHAT_MSG_RAID:
                case CHAT_MSG_RAID_LEADER:
                case CHAT_MSG_RAID_WARNING:
                {
                    if (worldConfig.player.isInterfactionGroupEnabled)
                        lang = LANG_UNIVERSAL;
                } break;
                case CHAT_MSG_EMOTE:
                {
                    lang = LANG_UNIVERSAL;
                } break;
                default:
                    break;
            }
        }
    }

    // Main chat message processing
    switch (type)
    {
        case CHAT_MSG_SAY:
        case CHAT_MSG_EMOTE:
        {
            GetPlayer()->sendChatPacket(type, lang, msg.c_str(), _player->GetGUID(), chatTag);

        }
        break;
        case CHAT_MSG_PARTY:
        case CHAT_MSG_PARTY_LEADER:
        case CHAT_MSG_RAID:
        case CHAT_MSG_RAID_LEADER:
        case CHAT_MSG_RAID_WARNING:
        {
            Group* pGroup = _player->GetGroup();
            if (pGroup == nullptr) break;

            if (GetPlayer()->m_modlanguage >= LANG_UNIVERSAL)
                data = sChatHandler.FillMessageData(type, GetPlayer()->m_modlanguage, msg.c_str(), _player->GetGUID(), chatTag);
            else if (lang == LANG_UNIVERSAL && worldConfig.player.isInterfactionChatEnabled)
                data = sChatHandler.FillMessageData(type, (CanUseCommand('0') && lang != LANG_ADDON) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->GetGUID(), chatTag);
            else
                data = sChatHandler.FillMessageData(type, (CanUseCommand('c') && lang != LANG_ADDON) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->GetGUID(), chatTag);
            if (type == CHAT_MSG_PARTY && pGroup->GetGroupType() == GROUP_TYPE_RAID)
            {
                // only send to that subgroup
                SubGroup* sgr = _player->GetGroup() ? _player->GetGroup()->GetSubGroup(_player->GetSubGroup()) : 0;

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
            if (_player->m_playerInfo->m_guild)
                sGuildMgr.getGuildById(_player->m_playerInfo->m_guild)->broadcastToGuild(this, false, msg.c_str(), lang);
        }
        break;
        case CHAT_MSG_OFFICER:
        {
            if (_player->m_playerInfo->m_guild)
                sGuildMgr.getGuildById(_player->m_playerInfo->m_guild)->broadcastToGuild(this, true, msg.c_str(), lang);
        }
        break;
        case CHAT_MSG_YELL:
        {
            if (lang > LANG_UNIVERSAL && LanguageSkills[lang] && _player->_HasSkillLine(LanguageSkills[lang]) == false)
                return;

            if (lang == LANG_UNIVERSAL && worldConfig.player.isInterfactionChatEnabled)
            {
                data = sChatHandler.FillMessageData(CHAT_MSG_YELL, (CanUseCommand('0') && lang != LANG_ADDON) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->GetGUID(), chatTag);
            }
            else if (GetPlayer()->m_modlanguage >= LANG_UNIVERSAL)
            {
                data = sChatHandler.FillMessageData(CHAT_MSG_YELL, GetPlayer()->m_modlanguage, msg.c_str(), _player->GetGUID(), chatTag);
            }
            else
            {
                data = sChatHandler.FillMessageData(CHAT_MSG_YELL, (CanUseCommand('c') && lang != LANG_ADDON) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->GetGUID(), chatTag);
            }

            _player->GetMapMgr()->SendChatMessageToCellPlayers(_player, data, 2, 1, lang, this);
            delete data;
        }
        break;
        case CHAT_MSG_WHISPER:
        {
            PlayerCache* playercache = objmgr.GetPlayerCache(to.c_str(), false);
            if (playercache == nullptr)
            {
                data = new WorldPacket(SMSG_CHAT_PLAYER_NOT_FOUND, to.length() + 1);
                *data << to;
                SendPacket(data);
                delete data;
                break;
            }

            if (_player->GetTeamInitial() != playercache->GetUInt32Value(CACHE_PLAYER_INITIALTEAM) && !worldConfig.player.isInterfactionChatEnabled && !playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM) && !_player->isGMFlagSet())
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
                data = sChatHandler.FillMessageData(CHAT_MSG_IGNORED, LANG_UNIVERSAL, msg.c_str(), playercache->GetGUID(), chatTag);
                SendPacket(data);
                delete data;
                playercache->DecRef();
                break;
            }
            else
            {
                data = sChatHandler.FillMessageData(CHAT_MSG_WHISPER, lang, msg.c_str(), _player->GetGUID(), chatTag);
                playercache->SendPacket(data);
            }


            //Sent the to Users id as the channel, this should be fine as it's not used for whisper
            if (lang != LANG_ADDON) //DO NOT SEND if its an addon message!
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

                data = sChatHandler.FillMessageData(CHAT_MSG_AFK, LANG_UNIVERSAL, reason.c_str(), playercache->GetGUID(), chatTag);
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
            chn = channelmgr.GetChannel(channel.c_str(), GetPlayer());
            if (chn)
            {
                //g_chatFilter->ParseEscapeCodes((char*)pMsg, (chn->m_flags & CHANNEL_PACKET_ALLOWLINKS)>0);
                chn->Say(GetPlayer(), msg.c_str(), nullptr, false);
            }
        }
        break;
        case CHAT_MSG_AFK:
        {
            std::string reason = "";
            recvData >> reason;

            GetPlayer()->SetAFKReason(reason);

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
            recvData >> reason;
            GetPlayer()->SetAFKReason(reason);

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
            if (_player->m_bg != nullptr)
            {
                data = sChatHandler.FillMessageData(type, lang, msg.c_str(), _player->GetGUID());
                _player->m_bg->DistributePacketToTeam(data, _player->GetTeam());
                delete data;
            }
        }
        break;
    }
}

void WorldSession::HandleEmoteOpcode(WorldPacket& recv_data)
{
    if (_player->isAlive() == false)
    {
        return;
    }

    uint32_t emote;
    recv_data >> emote;

    _player->Emote((EmoteType)emote);
    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE, emote, 0, 0);

    uint64_t guid = _player->GetGUID();
    sQuestMgr.OnPlayerEmote(_player, emote, guid);
}

void WorldSession::HandleTextEmoteOpcode(WorldPacket& recv_data)
{
    if (_player->isAlive() == false)
    {
        return;
    }

    uint64_t guid;
    uint32_t textEmote;
    uint32_t emoteNum;

    recv_data >> textEmote;
    recv_data >> emoteNum;
    recv_data >> guid;

    if (isSessionMuted() || isFloodProtectionTriggered())
    {
        return;
    }

    const char* name = " ";
    uint32_t namelen = 1;

    Unit* unit = _player->GetMapMgr()->GetUnit(guid);
    if (unit)
    {
        if (unit->IsPlayer())
        {
            name = static_cast<Player*>(unit)->GetName();
            namelen = (uint32_t)strlen(name) + 1;
        }
        else if (unit->IsPet())
        {
            name = static_cast<Pet*>(unit)->GetName().c_str();
            namelen = (uint32_t)strlen(name) + 1;
        }
        else
        {
            Creature* p = static_cast<Creature*>(unit);
            name = p->GetCreatureProperties()->Name.c_str();
            namelen = (uint32_t)strlen(name) + 1;
        }
    }

    DBC::Structures::EmotesTextEntry const* emoteTextEntry = sEmotesTextStore.LookupEntry(textEmote);
    if (emoteTextEntry == nullptr)
    {
        return;
    }

    sHookInterface.OnEmote(_player, (EmoteType)emoteTextEntry->textid, unit);
    if (unit)
    {
        CALL_SCRIPT_EVENT(unit, OnEmote)(_player, (EmoteType)emoteTextEntry->textid);
    }

    switch (emoteTextEntry->textid)
    {
        case EMOTE_STATE_READ:
        case EMOTE_STATE_DANCE:
        {
            _player->SetEmoteState(emoteTextEntry->textid);
        } break;
        case EMOTE_STATE_SLEEP:
        case EMOTE_STATE_SIT:
        case EMOTE_STATE_KNEEL:
        case EMOTE_ONESHOT_NONE:
            break;
        default:
        {
            _player->Emote((EmoteType)emoteTextEntry->textid);
        } break;
    }

    WorldPacket data(SMSG_TEXT_EMOTE, 28 + namelen);
    data << uint64_t(GetPlayer()->GetGUID());
    data << uint32_t(textEmote);
    data << uint32_t(emoteNum);
    data << uint32_t(namelen);
    if (namelen > 1)
    {
        data.append(name, namelen);
    }
    else
    {
        data << uint8_t(0x00);
    }

    GetPlayer()->SendMessageToSet(&data, true);

    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE, textEmote, 0, 0);

    sQuestMgr.OnPlayerEmote(_player, textEmote, guid);
}
