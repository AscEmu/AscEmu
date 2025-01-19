/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "GameObjectDefines.hpp"

#include <map>
#include <string>
#include <unordered_map>

struct QuestProperties;

struct GameObjectProperties
{
    uint32_t entry;
    uint32_t type; // uint8_t
    uint32_t display_id;
    std::string name;
    std::string category_name;
    std::string cast_bar_text;
    std::string Unkstr;

    // different data fields for GO-types
    // \todo add different structure for go types.
    union
    {
        // 0 GAMEOBJECT_TYPE_DOOR
        struct
        {
            uint32_t start_open;                  // parameter_0 client side opened/closed
            uint32_t lock_id;                     // parameter_1 from Lock.dbc
            uint32_t auto_close_time;             // parameter_2 in secs
            uint32_t no_damage_immune;            // parameter_3
            uint32_t open_text_id;                // parameter_4
            uint32_t close_text_id;               // parameter_5
            uint32_t ignored_by_pathing;          // parameter_6
        }door;
        // 1 GAMEOBJECT_TYPE_BUTTON
        struct
        {
            uint32_t start_open;                  // parameter_0
            uint32_t lock_id;                     // parameter_1 from Lock.dbc
            uint32_t auto_close_time;             // parameter_2 in secs
            uint32_t linked_trap_id;              // parameter_3
            uint32_t no_damage_immune;            // parameter_4 BgObject
            uint32_t large;                       // parameter_5
            uint32_t open_text_id;                // parameter_6
            uint32_t close_text_id;               // parameter_7
            uint32_t los_ok;                      // parameter_8
        }button;
        // 2 GAMEOBJECT_TYPE_QUESTGIVER
        struct
        {
            uint32_t lockId;                      // parameter_0 -> Lock.dbc
            uint32_t questList;                   // parameter_1
            uint32_t pageMaterial;                // parameter_2
            uint32_t gossipID;                    // parameter_3
            uint32_t customAnim;                  // parameter_4
            uint32_t noDamageImmune;              // parameter_5
            uint32_t openTextID;                  // parameter_6 can be used to replace castBarCaption?
            uint32_t losOK;                       // parameter_7
            uint32_t allowMounted;                // parameter_8 Is usable while on mount/vehicle. (0/1)
            uint32_t large;                       // parameter_9
            uint32_t conditionID1;                // parameter_10
        } questgiver;
        // 3 GAMEOBJECT_TYPE_CHEST
        struct
        {
            uint32_t lock_id;                     // parameter_0 from Lock.dbc
            uint32_t loot_id;                     // parameter_1
            uint32_t restock_time;                // parameter_2
            uint32_t consumable;                  // parameter_3 bool
            uint32_t min_success_opens;           // parameter_4
            uint32_t max_success_opens;           // parameter_5
            uint32_t event_id;                    // parameter_6 lootedEvent
            uint32_t linked_trap_id;              // parameter_7
            uint32_t quest_id;                    // parameter_8 quest required for GO activation
            uint32_t level;                       // parameter_9
            uint32_t los_OK;                      // parameter_10
            uint32_t leave_loot;                  // parameter_11
            uint32_t not_in_combat;               // parameter_12
            uint32_t log_loot;                    // parameter_13
            uint32_t open_text_id;                // parameter_14
            uint32_t group_loot_rules;            // parameter_15
            uint32_t floating_tooltip;            // parameter_16
        } chest;
        // 4 GAMEOBJECT_TYPE_BINDER
        // 5 GAMEOBJECT_TYPE_GENERIC
        struct
        {
            uint32_t floatingTooltip;             // parameter_0
            uint32_t highlight;                   // parameter_1
            uint32_t serverOnly;                  // parameter_2
            uint32_t large;                       // parameter_3
            uint32_t floatOnWater;                // parameter_4
            int32_t questID;                      // parameter_5
            uint32_t conditionID1;                // parameter_6
        } _generic;
        // 6 GAMEOBJECT_TYPE_TRAP
        struct
        {
            uint32_t lock_id;                     // parameter_0 from Lock.dbc
            uint32_t level;                       // parameter_1
            uint32_t radius;                      // parameter_2 radius for trap activation
            uint32_t spell_id;                    // parameter_3
            uint32_t charges;                     // parameter_4
            uint32_t cooldown;                    // parameter_5 in secs
            uint32_t auto_close_time;             // parameter_6 in secs
            uint32_t start_delay;                 // parameter_7
            uint32_t server_only;                 // parameter_8
            uint32_t stealthed;                   // parameter_9
            uint32_t large;                       // parameter_10
            uint32_t invisible;                   // parameter_11 not same as parameter_9
            uint32_t open_text_id;                // parameter_12
            uint32_t close_text_id;               // parameter_13
            uint32_t ignore_totems;               // parameter_14
        } trap;
        // 7 GAMEOBJECT_TYPE_CHAIR
        // 8 GAMEOBJECT_TYPE_SPELL_FOCUS
        struct
        {
            uint32_t focus_id;                    // parameter_0
            uint32_t distance;                    // parameter_1
            uint32_t linked_trap_id;              // parameter_2
            uint32_t server_only;                 // parameter_3
            uint32_t quest_id;                    // parameter_4
            uint32_t large;                       // parameter_5
            uint32_t floating_tooltip;            // parameter_6
        } spell_focus;
        // 9 GAMEOBJECT_TYPE_TEXT
        struct
        {
            uint32_t pageID;                      // parameter_0
            uint32_t language;                    // parameter_1
            uint32_t pageMaterial;                // parameter_2
            uint32_t allowMounted;                // parameter_3 Is usable while on mount/vehicle. (0/1)
            uint32_t conditionID1;                // parameter_4
        } text;
        // 10 GAMEOBJECT_TYPE_GOOBER
        struct
        {
            uint32_t lock_id;                     // parameter_0 from Lock.dbc
            uint32_t quest_id;                    // parameter_1
            uint32_t event_id;                    // parameter_2
            uint32_t auto_close_time;             // parameter_3 in secs
            uint32_t custom_anim;                 // parameter_4
            uint32_t consumable;                  // parameter_5
            uint32_t cooldown;                    // parameter_6
            uint32_t page_id;                     // parameter_7
            uint32_t language;                    // parameter_8
            uint32_t page_material;               // parameter_9
            uint32_t spell_id;                    // parameter_10
            uint32_t no_damage_immune;            // parameter_11
            uint32_t linked_trap_id;              // parameter_12
            uint32_t large;                       // parameter_13
            uint32_t open_text_iD;                // parameter_14
            uint32_t close_text_iD;               // parameter_15
            uint32_t los_ok;                      // parameter_16 BgObject
            uint32_t allow_mounted;               // parameter_17
            uint32_t floating_tooltip;            // parameter_18
            uint32_t gossip_id;                   // parameter_19
            uint32_t world_state_sets_state;      // parameter_20
        }goober;
        // 11 GAMEOBJECT_TYPE_TRANSPORT
        struct
        {
            uint32_t pause;                       //parameter_0
            uint32_t startOpen;                   //parameter_1
            uint32_t autoCloseTime;               //parameter_2 secs till autoclose = autoCloseTime / 0x10000
            uint32_t pause1EventID;               //parameter_3
            uint32_t pause2EventID;               //parameter_4
            uint32_t mapID;                       //parameter_5
        } transport;
        // 12 GAMEOBJECT_TYPE_AREADAMAGE
        struct
        {
            uint32_t lockId;                      // parameter_0
            uint32_t radius;                      // parameter_1
            uint32_t damageMin;                   // parameter_2
            uint32_t damageMax;                   // parameter_3
            uint32_t damageSchool;                // parameter_4
            uint32_t autoCloseTime;               // parameter_5 secs till autoclose = autoCloseTime / 0x10000
            uint32_t openTextID;                  // parameter_6
            uint32_t closeTextID;                 // parameter_7
        } areadamage;
        // 13 GAMEOBJECT_TYPE_CAMERA
        struct
        {
            uint32_t lock_id;                     // parameter_0 from Lock.dbc
            uint32_t cinematic_id;                // parameter_1
            uint32_t event_id;                    // parameter_2
            uint32_t open_text_id;                // parameter_3
        }camera;
        // 14 GAMEOBJECT_TYPE_MAP_OBJECT
        // 15 GAMEOBJECT_TYPE_MO_TRANSPORT
        struct
        {
            uint32_t taxi_path_id;                // parameter_0
            uint32_t move_speed;                  // parameter_1
            uint32_t accel_rate;                  // parameter_2
            uint32_t start_event_id;              // parameter_3
            uint32_t stop_event_id;               // parameter_4
            uint32_t transport_physics;           // parameter_5
            uint32_t map_id;                      // parameter_6
            uint32_t world_state;                 // parameter_7
            uint32_t can_be_stopped;              // parameter_8
        } mo_transport;
        // 16 GAMEOBJECT_TYPE_DUEL_ARBITER
        // 17 GAMEOBJECT_TYPE_FISHINGNODE
        // 18 GAMEOBJECT_TYPE_RITUAL
        struct
        {
            uint32_t req_participants;            // parameter_0
            uint32_t spell_id;                    // parameter_1
            uint32_t anim_spell;                  // parameter_2
            uint32_t ritual_persistent;           // parameter_3
            uint32_t caster_target_spell;         // parameter_4
            uint32_t caster_target_spell_targets; // parameter_5
            uint32_t casters_grouped;             // parameter_6
            uint32_t ritual_no_target_check;      // parameter_7
        }summoning_ritual;
        // 19 GAMEOBJECT_TYPE_MAILBOX
        // 20 GAMEOBJECT_TYPE_AUCTIONHOUSE
        // 21 GAMEOBJECT_TYPE_GUARDPOST
        struct
        {
            uint32_t creatureID;                  // parameter_0
            uint32_t charges;                     // parameter_1
        } guardpost;
        // 22 GAMEOBJECT_TYPE_SPELLCASTER
        struct
        {
            uint32_t spell_id;                    // parameter_0
            uint32_t charges;                     // parameter_1
            uint32_t party_only;                  // parameter_2
            uint32_t allow_mounted;               // parameter_3
            uint32_t large;                       // parameter_4
        }spell_caster;
        // 23 GAMEOBJECT_TYPE_MEETINGSTONE
        // 24 GAMEOBJECT_TYPE_FLAGSTAND
        struct
        {
            uint32_t lockId;                      // parameter_0
            uint32_t pickupSpell;                 // parameter_1
            uint32_t radius;                      // parameter_2
            uint32_t returnAura;                  // parameter_3
            uint32_t returnSpell;                 // parameter_4
            uint32_t noDamageImmune;              // parameter_5
            uint32_t openTextID;                  // parameter_6
            uint32_t losOK;                       // parameter_7
            uint32_t conditionID1;                // parameter_8
        } flagstand;
        // 25 GAMEOBJECT_TYPE_FISHINGHOLE
        struct
        {
            uint32_t radius;                      // parameter_0
            uint32_t loot_id;                     // parameter_1
            uint32_t min_success_opens;           // parameter_2
            uint32_t max_success_opens;           // parameter_3
            uint32_t lock_id;                     // parameter_4 from Lock.dbc
        }fishinghole;
        // 26 GAMEOBJECT_TYPE_FLAGDROP
        struct
        {
            uint32_t lockId;                      // parameter_0
            uint32_t eventID;                     // parameter_1
            uint32_t pickupSpell;                 // parameter_2
            uint32_t noDamageImmune;              // parameter_3
            uint32_t openTextID;                  // parameter_4
        } flagdrop;
        // 27 GAMEOBJECT_TYPE_MINI_GAME
        // 28 GAMEOBJECT_TYPE_LOTTERY_KIOSK
        // 29 GAMEOBJECT_TYPE_CAPTURE_POINT
        struct
        {
            uint32_t radius;                      // parameter_0
            uint32_t spell;                       // parameter_1
            uint32_t worldState1;                 // parameter_2
            uint32_t worldstate2;                 // parameter_3
            uint32_t winEventID1;                 // parameter_4
            uint32_t winEventID2;                 // parameter_5
            uint32_t contestedEventID1;           // parameter_6
            uint32_t contestedEventID2;           // parameter_7
            uint32_t progressEventID1;            // parameter_8
            uint32_t progressEventID2;            // parameter_9
            uint32_t neutralEventID1;             // parameter_10
            uint32_t neutralEventID2;             // parameter_11
            uint32_t neutralPercent;              // parameter_12
            uint32_t worldstate3;                 // parameter_13
            uint32_t minSuperiority;              // parameter_14
            uint32_t maxSuperiority;              // parameter_15
            uint32_t minTime;                     // parameter_16
            uint32_t maxTime;                     // parameter_17
            uint32_t large;                       // parameter_18
            uint32_t highlight;                   // parameter_19
            uint32_t startingValue;               // parameter_20
            uint32_t unidirectional;              // parameter_21
        } capturePoint;
        // 30 GAMEOBJECT_TYPE_AURA_GENERATOR
        // 31 GAMEOBJECT_TYPE_DUNGEON_DIFFICULTY
        // 32 GAMEOBJECT_TYPE_BARBER_CHAIR
        // 33 GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING
        struct
        {
            uint32_t intact_num_hits;             // parameter_0
            uint32_t credit_proxy_creature;       // parameter_1
            uint32_t state1_name;                 // parameter_2
            uint32_t intact_event;                // parameter_3
            uint32_t damaged_display_id;          // parameter_4
            uint32_t damaged_num_hits;            // parameter_5
            uint32_t unused1;                     // parameter_6
            uint32_t unused2;                     // parameter_7
            uint32_t unused3;                     // parameter_8
            uint32_t damaged_event;               // parameter_9
            uint32_t destroyed_display_id;        // parameter_10
            uint32_t unused4;                     // parameter_11
            uint32_t unused5;                     // parameter_12
            uint32_t unused6;                     // parameter_13
            uint32_t destroyed_event;             // parameter_14
            uint32_t unused7;                     // parameter_15
            uint32_t debuilding_time_secs;        // parameter_16
            uint32_t unused8;                     // parameter_17
            uint32_t destructible_data;           // parameter_18
            uint32_t rebuilding_event;            // parameter_19
            uint32_t unused9;                     // parameter_20
            uint32_t unused10;                    // parameter_21
            uint32_t damage_event;                // parameter_22
            uint32_t unused11;                    // parameter_23
        } destructible_building;
        // 34 GAMEOBJECT_TYPE_GUILD_BANK
        // 35 GAMEOBJECT_TYPE_TRAPDOOR

