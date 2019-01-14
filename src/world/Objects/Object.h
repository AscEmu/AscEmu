/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "../shared/StackBuffer.h"
#include "../shared/CommonDefines.hpp"
#include "WorldPacket.h"
#include "Units/Creatures/CreatureDefines.hpp"

#if VERSION_STRING < Cata
#include "Data/MovementInfo.h"
#endif


struct WoWObject;

class SpellInfo;

struct FactionDBC;

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
class UpdateMask;
class EventableObject;

enum CurrentSpellType : uint8_t
{
    CURRENT_MELEE_SPELL         = 0,
    CURRENT_GENERIC_SPELL       = 1,
    CURRENT_CHANNELED_SPELL     = 2,
    CURRENT_AUTOREPEAT_SPELL    = 3,
    CURRENT_SPELL_MAX
};

#define MAX_INTERACTION_RANGE 5.0f

typedef struct
{
	uint32 school_type;
	int32 full_damage;
	uint32 resisted_damage;
} dealdamage;

#if VERSION_STRING >= Cata

#if VERSION_STRING == Cata
#include "GameCata/Movement/MovementDefines.h"
#elif VERSION_STRING == Mop
#include "GameMop/Movement/MovementDefines.h"
#endif
#include "LocationVector.h"

class SERVER_DECL MovementInfo
{
    public:

        MovementInfo() : flags(MOVEFLAG_NONE), flags2(MOVEFLAG2_NONE), update_time(0),
            transport_time(0), transport_seat(-1), transport_time2(0), pitch_rate(0.0f), fall_time(0), spline_elevation(0.0f), byte_parameter(0) {}

        ObjectGuid const& getGuid() const { return guid; }
        ObjectGuid const& getGuid2() const { return guid2; }

        MovementFlags getMovementFlags() const { return MovementFlags(flags); }
        void addMovementFlag(MovementFlags _flags) { flags |= _flags; }
        void setMovementFlags(MovementFlags _flags) { flags = _flags; }
        bool hasMovementFlag(MovementFlags _flags) const { return (flags & _flags) != 0; }
        void removeMovementFlag(MovementFlags _flags) { flags &= ~_flags; }

        MovementFlags2 getMovementFlags2() const { return MovementFlags2(flags2); }
        void addMovementFlags2(MovementFlags2 _flags2) { flags2 |= _flags2; }
        bool hasMovementFlag2(MovementFlags2 _flags2) const { return (flags2 & _flags2) != 0; }

        void setUpdateTime(uint32_t time) { update_time = time; }
        uint32_t getUpdateTime() { return update_time; }

        LocationVector const* getPosition() const { return &position; }
        void changeOrientation(float o) { position.o = o; }
        void changePosition(float x, float y, float z, float o) { position.x = x; position.y = y; position.z = z; position.o = o; }

        float getPitch() const { return pitch_rate; }
        uint32_t fetFallTime() const { return fall_time; }
        float getSplineElevation() const { return spline_elevation; }
        int8_t fetByteParam() const { return byte_parameter; }

        struct JumpInfo
        {
            JumpInfo() : velocity(0.f), sinAngle(0.f), cosAngle(0.f), xyspeed(0.f) { }

            float velocity;
            float sinAngle;
            float cosAngle;
            float xyspeed;
        };
        JumpInfo const& getJumpInfo() const { return jump_info; }

        struct StatusInfo
        {
            StatusInfo() : hasFallData(false), hasFallDirection(false), hasOrientation(false),
                hasPitch(false), hasSpline(false), hasSplineElevation(false),
                hasTimeStamp(false), hasTransportTime2(false), hasTransportTime3(false) { }

            bool hasFallData : 1;
            bool hasFallDirection : 1;
            bool hasOrientation : 1;
            bool hasPitch : 1;
            bool hasSpline : 1;
            bool hasSplineElevation : 1;
            bool hasTimeStamp : 1;
            bool hasTransportTime2 : 1;
            bool hasTransportTime3 : 1;
        };
        StatusInfo const& getMovementStatusInfo() const { return status_info; }

