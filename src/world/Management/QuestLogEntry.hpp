/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <Database/Field.hpp>
#include "CommonDefines.hpp"
#include "Server/EventableObject.h"
#include "Management/Quest.h"
#include "Units/Players/Player.h"
#include "Server/Script/ScriptMgr.h"

enum QuestLogState : uint32_t
{
    QLS_None = 0x00,
    QLS_Failed = 0x02,
    QLS_Accepted = 0x04,
    QLS_ObjectiveCompleted = 0x01000000
};

class SERVER_DECL QuestLogEntry : public EventableObject
{
    friend class QuestMgr;

public:
    QuestLogEntry(QuestProperties const* questProperties, Player* player, uint8_t slot);
    ~QuestLogEntry();

    void initPlayerData();

    void loadFromDB(Field* fields);
    void saveToDB(QueryBuffer* queryBuffer);

    uint8_t getSlot() const;
    void setSlot(uint8_t slot);

    void setStateComplete();

    uint32_t getMobCountByIndex(uint8_t index) const;
    void setMobCountForIndex(uint8_t index, uint32_t count);
    void incrementMobCountForIndex(uint8_t index);

    uint32_t getExploredAreaByIndex(uint8_t index) const;
    void setExploredAreaForIndex(uint8_t index);

    bool isCastQuest() const;
    bool isEmoteQuest() const;

    QuestProperties const* getQuestProperties() const;

    bool isUnitAffected(Unit* unit) const;
    void addAffectedUnit(Unit* unit);
    void clearAffectedUnits();

    bool canBeFinished() const;
    void finishAndRemove();
    void sendQuestFailed(bool isTimerExpired = false);

    void updatePlayerFields();
    void sendQuestComplete();
    void SendUpdateAddKill(uint8_t index);

private:

    uint8_t m_slot = 0;
    uint32_t m_state = 0;
    uint32_t m_mobcount[4] = {0};
    uint32_t m_explored_areas[4] = {0};
    uint32_t m_expirytime = 0;

    bool m_isCastQuest = false;
    bool m_isEmoteQuest = false;

    QuestProperties const* m_questProperties = nullptr;
    Player* m_player = nullptr;

    std::set<uint64_t> m_affected_units;
};

#define CALL_QUESTSCRIPT_EVENT(obj, func) if (static_cast<QuestLogEntry*>(obj)->getQuestProperties()->pQuestScript != NULL) static_cast<QuestLogEntry*>(obj)->getQuestProperties()->pQuestScript->func