        // raw parameters
        struct
        {
            uint32_t parameter_0;
            uint32_t parameter_1;
            uint32_t parameter_2;
            uint32_t parameter_3;
            uint32_t parameter_4;
            uint32_t parameter_5;
            uint32_t parameter_6;
            uint32_t parameter_7;
            uint32_t parameter_8;
            uint32_t parameter_9;
            uint32_t parameter_10;
            uint32_t parameter_11;
            uint32_t parameter_12;
            uint32_t parameter_13;
            uint32_t parameter_14;
            uint32_t parameter_15;
            uint32_t parameter_16;
            uint32_t parameter_17;
            uint32_t parameter_18;
            uint32_t parameter_19;
            uint32_t parameter_20;
            uint32_t parameter_21;
            uint32_t parameter_22;
            uint32_t parameter_23;
        }raw;
    };
    float size;
    uint32_t QuestItems[6];

    // Quests
    // <quest, requirement_count>
    std::unordered_map<QuestProperties const*, uint32_t>  goMap;
    // <quest, [<item, item_count>]>
    std::unordered_map<QuestProperties const*, std::map<uint32_t, uint32_t>> itemMap;

    uint32_t getLootId() const
    {
        switch (type)
        {
            case GAMEOBJECT_TYPE_CHEST:       return chest.loot_id;
            case GAMEOBJECT_TYPE_FISHINGHOLE: return fishinghole.loot_id;
            default: return 0;
        }
    }

