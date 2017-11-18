/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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

//===================================================
const uint32 EOTS_GO_BE_TOWER = 184080;    // 184080 - Blood Elf Tower Cap Pt
const uint32 EOTS_GO_FELREAVER = 184081;    // 184081 - Fel Reaver Cap Pt
const uint32 EOTS_GO_MAGE_TOWER = 184082;    // 184082 - Human Tower Cap Pt
const uint32 EOTS_GO_DRAENEI_TOWER = 184083;    // 184083 - Draenei Tower Cap Pt

const uint32 EOTS_TOWER_BE = 0;
const uint32 EOTS_TOWER_FELREAVER = 1;
const uint32 EOTS_TOWER_MAGE = 2;
const uint32 EOTS_TOWER_DRAENEI = 3;

const uint32 EOTS_BANNER_NEUTRAL = 184382;
const uint32 EOTS_BANNER_ALLIANCE = 184381;
const uint32 EOTS_BANNER_HORDE = 184380;

const uint32 EOTS_CAPTURE_DISTANCE = 900; /*30*/

const uint32 EOTS_CAPTURE_RATE = 4;
const uint32 EOTS_TOWER_COUNT = 4;
const uint32 EOTS_RECENTLY_DROPPED_FLAG = 50327;
const uint32 EOTS_BUFF_RESPAWN_TIME = 90000;
const uint32 EOTS_NETHERWING_FLAG_SPELL = 34976;
const uint32 EOTS_NETHERWING_FLAG_READY = 2757;

class EyeOfTheStorm : public CBattleground
{
    public:

        EyeOfTheStorm(MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t);
        ~EyeOfTheStorm();

        bool HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam) override;
        void HookOnPlayerDeath(Player* plr) override;
        void HookFlagDrop(Player* plr, GameObject* obj) override;
        void HookFlagStand(Player* plr, GameObject* obj) override;
        void HookOnMount(Player* plr) override;
        void HookOnAreaTrigger(Player* plr, uint32 id) override;
        bool HookHandleRepop(Player* plr) override;
        void OnAddPlayer(Player* plr) override;
        void OnRemovePlayer(Player* plr) override;
        void OnCreate() override;
        void HookOnPlayerKill(Player* plr, Player* pVictim) override;
        void HookOnUnitKill(Player* plr, Unit* pVictim) override;
        void HookOnHK(Player* plr) override;
        void HookOnShadowSight() override;
        void HookGenerateLoot(Player* plr, Object* pCorpse) override;
        void SpawnBuff(uint32 x);
        LocationVector GetStartingCoords(uint32 Team) override;
        static CBattleground* Create(MapMgr* m, uint32 i, uint32 l, uint32 t) { return new EyeOfTheStorm(m, i, l, t); }
        uint64 GetFlagHolderGUID(uint32 /*faction*/) const override { return m_flagHolder; }

        uint32 GetNameID() override { return 44; }
        void OnStart() override;

        void UpdateCPs();
        void GeneratePoints();

        // returns true if that team won
        bool GivePoints(uint32 team, uint32 points);

        void RespawnCPFlag(uint32 i, uint32 id);        // 0 = Neutral, <0 = Leaning towards alliance, >0 Leaning towards horde

        bool HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell* pSpell) override;
        void DropFlag2(Player* plr, uint32 id);
        void HookOnFlagDrop(Player* plr) override;
        void EventResetFlag();
        void RepopPlayersOfTeam(int32 team, Creature* sh);

        void SetIsWeekend(bool isweekend) override;

    protected:

        int32 m_CPStatus[EOTS_TOWER_COUNT];
        uint32 m_flagHolder;

        GameObject* m_standFlag;
        GameObject* m_dropFlag;

        GameObject* m_CPStatusGO[EOTS_TOWER_COUNT];
        GameObject* m_CPBanner[EOTS_TOWER_COUNT];
        GameObject* m_CPBanner2[EOTS_TOWER_COUNT];
        GameObject* m_CPBanner3[EOTS_TOWER_COUNT];
        GameObject* m_bubbles[2];
        GameObject* EOTSm_buffs[4];

        typedef std::set<Player*> EOTSCaptureDisplayList;
        EOTSCaptureDisplayList m_CPDisplay[EOTS_TOWER_COUNT];

        uint32 m_lastHonorGainPoints[2];
        uint32 m_points[2];
        Creature* m_spiritGuides[EOTS_TOWER_COUNT];
};
