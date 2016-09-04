/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _NEW_WOW_GUID_HPP
#define _NEW_WOW_GUID_HPP

#include "Common.h"
#include "NewByteBuffer.hpp"
#include "../world/Server/WUtil.h"

enum TypeId
{
    TYPEID_OBJECT           = 0,
    TYPEID_ITEM             = 1,
    TYPEID_CONTAINER        = 2,
    TYPEID_UNIT             = 3,
    TYPEID_PLAYER           = 4,
    TYPEID_GAMEOBJECT       = 5,
    TYPEID_DYNAMICOBJECT    = 6,
    TYPEID_CORPSE           = 7,
    TYPEID_AIGROUP          = 8,
    TYPEID_AREATRIGGER      = 9,
    MAX_TYPE_ID
};

enum TypeMask
{
    TYPEMASK_OBJECT         = 0x0001,
    TYPEMASK_ITEM           = 0x0002,
    TYPEMASK_CONTAINER      = 0x0004,
    TYPEMASK_UNIT           = 0x0008,
    TYPEMASK_PLAYER         = 0x0010,
    TYPEMASK_GAMEOBJECT     = 0x0020,
    TYPEMASK_DYNAMICOBJECT  = 0x0040,
    TYPEMASK_CORPSE         = 0x0080,
    TYPEMASK_AIGROUP        = 0x0100,
    TYPEMASK_AREATRIGGER    = 0x0200,

    TYPEMASK_CREATURE_GO                = TYPEMASK_UNIT | TYPEMASK_GAMEOBJECT,
    TYPEMASK_CREATURE_GO_ITEM           = TYPEMASK_UNIT | TYPEMASK_GAMEOBJECT | TYPEMASK_ITEM,
    TYPEMASK_CREATURE_GO_PLAYER_ITEM    = TYPEMASK_UNIT | TYPEMASK_GAMEOBJECT | TYPEMASK_ITEM | TYPEMASK_PLAYER,

    TYPEMASK_WORLDOBJECT                = TYPEMASK_UNIT | TYPEMASK_PLAYER | TYPEMASK_GAMEOBJECT | TYPEMASK_DYNAMICOBJECT | TYPEMASK_CORPSE
};

enum HighGuidType
{
    HIGHGUID_TYPE_PLAYER            = 0x00000000,
    HIGHGUID_TYPE_CORPSE            = 0x30000000,
    HIGHGUID_TYPE_ITEM              = 0x40000000,
    HIGHGUID_TYPE_CONTAINER         = 0x50000000,
    HIGHGUID_TYPE_DYNAMICOBJECT     = 0x60000000,
    HIGHGUID_TYPE_WAYPOINT          = 0x10000000,
    HIGHGUID_TYPE_TRANSPORTER       = 0xF1200000,
    HIGHGUID_TYPE_GAMEOBJECT        = 0xF1100000,
    HIGHGUID_TYPE_UNIT              = 0xF1300000,
    HIGHGUID_TYPE_PET               = 0xF1400000,
    HIGHGUID_TYPE_VEHICLE           = 0xF1500000,
    HIGHGUID_TYPE_GROUP             = 0x1F500000,
    HIGHGUID_TYPE_GUILD             = 0x1FF70000,
    HIGHGUID_TYPE_MO_TRANSPORT      = 0x1FC00000,
    HIGHGUID_TYPE_BATTLEGROUND      = 0x1F100000,
    HIGHGUID_TYPE_AREATRIGGER       = 0xF1020000,
    HIGHGUID_TYPE_INSTANCE          = 0x1F400000
};

class NewWoWGuid;
class PackedWoWGuid;

struct PackedWoWGuidReader
{
    explicit PackedWoWGuidReader(NewWoWGuid& guid) : m_guidPtr(&guid) {}
    NewWoWGuid* m_guidPtr;
};


#define GUID_BYTES sizeof(uint64)


class SERVER_DECL NewWoWGuid
{
    public:

        NewWoWGuid() : m_guid(0) {}
        explicit NewWoWGuid(uint64 guid) : m_guid(guid) {}
        NewWoWGuid(HighGuidType hi, uint32 entry, uint32 counter) : m_guid(counter ? uint64(counter) | (uint64(entry) << 32) | (uint64(hi) << (IsLargeHigh(hi) ? 48 : 52)) : 0) {}
        NewWoWGuid(HighGuidType hi, uint32 counter) : m_guid(counter ? uint64(counter) | (uint64(hi) << (IsLargeHigh(hi) ? 48 : 52)) : 0) {}

        operator uint64() const { return m_guid; }

    private:

        explicit NewWoWGuid(uint32 const&);
        NewWoWGuid(HighGuidType, uint32, uint64 counter);
        NewWoWGuid(HighGuidType, uint64 counter);

