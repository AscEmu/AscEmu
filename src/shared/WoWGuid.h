/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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
 *
 */

#ifndef _WOWGUID_H
#define _WOWGUID_H

#include "CommonTypes.hpp"
#include "Errors.h"

#define BitCount1(x) ((x) & 1)
#define BitCount2(x) ( BitCount1(x) + BitCount1((x)>>1) )
#define BitCount4(x) ( BitCount2(x) + BitCount2((x)>>2) )
#define BitCount8(x) ( BitCount4(x) + BitCount4((x)>>4) )

enum HIGHGUID_TYPE
{
    HIGHGUID_TYPE_PLAYER = 0x00000000,
    HIGHGUID_TYPE_CORPSE = 0x30000000,
    HIGHGUID_TYPE_ITEM = 0x40000000,
    HIGHGUID_TYPE_CONTAINER = 0x50000000,			// confirm this pl0x
    HIGHGUID_TYPE_DYNAMICOBJECT = 0x60000000,
    HIGHGUID_TYPE_WAYPOINT = 0x10000000,
    HIGHGUID_TYPE_TRANSPORTER = 0x1FC00000,
    HIGHGUID_TYPE_GAMEOBJECT = 0xF1100000,
    HIGHGUID_TYPE_TRANSPORT = 0xF1200000,
    HIGHGUID_TYPE_UNIT = 0xF1300000,
    HIGHGUID_TYPE_PET = 0xF1400000,
    HIGHGUID_TYPE_VEHICLE = 0xF1500000,
    HIGHGUID_TYPE_AREATRIGGER = 0xF1020000,
    HIGHGUID_TYPE_BATTLEGROUND = 0x1F100000,
    HIGHGUID_TYPE_INSTANCE = 0x1F400000,
    HIGHGUID_TYPE_GROUP = 0x1F500000,
    HIGHGUID_TYPE_GUILD = 0x1FF70000,
    //===============================================
    HIGHGUID_TYPE_MASK = 0xFFF00000,
    LOWGUID_ENTRY_MASK = 0x00FFFFFF,
};

enum class HighGuid : uint64_t
{
    Player = 0x00000000,
    Corpse = 0x30000000,
    Item = 0x40000000,
    Container = 0x50000000,
    DynamicObject = 0x60000000,
    Waypoint = 0x10000000,
    Transporter = 0x1FC00000,
    GameObject = 0xF1100000,
    Transport = 0xF1200000,
    Unit = 0xF1300000,
    Pet = 0xF1400000,
    Vehicle = 0xF1500000,
    AreaTrigger = 0xF1020000,
    Battleground = 0x1F100000,
    Instance = 0x1F400000,
    Group = 0x1F500000,
    Guild = 0x1FF70000,
    HighGuidMask = 0xFFF00000,
    LowGuidMask = 0x00FFFFFF,
};

struct ObjectGuid
{
    public:

        ObjectGuid() { _data.u64 = 0LL; }
        ObjectGuid(uint64_t guid) { _data.u64 = guid; }
        ObjectGuid(ObjectGuid const& other) { _data.u64 = other._data.u64; }

        bool IsEmpty() const { return _data.u64 == 0; }

        uint8_t& operator[](uint32_t index)
        {
            ASSERT(index < sizeof(uint64_t));
            return _data.byte[index];
        }

        uint8_t const& operator[](uint32_t index) const
        {
            ASSERT(index < sizeof(uint64_t));
            return _data.byte[index];
        }

        operator uint64_t() { return _data.u64; }

        ObjectGuid& operator=(uint64_t guid)
        {
            _data.u64 = guid;
            return *this;
        }

        ObjectGuid& operator=(ObjectGuid const& other)
        {
            _data.u64 = other._data.u64;
            return *this;
        }

    private:

        union
        {
            uint64_t u64;
            uint8_t byte[8];
        } _data;
};


class SERVER_DECL WoWGuid
{
    public:

        WoWGuid()
        {
            Clear();
        }

        WoWGuid(uint64_t guid)
        {
            Init(static_cast<uint64_t>(guid));
        }

        WoWGuid(uint8_t mask)
        {
            Init(static_cast<uint8_t>(mask));
        }

        WoWGuid(uint8_t mask, uint8_t* fields)
        {
            Init(mask, fields);
        }

        ~WoWGuid()
        {
            Clear();
        }

        void Clear()
        {
            m_rawGuid = 0;
            guidmask = 0;

            *reinterpret_cast<uint32_t*>(m_guidfields) = 0;
            *reinterpret_cast<uint32_t*>(&m_guidfields[4]) = 0;
            m_compiled = false;
            m_fieldcount = 0;
        }

        void Init(uint64_t guid)
        {
            Clear();

            m_rawGuid = guid;

            _CompileByOld();
        }

        void Init(uint8_t mask)
        {
            Clear();

            guidmask = mask;

            if(!guidmask)
                _CompileByNew();
        }

        void Init(uint8_t mask, const uint8_t* fields)
        {
            Clear();

            guidmask = mask;

            if (!BitCount8(guidmask))
                return;

            for (int i = 0; i < BitCount8(guidmask); i++)
                m_guidfields[i] = (fields[i]);

            m_fieldcount = BitCount8(guidmask);

            _CompileByNew();
        }

        uint32_t getGuidLow() const { return static_cast<uint32_t>(m_rawGuid); }
        uint32_t getGuidLowPart() const
        {
            const uint32_t lowGuid = *(reinterpret_cast<const uint32_t*>(&m_rawGuid));
            return lowGuid & 0x00FFFFFF;
        }

        static uint32_t getGuidLowPartFromUInt64(uint64_t guid)
        {
            uint32_t lowGuid = *reinterpret_cast<const uint32_t*>(&guid);
            return lowGuid;
        }

        uint32_t getGuidHigh() const { return static_cast<uint32_t>(m_rawGuid >> 32); }
        uint32_t getGuidHighPart() const
        {
            const uint32_t highGuid = *(reinterpret_cast<const uint32_t*>(&m_rawGuid) + 1);
            return highGuid & 0xFFF00000;
        }

        static uint32_t getGuidHighPartFromUInt64(uint64_t guid)
        {
            uint32_t highGuid = *(reinterpret_cast<const uint32_t*>(&guid) + 1);
            return highGuid;
        }
        
        HighGuid getHigh() const { return static_cast<HighGuid>(getGuidHighPart()); }

        static uint64_t createPetGuid(uint32_t entry, uint32_t lowGuid)
        {
            uint64_t rawGuid = 0;

            rawGuid = uint64_t(HIGHGUID_TYPE_PET) << 32;
            rawGuid = rawGuid | uint64_t(entry) << 24;
            rawGuid = rawGuid | lowGuid;

            return rawGuid;
        }

        static uint64_t createItemGuid(uint32_t lowguid)
        {
            uint64_t rawGuid = 0;

            uint32_t* guidPart = reinterpret_cast<uint32_t*>(&rawGuid);

            guidPart[0] = lowguid;
            guidPart[1] = HIGHGUID_TYPE_ITEM;

            return rawGuid;
        }

        uint64_t GetOldGuid() const { return m_rawGuid; }
        const uint8_t* GetNewGuid() const { return m_guidfields; }
        uint8_t GetNewGuidLen() const { return BitCount8(guidmask); }
        uint8_t GetNewGuidMask() const { return guidmask; }

        bool operator !() const { return (!m_rawGuid); }
        bool operator ==(uint64_t someval) const { return (m_rawGuid == someval); }
        bool operator !=(uint64_t someval) const { return (m_rawGuid != someval); }
        uint64_t operator &(uint64_t someval) const { return (m_rawGuid & someval); }
        uint64_t operator &(unsigned int someval) const { return (m_rawGuid & someval); }
        operator bool() { return (m_rawGuid > 0); }
        operator uint64_t() { return m_rawGuid; }
        void operator =(uint64_t someval) { Clear(); Init(static_cast<uint64_t>(someval)); }

