/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

#include "CRitual.h"

void CRitual::Setup(unsigned long caster_guid, unsigned long target_guid, unsigned long spell_id)
{
    this->CasterGUID = caster_guid;
    this->TargetGUID = target_guid;
    this->SpellID = spell_id;

    AddMember(caster_guid);
}

bool CRitual::AddMember(unsigned long GUID)
{
    unsigned long i = 0;
    for (; i < MaxMembers; i++)
        if (Members[i] == 0)
            break;

    if (i == MaxMembers)
        return false;

    Members[i] = GUID;
    CurrentMembers++;

    return true;
}

bool CRitual::RemoveMember(unsigned long GUID)
{
    unsigned long i = 0;
    for (; i < MaxMembers; i++)
    {
        if (Members[i] == GUID)
        {
            Members[i] = 0;
            CurrentMembers--;
            return true;
        }
    }

    return false;
}

bool CRitual::HasMember(unsigned long GUID)
{
    for (unsigned long i = 0; i < MaxMembers; i++)
        if (Members[i] == GUID)
            return true;

    return false;
}
