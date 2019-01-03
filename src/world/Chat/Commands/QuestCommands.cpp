/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/QuestLogEntry.hpp"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"

class ChatHandler;

uint32 GetQuestIDFromLink(const char* questlink)
{
    if (questlink == NULL)
        return 0;

    const char* ptr = strstr(questlink, "|Hquest:");
    if (ptr == NULL)
    {
        return 0;
    }

    return atol(ptr + 8);       // quest id is just past "|Hquest:" (8 bytes)
}

std::string RemoveQuestFromPlayer(Player* plr, QuestProperties const* qst)
{
    std::string recout = "|cff00ff00";

    if (plr->HasQuests())
    {
        if (plr->HasFinishedQuest(qst->id))
            recout += "Player has already completed that quest.\n\n";
        else
        {
            QuestLogEntry* qLogEntry = plr->GetQuestLogForEntry(qst->id);
            if (qLogEntry)
            {
                CALL_QUESTSCRIPT_EVENT(qLogEntry, OnQuestCancel)(plr);
                qLogEntry->Finish();

                // Remove all items given by the questgiver at the beginning
                for (uint8 i = 0; i < 4; ++i)
                {
                    if (qst->receive_items[i])
                        plr->getItemInterface()->RemoveItemAmt(qst->receive_items[i], 1);
                }

                plr->UpdateNearbyGameObjects();
            }
            else
                recout += "No quest log entry found for that player.";
        }
    }
    else
    {
        recout += "Player has no quests to remove.";
    }

    recout += "\n\n";

    return recout;
}

bool ChatHandler::HandleQuestStatusCommand(const char* args, WorldSession* m_session)
{
    if (!*args) return false;

    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (plr == nullptr)
        return true;

    uint32 quest_id = atol(args);
    if (quest_id == 0)
    {
        quest_id = GetQuestIDFromLink(args);
        if (quest_id == 0)
            return false;
    }
    std::string recout = "|cff00ff00";

    QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
    if (qst)
    {
        if (plr->HasFinishedQuest(quest_id))
            recout += "Player has already completed that quest.";
        else
        {
            QuestLogEntry* IsPlrOnQuest = plr->GetQuestLogForEntry(quest_id);
            if (IsPlrOnQuest)
                recout += "Player is currently on that quest.";
            else
                recout += "Player has NOT finished that quest.";
        }
    }
    else
    {
        recout += "Quest Id [";
        recout += args;
        recout += "] was not found and unable to add it to the player's quest log.";
    }

    recout += "\n\n";

    SendMultilineMessage(m_session, recout.c_str());

    return true;
}

bool ChatHandler::HandleQuestStartCommand(const char* args, WorldSession* m_session)
{
    if (!*args) return false;

    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (plr == nullptr)
        return true;

    uint32 quest_id = atol(args);
    if (quest_id == 0)
    {
        quest_id = GetQuestIDFromLink(args);
        if (quest_id == 0)
            return false;
    }
    std::string recout = "|cff00ff00";

    QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
    if (qst)
    {
        if (plr->HasFinishedQuest(quest_id))
            recout += "Player has already completed that quest.";
        else
        {
            QuestLogEntry* IsPlrOnQuest = plr->GetQuestLogForEntry(quest_id);
            if (IsPlrOnQuest)
                recout += "Player is currently on that quest.";
            else
            {
                uint16 open_slot = plr->GetOpenQuestSlot();
                if (open_slot > MAX_QUEST_SLOT)
                {
                    sQuestMgr.SendQuestLogFull(plr);
                    recout += "Player's quest log is full.";
                }
                else
                {
                    if ((qst->time != 0) && plr->HasTimedQuest())
                    {
                        sQuestMgr.SendQuestInvalid(INVALID_REASON_HAVE_TIMED_QUEST, plr);
                        return true;
                    }

                    sGMLog.writefromsession(m_session, "started quest %u [%s] for player %s", qst->id, qst->title.c_str(), plr->getName().c_str());

                    QuestLogEntry* qle = new QuestLogEntry();
                    qle->Init(qst, plr, open_slot);
                    qle->UpdatePlayerFields();

                    // If the quest should give any items on begin, give them the items.
                    for (uint8 i = 0; i < 4; ++i)
                    {
                        if (qst->receive_items[i])
                        {
                            Item* item = objmgr.CreateItem(qst->receive_items[i], plr);
                            if (item == NULL)
                                return false;

                            if (!plr->getItemInterface()->AddItemToFreeSlot(item))
                                item->DeleteMe();
                        }
                    }

                    if (qst->srcitem && qst->srcitem != qst->receive_items[0])
                    {
                        Item* item = objmgr.CreateItem(qst->srcitem, plr);
                        if (item)
                        {
                            item->setStackCount(qst->srcitemcount ? qst->srcitemcount : 1);
                            if (!plr->getItemInterface()->AddItemToFreeSlot(item))
                                item->DeleteMe();
                        }
                    }


                    //if (qst->count_required_item || qst_giver->getObjectTypeId() == TYPEID_GAMEOBJECT)	// gameobject quests deactivate
                    //	plr->UpdateNearbyGameObjects();
                    //ScriptSystem->OnQuestEvent(qst, TO< Creature* >(qst_giver), _player, QUEST_EVENT_ON_ACCEPT);

                    sHookInterface.OnQuestAccept(plr, qst, NULL);

                    recout += "Quest has been added to the player's quest log.";
                }
            }
        }
    }
    else
    {
        recout += "Quest Id [";
        recout += args;
        recout += "] was not found and unable to add it to the player's quest log.";
    }

    recout += "\n\n";

    SendMultilineMessage(m_session, recout.c_str());

    return true;
}

