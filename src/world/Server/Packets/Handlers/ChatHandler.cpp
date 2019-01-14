/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
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
#include "Server/Packets/CmsgComplaint.h"
#include "Server/Packets/SmsgComplainResult.h"
#include "Server/Packets/CmsgChatIgnored.h"
#include "Server/Packets/CmsgSetChannelWatch.h"
#include "Management/GuildMgr.h"

#if VERSION_STRING >= Cata
#include "Spell/SpellAuras.h"
#endif

using namespace AscEmu::Packets;

extern std::string LogFileName;
extern bool bLogChat;

bool WorldSession::isSessionMuted()
{
    if (m_muted && m_muted >= static_cast<uint32_t>(UNIXTIME))
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
                _player->BroadcastMessage("Your message has triggered serverside flood protection. You can speak again in %u seconds.",
                    floodTime - UNIXTIME);
            }
            return true;
        }
    }

    return false;
}

#if VERSION_STRING < Cata
static const uint32_t LanguageSkills[NUM_LANGUAGES] =
{
    0,          // UNIVERSAL        0x00
    109,        // ORCISH           0x01
    113,        // DARNASSIAN       0x02
    115,        // TAURAHE          0x03
    0,          // -                0x04
    0,          // -                0x05
    111,        // DWARVISH         0x06
    98,         // COMMON           0x07
    139,        // DEMON TONGUE     0x08
    140,        // TITAN            0x09
    137,        // THALSSIAN        0x0A
    138,        // DRACONIC         0x0B
    0,          // KALIMAG          0x0C
    313,        // GNOMISH          0x0D
    315,        // TROLL            0x0E
    0,          // -                0x0F
    0,          // -                0x10
    0,          // -                0x11
    0,          // -                0x12
    0,          // -                0x13
    0,          // -                0x14
    0,          // -                0x15
    0,          // -                0x16
    0,          // -                0x17
    0,          // -                0x18
    0,          // -                0x19
    0,          // -                0x1A
    0,          // -                0x1B
    0,          // -                0x1C
    0,          // -                0x1D
    0,          // -                0x1E
    0,          // -                0x1F
    0,          // -                0x20
    673,        // -                0x21
    0,          // -                0x22
    759,        // -                0x23
};

