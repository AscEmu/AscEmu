/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/MsgQuestPushResult.h"
#include "Server/Packets/CmsgQuestgiverAcceptQuest.h"
#include "Server/Packets/CmsgQuestQuery.h"
#include "Server/Packets/CmsgQuestPoiQuery.h"
#include "Server/Packets/CmsgQuestgiverHello.h"
#include "Server/Packets/CmsgQuestgiverStatusQuery.h"
#include "Server/Packets/SmsgQuestgiverStatus.h"
#include "Server/Packets/CmsgQuestgiverQueryQuest.h"
#include "Server/Packets/CmsgQuestlogRemoveQuest.h"
#include "Server/Packets/CmsgQuestgiverRequestReward.h"
#include "Server/Packets/CmsgQuestgiverCompleteQuest.h"
#include "Server/Packets/CmsgQuestgiverChooseReward.h"
#include "Server/Packets/CmsgPushquesttoparty.h"
#include "Server/WorldSession.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/MapMgr.h"
#include "Management/ItemInterface.h"

using namespace AscEmu::Packets;

#if VERSION_STRING < Cata
WorldPacket* WorldSession::buildQuestQueryResponse(QuestProperties const* qst)
{
    // 2048 bytes should be more than enough. The fields cost ~200 bytes.
    // better to allocate more at startup than have to realloc the buffer later on.

    WorldPacket* data = new WorldPacket(SMSG_QUEST_QUERY_RESPONSE, 248);
    MySQLStructure::LocalesQuest const* lci = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;
    uint32_t i;

    *data << uint32_t(qst->id);                                       // Quest ID
    *data << uint32_t(2);                                             // Unknown, always seems to be 2
    *data << int32_t(qst->questlevel);                                // Quest level
    *data << uint32_t(qst->min_level);                                // Quest required level

    if (qst->quest_sort > 0)
        *data << int32_t(-(int32_t)qst->quest_sort);                    // Negative if pointing to a sort.
    else
        *data << uint32_t(qst->zone_id);                              // Positive if pointing to a zone.

    *data << uint32_t(qst->type);                                     // Info ID / Type
    *data << qst->suggestedplayers;                                 // suggested players

    *data << uint32_t(qst->required_rep_faction);                     // Faction ID
    *data << uint32_t(qst->required_rep_value);                       // Faction Amount

    *data << uint32_t(0);                                             // Unknown (always 0)
    *data << uint32_t(0);                                             // Unknown (always 0)

    *data << uint32_t(qst->next_quest_id);                            // Next Quest ID
    *data << uint32_t(0);                                             // Column id +1 from QuestXp.dbc, entry is quest level

    *data << uint32_t(sQuestMgr.GenerateRewardMoney(_player, qst));   // Copper reward
    *data << uint32_t(qst->reward_money < 0 ? -qst->reward_money : 0);    // Required Money

    *data << uint32_t(qst->reward_spell);                             // Spell added to spellbook upon completion
    *data << uint32_t(qst->effect_on_player);                         // Spell casted on player upon completion

    *data << uint32_t(qst->bonushonor);                               // 2.3.0 - bonus honor
    *data << float(0);                                              // 3.3.0 - some multiplier for honor
    *data << uint32_t(qst->srcitem);                                  // Item given at the start of a quest (srcitem)
    *data << uint32_t(qst->quest_flags);                              // Quest Flags
    *data << qst->rewardtitleid;                                    // 2.4.0 unk
    *data << uint32_t(0);                                             // playerkillcount
    *data << qst->rewardtalents;
    *data << uint32_t(0);                                             // 3.3.0 Unknown
    *data << uint32_t(0);                                             // 3.3.0 Unknown

    // (loop 4 times)
    for (i = 0; i < 4; ++i)
    {
        *data << qst->reward_item[i];               // Forced Reward Item [i]
        *data << qst->reward_itemcount[i];          // Forced Reward Item Count [i]
    }

    // (loop 6 times)
    for (i = 0; i < 6; ++i)
    {
        *data << qst->reward_choiceitem[i];         // Choice Reward Item [i]
        *data << qst->reward_choiceitemcount[i];    // Choice Reward Item Count [i]
    }

    // (loop 5 times) - these 3 loops are here to allow displaying rep rewards in client (not handled in core yet)
    for (i = 0; i < 5; ++i)
        *data << uint32_t(qst->reward_repfaction[i]); // reward factions ids

    for (i = 0; i < 5; ++i)
        *data << uint32_t(0);                         // column index in QuestFactionReward.dbc but use unknown

    for (i = 0; i < 5; ++i)                         // Unknown
        *data << uint32_t(0);

    *data << qst->point_mapid;
    *data << qst->point_x;
    *data << qst->point_y;
    *data << qst->point_opt;

    if (lci != nullptr)
    {
        *data << lci->title;
        *data << lci->objectives;
        *data << lci->details;
        *data << lci->endText;
        *data << uint8_t(0);
    }
    else
    {
        *data << qst->title;                        // Title / name of quest
        *data << qst->objectives;                   // Objectives / description
        *data << qst->details;                      // Details
        *data << qst->endtext;                      // Subdescription
        *data << uint8_t(0);                          // most 3.3.0 quests i seen have something like "Return to NPCNAME"
    }

    for (i = 0; i < 4; ++i)
    {
        *data << qst->required_mob_or_go[i];              // Kill mob entry ID [i]
        *data << qst->required_mob_or_go_count[i];         // Kill mob count [i]
        *data << uint32_t(0);                         // Unknown
        *data << uint32_t(0);                         // 3.3.0 Unknown
    }

    for (i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        *data << qst->required_item[i];             // Collect item [i]
        *data << qst->required_itemcount[i];        // Collect item count [i]
    }

    if (lci != nullptr)
    {
        *data << lci->objectiveText[0];
        *data << lci->objectiveText[1];
        *data << lci->objectiveText[2];
        *data << lci->objectiveText[3];
    }
    else
    {
        *data << qst->objectivetexts[0];            // Objective 1 - Used as text if mob not set
        *data << qst->objectivetexts[1];            // Objective 2 - Used as text if mob not set
        *data << qst->objectivetexts[2];            // Objective 3 - Used as text if mob not set
        *data << qst->objectivetexts[3];            // Objective 4 - Used as text if mob not set
    }

    return data;
}
#else
WorldPacket* WorldSession::buildQuestQueryResponse(QuestProperties const* qst)
{
    WorldPacket* data = new WorldPacket(SMSG_QUEST_QUERY_RESPONSE, 100);
    MySQLStructure::LocalesQuest const* lci = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;

    *data << uint32_t(qst->id);                                        // Quest ID
    *data << uint32_t(2);                                              // Unknown, always seems to be 2
    *data << int32_t(qst->questlevel);                                 // Quest level
    *data << uint32_t(qst->min_level);                                 // Quest required level

    if (qst->quest_sort > 0)
        *data << int32_t(-(int32_t)qst->quest_sort);                     // Negative if pointing to a sort.
    else
        *data << uint32_t(qst->zone_id);                               // Positive if pointing to a zone.

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
        *data << uint32_t(qst->reward_repfaction[i]);

    for (uint8_t i = 0; i < 5; ++i)
        *data << uint32_t(0);                                          // column index in QuestFactionReward.dbc but use unknown

    for (uint8_t i = 0; i < 5; ++i)
        *data << uint32_t(0);                                          // unk

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
        *data << (lci ? lci->objectiveText[i] : qst->objectivetexts[i]);

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
#endif

void WorldSession::handleQuestPushResultOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    MsgQuestPushResult srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received MSG_QUEST_PUSH_RESULT");

    if (_player->GetQuestSharer())
    {
        const auto questSharerPlayer = objmgr.GetPlayer(_player->GetQuestSharer());
        if (questSharerPlayer)
        {
            const uint64_t guid = recvPacket.size() >= 13 ? _player->getGuid() : srlPacket.giverGuid;
            questSharerPlayer->GetSession()->SendPacket(MsgQuestPushResult(guid, 0, srlPacket.pushResult).serialise().get());
            _player->SetQuestSharer(0);
        }
    }
}

