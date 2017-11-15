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
#if VERSION_STRING == Cata
    #include "Storage/DB2/DB2Structures.h"
#endif
#include "../shared/StackBuffer.h"
#include "../shared/CommonDefines.hpp"
#include "WorldPacket.h"
#include "Units/Creatures/CreatureDefines.hpp"

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


enum HIGHGUID_TYPE
{
    HIGHGUID_TYPE_PLAYER			= 0x00000000,
    HIGHGUID_TYPE_CORPSE			= 0x30000000,
    HIGHGUID_TYPE_ITEM				= 0x40000000,
    HIGHGUID_TYPE_CONTAINER			= 0x50000000,			// confirm this pl0x
    HIGHGUID_TYPE_DYNAMICOBJECT		= 0x60000000,
    HIGHGUID_TYPE_WAYPOINT			= 0x10000000,
    HIGHGUID_TYPE_TRANSPORTER		= 0x1FC00000,
    HIGHGUID_TYPE_GAMEOBJECT		= 0xF1100000,
    HIGHGUID_TYPE_UNIT				= 0xF1300000,
    HIGHGUID_TYPE_PET				= 0xF1400000,
    HIGHGUID_TYPE_VEHICLE			= 0xF1500000,
    HIGHGUID_TYPE_GROUP             = 0x1F500000,
    HIGHGUID_TYPE_GUILD             = 0x1FF70000,
//===============================================
    HIGHGUID_TYPE_MASK				= 0xFFF00000,
    LOWGUID_ENTRY_MASK				= 0x00FFFFFF,
};

#define GET_TYPE_FROM_GUID(x) (Arcemu::Util::GUID_HIPART((x)) & HIGHGUID_TYPE_MASK)
#define GET_LOWGUID_PART(x) (Arcemu::Util::GUID_LOPART((x)) & LOWGUID_ENTRY_MASK)

#define IS_GROUP(Guid) (Arcemu::Util::GUID_HIPART((Guid)) == HIGHGUID_TYPE_GROUP)
#define IS_PLAYER_GUID(Guid) (Arcemu::Util::GUID_HIPART((Guid)) == HIGHGUID_TYPE_PLAYER && Guid != 0)

#define MAX_INTERACTION_RANGE 5.0f

typedef struct
{
	uint32 school_type;
	int32 full_damage;
	uint32 resisted_damage;
} dealdamage;

#if VERSION_STRING == Cata
#include "GameCata/Movement/MovementDefines.h"

class SERVER_DECL MovementInfo
{
    public:

        MovementInfo() : move_flags(MOVEFLAG_NONE), move_flags2(MOVEFLAG2_NONE), update_time(0),
            transport_time(0), transport_seat(-1), transport_time2(0), pitch_rate(0.0f), fall_time(0), spline_elevation(0.0f), byte_parameter(0) {}

        ObjectGuid const& getGuid() const { return guid; }
        ObjectGuid const& getGuid2() const { return guid2; }

        MovementFlags getMovementFlags() const { return MovementFlags(move_flags); }
        void addMovementFlag(MovementFlags flags) { move_flags |= flags; }
        void setMovementFlags(MovementFlags flags) { move_flags = flags; }
        bool hasMovementFlag(MovementFlags flags) const { return (move_flags & flags) != 0; }
        void removeMovementFlag(MovementFlags flags) { move_flags &= ~flags; }

        MovementFlags2 getMovementFlags2() const { return MovementFlags2(move_flags2); }
        void addMovementFlags2(MovementFlags2 flags2) { move_flags2 |= flags2; }
        bool hasMovementFlag2(MovementFlags2 flags2) const { return (move_flags2 & flags2) != 0; }

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