void WorldSession::handleMessageChatOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgMessageChat srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    auto messageLanguage = srlPacket.language;
    auto player_can_speak_language = true;

    if (messageLanguage != LANG_ADDON)
    {
        if (const auto language_skill = LanguageSkills[messageLanguage])
            player_can_speak_language = _player->_HasSkillLine(language_skill);
    }

    if (messageLanguage != LANG_ADDON)
    {
        if (worldConfig.player.isInterfactionChatEnabled)
        {
            messageLanguage = LANG_UNIVERSAL;
            player_can_speak_language = true;
        }

        if (_player->m_modlanguage >= 0)
        {
            messageLanguage = _player->m_modlanguage;
            player_can_speak_language = true;
        }

        // GMs speak universal language
        if (GetPermissionCount() > 0)
        {
            messageLanguage = LANG_UNIVERSAL;
            player_can_speak_language = true;
        }
    }

    if (isSessionMuted() && isFloodProtectionTriggered())
        return;

    switch (srlPacket.type)
    {
        case CHAT_MSG_EMOTE:
        case CHAT_MSG_SAY:
        case CHAT_MSG_YELL:
        case CHAT_MSG_WHISPER:
        case CHAT_MSG_CHANNEL:
            if (m_muted >= static_cast<uint32_t>(UNIXTIME))
            {
                SystemMessage("You are currently muted by a moderator.");
                return;
            }
            break;
        default:
            break;
    }

    if (int(srlPacket.message.find("|T")) > -1)
    {
        //_player->BroadcastMessage("Don't even THINK about doing that again");
        return;
    }

    if (!sHookInterface.OnChat(_player, srlPacket.type, srlPacket.language, srlPacket.message.c_str(), srlPacket.destination.c_str()))
        return;

    if (g_chatFilter->isBlockedOrReplaceWord(srlPacket.message))
    {
        SystemMessage("Your chat message was blocked by a server-side filter.");
        return;
    }

    auto is_gm_command = false;
    switch (srlPacket.type)
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
            is_gm_command = sChatHandler.ParseCommands(srlPacket.message.c_str(), this) > 0;
            break;
    }

    switch (srlPacket.type)
    {
        case CHAT_MSG_EMOTE:
            // TODO Verify "strange gestures" for xfaction
            _player->SendMessageToSet(SmsgMessageChat(CHAT_MSG_EMOTE, messageLanguage, _player->getGuid(), srlPacket.message, _player->isGMFlagSet()).serialise().get(), true, true);
            LogDetail("[emote] %s: %s", _player->getName().c_str(), srlPacket.message.c_str());
            break;
        case CHAT_MSG_SAY:
            if (is_gm_command || !player_can_speak_language)
                break;

            _player->SendMessageToSet(SmsgMessageChat(CHAT_MSG_SAY, messageLanguage, _player->getGuid(), srlPacket.message, _player->isGMFlagSet()).serialise().get(), true);
            break;
        case CHAT_MSG_PARTY:
        case CHAT_MSG_PARTY_LEADER:
        case CHAT_MSG_RAID:
        case CHAT_MSG_RAID_LEADER:
        case CHAT_MSG_RAID_WARNING:
        {
            if (is_gm_command || !player_can_speak_language)
                break;

            const auto send_packet = SmsgMessageChat(static_cast<uint8_t>(srlPacket.type), messageLanguage, _player->getGuid(), srlPacket.message, _player->isGMFlagSet()).serialise();

            if (const auto group = _player->GetGroup())
            {
                if (srlPacket.type == CHAT_MSG_PARTY || srlPacket.type == CHAT_MSG_PARTY_LEADER
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
                LogDetail("[party] %s: %s", _player->getName().c_str(), srlPacket.message.c_str());
            }
        }
        break;
        case CHAT_MSG_GUILD:
            if (is_gm_command)
                break;

            if (const auto guild = _player->GetGuild())
                guild->broadcastToGuild(this, false, srlPacket.message, messageLanguage);
            break;
        case CHAT_MSG_OFFICER:
            if (is_gm_command)
                break;

            if (const auto guild = _player->GetGuild())
                guild->broadcastToGuild(this, true, srlPacket.message, messageLanguage);
            break;
        case CHAT_MSG_YELL:
        {
            if (is_gm_command || !player_can_speak_language)
                break;

            auto yell_packet = SmsgMessageChat(CHAT_MSG_YELL, messageLanguage, _player->getGuid(), srlPacket.message,
                _player->isGMFlagSet());
            _player->GetMapMgr()->SendChatMessageToCellPlayers(_player, yell_packet.serialise().get(), 2, 1, messageLanguage,
                this);
        }
        break;
        case CHAT_MSG_WHISPER:
            if (const auto player_cache = objmgr.GetPlayerCache(srlPacket.destination.c_str(), false))
            {
                const auto target_is_our_faction = _player->getInitialTeam() == player_cache->GetUInt32Value(CACHE_PLAYER_INITIALTEAM);
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
                        SendPacket(SmsgMessageChat(CHAT_MSG_IGNORED, LANG_UNIVERSAL, player_cache->GetGUID(), srlPacket.message, we_are_gm_flagged).serialise().get());
                        player_cache->DecRef();
                        break;
                    }

                    player_cache->SendPacket(*SmsgMessageChat(CHAT_MSG_WHISPER, messageLanguage, _player->getGuid(), srlPacket.message, we_are_gm_flagged).serialise().get());
                    if (messageLanguage != LANG_ADDON)
                        // TODO Verify should this be LANG_UNIVERSAL?
                        SendPacket(SmsgMessageChat(CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, player_cache->GetGUID(), srlPacket.message, we_are_gm_flagged).serialise().get());

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
                SendPacket(SmsgChatPlayerNotFound(srlPacket.destination).serialise().get());
            }
            break;
        case CHAT_MSG_CHANNEL:
            if (is_gm_command)
                break;

            if (const auto channel = channelmgr.GetChannel(srlPacket.destination.c_str(), _player))
                channel->Say(_player, srlPacket.message.c_str(), nullptr, false);

            break;
        case CHAT_MSG_AFK:
            _player->SetAFKReason(srlPacket.message);
            _player->toggleAfk();
            break;
        case CHAT_MSG_DND:
            _player->SetAFKReason(srlPacket.message);
            _player->toggleDnd();
            break;
        case CHAT_MSG_BATTLEGROUND:
        case CHAT_MSG_BATTLEGROUND_LEADER:
            if (is_gm_command || !player_can_speak_language)
                break;

            if (!_player->m_bg)
                break;

            _player->m_bg->DistributePacketToTeam(SmsgMessageChat(static_cast<uint8_t>(srlPacket.type), messageLanguage, _player->getGuid(), srlPacket.message, _player->isGMFlagSet()).serialise().get(), _player->getTeam());
            break;
    }
}
#else
static const uint32_t LanguageSkills[NUM_LANGUAGES] =
{
    0,              // UNIVERSAL          0x00
    109,            // ORCISH             0x01
    113,            // DARNASSIAN         0x02
    115,            // TAURAHE            0x03
    0,              // -                  0x04
    0,              // -                  0x05
    111,            // DWARVISH           0x06
    98,             // COMMON             0x07
    139,            // DEMON TONGUE       0x08
    140,            // TITAN              0x09
    137,            // THALSSIAN          0x0A
    138,            // DRACONIC           0x0B
    0,              // KALIMAG            0x0C
    313,            // GNOMISH            0x0D
    315,            // TROLL              0x0E
    0,              // -                  0x0F
    0,              // -                  0x10
    0,              // -                  0x11
    0,              // -                  0x12
    0,              // -                  0x13
    0,              // -                  0x14
    0,              // -                  0x15
    0,              // -                  0x16
    0,              // -                  0x17
    0,              // -                  0x18
    0,              // -                  0x19
    0,              // -                  0x1A
    0,              // -                  0x1B
    0,              // -                  0x1C
    0,              // -                  0x1D
    0,              // -                  0x1E
    0,              // -                  0x1F
    0,              // -                  0x20
    673,            // GUTTERSPEAK        0x21
    0,              // -                  0x22
    759,            // DRAENEI            0x23
    0,              // ZOMBIE             0x24
    0,              // GNOMISH_BINAR      0x25
    0,              // GOBLIN_BINARY      0x26
    791,            // WORGEN             0x27
    792,            // GOBLIN             0x28
};

