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

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "SpellProc.h"
#include "Units/Unit.h"
#include "Units/Players/Player.h"

class TwinBladesOfAzzinothSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new TwinBladesOfAzzinothSpellProc(); }

    void Init(Object* /*obj*/)
    {
        if (!mTarget->IsPlayer())
            return;

        /* The Twin Blades of Azzinoth.
            * According to comments on wowhead, this proc has ~0.75ppm (procs-per-minute). */
        Item* mh = static_cast<Player*>(mTarget)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        Item* of = static_cast<Player*>(mTarget)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
        if (mh != nullptr && of != nullptr)
        {
            uint32 mhs = mh->GetItemProperties()->Delay;
            uint32 ohs = of->GetItemProperties()->Delay;
            mProcChance = mhs * ohs / (800 * (mhs + ohs));     // 0.75 ppm
        }
    }
};

void SpellProcMgr::SetupItems()
{
    uint32 mindNumbingPoison[] =
    {
        //SPELL_HASH_THE_TWIN_BLADES_OF_AZZINOTH
        41434,
        41435,
        0
    };
    AddById(mindNumbingPoison, &TwinBladesOfAzzinothSpellProc::Create);
}