bool ChatHandler::HandleQuestFinishCommand(const char* args, WorldSession* m_session)
{
    if (!*args) return false;

    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (plr == nullptr)
        return true;

    uint32 quest_id = atol(args);
    // reward_slot is for when quest has choice of rewards (0 is the first choice, 1 is the second choice, ...)
    // reward_slot will default to 0 if none is specified
    uint32 reward_slot;
    if (quest_id == 0)
    {
        quest_id = GetQuestIDFromLink(args);
        if (quest_id == 0)
            return false;
        if (strstr(args, "|r"))
        {
            reward_slot = atol(strstr(args, "|r") + 2);
        }
        else
        {
            reward_slot = 0;
        }
    }
    else if (strchr(args, ' '))
    {
        reward_slot = atol(strchr(args, ' ') + 1);
    }
    else
    {
        reward_slot = 0;
    }
    // currently Quest::reward_choiceitem declaration is
    // uint32 reward_choiceitem[6];
    // so reward_slot must be 0 to 5
    if (reward_slot > 5)
    {
        reward_slot = 0;
    }
    std::string recout = "|cff00ff00";

    QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
    if (qst)
    {
        if (plr->HasFinishedQuest(quest_id))
            recout += "Player has already completed that quest.\n\n";
        else
        {
            QuestLogEntry* IsPlrOnQuest = plr->GetQuestLogForEntry(quest_id);
            if (IsPlrOnQuest)
            {
                uint32 giver_id = 0;
                QueryResult* creatureResult = WorldDatabase.Query("SELECT id FROM creature_quest_starter WHERE quest = %u AND min_build <= %u AND max_build >= %u", quest_id, VERSION_STRING, VERSION_STRING);

                if (creatureResult)
                {
                    Field* creatureFields = creatureResult->Fetch();
                    giver_id = creatureFields[0].GetUInt32();
                    delete creatureResult;
                }
                else
                {
                    QueryResult* objectResult = WorldDatabase.Query("SELECT id FROM gameobject_quest_starter WHERE quest = %u AND min_build <= %u AND max_build >= %u", quest_id, VERSION_STRING, VERSION_STRING);
                    if (objectResult)
                    {
                        Field* objectFields = objectResult->Fetch();
                        giver_id = objectFields[0].GetUInt32();
                        delete objectResult;
                    }
                }

                if (giver_id == 0)
                    SystemMessage(m_session, "Unable to find quest giver creature or object.");
                else
                {
                    // I need some way to get the guid without targeting the creature or looking through all the spawns...
                    Object* questGiver = nullptr;

                    for (auto pCreature: plr->GetMapMgr()->CreatureStorage)
                    {
                        if (pCreature)
                        {
                            if (pCreature->getEntry() == giver_id) //found creature
                            {
                                questGiver = pCreature;
                            }
                        }
                    }

                    if (questGiver)
                    {
                        GreenSystemMessage(m_session, "Found a quest_giver creature.");
                        sQuestMgr.OnActivateQuestGiver(questGiver, plr);
                        sQuestMgr.GiveQuestRewardReputation(plr, qst, questGiver);
                    }
                    else
                        RedSystemMessage(m_session, "Unable to find quest_giver object.");
                }

                IsPlrOnQuest->Finish();
                recout += "Player was on that quest, but has now completed it.";
            }
            else
            {
                recout += "The quest has now been completed for that player.";
            }

            sGMLog.writefromsession(m_session, "completed quest %u [%s] for player %s", quest_id, qst->title.c_str(), plr->getName().c_str());
            sQuestMgr.BuildQuestComplete(plr, qst);
            plr->AddToFinishedQuests(quest_id);

            // Quest Rewards : Copied from QuestMgr::OnQuestFinished()
            // Reputation reward
            for (uint8 z = 0; z < 6; z++)
            {
                if (qst->reward_repfaction[z])
                {
                    int32 amt = 0;
                    uint32 fact = qst->reward_repfaction[z];
                    if (qst->reward_repvalue[z])
                    {
                        amt = qst->reward_repvalue[z];
                    }
                    if (qst->reward_replimit && (plr->GetStanding(fact) >= (int32)qst->reward_replimit))
                    {
                        continue;
                    }
                    amt = float2int32(amt * worldConfig.getFloatRate(RATE_QUESTREPUTATION));
                    plr->ModStanding(fact, amt);
                }
            }
            // Static Item reward
            for (uint8 i = 0; i < 4; ++i)
            {
                if (qst->reward_item[i])
                {
                    ItemProperties const* proto = sMySQLStore.getItemProperties(qst->reward_item[i]);
                    if (!proto)
                    {
                        LOG_ERROR("Invalid item prototype in quest reward! ID %d, quest %d", qst->reward_item[i], qst->id);
                    }
                    else
                    {
                        auto item_add = plr->getItemInterface()->FindItemLessMax(qst->reward_item[i], qst->reward_itemcount[i], false);
                        if (!item_add)
                        {
                            auto slotresult = plr->getItemInterface()->FindFreeInventorySlot(proto);
                            if (!slotresult.Result)
                            {
                                plr->getItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                            }
                            else
                            {
                                auto item = objmgr.CreateItem(qst->reward_item[i], plr);
                                if (item)
                                {
                                    item->setStackCount(uint32(qst->reward_itemcount[i]));
                                    if (!plr->getItemInterface()->SafeAddItem(item, slotresult.ContainerSlot, slotresult.Slot))
                                    {
                                        item->DeleteMe();
                                    }
                                }
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
            // Choice Rewards -- Defaulting to choice 0 for ".quest complete" command
            if (qst->reward_choiceitem[reward_slot])
            {
                ItemProperties const* proto = sMySQLStore.getItemProperties(qst->reward_choiceitem[reward_slot]);
                if (!proto)
                {
                    LOG_ERROR("Invalid item prototype in quest reward! ID %d, quest %d", qst->reward_choiceitem[reward_slot], qst->id);
                }
                else
                {
                    auto item_add = plr->getItemInterface()->FindItemLessMax(qst->reward_choiceitem[reward_slot], qst->reward_choiceitemcount[reward_slot], false);
                    if (!item_add)
                    {
                        auto slotresult = plr->getItemInterface()->FindFreeInventorySlot(proto);
                        if (!slotresult.Result)
                        {
                            plr->getItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                        }
                        else
                        {
                            auto item = objmgr.CreateItem(qst->reward_choiceitem[reward_slot], plr);
                            if (item)
                            {
                                item->setStackCount(uint32(qst->reward_choiceitemcount[reward_slot]));
                                if (!plr->getItemInterface()->SafeAddItem(item, slotresult.ContainerSlot, slotresult.Slot))
                                {
                                    item->DeleteMe();
                                }
                            }
                        }
                    }
                    else
                    {
                        item_add->setStackCount(item_add->getStackCount() + qst->reward_choiceitemcount[reward_slot]);
                        item_add->m_isDirty = true;
                    }
                }
            }
            // if daily then append to finished dailies
            if (qst->is_repeatable == arcemu_QUEST_REPEATABLE_DAILY)
                plr->PushToFinishedDailies(qst->id);
            // Remove quests that are listed to be removed on quest complete.
            std::set<uint32>::iterator iter = qst->remove_quest_list.begin();
            for (; iter != qst->remove_quest_list.end(); ++iter)
            {
                if (!plr->HasFinishedQuest((*iter)))
                    plr->AddToFinishedQuests((*iter));
            }

#if VERSION_STRING > TBC
            plr->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT, 1, 0, 0);
#endif
            if (qst->reward_money > 0)
            {
                // Money reward
                // Check they don't have more than the max gold
                if (worldConfig.player.isGoldCapEnabled && (plr->getCoinage() + qst->reward_money) <= worldConfig.player.limitGoldAmount)
                {
                    plr->modCoinage(qst->reward_money);
                }
#if VERSION_STRING > TBC
                plr->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD, qst->reward_money, 0, 0);
#endif
            }
#if VERSION_STRING > TBC
            plr->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE, qst->zone_id, 0, 0);
            plr->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST, qst->id, 0, 0);
#endif
        }
    }
    else
    {
        recout += "Quest Id [";
        recout += args;
        recout += "] was not found and unable to add it to the player's quest log.";
    }

    recout += "\n\n";

    SendMultilineMessage(m_session, recout.c_str());

    return true;
}

bool ChatHandler::HandleQuestFailCommand(const char *args, WorldSession* m_session)
{
    if (args == NULL)
        return false;

    uint32 questid = static_cast<uint32>(atoi(args));

    if (questid == 0)
        return false;

    Player* player = m_session->GetPlayer();
    QuestLogEntry *qle = player->GetQuestLogForEntry(questid);

    if (qle == NULL)
    {
        RedSystemMessage(m_session, "quest %u not found in player's questlog", questid);
        return false;
    }

    qle->Fail(false);

    return true;
}

bool ChatHandler::HandleQuestItemCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string my_item_lookup = "SELECT item, item_count FROM gameobject_quest_item_binding WHERE quest = " + std::string(args);

    QueryResult* result = WorldDatabase.Query(my_item_lookup.c_str());
    std::string recout;
    if (!result)
    {
        recout = "|cff00ccffNo matches found.\n\n";
        SendMultilineMessage(m_session, recout.c_str());
        return true;
    }
    else
    {
        recout = "|cff00ff00Quest item matches: itemid: count -> Name\n\n";
        SendMultilineMessage(m_session, recout.c_str());

        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 id = fields[0].GetUInt32();
            std::string itemid = MyConvertIntToString(id);
            std::string itemcnt = MyConvertIntToString(fields[1].GetUInt32());
            auto tmpItem = sMySQLStore.getItemProperties(id);
            if (tmpItem != nullptr)
            {
                recout = "|cff00ccff";
                recout += itemid.c_str();
                recout += ": ";
                recout += itemcnt.c_str();
                recout += " -> ";
                recout += tmpItem->Name.c_str();
                recout += "\n";
            }
            else
                recout = "|cffff0000Invalid Item!\n";


            SendMultilineMessage(m_session, recout.c_str());

            ++count;

            if (count == 25)
            {
                RedSystemMessage(m_session, "More than 25 results returned. aborting.");
                break;
            }
        }
        while (result->NextRow());
        delete result;
    }

    return true;
}

