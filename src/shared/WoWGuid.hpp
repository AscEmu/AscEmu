/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Platform/SymbolVisibility.hpp"
#include "Debugging/Errors.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

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

enum class ModernHighGuid : uint8_t
{
    Null             = 0,
    Uniq             = 1,
    Player           = 2,
    Item             = 3,
    WorldTransaction = 4,
    StaticDoor       = 5,
    Transport        = 6,
    Conversation     = 7,
    Creature         = 8,
    Vehicle          = 9,
    Pet              = 10,
    GameObject       = 11,
    DynamicObject    = 12,
    AreaTrigger      = 13,
    Corpse           = 14,
    LootObject       = 15,
    SceneObject      = 16,
    Scenario         = 17,
    AIGroup          = 18,
    DynamicDoor      = 19,
    ClientActor      = 20,
    Vignette         = 21,
    CallForHelp      = 22,
    AIResource       = 23,
    AILock           = 24,
    AILockTicket     = 25,
    ChatChannel      = 26,
    Party            = 27,
    Guild            = 28,
    WowAccount       = 29,
    BNetAccount      = 30,
    GMTask           = 31,
    MobileSession    = 32,
    RaidGroup        = 33,
    Spell            = 34,
    Mail             = 35,
    WebObj           = 36,
    LFGObject        = 37,
    LFGList          = 38,
    UserRouter       = 39,
    PVPQueueGroup    = 40,
    UserClient       = 41,
    PetBattle        = 42,
    UniqUserClient   = 43,
    BattlePet        = 44,
    CommerceObj      = 45,
    ClientSession    = 46,
    Cast             = 47,
    ClientConnection = 48,
    ClubFinder       = 49,
    ToolsClient      = 50,
    WorldLayer       = 51,
    ArenaTeam        = 52,
    LMMParty         = 53,
    LMMLobby         = 54,
    Housing          = 55,
    MeshObject       = 56,
    Entity           = 57
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
    WoWGuid(WoWGuid const& other) noexcept = default;

    explicit WoWGuid(uint8_t mask) noexcept { init(mask); }
    explicit WoWGuid(uint8_t mask, const uint8_t* fields) noexcept { init(mask, fields); }

    WoWGuid(uint32_t id, uint32_t entry, uint32_t highType) noexcept
    {
        uint64_t raw = (uint64_t(highType) << 32)
                     | (uint64_t(entry)   << 24)
                     |  uint64_t(id);
        init(raw);
    }

    bool isEmpty() const noexcept { return _raw.value == 0 && _modernHigh == 0; }
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
    WoWGuid& operator=(WoWGuid const& other) noexcept = default;

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
        _modernHigh = 0;
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

    void init(WoWGuid const& guid) noexcept { *this = guid; }

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

    static constexpr std::size_t ModernBytesSize = 16;

    static WoWGuid createModern(uint64_t high, uint64_t low) noexcept
    {
        WoWGuid guid;
        guid._raw.value = low;
        guid._modernHigh = high;
        return guid;
    }

    static WoWGuid createModernPlayer(uint32_t realmId, uint64_t dbId, uint8_t subType = 0, uint32_t arg1 = 0) noexcept
    {
        return createModern((uint64_t(ModernHighGuid::Player) << 58U) | (uint64_t(realmId) << 42U) | (uint64_t(subType & 0x3U) << 40U) | (uint64_t(arg1 & 0xFFFFFFU) << 16U), dbId);
    }

    static WoWGuid createModernGuild(uint32_t realmId, uint64_t dbId) noexcept
    {
        return createModern((uint64_t(ModernHighGuid::Guild) << 58U) | (uint64_t(realmId) << 42U), dbId);
    }

    static WoWGuid createModernItem(uint32_t realmId, uint64_t dbId) noexcept
    {
        return createModern((uint64_t(ModernHighGuid::Item) << 58U) | (uint64_t(realmId) << 42U), dbId);
    }

