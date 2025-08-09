/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Battleground/Battleground.hpp"

class ArenaTeam;

class SERVER_DECL Arena : public Battleground
{
public:
    Arena(WorldMap* _worldMap, uint32_t _id, uint32_t _levelGroup, uint32_t _arenaType, uint32_t _playersPerSide);
    virtual ~Arena();

    bool HandleFinishBattlegroundRewardCalculation(PlayerTeam _winningTeam) override;
    bool HookHandleRepop(Player* _player) override;
    void OnAddPlayer(Player* _player) override;
    void OnRemovePlayer(Player* _player) override;
    void OnCreate() override;
    void HookOnPlayerDeath(Player* _player) override;
    void HookOnPlayerKill(Player* _player, Player* _playerVictim) override;
    void HookOnUnitKill(Player* _player, Unit* _unitVictim) override;
    void HookOnFlagDrop(Player* _player) override;
    void HookOnHK(Player* _player) override;
    void HookOnShadowSight() override;
    void HookGenerateLoot(Player* _player, Object* _objectCorpse) override;
    void UpdatePlayerCounts();
    LocationVector GetStartingCoords(uint32_t _teamId) override;
    uint32_t GetNameID() override;
    void OnStart() override;
    bool CanPlayerJoin(Player* _player, uint32_t _type) override;

    bool CreateCorpse(Player* /*plr*/) override;

    // dummy stuff
    void HookOnMount(Player* /*plr*/) override {}
    void HookFlagDrop(Player* /*plr*/, GameObject* /*obj*/) override {}
    void HookFlagStand(Player* /*plr*/, GameObject* /*obj*/) override {}
    void HookOnAreaTrigger(Player* _player, uint32_t id) override;

    int32_t GetFreeTeam() const;

    // Returns the faction of the _teamId
    uint32_t GetTeamFaction(uint32_t _teamId);
    uint8_t Rated() override;
    uint8_t GetArenaTeamType() const;
    ArenaTeam** GetTeams();
    uint32_t CalcDeltaRating(uint32_t _oldRating, uint32_t _opponentRating, bool _outcome);

    bool rated_match = false;

protected:
    std::set<GameObject*> m_gates;
    GameObject* m_buffs[2] = { nullptr };
    ArenaTeam* m_teams[2] = { nullptr };
    uint8_t m_arenateamtype;
    uint32_t m_playersCount[2] = { 0 };
    std::set<uint32_t> m_players2[2];
    std::set<uint32_t> m_playersAlive;
};