bool ChatHandler::HandleQuestGiverCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string recout;

    std::string my_query1 = "SELECT id FROM creature_quest_starter WHERE quest = " + std::string(args) + " AND min_build <= %u AND max_build >= %u";
    QueryResult* objectResult1 = WorldDatabase.Query(my_query1.c_str(), VERSION_STRING, VERSION_STRING);

    if (objectResult1)
    {
        Field* fields = objectResult1->Fetch();
        std::string creatureId1 = MyConvertIntToString(fields[0].GetUInt32());

        delete objectResult1;

        std::string creatureName1 = "N/A";
        CreatureProperties const* creatureResult1 = sMySQLStore.getCreatureProperties(atol(creatureId1.c_str()));
        if (creatureResult1)
        {
            creatureName1 = creatureResult1->Name;

            my_query1 = "SELECT id FROM creature_spawns WHERE entry = " + creatureId1 + " AND min_build <= %u AND max_build >= %u";
            QueryResult* spawnResult1 = WorldDatabase.Query(my_query1.c_str(), VERSION_STRING, VERSION_STRING);

            std::string spawnId1;
            if (spawnResult1)
            {
                fields = spawnResult1->Fetch();
                spawnId1 = fields[0].GetString();

                delete spawnResult1;
            }
            else
                spawnId1 = "N/A";

            recout = "|cff00ccffQuest Starter found: creature id, spawnid, name\n\n";
            SendMultilineMessage(m_session, recout.c_str());

            recout = "|cff00ccff";
            recout += creatureId1.c_str();
            recout += ", ";
            recout += spawnId1.c_str();
            recout += ", ";
            recout += creatureName1.c_str();
            recout += "\n\n";
            SendMultilineMessage(m_session, recout.c_str());
        }
        else
        {
            recout = "|cff00ccffNo creature quest starter info found.\n\n";
            SendMultilineMessage(m_session, recout.c_str());
        }

    }
    else
    {
        recout = "|cff00ccffNo creature quest starters found.\n\n";
        SendMultilineMessage(m_session, recout.c_str());
    }

    std::string my_query2 = "SELECT id FROM gameobject_quest_starter WHERE quest = " + std::string(args) + " AND min_build <= %u AND max_build >= %u";
    QueryResult* objectResult2 = WorldDatabase.Query(my_query2.c_str(), VERSION_STRING, VERSION_STRING);

    if (objectResult2)
    {
        Field* fields = objectResult2->Fetch();
        std::string itemId2 = MyConvertIntToString(fields[0].GetUInt32());

        delete objectResult2;

        std::string itemName2 = "N/A";
        ItemProperties const* itemResult2 = sMySQLStore.getItemProperties(atol(itemId2.c_str()));
        if (itemResult2)
        {
            itemName2 = itemResult2->Name;

            my_query2 = "SELECT id FROM gameobject_spawns WHERE entry = " + itemId2 + " min_build <= %u AND max_build >= %u";
            QueryResult* spawnResult2 = WorldDatabase.Query(my_query2.c_str(), VERSION_STRING, VERSION_STRING);

            std::string spawnId2;
            if (spawnResult2)
            {
                fields = spawnResult2->Fetch();
                spawnId2 = fields[0].GetString();

                delete spawnResult2;
            }
            else
                spawnId2 = "N/A";

            recout = "|cff00ccffQuest starter found: object id, spawnid, name\n\n";
            SendMultilineMessage(m_session, recout.c_str());

            recout = "|cff00ccff";
            recout += itemId2.c_str();
            recout += ", ";
            recout += spawnId2.c_str();
            recout += ", ";
            recout += itemName2.c_str();
            recout += "\n\n";
            SendMultilineMessage(m_session, recout.c_str());
        }
        else
        {
            recout = "|cff00ccffNo object quest starter info found.\n\n";
            SendMultilineMessage(m_session, recout.c_str());
        }
    }
    else
    {
        recout = "|cff00ccffNo object quest starters found.\n\n";
        SendMultilineMessage(m_session, recout.c_str());
    }

    return true;
}

