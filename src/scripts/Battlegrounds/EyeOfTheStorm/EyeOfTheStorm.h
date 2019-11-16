/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Battleground/Battleground.h"

enum
{
    EOTS_GO_BE_TOWER = 184080, // 184080 - Blood Elf Tower Cap Pt
    EOTS_GO_FELREAVER = 184081, // 184081 - Fel Reaver Cap Pt
    EOTS_GO_MAGE_TOWER = 184082, // 184082 - Human Tower Cap Pt
    EOTS_GO_DRAENEI_TOWER = 184083, // 184083 - Draenei Tower Cap Pt

    EOTS_TOWER_BE = 0,
    EOTS_TOWER_FELREAVER = 1,
    EOTS_TOWER_MAGE = 2,
    EOTS_TOWER_DRAENEI = 3,

    EOTS_BANNER_NEUTRAL = 184382,
    EOTS_BANNER_ALLIANCE = 184381,
    EOTS_BANNER_HORDE = 184380,

    EOTS_CAPTURE_DISTANCE = 900, /*30*/

    EOTS_CAPTURE_RATE = 4,
    EOTS_TOWER_COUNT = 4,
    EOTS_RECENTLY_DROPPED_FLAG = 50327,
    EOTS_BUFF_RESPAWN_TIME = 90000,
    EOTS_NETHERWING_FLAG_SPELL = 34976,
    EOTS_NETHERWING_FLAG_READY = 2757,
};

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
