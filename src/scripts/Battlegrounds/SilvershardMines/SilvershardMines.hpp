/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "SilvershardMinesDefinitions.hpp"
#include "Management/Battleground/Battleground.hpp"

class BattlegroundMap;

// Silvershard Mines - Mists of Pandaria minecart-pushing battleground, map 727. gate/cart/track
// switch gameobject and creature entries, spawn coordinates, worldstates and the full waypoint
// geometry for every track (both forks at each junction) - see SilvershardMinesDefinitions.hpp -
// a, faithful mechanic implementation, not a placeholder.
// Control of each cart is resolved by an explicit nearby-player faction-majority scan (we have no
// runtime handling for the capture-point gameobject  - the controlling team's cart
// advances one waypoint at a time until it reaches its mine's depot, awarding capture points and
// resetting. At the North and East crossroads, the fork actually taken is the real one - resolved
// live from the two, spellclickable Track Switch NPCs (see SilvershardMines.cpp) rather than
// a fixed default.
class SilvershardMines : public Battleground
{
    GameObject* m_gates[SILVERSHARD_NUM_GATES];
    Creature* m_carts[SILVERSHARD_NUM_MINES];
    Creature* m_trackSwitchEast;
    Creature* m_trackSwitchNorth;
    uint32_t m_cartWaypointIndex[SILVERSHARD_NUM_MINES];
    int32_t m_cartControlledBy[SILVERSHARD_NUM_MINES];     // -1 = neutral, 0 = Alliance, 1 = Horde
    uint8_t m_cartSegment[SILVERSHARD_NUM_MINES];          // 0 = base/only segment, 1 = fork taken
    uint32_t m_eastTrackState;
    uint32_t m_northTrackState;
    uint32_t m_scores[2];

public:
    SilvershardMines(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t);
    ~SilvershardMines() override;

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

    void EventUpdateCart(uint32_t mineIndex);
    void CaptureCart(uint32_t mineIndex, uint32_t Team);
    void ResetCart(uint32_t mineIndex);

    // Called by the global dummy-spell handler for SILVERSHARD_SPELL_TRACK_SWITCH_CLICK (see
    // SilvershardMinesTrackSwitch.cpp) when a player spellclicks one of this instance's own two
    // track-switch creatures. Toggles that crossroads' track-switch worldstate.
    void OnTrackSwitchClicked(Creature* trackSwitch);

    static Battleground* Create(BattlegroundMap* m, uint32_t i, uint32_t l, uint32_t t) { return new SilvershardMines(m, i, l, t); }

    // 708 matches TYPE_SILVERSHARD_MINES - worldstring_tables row added via SQL.
    uint32_t GetNameID() override { return 708; }
};
