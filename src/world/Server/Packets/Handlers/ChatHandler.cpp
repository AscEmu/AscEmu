/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Management/WordFilter.hpp"
#include "Chat/Channel.hpp"
#include "Chat/ChannelMgr.hpp"
#include "Management/Battleground/Battleground.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Chat/ChatDefines.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Chat/ChatCommandHandler.hpp"

#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Objects/Units/Players/Player.hpp"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestMgr.h"
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
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/HookInterface.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

#if VERSION_STRING >= Cata
#include "Spell/SpellAura.hpp"
#endif

using namespace AscEmu::Packets;

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
    if (!hasPermissions() && worldConfig.chat.linesBeforeProtection)
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
                _player->broadcastMessage("Your message has triggered serverside flood protection. You can speak again in %ld seconds.",
                    floodTime - UNIXTIME);
            }
            return true;
        }
    }

    return false;
}

std::vector<LanguageSkillSpell> languageSpellSkillStore =
{
     { LANG_ADDON,               0,                                   0 }
    ,{ LANG_UNIVERSAL,           0,                                   0 }
    ,{ LANG_ORCISH,              SKILL_LANG_ORCISH,                 669 }
    ,{ LANG_DARNASSIAN,          SKILL_LANG_DARNASSIAN,             671 }
    ,{ LANG_TAURAHE,             SKILL_LANG_TAURAHE,                670 }
    ,{ LANG_DWARVISH,            SKILL_LANG_DWARVEN,                672 }
    ,{ LANG_COMMON,              SKILL_LANG_COMMON,                 668 }
    ,{ LANG_DEMONIC,             SKILL_LANG_DEMON_TONGUE,           815 }
    ,{ LANG_TITAN,               SKILL_LANG_TITAN,                  816 }
    ,{ LANG_THELASSIAN,          SKILL_LANG_THALASSIAN,             813 }
    ,{ LANG_DRACONIC,            SKILL_LANG_DRACONIC,               814 }
    ,{ LANG_KALIMAG,             SKILL_LANG_OLD_TONGUE,             817 }
    ,{ LANG_GNOMISH,             SKILL_LANG_GNOMISH,               7340 }
    ,{ LANG_TROLL,               SKILL_LANG_TROLL,                 7341 }
    ,{ LANG_GUTTERSPEAK,         SKILL_LANG_GUTTERSPEAK,          17737 }
#if VERSION_STRING >= TBC
    ,{ LANG_DRAENEI,             SKILL_LANG_DRAENEI,              29932 }
#if VERSION_STRING >= Cata
    ,{ LANG_ZOMBIE,              0,                                   0 }
    ,{ LANG_GNOMISH_BINARY,      0,                                   0 }
    ,{ LANG_GOBLIN_BINARY,       0,                                   0 }
    ,{ LANG_WORGEN,              SKILL_LANG_GILNEAN,              69270 }
    ,{ LANG_GOBLIN,              SKILL_LANG_GOBLIN,               69269 }
#if VERSION_STRING == Mop
    ,{ LANG_PANDAREN_NEUTRAL,    SKILL_LANG_PANDAREN_NEUTRAL,    108127 }
    ,{ LANG_PANDAREN_ALLIANCE,   SKILL_LANG_PANDAREN_ALLIANCE,   108130 }
    ,{ LANG_PANDAREN_HORDE,      SKILL_LANG_PANDAREN_HORDE,      108131 }
#endif
#endif
#endif
};

LanguageSkillSpell getLanguageSkillSpell(uint8_t language)
{
    for (const auto& languageSkillSpell : languageSpellSkillStore)
    {
        if (languageSkillSpell.languageId == language)
            return languageSkillSpell;
    }
    return { LANG_UNIVERSAL, 0, 0 };
}

