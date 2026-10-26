/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AchievementConditions.hpp"

#include "AchievementTrees.hpp"
#include "Management/AchievementMgr.h"
#include "Management/Arenas.hpp"
#include "Management/Battleground/Battleground.hpp"
#include "Management/GameEvent.hpp"
#include "Management/GameEventMgr.hpp"
#include "Management/Group.h"
#include "Management/ItemProperties.hpp"
#include "Management/LFG/LFGMgr.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/CreatureDefines.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/AchievementScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Server/World.h"
#include "Server/WorldConfig.h"
#include "Server/WorldSession.h"
#include "Spell/AuraEffectModifier.hpp"
#include "Spell/Definitions/AuraEffects.hpp"
#include "Spell/Definitions/AuraStates.hpp"
#include "Spell/SpellAura.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Management/BattlePets/BattlePetCollection.hpp"
#include "Management/BattlePets/BattlePetMgr.hpp"
#include "Management/Conditions/PlayerConditions.hpp"
#include "Management/Conditions/WorldStateExpressions.hpp"

#include <algorithm>
#include <unordered_set>

namespace Achievements
{
    namespace
    {
        // modifier types of the client data, the values of 5.4 and later
        enum ConditionType : uint32_t
        {
            Inebriation                 = 1,
            PlayerCondition             = 2,
            MinimumItemLevel            = 3,
            TargetEntry                 = 4,
            TargetIsPlayer              = 5,
            TargetIsDead                = 6,
            TargetIsHostile             = 7,
            PlayerAura                  = 8,
            TargetAura                  = 10,
            TargetAuraEffect            = 11,
            TargetAuraState             = 12,
            ItemQualityAtLeast          = 14,
            ItemQualityExactly          = 15,
            PlayerIsAlive               = 16,
            PlayerArea                  = 17,
            TargetArea                  = 18,
            ItemEntry                   = 19,
            LegacyDifficulty            = 20,
            LevelAboveTarget            = 21,
            ArenaTeamSize               = 24,
            PlayerRace                  = 25,
            PlayerClass                 = 26,
            TargetRace                  = 27,
            TargetClass                 = 28,
            FewerTappers                = 29,
            TargetCreatureType          = 30,
            PlayerMap                   = 32,
            ClientVersionAtMost         = 33,
            BattlePetTeamLevel          = 34,
            PlayerWithoutGroup          = 35,
            PersonalRating              = 37,
            PlayerTitle                 = 38,
            PlayerLevelExactly          = 39,
            TargetLevelExactly          = 40,
            PlayerZone                  = 41,
            PlayerHealthBelow           = 43,
            TargetHealthBelow           = 46,
            TargetPlayerCondition       = 55,
            AchievementPointsAbove      = 56,
            RandomLfgDungeon            = 58,
            RatedArena                  = 60,
            GuildReputation             = 62,
            RatedBattleground           = 63,
            ResearchProjectRarity       = 65,
            ResearchProjectBranch       = 66,
            WorldStateExpression        = 67,
            Difficulty                  = 68,
            PlayerLevelAtLeast          = 69,
            TargetLevelAtLeast          = 70,
            LinkedModifierTree          = 73,
            TillersReputation           = 75,
            BattlePetAchievementPoints  = 76,
            BattlePetSpeciesType        = 78,
            GuildMembersInGroup         = 80,
            ChallengeModeMedal          = 83,
            PlayerOnQuest               = 84,
            AccountAchievement          = 86,
            CharacterAchievement        = 87,
            CloudSerpentReputation      = 88,
            BattlePetSpecies            = 91,
            ServerExpansionAtLeast      = 92,
            BattlePetJournalLock        = 93,
            FriendshipReaction          = 94,
            FactionReputation           = 95,
            ItemClassSubclass           = 96,
            PlayerGender                = 97,
            PlayerSkillAtLeast          = 99,
            TrackedCurrencyAtLeast      = 121,
            MapInstanceType             = 122,
            PvpSeason                   = 125
        };