        // transport
        ObjectGuid& getTransportGuid() { return transport_guid; }

        LocationVector const* getTransportPosition() const { return &transport_position; }

        int8_t getTransportSeat() const { return transport_seat; }

        uint32_t getTransportTime() const { return transport_time; }
        uint32_t getTransportTime2() const { return transport_time2; }

        void setTransportData(ObjectGuid _guid, float x, float y, float z, float o, uint32_t time, int8_t seat)
        {
            transport_guid = _guid;
            transport_position.x = x;
            transport_position.y = y;
            transport_position.z = z;
            transport_position.o = o;
            transport_time = time;
            transport_seat = seat;
        }

        void clearTransportData()
        {
            transport_guid = 0;
            transport_position.x = 0.0f;
            transport_position.y = 0.0f;
            transport_position.z = 0.0f;
            transport_position.o = 0.0f;
            transport_time = 0;
            transport_seat = -1;
        }

        void readMovementInfo(ByteBuffer& data, uint16_t opcode);
        void writeMovementInfo(ByteBuffer& data, uint16_t opcode, float custom_speed = 0.f) const;

        uint32_t flags;
        uint16_t flags2;

    private:

        ObjectGuid guid;
        ObjectGuid guid2;

        uint32_t update_time;

        LocationVector position;

        float pitch_rate;
        uint32_t fall_time;
        float spline_elevation;
        int8_t byte_parameter;

        JumpInfo jump_info;

        StatusInfo status_info;

        // transport
        ObjectGuid transport_guid;

        LocationVector transport_position;
        uint32_t transport_time;
        int8_t transport_seat;
        uint32_t transport_time2;
};

inline WorldPacket& operator<< (WorldPacket& buf, MovementInfo const& mi)
{
    mi.writeMovementInfo(buf, buf.GetOpcode());
    return buf;
}

inline WorldPacket& operator>> (WorldPacket& buf, MovementInfo& mi)
{
    mi.readMovementInfo(buf, buf.GetOpcode());
    return buf;
}

inline float normalizeOrientation(float orientation)
{
    if (orientation < 0)
    {
        float mod = orientation *-1;
        mod = fmod(mod, 2.0f * static_cast<float>(M_PI));
        mod = -mod + 2.0f * static_cast<float>(M_PI);
        return mod;
    }

    return fmod(orientation, 2.0f * static_cast<float>(M_PI));
}
#endif

// MIT Start
class SERVER_DECL Object : public EventableObject, public IUpdatable
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Object values