struct OpcodeToChatType
{
    uint16_t opcode;
    uint8_t chatType;
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

uint8_t getMessageTypeForOpcode(uint16_t opcode)
{
    for (auto& i : opcodeToChatTypeList)
    {
        if (i.opcode == opcode)
            return i.chatType;
    }

    return 0xFF;
}


void WorldSession::handleMessageChatOpcode(WorldPacket& recvPacket)
{
    WorldPacket* data = nullptr;

    int32_t lang;

    const char* pMisc = nullptr;
    const char* pMsg = nullptr;

    const uint8_t type = getMessageTypeForOpcode(recvPacket.GetOpcode());
    if (type == 0xFF)
    {
        LogError("HandleMessagechatOpcode : Unknown chat opcode (0x%X)", recvPacket.GetOpcode());
        recvPacket.clear();
        return;
    }

    if (type != CHAT_MSG_EMOTE && type != CHAT_MSG_AFK && type != CHAT_MSG_DND)
        recvPacket >> lang;
    else
        lang = LANG_UNIVERSAL;

    if (lang >= NUM_LANGUAGES)
    {
        LogError("HandleMessagechatOpcode : Player %s chat in unknown language %u!", lang);
        return;
    }

    if (_player->IsBanned())
    {
        _player->BroadcastMessage("You cannot do that when banned.");
        return;
    }

    if (lang != LANG_ADDON && isFloodProtectionTriggered())
        return;

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
                return;
        }
        break;
    }

    std::string msg;
    std::string to;
    std::string channel;
    std::string tmp;

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
            msg = recvPacket.ReadString(recvPacket.readBits(9));
            break;
        case CHAT_MSG_WHISPER:
        {
            toLength = recvPacket.readBits(10);
            msgLength = recvPacket.readBits(9);
            to = recvPacket.ReadString(toLength);
            msg = recvPacket.ReadString(msgLength);
        }
        break;
        case CHAT_MSG_CHANNEL:
        {
            toLength = recvPacket.readBits(10);
            msgLength = recvPacket.readBits(9);
            msg = recvPacket.ReadString(msgLength);
            channel = recvPacket.ReadString(toLength);
        } break;
        default:
        {
            LOG_ERROR("CHAT: unknown msg type %u, lang: %u", type, lang);
        } break;
    }


    if (int(msg.find("|T")) > -1)
    {
        _player->BroadcastMessage("Don't even THINK about doing that again");
        return;
    }

    if (pMsg && !sHookInterface.OnChat(_player, type, lang, pMsg, pMisc))
        return;

    if (_player->getAuraWithAuraEffect(SPELL_AURA_COMPREHEND_LANG))
    {
        LOG_ERROR("Player has no aura with effect SPELL_AURA_COMPREHEND_LANG... returning now!");
        return;
    }

    if (g_chatFilter->isBlockedOrReplaceWord(msg))
    {
        SystemMessage("Your chat message was blocked by a server-side filter.");
        return;
    }

    Channel* chn = nullptr;

    Player* playerSender = _player;
    uint8_t chatTag = 0;

    if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
    {
        LogDebug("Command parsed for player %s, input: '%s'", playerSender->getName().c_str(), msg.c_str());
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

    switch (type)
    {
        case CHAT_MSG_SAY:
        case CHAT_MSG_EMOTE:
        {
            _player->sendChatPacket(type, lang, msg.c_str(), _player->getGuid(), chatTag);
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

            if (_player->m_modlanguage >= LANG_UNIVERSAL)
                data = sChatHandler.FillMessageData(type, _player->m_modlanguage, msg.c_str(), _player->getGuid(), chatTag);
            else if (lang == LANG_UNIVERSAL && worldConfig.player.isInterfactionChatEnabled)
                data = sChatHandler.FillMessageData(type, (CanUseCommand('0') && lang != LANG_ADDON) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->getGuid(), chatTag);
            else
                data = sChatHandler.FillMessageData(type, (CanUseCommand('c') && lang != LANG_ADDON) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->getGuid(), chatTag);
            if (type == CHAT_MSG_PARTY && pGroup->getGroupType() == GROUP_TYPE_RAID)
            {
                SubGroup* sgr = _player->GetGroup() ? _player->GetGroup()->GetSubGroup(_player->GetSubGroup()) : 0;
                if (sgr)
                {
                    _player->GetGroup()->Lock();
                    for (auto itr = sgr->GetGroupMembersBegin(); itr != sgr->GetGroupMembersEnd(); ++itr)
                    {
                        if ((*itr)->m_loggedInPlayer)
                            (*itr)->m_loggedInPlayer->GetSession()->SendChatPacket(data, 1, lang, this);
                    }
                    _player->GetGroup()->Unlock();
                }
            }
            else
            {
                SubGroup* sgr;
                for (uint32_t i = 0; i < _player->GetGroup()->GetSubGroupCount(); ++i)
                {
                    sgr = _player->GetGroup()->GetSubGroup(i);
                    _player->GetGroup()->Lock();
                    for (auto itr = sgr->GetGroupMembersBegin(); itr != sgr->GetGroupMembersEnd(); ++itr)
                    {
                        if ((*itr)->m_loggedInPlayer)
                            (*itr)->m_loggedInPlayer->GetSession()->SendChatPacket(data, 1, lang, this);
                    }
                    _player->GetGroup()->Unlock();

                }
            }
            LogDetail("[party] %s: %s", _player->getName().c_str(), msg.c_str());
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
                data = sChatHandler.FillMessageData(CHAT_MSG_YELL, (CanUseCommand('0') && lang != LANG_ADDON) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->getGuid(), chatTag);
            else if (_player->m_modlanguage >= LANG_UNIVERSAL)
                data = sChatHandler.FillMessageData(CHAT_MSG_YELL, _player->m_modlanguage, msg.c_str(), _player->getGuid(), chatTag);
            else
                data = sChatHandler.FillMessageData(CHAT_MSG_YELL, (CanUseCommand('c') && lang != LANG_ADDON) ? LANG_UNIVERSAL : lang, msg.c_str(), _player->getGuid(), chatTag);

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

            if (_player->getInitialTeam() != playercache->GetUInt32Value(CACHE_PLAYER_INITIALTEAM) && !worldConfig.player.isInterfactionChatEnabled && !playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM) && !_player->isGMFlagSet())
            {
                WorldPacket response(SMSG_CHAT_PLAYER_NOT_FOUND, to.length() + 1);
                response << to;
                SendPacket(&response);
                playercache->DecRef();
                break;
            }

            if (!HasPermissions() && playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM) && playercache->CountValue64(CACHE_GM_TARGETS, _player->getGuid()) == 0)
            {
                std::string Reply = "The Game Master can not receive messages from you. Please submit a Ticket request if you need to speak to a GM.";
                data = sChatHandler.FillMessageData(CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, Reply.c_str(), playercache->GetGUID(), 4);
                SendPacket(data);
                delete data;
                playercache->DecRef();
                break;
            }

            if (playercache->CountValue64(CACHE_SOCIAL_IGNORELIST, _player->getGuidLow()) > 0)
            {
                data = sChatHandler.FillMessageData(CHAT_MSG_IGNORED, LANG_UNIVERSAL, msg.c_str(), playercache->GetGUID(), chatTag);
                SendPacket(data);
                delete data;
                playercache->DecRef();
                break;
            }
            else
            {
                data = sChatHandler.FillMessageData(CHAT_MSG_WHISPER, lang, msg.c_str(), _player->getGuid(), chatTag);
                playercache->SendPacket(data);
            }

            if (lang != LANG_ADDON)
            {
                data = sChatHandler.FillMessageData(CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, msg.c_str(), playercache->GetGUID(), playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM) ? 4 : 0);
                SendPacket(data);
                delete data;
            }

            if (playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_AFK))
            {
                std::string reason;
                playercache->GetStringValue(CACHE_AFK_DND_REASON, reason);

                data = sChatHandler.FillMessageData(CHAT_MSG_AFK, LANG_UNIVERSAL, reason.c_str(), playercache->GetGUID(), chatTag);
                SendPacket(data);
                delete data;
            }
            else if (playercache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_DND))
            {
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
            chn = channelmgr.GetChannel(channel.c_str(), _player);
            if (chn)
                chn->Say(_player, msg.c_str(), nullptr, false);
        }
        break;
        case CHAT_MSG_AFK:
        {
            std::string reason;
            recvPacket >> reason;

            _player->SetAFKReason(reason);

            if (_player->hasPlayerFlags(PLAYER_FLAG_AFK))
            {
                _player->removePlayerFlags(PLAYER_FLAG_AFK);
                if (worldConfig.getKickAFKPlayerTime())
                    sEventMgr.RemoveEvents(_player, EVENT_PLAYER_SOFT_DISCONNECT);
            }
            else
            {
                _player->addPlayerFlags(PLAYER_FLAG_AFK);

                if (_player->m_bg)
                    _player->m_bg->RemovePlayer(_player, false);

                if (worldConfig.getKickAFKPlayerTime())
                    sEventMgr.AddEvent(_player, &Player::SoftDisconnect, EVENT_PLAYER_SOFT_DISCONNECT, worldConfig.getKickAFKPlayerTime(), 1, 0);
            }
        }
        break;
        case CHAT_MSG_DND:
        {
            std::string reason;
            recvPacket >> reason;
            _player->SetAFKReason(reason);

            if (_player->hasPlayerFlags(PLAYER_FLAG_DND))
                _player->removePlayerFlags(PLAYER_FLAG_DND);
            else
                _player->addPlayerFlags(PLAYER_FLAG_DND);
        }
        break;

        case CHAT_MSG_BATTLEGROUND:
        case CHAT_MSG_BATTLEGROUND_LEADER:
        {
            if (_player->m_bg != nullptr)
            {
                data = sChatHandler.FillMessageData(type, lang, msg.c_str(), _player->getGuid());
                _player->m_bg->DistributePacketToTeam(data, _player->getTeam());
                delete data;
            }
        }
        break;
    }
}
#endif

