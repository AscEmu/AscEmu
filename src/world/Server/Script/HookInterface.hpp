/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#include <cstdint>
#include <memory>

class Aura;
class ArenaTeam;
class Object;
struct QuestProperties;
class Spell;
class SpellInfo;
class Unit;
class Guild;
class Player;
class WorldSession;

class SERVER_DECL HookInterface
{
    friend class ScriptMgr;

    HookInterface() = default;
    ~HookInterface() = default;

public:
    static HookInterface& getInstance();

    HookInterface(HookInterface&&) = delete;
    HookInterface(HookInterface const&) = delete;
    HookInterface& operator=(HookInterface&&) = delete;
    HookInterface& operator=(HookInterface const&) = delete;

    bool OnNewCharacter(uint32_t Race, uint32_t Class, WorldSession* Session, const char* Name);
    void OnKillPlayer(Player* pPlayer, Player* pVictim);
    void OnFirstEnterWorld(Player* pPlayer);
    void OnEnterWorld(Player* pPlayer);
    void OnGuildCreate(Player* pLeader, Guild* pGuild);
    void OnGuildJoin(Player* pPlayer, Guild* pGuild);
    void OnDeath(Player* pPlayer);
    bool OnRepop(Player* pPlayer);
    void OnEmote(Player* pPlayer, uint32_t Emote, Unit* pUnit);
    void OnEnterCombat(Player* pPlayer, Unit* pTarget);
    bool OnCastSpell(Player* pPlayer, SpellInfo const* pSpell, Spell* spell);
    bool OnLogoutRequest(Player* pPlayer);
    void OnLogout(Player* pPlayer);
    void OnQuestAccept(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver);
    void OnZone(Player* pPlayer, uint32_t Zone, uint32_t oldZone);
    bool OnChat(Player* pPlayer, uint32_t Type, uint32_t Lang, const char* Message, const char* Misc);
    void OnLoot(Player* pPlayer, Unit* pTarget, uint32_t Money, uint32_t ItemId);
    void OnFullLogin(Player* pPlayer);
    void OnCharacterCreate(Player* pPlayer);
    void OnQuestCancelled(Player* pPlayer, QuestProperties const* pQuest);
    void OnQuestFinished(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver);
    void OnHonorableKill(Player* pPlayer, Player* pKilled);
    void OnArenaFinish(Player* pPlayer, ArenaTeam* pTeam, bool victory, bool rated);
    void OnObjectLoot(Player* pPlayer, Object* pTarget, uint32_t Money, uint32_t ItemId);
    void OnAreaTrigger(Player* pPlayer, uint32_t areaTrigger);
    void OnPostLevelUp(Player* pPlayer);
    bool OnPreUnitDie(Unit* Killer, Unit* Victim);
    void OnAdvanceSkillLine(Player* pPlayer, uint32_t SkillLine, uint32_t Current);
    void OnDuelFinished(Player* Winner, Player* Looser);
    void OnAuraRemove(Aura* aura);
    bool OnResurrect(Player* pPlayer);
};

#define sHookInterface HookInterface::getInstance()
