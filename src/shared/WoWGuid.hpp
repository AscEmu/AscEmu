/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Platform/SymbolVisibility.hpp"
#include "Debugging/Errors.hpp"

#include <cstdint>
#include <functional>

#define BitCount1(x) ((x) & 1)
#define BitCount2(x) ( BitCount1(x) + BitCount1((x)>>1) )
#define BitCount4(x) ( BitCount2(x) + BitCount2((x)>>2) )
#define BitCount8(x) ( BitCount4(x) + BitCount4((x)>>4) )

enum HIGHGUID_TYPE : uint32_t
{
    HIGHGUID_TYPE_PLAYER        = 0x00000000,
    HIGHGUID_TYPE_CORPSE        = 0x30000000,
    HIGHGUID_TYPE_ITEM          = 0x40000000,
    HIGHGUID_TYPE_CONTAINER     = 0x50000000,
    HIGHGUID_TYPE_DYNAMICOBJECT = 0x60000000,
    HIGHGUID_TYPE_WAYPOINT      = 0x10000000,
    HIGHGUID_TYPE_TRANSPORTER   = 0x1FC00000,
    HIGHGUID_TYPE_GAMEOBJECT    = 0xF1100000,
    HIGHGUID_TYPE_TRANSPORT     = 0xF1200000,
    HIGHGUID_TYPE_UNIT          = 0xF1300000,
    HIGHGUID_TYPE_PET           = 0xF1400000,
    HIGHGUID_TYPE_VEHICLE       = 0xF1500000,
    HIGHGUID_TYPE_AREATRIGGER   = 0xF1020000,
    HIGHGUID_TYPE_BATTLEGROUND  = 0x1F100000,
    HIGHGUID_TYPE_INSTANCE      = 0x1F400000,
    HIGHGUID_TYPE_GROUP         = 0x1F500000,
    HIGHGUID_TYPE_GUILD         = 0x1FF70000,
    //===============================================
    HIGHGUID_TYPE_MASK          = 0xFFF00000,
    LOWGUID_ENTRY_MASK          = 0x00FFFFFF,
};

enum class HighGuid : uint64_t
{
    Player          = 0x00000000,
    Corpse          = 0x30000000,
    Item            = 0x40000000,
    Container       = 0x50000000,
    DynamicObject   = 0x60000000,
    Waypoint        = 0x10000000,
    Transporter     = 0x1FC00000,
    GameObject      = 0xF1100000,
    Transport       = 0xF1200000,
    Unit            = 0xF1300000,
    Pet             = 0xF1400000,
    Vehicle         = 0xF1500000,
    AreaTrigger     = 0xF1020000,
    Battleground    = 0x1F100000,
    Instance        = 0x1F400000,
    Group           = 0x1F500000,
    Guild           = 0x1FF70000,
    HighGuidMask    = 0xFFF00000,
    LowGuidMask     = 0x00FFFFFF,
};

// Guid types of the 128 bit guids, used by clients from 6.0 on (the values are the same in 6.x and 7.x).
enum class HighGuid128 : uint8_t
{
    Null            = 0,
    Uniq            = 1,
    Player          = 2,
    Item            = 3,
    Transport       = 6,
    Creature        = 8,
    Vehicle         = 9,
    Pet             = 10,
    GameObject      = 11,
    DynamicObject   = 12,
    AreaTrigger     = 13,
    Corpse          = 14,
    Party           = 27,
    Guild           = 28,
    WowAccount      = 29,
    BNetAccount     = 30,
    Cast            = 47
};

// 128 bit guid of 6.x and 7.x clients.
// high part: type (6 bit) | realm (16 bit, 13 bit for map bound types) | map (13 bit) | entry (23 bit) | sub type (6 bit)
// low part:  server (24 bit) | counter (40 bit)
// The server keeps its 64 bit guids; WoWGuid::toGuid128() and WoWGuid::fromGuid128() translate at the packet boundary.
struct WoWGuid128
{
    uint64_t low = 0;
    uint64_t high = 0;

    constexpr WoWGuid128() noexcept = default;
    constexpr WoWGuid128(uint64_t _high, uint64_t _low) noexcept : low(_low), high(_high) {}

    // types without realm and map: accounts, groups
    static constexpr WoWGuid128 global(HighGuid128 type, uint64_t counter) noexcept
    {
        return { uint64_t(type) << 58, counter };
    }