    static WoWGuid createModernWorldObject(ModernHighGuid type, uint8_t subType, uint32_t realmId, uint16_t mapId, uint32_t serverId, uint32_t entry, uint64_t counter) noexcept
    {
        const uint64_t high = (uint64_t(type) << 58U)
            | (uint64_t(realmId & 0x1FFFU) << 42U)
            | (uint64_t(mapId & 0x1FFFU) << 29U)
            | (uint64_t(entry & 0x7FFFFFU) << 6U)
            | uint64_t(subType & 0x3FU);

        const uint64_t low = (uint64_t(serverId & 0xFFFFFFU) << 40U)
            | (counter & UINT64_C(0xFFFFFFFFFF));

        return createModern(high, low);
    }

    static WoWGuid createModernTransport(ModernHighGuid type, uint32_t counter) noexcept
    {
        return createModern((uint64_t(type) << 58U) | (uint64_t(counter) << 38U), 0);
    }

    static WoWGuid createModernGlobal(ModernHighGuid type, uint64_t dbIdHigh, uint64_t dbIdLow) noexcept
    {
        return createModern((uint64_t(type) << 58U) | (dbIdHigh & UINT64_C(0x03FFFFFFFFFFFFFF)), dbIdLow);
    }

    static WoWGuid createModernRealmClient(ModernHighGuid type, uint32_t realmId, uint32_t arg1, uint64_t counter) noexcept
    {
        return createModern((uint64_t(type) << 58U) | (uint64_t(realmId & 0x1FFFU) << 42U) | (uint64_t(arg1) << 10U), counter);
    }

    static WoWGuid createModernEmpty() noexcept { return createModern(0, 0); }

    static HighGuid legacyTypeFromModern(ModernHighGuid type) noexcept
    {
        switch (type)
        {
            case ModernHighGuid::Player:        return HighGuid::Player;
            case ModernHighGuid::Item:          return HighGuid::Item;
            case ModernHighGuid::Transport:     return HighGuid::Transport;
            case ModernHighGuid::Creature:      return HighGuid::Unit;
            case ModernHighGuid::Vehicle:       return HighGuid::Vehicle;
            case ModernHighGuid::Pet:           return HighGuid::Pet;
            case ModernHighGuid::GameObject:    return HighGuid::GameObject;
            case ModernHighGuid::DynamicObject: return HighGuid::DynamicObject;
            case ModernHighGuid::AreaTrigger:   return HighGuid::AreaTrigger;
            case ModernHighGuid::Corpse:        return HighGuid::Corpse;
            case ModernHighGuid::Guild:         return HighGuid::Guild;
            default:                            return HighGuid::Player;
        }
    }

    uint64_t toLegacyRaw() const noexcept
    {
        if (isModernEmpty())
            return 0;

        switch (getModernHighType())
        {
            case ModernHighGuid::Player:
                return getModernLow();

            case ModernHighGuid::Item:
                return (uint64_t(HIGHGUID_TYPE_ITEM) << 32U) | (getModernLow() & UINT64_C(0xFFFFFFFF));

            case ModernHighGuid::Guild:
                return (uint64_t(HIGHGUID_TYPE_GUILD) << 32U) | (getModernLow() & UINT64_C(0xFFFFFFFF));

            case ModernHighGuid::Transport:
                return (uint64_t(HIGHGUID_TYPE_TRANSPORT) << 32U) | (getModernCounter() & UINT64_C(0xFFFFFFFF));

            case ModernHighGuid::Creature:
            case ModernHighGuid::Vehicle:
            case ModernHighGuid::Pet:
            case ModernHighGuid::GameObject:
            case ModernHighGuid::DynamicObject:
            case ModernHighGuid::AreaTrigger:
            case ModernHighGuid::Corpse:
            {
                const HighGuid legacyType = legacyTypeFromModern(getModernHighType());
                return (uint64_t(legacyType) << 32U) | (uint64_t(getModernEntry() & 0xFFFFFFU) << 24U) | (getModernCounter() & UINT64_C(0xFFFFFF));
            }

            default:
                return getModernLow();
        }
    }

