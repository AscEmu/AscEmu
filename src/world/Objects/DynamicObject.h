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

#ifndef WOWSERVER_DYNAMICOBJECT_H
#define WOWSERVER_DYNAMICOBJECT_H

#include "Object.h"
#include "Units/Unit.h"

class SpellInfo;

enum DynamicObjectType
{
    DYNAMIC_OBJECT_PORTAL           = 0x0,      // unused
    DYNAMIC_OBJECT_AREA_SPELL       = 0x1,
    DYNAMIC_OBJECT_FARSIGHT_FOCUS   = 0x2
};

typedef std::set<uint64> DynamicObjectList;

struct WoWDynamicObject;
class SERVER_DECL DynamicObject : public Object
{
    // MIT Start
    //////////////////////////////////////////////////////////////////////////////////////////
    // WoWData
    const WoWDynamicObject* dynamicObjectData() const { return reinterpret_cast<WoWDynamicObject*>(wow_data); }

public:

    uint64_t getCasterGuid() const;
    void setCasterGuid(uint64_t guid);

    //bytes start
    uint8_t getDynamicType() const;
    void setDynamicType(uint8_t type);
    //bytes end

    uint32_t getSpellId() const;
    void setSpellId(uint32_t id);

    float getRadius() const;
    void setRadius(float radius);

    // Position set for classic and TBC
    float getDynamicX() const;
    void setDynamicX(float x);

    float getDynamicY() const;
    void setDynamicY(float y);

    float getDynamicZ() const;
    void setDynamicZ(float z);

    float getDynamicO() const;
    void setDynamicO(float o);

    //\todo really not used on Classic?
#if VERSION_STRING > Classic
    uint32_t getCastTime() const;
    void setCastTime(uint32_t time);
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    void Create(Unit* caster, Spell* pSpell, LocationVector lv, uint32 duration, float radius, uint32 type);
    // MIT End

        DynamicObject(uint32 high, uint32 low);
        ~DynamicObject();


        void Create(Unit* caster, Spell* pSpell, float x, float y, float z, uint32 duration, float radius, uint32 type);
        void UpdateTargets();

        void addToInRangeObjects(Object* pObj);
        void onRemoveInRangeObject(Object* pObj);
        void Remove();

    protected:

        SpellInfo const* m_spellProto;
        Unit* u_caster;
        Player* p_caster;
        Spell* m_parentSpell;
        DynamicObjectList targets;

        uint32 m_aliveDuration;
        uint32 _fields[DYNAMICOBJECT_END];
};

#endif // WOWSERVER_DYNAMICOBJECT_H
