/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
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

#include "QuestMgr.h"
#include "Server/ClientProtocol.hpp"

#include "Group.h"
#include "MailMgr.h"
#include "ObjectMgr.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Logging/Log.hpp"
#include "Objects/Item.hpp"
#include "QuestLogEntry.hpp"
#include "Gossip/GossipMenu.hpp"
#include "Logging/Logger.hpp"
#include "Management/ItemInterface.h"
#include "Management/QuestDefines.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Map/Management/MapMgr.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellMgr.hpp"
#include "Server/Packets/MsgQuestPushResult.h"
#include "Server/Packets/SmsgQuestgiverOfferReward.h"
#include "Server/Packets/SmsgQuestgiverQuestComplete.h"
#include "Server/Packets/SmsgQuestgiverQuestDetails.h"
#include "Server/Packets/SmsgQuestgiverQuestList.h"
#include "Server/Packets/SmsgQuestgiverRequestItems.h"
#include "Server/Packets/SmsgQuestLogFull.h"
#include "Server/Packets/SmsgQuestgiverQuestInvalid.h"
#include "Server/Packets/SmsgQuestupdateFailedTimer.h"
#include "Server/Packets/SmsgQuestupdateFailed.h"
#include "Server/Packets/SmsgQuestgiverQuestFailed.h"
#include "Server/Packets/SmsgSpellStart.h"
#include "Server/Packets/SmsgSpellGo.h"
#include "Server/Packets/SmsgQuestupdateAddItem.h"
#include "Server/Packets/SmsgQuestupdateAddKill.h"
#include "Storage/WorldStrings.h"

#include <unordered_set>
#include <algorithm>
#include "Utilities/Strings.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"
#include "Spell/Spell.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/TimeTracker.hpp"
#include <Server/WorldSessionLog.hpp>

using namespace AscEmu::Packets;

// APGL End
// MIT Start
static bool matchesQuestItemObjective(uint32_t itemEntry, int32_t objectiveObjectId)
{
    if (objectiveObjectId <= 0)
        return false;

    const uint32_t objectiveItemId = static_cast<uint32_t>(objectiveObjectId);
    if (itemEntry == objectiveItemId)
        return true;

    ItemProperties const* itemProperties = sMySQLStore.getItemProperties(itemEntry);
    return itemProperties != nullptr && itemProperties->QuestLogItemId != 0 && itemProperties->QuestLogItemId == objectiveItemId;
}

QuestMgr& QuestMgr::getInstance()
{
    static QuestMgr mInstance;
    return mInstance;
}

void QuestMgr::onPlayerItemRemove(Player* plr, Item const* item)
{
    if (plr == nullptr || item == nullptr)
        return;

    const int32_t itemEntry = static_cast<int32_t>(item->getEntry());
    for (uint8_t slot = 0; slot < MAX_QUEST_SLOT; ++slot)
    {
        QuestLogEntry* questLog = plr->getQuestLogBySlotId(slot);
        if (questLog == nullptr)
            continue;

        const auto objectives = buildQuestObjectives(questLog->getQuestProperties(), 0);
        const auto objective = std::find_if(objectives.begin(), objectives.end(), [itemEntry](QuestObjectiveData const& value)
        {
            return value.type == QUEST_OBJECTIVE_ITEM && matchesQuestItemObjective(static_cast<uint32_t>(itemEntry), value.objectId);
        });
        if (objective == objectives.end())
            continue;

        questLog->updatePlayerFields();
        if (getQuestObjectiveProgress(plr, questLog, *objective) < static_cast<uint32_t>(std::max(objective->amount, 0)))
            plr->updateNearbyQuestGameObjects();
    }
}

// MIT End
// APGL Start
uint32_t QuestMgr::CalcQuestStatus(Object* quest_giver, Player* plr, QuestRelation* qst)
{
    return CalcQuestStatus(quest_giver, plr, qst->qst, qst->type, false);
}

bool QuestMgr::isRepeatableQuestFinished(Player* plr, QuestProperties const* qst)
{
    for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        if (qst->required_item[i])
        {
            if (plr->getItemInterface()->GetItemCount(qst->required_item[i]) < qst->required_itemcount[i])
            {
                return false;
            }
        }
    }

    return true;
}

uint32_t QuestMgr::PlayerMeetsReqs(Player* plr, QuestProperties const* qst, bool skiplevelcheck)
{
    uint32_t status;

    if (!IsQuestRepeatable(qst) && !IsQuestDaily(qst))
        status = QuestStatus::Available;
    else
    {
        status = QuestStatus::Repeatable;
        if (qst->is_repeatable == DEFINE_QUEST_REPEATABLE_DAILY && plr->hasQuestInFinishedDailies(qst->id))
            return QuestStatus::NotAvailable;
    }

    if (qst->required_class)
        if (!(qst->required_class & plr->getClassMask()))
            return QuestStatus::NotAvailable;

    if (qst->required_races)
    {
        if (!(qst->required_races & plr->getRaceMask()))
            return QuestStatus::NotAvailable;
    }

    if (qst->required_tradeskill)
    {
        if (!plr->hasSkillLine(qst->required_tradeskill))
            return QuestStatus::NotAvailable;
        if (qst->required_tradeskill_value && plr->getSkillLineCurrent(qst->required_tradeskill) < qst->required_tradeskill_value)
            return QuestStatus::NotAvailable;
    }

    // Check reputation
    if (qst->required_rep_faction && qst->required_rep_value)
        if (plr->getFactionStanding(qst->required_rep_faction) < (int32_t)qst->required_rep_value)
            return QuestStatus::NotAvailable;

    if (plr->hasQuestFinished(qst->id) && !IsQuestRepeatable(qst) && !IsQuestDaily(qst))
        return QuestStatus::NotAvailable;

    // dont display quests we already have
    if (plr->hasQuestInQuestLog(qst->id))
        status = QuestStatus::NotAvailable;

    // Check One of Quest Prequest
    if (!qst->quest_list.empty())
    {
        bool questsCompleted = false;
        for (unsigned int questId : qst->quest_list)
        {
            if (sMySQLStore.getQuestProperties(questId) != nullptr && plr->hasQuestFinished(questId))
            {
                questsCompleted = true;
                break;
            }
        }
        if (!questsCompleted) // If none of listed quests is done, next part isn't available.
            return QuestStatus::NotAvailable;
    }

    for (unsigned int requiredQuest : qst->required_quests)
    {
        if (requiredQuest > 0 && !plr->hasQuestFinished(requiredQuest))
        {
            return QuestStatus::NotAvailable;
        }
    }

    // Check level requirement last so gray question mark isn't sent for quests which player isn't even eligible for
    if (plr->getLevel() < qst->min_level && !skiplevelcheck)
        return QuestStatus::AvailableButLevelTooLow;

    // check quest level
    if (static_cast<int32_t>(plr->getLevel()) >= (qst->questlevel + 5) && (status != QuestStatus::Repeatable))
        return QuestStatus::AvailableChat;

    return status;
}

uint32_t QuestMgr::CalcQuestStatus(Object* /*quest_giver*/, Player* plr, QuestProperties const* qst, uint8_t type, bool skiplevelcheck)
{
    if (auto* questLog = plr->getQuestLogByQuestId(qst->id))
    {
        if (type & QUESTGIVER_QUEST_END)
        {
            if (!questLog->canBeFinished())
            {
                if (qst->is_repeatable)
                    return QuestStatus::Repeatable;

                return QuestStatus::NotFinished;
            }

            return QuestStatus::Finished;
        }
    }

    if (type & QUESTGIVER_QUEST_START)
    {
        return PlayerMeetsReqs(plr, qst, skiplevelcheck);
    }

    return QuestStatus::NotAvailable;
}

uint32_t QuestMgr::CalcQuestStatus(Player* plr, uint32_t qst)
{
    if (auto* questLog = plr->getQuestLogByQuestId(qst))
    {
        if (!questLog->canBeFinished())
            return QuestStatus::NotFinished;

        return QuestStatus::Finished;
    }

    return QuestStatus::NotAvailable;
}

