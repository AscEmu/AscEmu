/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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

#ifndef OBJECT_H
#define OBJECT_H

#include "ObjectDefines.h"

#include "Server/UpdateFieldInclude.h"
#include "Server/UpdateMask.h"
#include "CommonTypes.hpp"
#include "Server/EventableObject.h"
#include "Server/IUpdatable.h"

#include <set>
#include <map>

#include "WoWGuid.h"
#include "../shared/LocationVector.h"
#include "Storage/MySQLStructures.h"
#include "Storage/DBC/DBCStructures.hpp"
#if VERSION_STRING >= Cata
    #include "Storage/DB2/DB2Structures.h"
#endif
#include "../shared/CommonDefines.hpp"
#include "WorldPacket.h"
#include "Units/Creatures/CreatureDefines.hpp"
#include "Data/WoWObject.hpp"
#include "MovementInfo.h"
#include "Spell/Definitions/School.h"

struct WoWObject;

class SpellInfo;

struct FactionDBC;
struct AuraEffectModifier;

class Unit;
class Group;
class Transporter;
class WorldPacket;
class ByteBuffer;
class WorldSession;
class Player;
class MapCell;
class MapMgr;
class ObjectContainer;
class DynamicObject;
class Creature;
class GameObject;
class Pet;
class Spell;
class Aura;
class UpdateMask;
class EventableObject;

#define MAX_INTERACTION_RANGE 5.0f

// MIT Start
enum CurrentSpellType : uint8_t
{
    CURRENT_MELEE_SPELL         = 0,
    CURRENT_GENERIC_SPELL       = 1,
    CURRENT_CHANNELED_SPELL     = 2,
    CURRENT_AUTOREPEAT_SPELL    = 3,
    CURRENT_SPELL_MAX
};

struct DamageInfo
{
    SchoolMask schoolMask = SCHOOL_MASK_NORMAL;

    uint32_t realDamage = 0; // the damage after resist, absorb etc
    int32_t fullDamage = 0; // the damage before resist, absorb etc
    uint32_t absorbedDamage = 0;
    uint32_t resistedDamage = 0;
    uint32_t blockedDamage = 0;

    WeaponDamageType weaponType = MELEE;
    bool isHeal = false;
    bool isCritical = false;
    bool isPeriodic = false;

    uint8_t getSchoolTypeFromMask() const
    {
        for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; ++i)
        {
            if (schoolMask & (1 << i))
                return i;
        }

        // shouldn't happen
        return SCHOOL_NORMAL;
    }
};

class SERVER_DECL Object : public EventableObject, public IUpdatable
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Object values

protected:
    union
    {
        uint8_t* wow_data_ptr;
        WoWObject* wow_data;
        uint32_t* m_uint32Values;
    };

    bool skipping_updates = false;

    const WoWObject* objectData() const { return wow_data; }

public:
    bool write(const uint8_t& member, uint8_t val);
    bool write(const uint16_t& member, uint16_t val);
    bool write(const float& member, float val);
    bool write(const int32_t& member, int32_t val);
    bool write(const uint32_t& member, uint32_t val);
    bool write(const uint64_t& member, uint64_t val);
    bool write(const uint64_t& member, uint32_t low, uint32_t high);
    bool writeLow(const uint64_t& member, uint32_t val);
    bool writeHigh(const uint64_t& member, uint32_t val);

    //////////////////////////////////////////////////////////////////////////////////////////
    // WoWData
    uint64_t getGuid() const;
    void setGuid(uint64_t guid);
    void setGuid(uint32_t low, uint32_t high);

    uint32_t getGuidLow() const;
    void setGuidLow(uint32_t low);

    uint32_t getGuidHigh() const;
    void setGuidHigh(uint32_t high);

    //\todo choose one function!
#if VERSION_STRING < Cata
    uint32_t getOType() const;
    void setOType(uint32_t type);
    void setObjectType(uint8_t objectTypeId);
#else
    uint16_t getOType() const;
    void setOType(uint16_t type);
    void setObjectType(uint8_t objectTypeId);
#endif

    void setEntry(uint32_t entry);
    uint32_t getEntry() const;

    float getScale() const;
    void setScale(float scaleX);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Object update
    void updateObject();

    //! This includes any nested objects we have, inventory for example.
    virtual uint32_t buildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Object Type Id
protected:
    uint8_t m_objectTypeId;