void WorldSession::handleQuestgiverAcceptQuestOpcode(WorldPacket& recvPacket)
{
    CmsgQuestgiverAcceptQuest srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->AcceptQuest(srlPacket.guid, srlPacket.questId);
}

void WorldSession::handleQuestQueryOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgQuestQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (const auto questProperties = sMySQLStore.getQuestProperties(srlPacket.questId))
    {
        WorldPacket* worldPacket = buildQuestQueryResponse(questProperties);
        SendPacket(worldPacket);
        delete worldPacket;
    }
    else
    {
        LogDebugFlag(LF_OPCODE, "Invalid quest Id %u.", srlPacket.questId);
    }
}

#if VERSION_STRING > TBC
void WorldSession::handleQuestPOIQueryOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgQuestPoiQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_QUEST_POI_QUERY");

    if (srlPacket.questCount > MAX_QUEST_LOG_SIZE)
    {
        LogDebugFlag(LF_OPCODE, "Client sent Quest POI query for more than MAX_QUEST_LOG_SIZE quests.");

        srlPacket.questCount = MAX_QUEST_LOG_SIZE;
    }

    WorldPacket data(SMSG_QUEST_POI_QUERY_RESPONSE, 4 + (4 + 4) * srlPacket.questCount);
    data << srlPacket.questCount;
    for (auto questId : srlPacket.questIds)
        sQuestMgr.BuildQuestPOIResponse(data, questId);

    SendPacket(&data);
}
#endif