        constexpr uint32_t FactionGuild = 1168;
        constexpr uint32_t FactionTillers = 1272;
        constexpr uint32_t FactionCloudSerpent = 1271;

        // the modifier types a condition can be decided for
        std::unordered_set<uint32_t> const handledTypes =
        {
            Inebriation, MinimumItemLevel, TargetEntry, TargetIsPlayer, TargetIsDead, TargetIsHostile, PlayerAura,
            TargetAura, TargetAuraEffect, TargetAuraState, ItemQualityAtLeast, ItemQualityExactly, PlayerIsAlive, PlayerArea,
            TargetArea, ItemEntry, LegacyDifficulty, LevelAboveTarget, ArenaTeamSize, PlayerRace, PlayerClass, TargetRace, TargetClass,
            FewerTappers, TargetCreatureType, PlayerMap, PlayerWithoutGroup, PersonalRating,
            PlayerTitle, PlayerLevelExactly, TargetLevelExactly, PlayerZone, PlayerHealthBelow, TargetHealthBelow,
            AchievementPointsAbove, RandomLfgDungeon, RatedArena, GuildReputation, RatedBattleground, Difficulty,
            PlayerLevelAtLeast, TargetLevelAtLeast, LinkedModifierTree, TillersReputation,
            GuildMembersInGroup, ChallengeModeMedal, PlayerOnQuest, AccountAchievement, CharacterAchievement,
            CloudSerpentReputation, ServerExpansionAtLeast, FactionReputation, ItemClassSubclass, PlayerGender, PlayerSkillAtLeast,
            MapInstanceType, PvpSeason,
            TrackedCurrencyAtLeast,
            PlayerCondition, TargetPlayerCondition, WorldStateExpression, BattlePetTeamLevel, BattlePetAchievementPoints,
            BattlePetSpeciesType, BattlePetSpecies, BattlePetJournalLock, FriendshipReaction
        };

        // the requirements of the world database for criteria the client data leaves open, param1 and param2 per kind
        enum class RequirementKind : uint8_t
        {
            // the target of the event
            TargetCreature              = 1,    // creature entry
            TargetClassRace             = 2,    // class, race of the target player, 0 for any
            TargetHealthBelow           = 3,    // health percent of the target player
            TargetCorpse                = 4,    // 1 own team, 0 the enemy, the corpse not released
            TargetAura                  = 5,    // spell, effect index
            TargetLevel                 = 6,    // lowest level
            TargetGender                = 7,
            TargetTeam                  = 8,    // 0 Alliance, 1 Horde

            // the player
            PlayerAura                  = 10,   // spell, effect index
            PlayerArea                  = 11,   // zone or area
            PlayerMap                   = 12,
            PlayerClassRace             = 13,   // class, race, 0 for any
            PlayerDrunkenState          = 14,   // lowest drunken state
            PlayerKnownTitle            = 15,   // CharTitles id

            // the map of the player
            MapDifficulty               = 20,   // lowest difficulty, a raid of the other size never counts
            MapPlayersAtMost            = 21,   // players without game masters

            // the event itself
            EventValue                  = 30,   // value, comparison: 0 equal, 1 higher, 2 lower, 3 at least, 4 at most
            EventItemLevelQuality       = 31,   // lowest item level, lowest quality
            EventItemQuality            = 32,   // quality

            // the world
            RunningHoliday              = 40,   // holiday id of a running game event
            RunningGameEvent            = 41,   // gameevent_properties entry
            GameAnniversary             = 42,   // the n-th anniversary of the game
            LosingTeamScore             = 43,   // lowest, highest score of the losing team

            // scripts
            CriteriaScript              = 50,   // the criteria script decides
            InstanceScript              = 51,   // the criteria script of the instance decides

            // the criteria never counts
            Disabled                    = 60
        };

