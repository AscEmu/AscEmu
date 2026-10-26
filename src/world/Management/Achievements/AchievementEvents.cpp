/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AchievementEvents.hpp"

#include "AchievementTrees.hpp"
#include "Management/AchievementMgr.h"
#include "Management/ItemInterface.h"
#include "Management/QuestProperties.hpp"
#include "Objects/Item.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Server/WorldSession.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Management/BattlePets/BattlePetCollection.hpp"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace Achievements
{
    namespace
    {
        // criteria types of the client data the core knows, the values of 5.4 and later
        enum CriteriaType : uint16_t
        {
            KillCreature                = 0,
            ReachLevel                  = 5,
            SkillRaised                 = 7,
            EarnAchievement             = 8,
            CompletedQuests             = 9,
            CompletedQuestsInZone       = 11,
            DieOnMap                    = 16,
            Die                         = 17,
            KilledByCreature            = 20,
            KilledByPlayer              = 23,
            FallWithoutDying            = 24,
            CompleteQuest               = 27,
            BeSpellTarget               = 28,
            KillInPvpArea               = 31,
            KnowSpell                   = 34,
            HonorableKill               = 35,
            AcquireItem                 = 36,
            SkillStep                   = 40,
            UseItem                     = 41,
            LootItem                    = 42,
            RevealOverlay               = 43,
            BankSlots                   = 45,
            ReputationGained            = 46,
            ExaltedFactions             = 47,
            Haircut                     = 48,
            EquipItemInSlot             = 49,
            DoEmote                     = 54,
            KillingBlow                 = 56,
            EquipItem                   = 57,
            QuestMoney                  = 62,
            TaxiMoney                   = 63,
            BarberMoney                 = 65,
            LootedMoney                 = 67,
            UseGameObject               = 68,
            GainAura                    = 69,
            Login                       = 74,
            SpellsOfSkillLine           = 75,
            KillAnyCreature             = 78,
            MostMoneyOwned              = 86,
            ReveredFactions             = 87,
            HonoredFactions             = 88,
            KnownFactions               = 89,
            LootAnyItem                 = 90,
            ObtainAnyItem               = 91,
            BuyTaxi                     = 108,
            LandSpellOnTarget           = 110,
            WinBattleground             = 1,
            PlayBattleground            = 15,
            WinArena                    = 32,
            PlayArena                   = 33,
            WinRankedArena              = 37,
            MoneyFromVendors            = 59,
            PostageMoney                = 66,
            WinDuel                     = 76,
            LoseDuel                    = 77,
            AbandonQuest                = 107,
            AcceptSummon                = 114,
            CompleteLfgDungeon          = 118,
            GroupWithStrangers          = 119,

            CastSpell                   = 29,
            LearnSkillLine              = 112,
            LifetimeHonorableKills      = 113,
            AchievementPoints           = 115,
            LearnedNewPet               = 155,
            UniquePetsOwned             = 156,
            BattlePetReachedLevel       = 160
        };

        // a game event of the core updates its own type and the types derived from the same change
        std::unordered_map<uint16_t, std::vector<uint16_t>> const eventTypes =
        {
            { ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE,              { KillCreature } },
            { ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL,                { ReachLevel } },
            { ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL,          { SkillRaised, LearnSkillLine } },
            { ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT,       { EarnAchievement, AchievementPoints } },
            { ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT,       { CompletedQuests } },
            { ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE,    { CompletedQuestsInZone } },
            { ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP,               { DieOnMap } },
            { ACHIEVEMENT_CRITERIA_TYPE_DEATH,                      { Die } },
            { ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE,         { KilledByCreature } },
            { ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER,           { KilledByPlayer } },
            { ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING,         { FallWithoutDying } },
            { ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST,             { CompleteQuest } },
            { ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET,            { BeSpellTarget } },
            { ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA,     { KillInPvpArea } },
            { ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL,                { KnowSpell, SpellsOfSkillLine } },
            { ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM,                   { AcquireItem, ObtainAnyItem } },
            { ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL,          { SkillStep, LearnSkillLine } },
            { ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM,                   { UseItem } },
            { ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM,                  { LootItem } },
            { ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA,               { RevealOverlay } },
            { ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT,              { BankSlots } },
            { ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION,            { ReputationGained, ExaltedFactions, ReveredFactions, HonoredFactions, KnownFactions } },
            { ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION,    { ExaltedFactions, ReveredFactions, HonoredFactions } },
            { ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP,          { Haircut } },
            { ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM,            { EquipItemInSlot } },
            { ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE,                   { DoEmote } },
            { ACHIEVEMENT_CRITERIA_TYPE_KILLING_BLOW,               { KillingBlow } },
            { ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM,                 { EquipItem } },
            { ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD,          { QuestMoney, MostMoneyOwned } },
            { ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING,  { TaxiMoney } },
            { ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER,       { BarberMoney } },
            { ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY,                 { LootedMoney, MostMoneyOwned } },
            { ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT,             { UseGameObject } },
            { ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2,           { GainAura } },
            { ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS,           { SpellsOfSkillLine } },
            { ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE,         { KillAnyCreature } },
            { ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM,             { LootAnyItem } },
            { ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN,         { BuyTaxi } },
            { ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2,                { LandSpellOnTarget } },
            { ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL,                 { CastSpell } },
            { ACHIEVEMENT_CRITERIA_TYPE_WIN_BG,           { WinBattleground } },
            { ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND,          { PlayBattleground } },
            { ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA,                  { WinArena } },
            { ACHIEVEMENT_CRITERIA_TYPE_PLAY_ARENA,                 { PlayArena } },
            { ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA,            { WinRankedArena } },
            { ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_VENDORS,         { MoneyFromVendors, MostMoneyOwned } },
            { ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL,        { PostageMoney } },
            { ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL,                   { WinDuel } },
            { ACHIEVEMENT_CRITERIA_TYPE_LOSE_DUEL,                  { LoseDuel } },
            { ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED,            { AbandonQuest } },
            { ACHIEVEMENT_CRITERIA_TYPE_ACCEPTED_SUMMONINGS,        { AcceptSummon } },
            { ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_LFG_DUNGEON,       { CompleteLfgDungeon } },
            { ACHIEVEMENT_CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS, { GroupWithStrangers } },

            { ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL,        { HonorableKill, LifetimeHonorableKills } },
            { ACHIEVEMENT_CRITERIA_TYPE_LOGIN,                      { Login } },
            { ACHIEVEMENT_CRITERIA_TYPE_LEARNED_NEW_PET,            { LearnedNewPet, UniquePetsOwned, BattlePetReachedLevel } },
            { ACHIEVEMENT_CRITERIA_TYPE_BATTLE_PET_LEVEL,           { BattlePetReachedLevel } }
        };

        std::unordered_set<uint16_t> const derivedTypes =
        {
            ReachLevel, SkillRaised, EarnAchievement, CompletedQuests, CompletedQuestsInZone, CompleteQuest, KnowSpell, SkillStep,
            RevealOverlay, BankSlots, ReputationGained, ExaltedFactions, Login, SpellsOfSkillLine, MostMoneyOwned, ReveredFactions,
            HonoredFactions, KnownFactions, LearnSkillLine, LifetimeHonorableKills, AchievementPoints,
            UniquePetsOwned, BattlePetReachedLevel
        };

        bool assetMatches(Criteria const& criteria, int64_t value)
        {
            return criteria.record.asset == 0 || static_cast<int64_t>(criteria.record.asset) == value;
        }

        uint32_t countSpellsOfSkillLine(Player* player, uint32_t skillLine)
        {
            std::unordered_set<uint32_t> spells;
            for (uint32_t i = 0; i < sSkillLineAbilityStore.getNumRows(); ++i)
                if (auto const* ability = sSkillLineAbilityStore.lookupEntry(i))
                    if (ability->skilline == skillLine && player->hasSpell(ability->spell))
                        spells.insert(ability->spell);

            return static_cast<uint32_t>(spells.size());
        }

        uint32_t countFinishedQuestsInZone(Player* player, int32_t zoneId)
        {
            uint32_t count = 0;
            for (uint32_t const questId : player->getFinishedQuests())
                if (QuestProperties const* quest = sMySQLStore.getQuestProperties(questId))
                    if (static_cast<int32_t>(quest->zone_id) == zoneId)
                        ++count;

            return count;
        }

        BattlePetCollection* battlePetsOf(Player* player)
        {
            return player->getSession() != nullptr ? &player->getSession()->getBattlePets() : nullptr;
        }
    }

    bool isStartEventHooked(uint8_t event)
    {
        switch (event)
        {
            case StartReachLevel: case StartCompleteDailyQuest: case StartBattleground: case StartGainAura: case StartGainAuraEffect:
            case StartCastSpell: case StartBeSpellTarget: case StartAcceptQuest: case StartKillCreature: case StartKillPlayer:
            case StartUseItem: case StartSendEvent:
                return true;
            default:
                return false;
        }
    }

    bool isFailEventHooked(uint8_t event)
    {
        switch (event)
        {
            case FailDeath: case FailLeaveBattleground: case FailLoseAura: case FailGainAura: case FailGainAuraEffect:
            case FailCastSpell: case FailBeSpellTarget: case FailPartyStatus: case FailDailyQuestsCleared: case FailSendEvent:
                return true;
            default:
                return false;
        }
    }

    bool isCriteriaTypeTracked(uint16_t type)
    {
        for (auto const& [eventType, types] : eventTypes)
            for (uint16_t const tracked : types)
                if (tracked == type)
                    return true;

        return false;
    }

    std::vector<uint16_t> const& getCriteriaTypesOfEvent(uint16_t eventType)
    {
        static std::vector<uint16_t> const none;
        auto const itr = eventTypes.find(eventType);
        return itr != eventTypes.end() ? itr->second : none;
    }

    bool isDerivedType(uint16_t type)
    {
        return derivedTypes.contains(type);
    }

    bool deriveProgress(Player* player, Criteria const& criteria, uint64_t& value)
    {
        int32_t const asset = criteria.record.asset;
        switch (criteria.record.type)
        {
            case ReachLevel:
                value = player->getLevel();
                return true;
            case SkillRaised:
                value = player->getSkillLineCurrent(static_cast<uint16_t>(asset), true);
                return true;
            case SkillStep:
                value = player->getSkillLineMax(static_cast<uint16_t>(asset));
                return true;
            case LearnSkillLine:
                value = player->getSkillLineMax(static_cast<uint16_t>(asset)) != 0 ? 1 : 0;
                return true;
            case EarnAchievement:
                value = player->getAchievementMgr()->hasCompleted(static_cast<uint32_t>(asset)) ? 1 : 0;
                return true;
            case AchievementPoints:
                value = player->getAchievementMgr()->getAchievementPoints();
                return true;
            case CompletedQuests:
                value = player->getFinishedQuests().size();
                return true;
            case CompletedQuestsInZone:
                value = countFinishedQuestsInZone(player, asset);
                return true;
            case CompleteQuest:
                value = player->hasQuestFinished(static_cast<uint32_t>(asset)) ? 1 : 0;
                return true;
            case KnowSpell:
                value = player->hasSpell(static_cast<uint32_t>(asset)) ? 1 : 0;
                return true;
            case SpellsOfSkillLine:
                value = countSpellsOfSkillLine(player, static_cast<uint32_t>(asset));
                return true;
            case RevealOverlay:
                value = player->hasOverlayUncovered(static_cast<uint32_t>(asset)) ? 1 : 0;
                return true;
            case BankSlots:
                value = player->getBankSlots();
                return true;
            case ReputationGained:
                value = static_cast<uint64_t>(std::max(0, player->getFactionStanding(static_cast<uint32_t>(asset)).value_or(0)));
                return true;
            case ExaltedFactions:
                value = player->getExaltedCount();
                return true;
            case ReveredFactions:
                value = player->getFactionCountAtLeast(StandingValues::REVERED);
                return true;
            case HonoredFactions:
                value = player->getFactionCountAtLeast(StandingValues::HONORED);
                return true;
            case KnownFactions:
                value = player->getVisibleFactionCount();
                return true;
            case MostMoneyOwned:
                value = player->getCoinage();
                return true;
            case LifetimeHonorableKills:
                value = player->getLifetimeHonorableKills();
                return true;
            case Login:
                value = 1;
                return true;
            case UniquePetsOwned:
                if (BattlePetCollection* pets = battlePetsOf(player))
                {
                    value = pets->getUniqueSpeciesCount();
                    return true;
                }
                return false;
            case BattlePetReachedLevel:
                if (BattlePetCollection* pets = battlePetsOf(player))
                {
                    value = pets->getPetCountAtLevel(static_cast<uint16_t>(asset));
                    return true;
                }
                return false;
            default:
                break;
        }

        return false;
    }

    bool resolveEvent(Player* player, Criteria const& criteria, int32_t miscValue1, int32_t miscValue2, Object* reference, ProgressUpdate& update)
    {
        update = {};
        update.subject.miscValue1 = static_cast<uint64_t>(miscValue1);
        update.subject.miscValue2 = static_cast<uint64_t>(miscValue2);
        update.subject.eventValue = static_cast<uint64_t>(miscValue1);
        if (reference != nullptr && reference->isCreatureOrPlayer())
            update.subject.target = reference->ToUnit();

        // derived types follow the player, the event only says when
        if (isDerivedType(criteria.record.type))
        {
            if (!deriveProgress(player, criteria, update.value))
                return false;

            update.change = criteria.record.type == MostMoneyOwned ? ProgressChange::Highest : ProgressChange::Set;
            return true;
        }

        update.change = ProgressChange::Add;
        update.value = 1;

        switch (criteria.record.type)
        {
            case KillCreature:
                update.value = static_cast<uint64_t>(std::max(miscValue2, 1));
                return assetMatches(criteria, miscValue1);

            case DieOnMap:
            case KillInPvpArea:
            case UseItem:
            case UseGameObject:
            case BeSpellTarget:
            case GainAura:
            case LandSpellOnTarget:
            case CastSpell:
            case DoEmote:
                if (criteria.record.type == UseItem)
                    update.subject.itemId = static_cast<uint32_t>(miscValue1);
                if (criteria.record.type == DoEmote && update.subject.target == nullptr)
                    update.subject.target = player->getWorldMapUnit(player->getTargetGuid());
                return assetMatches(criteria, miscValue1);

            case WinBattleground:
            case PlayBattleground:
            case WinArena:
            case PlayArena:
                return assetMatches(criteria, miscValue1);

            case WinRankedArena:
            case WinDuel:
            case LoseDuel:
            case AbandonQuest:
            case AcceptSummon:
            case CompleteLfgDungeon:
                return true;

            case MoneyFromVendors:
            case PostageMoney:
            case GroupWithStrangers:
                update.value = static_cast<uint64_t>(std::max(miscValue1, 0));
                return update.value != 0;

            case Die:
            case KilledByPlayer:
            case Haircut:
            case HonorableKill:
            case BuyTaxi:
            case LearnedNewPet:
                return true;

            case KilledByCreature:
                // the killer comes as reference
                return update.subject.target != nullptr && update.subject.target->isCreature() && assetMatches(criteria, update.subject.target->getEntry());

            case KillingBlow:
                return true;

            case FallWithoutDying:
                update.change = ProgressChange::Highest;
                update.value = static_cast<uint64_t>(std::max(miscValue1, 0));
                return true;

            case AcquireItem:
            case LootItem:
                update.subject.itemId = static_cast<uint32_t>(miscValue1);
                update.value = static_cast<uint64_t>(std::max(miscValue2, 1));
                return assetMatches(criteria, miscValue1);

            case ObtainAnyItem:
            case LootAnyItem:
                update.subject.itemId = static_cast<uint32_t>(miscValue1);
                update.value = static_cast<uint64_t>(std::max(miscValue2, 1));
                return true;

            case EquipItem:
                update.subject.itemId = static_cast<uint32_t>(miscValue1);
                update.change = ProgressChange::Set;
                return assetMatches(criteria, miscValue1);

            case EquipItemInSlot:
            {
                // the event names the item, without it the one that sits in the slot
                update.subject.itemId = static_cast<uint32_t>(miscValue2);
                if (update.subject.itemId == 0)
                {
                    Item const* item = player->getItemInterface()->GetInventoryItem(static_cast<int16_t>(miscValue1));
                    if (item == nullptr)
                        return false;
                    update.subject.itemId = item->getEntry();
                }

                update.change = ProgressChange::Set;
                return static_cast<int32_t>(miscValue1) == criteria.record.asset;
            }

            case QuestMoney:
            case TaxiMoney:
            case BarberMoney:
            case LootedMoney:
                update.value = static_cast<uint64_t>(std::max(miscValue1, 0));
                return update.value != 0;

            case KillAnyCreature:
            {
                // the guid of the creature comes in both values
                uint64_t const guid = (static_cast<uint64_t>(static_cast<uint32_t>(miscValue1)) << 32) | static_cast<uint32_t>(miscValue2);
                update.subject.target = player->getWorldMapUnit(guid);
                if (update.subject.target == nullptr)
                    return false;

                // a value requirement compares the creature type
                if (update.subject.target->isCreature())
                    if (CreatureProperties const* properties = static_cast<Creature*>(update.subject.target)->GetCreatureProperties())
                        update.subject.eventValue = properties->Type;
                return true;
            }

            default:
                break;
        }

        return false;
    }
}
