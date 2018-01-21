/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Map/WorldCreatorDefines.hpp"

void WorldSession::HandleQuestPOIQueryOpcode(WorldPacket& recvData)
{
    uint32_t count = 0;
    recvData >> count;

    if (count > MAX_QUEST_LOG_SIZE)
    {
        LOG_DEBUG("Client sent Quest POI query for more than MAX_QUEST_LOG_SIZE quests.");

        count = MAX_QUEST_LOG_SIZE;
    }

    WorldPacket data(SMSG_QUEST_POI_QUERY_RESPONSE, 4 + (4 + 4) * count);
    data << uint32_t(count);

    for (uint32_t i = 0; i < count; ++i)
    {
        uint32_t questId;
        recvData >> questId;

        sQuestMgr.BuildQuestPOIResponse(data, questId);
    }

    SendPacket(&data);
}

WorldPacket* WorldSession::BuildQuestQueryResponse(QuestProperties const* qst)
{
    WorldPacket* data = new WorldPacket(SMSG_QUEST_QUERY_RESPONSE, 100);
    MySQLStructure::LocalesQuest const* lci = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;

    *data << uint32_t(qst->id);                                        // Quest ID
    *data << uint32_t(2);                                              // Unknown, always seems to be 2
    *data << int32_t(qst->questlevel);                                 // Quest level
    *data << uint32_t(qst->min_level);                                 // Quest required level

    if (qst->quest_sort > 0)
    {
        *data << int32_t(-(int32_t)qst->quest_sort);                     // Negative if pointing to a sort.
    }
    else
    {
        *data << uint32_t(qst->zone_id);                               // Positive if pointing to a zone.
    }

    *data << uint32_t(qst->type);                                      // Info ID / Type
    *data << qst->suggestedplayers;                                  // suggested players

    *data << uint32_t(qst->required_rep_faction);                      // Faction ID
    *data << uint32_t(qst->required_rep_value);                        // Faction Amount

    *data << uint32_t(0);                                              // Unknown (always 0)
    *data << uint32_t(0);                                              // Unknown (always 0)

    *data << uint32_t(qst->next_quest_id);                             // Next Quest ID
    *data << uint32_t(0);                                              // Column id +1 from QuestXp.dbc, entry is quest level

    *data << uint32_t(sQuestMgr.GenerateRewardMoney(_player, qst));    // Copper reward
    *data << uint32_t(qst->reward_money < 0 ? -qst->reward_money : 0); // Required Money

    *data << uint32_t(qst->reward_spell);                              // Spell added to spellbook upon completion
    *data << int32_t(qst->effect_on_player);                           // Spell casted on player upon completion

    *data << uint32_t(qst->bonushonor);
    *data << float(0);                                               // 3.3.0 - some multiplier for honor
    *data << uint32_t(qst->srcitem);                                   // Item given at the start of a quest (srcitem)
    *data << uint32_t(qst->quest_flags);                               // Quest Flags
    *data << uint32_t(0);                                              // target minimap
    *data << uint32_t(qst->rewardtitleid);
    *data << uint32_t(0);                                              // playerkillcount
    *data << uint32_t(qst->rewardtalents);
    *data << uint32_t(0);                                              // arena points
    *data << uint32_t(0);                                              // reward skill id
    *data << uint32_t(0);                                              // reward skill points
    *data << uint32_t(0);                                              // rep mask (unsure on what it does)
    *data << uint32_t(0);                                              // quest giver entry ?
    *data << uint32_t(0);                                              // quest turnin entry ?

    if (qst->HasFlag(QUEST_FLAGS_HIDDEN_REWARDS))
    {
        for (uint8_t i = 0; i < 4; ++i)
        {
            *data << uint32_t(0);
            *data << uint32_t(0);
        }

        for (uint8_t i = 0; i < 6; ++i)
        {
            *data << uint32_t(0);
            *data << uint32_t(0);
        }
    }
    else
    {
        for (uint8_t i = 0; i < 4; ++i)
        {
            *data << uint32_t(qst->reward_item[i]);
            *data << uint32_t(qst->reward_itemcount[i]);
        }
        for (uint8_t i = 0; i < 6; ++i)
        {
            *data << uint32_t(qst->reward_choiceitem[i]);
            *data << uint32_t(qst->reward_choiceitemcount[i]);
        }
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << uint32_t(qst->reward_repfaction[i]);
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << uint32_t(0);                                          // column index in QuestFactionReward.dbc but use unknown
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << uint32_t(0);                                          // unk
    }

    *data << qst->point_mapid;
    *data << qst->point_x;
    *data << qst->point_y;
    *data << qst->point_opt;

    *data << (lci ? lci->title : qst->title);
    *data << (lci ? lci->objectives : qst->objectives);
    *data << (lci ? lci->details : qst->details);
    *data << (lci ? lci->endText : qst->endtext);
    *data << "";                                                     // completed text

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(qst->required_mob_or_go[i]);                 // Kill mob entry ID [i]
        *data << uint32_t(qst->required_mob_or_go_count[i]);           // Kill mob count [i]
        *data << uint32_t(0);                                          // req src item
        *data << uint32_t(0);                                          // req src item count
    }

    for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        *data << uint32_t(qst->required_item[i]);                      // Collect item [i]
        *data << uint32_t(qst->required_itemcount[i]);                 // Collect item count [i]
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << (lci ? lci->objectiveText[i] : qst->objectivetexts[i]);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(0);                                          // currency id
        *data << uint32_t(0);                                          // currency count
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(0);                                          // req currency id
        *data << uint32_t(0);                                          // req currency count
    }

    *data << "";                                                     //questGiverTextWindow;
    *data << "";                                                     //questGiverTargetName;
    *data << "";                                                     //questTurnTextWindow;
    *data << "";                                                     //questTurnTargetName;
    *data << uint32_t(0);                                              // accept sound?
    *data << uint32_t(0);                                              // sound turn in?

    return data;
}

