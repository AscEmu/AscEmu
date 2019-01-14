/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Battleground/Battleground.h"

enum
{
    BUFF_COUNT = 3,
    SOTA_NPCS = 3,

    SOTA_NUM_BOMBS = 66,
    SOTA_NORTH_BOMBS = 22,

    SOTA_EAST_BOMBS_INDEX = 22,
    SOTA_WEST_BOMBS_INDEX = 31,

    SOTA_BOAT_ALLIANCE_W = 193182,
    SOTA_BOAT_ALLIANCE_E = 193185,
    SOTA_BOAT_HORDER_W = 193184,
    SOTA_BOAT_HORDER_E = 193183,

    SOTA_DEMOLISHER = 28781,
    SOTA_ANTI_PERSONNAL_CANNON = 27894,
    SOTA_RIGGER_SPARKLIGHT = 29260,
    SOTA_GORGRIL_RIGSPARK = 29262,
    SOTA_BOMBS = 190753,
    //SOTA_KANRETHAD = 29,

    SOTA_NUM_CANONS = 10,
    SOTA_NUM_DEMOLISHERS = 8,

    SOTA_NORTH_DEMOLISHERS = 4,

    SOTA_EAST_WS_DEMOLISHER_INDEX = 6,
    SOTA_WEST_WS_DEMOLISHER_INDEX = 4,

    //SOTA_SPELL_TELEPORT_DEFENDER = 52364,
    //SOTA_SPELL_TELEPORT_ATTACKERS = 60178,
    SOTA_SPELL_END_OF_ROUND = 52459,
    //SOTA_SPELL_REMOVE_SEAFORIUM = 59077,
    SOTA_SPELL_ALLIANCE_CONTROL_PHASE_SHIFT = 60027,
    SOTA_SPELL_HORDE_CONTROL_PHASE_SHIFT = 60028,

    SOTA_SOUND_DEFEAT_HORDE = 15905,
    SOTA_SOUND_VICTORY_HORDE = 15906,
    SOTA_SOUND_VICTORY_ALLIANCE = 15907,
    SOTA_SOUND_DEFEAT_ALLIANCE = 15908,
    //SOTA_SOUND_WALL_DESTROYED_ALLIANCE = 15909,
    //SOTA_SOUND_WALL_DESTROYED_HORDE = 15910,
    //SOTA_SOUND_WALL_ATTACKED_HORDE = 15911,
    //SOTA_SOUND_WALL_ATTACKED_ALLIANCE = 15912,

    SOTA_BOAT_WEST = 0,
    SOTA_BOAT_EAST = 1,

    //TEAM_DEFENDER = 0,
    //TEAM_ATTACKER = 1,

    GUN_LEFT = 0,
    GUN_RIGHT = 1,

    GO_RELIC = 192834,
    ROUND_LENGTH = 600, // in secs
};

enum SOTAControlPoints
{
    SOTA_CONTROL_POINT_EAST_GY    = 0,
    SOTA_CONTROL_POINT_WEST_GY    = 1,
    SOTA_CONTROL_POINT_SOUTH_GY   = 2,
    NUM_SOTA_CONTROL_POINTS
};

enum SOTAGraveyards
{
    SOTA_GY_EAST            = 0,
    SOTA_GY_WEST            = 1,
    SOTA_GY_SOUTH           = 2,
    SOTA_GY_DEFENDER        = 3,
    SOTA_GY_ATTACKER_BEACH  = 4,
    NUM_SOTA_GRAVEYARDS
};

enum SOTACPStates
{
    SOTA_CP_STATE_UNCONTROLLED    = 0,
    SOTA_CP_STATE_ALLY_CONTROL    = 1,
    SOTA_CP_STATE_HORDE_CONTROL   = 2,
    MAX_SOTA_CP_STATES
};

enum Gate
{
    GATE_GREEN      = 0,
    GATE_YELLOW     = 1,
    GATE_BLUE       = 2,
    GATE_RED        = 3,
    GATE_PURPLE     = 4,
    GATE_COUNT      = 5
};

enum SOTABattleRoundProgress
{
    SOTA_ROUND_PREPARATION,
    SOTA_ROUND_STARTED,
    SOTA_NUM_ROUND_STAGES
};

struct SOTAControlPoint
{
    GameObject* pole;
    GameObject* banner;
    SOTACPStates state;
    uint32 worldstate;

    SOTAControlPoint()
    {
        pole = NULL;
        banner = NULL;
        state = SOTA_CP_STATE_UNCONTROLLED;
        worldstate = 0;
    }

    ~SOTAControlPoint()
    {
        pole = NULL;
        banner = NULL;
        state = SOTA_CP_STATE_UNCONTROLLED;
        worldstate = 0;
    }
};

struct SOTAGraveyard
{
    Creature* spiritguide;
    uint32 faction;

    SOTAGraveyard()
    {
        spiritguide = NULL;
        faction = MAX_PLAYER_TEAMS;
    }

    ~SOTAGraveyard()
    {
        spiritguide = NULL;
        faction = MAX_PLAYER_TEAMS;
    }
};

class StrandOfTheAncient : public CBattleground
{
private:

    uint32 Attackers;   // 0 - horde / 1 - alliance
    uint32 Defenders;
    uint32 BattleRound;
    uint32 RoundTime;
    uint32 RoundFinishTime[2];
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

    static CBattleground* Create(MapMgr* m, uint32 i, uint32 l, uint32 t) { return new StrandOfTheAncient(m, i, l, t); }

    StrandOfTheAncient(MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t);
    ~StrandOfTheAncient();

    uint32 GetNameID() override { return 34; }   //\todo in worldstring_tables ?

    uint32 GetRoundTime() { return RoundTime; };
    LocationVector GetStartingCoords(uint32 team) override;
    bool HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam) override;
    void HookOnAreaTrigger(Player* plr, uint32 id) override;
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
    void SetRoundTime(uint32 secs) { RoundTime = secs; };
    void SetTime(uint32 secs);
    void TimeTick();
    void PrepareRound();
    void StartRound();
    void FinishRound();
    void Finish(uint32 winningteam);

    void SpawnControlPoint(SOTAControlPoints point, SOTACPStates state);
    void CaptureControlPoint(SOTAControlPoints point);
    void SpawnGraveyard(SOTAGraveyards gyid, uint32 team);

};