    uint32_t getCooldown() const                              // Cooldown preventing goober and traps to cast spell
    {
        switch (type)
        {
            case GAMEOBJECT_TYPE_TRAP:        return trap.cooldown;
            case GAMEOBJECT_TYPE_GOOBER:      return goober.cooldown;
            default: return 0;
        }
    }

    uint32_t getLockId() const
    {
        switch (type)
        {
            case GAMEOBJECT_TYPE_DOOR:       return door.lock_id;
            case GAMEOBJECT_TYPE_BUTTON:     return button.lock_id;
            case GAMEOBJECT_TYPE_QUESTGIVER: return questgiver.lockId;
            case GAMEOBJECT_TYPE_CHEST:      return chest.lock_id;
            case GAMEOBJECT_TYPE_TRAP:       return trap.lock_id;
            case GAMEOBJECT_TYPE_GOOBER:     return goober.lock_id;
            case GAMEOBJECT_TYPE_AREADAMAGE: return areadamage.lockId;
            case GAMEOBJECT_TYPE_CAMERA:     return camera.lock_id;
            case GAMEOBJECT_TYPE_FLAGSTAND:  return flagstand.lockId;
            case GAMEOBJECT_TYPE_FISHINGHOLE:return fishinghole.lock_id;
            case GAMEOBJECT_TYPE_FLAGDROP:   return flagdrop.lockId;
            default: return 0;
        }
    }