uint32_t QuestMgr::CalcStatus(Object* quest_giver, Player* plr)
{
    uint32_t status = QuestStatus::NotAvailable;
    QuestRelationList::const_iterator itr;
    QuestRelationList::const_iterator q_begin;
    QuestRelationList::const_iterator q_end;
    bool bValid = false;

    if (quest_giver->isGameObject())
    {
        bValid = false;

        GameObject* go = static_cast<GameObject*>(quest_giver);
        GameObject_QuestGiver* go_quest_giver = nullptr;
        if (go->getGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
        {
            go_quest_giver = static_cast<GameObject_QuestGiver*>(go);
            if (go_quest_giver->HasQuests())
                bValid = true;
        }
        if (bValid)
        {
            q_begin = go_quest_giver->QuestsBegin();
            q_end = go_quest_giver->QuestsEnd();
        }
    }
    else if (quest_giver->isCreature())
    {
        bValid = static_cast< Creature* >(quest_giver)->HasQuests();
        if (bValid)
        {
            q_begin = static_cast< Creature* >(quest_giver)->QuestsBegin();
            q_end = static_cast< Creature* >(quest_giver)->QuestsEnd();
        }
    }
    else if (quest_giver->isItem())
    {
        if (static_cast< Item* >(quest_giver)->getItemProperties()->QuestId)
            bValid = true;
    }
    //This will be handled at quest share so nothing important as status
    else if (quest_giver->isPlayer())
    {
        status = QuestStatus::Available;
    }

    if (!bValid)
    {
        //annoying message that is not needed since all objects don't exactly have quests
        //sLogger.debug("QUESTS: Warning, invalid NPC " I64FMT " specified for CalcStatus. TypeId: {}.", quest_giver->getGuid(), quest_giver->getObjectTypeId());
        return status;
    }

    if (quest_giver->isItem())
    {
        QuestProperties const* pQuest = sMySQLStore.getQuestProperties(static_cast< Item* >(quest_giver)->getItemProperties()->QuestId);
        if (pQuest)
        {
            QuestRelation qr;
            qr.qst = pQuest;
            qr.type = 1;

            uint32_t tmp_status = CalcQuestStatus(quest_giver, plr, &qr);
            if (tmp_status > status)
                status = tmp_status;
        }
    }

    for (itr = q_begin; itr != q_end; ++itr)
    {
        uint32_t tmp_status = CalcQuestStatus(quest_giver, plr, itr->get()); // save a call
        if (tmp_status > status)
            status = tmp_status;
    }

    return status;
}

uint32_t QuestMgr::ActiveQuestsCount(Object* quest_giver, Player* plr)
{
    QuestRelationList::const_iterator itr;
    std::map<uint32_t, uint8_t> tmp_map;
    uint32_t questCount = 0;

    QuestRelationList::const_iterator q_begin;
    QuestRelationList::const_iterator q_end;
    bool bValid = false;

    if (quest_giver->isGameObject())
    {
        bValid = false;

        GameObject* go = static_cast<GameObject*>(quest_giver);
        GameObject_QuestGiver* go_quest_giver = nullptr;
        if (go->getGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
        {
            go_quest_giver = static_cast<GameObject_QuestGiver*>(go);
            if (go_quest_giver->HasQuests())
                bValid = true;
        }
        if (bValid)
        {
            q_begin = go_quest_giver->QuestsBegin();
            q_end = go_quest_giver->QuestsEnd();

        }
    }
    else if (quest_giver->isCreature())
    {
        bValid = static_cast< Creature* >(quest_giver)->HasQuests();
        if (bValid)
        {
            q_begin = static_cast< Creature* >(quest_giver)->QuestsBegin();
            q_end = static_cast< Creature* >(quest_giver)->QuestsEnd();
        }
    }

    if (!bValid)
    {
        sLogger.debug("QUESTS: Warning, invalid NPC {} specified for ActiveQuestsCount. TypeId: {}.", std::to_string(quest_giver->getGuid()), quest_giver->getObjectTypeId());
        return 0;
    }

    for (itr = q_begin; itr != q_end; ++itr)
    {
        if (CalcQuestStatus(quest_giver, plr, itr->get()) >= QuestStatus::AvailableChat)
        {
            if (tmp_map.find((*itr)->qst->id) == tmp_map.end())
            {
                tmp_map.insert(std::map<uint32_t, uint8_t>::value_type((*itr)->qst->id, static_cast<uint8_t>(1)));
                questCount++;
            }
        }
    }

    return questCount;
}


void QuestMgr::BuildQuestComplete(Player* plr, QuestProperties const* qst)
{
    uint32_t xp;
    uint32_t rewardtalents = qst->rewardtalents;
    uint32_t playerlevel = plr->getLevel();

    if (playerlevel >= plr->getMaxLevel())
    {
        xp = 0;
    }
    else
    {
        xp = Util::float2int32(GenerateQuestXP(plr, qst) * worldConfig.getFloatRate(RATE_QUESTXP));
        plr->giveXp(xp, 0, false);
    }

    // Bonus talents
    if (rewardtalents > 0)
    {
        plr->setTalentPointsFromQuests(plr->getTalentPointsFromQuests() + rewardtalents);
        plr->setInitialTalentPoints();
    }

    // Reward title
    if (qst->rewardtitleid > 0)
        plr->setKnownPvPTitle(static_cast<RankTitles>(qst->rewardtitleid), true);

    // Some spells applied at quest reward
    SpellAreaForQuestMapBounds saBounds = sSpellMgr.getSpellAreaForQuestMapBounds(qst->id, false);
    if (saBounds.first != saBounds.second)
    {
        for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            const auto spellArea = itr->second;
            if (spellArea->autoCast && spellArea->fitsToRequirements(plr, plr->getZoneId(), plr->getAreaId()))
                if (!plr->hasAurasWithId(spellArea->spellId))
                    plr->castSpell(plr, spellArea->spellId, true);
        }
    }

    const bool hasNextQuest = qst->next_quest_id != 0;
    SmsgQuestgiverQuestComplete managedPacket(qst->id, xp, GenerateRewardMoney(plr, qst), qst->bonushonor * 10, rewardtalents, qst->bonusarenapoints, hasNextQuest, false, hasNextQuest, false);
    plr->getSession()->sendManagedPacket(managedPacket);
}

void QuestMgr::SendQuestUpdateAddKill(Player* plr, uint32_t questid, uint32_t entry, uint32_t count, uint32_t tcount, Object const* source)
{
    const uint64_t sourceGuid = source != nullptr ? source->getGuid() : 0;
    const uint16_t mapId = static_cast<uint16_t>(source != nullptr ? source->GetMapId() : plr->GetMapId());
    SmsgQuestupdateAddKill addPacket(questid, entry, count, tcount, sourceGuid, mapId);
    plr->getSession()->sendManagedPacket(addPacket);
}

void QuestMgr::SendPushToPartyResponse(Player* plr, Player* pTarget, uint8_t response)
{
    MsgQuestPushResult managedPacket(pTarget->getGuid(), 0, response);
    plr->getSession()->sendManagedPacket(managedPacket);
}

bool QuestMgr::OnGameObjectActivate(Player* plr, GameObject* go)
{
    if (plr == nullptr || go == nullptr)
        return false;

    QuestObjectiveCreditEvent event;
    event.type = QuestObjectiveCreditType::GameObjectActivate;
    event.objectId = static_cast<int32_t>(go->getEntry());
    event.source = go;
    return updateQuestObjectiveProgress(plr, event);
}

void QuestMgr::OnPlayerKill(Player* plr, Creature* victim, bool IsGroupKill)
{
    if (plr == nullptr || victim == nullptr)
        return;

    QuestObjectiveCreditEvent event;
    event.type = QuestObjectiveCreditType::MonsterKill;
    event.objectId = static_cast<int32_t>(victim->getEntry());
    event.source = victim;
    event.groupCredit = IsGroupKill;
    updateQuestObjectiveProgress(plr, event);

    // Extra credit (yay we wont have to script this anymore) - Shauren
    for (uint8_t i = 0; i < 2; ++i)
    {
        const uint32_t extraCredit = victim->GetCreatureProperties()->killcredit[i];
        if (extraCredit != 0 && sMySQLStore.getCreatureProperties(extraCredit) != nullptr)
        {
            event.objectId = static_cast<int32_t>(extraCredit);
            updateQuestObjectiveProgress(plr, event);
        }
    }
}

void QuestMgr::_OnPlayerKill(Player* plr, uint32_t entry, bool IsGroupKill, Object const* source)
{
    QuestObjectiveCreditEvent event;
    event.type = QuestObjectiveCreditType::MonsterKill;
    event.objectId = static_cast<int32_t>(entry);
    event.source = source;
    event.groupCredit = IsGroupKill;
    updateQuestObjectiveProgress(plr, event);
}

void QuestMgr::OnPlayerCast(Player* plr, uint32_t spellid, uint64_t& victimguid)
{
    if (plr == nullptr || !plr->hasQuestSpell(spellid))
        return;

    Unit* victim = plr->getWorldMap() != nullptr ? plr->getWorldMapUnit(victimguid) : nullptr;

    QuestObjectiveCreditEvent event;
    event.type = QuestObjectiveCreditType::SpellCast;
    event.objectId = victim != nullptr ? static_cast<int32_t>(victim->getEntry()) : 0;
    event.actionId = spellid;
    event.source = victim;
    updateQuestObjectiveProgress(plr, event);
}

void QuestMgr::OnPlayerItemPickup(Player* plr, Item* item)
{
    if (plr == nullptr || item == nullptr)
        return;

    QuestObjectiveCreditEvent event;
    event.type = QuestObjectiveCreditType::ItemPickup;
    event.objectId = static_cast<int32_t>(item->getEntry());
    event.amount = 1;
    updateQuestObjectiveProgress(plr, event);
}

void QuestMgr::OnPlayerExploreArea(Player* plr, uint32_t AreaID)
{
    QuestObjectiveCreditEvent event;
    event.type = QuestObjectiveCreditType::AreaTrigger;
    event.objectId = static_cast<int32_t>(AreaID);
    updateQuestObjectiveProgress(plr, event);
}

void QuestMgr::AreaExplored(Player* plr, uint32_t QuestID)
{
    QuestObjectiveCreditEvent event;
    event.type = QuestObjectiveCreditType::ScriptExplore;
    event.questId = QuestID;
    updateQuestObjectiveProgress(plr, event);
}

void QuestMgr::GiveQuestRewardReputation(Player* plr, QuestProperties const* qst, Object* qst_giver)
{
    // Reputation reward
    for (uint8_t z = 0; z < 6; ++z)
    {
        uint32_t fact = 19;   // default to 19 if no factiondbc
        int32_t amt = Util::float2int32(GenerateQuestXP(plr, qst) * 0.1f);      // guess
        if (!qst->reward_repfaction[z])
        {
            if (z >= 1)
                break;

            // Let's do this properly. Determine the faction of the creature, and give reputation to his faction.
            if (qst_giver->isCreature())
                if (qst_giver->getServersideFactionEntry() != NULL)
                    fact = qst_giver->getServersideFactionEntry()->id;
            if (qst_giver->isGameObject())
                fact = static_cast< GameObject* >(qst_giver)->getFactionTemplate();
        }
        else
        {
            fact = qst->reward_repfaction[z];
            if (qst->reward_repvalue[z])
                amt = qst->reward_repvalue[z];
        }

        if (qst->reward_replimit)
            if (plr->getFactionStanding(fact) >= (int32_t)qst->reward_replimit)
                continue;

        amt = Util::float2int32(amt * worldConfig.getFloatRate(RATE_QUESTREPUTATION));     // reputation rewards
        plr->modFactionStanding(fact, amt);
    }
}

void QuestMgr::OnQuestAccepted(Player* /*plr*/, QuestProperties const* /*qst*/, Object* /*qst_giver*/)
{}

void QuestMgr::OnQuestFinished(Player* plr, QuestProperties const* qst, Object* qst_giver, uint32_t reward_slot)
{
    //Re-Check for Gold Requirement (needed for possible xploit) - reward money < 0 means required money
    if (qst->reward_money < 0 && plr->getCoinage() < uint32_t(-qst->reward_money))
        return;

    // Check they don't have more than the max gold
    if (worldConfig.player.isGoldCapEnabled && (plr->getCoinage() + qst->reward_money) > worldConfig.player.limitGoldAmount)
    {
        plr->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
        return;
    }

    QuestLogEntry* questLog = plr->getQuestLogByQuestId(qst->id);
    if (!questLog)
        return;

    BuildQuestComplete(plr, qst);

    if (const auto questScript = questLog->getQuestScript())
        questScript->OnQuestComplete(plr, questLog);

    for (uint8_t x = 0; x < 4; x++)
    {
        if (qst->required_spell[x] != 0)
        {
            if (plr->hasQuestSpell(qst->required_spell[x]))
                plr->removeQuestSpell(qst->required_spell[x]);
        }
        else if (qst->required_mob_or_go[x] != 0)
        {
            if (plr->hasQuestMob(qst->required_mob_or_go[x]))
                plr->removeQuestMob(qst->required_mob_or_go[x]);
        }
    }

    questLog->clearAffectedUnits();
    questLog->finishAndRemove();

    if (qst_giver->isCreature())
    {
        if (!dynamic_cast<Creature*>(qst_giver)->HasQuest(qst->id, 2))
        {
            sGMLog.writefromsession(plr->getSession(), "Attempted to complete quest from invalid NPC."); // QuestID: {}, NPC Entry: {}.", qst->id, qst_giver->GetEntry());
            plr->getSession()->Disconnect();
            return;
        }
    }

    //details: hmm as i can remember, repeatable quests give faction rep still after first completion
    if (IsQuestRepeatable(qst) || IsQuestDaily(qst))
    {
        // Reputation reward
        GiveQuestRewardReputation(plr, qst, qst_giver);
        // Static Item reward
        for (uint8_t i = 0; i < 4; ++i)
        {
            if (qst->reward_item[i])
            {
                ItemProperties const* proto = sMySQLStore.getItemProperties(qst->reward_item[i]);
                if (!proto)
                {
                    sLogger.failure("Invalid item prototype in quest reward! ID {}, quest {}", qst->reward_item[i], qst->id);
                }
                else
                {
                    if (ownsUniqueRewardItem(plr, proto))
                        continue;

                    auto item_add = plr->getItemInterface()->FindItemLessMax(qst->reward_item[i], qst->reward_itemcount[i], false);
                    if (!item_add)
                    {
                        auto slotresult = plr->getItemInterface()->FindFreeInventorySlot(proto);
                        if (!slotresult.Result)
                        {
                            plr->getItemInterface()->buildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                        }
                        else
                        {
                            auto item = sObjectMgr.createItem(qst->reward_item[i], plr);
                            if (!item)
                                return;

                            item->setStackCount(uint32_t(qst->reward_itemcount[i]));
                            plr->getItemInterface()->SafeAddItem(std::move(item), slotresult.ContainerSlot, slotresult.Slot);
                        }
                    }
                    else
                    {
                        item_add->setStackCount(item_add->getStackCount() + qst->reward_itemcount[i]);
                        item_add->m_isDirty = true;
                    }
                }
            }
        }

        // Choice Rewards
        if (qst->reward_choiceitem[reward_slot])
        {
            ItemProperties const* proto = sMySQLStore.getItemProperties(qst->reward_choiceitem[reward_slot]);
            if (!proto)
            {
                sLogger.failure("Invalid item prototype in quest reward! ID {}, quest {}", qst->reward_choiceitem[reward_slot], qst->id);
            }
            else
            {
                auto item_add = plr->getItemInterface()->FindItemLessMax(qst->reward_choiceitem[reward_slot], qst->reward_choiceitemcount[reward_slot], false);
                if (!item_add)
                {
                    auto slotresult = plr->getItemInterface()->FindFreeInventorySlot(proto);
                    if (!slotresult.Result)
                    {
                        plr->getItemInterface()->buildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                    }
                    else
                    {
                        auto item = sObjectMgr.createItem(qst->reward_choiceitem[reward_slot], plr);
                        if (!item)
                            return;

                        item->setStackCount(uint32_t(qst->reward_choiceitemcount[reward_slot]));
                        plr->getItemInterface()->SafeAddItem(std::move(item), slotresult.ContainerSlot, slotresult.Slot);
                    }
                }
                else
                {
                    item_add->setStackCount(item_add->getStackCount() + qst->reward_choiceitemcount[reward_slot]);
                    item_add->m_isDirty = true;
                }
            }
        }

        // Remove items
        for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
        {
            if (qst->required_item[i]) plr->getItemInterface()->RemoveQuestItemAmt(qst->required_item[i], qst->required_itemcount[i]);
        }

        // Remove srcitem
        if (qst->srcitem && qst->srcitem != qst->receive_items[0])
            plr->getItemInterface()->RemoveItemAmt(qst->srcitem, qst->srcitemcount ? qst->srcitemcount : 1);

        // cast Effect Spell
        if (qst->effect_on_player)
        {
            SpellInfo const* spell_entry = sSpellMgr.getSpellInfo(qst->effect_on_player);
            if (spell_entry)
            {
                Spell* spe = sSpellMgr.newSpell(plr, spell_entry, true, NULL);
                SpellCastTargets tgt(plr->getGuid());
                spe->prepare(&tgt);
            }
        }

        plr->modCoinage(GenerateRewardMoney(plr, qst));

        // if daily then append to finished dailies
        if (qst->is_repeatable == DEFINE_QUEST_REPEATABLE_DAILY)
            plr->addQuestIdToFinishedDailies(qst->id);
    }
    else
    {
        plr->modCoinage(GenerateRewardMoney(plr, qst));

        // Reputation reward
        GiveQuestRewardReputation(plr, qst, qst_giver);
        // Static Item reward
        for (uint8_t i = 0; i < 4; ++i)
        {
            if (qst->reward_item[i])
            {
                ItemProperties const* proto = sMySQLStore.getItemProperties(qst->reward_item[i]);
                if (!proto)
                {
                    sLogger.failure("Invalid item prototype in quest reward! ID {}, quest {}", qst->reward_item[i], qst->id);
                }
                else
                {
                    if (ownsUniqueRewardItem(plr, proto))
                        continue;

                    auto item_add = plr->getItemInterface()->FindItemLessMax(qst->reward_item[i], qst->reward_itemcount[i], false);
                    if (!item_add)
                    {
                        auto slotresult = plr->getItemInterface()->FindFreeInventorySlot(proto);
                        if (!slotresult.Result)
                        {
                            plr->getItemInterface()->buildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                        }
                        else
                        {
                            auto item = sObjectMgr.createItem(qst->reward_item[i], plr);
                            if (!item)
                                return;

                            item->setStackCount(uint32_t(qst->reward_itemcount[i]));
                            plr->getItemInterface()->SafeAddItem(std::move(item), slotresult.ContainerSlot, slotresult.Slot);
                        }
                    }
                    else
                    {
                        item_add->setStackCount(item_add->getStackCount() + qst->reward_itemcount[i]);
                        item_add->m_isDirty = true;
                    }
                }
            }
        }

        // Choice Rewards
        if (qst->reward_choiceitem[reward_slot])
        {
            ItemProperties const* proto = sMySQLStore.getItemProperties(qst->reward_choiceitem[reward_slot]);
            if (!proto)
            {
                sLogger.failure("Invalid item prototype in quest reward! ID {}, quest {}", qst->reward_choiceitem[reward_slot], qst->id);
            }
            else
            {
                auto item_add = plr->getItemInterface()->FindItemLessMax(qst->reward_choiceitem[reward_slot], qst->reward_choiceitemcount[reward_slot], false);
                if (!item_add)
                {
                    auto slotresult = plr->getItemInterface()->FindFreeInventorySlot(proto);
                    if (!slotresult.Result)
                    {
                        plr->getItemInterface()->buildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                    }
                    else
                    {
                        auto item = sObjectMgr.createItem(qst->reward_choiceitem[reward_slot], plr);
                        if (!item)
                            return;

                        item->setStackCount(uint32_t(qst->reward_choiceitemcount[reward_slot]));
                        plr->getItemInterface()->SafeAddItem(std::move(item), slotresult.ContainerSlot, slotresult.Slot);
                    }
                }
                else
                {
                    item_add->setStackCount(item_add->getStackCount() + qst->reward_choiceitemcount[reward_slot]);
                    item_add->m_isDirty = true;
                }
            }
        }

        // Remove items
        for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
        {
            if (qst->required_item[i]) plr->getItemInterface()->RemoveQuestItemAmt(qst->required_item[i], qst->required_itemcount[i]);
        }

        // Remove srcitem
        if (qst->srcitem && qst->srcitem != qst->receive_items[0])
            plr->getItemInterface()->RemoveItemAmt(qst->srcitem, qst->srcitemcount ? qst->srcitemcount : 1);

        // cast learning spell
        if (qst->reward_spell && !qst->effect_on_player) // qst->reward_spell is the spell the quest finisher teaches you, OR the icon of the spell if effect_on_player is not 0
        {
            if (!plr->hasSpell(qst->reward_spell))
            {
                SpellCastTargets target;
                target.setTargetMask(2);
                target.setUnitTarget(plr->getGuid());

                SmsgSpellStart startPacket(qst_giver->GetNewGUID(), qst_giver->GetNewGUID(), 7763, 0, 0, 0, 0, target);
                plr->getSession()->sendManagedPacket(startPacket);

                SmsgSpellGo goPacket(qst_giver->GetNewGUID(), qst_giver->GetNewGUID(), 7763, 0, 0, 0, 0, target);
                goPacket.hittedTargets.push_back(SpellUniqueTarget(plr->getGuid(), {}));
                plr->getSession()->sendManagedPacket(goPacket);

                // Teach the spell
                plr->addSpell(qst->reward_spell);
            }
        }

        // cast Effect Spell
        if (qst->effect_on_player)
        {
            SpellInfo const* spell_entry = sSpellMgr.getSpellInfo(qst->effect_on_player);
            if (spell_entry)
            {
                Spell* spe = sSpellMgr.newSpell(plr, spell_entry, true, NULL);
                SpellCastTargets tgt(plr->getGuid());
                spe->prepare(&tgt);
            }
        }

        //Add to finished quests
        plr->addQuestToFinished(qst->id);
        if (qst->bonusarenapoints != 0)
        {
            plr->addArenaPoints(qst->bonusarenapoints, true);
        }

#if VERSION_STRING >= Cata
        for (uint8_t i = 0; i < 4; ++i)
        {
            if (qst->reward_currency_id[i] != 0 && qst->reward_currency_count[i] != 0)
                plr->modifyCurrency(qst->reward_currency_id[i], static_cast<int32_t>(qst->reward_currency_count[i]));
        }
#endif

#if VERSION_STRING > TBC
        plr->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT, 1, 0, 0);
        if (qst->reward_money)
            plr->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD, qst->reward_money, 0, 0);
        plr->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE, qst->zone_id, 0, 0);
        plr->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST, qst->id, 0, 0);