#if VERSION_STRING < Cata
void WorldSession::handleTextEmoteOpcode(WorldPacket& recvPacket)
{
    if (!_player->isAlive())
        return;

    CmsgTextEmote srlPacket;
    if (!srlPacket.deserialise(recvPacket))
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

    auto unit = _player->GetMapMgr()->GetUnit(srlPacket.guid);
    if (unit != nullptr)
    {
        if (unit->isPlayer())
            unitName = dynamic_cast<Player*>(unit)->getName();
        else if (unit->isPet())
            unitName = dynamic_cast<Pet*>(unit)->GetName();
        else
            unitName = dynamic_cast<Creature*>(unit)->GetCreatureProperties()->Name;

        nameLength = static_cast<uint32_t>(unitName.length() + 1);
    }

    auto emoteTextEntry = sEmotesTextStore.LookupEntry(srlPacket.text_emote);
    if (emoteTextEntry)
    {
        sHookInterface.OnEmote(_player, static_cast<EmoteType>(emoteTextEntry->textid), unit);
        if (unit)
            CALL_SCRIPT_EVENT(unit, OnEmote)(_player, static_cast<EmoteType>(emoteTextEntry->textid));

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

        _player->SendMessageToSet(SmsgEmote(emoteTextEntry->textid, _player->getGuid()).serialise().get(), true);

        _player->SendMessageToSet(SmsgTextEmote(nameLength, unitName, srlPacket.text_emote, _player->getGuid(), srlPacket.unk).serialise().get(), true);

#if VERSION_STRING > TBC
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE, srlPacket.text_emote, 0, 0);
#endif
        sQuestMgr.OnPlayerEmote(_player, srlPacket.text_emote, srlPacket.guid);
    }
}
#else
void WorldSession::handleTextEmoteOpcode(WorldPacket& recvPacket)
{
    if (!_player->isAlive())
        return;

    CmsgTextEmote srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (isSessionMuted() || isFloodProtectionTriggered())
        return;

    const char* unitName = " ";
    uint32_t nameLength = 1;

    Unit* unit = _player->GetMapMgr()->GetUnit(srlPacket.guid);
    if (unit)
    {
        if (unit->isPlayer())
        {
            unitName = dynamic_cast<Player*>(unit)->getName().c_str();
            nameLength = static_cast<uint32_t>(strlen(unitName)) + 1;
        }
        else if (unit->isPet())
        {
            unitName = dynamic_cast<Pet*>(unit)->GetName().c_str();
            nameLength = static_cast<uint32_t>(strlen(unitName)) + 1;
        }
        else
        {
            auto creature = dynamic_cast<Creature*>(unit);
            unitName = creature->GetCreatureProperties()->Name.c_str();
            nameLength = static_cast<uint32_t>(strlen(unitName)) + 1;
        }
    }

    DBC::Structures::EmotesTextEntry const* emoteTextEntry = sEmotesTextStore.LookupEntry(srlPacket.text_emote);
    if (emoteTextEntry == nullptr)
        return;

    sHookInterface.OnEmote(_player, static_cast<EmoteType>(emoteTextEntry->textid), unit);
    if (unit)
        CALL_SCRIPT_EVENT(unit, OnEmote)(_player, static_cast<EmoteType>(emoteTextEntry->textid));

    switch (emoteTextEntry->textid)
    {
        case EMOTE_STATE_READ:
        case EMOTE_STATE_DANCE:
        {
            _player->setEmoteState(emoteTextEntry->textid);
        } break;
        case EMOTE_STATE_SLEEP:
        case EMOTE_STATE_SIT:
        case EMOTE_STATE_KNEEL:
        case EMOTE_ONESHOT_NONE:
            break;
        default:
        {
            _player->Emote(static_cast<EmoteType>(emoteTextEntry->textid));
        } break;
    }

    _player->SendMessageToSet(SmsgTextEmote(nameLength, unitName, srlPacket.text_emote, _player->getGuid(), srlPacket.unk).serialise().get(), true);

    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE, srlPacket.text_emote, 0, 0);

    sQuestMgr.OnPlayerEmote(_player, srlPacket.text_emote, srlPacket.guid);
}
#endif

