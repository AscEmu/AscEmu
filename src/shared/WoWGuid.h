/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

enum class HighGuid
{
    Player = HIGHGUID_TYPE_PLAYER,
    Corpse = HIGHGUID_TYPE_CORPSE,
    Item = HIGHGUID_TYPE_ITEM,
    Container = HIGHGUID_TYPE_CONTAINER,
    DynamicObject = HIGHGUID_TYPE_DYNAMICOBJECT,
    Waypoint = HIGHGUID_TYPE_WAYPOINT,
    Transporter = HIGHGUID_TYPE_TRANSPORTER,
    GameObject = HIGHGUID_TYPE_GAMEOBJECT,
    Transport = HIGHGUID_TYPE_TRANSPORT,
    Unit = HIGHGUID_TYPE_UNIT,
    Pet = HIGHGUID_TYPE_PET,
    Vehicle = HIGHGUID_TYPE_VEHICLE,
    AreaTrigger = HIGHGUID_TYPE_AREATRIGGER,
    Battleground = HIGHGUID_TYPE_BATTLEGROUND,
    Instance = HIGHGUID_TYPE_INSTANCE,
    Group = HIGHGUID_TYPE_GROUP,
    Guild = HIGHGUID_TYPE_GUILD,
    HighGuidMask = HIGHGUID_TYPE_MASK,
    LowGuidMask = LOWGUID_ENTRY_MASK,
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

            *reinterpret_cast<uint32*>(guidfields) = 0;
            *reinterpret_cast<uint32*>(&guidfields[4]) = 0;
            compiled = false;
            fieldcount = 0;
        }

        void Init(uint64 guid)
        {
            Clear();

            m_rawGuid = guid;

            _CompileByOld();
        }

        void Init(uint8 mask)
        {
            Clear();

            guidmask = mask;

            if(!guidmask)
                _CompileByNew();
        }

        void Init(uint8 mask, const uint8* fields)
        {
            Clear();

            guidmask = mask;

            if(!BitCount8(guidmask))
                return;

            for(int i = 0; i < BitCount8(guidmask); i++)
                guidfields[i] = (fields[i]);

            fieldcount = BitCount8(guidmask);

            _CompileByNew();
        }

        uint32_t getGuidLow() const { return static_cast<uint32_t>(m_rawGuid); }
        uint32_t getGuidLowPart() const
        {
            const uint32_t lowGuid = *(reinterpret_cast<const uint32_t*>(&m_rawGuid));
            return lowGuid & 0x00FFFFFF;
        }

        uint32_t getGuidHigh() const { return static_cast<uint32_t>(m_rawGuid >> 32); }
        uint32_t getGuidHighPart() const
        {
            const uint32_t highGuid = *(reinterpret_cast<const uint32_t*>(&m_rawGuid) + 1);
            return highGuid & 0xFFF00000;
        }

        HighGuid getHigh() const { return static_cast<HighGuid>(getGuidHighPart()); }

        uint64 GetOldGuid() const { return m_rawGuid; }
        const uint8* GetNewGuid() const { return guidfields; }
        uint8 GetNewGuidLen() const { return BitCount8(guidmask); }
        uint8 GetNewGuidMask() const { return guidmask; }

        bool operator !() const { return (!m_rawGuid); }
        bool operator ==(uint64 someval) const { return (m_rawGuid == someval); }
        bool operator !=(uint64 someval) const { return (m_rawGuid != someval); }
        uint64 operator &(uint64 someval) const { return (m_rawGuid & someval); }
        uint64 operator &(unsigned int someval) const { return (m_rawGuid & someval); }
        operator bool() { return (m_rawGuid > 0); }
        operator uint64() { return m_rawGuid; }
        void operator =(uint64 someval) { Clear(); Init(static_cast<uint64>(someval)); }

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
        
        void AppendField(uint8 field)
        {
            ASSERT(!compiled);
            ASSERT(fieldcount < BitCount8(guidmask));

            guidfields[fieldcount++] = field;

            if(fieldcount == BitCount8(guidmask))
                _CompileByNew();
        }

    private:

        uint64_t m_rawGuid;
        uint8 guidmask;

        uint8 guidfields[8];

        uint8 fieldcount;
        bool compiled;

        void _CompileByOld()
        {
            ASSERT(!compiled);

            fieldcount = 0;

            for(uint32 x = 0; x < 8; x++)
            {

                uint8 p = ((uint8*)&m_rawGuid)[x];

                if(p)
                {
                    guidfields[fieldcount++] = p;
                    guidmask |= 1 << x;
                }
            }
            compiled = true;
        }
        void _CompileByNew()
        {
            ASSERT(!compiled || fieldcount == BitCount8(guidmask));

            int j = 0;

            if(guidmask & 0x01)  //1
            {
                m_rawGuid |= ((uint64)guidfields[j]);
                j++;
            }
            if(guidmask & 0x02)  //2
            {
                m_rawGuid |= ((uint64)guidfields[j] << 8);
                j++;
            }
            if(guidmask & 0x04) //4
            {
                m_rawGuid |= ((uint64)guidfields[j] << 16);
                j++;
            }
            if(guidmask & 0x08)  //8
            {
                m_rawGuid |= ((uint64)guidfields[j] << 24);
                j++;
            }
            if(guidmask & 0x10) //16
            {
                m_rawGuid |= ((uint64)guidfields[j] << 32);
                j++;
            }
            if(guidmask & 0x20) //32
            {
                m_rawGuid |= ((uint64)guidfields[j] << 40);
                j++;
            }
            if(guidmask & 0x40) //64
            {
                m_rawGuid |= ((uint64)guidfields[j] << 48);
                j++;
            }
            if(guidmask & 0x80)  //128
            {
                m_rawGuid |= ((uint64)guidfields[j] << 56);
                j++;
            }
            compiled = true;
        }
};

#endif