//\todo Rewrite for cata - after this all functions are copied from wotlk

void WorldSession::HandleQuestgiverStatusQueryOpcode(WorldPacket& recvData)
{
    if (_player->IsInBg())
        return;         //Added in 3.0.2, quests can be shared anywhere besides a BG

    uint64 guid;
    WorldPacket data(SMSG_QUESTGIVER_STATUS, 12);
    Object* qst_giver = nullptr;

    recvData >> guid;
    uint32 guidtype = GET_TYPE_FROM_GUID(guid);
    if (guidtype == HIGHGUID_TYPE_UNIT)
    {
        Creature* quest_giver = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        if (!quest_giver->isQuestGiver())
        {
            LOG_DEBUG("WORLD: Creature is not a questgiver.");
            return;
        }
    }
    else if (guidtype == HIGHGUID_TYPE_ITEM)
    {
        Item* quest_giver = GetPlayer()->GetItemInterface()->GetItemByGUID(guid);
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;
    }
    else if (guidtype == HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;
    }

    if (!qst_giver)
    {
        LOG_DEBUG("WORLD: Invalid questgiver GUID " I64FMT ".", guid);
        return;
    }

    data << guid;
    data << sQuestMgr.CalcStatus(qst_giver, GetPlayer());
    SendPacket(&data);
}

void WorldSession::HandleQuestgiverHelloOpcode(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;

    Creature* qst_giver = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!qst_giver)
    {
        LOG_DEBUG("WORLD: Invalid questgiver GUID.");
        return;
    }

    if (!qst_giver->isQuestGiver())
    {
        LOG_DEBUG("WORLD: Creature is not a questgiver.");
        return;
    }

    /*if (qst_giver->GetAIInterface()) // NPC Stops moving for 3 minutes
    qst_giver->GetAIInterface()->StopMovement(180000);*/

    //qst_giver->Emote(EMOTE_ONESHOT_TALK); // this doesn't work
    sQuestMgr.OnActivateQuestGiver(qst_giver, GetPlayer());
}

