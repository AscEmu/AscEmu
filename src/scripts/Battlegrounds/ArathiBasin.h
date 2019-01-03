/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Battleground/Battleground.h"

enum
{
    //BASE_RESOURCES_GAIN = 10,
    RESOURCES_WARNING_THRESHOLD = 1400,
    RESOURCES_WINVAL = 1600,

    AB_BUFF_RESPAWN_TIME = 90000
};

enum ABBuffs
{
    AB_BUFF_STABLES = 0,
    AB_BUFF_BLACKSMITH = 1,
    AB_BUFF_FARM = 2,
    AB_BUFF_LUMBERMILL = 3,
    AB_BUFF_MINE = 4,
    AB_NUM_BUFFS = 5
};

enum ABControlPoints
{
    AB_CONTROL_POINT_STABLE = 0,
    AB_CONTROL_POINT_FARM = 1,
    AB_CONTROL_POINT_BLACKSMITH = 2,
    AB_CONTROL_POINT_MINE = 3,
    AB_CONTROL_POINT_LUMBERMILL = 4,
    AB_NUM_CONTROL_POINTS = 5
};

enum ABSpawnTypes
{
    AB_SPAWN_TYPE_NEUTRAL = 0,
    AB_SPAWN_TYPE_ALLIANCE_ASSAULT = 1,
    AB_SPAWN_TYPE_HORDE_ASSAULT = 2,
    AB_SPAWN_TYPE_ALLIANCE_CONTROLLED = 3,
    AB_SPAWN_TYPE_HORDE_CONTROLLED = 4,
    AB_NUM_SPAWN_TYPES = 5
};

class ArathiBasin : public CBattleground
{
public:

    GameObject* m_buffs[AB_NUM_BUFFS];
    GameObject* m_controlPoints[AB_NUM_CONTROL_POINTS];
    GameObject* m_controlPointAuras[AB_NUM_CONTROL_POINTS];

protected:

    std::list<GameObject*> m_gates;

    uint32 m_resources[2];
    uint32 m_capturedBases[2];
    uint32 m_lastHonorGainResources[2];
    uint32 m_lastRepGainResources[2];
    int32 m_basesOwnedBy[AB_NUM_CONTROL_POINTS];
    int32 m_basesLastOwnedBy[AB_NUM_CONTROL_POINTS];
    int32 m_basesAssaultedBy[AB_NUM_CONTROL_POINTS];
    Creature* m_spiritGuides[AB_NUM_CONTROL_POINTS];
    bool m_nearingVictory[2];
    uint32 m_lgroup;
    bool DefFlag[AB_NUM_CONTROL_POINTS][2];

public:

    ArathiBasin(MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t);
    ~ArathiBasin();

    bool HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam) override;
    void HookOnPlayerDeath(Player* plr) override;
    void HookFlagDrop(Player* plr, GameObject* obj) override;
    void HookFlagStand(Player* plr, GameObject* obj) override;
    void HookOnMount(Player* plr) override;
    void HookOnAreaTrigger(Player* plr, uint32 trigger) override;
    bool HookHandleRepop(Player* plr) override;
    void OnAddPlayer(Player* plr) override;
    void OnRemovePlayer(Player* plr) override;
    void OnCreate() override;
    void HookOnPlayerKill(Player* plr, Player* pVictim) override;
    void HookOnUnitKill(Player* plr, Unit* pVictim) override;
    void HookOnHK(Player* plr) override;
    void HookOnShadowSight() override;
    void HookGenerateLoot(Player* plr, Object* pCorpse) override;
    void SpawnBuff(uint32 x);
    LocationVector GetStartingCoords(uint32 Team) override;
    void HookOnFlagDrop(Player* plr) override;

    static CBattleground* Create(MapMgr* m, uint32 i, uint32 l, uint32 t) { return new ArathiBasin(m, i, l, t); }

    uint32 GetNameID() override { return 40; }
    void OnStart() override;

    void EventUpdateResources(uint32 Team);
    bool HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell* pSpell) override;

    // AB Game Mechanics
    void SpawnControlPoint(uint32 Id, uint32 Type);
    void CaptureControlPoint(uint32 Id, uint32 Team);
    void AssaultControlPoint(Player* pPlayer, uint32 Id);

    void SetIsWeekend(bool isweekend) override;
};
