/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2008-2011 <http://www.ArcEmu.org/>
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

#include "../../StdAfx.h"
#include "Units/Summons/TotemSummon.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/SpellCastTargetFlags.h"
#include "Spell/Definitions/PowerType.h"
#include "Spell/Customization/SpellCustomizations.hpp"

TotemSummon::TotemSummon(uint64 GUID) : Summon(GUID)
{}

TotemSummon::~TotemSummon()
{}

void TotemSummon::Load(CreatureProperties const* properties_, Unit* pOwner, LocationVector & position, uint32 spellid, int32 pSummonslot)
{
    Summon::Load(properties_, pOwner, position, spellid, pSummonslot);
    uint32 displayID = 0;

    MySQLStructure::TotemDisplayIds const* totemdisplay = sMySQLStore.getTotemDisplayId(creature_properties->Male_DisplayID);
    if (totemdisplay != nullptr)
    {
        switch (pOwner->getRace())
        {
            case RACE_DRAENEI:
                displayID = totemdisplay->draeneiId;
                break;
            case RACE_TROLL:
                displayID = totemdisplay->trollId;
                break;
            case RACE_ORC:
                displayID = totemdisplay->orcId;
                break;
#if VERSION_STRING == Cata
            case RACE_TAUREN:
                displayID = totemdisplay->taurenId;
                break;
            case RACE_DWARF:
                displayID = totemdisplay->dwarfId;
                break;
            case RACE_GOBLIN:
                displayID = totemdisplay->goblinId;
                break;
#endif
            default:
                displayID = totemdisplay->displayId;
                break;
        }
    }
    else
    {
        displayID = creature_properties->Male_DisplayID;
    }

    // Set up the creature.
    SetMaxPower(POWER_TYPE_FOCUS, pOwner->getLevel() * 30);
    SetPower(POWER_TYPE_FOCUS, pOwner->getLevel() * 30);
    setLevel(pOwner->getLevel());
    setRace(0);
    setClass(1);
    setGender(2);
    SetPowerType(1);
    SetBaseAttackTime(MELEE, 2000);
    SetBaseAttackTime(OFFHAND, 2000);
    SetBoundingRadius(1.0f);
    SetCombatReach(1.0f);
    SetDisplayId(displayID);
    SetNativeDisplayId(creature_properties->Male_DisplayID);
    SetCastSpeedMod(1.0f);
    setUInt32Value(UNIT_DYNAMIC_FLAGS, 0);

    InheritSMMods(pOwner);

    for (uint8 school = 0; school < SCHOOL_COUNT; school++)
    {
        ModDamageDone[school] = pOwner->GetDamageDoneMod(school);
        HealDoneMod[school] = pOwner->HealDoneMod[school];
    }

    m_aiInterface->Init(this, AI_SCRIPT_TOTEM, Movement::WP_MOVEMENT_SCRIPT_NONE, pOwner);
    DisableAI();
}

void TotemSummon::OnPushToWorld()
{
    Summon::OnPushToWorld();

    SetupSpells();
}

void TotemSummon::OnPreRemoveFromWorld()
{
    Summon::OnPreRemoveFromWorld();
}

Group* TotemSummon::GetGroup()
{
    if (GetOwner() != NULL)
        return GetOwner()->GetGroup();
    else
        return NULL;
}

void TotemSummon::SetupSpells()
{
    if (GetOwner() == NULL)
        return;

    SpellInfo* creatorspell = sSpellCustomizations.GetSpellInfo(GetCreatedBySpell());
    SpellInfo* TotemSpell = sSpellCustomizations.GetSpellInfo(creature_properties->AISpells[0]);

    if (TotemSpell == NULL)
    {
        LOG_DEBUG("Totem %u does not have any spells to cast", creature_properties->Id);
        return;
    }

    // Set up AI, depending on our spells.
    bool castingtotem = true;

    if (TotemSpell->HasEffect(SPELL_EFFECT_SUMMON) ||
        TotemSpell->HasEffect(SPELL_EFFECT_APPLY_GROUP_AREA_AURA) ||
        TotemSpell->HasEffect(SPELL_EFFECT_APPLY_RAID_AREA_AURA) ||
        TotemSpell->HasEffect(SPELL_EFFECT_PERSISTENT_AREA_AURA) ||
        (TotemSpell->HasEffect(SPELL_EFFECT_APPLY_AURA) && TotemSpell->appliesAreaAura(SPELL_AURA_PERIODIC_TRIGGER_SPELL)))
        castingtotem = false;

    if (!castingtotem)
    {
        // We're an area aura. Simply cast the spell.

        m_aiInterface->totemspell = creatorspell;

        Spell* pSpell = sSpellFactoryMgr.NewSpell(this, TotemSpell, true, 0);
        SpellCastTargets targets;

        if (!TotemSpell->HasEffect(SPELL_AURA_PERIODIC_TRIGGER_SPELL))
        {
            targets.setDestination(GetPosition());
            targets.m_targetMask = TARGET_FLAG_DEST_LOCATION;
        }
        pSpell->prepare(&targets);

    }
    else
    {
        // We're a casting totem. Switch AI on, and tell it to cast this spell.
        EnableAI();
        m_aiInterface->totemspell = TotemSpell;
        m_aiInterface->m_totemspelltimer = 0;
        m_aiInterface->m_totemspelltime = 3 * MSTIME_SECOND;
    }
}

void TotemSummon::Die(Unit* /*pAttacker*/, uint32 /*damage*/, uint32 /*spellid*/)
{
    Despawn(1, 0);
}
