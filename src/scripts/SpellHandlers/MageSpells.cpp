/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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
#include "Spell/SpellAuras.h"
#include "Server/Script/ScriptMgr.h"
#include "Spell/Definitions/ProcFlags.h"

bool Cold_Snap(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->p_caster)
        return true;

    pSpell->p_caster->ClearCooldownsOnLine(6, pSpell->getSpellInfo()->getId());
    return true;
}

bool Living_Bomb(uint8_t effectIndex, Aura* pAura, bool apply)
{
    Unit* caster = pAura->GetUnitCaster();
    if (caster && !apply)
        caster->castSpell(pAura->GetTarget(), pAura->GetSpellInfo()->getEffectBasePoints(effectIndex) + 1, true);
    return true;
}

bool HotStreak(uint8_t effectIndex, Aura* pAura, bool apply)
{
#if VERSION_STRING < Cata
    if (effectIndex == 0)
    {
        auto caster = pAura->GetUnitCaster();
        if (caster == nullptr)
            return true;

        if (apply)
        {
            static uint32 classMask[3] = { 0x13, 0x21000, 0 };
            caster->AddProcTriggerSpell(48108, pAura->GetSpellInfo()->getId(), caster->getGuid(), pAura->GetSpellInfo()->getEffectBasePoints(effectIndex) + 1, PROC_ON_SPELL_CRIT_HIT | PROC_ON_SPELL_HIT, 0, pAura->GetSpellInfo()->getEffectSpellClassMask(effectIndex), classMask);
        }
        else
            caster->RemoveProcTriggerSpell(48108);
    }
#endif

    return true;
}

bool SummonWaterElemental(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* caster = pSpell->u_caster;
    if (caster == NULL)
        return true;

    if (caster->HasAura(70937))    // Glyph of Eternal Water
        caster->castSpell(caster, 70908, true);
    else
        caster->castSpell(caster, 70907, true);

    return true;
}

bool TormentOfTheWeak(uint8_t effectIndex, Aura* a, bool apply)
{
    Unit* m_target = a->GetTarget();

    if (m_target->isPlayer())
    {
        static_cast<Player*>(m_target)->m_IncreaseDmgSnaredSlowed += ((apply) ? 1 : -1) * (uint32)(((float)a->GetModAmount(effectIndex)) / 100);
    }

    return true;
}

bool FingersOfFrost(uint8_t effectIndex, Aura* a, bool apply) // Should be visible to client by using ID 74396
{
    Player* caster = a->GetPlayerCaster();

    if (caster == NULL)
        return true;

    if (apply)
        caster->SetTriggerChill(44544, a->GetModAmount(effectIndex), false);
    else
        caster->SetTriggerChill(0, 0, false);

    return true;
}

bool BrainFreeze(uint8_t effectIndex, Aura* a, bool apply)
{
    Player* caster = a->GetPlayerCaster();

    if (caster == NULL)
        return true;

    if (apply)
        caster->SetTriggerChill(57761, a->GetModAmount(effectIndex), false);
    else
        caster->SetTriggerChill(0, 0, false);

    return true;
}

bool MagicAbsorbtion(uint8_t effectIndex, Aura* a, bool apply)
{
    Unit* m_target = a->GetTarget();

    if (m_target->isPlayer())
    {
        Player* p_target = static_cast<Player*>(m_target);

        if (apply)
            p_target->m_RegenManaOnSpellResist += (a->GetModAmount(effectIndex) / 100);
        else
            p_target->m_RegenManaOnSpellResist -= (a->GetModAmount(effectIndex) / 100);
    }

    return true;
}

bool MirrorImage(uint8_t effectIndex, Aura* pAura, bool apply)
{
    Unit* caster = pAura->GetUnitCaster();
    if (caster != NULL && apply && effectIndex == 2)
        if (caster->getGuid() == pAura->GetTarget()->getCreatedByGuid())
            caster->castSpell(pAura->GetTarget(), pAura->GetSpellInfo()->getEffectTriggerSpell(effectIndex), true);

    return true;
}

void SetupMageSpells(ScriptMgr* mgr)
{
    mgr->register_dummy_spell(11958, &Cold_Snap);
    mgr->register_dummy_aura(44457, &Living_Bomb);
    mgr->register_dummy_aura(55359, &Living_Bomb);
    mgr->register_dummy_aura(55360, &Living_Bomb);

    uint32 HotStreakIds[] = { 44445, 44446, 44448, 0 };
    mgr->register_dummy_aura(HotStreakIds, &HotStreak);

    mgr->register_dummy_spell(31687, &SummonWaterElemental);

    uint32 tormentoftheweakids[] =
    {
        29447,
        55339,
        55340,
        0
    };
    mgr->register_dummy_aura(tormentoftheweakids, &TormentOfTheWeak);

    uint32 fingersoffrostids[] =
    {
        44543,
        44545,
        0
    };
    mgr->register_dummy_aura(fingersoffrostids, &FingersOfFrost);

    uint32 brainfreezeids[] =
    {
        44546,
        44548,
        44549,
        0
    };
    mgr->register_dummy_aura(brainfreezeids, &BrainFreeze);

    uint32 magicabsorbtionids[] =
    {
        29441,
        29444,
        0
    };
    mgr->register_dummy_aura(magicabsorbtionids, &MagicAbsorbtion);

    mgr->register_dummy_aura(55342, &MirrorImage);
}
