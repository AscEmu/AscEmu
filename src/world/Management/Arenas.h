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
        bool HookHandleRepop(Player* plr) override;
        void OnAddPlayer(Player* plr) override;
        void OnRemovePlayer(Player* plr) override;
        void OnCreate() override;
        void HookOnPlayerDeath(Player* plr) override;
        void HookOnPlayerKill(Player* plr, Player* pVictim) override;
        void HookOnUnitKill(Player* plr, Unit* pVictim) override;
        void HookOnFlagDrop(Player* plr) override;
        void HookOnHK(Player* plr) override;
        void HookOnShadowSight() override;
        void HookGenerateLoot(Player* plr, Object* pCorpse) override;
        void UpdatePlayerCounts();
        LocationVector GetStartingCoords(uint32 Team) override;
        uint32 GetNameID() override { return 50; }
        void OnStart() override;
        bool CanPlayerJoin(Player* plr, uint32 type) override
        {
            if (m_started)
                return false;

            return CBattleground::CanPlayerJoin(plr, type);
        }

        bool CreateCorpse(Player* /*plr*/) override { return false; }

        /* dummy stuff */
        void HookOnMount(Player* /*plr*/) override {}
        void HookFlagDrop(Player* /*plr*/, GameObject* /*obj*/) override {}
        void HookFlagStand(Player* /*plr*/, GameObject* /*obj*/) override {}
        void HookOnAreaTrigger(Player* plr, uint32 id) override;

        int32 GetFreeTeam() const
        {
            size_t c0 = m_players[0].size() + m_pendPlayers[0].size();
            size_t c1 = m_players[1].size() + m_pendPlayers[1].size();
            if (m_started)
                return -1;

            // Check if there is free room, if yes, return team with less members
            return ((c0 + c1 >= m_playerCountPerTeam * 2) ? -1 : (c0 > c1));
        }

        // Returns the faction of the team
        uint32 GetTeamFaction(uint32 team);
        uint8 Rated() override { return rated_match; }
        uint32 GetArenaTeamType() const { return m_arenateamtype; }
        ArenaTeam** GetTeams() { return m_teams; }
        uint32 CalcDeltaRating(uint32 oldRating, uint32 opponentRating, bool outcome);
};


#endif //ARENAS_H
