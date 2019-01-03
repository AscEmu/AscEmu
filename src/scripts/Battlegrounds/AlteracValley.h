/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Battleground/Battleground.h"

enum
{
    // GENERAL AV DEFINES
    AV_NUM_REINFORCEMENTS = 600,           // Amount of reinforcements we start off with

    //AV_SCORE_WARNING = 530,                // Dunno what this should be ;p

    //AV_ADD_POINTS_ON_CONTROLLED_MINE = 1,  // Points to give the team who controls (a) mine(s)

    AV_REINFORCEMENT_ADD_INTERVAL = 45000, // The interval (in milliseconds) that points from mines are awarded
 
    AV_POINTS_ON_DESTROY_BUNKER = 75,      // Points to remove for destroying a team's bunker
    AV_POINTS_ON_KILL = 1,                 // Points to remove when killing a member of the opposite team
    AV_POINTS_ON_KILL_CAPTAIN = 100,       // Points  to remove for killing a team's captain

    AV_HONOR_ON_KILL_BOSS = 62,            // Amount of honor awarded to players for killing a boss of the opposite team

    //AV_NUM_CONTESTED_AREAS = 9,            // Total contested areas (graveyards/mines)
    //AV_NUM_DESTROYABLE_AREAS = 8,          // Total destroyable areas (towers/bunkers)
    //AV_NUM_BOSS_UNITS = 14,                // Boss units (generals/captains/wing commanders etc)
    //AV_NUM_COLDTOOTH_UNITS = 5,            // Coldtooth mine NPC types
    //AV_NUM_IRONDEEP_UNITS = 4,             // Irondeep mine NPC types
    //AV_NUM_SNOWFALL_FLAGS = 2,             // Count of Snowfall flags (used when changing them on a team's acquiration of the CP

    //AV_CONTESTED_AREAS_START = 0,          // ID at which contested points start (for loops/ifs)
    //AV_CONTESTED_AREAS_END = 8,            // ID at which contested points end (for loops/ifs)

    //AV_DESTROYABLE_AREAS_START = 9,        // ID at which destroyable points start (for loops/ifs)
    //AV_DESTROYABLE_AREAS_END = 16,         // ID at which contested points finish (for loops/ifs)

};

enum AVControlPoints
{
    AV_CONTROL_POINT_STORMPIKE_AID_STATION = 0,
    AV_CONTROL_POINT_STORMPIKE_GRAVEYARD = 1,
    AV_CONTROL_POINT_STONEHEARTH_GRAVEYARD = 2,
    AV_CONTROL_POINT_SNOWFALL_GRAVEYARD = 3,
    AV_CONTROL_POINT_COLDTOOTH_MINE = 4,
    AV_CONTROL_POINT_IRONDEEP_MINE = 5,
    AV_CONTROL_POINT_ICEBLOOD_GRAVEYARD = 6,
    AV_CONTROL_POINT_FROSTWOLF_GRAVEYARD = 7,
    AV_CONTROL_POINT_FROSTWOLF_RELIEF_HUT = 8,
    AV_CONTROL_POINT_DUN_BALDAR_NORTH_BUNKER = 9,
    AV_CONTROL_POINT_DUN_BALDAR_SOUTH_BUNKER = 10,
    AV_CONTROL_POINT_ICEWING_BUNKER = 11,
    AV_CONTROL_POINT_STONEHEARTH_BUNKER = 12,
    AV_CONTROL_POINT_ICEBLOOD_TOWER = 13,
    AV_CONTROL_POINT_TOWER_POINT = 14,
    AV_CONTROL_POINT_EAST_FROSTWOLF_TOWER = 15,
    AV_CONTROL_POINT_WEST_FROSTWOLF_TOWER = 16,
    AV_NUM_CONTROL_POINTS = 17
};

enum AVSpawnTypes
{
    AV_SPAWN_TYPE_NEUTRAL = 0,
    AV_SPAWN_TYPE_ALLIANCE_ASSAULT = 1,
    AV_SPAWN_TYPE_HORDE_ASSAULT = 2,
    AV_SPAWN_TYPE_ALLIANCE_CONTROLLED_OR_DESTROYED = 3,
    AV_SPAWN_TYPE_HORDE_CONTROLLED_OR_DESTROYED = 4,
    AV_NUM_SPAWN_TYPES = 5
};

enum AVNodeStates
{
    AV_NODE_STATE_NEUTRAL_CONTROLLED = 0,
    AV_NODE_STATE_ALLIANCE_ASSAULTING = 1,
    AV_NODE_STATE_ALLIANCE_CONTROLLED = 2,
    AV_NODE_STATE_HORDE_ASSAULTING = 3,
    AV_NODE_STATE_HORDE_CONTROLLED = 4,
    AV_NODE_STATE_COUNT = 5
};