void WorldSession::handleQuestgiverCancelOpcode(WorldPacket& /*recvPacket*/)
{
    CHECK_INWORLD_RETURN

    OutPacket(SMSG_GOSSIP_COMPLETE, 0, nullptr);

    LogDebugFlag(LF_OPCODE, "Sent SMSG_GOSSIP_COMPLETE");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WorldSession::handleQuestgiverHelloOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgQuestgiverHello srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (const auto questGiver = _player->GetMapMgr()->GetCreature(srlPacket.questGiverGuid.getGuidLowPart()))
    {
        if (!questGiver->isQuestGiver())
        {
            LogDebugFlag(LF_OPCODE, "Creature with guid %u is not a questgiver.", srlPacket.questGiverGuid.getGuidLowPart());
            return;
        }

        sQuestMgr.OnActivateQuestGiver(questGiver, _player);
    }
    else
    {
        LogDebugFlag(LF_OPCODE, "Invalid questgiver guid %u.", srlPacket.questGiverGuid.getGuidLowPart());
    }
}

void WorldSession::handleQuestgiverStatusQueryOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgQuestgiverStatusQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (_player->IsInBg())
        return;

    Object* qst_giver = nullptr;

    if (srlPacket.questGiverGuid.isUnit())
    {
        Creature* quest_giver = _player->GetMapMgr()->GetCreature(srlPacket.questGiverGuid.getGuidLowPart());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        if (!quest_giver->isQuestGiver())
        {
            LogDebugFlag(LF_OPCODE, "Creature is not a questgiver.");
            return;
        }
    }
    else if (srlPacket.questGiverGuid.isItem())
    {
        Item* quest_giver = _player->getItemInterface()->GetItemByGUID(srlPacket.questGiverGuid.GetOldGuid());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;
    }
    else if (srlPacket.questGiverGuid.isGameObject())
    {
        GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(srlPacket.questGiverGuid.getGuidLowPart());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;
    }

    if (!qst_giver)
    {
        LogDebugFlag(LF_OPCODE, "Invalid questgiver GUID " I64FMT ".", srlPacket.questGiverGuid.GetOldGuid());
        return;
    }

    const uint32_t questStatus = sQuestMgr.CalcStatus(qst_giver, _player);
    SendPacket(SmsgQuestgiverStatus(srlPacket.questGiverGuid.GetOldGuid(), questStatus).serialise().get());
}