    uint32_t getCharges() const                               // despawn at uses amount
    {
        switch (type)
        {
            //case GAMEOBJECT_TYPE_TRAP:        return trap.charges;
            case GAMEOBJECT_TYPE_GUARDPOST:   return guardpost.charges;
            case GAMEOBJECT_TYPE_SPELLCASTER: return spell_caster.charges;
            default: return 0;
        }
    }

    bool isDespawnAtAction() const
    {
        switch (type)
        {
            case GAMEOBJECT_TYPE_CHEST:  return chest.consumable != 0;
            case GAMEOBJECT_TYPE_GOOBER: return goober.consumable != 0;
            default: return false;
        }
    }

    bool getDespawnPossibility() const
    {
        switch (type)
        {
            case GAMEOBJECT_TYPE_DOOR:       return door.no_damage_immune != 0;
            case GAMEOBJECT_TYPE_BUTTON:     return button.no_damage_immune != 0;
            case GAMEOBJECT_TYPE_QUESTGIVER: return questgiver.noDamageImmune != 0;
            case GAMEOBJECT_TYPE_GOOBER:     return goober.no_damage_immune != 0;
            case GAMEOBJECT_TYPE_FLAGSTAND:  return flagstand.noDamageImmune != 0;
            case GAMEOBJECT_TYPE_FLAGDROP:   return flagdrop.noDamageImmune != 0;
            default: return true;
        }
    }

