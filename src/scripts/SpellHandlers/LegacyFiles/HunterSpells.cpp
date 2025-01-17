/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
#include "Spell/SpellAura.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Spell/Definitions/SpellMechanics.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Utilities/Narrow.hpp"

bool Refocus(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* playerTarget = pSpell->getPlayerTarget();
    if (playerTarget == 0) return true;

    SpellSet::const_iterator itr = playerTarget->getSpellSet().begin();
    for (; itr != playerTarget->getSpellSet().end(); ++itr)
    {
        if ((*itr) == 24531)       // skip calling spell.. otherwise spammies! :D
            continue;

        if ((*itr) == 19434 || (*itr) == 20900 || (*itr) == 20901 || (*itr) == 20902 || (*itr) == 20903 || (*itr) == 20904 || (*itr) == 27632
            || (*itr) == 2643 || (*itr) == 14288 || (*itr) == 14289 || (*itr) == 14290 || (*itr) == 25294 || (*itr) == 14443 || (*itr) == 18651 || (*itr) == 20735 || (*itr) == 21390
            || (*itr) == 1510 || (*itr) == 14294 || (*itr) == 14295 || (*itr) == 1540 || (*itr) == 22908
            || (*itr) == 3044 || (*itr) == 14281 || (*itr) == 14282 || (*itr) == 14283 || (*itr) == 14284 || (*itr) == 14285 || (*itr) == 14286 || (*itr) == 14287)
            playerTarget->clearCooldownForSpell((*itr));
    }
    return true;
}

bool Readiness(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster())
        return true;
    pSpell->getPlayerCaster()->clearCooldownsOnLine(50, pSpell->getSpellInfo()->getId());//Beast Mastery
    pSpell->getPlayerCaster()->clearCooldownsOnLine(163, pSpell->getSpellInfo()->getId());//Marksmanship
    pSpell->getPlayerCaster()->clearCooldownsOnLine(51, pSpell->getSpellInfo()->getId());//Survival
    return true;
}

bool MastersCall(uint8_t effectIndex, Spell* pSpell)
{
    Player* caster = pSpell->getPlayerCaster();

    if (caster == NULL)
        return true;

    Pet* Summon = caster->getPet();
    if (Summon == NULL || Summon->isDead())
        return true;

    switch (effectIndex)
    {
        case 0:
            Summon->castSpell(caster, pSpell->damage, true);
            return true;
        case 1:
            Summon->castSpell(Summon, 62305, true);
            return true;
    }

    return true;
}

bool TheBeastWithin(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* m_target = a->getOwner();

    const auto mechanics = sSpellMgr.getCrowdControlMechanicList(false);
    for (int x = 0; mechanics[x] != MECHANIC_NONE; ++x)
    {
        if (apply)
            m_target->m_mechanicsDispels[mechanics[x]]++;
        else
            m_target->m_mechanicsDispels[mechanics[x]]--;
    }

    if (apply)
        m_target->removeAllAurasBySpellMechanic(mechanics);

    return true;
}


bool BestialWrath(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* m_target = a->getOwner();

    const auto mechanics = sSpellMgr.getCrowdControlMechanicList(false);
    for (int x = 0; mechanics[x] != MECHANIC_NONE; ++x)
    {
        if (apply)
            m_target->m_mechanicsDispels[mechanics[x]]++;
        else
            m_target->m_mechanicsDispels[mechanics[x]]--;
    }

    if (apply)
        m_target->removeAllAurasBySpellMechanic(mechanics);

    return true;
}

