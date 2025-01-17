/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008 WEmu Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Management/ItemInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Spell/Spell.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

enum
{
    SPELL_BENDINGSHINBONE = 8856,
    ITEM_STURDYSHINBONE = 7134,
    ITEM_BROKENSHINBONE = 7135,
};

bool BendingShinbone(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster())
    {
        if (Util::getRandomUInt(100) < 17) // 17% chance
            pSpell->getPlayerCaster()->getItemInterface()->AddItemById(ITEM_STURDYSHINBONE, 1, 0); // Sturdy Dragon
        else
            pSpell->getPlayerCaster()->getItemInterface()->AddItemById(ITEM_BROKENSHINBONE, 1, 0);
    }
    return true;
}

void SetupWetlands(ScriptMgr* mgr)
{
    mgr->register_script_effect(SPELL_BENDINGSHINBONE, &BendingShinbone);
}
