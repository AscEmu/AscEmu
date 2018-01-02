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

const uint32 TIME_LEFT = 25;
const uint32 TIME_FOCUSED_ASSAULT = 10;
const uint32 TIME_BRUTAL_ASSAULT = 15;
const uint32 BUFF_RESPAWN_TIME = 90000;
const uint32 SILVERWING_FLAG = 179785;
const uint32 WARSONG_FLAG = 179786;
const uint32 SPELL_FOCUSED_ASSAULT = 46392;
const uint32 SPELL_BRUTAL_ASSAULT = 46393;

enum WarsongGulchAreaTriggers
{
    AREATRIGGER_A_SPEED = 3686,
    AREATRIGGER_H_SPEED = 3687,
    AREATRIGGER_A_RESTORATION = 3706,
    AREATRIGGER_H_RESTORATION = 3708,
    AREATRIGGER_A_BERSERKING = 3707,
    AREATRIGGER_H_BERSERKING = 3709,
    AREATRIGGER_WSG_ENCOUNTER_01 = 3649,
    AREATRIGGER_WSG_ENCOUNTER_02 = 3688,
    AREATRIGGER_WSG_ENCOUNTER_03 = 4628,
    AREATRIGGER_WSG_ENCOUNTER_04 = 4629,
    AREATRIGGER_WSG_A_SPAWN = 3646,
    AREATRIGGER_WSG_H_SPAWN = 3647
};

class WarsongGulch : public CBattleground
{
    GameObject* m_buffs[6];
    GameObject* m_homeFlags[2];
    GameObject* m_dropFlags[2];
    uint32 m_flagHolders[2];
    std::list<GameObject*> m_gates;
    uint32 m_scores[2];
    uint32 m_lgroup;
    uint8 m_time_left;

    void TimeLeft();

    public:

        WarsongGulch(MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t);
        ~WarsongGulch();

        bool HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam) override;
        void HookOnPlayerDeath(Player* plr);
        void HookFlagDrop(Player* plr, GameObject* obj);
        void HookFlagStand(Player* plr, GameObject* obj);
        void HookOnMount(Player* plr);
        void HookOnAreaTrigger(Player* plr, uint32 id);
        bool HookHandleRepop(Player* plr);
        void OnAddPlayer(Player* plr);
        void OnRemovePlayer(Player* plr);
        void OnCreate();
        void HookOnPlayerKill(Player* plr, Player* pVictim);
        void HookOnUnitKill(Player* plr, Unit* pVictim);
        void HookOnHK(Player* plr);
        void HookOnShadowSight();
        void HookGenerateLoot(Player* plr, Object* pCorpse);
        void SpawnBuff(uint32 x);
        LocationVector GetStartingCoords(uint32 Team);
        void HookOnFlagDrop(Player* plr);
        void ReturnFlag(PlayerTeam team);

        void EventReturnFlags();

        static CBattleground* Create(MapMgr* m, uint32 i, uint32 l, uint32 t) { return new WarsongGulch(m, i, l, t); }

        uint32 GetNameID() { return 39; }
        uint64 GetFlagHolderGUID(uint32 faction) const { return m_flagHolders[faction]; }
        void OnStart();

        void SetIsWeekend(bool isweekend);
        void DespawnGates(uint32 delay);
};
