/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2011 ArcEmu Team <http://www.ArcEmu.org/>
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
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"

bool Penance(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster() || !pSpell->getPlayerCaster()->isAlive() ||
        !pSpell->getUnitTarget() || !pSpell->getUnitTarget()->isAlive())
        return true;

    Unit* target = pSpell->getUnitTarget();
    Player* player = pSpell->getPlayerCaster();

    // index 0 contains the spell for the first tick, index 1 is the peroidic cast spell.
    uint32_t hostileSpell[] = { 0, 0 };
    uint32_t friendlySpell[] = { 0, 0 };

    switch (pSpell->getSpellInfo()->getId())
    {
        case 47540: //Rank 1
            hostileSpell[0] = 47666;
            hostileSpell[1] = 47758;

            friendlySpell[0] = 47750;
            friendlySpell[1] = 47757;
            break;
        case 53005:
            hostileSpell[0] = 52998;
            hostileSpell[1] = 53001;

            friendlySpell[0] = 52983;
            friendlySpell[1] = 52986;
            break;
        case 53006:
            hostileSpell[0] = 52999;
            hostileSpell[1] = 53002;

            friendlySpell[0] = 52984;
            friendlySpell[1] = 52987;
            break;
        case 53007:
            hostileSpell[0] = 53000;
            hostileSpell[1] = 53003;

            friendlySpell[0] = 52985;
            friendlySpell[1] = 52988;
            break;
    }

    if (player->isValidTarget(target))   // Do holy damage
    {
        // First tick is instant.
        player->castSpell(target, hostileSpell[0], true);
        player->castSpell(target, hostileSpell[1], false);
    }
    else // Heal
    {
        player->castSpell(target, friendlySpell[0], true);
        player->castSpell(target, friendlySpell[1], false);
    }
    return true;
}

bool PainAndSufferingProc(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* caster = pSpell->getPlayerCaster();
    if (caster == NULL)
        return true;

    Unit* target = pSpell->getUnitTarget();
    if (target == NULL)
        return true;

    uint32_t shadowWordPain[] =
    {
        //SPELL_HASH_SHADOW_WORD__PAIN
        589,
        594,
        970,
        992,
        2767,
        10892,
        10893,
        10894,
        11639,
        14032,
        15654,
        17146,
        19776,
        23268,
        23952,
        24212,
        25367,
        25368,
        27605,
        30854,
        30898,
        34441,
        34941,
        34942,
        37275,
        41355,
        46560,
        48124,
        48125,
        57778,
        59864,
        60005,
        60446,
        65541,
        68088,
        68089,
        68090,
        72318,
        72319,
        0
    };
    Aura* aura = target->getAuraWithIdForGuid(shadowWordPain, caster->getGuid());
    if (aura == nullptr)
        return true;

    // Set new aura's duration, reset event timer and set client visual aura
    aura->refreshOrModifyStack(true);

    return true;
}

void SetupLegacyPriestSpells(ScriptMgr* mgr)
{
    uint32_t PenanceIds[] =
    {
        47540, // Rank 1
        53005, // Rank 2
        53006, // Rank 3
        53007, // Rank 4
        0,
    };
    mgr->register_dummy_spell(PenanceIds, &Penance);

    mgr->register_script_effect(47948, &PainAndSufferingProc);
}
