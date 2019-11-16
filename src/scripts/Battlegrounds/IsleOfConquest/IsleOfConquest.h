/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Battleground/Battleground.h"
#include "Units/Creatures/Vehicle.h"
#include "IsleOfConquestDefinitions.h"


class IsleOfConquest : public CBattleground
{
public:

    IsleOfConquest(MapMgr* mgr, uint32_t id, uint32_t lgroup, uint32_t t);
    ~IsleOfConquest();

    static CBattleground* Create(MapMgr* m, uint32_t i, uint32_t l, uint32_t t) { return new IsleOfConquest(m, i, l, t); }

    void Init();
    void OnCreate() override;
    void OnStart() override;
    void OpenGates();
    void CloseGates();
    void SpawnControlPoint(uint32_t Id, uint32_t Type);
    void SpawnGraveyard(uint32_t id, uint32_t team);
    void Finish(uint32_t losingTeam);
    bool HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam) override;
    void HookOnAreaTrigger(Player* plr, uint32_t id) override;
    void HookOnPlayerDeath(Player* plr) override;
    void HookOnPlayerResurrect(Player* player) override;
    void HookOnPlayerKill(Player* /*plr*/, Player* /*pVictim*/) override {}
    void HookFlagDrop(Player* /*plr*/, GameObject* /*obj*/) override {}
    void HookOnFlagDrop(Player* /*plr*/) override {}
    void HookFlagStand(Player* /*plr*/, GameObject* /*obj*/) override {}
    bool HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell *pSpell) override;
    void HookOnMount(Player* /*plr*/)  override {}
    void HookGenerateLoot(Player* /*plr*/, Object* /*pCorpse*/) override {}
    void OnAddPlayer(Player* plr) override;
    void OnRemovePlayer(Player* plr) override;
    void HookOnShadowSight() override;
    void SetIsWeekend(bool isweekend) override;
    void HookOnUnitKill(Player* plr, Unit* pVictim) override;
    void HookOnUnitDied(Unit* victim) override;
    LocationVector GetStartingCoords(uint32_t Team) override;
    void AddReinforcements(uint32_t team, uint32_t amount);
    void RemoveReinforcements(uint32_t team, uint32_t amount);
    void UpdateResources();
    void HookOnHK(Player* plr) override;
    void AssaultControlPoint(Player* player, uint32_t id);
    void CaptureControlPoint(uint32_t id);
    bool HookHandleRepop(Player* plr) override;
    void BuildWorkshopVehicle(uint32_t delay);

    // Capture events
    void EventRefineryCaptured();
    void EventQuarryCaptured();
    void EventDocksCaptured();
    void EventHangarCaptured();
    void EventWorkshopCaptured();
    void EventAllianceKeepCaptured();
    void EventHordeKeepCaptured();

private:

    IOCTeleporter teleporter[IOC_NUM_TELEPORTERS];
    IOCControlPoint controlpoint[IOC_NUM_CONTROL_POINTS];
    IOCGate gates[MAX_PLAYER_TEAMS][IOC_NUM_GATES_PER_TEAM];
    IOCVehicle workshopvehicle[MAX_PLAYER_TEAMS];
    IOCVehicle workshopdemolisher[MAX_PLAYER_TEAMS][IOC_NUM_WORKSHOP_DEMOLISHERS];
    IOCVehicle dockvehicle[MAX_PLAYER_TEAMS][IOC_NUM_DOCKVEHICLES];
    IOCGraveyard graveyards[IOC_NUM_GRAVEYARDS];
    GameObject* towergates[MAX_PLAYER_TEAMS][2];
    Unit* generals[MAX_PLAYER_TEAMS];
    uint32_t m_reinforcements[MAX_PLAYER_TEAMS];
};
