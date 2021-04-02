/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Log.hpp"
#include "QuestLogEntry.hpp"
#include "Server/WorldSession.h"
#include "Server/MainServerDefines.h"
#include "Database/Database.h"
#include "Management/ItemInterface.h"
#include "QuestMgr.h"


QuestLogEntry::QuestLogEntry(QuestProperties const* questProperties, Player* player, uint8_t slot) : m_questProperties(questProperties), m_player(player), m_slot(slot)
{
    if (m_questProperties->time > 0)
        m_expirytime = static_cast<uint32_t>(UNIXTIME + m_questProperties->time / 1000);
    else
        m_expirytime = 0;

    initPlayerData();
}

QuestLogEntry::~QuestLogEntry() {}

void QuestLogEntry::initPlayerData()
{
    for (uint8_t i = 0; i < 4; ++i)
    {
        if (m_questProperties->required_spell[i] != 0)
        {
            m_isCastQuest = true;

            if (!m_player->HasQuestSpell(m_questProperties->required_spell[i]))
                m_player->quest_spells.insert(m_questProperties->required_spell[i]);
        }
        else if (m_questProperties->required_emote[i] != 0)
        {
            m_isEmoteQuest = true;
        }

        if (m_questProperties->required_mob_or_go[i] != 0)
        {
            if (!m_player->HasQuestMob(m_questProperties->required_mob_or_go[i]))
                m_player->quest_mobs.insert(m_questProperties->required_mob_or_go[i]);
        }
    }

    m_player->setQuestLogInSlot(this, m_slot);

    if (!m_player->GetSession()->m_loggingInPlayer)
        CALL_QUESTSCRIPT_EVENT(this, OnQuestStart)(m_player, this);
}

void QuestLogEntry::loadFromDB(Field* fields)
{   
    //     1          2         3       4      5      6      7      8      9     10     11     12
    // playerguid, questid, timeleft, area0, area1, area2, area3, kill0, kill1, kill2, kill3, state

    m_expirytime = fields[3].GetUInt32();

    for (uint8_t i = 0; i < 4; ++i)
    {
        m_explored_areas[i] = fields[4 + i].GetUInt32();
        CALL_QUESTSCRIPT_EVENT(this, OnExploreArea)(m_explored_areas[i], m_player, this);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        m_mobcount[i] = fields[8 + i].GetUInt32();

        if (getQuestProperties()->required_mobtype[i] == QUEST_MOB_TYPE_CREATURE)
            CALL_QUESTSCRIPT_EVENT(this, OnCreatureKill)(getQuestProperties()->required_mob_or_go[i], m_player, this);
        else
            CALL_QUESTSCRIPT_EVENT(this, OnGameObjectActivate)(getQuestProperties()->required_mob_or_go[i], m_player, this);
    }

    m_state = fields[12].GetUInt32();
}

void QuestLogEntry::saveToDB(QueryBuffer* queryBuffer)
{
    std::stringstream ss;

    ss << "REPLACE INTO questlog VALUES(";
    ss << m_player->getGuidLow() << "," << m_questProperties->id << "," << uint32_t(m_slot) << "," << m_expirytime;

    for (uint8_t i = 0; i < 4; ++i)
        ss << "," << m_explored_areas[i];

    for (uint8_t i = 0; i < 4; ++i)
        ss << "," << m_mobcount[i];

    ss << "," << m_state << ");";

    if (queryBuffer == nullptr)
        CharacterDatabase.Execute(ss.str().c_str());
    else
        queryBuffer->AddQueryStr(ss.str());
}

uint8_t QuestLogEntry::getSlot() const { return m_slot; }
void QuestLogEntry::setSlot(uint8_t slot)
{
    if (slot > MAX_QUEST_LOG_SIZE)
    {
        LogError("%u is not a valid questlog slot!", uint32_t(slot));
        return;
    }

    m_slot = slot;
}

void QuestLogEntry::setStateComplete() { m_state = QUEST_COMPLETE; }

uint32_t QuestLogEntry::getMobCountByIndex(uint8_t index) const
{
    if (index >= 4)
    {
        LogError("%u is not a valid index for questlog mob count!", uint32_t(index));
        return 0;
    }

    return m_mobcount[index];
}