void WorldSession::handleQuestGiverQueryQuestOpcode(WorldPacket& recvPacket)
{
    CmsgQuestgiverQueryQuest srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    Object* qst_giver = nullptr;

    bool bValid = false;

    QuestProperties const* qst = sMySQLStore.getQuestProperties(srlPacket.questId);
    if (!qst)
    {
        LogDebugFlag(LF_OPCODE, "Invalid quest with id %u", srlPacket.questId);
        return;
    }

    uint32_t status = QuestStatus::NotAvailable;

    if (srlPacket.guid.isUnit())
    {
        Creature* quest_giver = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;
        if (quest_giver->isQuestGiver())
        {
            bValid = true;
            status = sQuestMgr.CalcQuestStatus(qst_giver, _player, qst, static_cast<uint8_t>(quest_giver->GetQuestRelation(qst->id)), false);
        }
    }
    else if (srlPacket.guid.isGameObject())
    {
        GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(srlPacket.guid.getGuidLowPart());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;
        bValid = false;
        if (quest_giver->getGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
        {
            bValid = true;
            auto go_quest_giver = dynamic_cast<GameObject_QuestGiver*>(quest_giver);
            status = sQuestMgr.CalcQuestStatus(qst_giver, _player, qst, static_cast<uint8_t>(go_quest_giver->GetQuestRelation(qst->id)), false);
        }
    }
    else if (srlPacket.guid.isItem())
    {
        Item* quest_giver = _player->getItemInterface()->GetItemByGUID(srlPacket.guid.GetOldGuid());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        const auto itemProto = quest_giver->getItemProperties();
        if (itemProto->Bonding != ITEM_BIND_ON_USE || quest_giver->isSoulbound())
        {
            if (sScriptMgr.CallScriptedItem(quest_giver, _player))
                return;
        }

        if (itemProto->Bonding == ITEM_BIND_ON_USE)
            quest_giver->addFlags(ITEM_FLAG_SOULBOUND);

        bValid = true;
        status = sQuestMgr.CalcQuestStatus(qst_giver, _player, qst, 1, false);
    }

    if (!qst_giver)
    {
        LogDebugFlag(LF_OPCODE, "Invalid questgiver GUID.");
        return;
    }

    if (!bValid)
    {
        LogDebugFlag(LF_OPCODE, "object is not a questgiver.");
        return;
    }

    WorldPacket data;

    if ((status == QuestStatus::Available) || (status == QuestStatus::Repeatable) || (status == QuestStatus::AvailableChat))
    {
        sQuestMgr.BuildQuestDetails(&data, qst, qst_giver, 1, language, _player);
        SendPacket(&data);
        LogDebugFlag(LF_OPCODE, "Sent SMSG_QUESTGIVER_QUEST_DETAILS.");

        if (qst->HasFlag(QUEST_FLAGS_AUTO_ACCEPT))
            _player->AcceptQuest(qst_giver->getGuid(), qst->id);
    }
    else if (status == QuestStatus::NotFinished || status == QuestStatus::Finished)
    {
        sQuestMgr.BuildRequestItems(&data, qst, qst_giver, status, language);
        SendPacket(&data);
        LogDebugFlag(LF_OPCODE, "Sent SMSG_QUESTGIVER_REQUEST_ITEMS.");
    }
}

void WorldSession::handleQuestlogRemoveQuestOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgQuestlogRemoveQuest srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.questLogSlot >= 25)
        return;

    QuestLogEntry* qEntry = _player->GetQuestLogInSlot(srlPacket.questLogSlot);
    if (!qEntry)
    {
        LogDebugFlag(LF_OPCODE, " No quest in slot %d.", srlPacket.questLogSlot);
        return;
    }
    QuestProperties const* qPtr = qEntry->GetQuest();
    CALL_QUESTSCRIPT_EVENT(qEntry, OnQuestCancel)(_player);
    qEntry->Finish();

    for (uint8_t i = 0; i < 4; ++i)
    {
        if (qPtr->receive_items[i])
            _player->getItemInterface()->RemoveItemAmt(qPtr->receive_items[i], 1);
    }

    if (qPtr->srcitem && qPtr->srcitem != qPtr->receive_items[0])
    {
        ItemProperties const* itemProto = sMySQLStore.getItemProperties(qPtr->srcitem);
        if (itemProto != nullptr)
            if (itemProto->QuestId != qPtr->id)
                _player->getItemInterface()->RemoveItemAmt(qPtr->srcitem, qPtr->srcitemcount ? qPtr->srcitemcount : 1);
    }

    for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        if (qPtr->required_item[i] != 0)
        {
            ItemProperties const* itemProto = sMySQLStore.getItemProperties(qPtr->required_item[i]);
            if (itemProto != nullptr && itemProto->Class == ITEM_CLASS_QUEST)
                _player->getItemInterface()->RemoveItemAmt(qPtr->required_item[i], qPtr->required_itemcount[i]);
        }
    }

    _player->UpdateNearbyGameObjects();

    sHookInterface.OnQuestCancelled(_player, qPtr);
}