struct AVLocation { float x; float y; float z; };
struct AVSpawnLocation { float x; float y; float z; float o; };
struct AVGameObject { uint32 id[AV_NODE_STATE_COUNT]; float x; float y; float z; float o; float rot1; float rot2; };
struct AVNodeTemplate
{
    const char* m_name;                             // Stormpike Aid Station
    bool m_isGraveyard;                             // Is this a graveyard?
    bool m_capturable;                              // Is this capturable?
    AVLocation m_graveyardLocation;                 // Revive location, also location of spirit guide
    AVGameObject m_flagLocation;                    // Flag location (need to add GO id/properties here)
    AVGameObject m_auraLocation;                    // Aura location
    AVGameObject m_glowLocation;                    // Aura glow location
    uint32 m_guardId[3];                            // Horde/alliance guard ids
    uint32 m_guardCount;                            // Count of guards to spawn
    uint32 m_bossId[3];                             // Boss ID (e.g. Balinda), 0 = A, 1 = H, 2 = Neutral
    AVLocation* m_peonLocations;                    // Used by mines.
    AVLocation m_bossLocation;                      // Location of boss if there is one
    uint32 m_initialSpawnId;                        // Initial spawn (Bowmen) ID
    uint32 m_worldStateFields[AV_NODE_STATE_COUNT]; // State fields
    uint32 m_defaultState;                          // State of the node when battleground is spawned
};

enum GameObjectEntry
{
    AV_GAMEOBJECT_FIRE = 179065,
    AV_GAMEOBJECT_GATE = 180424
};

enum CreatureEntry
{
    AV_NPC_GENERAL_VANNDAR_STORMPIKE = 11948,
    AV_NPC_CAPTAIN_BALINDA_STONEHEARTH = 11949,
    AV_NPC_ARCH_DRUID_RENFERAL = 13442,
    AV_NPC_WING_COMMANDER_SLIDORE = 13438,
    AV_NPC_WING_COMMANDER_VIPORE = 13439,
    AV_NPC_WING_COMMANDER_ICHMAN = 13437,

    AV_NPC_GENERAL_DREK_THAR = 11946,
    AV_NPC_CAPTAIN_GALVANGAR = 11947,
    AV_NPC_PRIMALIST_THURLOGA = 13236,
    AV_NPC_WING_COMMANDER_GUSE = 13179,
    AV_NPC_WING_COMMANDER_JEZTOR = 13180,
    AV_NPC_WING_COMMANDER_MULVERICK = 13181,

    AV_NPC_IVUS_FOREST_LORD = 13419,
    AV_NPC_LOKHOLAR_ICE_LORD = 13256,

    AV_NPC_TASKMASTER_SNIVVLE = 11677,
    AV_NPC_WHITEWISKER_DIGGER = 11603,
    AV_NPC_WHITEWISKER_GEOMANCER = 11604,
    AV_NPC_WHITEWISKER_OVERSEER = 11605,
    AV_NPC_WHITEWISKER_VERMIN = 10982,

    AV_NPC_MORLOCH = 11657,
    AV_NPC_IRONDEEP_TROGG = 10987,
    AV_NPC_IRONDEEP_SHAMAN = 11600,
    AV_NPC_IRONDEEP_SKULLTHUMPER = 11602
};


class AlteracValley : public CBattleground
{
protected:

    std::list<GameObject*> m_gates;
    uint32 m_reinforcements[2];
    bool m_nearingVictory[2];
    inline std::map<Creature*, std::set<uint32> > Get_m_resurrectMap() { return CBattleground::m_resurrectMap; }
public:

    AlteracValley(MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t);
    ~AlteracValley();

    void EventAssaultControlPoint(uint32 x);

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
    LocationVector GetStartingCoords(uint32 Team) override;
    void DropFlag(Player* plr);

    static CBattleground* Create(MapMgr* m, uint32 i, uint32 l, uint32 t) { return new AlteracValley(m, i, l, t); }

    const char* GetName() { return "Alterac Valley"; }
    void OnStart() override;

    void EventUpdateResources();
    bool HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell* pSpell) override;

    // AV Functions
    void AddReinforcements(uint32 teamId, uint32 amt);
    void RemoveReinforcements(uint32 teamId, uint32 amt);
    void Finish(uint32 losingTeam);

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
        uint32 m_state;
        uint32 m_lastState;
        uint32 m_nodeId;

        // spirit guides
        Creature* m_spiritGuide;

public:

        friend class AlteracValley;

        // constructor
        AVNode(AlteracValley* parent, AVNodeTemplate* tmpl, uint32 node_id);
        ~AVNode();

        // initial spawn
        void Spawn();

        // assault
        void Assault(Player* plr);

        // capture event
        void Capture();

        // spawn guards
        void SpawnGuards(uint32 x);

        // state change
        void ChangeState(uint32 new_state);

        // spawn home buff guard
        void SpawnHomeGuard();
    };

protected:

    AVNode* m_nodes[AV_NUM_CONTROL_POINTS];
};
