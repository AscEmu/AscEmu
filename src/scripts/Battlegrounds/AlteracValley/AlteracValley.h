/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Battleground/Battleground.h"
#include "AlteracValleyDefinitions.h"

class AlteracValley : public CBattleground
{
protected:

    std::list<GameObject*> m_gates;
    uint32_t m_reinforcements[2];
    bool m_nearingVictory[2];
    std::map<Creature*, std::set<uint32_t> > Get_m_resurrectMap() const { return m_resurrectMap; }
public:

    AlteracValley(MapMgr* mgr, uint32_t id, uint32_t lgroup, uint32_t t);
    ~AlteracValley();

    void EventAssaultControlPoint(uint32_t x);

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
    LocationVector GetStartingCoords(uint32_t Team) override;
    void DropFlag(Player* plr);

    static CBattleground* Create(MapMgr* m, uint32_t i, uint32_t l, uint32_t t) { return new AlteracValley(m, i, l, t); }

    const char* GetName() { return "Alterac Valley"; }
    void OnStart() override;

    void EventUpdateResources();
    bool HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell* pSpell) override;

    // AV Functions
    void AddReinforcements(uint32_t teamId, uint32_t amt);
    void RemoveReinforcements(uint32_t teamId, uint32_t amt);
    void Finish(uint32_t losingTeam);

    // loot
    bool SupportsPlayerLoot() { return true; }
    void HookGenerateLoot(Player* plr, Object* pCorpse) override;

    // herald
    void Herald(const char* format, ...);

    void HookOnFlagDrop(Player* plr) override;
    void HookOnShadowSight() override;

    class AVNode
    {
        AlteracValley* m_bg;
        AVNodeTemplate* m_template;

        // boss, changes ownership upon death?
        Creature* m_boss;

        // guards, need to be respawned when changes ownership
        std::vector<Creature*> m_guards;

        ///\todo  peon locations, used in mines
        std::vector<Creature*> m_peonLocations;

        // control point (capturable)
        GameObject* m_flag;

        // aura (light-shiny stuff)
        GameObject* m_aura;
        GameObject* m_glow;

        // home NPc
        Creature* m_homeNPC;

        // destroyed flag (prevent all actions)
        bool m_destroyed;

        // state
        uint32_t m_state;
        uint32_t m_lastState;
        uint32_t m_nodeId;

        // spirit guides
        Creature* m_spiritGuide;

public:

        friend class AlteracValley;

        // constructor
        AVNode(AlteracValley* parent, AVNodeTemplate* tmpl, uint32_t node_id);
        ~AVNode();

        // initial spawn
        void Spawn();

        // assault
        void Assault(Player* plr);

        // capture event
        void Capture();

        // spawn guards
        void SpawnGuards(uint32_t x);

        // state change
        void ChangeState(uint32_t new_state);

        // spawn home buff guard
        void SpawnHomeGuard();
    };

protected:

    AVNode* m_nodes[AV_NUM_CONTROL_POINTS];
};
