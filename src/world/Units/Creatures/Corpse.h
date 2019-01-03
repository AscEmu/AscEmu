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

#ifndef WOWSERVER_CORPSE_H
#define WOWSERVER_CORPSE_H

#include "Objects/Object.h"
#include "Management/LootMgr.h"

enum CORPSE_STATE
{
    CORPSE_STATE_BODY   = 0,
    CORPSE_STATE_BONES  = 1
};

#define CORPSE_RECLAIM_TIME 30
#define CORPSE_RECLAIM_TIME_MS CORPSE_RECLAIM_TIME * 1000
#define CORPSE_MINIMUM_RECLAIM_RADIUS 39
#define CORPSE_MINIMUM_RECLAIM_RADIUS_SQ CORPSE_MINIMUM_RECLAIM_RADIUS * CORPSE_MINIMUM_RECLAIM_RADIUS

// MIT Start

enum CorpseFlags
{
    CORPSE_FLAG_NONE = 0,
    CORPSE_FLAG_BONE = 1,
    CORPSE_FLAG_UNK = 2,
    CORPSE_FLAG_UNK1 = 4,
    CORPSE_FLAG_HIDDEN_HELM = 8,
    CORPSE_FLAG_HIDDEN_CLOAK = 16,
    CORPSE_FLAG_LOOT = 32
};

struct WoWCorpse;
class SERVER_DECL Corpse : public Object
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // WoWData
    const WoWCorpse* corpseData() const { return reinterpret_cast<WoWCorpse*>(wow_data); }

public:

    uint64_t getOwnerGuid() const;
    void setOwnerGuid(uint64_t guid);

    //party

    uint32_t getDisplayId() const;
    void setDisplayId(uint32_t id);

    uint32_t getItem(uint8_t slot) const;
    void setItem(uint8_t slot, uint32_t item);

    //bytes 1 start
    //unk1

    uint8_t getRace() const;
    void setRace(uint8_t race);

    //unk2

    uint8_t getSkinColor() const;
    void setSkinColor(uint8_t color);
    //bytes 1 end

    //bytes 2 start
    uint8_t getFace() const;
    void setFace(uint8_t face);

    uint8_t getHairStyle() const;
    void setHairStyle(uint8_t style);

    uint8_t getHairColor() const;
    void setHairColor(uint8_t color);

    uint8_t getFacialFeatures() const;
    void setFacialFeatures(uint8_t feature);
    //bytes 2 end

    //guild - removed in cata

    uint32_t getFlags() const;
    void setFlags(uint32_t flags);

    uint32_t getDynamicFlags() const;
    void setDynamicFlags(uint32_t flags);

// MIT End
// AGPL Start
        Corpse(uint32 high, uint32 low);
        ~Corpse();

        // void Create();
        void Create(Player* owner, uint32 mapid, float x, float y, float z, float ang);

        void SaveToDB();
        void DeleteFromDB();
        inline void SetCorpseState(uint32 state) { m_state = state; }
        inline uint32 GetCorpseState() { return m_state; }
        void Despawn();

        inline void SetLoadedFromDB(bool value) { _loadedfromdb = value; }
        inline bool GetLoadedFromDB(void) { return _loadedfromdb; }
        Loot loot;
        void generateLoot();

        void SpawnBones();
        void Delink();

        void ResetDeathClock() { m_time = time(NULL); }
        time_t GetDeathClock() { return m_time; }

        //Easy functions
        void SetOwner(uint64 guid);


    private:

        uint32 m_state;
        time_t m_time;
        uint32 _fields[CORPSE_END];
        bool _loadedfromdb;
};

#endif // _WOWSERVER_CORPSE_H