    // types bound to a realm: players, items, guilds, transports
    static constexpr WoWGuid128 realmSpecific(HighGuid128 type, uint32_t realmId, uint64_t counter) noexcept
    {
        return { (uint64_t(type) << 58) | (uint64_t(realmId & 0xFFFF) << 42), counter };
    }

    // types bound to a map: creatures, pets, vehicles, gameobjects, dynamic objects, area triggers, corpses
    static constexpr WoWGuid128 mapSpecific(HighGuid128 type, uint32_t realmId, uint32_t mapId, uint32_t entry, uint64_t counter, uint8_t subType = 0, uint32_t serverId = 0) noexcept
    {
        return { (uint64_t(type) << 58) | (uint64_t(realmId & 0x1FFF) << 42) | (uint64_t(mapId & 0x1FFF) << 29) | (uint64_t(entry & 0x7FFFFF) << 6) | uint64_t(subType & 0x3F),
            (uint64_t(serverId & 0xFFFFFF) << 40) | (counter & UINT64_C(0xFFFFFFFFFF)) };
    }

    // a spell cast: bound to the map of the caster, the spell is the entry
    static constexpr WoWGuid128 cast(uint32_t realmId, uint32_t mapId, uint32_t spellId, uint64_t counter) noexcept
    {
        return mapSpecific(HighGuid128::Cast, realmId, mapId, spellId, counter);
    }

    constexpr bool isEmpty() const noexcept { return low == 0 && high == 0; }

    constexpr HighGuid128 getHighType() const noexcept { return static_cast<HighGuid128>((high >> 58) & 0x3F); }
    constexpr uint32_t getRealmId() const noexcept { return static_cast<uint32_t>((high >> 42) & 0x1FFF); }
    constexpr uint32_t getMapId() const noexcept { return static_cast<uint32_t>((high >> 29) & 0x1FFF); }
    constexpr uint32_t getEntry() const noexcept { return static_cast<uint32_t>((high >> 6) & 0x7FFFFF); }
    constexpr uint8_t getSubType() const noexcept { return static_cast<uint8_t>(high & 0x3F); }
    constexpr uint64_t getCounter() const noexcept { return low & UINT64_C(0xFFFFFFFFFF); }

    constexpr bool operator==(WoWGuid128 const& other) const noexcept { return low == other.low && high == other.high; }
    constexpr bool operator!=(WoWGuid128 const& other) const noexcept { return !(*this == other); }
};

class SERVER_DECL WoWGuid
{
public:
    WoWGuid() noexcept { clear(); }
    WoWGuid(uint64_t guid) noexcept { init(guid); }
    WoWGuid(WoWGuid const& other) noexcept { init(other._raw.value); }

    explicit WoWGuid(uint8_t mask) noexcept { init(mask); }
    explicit WoWGuid(uint8_t mask, const uint8_t* fields) noexcept { init(mask, fields); }

    WoWGuid(uint32_t id, uint32_t entry, uint32_t highType) noexcept
    {
        uint64_t raw = (uint64_t(highType) << 32)
                     | (uint64_t(entry)   << 24)
                     |  uint64_t(id);
        init(raw);
    }

    bool isEmpty() const noexcept { return _raw.value == 0; }
    explicit operator bool() const noexcept { return !isEmpty(); }

    uint8_t& operator[](uint32_t index)
    {
        ASSERT(index < sizeof(uint64_t));
        return _raw.byte[index];
    }

    uint8_t const& operator[](uint32_t index) const
    {
        ASSERT(index < sizeof(uint64_t));
        return _raw.byte[index];
    }

    operator uint64_t() const noexcept { return _raw.value; }

    WoWGuid& operator=(uint64_t guid) noexcept { init(guid); return *this; }
    WoWGuid& operator=(WoWGuid const& other) noexcept { init(other._raw.value); return *this; }

    // Raw lower 32 bits of the packed GUID. This is not necessarily the object counter.
    uint32_t getLowGuid() const noexcept
    {
        return static_cast<uint32_t>(_raw.value);
    }

    // Runtime counter stored in the low 24 bits.
    uint32_t getCounter() const noexcept
    {
        return static_cast<uint32_t>(_raw.value & UINT64_C(0x00FFFFFF));
    }