bool ChatHandler::HandleQuestListCommand(const char* args, WorldSession* m_session)
{
    uint32 quest_giver = 0;
    if (*args)
        quest_giver = atol(args);
    else
    {
        WoWGuid wowGuid;
        wowGuid.Init(m_session->GetPlayer()->GetSelection());
        if (wowGuid.GetOldGuid() == 0)
        {
            SystemMessage(m_session, "You must target an npc or specify an id.");
            return true;
        }

        Creature* unit = m_session->GetPlayer()->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
        if (unit)
        {
            if (!unit->isQuestGiver())
            {
                SystemMessage(m_session, "Unit is not a valid quest giver.");
                return true;
            }

            if (!unit->HasQuests())
            {
                SystemMessage(m_session, "NPC does not have any quests.");
                return true;
            }

            quest_giver = unit->getEntry();
        }
    }

    std::string recout = "|cff00ff00Quest matches: id: title\n\n";
    SendMultilineMessage(m_session, recout.c_str());

    uint32 count = 0;
    uint32 quest_id = 0;
    QuestProperties const* qst;
    Field* fields;

    if (quest_giver != 0)
    {
        QueryResult* creatureResult = WorldDatabase.Query("SELECT quest FROM creature_quest_starter WHERE id = %u AND min_build <= %u AND max_build >= %u", quest_giver, VERSION_STRING, VERSION_STRING);

        if (!creatureResult)
        {
            recout = "|cff00ccffNo quests found for the specified NPC id.\n\n";
            SendMultilineMessage(m_session, recout.c_str());
            return true;
        }

        do
        {
            fields = creatureResult->Fetch();
            quest_id = fields[0].GetUInt32();

            qst = sMySQLStore.getQuestProperties(quest_id);
            if (qst == NULL)
                continue;

            std::string qid = MyConvertIntToString(quest_id);
            std::string qname = qst->title;

            recout = "|cff00ccff";
            recout += qid.c_str();
            recout += ": ";
            recout += qname;
            recout += "\n";

            SendMultilineMessage(m_session, recout.c_str());

            ++count;

            if (count == 25)
            {
                RedSystemMessage(m_session, "More than 25 results returned. aborting.");
                break;
            }
        }
        while (creatureResult->NextRow());

        delete creatureResult;
    }

    if (count == 0)
    {
        recout = "|cff00ccffNo matches found.\n\n";
        SendMultilineMessage(m_session, recout.c_str());
    }

    return true;
}

