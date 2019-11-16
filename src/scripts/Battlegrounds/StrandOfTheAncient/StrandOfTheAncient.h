/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Battleground/Battleground.h"
#include "StrandOfTheAncientDefinitions.h"


class StrandOfTheAncient : public CBattleground
{
private:

    uint32_t Attackers;   // 0 - horde / 1 - alliance
    uint32_t Defenders;
    uint32_t BattleRound;
    uint32_t RoundTime;
    uint32_t RoundFinishTime[2];
    SOTABattleRoundProgress roundprogress;
    GameObject* m_boats[4];
    GameObject* m_buffs[BUFF_COUNT];
    GameObject* m_relic;
    GameObject* m_endgate;
    GameObject* m_bomb[SOTA_BOMBS];
    GameObject* m_gates[GATE_COUNT];
    GameObject* m_gateSigils[GATE_COUNT];
    GameObject* m_gateTransporters[GATE_COUNT];
    //PassengerMap boat1Crew;
    //PassengerMap boat2Crew;
    Creature* npc[SOTA_NPCS];
    Creature* canon[SOTA_NUM_CANONS];
    Creature* demolisher[SOTA_NUM_DEMOLISHERS];

    SOTAControlPoint controlpoint[NUM_SOTA_CONTROL_POINTS];
    SOTAGraveyard graveyard[NUM_SOTA_GRAVEYARDS];

public:

    static CBattleground* Create(MapMgr* m, uint32_t i, uint32_t l, uint32_t t) { return new StrandOfTheAncient(m, i, l, t); }

    StrandOfTheAncient(MapMgr* mgr, uint32_t id, uint32_t lgroup, uint32_t t);
    ~StrandOfTheAncient();

    uint32_t GetNameID() override { return 34; }   //\todo in worldstring_tables ?

    uint32_t GetRoundTime() { return RoundTime; };
    LocationVector GetStartingCoords(uint32_t team) override;
    bool HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam) override;
    void HookOnAreaTrigger(Player* plr, uint32_t id) override;
    void HookFlagStand(Player* plr, GameObject* obj) override;
    void HookOnFlagDrop(Player* plr) override;
    void HookFlagDrop(Player* plr, GameObject* obj) override;
    void HookOnPlayerKill(Player* plr, Player* pVictim) override;
    void HookOnHK(Player* plr) override;
    void HookOnShadowSight() override;
    void HookGenerateLoot(Player* plr, Object* pOCorpse) override;
    void HookOnUnitKill(Player* plr, Unit* pVictim) override;
    void HookOnUnitDied(Unit* victim) override;
    bool HookSlowLockOpen(GameObject* go, Player* player, Spell* spell ) override;
    bool HookQuickLockOpen(GameObject* go, Player* player, Spell* spell ) override;
    void HookOnPlayerDeath(Player* plr) override;
    void HookOnMount(Player* plr) override;
    bool HookHandleRepop(Player* plr) override;
    void OnAddPlayer(Player* plr) override;
    void OnRemovePlayer(Player* plr) override;
    void OnPlatformTeleport(Player* plr);
    void OnCreate() override;
    void OnStart() override;
    void SetIsWeekend(bool isweekend) override;
    void SetRoundTime(uint32_t secs) { RoundTime = secs; };
    void SetTime(uint32_t secs);
    void TimeTick();
    void PrepareRound();
    void StartRound();
    void FinishRound();
    void Finish(uint32_t winningteam);

    void SpawnControlPoint(SOTAControlPoints point, SOTACPStates state);
    void CaptureControlPoint(SOTAControlPoints point);
    void SpawnGraveyard(SOTAGraveyards gyid, uint32_t team);

};