    // Template entry stored in bits 24..47 for entry-bearing GUID types.
    uint32_t getEntry() const noexcept
    {
        return static_cast<uint32_t>((_raw.value >> 24) & UINT64_C(0x00FFFFFF));
    }

    void clear() noexcept
    {
        _raw.value = 0;
        guidmask   = 0;
        m_fieldcount = 0;
        m_compiled = false;

        for (auto& field : m_guidfields)
            field = 0;
    }

    void init(uint64_t guid) noexcept
    {
        clear();
        _raw.value = guid;
        _compileByOld();
    }

    void init(uint8_t mask) noexcept
    {
        clear();
        guidmask = mask;
        if (!guidmask)
            _compileByNew();
    }

    void init(uint8_t mask, const uint8_t* fields) noexcept
    {
        clear();
        guidmask = mask;

        const uint8_t n = BitCount8(guidmask);
        if (!n)
        {
            _compileByNew();
            return;
        }

        for (uint8_t i = 0; i < n; ++i)
            m_guidfields[i] = fields[i];

        m_fieldcount = n;
        _compileByNew();
    }

    void init(WoWGuid const& guid) noexcept { init(guid._raw.value); }

    // Raw lower 32 bits of a packed GUID.
    static uint32_t getLowGuidFromRaw(uint64_t guid) noexcept
    {
        return static_cast<uint32_t>(guid);
    }

    // Raw upper 32 bits of the packed GUID.
    uint32_t getHighGuid() const noexcept
    {
        return static_cast<uint32_t>(_raw.value >> 32);
    }

    // Object type encoded in the masked upper GUID bits.
    HighGuid getHighType() const noexcept
    {
        return static_cast<HighGuid>(getHighGuid() & HIGHGUID_TYPE_MASK);
    }

    // Raw upper 32 bits of a packed GUID.
    static uint32_t getHighGuidFromRaw(uint64_t guid) noexcept
    {
        return static_cast<uint32_t>(guid >> 32);
    }

    // Object type encoded in the masked upper GUID bits of a packed GUID.
    static HighGuid getHighTypeFromRaw(uint64_t guid) noexcept
    {
        return static_cast<HighGuid>(getHighGuidFromRaw(guid) & HIGHGUID_TYPE_MASK);
    }

    static uint64_t createItemGuid(uint32_t lowGuid) noexcept
    {
        return (uint64_t(HIGHGUID_TYPE_ITEM) << 32) | uint64_t(lowGuid);
    }

    uint64_t getRawGuid() const noexcept { return _raw.value; }

    // This guid for a 6.x or 7.x client. Map bound types carry the map the receiving player is on.
    WoWGuid128 toGuid128(uint32_t realmId, uint32_t mapId) const noexcept
    {
        if (isEmpty())
            return {};

        // the type mask keeps the upper 12 bits, the types are compared the same way
        constexpr auto masked = [](uint32_t type) constexpr { return type & static_cast<uint32_t>(HIGHGUID_TYPE_MASK); };

        switch (masked(getHighGuid()))
        {
            case masked(HIGHGUID_TYPE_PLAYER):          return WoWGuid128::realmSpecific(HighGuid128::Player, realmId, getLowGuid());
            case masked(HIGHGUID_TYPE_ITEM):
            case masked(HIGHGUID_TYPE_CONTAINER):       return WoWGuid128::realmSpecific(HighGuid128::Item, realmId, getLowGuid());
            case masked(HIGHGUID_TYPE_GUILD):           return WoWGuid128::realmSpecific(HighGuid128::Guild, realmId, getLowGuid());
            case masked(HIGHGUID_TYPE_TRANSPORTER):
            case masked(HIGHGUID_TYPE_TRANSPORT):       return WoWGuid128::realmSpecific(HighGuid128::Transport, realmId, getLowGuid());
            case masked(HIGHGUID_TYPE_GROUP):           return WoWGuid128::global(HighGuid128::Party, getLowGuid());
            case masked(HIGHGUID_TYPE_UNIT):            return WoWGuid128::mapSpecific(HighGuid128::Creature, realmId, mapId, getEntry(), getCounter());
            case masked(HIGHGUID_TYPE_VEHICLE):         return WoWGuid128::mapSpecific(HighGuid128::Vehicle, realmId, mapId, getEntry(), getCounter());
            case masked(HIGHGUID_TYPE_PET):             return WoWGuid128::mapSpecific(HighGuid128::Pet, realmId, mapId, getEntry(), getCounter());
            case masked(HIGHGUID_TYPE_GAMEOBJECT):      return WoWGuid128::mapSpecific(HighGuid128::GameObject, realmId, mapId, getEntry(), getCounter());
            case masked(HIGHGUID_TYPE_AREATRIGGER):     return WoWGuid128::mapSpecific(HighGuid128::AreaTrigger, realmId, mapId, getEntry(), getCounter());
            case masked(HIGHGUID_TYPE_DYNAMICOBJECT):   return WoWGuid128::mapSpecific(HighGuid128::DynamicObject, realmId, mapId, 0, getLowGuid());
            case masked(HIGHGUID_TYPE_CORPSE):          return WoWGuid128::mapSpecific(HighGuid128::Corpse, realmId, mapId, 0, getLowGuid());
            default:
                break;
        }

        // no counterpart in the client (waypoints, instances, battlegrounds)
        return {};
    }

