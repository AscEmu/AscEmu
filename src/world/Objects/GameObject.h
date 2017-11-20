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

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Management/CRitual.h"
#include "Management/QuestMgr.h"
#include "Map/Map.h"

enum GameObject_State : uint8_t
{
    GO_STATE_OPEN               = 0,
    GO_STATE_CLOSED             = 1,
    GO_STATE_ALTERNATIVE_OPEN   = 2
};

enum GameObject_Flags
{
    GO_FLAG_NONE                = 0x000,
    GO_FLAG_NONSELECTABLE       = 0x001,
    GO_FLAG_LOCKED              = 0x002,
    GO_FLAG_UNTARGETABLE        = 0x004,
    GO_FLAG_TRANSPORT           = 0x008,
    GO_FLAG_NOT_SELECTABLE      = 0x010,
    GO_FLAG_NEVER_DESPAWN       = 0x020,
    GO_FLAG_TRIGGERED           = 0x040,
    GO_FLAG_UNK1                = 0x080,
    GO_FLAG_UNK2                = 0x100,
    GO_FLAG_DAMAGED             = 0x200,
    GO_FLAG_DESTROYED           = 0x400
};

class Player;
class GameObjectAIScript;
class GameObjectModel;

enum GameObjectOverrides
{
    GAMEOBJECT_INFVIS = 0x01,                   /// Makes the gameobject forever visible on the map after you saw it at least once - for various transports; actually it just doesn't erase it while you're on the same map.
    GAMEOBJECT_MAPWIDE = 0x02,                  /// When you enter its map, the gameobject gets pushed to you no matter how far it is (but only for players), especially for Deeprun and Ulduar Trams.
    GAMEOBJECT_AREAWIDE = 0x04,                 ///\todo UNIMPLEMENTED, but will work like this: the Map will get marked that it contains an object like this, and on player movement these objects will get distance-checked to spawn them from a greater distance than normal if needed - for few objects on smaller maps, like on battlegrounds; maybe they'll get area-triggered, haven't decided yet.
    GAMEOBJECT_ONMOVEWIDE = 0x08,               /// When this gameobject moves and sends updates about it's position, do so in the second range - MapMgr::ChangeObjectLocation, +/- 6 units wide instead of +/- 1.
    GAMEOBJECT_OVERRIDE_FLAGS = 0x10,           ///\todo UNIMPLEMENTED, Let the core decide about the flags sent in the A9 - example: 252 instead of 352 for Deeprun Tram.
    GAMEOBJECT_OVERRIDE_BYTES1 = 0x20,          ///\todo UNIMPLEMENTED, Let the core use the full field instead an uint8 in GAMEOBJECT_BYTES_1, if the database creator knows what to do with it.
    GAMEOBJECT_OVERRIDE_PARENTROT = 0x40,       /// Makes it possible for the core to skip calculating these fields and use whatever was specified in the spawn.
    /// Later other types might folow, or the upper bytes might get used for the AREAWIDE option in the overrides variable...
};

typedef std::unordered_map<QuestProperties const*, uint32 > GameObjectGOMap;
typedef std::unordered_map<QuestProperties const*, std::map<uint32, uint32> > GameObjectItemMap;

