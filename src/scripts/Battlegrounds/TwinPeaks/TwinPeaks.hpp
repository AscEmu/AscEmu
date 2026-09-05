/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "TwinPeaksDefinitions.hpp"
#include "Management/Battleground/Battleground.hpp"

class BattlegroundMap;

// Twin Peaks - Cataclysm capture-the-flag battleground, map 726. Mechanically identical to
// Warsong Gulch (same 6-buff/2-flag/first-to-3 design), just a different map - this class
// mirrors WarsongGulch.cpp's structure and logic almost verbatim, with Twin Peaks' own real
// gameobject entries/coordinates (spawn-confirmed via a Mop-era reference database's own
// creature/gameobject tables, cross-checked against a Cata-era reference for entry/type
// compatibility) and its own real AreaTrigger.dbc entries in place of Warsong Gulch's.
class TwinPeaks : public Battleground
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
    TwinPeaks(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t);
    ~TwinPeaks();

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

    static Battleground* Create(BattlegroundMap* m, uint32_t i, uint32_t l, uint32_t t) { return new TwinPeaks(m, i, l, t); }

    // TODO: 108 is a placeholder, not a sourced worldstring_tables id (unlike WSG's real id=39)
    // - no "Twin Peaks" row exists in worldstring_tables yet, needs a real one added via SQL.
    uint32_t GetNameID() override { return 108; }
    uint64_t GetFlagHolderGUID(uint32_t faction) const override { return m_flagHolders[faction]; }
    void OnStart() override;

    void SetIsWeekend(bool isweekend) override;
    void DespawnGates(uint32_t delay);
};
