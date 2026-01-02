/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Debugging/Errors.h"

#include <cstdint>

#define BitCount1(x) ((x) & 1)
#define BitCount2(x) ( BitCount1(x) + BitCount1((x)>>1) )
#define BitCount4(x) ( BitCount2(x) + BitCount2((x)>>2) )
#define BitCount8(x) ( BitCount4(x) + BitCount4((x)>>4) )

enum HIGHGUID_TYPE
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

    uint32_t getCounter() const noexcept
    {
        return uint32_t(_raw.value & UINT64_C(0x00000000FFFFFFFF));
    }

    void clear() noexcept
    {
        _raw.value = 0;
        guidmask   = 0;
        m_fieldcount = 0;
        m_compiled = false;

        *reinterpret_cast<uint32_t*>(m_guidfields) = 0;
        *reinterpret_cast<uint32_t*>(&m_guidfields[4]) = 0;
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

    uint32_t getGuidLow() const noexcept { return static_cast<uint32_t>(_raw.value); }

    uint32_t getGuidLowPart() const noexcept
    {
        const uint32_t low = *(reinterpret_cast<const uint32_t*>(&_raw.value));
        return low & 0x00FFFFFF;
    }

    static uint32_t getGuidLowPartFromUInt64(uint64_t guid) noexcept
    {
        return *reinterpret_cast<const uint32_t*>(&guid);
    }

    uint32_t getGuidHigh() const noexcept { return static_cast<uint32_t>(_raw.value >> 32); }

    uint32_t getGuidHighPart() const noexcept
    {
        const uint32_t high = *(reinterpret_cast<const uint32_t*>(&_raw.value) + 1);
        return high & 0xFFF00000;
    }

    static uint32_t getGuidHighPartFromUInt64(uint64_t guid) noexcept
    {
        return *(reinterpret_cast<const uint32_t*>(&guid) + 1);
    }

    HighGuid getHigh() const noexcept { return static_cast<HighGuid>(getGuidHighPart()); }

    static uint64_t createItemGuid(uint32_t lowguid) noexcept
    {
        uint64_t v = 0;
        uint32_t* part = reinterpret_cast<uint32_t*>(&v);
        part[0] = lowguid;
        part[1] = HIGHGUID_TYPE_ITEM;
        return v;
    }

    uint64_t getRawGuid() const noexcept { return _raw.value; }

    const uint8_t* getNewGuid() const noexcept { return m_guidfields; }
    uint8_t getNewGuidLen() const noexcept { return BitCount8(guidmask); }
    uint8_t getNewGuidMask() const noexcept { return guidmask; }

    // helpers
    bool     operator!() const noexcept { return _raw.value == 0; }
    bool     operator==(uint64_t v) const noexcept { return _raw.value == v; }
    bool     operator!=(uint64_t v) const noexcept { return _raw.value != v; }
    uint64_t operator&(uint64_t v) const noexcept  { return _raw.value & v; }
    uint64_t operator&(unsigned int v) const noexcept { return _raw.value & uint64_t(v); }

    bool     operator==(WoWGuid const& other) const noexcept { return _raw.value == other._raw.value; }
    bool     operator!=(WoWGuid const& other) const noexcept { return _raw.value != other._raw.value; }

    // symmetric operators
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
    bool isPlayer()       const noexcept { return getHigh() == HighGuid::Player; }
    bool isCorpse()       const noexcept { return getHigh() == HighGuid::Corpse; }
    bool isItem()         const noexcept { return getHigh() == HighGuid::Item; }
    bool isContainer()    const noexcept { return getHigh() == HighGuid::Container; }
    bool isDynamicObject()const noexcept { return getHigh() == HighGuid::DynamicObject; }
    bool isWaypoint()     const noexcept { return getHigh() == HighGuid::Waypoint; }
    bool isTransporter()  const noexcept { return getHigh() == HighGuid::Transporter; }
    bool isGameObject()   const noexcept { return getHigh() == HighGuid::GameObject; }
    bool isTransport()    const noexcept { return getHigh() == HighGuid::Transport; }
    bool isUnit()         const noexcept { return getHigh() == HighGuid::Unit; }
    bool isPet()          const noexcept { return getHigh() == HighGuid::Pet; }
    bool isVehicle()      const noexcept { return getHigh() == HighGuid::Vehicle; }
    bool isAreaTrigger()  const noexcept { return getHigh() == HighGuid::AreaTrigger; }
    bool isBattleground() const noexcept { return getHigh() == HighGuid::Battleground; }
    bool isInstance()     const noexcept { return getHigh() == HighGuid::Instance; }
    bool isGroup()        const noexcept { return getHigh() == HighGuid::Group; }
    bool isGuild()        const noexcept { return getHigh() == HighGuid::Guild; }

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