    public:

        PackedWoWGuidReader ReadAsPacked() { return PackedWoWGuidReader(*this); }

        void Set(uint64 guid) { m_guid = guid; }
        void Clear() { m_guid = 0; }

        PackedWoWGuid WriteAsPacked() const;

    public:

        uint64   GetRawValue() const { return m_guid; }
        HighGuidType GetHigh() const
        {
            HighGuidType high = HighGuidType((m_guid >> 48) & 0xFFFF);
            return HighGuidType(IsLargeHigh(high) ? high : (m_guid >> 52) & 0xFFF);
        }
        uint32 GetEntry() const { return HasEntry() ? uint32((m_guid >> 32) & uint64(0x000000000000FFFF)) : 0; }
        uint32 GetCounter() const { return HasEntry() ? uint32(m_guid & uint64(0x00000000FFFFFFFF)) : uint32(m_guid & uint64(0x00000000FFFFFFFF)); }

        static uint32 GetMaxCounter(HighGuidType high) { return HasEntry(high) ? uint32(0xFFFFFFFF) : uint32(0xFFFFFFFF); }

        uint32 GetMaxCounter() const { return GetMaxCounter(GetHigh()); }

        bool IsEmpty()             const { return m_guid == 0; }
        bool IsCreature()          const { return GetHigh() == HIGHGUID_TYPE_UNIT; }
        bool IsPet()               const { return GetHigh() == HIGHGUID_TYPE_PET; }
        bool IsVehicle()           const { return GetHigh() == HIGHGUID_TYPE_VEHICLE; }
        bool IsCreatureOrPet()     const { return IsCreature() || IsPet(); }
        bool IsCreatureOrVehicle() const { return IsCreature() || IsVehicle(); }
        bool IsAnyTypeCreature()   const { return IsCreature() || IsPet() || IsVehicle(); }
        bool IsPlayer()            const { return !IsEmpty() && GetHigh() == HIGHGUID_TYPE_PLAYER; }
        bool IsUnit()              const { return IsAnyTypeCreature() || IsPlayer(); }
        bool IsItem()              const { return GetHigh() == HIGHGUID_TYPE_ITEM; }
        bool IsGameObject()        const { return GetHigh() == HIGHGUID_TYPE_GAMEOBJECT; }
        bool IsDynamicObject()     const { return GetHigh() == HIGHGUID_TYPE_DYNAMICOBJECT; }
        bool IsCorpse()            const { return GetHigh() == HIGHGUID_TYPE_CORPSE; }
        bool IsTransport()         const { return GetHigh() == HIGHGUID_TYPE_TRANSPORTER; }
        bool IsMOTransport()       const { return GetHigh() == HIGHGUID_TYPE_MO_TRANSPORT; }
        bool IsInstance()          const { return GetHigh() == HIGHGUID_TYPE_INSTANCE; }
        bool IsGroup()             const { return GetHigh() == HIGHGUID_TYPE_GROUP; }
        bool IsBattleGround()      const { return GetHigh() == HIGHGUID_TYPE_BATTLEGROUND; }
        bool IsGuild()             const { return GetHigh() == HIGHGUID_TYPE_GUILD; }

        static TypeId GetTypeId(HighGuidType high)
        {
            switch (high)
            {
                case HIGHGUID_TYPE_ITEM:
                    return TYPEID_ITEM;
                case HIGHGUID_TYPE_UNIT:
                    return TYPEID_UNIT;
                case HIGHGUID_TYPE_PET:
                    return TYPEID_UNIT;
                case HIGHGUID_TYPE_PLAYER:
                    return TYPEID_PLAYER;
                case HIGHGUID_TYPE_GAMEOBJECT:
                    return TYPEID_GAMEOBJECT;
                case HIGHGUID_TYPE_DYNAMICOBJECT:
                    return TYPEID_DYNAMICOBJECT;
                case HIGHGUID_TYPE_CORPSE:
                    return TYPEID_CORPSE;
                case HIGHGUID_TYPE_MO_TRANSPORT:
                    return TYPEID_GAMEOBJECT;
                case HIGHGUID_TYPE_VEHICLE:
                    return TYPEID_UNIT;
                case HIGHGUID_TYPE_AREATRIGGER:
                    return TYPEID_AREATRIGGER;
                case HIGHGUID_TYPE_INSTANCE:
                case HIGHGUID_TYPE_GROUP:
                default:
                    return TYPEID_OBJECT;
            }
        }

        TypeId GetTypeId() const { return GetTypeId(GetHigh()); }