protected:
    union
    {
        uint8_t* wow_data_ptr;
        WoWObject* wow_data;
        int32_t* m_int32Values;
        uint32_t* m_uint32Values;
        float* m_floatValues;
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
    uint32_t getOType() const;
    void setOType(uint32_t type);
    void setObjectType(uint8_t objectTypeId);

    void setEntry(uint32_t entry);
    uint32_t getEntry() const;

    float getScale() const;
    void setScale(float scaleX);

    // old update data handling
    void setByteValue(uint16_t index, uint8_t offset, uint8_t value);
    uint8_t getByteValue(uint16_t index, uint8_t offset) const;

    void setByteFlag(uint16_t index, uint8_t offset, uint8_t newFlag);
    void removeByteFlag(uint16_t index, uint8_t offset, uint8_t oldFlag);

    bool hasByteFlag(uint16_t index, uint8_t offset, uint8_t flag);

    void setUInt16Value(uint16_t index, uint8_t offset, uint16_t value);
    uint16_t getUInt16Value(uint16_t index, uint8_t offset) const;

    void setUInt32Value(uint16_t index, uint32_t value);
    uint32_t getUInt32Value(uint16_t index) const;
    void modUInt32Value(uint16_t index, int32_t mod);

    uint32_t getPercentModUInt32Value(uint16_t index, const int32_t value);

    void setInt32Value(uint16_t index, int32_t value);
    uint32_t getInt32Value(uint16_t index) const;
    void modInt32Value(uint16_t index, int32_t value);

    void setUInt64Value(uint16_t index, uint64_t value);
    uint64_t getUInt64Value(uint16_t index) const;

    void setFloatValue(uint16_t index, float value);
    float getFloatValue(uint16_t index) const;

    void modFloatValue(uint16_t index, float value);
    void modFloatValueByPCT(uint16_t index, int32 byPct);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Object update
    void updateObject();

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

    void _UpdateSpells(uint32_t time); // moved here from Unit class since GameObject can be caster as well

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

    virtual Object* getPlayerOwner();

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

    uint32 GetTypeFromGUID() const { return (m_uint32Values[OBJECT_FIELD_GUID + 1] & HIGHGUID_TYPE_MASK); }
    uint32 GetUIdFromGUID() const { return (m_uint32Values[OBJECT_FIELD_GUID] & LOWGUID_ENTRY_MASK); }

        // typeFlags
        bool IsType(TYPE type_mask) const { return (type_mask & m_objectType) != 0; }

        //! This includes any nested objects we have, inventory for example.
        virtual uint32 buildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target);
        uint32 BuildValuesUpdateBlockForPlayer(ByteBuffer* buf, Player* target);
        uint32 BuildValuesUpdateBlockForPlayer(ByteBuffer* buf, UpdateMask* mask);

        WorldPacket* BuildFieldUpdatePacket(uint32 index, uint32 value);
        void BuildFieldUpdatePacket(Player* Target, uint32 Index, uint32 Value);
        void BuildFieldUpdatePacket(ByteBuffer* buf, uint32 Index, uint32 Value);

        virtual void DealDamage(Unit* pVictim, uint32 damage, uint32 targetEvent, uint32 unitEvent, uint32 spellId, bool no_remove_auras = false);

        bool SetPosition(float newX, float newY, float newZ, float newOrientation, bool allowPorting = false);
        bool SetPosition(const LocationVector & v, bool allowPorting = false);

        const float & GetPositionX() const { return m_position.x; }
        const float & GetPositionY() const { return m_position.y; }
        const float & GetPositionZ() const { return m_position.z; }
        const float & GetOrientation() const { return m_position.o; }
        void SetOrientation(float o) { m_position.o = o; }

        const float & GetSpawnX() const { return m_spawnLocation.x; }
        const float & GetSpawnY() const { return m_spawnLocation.y; }
        const float & GetSpawnZ() const { return m_spawnLocation.z; }
        const float & GetSpawnO() const { return m_spawnLocation.o; }
        LocationVector GetSpawnPosition() const { return m_spawnLocation; }

        ::DBC::Structures::AreaTableEntry const* GetArea();

        LocationVector GetPosition() const { return LocationVector(m_position); }
        LocationVector & GetPositionNC() { return m_position; }
        LocationVector* GetPositionV() { return &m_position; }

#if VERSION_STRING < Cata
        // TransporterInfo
        float GetTransPositionX() const { return obj_movement_info.transport_data.relativePosition.x; }
        float GetTransPositionY() const { return obj_movement_info.transport_data.relativePosition.y; }
        float GetTransPositionZ() const { return obj_movement_info.transport_data.relativePosition.z; }
        float GetTransPositionO() const { return obj_movement_info.transport_data.relativePosition.o; }
        uint32 GetTransTime() const { return obj_movement_info.transport_time; }
#ifdef FT_VEHICLES
        // TODO check if this is in BC
        uint8 GetTransSeat() const { return obj_movement_info.transport_seat; }
