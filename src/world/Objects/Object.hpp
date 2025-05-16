/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ObjectDefines.hpp"
#include "Server/UpdateMask.h"
#include "CommonTypes.hpp"
#include "Server/EventableObject.h"
#include "CommonDefines.hpp"
#include "Units/Creatures/CreatureDefines.hpp"
#include "MovementInfo.hpp"
#include "Macros/MapsMacros.hpp"
#include "Spell/Definitions/ProcFlags.hpp"
#include "Units/UnitDefines.hpp"

#include <set>
#include <map>
#include <mutex>
#include <shared_mutex>

#include "DamageInfo.hpp"

namespace WDB::Structures
{
    struct AreaTableEntry;
    struct FactionEntry;
    struct FactionTemplateEntry;
}

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
class WorldMap;
class InstanceMap;
class BattlegroundMap;
class ObjectContainer;
class DynamicObject;
class Creature;
class GameObject;
class Summon;
class Pet;
class Spell;
class Aura;
class UpdateMask;
class EventableObject;
enum ZLiquidStatus : uint32_t;
enum Standing : uint8_t;

#define MAX_INTERACTION_RANGE 5.0f
float const DEFAULT_COLLISION_HEIGHT = 2.03128f; // Most common value in dbc

enum CurrentSpellType : uint8_t
{
    CURRENT_MELEE_SPELL         = 0,
    CURRENT_GENERIC_SPELL       = 1,
    CURRENT_CHANNELED_SPELL     = 2,
    CURRENT_AUTOREPEAT_SPELL    = 3,
    CURRENT_SPELL_MAX
};

class SERVER_DECL Object : public EventableObject
{
public:
    Object();
    virtual ~Object();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Essential functions (mostly inherited by other classes)

    // updated by EventableObject
    void Update(unsigned long /*time_passed*/) {}

    // adds/queues object to world and links WorldMap to it if possible
    virtual void AddToWorld();

    // adds/queues objext to world and links WorldMap to it
    virtual void AddToWorld(WorldMap* pMapMgr);

    // Unlike addtoworld it pushes it directly ignoring add pool this can only be called from the thread of WorldMap!
    void PushToWorld(WorldMap*);

    // removes object from world and queue
    virtual void RemoveFromWorld(bool free_guid);

    // True if object exists in world, else false
    bool IsInWorld() const { return m_WorldMap != NULL; }

    // is called BEFORE pushing the Object in the game world
    virtual void OnPrePushToWorld() {}

    // is called AFTER pushing the Object in the game world
    virtual void OnPushToWorld() {}

    // is called BEFORE removing the Object from the game world
    virtual void OnPreRemoveFromWorld() {}

    // is called AFTER removing the Object from the game world
    virtual void OnRemoveFromWorld() {}

    //////////////////////////////////////////////////////////////////////////////////////////
    // Object values

protected:
    union
    {
        uint8_t* wow_data_ptr;
        WoWObject* wow_data;
        uint32_t* m_uint32Values = nullptr;
    };

    const WoWObject* objectData() const { return wow_data; }

public:
    bool write(const uint8_t& member, uint8_t val, bool skipObjectUpdate = false);
    bool write(const uint16_t& member, uint16_t val, bool skipObjectUpdate = false);
    bool write(const float& member, float val, bool skipObjectUpdate = false);
    bool write(const int32_t& member, int32_t val, bool skipObjectUpdate = false);
    bool write(const uint32_t& member, uint32_t val, bool skipObjectUpdate = false);
    bool write(const uint64_t& member, uint64_t val, bool skipObjectUpdate = false);
    bool write(const uint64_t& member, uint32_t low, uint32_t high, bool skipObjectUpdate = false);
    bool writeLow(const uint64_t& member, uint32_t val, bool skipObjectUpdate = false);
    bool writeHigh(const uint64_t& member, uint32_t val, bool skipObjectUpdate = false);

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

#if VERSION_STRING >= Mop
    uint16_t getDynamicFlags() const;
    int16_t getDynamicPathProgress() const;
    void setDynamicFlags(uint16_t dynamicFlags);
    void addDynamicFlags(uint16_t dynamicFlags);
    void removeDynamicFlags(uint16_t dynamicFlags);
    bool hasDynamicFlags(uint16_t dynamicFlags) const;
    void setDynamicPathProgress(int16_t pathProgress);
#endif

