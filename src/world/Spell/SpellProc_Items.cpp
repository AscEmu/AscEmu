/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "SpellProc.hpp"
#include "Objects/Item.hpp"
#include "Management/ItemInterface.h"
#include "Objects/Units/Unit.hpp"
#include "Objects/Units/Players/Player.hpp"

class TwinBladesOfAzzinothSpellProc : public SpellProc
{
public:
    static std::unique_ptr<SpellProc> Create() { return std::make_unique<TwinBladesOfAzzinothSpellProc>(); }

    void init(Object* /*obj*/) override
    {
        if (!getProcOwner()->isPlayer())
            return;

        /* The Twin Blades of Azzinoth.
            * According to comments on wowhead, this proc has ~0.75ppm (procs-per-minute). */
        Item* mh = static_cast<Player*>(getProcOwner())->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        Item* of = static_cast<Player*>(getProcOwner())->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
        if (mh != nullptr && of != nullptr)
        {
            uint32_t mhs = mh->getItemProperties()->Delay;
            uint32_t ohs = of->getItemProperties()->Delay;
            setProcChance(mhs * ohs / (800 * (mhs + ohs)));     // 0.75 ppm
        }
    }
};

void SpellProcMgr::SetupItems()
{
    uint32_t mindNumbingPoison[] =
    {
        //SPELL_HASH_THE_TWIN_BLADES_OF_AZZINOTH
        41434,
        41435,
        0
    };
    addByIds(mindNumbingPoison, &TwinBladesOfAzzinothSpellProc::Create);
}