void WorldSession::HandleQuestGiverQueryQuestOpcode(WorldPacket& recvData)
{
    WorldPacket data;
    uint64 guid;
    uint32 quest_id;
    uint32 status = 0;
    uint8 unk;

    recvData >> guid;
    recvData >> quest_id;
    recvData >> unk;

    Object* qst_giver = nullptr;

    bool bValid = false;

    QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
    if (!qst)
    {
        LOG_DEBUG("WORLD: Invalid quest ID.");
        return;
    }

    uint32 guidtype = GET_TYPE_FROM_GUID(guid);
    if (guidtype == HIGHGUID_TYPE_UNIT)
    {
        Creature* quest_giver = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;
        if (quest_giver->isQuestGiver())
        {
            bValid = true;
            status = sQuestMgr.CalcQuestStatus(qst_giver, GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id), false);
        }
    }
    else if (guidtype == HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;
        bValid = false;
        if (quest_giver->GetType() == GAMEOBJECT_TYPE_QUESTGIVER)
        {
            bValid = true;
            GameObject_QuestGiver* go_quest_giver = static_cast<GameObject_QuestGiver*>(quest_giver);
            status = sQuestMgr.CalcQuestStatus(qst_giver, GetPlayer(), qst, (uint8)go_quest_giver->GetQuestRelation(qst->id), false);
        }
    }
    else if (guidtype == HIGHGUID_TYPE_ITEM)
    {
        Item* quest_giver = GetPlayer()->GetItemInterface()->GetItemByGUID(guid);
        //added it for script engine
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        ItemProperties const* itemProto = quest_giver->GetItemProperties();

        if (itemProto->Bonding != ITEM_BIND_ON_USE || quest_giver->IsSoulbound())     // SoulBind item will be used after SoulBind()
        {
            if (sScriptMgr.CallScriptedItem(quest_giver, GetPlayer()))
                return;
        }

        if (itemProto->Bonding == ITEM_BIND_ON_USE)
            quest_giver->SoulBind();

        bValid = true;
        status = sQuestMgr.CalcQuestStatus(qst_giver, GetPlayer(), qst, 1, false);
    }

    if (!qst_giver)
    {
        LOG_DEBUG("WORLD: Invalid questgiver GUID.");
        return;
    }

    if (!bValid)
    {
        LOG_DEBUG("WORLD: object is not a questgiver.");
        return;
    }

    if ((status == QMGR_QUEST_AVAILABLE) || (status == QMGR_QUEST_REPEATABLE) || (status == QMGR_QUEST_CHAT))
    {
        sQuestMgr.BuildQuestDetails(&data, qst, qst_giver, 1, language, _player);	 // 0 because we want goodbye to function
        SendPacket(&data);
        LOG_DEBUG("WORLD: Sent SMSG_QUESTGIVER_QUEST_DETAILS.");

        if (qst->HasFlag(QUEST_FLAGS_AUTO_ACCEPT))
            _player->AcceptQuest(qst_giver->GetGUID(), qst->id);
    }
    else if (status == QMGR_QUEST_NOT_FINISHED || status == QMGR_QUEST_FINISHED)
    {
        sQuestMgr.BuildRequestItems(&data, qst, qst_giver, status, language);
        SendPacket(&data);
        LOG_DEBUG("WORLD: Sent SMSG_QUESTGIVER_REQUEST_ITEMS.");
    }
}

void WorldSession::HandleQuestgiverAcceptQuestOpcode(WorldPacket& recvData)
{
    uint64 guid;
    uint32 quest_id;

    recvData >> guid;
    recvData >> quest_id;

    _player->AcceptQuest(guid, quest_id);
}

void WorldSession::HandleQuestgiverCancelOpcode(WorldPacket& /*recvPacket*/)
{
    OutPacket(SMSG_GOSSIP_COMPLETE, 0, nullptr);

    LOG_DEBUG("WORLD: Sent SMSG_GOSSIP_COMPLETE");
}

