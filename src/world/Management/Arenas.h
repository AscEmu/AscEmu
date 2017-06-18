/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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

#ifndef ARENAS_H
#define ARENAS_H

#include "Management/Battleground/Battleground.h"

class ArenaTeam;

class SERVER_DECL Arena : public CBattleground
{
    protected:

        std::set<GameObject*> m_gates;
        GameObject* m_buffs[2];
        ArenaTeam* m_teams[2];
        uint32 m_arenateamtype;
        uint32 m_playersCount[2];
        std::set<uint32> m_players2[2];
        std::set<uint32> m_playersAlive;

    public:

        bool rated_match;
        Arena(MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t, uint32 players_per_side);
        virtual ~Arena();

        bool HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam) override;
        bool HookHandleRepop(Player* plr);
        void OnAddPlayer(Player* plr);
        void OnRemovePlayer(Player* plr);
        void OnCreate();
        void HookOnPlayerDeath(Player* plr);
        void HookOnPlayerKill(Player* plr, Player* pVictim);
        void HookOnUnitKill(Player* plr, Unit* pVictim);
        void HookOnFlagDrop(Player* plr);
        void HookOnHK(Player* plr);
        void HookOnShadowSight();
        void HookGenerateLoot(Player* plr, Object* pCorpse);
        void UpdatePlayerCounts();
        LocationVector GetStartingCoords(uint32 Team);
        uint32 GetNameID() { return 50; }
        void OnStart();
        bool CanPlayerJoin(Player* plr, uint32 type)
        {
            if (m_started)
                return false;
            else
                return CBattleground::CanPlayerJoin(plr, type);
        }

        bool CreateCorpse(Player* plr) { return false; }

        /* dummy stuff */
        void HookOnMount(Player* plr) {}
        void HookFlagDrop(Player* plr, GameObject* obj) {}
        void HookFlagStand(Player* plr, GameObject* obj) {}
        void HookOnAreaTrigger(Player* plr, uint32 id);

        int32 GetFreeTeam()
        {
            size_t c0 = m_players[0].size() + m_pendPlayers[0].size();
            size_t c1 = m_players[1].size() + m_pendPlayers[1].size();
            if (m_started) return -1;

            /// Check if there is free room, if yes, return team with less members
            return ((c0 + c1 >= m_playerCountPerTeam * 2) ? -1 : (c0 > c1));

            /* We shouldn't reach here. */
        }

        /// Returns the faction of the team
        uint32 GetTeamFaction(uint32 team);
        inline uint8 Rated() { return rated_match; }
        inline uint32 GetArenaTeamType() { return m_arenateamtype; }
        inline ArenaTeam** GetTeams() { return m_teams; }
        uint32 CalcDeltaRating(uint32 oldRating, uint32 opponentRating, bool outcome);
};


#endif //ARENAS_H