public:
    uint8_t getObjectTypeId() const;

    bool isCreatureOrPlayer() const;
    bool isPlayer() const;
    bool isCreature() const;
    bool isItem() const;
    bool isGameObject() const;
    bool isCorpse() const;
    bool isContainer() const;

    virtual bool isPet() const { return false; }
    virtual bool isTotem() const { return false; }
    virtual bool isSummon() const { return false; }
    virtual bool isVehicle() const { return false; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Position functions
    bool isInRange(LocationVector location, float square_r) const;
    bool isInRange(float x, float y, float z, float square_r) const;

    float getDistanceSq(LocationVector target) const;
    float getDistanceSq(float x, float y, float z) const;
    Player* asPlayer();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spell functions
private:
    Spell* m_currentSpell[CURRENT_SPELL_MAX];

    std::map<Spell*, bool> m_travelingSpells;
    std::list<Spell*> m_garbageSpells;

public:
    Spell* getCurrentSpell(CurrentSpellType spellType) const;
    Spell* getCurrentSpellById(uint32_t spellId) const;
    void setCurrentSpell(Spell* curSpell);

    // If spellid is set to 0, function will interrupt any current spell
    void interruptSpell(uint32_t spellId = 0, bool checkMeleeSpell = true);
    void interruptSpellWithSpellType(CurrentSpellType spellType);

    // Searches for current casted spell, but skips 'on next melee' spells
    bool isCastingSpell(bool skipChanneled = false, bool skipAutorepeat = false, bool isAutoshoot = false) const;
    Spell* findCurrentCastedSpellBySpellId(uint32_t spellId);

    // Deals magic damage to target with proper logging, used with periodic damage effects and direct damage effects
    // Returns damage info structure
    DamageInfo doSpellDamage(Unit* victim, uint32_t spellId, float_t damage, uint8_t effIndex, bool isTriggered = false, bool isPeriodic = false, bool isLeech = false, bool forceCrit = false, Spell* spell = nullptr, Aura* aur = nullptr, AuraEffectModifier* aurEff = nullptr);
    // Heals target with proper logging, used with periodic heal effects and direct healing
    DamageInfo doSpellHealing(Unit* victim, uint32_t spellId, float_t heal, bool isTriggered = false, bool isPeriodic = false, bool isLeech = false, bool forceCrit = false, Spell* spell = nullptr, Aura* aur = nullptr, AuraEffectModifier* aurEff = nullptr);

    void _UpdateSpells(uint32_t time);

    void addTravelingSpell(Spell* spell);
    void removeTravelingSpell(Spell* spell);
    void addGarbageSpell(Spell* spell);
    void removeGarbageSpells();

    void removeSpellModifierFromCurrentSpells(AuraEffectModifier const* aur);

    //////////////////////////////////////////////////////////////////////////////////////////
    // InRange sets
private:
    std::vector<Object*> mInRangeObjectsSet;
    std::vector<Object*> mInRangePlayersSet;
    std::vector<Object*> mInRangeOppositeFactionSet;
    std::vector<Object*> mInRangeSameFactionSet;

public:
    // general
    virtual void clearInRangeSets();
    virtual void addToInRangeObjects(Object* pObj);
    virtual void onRemoveInRangeObject(Object* /*pObj*/) {}

    void removeSelfFromInrangeSets();

    // Objects
    std::vector<Object*> getInRangeObjectsSet();

    bool hasInRangeObjects();
    size_t getInRangeObjectsCount();

    bool isObjectInInRangeObjectsSet(Object* pObj);
    void removeObjectFromInRangeObjectsSet(Object* pObj);

    // Players
    std::vector<Object*> getInRangePlayersSet();

    size_t getInRangePlayersCount();


    // Opposite Faction
    std::vector<Object*> getInRangeOppositeFactionSet();

    bool isObjectInInRangeOppositeFactionSet(Object* pObj);
    void updateInRangeOppositeFactionSet();

    void addInRangeOppositeFaction(Object* obj);
    void removeObjectFromInRangeOppositeFactionSet(Object* obj);

    // same faction
    std::vector<Object*> getInRangeSameFactionSet();

    bool isObjectInInRangeSameFactionSet(Object* pObj);
    void updateInRangeSameFactionSet();

    void addInRangeSameFaction(Object* obj);
    void removeObjectFromInRangeSameFactionSet(Object* obj);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Owner

    // Returns player charmer, player owner or self
    virtual Player* getPlayerOwner();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc

    void sendGameobjectDespawnAnim();

    // MIT End

        Object();
        virtual ~Object();

        void Update(unsigned long /*time_passed*/) {}

        // True if object exists in world, else false
        bool IsInWorld() { return m_mapMgr != NULL; }
        virtual void AddToWorld();
        virtual void AddToWorld(MapMgr* pMapMgr);
        void PushToWorld(MapMgr*);
        virtual void RemoveFromWorld(bool free_guid);

        // Virtual method that is called, BEFORE pushing the Object in the game world
        virtual void OnPrePushToWorld() {}

        // Virtual method that is called, AFTER pushing the Object in the game world
        virtual void OnPushToWorld() {}

        // Virtual method that is called, BEFORE removing the Object from the game world
        virtual void OnPreRemoveFromWorld() {}

        // Virtual method that is called, AFTER removing the Object from the game world
        virtual void OnRemoveFromWorld() {}

        // Guid always comes first

        const WoWGuid & GetNewGUID() const { return m_wowGuid; }

    uint32 GetTypeFromGUID() const { return (getGuidHigh() & HIGHGUID_TYPE_MASK); }
    uint32 GetUIdFromGUID() const { return (getGuidLow() & LOWGUID_ENTRY_MASK); }

        // typeFlags
        bool IsType(TYPE type_mask) const { return (type_mask & m_objectType) != 0; }

        uint32 BuildValuesUpdateBlockForPlayer(ByteBuffer* buf, Player* target);
        uint32 BuildValuesUpdateBlockForPlayer(ByteBuffer* buf, UpdateMask* mask);

        void BuildFieldUpdatePacket(Player* Target, uint32 Index, uint32 Value);
        void BuildFieldUpdatePacket(ByteBuffer* buf, uint32 Index, uint32 Value);

        bool SetPosition(float newX, float newY, float newZ, float newOrientation, bool allowPorting = false);
        bool SetPosition(const LocationVector & v, bool allowPorting = false);

        const float & GetPositionX() const { return m_position.x; }
        const float & GetPositionY() const { return m_position.y; }
        const float & GetPositionZ() const { return m_position.z; }
        const float & GetOrientation() const { return m_position.o; }
        void SetOrientation(float o) { m_position.o = o; }

        void SetSpawnLocation(float newX, float newY, float newZ, float newOrientation) { m_spawnLocation.changeCoords(newX, newY, newZ, newOrientation); }
        void SetSpawnLocation(LocationVector loc) { m_spawnLocation.ChangeCoords(loc); }
        const float & GetSpawnX() const { return m_spawnLocation.x; }
        const float & GetSpawnY() const { return m_spawnLocation.y; }
        const float & GetSpawnZ() const { return m_spawnLocation.z; }
        const float & GetSpawnO() const { return m_spawnLocation.o; }
        LocationVector GetSpawnPosition() const { return m_spawnLocation; }

        ::DBC::Structures::AreaTableEntry const* GetArea();

        LocationVector GetPosition() const { return LocationVector(m_position); }
        LocationVector & GetPositionNC() { return m_position; }
        LocationVector* GetPositionV() { return &m_position; }

        // TransporterInfo
        void SetTransport(Transporter* t) { m_transport = t; }
        Transporter* GetTransport() const { return m_transport; }

        float GetTransOffsetX() const { return obj_movement_info.transport_position.x; }
        float GetTransOffsetY() const { return obj_movement_info.transport_position.y; }
        float GetTransOffsetZ() const { return obj_movement_info.transport_position.z; }
        float GetTransOffsetO() const { return obj_movement_info.transport_position.o; }
        uint32 GetTransTime() const { return obj_movement_info.transport_time; }
#ifdef FT_VEHICLES
        // TODO check if this is in BC
        uint8 GetTransSeat() const { return obj_movement_info.transport_seat; }
#endif

        // Distance Calculation
        float CalcDistance(Object* Ob);
        float CalcDistance(float ObX, float ObY, float ObZ);
        float CalcDistance(Object* Oa, Object* Ob);
        float CalcDistance(Object* Oa, float ObX, float ObY, float ObZ);
        float CalcDistance(float OaX, float OaY, float OaZ, float ObX, float ObY, float ObZ);
        // NYS: scriptdev2
        bool IsInMap(Object* obj) { return GetMapId() == obj->GetMapId() && GetInstanceID() == obj->GetInstanceID(); }
        bool IsWithinDistInMap(Object* obj, const float dist2compare) const;
        bool IsWithinLOSInMap(Object* obj);
        bool IsWithinLOS(LocationVector location);

        // Only for MapMgr use
        MapCell* GetMapCell() const;
    uint32 GetMapCellX() { return m_mapCell_x; }
    uint32 GetMapCellY() { return m_mapCell_y; }
        // Only for MapMgr use
        void SetMapCell(MapCell* cell);
        // Only for MapMgr use
        MapMgr* GetMapMgr() const { return m_mapMgr; }

        Object* GetMapMgrObject(const uint64 & guid);
        Pet* GetMapMgrPet(const uint64 & guid);
        Unit* GetMapMgrUnit(const uint64 & guid);
        Player* GetMapMgrPlayer(const uint64 & guid);
        Creature* GetMapMgrCreature(const uint64 & guid);
        GameObject* GetMapMgrGameObject(const uint64 & guid);
        DynamicObject* GetMapMgrDynamicObject(const uint64 & guid);

        void SetMapId(uint32 newMap) { m_mapId = newMap; }
        void SetZoneId(uint32 newZone);

    uint32 GetMapId() const { return m_mapId; }
        const uint32 & GetZoneId() const { return m_zoneId; }

        void SetNewGuid(uint32 Guid)
        {
            setGuidLow(Guid);
            m_wowGuid.Init(getGuid());
        }

        ////////////////////////////////////////
        void ClearUpdateMask()
        {
            m_updateMask.Clear();
            m_objectUpdated = false;
        }

        bool HasUpdateField(uint32 index)
        {
            ARCEMU_ASSERT(index < m_valuesCount)
            return m_updateMask.GetBit(index);
        }

        // Use this to check if a object is in range of another
        bool isInRange(Object* target, float range);

        // Use this to Check if a object is in front of another object.
        bool isInFront(Object* target);
        // Use this to Check if a object is in back of another object.
        bool isInBack(Object* target);
        // Check to see if an object is in front of a target in a specified arc (in degrees)
        bool isInArc(Object* target, float degrees);
        // NYS: Scriptdev2
        bool HasInArc(float degrees, Object* target);
        // Calculates the angle between two positions
        float calcAngle(float Position1X, float Position1Y, float Position2X, float Position2Y);
        float calcRadAngle(float Position1X, float Position1Y, float Position2X, float Position2Y);

        // Converts to 360 > x > 0
        float getEasyAngle(float angle);

        float getAbsoluteAngle(float x, float y) const
        {
            float dx = x - GetPositionX();
            float dy = y - GetPositionY();
            return normalizeOrientation(std::atan2(dy, dx));
            
        }
        float getAbsoluteAngle(LocationVector const& pos) { return getAbsoluteAngle(pos.x, pos.y); }
        float getAbsoluteAngle(Object const* obj) { return getAbsoluteAngle(obj->GetPosition()); }

    float getDistanceSq(Object* obj)
        {
            if (obj->GetMapId() != m_mapId)
                return 40000.0f;                        // enough for out of range
            return m_position.distanceSquare(obj->GetPosition());
        }

        float CalcDistance(LocationVector & comp)
        {
            return comp.Distance(m_position);
        }

    float GetDistance2dSq(Object* obj)
        {
            if (obj->GetMapId() != m_mapId)
                return 40000.0f;                        // enough for out of range
            return m_position.Distance2DSq(obj->m_position);
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        // void OutPacket(uint16 opcode, uint16 len, const void *data)
        // Sends a packet to the Player
        //
        // \param uint16 opcode      -   opcode of the packet
        // \param uint16 len         -   length/size of the packet
        // \param const void *data   -   the data that needs to be sent
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        virtual void OutPacket(uint16_t /*opcode*/, uint16_t /*len*/, const void* /*data*/) {};

        //////////////////////////////////////////////////////////////////////////////////////////
        // void SendPacket(WorldPacket *packet)
        //  Sends a packet to the Player
        //
        // \param WorldPAcket *packet      -     the packet that needs to be sent
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        virtual void SendPacket(WorldPacket* /*packet*/) {};

        void SendCreatureChatMessageInRange(Creature* creature, uint32_t textId);
        void SendMonsterSayMessageInRange(Creature* creature, MySQLStructure::NpcMonsterSay* npcMonsterSay, int randChoice, uint32_t event);

        virtual void SendMessageToSet(WorldPacket* data, bool self, bool myteam_only = false);
        //void SendMessageToSet(StackBufferBase* data, bool self) { OutPacketToSet(data->GetOpcode(), static_cast<uint16>(data->GetSize()), data->GetBufferPointer(), self); }
        virtual void OutPacketToSet(uint16 Opcode, uint16 Len, const void* Data, bool self);

        //////////////////////////////////////////////////////////////////////////////////////////
        // void SendAIReaction(uint32 reaction = 2)
        // Notifies the player's clients about the AI reaction of this object
        // (NPC growl for example "aggro sound")
        //
        // \param uint32 reaction  -  Reaction type
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void SendAIReaction(uint32 reaction = 2);

        //////////////////////////////////////////////////////////////////////////////////////////
        //void sendDestroyObjectPacket()
        // Destroys this Object for the players' clients that are nearby
        // (removes object from the scene)
        //
        // \param none
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void SendDestroyObject();

        uint16 GetValuesCount() const { return m_valuesCount; }

        MovementInfo obj_movement_info;

        uint32 m_phase;         // This stores the phase, if two objects have the same bit set, then they can see each other. The default phase is 0x1.

    uint32 GetPhase() { return m_phase; }
        virtual void Phase(uint8 command = PHASE_SET, uint32 newphase = 1);

        // SpellLog packets just to keep the code cleaner and better to read
        void SendSpellLog(Object* Caster, Object* Target, uint32 Ability, uint8 SpellLogType);

        // object faction
        void setServersideFaction();
        uint32 getServersideFaction();

        DBC::Structures::FactionTemplateEntry const* m_factionTemplate;
        DBC::Structures::FactionEntry const* m_factionEntry;

        void SetInstanceID(int32 instance) { m_instanceId = instance; }
        int32 GetInstanceID() { return m_instanceId; }

        int32 event_GetInstanceID();

        // Object activation
    private:

        bool Active;
    public:

        bool IsActive() { return Active; }
        virtual bool CanActivate();
        virtual void Activate(MapMgr* mgr);
        virtual void Deactivate(MapMgr* mgr);
        // Player is in pvp queue.
        bool m_inQueue;
        void SetMapMgr(MapMgr* mgr) { m_mapMgr = mgr; }

        void Delete()
        {
            if (IsInWorld())
                RemoveFromWorld(true);
            delete this;
        }
        // Play's a sound to players in range.
        void PlaySoundToSet(uint32 sound_entry);
        // Is the player in a battleground?
        bool IsInBg();
        // What's their faction? Horde/Ally.
        uint32 GetTeam();
        // Objects directly cannot be in a group.
        //virtual Group* getGroup() { return NULL; }

    protected:

        //void _Create (uint32 guidlow, uint32 guidhigh);
        void _Create(uint32 mapid, float x, float y, float z, float ang);

        // Mark values that need updating for specified player.
        virtual void _SetUpdateBits(UpdateMask* updateMask, Player* target) const;
        // Mark values that player should get when he/she/it sees object for first time.
        virtual void _SetCreateBits(UpdateMask* updateMask, Player* target) const;

        // Create updates that player will see
#if VERSION_STRING < WotLK
        void buildMovementUpdate(ByteBuffer* data, uint8_t updateFlags, Player* target);
#else
        void buildMovementUpdate(ByteBuffer* data, uint16_t updateFlags, Player* target);
#endif

        void buildValuesUpdate(ByteBuffer* data, UpdateMask* updateMask, Player* target);

        // WoWGuid class
        WoWGuid m_wowGuid;

        // Type mask
        uint16 m_objectType;

        //update flag
        uint16 m_updateFlag;

        // Zone id.
        uint32 m_zoneId;
        // Continent/map id.
        uint32 m_mapId;
        // Map manager
        MapMgr* m_mapMgr;
        // Current map cell row and column
        uint32 m_mapCell_x, m_mapCell_y;

        // Main Function called by isInFront();
        bool inArc(float Position1X, float Position1Y, float FOV, float Orientation, float Position2X, float Position2Y);

        LocationVector m_position;
        LocationVector m_lastMapUpdatePosition;
        LocationVector m_spawnLocation;

        // Number of properties
        uint16 m_valuesCount;

        // List of object properties that need updating.
        UpdateMask m_updateMask;

        // True if object was updated
        bool m_objectUpdated;

        int32 m_instanceId;

        // Transporters
        Transporter* m_transport;

    public:

        bool m_loadedFromDB;

        // Andy's crap
        std::set<Spell*> m_pendingSpells;

        bool GetPoint(float angle, float rad, float & outx, float & outy, float & outz, bool sloppypath = false);
        bool GetRandomPoint(float rad, float & outx, float & outy, float & outz) { return GetPoint(Util::getRandomFloat(float(M_PI * 2)), rad, outx, outy, outz); }
        bool GetRandomPoint(float rad, LocationVector & out) { return GetRandomPoint(rad, out.x, out.y, out.z); }
};

#endif // OBJECT_H