void WorldSession::handleMessageChatOpcode(WorldPacket& recvPacket)
{
    CmsgMessageChat srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    auto messageLanguage = srlPacket.language;
    auto player_can_speak_language = true;

    if (messageLanguage != LANG_ADDON)
    {
        if (messageLanguage <= languageSpellSkillStore.size())
        {
            if (auto languageSkill = getLanguageSkillSpell(messageLanguage).skillId)
                player_can_speak_language = _player->hasSkillLine(languageSkill);
        }

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
        if (hasPermissions())
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
        {
            if (m_muted >= static_cast<uint32_t>(UNIXTIME))
            {
                SystemMessage("You are currently muted by a moderator.");
                return;
            }
        } break;
        default:
            break;
    }

    if (srlPacket.message.find("|T") > -1)
    {
        //_player->broadcastMessage("Don't even THINK about doing that again");
        return;
    }

    if (!sHookInterface.OnChat(_player, srlPacket.type, srlPacket.language, srlPacket.message.c_str(), srlPacket.destination.c_str()))
        return;

    if (g_chatFilter->isBlockedOrReplaceWord(srlPacket.message))
    {
        SystemMessage("Your chat message was blocked by a server-side filter.");
        return;
    }

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
        {
            // Parse command and get out of the message handling :-)
            if (sChatHandler.ParseCommands(srlPacket.message.c_str(), this) > 0)
                return;
        }
    }

    uint8_t gmFlag = _player->isGMFlagSet() ? 4 : 0;
    switch (srlPacket.type)
    {
        case CHAT_MSG_EMOTE:
        {
            // TODO Verify "strange gestures" for xfaction
            _player->sendMessageToSet(SmsgMessageChat(CHAT_MSG_EMOTE, messageLanguage, gmFlag, srlPacket.message, _player->getGuid()).serialise().get(), true, true);
            sLogger.info("[emote] {}: {}", _player->getName(), srlPacket.message);
        } break;
        case CHAT_MSG_SAY:
        {
            if (!player_can_speak_language)
                break;

            _player->sendMessageToSet(SmsgMessageChat(CHAT_MSG_SAY, messageLanguage, gmFlag, srlPacket.message, _player->getGuid()).serialise().get(), true);
        } break;
        case CHAT_MSG_PARTY:
        case CHAT_MSG_PARTY_LEADER:
        case CHAT_MSG_RAID:
        case CHAT_MSG_RAID_LEADER:
        case CHAT_MSG_RAID_WARNING:
        {
            if (!player_can_speak_language)
                break;

#if VERSION_STRING >= Cata
            // Use correct type for group/raid leader
            if (srlPacket.type == CHAT_MSG_PARTY || srlPacket.type == CHAT_MSG_RAID)
            {
                if (auto const group = _player->getGroup())
                    if (group->GetLeader() == _player->getPlayerInfo())
                        srlPacket.type = srlPacket.type == CHAT_MSG_PARTY ? CHAT_MSG_PARTY_LEADER : CHAT_MSG_RAID_LEADER;
            }
#endif

            const auto send_packet = SmsgMessageChat(static_cast<uint8_t>(srlPacket.type), messageLanguage, gmFlag, srlPacket.message, _player->getGuid()).serialise();

            if (auto const group = _player->getGroup())
            {
                if (srlPacket.type == CHAT_MSG_PARTY || srlPacket.type == CHAT_MSG_PARTY_LEADER && group->isRaid())
                {
                    if (auto* const subgroup = group->GetSubGroup(_player->getSubGroupSlot()))
                    {
                        group->Lock();
                        for (auto group_member : subgroup->getGroupMembers())
                            if (Player* loggedInPlayer = sObjectMgr.getPlayer(group_member->guid))
                                loggedInPlayer->sendPacket(send_packet.get());
                        group->Unlock();
                    }
                }
                else
                {
                    for (uint32_t i = 0; i < group->GetSubGroupCount(); ++i)
                    {
                        if (auto* const sub_group = group->GetSubGroup(i))
                        {
                            group->Lock();
                            for (auto group_member : sub_group->getGroupMembers())
                                if (Player* loggedInPlayer = sObjectMgr.getPlayer(group_member->guid))
                                    loggedInPlayer->sendPacket(send_packet.get());
                            group->Unlock();
                        }
                    }
                }
                sLogger.info("[party] {}: {}", _player->getName(), srlPacket.message);
            }
        } break;
        case CHAT_MSG_GUILD:
        {
            if (auto* const guild = _player->getGuild())
                guild->broadcastToGuild(this, false, srlPacket.message, messageLanguage);
        } break;
        case CHAT_MSG_OFFICER:
        {
            if (auto* const guild = _player->getGuild())
                guild->broadcastToGuild(this, true, srlPacket.message, messageLanguage);
        } break;
        case CHAT_MSG_YELL:
        {
            if (!player_can_speak_language)
                break;

            auto yell_packet = SmsgMessageChat(CHAT_MSG_YELL, messageLanguage, gmFlag, srlPacket.message, _player->getGuid());
            _player->getWorldMap()->sendChatMessageToCellPlayers(_player, yell_packet.serialise().get(), 2, 1, messageLanguage, this);
        } break;
        case CHAT_MSG_WHISPER:
        {
            if (auto* const playerTarget = sObjectMgr.getPlayer(srlPacket.destination.c_str(), false))
            {
                const auto target_is_our_faction = _player->getInitialTeam() == playerTarget->getInitialTeam();
                const auto target_is_gm_flagged = playerTarget->hasPlayerFlags(PLAYER_FLAG_GM);
                if (target_is_our_faction || worldConfig.player.isInterfactionChatEnabled || target_is_gm_flagged)
                {
                    const auto target_gm_is_speaking_to_us = playerTarget->isOnGMTargetList(_player->getGuidLow());
                    if (!gmFlag && target_is_gm_flagged && target_gm_is_speaking_to_us)
                    {
                        std::string reply = "SYSTEM: This Game Master does not currently have an open ticket from you and did not receive your whisper. Please submit a new GM Ticket request if you need to speak to a GM. This is an automatic message.";
                        SendPacket(SmsgMessageChat(CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, gmFlag, reply, playerTarget->getGuid()).serialise().get());
                        break;
                    }

                    const auto we_are_being_ignored = playerTarget->isIgnored(_player->getGuidLow());
                    if (we_are_being_ignored)
                    {
                        SendPacket(SmsgMessageChat(CHAT_MSG_IGNORED, LANG_UNIVERSAL, gmFlag, srlPacket.message, playerTarget->getGuid()).serialise().get());
                        break;
                    }

                    playerTarget->sendPacket(SmsgMessageChat(CHAT_MSG_WHISPER, messageLanguage, gmFlag, srlPacket.message, _player->getGuid()).serialise().get());
                    if (messageLanguage != LANG_ADDON)
                    {
                        // TODO Verify should this be LANG_UNIVERSAL?
                        SendPacket(SmsgMessageChat(CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, gmFlag, srlPacket.message, playerTarget->getGuid()).serialise().get());
                    }

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
        {
            if (auto channel = sChannelMgr.getChannel(srlPacket.destination, _player))
                channel->say(_player, srlPacket.message, nullptr, false);

        } break;
        case CHAT_MSG_AFK:
        {
            _player->setAFKReason(srlPacket.message);
            _player->toggleAfk();
        } break;
        case CHAT_MSG_DND:
        {
            _player->setAFKReason(srlPacket.message);
            _player->toggleDnd();
        } break;
        case CHAT_MSG_BATTLEGROUND:
        case CHAT_MSG_BATTLEGROUND_LEADER:
        {
            if (!player_can_speak_language || !_player->m_bg)
                break;

#if VERSION_STRING >= Cata
            // Use correct type for group leader
            if (auto const group = _player->getGroup())
                if (group->GetLeader() == _player->getPlayerInfo())
                    srlPacket.type = CHAT_MSG_BATTLEGROUND_LEADER;
#endif

            _player->m_bg->distributePacketToTeam(SmsgMessageChat(static_cast<uint8_t>(srlPacket.type), messageLanguage, gmFlag, srlPacket.message, _player->getGuid()).serialise().get(), _player->getTeam());
        } break;
        default: 
            break;
    }
}

void WorldSession::handleTextEmoteOpcode(WorldPacket& recvPacket)
{
    if (!_player->isAlive())
        return;

    CmsgTextEmote srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (isSessionMuted() || isFloodProtectionTriggered())
        return;

    WoWGuid targetGuid = 0;
    uint32_t nameLength = 1;
    std::string unitName;

    auto unit = _player->getWorldMap()->getUnit(srlPacket.guid);
    if (unit)
    {
        targetGuid = unit->getGuid();

        if (unit->isPlayer())
        {
            unitName = dynamic_cast<Player*>(unit)->getName();
            nameLength = static_cast<uint32_t>(unitName.length()) + 1;
        }
        else if (unit->isPet())
        {
            unitName = dynamic_cast<Pet*>(unit)->getName();
            nameLength = static_cast<uint32_t>(unitName.length()) + 1;
        }
        else
        {
            auto creature = dynamic_cast<Creature*>(unit);
            unitName = creature->GetCreatureProperties()->Name;
            nameLength = static_cast<uint32_t>(unitName.length()) + 1;
        }
    }

    if (const auto emoteTextEntry = sEmotesTextStore.lookupEntry(srlPacket.text_emote))
    {
        sHookInterface.OnEmote(_player, emoteTextEntry->textid, unit);
        if (unit)
        {
            if (unit->IsInWorld() && unit->isCreature() && dynamic_cast<Creature*>(unit)->GetScript())
                dynamic_cast<Creature*>(unit)->GetScript()->OnEmote(_player, static_cast<EmoteType>(emoteTextEntry->textid));
        }

    #if VERSION_STRING < Cata
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
    #else // >=Cata
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
    #endif

    #if VERSION_STRING < Cata
        _player->sendMessageToSet(SmsgEmote(emoteTextEntry->textid, _player->getGuid()).serialise().get(), true);
    #endif

        _player->sendMessageToSet(SmsgTextEmote(nameLength, unitName, srlPacket.text_emote, _player->getGuid(), srlPacket.numEmote, targetGuid).serialise().get(), true);

#if VERSION_STRING > TBC
    #if VERSION_STRING == WotLK
        _player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE, srlPacket.text_emote, 0, 0);
    #else
        _player->getAchievementMgr()->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE, srlPacket.text_emote, 0, 0);
    #endif
#endif
        sQuestMgr.OnPlayerEmote(_player, srlPacket.text_emote, srlPacket.guid);
    }
}

