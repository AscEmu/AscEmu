/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/MainServerDefines.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Spell/SpellMgr.h"

void QuestMgr::BuildQuestDetails(WorldPacket* data, QuestProperties const* qst, Object* qst_giver, uint32_t /*menutype*/, uint32_t language, Player* plr)
{
    MySQLStructure::LocalesQuest const* lq = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;
    std::map<uint32_t, uint8_t>::const_iterator itr;

    std::string questEndText = "";
    std::string questGiverTextWindow = "";
    std::string questGiverTargetName = "";
    std::string questTurnTextWindow = "";
    std::string questTurnTargetName = "";

    data->SetOpcode(SMSG_QUESTGIVER_QUEST_DETAILS);
    *data << uint64_t(qst_giver->GetGUID());   // npc guid
    *data << uint64_t(0);                      // (questsharer?) guid
    *data << uint32_t(qst->id);

    *data << (lq ? lq->title : qst->title);
    *data << (lq ? lq->details : qst->details);
    *data << (lq ? lq->objectives : qst->objectives);

    *data << questGiverTextWindow;           // 4.x
    *data << questGiverTargetName;           // 4.x
    *data << questTurnTextWindow;            // 4.x
    *data << questTurnTargetName;            // 4.x

    *data << uint32_t(0);                      // 4.x - qgportait
    *data << uint32_t(0);                      // 4.x - qgturninportrait

    *data << uint8_t(1);						// Activate accept

    *data << uint32_t(qst->quest_flags);
    *data << uint32_t(qst->suggestedplayers);

    *data << uint8_t(0);                       // finished? value is sent back to server in quest accept packet
    *data << uint8_t(0);                       // 4.x Starts at AreaTrigger
    *data << uint32_t(0);                      // required spell

    *data << uint32_t(qst->count_reward_choiceitem);
    for (uint8_t i = 0; i < 6; ++i)
    {
        *data << uint32_t(qst->reward_choiceitem[i]);
    }

    for (uint8_t i = 0; i < 6; ++i)
    {
        *data << uint32_t(qst->reward_choiceitemcount[i]);
    }

    for (uint8_t i = 0; i < 6; ++i)
    {
        if (ItemProperties const* ip = sMySQLStore.getItemProperties(qst->reward_choiceitem[i]))
        {
            *data << uint32_t(ip->DisplayInfoID);
        }
        else
        {
            *data << uint32_t(0);
        }
    }

    *data << uint32_t(qst->count_required_item);

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(qst->reward_item[i]);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(qst->reward_itemcount[i]);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        if (ItemProperties const* ip = sMySQLStore.getItemProperties(qst->reward_item[i]))
        {
            *data << uint32_t(ip->DisplayInfoID);
        }
        else
        {
            *data << uint32_t(0);
        }
    }

    *data << uint32_t(GenerateRewardMoney(plr, qst));  // Money reward
    *data << uint32_t(GenerateQuestXP(plr, qst));

    *data << uint32_t(qst->rewardtitleid);
    *data << uint32_t(0);                              // Honor reward
    *data << float(0.0f);                            // New 3.3
    *data << uint32_t(0);                              // reward talent
    *data << uint32_t(0);                              // unk
    *data << uint32_t(0);                              // reputationmask

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << uint32_t(0);
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << int32_t(0);
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << uint32_t(0);
    }

    *data << uint32_t(0);      // reward spell
    *data << uint32_t(0);      // reward spell cast

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(0);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(0);
    }

    *data << uint32_t(0);      //rewskill
    *data << uint32_t(0);      //rewskillpoint

    *data << uint32_t(4);      // emote count

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(qst->detailemote[i]);
        *data << uint32_t(qst->detailemotedelay[i]);
    }
}

void QuestMgr::BuildOfferReward(WorldPacket* data, QuestProperties const* qst, Object* qst_giver, uint32_t /*menutype*/, uint32_t language, Player* plr)
{
    MySQLStructure::LocalesQuest const* lq = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;

    std::string questGiverTextWindow = "";
    std::string questGiverTargetName = "";
    std::string questTurnTextWindow = "";
    std::string questTurnTargetName = "";

    data->SetOpcode(SMSG_QUESTGIVER_OFFER_REWARD);
    *data << uint64_t(qst_giver->GetGUID());
    *data << uint32_t(qst->id);

    *data << (lq ? lq->title : qst->title);
    *data << (lq ? lq->completionText : qst->completiontext);

    *data << questGiverTextWindow;
    *data << questGiverTargetName;
    *data << questTurnTextWindow;
    *data << questTurnTargetName;

    *data << uint32_t(0);                  // giver portrait
    *data << uint32_t(0);                  // turn in portrait

    *data << uint8_t(qst->next_quest_id ? 1 : 0);
    *data << uint32_t(qst->quest_flags);
    *data << uint32_t(qst->suggestedplayers);

    *data << qst->completionemotecount;
    for (uint8_t i = 0; i < qst->completionemotecount; i++)
    {
        *data << uint32_t(qst->completionemote[i]);
        *data << uint32_t(qst->completionemotedelay[i]);
    }

    *data << uint32_t(qst->count_reward_choiceitem);
    for (uint8_t i = 0; i < 6; ++i)
    {
        *data << uint32_t(qst->reward_choiceitem[i]);
    }

    for (uint8_t i = 0; i < 6; ++i)
    {
        *data << uint32(qst->reward_choiceitemcount[i]);
    }

    for (uint8_t i = 0; i < 6; ++i)
    {
        if (ItemProperties const* ip = sMySQLStore.getItemProperties(qst->reward_choiceitem[i]))
        {
            *data << uint32_t(ip->DisplayInfoID);
        }
        else
        {
            *data << uint32_t(0);
        }
    }

    *data << uint32_t(qst->count_required_item);
    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(qst->reward_item[i]);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(qst->reward_itemcount[i]);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        if (ItemProperties const* ip = sMySQLStore.getItemProperties(qst->reward_item[i]))
        {
            *data << uint32_t(ip->DisplayInfoID);
        }
        else
        {
            *data << uint32_t(0);
        }
    }

    *data << uint32_t(GenerateRewardMoney(plr, qst));  // Money reward
    *data << uint32_t(GenerateQuestXP(plr, qst));

    *data << uint32_t(qst->rewardtitleid);
    *data << uint32_t(0);                              // Honor reward
    *data << float(0.0f);                            // New 3.3
    *data << uint32_t(0);                              // reward talent
    *data << uint32_t(0);                              // unk
    *data << uint32_t(0);                              // reputationmask

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << uint32_t(0);
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << int32_t(0);
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << uint32_t(0);
    }

    *data << uint32_t(0);      // reward spell
    *data << uint32_t(0);      // reward spell cast

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(0);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(0);
    }

    *data << uint32_t(0);      //rewskill
    *data << uint32_t(0);      //rewskillpoint

    *data << uint32_t(4);      // emote count
    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(qst->detailemote[i]);
        *data << uint32_t(qst->detailemotedelay[i]);
    }
}

