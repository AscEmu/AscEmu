/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AchievementMgr.h"
#include "Server/PacketBroadcast.hpp"

#include "Group.h"
#include "MailMgr.h"
#include "ObjectMgr.hpp"
#include "QuestProperties.hpp"
#include "Macros/AIInterfaceMacros.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Objects/Item.hpp"
#include "Objects/Units/Stats.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Spell/Definitions/SpellMechanics.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Server/Packets/SmsgServerFirstAchievement.h"
#include "Server/Packets/SmsgAchievementDeleted.h"
#include "Server/Packets/SmsgAchievementEarned.h"
#include "Server/Packets/SmsgAllAchievementData.h"
#include "Server/Packets/SmsgAllAchievementDataLegacy.h"
#include "Server/Packets/SmsgCriteriaDeleted.h"
#include "Server/Packets/SmsgCriteriaUpdate.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Server/Packets/SmsgRespondInspectAchievements.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Spell/SpellInfo.hpp"
#include "Storage/WDB/WDBStructures.hpp"

using namespace AscEmu::Packets;

AchievementMgr::AchievementMgr(Player* _player) : m_player(_player), isCharacterLoading(true) {}
AchievementMgr::~AchievementMgr()
{
    m_criteriaProgress.clear();
    m_completedAchievements.clear();
}

void AchievementMgr::loadFromDb(QueryResult* _achievementResult, QueryResult* _criteriaResult)
{
    if (_achievementResult)
    {
        do
        {
            Field* field = _achievementResult->fetch();
            uint32_t id = field[0].asUint32();
            if (m_completedAchievements[id] == 0)
                m_completedAchievements[id] = field[1].asUint32();
            else
                sLogger.failure("Duplicate completed achievement {} for player {}, skipping", id, m_player->getGuidLow());
        } while (_achievementResult->nextRow());
    }

    if (_criteriaResult)
    {
        do
        {
            Field* field = _criteriaResult->fetch();
            uint32_t progress_id = field[0].asUint32();
            if (m_criteriaProgress[progress_id] == nullptr)
            {
                m_criteriaProgress[progress_id] = std::make_unique<CriteriaProgress>(progress_id, field[1].asUint32(), static_cast<time_t>(field[2].asUint64()));
            }
            else
                sLogger.failure("Duplicate criteria progress {} for player {}, skipping", progress_id, m_player->getGuidLow());

        } while (_criteriaResult->nextRow());
    }

    moveAccountData();
}

bool AchievementMgr::isGroupCriteriaType(AchievementCriteriaTypes _type) const
{
    switch (_type)
    {
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_BG:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:         // NYI
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:        // NYI
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_BATTLEGROUND:  // NYI
            return true;
        default:
            break;
    }

    return false;
}

Player* AchievementMgr::getPlayer() const { return m_player; }