void WorldSession::handleQuestgiverRequestRewardOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgQuestgiverRequestReward srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    bool bValid = false;
    QuestProperties const* qst = nullptr;
    Object* qst_giver = nullptr;
    uint32_t status = 0;

    if (srlPacket.questgiverGuid.isUnit())
    {
        Creature* quest_giver = _player->GetMapMgr()->GetCreature(srlPacket.questgiverGuid.getGuidLowPart());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        if (quest_giver->isQuestGiver())
        {
            bValid = true;
            qst = quest_giver->FindQuest(srlPacket.questId, QUESTGIVER_QUEST_END);
            if (!qst)
                qst = quest_giver->FindQuest(srlPacket.questId, QUESTGIVER_QUEST_START);

            if (!qst)
            {
                LogDebugFlag(LF_OPCODE, "Cannot get reward for quest %u, as it doesn't exist at Unit %u.", srlPacket.questId, quest_giver->getEntry());
                return;
            }
            status = sQuestMgr.CalcQuestStatus(qst_giver, _player, qst, static_cast<uint8_t>(quest_giver->GetQuestRelation(qst->id)), false);
        }
    }
    else if (srlPacket.questgiverGuid.isGameObject())
    {
        GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(srlPacket.questgiverGuid.getGuidLowPart());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        bValid = false;
        if (quest_giver->getGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
        {
            bValid = true;
            GameObject_QuestGiver* go_quest_giver = static_cast<GameObject_QuestGiver*>(quest_giver);
            qst = go_quest_giver->FindQuest(srlPacket.questId, QUESTGIVER_QUEST_END);
            if (!qst)
            {
                LogDebugFlag(LF_OPCODE, "Cannot get reward for quest %u, as it doesn't exist at GO %u.", srlPacket.questId, quest_giver->getEntry());
                return;
            }
            status = sQuestMgr.CalcQuestStatus(qst_giver, _player, qst, static_cast<uint8_t>(go_quest_giver->GetQuestRelation(qst->id)), false);
        }
    }

    if (!qst_giver)
    {
        LogDebugFlag(LF_OPCODE, "Invalid questgiver GUID.");
        return;
    }

    if (!bValid || qst == nullptr)
    {
        LogDebugFlag(LF_OPCODE, "Creature is not a questgiver.");
        return;
    }

    if (status == QuestStatus::Finished)
    {
        WorldPacket data;
        sQuestMgr.BuildOfferReward(&data, qst, qst_giver, 1, language, _player);
        SendPacket(&data);
        LogDebugFlag(LF_OPCODE, "Sent SMSG_QUESTGIVER_REQUEST_ITEMS.");
    }
}