    // The server guid for a guid a 6.x or 7.x client sent. Item guids come back as items, a container has
    // to be looked up by its counter.
    static WoWGuid fromGuid128(WoWGuid128 const& guid) noexcept
    {
        const uint32_t counter = static_cast<uint32_t>(guid.getCounter());

        switch (guid.getHighType())
        {
            case HighGuid128::Player:           return WoWGuid(static_cast<uint64_t>(counter));
            case HighGuid128::Item:             return WoWGuid((uint64_t(HIGHGUID_TYPE_ITEM) << 32) | counter);
            case HighGuid128::Guild:            return WoWGuid((uint64_t(HIGHGUID_TYPE_GUILD) << 32) | counter);
            case HighGuid128::Transport:        return WoWGuid((uint64_t(HIGHGUID_TYPE_TRANSPORTER) << 32) | counter);
            case HighGuid128::Party:            return WoWGuid((uint64_t(HIGHGUID_TYPE_GROUP) << 32) | counter);
            case HighGuid128::Creature:         return WoWGuid(counter & LOWGUID_ENTRY_MASK, guid.getEntry(), HIGHGUID_TYPE_UNIT);
            case HighGuid128::Vehicle:          return WoWGuid(counter & LOWGUID_ENTRY_MASK, guid.getEntry(), HIGHGUID_TYPE_VEHICLE);
            case HighGuid128::Pet:              return WoWGuid(counter & LOWGUID_ENTRY_MASK, guid.getEntry(), HIGHGUID_TYPE_PET);
            case HighGuid128::GameObject:       return WoWGuid(counter & LOWGUID_ENTRY_MASK, guid.getEntry(), HIGHGUID_TYPE_GAMEOBJECT);
            case HighGuid128::AreaTrigger:      return WoWGuid(counter & LOWGUID_ENTRY_MASK, guid.getEntry(), HIGHGUID_TYPE_AREATRIGGER);
            case HighGuid128::DynamicObject:    return WoWGuid((uint64_t(HIGHGUID_TYPE_DYNAMICOBJECT) << 32) | counter);
            case HighGuid128::Corpse:           return WoWGuid((uint64_t(HIGHGUID_TYPE_CORPSE) << 32) | counter);
            default:
                break;
        }

        return WoWGuid();
    }

    const uint8_t* getNewGuid() const noexcept { return m_guidfields; }
    uint8_t getNewGuidLen() const noexcept { return BitCount8(guidmask); }
    uint8_t getNewGuidMask() const noexcept { return guidmask; }

    // helpers
    bool     operator!() const noexcept { return _raw.value == 0; }

    bool     operator==(int v) const noexcept { return _raw.value == static_cast<uint64_t>(v); }
    bool     operator!=(int v) const noexcept { return _raw.value != static_cast<uint64_t>(v); }

    bool     operator==(uint64_t v) const noexcept { return _raw.value == v; }
    bool     operator!=(uint64_t v) const noexcept { return _raw.value != v; }

    uint64_t operator&(uint64_t v) const noexcept { return _raw.value & v; }
    uint64_t operator&(unsigned int v) const noexcept { return _raw.value & uint64_t(v); }

    bool     operator==(WoWGuid const& other) const noexcept { return _raw.value == other._raw.value; }
    bool     operator!=(WoWGuid const& other) const noexcept { return _raw.value != other._raw.value; }
    bool     operator<(WoWGuid const& other) const noexcept { return _raw.value < other._raw.value; }