        bool isPlayer() const { return getHigh() == HighGuid::Player; }
        bool isCorpse() const { return getHigh() == HighGuid::Corpse; }
        bool isItem() const { return getHigh() == HighGuid::Item; }
        bool isContainer() const { return getHigh() == HighGuid::Container; }
        bool isDynamicObject() const { return getHigh() == HighGuid::DynamicObject; }
        bool isWaypoint() const { return getHigh() == HighGuid::Waypoint; }
        bool isTransporter() const { return getHigh() == HighGuid::Transporter; }
        bool isGameObject() const { return getHigh() == HighGuid::GameObject; }
        bool isTransport() const { return getHigh() == HighGuid::Transport; }
        bool isUnit() const { return getHigh() == HighGuid::Unit; }
        bool isPet() const { return getHigh() == HighGuid::Pet; }
        bool isVehicle() const { return getHigh() == HighGuid::Vehicle; }
        bool isAreaTrigger() const { return getHigh() == HighGuid::AreaTrigger; }
        bool isBattleground() const { return getHigh() == HighGuid::Battleground; }
        bool isInstance() const { return getHigh() == HighGuid::Instance; }
        bool isGroup() const { return getHigh() == HighGuid::Group; }
        bool isGuild() const { return getHigh() == HighGuid::Guild; }

        void AppendField(uint8_t field)
        {
            ASSERT(!m_compiled);
            ASSERT(m_fieldcount < BitCount8(guidmask));

            m_guidfields[m_fieldcount++] = field;

            if (m_fieldcount == BitCount8(guidmask))
                _CompileByNew();
        }

    private:

        uint64_t m_rawGuid{};
        uint8_t guidmask{};

        uint8_t m_guidfields[8]{};

        uint8_t m_fieldcount{};
        bool m_compiled{};

        void _CompileByOld()
        {
            ASSERT(!m_compiled);

            m_fieldcount = 0;

            for (uint8_t x = 0; x < 8; x++)
            {

                uint8_t p = reinterpret_cast<uint8_t*>(&m_rawGuid)[x];

                if (p)
                {
                    m_guidfields[m_fieldcount++] = p;
                    guidmask |= 1 << x;
                }
            }

            m_compiled = true;
        }

        void _CompileByNew()
        {
            ASSERT(!m_compiled || m_fieldcount == BitCount8(guidmask));

            int j = 0;

            if (guidmask & 0x01)  //1
            {
                m_rawGuid |= static_cast<uint64_t>(m_guidfields[j]);
                j++;
            }
            if (guidmask & 0x02)  //2
            {
                m_rawGuid |= (static_cast<uint64_t>(m_guidfields[j]) << 8);
                j++;
            }
            if (guidmask & 0x04) //4
            {
                m_rawGuid |= (static_cast<uint64_t>(m_guidfields[j]) << 16);
                j++;
            }
            if (guidmask & 0x08)  //8
            {
                m_rawGuid |= (static_cast<uint64_t>(m_guidfields[j]) << 24);
                j++;
            }
            if (guidmask & 0x10) //16
            {
                m_rawGuid |= (static_cast<uint64_t>(m_guidfields[j]) << 32);
                j++;
            }
            if (guidmask & 0x20) //32
            {
                m_rawGuid |= (static_cast<uint64_t>(m_guidfields[j]) << 40);
                j++;
            }
            if (guidmask & 0x40) //64
            {
                m_rawGuid |= (static_cast<uint64_t>(m_guidfields[j]) << 48);
                j++;
            }
            if (guidmask & 0x80)  //128
            {
                m_rawGuid |= (static_cast<uint64_t>(m_guidfields[j]) << 56);
                j++;
            }

            m_compiled = true;
        }
};

#endif