struct GameObjectProperties
{
    uint32 entry;
    uint32 type;
    uint32 display_id;
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
            uint32 start_open;                  // parameter_0 client side opened/closed
            uint32 lock_id;                     // parameter_1 from Lock.dbc
            uint32 auto_close_time;             // parameter_2 in secs
            uint32 no_damage_immune;            // parameter_3
            uint32 open_text_id;                // parameter_4
            uint32 close_text_id;               // parameter_5
            uint32 ignored_by_pathing;          // parameter_6
        }door;
        // 1 GAMEOBJECT_TYPE_BUTTON
        struct
        {
            uint32 start_open;                  // parameter_0
            uint32 lock_id;                     // parameter_1 from Lock.dbc
            uint32 auto_close_time;             // parameter_2 in secs
            uint32 linked_trap_id;              // parameter_3
            uint32 no_damage_immune;            // parameter_4 BgObject
            uint32 large;                       // parameter_5
            uint32 open_text_id;                // parameter_6
            uint32 close_text_id;               // parameter_7
            uint32 los_ok;                      // parameter_8
        }button;
        // 2 GAMEOBJECT_TYPE_QUESTGIVER
        // 3 GAMEOBJECT_TYPE_CHEST
        struct
        {
            uint32 lock_id;                     // parameter_0 from Lock.dbc
            uint32 loot_id;                     // parameter_1
            uint32 restock_time;                // parameter_2
            uint32 consumable;                  // parameter_3 bool
            uint32 min_success_opens;           // parameter_4
            uint32 max_success_opens;           // parameter_5
            uint32 event_id;                    // parameter_6 lootedEvent
            uint32 linked_trap_id;              // parameter_7
            uint32 quest_id;                    // parameter_8 quest required for GO activation
            uint32 level;                       // parameter_9
            uint32 los_OK;                      // parameter_10
            uint32 leave_loot;                  // parameter_11
            uint32 not_in_combat;               // parameter_12
            uint32 log_loot;                    // parameter_13
            uint32 open_text_id;                // parameter_14
            uint32 group_loot_rules;            // parameter_15
            uint32 floating_tooltip;            // parameter_16
        } chest;
        // 4 GAMEOBJECT_TYPE_BINDER
        // 5 GAMEOBJECT_TYPE_GENERIC
        // 6 GAMEOBJECT_TYPE_TRAP
        struct
        {
            uint32 lock_id;                     // parameter_0 from Lock.dbc
            uint32 level;                       // parameter_1
            uint32 radius;                      // parameter_2 radius for trap activation
            uint32 spell_id;                    // parameter_3
            uint32 charges;                     // parameter_4
            uint32 cooldown;                    // parameter_5 in secs
            uint32 auto_close_time;             // parameter_6 in secs
            uint32 start_delay;                 // parameter_7
            uint32 server_only;                 // parameter_8
            uint32 stealthed;                   // parameter_9
            uint32 large;                       // parameter_10
            uint32 stealth_affected;            // parameter_11
            uint32 open_text_id;                // parameter_12
            uint32 close_text_id;               // parameter_13
            uint32 ignore_totems;               // parameter_14
        } trap;
        // 7 GAMEOBJECT_TYPE_CHAIR
        // 8 GAMEOBJECT_TYPE_SPELL_FOCUS
        struct
        {
            uint32 focus_id;                    // parameter_0
            uint32 distance;                    // parameter_1
            uint32 linked_trap_id;              // parameter_2
            uint32 server_only;                 // parameter_3
            uint32 quest_id;                    // parameter_4
            uint32 large;                       // parameter_5
            uint32 floating_tooltip;            // parameter_6
        } spell_focus;
        // 9 GAMEOBJECT_TYPE_TEXT
        // 10 GAMEOBJECT_TYPE_GOOBER
        struct
        {
            uint32 lock_id;                     // parameter_0 from Lock.dbc
            uint32 quest_id;                    // parameter_1
            uint32 event_id;                    // parameter_2
            uint32 auto_close_time;             // parameter_3 in secs
            uint32 custom_anim;                 // parameter_4
            uint32 consumable;                  // parameter_5
            uint32 cooldown;                    // parameter_6
            uint32 page_id;                     // parameter_7
            uint32 language;                    // parameter_8
            uint32 page_material;               // parameter_9
            uint32 spell_id;                    // parameter_10
            uint32 no_damage_immune;            // parameter_11
            uint32 linked_trap_id;              // parameter_12
            uint32 large;                       // parameter_13
            uint32 open_text_iD;                // parameter_14
            uint32 close_text_iD;               // parameter_15
            uint32 los_ok;                      // parameter_16 BgObject
            uint32 allow_mounted;               // parameter_17
            uint32 floating_tooltip;            // parameter_18
            uint32 gossip_id;                   // parameter_19
            uint32 world_state_sets_state;      // parameter_20
        }goober;
        // 11 GAMEOBJECT_TYPE_TRANSPORT
        // 12 GAMEOBJECT_TYPE_AREADAMAGE
        // 13 GAMEOBJECT_TYPE_CAMERA
        struct
        {
            uint32 lock_id;                     // parameter_0 from Lock.dbc
            uint32 cinematic_id;                // parameter_1
            uint32 event_id;                    // parameter_2
            uint32 open_text_id;                // parameter_3
        }camera;
        // 14 GAMEOBJECT_TYPE_MAP_OBJECT
        // 15 GAMEOBJECT_TYPE_MO_TRANSPORT
        struct
        {
            uint32 taxi_path_id;                // parameter_0
            uint32 move_speed;                  // parameter_1
            uint32 accel_rate;                  // parameter_2
            uint32 start_event_id;              // parameter_3
            uint32 stop_event_id;               // parameter_4
            uint32 transport_physics;           // parameter_5
            uint32 map_id;                      // parameter_6
            uint32 world_state;                 // parameter_7
            uint32 can_be_stopped;              // parameter_8
        } mo_transport;
        // 16 GAMEOBJECT_TYPE_DUEL_ARBITER
        // 17 GAMEOBJECT_TYPE_FISHINGNODE
        // 18 GAMEOBJECT_TYPE_RITUAL
        struct
        {
            uint32 req_participants;            // parameter_0
            uint32 spell_id;                    // parameter_1
            uint32 anim_spell;                  // parameter_2
            uint32 ritual_persistent;           // parameter_3
            uint32 caster_target_spell;         // parameter_4
            uint32 caster_target_spell_targets; // parameter_5
            uint32 casters_grouped;             // parameter_6
            uint32 ritual_no_target_check;      // parameter_7
        }summoning_ritual;
        // 19 GAMEOBJECT_TYPE_MAILBOX
        // 20 GAMEOBJECT_TYPE_AUCTIONHOUSE
        // 21 GAMEOBJECT_TYPE_GUARDPOST
        // 22 GAMEOBJECT_TYPE_SPELLCASTER
        struct
        {
            uint32 spell_id;                    // parameter_0
            uint32 charges;                     // parameter_1
            uint32 party_only;                  // parameter_2
            uint32 allow_mounted;               // parameter_3
            uint32 large;                       // parameter_4
        }spell_caster;
        // 23 GAMEOBJECT_TYPE_MEETINGSTONE
        // 24 GAMEOBJECT_TYPE_FLAGSTAND
        // 25 GAMEOBJECT_TYPE_FISHINGHOLE
        struct
        {
            uint32 radius;                      // parameter_0
            uint32 loot_id;                     // parameter_1
            uint32 min_success_opens;           // parameter_2
            uint32 max_success_opens;           // parameter_3
            uint32 lock_id;                     // parameter_4 from Lock.dbc
        }fishinghole;
        // 26 GAMEOBJECT_TYPE_FLAGDROP
        // 27 GAMEOBJECT_TYPE_MINI_GAME
        // 28 GAMEOBJECT_TYPE_LOTTERY_KIOSK
        // 29 GAMEOBJECT_TYPE_CAPTURE_POINT
        // 30 GAMEOBJECT_TYPE_AURA_GENERATOR
        // 31 GAMEOBJECT_TYPE_DUNGEON_DIFFICULTY
        // 32 GAMEOBJECT_TYPE_BARBER_CHAIR
        // 33 GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING
        struct
        {
            uint32 intact_num_hits;             // parameter_0
            uint32 credit_proxy_creature;       // parameter_1
            uint32 state1_name;                 // parameter_2
            uint32 intact_event;                // parameter_3
            uint32 damaged_display_id;          // parameter_4
            uint32 damaged_num_hits;            // parameter_5
            uint32 unused1;                     // parameter_6
            uint32 unused2;                     // parameter_7
            uint32 unused3;                     // parameter_8
            uint32 damaged_event;               // parameter_9
            uint32 destroyed_display_id;        // parameter_10
            uint32 unused4;                     // parameter_11
            uint32 unused5;                     // parameter_12
            uint32 unused6;                     // parameter_13
            uint32 destroyed_event;             // parameter_14
            uint32 unused7;                     // parameter_15
            uint32 debuilding_time_secs;        // parameter_16
            uint32 unused8;                     // parameter_17
            uint32 destructible_data;           // parameter_18
            uint32 rebuilding_event;            // parameter_19
            uint32 unused9;                     // parameter_20
            uint32 unused10;                    // parameter_21
            uint32 damage_event;                // parameter_22
            uint32 unused11;                    // parameter_23
        } destructible_building;
        // 34 GAMEOBJECT_TYPE_GUILD_BANK
        // 35 GAMEOBJECT_TYPE_TRAPDOOR

        // raw parameters
        struct
        {
            uint32 parameter_0;
            uint32 parameter_1;
            uint32 parameter_2;
            uint32 parameter_3;
            uint32 parameter_4;
            uint32 parameter_5;
            uint32 parameter_6;
            uint32 parameter_7;
            uint32 parameter_8;
            uint32 parameter_9;
            uint32 parameter_10;
            uint32 parameter_11;
            uint32 parameter_12;
            uint32 parameter_13;
            uint32 parameter_14;
            uint32 parameter_15;
            uint32 parameter_16;
            uint32 parameter_17;
            uint32 parameter_18;
            uint32 parameter_19;
            uint32 parameter_20;
            uint32 parameter_21;
            uint32 parameter_22;
            uint32 parameter_23;
        }raw;
    };
    float size;
    uint32 QuestItems[6];

    // Quests
    GameObjectGOMap goMap;
    GameObjectItemMap itemMap;
};

