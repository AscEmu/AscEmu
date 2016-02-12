/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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
 *
 */

#include "StdAfx.h"

void World::InitWarriorSpells()
{
    SpellEntry * sp = NULL;
    if (sp = dbcSpell.LookupEntryForced(60503))
    {
        sp->ProcOnNameHash[0] = SPELL_HASH_REND;
        sp->RecoveryTime = 6000;
    }

    if (sp = dbcSpell.LookupEntryForced(46854))
        sp->procFlags = PROC_ON_CRIT_ATTACK;

    if (sp = dbcSpell.LookupEntryForced(46855))
        sp->procFlags = PROC_ON_CRIT_ATTACK;

    if (sp = dbcSpell.LookupEntryForced(46913))
    {
        sp->procChance = 7;
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    if (sp = dbcSpell.LookupEntryForced(46914))
    {
        sp->procChance = 13;
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    if (sp = dbcSpell.LookupEntryForced(46915))
    {
        sp->procChance = 20;
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    if (sp = dbcSpell.LookupEntryForced(12797))
        sp->procFlags = PROC_ON_CAST_SPELL;

    if (sp = dbcSpell.LookupEntryForced(12799))
        sp->procFlags = PROC_ON_CAST_SPELL;

    if (sp = dbcSpell.LookupEntryForced(50720))
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM;

    if (sp = dbcSpell.LookupEntryForced(29593))
        sp->procFlags = PROC_ON_BLOCK_VICTIM | PROC_ON_DODGE_VICTIM | PROC_ON_RESIST_VICTIM;

    if (sp = dbcSpell.LookupEntryForced(29594))
        sp->procFlags = PROC_ON_BLOCK_VICTIM | PROC_ON_DODGE_VICTIM | PROC_ON_RESIST_VICTIM;

    if (sp = dbcSpell.LookupEntryForced(46945))
        sp->procFlags = PROC_ON_CAST_SPELL;

    if (sp = dbcSpell.LookupEntryForced(46946))
        sp->rangeIndex = 95;

    if (sp = dbcSpell.LookupEntryForced(46947))
        sp->rangeIndex = 95;

    if (sp = dbcSpell.LookupEntryForced(64867))
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT | PROC_ON_CRIT_ATTACK;
        sp->procChance = 2;
    }
    if (sp = dbcSpell.LookupEntryForced(56611))
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT | PROC_ON_CRIT_ATTACK;
        sp->procChance = 4;
    }
    if (sp = dbcSpell.LookupEntryForced(56612))
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT | PROC_ON_CRIT_ATTACK | PROC_ON_CRIT_HIT_VICTIM;
        sp->procChance = 6;
    }
    if (sp = dbcSpell.LookupEntryForced(56613))
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT | PROC_ON_CRIT_ATTACK | PROC_ON_CRIT_HIT_VICTIM;
        sp->procChance = 8;
    }
    if (sp = dbcSpell.LookupEntryForced(56614))
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT | PROC_ON_CRIT_ATTACK | PROC_ON_CRIT_HIT_VICTIM;
        sp->procChance = 10;
    }
    if (sp = dbcSpell.LookupEntryForced(18371))
    {
        //sp->logsId = 18371;
    }

    if (sp = dbcSpell.LookupEntryForced(50227))
    {
        sp->ProcOnNameHash[0] = SPELL_HASH_DEVASTATE;
        sp->ProcOnNameHash[1] = SPELL_HASH_REVENGE;
    }

    //if (sp = dbcSpell.LookupEntryForced(676))
    //{
    //      sp->RequiredShapeShift = (uint32)1 << (FORM_DEFENSIVESTANCE-1);
    //}
}