#endif
        // Remove quests that are listed to be removed on quest complete.
        std::set<uint32_t>::iterator iter = qst->remove_quest_list.begin();
        for (; iter != qst->remove_quest_list.end(); ++iter)
        {
            if (!plr->hasQuestFinished((*iter)))
                plr->addQuestToFinished((*iter));
        }
    }

    if (qst->MailTemplateId != 0)
    {
        auto mail_template = sMailTemplateStore.lookupEntry(qst->MailTemplateId);
        if (mail_template != nullptr)
        {
            uint8_t mailType = MAIL_TYPE_NORMAL;

            uint64_t itemGuid = 0;

            if (qst_giver->isCreature())
                mailType = MAIL_TYPE_CREATURE;
            else if (qst_giver->isGameObject())
                mailType = MAIL_TYPE_GAMEOBJECT;

            if (qst->MailSendItem != 0)
            {
                // the way it's done in World::PollMailboxInsertQueue
                auto pItem = sObjectMgr.createItem(qst->MailSendItem, NULL);
                if (pItem != NULL)
                {
                    pItem->setStackCount(1);
                    pItem->saveToDB(0, 0, true, NULL);
                    itemGuid = pItem->getGuid();
                }
            }
#if VERSION_STRING > Classic
            sMailSystem.SendCreatureGameobjectMail(mailType, qst_giver->getEntry(), plr->getGuid(), mail_template->subject, mail_template->content, 0, 0, itemGuid, MAIL_STATIONERY_TEST1, MAIL_CHECK_MASK_HAS_BODY, qst->MailDelaySecs);
#endif
        }
    }

    // Hook to Creature Script
    if (qst_giver && qst_giver->ToCreature() && qst_giver->ToCreature()->GetScript())
    {
        qst_giver->ToCreature()->GetScript()->onQuestRewarded(plr, qst);
    }

    plr->updateNearbyQuestGameObjects();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Quest Management

void QuestMgr::LoadNPCQuests(Creature* qst_giver)
{
    qst_giver->SetQuestList(GetCreatureQuestList(qst_giver->getEntry()));
}