/// true if the achievement should be shown; false otherwise
bool AchievementMgr::showCompletedAchievement(uint32_t _achievementId, const Player* _player)
{
    switch (_achievementId)
    {
        case  457: // Realm First! Level 80
        case  467: // Realm First! Level 80 Shaman
        case  466: // Realm First! Level 80 Druid
        case  465: // Realm First! Level 80 Paladin
        case  464: // Realm First! Level 80 Priest
        case  463: // Realm First! Level 80 Warlock
        case  462: // Realm First! Level 80 Hunter
        case  461: // Realm First! Level 80 Death Knight
        case  460: // Realm First! Level 80 Mage
        case  459: // Realm First! Level 80 Warrior
        case  458: // Realm First! Level 80 Rogue
        case 1404: // Realm First! Level 80 Gnome
        case 1405: // Realm First! Level 80 Blood Elf
        case 1406: // Realm First! Level 80 Draenei
        case 1407: // Realm First! Level 80 Dwarf
        case 1408: // Realm First! Level 80 Human
        case 1409: // Realm First! Level 80 Night Elf
        case 1410: // Realm First! Level 80 Orc
        case 1411: // Realm First! Level 80 Tauren
        case 1412: // Realm First! Level 80 Troll
        case 1413: // Realm First! Level 80 Forsaken
        case 1415: // Realm First! Grand Master Alchemist
        case 1414: // Realm First! Grand Master Blacksmith
        case 1416: // Realm First! Cooking Grand Master
        case 1417: // Realm First! Grand Master Enchanter
        case 1418: // Realm First! Grand Master Engineer
        case 1419: // Realm First! First Aid Grand Master
        case 1420: // Realm First! Grand Master Angler
        case 1421: // Realm First! Grand Master Herbalist
        case 1422: // Realm First! Grand Master Scribe
        case 1423: // Realm First! Grand Master Jewelcrafter
        case 1424: // Realm First! Grand Master Leatherworker
        case 1425: // Realm First! Grand Master Miner
        case 1426: // Realm First! Grand Master Skinner
        case 1427: // Realm First! Grand Master Tailor
        case 1463: // Realm First! Northrend Vanguard: First player on the realm to gain exalted reputation with the Argent Crusade, Wyrmrest Accord, Kirin Tor and Knights of the Ebon Blade.
        {
            auto achievementResult = CharacterDatabase.query("SELECT guid FROM character_achievement WHERE achievement=%u ORDER BY date LIMIT 1", _achievementId);
            if (achievementResult != nullptr)
            {
                Field* field = achievementResult->fetch();
                if (field != nullptr)
                {
                    // somebody has this Realm First achievement... is it this player?
                    uint64_t firstguid = field->asUint32();
                    if (firstguid != (uint32_t)_player->getGuid())
                    {
                        // nope, somebody else was first.
                        return false;
                    }
                }
            }
        }
        break;
        /* All raid members should receive these last 3 Realm First achievements when they first occur.
           (not implemented yet)
               case 1400: // Realm First! Magic Seeker: Participated in the realm first defeat of Malygos on Heroic Difficulty.
               case  456: // Realm First! Obsidian Slayer: Participated in the realm first defeat of Sartharion the Onyx Guardian on Heroic Difficulty.
               case 1402: // Realm First! Conqueror of Naxxramas: Participated in the realm first defeat of Kel'Thuzad on Heroic Difficulty in Naxxramas. */
        default:
            break;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Gives reward to player for completing the achievement.
void AchievementMgr::giveAchievementReward(WDB::Structures::AchievementEntry const* _entry)
{
    if (_entry == nullptr || isCharacterLoading)
        return;

    AchievementReward const* Reward = sObjectMgr.getAchievementReward(_entry->ID, getPlayer()->getGender());

    if (!Reward)
        return;

    //Reward Titel
    if (getPlayer()->getTeam() == TEAM_ALLIANCE)
    {
        if (Reward->titel_A)
        {
            auto char_title = sCharTitlesStore.lookupEntry(Reward->titel_A);
            if (char_title)
                getPlayer()->setKnownPvPTitle(static_cast<RankTitles>(char_title->bitIndex), true);
        }
    }
    if (getPlayer()->getTeam() == TEAM_HORDE)
    {
        if (Reward->titel_H)
        {
            auto char_title = sCharTitlesStore.lookupEntry(Reward->titel_H);
            if (char_title)
                getPlayer()->setKnownPvPTitle(static_cast<RankTitles>(char_title->bitIndex), true);
        }
    }

    //Reward Mail
    if (Reward->sender)
    {
        Creature* creature = getPlayer()->getWorldMap()->getSpawnManager().summonCreature(Reward->sender, {});
        if (creature == nullptr)
        {
            sLogger.failure("can not create sender for achievement {}", _entry->ID);
            return;
        }

        uint32_t sender = Reward->sender;
        uint64_t receiver = getPlayer()->getGuid();

        const auto loc = (getPlayer()->getSession()->language > 0) ? sMySQLStore.getLocalizedAchievementReward(_entry->ID, getPlayer()->getGender(), getPlayer()->getSession()->language) : nullptr;
        const auto subject = loc ? loc->subject : Reward->subject;
        const auto rewardText = loc ? loc->text : Reward->text;

        std::string messageSubject = subject;
        std::string messageBody = rewardText;

        //Create Item
        auto item = sObjectMgr.createItem(Reward->itemId, getPlayer());

        if (Reward->itemId == 0)
        {
            sMailSystem.SendCreatureGameobjectMail(MAIL_TYPE_CREATURE, sender, receiver, messageSubject, messageBody, 0, 0, 0, 0, MAIL_CHECK_MASK_HAS_BODY, MAIL_DEFAULT_EXPIRATION_TIME);
        }
        else if (item != nullptr)
        {
            item->saveToDB(-1, -1, true, nullptr);

            sMailSystem.SendCreatureGameobjectMail(MAIL_TYPE_CREATURE, sender, receiver, messageSubject, messageBody, 0, 0, item->getGuid(), 0, MAIL_CHECK_MASK_HAS_BODY, MAIL_DEFAULT_EXPIRATION_TIME);

            //removing pItem
            item = nullptr;

            //removing sender
            creature->destroy();
        }
        else
        {
            sLogger.failure("Can not create item for message! (nullptr)");
            return;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Sends message to player(s) that the achievement has been completed.
/// Realm first! achievements get sent to all players currently online.
/// All other achievements get sent to all of the achieving player's guild members,
/// group members, and other in-range players
void AchievementMgr::sendAchievementEarned(WDB::Structures::AchievementEntry const* _entry)
{
    if (_entry == nullptr || isCharacterLoading)
        return;

    // Send Achievement message to everyone currently on the server
    if (_entry->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_KILL | ACHIEVEMENT_FLAG_REALM_FIRST_REACH))
    {
        std::string playerName = getPlayer()->getName();
        uint64_t guid = getPlayer()->getGuid();

        // own team = clickable name
        SmsgServerFirstAchievement ownTeamPacket(playerName, guid, _entry->ID, 1);
        PacketBroadcast::sendFromWorld(sWorld, ownTeamPacket, nullptr, getPlayer()->GetTeam());

        SmsgServerFirstAchievement otherTeamPacket(playerName, guid, _entry->ID, 0);
        PacketBroadcast::sendFromWorld(sWorld, otherTeamPacket, nullptr, getPlayer()->GetTeam() == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE);
    }
    else
    {
        const char* msg = "|Hplayer:$N|h[$N]|h has earned the achievement $a!";
        uint32_t guidCount = 0;
        uint32_t guidIndex;
        // allocate enough space
        auto guidList = std::make_unique<uint32_t[]>(sWorld.getSessionCount() + 256);

        bool alreadySent;

        // Send Achievement message to group members
        if (const auto group = getPlayer()->getGroup())
        {
            // grp->SendPacketToAll(&cdata);
            group->Lock();
            for (uint8_t i = 0; i < group->GetSubGroupCount(); ++i)
            {
                SubGroup* sg = group->GetSubGroup(i);
                if (sg == nullptr)
                    continue;

                for (const auto groupItr : sg->getGroupMembers())
                {
                    if (Player* loggedInPlayer = sObjectMgr.getPlayer(groupItr->guid))
                    {
                        if (loggedInPlayer->getSession())
                        {
                            // check if achievement message has already been sent to this player (if they received a guild achievement message already)
                            alreadySent = false;
                            for (guidIndex = 0; guidIndex < guidCount; ++guidIndex)
                            {
                                if (guidList[guidIndex] == groupItr->guid)
                                {
                                    alreadySent = true;
                                    guidIndex = guidCount;
                                }
                            }

                            if (!alreadySent)
                            {
                                SmsgMessageChat messagePacket(CHAT_MSG_ACHIEVEMENT, LANG_UNIVERSAL, 0, msg, getPlayer()->getGuid(),
                                    "", getPlayer()->getGuid(), "", _entry->ID);
                                loggedInPlayer->getSession()->sendManagedPacket(messagePacket);
                                guidList[guidCount++] = groupItr->guid;
                            }
                        }
                    }
                }
            }
            group->Unlock();
        }

        // Send Achievement message to nearby players
        for (const auto& inRangeItr : getPlayer()->getInRangePlayersSet())
        {
            const Player* player = dynamic_cast<Player*>(inRangeItr);
            if (player && player->getSession() && !player->isIgnored(getPlayer()->getGuidLow()))
            {
                // check if achievement message has already been sent to this player (in guild or group)
                alreadySent = false;
                for (guidIndex = 0; guidIndex < guidCount; ++guidIndex)
                {
                    if (guidList[guidIndex] == player->getGuidLow())
                    {
                        alreadySent = true;
                        guidIndex = guidCount;
                    }
                }
                if (!alreadySent)
                {
                    SmsgMessageChat messagePacket(CHAT_MSG_ACHIEVEMENT, LANG_UNIVERSAL, 0, msg, getPlayer()->getGuid(),
                        "", getPlayer()->getGuid(), "", _entry->ID);
                    player->getSession()->sendManagedPacket(messagePacket);
                    guidList[guidCount++] = player->getGuidLow();
                }
            }
        }
        // Have we sent the message to the achieving player yet?
        alreadySent = false;
        for (guidIndex = 0; guidIndex < guidCount; ++guidIndex)
        {
            if (guidList[guidIndex] == getPlayer()->getGuidLow())
            {
                alreadySent = true;
                guidIndex = guidCount;
            }

            if (!alreadySent)
            {
                SmsgMessageChat messagePacket(CHAT_MSG_ACHIEVEMENT, LANG_UNIVERSAL, 0, msg, getPlayer()->getGuid(),
                    "", getPlayer()->getGuid(), "", _entry->ID);
                getPlayer()->getSession()->sendManagedPacket(messagePacket);
            }
        }
    }
    //    GetPlayer()->sendMessageToSet(&cdata, true);

    SmsgAchievementEarned managedPacket{ getPlayer()->GetNewGUID(), uint32_t(_entry->ID), uint32_t(secsToTimeBitFields(UNIXTIME)), UNIXTIME };
    getPlayer()->getSession()->sendManagedPacket(managedPacket);
}