bool ChatHandler::HandleQuestAddStartCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->GetSelection());

    if (wowGuid.getGuidLowPart() == 0)
    {
        SystemMessage(m_session, "You must target an npc.");
        return false;
    }

    Creature* unit = m_session->GetPlayer()->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
    if (!unit)
    {
        SystemMessage(m_session, "You must target an npc.");
        return false;
    }

    if (!unit->isQuestGiver())
    {
        SystemMessage(m_session, "Unit is not a valid quest giver.");
        return false;
    }

    uint32 quest_id = atol(args);
    if (quest_id == 0)
    {
        quest_id = GetQuestIDFromLink(args);
        if (quest_id == 0)
            return false;
    }

    QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
    if (qst == nullptr)
    {
        SystemMessage(m_session, "Invalid quest selected, unable to add quest to the specified NPC.");
        return false;
    }

    std::string quest_giver = MyConvertIntToString(unit->getEntry());

    std::string my_query1 = "SELECT id FROM creature_quest_starter WHERE id = " + quest_giver + " AND quest = " + std::string(args) + " AND min_build <= %u AND max_build >= %u";
    QueryResult* selectResult1 = WorldDatabase.Query(my_query1.c_str(), VERSION_STRING, VERSION_STRING);
    if (selectResult1)
    {
        delete selectResult1;
        SystemMessage(m_session, "Quest was already found for the specified NPC.");
    }
    else
    {
        std::string my_insert1 = "INSERT INTO creature_quest_starter (id, quest, min_build, max_build) VALUES (" + quest_giver + "," + std::string(args) + " %u, %u)";
        QueryResult* insertResult1 = WorldDatabase.Query(my_insert1.c_str(), VERSION_STRING, VERSION_STRING);
        if (insertResult1)
            delete insertResult1;
    }

    sQuestMgr.LoadExtraQuestStuff();

    QuestRelation* qstrel = new QuestRelation;
    qstrel->qst = qst;
    qstrel->type = QUESTGIVER_QUEST_START;

    uint8 qstrelid;
    if (unit->HasQuests())
    {
        qstrelid = (uint8)unit->GetQuestRelation(quest_id);
        unit->DeleteQuest(qstrel);
    }

    unit->_LoadQuests();

    std::string qname = qst->title;

    std::string recout = "|cff00ff00Added Quest to NPC as starter: ";
    recout += "|cff00ccff";
    recout += qname;
    recout += "\n\n";
    SendMultilineMessage(m_session, recout.c_str());
    sGMLog.writefromsession(m_session, "added starter of quest %u [%s] to NPC %u [%s]", qst->id, qst->title.c_str(), unit->getEntry(), unit->GetCreatureProperties()->Name.c_str());

    delete qstrel;
    return true;
}

bool ChatHandler::HandleQuestAddFinishCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->GetSelection());

    if (wowGuid.GetOldGuid() == 0)
    {
        SystemMessage(m_session, "You must target an npc.");
        return false;
    }

    Creature* unit = m_session->GetPlayer()->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
    if (!unit)
    {
        SystemMessage(m_session, "You must target an npc.");
        return false;
    }

    if (!unit->isQuestGiver())
    {
        SystemMessage(m_session, "Unit is not a valid quest giver.");
        return false;
    }

    uint32 quest_id = atol(args);
    if (quest_id == 0)
    {
        quest_id = GetQuestIDFromLink(args);
        if (quest_id == 0)
            return false;
    }

    QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
    if (qst == nullptr)
    {
        SystemMessage(m_session, "Invalid quest selected, unable to add quest to the specified NPC.");
        return false;
    }

    std::string quest_giver = MyConvertIntToString(unit->getEntry());

    std::string my_query1 = "SELECT id FROM creature_quest_finisher WHERE id = " + quest_giver + " AND quest = " + std::string(args) + " AND min_build <= %u AND max_build >= %u";
    QueryResult* selectResult1 = WorldDatabase.Query(my_query1.c_str(), VERSION_STRING, VERSION_STRING);
    if (selectResult1)
    {
        delete selectResult1;
        SystemMessage(m_session, "Quest was already found for the specified NPC.");
    }
    else
    {
        std::string my_insert1 = "INSERT INTO creature_quest_finisher (id, quest, min_build, max_build) VALUES (" + quest_giver + "," + std::string(args) + ", %u, %u)";
        QueryResult* insertResult1 = WorldDatabase.Query(my_insert1.c_str(), VERSION_STRING, VERSION_STRING);
        if (insertResult1)
            delete insertResult1;
    }

    sQuestMgr.LoadExtraQuestStuff();

    QuestRelation* qstrel = new QuestRelation;
    qstrel->qst = qst;
    qstrel->type = QUESTGIVER_QUEST_END;

    uint8 qstrelid;
    if (unit->HasQuests())
    {
        qstrelid = (uint8)unit->GetQuestRelation(quest_id);
        unit->DeleteQuest(qstrel);
    }

    unit->_LoadQuests();

    std::string qname = qst->title;

    std::string recout = "|cff00ff00Added Quest to NPC as finisher: ";
    recout += "|cff00ccff";
    recout += qname;
    recout += "\n\n";
    SendMultilineMessage(m_session, recout.c_str());
    sGMLog.writefromsession(m_session, "added finisher of quest %u [%s] to NPC %u [%s]", qst->id, qst->title.c_str(), unit->getEntry(), unit->GetCreatureProperties()->Name.c_str());

    delete qstrel;
    return true;
}

bool ChatHandler::HandleQuestAddBothCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    bool bValid = ChatHandler::HandleQuestAddStartCommand(args, m_session);

    if (bValid)
        ChatHandler::HandleQuestAddFinishCommand(args, m_session);

    return true;
}