void WorldSession::HandleQuestlogRemoveQuestOpcode(WorldPacket& recvPacket)
{
    uint8 quest_slot;
    recvPacket >> quest_slot;
    if (quest_slot >= 25)
        return;

    QuestLogEntry* qEntry = GetPlayer()->GetQuestLogInSlot(quest_slot);
    if (!qEntry)
    {
        LOG_DEBUG("WORLD: No quest in slot %d.", quest_slot);
        return;
    }
    QuestProperties const* qPtr = qEntry->GetQuest();
    CALL_QUESTSCRIPT_EVENT(qEntry, OnQuestCancel)(GetPlayer());
    qEntry->Finish();

    // Remove all items given by the questgiver at the beginning
    for (uint8 i = 0; i < 4; ++i)
    {
        if (qPtr->receive_items[i])
            GetPlayer()->GetItemInterface()->RemoveItemAmt(qPtr->receive_items[i], 1);
    }

    if (qPtr->srcitem && qPtr->srcitem != qPtr->receive_items[0])
    {
        ItemProperties const* itemProto = sMySQLStore.getItemProperties(qPtr->srcitem);
        if (itemProto != nullptr)
            if (itemProto->QuestId != qPtr->id)
                _player->GetItemInterface()->RemoveItemAmt(qPtr->srcitem, qPtr->srcitemcount ? qPtr->srcitemcount : 1);
    }
    //remove all quest items (but not trade goods) collected and required only by this quest
    for (uint8 i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        if (qPtr->required_item[i] != 0)
        {
            ItemProperties const* itemProto = sMySQLStore.getItemProperties(qPtr->required_item[i]);
            if (itemProto != nullptr && itemProto->Class == ITEM_CLASS_QUEST)
                GetPlayer()->GetItemInterface()->RemoveItemAmt(qPtr->required_item[i], qPtr->required_itemcount[i]);
        }
    }

    GetPlayer()->UpdateNearbyGameObjects();

    sHookInterface.OnQuestCancelled(_player, qPtr);
}

void WorldSession::HandleQuestQueryOpcode(WorldPacket& recvData)
{
    uint32 quest_id;
    recvData >> quest_id;

    QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
    if (!qst)
    {
        LOG_DEBUG("WORLD: Invalid quest ID.");
        return;
    }

    WorldPacket* pkt = BuildQuestQueryResponse(qst);
    SendPacket(pkt);
    delete pkt;
}

void WorldSession::HandleQuestgiverRequestRewardOpcode(WorldPacket& recvData)
{
    uint64 guid;
    uint32 quest_id;

    recvData >> guid;
    recvData >> quest_id;

    bool bValid = false;
    QuestProperties const* qst = nullptr;
    Object* qst_giver = nullptr;
    uint32 status = 0;
    uint32 guidtype = GET_TYPE_FROM_GUID(guid);

    if (guidtype == HIGHGUID_TYPE_UNIT)
    {
        Creature* quest_giver = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;
        if (quest_giver->isQuestGiver())
        {
            bValid = true;
            qst = quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_END);
            if (!qst)
                qst = quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_START);

            if (!qst)
            {
                LOG_ERROR("WARNING: Cannot get reward for quest %u, as it doesn't exist at Unit %u.", quest_id, quest_giver->GetEntry());
                return;
            }
            status = sQuestMgr.CalcQuestStatus(qst_giver, GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id), false);
        }
    }
    else if (guidtype == HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return; // oops..
        bValid = false;
        if (quest_giver->GetType() == GAMEOBJECT_TYPE_QUESTGIVER)
        {
            bValid = true;
            GameObject_QuestGiver* go_quest_giver = static_cast<GameObject_QuestGiver*>(quest_giver);
            qst = go_quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_END);
            if (!qst)
            {
                LOG_ERROR("WARNING: Cannot get reward for quest %u, as it doesn't exist at GO %u.", quest_id, quest_giver->GetEntry());
                return;
            }
            status = sQuestMgr.CalcQuestStatus(qst_giver, GetPlayer(), qst, (uint8)go_quest_giver->GetQuestRelation(qst->id), false);
        }
    }

    if (!qst_giver)
    {
        LOG_DEBUG("WORLD: Invalid questgiver GUID.");
        return;
    }

    if (!bValid || qst == nullptr)
    {
        LOG_DEBUG("WORLD: Creature is not a questgiver.");
        return;
    }

    if (status == QMGR_QUEST_FINISHED)
    {
        WorldPacket data;
        sQuestMgr.BuildOfferReward(&data, qst, qst_giver, 1, language, _player);
        SendPacket(&data);
        LOG_DEBUG("WORLD: Sent SMSG_QUESTGIVER_REQUEST_ITEMS.");
    }

    // if we got here it means we're cheating
}

