/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Object.hpp"
#include "Data/WoWCorpse.hpp"
#include "Management/Loot/Loot.hpp"
#include "Server/UpdateFieldInclude.h"

enum CORPSE_STATE
{
    CORPSE_STATE_BODY   = 0,
    CORPSE_STATE_BONES  = 1
};

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
class Player;

class SERVER_DECL Corpse : public Object
{
public:
    Corpse(uint32_t high, uint32_t low);
    ~Corpse();

    void create(Player* owner, uint32_t mapid, LocationVector lv);
    void setCorpseDataFromDbString(std::string dbString);
    void saveToDB();
    void deleteFromDB();

    void setLoadedFromDB(bool value);
    bool getLoadedFromDB();

    void setCorpseState(uint32_t state);
    uint32_t getCorpseState();

    void setOwnerNotifyMap(uint64_t guid);
    
    Loot loot;
    void generateLoot();

    void despawn();
    void spawnBones();
    void delink();

    void resetDeathClock();
    time_t getDeathClock();

    //////////////////////////////////////////////////////////////////////////////////////////
    // WoWData
private:
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
    uint32_t getBytes1() const;
    void setBytes1(uint32_t bytes);

    //unk1

    uint8_t getRace() const;
    void setRace(uint8_t race);

    uint8_t getGender() const;
    void setGender(uint8_t gender);

    uint8_t getSkinColor() const;
    void setSkinColor(uint8_t color);
    //bytes 1 end

    //bytes 2 start
    uint32_t getBytes2() const;
    void setBytes2(uint32_t bytes);

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

protected:
    uint32_t m_state = CORPSE_STATE_BODY;
    time_t m_time = 0;
    bool _loadedfromdb = false;

    uint32_t _fields[getSizeOfStructure(WoWCorpse)];
};