bool ChatHandler::HandleQuestDelStartCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->GetSelection());

    if (wowGuid.GetOldGuid() == 0)
    {
        SystemMessage(m_session, "You must target an npc.");
        return false;
    }

    Creature* unit = m_session->GetPlayer()->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
    if (!unit)
    {
        SystemMessage(m_session, "You must target an npc.");
        return false;
    }

    if (!unit->isQuestGiver())
    {
        SystemMessage(m_session, "Unit is not a valid quest giver.");
        return false;
    }

    uint32 quest_id = atol(args);
    if (quest_id == 0)
    {
        quest_id = GetQuestIDFromLink(args);
        if (quest_id == 0)
            return false;
    }

    QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
    if (qst == nullptr)
    {
        SystemMessage(m_session, "Invalid Quest selected.");
        return false;
    }

    std::string quest_giver = MyConvertIntToString(unit->getEntry());

    std::string my_query1 = "SELECT id FROM creature_quest_starter WHERE id = " + quest_giver + " AND quest = " + std::string(args) + " AND min_build <= %u AND max_build >= %u";
    QueryResult* selectResult1 = WorldDatabase.Query(my_query1.c_str(), VERSION_STRING, VERSION_STRING);
    if (selectResult1)
        delete selectResult1;
    else
    {
        SystemMessage(m_session, "Quest was NOT found for the specified NPC.");
        return false;
    }

    std::string my_delete1 = "DELETE FROM creature_quest_starter WHERE id = " + quest_giver + " AND quest = " + std::string(args) + " AND min_build <= %u AND max_build >= %u";
    QueryResult* deleteResult1 = WorldDatabase.Query(my_delete1.c_str(), VERSION_STRING, VERSION_STRING);
    if (deleteResult1)
        delete deleteResult1;

    sQuestMgr.LoadExtraQuestStuff();

    QuestRelation* qstrel = new QuestRelation;
    qstrel->qst = qst;
    qstrel->type = QUESTGIVER_QUEST_START;

    uint8 qstrelid;
    if (unit->HasQuests())
    {
        qstrelid = (uint8)unit->GetQuestRelation(quest_id);
        unit->DeleteQuest(qstrel);
    }
    unit->_LoadQuests();

    std::string qname = qst->title;

    std::string recout = "|cff00ff00Deleted Quest from NPC: ";
    recout += "|cff00ccff";
    recout += qname;
    recout += "\n\n";
    SendMultilineMessage(m_session, recout.c_str());
    sGMLog.writefromsession(m_session, "deleted starter of quest %u [%s] to NPC %u [%s]", qst->id, qst->title.c_str(), unit->getEntry(), unit->GetCreatureProperties()->Name.c_str());

    delete qstrel;
    return true;
}

bool ChatHandler::HandleQuestDelFinishCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->GetSelection());
    if (wowGuid.getGuidLowPart() == 0)
    {
        SystemMessage(m_session, "You must target an npc.");
        return false;
    }

    Creature* unit = m_session->GetPlayer()->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
    if (!unit)
    {
        SystemMessage(m_session, "You must target an npc.");
        return false;
    }

    if (!unit->isQuestGiver())
    {
        SystemMessage(m_session, "Unit is not a valid quest giver.");
        return false;
    }

    uint32 quest_id = atol(args);
    if (quest_id == 0)
    {
        quest_id = GetQuestIDFromLink(args);
        if (quest_id == 0)
            return false;
    }

    QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
    if (qst == nullptr)
    {
        SystemMessage(m_session, "Invalid Quest selected.");
        return false;
    }

    std::string quest_giver = MyConvertIntToString(unit->getEntry());

    std::string my_query1 = "SELECT id FROM creature_quest_finisher WHERE id = " + quest_giver + " AND quest = " + std::string(args) + " AND min_build <= %u AND max_build >= %u";
    QueryResult* selectResult1 = WorldDatabase.Query(my_query1.c_str(), VERSION_STRING, VERSION_STRING);
    if (selectResult1)
        delete selectResult1;
    else
    {
        SystemMessage(m_session, "Quest was NOT found for the specified NPC.");
        return true;
    }

    std::string my_delete1 = "DELETE FROM creature_quest_finisher WHERE id = " + quest_giver + " AND quest = " + std::string(args) + " AND min_build <= %u AND max_build >= %u";
    QueryResult* deleteResult1 = WorldDatabase.Query(my_delete1.c_str(), VERSION_STRING, VERSION_STRING);
    if (deleteResult1)
        delete deleteResult1;

    sQuestMgr.LoadExtraQuestStuff();

    QuestRelation* qstrel = new QuestRelation;
    qstrel->qst = qst;
    qstrel->type = QUESTGIVER_QUEST_END;

    uint8 qstrelid;
    if (unit->HasQuests())
    {
        qstrelid = (uint8)unit->GetQuestRelation(quest_id);
        unit->DeleteQuest(qstrel);
    }

    unit->_LoadQuests();

    std::string qname = qst->title;

    std::string recout = "|cff00ff00Deleted Quest from NPC: ";
    recout += "|cff00ccff";
    recout += qname;
    recout += "\n\n";
    SendMultilineMessage(m_session, recout.c_str());
    sGMLog.writefromsession(m_session, "deleted finisher of quest %u [%s] to NPC %u [%s]", qst->id, qst->title.c_str(), unit->getEntry(), unit->GetCreatureProperties()->Name.c_str());

    delete qstrel;
    return true;
}

bool ChatHandler::HandleQuestDelBothCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    bool bValid = ChatHandler::HandleQuestDelStartCommand(args, m_session);

    if (bValid)
        ChatHandler::HandleQuestDelFinishCommand(args, m_session);

    return true;
}