        void setTransportData(ObjectGuid guid, float x, float y, float z, float o, uint32_t time, int8_t seat)
        {
            transport_guid = guid;
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

        void readMovementInfo(ByteBuffer& data, uint32_t opcode);
        void writeMovementInfo(ByteBuffer& data, uint32_t opcode, float custom_speed = 0.f) const;

    private:

        ObjectGuid guid;
        ObjectGuid guid2;

        uint32_t move_flags;
        uint16_t move_flags2;

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

static float normalizeOrientation(float orientation)
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
#else
struct MovementInfo
{
    WoWGuid object_guid;
    uint32_t flags;
    uint16_t flags2;
    LocationVector position;
    uint32_t time;

    //pitch
    //-1.55=looking down, 0=looking forward, +1.55=looking up
    float pitch;

    //jumping related
    float redirectVelocity;
    float redirectSin;      //on slip 8 is zero, on jump some other number
    float redirectCos;
    float redirect2DSpeed;  //9,10 changes if you are not on foot

    uint32_t fall_time;       //fall_time in ms

    float spline_elevation;

    struct TransporterInfo
    {
        Transporter* m_transporter;
        WoWGuid transGuid;
        uint64_t guid;        // switch to WoWGuid
        LocationVector position;
        uint32_t time;
        uint32_t time2;
        uint8_t seat;

        void Clear()
        {
            m_transporter = nullptr;
            transGuid = 0;
            guid = 0;
            position.ChangeCoords(0.0f, 0.0f, 0.0f, 0.0f);
            time = 0;
            time2 = 0;
            seat = 0;
        }
    }transporter_info;

    MovementInfo()
    {
        object_guid = 0;
        flags = 0;
        flags2 = 0;
        position.ChangeCoords(0.0f, 0.0f, 0.0f, 0.0f);

        time = 0;

        pitch = 0.0f;

        redirectVelocity = 0.0f;
        redirectSin = 0.0f;
        redirectCos = 0.0f;
        redirect2DSpeed = 0.0f;

        fall_time = 0;
        spline_elevation = 0;

        transporter_info.Clear();
    }

    void init(WorldPacket& data);
    void write(WorldPacket& data);

    bool IsOnTransport() const { return this->transporter_info.guid != 0; };

    //flags uint32_t
    bool HasMovementFlag(uint32_t move_flags) const { return (flags & move_flags) != 0; }
    uint32_t GetMovementFlags() const { return flags; }
    void RemoveMovementFlag(uint32_t move_flags) { flags &= ~move_flags; }

    //flags2 uint16_t
    bool HasMovementFlag2(uint16_t move_flags2) const { return (flags2 & move_flags2) != 0; }
    uint16_t GetMovementFlags2() const { return flags2; }
};
#endif

//////////////////////////////////////////////////////////////////////////////////////////
/// class Object:Base object for every item, unit, player, corpse, container, etc
//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL Object : public EventableObject, public IUpdatable
{
public:

    // MIT Start
    //////////////////////////////////////////////////////////////////////////////////////////
    // Object values

    void setByteValue(uint16_t index, uint8_t offset, uint8_t value);
    uint8_t getByteValue(uint16_t index, uint8_t offset) const;

    void setByteFlag(uint16_t index, uint8_t offset, uint8_t newFlag);
    void removeByteFlag(uint16_t index, uint8_t offset, uint8_t oldFlag);

    bool hasByteFlag(uint16_t index, uint8_t offset, uint8_t flag);

    void setUInt16Value(uint16_t index, uint8_t offset, uint16_t value);
    uint16_t getUInt16Value(uint16_t index, uint8_t offset) const;

    void setUInt32Value(uint16_t index, uint32_t value);
    uint32_t getUInt32Value(uint16_t index) const;

    void setUInt64Value(uint16_t index, uint64_t value);
    uint64_t getUInt64Value(uint16_t index) const;

    void setFloatValue(uint16_t index, float value);
    float getFloatValue(uint16_t index) const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Object update
    void updateObject();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Position functions
    bool isInRange(LocationVector location, float square_r) const;
    bool isInRange(float x, float y, float z, float square_r) const;

    float getDistanceSq(LocationVector target) const;
    float getDistanceSq(float x, float y, float z) const;
    // MIT End

        typedef std::set<Object*> InRangeSet;

        Object();
        virtual ~Object();

        void Update(unsigned long time_passed) {}

        /// True if object exists in world, else false
        bool IsInWorld() { return m_mapMgr != NULL; }
        virtual void AddToWorld();
        virtual void AddToWorld(MapMgr* pMapMgr);
        void PushToWorld(MapMgr*);
        virtual void RemoveFromWorld(bool free_guid);


        //////////////////////////////////////////////////////////////////////////////////////////
        /// virtual void OnPrePushToWorld()
        /// Virtual method that is called, BEFORE pushing the Object in the game world
        ///
        /// \param none
        ///
        /// \return none
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
        virtual void OnPrePushToWorld() {}


        //////////////////////////////////////////////////////////////////////////////////////////
        /// virtual void OnPushToWorld()
        /// Virtual method that is called, AFTER pushing the Object in the game world
        ///
        /// \param none
        ///
        /// \return none
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
        virtual void OnPushToWorld() {}


        //////////////////////////////////////////////////////////////////////////////////////////
        /// virtual void OnPreRemoveFromWorld()
        /// Virtual method that is called, BEFORE removing the Object from the game world
        ///
        /// \param none
        ///
        /// \return none
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
        virtual void OnPreRemoveFromWorld() {}


        //////////////////////////////////////////////////////////////////////////////////////////
        /// virtual void OnRemoveFromWorld()
        /// Virtual method that is called, AFTER removing the Object from the game world
        ///
        /// \param none
        ///
        /// \return none
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
        virtual void OnRemoveFromWorld() {}


        /// Guid always comes first
        uint64 GetGUID() const { return getUInt64Value(OBJECT_FIELD_GUID); }
        void SetGUID(uint64 GUID) { setUInt64Value(OBJECT_FIELD_GUID, GUID); }
        const uint32 GetLowGUID() const { return m_uint32Values[OBJECT_FIELD_GUID]; }
        uint32 GetHighGUID() { return m_uint32Values[OBJECT_FIELD_GUID + 1]; }
        void SetLowGUID(uint32 val) { m_uint32Values[OBJECT_FIELD_GUID] = val; }
        void SetHighGUID(uint32 val) { m_uint32Values[OBJECT_FIELD_GUID + 1] = val; }

        const WoWGuid & GetNewGUID() const { return m_wowGuid; }
        uint32 GetEntry() { return m_uint32Values[OBJECT_FIELD_ENTRY]; }
        void SetEntry(uint32 value) { setUInt32Value(OBJECT_FIELD_ENTRY, value); }

        float GetScale() { return m_floatValues[OBJECT_FIELD_SCALE_X]; }
        void SetScale(float scale) { setFloatValue(OBJECT_FIELD_SCALE_X, scale); };

        const uint32 GetTypeFromGUID() const { return (m_uint32Values[OBJECT_FIELD_GUID + 1] & HIGHGUID_TYPE_MASK); }
        const uint32 GetUIdFromGUID() const { return (m_uint32Values[OBJECT_FIELD_GUID] & LOWGUID_ENTRY_MASK); }

        // type
        const uint8 & GetTypeId() const { return m_objectTypeId; }
        bool IsType(TYPE type_mask) const { return (type_mask & m_objectType) != 0; }

        bool IsUnit() { return (m_objectTypeId == TYPEID_UNIT || m_objectTypeId == TYPEID_PLAYER); }
        bool IsPlayer() { return m_objectTypeId == TYPEID_PLAYER; }
        bool IsCreature() { return m_objectTypeId == TYPEID_UNIT; }
        bool IsItem() { return m_objectTypeId == TYPEID_ITEM; }
        virtual bool IsPet() { return false; }
        virtual bool IsTotem() { return false; }
        virtual bool IsSummon() { return false; }
        virtual bool IsVehicle() { return false; }
        bool IsGameObject() { return m_objectTypeId == TYPEID_GAMEOBJECT; }
        bool IsCorpse() { return m_objectTypeId == TYPEID_CORPSE; }
        bool IsContainer() { return m_objectTypeId == TYPEID_CONTAINER; }

        //! This includes any nested objects we have, inventory for example.
        virtual uint32 BuildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target);
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

#if VERSION_STRING != Cata
        // TransporterInfo
        float GetTransPositionX() const { return obj_movement_info.transporter_info.position.x; }
        float GetTransPositionY() const { return obj_movement_info.transporter_info.position.y; }
        float GetTransPositionZ() const { return obj_movement_info.transporter_info.position.z; }
        float GetTransPositionO() const { return obj_movement_info.transporter_info.position.o; }
        uint32 GetTransTime() const { return obj_movement_info.transporter_info.time; }
        uint8 GetTransSeat() const { return obj_movement_info.transporter_info.seat; }
#else
        float GetTransPositionX() const { return obj_movement_info.getTransportPosition()->x; }
        float GetTransPositionY() const { return obj_movement_info.getTransportPosition()->y; }
        float GetTransPositionZ() const { return obj_movement_info.getTransportPosition()->z; }
        float GetTransPositionO() const { return obj_movement_info.getTransportPosition()->o; }
        uint32 GetTransTime() const { return obj_movement_info.getTransportTime(); }
        uint8 GetTransSeat() const { return obj_movement_info.getTransportSeat(); }
#endif

        /// Distance Calculation
        float CalcDistance(Object* Ob);
        float CalcDistance(float ObX, float ObY, float ObZ);
        float CalcDistance(Object* Oa, Object* Ob);
        float CalcDistance(Object* Oa, float ObX, float ObY, float ObZ);
        float CalcDistance(float OaX, float OaY, float OaZ, float ObX, float ObY, float ObZ);
        /// NYS: scriptdev2
        bool IsInMap(Object* obj) { return GetMapId() == obj->GetMapId() && GetInstanceID() == obj->GetInstanceID(); }
        bool IsWithinDistInMap(Object* obj, const float dist2compare) const;
        bool IsWithinLOSInMap(Object* obj);
        bool IsWithinLOS(LocationVector location);

        /// Only for MapMgr use
        MapCell* GetMapCell() const;
        const uint32 GetMapCellX() { return m_mapCell_x; }
        const uint32 GetMapCellY() { return m_mapCell_y; }
        /// Only for MapMgr use
        void SetMapCell(MapCell* cell);
        /// Only for MapMgr use
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

        const uint32 GetMapId() const { return m_mapId; }
        const uint32 & GetZoneId() const { return m_zoneId; }

        void ModFloatValue(const uint32 index, const float value);
        void ModFloatValueByPCT(const uint32 index, int32 byPct);
        void ModSignedInt32Value(uint32 index, int32 value);
        void ModUnsigned32Value(uint32 index, int32 mod);
        uint32 GetModPUInt32Value(const uint32 index, const int32 value);

        void SetNewGuid(uint32 Guid)
        {
            SetLowGUID(Guid);
            m_wowGuid.Init(GetGUID());
        }

        void EventSetUInt32Value(uint32 index, uint32 value);

        void SetFlag(const uint32 index, uint32 newFlag);

        void RemoveFlag(const uint32 index, uint32 oldFlag);

        uint32 HasFlag(const uint32 index, uint32 flag) const
        {
            ARCEMU_ASSERT(index < m_valuesCount);
            return m_uint32Values[index] & flag;
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

        /// Use this to check if a object is in range of another
        bool isInRange(Object* target, float range);

        /// Use this to Check if a object is in front of another object.
        bool isInFront(Object* target);
        /// Use this to Check if a object is in back of another object.
        bool isInBack(Object* target);
        /// Check to see if an object is in front of a target in a specified arc (in degrees)
        bool isInArc(Object* target, float degrees);
        /// NYS: Scriptdev2
        bool HasInArc(float degrees, Object* target);
        /// Calculates the angle between two positions
        float calcAngle(float Position1X, float Position1Y, float Position2X, float Position2Y);
        float calcRadAngle(float Position1X, float Position1Y, float Position2X, float Position2Y);

        /// Converts to 360 > x > 0
        float getEasyAngle(float angle);

        const float getDistanceSq(Object* obj)
        {
            if (obj->GetMapId() != m_mapId)
                return 40000.0f;                        /// enough for out of range
            return m_position.distanceSquare(obj->GetPosition());
        }

        float CalcDistance(LocationVector & comp)
        {
            return comp.Distance(m_position);
        }

        const float GetDistance2dSq(Object* obj)
        {
            if (obj->GetMapId() != m_mapId)
                return 40000.0f;                        /// enough for out of range
            return m_position.Distance2DSq(obj->m_position);
        }

        /// In-range object management, not sure if we need it
        bool IsInRangeSet(Object* pObj)
        {
            return !(m_objectsInRange.find(pObj) == m_objectsInRange.end());
        }

        virtual void AddInRangeObject(Object* pObj);

        Mutex m_inrangechangelock;

        void RemoveInRangeObject(Object* pObj);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// void RemoveSelfFromInrangeSets()
        /// Removes the Object from the inrangesets of the Objects in range
        ///
        /// \param none
        ///
        /// \return none
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
        void RemoveSelfFromInrangeSets();


        bool HasInRangeObjects()
        {
            return (m_objectsInRange.size() > 0);
        }

        virtual void OnRemoveInRangeObject(Object* pObj);

        virtual void ClearInRangeSet()
        {
            m_objectsInRange.clear();
            m_inRangePlayers.clear();
            m_oppFactsInRange.clear();
            m_sameFactsInRange.clear();
        }

        size_t GetInRangeCount() { return m_objectsInRange.size(); }
        size_t GetInRangePlayersCount() { return m_inRangePlayers.size(); }

        auto GetInRangeSet() { return m_objectsInRange; }

        InRangeSet::iterator GetInRangeSetBegin() { return m_objectsInRange.begin(); }
        InRangeSet::iterator GetInRangeSetEnd() { return m_objectsInRange.end(); }
        InRangeSet::iterator FindInRangeSet(Object* obj) { return m_objectsInRange.find(obj); }

        void RemoveInRangeObject(InRangeSet::iterator itr)
        {
            OnRemoveInRangeObject(*itr);
            m_objectsInRange.erase(itr);
        }

        bool RemoveIfInRange(Object* obj)
        {
            InRangeSet::iterator itr = m_objectsInRange.find(obj);
            if (obj->IsPlayer())
                m_inRangePlayers.erase(obj);

            if (itr == m_objectsInRange.end())
                return false;

            m_objectsInRange.erase(itr);

            return true;
        }

        bool IsInRangeSameFactSet(Object* pObj) { return (m_sameFactsInRange.count(pObj) > 0); }
        void UpdateSameFactionSet();
        std::set<Object*>::iterator GetInRangeSameFactsSetBegin() { return m_sameFactsInRange.begin(); }
        std::set<Object*>::iterator GetInRangeSameFactsSetEnd() { return m_sameFactsInRange.end(); }

        bool IsInRangeOppFactSet(Object* pObj) { return (m_oppFactsInRange.count(pObj) > 0); }
        void UpdateOppFactionSet();
        size_t GetInRangeOppFactsSize() { return m_oppFactsInRange.size(); }
        std::set<Object*>::iterator GetInRangeOppFactsSetBegin() { return m_oppFactsInRange.begin(); }
        std::set<Object*>::iterator GetInRangeOppFactsSetEnd() { return m_oppFactsInRange.end(); }
        std::set<Object*>::iterator GetInRangePlayerSetBegin() { return m_inRangePlayers.begin(); }
        std::set<Object*>::iterator GetInRangePlayerSetEnd() { return m_inRangePlayers.end(); }
        std::set<Object*> * GetInRangePlayerSet() { return &m_inRangePlayers; }
        std::set<Object*> & GetInRangePlayers() { return m_inRangePlayers; }
        std::set<Object*> & GetInRangeOpposingFactions() { return m_oppFactsInRange; }
        std::set<Object*> & GetInRangeSameFactions() { return m_sameFactsInRange; }
        std::set<Object*> & GetInRangeObjects() { return m_objectsInRange; }


        //////////////////////////////////////////////////////////////////////////////////////////
        /// void OutPacket(uint16 opcode, uint16 len, const void *data)
        /// Sends a packet to the Player
        ///
        /// \param uint16 opcode      -   opcode of the packet
        /// \param uint16 len         -   length/size of the packet
        /// \param const void *data   -   the data that needs to be sent
        ///
        /// \return none
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
#if VERSION_STRING == Cata
        virtual void OutPacket(uint32 opcode, uint16 len, const void* data) {};
#else
        virtual void OutPacket(uint16 opcode, uint16 len, const void* data) {};
#endif

        //////////////////////////////////////////////////////////////////////////////////////////
        /// void SendPacket(WorldPacket *packet)
        ///  Sends a packet to the Player
        ///
        /// \param WorldPAcket *packet      -     the packet that needs to be sent
        ///
        /// \return none
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
        virtual void SendPacket(WorldPacket* packet) {};


        void SendCreatureChatMessageInRange(Creature* creature, uint32_t textId);
        void SendMonsterSayMessageInRange(Creature* creature, MySQLStructure::NpcMonsterSay* npcMonsterSay, int randChoice, uint32_t event);

        virtual void SendMessageToSet(WorldPacket* data, bool self, bool myteam_only = false);
        void SendMessageToSet(StackBufferBase* data, bool self) { OutPacketToSet(data->GetOpcode(), static_cast<uint16>(data->GetSize()), data->GetBufferPointer(), self); }
#if VERSION_STRING == Cata
        virtual void OutPacketToSet(uint32 Opcode, uint16 Len, const void* Data, bool self);
#else
        virtual void OutPacketToSet(uint16 Opcode, uint16 Len, const void* Data, bool self);
#endif

        //////////////////////////////////////////////////////////////////////////////////////////
        ///void SendAIReaction(uint32 reaction = 2)
        /// Notifies the player's clients about the AI reaction of this object
        /// (NPC growl for example "aggro sound")
        ///
        /// \param uint32 reaction  -  Reaction type
        ///
        /// \return none
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
        void SendAIReaction(uint32 reaction = 2);


        //////////////////////////////////////////////////////////////////////////////////////////
        ///void SendDestroyObject()
        /// Destroys this Object for the players' clients that are nearby
        /// (removes object from the scene)
        ///
        /// \param none
        ///
        /// \return none
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
        void SendDestroyObject();


        /// Fill values with data from a space separated string of uint32s.
        void LoadValues(const char* data);

        uint16 GetValuesCount() const { return m_valuesCount; }

        MovementInfo obj_movement_info;
        Transporter* GetTransport() const;

        uint32 m_phase;         /// This stores the phase, if two objects have the same bit set, then they can see each other. The default phase is 0x1.

        const uint32 GetPhase() { return m_phase; }
        virtual void Phase(uint8 command = PHASE_SET, uint32 newphase = 1);

        void EventSpellDamage(uint64 Victim, uint32 SpellID, uint32 Damage);
        void SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage, bool allowProc, bool static_damage = false, bool no_remove_auras = false);
        virtual bool IsCriticalDamageForSpell(Object* victim, SpellInfo* spell) { return false; }
        virtual float GetCriticalDamageBonusForSpell(Object* victim, SpellInfo* spell, float amount) { return 0; }
        virtual bool IsCriticalHealForSpell(Object* victim, SpellInfo* spell) { return false; }
        virtual float GetCriticalHealBonusForSpell(Object* victim, SpellInfo* spell, float amount) { return 0; }

        /// SpellLog packets just to keep the code cleaner and better to read
        void SendSpellLog(Object* Caster, Object* Target, uint32 Ability, uint8 SpellLogType);

        void SendSpellNonMeleeDamageLog(Object* Caster, Object* Target, uint32 SpellID, uint32 Damage, uint8 School, uint32 AbsorbedDamage, uint32 ResistedDamage, bool PhysicalDamage, uint32 BlockedDamage, bool CriticalHit, bool bToSet);
        void SendAttackerStateUpdate(Object* Caster, Object* Target, dealdamage* Dmg, uint32 Damage, uint32 Abs, uint32 BlockedDamage, uint32 HitStatus, uint32 VState);

        /// object faction
        void _setFaction();
        uint32 _getFaction();

        DBC::Structures::FactionTemplateEntry const* m_faction;
        DBC::Structures::FactionEntry const* m_factionDBC;

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
        /// Player is in pvp queue.
        bool m_inQueue;
        void SetMapMgr(MapMgr* mgr) { m_mapMgr = mgr; }

        void Delete()
        {
            if (IsInWorld())
                RemoveFromWorld(true);
            delete this;
        }
        /// Play's a sound to players in range.
        void PlaySoundToSet(uint32 sound_entry);
        /// Is the player in a battleground?
        bool IsInBg();
        /// What's their faction? Horde/Ally.
        uint32 GetTeam();
        /// Objects directly cannot be in a group.
        virtual Group* GetGroup() { return NULL; }

    protected:

        //void _Create (uint32 guidlow, uint32 guidhigh);
        void _Create(uint32 mapid, float x, float y, float z, float ang);

        /// Mark values that need updating for specified player.
        virtual void _SetUpdateBits(UpdateMask* updateMask, Player* target) const;
        /// Mark values that player should get when he/she/it sees object for first time.
        virtual void _SetCreateBits(UpdateMask* updateMask, Player* target) const;

        /// Create updates that player will see
        void _BuildMovementUpdate(ByteBuffer* data, uint16 flags, Player* target);
        void _BuildValuesUpdate(ByteBuffer* data, UpdateMask* updateMask, Player* target);

        /// WoWGuid class
        WoWGuid m_wowGuid;

        // Type mask
        uint16 m_objectType;

        /// Type id.
        uint8 m_objectTypeId;

        //update flag
        uint16 m_updateFlag;

        /// Zone id.
        uint32 m_zoneId;
        /// Continent/map id.
        uint32 m_mapId;
        /// Map manager
        MapMgr* m_mapMgr;
        /// Current map cell row and column
        uint32 m_mapCell_x, m_mapCell_y;

        /// Main Function called by isInFront();
        bool inArc(float Position1X, float Position1Y, float FOV, float Orientation, float Position2X, float Position2Y);

        LocationVector m_position;
        LocationVector m_lastMapUpdatePosition;
        LocationVector m_spawnLocation;

        /// Object properties.
        union
        {
            uint32* m_uint32Values;
            float* m_floatValues;
        };

        /// Number of properties
        uint16 m_valuesCount;

        /// List of object properties that need updating.
        UpdateMask m_updateMask;

        /// True if object was updated
        bool m_objectUpdated;

        /// Set of Objects in range.
        ///\todo that functionality should be moved into WorldServer.
        std::set<Object*> m_objectsInRange;
        std::set<Object*> m_inRangePlayers;
        std::set<Object*> m_oppFactsInRange;
        std::set<Object*> m_sameFactsInRange;

        int32 m_instanceId;

    public:

        bool m_loadedFromDB;

        // Spell currently casting
        Spell* m_currentSpell;
        Spell* GetCurrentSpell() { return m_currentSpell; }
        void SetCurrentSpell(Spell* cSpell) { m_currentSpell = cSpell; }

        // Andy's crap
        virtual Object* GetPlayerOwner();
        std::set<Spell*> m_pendingSpells;

        bool GetPoint(float angle, float rad, float & outx, float & outy, float & outz, bool sloppypath = false);
        bool GetRandomPoint(float rad, float & outx, float & outy, float & outz) { return GetPoint(RandomFloat(float(M_PI * 2)), rad, outx, outy, outz); }
        bool GetRandomPoint(float rad, LocationVector & out) { return GetRandomPoint(rad, out.x, out.y, out.z); }
};


#endif // OBJECT_H