    float getScale() const;
    void setScale(float scaleX);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Object update
    void updateObject();

    //! This includes any nested objects we have, inventory for example.
    virtual uint32_t buildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target);

    // Forces update for WoWData field
    void forceBuildUpdateValueForField(uint32_t field, Player* target);
    // Forces update for multiple WoWData fields
    void forceBuildUpdateValueForFields(uint32_t const* fields, Player* target);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Object Type Id
protected:
    uint8_t m_objectTypeId = TYPEID_UNIT;

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
    virtual bool isTransporter() const { return false; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Position functions
    bool isInRange(LocationVector location, float square_r) const;
    bool isInRange(float x, float y, float z, float square_r) const;

    float getDistanceSq(LocationVector target) const;
    float getDistanceSq(float x, float y, float z) const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spell functions
private:
    Spell* m_currentSpell[CURRENT_SPELL_MAX] = {nullptr};

    std::map<Spell*, bool> m_travelingSpells;
    std::list<Spell*> m_garbageSpells;
    std::mutex m_garbageMutex;

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

    mutable std::mutex m_inRangeSetMutex;
    mutable std::mutex m_inRangeFactionSetMutex;
    mutable std::shared_mutex m_inRangePlayerSetMutex;

public:
    // general
    virtual void clearInRangeSets();
    virtual void addToInRangeObjects(Object* pObj);
    virtual void onRemoveInRangeObject(Object* /*pObj*/) {}

    void removeSelfFromInrangeSets();

    // Objects
    std::vector<Object*> getInRangeObjectsSet() const;

    bool hasInRangeObjects() const;
    size_t getInRangeObjectsCount() const;

    bool isObjectInInRangeObjectsSet(Object* pObj) const;
    void removeObjectFromInRangeObjectsSet(Object* pObj);

    // Players
    std::vector<Object*> getInRangePlayersSet() const;
    size_t getInRangePlayersCount() const;

    // Opposite Faction
    std::vector<Object*> getInRangeOppositeFactionSet() const;

    bool isObjectInInRangeOppositeFactionSet(Object* pObj) const;
    void updateInRangeOppositeFactionSet();

    void addInRangeOppositeFaction(Object* obj);
    void removeObjectFromInRangeOppositeFactionSet(Object* obj);

    // same faction
    std::vector<Object*> getInRangeSameFactionSet() const;

    bool isObjectInInRangeSameFactionSet(Object* pObj) const;
    void updateInRangeSameFactionSet();

    void addInRangeSameFaction(Object* obj);
    void removeObjectFromInRangeSameFactionSet(Object* obj);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Owner

    // Returns unit charmer or unit owner
    virtual Unit* getUnitOwner();
    // Returns unit charmer or unit owner
    virtual Unit const* getUnitOwner() const;
    // Returns unit charmer, unit owner or self
    virtual Unit* getUnitOwnerOrSelf();
    // Returns unit charmer, unit owner or self
    virtual Unit const* getUnitOwnerOrSelf() const;
    // Returns player charmer or player owner
    virtual Player* getPlayerOwner();
    // Returns player charmer or player owner
    virtual Player const* getPlayerOwner() const;
    // Returns player charmer, player owner or self
    virtual Player* getPlayerOwnerOrSelf();
    // Returns player charmer, player owner or self
    virtual Player const* getPlayerOwnerOrSelf() const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc

    void sendGameobjectDespawnAnim();

    //////////////////////////////////////////////////////////////////////////////////////////
    // AGPL Starts

        // Guid always comes first

        const WoWGuid & GetNewGUID() const { return m_wowGuid; }

    uint32_t GetTypeFromGUID() const { return (getGuidHigh() & HIGHGUID_TYPE_MASK); }
    uint32_t GetUIdFromGUID() const { return (getGuidLow() & LOWGUID_ENTRY_MASK); }

        // typeFlags
        bool IsType(TYPE type_mask) const { return (type_mask & m_objectType) != 0; }

        uint32_t BuildValuesUpdateBlockForPlayer(ByteBuffer* buf, Player* target);
        uint32_t BuildValuesUpdateBlockForPlayer(ByteBuffer* buf, UpdateMask* mask);

        void BuildFieldUpdatePacket(Player* Target, uint32_t Index, uint32_t Value);
        void BuildFieldUpdatePacket(ByteBuffer* buf, uint32_t Index, uint32_t Value);

        void updatePositionData();

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

        ::WDB::Structures::AreaTableEntry const* GetArea() const;

        void getPosition(float &x, float &y) const { x = GetPositionX(); y = GetPositionY(); }
        void getPosition(float &x, float &y, float &z) const { getPosition(x, y); z = GetPositionZ(); }
        void getPosition(float &x, float &y, float &z, float &o) const { getPosition(x, y, z); o = GetOrientation(); }
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
        uint32_t GetTransTime() const { return obj_movement_info.transport_time; }
#ifdef FT_VEHICLES
        // TODO check if this is in BC
        uint8_t GetTransSeat() const { return obj_movement_info.transport_seat; }
#endif

        Player* ToPlayer() { if (isPlayer()) return reinterpret_cast<Player*>(this); else return nullptr; }
        Player const* ToPlayer() const { if (isPlayer()) return (Player*)this; else return nullptr; }
        Creature* ToCreature() { if (isCreature()) return reinterpret_cast<Creature*>(this); else return nullptr; }
        Creature const* ToCreature() const { if (isCreature()) return (Creature*)this; else return nullptr; }
        Summon* ToSummon() { if (isSummon()) return reinterpret_cast<Summon*>(this); else return nullptr; }
        Summon const* ToSummon() const { if (isSummon()) return (Summon*)this; else return nullptr; }
        Unit* ToUnit() { if (isCreatureOrPlayer()) return reinterpret_cast<Unit*>(this); else return nullptr; }
        Unit const* ToUnit() const { if (isCreatureOrPlayer()) return (Unit*)this; else return nullptr; }
        GameObject* ToGameObject() { if (isGameObject()) return reinterpret_cast<GameObject*>(this); else return nullptr; }
        GameObject const* ToGameObject() const { if (isGameObject()) return (GameObject*)this; else return nullptr; }

        float getExactDist2dSq(const float x, const float y) const
        {
            float dx = x - GetPositionX();
            float dy = y - GetPositionY();
            return dx * dx + dy * dy;
        }
        float getExactDist2dSq(LocationVector const& pos) const { return getExactDist2dSq(pos.x, pos.y); }
        float getExactDist2dSq(LocationVector const* pos) const { return getExactDist2dSq(*pos); }

        float getExactDist2d(const float x, const float y) const { return std::sqrt(getExactDist2dSq(x, y)); }
        float getExactDist2d(LocationVector const& pos) const { return getExactDist2d(pos.x, pos.y); }
        float getExactDist2d(LocationVector const* pos) const { return getExactDist2d(*pos); }

        float getExactDistSq(float x, float y, float z) const
        {
            float dz = z - GetPositionZ();
            return getExactDist2dSq(x, y) + dz * dz;
        }
        float getExactDistSq(LocationVector const& pos) const { return getExactDistSq(pos.x, pos.y, pos.z); }
        float getExactDistSq(LocationVector const* pos) const { return getExactDistSq(*pos); }

        float getExactDist(float x, float y, float z) const { return std::sqrt(getExactDistSq(x, y, z)); }
        float getExactDist(LocationVector const& pos) const { return getExactDist(pos.x, pos.y, pos.z); }
        float getExactDist(LocationVector const* pos) const { return getExactDist(*pos); }

        float getDistance(Object const* obj) const;
        float getDistance(LocationVector const& pos) const;
        float getDistance(float x, float y, float z) const;
        float getDistance2d(Object const* obj) const;
        float getDistance2d(float x, float y) const;

        float getDistanceZ(Object const* obj) const;

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

        // Only for WorldMap use
        MapCell* GetMapCell() const;
        uint32_t GetMapCellX() { return m_mapCell_x; }
        uint32_t GetMapCellY() { return m_mapCell_y; }
        // Only for WorldMap use
        void SetMapCell(MapCell* cell);
        // Only for WorldMap use
        WorldMap* getWorldMap() const { return m_WorldMap; }

        Object* getWorldMapObject(const uint64_t & guid) const;
        Pet* getWorldMapPet(const uint64_t & guid) const;
        Unit* getWorldMapUnit(const uint64_t & guid) const;
        Player* getWorldMapPlayer(const uint64_t & guid) const;
        Creature* getWorldMapCreature(const uint64_t & guid) const;
        GameObject* getWorldMapGameObject(const uint64_t & guid) const;
        DynamicObject* getWorldMapDynamicObject(const uint64_t & guid) const;

        void SetMapId(uint32_t newMap) { m_mapId = newMap; }
        void setZoneId(uint32_t newZone);
        void setAreaId(uint32_t area) { m_areaId = area; }

        uint32_t GetMapId() const { return m_mapId; }
        const uint32_t & getZoneId() const { return m_zoneId; }
        const uint32_t & getAreaId() const { return m_areaId; }

        bool isOutdoors() const { return m_outdoors; }
        ZLiquidStatus getLiquidStatus() const { return m_liquidStatus; }

        void SetNewGuid(uint32_t Guid)
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

        bool HasUpdateField(uint32_t index)
        {
            if (index < m_valuesCount)
                return m_updateMask.GetBit(index);
            return false;
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
        float getAbsoluteAngle(LocationVector const* pos) const { return getAbsoluteAngle(pos->x, pos->y); }
        float toAbsoluteAngle(float relAngle) const { return normalizeOrientation(relAngle + GetOrientation()); }

        float toRelativeAngle(float absAngle) const { return normalizeOrientation(absAngle - GetOrientation()); }
        float getRelativeAngle(Object const* obj) { return getRelativeAngle(obj->GetPosition()); }
        float getRelativeAngle(float x, float y) const { return toRelativeAngle(getAbsoluteAngle(x, y)); }
        float getRelativeAngle(LocationVector const& pos) const { return toRelativeAngle(getAbsoluteAngle(pos.x, pos.y)); }
        float getRelativeAngle(LocationVector const* pos) const { return toRelativeAngle(getAbsoluteAngle(pos)); }

        bool isInDist2d(LocationVector const& pos, float dist) const { return pos.getExactDist2dSq(pos) < dist * dist; }
        bool isInDist(Object* pos, float dist) { return GetPosition().getExactDistSq(pos->GetPositionX(), pos->GetPositionY(), pos->GetPositionZ()) < dist * dist; }
        bool isInDist(LocationVector const& pos, float dist) { return pos.getExactDistSq(pos) < dist * dist; }

        void getNearPoint2D(Object* searcher, float& x, float& y, float distance, float absAngle);
        void getNearPoint(Object* searcher, float& x, float& y, float& z, float distance2d, float absAngle);
        void getClosePoint(float& x, float& y, float& z, float size, float distance2d = 0, float relAngle = 0);

        LocationVector getHitSpherePointFor(LocationVector const& dest);
        void getHitSpherePointFor(LocationVector const& dest, float& x, float& y, float& z) const;
        LocationVector getHitSpherePointFor(LocationVector const& dest) const;
        void updateGroundPositionZ(float x, float y, float& z);
        void updateAllowedPositionZ(float x, float y, float &z, float* groundZ = nullptr);
        float getMapWaterOrGroundLevel(float x, float y, float z, float* ground = nullptr);
        float getFloorZ();
        float getMapHeight(LocationVector pos, bool vmap = true, float distanceToSearch = 50.0f);
        void movePositionToFirstCollision(LocationVector &pos, float dist, float angle);
        LocationVector getFirstCollisionPosition(float dist, float angle);

        virtual float getCombatReach() const { return 0.0f; } // overridden (only) in Unit

        GameObject* summonGameObject(uint32_t entryID, LocationVector pos, QuaternionData const& rot, uint32_t spawnTime = 0, GOSummonType summonType = GO_SUMMON_TIMED_OR_CORPSE_DESPAWN);
        Creature* summonCreature(uint32_t entry, LocationVector position, CreatureSummonDespawnType despawnType = MANUAL_DESPAWN, uint32_t duration = 0, uint32_t spellId = 0);

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

        virtual float getCollisionHeight() const { return 0.0f; }

        //////////////////////////////////////////////////////////////////////////////////////////
        // void outPacket(uint16_t opcode, uint16_t len, const void *data)
        // Sends a packet to the Player
        //
        // \param uint16_t opcode    -   opcode of the packet
        // \param uint16_t len       -   length/size of the packet
        // \param const void *data   -   the data that needs to be sent
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        virtual void outPacket(uint16_t /*opcode*/, uint16_t /*len*/, const void* /*data*/) {};

        //////////////////////////////////////////////////////////////////////////////////////////
        // void sendPacket(WorldPacket *packet)
        //  Sends a packet to the Player
        //
        // \param WorldPAcket *packet      -     the packet that needs to be sent
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        virtual void sendPacket(WorldPacket* /*packet*/) {};

        void SendCreatureChatMessageInRange(Creature* creature, uint32_t textId, Unit* target = nullptr);

        virtual void sendMessageToSet(WorldPacket* data, bool self, bool myteam_only = false);
        virtual void sendMessageToSet(WorldPacket* data, Player const* /*skipp*/);
        virtual void outPacketToSet(uint16_t Opcode, uint16_t Len, const void* Data, bool self);

        //////////////////////////////////////////////////////////////////////////////////////////
        // void SendAIReaction(uint32_t reaction = 2)
        // Notifies the player's clients about the AI reaction of this object
        // (NPC growl for example "aggro sound")
        //
        // \param uint32_t reaction  -  Reaction type
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void SendAIReaction(uint32_t reaction = 2);

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

        uint16_t GetValuesCount() const { return m_valuesCount; }

        MovementInfo obj_movement_info;

        uint32_t m_phase = 1;         // This stores the phase, if two objects have the same bit set, then they can see each other. The default phase is 0x1.

        uint32_t GetPhase() const { return m_phase; }
        virtual void Phase(uint8_t command = PHASE_SET, uint32_t newphase = 1);

        // SpellLog packets just to keep the code cleaner and better to read
        void SendSpellLog(Object* Caster, Object* Target, uint32_t Ability, uint8_t SpellLogType);

        // object faction
        void setServersideFaction();
        uint32_t getServersideFaction();

        WDB::Structures::FactionTemplateEntry const* m_factionTemplate = nullptr;
        WDB::Structures::FactionEntry const* m_factionEntry = nullptr;

        Standing getEnemyReaction(Object* target);
        Standing getFactionReaction(WDB::Structures::FactionTemplateEntry const* factionTemplateEntry, Object* target);

        bool isHostileTo(Object* target);
        bool IsHostileToPlayers();

        bool isFriendlyTo(Object* target);

        bool isNeutralTo(Object* target) const;
        bool isNeutralToAll() const;

        bool isValidTarget(Object* target, SpellInfo const* bySpell = nullptr);       // used for findTarget
        bool isValidAssistTarget(Unit* target, SpellInfo const* bySpell = nullptr); // used for Escorts

        void SetInstanceID(int32_t instance) { m_instanceId = instance; }
        int32_t GetInstanceID() { return m_instanceId; }

        int32_t event_GetInstanceID();

        // Object activation

    private:
        bool Active = false;

    public:
        bool IsActive() { return Active; }
        virtual bool CanActivate();
        virtual void Activate(WorldMap* mgr);
        virtual void deactivate(WorldMap* mgr);
        // Player is in pvp queue.
        bool m_inQueue = false;
        void SetMapMgr(WorldMap* mgr) { m_WorldMap = mgr; }

        void Delete()
        {
            if (IsInWorld())
                RemoveFromWorld(true);
            delete this;
        }
        // Play's a sound to players in range.
        void PlaySoundToSet(uint32_t sound_entry);
        // Is the player in a battleground?
        bool IsInBg();
        // What's their faction? Horde/Ally.
        uint32_t GetTeam() const;
        // Objects directly cannot be in a group.
        //virtual Group* getGroup() { return NULL; }

    protected:
        //void _Create (uint32_t guidlow, uint32_t guidhigh);
        void _Create(uint32_t mapid, float x, float y, float z, float ang);

        // Mark values that need updating for specified player.
        virtual void setUpdateBits(UpdateMask* updateMask, Player* target) const;
        // Mark values that player should get when he/she/it sees object for first time.
        virtual void setCreateBits(UpdateMask* updateMask, Player* target) const;

        // Create updates that player will see
#if VERSION_STRING < WotLK
        void buildMovementUpdate(ByteBuffer* data, uint8_t updateFlags, Player* target);
#else
        void buildMovementUpdate(ByteBuffer* data, uint16_t updateFlags, Player* target);
#endif

        void buildValuesUpdate(uint8_t updateType, ByteBuffer* data, UpdateMask* updateMask, Player* target);

        // WoWGuid class
        WoWGuid m_wowGuid;

        // Type mask
        uint16_t m_objectType;

        //update flag
        uint16_t m_updateFlag;

        // indoorcheck
        bool m_outdoors;
        ZLiquidStatus m_liquidStatus;

        float m_staticFloorZ = -100000.0f;

        // Zone id.
        uint32_t m_zoneId = 0;
        uint32_t m_areaId = 0;
        // Continent/map id.
        uint32_t m_mapId = MAPID_NOT_IN_WORLD;
        // Map manager
        WorldMap* m_WorldMap = nullptr;
        // Current map cell row and column
        uint32_t m_mapCell_x = uint32_t(-1);
        uint32_t m_mapCell_y = uint32_t(-1);

        // Main Function called by isInFront();
        bool inArc(float Position1X, float Position1Y, float FOV, float Orientation, float Position2X, float Position2Y);

        LocationVector m_position = {0, 0, 0, 0};
        LocationVector m_lastMapUpdatePosition;
        LocationVector m_spawnLocation = { 0, 0, 0, 0 };

        // Number of properties
        uint16_t m_valuesCount = 0;

        // List of object properties that need updating.
        UpdateMask m_updateMask;

        // True if object was updated
        bool m_objectUpdated = false;

        int32_t m_instanceId = INSTANCEID_NOT_IN_WORLD;

        // Transporters
        Transporter* m_transport = nullptr;

    public:

        bool m_loadedFromDB = false;

        // Andy's crap
        std::set<Spell*> m_pendingSpells;

        bool GetPoint(float angle, float rad, float & outx, float & outy, float & outz, bool sloppypath = false);
        bool GetRandomPoint(float rad, float & outx, float & outy, float & outz);
        bool GetRandomPoint(float rad, LocationVector & out);
};
