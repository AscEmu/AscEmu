/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "TempleOfKotmoguDefinitions.hpp"
#include "Management/Battleground/Battleground.hpp"

class BattlegroundMap;

// Temple of Kotmogu - Mists of Pandaria orb-carrying battleground, map 998. Real orb/door gameobject
// entries, spawn coordinates, worldstates and tick-scoring data - see TempleOfKotmoguDefinitions.hpp -
// a faithful mechanic implementation, not a placeholder.
// Each team scores on a timer while holding at least one orb, ticking faster the more orbs it
// holds simultaneously (0-4); a player can carry only one orb at a time (real Blizzard rule) and
// drops it - it respawns at its own pad immediately - on death.
class TempleOfKotmogu : public Battleground
{
    GameObject* m_doors[2];
    GameObject* m_orbs[KOTMOGU_NUM_ORBS];
    uint64_t m_orbCarrier[KOTMOGU_NUM_ORBS];
    uint32_t m_orbsHeld[2];
    uint32_t m_scores[2];

public:
    TempleOfKotmogu(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t);
    ~TempleOfKotmogu() override;

    void HookOnPlayerDeath(Player* plr) override;
    bool HookHandleRepop(Player* plr) override;
    void HookOnMount(Player* plr) override;
    void HookFlagDrop(Player* plr, GameObject* obj) override;
    void HookFlagStand(Player* plr, GameObject* obj) override;
    void HookOnFlagDrop(Player* plr) override;
    void HookOnPlayerKill(Player* plr, Player* pVictim) override;
    void HookOnHK(Player* plr) override;
    void HookOnAreaTrigger(Player* plr, uint32_t id) override;
    void HookOnShadowSight() override;
    void HookGenerateLoot(Player* plr, Object* pCorpse) override;
    void HookOnUnitKill(Player* plr, Unit* pVictim) override;
    void OnAddPlayer(Player* plr) override;
    void OnCreate() override;
    void OnRemovePlayer(Player* plr) override;
    void OnStart() override;

    LocationVector GetStartingCoords(uint32_t Team) override;

    void EventUpdateResources(uint32_t Team);
    void ReturnOrb(uint32_t orbIndex);

    static Battleground* Create(BattlegroundMap* m, uint32_t i, uint32_t l, uint32_t t) { return new TempleOfKotmogu(m, i, l, t); }

    // 699 matches TYPE_TEMPLE_OF_KOTMOGU - worldstring_tables row added via SQL.
    uint32_t GetNameID() override { return 699; }
};