bool Misdirection(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Player* caster = a->GetPlayerCaster();

    if (caster == NULL)
        return true;

    if (!apply)
        sEventMgr.AddEvent(caster, &Player::setMisdirectionTarget, (uint64_t)0, EVENT_UNK, 250, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    return true;
}

bool ExplosiveShot(uint8_t effectIndex, Aura* a, bool apply)
{
    if (!apply)
        return true;

    Unit* m_target = a->getOwner();

    int32_t dmg = a->getEffectDamage(effectIndex);
    dmg += Util::float2int32(m_target->getRangedAttackPower() * 0.16f);

    //\ todo: fix me
    //a->EventPeriodicDamage(&a->getAuraEffect(effectIndex), dmg);

    return true;
}

class HasNameHash : public AuraCondition
{
public:
    bool operator()(Aura *aura)
    {
        uint32_t spellId = aura->getSpellInfo()->getId();

        if (std::find(spellIds.begin(), spellIds.end(), spellId) != spellIds.end())
            return true;
        else
            return false;
    }

    void AddSpellIdToCheck(uint32_t hash)
    {
        spellIds.push_back(hash);
    }

private:
    std::vector< uint32_t > spellIds;
};

class ChimeraShotAction : public AuraAction
{
public:
    void operator()(Aura *a) {
        uint32_t spellId = a->getSpellInfo()->getId();

        Unit *caster = a->GetUnitCaster();
        Unit *target = a->getOwner();

        if (caster == NULL)
            return;

        a->refreshOrModifyStack();

        switch (spellId)
        {
            // SPELL_HASH_SCORPID_STING:
            case 3043:
            case 18545:
            case 52604:
                caster->castSpell(target, 53359, true);
                break;

            // SPELL_HASH_WYVERN_STING:
            case 19386:
            case 24131:
            case 24132:
            case 24133:
            case 24134:
            case 24135:
            case 24335:
            case 24336:
            case 26180:
            case 26233:
            case 26748:
            case 27068:
            case 27069:
            case 41186:
            case 49009:
            case 49010:
            case 49011:
            case 49012:
            case 65877:
            case 65878:
                caster->castSpell(target, 53366, true);
                break;

            // SPELL_HASH_SERPENT_STING:
            case 1978:
            case 13549:
            case 13550:
            case 13551:
            case 13552:
            case 13553:
            case 13554:
            case 13555:
            case 25295:
            case 27016:
            case 31975:
            case 35511:
            case 36984:
            case 38859:
            case 38914:
            case 39182:
            case 49000:
            case 49001:
                caster->castSpell(target, 53353, true);
                break;

            // SPELL_HASH_VIPER_STING:
            case 3034:
            case 31407:
            case 37551:
            case 39413:
            case 65881:
            case 67991:
            case 67992:
            case 67993:
                caster->castSpell(target, 53358, true);
                break;
        }
    }
};

bool ChimeraShot(uint8_t /*effectIndex*/, Spell *spell)
{
    Unit *target = spell->getUnitTarget();

    HasNameHash condition;
    ChimeraShotAction action;

    //SPELL_HASH_SCORPID_STING
    condition.AddSpellIdToCheck(3043);
    condition.AddSpellIdToCheck(18545);
    condition.AddSpellIdToCheck(52604);

    //SPELL_HASH_WYVERN_STING
    condition.AddSpellIdToCheck(19386);
    condition.AddSpellIdToCheck(24131);
    condition.AddSpellIdToCheck(24132);
    condition.AddSpellIdToCheck(24133);
    condition.AddSpellIdToCheck(24134);
    condition.AddSpellIdToCheck(24135);
    condition.AddSpellIdToCheck(24335);
    condition.AddSpellIdToCheck(24336);
    condition.AddSpellIdToCheck(26180);
    condition.AddSpellIdToCheck(26233);
    condition.AddSpellIdToCheck(26748);
    condition.AddSpellIdToCheck(27068);
    condition.AddSpellIdToCheck(27069);
    condition.AddSpellIdToCheck(41186);
    condition.AddSpellIdToCheck(49009);
    condition.AddSpellIdToCheck(49010);
    condition.AddSpellIdToCheck(49011);
    condition.AddSpellIdToCheck(49012);
    condition.AddSpellIdToCheck(65877);
    condition.AddSpellIdToCheck(65878);

    //SPELL_HASH_SERPENT_STING
    condition.AddSpellIdToCheck(1978);
    condition.AddSpellIdToCheck(13549);
    condition.AddSpellIdToCheck(13550);
    condition.AddSpellIdToCheck(13551);
    condition.AddSpellIdToCheck(13552);
    condition.AddSpellIdToCheck(13553);
    condition.AddSpellIdToCheck(13554);
    condition.AddSpellIdToCheck(13555);
    condition.AddSpellIdToCheck(25295);
    condition.AddSpellIdToCheck(27016);
    condition.AddSpellIdToCheck(31975);
    condition.AddSpellIdToCheck(35511);
    condition.AddSpellIdToCheck(36984);
    condition.AddSpellIdToCheck(38859);
    condition.AddSpellIdToCheck(38914);
    condition.AddSpellIdToCheck(39182);
    condition.AddSpellIdToCheck(49000);
    condition.AddSpellIdToCheck(49001);

    //SPELL_HASH_VIPER_STING
    condition.AddSpellIdToCheck(3034);
    condition.AddSpellIdToCheck(31407);
    condition.AddSpellIdToCheck(37551);
    condition.AddSpellIdToCheck(39413);
    condition.AddSpellIdToCheck(65881);
    condition.AddSpellIdToCheck(67991);
    condition.AddSpellIdToCheck(67992);
    condition.AddSpellIdToCheck(67993);

    target->auraActionIf(&action, &condition);

    return true;
}

void SetupLegacyHunterSpells(ScriptMgr* mgr)
{
    mgr->register_dummy_spell(53209, &ChimeraShot);
    mgr->register_dummy_spell(24531, &Refocus);
    mgr->register_dummy_spell(23989, &Readiness);
    mgr->register_dummy_spell(53271, &MastersCall);
    mgr->register_dummy_aura(19574, &BestialWrath);
    mgr->register_dummy_aura(34471, &TheBeastWithin);
    mgr->register_dummy_aura(34477, &Misdirection);

    uint32_t explosiveshotids[] =
    {
        53301,
        60051,
        60052,
        60053,
        0
    };
    mgr->register_dummy_aura(explosiveshotids, &ExplosiveShot);
}