void WorldSession::HandleQuestgiverCompleteQuestOpcode(WorldPacket& recvPacket)
{
    uint64 guid;
    uint32 quest_id;

    recvPacket >> guid;
    recvPacket >> quest_id;

    bool bValid = false;
    QuestProperties const* qst = nullptr;
    Object* qst_giver = nullptr;
    uint32 status = 0;
    uint32 guidtype = GET_TYPE_FROM_GUID(guid);

    if (guidtype == HIGHGUID_TYPE_UNIT)
    {
        Creature* quest_giver = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;
        if (quest_giver->isQuestGiver())
        {
            bValid = true;
            qst = quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_END);
            if (!qst)
            {
                LOG_ERROR("WARNING: Cannot complete quest %u, as it doesn't exist at Unit %u.", quest_id, quest_giver->GetEntry());
                return;
            }
            status = sQuestMgr.CalcQuestStatus(qst_giver, GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id), false);
        }
    }
    else if (guidtype == HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return; // oops..
        bValid = false;
        if (quest_giver->GetType() == GAMEOBJECT_TYPE_QUESTGIVER)
        {
            GameObject_QuestGiver* go_quest_giver = static_cast<GameObject_QuestGiver*>(quest_giver);
            qst = go_quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_END);
            if (!qst)
            {
                LOG_ERROR("WARNING: Cannot complete quest %u, as it doesn't exist at GO %u.", quest_id, quest_giver->GetEntry());
                return;
            }
            status = sQuestMgr.CalcQuestStatus(qst_giver, GetPlayer(), qst, (uint8)go_quest_giver->GetQuestRelation(qst->id), false);
        }
    }

    if (!qst_giver)
    {
        LOG_DEBUG("WORLD: Invalid questgiver GUID.");
        return;
    }

    if (!bValid || qst == nullptr)
    {
        LOG_DEBUG("WORLD: Creature is not a questgiver.");
        return;
    }

    if (status == QMGR_QUEST_NOT_FINISHED || status == QMGR_QUEST_REPEATABLE)
    {
        WorldPacket data;
        sQuestMgr.BuildRequestItems(&data, qst, qst_giver, status, language);
        SendPacket(&data);
        LOG_DEBUG("WORLD: Sent SMSG_QUESTGIVER_REQUEST_ITEMS.");
    }

    if (status == QMGR_QUEST_FINISHED)
    {
        WorldPacket data;
        sQuestMgr.BuildOfferReward(&data, qst, qst_giver, 1, language, _player);
        SendPacket(&data);
        LOG_DEBUG("WORLD: Sent SMSG_QUESTGIVER_REQUEST_ITEMS.");
    }

    sHookInterface.OnQuestFinished(_player, qst, qst_giver);
}

