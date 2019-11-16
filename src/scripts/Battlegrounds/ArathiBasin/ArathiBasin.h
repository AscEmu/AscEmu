/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Battleground/Battleground.h"
#include "ArathiBasinDefinitions.h"


class ArathiBasin : public CBattleground
{
public:

    GameObject* m_buffs[AB_NUM_BUFFS];
    GameObject* m_controlPoints[AB_NUM_CONTROL_POINTS];
    GameObject* m_controlPointAuras[AB_NUM_CONTROL_POINTS];

protected:

    std::list<GameObject*> m_gates;

    uint32_t m_resources[2];
    uint32_t m_capturedBases[2];
    uint32_t m_lastHonorGainResources[2];
    uint32_t m_lastRepGainResources[2];
    int32_t m_basesOwnedBy[AB_NUM_CONTROL_POINTS];
    int32_t m_basesLastOwnedBy[AB_NUM_CONTROL_POINTS];
    int32_t m_basesAssaultedBy[AB_NUM_CONTROL_POINTS];
    Creature* m_spiritGuides[AB_NUM_CONTROL_POINTS];
    bool m_nearingVictory[2];
    uint32_t m_lgroup;
    bool DefFlag[AB_NUM_CONTROL_POINTS][2];

public:

    ArathiBasin(MapMgr* mgr, uint32_t id, uint32_t lgroup, uint32_t t);
    ~ArathiBasin();

    bool HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam) override;
    void HookOnPlayerDeath(Player* plr) override;
    void HookFlagDrop(Player* plr, GameObject* obj) override;
    void HookFlagStand(Player* plr, GameObject* obj) override;
    void HookOnMount(Player* plr) override;
    void HookOnAreaTrigger(Player* plr, uint32_t trigger) override;
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

    static CBattleground* Create(MapMgr* m, uint32_t i, uint32_t l, uint32_t t) { return new ArathiBasin(m, i, l, t); }

    uint32_t GetNameID() override { return 40; }
    void OnStart() override;

    void EventUpdateResources(uint32_t Team);
    bool HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell* pSpell) override;

    // AB Game Mechanics
    void SpawnControlPoint(uint32_t Id, uint32_t Type);
    void CaptureControlPoint(uint32_t Id, uint32_t Team);
    void AssaultControlPoint(Player* pPlayer, uint32_t Id);

    void SetIsWeekend(bool isweekend) override;
};
