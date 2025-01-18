/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum class EasyScriptTypes : uint8_t;
enum class ScriptCommands : uint8_t;

//\note: used for db scripts table event_scripts
struct SimpleEventScript
{
    uint32_t eventId;
    ScriptCommands function;
    EasyScriptTypes scripttype;
    uint32_t data_1;
    uint32_t data_2;
    uint32_t data_3;
    uint32_t data_4;
    uint32_t data_5;
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t o;
    uint32_t delay;
    uint32_t nextevent;
};

enum class ScriptCommands : uint8_t
{
    SCRIPT_COMMAND_TALK                     = 0,
    SCRIPT_COMMAND_EMOTE                    = 1,
    SCRIPT_COMMAND_FIELD_SET                = 2,
    SCRIPT_COMMAND_MOVE_TO                  = 3,
    SCRIPT_COMMAND_FLAG_SET                 = 4,
    SCRIPT_COMMAND_FLAG_REMOVE              = 5,
    SCRIPT_COMMAND_TELEPORT_TO              = 6,
    SCRIPT_COMMAND_QUEST_EXPLORED           = 7,
    SCRIPT_COMMAND_KILL_CREDIT              = 8,       // Implemented (   data_1 (spellid), data_2 (quest id), data_3 (targettype 0 Creature/ 1 Gameobject), data_4 (target id), data_5 (killcredit), delay (when script needs to start ( in ms), next_event (next event_id when you want to add more )
    SCRIPT_COMMAND_RESPAWN_GAMEOBJECT       = 9,       // Implemented (   data_1 (GoId), data_2 (respawntime), delay (when script needs to start ( in ms), next_event (next event_id when you want to add more )
    SCRIPT_COMMAND_TEMP_SUMMON_CREATURE     = 10,
    SCRIPT_COMMAND_OPEN_DOOR                = 11,
    SCRIPT_COMMAND_CLOSE_DOOR               = 12,
    SCRIPT_COMMAND_ACTIVATE_OBJECT          = 13,      // Implemented ( data_1 (Go id),  when dont wanna use get pos then type in x y z the coords, delay (when script needs to start ( in ms), next_event (next event_id when you want to add more )
    SCRIPT_COMMAND_REMOVE_AURA              = 14,
    SCRIPT_COMMAND_CAST_SPELL               = 15,
    SCRIPT_COMMAND_PLAY_SOUND               = 16,
    SCRIPT_COMMAND_CREATE_ITEM              = 17,
    SCRIPT_COMMAND_DESPAWN_SELF             = 18,
    SCRIPT_COMMAND_KILL                     = 19,
    SCRIPT_COMMAND_ORIENTATION              = 20,
    SCRIPT_COMMAND_EQUIP                    = 21,
    SCRIPT_COMMAND_MODEL                    = 22,
    SCRIPT_COMMAND_PLAYMOVIE                = 23
};

enum class EasyScriptTypes : uint8_t
{
    SCRIPT_TYPE_SPELL_EFFECT    = 1,
    SCRIPT_TYPE_GAMEOBJECT      = 2,
    SCRIPT_TYPE_CREATURE        = 3,
    SCRIPT_TYPE_PLAYER          = 4,
    SCRIPT_TYPE_DUMMY           = 5
};

typedef std::multimap<uint32_t, SimpleEventScript> EventScriptMaps;
typedef std::multimap<uint32_t, SimpleEventScript const*> SpellEffectMaps;
typedef std::pair<EventScriptMaps::const_iterator, EventScriptMaps::const_iterator> EventScriptBounds;
typedef std::pair<SpellEffectMaps::const_iterator, SpellEffectMaps::const_iterator> SpellEffectMapBounds;
