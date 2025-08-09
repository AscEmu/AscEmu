/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "HookInterface.hpp"
#include "HookInterfaceDefines.hpp"
#include "ScriptMgr.hpp"

HookInterface& HookInterface::getInstance()
{
    static HookInterface mInstance;
    return mInstance;
}

bool HookInterface::OnNewCharacter(uint32_t Race, uint32_t Class, WorldSession* Session, const char* Name)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_NEW_CHARACTER];
    bool ret_val = true;
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
    {
        bool rv = ((tOnNewCharacter)* itr)(Race, Class, Session, Name);
        if (rv == false)  // never set ret_val back to true, once it's false
            ret_val = false;
    }
    return ret_val;
}

void HookInterface::OnKillPlayer(Player* pPlayer, Player* pVictim)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_KILL_PLAYER];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnKillPlayer)*itr)(pPlayer, pVictim);
}

void HookInterface::OnFirstEnterWorld(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnFirstEnterWorld)*itr)(pPlayer);
}

void HookInterface::OnCharacterCreate(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_CHARACTER_CREATE];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOCharacterCreate)*itr)(pPlayer);
}

void HookInterface::OnEnterWorld(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_ENTER_WORLD];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnEnterWorld)*itr)(pPlayer);
}

void HookInterface::OnGuildCreate(Player* pLeader, Guild* pGuild)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_GUILD_CREATE];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnGuildCreate)*itr)(pLeader, pGuild);
}

void HookInterface::OnGuildJoin(Player* pPlayer, Guild* pGuild)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_GUILD_JOIN];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnGuildJoin)*itr)(pPlayer, pGuild);
}

void HookInterface::OnDeath(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_DEATH];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnDeath)*itr)(pPlayer);
}

bool HookInterface::OnRepop(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_REPOP];
    bool ret_val = true;
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
    {
        bool rv = ((tOnRepop)* itr)(pPlayer);
        if (rv == false)  // never set ret_val back to true, once it's false
            ret_val = false;
    }
    return ret_val;
}

void HookInterface::OnEmote(Player* pPlayer, uint32_t Emote, Unit* pUnit)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_EMOTE];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnEmote)*itr)(pPlayer, Emote, pUnit);
}

void HookInterface::OnEnterCombat(Player* pPlayer, Unit* pTarget)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_ENTER_COMBAT];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnEnterCombat)*itr)(pPlayer, pTarget);
}

bool HookInterface::OnCastSpell(Player* pPlayer, SpellInfo const* pSpell, Spell* spell)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_CAST_SPELL];
    bool ret_val = true;
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
    {
        bool rv = ((tOnCastSpell)* itr)(pPlayer, pSpell, spell);
        if (rv == false)  // never set ret_val back to true, once it's false
            ret_val = false;
    }
    return ret_val;
}

bool HookInterface::OnLogoutRequest(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST];
    bool ret_val = true;
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
    {
        bool rv = ((tOnLogoutRequest)* itr)(pPlayer);
        if (rv == false)  // never set ret_val back to true, once it's false
            ret_val = false;
    }
    return ret_val;
}

void HookInterface::OnLogout(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_LOGOUT];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnLogout)*itr)(pPlayer);
}

void HookInterface::OnQuestAccept(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_QUEST_ACCEPT];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnQuestAccept)*itr)(pPlayer, pQuest, pQuestGiver);
}

void HookInterface::OnZone(Player* pPlayer, uint32_t zone, uint32_t oldZone)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_ZONE];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnZone)*itr)(pPlayer, zone, oldZone);
}

bool HookInterface::OnChat(Player* pPlayer, uint32_t type, uint32_t lang, const char* message, const char* misc)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_CHAT];
    bool ret_val = true;
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
    {
        bool rv = ((tOnChat)* itr)(pPlayer, type, lang, message, misc);
        if (rv == false)  // never set ret_val back to true, once it's false
            ret_val = false;
    }
    return ret_val;
}

void HookInterface::OnLoot(Player* pPlayer, Unit* pTarget, uint32_t money, uint32_t itemId)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_LOOT];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnLoot)*itr)(pPlayer, pTarget, money, itemId);
}

void HookInterface::OnObjectLoot(Player* pPlayer, Object* pTarget, uint32_t money, uint32_t itemId)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_OBJECTLOOT];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnObjectLoot)*itr)(pPlayer, pTarget, money, itemId);
}

void HookInterface::OnFullLogin(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_FULL_LOGIN];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnEnterWorld)*itr)(pPlayer);
}

void HookInterface::OnQuestCancelled(Player* pPlayer, QuestProperties const* pQuest)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_QUEST_CANCELLED];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnQuestCancel)*itr)(pPlayer, pQuest);
}

void HookInterface::OnQuestFinished(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_QUEST_FINISHED];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnQuestFinished)*itr)(pPlayer, pQuest, pQuestGiver);
}

void HookInterface::OnHonorableKill(Player* pPlayer, Player* pKilled)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_HONORABLE_KILL];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnHonorableKill)*itr)(pPlayer, pKilled);
}

void HookInterface::OnArenaFinish(Player* pPlayer, ArenaTeam* pTeam, bool victory, bool rated)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_ARENA_FINISH];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnArenaFinish)*itr)(pPlayer, pTeam, victory, rated);
}

void HookInterface::OnAreaTrigger(Player* pPlayer, uint32_t areaTrigger)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_AREATRIGGER];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnAreaTrigger)*itr)(pPlayer, areaTrigger);
}

void HookInterface::OnPostLevelUp(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_POST_LEVELUP];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnPostLevelUp)*itr)(pPlayer);
}

bool HookInterface::OnPreUnitDie(Unit* killer, Unit* victim)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_PRE_DIE];
    bool ret_val = true;
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
    {
        bool rv = ((tOnPreUnitDie)* itr)(killer, victim);
        if (rv == false)  // never set ret_val back to true, once it's false
            ret_val = false;
    }
    return ret_val;
}


void HookInterface::OnAdvanceSkillLine(Player* pPlayer, uint32_t skillLine, uint32_t current)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnAdvanceSkillLine)*itr)(pPlayer, skillLine, current);
}

void HookInterface::OnDuelFinished(Player* Winner, Player* Looser)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_DUEL_FINISHED];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnDuelFinished)*itr)(Winner, Looser);
}

void HookInterface::OnAuraRemove(Aura* aura)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_AURA_REMOVE];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnAuraRemove)*itr)(aura);
}

bool HookInterface::OnResurrect(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_RESURRECT];
    bool ret_val = true;
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
    {
        bool rv = ((tOnResurrect)* itr)(pPlayer);
        if (rv == false)  // never set ret_val back to true, once it's false
            ret_val = false;
    }
    return ret_val;
}