void QuestLogEntry::setMobCountForIndex(uint8_t index, uint32_t count)
{
    if (index >= 4)
    {
        LogError("%u is not a valid index for questlog mob count!", uint32_t(index));
        return;
    }

    m_mobcount[index] = count;
}

void QuestLogEntry::incrementMobCountForIndex(uint8_t index)
{
    if (index >= 4)
    {
        LogError("%u is not a valid index for questlog mob count!", uint32_t(index));
        return;
    }

    ++m_mobcount[index];
}

uint32_t QuestLogEntry::getExploredAreaByIndex(uint8_t index) const
{
    if (index >= 4)
    {
        LogError("%u is not a valid index for questlog explore areas!", uint32_t(index));
        return 0;
    }

    return m_explored_areas[index];
}

void QuestLogEntry::setExploredAreaForIndex(uint8_t index)
{
    if (index >= 4)
    {
        LogError("%u is not a valid index for questlog explore areas!", uint32_t(index));
        return;
    }
    m_explored_areas[index] = 1;
}

bool QuestLogEntry::isCastQuest() const { return m_isCastQuest; }
bool QuestLogEntry::isEmoteQuest() const { return m_isEmoteQuest; }

QuestProperties const* QuestLogEntry::getQuestProperties() const { return m_questProperties; }

bool QuestLogEntry::isUnitAffected(Unit* unit) const
{
    if (unit)
    {
        if (m_affected_units.find(unit->getGuid()) != m_affected_units.end())
            return true;
    }

    return false;
}

void QuestLogEntry::addAffectedUnit(Unit* unit)
{
    if (unit)
    {
        if (!isUnitAffected(unit))
            m_affected_units.insert(unit->getGuid());
    }
}

void QuestLogEntry::clearAffectedUnits()
{
    if (!m_affected_units.empty())
        m_affected_units.clear();
}

bool QuestLogEntry::canBeFinished() const
{
    if (m_questProperties->iscompletedbyspelleffect && m_state == QUEST_INCOMPLETE)
        return false;

    if (m_state == QUEST_FAILED)
        return false;
    
    if (m_state == QUEST_COMPLETE)
        return true;

    for (uint8_t i = 0; i < 4; ++i)
    {
        if (m_questProperties->required_mob_or_go[i])
        {
            if (m_mobcount[i] < m_questProperties->required_mob_or_go_count[i])
                return false;
        }

        if (m_questProperties->required_spell[i])
        {
            if (m_mobcount[i] == 0 || m_mobcount[i] < m_questProperties->required_mob_or_go_count[i])
                return false;
        }

        if (m_questProperties->required_emote[i])
        {
            if (m_mobcount[i] == 0 || m_mobcount[i] < m_questProperties->required_mob_or_go_count[i])
                return false;
        }
    }

    for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        if (m_questProperties->required_item[i])
        {
            if (m_player->getItemInterface()->GetItemCount(m_questProperties->required_item[i]) < m_questProperties->required_itemcount[i])
                return false;
        }
    }

    if (m_questProperties->reward_money < 0 && m_player->getCoinage() < uint32_t(-m_questProperties->reward_money))
        return false;

    for (uint8_t i = 0; i < 4; ++i)
    {
        if (m_questProperties->required_triggers[i])
        {
            if (m_explored_areas[i] == 0)
                return false;
        }
    }

    return true;
}

void QuestLogEntry::finishAndRemove()
{
    sEventMgr.RemoveEvents(m_player, EVENT_TIMED_QUEST_EXPIRE);

    m_player->setQuestLogEntryBySlot(m_slot, 0);
    m_player->setQuestLogStateBySlot(m_slot, 0);
    m_player->setQuestLogRequiredMobOrGoBySlot(m_slot, 0);
    m_player->setQuestLogExpireTimeBySlot(m_slot, 0);

    m_player->setQuestLogInSlot(nullptr, m_slot);
    m_player->PushToRemovedQuests(m_questProperties->id);
    m_player->UpdateNearbyGameObjects();

    delete this;
}

void QuestLogEntry::sendQuestFailed(bool isTimerExpired /*=false*/)
{
    sEventMgr.RemoveEvents(m_player, EVENT_TIMED_QUEST_EXPIRE);

    m_state = QUEST_FAILED;
    m_expirytime = 0;

    m_player->setQuestLogStateBySlot(m_slot, QLS_Failed);

    if (isTimerExpired)
        sQuestMgr.SendQuestUpdateFailedTimer(m_questProperties, m_player);
    else
        sQuestMgr.SendQuestUpdateFailed(m_questProperties, m_player);
}