enum GameObjectBytes
{
    GAMEOBJECT_BYTES_STATE          = 0,
    GAMEOBJECT_BYTES_TYPE_ID        = 1,
    GAMEOBJECT_BYTES_UNK            = 2,        //\todo unknown atm
    GAMEOBJECT_BYTES_ANIMPROGRESS   = 3,
};

enum GameObjectTypes
{
    GAMEOBJECT_TYPE_DOOR                    = 0,
    GAMEOBJECT_TYPE_BUTTON                  = 1,
    GAMEOBJECT_TYPE_QUESTGIVER              = 2,
    GAMEOBJECT_TYPE_CHEST                   = 3,
    GAMEOBJECT_TYPE_BINDER                  = 4,
    GAMEOBJECT_TYPE_GENERIC                 = 5,
    GAMEOBJECT_TYPE_TRAP                    = 6,
    GAMEOBJECT_TYPE_CHAIR                   = 7,
    GAMEOBJECT_TYPE_SPELL_FOCUS             = 8,
    GAMEOBJECT_TYPE_TEXT                    = 9,
    GAMEOBJECT_TYPE_GOOBER                  = 10,
    GAMEOBJECT_TYPE_TRANSPORT               = 11,
    GAMEOBJECT_TYPE_AREADAMAGE              = 12,
    GAMEOBJECT_TYPE_CAMERA                  = 13,
    GAMEOBJECT_TYPE_MAP_OBJECT              = 14,
    GAMEOBJECT_TYPE_MO_TRANSPORT            = 15,
    GAMEOBJECT_TYPE_DUEL_ARBITER            = 16,
    GAMEOBJECT_TYPE_FISHINGNODE             = 17,
    GAMEOBJECT_TYPE_RITUAL                  = 18,
    GAMEOBJECT_TYPE_MAILBOX                 = 19,
    GAMEOBJECT_TYPE_AUCTIONHOUSE            = 20,
    GAMEOBJECT_TYPE_GUARDPOST               = 21,
    GAMEOBJECT_TYPE_SPELLCASTER             = 22,
    GAMEOBJECT_TYPE_MEETINGSTONE            = 23,
    GAMEOBJECT_TYPE_FLAGSTAND               = 24,
    GAMEOBJECT_TYPE_FISHINGHOLE             = 25,
    GAMEOBJECT_TYPE_FLAGDROP                = 26,
    GAMEOBJECT_TYPE_MINI_GAME               = 27,
    GAMEOBJECT_TYPE_LOTTERY_KIOSK           = 28,
    GAMEOBJECT_TYPE_CAPTURE_POINT           = 29,
    GAMEOBJECT_TYPE_AURA_GENERATOR          = 30,
    GAMEOBJECT_TYPE_DUNGEON_DIFFICULTY      = 31,
    GAMEOBJECT_TYPE_BARBER_CHAIR            = 32,
    GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING   = 33,
    GAMEOBJECT_TYPE_GUILD_BANK              = 34,
    GAMEOBJECT_TYPE_TRAPDOOR                = 35
};