void WorldSession::handleEmoteOpcode(WorldPacket& recvPacket)
{
    CmsgEmote srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (!_player->isAlive())
        return;

    _player->emote(static_cast<EmoteType>(srlPacket.emote));

#if VERSION_STRING > TBC
    _player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE, srlPacket.emote, 0, 0);
#endif

    uint64_t guid = _player->getGuid();
    sQuestMgr.OnPlayerEmote(_player, srlPacket.emote, guid);
}

void WorldSession::handleReportSpamOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING < Cata
    CmsgComplaint srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debug("REPORT SPAM: type {}, guid {}, unk1 {}, unk2 {}, unk3 {}, unk4 {}, message {}", srlPacket.spam_type, srlPacket.spammer_guid.getGuidLow(),
        srlPacket.unk1, srlPacket.unk2, srlPacket.unk3, srlPacket.unk4, srlPacket.description);

    SendPacket(SmsgComplainResult(0).serialise().get());
#endif
}

void WorldSession::handleChatIgnoredOpcode(WorldPacket& recvPacket)
{
    CmsgChatIgnored srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto player = sObjectMgr.getPlayer(srlPacket.guid.getGuidLow());
    if (player == nullptr || player->getSession() == nullptr)
        return;

    player->getSession()->SendPacket(SmsgMessageChat(CHAT_MSG_IGNORED, LANG_UNIVERSAL, 0, _player->getName(), _player->getGuid()).serialise().get());
}

void WorldSession::handleChatChannelWatchOpcode(WorldPacket& recvPacket)
{
    CmsgSetChannelWatch srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debug("Unhandled... Player {} watch channel: {}", _player->getName(), srlPacket.name);
}