        bool operator!() const { return IsEmpty(); }
        bool operator== (NewWoWGuid const& guid) const { return GetRawValue() == guid.GetRawValue(); }
        bool operator!= (NewWoWGuid const& guid) const { return GetRawValue() != guid.GetRawValue(); }
        bool operator< (NewWoWGuid const& guid) const { return GetRawValue() < guid.GetRawValue(); }

        uint8& operator[] (uint8 index)
        {
            ARCEMU_ASSERT(index < GUID_BYTES);
            return m_guidBytes[index];
        }

        uint8 const& operator[] (uint8 index) const
        {
            ARCEMU_ASSERT(index < GUID_BYTES);
            return m_guidBytes[index];
        }

    public:

        static char const* GetHighGuidTypeName(HighGuidType high);
        char const* GetHighGuidTypeName() const { return !IsEmpty() ? GetHighGuidTypeName(GetHigh()) : "None"; }
        std::string GetObjectString() const;

    private:

        static bool HasEntry(HighGuidType high)
        {
            switch (high)
            {
                case HIGHGUID_TYPE_ITEM:
                case HIGHGUID_TYPE_PLAYER:
                case HIGHGUID_TYPE_DYNAMICOBJECT:
                case HIGHGUID_TYPE_CORPSE:
                case HIGHGUID_TYPE_MO_TRANSPORT:
                case HIGHGUID_TYPE_INSTANCE:
                case HIGHGUID_TYPE_GROUP:
                    return false;
                case HIGHGUID_TYPE_GAMEOBJECT:
                case HIGHGUID_TYPE_TRANSPORTER:
                case HIGHGUID_TYPE_UNIT:
                case HIGHGUID_TYPE_PET:
                case HIGHGUID_TYPE_VEHICLE:
                case HIGHGUID_TYPE_BATTLEGROUND:
                default:
                    return true;
            }
        }

        bool HasEntry() const { return HasEntry(GetHigh()); }

        static bool IsLargeHigh(HighGuidType high)
        {
            if (high == HIGHGUID_TYPE_GUILD)
                return true;
            else
                return false;
        }

        bool IsLargeHigh() const { return IsLargeHigh(GetHigh()); }

    private:

        union
        {
            uint64 m_guid;
            uint8 m_guidBytes[GUID_BYTES];
        };
};


typedef std::set<NewWoWGuid> GuidSet;
typedef std::list<NewWoWGuid> GuidList;
typedef std::vector<NewWoWGuid> GuidVector;


#define MIN_BUFFER_SIZE 9


class PackedWoWGuid
{
    friend NewByteBuffer& operator<< (NewByteBuffer& buf, PackedWoWGuid const& guid);

    public:

        explicit PackedWoWGuid() : m_packedGuid(MIN_BUFFER_SIZE) { m_packedGuid.appendPackGUID(0); }
        explicit PackedWoWGuid(uint64 const& guid) : m_packedGuid(MIN_BUFFER_SIZE) { m_packedGuid.appendPackGUID(guid); }
        explicit PackedWoWGuid(NewWoWGuid const& guid) : m_packedGuid(MIN_BUFFER_SIZE) { m_packedGuid.appendPackGUID(guid.GetRawValue()); }

    public:

        void Set(uint64 const& guid) { m_packedGuid.wpos(0); m_packedGuid.appendPackGUID(guid); }
        void Set(NewWoWGuid const& guid) { m_packedGuid.wpos(0); m_packedGuid.appendPackGUID(guid.GetRawValue()); }

    public:

        size_t size() const { return m_packedGuid.size(); }

    private:

        NewByteBuffer m_packedGuid;
};


template<HighGuidType high>
class NewWoWGuidGenerator
{
    public:

        explicit NewWoWGuidGenerator(uint32 start = 1) : m_nextGuid(start) {}

    public:

        void Set(uint32 val) { m_nextGuid = val; }
        uint32 Generate();

    public:

        uint32 GetNextAfterMaxUsed() const { return m_nextGuid; }

    private:

        uint32 m_nextGuid;
};

NewByteBuffer& operator<< (NewByteBuffer& buf, NewWoWGuid const& guid);
NewByteBuffer& operator >> (NewByteBuffer& buf, NewWoWGuid&       guid);

NewByteBuffer& operator<< (NewByteBuffer& buf, PackedWoWGuid const& guid);
NewByteBuffer& operator >> (NewByteBuffer& buf, PackedWoWGuidReader const& guid);

inline PackedWoWGuid NewWoWGuid::WriteAsPacked() const { return PackedWoWGuid(*this); }

namespace std {
    template<>
    class hash<NewWoWGuid>
    {
    public:

        size_t operator()(NewWoWGuid const& key) const
        {
            return hash<uint64>()(key.GetRawValue());
        }
    };
}