    // symmetric operators
    friend bool operator==(int v, WoWGuid const& a) noexcept { return a == v; }
    friend bool operator!=(int v, WoWGuid const& a) noexcept { return a != v; }

    friend bool operator==(uint64_t v, WoWGuid const& a) noexcept { return a == v; }
    friend bool operator!=(uint64_t v, WoWGuid const& a) noexcept { return a != v; }

    void appendField(uint8_t field)
    {
        ASSERT(!m_compiled);
        ASSERT(m_fieldcount < BitCount8(guidmask));
        m_guidfields[m_fieldcount++] = field;
        if (m_fieldcount == BitCount8(guidmask))
            _compileByNew();
    }

    // helpers for type checking
    bool isPlayer()       const noexcept { return getHighType() == HighGuid::Player; }
    bool isCorpse()       const noexcept { return getHighType() == HighGuid::Corpse; }
    bool isItem()         const noexcept { return getHighType() == HighGuid::Item; }
    bool isContainer()    const noexcept { return getHighType() == HighGuid::Container; }
    bool isDynamicObject()const noexcept { return getHighType() == HighGuid::DynamicObject; }
    bool isWaypoint()     const noexcept { return getHighType() == HighGuid::Waypoint; }
    bool isTransporter()  const noexcept { return getHighType() == HighGuid::Transporter; }
    bool isGameObject()   const noexcept { return getHighType() == HighGuid::GameObject; }
    bool isTransport()    const noexcept { return getHighType() == HighGuid::Transport; }
    bool isUnit()         const noexcept { return getHighType() == HighGuid::Unit; }
    bool isPet()          const noexcept { return getHighType() == HighGuid::Pet; }
    bool isVehicle()      const noexcept { return getHighType() == HighGuid::Vehicle; }
    bool isAreaTrigger()  const noexcept { return getHighType() == HighGuid::AreaTrigger; }
    bool isBattleground() const noexcept { return getHighType() == HighGuid::Battleground; }
    bool isInstance()     const noexcept { return getHighType() == HighGuid::Instance; }
    bool isGroup()        const noexcept { return getHighType() == HighGuid::Group; }
    bool isGuild()        const noexcept { return getHighType() == HighGuid::Guild; }

private:
    // raw data union
    union Raw {
        uint64_t value;
        uint8_t  byte[8];
    } _raw{};

    uint8_t guidmask{};
    uint8_t m_guidfields[8]{};
    uint8_t m_fieldcount{};
    bool    m_compiled{};

    void _compileByOld() noexcept
    {
        m_fieldcount = 0;
        guidmask = 0;
        for (uint8_t x = 0; x < 8; ++x)
        {
            const uint8_t p = _raw.byte[x];
            if (p)
            {
                m_guidfields[m_fieldcount++] = p;
                guidmask |= uint8_t(1u << x);
            }
        }
        m_compiled = true;
    }

    void _compileByNew() noexcept
    {
        _raw.value = 0;
        int j = 0;
        if (guidmask & 0x01)  { _raw.value |= (uint64_t(m_guidfields[j])      ); ++j; }
        if (guidmask & 0x02)  { _raw.value |= (uint64_t(m_guidfields[j]) <<  8); ++j; }
        if (guidmask & 0x04)  { _raw.value |= (uint64_t(m_guidfields[j]) << 16); ++j; }
        if (guidmask & 0x08)  { _raw.value |= (uint64_t(m_guidfields[j]) << 24); ++j; }
        if (guidmask & 0x10)  { _raw.value |= (uint64_t(m_guidfields[j]) << 32); ++j; }
        if (guidmask & 0x20)  { _raw.value |= (uint64_t(m_guidfields[j]) << 40); ++j; }
        if (guidmask & 0x40)  { _raw.value |= (uint64_t(m_guidfields[j]) << 48); ++j; }
        if (guidmask & 0x80)  { _raw.value |= (uint64_t(m_guidfields[j]) << 56); ++j; }
        m_compiled = true;
    }
};

namespace std
{
    template <>
    struct hash<WoWGuid>
    {
        std::size_t operator()(const WoWGuid& guid) const noexcept
        {
            return std::hash<uint64_t>{}(guid.getRawGuid());
        }
    };
}