#define CALL_GO_SCRIPT_EVENT(obj, func) if (obj->IsGameObject() && static_cast< GameObject* >(obj)->GetScript() != NULL) static_cast< GameObject* >(obj)->GetScript()->func

class SERVER_DECL GameObject : public Object
{
    public:

        GameObject(uint64 guid);
        ~GameObject();

        GameEvent* mEvent = nullptr;

        GameObjectProperties const* GetGameObjectProperties() { return gameobject_properties; }
        void SetGameObjectProperties(GameObjectProperties const* go_prop) { gameobject_properties = go_prop; }

        bool CreateFromProto(uint32 entry, uint32 mapid, float x, float y, float z, float ang, float r0 = 0.0f, float r1 = 0.0f, float r2 = 0.0f, float r3 = 0.0f, uint32 overrides = 0);

        bool Load(GameobjectSpawn* spawn);

        virtual bool IsLootable() { return false; }

        virtual void Use(uint64 /*GUID*/) {}
        void CastSpell(uint64 TargetGUID, SpellInfo* sp);
        void CastSpell(uint64 TargetGUID, uint32 SpellID);

        void Update(unsigned long time_passed);

        void Spawn(MapMgr* m);
        void Despawn(uint32 delay, uint32 respawntime);

        //void _EnvironmentalDamageUpdate();
        // Serialization
        void SaveToDB();
        void SaveToFile(std::stringstream & name);
        void DeleteFromDB();