void QuestLogEntry::updatePlayerFields()
{
    if (!m_player)
        return;

    m_player->setQuestLogEntryBySlot(m_slot, m_questProperties->id);
    uint32_t state = QLS_None;

#if VERSION_STRING > TBC
    uint64_t mobOrGoCount = 0;
#else
    uint32_t mobOrGoCount = 0;
#endif

    if (m_questProperties->count_requiredtriggers)
    {
        uint32_t count = 0;
        for (uint8_t i = 0; i < 4; ++i)
        {
            if (m_questProperties->required_triggers[i])
            {
                if (m_explored_areas[i] == 1)
                {
                    count++;
                }
            }
        }

        if (count == m_questProperties->count_requiredtriggers)
            mobOrGoCount |= 0x01000000;
    }

    if (m_isCastQuest)
    {
        bool cast_complete = true;

        for (uint8_t i = 0; i < 4; ++i)
        {
            if (m_questProperties->required_spell[i] && m_questProperties->required_mob_or_go_count[i] > m_mobcount[i])
            {
                cast_complete = false;
                break;
            }
        }

        if (cast_complete)
            state |= QLS_ObjectiveCompleted;
    }
    else if (m_isEmoteQuest)
    {
        bool emote_complete = true;

        for (uint8_t i = 0; i < 4; ++i)
        {
            if (m_questProperties->required_emote[i] && m_questProperties->required_mob_or_go_count[i] > m_mobcount[i])
            {
                emote_complete = false;
                break;
            }
        }

        if (emote_complete)
            state |= QLS_ObjectiveCompleted;
    }

    if (m_questProperties->count_required_mob)
    {
#if VERSION_STRING > TBC
        uint8_t* p = reinterpret_cast<uint8_t*>(&mobOrGoCount);
        for (uint8_t i = 0; i < 4; ++i)
        {
            if (m_questProperties->required_mob_or_go[i] && m_mobcount[i] > 0)
                p[2 * i] |= static_cast<uint8_t>(m_mobcount[i]);
        }
#else
        uint8_t* p = reinterpret_cast<uint8_t*>(&mobOrGoCount);
        for (uint8_t i = 0; i < 4; ++i)
        {
            if (m_questProperties->required_mob_or_go[i] && m_mobcount[i] > 0)
                p[i] |= static_cast<uint8_t>(m_mobcount[i]);
        }
#endif
    }

    if (m_questProperties->time != 0 && m_expirytime < UNIXTIME)
        m_state = QUEST_FAILED;

    if (m_state == QUEST_FAILED)
        state |= QLS_Failed;

    m_player->setQuestLogStateBySlot(m_slot, state);
    m_player->setQuestLogRequiredMobOrGoBySlot(m_slot, mobOrGoCount);

    if (m_questProperties->time != 0 && m_state != QUEST_FAILED)
    {
        m_player->setQuestLogExpireTimeBySlot(m_slot, m_expirytime);
        sEventMgr.AddEvent(m_player, &Player::EventTimedQuestExpire, m_questProperties->id, EVENT_TIMED_QUEST_EXPIRE, 
            (m_expirytime - UNIXTIME) * 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
    else
    {
        m_player->setQuestLogExpireTimeBySlot(m_slot, 0);
    }
}

void QuestLogEntry::sendQuestComplete()
{
    WorldPacket data(SMSG_QUESTUPDATE_COMPLETE, 4);
    data << m_questProperties->id;
    m_player->GetSession()->SendPacket(&data);

    m_player->UpdateNearbyGameObjects();

    CALL_QUESTSCRIPT_EVENT(this, OnQuestComplete)(m_player, this);
}

void QuestLogEntry::SendUpdateAddKill(uint8_t index)
{
    if (index >= 4)
    {
        LogError("%u is not a valid index for questlog mob count!", uint32_t(index));
        return;
    }

    sQuestMgr.SendQuestUpdateAddKill(m_player, m_questProperties->id, m_questProperties->required_mob_or_go[index], 
        m_mobcount[index], m_questProperties->required_mob_or_go_count[index], 0);
}
