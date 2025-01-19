/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Object.hpp"
#include "Units/Unit.hpp"
#include "Data/WoWDynamicObject.hpp"
#include "Server/UpdateFieldInclude.h"

enum DynamicObjectType
{
    DYNAMIC_OBJECT_PORTAL           = 0x0,      // unused
    DYNAMIC_OBJECT_AREA_SPELL       = 0x1,
    DYNAMIC_OBJECT_FARSIGHT_FOCUS   = 0x2
};

class SpellInfo;
struct WoWDynamicObject;

class SERVER_DECL DynamicObject : public Object
{
public:
    DynamicObject(uint32_t high, uint32_t low);
    ~DynamicObject();

    void create(Unit* caster, Spell* pSpell, LocationVector lv, uint32_t duration, float radius, uint32_t type);
    void updateTargets();

    void onRemoveInRangeObject(Object* pObj) override;
    void remove();

    //////////////////////////////////////////////////////////////////////////////////////////
    // WoWData
private:
    const WoWDynamicObject* dynamicObjectData() const { return reinterpret_cast<WoWDynamicObject*>(wow_data); }

public:
    uint64_t getCasterGuid() const;
    void setCasterGuid(uint64_t guid);

    uint8_t getDynamicType() const;
    void setDynamicType(uint8_t type);

    uint32_t getSpellId() const;
    void setSpellId(uint32_t id);

    float getRadius() const;
    void setRadius(float radius);

    float getDynamicX() const;
    void setDynamicX(float x);

    float getDynamicY() const;
    void setDynamicY(float y);

    float getDynamicZ() const;
    void setDynamicZ(float z);

    float getDynamicO() const;
    void setDynamicO(float o);

#if VERSION_STRING > Classic
    uint32_t getCastTime() const;
    void setCastTime(uint32_t time);
#endif

protected:
    SpellInfo const* m_spellInfo = nullptr;
    Unit* m_unitCaster = nullptr;
    Player* m_playerCaster = nullptr;
    Spell* m_parentSpell = nullptr;

    std::set<uint64_t> m_targets = { 0 };

    uint32_t m_aliveDuration = 0;

    uint32_t _fields[getSizeOfStructure(WoWDynamicObject)];
};