    uint32_t getLinkedGameObjectEntry() const
    {
        switch (type)
        {
            case GAMEOBJECT_TYPE_BUTTON:      return button.linked_trap_id;
            case GAMEOBJECT_TYPE_CHEST:       return chest.linked_trap_id;
            case GAMEOBJECT_TYPE_SPELL_FOCUS: return spell_focus.linked_trap_id;
            case GAMEOBJECT_TYPE_GOOBER:      return goober.linked_trap_id;
            default: return 0;
        }
    }

    uint32_t getAutoCloseTime() const
    {
        uint32_t autoCloseTime = 0;
        switch (type)
        {
            case GAMEOBJECT_TYPE_DOOR:          autoCloseTime = door.auto_close_time; break;
            case GAMEOBJECT_TYPE_BUTTON:        autoCloseTime = button.auto_close_time; break;
            case GAMEOBJECT_TYPE_TRAP:          autoCloseTime = trap.auto_close_time; break;
            case GAMEOBJECT_TYPE_GOOBER:        autoCloseTime = goober.auto_close_time; break;
            case GAMEOBJECT_TYPE_TRANSPORT:     autoCloseTime = transport.autoCloseTime; break;
            case GAMEOBJECT_TYPE_AREADAMAGE:    autoCloseTime = areadamage.autoCloseTime; break;
            default: break;
        }
        return autoCloseTime; // prior to 3.0.3, conversion was / 0x10000;
    }