        // z_rot, y_rot, x_rot - rotation angles around z, y and x axes
        void SetRotationAngles(float z_rot, float y_rot, float x_rot);
        int64 GetRotation() const { return m_rotation; }

        void SetSummoned(Unit* mob)
        {
            m_summoner = mob;
            m_summonedGo = true;
        }
        void _Expire();

        void ExpireAndDelete();

        int32 charges;

        virtual void InitAI();

        bool invisible;     // invisible
        uint8 invisibilityFlag;
        Unit* m_summoner;

        void CallScriptUpdate();

        GameObjectAIScript* GetScript() { return myScript; }

        GameobjectSpawn* m_spawn;
        void OnPushToWorld();
        void OnRemoveInRangeObject(Object* pObj);
        void RemoveFromWorld(bool free_guid);

        uint32 GetGOReqSkill();
        MapCell* m_respawnCell;

#if VERSION_STRING < WotLK
        void SetState(uint8 state) { setUInt32Value(GAMEOBJECT_STATE, state); }
        uint8 GetState() { return getUInt32Value(GAMEOBJECT_STATE); }

        void SetType(uint8 type) { setUInt32Value(GAMEOBJECT_TYPE_ID, type); }
        uint32 GetType() { return this->GetGameObjectProperties()->type; }

        void SetArtKit(uint8 artkit) { setUInt32Value(GAMEOBJECT_ARTKIT, artkit); }
        uint8 GetArtkKit() { return getUInt32Value(GAMEOBJECT_ARTKIT); }