        std::unordered_set<RequirementKind> const handledRequirements =
        {
            RequirementKind::TargetCreature, RequirementKind::TargetClassRace, RequirementKind::TargetHealthBelow,
            RequirementKind::TargetCorpse, RequirementKind::TargetAura, RequirementKind::TargetLevel, RequirementKind::TargetGender,
            RequirementKind::TargetTeam, RequirementKind::PlayerAura, RequirementKind::PlayerArea, RequirementKind::PlayerMap,
            RequirementKind::PlayerClassRace, RequirementKind::PlayerDrunkenState, RequirementKind::PlayerKnownTitle,
            RequirementKind::MapDifficulty, RequirementKind::MapPlayersAtMost, RequirementKind::EventValue,
            RequirementKind::EventItemLevelQuality, RequirementKind::EventItemQuality, RequirementKind::RunningHoliday,
            RequirementKind::RunningGameEvent, RequirementKind::CriteriaScript, RequirementKind::InstanceScript, RequirementKind::Disabled
        };

        bool hasAuraEffectAt(Unit* unit, uint32_t spellId, uint32_t effectIndex)
        {
            Aura const* aura = unit != nullptr ? unit->getAuraWithId(spellId) : nullptr;
            if (aura == nullptr || effectIndex >= MAX_SPELL_EFFECTS)
                return false;

            AuraEffectModifier const* effect = aura->getAuraEffect(static_cast<uint8_t>(effectIndex));
            return effect != nullptr && effect->getAuraEffectType() != SPELL_AURA_NONE;
        }

        bool compareValue(uint64_t value, uint32_t wanted, uint32_t comparison)
        {
            switch (comparison)
            {
                case 0: return value == wanted;
                case 1: return value > wanted;
                case 2: return value < wanted;
                case 3: return value >= wanted;
                case 4: return value <= wanted;
                default: return false;
            }
        }

        bool hasTitle(Player* player, uint32_t titleId)
        {
            auto const* title = sCharTitlesStore.lookupEntry(titleId);
            return title != nullptr && player->hasPvPTitle(static_cast<RankTitles>(title->bitIndex));
        }

