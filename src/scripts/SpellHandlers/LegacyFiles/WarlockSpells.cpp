/*
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
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
#include "Objects/Units/Stats.h"
#include "Storage/MySQLDataStore.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Management/ItemInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Objects/GameObject.h"
#include "Utilities/CommonTime.hpp"

bool LifeTap(uint8_t effectIndex, Spell* s)
{
    Player* playerTarget = s->getPlayerTarget();

    if (!s->getPlayerCaster() || !playerTarget)
    {
        return false;
    }

    uint32_t mod;    // spirit bonus coefficient multiplied by 2
    if (s->getSpellInfo()->getId() == 1454)
        mod = 2;
    else if (s->getSpellInfo()->getId() == 1455)
        mod = 3;
    else if (s->getSpellInfo()->getId() == 1456)
        mod = 4;
    else if (s->getSpellInfo()->getId() == 11687)
        mod = 5;
    else
        mod = 6;

    uint32_t damage = s->getSpellInfo()->getEffectBasePoints(effectIndex) + 1 + mod * playerTarget->getStat(STAT_SPIRIT) / 2;

    if (damage >= playerTarget->getHealth())
        return false;

    s->getPlayerCaster()->dealDamage(playerTarget, damage, s->getSpellInfo()->getId());
    damage = damage * (100 + playerTarget->m_lifeTapBonus) / 100;    // Apply improved life tap
    s->getPlayerCaster()->energize(playerTarget, s->getSpellInfo()->getId(), damage, POWER_TYPE_MANA);

    return true;
}

bool SoulShatter(uint8_t /*effectIndex*/, Spell* s)
{
    Unit* unitTarget = s->getUnitTarget();

    if (!s->getPlayerCaster() || !s->getPlayerCaster()->isAlive() || !unitTarget || !unitTarget->isAlive())
        return false;

    s->getPlayerCaster()->castSpell(unitTarget, 32835, false);

    return true;
}

bool MinorHealthStone(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getPlayerCaster() == nullptr)
        return false;

    if (s->getPlayerCaster()->hasSpell(18692))
        s->CreateItem(19004);
    else if (s->getPlayerCaster()->hasSpell(18693))
        s->CreateItem(19005);
    else
        s->CreateItem(5512);

    return true;
}

bool LesserHealthStone(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getPlayerCaster() == nullptr)
        return false;

    if (s->getPlayerCaster()->hasSpell(18693))    // Improved Healthstone (2)
        s->CreateItem(19007);
    else if (s->getPlayerCaster()->hasSpell(18692))    // Improved Healthstone (1)
        s->CreateItem(19006);
    else
        s->getPlayerCaster()->getItemInterface()->AddItemById(5511, 1, 0);
    return true;
}

bool HealthStone(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getPlayerCaster() == nullptr)
        return false;

    if (s->getPlayerCaster()->hasSpell(18693))    // Improved Healthstone (2)
        s->CreateItem(19009);
    else if (s->getPlayerCaster()->hasSpell(18692))    // Improved Healthstone (1)
        s->CreateItem(19008);
    else
        s->CreateItem(5509);

    return true;
}

bool GreaterHealthStone(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getPlayerCaster() == nullptr)
        return false;

    if (s->getPlayerCaster()->hasSpell(18693))    // Improved Healthstone (2)
        s->CreateItem(19011);
    else if (s->getPlayerCaster()->hasSpell(18692))    // Improved Healthstone (1)
        s->CreateItem(19010);
    else
        s->CreateItem(5510);

    return true;
}

bool MajorHealthStone(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getPlayerCaster() == nullptr)
        return false;

    if (s->getPlayerCaster()->hasSpell(18693))    // Improved Healthstone (2)
        s->CreateItem(19013);
    else if (s->getPlayerCaster()->hasSpell(18692))    // Improved Healthstone (1)
        s->CreateItem(19012);
    else
        s->CreateItem(9421);

    return true;
}

bool MasterHealthStone(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getPlayerCaster() == nullptr)
        return false;

    if (s->getPlayerCaster()->hasSpell(18693))    // Improved Healthstone (2)
        s->CreateItem(22105);
    else if (s->getPlayerCaster()->hasSpell(18692))    // Improved Healthstone (1)
        s->CreateItem(22104);
    else
        s->CreateItem(22103);

    return true;
}

bool DemonicHealthStone(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getPlayerCaster() == nullptr)
        return false;

    if (s->getPlayerCaster()->hasSpell(18693))    // Improved Healthstone (2)
        s->CreateItem(36891);
    else if (s->getPlayerCaster()->hasSpell(18692))    // Improved Healthstone (1)
        s->CreateItem(36890);
    else
        s->CreateItem(36889);

    return true;
}

bool FelHealthStone(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getPlayerCaster() == nullptr)
        return false;

    if (s->getPlayerCaster()->hasSpell(18693))    // Improved Healthstone (2)
        s->CreateItem(36894);
    else if (s->getPlayerCaster()->hasSpell(18692))    // Improved Healthstone (1)
        s->CreateItem(36893);
    else
        s->CreateItem(36892);

    return true;
}

bool SummonSuccubusQuest(uint8_t /*effectIndex*/, Spell* s)
{
    Player* p_caster = s->getPlayerCaster();

    if (p_caster == nullptr)
        return false;

    if (Creature* pCreature = p_caster->summonCreature(5677, p_caster->GetPosition(), CreatureSummonDespawnType::TIMED_OR_CORPSE_DESPAWN, 1 * TimeVarsMs::Minute))
    {
        pCreature->getAIInterface()->Init(pCreature);
        pCreature->getThreatManager().tauntUpdate();
        return true;
    }

    return false;
}