void QuestMgr::BuildRequestItems(WorldPacket* data, QuestProperties const* qst, Object* qst_giver, uint32_t status, uint32_t language)
{
    MySQLStructure::LocalesQuest const* lq = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;

    data->SetOpcode(SMSG_QUESTGIVER_REQUEST_ITEMS);
    *data << uint64_t(qst_giver->GetGUID());
    *data << uint32_t(qst->id);

    *data << (lq ? lq->title : qst->title);
    *data << (lq ? lq->incompleteText : qst->incompletetext);

    *data << uint32_t(0);

    if (status == QMGR_QUEST_NOT_FINISHED)
    {
        *data << qst->incompleteemote;
    }
    else
    {
        *data << qst->completeemote;
    }

    *data << uint32_t(1);          // close on cancel
    *data << uint32_t(qst->quest_flags);
    *data << uint32_t(qst->suggestedplayers);

    *data << uint32_t(qst->reward_money < 0 ? -qst->reward_money : 0);	     // Required Money

                                                                         // item count
    *data << uint32_t(qst->count_required_item);

    // (loop for each item)
    for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        if (!qst->required_item[i])
        {
            continue;
        }

        *data << uint32_t(qst->required_item[i]);
        *data << uint32_t(qst->required_itemcount[i]);
        if (ItemProperties const* it = sMySQLStore.getItemProperties(qst->required_item[i]))
        {
            *data << uint32_t(it->DisplayInfoID);
        }
        else
        {
            *data << uint32_t(0);
        }
    }

    *data << uint32_t(0);      // required currency count


    if (status == QMGR_QUEST_NOT_FINISHED)
    {
        *data << uint32_t(0); //incomplete button
    }
    else
    {
        *data << uint32_t(2);
    }

    *data << uint32_t(4);
    *data << uint32_t(8);
    *data << uint32_t(16);
    *data << uint32_t(64);
}

void QuestMgr::BuildQuestComplete(Player* plr, QuestProperties const* qst)
{
    uint32_t xp;
    uint32_t currtalentpoints = plr->GetCurrentTalentPoints();
    uint32_t rewardtalents = qst->rewardtalents;
    uint32_t playerlevel = plr->getLevel();

    if (playerlevel >= plr->GetMaxLevel())
    {
        xp = 0;
    }
    else
    {
        xp = float2int32(GenerateQuestXP(plr, qst) * worldConfig.getFloatRate(RATE_QUESTXP));
        plr->GiveXP(xp, 0, false);
    }

    if (currtalentpoints <= (playerlevel - 9 - rewardtalents))
    {
        plr->AddTalentPointsToAllSpec(rewardtalents);
    }

    // Reward title
    if (qst->rewardtitleid > 0)
    {
        plr->SetKnownTitle(static_cast<RankTitles>(qst->rewardtitleid), true);
    }

    // Some spells applied at quest reward
    SpellAreaForQuestMapBounds saBounds = sSpellFactoryMgr.GetSpellAreaForQuestMapBounds(qst->id, false);
    if (saBounds.first != saBounds.second)
    {
        for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            if (itr->second->autocast && itr->second->IsFitToRequirements(plr, plr->GetZoneId(), plr->GetAreaID()))
            {
                if (!plr->HasAura(itr->second->spellId))
                {
                    plr->CastSpell(plr, itr->second->spellId, true);
                }
            }
        }
    }

    LogDebug("BuildQuestComplete", "Called SMSG_QUESTGIVER_QUEST_COMPLETE");

    WorldPacket data(SMSG_QUESTGIVER_QUEST_COMPLETE, 72);

    data << uint32_t(0);                  // talents?
    data << uint32_t(0);                  // points?
    data << uint32_t(GenerateRewardMoney(plr, qst));
    data << uint32_t(xp);
    data << uint32_t(qst->id);
    data << uint32_t(0);                  // skill id?

    data.writeBit(0);                   // unk
    data.writeBit(1);
    data.flushBits();

    plr->SendPacket(&data);
}