void QuestMgr::LoadGOQuests(GameObject* go)
{
    if (go->getGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
    {
        GameObject_QuestGiver* go_quest_giver = static_cast<GameObject_QuestGiver*>(go);
        go_quest_giver->SetQuestList(GetGOQuestList(go->getEntry()));
    }
}

QuestRelationList* QuestMgr::GetGOQuestList(uint32_t entryid)
{
    const auto& olist = m_obj_quests;
    const auto itr = olist.find(entryid);
    return itr == olist.end() ? nullptr : itr->second.get();
}

QuestRelationList* QuestMgr::GetCreatureQuestList(uint32_t entryid)
{
    const auto& olist = m_npc_quests;
    const auto itr = olist.find(entryid);
    return itr == olist.end() ? nullptr : itr->second.get();
}

std::vector<uint32_t> const* QuestMgr::getQuestFinisherEntries(uint32_t questId) const
{
    const auto itr = m_questFinisherEntries.find(questId);
    if (itr == m_questFinisherEntries.end())
        return nullptr;

    return &itr->second;
}

void QuestMgr::addCreatureQuest(uint32_t _entry, const QuestProperties* _questProp, uint8_t _type)
{
    if (_type & QUESTGIVER_QUEST_END)
    {
        auto& finisherEntries = m_questFinisherEntries[_questProp->id];
        if (std::find(finisherEntries.begin(), finisherEntries.end(), _entry) == finisherEntries.end())
            finisherEntries.push_back(_entry);
    }

    const auto [itr, _] = m_npc_quests.try_emplace(_entry, Util::LazyInstanceCreator([] {
        return std::make_unique<QuestRelationList>();
    }));

    auto* questRelationList = itr->second.get();
    for (const auto& relation : *questRelationList)
    {
        if (relation->qst == _questProp)
        {
            relation->type |= _type;
            return;
        }
    }

    questRelationList->emplace_back(std::make_unique<QuestRelation>(_questProp, _type));
}

void QuestMgr::addGameObjectQuest(uint32_t _entry, const QuestProperties* _questProp, uint8_t _type)
{
    if (_type & QUESTGIVER_QUEST_END)
    {
        auto& finisherEntries = m_questFinisherEntries[_questProp->id];
        const uint32_t gameObjectEntry = _entry | 0x80000000;
        if (std::find(finisherEntries.begin(), finisherEntries.end(), gameObjectEntry) == finisherEntries.end())
            finisherEntries.push_back(gameObjectEntry);
    }

    const auto [itr, _] = m_obj_quests.try_emplace(_entry, Util::LazyInstanceCreator([] {
        return std::make_unique<QuestRelationList>();
    }));

    auto* questRelationList = itr->second.get();
    for (const auto& relation : *questRelationList)
    {
        if (relation->qst == _questProp)
        {
            relation->type |= _type;
            return;
        }
    }

    questRelationList->emplace_back(std::make_unique<QuestRelation>(_questProp, _type));
}

//template <class T> void QuestMgr::_AddQuest(uint32_t entryid, QuestProperties const* qst, uint8_t type)
//{
//    std::unordered_map<uint32_t, std::list<QuestRelation*>* > &olist = _GetList<T>();
//    std::list<QuestRelation*>* nlist;
//    QuestRelation* ptr = NULL;
//
//    if (olist.find(entryid) == olist.end())
//    {
//        nlist = new std::list < QuestRelation* > ;
//
//        olist.insert(std::unordered_map<uint32_t, std::list<QuestRelation*>* >::value_type(entryid, nlist));
//    }
//    else
//    {
//        nlist = olist.find(entryid)->second;
//    }
//
//    std::list<QuestRelation*>::iterator it;
//    for (it = nlist->begin(); it != nlist->end(); ++it)
//    {
//        if ((*it)->qst == qst)
//        {
//            ptr = (*it);
//            break;
//        }
//    }
//
//    if (ptr == NULL)
//    {
//        ptr = new QuestRelation;
//        ptr->qst = qst;
//        ptr->type = type;
//
//        nlist->push_back(ptr);
//    }
//    else
//    {
//        ptr->type |= type;
//    }
//}

// Zyres: not used 2022/03/06
//void QuestMgr::_CleanLine(std::string* str)
//{
//    _RemoveChar("\r", str);
//    _RemoveChar("\n", str);
//
//    while (str->c_str()[0] == 32)
//    {
//        str->erase(0, 1);
//    }
//}

void QuestMgr::_RemoveChar(char* c, std::string* str)
{
    std::string::size_type pos = str->find(c, 0);

    while (pos != std::string::npos)
    {
        str->erase(pos, 1);
        pos = str->find(c, 0);
    }
}

uint32_t QuestMgr::GenerateQuestXP(Player* plr, QuestProperties const* qst)
{
    if (qst->is_repeatable != 0)
        return 0;

    // Leaving this for compatibility reason for the old system + custom quests ^^
    if (qst->reward_xp != 0)
    {
        float modifier = 0.0f;
        uint32_t playerlevel = plr->getLevel();
        int32_t questlevel = qst->questlevel;

        if (static_cast<int32_t>(playerlevel) < (questlevel + 6))
            return qst->reward_xp;

        if (static_cast<int32_t>(playerlevel) > (questlevel + 9))
            return 0;

        if (static_cast<int32_t>(playerlevel) == (questlevel + 6))
            modifier = 0.8f;

        if (static_cast<int32_t>(playerlevel) == (questlevel + 7))
            modifier = 0.6f;

        if (static_cast<int32_t>(playerlevel) == (questlevel + 8))
            modifier = 0.4f;

        if (static_cast<int32_t>(playerlevel) == (questlevel + 9))
            modifier = 0.2f;


        return static_cast<uint32_t>(modifier * qst->reward_xp);

    }
    else
    {
        // new quest reward xp calculation mechanism based on DBC values + index taken from DB

        uint32_t realXP = 0;
        uint32_t xpMultiplier = 0;
        int32_t baseLevel = 0;
        int32_t playerLevel = plr->getLevel();
        int32_t QuestLevel = qst->questlevel;

        if (QuestLevel != -1)
            baseLevel = QuestLevel;

        if (((baseLevel - playerLevel) + 10) * 2 > 10)
        {
            baseLevel = playerLevel;

            if (QuestLevel != -1)
                baseLevel = QuestLevel;

            if (((baseLevel - playerLevel) + 10) * 2 <= 10)
            {
                if (QuestLevel == -1)
                    baseLevel = playerLevel;

                xpMultiplier = 2 * (baseLevel - playerLevel) + 20;
            }
            else
            {
                xpMultiplier = 10;
            }
        }
        else
        {
            baseLevel = playerLevel;

            if (QuestLevel != -1)
                baseLevel = QuestLevel;

            if (((baseLevel - playerLevel) + 10) * 2 >= 1)
            {
                baseLevel = playerLevel;

                if (QuestLevel != -1)
                    baseLevel = QuestLevel;

                if (((baseLevel - playerLevel) + 10) * 2 <= 10)
                {
                    if (QuestLevel == -1)
                        baseLevel = playerLevel;

                    xpMultiplier = 2 * (baseLevel - playerLevel) + 20;
                }
                else
                {
                    xpMultiplier = 10;
                }
            }
            else
            {
                xpMultiplier = 1;
            }
        }

#if VERSION_STRING > TBC
        if (const auto pXPData = sQuestXPStore.lookupEntry(baseLevel))
        {
            uint32_t rawXP = xpMultiplier * pXPData->xpIndex[qst->RewXPId] / 10;

            realXP = static_cast<uint32_t>(std::round(rawXP));
        }
#endif

        return realXP;
    }
}

uint32_t QuestMgr::GenerateRewardMoney(Player* /*plr*/, QuestProperties const* qst)
{
    return qst->reward_money;
}

void QuestMgr::SendQuestInvalid(INVALID_REASON reason, Player* plyr)
{
    if (!plyr)
        return;

    SmsgQuestgiverQuestInvalid managedPacket(reason);
    plyr->getSession()->sendManagedPacket(managedPacket);

    sLogger.debug("WORLD:Sent SMSG_QUESTGIVER_QUEST_INVALID");
}

void QuestMgr::SendQuestFailed(FAILED_REASON failed, QuestProperties const* qst, Player* plyr)
{
    if (!plyr)
        return;

    SmsgQuestgiverQuestFailed managedPacket(qst->id, failed);
    plyr->getSession()->sendManagedPacket(managedPacket);

    sLogger.debug("WORLD:Sent SMSG_QUESTGIVER_QUEST_FAILED");
}

void QuestMgr::SendQuestUpdateFailedTimer(QuestProperties const* pQuest, Player* plyr)
{
    if (!plyr)
        return;

    SmsgQuestupdateFailedTimer managedPacket(pQuest->id);
    plyr->getSession()->sendManagedPacket(managedPacket);

    sLogger.debug("WORLD:Sent SMSG_QUESTUPDATE_FAILEDTIMER");
}

void QuestMgr::SendQuestUpdateFailed(QuestProperties const* pQuest, Player* plyr)
{
    if (!plyr)
        return;

    SmsgQuestupdateFailed managedPacket(pQuest->id);
    plyr->getSession()->sendManagedPacket(managedPacket);

    sLogger.debug("WORLD:Sent SMSG_QUESTUPDATE_FAILED");
}

void QuestMgr::SendQuestLogFull(Player* plyr)
{
    if (!plyr)
        return;

    SmsgQuestLogFull managedPacket;
    plyr->getSession()->sendManagedPacket(managedPacket);

    sLogger.debug("WORLD:Sent QUEST_LOG_FULL_MESSAGE");
}

uint32_t QuestMgr::GetGameObjectLootQuest(uint32_t GO_Entry)
{
    std::unordered_map<uint32_t, uint32_t>::iterator itr = m_ObjectLootQuestList.find(GO_Entry);
    if (itr == m_ObjectLootQuestList.end())
        return 0;

    return itr->second;
}

void QuestMgr::SetGameObjectLootQuest(uint32_t GO_Entry, uint32_t Item_Entry)
{
    uint32_t QuestID = 0;
    MySQLDataStore::QuestPropertiesContainer const* its = sMySQLStore.getQuestPropertiesStore();
    for (MySQLDataStore::QuestPropertiesContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        QuestProperties const* qst = sMySQLStore.getQuestProperties(itr->second.id);
        if (qst == nullptr)
            continue;

        for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
        {
            if (qst->required_item[i] == Item_Entry)
            {
                QuestID = qst->id;
                m_ObjectLootQuestList[GO_Entry] = QuestID;
                return;
            }
        }
    }

    /*if (QuestID == 0)
        sLogger.debug("QuestMgr : No corresponding quest was found for loot_gameobjects entryid {} quest item {}", GO_Entry, Item_Entry);*/
}

QuestgiverOfferRewardInput QuestMgr::buildOfferRewardInput(QuestProperties const* qst, Object* qst_giver, Player* plr, uint32_t language)
{
    QuestgiverOfferRewardInput input;

    MySQLStructure::LocalesQuest const* lq = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;

    input.questGiverGuid = qst_giver->getGuid();
    input.mapId = static_cast<uint16_t>(qst_giver->GetMapId());
    input.questGiverCreatureId = qst_giver->isCreature() ? qst_giver->getEntry() : 0;
    input.questId = qst->id;
    input.title = lq ? lq->title : qst->title;
    input.completionText = lq ? lq->completionText : qst->completiontext;
    input.hasNextQuest = qst->next_quest_id != 0;
    input.questFlags = qst->quest_flags;
    input.suggestedPlayers = qst->suggestedplayers;

    for (uint8_t i = 0; i < qst->completionemotecount; i++)
        input.completionEmotes.push_back({ qst->completionemote[i], qst->completionemotedelay[i] });

    input.countRewardChoiceItem = qst->count_reward_choiceitem;
    for (uint8_t i = 0; i < 6; ++i)
    {
        input.rewardChoiceItems[i].itemId = qst->reward_choiceitem[i];
        input.rewardChoiceItems[i].count = qst->reward_choiceitemcount[i];
        if (ItemProperties const* ip = sMySQLStore.getItemProperties(qst->reward_choiceitem[i]))
            input.rewardChoiceItems[i].displayId = ip->DisplayInfoID;
    }

    input.countRewardItem = qst->count_reward_item;
    input.countRequiredItem = qst->count_required_item;
    for (uint8_t i = 0; i < 4; ++i)
    {
        input.rewardItems[i].itemId = qst->reward_item[i];
        input.rewardItems[i].count = qst->reward_itemcount[i];
        if (ItemProperties const* ip = sMySQLStore.getItemProperties(qst->reward_item[i]))
            input.rewardItems[i].displayId = ip->DisplayInfoID;
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        input.rewardCurrencyId[i] = qst->reward_currency_id[i];
        input.rewardCurrencyCount[i] = qst->reward_currency_count[i];
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        input.rewardRepFaction[i] = qst->reward_repfaction[i];
        input.rewardRepValue[i] = qst->reward_repvalue[i];
    }

    if (plr->getLevel() < plr->getMaxLevel())
        input.xp = Util::float2int32(GenerateQuestXP(plr, qst) * worldConfig.getFloatRate(RATE_QUESTXP));

    input.bonusHonor = qst->bonushonor;
    input.rewardSpell = qst->reward_spell;
    input.effectOnPlayer = qst->effect_on_player;
    input.rewardTitleId = qst->rewardtitleid;
    input.rewardTalents = qst->rewardtalents;
    input.bonusArenaPoints = qst->bonusarenapoints;

    input.rewardMoney = GenerateRewardMoney(plr, qst);

    for (uint8_t i = 0; i < 4; ++i)
        input.detailEmotes[i] = { qst->detailemote[i], qst->detailemotedelay[i] };

    return input;
}

std::vector<QuestObjectiveData> QuestMgr::buildQuestObjectives(QuestProperties const* qst, uint32_t language) const
{
    std::vector<QuestObjectiveData> objectives;
    if (qst == nullptr)
        return objectives;

    MySQLStructure::LocalesQuest const* lq = language > 0 ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;
    uint8_t ordinal = 0;
    const auto makeId = [qst](uint8_t index) -> uint32_t { return 0x70000000u | ((qst->id & 0x00FFFFFFu) << 4) | (index & 0x0Fu); };

    for (uint8_t i = 0; i < 4; ++i)
    {
        if (qst->required_mob_or_go[i] == 0 && qst->required_spell[i] == 0 && qst->required_emote[i] == 0)
            continue;

        QuestObjectiveData objective;
        objective.id = makeId(ordinal);
        objective.storageIndex = static_cast<int8_t>(ordinal++);
        objective.sourceIndex = static_cast<int8_t>(i);
        objective.amount = static_cast<int32_t>(qst->required_mob_or_go_count[i]);
        objective.requiredSpellId = qst->required_spell[i];
        objective.requiredEmoteId = qst->required_emote[i];
        objective.description = lq != nullptr ? lq->objectiveText[i] : qst->objectivetexts[i];

        if (qst->required_mob_or_go[i] != 0)
        {
            objective.type = qst->required_mobtype[i] == QUEST_MOB_TYPE_GAMEOBJECT ? QUEST_OBJECTIVE_GAMEOBJECT : QUEST_OBJECTIVE_MONSTER;
            objective.objectId = qst->required_mob_or_go[i] < 0 ? -qst->required_mob_or_go[i] : qst->required_mob_or_go[i];
        }
        else if (qst->required_spell[i] != 0)
        {
            objective.type = QUEST_OBJECTIVE_LEARNSPELL;
            objective.objectId = static_cast<int32_t>(qst->required_spell[i]);
        }
        else
        {
            objective.type = QUEST_OBJECTIVE_TALKTO;
            objective.objectId = 0;
            objective.clientVisible = false;
            objective.storageIndex = -1;
        }

        objectives.push_back(std::move(objective));
    }

    for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        if (qst->required_item[i] == 0)
            continue;

        QuestObjectiveData objective;
        objective.id = makeId(ordinal);
        objective.type = QUEST_OBJECTIVE_ITEM;
        objective.storageIndex = static_cast<int8_t>(ordinal++);
        objective.sourceIndex = static_cast<int8_t>(i);
        objective.objectId = static_cast<int32_t>(qst->required_item[i]);
        objective.amount = static_cast<int32_t>(qst->required_itemcount[i]);
        if (ItemProperties const* item = sMySQLStore.getItemProperties(qst->required_item[i]))
            objective.description = item->Name;
        objectives.push_back(std::move(objective));
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        if (qst->required_triggers[i] == 0)
            continue;

        QuestObjectiveData objective;
        objective.id = makeId(ordinal++);
        objective.type = QUEST_OBJECTIVE_AREATRIGGER;
        objective.storageIndex = -1;
        objective.sourceIndex = static_cast<int8_t>(i);
        objective.objectId = static_cast<int32_t>(qst->required_triggers[i]);
        objective.amount = 1;
        objective.clientVisible = false;
        objectives.push_back(std::move(objective));
    }

    return objectives;
}

