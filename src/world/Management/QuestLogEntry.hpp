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

class SERVER_DECL QuestLogEntry : public EventableObject
{
    friend class QuestMgr;

public:
    QuestLogEntry();
    ~QuestLogEntry();

    inline QuestProperties const* GetQuest() { return m_quest; };
    void Init(QuestProperties const* quest, Player* plr, uint16_t slot);

    bool CanBeFinished();
    void Complete();
    void SaveToDB(QueryBuffer* buf);
    bool LoadFromDB(Field* fields);
    void UpdatePlayerFields();

    void SetTrigger(uint32_t i);
    void SetMobCount(uint32_t i, uint32_t count);
    void IncrementMobCount(uint32_t i);

    bool IsUnitAffected(Unit* target);
    inline bool IsCastQuest() { return iscastquest; }
    inline bool IsEmoteQuest() { return isemotequest; }
    void AddAffectedUnit(Unit* target);
    void ClearAffectedUnits();

    void SetSlot(uint16_t i);
    void Finish();


    //////////////////////////////////////////////////////////////////////////////////////////
    /// void Fail(bool timerexpired)
    /// Marks the quest as failed
    ///
    /// \param bool timerexpired  -  true if the reason for failure is timer expiration.
    ///
    /// \return none
    ///
    //////////////////////////////////////////////////////////////////////////////////////////
    void Fail(bool timerexpired);


    //////////////////////////////////////////////////////////////////////////////////////////
    /// bool HasFailed()
    /// Tells if the Quest has failed.
    ///
    /// \param none
    ///
    /// \return true if the quest has failed, false otherwise.
    ///
    //////////////////////////////////////////////////////////////////////////////////////////
    bool HasFailed();


    void SendQuestComplete();
    void SendUpdateAddKill(uint32_t i);
    inline uint32_t GetMobCount(uint32_t i) { return m_mobcount[i]; }
    inline uint32_t GetExploredAreas(uint32_t i) { return m_explored_areas[i]; }

    uint16_t GetBaseField(uint16_t slot) { return PLAYER_QUEST_LOG_1_1 + (slot * 5); }
    uint16_t GetSlot() { return m_slot; }

private:
    uint32_t completed;

    bool mInitialized;
    bool mDirty;

    QuestProperties const* m_quest;
    Player* m_plr;

    uint32_t m_mobcount[4];
    uint32_t m_explored_areas[4];

    std::set<uint64_t> m_affected_units;
    bool iscastquest;
    bool isemotequest;

    uint32_t expirytime;
    uint16_t m_slot;
};

#define CALL_QUESTSCRIPT_EVENT(obj, func) if (static_cast<QuestLogEntry*>(obj)->GetQuest()->pQuestScript != NULL) static_cast<QuestLogEntry*>(obj)->GetQuest()->pQuestScript->func
