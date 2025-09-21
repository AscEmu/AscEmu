/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AchievementMgr.h"

#include "Group.h"
#include "MailMgr.h"
#include "ObjectMgr.hpp"
#include "QuestProperties.hpp"
#include "Macros/AIInterfaceMacros.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Objects/Item.hpp"
#include "Objects/Units/Stats.h"
#include "Server/WorldSocket.h"
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
#include "Server/Packets/SmsgCriteriaDeleted.h"
#include "Server/Packets/SmsgCriteriaUpdate.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Spell/SpellInfo.hpp"
#include "Storage/WDB/WDBStructures.hpp"

using namespace AscEmu::Packets;

#if VERSION_STRING > TBC
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
            Field* field = _achievementResult->Fetch();
            uint32_t id = field[0].asUint32();
            if (m_completedAchievements[id] == 0)
                m_completedAchievements[id] = field[1].asUint32();
            else
                sLogger.failure("Duplicate completed achievement {} for player {}, skipping", id, m_player->getGuidLow());
        } while (_achievementResult->NextRow());
    }

    if (_criteriaResult)
    {
        do
        {
            Field* field = _criteriaResult->Fetch();
            uint32_t progress_id = field[0].asUint32();
            if (m_criteriaProgress[progress_id] == nullptr)
            {
                m_criteriaProgress[progress_id] = std::make_unique<CriteriaProgress>(progress_id, field[1].asUint32(), static_cast<time_t>(field[2].asUint64()));
            }
            else
                sLogger.failure("Duplicate criteria progress {} for player {}, skipping", progress_id, m_player->getGuidLow());

        } while (_criteriaResult->NextRow());
    }
}

void AchievementMgr::saveToDb(QueryBuffer* _buffer)
{
    if (!m_completedAchievements.empty())
    {
        std::ostringstream ss;

        ss << "DELETE FROM character_achievement WHERE guid = ";
        ss << m_player->getGuidLow();
        ss << ";";

        if (_buffer == nullptr)
            CharacterDatabase.ExecuteNA(ss.str().c_str());
        else
            _buffer->AddQueryNA(ss.str().c_str());

        ss.rdbuf()->str("");

        ss << "INSERT INTO character_achievement VALUES ";
        bool first = true;
        for (auto iterCompletedAchievements : m_completedAchievements)
        {
            if (ss.str().length() >= 16000)
            {
                // SQL query length is limited to 16384 characters
                if (_buffer == nullptr)
                    CharacterDatabase.ExecuteNA(ss.str().c_str());
                else
                    _buffer->AddQueryNA(ss.str().c_str());

                ss.str("");
                ss << "INSERT INTO character_achievement VALUES ";
                first = true;
            }

            if (!first)
                ss << ", ";
            else
                first = false;

            ss << "(" << m_player->getGuidLow() << ", " << iterCompletedAchievements.first << ", " << iterCompletedAchievements.second << ")";
        }

        if (_buffer == nullptr)
            CharacterDatabase.ExecuteNA(ss.str().c_str());
        else
            _buffer->AddQueryNA(ss.str().c_str());
    }

    if (!m_criteriaProgress.empty())
    {
        std::ostringstream ss;

        ss << "DELETE FROM character_achievement_progress WHERE guid = ";
        ss << m_player->getGuidLow();
        ss << ";";

        if (_buffer == nullptr)
            CharacterDatabase.ExecuteNA(ss.str().c_str());
        else
            _buffer->AddQueryNA(ss.str().c_str());

        ss.rdbuf()->str("");

        ss << "INSERT INTO character_achievement_progress VALUES ";

        bool first = true;
        for (const auto& iterCriteriaProgress : m_criteriaProgress)
        {
            if (canSaveAchievementProgressToDB(iterCriteriaProgress.second.get()))
            {
                // only save some progresses, others will be updated when character logs in
                if (ss.str().length() >= 16000)
                {
                    // SQL query length is limited to 16384 characters
                    if (_buffer == nullptr)
                        CharacterDatabase.ExecuteNA(ss.str().c_str());
                    else
                        _buffer->AddQueryNA(ss.str().c_str());
                    ss.str("");
                    ss << "INSERT INTO character_achievement_progress VALUES ";
                    first = true;
                }

                if (!first)
                    ss << ", ";
                else
                    first = false;

                ss << "(" << m_player->getGuidLow() << ", " << iterCriteriaProgress.first << ", " << iterCriteriaProgress.second->counter << ", " << iterCriteriaProgress.second->date << ")";
            }
        }

        if (!first)
        {
            // don't execute query if there's no entries to save
            if (_buffer == nullptr)
                CharacterDatabase.ExecuteNA(ss.str().c_str());
            else
                _buffer->AddQueryNA(ss.str().c_str());
        }
    }
}

bool AchievementMgr::canCompleteCriteria(WDB::Structures::AchievementCriteriaEntry const* _achievementCriteria, AchievementCriteriaTypes _type, Player* _player) const
{
    switch (_type)
    {
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS:
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
            return true;
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
            return _player->hasOverlayUncovered(_achievementCriteria->explore_area.areaReference);
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
            return m_completedAchievements.find(_achievementCriteria->complete_achievement.linkedAchievement) != m_completedAchievements.end();
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
            return _player->hasSpell(_achievementCriteria->learn_spell.spellID);
        default:
            break;
    }

    return false;
}