void WorldSession::handleQuestgiverCompleteQuestOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgQuestgiverCompleteQuest srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    bool bValid = false;
    QuestProperties const* qst = nullptr;
    Object* qst_giver = nullptr;
    uint32_t status = 0;

    if (srlPacket.questgiverGuid.isUnit())
    {
        Creature* quest_giver = _player->GetMapMgr()->GetCreature(srlPacket.questgiverGuid.getGuidLowPart());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        if (quest_giver->isQuestGiver())
        {
            bValid = true;
            qst = quest_giver->FindQuest(srlPacket.questId, QUESTGIVER_QUEST_END);
            if (!qst)
            {
                LogDebugFlag(LF_OPCODE, "Cannot complete quest %u, as it doesn't exist at Unit %u.", srlPacket.questId, quest_giver->getEntry());
                return;
            }
            status = sQuestMgr.CalcQuestStatus(qst_giver, _player, qst, static_cast<uint8_t>(quest_giver->GetQuestRelation(qst->id)), false);
        }
    }
    else if (srlPacket.questgiverGuid.isGameObject())
    {
        GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(srlPacket.questgiverGuid.getGuidLowPart());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        bValid = false;
        if (quest_giver->getGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
        {
            GameObject_QuestGiver* go_quest_giver = static_cast<GameObject_QuestGiver*>(quest_giver);
            qst = go_quest_giver->FindQuest(srlPacket.questId, QUESTGIVER_QUEST_END);
            if (!qst)
            {
                LogDebugFlag(LF_OPCODE, "Cannot complete quest %u, as it doesn't exist at GO %u.", srlPacket.questId, quest_giver->getEntry());
                return;
            }
            status = sQuestMgr.CalcQuestStatus(qst_giver, _player, qst, static_cast<uint8_t>(go_quest_giver->GetQuestRelation(qst->id)), false);
        }
    }

    if (!qst_giver)
    {
        LogDebugFlag(LF_OPCODE, "Invalid questgiver GUID.");
        return;
    }

    if (!bValid || qst == nullptr)
    {
        LogDebugFlag(LF_OPCODE, "Creature is not a questgiver.");
        return;
    }

    if (status == QuestStatus::NotFinished || status == QuestStatus::Repeatable)
    {
        WorldPacket data;
        sQuestMgr.BuildRequestItems(&data, qst, qst_giver, status, language);
        SendPacket(&data);
        LogDebugFlag(LF_OPCODE, "Sent SMSG_QUESTGIVER_REQUEST_ITEMS.");
    }

    if (status == QuestStatus::Finished)
    {
        WorldPacket data;
        sQuestMgr.BuildOfferReward(&data, qst, qst_giver, 1, language, _player);
        SendPacket(&data);
        LogDebugFlag(LF_OPCODE, "Sent SMSG_QUESTGIVER_REQUEST_ITEMS.");
    }

    sHookInterface.OnQuestFinished(_player, qst, qst_giver);
}

void WorldSession::handleQuestgiverChooseRewardOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgQuestgiverChooseReward srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.rewardSlot >= 6)
        return;

    bool bValid = false;
    QuestProperties const* qst = nullptr;
    Object* qst_giver = nullptr;

    if (srlPacket.questgiverGuid.isUnit())
    {
        Creature* quest_giver = _player->GetMapMgr()->GetCreature(srlPacket.questgiverGuid.getGuidLowPart());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        if (quest_giver->isQuestGiver())
        {
            bValid = true;
            qst = sMySQLStore.getQuestProperties(srlPacket.questId);
        }
    }
    else if (srlPacket.questgiverGuid.isGameObject())
    {
        GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(srlPacket.questgiverGuid.getGuidLowPart());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        bValid = true;
        qst = sMySQLStore.getQuestProperties(srlPacket.questId);
    }

    if (!qst_giver)
    {
        LogDebugFlag(LF_OPCODE, "Invalid questgiver GUID.");
        return;
    }

    if (!bValid || qst == nullptr)
    {
        LogDebugFlag(LF_OPCODE, "Creature is not a questgiver.");
        return;
    }

    QuestLogEntry* qle = _player->GetQuestLogForEntry(srlPacket.questId);
    if (!qle && !qst->is_repeatable)
    {
        LogDebugFlag(LF_OPCODE, "QuestLogEntry not found.");
        return;
    }

    if (qle && !qle->CanBeFinished())
    {
        LogDebugFlag(LF_OPCODE, "Quest not finished.");
        return;
    }

    if (!sQuestMgr.CanStoreReward(_player, qst, srlPacket.rewardSlot))
    {
        sQuestMgr.SendQuestFailed(FAILED_REASON_INV_FULL, qst, _player);
        return;
    }

    sQuestMgr.OnQuestFinished(_player, qst, qst_giver, srlPacket.rewardSlot);

    if (qst->next_quest_id)
    {
        WorldPacket data(12);
        data.Initialize(CMSG_QUESTGIVER_QUERY_QUEST);
        data << srlPacket.questgiverGuid.GetOldGuid();
        data << qst->next_quest_id;
        handleQuestGiverQueryQuestOpcode(data);
    }
}