uint32_t QuestMgr::getQuestObjectiveProgress(Player* plr, QuestLogEntry const* questLog, QuestObjectiveData const& objective) const
{
    if (plr == nullptr || questLog == nullptr)
        return 0;

    if (objective.type == QUEST_OBJECTIVE_ITEM)
        return std::min<uint32_t>(plr->getItemInterface()->GetQuestItemCount(static_cast<uint32_t>(objective.objectId), true), static_cast<uint32_t>(std::max(objective.amount, 0)));

    if (objective.type == QUEST_OBJECTIVE_AREATRIGGER)
    {
        if (objective.sourceIndex < 0 || objective.sourceIndex >= 4)
            return 0;
        return questLog->getExploredAreaByIndex(static_cast<uint8_t>(objective.sourceIndex)) != 0 ? 1u : 0u;
    }

    if (objective.sourceIndex < 0 || objective.sourceIndex >= 4)
        return 0;

    return questLog->getMobCountByIndex(static_cast<uint8_t>(objective.sourceIndex));
}

bool QuestMgr::updateQuestObjectiveProgress(Player* plr, QuestObjectiveCreditEvent const& event)
{
    if (plr == nullptr)
        return false;

    bool updated = false;

    for (uint8_t slot = 0; slot < MAX_QUEST_SLOT; ++slot)
    {
        QuestLogEntry* questLog = plr->getQuestLogBySlotId(slot);
        if (questLog == nullptr)
            continue;

        QuestProperties const* qst = questLog->getQuestProperties();
        if (qst == nullptr || (event.questId != 0 && qst->id != event.questId))
            continue;

        const auto objectives = buildQuestObjectives(qst, 0);
        for (QuestObjectiveData const& objective : objectives)
        {
            bool matches = false;
            switch (event.type)
            {
                case QuestObjectiveCreditType::MonsterKill:
                    matches = objective.type == QUEST_OBJECTIVE_MONSTER && objective.objectId == event.objectId && objective.requiredSpellId == 0 && objective.requiredEmoteId == 0;
                    break;
                case QuestObjectiveCreditType::GameObjectActivate:
                    matches = objective.type == QUEST_OBJECTIVE_GAMEOBJECT && objective.objectId == event.objectId && objective.requiredSpellId == 0 && objective.requiredEmoteId == 0;
                    break;
                case QuestObjectiveCreditType::ItemPickup:
                    matches = objective.type == QUEST_OBJECTIVE_ITEM && matchesQuestItemObjective(static_cast<uint32_t>(event.objectId), objective.objectId);
                    break;
                case QuestObjectiveCreditType::SpellCast:
                    matches = objective.requiredSpellId == event.actionId && objective.requiredSpellId != 0 && (objective.objectId == event.objectId || (objective.type == QUEST_OBJECTIVE_LEARNSPELL && event.objectId == 0));
                    break;
                case QuestObjectiveCreditType::Emote:
                    matches = objective.requiredEmoteId == event.actionId && objective.requiredEmoteId != 0 && (objective.objectId == event.objectId || objective.objectId == 0);
                    break;
                case QuestObjectiveCreditType::AreaTrigger:
                    matches = objective.type == QUEST_OBJECTIVE_AREATRIGGER && objective.objectId == event.objectId;
                    break;
                case QuestObjectiveCreditType::ScriptExplore:
                    matches = objective.type == QUEST_OBJECTIVE_AREATRIGGER && getQuestObjectiveProgress(plr, questLog, objective) == 0;
                    break;
            }

            if (!matches)
                continue;

            const uint32_t oldProgress = getQuestObjectiveProgress(plr, questLog, objective);
            const uint32_t requiredAmount = static_cast<uint32_t>(std::max(objective.amount, 0));
            uint32_t newProgress = oldProgress;

            if (event.type == QuestObjectiveCreditType::ItemPickup)
            {
                const uint32_t inventoryCount = plr->getItemInterface()->GetQuestItemCount(static_cast<uint32_t>(objective.objectId), true);
                newProgress = std::min(inventoryCount, requiredAmount);
                const uint32_t previousInventoryCount = inventoryCount > event.amount ? inventoryCount - event.amount : 0;
                if (previousInventoryCount >= requiredAmount)
                    continue;
            }
            else if (event.type == QuestObjectiveCreditType::AreaTrigger || event.type == QuestObjectiveCreditType::ScriptExplore)
            {
                if (oldProgress >= 1 || objective.sourceIndex < 0 || objective.sourceIndex >= 4)
                    continue;
                questLog->setExploredAreaForIndex(static_cast<uint8_t>(objective.sourceIndex));
                newProgress = 1;
            }
            else
            {
                if (oldProgress >= requiredAmount || objective.sourceIndex < 0 || objective.sourceIndex >= 4)
                    continue;

                if (event.source != nullptr && (event.type == QuestObjectiveCreditType::SpellCast || event.type == QuestObjectiveCreditType::Emote))
                {
                    Unit* unit = const_cast<Unit*>(dynamic_cast<Unit const*>(event.source));
                    if (unit != nullptr)
                    {
                        if (questLog->isUnitAffected(unit))
                            continue;
                        questLog->addAffectedUnit(unit);
                    }
                }

                questLog->incrementMobCountForIndex(static_cast<uint8_t>(objective.sourceIndex));
                newProgress = getQuestObjectiveProgress(plr, questLog, objective);
            }

            if (event.type == QuestObjectiveCreditType::MonsterKill || event.type == QuestObjectiveCreditType::GameObjectActivate || event.type == QuestObjectiveCreditType::SpellCast)
            {
                if (objective.sourceIndex >= 0 && objective.sourceIndex < 4 && (event.type != QuestObjectiveCreditType::SpellCast || objective.objectId != 0))
                {
                    const uint32_t rawEntry = static_cast<uint32_t>(qst->required_mob_or_go[static_cast<uint8_t>(objective.sourceIndex)]);
                    SendQuestUpdateAddKill(plr, qst->id, rawEntry, newProgress, requiredAmount, event.source);
                }
            }
            else if (event.type == QuestObjectiveCreditType::Emote && qst->id == 11224 && objective.sourceIndex >= 0 && objective.sourceIndex < 4)
            {
                const uint32_t rawEntry = static_cast<uint32_t>(qst->required_mob_or_go[static_cast<uint8_t>(objective.sourceIndex)]);
                SendQuestUpdateAddKill(plr, qst->id, rawEntry, newProgress, requiredAmount, event.source);
            }
            else if (event.type == QuestObjectiveCreditType::ItemPickup)
            {
                if (const auto questScript = questLog->getQuestScript())
                {
                    const uint32_t inventoryCount = plr->getItemInterface()->GetQuestItemCount(static_cast<uint32_t>(objective.objectId), true);
                    questScript->OnPlayerItemPickup(static_cast<uint32_t>(event.objectId), inventoryCount, plr, questLog);
                }

                if (plr->getSession() != nullptr && plr->getSession()->getClientProtocol().isForever())
                {
                    ItemProperties const* itemProperties = sMySQLStore.getItemProperties(static_cast<uint32_t>(event.objectId));
                    const uint32_t proxyItemId = itemProperties != nullptr ? itemProperties->QuestLogItemId : 0;
                    plr->sendQuestItemPushResultPacket(static_cast<uint32_t>(event.objectId), event.amount, std::min(newProgress, requiredAmount), proxyItemId);
                }
                else if (newProgress < requiredAmount)
                {
                    SmsgQuestupdateAddItem addPacket(static_cast<uint32_t>(objective.objectId), event.amount);
                    plr->getSession()->sendManagedPacket(addPacket);
                }
            }

            if (event.type == QuestObjectiveCreditType::MonsterKill && event.source != nullptr)
            {
                if (const auto questScript = questLog->getQuestScript())
                    questScript->OnCreatureKill(static_cast<uint32_t>(objective.objectId), plr, questLog);
            }
            else if (event.type == QuestObjectiveCreditType::GameObjectActivate && event.source != nullptr)
            {
                if (const auto questScript = questLog->getQuestScript())
                    questScript->OnGameObjectActivate(static_cast<uint32_t>(objective.objectId), plr, questLog);
            }
            else if (event.type == QuestObjectiveCreditType::AreaTrigger || event.type == QuestObjectiveCreditType::ScriptExplore)
            {
                if (const auto questScript = questLog->getQuestScript())
                    questScript->OnExploreArea(questLog->m_explored_areas[static_cast<uint8_t>(objective.sourceIndex)], plr, questLog);
            }

            questLog->updatePlayerFields();
            if (questLog->canBeFinished())
                questLog->sendQuestComplete();
            else if (event.type == QuestObjectiveCreditType::GameObjectActivate || event.type == QuestObjectiveCreditType::ItemPickup)
                plr->updateNearbyQuestGameObjects();

            updated = true;

            if (event.type == QuestObjectiveCreditType::GameObjectActivate || event.type == QuestObjectiveCreditType::SpellCast || event.type == QuestObjectiveCreditType::Emote || event.type == QuestObjectiveCreditType::AreaTrigger || event.type == QuestObjectiveCreditType::ScriptExplore)
                break;
        }
    }

    if (event.type == QuestObjectiveCreditType::MonsterKill && event.groupCredit && plr->isInGroup())
    {
        if (auto group = plr->getGroup())
        {
            group->Lock();
            for (uint32_t k = 0; k < group->GetSubGroupCount(); ++k)
            {
                for (const auto& member : group->GetSubGroup(k)->getGroupMembers())
                {
                    Player* groupPlayer = sObjectMgr.getPlayer(member->guid);
                    if (groupPlayer == nullptr || groupPlayer == plr || !plr->isInRange(groupPlayer, 300))
                        continue;

                    QuestObjectiveCreditEvent groupEvent = event;
                    groupEvent.groupCredit = false;
                    updateQuestObjectiveProgress(groupPlayer, groupEvent);
                }
            }
            group->Unlock();
        }
    }

    return updated;
}