bool SummonVoidWalkerQuest(uint8_t /*effectIndex*/, Spell* s)
{
    Player* p_caster = s->getPlayerCaster();

    if (p_caster == nullptr)
        return false;

    if (Creature* pCreature = p_caster->summonCreature(5676, p_caster->GetPosition(), CreatureSummonDespawnType::TIMED_OR_CORPSE_DESPAWN, 1 * TimeVarsMs::Minute))
    {
        pCreature->getAIInterface()->Init(pCreature);
        pCreature->getThreatManager().tauntUpdate();
        return true;
    }

    return false;
}

bool SummonFelHunterQuest(uint8_t /*effectIndex*/, Spell* s)
{
    Player* p_caster = s->getPlayerCaster();

    if (p_caster == nullptr)
        return false;

    if (Creature* pCreature = p_caster->summonCreature(6268, p_caster->GetPosition(), CreatureSummonDespawnType::TIMED_OR_CORPSE_DESPAWN, 1 * TimeVarsMs::Minute))
    {
        pCreature->getAIInterface()->Init(pCreature);
        pCreature->getThreatManager().tauntUpdate();
        return true;
    }

    return false;
}

bool ImprovedLifeTap(uint8_t effectIndex, Aura* a, bool apply)
{
    Unit* u_target = a->getOwner();
    if (!u_target->isPlayer())
        return true;

    Player* p_target = static_cast<Player*>(u_target);
    int32_t amount = a->getEffectDamage(effectIndex);


    if (apply)
        p_target->m_lifeTapBonus = amount;
    else
        p_target->m_lifeTapBonus = 0;

    return true;
}

bool SoulSiphon(uint8_t effectIndex, Aura* a, bool apply)
{
    Unit* caster = a->getUnitCaster();
    int32_t amount = a->getEffectDamage(effectIndex);

    if (caster)
    {
        if (apply)
            caster->m_soulSiphon.m_amount += amount;
        else
            caster->m_soulSiphon.m_amount -= amount;
    }

    return true;
}

bool SoulStoneResurrection(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* u_target = a->getOwner();
    if (!u_target->isPlayer())
        return true;

    Player* p_target = static_cast<Player*>(u_target);
    uint32_t soulstone = a->getSpellInfo()->getEffectMiscValue(0);

    if (apply)
    {
        p_target->setSelfResurrectSpell(soulstone);
    }
    else if (p_target->isAlive())
    {
        p_target->setSelfResurrectSpell(0);
    }
    return true;
}

bool DemonicCircleSummon(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* m_target = a->getOwner();

    if (m_target->getWorldMap() == nullptr)
        return true;

    if (apply)
    {

        GameObject* circle = m_target->getWorldMapGameObject(a->getOwner()->m_objectSlots[0].getRawGuid());
        SpellInfo const* sp = sSpellMgr.getSpellInfo(48020);

        if (circle != NULL && sp != NULL && m_target->CalcDistance(circle) <= sp->getMaxRange())
        {
            if (!m_target->hasAurasWithId(62388))
                m_target->castSpell(m_target, 62388, true);
        }
        else
            m_target->removeAllAurasById(62388);
    }
    else
    {
        m_target->removeAllAurasById(62388);
    }

    return true;
}

void SetupLegacyWarlockSpells(ScriptMgr* mgr)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Dummy Effect

    mgr->register_dummy_spell(1454, &LifeTap);
    mgr->register_dummy_spell(1455, &LifeTap);
    mgr->register_dummy_spell(1456, &LifeTap);
    mgr->register_dummy_spell(11687, &LifeTap);
    mgr->register_dummy_spell(11688, &LifeTap);
    mgr->register_dummy_spell(11689, &LifeTap);
    mgr->register_dummy_spell(27222, &LifeTap);
    mgr->register_dummy_spell(57946, &LifeTap);

    mgr->register_dummy_spell(29858, &SoulShatter);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Scripted Effect

    mgr->register_script_effect(6201, &MinorHealthStone);

    mgr->register_script_effect(6202, &LesserHealthStone);

    mgr->register_script_effect(5699, &HealthStone);

    mgr->register_script_effect(11729, &GreaterHealthStone);

    mgr->register_script_effect(11730, &MajorHealthStone);

    mgr->register_script_effect(27230, &MasterHealthStone);

    mgr->register_script_effect(47871, &DemonicHealthStone);

    mgr->register_script_effect(47878, &FelHealthStone);

    mgr->register_script_effect(8674, &SummonSuccubusQuest);
    mgr->register_script_effect(9223, &SummonSuccubusQuest);
    mgr->register_script_effect(9224, &SummonSuccubusQuest);

    mgr->register_script_effect(7728, &SummonVoidWalkerQuest);
    mgr->register_script_effect(9221, &SummonVoidWalkerQuest);
    mgr->register_script_effect(9222, &SummonVoidWalkerQuest);
    mgr->register_script_effect(30208, &SummonVoidWalkerQuest);

    mgr->register_script_effect(8712, &SummonFelHunterQuest);

    uint32_t improvedlifetapids[] =
    {
        18182,
        18183,
        0
    };
    mgr->register_dummy_aura(improvedlifetapids, &ImprovedLifeTap);

    uint32_t soulsiphonids[] =
    {
        17804,
        17805,
        0
    };
    mgr->register_dummy_aura(soulsiphonids, &SoulSiphon);

    uint32_t soulstoneresurrectionids[] =
    {
        20707,
        20762,
        20763,
        20764,
        20765,
        27239,
        47883,
        0
    };
    mgr->register_dummy_aura(soulstoneresurrectionids, &SoulStoneResurrection);

    mgr->register_dummy_aura(48018, &DemonicCircleSummon);
}
