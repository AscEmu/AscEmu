/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Battleground/Battleground.h"

enum
{
    TIME_LEFT = 25,
    //TIME_FOCUSED_ASSAULT = 10,
    //TIME_BRUTAL_ASSAULT = 15,

    BUFF_RESPAWN_TIME = 90000,

    SILVERWING_FLAG = 179785,

    WARSONG_FLAG = 179786,

    //SPELL_FOCUSED_ASSAULT = 46392,
    //SPELL_BRUTAL_ASSAULT = 46393,
};

enum WarsongGulchAreaTriggers
{
    AREATRIGGER_A_SPEED = 3686,
    AREATRIGGER_H_SPEED = 3687,
    AREATRIGGER_A_RESTORATION = 3706,
    AREATRIGGER_H_RESTORATION = 3708,
    AREATRIGGER_A_BERSERKING = 3707,
    AREATRIGGER_H_BERSERKING = 3709,
    AREATRIGGER_WSG_ENCOUNTER_01 = 3649,
    AREATRIGGER_WSG_ENCOUNTER_02 = 3688,
    AREATRIGGER_WSG_ENCOUNTER_03 = 4628,
    AREATRIGGER_WSG_ENCOUNTER_04 = 4629,
    AREATRIGGER_WSG_A_SPAWN = 3646,
    AREATRIGGER_WSG_H_SPAWN = 3647
};

class WarsongGulch : public CBattleground
{
    GameObject* m_buffs[6];
    GameObject* m_homeFlags[2];
    GameObject* m_dropFlags[2];
    uint32_t m_flagHolders[2];
    std::list<GameObject*> m_gates;
    uint32_t m_scores[2];
    uint32_t m_lgroup;
    uint8_t m_time_left;

    void TimeLeft();

public:

    WarsongGulch(MapMgr* mgr, uint32_t id, uint32_t lgroup, uint32_t t);
    ~WarsongGulch();

    bool HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam) override;
    void HookOnPlayerDeath(Player* plr) override;
    void HookFlagDrop(Player* plr, GameObject* obj) override;
    void HookFlagStand(Player* plr, GameObject* obj) override;
    void HookOnMount(Player* plr) override;
    void HookOnAreaTrigger(Player* plr, uint32_t id) override;
    bool HookHandleRepop(Player* plr) override;
    void OnAddPlayer(Player* plr) override;
    void OnRemovePlayer(Player* plr) override;
    void OnCreate() override;
    void HookOnPlayerKill(Player* plr, Player* pVictim) override;
    void HookOnUnitKill(Player* plr, Unit* pVictim) override;
    void HookOnHK(Player* plr) override;
    void HookOnShadowSight() override;
    void HookGenerateLoot(Player* plr, Object* pCorpse) override;
    void SpawnBuff(uint32_t x);
    LocationVector GetStartingCoords(uint32_t Team) override;
    void HookOnFlagDrop(Player* plr) override;
    void ReturnFlag(PlayerTeam team);

    void EventReturnFlags();

    static CBattleground* Create(MapMgr* m, uint32_t i, uint32_t l, uint32_t t) { return new WarsongGulch(m, i, l, t); }

    uint32_t GetNameID() override { return 39; }
    uint64_t GetFlagHolderGUID(uint32_t faction) const override { return m_flagHolders[faction]; }
    void OnStart() override;

    void SetIsWeekend(bool isweekend) override;
    void DespawnGates(uint32_t delay);
};