    bool isLargeGameObject() const
    {
        //todo disable for now Gameobjects wont Update in WorldMap
        return false;
        /*switch (type)
        {
            case GAMEOBJECT_TYPE_BUTTON:            return button.large != 0;
            case GAMEOBJECT_TYPE_QUESTGIVER:        return questgiver.large != 0;
            case GAMEOBJECT_TYPE_GENERIC:           return _generic.large != 0;
            case GAMEOBJECT_TYPE_TRAP:              return trap.large != 0;
            case GAMEOBJECT_TYPE_SPELL_FOCUS:       return spell_focus.large != 0;
            case GAMEOBJECT_TYPE_GOOBER:            return goober.large != 0;
            case GAMEOBJECT_TYPE_SPELLCASTER:       return spell_caster.large != 0;
            case GAMEOBJECT_TYPE_CAPTURE_POINT:     return capturePoint.large != 0;
            default: return false;
        }*/
    }

    bool isInfiniteGameObject() const
    {
        //todo disable for now Gameobjects wont Update in WorldMap
        return false;
        /*switch (type)
        {
            case GAMEOBJECT_TYPE_DOOR:                  return true;
            case GAMEOBJECT_TYPE_FLAGSTAND:             return true;
            case GAMEOBJECT_TYPE_FLAGDROP:              return true;
            case GAMEOBJECT_TYPE_TRAPDOOR:              return true;
            case GAMEOBJECT_TYPE_TRANSPORT:             return true;
            default: return false;
        }*/
    }

    bool cannotBeUsedUnderImmunity() const // Cannot be used/activated/looted by players under immunity effects (example: Divine Shield)
    {
        switch (type)
        {
            case GAMEOBJECT_TYPE_DOOR:       return door.no_damage_immune != 0;
            case GAMEOBJECT_TYPE_BUTTON:     return button.no_damage_immune != 0;
            case GAMEOBJECT_TYPE_QUESTGIVER: return questgiver.noDamageImmune != 0;
            case GAMEOBJECT_TYPE_CHEST:      return true;                           // All chests cannot be opened while immune on 3.3.5a
            case GAMEOBJECT_TYPE_GOOBER:     return goober.no_damage_immune != 0;
            case GAMEOBJECT_TYPE_FLAGSTAND:  return flagstand.noDamageImmune != 0;
            case GAMEOBJECT_TYPE_FLAGDROP:   return flagdrop.noDamageImmune != 0;
            default: return false;
        }
    }

    bool isUsableMounted() const
    {
        switch (type)
        {
            case GAMEOBJECT_TYPE_MAILBOX: return true;
            case GAMEOBJECT_TYPE_BARBER_CHAIR: return false;
            case GAMEOBJECT_TYPE_QUESTGIVER: return questgiver.allowMounted != 0;
            case GAMEOBJECT_TYPE_TEXT: return text.allowMounted != 0;
            case GAMEOBJECT_TYPE_GOOBER: return goober.allow_mounted != 0;
            case GAMEOBJECT_TYPE_SPELLCASTER: return spell_caster.allow_mounted != 0;
            default: return false;
        }
    }
};