    static ModernHighGuid modernTypeFromLegacy(HighGuid type) noexcept
    {
        switch (type)
        {
            case HighGuid::Player:        return ModernHighGuid::Player;
            case HighGuid::Item:
            case HighGuid::Container:     return ModernHighGuid::Item;
            case HighGuid::Transporter:
            case HighGuid::Transport:     return ModernHighGuid::Transport;
            case HighGuid::GameObject:    return ModernHighGuid::GameObject;
            case HighGuid::DynamicObject: return ModernHighGuid::DynamicObject;
            case HighGuid::Unit:          return ModernHighGuid::Creature;
            case HighGuid::Pet:           return ModernHighGuid::Pet;
            case HighGuid::Vehicle:       return ModernHighGuid::Vehicle;
            case HighGuid::AreaTrigger:   return ModernHighGuid::AreaTrigger;
            case HighGuid::Corpse:        return ModernHighGuid::Corpse;
            case HighGuid::Guild:         return ModernHighGuid::Guild;
            default:                      return ModernHighGuid::Null;
        }
    }

    static WoWGuid createModernFromLegacy(uint64_t legacyGuid, uint32_t realmId, uint16_t mapId = 0, uint32_t serverId = 0, uint8_t subType = 0) noexcept
    {
        if (legacyGuid == 0)
            return createModernEmpty();

        const HighGuid legacyType = getHighTypeFromRaw(legacyGuid);
        const ModernHighGuid modernType = modernTypeFromLegacy(legacyType);
        const uint64_t lowPart = uint64_t(getLowGuidFromRaw(legacyGuid));

        switch (modernType)
        {
            case ModernHighGuid::Player:
                return createModernPlayer(realmId, lowPart);

            case ModernHighGuid::Item:
                return createModernItem(realmId, lowPart);

            case ModernHighGuid::Guild:
                return createModernGuild(realmId, lowPart);

            case ModernHighGuid::Transport:
                return createModernTransport(modernType, static_cast<uint32_t>(lowPart));

            case ModernHighGuid::Creature:
            case ModernHighGuid::Vehicle:
            case ModernHighGuid::Pet:
            case ModernHighGuid::GameObject:
            case ModernHighGuid::DynamicObject:
            case ModernHighGuid::AreaTrigger:
            case ModernHighGuid::Corpse:
                return createModernWorldObject(modernType, subType, realmId, mapId, serverId, static_cast<uint32_t>((legacyGuid >> 24U) & UINT64_C(0x00FFFFFF)), legacyGuid & UINT64_C(0x00FFFFFF));

            default:
                return createModernEmpty();
        }
    }

    uint64_t getModernRawValue(std::size_t index) const noexcept { ASSERT(index < 2); return index == 0 ? _raw.value : _modernHigh; }
    uint64_t getModernLow() const noexcept { return _raw.value; }
    uint64_t getModernHigh() const noexcept { return _modernHigh; }
    ModernHighGuid getModernHighType() const noexcept { return static_cast<ModernHighGuid>((_modernHigh >> 58U) & 0x3FU); }
    uint32_t getModernRealmId() const noexcept { return static_cast<uint32_t>((_modernHigh >> 42U) & 0xFFFFU); }
    uint32_t getModernMapId() const noexcept { return static_cast<uint32_t>((_modernHigh >> 29U) & 0x1FFFU); }
    uint32_t getModernEntry() const noexcept { return static_cast<uint32_t>((_modernHigh >> 6U) & 0x7FFFFFU); }
    uint8_t getModernSubType() const noexcept { return static_cast<uint8_t>(_modernHigh & 0x3FU); }
    uint32_t getModernServerId() const noexcept { return static_cast<uint32_t>((_raw.value >> 40U) & 0xFFFFFFU); }
    uint64_t getModernCounter() const noexcept
    {
        if (getModernHighType() == ModernHighGuid::Transport)
            return (_modernHigh >> 38U) & UINT64_C(0xFFFFF);
        return _raw.value & UINT64_C(0xFFFFFFFFFF);
    }
    bool isModernEmpty() const noexcept { return _raw.value == 0 && _modernHigh == 0; }