        bool isRequirementMet(Criteria const& criteria, CriteriaRequirement const& requirement, Player* player, EventSubject const& subject)
        {
            uint32_t const param1 = requirement.param1;
            uint32_t const param2 = requirement.param2;
            Unit* target = subject.target;
            Player* targetPlayer = target != nullptr && target->isPlayer() ? static_cast<Player*>(target) : nullptr;

            switch (static_cast<RequirementKind>(requirement.kind))
            {
                case RequirementKind::TargetCreature:
                    return target != nullptr && target->isCreature() && target->getEntry() == param1;
                case RequirementKind::TargetClassRace:
                    return targetPlayer != nullptr && (param1 == 0 || targetPlayer->getClass() == param1) && (param2 == 0 || targetPlayer->getRace() == param2);
                case RequirementKind::TargetHealthBelow:
                    return targetPlayer != nullptr && targetPlayer->getHealthPct() <= param1;
                case RequirementKind::TargetCorpse:
                    if (targetPlayer == nullptr || targetPlayer->isAlive() || targetPlayer->getDeathState() == DEAD)
                        return false;
                    return (targetPlayer->getTeam() == player->getTeam()) == (param1 != 0);
                case RequirementKind::PlayerAura:
                    return hasAuraEffectAt(player, param1, param2);
                case RequirementKind::PlayerArea:
                    return player->getZoneId() == param1 || player->getAreaId() == param1;
                case RequirementKind::TargetAura:
                    return hasAuraEffectAt(target, param1, param2);
                case RequirementKind::EventValue:
                    return compareValue(subject.eventValue, param1, param2);
                case RequirementKind::TargetLevel:
                    return target != nullptr && target->getLevel() >= param1;
                case RequirementKind::TargetGender:
                    return target != nullptr && target->getGender() == param1;
                case RequirementKind::CriteriaScript:
                case RequirementKind::InstanceScript:
                    // the script itself is asked with every update
                    return sScriptMgr.getAchievementCriteriaScript(criteria.record.id) != nullptr;
                case RequirementKind::MapDifficulty:
                {
                    WorldMap* map = player->getWorldMap();
                    if (map == nullptr)
                        return false;

                    uint8_t const mode = map->getSpawnMode();
                    if (map->getBaseMap()->isRaid())
                    {
                        bool const wants25 = param1 == InstanceDifficulty::RAID_25MAN_NORMAL || param1 == InstanceDifficulty::RAID_25MAN_HEROIC;
                        bool const is25 = mode == InstanceDifficulty::RAID_25MAN_NORMAL || mode == InstanceDifficulty::RAID_25MAN_HEROIC;
                        if (wants25 != is25)
                            return false;
                    }
                    return mode >= param1;
                }
                case RequirementKind::MapPlayersAtMost:
                {
                    WorldMap* map = player->getWorldMap();
                    if (map == nullptr)
                        return false;

                    uint32_t players = 0;
                    for (auto const& [guid, member] : map->getPlayers())
                        if (member != nullptr && !member->isGMFlagSet())
                            ++players;
                    return players <= param1;
                }
                case RequirementKind::TargetTeam:
                    return targetPlayer != nullptr && static_cast<uint32_t>(targetPlayer->getTeam()) == param1;
                case RequirementKind::PlayerDrunkenState:
                    return Player::getDrunkStateByValue(player->getServersideDrunkValue()) >= param1;
                case RequirementKind::RunningHoliday:
                    for (uint16_t const eventId : sGameEventMgr.GetActiveEventList())
                        if (GameEvent const* gameEvent = sGameEventMgr.GetEventById(eventId))
                            if (static_cast<uint32_t>(gameEvent->holiday_id) == param1)
                                return true;
                    return false;
                case RequirementKind::EventItemLevelQuality:
                {
                    ItemProperties const* item = subject.itemId != 0 ? sMySQLStore.getItemProperties(subject.itemId) : nullptr;
                    return item != nullptr && item->ItemLevel >= param1 && item->Quality >= param2;
                }
                case RequirementKind::PlayerMap:
                    return player->GetMapId() == param1;
                case RequirementKind::PlayerClassRace:
                    return (param1 == 0 || player->getClass() == param1) && (param2 == 0 || player->getRace() == param2);
                case RequirementKind::PlayerKnownTitle:
                    return hasTitle(player, param1);
                case RequirementKind::RunningGameEvent:
                    return sGameEventMgr.GetActiveEventList().contains(static_cast<uint16_t>(param1));
                case RequirementKind::EventItemQuality:
                {
                    ItemProperties const* item = subject.itemId != 0 ? sMySQLStore.getItemProperties(subject.itemId) : nullptr;
                    return item != nullptr && item->Quality == param1;
                }
                case RequirementKind::Disabled:
                    return false;
                default:
                    break;
            }

            // a type the core does not know never holds
            return false;
        }

        // the area or one of the areas it lies in
        bool isInArea(uint32_t areaId, uint32_t wanted)
        {
            for (uint8_t depth = 0; areaId != 0 && depth < 8; ++depth)
            {
                if (areaId == wanted)
                    return true;

                auto const* area = MapManagement::AreaManagement::AreaStorage::getAreaById(areaId);
                areaId = area != nullptr ? area->zone : 0;
            }
            return false;
        }

        int32_t standingWith(Player* player, uint32_t factionId)
        {
            return player->getFactionStanding(factionId).value_or(0);
        }

        // the difficulty ids of the 5.x client data for the instance modes of the core
        uint32_t difficultyIdOf(Player* player)
        {
            WorldMap* map = player->getWorldMap();
            if (map == nullptr)
                return 0;

            uint8_t const mode = map->getSpawnMode();
            if (map->getBaseMap()->isRaid())
                return 3u + mode;           // 10 normal, 25 normal, 10 heroic, 25 heroic

            if (map->getBaseMap()->isDungeon())
                return 1u + mode;           // 5 normal, 5 heroic

            return 0;
        }