bool ChatHandler::HandleQuestFinisherCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string recout;

    std::string my_query1 = "SELECT id FROM creature_quest_finisher WHERE quest = " + std::string(args) + " AND min_build <= %u AND max_build >= %u";
    QueryResult* objectResult1 = WorldDatabase.Query(my_query1.c_str(), VERSION_STRING, VERSION_STRING);

    if (objectResult1)
    {
        Field* fields = objectResult1->Fetch();
        std::string creatureId1 = MyConvertIntToString(fields[0].GetUInt32());

        delete objectResult1;

        std::string creatureName1 = "N/A";
        CreatureProperties const* creatureResult1 = sMySQLStore.getCreatureProperties(atol(creatureId1.c_str()));

        if (creatureResult1)
        {
            creatureName1 = creatureResult1->Name;

            my_query1 = "SELECT id FROM creature_spawns WHERE entry = " + creatureId1 + " AND min_build <= %u AND max_build >= %u";
            QueryResult* spawnResult1 = WorldDatabase.Query(my_query1.c_str(), VERSION_STRING, VERSION_STRING);

            std::string spawnId1;
            if (spawnResult1)
            {
                fields = spawnResult1->Fetch();
                spawnId1 = fields[0].GetString();

                delete spawnResult1;
            }
            else
                spawnId1 = "N/A";

            recout = "|cff00ccffQuest Finisher found: creature id, spawnid, name\n\n";
            SendMultilineMessage(m_session, recout.c_str());

            recout = "|cff00ccff";
            recout += creatureId1.c_str();
            recout += ", ";
            recout += spawnId1.c_str();
            recout += ", ";
            recout += creatureName1.c_str();
            recout += "\n\n";
            SendMultilineMessage(m_session, recout.c_str());
        }
        else
        {
            recout = "|cff00ccffNo creature quest finisher info found.\n\n";
            SendMultilineMessage(m_session, recout.c_str());
        }
    }
    else
    {
        recout = "|cff00ccffNo creature quest finishers found.\n\n";
        SendMultilineMessage(m_session, recout.c_str());
    }

    std::string my_query2 = "SELECT id FROM gameobject_quest_finisher WHERE quest = " + std::string(args) + " AND min_build <= %u AND max_build >= %u";
    QueryResult* objectResult2 = WorldDatabase.Query(my_query2.c_str(), VERSION_STRING, VERSION_STRING);

    if (objectResult2)
    {
        Field* fields = objectResult2->Fetch();
        std::string itemId2 = MyConvertIntToString(fields[0].GetUInt32());

        delete objectResult2;

        std::string itemName2 = "N/A";
        ItemProperties const* itemResult2 = sMySQLStore.getItemProperties(atol(itemId2.c_str()));
        if (itemResult2)
        {
            itemName2 = itemResult2->Name;

            my_query2 = "SELECT id FROM gameobject_spawns WHERE entry = " + itemId2 + " min_build <= %u AND max_build >= %u";
            QueryResult* spawnResult2 = WorldDatabase.Query(my_query2.c_str(), VERSION_STRING, VERSION_STRING);

            std::string spawnId2;
            if (spawnResult2)
            {
                fields = spawnResult2->Fetch();
                spawnId2 = fields[0].GetString();

                delete spawnResult2;
            }
            else
                spawnId2 = "N/A";

            recout = "|cff00ccffQuest Finisher found: object id, spawnid, name\n\n";
            SendMultilineMessage(m_session, recout.c_str());

            recout = "|cff00ccff";
            recout += itemId2.c_str();
            recout += ", ";
            recout += spawnId2.c_str();
            recout += ", ";
            recout += itemName2.c_str();
            recout += "\n\n";
            SendMultilineMessage(m_session, recout.c_str());
        }
        else
        {
            recout = "|cff00ccffNo object quest finisher info found.\n\n";
            SendMultilineMessage(m_session, recout.c_str());
        }
    }
    else
    {
        recout = "|cff00ccffNo object quest finishers found.\n\n";
        SendMultilineMessage(m_session, recout.c_str());
    }

    return true;
}

bool ChatHandler::HandleQuestStarterSpawnCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string recout;

    std::string my_query = "SELECT id FROM creature_quest_starter WHERE quest = " + std::string(args) + " AND min_build <= %u AND max_build >= %u";
    QueryResult* objectResult = WorldDatabase.Query(my_query.c_str(), VERSION_STRING, VERSION_STRING);

    std::string starterId;
    if (objectResult)
    {
        Field* fields = objectResult->Fetch();
        starterId = MyConvertIntToString(fields[0].GetUInt32());
    }
    else
    {
        recout = "|cff00ccffNo quest starter NPCs found.\n\n";
        SendMultilineMessage(m_session, recout.c_str());
        return true;
    }

    delete objectResult;

    std::string starterName = "N/A";
    CreatureProperties const* creatureResult = sMySQLStore.getCreatureProperties(atol(starterId.c_str()));
    if (creatureResult)
    {
        starterName = creatureResult->Name;
    }
    else
    {
        recout = "|cff00ccffNo quest starter info found.\n\n";
        SendMultilineMessage(m_session, recout.c_str());
        return true;
    }

    my_query = "SELECT map, position_x, position_y, position_z FROM creature_spawns WHERE entry = " + starterId + " AND min_build <= %u AND max_build >= %u";
    QueryResult* spawnResult = WorldDatabase.Query(my_query.c_str(), VERSION_STRING, VERSION_STRING);

    if (!spawnResult)
    {
        recout = "|cff00ccffNo spawn location for quest starter was found.\n\n";
        SendMultilineMessage(m_session, recout.c_str());
        return true;
    }

    Field* fields = spawnResult->Fetch();
    uint32 locmap = fields[0].GetUInt32();
    float x = fields[1].GetFloat();
    float y = fields[2].GetFloat();
    float z = fields[3].GetFloat();

    delete spawnResult;

    recout = "|cff00ccffPorting to Quest Starter/Giver: id, name\n\n";
    SendMultilineMessage(m_session, recout.c_str());

    recout = "|cff00ccff";
    recout += starterId.c_str();
    recout += ", ";
    recout += starterName.c_str();
    recout += "\n\n";
    SendMultilineMessage(m_session, recout.c_str());

    m_session->GetPlayer()->SafeTeleport(locmap, 0, LocationVector(x, y, z));

    return true;
}

