/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <set>
#include <memory>

enum ServerHookEvents
{
    //////////////////////////////////////////////////////////////////////////////////////////   
    //Register Server Hooks
    // Server Hook callbacks can be made by using the function RegisterServerHook(EventId, function)

    SERVER_HOOK_EVENT_ON_NEW_CHARACTER                   = 1,  // -- (event, pName, int Race, int Class)
    SERVER_HOOK_EVENT_ON_KILL_PLAYER                     = 2,  // -- (event, pKiller, pVictim)
    SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD               = 3,  // -- (event, pPlayer)                 / a new created character enters for first time the world
    SERVER_HOOK_EVENT_ON_ENTER_WORLD                     = 4,  // -- (event, pPlayer)                 / a character enters the world (login) or moves to another map
    SERVER_HOOK_EVENT_ON_GUILD_JOIN                      = 5,  // -- (event, pPlayer, str GuildName)
    SERVER_HOOK_EVENT_ON_DEATH                           = 6,  // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_REPOP                           = 7,  // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_EMOTE                           = 8,  // -- (event, pPlayer, pUnit, EmoteId)
    SERVER_HOOK_EVENT_ON_ENTER_COMBAT                    = 9,  // -- (event, pPlayer, pTarget)
    SERVER_HOOK_EVENT_ON_CAST_SPELL                      = 10, // -- (event, pPlayer, SpellId, pSpellObject)
    SERVER_HOOK_EVENT_ON_TICK                            = 11, // -- No arguments passed.
    SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST                  = 12, // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_LOGOUT                          = 13, // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_QUEST_ACCEPT                    = 14, // -- (event, pPlayer, QuestId, pQuestGiver)
    SERVER_HOOK_EVENT_ON_ZONE                            = 15, // -- (event, pPlayer, ZoneId, OldZoneId)
    SERVER_HOOK_EVENT_ON_CHAT                            = 16, // -- (event, pPlayer, str Message, Type, Language, Misc)
    SERVER_HOOK_EVENT_ON_LOOT                            = 17, // -- (event, pPlayer, pTarget, Money, ItemId)
    SERVER_HOOK_EVENT_ON_GUILD_CREATE                    = 18, // -- (event, pPlayer, pGuildName)
    SERVER_HOOK_EVENT_ON_FULL_LOGIN                      = 19, // -- (event, pPlayer)                 / a character enters the world (login)
    SERVER_HOOK_EVENT_ON_CHARACTER_CREATE                = 20, // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_QUEST_CANCELLED                 = 21, // -- (event, pPlayer, QuestId)
    SERVER_HOOK_EVENT_ON_QUEST_FINISHED                  = 22, // -- (event, pPlayer, QuestId, pQuestGiver)
    SERVER_HOOK_EVENT_ON_HONORABLE_KILL                  = 23, // -- (event, pPlayer, pKilled)
    SERVER_HOOK_EVENT_ON_ARENA_FINISH                    = 24, // -- (event, pPlayer, str TeamName, bWinner, bRated)
    SERVER_HOOK_EVENT_ON_OBJECTLOOT                      = 25, // -- (event, pPlayer, pTarget, Money, ItemId)
    SERVER_HOOK_EVENT_ON_AREATRIGGER                     = 26, // -- (event, pPlayer, AreaTriggerId)
    SERVER_HOOK_EVENT_ON_POST_LEVELUP                    = 27, // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_PRE_DIE                         = 28, // -- (event, pKiller, pDied)          / general unit die, not only based on players
    SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE               = 29, // -- (event, pPlayer, SkillId, SkillLevel)
    SERVER_HOOK_EVENT_ON_DUEL_FINISHED                   = 30, // -- (event, pWinner, pLoser)
    SERVER_HOOK_EVENT_ON_AURA_REMOVE                     = 31, // -- (event, pAuraObject)
    SERVER_HOOK_EVENT_ON_RESURRECT                       = 32, // -- (event, pPlayer)
    NUM_SERVER_HOOKS
};

typedef std::set<void*> ServerHookList;

struct QuestProperties;

class Item;
class WorldSession;
class Player;
class Guild;
class Unit;
class SpellInfo;
class Spell;
class Object;
class ArenaTeam;
class Aura;

// Hook typedefs
typedef bool(*tOnNewCharacter)(uint32_t Race, uint32_t Class, WorldSession* Session, const char* Name);
typedef void(*tOnKillPlayer)(Player* pPlayer, Player* pVictim);
typedef void(*tOCharacterCreate)(Player* pPlayer);
typedef void(*tOnFirstEnterWorld)(Player* pPlayer);
typedef void(*tOnEnterWorld)(Player* pPlayer);
typedef void(*tOnGuildCreate)(Player* pLeader, Guild* pGuild);
typedef void(*tOnGuildJoin)(Player* pPlayer, Guild* pGuild);
typedef void(*tOnDeath)(Player* pPlayer);
typedef bool(*tOnRepop)(Player* pPlayer);
typedef void(*tOnEmote)(Player* pPlayer, uint32_t Emote, Unit* pUnit);
typedef void(*tOnEnterCombat)(Player* pPlayer, Unit* pTarget);
typedef bool(*tOnCastSpell)(Player* pPlayer, SpellInfo const* pSpell, Spell* spell);
typedef void(*tOnTick)();
typedef bool(*tOnLogoutRequest)(Player* pPlayer);
typedef void(*tOnLogout)(Player* pPlayer);
typedef void(*tOnQuestAccept)(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver);
typedef void(*tOnZone)(Player* pPlayer, uint32_t Zone, uint32_t oldzone);
typedef bool(*tOnChat)(Player* pPlayer, uint32_t Type, uint32_t Lang, const char* Message, const char* Misc);
typedef void(*tOnLoot)(Player* pPlayer, Unit* pTarget, uint32_t Money, uint32_t ItemId);
typedef bool(*ItemScript)(Item* pItem, Player* pPlayer);
typedef void(*tOnQuestCancel)(Player* pPlayer, QuestProperties const* pQuest);
typedef void(*tOnQuestFinished)(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver);
typedef void(*tOnHonorableKill)(Player* pPlayer, Player* pKilled);
typedef void(*tOnArenaFinish)(Player* pPlayer, ArenaTeam* pTeam, bool victory, bool rated);
typedef void(*tOnObjectLoot)(Player* pPlayer, Object* pTarget, uint32_t Money, uint32_t ItemId);
typedef void(*tOnAreaTrigger)(Player* pPlayer, uint32_t areaTrigger);
typedef void(*tOnPostLevelUp)(Player* pPlayer);
typedef bool(*tOnPreUnitDie)(Unit* killer, Unit* target);
typedef void(*tOnAdvanceSkillLine)(Player* pPlayer, uint32_t SkillLine, uint32_t Current);
typedef void(*tOnDuelFinished)(Player* Winner, Player* Looser);
typedef void(*tOnAuraRemove)(Aura* aura);
typedef bool(*tOnResurrect)(Player* pPlayer);