        constexpr uint16_t AchievementCategoryPetBattles = 15117;

        uint16_t rootCategory(int32_t categoryId)
        {
            for (uint8_t depth = 0; depth < 8; ++depth)
            {
                auto const* category = sAchievementCategoryStore.lookupEntry(static_cast<uint32_t>(categoryId));
                if (category == nullptr || category->parent == -1)
                    break;
                categoryId = category->parent;
            }
            return static_cast<uint16_t>(categoryId);
        }

        uint32_t battlePetAchievementPoints(Player* player)
        {
            uint32_t points = 0;
            for (uint32_t const achievementId : player->getAchievementMgr()->getCompletedAchievementIds())
                if (auto const* achievement = sAchievementStore.lookupEntry(achievementId))
                    if (rootCategory(static_cast<int32_t>(achievement->categoryId)) == AchievementCategoryPetBattles)
                        points += achievement->points;

            return points;
        }

        bool isConditionMet(ConditionRecord const& condition, Player* player, EventSubject const& subject)
        {
            int32_t const asset = condition.asset;
            uint32_t const value = static_cast<uint32_t>(asset);
            Unit* target = subject.target;
            ItemProperties const* item = subject.itemId != 0 ? sMySQLStore.getItemProperties(subject.itemId) : nullptr;

            switch (condition.type)
            {
                case Inebriation:
                    return std::min<uint32_t>(player->getDrunkValue(), 100u) >= value;
                case MinimumItemLevel:
                    return item != nullptr && item->ItemLevel >= value;
                case TargetEntry:
                    return target != nullptr && target->getEntry() == value;
                case TargetIsPlayer:
                    return target != nullptr && target->isPlayer();
                case TargetIsDead:
                    return target != nullptr && !target->isAlive();
                case TargetIsHostile:
                    return target != nullptr && player->isHostileTo(target);
                case PlayerAura:
                    return player->hasAurasWithId(value);
                case TargetAura:
                    return target != nullptr && target->hasAurasWithId(value);
                case TargetAuraEffect:
                    return target != nullptr && target->hasAuraWithAuraEffect(static_cast<AuraEffect>(value));
                case TargetAuraState:
                    return target != nullptr && target->hasAuraState(static_cast<AuraState>(value));
                case ItemQualityAtLeast:
                    return item != nullptr && item->Quality >= value;
                case ItemQualityExactly:
                    return item != nullptr && item->Quality == value;
                case PlayerIsAlive:
                    return player->isAlive();
                case PlayerArea:
                    return isInArea(player->getAreaId(), value);
                case TargetArea:
                    return target != nullptr && isInArea(target->getAreaId(), value);
                case ItemEntry:
                    return subject.itemId == value;
                case LegacyDifficulty:
                {
                    WorldMap* map = player->getWorldMap();
                    return map != nullptr && (map->getBaseMap()->isDungeon() || map->getBaseMap()->isRaid()) && map->getSpawnMode() == value;
                }
                case LevelAboveTarget:
                    return target != nullptr && player->getLevel() >= target->getLevel() + value;
                case ArenaTeamSize:
                {
                    Battleground* battleground = player->getBattleground();
                    if (battleground == nullptr || !battleground->isArena())
                        return false;

                    // the team types of the core are 2v2, 3v3 and 5v5
                    static constexpr uint32_t teamSizes[] = { 2, 3, 5 };
                    uint8_t const teamType = static_cast<Arena*>(battleground)->GetArenaTeamType();
                    return teamType < 3 && teamSizes[teamType] == value;
                }
                case PlayerRace:
                    return player->getRace() == value;
                case PlayerClass:
                    return player->getClass() == value;
                case TargetRace:
                    return target != nullptr && target->getRace() == value;
                case TargetClass:
                    return target != nullptr && target->getClass() == value;
                case FewerTappers:
                    return player->getGroup() == nullptr || player->getGroup()->MemberCount() < value;
                case TargetCreatureType:
                {
                    if (target == nullptr || !target->isCreature())
                        return false;

                    CreatureProperties const* properties = static_cast<Creature*>(target)->GetCreatureProperties();
                    return properties != nullptr && properties->Type == value;
                }
                case PlayerMap:
                    return player->GetMapId() == value;
                case PlayerWithoutGroup:
                    return player->getGroup() == nullptr;
                case PersonalRating:
                    return player->getMaxPersonalRating() >= value;
                case PlayerTitle:
                    return hasTitle(player, value);
                case PlayerLevelExactly:
                    return player->getLevel() == value;
                case TargetLevelExactly:
                    return target != nullptr && target->getLevel() == value;
                case PlayerZone:
                    return player->getZoneId() == value;
                case PlayerHealthBelow:
                    return player->getHealthPct() <= value;
                case TargetHealthBelow:
                    return target != nullptr && target->getHealthPct() <= value;
                case AchievementPointsAbove:
                    return player->getAchievementMgr()->getAchievementPoints() > value;
                case RandomLfgDungeon:
                {
                    if (sLfgMgr.GetState(player->getGuid()) != LFG_STATE_DUNGEON)
                        return false;

                    for (uint32_t const dungeonId : sLfgMgr.GetSelectedDungeons(player->getGuid()))
                        if (auto const* dungeon = sLFGDungeonStore.lookupEntry(dungeonId & 0x00FFFFFF))
                            if (dungeon->type == LFG_TYPE_RANDOM)
                                return true;
                    return false;
                }
                case RatedArena:
                {
                    Battleground* battleground = player->getBattleground();
                    return battleground != nullptr && battleground->isArena() && battleground->Rated() != 0;
                }
                case GuildReputation:
                    return standingWith(player, FactionGuild) >= asset;
                case RatedBattleground:
                {
                    Battleground* battleground = player->getBattleground();
                    return battleground != nullptr && !battleground->isArena() && battleground->Rated() != 0;
                }
                case Difficulty:
                    return difficultyIdOf(player) == value;
                case PlayerLevelAtLeast:
                    return player->getLevel() >= value;
                case TargetLevelAtLeast:
                    return target != nullptr && target->getLevel() >= value;
                case LinkedModifierTree:
                    return isConditionTreeMet(sAchievementTrees.getConditionTree(value), player, subject);
                case TillersReputation:
                    return standingWith(player, FactionTillers) >= asset;
                case GuildMembersInGroup:
                {
                    Group* group = player->getGroup();
                    if (group == nullptr || player->getGuildId() == 0)
                        return false;

                    uint32_t members = 0;
                    group->Lock();
                    for (uint32_t i = 0; i < group->GetSubGroupCount(); ++i)
                        if (SubGroup* subGroup = group->GetSubGroup(i))
                            for (CachedCharacterInfo const* member : subGroup->getGroupMembers())
                                if (member != nullptr && member->m_guild == player->getGuildId())
                                    ++members;
                    group->Unlock();
                    return members >= value;
                }
                case ChallengeModeMedal:
                    // the medal of the old challenge modes is no longer part of the data
                    return false;
                case PlayerOnQuest:
                    return player->hasQuestInQuestLog(value);
                case AccountAchievement:
                    return player->getAchievementMgr()->hasCompletedOnAccount(value);
                case CharacterAchievement:
                    return player->getAchievementMgr()->hasCompletedOnCharacter(value);
                case CloudSerpentReputation:
                    return standingWith(player, FactionCloudSerpent) >= asset;
                case ServerExpansionAtLeast:
                    return static_cast<uint32_t>(WoW::getServerExpansion()) >= value;
                case FactionReputation:
                    return standingWith(player, value) >= condition.secondaryAsset;
                case ItemClassSubclass:
                    return item != nullptr && item->Class == value && item->SubClass == static_cast<uint32_t>(condition.secondaryAsset);
                case PlayerGender:
                    return player->getGender() == value;
                case PlayerSkillAtLeast:
                    return player->getSkillLineCurrent(static_cast<uint16_t>(value), false) >= static_cast<uint32_t>(condition.secondaryAsset);
                case TrackedCurrencyAtLeast:
                    return player->getTrackedCurrency(value) >= static_cast<uint32_t>(condition.secondaryAsset);
                case PlayerCondition:
                    return Conditions::isPlayerConditionMet(player, value);
                case TargetPlayerCondition:
                    return target != nullptr && target->isPlayer() && Conditions::isPlayerConditionMet(static_cast<Player*>(target), value);
                case WorldStateExpression:
                    return Conditions::isWorldStateExpressionMet(player, value);
                case BattlePetTeamLevel:
                    return player->getSession() != nullptr && player->getSession()->getBattlePets().getLowestSlotLevel() >= value;
                case BattlePetAchievementPoints:
                    return battlePetAchievementPoints(player) >= value;
                case BattlePetSpeciesType:
                {
                    auto const* species = sBattlePetMgr.getSpecies(static_cast<uint32_t>(subject.miscValue1));
                    return species != nullptr && species->petType == value;
                }
                case BattlePetSpecies:
                    return subject.miscValue1 == value;
                case BattlePetJournalLock:
                    // the journal lock is always with the one session of an account
                    return player->getSession() != nullptr;
                case FriendshipReaction:
                {
                    auto const* reaction = sFriendshipRepReactionStore.lookupEntry(value);
                    if (reaction == nullptr)
                        return false;

                    auto const* friendship = sFriendshipReputationStore.lookupEntry(reaction->friendshipRepId);
                    return friendship != nullptr && standingWith(player, friendship->factionId) >= static_cast<int32_t>(reaction->reactionThreshold);
                }
                case MapInstanceType:
                {
                    WorldMap* map = player->getWorldMap();
                    return map != nullptr && map->getBaseMap()->getMapEntry() != nullptr && map->getBaseMap()->getMapEntry()->mapType == value;
                }
                case PvpSeason:
                    return static_cast<uint32_t>(worldConfig.arena.arenaSeason) == value;
                default:
                    break;
            }

            // a type the core does not know never holds
            return false;
        }
    }