void WorldSession::handlePushQuestToPartyOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgPushquesttoparty srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    QuestProperties const* pQuest = sMySQLStore.getQuestProperties(srlPacket.questId);
    if (pQuest)
    {
        Group* pGroup = _player->GetGroup();
        if (pGroup)
        {
            uint32_t pguid = _player->getGuidLow();
            SubGroup* sgr = _player->GetGroup() ?
                _player->GetGroup()->GetSubGroup(_player->GetSubGroup()) : 0;

            if (sgr)
            {
                _player->GetGroup()->Lock();
                for (GroupMembersSet::iterator itr = sgr->GetGroupMembersBegin(); itr != sgr->GetGroupMembersEnd(); ++itr)
                {
                    Player* pPlayer = (*itr)->m_loggedInPlayer;
                    if (pPlayer && pPlayer->getGuid() != pguid)
                    {
                        _player->GetSession()->SendPacket(MsgQuestPushResult(pPlayer->getGuid(), 0, QUEST_SHARE_MSG_SHARING_QUEST).serialise().get());

                        uint8_t response = QUEST_SHARE_MSG_SHARING_QUEST;
                        uint32_t status = sQuestMgr.PlayerMeetsReqs(pPlayer, pQuest, false);

                        if (pPlayer->HasQuest(srlPacket.questId))
                        {
                            response = QUEST_SHARE_MSG_HAVE_QUEST;
                        }
                        else if (pPlayer->HasFinishedQuest(srlPacket.questId))
                        {
                            response = QUEST_SHARE_MSG_FINISH_QUEST;
                        }
                        else if (status != QuestStatus::Available && status != QuestStatus::AvailableChat)
                        {
                            response = QUEST_SHARE_MSG_CANT_TAKE_QUEST;
                        }
                        else if (pPlayer->GetOpenQuestSlot() > MAX_QUEST_SLOT)
                        {
                            response = QUEST_SHARE_MSG_LOG_FULL;
                        }
                        else if (pPlayer->DuelingWith)
                        {
                            response = QUEST_SHARE_MSG_BUSY;
                        }
                        
                        if (response == QUEST_SHARE_MSG_SHARING_QUEST && !pPlayer->IsVisible(_player->getGuid()))
                        {
                            response = QUEST_SHARE_MSG_BUSY;
                        }

                        if (response != QUEST_SHARE_MSG_SHARING_QUEST)
                        {
                            sQuestMgr.SendPushToPartyResponse(_player, pPlayer, response);
                            continue;
                        }

                        WorldPacket data;
                        sQuestMgr.BuildQuestDetails(&data, pQuest, _player, 1, pPlayer->GetSession()->language, pPlayer);
                        pPlayer->SetQuestSharer(pguid);
                        pPlayer->GetSession()->SendPacket(&data);
                    }
                }
                _player->GetGroup()->Unlock();
            }
        }
    }
}
