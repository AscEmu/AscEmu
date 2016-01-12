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

class FireNova : public Spell
{
    SPELL_FACTORY_FUNCTION(FireNova);
    bool HasFireTotem = false;
    uint8 CanCast(bool tolerate)
    {
        uint8 result = Spell::CanCast(tolerate);

        if (result == SPELL_CANCAST_OK)
        {
            if (u_caster)
            {
                // If someone has a better solutionen, your welcome :-)
                int totem_ids[32] = {
                //Searing Totems
                2523, 3902, 3903, 3904, 7400, 7402, 15480, 31162, 31164, 31165,
                //Magma Totems
                8929, 7464, 7435, 7466, 15484, 31166, 31167,
                //Fire Elementel
                15439,
                //Flametongue Totems
                5950, 6012, 7423, 10557, 15485, 31132, 31158, 31133,
                //Frost Resistance Totems
                5926, 7412, 7413, 15486, 31171, 31172
                };
                Unit* totem;
                for (uint8 i = 0; i < 32; i++)
                {
                    totem = u_caster->summonhandler.GetSummonWithEntry(totem_ids[i]);   // Get possible firetotem
                    if (totem != NULL)
                    {
                        HasFireTotem = true;
                        CastSpell(totem);
                    }
                }
                if (!HasFireTotem)
                {
                    SetExtraCastResult(SPELL_EXTRA_ERROR_MUST_HAVE_FIRE_TOTEM);
                    result = SPELL_FAILED_CUSTOM_ERROR;
                }
            }
        }
        return result;
    }

    void CastSpell(Unit* totem)
    {
        uint32 fireNovaSpells = Spell::GetProto()->Id;
        //Cast spell. NOTICE All ranks are linked with a extra spell in HackFixes.cpp
        totem->CastSpellAoF(totem->GetPositionX(), totem->GetPositionY(), totem->GetPositionZ(), dbcSpell.LookupEntryForced(fireNovaSpells), true);
    }
};

void SpellFactoryMgr::SetupShaman()
{
    AddSpellById(1535, FireNova::Create);   //Rank 1
    AddSpellById(8498, FireNova::Create);   //Rank 2
    AddSpellById(8499, FireNova::Create);   //Rank 3
    AddSpellById(11314, FireNova::Create);  //Rank 4
    AddSpellById(11315, FireNova::Create);  //Rank 5
    AddSpellById(25546, FireNova::Create);  //Rank 6
    AddSpellById(25547, FireNova::Create);  //Rank 7
    AddSpellById(61649, FireNova::Create);  //Rank 8
    AddSpellById(61657, FireNova::Create);  //Rank 9
}