void WorldSession::handleEmoteOpcode(WorldPacket& recvPacket)
{
    CmsgEmote srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (!_player->isAlive())
        return;

    _player->Emote(static_cast<EmoteType>(srlPacket.emote));

#if VERSION_STRING > TBC
    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE, srlPacket.emote, 0, 0);
#endif

    uint64_t guid = _player->getGuid();
    sQuestMgr.OnPlayerEmote(_player, srlPacket.emote, guid);
}

#if VERSION_STRING < Cata
void WorldSession::handleReportSpamOpcode(WorldPacket& recvPacket)
{
    CmsgComplaint srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LOG_DEBUG("REPORT SPAM: type %u, guid %u, unk1 %u, unk2 %u, unk3 %u, unk4 %u, message %s", srlPacket.spam_type, srlPacket.spammer_guid.getGuidLow(),
        srlPacket.unk1, srlPacket.unk2, srlPacket.unk3, srlPacket.unk4, srlPacket.description.c_str());

    SendPacket(SmsgComplainResult(0).serialise().get());
}
#endif

#if VERSION_STRING < Cata
void WorldSession::handleChatIgnoredOpcode(WorldPacket& recvPacket)
{
    CmsgChatIgnored srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto player = objmgr.GetPlayer(srlPacket.guid.getGuidLow());
    if (player == nullptr || player->GetSession() == nullptr)
        return;

    WorldPacket* data = sChatHandler.FillMessageData(CHAT_MSG_IGNORED, LANG_UNIVERSAL, _player->getName().c_str(), _player->getGuid());
    player->GetSession()->SendPacket(data);
    delete data;
}
#else
void WorldSession::handleChatIgnoredOpcode(WorldPacket& recvPacket)
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

    const auto player = objmgr.GetPlayer(static_cast<uint32_t>(playerGuid));
    if (player == nullptr || player->GetSession() == nullptr)
        return;

    WorldPacket* data = sChatHandler.FillMessageData(CHAT_MSG_IGNORED, LANG_UNIVERSAL, _player->getName().c_str(), _player->getGuid());
    player->GetSession()->SendPacket(data);
    delete data;
}
#endif

void WorldSession::handleChatChannelWatchOpcode(WorldPacket& recvPacket)
{
    CmsgSetChannelWatch srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Unhandled... Player %s watch channel: %s", _player->getName().c_str(), srlPacket.name.c_str());
}