QuestgiverQuestDetailsInput QuestMgr::buildQuestDetailsInput(QuestProperties const* qst, Object* qst_giver, Player* plr, uint32_t language)
{
    QuestgiverQuestDetailsInput input;

    MySQLStructure::LocalesQuest const* lq = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;

    input.questGiverGuid = qst_giver->getGuid();
    input.questSharerGuid = qst_giver->isPlayer() ? qst_giver->getGuid() : 0;
    input.mapId = static_cast<uint16_t>(qst_giver->GetMapId());
    input.questGiverCreatureId = qst_giver->isCreature() ? qst_giver->getEntry() : 0;
    input.questStartItemId = qst->srcitem;
    input.questId = qst->id;
    input.title = lq ? lq->title : qst->title;
    input.details = lq ? lq->details : qst->details;
    input.objectives = lq ? lq->objectives : qst->objectives;
    input.questFlags = qst->quest_flags;
    input.suggestedPlayers = qst->suggestedplayers;

    input.countRewardChoiceItem = qst->count_reward_choiceitem;
    for (uint8_t i = 0; i < 6; ++i)
    {
        input.rewardChoiceItems[i].itemId = qst->reward_choiceitem[i];
        input.rewardChoiceItems[i].count = qst->reward_choiceitemcount[i];
        if (ItemProperties const* ip = sMySQLStore.getItemProperties(qst->reward_choiceitem[i]))
            input.rewardChoiceItems[i].displayId = ip->DisplayInfoID;
    }

    input.countRewardItem = qst->count_reward_item;
    input.countRequiredItem = qst->count_required_item;
    for (uint8_t i = 0; i < 4; ++i)
    {
        input.rewardItems[i].itemId = qst->reward_item[i];
        input.rewardItems[i].count = qst->reward_itemcount[i];
        if (ItemProperties const* ip = sMySQLStore.getItemProperties(qst->reward_item[i]))
            input.rewardItems[i].displayId = ip->DisplayInfoID;
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        input.rewardCurrencyId[i] = qst->reward_currency_id[i];
        input.rewardCurrencyCount[i] = qst->reward_currency_count[i];
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        input.rewardRepFaction[i] = qst->reward_repfaction[i];
        input.rewardRepValue[i] = qst->reward_repvalue[i];
    }

    input.rewardMoney = GenerateRewardMoney(plr, qst);
    if (plr->getLevel() < plr->getMaxLevel())
        input.xp = Util::float2int32(GenerateQuestXP(plr, qst) * worldConfig.getFloatRate(RATE_QUESTXP));
    input.bonusHonor = qst->bonushonor;
    input.rewardSpell = qst->reward_spell;
    input.effectOnPlayer = qst->effect_on_player;
    input.rewardTitleId = qst->rewardtitleid;
    input.rewardTalents = qst->rewardtalents;
    input.bonusArenaPoints = qst->bonusarenapoints;
    input.detailEmoteCount = qst->detailemotecount;
    for (uint8_t i = 0; i < 4; ++i)
        input.detailEmotes[i] = { qst->detailemote[i], qst->detailemotedelay[i] };

    if (plr->getSession()->getClientProtocol().isForever())
    {
        const auto objectives = buildQuestObjectives(qst, language);
        input.objectiveEntries.reserve(objectives.size());
        for (QuestObjectiveData const& source : objectives)
        {
            if (!source.clientVisible)
                continue;

            QuestObjectiveSimpleEntry objective;
            objective.id = static_cast<int32_t>(source.id);
            objective.type = source.type;
            objective.objectId = source.objectId;
            objective.amount = source.amount;
            input.objectiveEntries.push_back(objective);
        }
    }
    return input;
}

QuestgiverRequestItemsInput QuestMgr::buildRequestItemsInput(QuestProperties const* qst, Object* qst_giver, uint32_t status, uint32_t language)
{
    QuestgiverRequestItemsInput input;

    MySQLStructure::LocalesQuest const* lq = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;

    input.questGiverGuid = qst_giver->getGuid();
    input.mapId = static_cast<uint16_t>(qst_giver->GetMapId());
    input.questGiverCreatureId = qst_giver->isCreature() ? qst_giver->getEntry() : 0;
    input.questId = qst->id;

    if (lq != nullptr)
    {
        input.title = lq->title;
        input.requestItemsText = (lq->incompleteText[0]) ? lq->incompleteText : lq->details;
    }
    else
    {
        input.title = qst->title;
        input.requestItemsText = qst->incompletetext[0] ? qst->incompletetext : qst->details;
    }

    input.isNotFinished = (status == QuestStatus::NotFinished);
    input.statusEmote = input.isNotFinished ? qst->incompleteemote : qst->completeemote;
    input.questFlags = qst->quest_flags;
    input.suggestedPlayers = qst->suggestedplayers;
    input.requiredMoney = static_cast<uint32_t>(qst->reward_money < 0 ? -qst->reward_money : 0);
    input.countRequiredItem = qst->count_required_item;

    for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        input.requiredItems[i].itemId = qst->required_item[i];
        input.requiredItems[i].count = qst->required_itemcount[i];
        if (qst->required_item[i])
        {
            if (ItemProperties const* it = sMySQLStore.getItemProperties(qst->required_item[i]))
                input.requiredItems[i].displayId = it->DisplayInfoID;
        }
    }

    return input;
}

