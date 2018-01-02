/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Management/Battleground/Battleground.h"

const uint32 BUFF_COUNT = 3;
const uint32 SOTA_NPCS = 3;
const uint32 SOTA_NUM_BOMBS = 66;
const uint32 SOTA_NORTH_BOMBS = 22;
const uint32 SOTA_EAST_BOMBS_INDEX = 22;
const uint32 SOTA_WEST_BOMBS_INDEX = 31;
const uint32 SOTA_BOAT_ALLIANCE_W = 193182;
const uint32 SOTA_BOAT_ALLIANCE_E = 193185;
const uint32 SOTA_BOAT_HORDER_W = 193184;
const uint32 SOTA_BOAT_HORDER_E = 193183;
const uint32 SOTA_DEMOLISHER = 28781;
const uint32 SOTA_ANTI_PERSONNAL_CANNON = 27894;
const uint32 SOTA_RIGGER_SPARKLIGHT = 29260;
const uint32 SOTA_GORGRIL_RIGSPARK = 29262;
const uint32 SOTA_BOMBS = 190753;
const uint32 SOTA_KANRETHAD = 29;
const uint32 SOTA_NUM_CANONS = 10;
const uint32 SOTA_NUM_DEMOLISHERS = 8;
const uint32 SOTA_NORTH_DEMOLISHERS = 4;
const uint32 SOTA_EAST_WS_DEMOLISHER_INDEX = 6;
const uint32 SOTA_WEST_WS_DEMOLISHER_INDEX = 4;

const uint32 SOTA_SPELL_TELEPORT_DEFENDER = 52364;
const uint32 SOTA_SPELL_TELEPORT_ATTACKERS = 60178;
const uint32 SOTA_SPELL_END_OF_ROUND = 52459;
const uint32 SOTA_SPELL_REMOVE_SEAFORIUM = 59077;
const uint32 SOTA_SPELL_ALLIANCE_CONTROL_PHASE_SHIFT = 60027;
const uint32 SOTA_SPELL_HORDE_CONTROL_PHASE_SHIFT = 60028;

const uint32 SOTA_SOUND_DEFEAT_HORDE = 15905;
const uint32 SOTA_SOUND_VICTORY_HORDE = 15906;
const uint32 SOTA_SOUND_VICTORY_ALLIANCE = 15907;
const uint32 SOTA_SOUND_DEFEAT_ALLIANCE = 15908;
const uint32 SOTA_SOUND_WALL_DESTROYED_ALLIANCE = 15909;
const uint32 SOTA_SOUND_WALL_DESTROYED_HORDE = 15910;
const uint32 SOTA_SOUND_WALL_ATTACKED_HORDE = 15911;
const uint32 SOTA_SOUND_WALL_ATTACKED_ALLIANCE = 15912;

const uint32 SOTA_BOAT_WEST = 0;
const uint32 SOTA_BOAT_EAST = 1;
const uint32 TEAM_DEFENDER = 0;
const uint32 TEAM_ATTACKER = 1;
const uint32 GUN_LEFT = 0;
const uint32 GUN_RIGHT = 1;

const uint32 GO_RELIC = 192834;
const uint32 ROUND_LENGTH = 600; //in secs

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

        uint32 GetNameID() { return 34; }   ///\todo in worldstring_tables ?

        uint32 GetRoundTime() { return RoundTime; };
        LocationVector GetStartingCoords(uint32 team);
        bool HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam) override;
        void HookOnAreaTrigger(Player* plr, uint32 id);
        void HookFlagStand(Player* plr, GameObject* obj);
        void HookOnFlagDrop(Player* plr);
        void HookFlagDrop(Player* plr, GameObject* obj);
        void HookOnPlayerKill(Player* plr, Player* pVictim);
        void HookOnHK(Player* plr);
        void HookOnShadowSight();
        void HookGenerateLoot(Player* plr, Object* pOCorpse);
        void HookOnUnitKill(Player* plr, Unit* pVictim);
        void HookOnUnitDied(Unit* victim);
        bool HookSlowLockOpen(GameObject* go, Player* player, Spell* spell );
        bool HookQuickLockOpen(GameObject* go, Player* player, Spell* spell );
        void HookOnPlayerDeath(Player* plr);
        void HookOnMount(Player* plr);
        bool HookHandleRepop(Player* plr);
        void OnAddPlayer(Player* plr);
        void OnRemovePlayer(Player* plr);
        void OnPlatformTeleport(Player* plr);
        void OnCreate();
        void OnStart();
        void SetIsWeekend(bool isweekend);
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