bool AchievementMgr::canCompleteCriteria(WDB::Structures::AchievementCriteriaEntry const* _achievementCriteria, AchievementCriteriaTypes _type, int32_t _miscValue1, int32_t /*miscValue2*/, Player* _player) const
{
    switch (_type)
    {
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_KILLING_BLOW:
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE:
        case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
        case ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
        case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER:
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE:
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
            return true;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM:
            return _achievementCriteria->loot_item.itemID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
            return _player->hasOverlayUncovered(_achievementCriteria->explore_area.areaReference);
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            return _achievementCriteria->complete_quests_in_zone.zoneID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
            return _achievementCriteria->complete_quest.questID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
            return _achievementCriteria->gain_reputation.factionID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
            return _achievementCriteria->learn_spell.spellID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS:
            return _achievementCriteria->number_of_mounts.unknown == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
            return _achievementCriteria->be_spell_target.spellID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
            return _achievementCriteria->kill_creature.creatureID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
            return _achievementCriteria->reach_skill_level.skillID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
            return _achievementCriteria->learn_skill_level.skillID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM:
            return _achievementCriteria->equip_item.itemID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
            return _achievementCriteria->equip_epic_item.itemSlot == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
            return _achievementCriteria->do_emote.emoteID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM:
            return _achievementCriteria->use_item.itemID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT:
            return _achievementCriteria->use_gameobject.goEntry == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
            return _achievementCriteria->honorable_kill_at_area.areaID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS:
            return _achievementCriteria->hk_class.classID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE:
            return _achievementCriteria->hk_race.raceID == static_cast<uint32_t>(_miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP:
            return _achievementCriteria->death_at_map.mapID == static_cast<uint32_t>(_miscValue1);
        default:
            break;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// \brief This is called during player login to update some criteria which aren't
/// saved in achievement progress DB, since they are saved in the character DB or
/// can easily be computed.
void AchievementMgr::updateAllAchievementCriteria()
{
    for (uint8_t i = 0; i < ACHIEVEMENT_CRITERIA_TYPE_TOTAL; i++)
        updateAchievementCriteria(static_cast<AchievementCriteriaTypes>(i));
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Updates achievement criteria of the specified type
/// \brief This is what should be called from other places in the code (upon killing a
/// monster, or looting an object, or completing a quest, etc.). miscvalue1, miscvalue2
/// depend on the achievement type. Generally, miscvalue1 is an ID of some type (quest ID,
/// item ID, faction ID, etc.), and miscvalue2 is the amount to increase the progress.
void AchievementMgr::updateAchievementCriteria(AchievementCriteriaTypes _type, int32_t _miscvalue1, int32_t _miscvalue2, uint32_t /*time*/, Object* _reference /*nullptr*/)
{
    if (m_player->getSession()->HasGMPermissions() && worldConfig.gm.disableAchievements)
        return;

    uint64_t selectedGUID = 0;
    if (_type == ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE)
    {
        selectedGUID = getPlayer()->getTargetGuid();
    }
    AchievementCriteriaEntryList const & achievementCriteriaList = sObjectMgr.getAchievementCriteriaByType(_type);
    for (const auto achievementCriteria : achievementCriteriaList)
    {
        if (isCompletedCriteria(achievementCriteria))
            continue;

        if (achievementCriteria->groupFlag & ACHIEVEMENT_CRITERIA_GROUP_NOT_IN_GROUP && getPlayer()->getGroup())
            continue;

        const auto achievement = sAchievementStore.lookupEntry(achievementCriteria->referredAchievement);
        if (!achievement)
            continue;

        // achievement requires a faction of which the player is not a member
        if ((achievement->factionFlag == ACHIEVEMENT_FACTION_FLAG_HORDE && getPlayer()->isTeamHorde() == false) ||
            (achievement->factionFlag == ACHIEVEMENT_FACTION_FLAG_ALLIANCE && getPlayer()->isTeamAlliance() == false))
            continue;

        if (!canCompleteCriteria(achievementCriteria, _type, _miscvalue1, _miscvalue2, getPlayer()))
            continue;

        // Check scripted criteria checks
        const auto scriptResult = sScriptMgr.callScriptedAchievementCriteriaCanComplete(achievementCriteria->ID, getPlayer(), _reference);
        if (!scriptResult)
            continue;

        switch (_type)
        {
            //Start of Achievement List
            case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
                setCriteriaProgress(achievementCriteria, getPlayer()->getLevel());
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM:
            case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM:
                updateCriteriaProgress(achievementCriteria, _miscvalue2);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
            case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM:
                setCriteriaProgress(achievementCriteria, 1);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
            case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS:
            case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE:
            case ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP:
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
            case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
            case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
            case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT:
                updateCriteriaProgress(achievementCriteria, 1);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS:
                // Vanity pets owned - miscvalue1==778
                // Number of mounts  - miscvalue1==777
                updateCriteriaProgress(achievementCriteria, 1);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
            case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
            case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
            case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
                setCriteriaProgress(achievementCriteria, _miscvalue2);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD:
            case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY:
            case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
            case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
            case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP:
            case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
            case ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL:
            case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER:
            case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE:
            case ACHIEVEMENT_CRITERIA_TYPE_DEATH:
                updateCriteriaProgress(achievementCriteria, _miscvalue1);
                break;

            // TODO: add achievement scripts for following cases
            case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
                switch (achievement->ID)
                {
                    case 2556: // Pest Control
                        if ((_miscvalue1 == 3300 && achievementCriteria->index == 1)       // Adder
                            || (_miscvalue1 == 32261 && achievementCriteria->index == 2)   // Crystal Spider
                            || (_miscvalue1 == 24270 && achievementCriteria->index == 3)   // Devouring Maggot
                            || (_miscvalue1 == 9699 && achievementCriteria->index == 4)    // Fire Beetle
                            || (_miscvalue1 == 24174 && achievementCriteria->index == 5)   // Fjord Rat
                            || (_miscvalue1 == 32258 && achievementCriteria->index == 6)   // Gold Beetle
                            || (_miscvalue1 == 16068 && achievementCriteria->index == 7)   // Larva
                            || (_miscvalue1 == 16030 && achievementCriteria->index == 8)   // Maggot
                            || (_miscvalue1 == 4953 && achievementCriteria->index == 9)    // Moccasin
                            || (_miscvalue1 == 6271 && achievementCriteria->index == 10)   // Mouse
                            || (_miscvalue1 == 4075 && achievementCriteria->index == 11)   // Rat
                            || (_miscvalue1 == 4076 && achievementCriteria->index == 12)   // Roach
                            || (_miscvalue1 == 15476 && achievementCriteria->index == 13)  // Scorpion
                            || (_miscvalue1 == 2914 && achievementCriteria->index == 14)   // Snake
                            || (_miscvalue1 == 14881 && achievementCriteria->index == 15)  // Spider
                            || (_miscvalue1 == 32428 && achievementCriteria->index == 16)  // Underbelly Rat
                            || (_miscvalue1 == 28202 && achievementCriteria->index == 17)) // Zul'Drak Rat
                        {
                            setCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                        // Kill creature X in Heroic dungeon
                    case 489: // Heroic: Utgarde Keep
                    case 490: // Heroic: The Nexus
                    case 491: // Heroic: Azjol-Nerub
                    case 492: // Heroic: Ahn'kahet: The Old Kingdom
                    case 493: // Heroic: Drak'Tharon Keep
                    case 494: // Heroic: The Violet Hold
                    case 495: // Heroic: Gundrak
                    case 496: // Heroic: Halls of Stone
                    case 497: // Heroic: Halls of Lightning
                    case 498: // Heroic: The Oculus
                    case 499: // Heroic: Utgarde Pinnacle
                    case 500: // Heroic: The Culling of Stratholme
                    case 563: // Heroic: The Arachnid Quarter
                    case 565: // Heroic: The Construct Quarter
                    case 567: // Heroic: The Plague Quarter
                    case 569: // Heroic: The Military Quarter
                    case 573: // Heroic: Sapphiron's Demise
                    case 575: // Heroic: Kel'Thuzad's Defeat
                    case 577: // Heroic: The Fall of Naxxramas
                    case 623: // Heroic: The Spellweaver's Downfall
                    case 625: // Heroic: Besting the Black Dragonflight
                    case 667: // Heroic: Hellfire Ramparts
                    case 668: // Heroic: The Blood Furnace
                    case 669: // Heroic: The Slave Pens
                    case 670: // Heroic: Underbog
                    case 671: // Heroic: Mana-Tombs
                    case 672: // Heroic: Auchenai Crypts
                    case 673: // Heroic: The Escape From Durnholde
                    case 674: // Heroic: Sethekk Halls
                    case 675: // Heroic: Shadow Labyrinth
                    case 676: // Heroic: Opening of the Dark Portal
                    case 677: // Heroic: The Steamvault
                    case 678: // Heroic: The Shattered Halls
                    case 679: // Heroic: The Mechanar
                    case 680: // Heroic: The Botanica
                    case 681: // Heroic: The Arcatraz
                    case 682: // Heroic: Magister's Terrace
                    case 1312: // Utgarde Keep bosses on Heroic Difficulty.
                    case 1504: // Ingvar the Plunderer kills (Heroic Utgarde Keep)
                    case 1505: // Keristrasza kills (Heroic Nexus)
                    case 1506: // Anub'arak kills (Heroic Azjol-Nerub)
                    case 1507: // Herald Volazj kills (Heroic Ahn'kahet)
                    case 1508: // The Prophet Tharon'ja kills (Heroic Drak'Tharon Keep)
                    case 1509: // Cyanigosa kills (Heroic Violet Hold)
                    case 1510: // Gal'darah kills (Heroic Gundrak)
                    case 1511: // Sjonnir the Ironshaper kills (Heroic Halls of Stone)
                    case 1512: // Loken kills (Heroic Halls of Lightning)
                    case 1513: // Ley-Guardian Eregos kills (Heroic Oculus)
                    case 1514: // King Ymiron kills (Heroic Utgarde Pinnacle)
                    case 1515: // Mal'Ganis defeated (Heroic CoT: Stratholme)
                    case 1721: // Heroic: Archavon the Stone Watcher
                    case 1817: // The Culling of Time
                    case 1865: // Lockdown!
                        if (getPlayer()->getDungeonDifficulty() >= InstanceDifficulty::DUNGEON_HEROIC)
                            updateCriteriaProgress(achievementCriteria, 1);
                        break;
                        ///\todo More complicated achievements: time limits, group size limits, other criteria...
                    case 1870: // Heroic: A Poke In The Eye
                        // Defeat Malygos on Heroic Difficulty with fewer than 21.
                    case 2056: // Volunteer Work
                        // Defeat Jedoga Shadowseeker in Ahn'kahet on Heroic Difficulty without killing any Twilight Volunteers.
                    case 1875: // Heroic: You Don't Have An Eternity
                        // Defeat Malygos in 6 minutes or less on Heroic Difficulty.
                    case 2185: // Heroic: Just Can't Get Enough
                        // Defeat Kel'Thuzad on Heroic Difficulty in Naxxramas while killing at least 18 abominations in his chamber.
                    case 1862: // Volazj's Quick Demise
                        // Defeat Herald Volazj in Ahn'kahet on Heroic Difficulty in 2 minutes or less.
                    case 2186: // The Immortal
                        // Within one raid lockout period, defeat every boss in Naxxramas on Heroic Difficulty without allowing any raid member to die during any of the boss encounters.
                    case 2038: // Respect Your Elders
                        // Defeat Elder Nadox in Ahn'kahet on Heroic Difficulty without killing any Ahn'kahar Guardians.
                    case 2183: // Heroic: Spore Loser
                        // Defeat Loatheb in Naxxramas on Heroic Difficulty without killing any spores.
                    case 1297: // Hadronox Denied
                        // Defeat Hadronox in Azjol-Nerub on Heroic Difficulty before he webs the top doors and prevents more creatures from spawning.
                    case 2177: // Heroic: And They Would All Go Down Together
                        // Defeat the 4 Horsemen in Naxxramas on Heroic Difficulty, ensuring that they all die within 15 seconds of each other.
                    case 1860: // Gotta Go!
                        // Defeat Anub'arak in Azjol-Nerub on Heroic Difficulty in 2 minutes or less.
                    case 2147: // Heroic: The Hundred Club
                        // Defeat Sapphiron on Heroic Difficulty in Naxxramas without any member of the raid having a frost resist value over 100.
                    case 1861: // The Party's Over
                        // Defeat Prince Taldaram in Ahn'kahet on Heroic Difficulty with less than 5 people.
                    case 2181: // Heroic: Subtraction
                        // Defeat Thaddius in Naxxramas on Heroic Difficulty with less than 21 people.
                    case 579: // Heroic: The Dedicated Few
                        // Defeat the bosses of Naxxramas with less than 21 people in the zone on Heroic Difficulty.
                    case 1296: // Watch Him Die
                        // Defeat Krik'thir the Gatewatcher in Azjol-Nerub on Heroic Difficulty while Watcher Gashra, Watcher Narjil and Watcher Silthik are still alive.
                    case 1589: // Heroic: Arachnophobia
                        // Kill Maexxna in Naxxramas within 20 minutes of Anub'Rekhan's death on Heroic Difficulty.
                    case 1857: // Heroic: Make Quick Werk Of Him
                        // Kill Patchwerk in Naxxramas in 3 minutes or less on Heroic Difficulty.
                    case 1877: // Heroic: Less Is More
                        // Defeat Sartharion the Onyx Guardian and the Twilight Drakes on Heroic Difficulty with fewer than 21.
                    case 1919: // On The Rocks
                        // Defeat Prince Keleseth in Utgarde Keep on Heroic Difficulty without shattering any Frost Tombs.
                    case 2036: // Intense Cold
                        // Defeat Keristrasza in The Nexus on Heroic Difficulty without allowing Intense Cold to reach more than two stacks.
                    case 2139: // Heroic: The Safety Dance
                        // Defeat Heigan the Unclean in Naxxramas on Heroic Difficulty without anyone in the raid dying.
                    case 2140: // Heroic: Momma Said Knock You Out
                        // Defeat Grand Widow Faerlina in Naxxramas on Heroic Difficulty without dispelling frenzy.
                    case 2150: // Split Personality
                        // Defeat Grand Magus Telestra in The Nexus on Heroic Difficulty after having killed her images within 5 seconds of each other during both splits.
                    case 2151: // Consumption Junction
                        // Defeat Trollgore in Drak'Tharon Keep on Heroic Difficulty before Consume reaches ten stacks.
                    case 2179: // Heroic: Shocking!
                        // Defeat Thaddius in Naxxramas on Heroic Difficulty without anyone in the raid crossing the negative and positive charges.
                    case 2037: // Chaos Theory
                        // Defeat Anomalus in The Nexus on Heroic Difficulty without destroying any Chaotic Rifts.
                    case 2039: // Better Off Dred
                        // Engage King Dred in Drak'Tharon Keep on Heroic Difficulty and slay 6 Drakkari Gutrippers or Drakkari Scytheclaw during his defeat.
                    case 2048: // Heroic: Gonna Go When the Volcano Blows
                        // Defeat Sartharion the Onyx Guardian on Heroic Difficulty without getting hit by Lava Strike.
                    case 2057: // Oh Novos!
                        // Defeat Novos the Summoner in Drak'Tharon Keep on Heroic Difficulty without allowing any undead minions to reach the floor.
                    case 1816: // Defenseless
                        // Defeat Cyanigosa in The Violet Hold without using Defense Control Crystals and with Prison Seal Integrity at 100% while in Heroic Difficulty.
                    case 2052: // Heroic: Twilight Assist
                        // With at least one Twilight Drake still alive, engage and defeat Sartharion the Onyx Guardian on Heroic Difficulty.
                    case 2053: // Heroic: Twilight Duo
                        // With at least two Twilight Drakes still alive, engage and defeat Sartharion the Onyx Guardian on Heroic Difficulty.
                    case 2041: // Dehydration
                        // Defeat Ichoron in the Violet Hold on Heroic Difficulty without allowing any Ichor Globules to merge.
                    case 2054: // Heroic: The Twilight Zone
                        // With all three Twilight Drakes still alive, engage and defeat Sartharion the Onyx Guardian on Heroic Difficulty.
                    case 1864: // What the Eck?
                        // Defeat Gal'darah in Gundrak on Heroic Difficulty while under the effects of Eck Residue.
                    case 2152: // Share The Love
                        // Defeat Gal'darah in Gundrak on Heroic Difficulty and have 5 unique party members get impaled throughout the fight.
                    case 2040: // Less-rabi
                        // Defeat Moorabi in Gundrak on Heroic Difficulty while preventing him from transforming into a mammoth at any point during the encounter.
                    case 2058: // Snakes. Why'd It Have To Be Snakes?
                        // Defeat Slad'ran in Gundrak on Heroic Difficulty without getting snake wrapped.
                    case 1866: // Good Grief
                        // Defeat the Maiden of Grief in the Halls of Stone on Heroic Difficulty in 1 minute or less.
                    case 2155: // Abuse the Ooze
                        // Defeat Sjonnir the Ironshaper in the Halls of Stone on Heroic Difficulty and kill 5 Iron Sludges during the encounter.
                    case 2154: // Brann Spankin' New
                        // Defeat the Tribunal of Ages encounter in the Halls of Stone on Heroic Difficulty without allowing Brann Bronzebeard to take any damage.
                    case 1867: // Timely Death
                        // Defeat Loken in the Halls of Lightning on Heroic Difficulty in 2 minutes or less.
                    case 1834: //Lightning Struck
                        // Defeat General Bjarngrim in the Halls of Lightning on Heroic Difficulty while he has a Temporary Electrical Charge.
                    case 2042: // Shatter Resistant
                        // Defeat Volkhan in the Halls of Lightning on Heroic Difficulty without allowing him to shatter more than 4 Brittle Golems.
                    case 1872: // Zombiefest!
                        // Kill 100 Risen Zombies in 1 minute in The Culling of Stratholme on Heroic Difficulty.
                    case 2043: // The Incredible Hulk
                        // Force Svala Sorrowgrave to kill a Scourge Hulk on Heroic Difficulty in Utgarde Pinnacle.
                    case 1873: // Lodi Dodi We Loves the Skadi
                        // Defeat Skadi the Ruthless in Utgarde Pinnacle on Heroic Difficulty within 3 minutes of starting the gauntlet event.
                    case 2156: // My Girl Loves to Skadi All the Time
                        // Defeat Skadi the Ruthless in Utgarde Pinnacle on Heroic Difficulty after having killed Grauf from 100% to dead in a single pass.
                    case 2157: // King's Bane
                        // Defeat King Ymiron in Utgarde Pinnacle on Heroic Difficulty without anyone in the party triggering Bane.
                    case 1871: // Experienced Drake Rider
                        // On three different visits to The Oculus, get credit for defeating Ley-Guardian Eregos while riding an Amber, Emerald, and Ruby drake on Heroic Difficulty.
                    case 1868: // Make It Count
                        // Defeat Ley-Guardian Eregos in The Oculus on Heroic Difficulty within 20 minutes of Drakos the Interrogator's death.
                    case 2044: // Ruby Void
                        // Defeat Ley-Guardian Eregos in The Oculus on Heroic Difficulty without anyone in your party using a Ruby Drake.
                    case 2045: // Emerald Void
                        // Defeat Ley-Guardian Eregos in The Oculus on Heroic Difficulty without anyone in your party using an Emerald Drake.
                    case 2046: // Amber Void
                        // Defeat Ley-Guardian Eregos in The Oculus on Heroic Difficulty without anyone in your party using an Amber Drake.
                        break;
                    default:
                        // already tested heroic achievements above, the rest should be normal or non-dungeon
                        if (!IS_INSTANCE(getPlayer()->GetMapId()) || (getPlayer()->getDungeonDifficulty() == InstanceDifficulty::DUNGEON_NORMAL))
                            updateCriteriaProgress(achievementCriteria, 1);
                        break;
                }
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
                // Achievement ID:556 description Equip an epic item in every slot with a minimum item level of 213.
                // "213" value not found in achievement or criteria entries (dbc), have to hard-code it here? :(
                // Achievement ID:557 description Equip a superior item in every slot with a minimum item level of 187.
                // "187" value not found in achievement or criteria entries (dbc), have to hard-code it here? :(
                // AchievementType for both Achievement ID:556 (Equip epic items) and ID:557 (Equip superior items)
                //    is the same (47) ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM
                // Going to send item slot in miscvalue1 and item quality in miscvalue2 when calling UpdateAchievementCriteria.
                if (achievementCriteria->referredAchievement == 556 && _miscvalue2 == ITEM_QUALITY_EPIC_PURPLE)
                    setCriteriaProgress(achievementCriteria, 1);
                else if (achievementCriteria->referredAchievement == 557 && (_miscvalue2 == ITEM_QUALITY_RARE_BLUE))
                    setCriteriaProgress(achievementCriteria, 1);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
                // emote matches, check the achievement target ... (if required)
                switch (achievement->ID)
                {
                    case 1206: // To All The Squirrels I've Loved Before
                    {
                        // requires a target
                        if (const auto unit = getPlayer()->getWorldMap()->getUnit(selectedGUID))
                        {
                            const uint32_t unitEntry = unit->getEntry();
                            if ((unitEntry == 1412 && achievementCriteria->index == 1)      // Squirrel
                                || (unitEntry == 25679 && achievementCriteria->index == 2)  // Steam Frog
                                || (unitEntry == 25677 && achievementCriteria->index == 3)  // Borean Frog
                                || (unitEntry == 6368 && achievementCriteria->index == 4)   // Cat
                                || (unitEntry == 620 && achievementCriteria->index == 5)    // Chicken
                                || (unitEntry == 2442 && achievementCriteria->index == 6)   // Cow
                                || (unitEntry == 6827 && achievementCriteria->index == 7)   // Crab
                                || (unitEntry == 883 && achievementCriteria->index == 8)    // Deer
                                || (unitEntry == 19665 && achievementCriteria->index == 9)  // Ewe
                                || (unitEntry == 890 && achievementCriteria->index == 10)   // Fawn
                                || (unitEntry == 13321 && achievementCriteria->index == 11) // Frog
                                || (unitEntry == 4166 && achievementCriteria->index == 12)  // Gazelle
                                || (unitEntry == 5951 && achievementCriteria->index == 13)  // Hare
                                || (unitEntry == 9600 && achievementCriteria->index == 14)  // Parrot
                                || (unitEntry == 721 && achievementCriteria->index == 15)   // Rabbit
                                || (unitEntry == 2098 && achievementCriteria->index == 16)  // Ram
                                || (unitEntry == 1933 && achievementCriteria->index == 17)  // Sheep
                                || (unitEntry == 17467 && achievementCriteria->index == 18) // Skunk
                                || (unitEntry == 10685 && achievementCriteria->index == 19) // Swine
                                || (unitEntry == 1420 && achievementCriteria->index == 20)  // Toad
                                || (unitEntry == 2620 && achievementCriteria->index == 21)) // Prairie Dog
                            {
                                setCriteriaProgress(achievementCriteria, 1);
                            }
                        }
                    } break;
                    case 2557: // To All The Squirrels Who Shared My Life
                    {
                        // requires a target
                        if (const auto unit = getPlayer()->getWorldMap()->getUnit(selectedGUID))
                        {
                            const uint32_t unitEntry = unit->getEntry();
                            if ((unitEntry == 29328 && achievementCriteria->index == 1)      // Arctic Hare
                                || (unitEntry == 31685 && achievementCriteria->index == 2)   // Borean Marmot
                                || (unitEntry == 28407 && achievementCriteria->index == 3)   // Fjord Penguin
                                || (unitEntry == 24746 && achievementCriteria->index == 4)   // Fjord Turkey
                                || (unitEntry == 32498 && achievementCriteria->index == 5)   // Glacier Penguin (not in db?)
                                || (unitEntry == 31889 && achievementCriteria->index == 6)   // Grizzly Squirrel
                                || (unitEntry == 6653 && achievementCriteria->index == 7)    // Huge Toad
                                || (unitEntry == 9700 && achievementCriteria->index == 8)    // Lava Crab
                                || (unitEntry == 31890 && achievementCriteria->index == 9)   // Mountain Skunk
                                || (unitEntry == 26503 && achievementCriteria->index == 10)  // Scalawag Frog
                                || (unitEntry == 28093 && achievementCriteria->index == 11)  // Sholazar Tickbird
                                || (unitEntry == 28440 && achievementCriteria->index == 12)) // Tundra Penguin
                            {
                                setCriteriaProgress(achievementCriteria, 1);
                            }
                        }
                    } break;
                    case 247: // Make Love, Not Warcraft
                    {
                        Player* pTarget = sObjectMgr.getPlayer(static_cast<uint32_t>(selectedGUID));
                        if (pTarget && pTarget->isDead() && pTarget->isHostileTo(getPlayer()))
                            updateCriteriaProgress(achievementCriteria, 1);
                    }
                    break;
                    case 293: ///\todo Disturbing the Peace
                        // While wearing 3 pieces of Brewfest clothing, get completely smashed and dance in Dalaran.
                        break;
                    case 1280: ///\todo Flirt With Disaster
                        // Get completely smashed, put on your best perfume, throw a handful of rose petals on Jeremiah Payson and then kiss him. You'll regret it in the morning.
                        break;
                    case 1279: ///\todo Flirt With Disaster
                        // Get completely smashed, put on your best perfume, throw a handful of rose petals on Sraaz and then kiss him. You'll regret it in the morning.
                        break;
                    case 1690: ///\todo A Frosty Shake
                        // During the Feast of Winter Veil, use your Winter Veil Disguise kit to become a snowman and then dance with another snowman in Dalaran.
                        break;
                    case 1704: ///\todo I Pitied The Fool
                        // Pity the Love Fool in the locations specified below.
                        // Wintergrasp (achievementCriteria->index==1)
                        // Battle Ring of Gurubashi Arena (achievementCriteria->index==2)
                        // Arathi Basin Blacksmith (achievementCriteria->index==3)
                        // The Culling of Stratholme (achievementCriteria->index==4)
                        // Naxxramas (achievementCriteria->index==5)
                        break;

                        // Statistics for emotes
                    case 1042: // Number of Hugs
                    case 1045: // Total cheers
                    case 1047: // Total facepalms
                    case 1065: // Total waves
                    case 1066: // Total times LOL'd (laugh, guffaw, rofl, giggle, chuckle)
                    case 1067: // Total times playing world's smallest violin
                        updateCriteriaProgress(achievementCriteria, 1);
                        break;
                    default:
                        break;
                }
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_KILLING_BLOW:
                // miscvalue1 contain Map ID
                switch (achievementCriteria->referredAchievement)
                {
                    case 231:
                        if (((_miscvalue1 == 30) && (achievementCriteria->index == 1))       // Alterac Valley
                            || ((_miscvalue1 == 529) && (achievementCriteria->index == 2))   // Arathi Basin
                            || ((_miscvalue1 == 566) && (achievementCriteria->index == 3))   // Eye of the Storm
                            || ((_miscvalue1 == 489) && (achievementCriteria->index == 4))   // Warsong Gulch
                            || ((_miscvalue1 == 607) && (achievementCriteria->index == 5)))  // Strand of the Ancients
                        {
                            updateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                    case 233: ///\todo Berserking killing blow
                        break;
                    case 1487: // Total Killing Blows
                        updateCriteriaProgress(achievementCriteria, 1);
                        break;
                    case 1488:
                        if (((_miscvalue1 == 0) && (achievementCriteria->index == 1))        // Eastern Kingdoms
                            || ((_miscvalue1 == 1) && (achievementCriteria->index == 2))     // Kalimdor
                            || ((_miscvalue1 == 530) && (achievementCriteria->index == 3))   // Burning Crusade Areas
                            || ((_miscvalue1 == 571) && (achievementCriteria->index == 4)))  // Northrend
                        {
                            updateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                    case 1490:
                        if (((_miscvalue1 == 559) && (achievementCriteria->index == 1))      // Nagrand Arena
                            || ((_miscvalue1 == 562) && (achievementCriteria->index == 2))   // Blade's Edge Arena
                            || ((_miscvalue1 == 572) && (achievementCriteria->index == 3))   // Ruins of Lordaeron
                            || ((_miscvalue1 == 617) && (achievementCriteria->index == 4))   // Dalaran Sewers
                            || ((_miscvalue1 == 618) && (achievementCriteria->index == 5)))  // Ring of Valor
                        {
                            updateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                    case 1491:
                        if (((_miscvalue1 == 30) && (achievementCriteria->index == 1))       // Alterac Valley
                            || ((_miscvalue1 == 529) && (achievementCriteria->index == 2))   // Arathi Basin
                            || ((_miscvalue1 == 489) && (achievementCriteria->index == 3))   // Warsong Gulch
                            || ((_miscvalue1 == 566) && (achievementCriteria->index == 4))   // Eye of the Storm
                            || ((_miscvalue1 == 607) && (achievementCriteria->index == 5)))  // Strand of the Ancients
                        {
                            updateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                    case 1492: ///\todo 2v2 Arena Killing Blows
                        break;
                    case 1493: ///\todo 3v3 Arena Killing Blows
                        break;
                    case 1494: ///\todo 5v5 Arena Killing Blows
                        break;
                    case 1495: // Alterac Valley Killing Blows
                        if (_miscvalue1 == 30)
                            updateCriteriaProgress(achievementCriteria, 1);
                        break;
                    case 1496: // Arathi Basin Killing Blows
                        if (_miscvalue1 == 529)
                            updateCriteriaProgress(achievementCriteria, 1);
                        break;
                    case 1497: // Warsong Gulch Killing Blows
                        if (_miscvalue1 == 489)
                            updateCriteriaProgress(achievementCriteria, 1);
                        break;
                    case 1498: // Eye of the Storm Killing Blows
                        if (_miscvalue1 == 566)
                            updateCriteriaProgress(achievementCriteria, 1);
                        break;
                    case 1499: // Strand of the Ancients Killing Blows
                        if (_miscvalue1 == 607)
                            updateCriteriaProgress(achievementCriteria, 1);
                        break;
                    case 2148: ///\todo Deliver a killing blow to a Scion of Eternity while riding on a hover disk
                        break;
                    case 2149: ///\todo Deliver a killing blow to a Scion of Eternity while riding on a hover disk
                        break;
                    default:
                        break;
                }
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM: // itemID in miscvalue1
                switch (achievementCriteria->referredAchievement)
                {
                    case 1281: // Shoot off 10 Red Rocket Clusters in 25 seconds or less
                    case 1552: // Shoot off 10 Festival Firecrackers in 30 seconds or less
                    case 1696: // Shoot off 10 Love Rockets in 20 seconds or less
                    case 1781: // Get 10 critters in 3 minutes
                    case 1791: // Hearthstone with kid out
                        break;
                    default:
                        updateCriteriaProgress(achievementCriteria, 1);
                        break;
                }
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE:
                // Total NPC kills, Kill an NPC that yields XP, Beasts, Critters killed, Demons, Dragonkin ...
                // miscvalue1 = killed creature high GUID
                // miscvalue2 = killed creature low GUID
            {
                uint64_t creatureGuid = _miscvalue1;
                creatureGuid <<= 32; // shift to high 32-bits
                creatureGuid += _miscvalue2;
                if (const auto unit = getPlayer()->getWorldMap()->getUnit(creatureGuid))
                {
                    bool yieldXP = CalculateXpToGive(unit, getPlayer()) > 0;
                    if (unit->isCreature())
                    {
                        bool isTotem = unit->isTotem();
                        const uint32_t creatureType = dynamic_cast<Creature*>(unit)->GetCreatureProperties()->Type;
                        if ((achievementCriteria->ID == 4944)                                                  // Total NPC kills              refAch==1197
                            || ((achievementCriteria->ID == 4946) && (yieldXP))                                // Kill an NPC that yields XP   refAch==1198
                            || ((achievementCriteria->ID == 4948) && (creatureType == UNIT_TYPE_BEAST))        // Beasts                       refAch== 107
                            || ((achievementCriteria->ID == 4958) && (creatureType == UNIT_TYPE_CRITTER))      // Critters killed              refAch== 107
                            || ((achievementCriteria->ID == 4949) && (creatureType == UNIT_TYPE_DEMON))        // Demons                       refAch== 107
                            || ((achievementCriteria->ID == 4950) && (creatureType == UNIT_TYPE_DRAGONKIN))    // Dragonkin                    refAch== 107
                            || ((achievementCriteria->ID == 4951) && (creatureType == UNIT_TYPE_ELEMENTAL))    // Elemental                    refAch== 107
                            || ((achievementCriteria->ID == 4952) && (creatureType == UNIT_TYPE_GIANT))        // Giant                        refAch== 107
                            || ((achievementCriteria->ID == 4953) && (creatureType == UNIT_TYPE_HUMANOID))     // Humanoid                     refAch== 107
                            || ((achievementCriteria->ID == 4954) && (creatureType == UNIT_TYPE_MECHANICAL))   // Mechanical                   refAch== 107
                            || ((achievementCriteria->ID == 4955) && (creatureType == UNIT_TYPE_UNDEAD))       // Undead                       refAch== 107
                            || ((achievementCriteria->ID == 4956) && (creatureType == UNIT_TYPE_NONE))         // Unspecified                  refAch== 107
                            || ((achievementCriteria->ID == 4957) && (isTotem)))                               // Totems                       refAch== 107
                        {
                            updateCriteriaProgress(achievementCriteria, 1);
                        }
                    }
                }
            }
            break;
            case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
                // fall distance (>=65) has been checked before UpdateAchievementCriteria() call, but it is sent in miscvalue1 just in case "they" add more...
                if (achievement->ID == 1260)
                {
                    // Fall 65 yards without dying while completely smashed during the Brewfest Holiday.
                    if (_miscvalue2 == DRUNKEN_SMASHED)
                    {
                        // drunken state, "completely smashed"
                        ///\todo Check if it is during the Brewfest Holiday ...
                        updateCriteriaProgress(achievementCriteria, 1);
                    }
                }
                else
                {
                    // achievement->ID==964 // Fall 65 yards without dying.
                    updateCriteriaProgress(achievementCriteria, 1);
                }
                break;

            //End of Achievement List
            default:
                return;
        }
        completedCriteria(achievementCriteria);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Updates all achievement criteria of the specified type.
/// \brief This is only called from updateAllAchievementCriteria(), during player login
void AchievementMgr::updateAchievementCriteria(AchievementCriteriaTypes _type)
{
    if (m_player->getSession()->HasGMPermissions() && worldConfig.gm.disableAchievements)
        return;

    AchievementCriteriaEntryList const & achievementCriteriaList = sObjectMgr.getAchievementCriteriaByType(_type);
    for (auto achievementCriteria : achievementCriteriaList)
    {
        const auto achievement = sAchievementStore.lookupEntry(achievementCriteria->referredAchievement);
        if (!achievement  //|| IsCompletedCriteria(achievementCriteria)
            || (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
            || (achievement->factionFlag == ACHIEVEMENT_FACTION_FLAG_HORDE && !m_player->isTeamHorde())
            || (achievement->factionFlag == ACHIEVEMENT_FACTION_FLAG_ALLIANCE && !m_player->isTeamAlliance()))
        {
            continue;
        }

        if (!canCompleteCriteria(achievementCriteria, _type, getPlayer()))
            continue;

        // Check scripted criteria checks
        const auto scriptResult = sScriptMgr.callScriptedAchievementCriteriaCanComplete(achievementCriteria->ID, getPlayer(), nullptr);
        if (!scriptResult)
            continue;

        switch (_type)
        {
            //Start of Achievement List
            case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
                setCriteriaProgress(achievementCriteria, getPlayer()->getLevel());
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
            case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
                setCriteriaProgress(achievementCriteria, 1);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
                setCriteriaProgress(achievementCriteria, 1, true);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
                setCriteriaProgress(achievementCriteria, (int32_t)getPlayer()->getFinishedQuests().size());
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
                setCriteriaProgress(achievementCriteria, getPlayer()->getFactionStanding(achievementCriteria->gain_reputation.factionID));
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
                setCriteriaProgress(achievementCriteria, getPlayer()->getExaltedCount());
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
                setCriteriaProgress(achievementCriteria, getPlayer()->getSkillLineCurrent(static_cast<uint16_t>(achievementCriteria->reach_skill_level.skillID), true));
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
                setCriteriaProgress(achievementCriteria, getPlayer()->getSkillLineMax(static_cast<uint16_t>(achievementCriteria->learn_skill_level.skillID) / 75U));
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
                setCriteriaProgress(achievementCriteria, getPlayer()->getBankSlots());
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            {
                uint32_t qcinzone = 0;
                for (const uint32_t finishedQuestId : getPlayer()->getFinishedQuests())
                {
                    QuestProperties const* qst = sMySQLStore.getQuestProperties(finishedQuestId);
                    if (qst && qst->zone_id == achievementCriteria->complete_quests_in_zone.zoneID)
                        ++qcinzone;
                }
                setCriteriaProgress(achievementCriteria, qcinzone);
            } break;
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
            {
                uint32_t completed = 0;
                for (const uint32_t finishedQuestId : getPlayer()->getFinishedQuests())
                {
                    if (QuestProperties const* qst = sMySQLStore.getQuestProperties(finishedQuestId))
                        if (finishedQuestId == achievementCriteria->complete_quest.questID)
                            ++completed;
                }

                setCriteriaProgress(achievementCriteria, completed);
            } break;
            case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS:
            {
                // achievementCriteria field4 = 777 for mounts, 778 for companion pets
                auto sl = getPlayer()->getSpellSet().begin();
                uint32_t nm = 0;
                while (sl != getPlayer()->getSpellSet().end())
                {
                    SpellInfo const* sp = sSpellMgr.getSpellInfo(*sl);
                    if (achievementCriteria->number_of_mounts.unknown == 777 && sp && sp->getMechanicsType() == MECHANIC_MOUNTED)
                    {
                        // mount spell
                        ++nm;
                    }
                    else if (achievementCriteria->number_of_mounts.unknown == 778 && sp && (sp->getEffect(0) == SPELL_EFFECT_SUMMON))
                    {
                        // Companion pet? Make sure it's a companion pet, not some other summon-type spell
                        // temporary solution since spell description is no longer loaded -Appled
                        const auto creatureEntry = sp->getEffectMiscValue(0);
                        auto creatureProperties = sMySQLStore.getCreatureProperties(creatureEntry);
                        if (creatureProperties != nullptr && creatureProperties->Type == UNIT_TYPE_NONCOMBAT_PET)
                            ++nm;
                    }
                    ++sl;
                }
                setCriteriaProgress(achievementCriteria, nm);
            } break;
            //End of Achievement List
            default:
                break;
        }
        completedCriteria(achievementCriteria);
    }
}

bool AchievementMgr::updateAchievementCriteria(Player* _player, int32_t _criteriaId, uint32_t _count)
{
    const auto criteria = sAchievementCriteriaStore.lookupEntry(_criteriaId);
    if (!criteria)
    {
        sLogger.debug("Achievement ID {} is Invalid", _criteriaId);
        return false;
    }
    if (isCompletedCriteria(criteria))
    {
        sLogger.debug("Achievement criteria {} already completed.", _criteriaId);
        return false;
    }
    auto* achievement = sAchievementStore.lookupEntry(criteria->referredAchievement);
    if (!achievement)
    {
        // achievement not found
        sLogger.debug("Referred achievement ({}) entry not found.", criteria->referredAchievement);
        return false;
    }
    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
    {
        // can't complete this type of achivement (counter)
        sLogger.debug("AchievementMgr Referred achievement ({}) |Hachievement:{}:{}:0:0:0:-1:0:0:0:0|h[{}]|h is a counter and cannot be completed.",
            achievement->ID, achievement->ID, std::to_string(_player->getGuid()), achievement->name[0]);
        return false;
    }

    const auto [itr, _] = m_criteriaProgress.try_emplace(_criteriaId, Util::LazyInstanceCreator([_criteriaId] {
        return std::make_unique<CriteriaProgress>(_criteriaId, 0);
    }));

    auto* progress = itr->second.get();
    progress->counter = progress->counter + _count;
    sendCriteriaUpdate(progress);
    completedCriteria(criteria);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Returns the number of achievement progresses that get sent to the player.
uint32_t AchievementMgr::getCriteriaProgressCount()
{
    uint32_t criteriapc = 0;
    for (const auto& iterCriteriaProgress : m_criteriaProgress)
    {
        //AchievementEntry const *achievement = dbcAchievementStore.lookupEntry(iterCriteriaProgress.second->id);
        if (canSendAchievementProgress(iterCriteriaProgress.second.get()))
            ++criteriapc;
    }
    return criteriapc;
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
#if VERSION_STRING > WotLK
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_BATTLEGROUND:  // NYI
#endif
            return true;
        default:
            break;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// \brief GM has used a command to make the specified achievement criteria to be completed.
/// If finishAll is true, all achievement criteria get marked as completed
/// \return true if able to complete the achievement criteria, otherwise false
bool AchievementMgr::gmCompleteCriteria(WorldSession* _gmSession, uint32_t _criteriaId, bool _finishAll/* = false*/)
{
    if (_finishAll)
    {
        uint32_t nr = sAchievementCriteriaStore.getNumRows();
        for (uint32_t i = 0, j = 0; j < nr; ++i)
        {
            WDB::Structures::AchievementCriteriaEntry const* crt = sAchievementCriteriaStore.lookupEntry(i);
            if (crt == nullptr)
            {
                sLogger.failure("Achievement Criteria {} entry not found.", i);
                continue;
            }
            ++j;
            if (crt->raw.field4)
            {
                setCriteriaProgress(crt, crt->raw.field4);
                completedCriteria(crt);
            }
        }
        m_player->getSession()->SystemMessage("All achievement criteria completed.");
        return true;
    }

    const auto criteria = sAchievementCriteriaStore.lookupEntry(_criteriaId);
    if (!criteria)
    {
        _gmSession->SystemMessage("Achievement criteria %d not found.", _criteriaId);
        return false;
    }

    if (isCompletedCriteria(criteria))
    {
        _gmSession->SystemMessage("Achievement criteria %d already completed.", _criteriaId);
        return false;
    }

    const auto achievement = sAchievementStore.lookupEntry(criteria->referredAchievement);
    if (!achievement)
    {
        // achievement not found
        _gmSession->SystemMessage("Referred achievement (%u) entry not found.", criteria->referredAchievement);
        return false;
    }

    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
    {
        // can't complete this type of achivement (counter)
        _gmSession->SystemMessage("Referred achievement (%u) |Hachievement:%u:%s:0:0:0:-1:0:0:0:0|h[%s]|h is a counter and cannot be completed.",
            achievement->ID, achievement->ID, std::to_string(_gmSession->GetPlayer()->getGuid()).c_str(), achievement->name);
        return false;
    }

    const auto [itr, _] = m_criteriaProgress.try_emplace(_criteriaId, Util::LazyInstanceCreator([_criteriaId] {
        return std::make_unique<CriteriaProgress>(_criteriaId, 0);
    }));

    auto* progress = itr->second.get();
    progress->counter = criteria->raw.field4;
    sendCriteriaUpdate(progress);
    completedCriteria(criteria);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// GM has used a command to reset achievement criteria for this player. If criteriaID
/// is finishAll true, all achievement criteria get reset, otherwise only the one specified gets reset
void AchievementMgr::gmResetCriteria(uint32_t _criteriaId, bool _finishAll/* = false*/)
{
    if (_finishAll)
    {
        for (const auto& criteriaProgress : m_criteriaProgress)
        {
            getPlayer()->sendPacket(SmsgCriteriaDeleted(criteriaProgress.first).serialise().get());
        }

        m_criteriaProgress.clear();
        CharacterDatabase.Execute("DELETE FROM character_achievement_progress WHERE guid = %u", m_player->getGuidLow());
    }
    else
    {
        getPlayer()->sendPacket(SmsgCriteriaDeleted(_criteriaId).serialise().get());

        m_criteriaProgress.erase(_criteriaId);
        CharacterDatabase.Execute("DELETE FROM character_achievement_progress WHERE guid = %u AND criteria = %u", m_player->getGuidLow(), static_cast<uint32_t>(_criteriaId));
    }

    updateAllAchievementCriteria();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Sends message to player(s) that the achievement has been completed.
/// Realm first! achievements get sent to all players currently online.
/// All other achievements get sent to all of the achieving player's guild members,
/// group members, and other in-range players
#if VERSION_STRING <= WotLK
void AchievementMgr::sendAllAchievementData(Player* _player)
{
    // maximum size for the SMSG_ALL_ACHIEVEMENT_DATA packet without causing client problems seems to be 0x7fff
    uint32_t packetSize = 18 + (static_cast<uint32_t>(m_completedAchievements.size()) * 8) + (getCriteriaProgressCount() * 36);
    bool doneCompleted = false;
    bool doneProgress = false;
    WDB::Structures::AchievementCriteriaEntry const* acEntry;
    WDB::Structures::AchievementEntry const* achievement;

    WorldPacket data;
    if (packetSize < 0x8000)
        data.resize(packetSize);
    else
        data.resize(0x7fff);

    CompletedAchievementMap::iterator completeIter = m_completedAchievements.begin();
    CriteriaProgressMap::iterator progressIter = m_criteriaProgress.begin();
    bool packetFull;

    while (!doneCompleted || !doneProgress)
    {
        data.clear();
        if (_player == m_player)
        {
            data.SetOpcode(SMSG_ALL_ACHIEVEMENT_DATA);
        }
        else
        {
            data.SetOpcode(SMSG_RESPOND_INSPECT_ACHIEVEMENTS);
            FastGUIDPack(data, m_player->getGuid());
        }
        packetFull = false;

        // add the completed achievements
        if (!doneCompleted)
        {
            for (; completeIter != m_completedAchievements.end() && !packetFull; ++completeIter)
            {
                if (showCompletedAchievement(completeIter->first, m_player))
                {
                    data << uint32_t(completeIter->first);
                    data << uint32_t(secsToTimeBitFields(completeIter->second));
                }
                packetFull = data.size() > 0x7f00;
            }
            if (completeIter == m_completedAchievements.end())
            {
                doneCompleted = true;
            }
        }

        // 0xffffffff separates between completed achievements and ones in progress
        data << int32_t(-1);
        for (; progressIter != m_criteriaProgress.end() && !packetFull; ++progressIter)
        {
            acEntry = sAchievementCriteriaStore.lookupEntry(progressIter->first);
            if (!acEntry)
            {
                continue;
            }
            achievement = sAchievementStore.lookupEntry(acEntry->referredAchievement);
            if (!achievement)
            {
                continue;
            }
            // achievement progress to send to self
            if (_player == m_player)
            {
                if (canSendAchievementProgress(progressIter->second.get()))
                {
                    data << uint32_t(progressIter->first);
                    data.appendPackGUID(progressIter->second->counter);
                    data << getPlayer()->GetNewGUID();
                    data << uint32_t(0);
                    data << uint32_t(secsToTimeBitFields(progressIter->second->date));
                    data << uint32_t(0);
                    data << uint32_t(0);
                }
            }
            // achievement progress to send to other players (inspect)
            else
            {
                // only send statistics, no other unfinished achievement progress, since client only displays them as completed or not completed
                if ((progressIter->second->counter > 0) && (achievement->flags & ACHIEVEMENT_FLAG_COUNTER))
                {
                    data << uint32_t(progressIter->first);
                    data.appendPackGUID(progressIter->second->counter);
                    data << getPlayer()->GetNewGUID();
                    data << uint32_t(0);
                    data << uint32_t(secsToTimeBitFields(progressIter->second->date));
                    data << uint32_t(0);
                    data << uint32_t(0);
                }
            }
            packetFull = data.size() > 0x7f00;
        }
        if (progressIter == m_criteriaProgress.end())
        {
            doneProgress = true;
        }

        // another 0xffffffff denotes end of the packet
        data << int32_t(-1);
        _player->getSession()->SendPacket(&data);
    }
    if (isCharacterLoading && _player == m_player)
    {
        // a SMSG_ALL_ACHIEVEMENT_DATA packet has been sent to the player, so the achievement manager can send SMSG_CRITERIA_UPDATE and SMSG_ACHIEVEMENT_EARNED when it gets them
        isCharacterLoading = false;
    }
}

#elif VERSION_STRING == Cata

struct VisibleAchievementPred
{
    bool operator()(CompletedAchievementMap::value_type const& completedAchievementPair)
    {
        auto achievement = sAchievementStore.lookupEntry(completedAchievementPair.first);
        return achievement && !(achievement->flags & ACHIEVEMENT_FLAG_HIDDEN);
    }
};

void AchievementMgr::sendAllAchievementData(Player* _player)
{
    VisibleAchievementPred isVisible;

    size_t numCriteria = m_criteriaProgress.size();
    size_t numAchievements = std::count_if(m_completedAchievements.begin(), m_completedAchievements.end(), isVisible);

    ByteBuffer criteriaData(m_criteriaProgress.size() * (4 + 4 + 4 + 4 + 8 + 8));
    ObjectGuid guid = m_player->getGuid();
    ObjectGuid counter;

    WorldPacket data(SMSG_ALL_ACHIEVEMENT_DATA, 4 + numAchievements * (4 + 4) + 4 + numCriteria * (4 + 4 + 4 + 4 + 8 + 8));
    data.writeBits(numCriteria, 21);

    for (const auto& progressIter : m_criteriaProgress)
    {
        WDB::Structures::AchievementCriteriaEntry const* acEntry = sAchievementCriteriaStore.lookupEntry(progressIter.first);
        if (!acEntry)
            continue;

        if (!sAchievementStore.lookupEntry(acEntry->referredAchievement))
            continue;

        counter = uint64_t(progressIter.second->counter);

        data.writeBit(guid[4]);
        data.writeBit(counter[3]);
        data.writeBit(guid[5]);
        data.writeBit(counter[0]);
        data.writeBit(counter[6]);
        data.writeBit(guid[3]);
        data.writeBit(guid[0]);
        data.writeBit(counter[4]);
        data.writeBit(guid[2]);
        data.writeBit(counter[7]);
        data.writeBit(guid[7]);
        data.writeBits(0u, 2);
        data.writeBit(guid[6]);
        data.writeBit(counter[2]);
        data.writeBit(counter[1]);
        data.writeBit(counter[5]);
        data.writeBit(guid[1]);

        criteriaData.WriteByteSeq(guid[3]);
        criteriaData.WriteByteSeq(counter[5]);
        criteriaData.WriteByteSeq(counter[6]);
        criteriaData.WriteByteSeq(guid[4]);
        criteriaData.WriteByteSeq(guid[6]);
        criteriaData.WriteByteSeq(counter[2]);
        criteriaData << uint32_t(0);    // timer 2
        criteriaData.WriteByteSeq(guid[2]);

        criteriaData << uint32_t(progressIter.first);   // criteria id
        criteriaData.WriteByteSeq(guid[5]);
        criteriaData.WriteByteSeq(counter[0]);
        criteriaData.WriteByteSeq(counter[3]);
        criteriaData.WriteByteSeq(counter[1]);
        criteriaData.WriteByteSeq(counter[4]);
        criteriaData.WriteByteSeq(guid[0]);
        criteriaData.WriteByteSeq(guid[7]);
        criteriaData.WriteByteSeq(counter[7]);
        criteriaData << uint32_t(0); // timer 1
        criteriaData.appendPackedTime(progressIter.second->date);   // criteria date
        criteriaData.WriteByteSeq(guid[1]);
    }

    data.writeBits(m_completedAchievements.size(), 23);
    data.flushBits();
    data.append(criteriaData);

    for (auto completeIter : m_completedAchievements)
    {
        if (!isVisible(completeIter))
            continue;

        data << uint32_t(completeIter.first);
        data.appendPackedTime(completeIter.second);
    }

    _player->getSession()->SendPacket(&data);

    if (isCharacterLoading && _player == m_player)
    {
        // a SMSG_ALL_ACHIEVEMENT_DATA packet has been sent to the player, so the achievement manager can send SMSG_CRITERIA_UPDATE and SMSG_ACHIEVEMENT_EARNED when it gets them
        isCharacterLoading = false;
    }
}

void AchievementMgr::sendRespondInspectAchievements(Player* _player)
{
    VisibleAchievementPred isVisible;

    ObjectGuid guid = m_player->getGuid();
    ObjectGuid counter;

    size_t numCriteria = m_criteriaProgress.size();
    size_t numAchievements = std::count_if(m_completedAchievements.begin(), m_completedAchievements.end(), isVisible);
    ByteBuffer criteriaData(numCriteria * (0));

    WorldPacket data(SMSG_RESPOND_INSPECT_ACHIEVEMENTS, 1 + 8 + 3 + 3 + numAchievements * (4 + 4) + numCriteria * (0));
    data.writeBit(guid[7]);
    data.writeBit(guid[4]);
    data.writeBit(guid[1]);
    data.writeBits(numAchievements, 23);
    data.writeBit(guid[0]);
    data.writeBit(guid[3]);
    data.writeBits(numCriteria, 21);
    data.writeBit(guid[2]);

    for (const auto& progressIter : m_criteriaProgress)
    {
        WDB::Structures::AchievementCriteriaEntry const* acEntry = sAchievementCriteriaStore.lookupEntry(progressIter.first);
        if (!acEntry)
            continue;

        if (!sAchievementStore.lookupEntry(acEntry->referredAchievement))
            continue;

        counter = uint64_t(progressIter.second->counter);

        data.writeBit(counter[5]);
        data.writeBit(counter[3]);
        data.writeBit(guid[1]);
        data.writeBit(guid[4]);
        data.writeBit(guid[2]);
        data.writeBit(counter[6]);
        data.writeBit(guid[0]);
        data.writeBit(counter[4]);
        data.writeBit(counter[1]);
        data.writeBit(counter[2]);
        data.writeBit(guid[3]);
        data.writeBit(guid[7]);
        data.writeBits(0, 2);   // criteria progress flags
        data.writeBit(counter[0]);
        data.writeBit(guid[5]);
        data.writeBit(guid[6]);
        data.writeBit(counter[7]);

        criteriaData.WriteByteSeq(guid[3]);
        criteriaData.WriteByteSeq(counter[4]);
        criteriaData << uint32_t(0);    // timer 1
        criteriaData.WriteByteSeq(guid[1]);
        criteriaData.appendPackedTime(progressIter.second->date);
        criteriaData.WriteByteSeq(counter[3]);
        criteriaData.WriteByteSeq(counter[7]);
        criteriaData.WriteByteSeq(guid[5]);
        criteriaData.WriteByteSeq(counter[0]);
        criteriaData.WriteByteSeq(guid[4]);
        criteriaData.WriteByteSeq(guid[2]);
        criteriaData.WriteByteSeq(guid[6]);
        criteriaData.WriteByteSeq(guid[7]);
        criteriaData.WriteByteSeq(counter[6]);
        criteriaData << uint32_t(progressIter.first);
        criteriaData << uint32_t(0);    // timer 2
        criteriaData.WriteByteSeq(counter[1]);
        criteriaData.WriteByteSeq(counter[5]);
        criteriaData.WriteByteSeq(guid[0]);
        criteriaData.WriteByteSeq(counter[2]);
    }

    data.writeBit(guid[6]);
    data.writeBit(guid[5]);
    data.flushBits();
    data.append(criteriaData);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[2]);

    for (auto completeIter : m_completedAchievements)
    {
        if (!isVisible(completeIter))
            continue;

        data << uint32_t(completeIter.first);
        data.appendPackedTime(completeIter.second);
    }

    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[5]);

    _player->getSession()->SendPacket(&data);
}
#else
struct VisibleAchievementPred
{
    bool operator()(CompletedAchievementMap::value_type const& completedAchievementPair)
    {
        auto achievement = sAchievementStore.lookupEntry(completedAchievementPair.first);
        return achievement && !(achievement->flags & ACHIEVEMENT_FLAG_HIDDEN);
    }
};

void AchievementMgr::sendAllAchievementData(Player* _player)
{
    VisibleAchievementPred isVisible;

    size_t numCriteria = m_criteriaProgress.size();
    size_t numAchievements = std::count_if(m_completedAchievements.begin(), m_completedAchievements.end(), isVisible);

    ByteBuffer criteriaData(m_criteriaProgress.size() * (4 + 4 + 4 + 4 + 8 + 8));
    ByteBuffer completedData(numAchievements * (4 + 4 + 4 + 4 + 8));
    ObjectGuid guid = m_player->getGuid();
    ObjectGuid counter;

    WorldPacket data(SMSG_ALL_ACHIEVEMENT_DATA, 4 + numAchievements * (4 + 4) + 4 + numCriteria * (4 + 4 + 4 + 4 + 8 + 8));
    data.writeBits(numCriteria, 21);

    for (const auto& progressIter : m_criteriaProgress)
    {
        WDB::Structures::AchievementCriteriaEntry const* acEntry = sAchievementCriteriaStore.lookupEntry(progressIter.first);
        if (!acEntry)
            continue;

        if (!sAchievementStore.lookupEntry(acEntry->referredAchievement))
            continue;

        counter = uint64_t(progressIter.second->counter);

        data.writeBit(counter[3]);
        data.writeBit(guid[3]);
        data.writeBit(guid[6]);
        data.writeBit(counter[0]);
        data.writeBit(guid[7]);
        data.writeBit(counter[1]);
        data.writeBit(counter[5]);
        data.writeBit(guid[2]);
        data.writeBit(guid[1]);
        data.writeBit(counter[7]);
        data.writeBit(guid[4]);
        data.writeBit(guid[0]);
        data.writeBit(counter[2]);
        data.writeBit(guid[5]);
        data.writeBit(counter[4]);
        data.writeBits(0, 4);
        data.writeBit(counter[6]);


        criteriaData.WriteByteSeq(counter[7]);
        criteriaData << uint32_t(0);                                // timer 1
        criteriaData.WriteByteSeq(counter[6]);
        criteriaData.WriteByteSeq(guid[1]);
        criteriaData << uint32_t(progressIter.first);               // criteria id
        criteriaData.WriteByteSeq(counter[4]);
        criteriaData.WriteByteSeq(guid[0]);
        criteriaData.WriteByteSeq(guid[4]);
        criteriaData.WriteByteSeq(guid[6]);
        criteriaData.WriteByteSeq(counter[1]);
        criteriaData.WriteByteSeq(counter[5]);
        criteriaData.WriteByteSeq(guid[7]);
        criteriaData.WriteByteSeq(guid[2]);
        criteriaData.WriteByteSeq(counter[2]);
        criteriaData.WriteByteSeq(counter[0]);
        criteriaData.WriteByteSeq(guid[3]);
        criteriaData.WriteByteSeq(counter[3]);
        criteriaData << uint32_t(0);                                // timer 2
        criteriaData.WriteByteSeq(guid[5]);
        criteriaData.appendPackedTime(progressIter.second->date);   // criteria date
    }

    data.writeBits(m_completedAchievements.size(), 20);
    for (auto completeIter : m_completedAchievements)
    {
        if (!isVisible(completeIter))
            continue;

        data.writeBit(guid[0]);
        data.writeBit(guid[7]);
        data.writeBit(guid[1]);
        data.writeBit(guid[5]);
        data.writeBit(guid[2]);
        data.writeBit(guid[4]);
        data.writeBit(guid[6]);
        data.writeBit(guid[3]);

        completedData << uint32_t(completeIter.first);              // achievement Id
        completedData << uint32_t(1);
        completedData.WriteByteSeq(guid[5]);
        completedData.WriteByteSeq(guid[7]);
        completedData << uint32_t(1);
        completedData.appendPackedTime(completeIter.second);        // achievement date
        completedData.WriteByteSeq(guid[0]);
        completedData.WriteByteSeq(guid[4]);
        completedData.WriteByteSeq(guid[1]);
        completedData.WriteByteSeq(guid[6]);
        completedData.WriteByteSeq(guid[2]);
        completedData.WriteByteSeq(guid[3]);

        data << uint32_t(completeIter.first);
        data.appendPackedTime(completeIter.second);
    }

    data.flushBits();
    data.append(completedData);
    data.append(criteriaData);

    _player->getSession()->SendPacket(&data);

    if (isCharacterLoading && _player == m_player)
    {
        // a SMSG_ALL_ACHIEVEMENT_DATA packet has been sent to the player, so the achievement manager can send SMSG_CRITERIA_UPDATE and SMSG_ACHIEVEMENT_EARNED when it gets them
        isCharacterLoading = false;
    }
}

void AchievementMgr::sendRespondInspectAchievements(Player* _player)
{
    VisibleAchievementPred isVisible;

    ObjectGuid guid = m_player->getGuid();
    ObjectGuid counter;

    size_t numCriteria = m_criteriaProgress.size();
    size_t numAchievements = std::count_if(m_completedAchievements.begin(), m_completedAchievements.end(), isVisible);
    ByteBuffer criteriaData(numCriteria * (0));

    WorldPacket data(SMSG_RESPOND_INSPECT_ACHIEVEMENTS, 1 + 8 + 3 + 3 + numAchievements * (4 + 4) + numCriteria * (0));
    data.writeBit(guid[7]);
    data.writeBit(guid[4]);
    data.writeBit(guid[1]);
    data.writeBits(numAchievements, 23);
    data.writeBit(guid[0]);
    data.writeBit(guid[3]);
    data.writeBits(numCriteria, 21);
    data.writeBit(guid[2]);

    for (const auto& progressIter : m_criteriaProgress)
    {
        WDB::Structures::AchievementCriteriaEntry const* acEntry = sAchievementCriteriaStore.lookupEntry(progressIter.first);
        if (!acEntry)
            continue;

        if (!sAchievementStore.lookupEntry(acEntry->referredAchievement))
            continue;

        counter = uint64_t(progressIter.second->counter);

        data.writeBit(counter[5]);
        data.writeBit(counter[3]);
        data.writeBit(guid[1]);
        data.writeBit(guid[4]);
        data.writeBit(guid[2]);
        data.writeBit(counter[6]);
        data.writeBit(guid[0]);
        data.writeBit(counter[4]);
        data.writeBit(counter[1]);
        data.writeBit(counter[2]);
        data.writeBit(guid[3]);
        data.writeBit(guid[7]);
        data.writeBits(0, 2);   // criteria progress flags
        data.writeBit(counter[0]);
        data.writeBit(guid[5]);
        data.writeBit(guid[6]);
        data.writeBit(counter[7]);

        criteriaData.WriteByteSeq(guid[3]);
        criteriaData.WriteByteSeq(counter[4]);
        criteriaData << uint32_t(0);    // timer 1
        criteriaData.WriteByteSeq(guid[1]);
        criteriaData.appendPackedTime(progressIter.second->date);
        criteriaData.WriteByteSeq(counter[3]);
        criteriaData.WriteByteSeq(counter[7]);
        criteriaData.WriteByteSeq(guid[5]);
        criteriaData.WriteByteSeq(counter[0]);
        criteriaData.WriteByteSeq(guid[4]);
        criteriaData.WriteByteSeq(guid[2]);
        criteriaData.WriteByteSeq(guid[6]);
        criteriaData.WriteByteSeq(guid[7]);
        criteriaData.WriteByteSeq(counter[6]);
        criteriaData << uint32_t(progressIter.first);
        criteriaData << uint32_t(0);    // timer 2
        criteriaData.WriteByteSeq(counter[1]);
        criteriaData.WriteByteSeq(counter[5]);
        criteriaData.WriteByteSeq(guid[0]);
        criteriaData.WriteByteSeq(counter[2]);
    }

    data.writeBit(guid[6]);
    data.writeBit(guid[5]);
    data.flushBits();
    data.append(criteriaData);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[2]);

    for (auto completeIter : m_completedAchievements)
    {
        if (!isVisible(completeIter))
            continue;

        data << uint32_t(completeIter.first);
        data.appendPackedTime(completeIter.second);
    }

    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[5]);

    _player->getSession()->SendPacket(&data);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
/// \brief GM has used a command to make the specified achievement to be completed.
/// If finishAll is true, all achievements available for the player's faction get
/// marked as completed
/// \return true if able to complete specified achievement successfully, otherwise false
bool AchievementMgr::gmCompleteAchievement(WorldSession* _gmSession, uint32_t _achievementId, bool _finishAll/* = false*/)
{
    if (_finishAll)
    {
        uint32_t nr = sAchievementStore.getNumRows();

        for (uint32_t i = 0; i < nr; ++i)
        {
            auto achievementEntry = sAchievementStore.lookupEntry(i);
            if (achievementEntry == nullptr)
            {
                m_player->getSession()->SystemMessage("Achievement %u entry not found.", i);
            }
            else
            {
                if (!(achievementEntry->flags & ACHIEVEMENT_FLAG_COUNTER))
                {
                    if ((achievementEntry->factionFlag == ACHIEVEMENT_FACTION_FLAG_HORDE && !m_player->isTeamHorde()) ||
                        (achievementEntry->factionFlag == ACHIEVEMENT_FACTION_FLAG_ALLIANCE && !m_player->isTeamAlliance()))
                    {
                        continue;
                    }
                    completedAchievement(achievementEntry);
                }
            }
        }
        m_player->getSession()->SystemMessage("All achievements completed.");
        return true;
    }

    if (m_completedAchievements.contains(_achievementId))
    {
        _gmSession->SystemMessage("Player has already completed that achievement.");
        return false;
    }

    const auto achievement = sAchievementStore.lookupEntry(_achievementId);
    if (!achievement)
    {
        _gmSession->SystemMessage("Achievement %d entry not found.", _achievementId);
        return false;
    }

    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
    {
        _gmSession->SystemMessage("Achievement (%u) |Hachievement:%u:%s:0:0:0:-1:0:0:0:0|h[%s]|h is a counter and cannot be completed.",
            achievement->ID, achievement->ID, std::to_string(_gmSession->GetPlayer()->getGuid()).c_str(), achievement->name);
        return false;
    }
    completedAchievement(achievement);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// \brief GM has used a command to reset achievement(s) for this player. If
/// finishAll is true, all achievements get reset, otherwise the one specified gets reset
void AchievementMgr::gmResetAchievement(uint32_t _achievementId, bool _finishAll/* = false*/)
{
    if (_finishAll)
    {
        for (const auto& completedAchievement : m_completedAchievements)
            getPlayer()->sendPacket(SmsgAchievementDeleted(completedAchievement.first).serialise().get());

        m_completedAchievements.clear();
        CharacterDatabase.Execute("DELETE FROM character_achievement WHERE guid = %u", m_player->getGuidLow());
    }
    else
    {
        getPlayer()->sendPacket(SmsgAchievementDeleted(_achievementId).serialise().get());

        m_completedAchievements.erase(_achievementId);
        CharacterDatabase.Execute("DELETE FROM character_achievement WHERE guid = %u AND achievement = %u", m_player->getGuidLow(), static_cast<uint32_t>(_achievementId));
    }
}

time_t AchievementMgr::getCompletedTime(WDB::Structures::AchievementEntry const* _achievement)
{
    auto iter = m_completedAchievements.find(_achievement->ID);
    if (iter != m_completedAchievements.end())
        return iter->second;
    return 0;
}

uint32_t AchievementMgr::getCompletedAchievementsCount() const
{
    return static_cast<uint32_t>(m_completedAchievements.size());
}

bool AchievementMgr::hasCompleted(uint32_t _achievementId) const
{
    return m_completedAchievements.contains(_achievementId);
}

Player* AchievementMgr::getPlayer() const { return m_player; }

//////////////////////////////////////////////////////////////////////////////////////////
/// Completes the achievement for the player.
void AchievementMgr::completedAchievement(WDB::Structures::AchievementEntry const* achievement)
{
    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER || m_completedAchievements.find(achievement->ID) != m_completedAchievements.end())
        return;

    if (showCompletedAchievement(achievement->ID, getPlayer()))
        sendAchievementEarned(achievement);

    m_completedAchievements[achievement->ID] = time(nullptr);

    sObjectMgr.addCompletedAchievement(achievement->ID);
    updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT);

    // check for reward
    giveAchievementReward(achievement);
}

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
            auto achievementResult = CharacterDatabase.Query("SELECT guid FROM character_achievement WHERE achievement=%u ORDER BY date LIMIT 1", _achievementId);
            if (achievementResult != nullptr)
            {
                Field* field = achievementResult->Fetch();
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
                getPlayer()->setKnownPvPTitle(static_cast<RankTitles>(char_title->bit_index), true);
        }
    }
    if (getPlayer()->getTeam() == TEAM_HORDE)
    {
        if (Reward->titel_H)
        {
            auto char_title = sCharTitlesStore.lookupEntry(Reward->titel_H);
            if (char_title)
                getPlayer()->setKnownPvPTitle(static_cast<RankTitles>(char_title->bit_index), true);
        }
    }

    //Reward Mail
    if (Reward->sender)
    {
        Creature* creature = getPlayer()->getWorldMap()->createCreature(Reward->sender);
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
            creature->Delete();
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
        sWorld.sendGlobalMessage(SmsgServerFirstAchievement(playerName, guid, _entry->ID, 1).serialise().get(), nullptr, getPlayer()->GetTeam());

        sWorld.sendGlobalMessage(SmsgServerFirstAchievement(playerName, guid, _entry->ID, 0).serialise().get(), nullptr, getPlayer()->GetTeam() == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE);
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
                                loggedInPlayer->getSession()->SendPacket(SmsgMessageChat(CHAT_MSG_ACHIEVEMENT, LANG_UNIVERSAL, 0, msg, getPlayer()->getGuid(), "", getPlayer()->getGuid(), "", _entry->ID).serialise().get());
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
                    player->getSession()->SendPacket(SmsgMessageChat(CHAT_MSG_ACHIEVEMENT, LANG_UNIVERSAL, 0, msg, getPlayer()->getGuid(), "", getPlayer()->getGuid(), "", _entry->ID).serialise().get());
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
                getPlayer()->getSession()->SendPacket(SmsgMessageChat(CHAT_MSG_ACHIEVEMENT, LANG_UNIVERSAL, 0, msg, getPlayer()->getGuid(), "", getPlayer()->getGuid(), "", _entry->ID).serialise().get());
        }
    }
    //    GetPlayer()->sendMessageToSet(&cdata, true);

    WorldPacket data(SMSG_ACHIEVEMENT_EARNED, 30);
    data << getPlayer()->GetNewGUID();
    data << uint32_t(_entry->ID);
    data << uint32_t(secsToTimeBitFields(UNIXTIME));
    data << uint32_t(0);

    getPlayer()->getSession()->SendPacket(&data);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Returns the completion state of the achievement.
/// \brief ACHIEVEMENT_COMPLETED_COMPLETED_STORED: has been completed and stored already.
/// ACHIVEMENT_COMPLETED_COMPLETED_NOT_STORED: has been completed but not stored yet.
/// ACHIEVEMENT_COMPLETED_NONE: has not been completed yet
AchievementCompletionState AchievementMgr::getAchievementCompletionState(WDB::Structures::AchievementEntry const* _entry)
{
    if (m_completedAchievements.contains(_entry->ID))
        return ACHIEVEMENT_COMPLETED_COMPLETED_STORED;

    uint32_t completedCount = 0;
    bool foundOutstanding = false;
    for (uint32_t rowId = 0; rowId < sAchievementCriteriaStore.getNumRows(); ++rowId)
    {
        const auto criteria = sAchievementCriteriaStore.lookupEntry(rowId);
        if (criteria == nullptr || criteria->referredAchievement != _entry->ID)
            continue;

        if (isCompletedCriteria(criteria) && criteria->completionFlag & ACHIEVEMENT_CRITERIA_COMPLETE_FLAG_ALL)
            return ACHIEVEMENT_COMPLETED_COMPLETED_NOT_STORED;

        if (!isCompletedCriteria(criteria))
            foundOutstanding = true;
        else
            ++completedCount;
    }

    if (!foundOutstanding)
        return ACHIEVEMENT_COMPLETED_COMPLETED_NOT_STORED;

    if (_entry->count > 1 && completedCount >= _entry->count)
        return ACHIEVEMENT_COMPLETED_COMPLETED_NOT_STORED;

    return ACHIEVEMENT_COMPLETED_NONE;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// True if CriteriaProgress should be sent to Player; False if CriteriaProgress should not be sent.
///    If the CriteriaProgress specified should not be sent to the Player, it returns false, otherwise it returns true.
///    Examples of CriteriaProgress that should not be sent to the Player are:
///    1. When counter is zero or negative, which would indicate the achievement hasn't been started yet.
///    2. Reputation type achievements, where the progress is not shown in the client.
///    3. Reach-Level type achievements, where the progress is not shown in the client.
bool AchievementMgr::canSendAchievementProgress(const CriteriaProgress* _criteriaProgress)
{
    // achievement not started yet, don't send progress
    if (_criteriaProgress == nullptr || _criteriaProgress->counter <= 0)
        return false;

    const auto acEntry = sAchievementCriteriaStore.lookupEntry(_criteriaProgress->id);
    if (!acEntry)
        return false;

    // Exalted with X faction (don't send 12323/42000 progress, it's not shown anyway)
    if (acEntry->requiredType == ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION)
        return false;

    // Reach level (don't send 7/80 progress, it's not shown anyway)
    if (acEntry->requiredType == ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL)
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
///  True if CriteriaProgress should be saved to database.  False if CriteriaProgress should not be saved to database.
///    Not all achievement progresses get saved to progress database, since some are saved in the character database,
///    or are easily computable when the player logs in.
bool AchievementMgr::canSaveAchievementProgressToDB(const CriteriaProgress* _criteriaProgress)
{
    // don't save it if it's not started yet
    if (_criteriaProgress->counter <= 0)
        return false;

    auto achievement = sAchievementCriteriaStore.lookupEntry(_criteriaProgress->id);
    if (achievement == nullptr)
        return false;

    switch (achievement->requiredType)
    {
        // these get updated when character logs on, don't save to character progress db
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS:
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
            return false;
        default:
            break;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// \brief Sends update to achievement criteria to the player.
void AchievementMgr::sendCriteriaUpdate(const CriteriaProgress* _criteriaProgress)
{
    if (_criteriaProgress == nullptr || isCharacterLoading)
        return;

    getPlayer()->sendPacket(SmsgCriteriaUpdate(_criteriaProgress->id, _criteriaProgress->counter, getPlayer()->GetNewGUID(), secsToTimeBitFields(_criteriaProgress->date)).serialise().get());
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Sets progress of the achievement criteria.
/// \brief If relative argument is true, this behaves the same as UpdateCriteriaProgress
void AchievementMgr::setCriteriaProgress(WDB::Structures::AchievementCriteriaEntry const* _entry, int32_t _newValue, bool /*relative*/)
{
    CriteriaProgress* progress;

    if (!m_criteriaProgress.contains(_entry->ID))
    {
        if (_newValue < 1)
            return;

        const auto [progressItr, _] = m_criteriaProgress.try_emplace(_entry->ID, std::make_unique<CriteriaProgress>(_entry->ID, _newValue));
        progress = progressItr->second.get();
    }
    else
    {
        progress = m_criteriaProgress[_entry->ID].get();
        if (progress->counter == static_cast<uint32_t>(_newValue))
            return;

        progress->counter = _newValue;
    }

    // Send update only if criteria is started (counter > 0)
    if (progress->counter > 0)
        sendCriteriaUpdate(progress);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Updates progress of the achievement criteria.
/// \brief updateByValue is added to the current progress counter
void AchievementMgr::updateCriteriaProgress(WDB::Structures::AchievementCriteriaEntry const* _entry, int32_t _updateByValue)
{
    CriteriaProgress* progress;

    if (!m_criteriaProgress.contains(_entry->ID))
    {
        if (_updateByValue < 1)
            return;

        const auto [progressItr, _] = m_criteriaProgress.try_emplace(_entry->ID, std::make_unique<CriteriaProgress>(_entry->ID, _updateByValue));
        progress = progressItr->second.get();
    }
    else
    {
        progress = m_criteriaProgress[_entry->ID].get();
        progress->counter += _updateByValue;
    }

    // Send update only if criteria is started (counter > 0)
    if (progress->counter > 0)
        sendCriteriaUpdate(progress);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// If achievement criteria has been completed, checks whether to complete the achievement too.
void AchievementMgr::completedCriteria(WDB::Structures::AchievementCriteriaEntry const* criteria)
{
    if (!isCompletedCriteria(criteria))
        return;

    const auto achievement = sAchievementStore.lookupEntry(criteria->referredAchievement);
    if (achievement == nullptr)
        return;

    if (criteria->completionFlag & ACHIEVEMENT_CRITERIA_COMPLETE_FLAG_ALL || getAchievementCompletionState(achievement) == ACHIEVEMENT_COMPLETED_COMPLETED_NOT_STORED)
        completedAchievement(achievement);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// \return True if the criteria has been completed otherwise false (error...)
bool AchievementMgr::isCompletedCriteria(WDB::Structures::AchievementCriteriaEntry const* achievementCriteria)
{
    if (!achievementCriteria)
        return false;

    const auto achievement = sAchievementStore.lookupEntry(achievementCriteria->referredAchievement);
    if (achievement == nullptr)
        return false;

    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
        return false;

    if (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
        if (sObjectMgr.isInCompletedAchievements(achievement->ID))
            return false;

    const auto criteriaProgressIter = m_criteriaProgress.find(achievementCriteria->ID);
    if (criteriaProgressIter == m_criteriaProgress.end())
        return false;

    const CriteriaProgress* progress = criteriaProgressIter->second.get();
    if (progress->counter < 1)
        return false;

    uint32_t progresscounter = progress->counter;
    switch (achievementCriteria->requiredType)
    {
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
            if ((achievement->ID == 467 && getPlayer()->getClass() != SHAMAN) ||
                (achievement->ID == 466 && getPlayer()->getClass() != DRUID) ||
                (achievement->ID == 465 && getPlayer()->getClass() != PALADIN) ||
                (achievement->ID == 464 && getPlayer()->getClass() != PRIEST) ||
                (achievement->ID == 463 && getPlayer()->getClass() != WARLOCK) ||
                (achievement->ID == 462 && getPlayer()->getClass() != HUNTER) ||
                (achievement->ID == 461 && getPlayer()->getClass() != DEATHKNIGHT) ||
                (achievement->ID == 460 && getPlayer()->getClass() != MAGE) ||
                (achievement->ID == 459 && getPlayer()->getClass() != WARRIOR) ||
                (achievement->ID == 458 && getPlayer()->getClass() != ROGUE) ||
                (achievement->ID == 1404 && getPlayer()->getRace() != RACE_GNOME) ||
                (achievement->ID == 1405 && getPlayer()->getRace() != RACE_BLOODELF) ||
                (achievement->ID == 1406 && getPlayer()->getRace() != RACE_DRAENEI) ||
                (achievement->ID == 1407 && getPlayer()->getRace() != RACE_DWARF) ||
                (achievement->ID == 1408 && getPlayer()->getRace() != RACE_HUMAN) ||
                (achievement->ID == 1409 && getPlayer()->getRace() != RACE_NIGHTELF) ||
                (achievement->ID == 1410 && getPlayer()->getRace() != RACE_ORC) ||
                (achievement->ID == 1411 && getPlayer()->getRace() != RACE_TAUREN) ||
                (achievement->ID == 1412 && getPlayer()->getRace() != RACE_TROLL) ||
                (achievement->ID == 1413 && getPlayer()->getRace() != RACE_UNDEAD))
            {
                return false;
            }
            return progresscounter >= achievementCriteria->reach_level.level;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM:
            return progresscounter >= achievementCriteria->loot_item.itemCount;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY:
            return progresscounter >= achievementCriteria->loot_money.goldInCopper;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
            return progresscounter >= achievementCriteria->complete_quest_count.totalQuestCount;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            return progresscounter >= achievementCriteria->complete_quests_in_zone.questCount;
        case ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD:
            return progresscounter >= achievementCriteria->quest_reward_money.goldInCopper;
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
            return progresscounter >= achievementCriteria->gain_reputation.reputationAmount;
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
            return progresscounter >= achievementCriteria->gain_exalted_reputation.numberOfExaltedFactions;
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS:
            return progresscounter >= achievementCriteria->number_of_mounts.mountCount;
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
            return progresscounter >= achievementCriteria->be_spell_target.spellCount;
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
            return progresscounter >= achievementCriteria->kill_creature.creatureCount;
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
            return progresscounter >= achievementCriteria->reach_skill_level.skillLevel;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
            return progresscounter >= achievementCriteria->learn_skill_level.skillLevel;
        case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM:
            return progresscounter >= achievementCriteria->use_item.itemCount;
        case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT:
            return progresscounter >= achievementCriteria->use_gameobject.useCount;
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
            return progresscounter >= achievementCriteria->buy_bank_slot.numberOfSlots;
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL:
            return progresscounter >= achievementCriteria->honorable_kill.killCount;
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
            return progresscounter >= achievementCriteria->honorable_kill_at_area.killCount;
        case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS:
            return progresscounter >= achievementCriteria->hk_class.count;
        case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE:
            return progresscounter >= achievementCriteria->hk_race.count;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
            return m_completedAchievements.contains(achievementCriteria->complete_achievement.linkedAchievement);

        // These achievements only require counter to be 1 (or higher)
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
        case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP:
        case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
            return progresscounter >= 1;

        // unknown or need to be finished:
        default:
            if (achievementCriteria->raw.field4 > 0)
                return progresscounter >= achievementCriteria->raw.field4;
            break;
    }

    return false;
}
#endif