#endif
#else
        float GetTransPositionX() const { return obj_movement_info.getTransportPosition()->x; }
        float GetTransPositionY() const { return obj_movement_info.getTransportPosition()->y; }
        float GetTransPositionZ() const { return obj_movement_info.getTransportPosition()->z; }
        float GetTransPositionO() const { return obj_movement_info.getTransportPosition()->o; }
        uint32 GetTransTime() const { return obj_movement_info.getTransportTime(); }
        uint8 GetTransSeat() const { return obj_movement_info.getTransportSeat(); }
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

        void EventSetUInt32Value(uint16 index, uint32 value);

        void SetFlag(const uint16 index, uint32 newFlag);

        void RemoveFlag(const uint16 index, uint32 oldFlag);

        bool HasFlag(const uint16 index, uint32 flag) const
        {
            ARCEMU_ASSERT(index < m_valuesCount);
            return (m_uint32Values[index] & flag) != 0;
        }

        void ApplyModFlag(uint16 index, uint32 flag, bool apply)
        {
            if (apply)
                SetFlag(index, flag);
            else
                RemoveFlag(index, flag);
        }

        ////////////////////////////////////////
        void ClearUpdateMask()
        {
            m_updateMask.Clear();
            m_objectUpdated = false;
        }

        bool HasUpdateField(uint32 index)
        {
            ARCEMU_ASSERT(index < m_valuesCount);
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
        void SendMessageToSet(StackBufferBase* data, bool self) { OutPacketToSet(data->GetOpcode(), static_cast<uint16>(data->GetSize()), data->GetBufferPointer(), self); }
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

        // Fill values with data from a space separated string of uint32s.
        void LoadValues(const char* data);

        uint16 GetValuesCount() const { return m_valuesCount; }

        MovementInfo obj_movement_info;
        Transporter* GetTransport() const;

        uint32 m_phase;         // This stores the phase, if two objects have the same bit set, then they can see each other. The default phase is 0x1.

    uint32 GetPhase() { return m_phase; }
        virtual void Phase(uint8 command = PHASE_SET, uint32 newphase = 1);

        void EventSpellDamage(uint64 Victim, uint32 SpellID, uint32 Damage);
        void SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage, bool allowProc, bool static_damage = false, bool no_remove_auras = false);
        virtual bool IsCriticalDamageForSpell(Object* /*victim*/, SpellInfo const* /*spell*/) { return false; }
        virtual float GetCriticalDamageBonusForSpell(Object* /*victim*/, SpellInfo const* /*spell*/, float /*amount*/) { return 0; }
        virtual bool IsCriticalHealForSpell(Object* /*victim*/, SpellInfo const* /*spell*/) { return false; }
        virtual float GetCriticalHealBonusForSpell(Object* /*victim*/, SpellInfo const* /*spell*/, float /*amount*/) { return 0; }

        // SpellLog packets just to keep the code cleaner and better to read
        void SendSpellLog(Object* Caster, Object* Target, uint32 Ability, uint8 SpellLogType);

        void SendSpellNonMeleeDamageLog(Object* Caster, Object* Target, uint32 SpellID, uint32 Damage, uint8 School, uint32 AbsorbedDamage, uint32 ResistedDamage, bool PhysicalDamage, uint32 BlockedDamage, bool CriticalHit, bool bToSet);
        void SendAttackerStateUpdate(Object* Caster, Object* Target, dealdamage* Dmg, uint32 Damage, uint32 Abs, uint32 BlockedDamage, uint32 HitStatus, uint32 VState);

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
        virtual Group* GetGroup() { return NULL; }

    protected:

        //void _Create (uint32 guidlow, uint32 guidhigh);
        void _Create(uint32 mapid, float x, float y, float z, float ang);

        // Mark values that need updating for specified player.
        virtual void _SetUpdateBits(UpdateMask* updateMask, Player* target) const;
        // Mark values that player should get when he/she/it sees object for first time.
        virtual void _SetCreateBits(UpdateMask* updateMask, Player* target) const;

        // Create updates that player will see
#if VERSION_STRING < WotLK
        void buildMovementUpdate(ByteBuffer* data, uint8_t flags, Player* target);
#else
        void buildMovementUpdate(ByteBuffer* data, uint16 flags, Player* target);
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

    public:

        bool m_loadedFromDB;

        // Andy's crap
        std::set<Spell*> m_pendingSpells;

        bool GetPoint(float angle, float rad, float & outx, float & outy, float & outz, bool sloppypath = false);
        bool GetRandomPoint(float rad, float & outx, float & outy, float & outz) { return GetPoint(Util::getRandomFloat(float(M_PI * 2)), rad, outx, outy, outz); }
        bool GetRandomPoint(float rad, LocationVector & out) { return GetRandomPoint(rad, out.x, out.y, out.z); }
};

#endif // OBJECT_H