bool ChatHandler::HandleQuestFinisherSpawnCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string recout;

    std::string my_query = "SELECT id FROM creature_quest_finisher WHERE quest = " + std::string(args) + " AND min_build <= %u AND max_build >= %u";
    QueryResult* objectResult = WorldDatabase.Query(my_query.c_str(), VERSION_STRING, VERSION_STRING);

    std::string finisherId;
    if (objectResult)
    {
        Field* fields = objectResult->Fetch();
        finisherId = MyConvertIntToString(fields[0].GetUInt32());
    }
    else
    {
        recout = "|cff00ccffNo quest finisher NPCs found.\n\n";
        SendMultilineMessage(m_session, recout.c_str());
        return true;
    }

    delete objectResult;

    std::string finisherName = "N/A";
    CreatureProperties const* creatureResult = sMySQLStore.getCreatureProperties(atol(finisherId.c_str()));
    if (creatureResult)
    {
        finisherName = creatureResult->Name;
    }
    else
    {
        recout = "|cff00ccffNo quest finisher info found.\n\n";
        SendMultilineMessage(m_session, recout.c_str());
        return true;
    }

    my_query = "SELECT map, position_x, position_y, position_z FROM creature_spawns WHERE entry = " + finisherId + " AND min_build <= %u AND max_build >= %u";
    QueryResult* spawnResult = WorldDatabase.Query(my_query.c_str(), VERSION_STRING, VERSION_STRING);

    if (!spawnResult)
    {
        recout = "|cff00ccffNo spawn location for quest finisher was found.\n\n";
        SendMultilineMessage(m_session, recout.c_str());
        return true;
    }

    Field* fields = spawnResult->Fetch();
    uint32 locmap = fields[0].GetUInt32();
    float x = fields[1].GetFloat();
    float y = fields[2].GetFloat();
    float z = fields[3].GetFloat();

    delete spawnResult;

    recout = "|cff00ccffPorting to Quest Finisher: id, name\n\n";
    SendMultilineMessage(m_session, recout.c_str());

    recout = "|cff00ccff";
    recout += finisherId.c_str();
    recout += ", ";
    recout += finisherName.c_str();
    recout += "\n\n";
    SendMultilineMessage(m_session, recout.c_str());

    m_session->GetPlayer()->SafeTeleport(locmap, 0, LocationVector(x, y, z));

    return true;
}

bool ChatHandler::HandleQuestLoadCommand(const char* /*args*/, WorldSession* m_session)
{
    BlueSystemMessage(m_session, "Load of quests from the database has been initiated ...", "");
    auto startTime = Util::TimeNow();

    sQuestMgr.LoadExtraQuestStuff();

    BlueSystemMessage(m_session, "Load completed in %u ms.", Util::GetTimeDifferenceToNow(startTime));

    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->GetSelection());

    if (wowGuid.GetOldGuid() == 0)
        return true;

    Creature* unit = m_session->GetPlayer()->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
    if (!unit)
        return true;

    if (!unit->isQuestGiver())
        return true;

    // If player targeted a questgiver assume they want the NPC reloaded, too
    unit->_LoadQuests();

    return true;
}

bool ChatHandler::HandleQuestRemoveCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (plr == nullptr)
        return true;

    std::string recout = "";
    uint32 quest_id = atol(args);
    if (quest_id == 0)
    {
        quest_id = GetQuestIDFromLink(args);
        if (quest_id == 0)
            return false;
    }

    QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
    if (qst)
    {
        recout = RemoveQuestFromPlayer(plr, qst);
        sGMLog.writefromsession(m_session, "removed quest %u [%s] from player %s%", qst->id, qst->title.c_str(), plr->getName().c_str());
    }
    else
        recout = "Invalid quest selected, unable to remove.\n\n";

    SystemMessage(m_session, recout.c_str());

    return true;
}

bool ChatHandler::HandleQuestRewardCommand(const char* args, WorldSession* m_session)
{
    if (!*args) return false;

    std::stringstream recout;

    uint32 qu_id = atol(args);
    if (qu_id == 0)
    {
        qu_id = GetQuestIDFromLink(args);
        if (qu_id == 0)
            return false;
    }

    QuestProperties const* q = sMySQLStore.getQuestProperties(qu_id);
    if (q)
    {
        for (uint32 r = 0; r < q->count_reward_item; ++r)
        {
            uint32 itemid = q->reward_item[r];
            ItemProperties const* itemProto = sMySQLStore.getItemProperties(itemid);
            if (!itemProto)
            {
                recout << "Unknown item id %lu" << itemid;
                LOG_ERROR("WORLD: Unknown item id 0x%08x", itemid);
            }
            else
            {
                recout << "Reward (" << itemid << "): " << GetItemLinkByProto(itemProto, m_session->language);
                if (q->reward_itemcount[r] == 1)
                    recout << "\n";
                else
                    recout << "[x" << q->reward_itemcount[r] << "]\n";
            }
        }
        for (uint32 r = 0; r < q->count_reward_choiceitem; ++r)
        {
            uint32 itemid = q->reward_choiceitem[r];
            ItemProperties const* itemProto = sMySQLStore.getItemProperties(itemid);
            if (!itemProto)
            {
                recout << "Unknown item id %lu" << itemid;
                LOG_ERROR("WORLD: Unknown item id 0x%08x", itemid);
            }
            else
            {
                recout << "Reward choice (" << itemid << "): " << GetItemLinkByProto(itemProto, m_session->language);
                if (q->reward_choiceitemcount[r] == 1)
                    recout << "\n";
                else
                    recout << "[x" << q->reward_choiceitemcount[r] << "]\n";
            }
        }
        if ((q->count_reward_choiceitem == 0) && (q->count_reward_item == 0))
        {
            recout << "Quest " << qu_id << " has no item rewards.";
        }
    }
    else
    {
        recout << "Quest ID " << qu_id << " not found.\n";
        LOG_ERROR("Quest ID %lu not found.", qu_id);
    }

    SendMultilineMessage(m_session, recout.str().data());

    return true;
}