QuestgiverQuestListInput QuestMgr::buildQuestListInput(Object* qst_giver, Player* plr, uint32_t language)
{
    QuestgiverQuestListInput input;

    input.questGiverGuid = qst_giver->getGuid();
    input.mapId = static_cast<uint16_t>(qst_giver->GetMapId());
    input.greeting = qst_giver->isGameObject() ? "" : plr->getSession()->localizedWorldSrv(ServerString::SS_HEY_HOW_CAN_I_HELP_YOU);

    QuestRelationList::iterator st{};
    QuestRelationList::iterator ed{};
    bool bValid = false;

    if (qst_giver->isGameObject())
    {
        GameObject* go = static_cast<GameObject*>(qst_giver);
        GameObject_QuestGiver* go_quest_giver = nullptr;
        if (go->getGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
        {
            go_quest_giver = static_cast<GameObject_QuestGiver*>(go);
            if (go_quest_giver->HasQuests())
                bValid = true;
        }
        if (bValid)
        {
            st = go_quest_giver->QuestsBegin();
            ed = go_quest_giver->QuestsEnd();
        }
    }
    else if (qst_giver->isCreature())
    {
        bValid = static_cast<Creature*>(qst_giver)->HasQuests();
        if (bValid)
        {
            st = static_cast<Creature*>(qst_giver)->QuestsBegin();
            ed = static_cast<Creature*>(qst_giver)->QuestsEnd();
        }
    }

    input.isValid = bValid;
    if (!bValid)
        return input;

    input.activeQuestsCount = static_cast<uint8_t>(ActiveQuestsCount(qst_giver, plr));

    std::map<uint32_t, uint8_t> tmp_map;

    for (auto it = st; it != ed; ++it)
    {
        const uint32_t status = CalcQuestStatus(qst_giver, plr, it->get());
        if (status < QuestStatus::AvailableChat)
            continue;

        if (tmp_map.find((*it)->qst->id) != tmp_map.end())
            continue;

        tmp_map.insert(std::map<uint32_t, uint8_t>::value_type((*it)->qst->id, static_cast<uint8_t>(1)));
        MySQLStructure::LocalesQuest const* lq = (language > 0) ? sMySQLStore.getLocalizedQuest((*it)->qst->id, language) : nullptr;

        QuestListEntry entry;
        entry.questId = (*it)->qst->id;

        const auto questProp = (*it)->qst;
        switch (status)
        {
            case QuestStatus::NotFinished:
            case QuestStatus::Finished:
                entry.statusIcon = 4;
                break;
            default:
                if (questProp->HasFlag(QUEST_FLAGS_AUTOCOMPLETE) && (questProp->HasFlag(QUEST_FLAGS_DAILY) || questProp->HasFlag(QUEST_FLAGS_WEEKLY)))
                    entry.statusIcon = 0;
                else if (questProp->HasFlag(QUEST_FLAGS_AUTOCOMPLETE))
                    entry.statusIcon = 4;
                else
                    entry.statusIcon = 2;
                break;
        }
        entry.questLevel = (*it)->qst->questlevel;
        entry.questFlags = (*it)->qst->quest_flags;
        entry.isRepeatable = questProp->is_repeatable > 0 && !questProp->HasFlag(QUEST_FLAGS_DAILY) && !questProp->HasFlag(QUEST_FLAGS_WEEKLY);
        entry.title = lq ? lq->title : (*it)->qst->title;

        if (plr->getSession() != nullptr && plr->getSession()->getClientProtocol().isForever())

        input.quests.push_back(entry);
    }

    return input;
}

bool QuestMgr::OnActivateQuestGiver(Object* qst_giver, Player* plr)
{
    if (qst_giver->isGameObject())
    {
        GameObject* gameobject = static_cast<GameObject*>(qst_giver);
        if (gameobject->getGoType() != GAMEOBJECT_TYPE_QUESTGIVER)
            return false;

        GameObject_QuestGiver* go_quest_giver = static_cast<GameObject_QuestGiver*>(gameobject);
        if (!go_quest_giver->HasQuests())
            return false;
    }

    uint32_t questCount = ActiveQuestsCount(qst_giver, plr);

    if (questCount == 0)
    {
        sLogger.debug("WORLD: Invalid NPC for CMSG_QUESTGIVER_HELLO.");
        return false;
    }

    if (questCount == 1)
    {
        QuestRelationList::const_iterator itr;
        QuestRelationList::const_iterator q_begin;
        QuestRelationList::const_iterator q_end;

        bool bValid = false;

        if (qst_giver->isGameObject())
        {
            bValid = false;

            GameObject* gameobject = static_cast<GameObject*>(qst_giver);
            GameObject_QuestGiver* go_quest_giver = nullptr;
            if (gameobject->getGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
            {
                go_quest_giver = static_cast<GameObject_QuestGiver*>(gameobject);
                if (go_quest_giver->HasQuests())
                    bValid = true;
            }
            if (bValid)
            {
                q_begin = go_quest_giver->QuestsBegin();
                q_end = go_quest_giver->QuestsEnd();
            }
        }
        else if (qst_giver->isCreature())
        {
            bValid = static_cast< Creature* >(qst_giver)->HasQuests();
            if (bValid)
            {
                q_begin = static_cast< Creature* >(qst_giver)->QuestsBegin();
                q_end = static_cast< Creature* >(qst_giver)->QuestsEnd();
            }
        }

        if (!bValid)
        {
            sLogger.debug("QUESTS: Warning, invalid NPC {} specified for OnActivateQuestGiver. TypeId: {}.", std::to_string(qst_giver->getGuid()), qst_giver->getObjectTypeId());
            return false;
        }

        for (itr = q_begin; itr != q_end; ++itr)
            if (CalcQuestStatus(qst_giver, plr, itr->get()) >= QuestStatus::AvailableChat)
                break;

        if (CalcStatus(qst_giver, plr) < QuestStatus::AvailableChat)
            return false;

        uint32_t status = CalcStatus(qst_giver, plr);

        if ((status == QuestStatus::Available) || (status == QuestStatus::Repeatable) || (status == QuestStatus::AvailableChat))
        {
            SmsgQuestgiverQuestDetails detailsPacket(buildQuestDetailsInput((*itr)->qst, qst_giver, plr, plr->getSession()->language)); // 1 because we have 1 quest, and we want goodbye to function
            plr->getSession()->sendManagedPacket(detailsPacket);
            sLogger.debug("WORLD: Sent SMSG_QUESTGIVER_QUEST_DETAILS.");

            if ((*itr)->qst->HasFlag(QUEST_FLAGS_AUTO_ACCEPT))
                plr->acceptQuest(qst_giver->getGuid(), (*itr)->qst->id);
        }
        else if (status == QuestStatus::Finished)
        {
            SmsgQuestgiverOfferReward rewardPacket(buildOfferRewardInput((*itr)->qst, qst_giver, plr, plr->getSession()->language));
            plr->getSession()->sendManagedPacket(rewardPacket);
            sLogger.debug("WORLD: Sent SMSG_QUESTGIVER_OFFER_REWARD.");
        }
        else if (status == QuestStatus::NotFinished)
        {
            SmsgQuestgiverRequestItems requestItemsPacket(buildRequestItemsInput((*itr)->qst, qst_giver, status, plr->getSession()->language));
            plr->getSession()->sendManagedPacket(requestItemsPacket);
            sLogger.debug("WORLD: Sent SMSG_QUESTGIVER_REQUEST_ITEMS.");
        }
    }
    else
    {
        SmsgQuestgiverQuestList listPacket(buildQuestListInput(qst_giver, plr, plr->getSession()->language));
        plr->getSession()->sendManagedPacket(listPacket);
        sLogger.debug("WORLD: Sent SMSG_QUESTGIVER_QUEST_LIST.");
    }
    return true;
}

void QuestMgr::finalize()
{
    std::unordered_map<uint32_t, std::list<QuestRelation*>* >::iterator itr2;
    std::list<QuestRelation*>::iterator itr3;

    // clear relations
    m_obj_quests.clear();
    m_npc_quests.clear();

    // todo: m_itm_quests is not used, possibly remove it -Appled
    for (itr2 = m_itm_quests.begin(); itr2 != m_itm_quests.end(); ++itr2)
    {
        if (!itr2->second)
            continue;

        itr3 = itr2->second->begin();
        for (; itr3 != itr2->second->end(); ++itr3)
        {
            delete(*itr3);
        }
        itr2->second->clear();
        delete itr2->second;
    }
    m_itm_quests.clear();
    // NTY.
    m_quest_associations.clear();
}

bool QuestMgr::ownsUniqueRewardItem(Player* plr, ItemProperties const* proto)
{
    return proto->Unique != 0 && plr->getItemInterface()->GetItemCount(proto->ItemId, true) >= proto->Unique;
}

bool QuestMgr::CanStoreReward(Player* plyr, QuestProperties const* qst, uint32_t reward_slot)
{
    uint32_t available_slots = 0;
    uint32_t slotsrequired = 0;
    available_slots = plyr->getItemInterface()->CalculateFreeSlots(NULL);
    // Static Item reward
    for (uint8_t i = 0; i < 4; ++i)
    {
        if (qst->reward_item[i])
        {
            ItemProperties const* proto = sMySQLStore.getItemProperties(qst->reward_item[i]);
            if (!proto)
            {
                sLogger.failure("Invalid item prototype in quest reward! ID {}, quest {}", qst->reward_item[i], qst->id);
                slotsrequired++;
            }
            else if (ownsUniqueRewardItem(plyr, proto))
            {
                continue;
            }
            else
            {
                slotsrequired++;
                if (plyr->getItemInterface()->CanReceiveItem(proto, qst->reward_itemcount[i]))
                    return false;
            }
        }
    }

    // Choice Rewards
    if (qst->reward_choiceitem[reward_slot])
    {
        slotsrequired++;
        ItemProperties const* proto = sMySQLStore.getItemProperties(qst->reward_choiceitem[reward_slot]);
        if (!proto)
            sLogger.failure("Invalid item prototype in quest reward! ID {}, quest {}", qst->reward_choiceitem[reward_slot], qst->id);
        else if (plyr->getItemInterface()->CanReceiveItem(proto, qst->reward_choiceitemcount[reward_slot]))
            return false;
    }
    if (available_slots < slotsrequired)
    {
        return false;
    }

    return true;
}

void QuestMgr::LoadExtraQuestStuff()
{
    MySQLDataStore::QuestPropertiesContainer const* its = sMySQLStore.getQuestPropertiesStore();
    for (MySQLDataStore::QuestPropertiesContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        QuestProperties const* qst = sMySQLStore.getQuestProperties(itr->second.id);
        if (qst == nullptr)
            continue;

        // 0 them out
        const_cast<QuestProperties*>(qst)->count_required_item = 0;
        const_cast<QuestProperties*>(qst)->count_required_mob = 0;
        const_cast<QuestProperties*>(qst)->count_requiredtriggers = 0;
        const_cast<QuestProperties*>(qst)->count_receiveitems = 0;
        const_cast<QuestProperties*>(qst)->count_reward_item = 0;
        const_cast<QuestProperties*>(qst)->count_reward_choiceitem = 0;

        const_cast<QuestProperties*>(qst)->required_mobtype[0] = 0;
        const_cast<QuestProperties*>(qst)->required_mobtype[1] = 0;
        const_cast<QuestProperties*>(qst)->required_mobtype[2] = 0;
        const_cast<QuestProperties*>(qst)->required_mobtype[3] = 0;

        const_cast<QuestProperties*>(qst)->count_requiredquests = 0;

        if (qst->x_or_y_quest_string.size())
        {
            const_cast<QuestProperties*>(qst)->quest_list.clear();
            std::string quests = std::string(qst->x_or_y_quest_string);
            std::vector<std::string> qsts = AscEmu::Util::Strings::split(quests, " ");
            for (std::vector<std::string>::iterator iter = qsts.begin(); iter != qsts.end(); ++iter)
            {
                uint32_t id = std::stoul((*iter).c_str());
                if (id)
                    const_cast<QuestProperties*>(qst)->quest_list.insert(id);
            }
        }

        if (qst->remove_quests.size())
        {
            std::string quests = std::string(qst->remove_quests);
            std::vector<std::string> qsts = AscEmu::Util::Strings::split(quests, " ");
            for (std::vector<std::string>::iterator iter = qsts.begin(); iter != qsts.end(); ++iter)
            {
                uint32_t id = std::stoul((*iter).c_str());
                if (id)
                    const_cast<QuestProperties*>(qst)->remove_quest_list.insert(id);
            }
        }

        for (uint8_t i = 0; i < 4; ++i)
        {
            if (qst->required_mob_or_go[i] != 0)
            {
                if (qst->required_mob_or_go[i] < 0)
                {
                    auto gameobject_info = sMySQLStore.getGameObjectProperties(qst->required_mob_or_go[i] * -1);
                    if (gameobject_info)
                    {
                        const_cast<QuestProperties*>(qst)->required_mobtype[i] = QUEST_MOB_TYPE_GAMEOBJECT;
                        const_cast<QuestProperties*>(qst)->required_mob_or_go[i] *= -1;
                    }
                    else
                    {
                        // if quest has neither valid gameobject, log it.
                        sLogger.debugDbTables("Quest {} has required_mobtype[{}]=={}, it's not a valid GameObject.", qst->id, i, qst->required_mob_or_go[i]);
                    }
                }
                else
                {
                    CreatureProperties const* c_info = sMySQLStore.getCreatureProperties(qst->required_mob_or_go[i]);
                    if (c_info)
                        const_cast<QuestProperties*>(qst)->required_mobtype[i] = QUEST_MOB_TYPE_CREATURE;
                    else
                    {
                        // if quest has neither valid creature, log it.
                        sLogger.debugDbTables("Quest {} has required_mobtype[{}]=={}, it's not a valid Creature.", qst->id, i, qst->required_mob_or_go[i]);
                    }
                }

                const_cast<QuestProperties*>(qst)->count_required_mob++;
            }

            if (qst->reward_item[i])
                const_cast<QuestProperties*>(qst)->count_reward_item++;

            if (qst->required_triggers[i])
                const_cast<QuestProperties*>(qst)->count_requiredtriggers++;

            if (qst->receive_items[i])
                const_cast<QuestProperties*>(qst)->count_receiveitems++;

            if (qst->required_quests[i])
                const_cast<QuestProperties*>(qst)->count_requiredquests++;
        }

        for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
            if (qst->required_item[i] != 0)
                const_cast<QuestProperties*>(qst)->count_required_item++;

        for (uint8_t i = 0; i < 6; ++i)
        {
            if (qst->reward_choiceitem[i])
                const_cast<QuestProperties*>(qst)->count_reward_choiceitem++;
        }

        const_cast<QuestProperties*>(qst)->pQuestScript = nullptr;
    }

    // load creature starters
    uint32_t entry, quest;

    auto pResult = sMySQLStore.getWorldDBQuery("SELECT * FROM creature_quest_starter WHERE min_build <= %u AND max_build >= %u", WoW::getConfigBuild(), WoW::getConfigBuild());
    if (pResult)
    {
        do
        {
            Field* data = pResult->fetch();
            entry = data[0].asUint32();
            quest = data[1].asUint32();

            if (auto qst = sMySQLStore.getQuestProperties(quest))
                addCreatureQuest(entry, qst, 1);  // 1 = starter
            else
                sLogger.debugDbTables("Tried to add starter to npc {} for non-existent quest {} in table creature_quest_starter.", entry, quest);
        } while (pResult->nextRow());
    }

    pResult = sMySQLStore.getWorldDBQuery("SELECT * FROM creature_quest_finisher WHERE min_build <= %u AND max_build >= %u", WoW::getConfigBuild(), WoW::getConfigBuild());
    if (pResult)
    {
        do
        {
            Field* data = pResult->fetch();
            entry = data[0].asUint32();
            quest = data[1].asUint32();

            if (auto qst = sMySQLStore.getQuestProperties(quest))
                addCreatureQuest(entry, qst, 2); // 2 = finisher
            else
                sLogger.debugDbTables("Tried to add finisher to npc {} for non-existent quest {} in table creature_quest_finisher.", entry, quest);
        } while (pResult->nextRow());
    }

    pResult = sMySQLStore.getWorldDBQuery("SELECT * FROM gameobject_quest_starter WHERE min_build <= %u AND max_build >= %u", WoW::getConfigBuild(), WoW::getConfigBuild());
    if (pResult)
    {
        do
        {
            Field* data = pResult->fetch();
            entry = data[0].asUint32();
            quest = data[1].asUint32();

            if (auto qst = sMySQLStore.getQuestProperties(quest))
                addGameObjectQuest(entry, qst, 1); // 1 = starter
            else
                sLogger.debugDbTables("Tried to add starter to go {} for non-existent quest {} in table gameobject_quest_starter.", entry, quest);
        } while (pResult->nextRow());
    }

    pResult = sMySQLStore.getWorldDBQuery("SELECT * FROM gameobject_quest_finisher WHERE min_build <= %u AND max_build >= %u", WoW::getConfigBuild(), WoW::getConfigBuild());
    if (pResult)
    {
        do
        {
            Field* data = pResult->fetch();
            entry = data[0].asUint32();
            quest = data[1].asUint32();

            if (auto qst = sMySQLStore.getQuestProperties(quest))
                addGameObjectQuest(entry, qst, 2); // 2 = finish
            else
                sLogger.debugDbTables("Tried to add finisher to go {} for non-existent quest {} in table gameobject_quest_finisher.", entry, quest);
        } while (pResult->nextRow());
    }

    //sObjectMgr.ProcessGameobjectQuests();

    //load item quest associations
    uint32_t item;
    uint8_t item_count;

    pResult = WorldDatabase.query("SELECT * FROM item_quest_association");
    if (pResult != NULL)
    {
        do
        {
            Field* data = pResult->fetch();
            item = data[0].asUint32();
            quest = data[1].asUint32();
            item_count = data[2].asUint8();

            auto qst = sMySQLStore.getQuestProperties(quest);
            if (qst == nullptr)
            {
                sLogger.debugDbTables("Tried to add association to item {} for non-existent quest {}.", item, quest);
            }
            else
            {
                AddItemQuestAssociation(item, qst, item_count);
            }
        }
        while (pResult->nextRow());
    }

    m_QuestPOIMap.clear();

#if defined(AE_FOREVER)
    for (auto const& [questId, db2Blobs] : sForeverQuestPOIStore)
    {
        QuestPOIVector& target = m_QuestPOIMap[questId];
        target.reserve(db2Blobs.size());
        uint32_t blobIndex = 0;
        for (ForeverQuestPOIBlobData const& db2Blob : db2Blobs)
        {
            QuestPOI poi(blobIndex++, db2Blob.objectiveIndex, db2Blob.mapId, db2Blob.uiMapId, 0, 0, db2Blob.flags);
            poi.QuestObjectiveId = db2Blob.objectiveId;
            poi.PlayerConditionId = db2Blob.playerConditionId;
            poi.NavigationPlayerConditionId = db2Blob.navigationPlayerConditionId;
            poi.points.reserve(db2Blob.points.size());
            for (ForeverQuestPOIPointData const& db2Point : db2Blob.points)
                poi.points.emplace_back(db2Point.x, db2Point.y, db2Point.z);
            target.push_back(std::move(poi));
        }
    }
    sLogger.info("QuestMgr : seeded POI data for {} quests from Forever DB2.", sForeverQuestPOIStore.size());
#endif

    auto result = WorldDatabase.query("SELECT build, questId, poiId, objIndex, mapId, mapAreaId, floorId, unk3, unk4 FROM quest_poi base WHERE build=(SELECT MAX(build) FROM quest_poi buildspecific WHERE base.questId = buildspecific.questId AND buildspecific.build <= %u) ORDER BY questId, poiId", VERSION_STRING);
    if (result != NULL)
    {
        uint32_t count = 0;
        std::unordered_set<uint32_t> sqlOverrideQuests;

        do
        {
            Field* fields = result->fetch();

            const uint32_t selectedBuild = fields[0].asUint32();
            const uint32_t questId = fields[1].asUint32();
#if defined(AE_FOREVER)
            if (selectedBuild == 0 && sForeverQuestPOIStore.contains(questId))
                continue;
#endif
            if (sqlOverrideQuests.insert(questId).second)
                m_QuestPOIMap[questId].clear();
            uint32_t poiId = fields[2].asUint32();
            int32_t  objIndex = fields[3].asInt32();
            uint32_t mapId = fields[4].asUint32();
            uint32_t mapAreaId = fields[5].asUint32();
            uint32_t floorId = fields[6].asUint32();
            uint32_t unk3 = fields[7].asUint32();
            uint32_t unk4 = fields[8].asUint32();

            QuestPOI POI(poiId, objIndex, mapId, mapAreaId, floorId, unk3, unk4);
            m_QuestPOIMap[questId].push_back(POI);

            count++;
        }
        while (result->nextRow());

        sLogger.info("QuestMgr : Point Of Interest (POI) data loaded for {} quests.", count);

        auto points = WorldDatabase.query("SELECT points.questId, points.poiId, points.x, points.y FROM quest_poi_points points WHERE points.build=(SELECT MAX(poi.build) FROM quest_poi poi WHERE poi.questId = points.questId AND poi.build <= %u) ORDER BY points.questId, points.poiId", VERSION_STRING);
        if (points != NULL)
        {
            count = 0;
            do
            {
                Field* pointFields = points->fetch();

                uint32_t questId = pointFields[0].asUint32();
                if (!sqlOverrideQuests.contains(questId))
                    continue;
                uint32_t poiId = pointFields[1].asUint32();
                int32_t  x = pointFields[2].asInt32();
                int32_t  y = pointFields[3].asInt32();

                QuestPOIVector & vect = m_QuestPOIMap[questId];

                for (QuestPOIVector::iterator itr = vect.begin(); itr != vect.end(); ++itr)
                {
                    if (itr->PoiId != poiId)
                        continue;

                    QuestPOIPoint point(x, y);
                    itr->points.push_back(point);
                    break;
                }

                count++;
            }
            while (points->nextRow());

            sLogger.info("QuestMgr : {} quest Point Of Interest points loaded.", count);
        }
    }
}

void QuestMgr::AddItemQuestAssociation(uint32_t itemId, QuestProperties const* qst, uint8_t item_count)
{
    // look for the item in the associationList
    // Create a new QuestAssociationList or search through existing QuestAssociationList
    const auto [itr, _] = m_quest_associations.try_emplace(itemId, Util::LazyInstanceCreator([] {
        return std::make_unique<QuestAssociationList>();
    }));

    auto* tempList = itr->second.get();

    // look through this item's QuestAssociationList for a matching quest entry
    for (auto it = tempList->cbegin(); it != tempList->cend(); ++it)
    {
        if ((*it)->qst == qst)
        {
            // matching quest found
            // update the QuestAssociation with the new item_count information
            (*it)->item_count = item_count;
            sLogger.debug("WARNING: Duplicate entries found in item_quest_association, updating item #{} with new item_count: {}.", itemId, item_count);
            return;
        }
    }

    // create a new QuestAssociation for this item and quest
    tempList->emplace_back(std::make_unique<QuestAssociation>(qst, item_count));
}

QuestAssociationList* QuestMgr::GetQuestAssociationListForItemId(uint32_t itemId)
{
    const auto itr = m_quest_associations.find(itemId);
    if (itr == m_quest_associations.end())
        return nullptr;

    return itr->second.get();
}

void QuestMgr::OnPlayerEmote(Player* plr, uint32_t emoteid, uint64_t& victimguid)
{
    if (plr == nullptr || emoteid == 0)
        return;

    Unit* victim = victimguid != 0 && plr->getWorldMap() != nullptr ? plr->getWorldMapUnit(victimguid) : nullptr;

    QuestObjectiveCreditEvent event;
    event.type = QuestObjectiveCreditType::Emote;
    event.objectId = victim != nullptr ? static_cast<int32_t>(victim->getEntry()) : 0;
    event.actionId = emoteid;
    event.source = victim;
    updateQuestObjectiveProgress(plr, event);
}

QuestPOIVector* QuestMgr::getQuestPOIMap(uint32_t questId)
{
    QuestPOIMap::iterator itr = m_QuestPOIMap.find(questId);
    if (itr != m_QuestPOIMap.end())
        return &(itr->second);

    return nullptr;
}

void QuestMgr::FillQuestMenu(Creature* giver, Player* plr, GossipMenu & menu)
{
    uint8_t icon;
    if (giver->isQuestGiver() && giver->HasQuests())
    {
        for (auto itr = giver->QuestsBegin(); itr != giver->QuestsEnd(); ++itr)
        {
            uint32_t status = CalcQuestStatus(giver, plr, itr->get());
            if (status >= QuestStatus::AvailableChat)
            {
                const auto questProp = (*itr)->qst;
                switch (status)
                {
                    case QuestStatus::NotFinished:
                    case QuestStatus::Finished:
                        icon = 4;
                        break;
                    default:
                        if (questProp->HasFlag(QUEST_FLAGS_AUTOCOMPLETE) && (questProp->HasFlag(QUEST_FLAGS_DAILY) || questProp->HasFlag(QUEST_FLAGS_WEEKLY)))
                            icon = 0;
                        else if (questProp->HasFlag(QUEST_FLAGS_AUTOCOMPLETE))
                            icon = 4;
                        else
                            icon = 2;
                        break;
                }

                menu.addQuest((*itr)->qst, icon);
            }
        }
    }
}
