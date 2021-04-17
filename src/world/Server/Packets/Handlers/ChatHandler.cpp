/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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
#include "Management/Guild/GuildMgr.hpp"
#include "Server/OpcodeTable.hpp"

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
                _player->BroadcastMessage("Your message has triggered serverside flood protection. You can speak again in %ld seconds.",
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
#endif

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

    uint8_t gmFlag = _player->isGMFlagSet() ? 4 : 0;
    switch (srlPacket.type)
    {
        case CHAT_MSG_EMOTE:
            // TODO Verify "strange gestures" for xfaction
            _player->SendMessageToSet(SmsgMessageChat(CHAT_MSG_EMOTE, messageLanguage, gmFlag, srlPacket.message, _player->getGuid()).serialise().get(), true, true);
            LogDetail("[emote] %s: %s", _player->getName().c_str(), srlPacket.message.c_str());
            break;
        case CHAT_MSG_SAY:
            if (is_gm_command || !player_can_speak_language)
                break;

            _player->SendMessageToSet(SmsgMessageChat(CHAT_MSG_SAY, messageLanguage, gmFlag, srlPacket.message, _player->getGuid()).serialise().get(), true);
            break;
        case CHAT_MSG_PARTY:
        case CHAT_MSG_PARTY_LEADER:
        case CHAT_MSG_RAID:
        case CHAT_MSG_RAID_LEADER:
        case CHAT_MSG_RAID_WARNING:
        {
            if (is_gm_command || !player_can_speak_language)
                break;

            const auto send_packet = SmsgMessageChat(static_cast<uint8_t>(srlPacket.type), messageLanguage, gmFlag, srlPacket.message, _player->getGuid()).serialise();

            if (const auto group = _player->getGroup())
            {
                if (srlPacket.type == CHAT_MSG_PARTY || srlPacket.type == CHAT_MSG_PARTY_LEADER
                    && group->isRaid())
                {
                    if (const auto subgroup = group->GetSubGroup(_player->getSubGroupSlot()))
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

            if (const auto guild = _player->getGuild())
                guild->broadcastToGuild(this, false, srlPacket.message, messageLanguage);
            break;
        case CHAT_MSG_OFFICER:
            if (is_gm_command)
                break;

            if (const auto guild = _player->getGuild())
                guild->broadcastToGuild(this, true, srlPacket.message, messageLanguage);
            break;
        case CHAT_MSG_YELL:
        {
            if (is_gm_command || !player_can_speak_language)
                break;

            auto yell_packet = SmsgMessageChat(CHAT_MSG_YELL, messageLanguage, gmFlag, srlPacket.message, _player->getGuid());
            _player->GetMapMgr()->SendChatMessageToCellPlayers(_player, yell_packet.serialise().get(), 2, 1, messageLanguage, this);
        }
        break;
        case CHAT_MSG_WHISPER:
        {
            if (const auto playerTarget = sObjectMgr.GetPlayer(srlPacket.destination.c_str(), false))
            {
                const auto target_is_our_faction = _player->getInitialTeam() == playerTarget->getInitialTeam();
                const auto target_is_gm_flagged = playerTarget->hasPlayerFlags(PLAYER_FLAG_GM);
                if (target_is_our_faction
                    || worldConfig.player.isInterfactionChatEnabled
                    || target_is_gm_flagged)
                {
                    const auto target_gm_is_speaking_to_us = playerTarget->isOnGMTargetList(_player->getGuidLow());
                    if (!gmFlag && target_is_gm_flagged && target_gm_is_speaking_to_us)
                    {
                        const auto reply = "SYSTEM: This Game Master does not currently have an open ticket from you and did not receive your whisper. Please submit a new GM Ticket request if you need to speak to a GM. This is an automatic message.";
                        SendPacket(SmsgMessageChat(CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, gmFlag, reply, playerTarget->getGuid()).serialise().get());
                        break;
                    }

                    const auto we_are_being_ignored = playerTarget->isIgnored(_player->getGuidLow());
                    if (we_are_being_ignored)
                    {
                        SendPacket(SmsgMessageChat(CHAT_MSG_IGNORED, LANG_UNIVERSAL, gmFlag, srlPacket.message, playerTarget->getGuid()).serialise().get());
                        break;
                    }

                    playerTarget->SendPacket(SmsgMessageChat(CHAT_MSG_WHISPER, messageLanguage, gmFlag, srlPacket.message, _player->getGuid()).serialise().get());
                    if (messageLanguage != LANG_ADDON)
                        // TODO Verify should this be LANG_UNIVERSAL?
                        SendPacket(SmsgMessageChat(CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, gmFlag, srlPacket.message, playerTarget->getGuid()).serialise().get());

                    if (playerTarget->hasPlayerFlags(PLAYER_FLAG_AFK))
                    {
                        std::string reason = playerTarget->getAFKReason();
                        SendPacket(SmsgMessageChat(CHAT_MSG_AFK, LANG_UNIVERSAL, gmFlag, reason, playerTarget->getGuid()).serialise().get());
                    }
                    else if (playerTarget->hasPlayerFlags(PLAYER_FLAG_DND))
                    {
                        std::string reason = playerTarget->getAFKReason();
                        SendPacket(SmsgMessageChat(CHAT_MSG_DND, LANG_UNIVERSAL, gmFlag, reason, playerTarget->getGuid()).serialise().get());
                    }
                }
            }
            else
            {
                SendPacket(SmsgChatPlayerNotFound(srlPacket.destination).serialise().get());
            }
        } break;
        case CHAT_MSG_CHANNEL:
            if (is_gm_command)
                break;

            if (const auto channel = sChannelMgr.getChannel(srlPacket.destination, _player))
                channel->Say(_player, srlPacket.message.c_str(), nullptr, false);

            break;
        case CHAT_MSG_AFK:
            _player->setAFKReason(srlPacket.message);
            _player->toggleAfk();
            break;
        case CHAT_MSG_DND:
            _player->setAFKReason(srlPacket.message);
            _player->toggleDnd();
            break;
        case CHAT_MSG_BATTLEGROUND:
        case CHAT_MSG_BATTLEGROUND_LEADER:
            if (is_gm_command || !player_can_speak_language)
                break;

            if (!_player->m_bg)
                break;

            _player->m_bg->DistributePacketToTeam(SmsgMessageChat(static_cast<uint8_t>(srlPacket.type), messageLanguage, gmFlag, srlPacket.message, _player->getGuid()).serialise().get(), _player->getTeam());
            break;
    }
}

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
            _player->emote(static_cast<EmoteType>(emoteTextEntry->textid));
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

    _player->emote(static_cast<EmoteType>(srlPacket.emote));

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

void WorldSession::handleChatIgnoredOpcode(WorldPacket& recvPacket)
{
    CmsgChatIgnored srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto player = sObjectMgr.GetPlayer(srlPacket.guid.getGuidLow());
    if (player == nullptr || player->GetSession() == nullptr)
        return;

    player->GetSession()->SendPacket(SmsgMessageChat(CHAT_MSG_IGNORED, LANG_UNIVERSAL, 0, _player->getName(), _player->getGuid()).serialise().get());
}

void WorldSession::handleChatChannelWatchOpcode(WorldPacket& recvPacket)
{
    CmsgSetChannelWatch srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Unhandled... Player %s watch channel: %s", _player->getName().c_str(), srlPacket.name.c_str());
}