#define READ_GUIDMASK(T1, T2) template <T1> \
    void NewByteBuffer::ReadGuidMask(NewWoWGuid& guid) \
    { \
        uint8 maskArr[] = { T2 }; \
        for (uint8 i = 0; i < countof(maskArr); ++i) \
            guid[maskArr[i]] = ReadBit(); \
    }

    READ_GUIDMASK(DEF_BITS_1, DEF_BIT_VALS_1)
    READ_GUIDMASK(DEF_BITS_2, DEF_BIT_VALS_2)
    READ_GUIDMASK(DEF_BITS_3, DEF_BIT_VALS_3)
    READ_GUIDMASK(DEF_BITS_4, DEF_BIT_VALS_4)
    READ_GUIDMASK(DEF_BITS_5, DEF_BIT_VALS_5)
    READ_GUIDMASK(DEF_BITS_6, DEF_BIT_VALS_6)
    READ_GUIDMASK(DEF_BITS_7, DEF_BIT_VALS_7)
    READ_GUIDMASK(DEF_BITS_8, DEF_BIT_VALS_8)


#define WRITE_GUIDMASK(T1, T2) template <T1> \
    void NewByteBuffer::WriteGuidMask(NewWoWGuid guid) \
    { \
        uint8 maskArr[] = { T2 }; \
        for (uint8 i = 0; i < countof(maskArr); ++i) \
            WriteBit(guid[maskArr[i]]); \
    }

    WRITE_GUIDMASK(DEF_BITS_1, DEF_BIT_VALS_1)
    WRITE_GUIDMASK(DEF_BITS_2, DEF_BIT_VALS_2)
    WRITE_GUIDMASK(DEF_BITS_3, DEF_BIT_VALS_3)
    WRITE_GUIDMASK(DEF_BITS_4, DEF_BIT_VALS_4)
    WRITE_GUIDMASK(DEF_BITS_5, DEF_BIT_VALS_5)
    WRITE_GUIDMASK(DEF_BITS_6, DEF_BIT_VALS_6)
    WRITE_GUIDMASK(DEF_BITS_7, DEF_BIT_VALS_7)
    WRITE_GUIDMASK(DEF_BITS_8, DEF_BIT_VALS_8)


#define READ_GUIDBYTES(T1, T2) template <T1> \
    void NewByteBuffer::ReadGuidBytes(NewWoWGuid& guid) \
    { \
        uint8 maskArr[] = { T2 }; \
        for (uint8 i = 0; i < countof(maskArr); ++i) \
            if (guid[maskArr[i]] != 0) \
                guid[maskArr[i]] ^= read<uint8>(); \
    }

    READ_GUIDBYTES(DEF_BITS_1, DEF_BIT_VALS_1)
    READ_GUIDBYTES(DEF_BITS_2, DEF_BIT_VALS_2)
    READ_GUIDBYTES(DEF_BITS_3, DEF_BIT_VALS_3)
    READ_GUIDBYTES(DEF_BITS_4, DEF_BIT_VALS_4)
    READ_GUIDBYTES(DEF_BITS_5, DEF_BIT_VALS_5)
    READ_GUIDBYTES(DEF_BITS_6, DEF_BIT_VALS_6)
    READ_GUIDBYTES(DEF_BITS_7, DEF_BIT_VALS_7)
    READ_GUIDBYTES(DEF_BITS_8, DEF_BIT_VALS_8)


#define WRITE_GUIDBYTES(T1, T2) template <T1> \
    void NewByteBuffer::WriteGuidBytes(NewWoWGuid guid) \
    { \
        uint8 maskArr[] = { T2 }; \
        for (uint8 i = 0; i < countof(maskArr); ++i) \
            if (guid[maskArr[i]] != 0) \
                (*this) << uint8(guid[maskArr[i]] ^ 1); \
    }

    WRITE_GUIDBYTES(DEF_BITS_1, DEF_BIT_VALS_1)
    WRITE_GUIDBYTES(DEF_BITS_2, DEF_BIT_VALS_2)
    WRITE_GUIDBYTES(DEF_BITS_3, DEF_BIT_VALS_3)
    WRITE_GUIDBYTES(DEF_BITS_4, DEF_BIT_VALS_4)
    WRITE_GUIDBYTES(DEF_BITS_5, DEF_BIT_VALS_5)
    WRITE_GUIDBYTES(DEF_BITS_6, DEF_BIT_VALS_6)
    WRITE_GUIDBYTES(DEF_BITS_7, DEF_BIT_VALS_7)
    WRITE_GUIDBYTES(DEF_BITS_8, DEF_BIT_VALS_8)

#endif  //_NEW_WOW_GUID_HPP