    bool isConditionTypeHandled(uint32_t type)
    {
        return handledTypes.contains(type);
    }

    bool isRequirementKindHandled(uint8_t kind)
    {
        return handledRequirements.contains(static_cast<RequirementKind>(kind));
    }

    bool areRequirementsMet(Criteria const& criteria, Player* player, EventSubject const& subject)
    {
        for (CriteriaRequirement const& requirement : criteria.requirements)
            if (!isRequirementMet(criteria, requirement, player, subject))
                return false;
        return true;
    }

    bool isConditionTreeMet(ConditionNode const* tree, Player* player, EventSubject const& subject)
    {
        if (tree == nullptr)
            return false;

        switch (static_cast<ConditionRule>(tree->record.rule))
        {
            case ConditionRule::Single:
                return tree->record.type != 0 && isConditionMet(tree->record, player, subject);
            case ConditionRule::SingleNot:
                return tree->record.type != 0 && !isConditionMet(tree->record, player, subject);
            case ConditionRule::AllChildren:
                for (ConditionNode const* child : tree->children)
                    if (!isConditionTreeMet(child, player, subject))
                        return false;
                return true;
            case ConditionRule::SomeChildren:
            {
                int32_t needed = std::max(tree->record.amount, 1);
                for (ConditionNode const* child : tree->children)
                    if (isConditionTreeMet(child, player, subject) && --needed == 0)
                        return true;
                return false;
            }
            default:
                break;
        }

        return false;
    }
}
