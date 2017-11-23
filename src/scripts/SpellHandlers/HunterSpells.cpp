/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2007 Moon++ <http://www.moonplusplus.info/>
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
#include <Spell/Definitions/SpellMechanics.h>
#include <Units/Creatures/Pet.h>

bool Refocus(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* playerTarget = pSpell->GetPlayerTarget();
    if (playerTarget == 0) return true;

    SpellSet::const_iterator itr = playerTarget->mSpells.begin();
    for (; itr != playerTarget->mSpells.end(); ++itr)
    {
        if ((*itr) == 24531)       // skip calling spell.. otherwise spammies! :D
            continue;

        if ((*itr) == 19434 || (*itr) == 20900 || (*itr) == 20901 || (*itr) == 20902 || (*itr) == 20903 || (*itr) == 20904 || (*itr) == 27632
            || (*itr) == 2643 || (*itr) == 14288 || (*itr) == 14289 || (*itr) == 14290 || (*itr) == 25294 || (*itr) == 14443 || (*itr) == 18651 || (*itr) == 20735 || (*itr) == 21390
            || (*itr) == 1510 || (*itr) == 14294 || (*itr) == 14295 || (*itr) == 1540 || (*itr) == 22908
            || (*itr) == 3044 || (*itr) == 14281 || (*itr) == 14282 || (*itr) == 14283 || (*itr) == 14284 || (*itr) == 14285 || (*itr) == 14286 || (*itr) == 14287)
            playerTarget->ClearCooldownForSpell((*itr));
    }
    return true;
}

bool Readiness(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->p_caster)
        return true;
    pSpell->p_caster->ClearCooldownsOnLine(50, pSpell->GetSpellInfo()->getId());//Beast Mastery
    pSpell->p_caster->ClearCooldownsOnLine(163, pSpell->GetSpellInfo()->getId());//Marksmanship
    pSpell->p_caster->ClearCooldownsOnLine(51, pSpell->GetSpellInfo()->getId());//Survival
    return true;
}

bool MastersCall(uint8_t effectIndex, Spell* pSpell)
{
    Player* caster = pSpell->p_caster;

    if (caster == NULL)
        return true;

    Pet* Summon = caster->GetSummon();
    if (Summon == NULL || Summon->IsDead())
        return true;

    switch (effectIndex)
    {
        case 0:
            Summon->CastSpell(caster, pSpell->damage, true);
            return true;
        case 1:
            Summon->CastSpell(Summon, 62305, true);
            return true;
    }

    return true;
}

bool TheBeastWithin(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* m_target = a->GetTarget();

    uint32 mechanics[15] = { MECHANIC_CHARMED, MECHANIC_DISORIENTED,    MECHANIC_DISTRACED, MECHANIC_FLEEING,
                             MECHANIC_ROOTED, MECHANIC_ASLEEP, MECHANIC_ENSNARED, MECHANIC_STUNNED,
                             MECHANIC_FROZEN, MECHANIC_INCAPACIPATED, MECHANIC_POLYMORPHED, MECHANIC_BANISHED,
                             MECHANIC_SEDUCED, MECHANIC_HORRIFIED, MECHANIC_SAPPED
    };

    for (uint32 x = 0; x < 15; x++)
    {
        if (apply)
        {
            m_target->MechanicsDispels[mechanics[x]]++;
            m_target->RemoveAllAurasByMechanic(mechanics[x], 0, false);
        }
        else
            m_target->MechanicsDispels[mechanics[x]]--;
    }

    return true;
}


bool BestialWrath(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* m_target = a->GetTarget();

    uint32 mechanics[15] = { MECHANIC_CHARMED, MECHANIC_DISORIENTED,    MECHANIC_DISTRACED, MECHANIC_FLEEING,
                             MECHANIC_ROOTED, MECHANIC_ASLEEP, MECHANIC_ENSNARED, MECHANIC_STUNNED,
                             MECHANIC_FROZEN, MECHANIC_INCAPACIPATED, MECHANIC_POLYMORPHED, MECHANIC_BANISHED,
                             MECHANIC_SEDUCED, MECHANIC_HORRIFIED, MECHANIC_SAPPED
    };

    for (uint32 x = 0; x < 15; x++)
    {
        if (apply)
        {
            m_target->MechanicsDispels[mechanics[x]]++;
            m_target->RemoveAllAurasByMechanic(mechanics[x], 0, false);
        }
        else
            m_target->MechanicsDispels[mechanics[x]]--;
    }
    return true;
}

bool Misdirection(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Player* caster = a->GetPlayerCaster();

    if (caster == NULL)
        return true;

    if (!apply)
        sEventMgr.AddEvent(caster, &Player::SetMisdirectionTarget, (uint64)0, EVENT_UNK, 250, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    return true;
}

bool ExplosiveShot(uint8_t effectIndex, Aura* a, bool apply)
{
    if (!apply)
        return true;

    Unit* m_target = a->GetTarget();

    a->SetNegative();
    int32 dmg = a->GetModAmount(effectIndex);
    dmg += float2int32(m_target->GetRangedAttackPower() * 0.16f);

    a->EventPeriodicDamage(dmg);

    return true;
}

class HasNameHash : public AuraCondition
{
public:
    bool operator()(Aura *aura)
    {
        uint32 spellId = aura->GetSpellInfo()->getId();

        if (std::find(spellIds.begin(), spellIds.end(), spellId) != spellIds.end())
            return true;
        else
            return false;
    }

    void AddSpellIdToCheck(uint32 hash)
    {
        spellIds.push_back(hash);
    }

private:
    std::vector< uint32 > spellIds;
};

class ChimeraShotAction : public AuraAction
{
public:
    void operator()(Aura *a) {
        uint32 spellId = a->GetSpellInfo()->getId();

        Unit *caster = a->GetUnitCaster();
        Unit *target = a->GetTarget();

        if (caster == NULL)
            return;

        a->Refresh();

        switch (spellId)
        {
            // SPELL_HASH_SCORPID_STING:
            case 3043:
            case 18545:
            case 52604:
                caster->CastSpell(target, 53359, true);
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
                caster->CastSpell(target, 53366, true);
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
                caster->CastSpell(target, 53353, true);
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
                caster->CastSpell(target, 53358, true);
                break;
        }
    }
};

bool ChimeraShot(uint8_t /*effectIndex*/, Spell *spell)
{
    Unit *target = spell->GetUnitTarget();

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

    target->AuraActionIf(&action, &condition);

    return true;
}

void SetupHunterSpells(ScriptMgr* mgr)
{
    mgr->register_dummy_spell(53209, &ChimeraShot);
    mgr->register_dummy_spell(24531, &Refocus);
    mgr->register_dummy_spell(23989, &Readiness);
    mgr->register_dummy_spell(53271, &MastersCall);
    mgr->register_dummy_aura(19574, &BestialWrath);
    mgr->register_dummy_aura(34471, &TheBeastWithin);
    mgr->register_dummy_aura(34477, &Misdirection);

    uint32 explosiveshotids[] =
    {
        53301,
        60051,
        60052,
        60053,
        0
    };
    mgr->register_dummy_aura(explosiveshotids, &ExplosiveShot);
}
