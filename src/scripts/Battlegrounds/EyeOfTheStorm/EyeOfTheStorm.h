/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Battleground/Battleground.h"
#include "EyeOfTheStormDefinitions.h"


class EyeOfTheStorm : public CBattleground
{
public:

    EyeOfTheStorm(MapMgr* mgr, uint32_t id, uint32_t lgroup, uint32_t t);
    ~EyeOfTheStorm();

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
    static CBattleground* Create(MapMgr* m, uint32_t i, uint32_t l, uint32_t t) { return new EyeOfTheStorm(m, i, l, t); }
    uint64_t GetFlagHolderGUID(uint32_t /*faction*/) const override { return m_flagHolder; }

    uint32_t GetNameID() override { return 44; }
    void OnStart() override;

    void UpdateCPs();
    void GeneratePoints();

    // returns true if that team won
    bool GivePoints(uint32_t team, uint32_t points);

    void RespawnCPFlag(uint32_t i, uint32_t id); // 0 = Neutral, <0 = Leaning towards alliance, >0 Leaning towards horde

    bool HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell* pSpell) override;
    void DropFlag2(Player* plr, uint32_t id);
    void HookOnFlagDrop(Player* plr) override;
    void EventResetFlag();
    void RepopPlayersOfTeam(int32_t team, Creature* sh);

    void SetIsWeekend(bool isweekend) override;

protected:

    int32_t m_CPStatus[EOTS_TOWER_COUNT];
    uint32_t m_flagHolder;

    GameObject* m_standFlag;
    GameObject* m_dropFlag;

    GameObject* m_CPStatusGO[EOTS_TOWER_COUNT];
    GameObject* m_CPBanner[EOTS_TOWER_COUNT];
    GameObject* m_CPBanner2[EOTS_TOWER_COUNT];
    GameObject* m_CPBanner3[EOTS_TOWER_COUNT];
    GameObject* m_bubbles[2];
    GameObject* EOTSm_buffs[4];

    typedef std::set<Player*> EOTSCaptureDisplayList;
    EOTSCaptureDisplayList m_CPDisplay[EOTS_TOWER_COUNT];

    uint32_t m_lastHonorGainPoints[2];
    uint32_t m_points[2];
    Creature* m_spiritGuides[EOTS_TOWER_COUNT];
};