void WorldSession::HandleQuestgiverChooseRewardOpcode(WorldPacket& recvPacket)
{
    uint64 guid;
    uint32 quest_id;
    uint32 reward_slot;

    recvPacket >> guid;
    recvPacket >> quest_id;
    recvPacket >> reward_slot;

    if (reward_slot >= 6)
        return;

    bool bValid = false;
    QuestProperties const* qst = nullptr;
    Object* qst_giver = nullptr;
    uint32 guidtype = GET_TYPE_FROM_GUID(guid);

    if (guidtype == HIGHGUID_TYPE_UNIT)
    {
        Creature* quest_giver = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;
        if (quest_giver->isQuestGiver())
        {
            bValid = true;
            qst = sMySQLStore.getQuestProperties(quest_id);
        }
    }
    else if (guidtype == HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        bValid = true;
        qst = sMySQLStore.getQuestProperties(quest_id);
    }

    if (!qst_giver)
    {
        LOG_DEBUG("WORLD: Invalid questgiver GUID.");
        return;
    }

    if (!bValid || qst == nullptr)
    {
        LOG_DEBUG("WORLD: Creature is not a questgiver.");
        return;
    }

    //FIX ME: Some Quest givers talk in the end of the quest.
    //   qst_giver->SendChatMessage(CHAT_MSG_MONSTER_SAY,LANG_UNIVERSAL,qst->GetQuestEndMessage().c_str());
    QuestLogEntry* qle = _player->GetQuestLogForEntry(quest_id);
    if (!qle && !qst->is_repeatable)
    {
        LOG_DEBUG("WORLD: QuestLogEntry not found.");
        return;
    }

    if (qle && !qle->CanBeFinished())
    {
        LOG_DEBUG("WORLD: Quest not finished.");
        return;
    }

    // remove icon
    /*if (qst_giver->GetTypeId() == TYPEID_UNIT)
    {
    qst_giver->BuildFieldUpdatePacket(GetPlayer(), UNIT_DYNAMIC_FLAGS, qst_giver->GetUInt32Value(UNIT_DYNAMIC_FLAGS));
    }*/

    //check for room in inventory for all items
    if (!sQuestMgr.CanStoreReward(GetPlayer(), qst, reward_slot))
    {
        sQuestMgr.SendQuestFailed(FAILED_REASON_INV_FULL, qst, GetPlayer());
        return;
    }

    sQuestMgr.OnQuestFinished(GetPlayer(), qst, qst_giver, reward_slot);
    //if (qst_giver->GetTypeId() == TYPEID_UNIT) qst->LUA_SendEvent(TO< Creature* >(qst_giver),GetPlayer(),ON_QUEST_COMPLETEQUEST);

    if (qst->next_quest_id)
    {
        WorldPacket data(12);
        data.Initialize(CMSG_QUESTGIVER_QUERY_QUEST);
        data << guid;
        data << qst->next_quest_id;
        HandleQuestGiverQueryQuestOpcode(data);
    }
}