    static bool unpackModern(const uint8_t* data, std::size_t size, WoWGuid& guid, std::size_t& consumed)
    {
        consumed = 0;
        if (data == nullptr || size < sizeof(uint16_t))
            return false;

        const uint16_t mask = static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8U);
        std::array<uint8_t, ModernBytesSize> raw{};
        std::size_t offset = sizeof(uint16_t);

        for (std::size_t i = 0; i < raw.size(); ++i)
        {
            if ((mask & (uint16_t(1U) << i)) == 0)
                continue;
            if (offset >= size)
                return false;
            raw[i] = data[offset++];
        }

        uint64_t low = 0;
        uint64_t high = 0;
        for (std::size_t i = 0; i < sizeof(uint64_t); ++i)
        {
            low |= static_cast<uint64_t>(raw[i]) << (i * 8U);
            high |= static_cast<uint64_t>(raw[i + sizeof(uint64_t)]) << (i * 8U);
        }

        guid = createModern(high, low);
        consumed = offset;
        return true;
    }

    static bool unpackModern(const uint8_t* data, std::size_t size, WoWGuid& guid)
    {
        std::size_t consumed = 0;
        return unpackModern(data, size, guid, consumed) && consumed == size;
    }

    std::vector<uint8_t> packModern() const
    {
        std::array<uint8_t, ModernBytesSize> raw{};
        const std::array<uint64_t, 2> words{ _raw.value, _modernHigh };
        for (std::size_t word = 0; word < words.size(); ++word)
        {
            for (std::size_t byte = 0; byte < sizeof(uint64_t); ++byte)
                raw[word * sizeof(uint64_t) + byte] = static_cast<uint8_t>((words[word] >> (byte * 8U)) & 0xFFU);
        }

        uint16_t mask = 0;
        std::vector<uint8_t> packed;
        packed.reserve(2 + ModernBytesSize);
        packed.push_back(0);
        packed.push_back(0);

        for (std::size_t i = 0; i < raw.size(); ++i)
        {
            if (raw[i] == 0)
                continue;
            mask |= static_cast<uint16_t>(uint16_t(1U) << i);
            packed.push_back(raw[i]);
        }

        packed[0] = static_cast<uint8_t>(mask & 0xFFU);
        packed[1] = static_cast<uint8_t>((mask >> 8U) & 0xFFU);
        return packed;
    }

    uint64_t getRawGuid() const noexcept { return _raw.value; }

    const uint8_t* getNewGuid() const noexcept { return m_guidfields; }
    uint8_t getNewGuidLen() const noexcept { return BitCount8(guidmask); }
    uint8_t getNewGuidMask() const noexcept { return guidmask; }

    // helpers
    bool     operator!() const noexcept { return isEmpty(); }

    bool     operator==(int v) const noexcept { return _raw.value == static_cast<uint64_t>(v); }
    bool     operator!=(int v) const noexcept { return _raw.value != static_cast<uint64_t>(v); }

    bool     operator==(uint64_t v) const noexcept { return _raw.value == v; }
    bool     operator!=(uint64_t v) const noexcept { return _raw.value != v; }

    uint64_t operator&(uint64_t v) const noexcept { return _raw.value & v; }
    uint64_t operator&(unsigned int v) const noexcept { return _raw.value & uint64_t(v); }

    bool     operator==(WoWGuid const& other) const noexcept { return _raw.value == other._raw.value && _modernHigh == other._modernHigh; }
    bool     operator!=(WoWGuid const& other) const noexcept { return !(*this == other); }
    bool     operator<(WoWGuid const& other) const noexcept { return _modernHigh != other._modernHigh ? _modernHigh < other._modernHigh : _raw.value < other._raw.value; }

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

    uint64_t _modernHigh{};
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
            const std::size_t legacyHash = std::hash<uint64_t>{}(guid.getRawGuid());
            if (guid.getModernHigh() == 0)
                return legacyHash;
            return legacyHash ^ (std::hash<uint64_t>{}(guid.getModernHigh()) << 1U);
        }
    };
}