        void SetAnimProgress(uint8 progress) { setUInt32Value(GAMEOBJECT_ANIMPROGRESS, progress); }
        uint8 GetAnimProgress() { return getUInt32Value(GAMEOBJECT_ANIMPROGRESS); }
#else
        void SetState(uint8 state) { setByteValue(GAMEOBJECT_BYTES_1, 0, state); }
        uint8 GetState() { return getByteValue(GAMEOBJECT_BYTES_1, 0); }

        void SetType(uint8 type) { setByteValue(GAMEOBJECT_BYTES_1, 1, type); }
        uint32 GetType() { return this->GetGameObjectProperties()->type; }

        void SetArtKit(uint8 artkit) { setByteValue(GAMEOBJECT_BYTES_1, 2, artkit); }
        uint8 GetArtkKit() { return getByteValue(GAMEOBJECT_BYTES_1, 2); }

        void SetAnimProgress(uint8 progress) { setByteValue(GAMEOBJECT_BYTES_1, 3, progress); }
        uint8 GetAnimProgress() { return getByteValue(GAMEOBJECT_BYTES_1, 3); }
#endif

        void SetOverrides(uint32 go_overrides) { m_overrides = go_overrides; }
        uint32 GetOverrides() { return m_overrides; }

        void Deactivate() { setUInt32Value(GAMEOBJECT_DYNAMIC, 0); }
        void Activate() { setUInt32Value(GAMEOBJECT_DYNAMIC, 1); }
        bool IsActive()
        {
            if (m_uint32Values[GAMEOBJECT_DYNAMIC] == 1)
                return true;
            else
                return false;
        }

        void SetDisplayId(uint32 id) { setUInt32Value(GAMEOBJECT_DISPLAYID, id); }
        uint32 GetDisplayId() { return getUInt32Value(GAMEOBJECT_DISPLAYID); }

        void SetRotationQuat(float qx, float qy, float qz, float qw);

        void SetParentRotation(uint8 rot, float value) { setFloatValue(GAMEOBJECT_PARENTROTATION + rot, value); }
        float GetParentRotation(uint8 rot) { return getFloatValue(GAMEOBJECT_PARENTROTATION + rot); }

        void SetFaction(uint32 id)
        {
            setUInt32Value(GAMEOBJECT_FACTION, id);
            _setFaction();
        }
        uint32 GetFaction() { return getUInt32Value(GAMEOBJECT_FACTION); }

        void SetLevel(uint32 level) { setUInt32Value(GAMEOBJECT_LEVEL, level); }
        uint32 GetLevel() { return getUInt32Value(GAMEOBJECT_LEVEL); }

        void SetFlags(uint32 flags) { setUInt32Value(GAMEOBJECT_FLAGS, flags); }
        uint32 GetFlags() { return getUInt32Value(GAMEOBJECT_FLAGS); }
        void RemoveFlags(uint32 flags) { RemoveFlag(GAMEOBJECT_FLAGS, flags); }

        bool HasFlags(uint32 flags)
        {
            if (HasFlag(GAMEOBJECT_FLAGS, flags) != 0)
                return true;
            else
                return false;
        }

        GameObjectModel* m_model;

    protected:

        bool m_summonedGo;
        bool m_deleted;
        GameObjectProperties const* gameobject_properties;
        GameObjectAIScript* myScript;
        uint32 _fields[GAMEOBJECT_END];

        uint32 m_overrides;             ///See enum GAMEOBJECT_OVERRIDES!

        uint64 m_rotation;

    //MIT
    public:

        void SetCustomAnim(uint32_t anim = 0);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Abstract Base Class for lootables (fishing node, fishing hole, and chests)
//////////////////////////////////////////////////////////////////////////////////////////
class GameObject_Lootable : public GameObject
{
    public:

        GameObject_Lootable(uint64 GUID) : GameObject(GUID) {}
        ~GameObject_Lootable() {}

        virtual bool HasLoot() = 0;

        void resetLoot()
        {
            if (loot.items.empty() || !loot.looters.empty() || loot.HasRoll())
                return;

            lootmgr.FillGOLoot(&loot, gameobject_properties->raw.parameter_1, 0);
        }

        Loot loot;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Implements Type 0 (DOOR) GameObjects
//////////////////////////////////////////////////////////////////////////////////////////
class GameObject_Door : public GameObject
{
    public:

        GameObject_Door(uint64 GUID);
        ~GameObject_Door();

        void InitAI();

        void Use(uint64 GUID);

        void Open();
        void Close();

        /// Opens the door with a special way. Like an explosion.
        void SpecialOpen();
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Implements Type 1 (BUTTON) GameObjects
//////////////////////////////////////////////////////////////////////////////////////////
class GameObject_Button : public GameObject
{
    public:

        GameObject_Button(uint64 GUID);
        ~GameObject_Button();

        void InitAI();

        void Use(uint64 GUID);

        void Open();
        void Close();

    private:

        SpellInfo* spell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// implementing Type 2 (QUESTGIVER) GameObjects
//////////////////////////////////////////////////////////////////////////////////////////
class GameObject_QuestGiver : public GameObject
{
    public:

        GameObject_QuestGiver(uint64 GUID);
        ~GameObject_QuestGiver();

        void InitAI();

        bool HasQuests()
        {
            if (m_quests == NULL)
                return false;

            if (m_quests->size() == 0)
                return false;

            return true;
        };


        uint32 NumOfQuests()
        {
            return static_cast<uint32>(m_quests->size());
        }

        void AddQuest(QuestRelation* Q)
        {
            m_quests->push_back(Q);
        }

        void DeleteQuest(QuestRelation* Q);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Searches for a QuestRelation in the GO and if found, returns the Quest
        /// \param uint32 quest_id  -  Identifier of the Quest
        /// \param uint8 quest_relation  -  QuestRelation type
        /// \return the Quest on success NULL on failure
        QuestProperties const* FindQuest(uint32 quest_id, uint8 quest_relation);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Finds the Quest with quest_id in the GO, and returns it's QuestRelation type
        /// \param uint32 quest_id  -  Identifier of the Quest
        /// \return Returns the QuestRelation type on success, 0 on failure
        uint16 GetQuestRelation(uint32 quest_id);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Returns an iterator to the GO's QuestRelation list beginning
        /// \return an iterator to the QuestRelation list's beginning
        std::list<QuestRelation*>::iterator QuestsBegin()
        {
            return m_quests->begin();
        };

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Returns an iterator to the GO's QuestRelation list end
        /// \return an iterator to the QuestRelation list's end
        std::list<QuestRelation*>::iterator QuestsEnd()
        {
            return m_quests->end();
        };

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Initializes the QuestRelation list with another
        /// \param std::list< QuestRelation* >* qst_lst  -  pointer to the other list
        void SetQuestList(std::list<QuestRelation*>* qst_lst)
        {
            m_quests = qst_lst;
        };

    private:

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Loads the QuestRelations from QuestMgr for this GO
        void LoadQuests() { sQuestMgr.LoadGOQuests(this); }

        std::list<QuestRelation*>* m_quests;

};

//////////////////////////////////////////////////////////////////////////////////////////
/// implementing type 3 (CHEST) GameObjects
//////////////////////////////////////////////////////////////////////////////////////////
class GameObject_Chest : public GameObject_Lootable
{
    public:

        GameObject_Chest(uint64 GUID);
        ~GameObject_Chest();

        void InitAI();

        bool IsLootable() { return true; }
        bool HasLoot();

        void Use(uint64 GUID);

        void Open();
        void Close();

    private:

        SpellInfo* spell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// implementing Type 6 (TRAP) GameObjects
//////////////////////////////////////////////////////////////////////////////////////////
class GameObject_Trap : public GameObject
{
    public:

        GameObject_Trap(uint64 GUID);
        ~GameObject_Trap();

        void InitAI();
        void Update(unsigned long time_passed);

    private:

        SpellInfo* spell;
        uint32 targetupdatetimer;
        float maxdistance;
        uint32 cooldown;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// implementing Type 8 (SPELL_FOCUS) GameObjects
//////////////////////////////////////////////////////////////////////////////////////////
class GameObject_SpellFocus : public GameObject
{
    public:

        GameObject_SpellFocus(uint64 GUID);
        ~GameObject_SpellFocus();

        void OnPushToWorld();

    private:

        void SpawnLinkedTrap();
};

//////////////////////////////////////////////////////////////////////////////////////////
/// implementing Type 10 (GOOBER) GameObjects
//////////////////////////////////////////////////////////////////////////////////////////
class GameObject_Goober : public GameObject
{
    public:

        GameObject_Goober(uint64 GUID);
        ~GameObject_Goober();

        void InitAI();

        void Use(uint64 GUID);

        void Open();
        void Close();

    private:
        SpellInfo* spell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// implements Type 17 (FISHINGNODE) GameObjects
//////////////////////////////////////////////////////////////////////////////////////////
class GameObject_FishingNode : public GameObject_Lootable
{
    public:

        GameObject_FishingNode(uint64 GUID);
        ~GameObject_FishingNode();

        void OnPushToWorld();

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Handles the click on the bobber, if there is a fish hooked, otherwise end session
        /// \return true on success, false otherwise.
        bool UseNode();

        void EndFishing(bool abort);

        void EventFishHooked();

        bool IsLootable() { return true; }

        bool HasLoot();

    private:

        bool FishHooked;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Class implementing Type 18 (SUMMONING_RITUAL) GameObjects
//////////////////////////////////////////////////////////////////////////////////////////
class GameObject_Ritual : public GameObject
{
    public:

        GameObject_Ritual(uint64 GUID);
        ~GameObject_Ritual();

        void InitAI();

        CRitual* GetRitual() const
        {
            return Ritual;
        }

    private:
        CRitual* Ritual;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Implements Type 22 (SPELLCASTER) GameObjects
//////////////////////////////////////////////////////////////////////////////////////////
class GameObject_SpellCaster : public GameObject
{
    public:

        GameObject_SpellCaster(uint64 GUID);
        ~GameObject_SpellCaster();

        void InitAI();

        void Use(uint64 GUID);

    private:

        SpellInfo* spell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// implementing Type 26 (FISHINGHOLE) GameObjects
//////////////////////////////////////////////////////////////////////////////////////////
class GameObject_FishingHole : public GameObject_Lootable
{
    public:

        GameObject_FishingHole(uint64 GUID);
        ~GameObject_FishingHole();

        void InitAI();

        bool IsLootable() { return true; }
        bool HasLoot();

        bool CanFish();

        void CatchFish();

        void CalcFishRemaining(bool force);

    private:

        uint32 usage_remaining;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Implements Type 33 (DESTRUCTIBLE) GameObjects
//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL GameObject_Destructible : public GameObject
{
    public:

        GameObject_Destructible(uint64 GUID);
        ~GameObject_Destructible();

        void InitAI();

        void Damage(uint32 damage, uint64 AttackerGUID, uint64 ControllerGUID, uint32 SpellID);

        void Rebuild();

        uint32 GetHP() { return hitpoints; }

        uint32 GetMaxHP() { return maxhitpoints; }

    private:

        void SendDamagePacket(uint32 damage, uint64 AttackerGUID, uint64 ControllerGUID, uint32 SpellID);

        uint32 hitpoints;
        uint32 maxhitpoints;
};

#endif // GAMEOBJECT_H