void WorldSession::HandlePushQuestToPartyOpcode(WorldPacket& recvData)
{
    uint32 questid;
    recvData >> questid;

    QuestProperties const* pQuest = sMySQLStore.getQuestProperties(questid);
    if (pQuest)
    {
        Group* pGroup = _player->GetGroup();
        if (pGroup)
        {
            uint32 pguid = _player->GetLowGUID();
            SubGroup* sgr = _player->GetGroup() ?
                _player->GetGroup()->GetSubGroup(_player->GetSubGroup()) : 0;

            if (sgr)
            {
                _player->GetGroup()->Lock();
                GroupMembersSet::iterator itr;
                for (itr = sgr->GetGroupMembersBegin(); itr != sgr->GetGroupMembersEnd(); ++itr)
                {
                    Player* pPlayer = (*itr)->m_loggedInPlayer;
                    if (pPlayer && pPlayer->GetGUID() != pguid)
                    {
                        WorldPacket data(MSG_QUEST_PUSH_RESULT, 9);
                        data << uint64(pPlayer->GetGUID());
                        data << uint8(QUEST_SHARE_MSG_SHARING_QUEST);
                        _player->GetSession()->SendPacket(&data);

                        uint8 response = QUEST_SHARE_MSG_SHARING_QUEST;
                        uint32 status = sQuestMgr.PlayerMeetsReqs(pPlayer, pQuest, false);

                        // Checks if the player has the quest
                        if (pPlayer->HasQuest(questid))
                        {
                            response = QUEST_SHARE_MSG_HAVE_QUEST;
                        }
                        // Checks if the player has finished the quest
                        else if (pPlayer->HasFinishedQuest(questid))
                        {
                            response = QUEST_SHARE_MSG_FINISH_QUEST;
                        }
                        // Checks if the player is able to take the quest
                        else if (status != QMGR_QUEST_AVAILABLE && status != QMGR_QUEST_CHAT)
                        {
                            response = QUEST_SHARE_MSG_CANT_TAKE_QUEST;
                        }
                        // Checks if the player has room in his/her questlog
                        else if (pPlayer->GetOpenQuestSlot() > MAX_QUEST_SLOT)
                        {
                            response = QUEST_SHARE_MSG_LOG_FULL;
                        }
                        // Checks if the player is dueling
                        else if (pPlayer->DuelingWith)   // || pPlayer->GetQuestSharer()) //VLack: A possible lock up can occur if we don't zero out questsharer, because sometimes the client does not send the reply packet.. This of course eliminates the check on it, so it is possible to spam group members with quest sharing, but hey, they are YOUR FRIENDS, and better than not being able to receive quest sharing requests at all!
                        {
                            response = QUEST_SHARE_MSG_BUSY;
                        }

                        //VLack: The quest giver player has to be visible for pPlayer, or else the client will show a non-functional "complete quest" panel instead of the "accept quest" one!
                        //We could either push a full player create for pPlayer that would cause problems later (because they are still out of range and this would have to be handled somehow),
                        //or create a fake bad response, as we no longer have an out of range response. I'll go with the latter option and send that the other player is busy...
                        //Also, pPlayer's client can send a busy response automatically even if the players see each other, but they are still too far away.
                        //But sometimes nothing happens on pPlayer's client (near the end of mutual visibility line), no quest window and no busy response either. This has to be solved later, maybe a distance check here...
                        if (response == QUEST_SHARE_MSG_SHARING_QUEST && !pPlayer->IsVisible(_player->GetGUID()))
                        {
                            response = QUEST_SHARE_MSG_BUSY;
                        }

                        if (response != QUEST_SHARE_MSG_SHARING_QUEST)
                        {
                            sQuestMgr.SendPushToPartyResponse(_player, pPlayer, response);
                            continue;
                        }

                        data.clear();
                        sQuestMgr.BuildQuestDetails(&data, pQuest, _player, 1, pPlayer->GetSession()->language, pPlayer);
                        pPlayer->SetQuestSharer(pguid); //VLack: better to set this _before_ sending out the packet, so no race conditions can happen on heavily loaded servers.
                        pPlayer->GetSession()->SendPacket(&data);
                    }
                }
                _player->GetGroup()->Unlock();
            }
        }
    }
}

void WorldSession::HandleQuestPushResult(WorldPacket& recvPacket)
{
    uint64 guid;
    uint8 msg;
    recvPacket >> guid;
    uint32 questid = 0;
    if (recvPacket.size() >= 13)  //VLack: The client can send a 13 byte packet, where the result message is the 13th byte, and we have some data before it... Usually it is the quest id, but I have seen this as uint32(0) too.
        recvPacket >> questid;
    recvPacket >> msg;

    //LOG_DETAIL("WORLD: Received MSG_QUEST_PUSH_RESULT");

    if (GetPlayer()->GetQuestSharer())
    {
        Player* pPlayer = objmgr.GetPlayer(GetPlayer()->GetQuestSharer());
        if (pPlayer)
        {
            WorldPacket data(MSG_QUEST_PUSH_RESULT, 9);
            if (recvPacket.size() >= 13)  //VLack: In case the packet was the longer one, its guid is the quest giver player, thus in the response we have to tell him that _this_ player reported the particular state. I think this type of response could also eliminate our SetQuestSharer/GetQuestSharer mess and its possible lock up conditions...
                data << uint64(_player->GetGUID());
            else
                data << uint64(guid);
            data << uint8(msg);
            pPlayer->GetSession()->SendPacket(&data);
            GetPlayer()->SetQuestSharer(0);
        }
    }
}

