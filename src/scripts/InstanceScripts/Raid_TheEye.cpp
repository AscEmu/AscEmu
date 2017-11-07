/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
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

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Objects/Faction.h"


// APPRENTICE STAR SCRYER
const uint32 CN_A_STAR_SCRYER = 20043;

const uint32 ARCANE_VOLLEY = 40424;
const uint32 A_STAR_ARCANE_EXPLOSION = 27082;

class AStarScryerAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(AStarScryerAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        AStarScryerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(ARCANE_VOLLEY);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(A_STAR_ARCANE_EXPLOSION);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 5;
            spells[1].perctrigger = 50.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = _unit->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                _unit->CastSpell(_unit, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                _unit->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;
                for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(_unit, (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(_unit, (*itr)) && (*itr) != _unit)) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && _unit->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && _unit->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (_unit->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(_unit, RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (_unit->GetHealthPct() >= minhp2cast && _unit->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(_unit);

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:
        uint8 nrspells;
};

// STAR SCRYER
const uint32 CN_STAR_SCRYER = 20034;

const uint32 STARFALL = 37124;

class StarScryerAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(StarScryerAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        StarScryerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(STARFALL);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = true;
            spells[0].cooldown = 12;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = _unit->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                _unit->CastSpell(_unit, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                _unit->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;
                for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(_unit, (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(_unit, (*itr)) && (*itr) != _unit)) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && _unit->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && _unit->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (_unit->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(_unit, RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (_unit->GetHealthPct() >= minhp2cast && _unit->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(_unit);

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:

        uint8 nrspells;
};

// ASTROMANCER
const uint32 CN_ASTROMANCER = 20033;

const uint32 ASTROMANCER_FIREBALL = 36971;
const uint32 ASTROMANCER_CONFLAGRATION = 37018;
const uint32 ASTROMANCER_ARCANE_BURST = 36970;

class AstromancerAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(AstromancerAI, MoonScriptCreatureAI);
        AstromancerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            //spells
            mArcaneBurst = AddSpell(ASTROMANCER_ARCANE_BURST, Target_Self, 0, 0, 0, 0, 10);
            AddSpell(ASTROMANCER_FIREBALL, Target_Current, 75, 2, 0, 0, 35);
            AddSpell(ASTROMANCER_CONFLAGRATION, Target_RandomUnit, 25, 0, 0, 0, 30);

            mArcaneBurstTimer = -1;
            SetAIUpdateFreq(250);
        }

        void AIUpdate()
        {
            if (!_isCasting())
            {
                if (mArcaneBurstTimer == -1 || _isTimerFinished(mArcaneBurstTimer))
                {
                    Unit* unit = GetBestUnitTarget(TargetFilter_Closest);
                    if (unit && getRangeToObject(unit) <= 10.0f)
                    {
                        CastSpellNowNoScheduling(mArcaneBurst);
                        if (mArcaneBurstTimer == -1)
                            mArcaneBurstTimer = _addTimer(6000);
                        else
                            _resetTimer(mArcaneBurstTimer, 6000);
                        ParentClass::AIUpdate();
                        return;
                    }
                }
            }

            ParentClass::AIUpdate();
        }

        SpellDesc* mArcaneBurst;
        int32 mArcaneBurstTimer;
};


// ASTROMANCER LORD
const uint32 CN_ASTROMANCER_LORD = 20046;

const uint32 A_LORD_FIREBALL_VOLLEY = 29922;
const uint32 BLAST_WEAVE = 33933;
const uint32 DRAGONS_BREATH = 33043;

class AstromancerLordAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(AstromancerLordAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        AstromancerLordAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(A_LORD_FIREBALL_VOLLEY);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(BLAST_WEAVE);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 7;
            spells[1].perctrigger = 50.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(DRAGONS_BREATH);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = 10;
            spells[2].perctrigger = 50.0f;
            spells[2].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = _unit->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                _unit->CastSpell(_unit, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                _unit->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;
                for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(_unit, (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(_unit, (*itr)) && (*itr) != _unit)) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && _unit->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && _unit->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (_unit->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(_unit, RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (_unit->GetHealthPct() >= minhp2cast && _unit->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(_unit);

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:
        uint8 nrspells;
};

// BLOODWARDER VINDICATOR
const uint32 CN_BLOOD_VINDICATOR = 20032;

const uint32 HAMMER_OF_JUSTICE = 32416;
const uint32 FLASH_HEAL = 38588;

class BloodVindicatorAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(BloodVindicatorAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        BloodVindicatorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(HAMMER_OF_JUSTICE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 10;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(FLASH_HEAL);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = false;
            spells[1].cooldown = 15;
            spells[1].perctrigger = 50.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = _unit->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                _unit->CastSpell(_unit, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                _unit->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;
                for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(_unit, (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(_unit, (*itr)) && (*itr) != _unit)) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && _unit->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && _unit->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (_unit->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(_unit, RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (_unit->GetHealthPct() >= minhp2cast && _unit->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(_unit);

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:
        uint8 nrspells;
};

// BLOODWARDER LEGIONNARE
const uint32 CN_BLOOD_LEGIONNARE = 20031;

const uint32 LEGION_WHIRLWIND = 36981;
const uint32 CLEAVE = 26350;

class BloodLegionnareAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(BloodLegionnareAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        BloodLegionnareAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(LEGION_WHIRLWIND);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(CLEAVE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = 10;
            spells[1].perctrigger = 40.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = _unit->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                _unit->CastSpell(_unit, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                _unit->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;
                for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(_unit, (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(_unit, (*itr)) && (*itr) != _unit)) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && _unit->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && _unit->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (_unit->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(_unit, RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (_unit->GetHealthPct() >= minhp2cast && _unit->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(_unit);

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:
        uint8 nrspells;
};

// BLOODWARDER MARSHAL
const uint32 CN_BLOOD_MARSHAL = 20035;

const uint32 WHIRLWIND = 36981;
const uint32 UPPERCUT = 26007;

class BloodMarshalAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(BloodMarshalAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        BloodMarshalAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(WHIRLWIND);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(UPPERCUT);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = 20;
            spells[1].perctrigger = 50.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = _unit->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                _unit->CastSpell(_unit, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                _unit->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;
                for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(_unit, (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(_unit, (*itr)) && (*itr) != _unit)) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && _unit->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && _unit->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (_unit->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(_unit, RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (_unit->GetHealthPct() >= minhp2cast && _unit->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(_unit);

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:
        uint8 nrspells;
};

// PHOENIX HAWK
const uint32 CN_PHOENIX_HAWK = 20039;

const uint32 MANA_BURN = 25380;
const uint32 HAWK_CHARGE = 16636;

class PhoenixHawkAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(PhoenixHawkAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        PhoenixHawkAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(MANA_BURN);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(HAWK_CHARGE);
            spells[1].targettype = TARGET_RANDOM_SINGLE;
            spells[1].instant = true;
            spells[1].cooldown = 12;
            spells[1].perctrigger = 50.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].mindist2cast = 15;
            spells[1].maxdist2cast = 45;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = _unit->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                _unit->CastSpell(_unit, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                _unit->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;
                for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(_unit, (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(_unit, (*itr)) && (*itr) != _unit)) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && _unit->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && _unit->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (_unit->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(_unit, RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (_unit->GetHealthPct() >= minhp2cast && _unit->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(_unit);

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:
        uint8 nrspells;
};

// CRYSTALCORE SENTINEL
const uint32 CN_CRYSTAL_SENTINEL = 20041;

const uint32 TRAMPLE = 40340;
const uint32 CHARGET_ARCANE_EXPLOSION = 37106;

class CrystalSentinelAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CrystalSentinelAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        CrystalSentinelAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(TRAMPLE);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(CHARGET_ARCANE_EXPLOSION);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = false;
            spells[1].cooldown = 20;
            spells[1].perctrigger = 50.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = _unit->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                _unit->CastSpell(_unit, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                _unit->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;
                for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(_unit, (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(_unit, (*itr)) && (*itr) != _unit)) && (*itr)->IsUnit()) // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && _unit->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && _unit->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (_unit->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(_unit, RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (_unit->GetHealthPct() >= minhp2cast && _unit->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(_unit);

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:
        uint8 nrspells;
};

// CRYSTALCORE MECHANIC
const uint32 CN_CRYSTAL_MECHANIC = 20052;

const uint32 SAW_BLADE = 37123;

class CrystalMechanicAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CrystalMechanicAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        CrystalMechanicAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(SAW_BLADE);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = true;
            spells[0].cooldown = 6;
            spells[0].perctrigger = 100.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].mindist2cast = 0.0f;
            spells[0].maxdist2cast = 40.0f;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = _unit->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                _unit->CastSpell(_unit, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                _unit->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;
                for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(_unit, (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(_unit, (*itr)) && (*itr) != _unit)) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && _unit->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && _unit->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (_unit->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(_unit, RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (_unit->GetHealthPct() >= minhp2cast && _unit->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(_unit);

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:
        uint8 nrspells;
};


//////////////////////////////////////////////////////////////////////////////////////////
//Bosses
//////////////////////////////////////////////////////////////////////////////////////////

//------------------------------------
//    -= Void Reaver =-
//------------------------------------

const uint32 CN_VOID_REAVER = 19516;

const uint32 VOID_REAVER_POUNDING = 34164;
const uint32 VOID_REAVER_ARCANE_ORB = 34190;
const uint32 VOID_REAVER_ARCANE_ORB_TRIGGER = 34172;
const uint32 VOID_REAVER_KNOCK_AWAY = 25778;
const uint32 VOID_REAVER_ENRAGE = 27680; // Needs checking (as it can be wrong [or maybe IS wrong])

class VoidReaverAI : public MoonScriptBossAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(VoidReaverAI, MoonScriptBossAI);
        VoidReaverAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            SpellDesc* pPounding = AddSpell(VOID_REAVER_POUNDING, Target_Self, 100, 0, 12);
            if (pPounding != NULL)
            {
                pPounding->AddEmote("Alternative measure commencing...", CHAT_MSG_MONSTER_YELL, 11218);
                pPounding->AddEmote("Calculating force parameters...", CHAT_MSG_MONSTER_YELL, 11219);
            }

            mArcaneOrb = AddSpell(VOID_REAVER_ARCANE_ORB_TRIGGER, Target_RandomPlayerDestination, 100, 0, 3);
            AddSpell(VOID_REAVER_KNOCK_AWAY, Target_Current, 100, 0, 20, 0, 12);    // 12 is experimental value
            SetEnrageInfo(AddSpell(VOID_REAVER_ENRAGE, Target_Self, 0, 0, 0), 600000);

            AddEmote(Event_OnCombatStart, "Alert, you are marked for extermination!", CHAT_MSG_MONSTER_YELL, 11213);
            AddEmote(Event_OnTargetDied, "Extermination, successful.", CHAT_MSG_MONSTER_YELL, 11215);
            AddEmote(Event_OnTargetDied, "Imbecile life form, no longer functional.", CHAT_MSG_MONSTER_YELL, 11216);
            AddEmote(Event_OnTargetDied, "Threat neutralized.", CHAT_MSG_MONSTER_YELL, 11217);
            AddEmote(Event_OnDied, "Systems... shutting... down...", CHAT_MSG_MONSTER_YELL, 11214);

            mArcaneOrbTimer = INVALIDATE_TIMER;
        }

        void OnCombatStart(Unit* mTarget)
        {
            ParentClass::OnCombatStart(mTarget);
            if (mArcaneOrb != NULL)
            {
                mArcaneOrbTimer = _addTimer(10000);
                mArcaneOrb->mEnabled = false;
            }
        }

        void AIUpdate()
        {
            if (mArcaneOrb != NULL && !mArcaneOrb->mEnabled && _isTimerFinished(mArcaneOrbTimer))
            {
                _removeTimer(mArcaneOrbTimer);
                mArcaneOrb->mEnabled = true;
            }

            ParentClass::AIUpdate();
        }

        uint32 mArcaneOrbTimer;
        SpellDesc* mArcaneOrb;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//High Astromancer Solarian AI Script
//
// Phase timers based on boss mods:
// - Split every 90sec (except first split happens 50sec after engage)
// - 3x4 Solarium Agents spawns 6sec after split
// - Solarian comes back with two Solarium Priest 22sec after split (end of phase 2)
// - Once phase 2 is finished, phase 1 starts again
// - At 20% health, Solarian enter phase 3 until she dies
//
const uint32 CN_SOLARIAN = 18805;
const uint32 CN_SOLARIUMAGENT = 18925;
const uint32 CN_SOLARIUMPRIEST = 18806;
const uint32 CN_SOLARIUM_SPOT_LIGHT = 15631;
const uint32 SOLARIAN_WRATH_OF_THE_ASTROMANCER = 42783;    //Infuses an enemy with Arcane power, causing them to harm nearby allies for 5400 to 6600. Arcane damage after 6 sec.
const uint32 SOLARIAN_WRATH_OF_THE_ASTROMANCER_BOMB = 42787;    //The actual spell that triggers the explosion with arcane damage and slow fall
const uint32 SOLARIAN_ARCANE_MISSILES = 33031;   //Launches magical missiles at an enemy, inflicting Arcane damage each second for 3 sec. Trigger spell (3000 arcane damage)
const uint32 SOLARIAN_BLINDING_LIGHT = 33009;    //Hits everyone in the raid for 2280 to 2520 arcane damage. 20sec cooldown.
const uint32 SOLARIAN_SOLARIANS_TRANSFORM = 39117;    //Transforms into void walker.
const uint32 SOLARIAN_VOID_BOLT = 39329;    //The Void Walker casts this every 10 seconds. It deals 4394 to 5106 shadow damage to the target with the highest aggro.
const uint32 SOLARIAN_PSYCHIC_SCREAM = 34322;    //Fears up to 5 targets in melee range.
const uint32 SOLARIUMPRIEST_GREATER_HEAL = 38580;    //Heals 23125 to 26875 any friendly target
const uint32 SOLARIUMPRIEST_HOLY_SMITE = 31740;   //Deals 553 to 747 holy damage

bool Dummy_Solarian_WrathOfTheAstromancer(uint32 pEffectIndex, Spell* pSpell);
void SpellFunc_Solarian_Disappear(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType);
void SpellFunc_Solarian_Reappear(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType);

class HighAstromancerSolarianAI : public MoonScriptBossAI
{
        MOONSCRIPT_FACTORY_FUNCTION(HighAstromancerSolarianAI, MoonScriptBossAI);
        HighAstromancerSolarianAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            //Initialize timers
            mSplitTimer = mAgentsTimer = mSolarianTimer = INVALIDATE_TIMER;

            //Phase 1 spells
            AddPhaseSpell(1, AddSpell(SOLARIAN_ARCANE_MISSILES, Target_RandomUnit, 60, 3, 0, 0, 45));
            AddPhaseSpell(1, AddSpell(SOLARIAN_WRATH_OF_THE_ASTROMANCER, Target_RandomPlayerNotCurrent, 20, 0, 6, 0, 50000));
            AddPhaseSpell(1, AddSpell(SOLARIAN_BLINDING_LIGHT, Target_Self, 20, 0, 20, 0, 50));
            mDisappear = AddSpellFunc(&SpellFunc_Solarian_Disappear, Target_Self, 0, 22, 0);
            mDisappear->AddEmote("You are hopelessly outmatched!", CHAT_MSG_MONSTER_YELL, 11139);
            mDisappear->AddEmote("I will crush your delusions of grandeur!", CHAT_MSG_MONSTER_YELL, 11140);

            //Phase 2 spells
            mReappear = AddSpellFunc(&SpellFunc_Solarian_Reappear, Target_Self, 0, 0, 0);

            //Phase 3 spells
            AddPhaseSpell(3, AddSpell(SOLARIAN_VOID_BOLT, Target_Current, 100, 3, 10, 0, 100));
            AddPhaseSpell(3, AddSpell(SOLARIAN_PSYCHIC_SCREAM, Target_Self, 10, 0, 0));
            mVoidForm = AddSpell(SOLARIAN_SOLARIANS_TRANSFORM, Target_Self, 0, 0, 0);
            mVoidForm->AddEmote("Enough of this! Now I call upon the fury of the cosmos itself.", CHAT_MSG_MONSTER_YELL);
            mVoidForm->AddEmote("I become ONE... with the VOID!", CHAT_MSG_MONSTER_YELL);

            //Emotes
            AddEmote(Event_OnCombatStart, "Tal anu'men no sin'dorei!", CHAT_MSG_MONSTER_YELL, 11134);
            AddEmote(Event_OnDied, "The warmth of the sun... awaits.", CHAT_MSG_MONSTER_YELL, 11135);
            AddEmote(Event_OnTargetDied, "Your soul belongs to the Abyss!", CHAT_MSG_MONSTER_YELL, 11136);
            AddEmote(Event_OnTargetDied, "By the blood of the Highborne!", CHAT_MSG_MONSTER_YELL, 11137);
            AddEmote(Event_OnTargetDied, "For the Sunwell!", CHAT_MSG_MONSTER_YELL, 11138);
        }

        void OnCombatStart(Unit* pTarget)
        {
            mSplitTimer = _addTimer(50000);    //First split after 50sec
            ParentClass::OnCombatStart(pTarget);
        }

        void AIUpdate()
        {
            if (GetPhase() == 1)
            {
                if (_getHealthPercent() <= 20 && !_isCasting())
                {
                    SetPhase(3, mVoidForm);
                    _cancelAllTimers();
                }
                else if (_isTimerFinished(mSplitTimer) && !_isCasting())
                {
                    SetPhase(2, mDisappear);
                    _resetTimer(mSplitTimer, 90000);        //Next split in 90sec
                    mAgentsTimer = _addTimer(6000);        //Agents spawns 6sec after the split
                    mSolarianTimer = _addTimer(22000);    //Solarian with 2 priests spawns 22sec after split
                }
            }
            else if (GetPhase() == 2)
            {
                if (_isTimerFinished(mSolarianTimer) && !_isCasting())
                {
                    SetPhase(1, mReappear);
                    _removeTimer(mSolarianTimer);
                }
                else if (_isTimerFinished(mAgentsTimer) && !_isCasting())
                {
                    for (uint8 SpawnIter = 0; SpawnIter < 4; SpawnIter++)
                    {
                        SpawnCreature(CN_SOLARIUMAGENT, mSpawnPositions[0][0], mSpawnPositions[0][1], 17, 0, true);
                        SpawnCreature(CN_SOLARIUMAGENT, mSpawnPositions[1][0], mSpawnPositions[1][1], 17, 0, true);
                        SpawnCreature(CN_SOLARIUMAGENT, mSpawnPositions[2][0], mSpawnPositions[2][1], 17, 0, true);
                    }
                    _removeTimer(mAgentsTimer);
                }
            }
            ParentClass::AIUpdate();
        }

        SpellDesc* mVoidForm;
        SpellDesc* mDisappear;
        SpellDesc* mReappear;
        int32 mSplitTimer;
        uint32 mAgentsTimer;
        uint32 mSolarianTimer;
        float mSpawnPositions[3][2];
};

bool Dummy_Solarian_WrathOfTheAstromancer(uint32 pEffectIndex, Spell* pSpell)
{
    Unit* Caster = pSpell->u_caster;
    if (!Caster) return true;

    Unit* Target = Caster->GetAIInterface()->getNextTarget();
    if (!Target) return true;

    SpellInfo* SpellInfo = sSpellCustomizations.GetSpellInfo(SOLARIAN_WRATH_OF_THE_ASTROMANCER_BOMB);
    if (!SpellInfo) return true;

    //Explode bomb after 6sec
    sEventMgr.AddEvent(Target, &Unit::EventCastSpell, Target, SpellInfo, EVENT_UNK, 6000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    return true;
}

void SpellFunc_Solarian_Disappear(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType)
{
    HighAstromancerSolarianAI* Solarian = (pCreatureAI) ? static_cast< HighAstromancerSolarianAI* >(pCreatureAI) : NULL;
    if (Solarian)
    {
        SpellFunc_Disappear(pThis, pCreatureAI, pTarget, pType);

        //Spawn spot lights, and despawn them after 26sec X(400,460) Y(-340,-400)
        Solarian->mSpawnPositions[0][0] = 400 + RandomFloat(60);
        Solarian->mSpawnPositions[0][1] = -400 + RandomFloat(60);
        Solarian->SpawnCreature(CN_SOLARIUM_SPOT_LIGHT, Solarian->mSpawnPositions[0][0], Solarian->mSpawnPositions[0][1], 17)->despawn(26000);
        Solarian->mSpawnPositions[1][0] = 400 + RandomFloat(60);
        Solarian->mSpawnPositions[1][1] = -400 + RandomFloat(60);
        Solarian->SpawnCreature(CN_SOLARIUM_SPOT_LIGHT, Solarian->mSpawnPositions[1][0], Solarian->mSpawnPositions[1][1], 17)->despawn(26000);
        Solarian->mSpawnPositions[2][0] = 400 + RandomFloat(60);
        Solarian->mSpawnPositions[2][1] = -400 + RandomFloat(60);
        Solarian->SpawnCreature(CN_SOLARIUM_SPOT_LIGHT, Solarian->mSpawnPositions[2][0], Solarian->mSpawnPositions[2][1], 17)->despawn(26000);
    }
}

void SpellFunc_Solarian_Reappear(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType)
{
    HighAstromancerSolarianAI* Solarian = (pCreatureAI) ? static_cast< HighAstromancerSolarianAI* >(pCreatureAI) : NULL;
    if (Solarian)
    {
        //Spawn two priest friend to help Solarian
        Solarian->SpawnCreature(CN_SOLARIUMPRIEST, Solarian->mSpawnPositions[0][0], Solarian->mSpawnPositions[0][1], 17);
        Solarian->SpawnCreature(CN_SOLARIUMPRIEST, Solarian->mSpawnPositions[1][0], Solarian->mSpawnPositions[1][1], 17);
        //Solarian->MoveTo(Solarian->mSpawnPositions[2][0], Solarian->mSpawnPositions[2][1], 17);    //Doesn't work quite right yet

        SpellFunc_Reappear(pThis, pCreatureAI, pTarget, pType);
    }
}

class SolariumAgentAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(SolariumAgentAI, MoonScriptCreatureAI);
        SolariumAgentAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            _setDespawnWhenInactive(true);    //despawn creature if it gets out of combat or dead
        }

        void OnLoad()
        {
            AggroNearestUnit(); //Aggro on spawn
        }
};

class SolariumPriestAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(SolariumPriestAI, MoonScriptCreatureAI);
        SolariumPriestAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(SOLARIUMPRIEST_GREATER_HEAL, Target_WoundedFriendly, 20, 2, 0, 0, 40);
            AddSpell(SOLARIUMPRIEST_HOLY_SMITE, Target_Current, 80, 2.5f, 0, 0, 40);
            _setDespawnWhenInactive(true);    //despawn creature if it gets out of combat or dead
        }

        void OnLoad()
        {
            AggroNearestUnit(); //Aggro on spawn
        }
};

class SolariumSpotLight : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(SolariumSpotLight, MoonScriptCreatureAI);
        SolariumSpotLight(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            setCanEnterCombat(false);
        }
};

// Al'ar AI

class AlarAuxClass: public Object
{
    public:
        AlarAuxClass(CreatureAIScript*);
        ~AlarAuxClass();
        void Rebirth();

    protected:
        CreatureAIScript* alar;
};


const uint32 CN_ALAR = 19514;

// Phase1 spells
const uint32 FLAME_BUFFET = 34121;   // casted only when there is no player in melee range!
const uint32 FLAME_QUILLS = 34316;    // it flies to a high point in a center of room and use this on players located on platform    // also can be many others (Idk even if this spell works =S)
const uint32 SUMMON_PHOENIX_ADDS = 18814;    // Summons 3 phoenixs

// Phase2 spells
const uint32 FLAME_PATCH = 35383;    // 35383, 35380;
const uint32 METEOR = 35181;       // but shouldn't be instant imho --Dive Bomb 35367
//const uint32 EMBER_BLAST  = 34133;    // used when one of adds has low health
const uint32 MELT_ARMOR = 35410; // maybe they are used, but not sure
//const uint32 RANDOM_CHARGE

// Other spells
const uint32 REBIRTH = 34342;

static Movement::Location fly[] =
{
    {  },
    { 337.864868f,  65.702301f, 33.171944f, 1.329919f },    // fly 1 to ...
    { 391.245148f,  35.472462f, 36.886353f, 0.822089f },
    { 392.959869f, -34.031734f, 33.630096f, 3.342431f },
    { 321.277008f, -70.101540f, 43.650482f, 3.844443f },
    { 256.202911f,  -1.549352f, 44.718426f, 0.069512f },    // ... fly 5
    { 342.090088f,  55.661064f, 27.638260f, 0.739368f },    // attack/tank positions from phase 1 from here to ...
    { 392.815369f,  31.636963f, 25.414761f, 0.551340f },
    { 388.397308f, -38.834595f, 22.335297f, 5.702067f },
    { 333.922229f, -60.645069f, 24.484278f, 1.454599f },    // ... to here
    { 328.103455f,  -0.192393f, 50.216309f, 4.188326f },    // fire quills cast position    // EMOTE_STATE_WHIRLWIND = 382,
    { 326.225647f,   2.381837f, -2.389485f, 4.877070f }    // center of the room which is used in phase 2
};

class AlarAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(AlarAI);
        SP_AI_Spell spells[7];
        bool m_spellcheck[7];

        AlarAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Waypoints
            for (uint8 i = 1; i < 12; i++)
                AddWaypoint(CreateWaypoint(i, 0, Movement::WP_MOVE_TYPE_FLY, fly[i]));

            // Spells
            nrspells = 7;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(FLAME_BUFFET);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].cooldown = 4;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(FLAME_QUILLS);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 3;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SUMMON_PHOENIX_ADDS);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = true;
            spells[2].cooldown = 5;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(FLAME_PATCH);
            spells[3].targettype = TARGET_VARIOUS;
            spells[3].instant = true;
            spells[3].cooldown = 10;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(METEOR);
            spells[4].targettype = TARGET_ATTACKING;
            spells[4].instant = true;    // =(
            spells[4].cooldown = 30;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;

            spells[5].info = sSpellCustomizations.GetSpellInfo(MELT_ARMOR);
            spells[5].targettype = TARGET_ATTACKING;
            spells[5].instant = true;
            spells[5].cooldown = 60;
            spells[5].perctrigger = 0.0f;
            spells[5].attackstoptimer = 1000;

            spells[6].info = sSpellCustomizations.GetSpellInfo(REBIRTH);
            spells[6].targettype = TARGET_SELF;
            spells[6].instant = false;
            spells[6].cooldown = -1;
            spells[6].perctrigger = 0.0f;
            spells[6].attackstoptimer = 1000;

            // Additional Settings

            _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
            _unit->GetAIInterface()->setSplineFlying();

            FlameQuills = false;
            Meteor = false;
            SetPhase(0);
            nDeath = 0;
            timer = lasttime = 0;
            _unit->GetAIInterface()->setOutOfCombatRange(200000);

            _unit->GetAIInterface()->setSplineFlying();
            _unit->GetAIInterface()->StopMovement(0);
            _unit->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
            _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
            _unit->GetAIInterface()->setWayPointToMove(1);
            Flying = true;
            CastTime();

            QuillsCount = 0;
            NextWP = 0;
            m_entry = 0;
            FlyWay = 0;
        }

        void OnDamageTaken(Unit* mAttacker, uint32 fAmount)
        {
            if (!_unit->event_HasEvents())
            {
                RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
                ModifyAIUpdateEvent(1000);
                NextWP = 6;
            }
        }

        void OnCombatStart(Unit* mTarget)
        {
            FlameQuills = false;
            Meteor = false;
            NextWP = 6;

            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            ModifyAIUpdateEvent(1000);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void SetPhase(uint32 ph)
        {
            Phase = ph;
            _unit->SetBaseMana(Phase);
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();

            FlameQuills = false;
            Meteor = false;
            SetPhase(0);

            RemoveAIUpdateEvent();
        }

        void Rebirth()
        {
            //_unit->SetPosition(fly[11].x, fly[11].y, fly[11].z, fly[11].o);
            SetPhase(2);
            _unit->CastSpell(_unit, spells[6].info, spells[6].instant);
            _unit->setUInt64Value(UNIT_FIELD_HEALTH, _unit->getUInt32Value(UNIT_FIELD_MAXHEALTH));
            _unit->setDeathState(ALIVE);
            _unit->RemoveFlag(UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_TAGGED_BY_OTHER);
            _unit->SetFlag(UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_LOOTABLE);
            // no idea what this tag means, smth loot/party related
            //_unit->ClearTag();
            _unit->GetAIInterface()->StopMovement(0); // after respawn monster can move

            _unit->GetAIInterface()->WipeTargetList();
            _unit->GetAIInterface()->WipeHateList();
            _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
            _unit->GetAIInterface()->m_canMove = true;
            Flying = false;
            lasttime = timer;
        }

        void OnDied(Unit* mKiller)
        {
            nDeath++;
            SetPhase(0);
            nDeath = 0;
            FlameQuills = false;
            Meteor = false;
            CastTime();
        }

        bool HostileInMeleeRange()
        {
            for (Object::InRangeSet::iterator i = _unit->GetInRangeSetBegin(); i != _unit->GetInRangeSetEnd(); ++i)
            {
                if (isHostile(_unit, (*i)) && _unit->GetDistance2dSq((*i)) < 225)
                {
                    return true;
                }
            }
            return false;
        }

        void UPCastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                if (spells[i].casttime > 0) spells[i].casttime--;
        }

        void AIUpdate()
        {
            UPCastTime();
            if (!HostileInMeleeRange() && spells[0].casttime == 0 && !FlameQuills && !Flying)
            {
                _unit->CastSpell(_unit, spells[0].info, spells[0].instant);
                spells[0].casttime = spells[0].cooldown;
            }
            if (!Flying) timer++;

            switch (Phase)
            {
                case 0:
                    return;
                case 1:
                    {
                        PhaseOne();
                    }
                    break;
                case 2:
                    {
                        PhaseTwo();
                    }
                    break;
                default:
                    {
                        SetPhase(0);
                    }
            }
        }

        void PhaseOne()
        {
            if (FlameQuills == true)
            {

                //_unit->CastSpell(_unit, sSpellCustomizations.GetSpellInfo(34229), true);
                if (lasttime + 11 == timer)
                {
                    _unit->CastSpellAoF(LocationVector(fly[1].x, fly[1].y, fly[1].z), spells[1].info, spells[1].instant);
                    _unit->CastSpellAoF(LocationVector(fly[2].x, fly[2].y, fly[2].z), spells[1].info, spells[1].instant);
                    _unit->CastSpellAoF(LocationVector(fly[3].x, fly[3].y, fly[3].z), spells[1].info, spells[1].instant);
                    _unit->CastSpellAoF(LocationVector(fly[4].x, fly[4].y, fly[4].z), spells[1].info, spells[1].instant);
                    _unit->GetAIInterface()->m_canMove = true;
                    _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    _unit->GetAIInterface()->setWayPointToMove(NextWP);
                    FlameQuills = false;
                    Flying = true;
                    lasttime = timer;
                }
            }
            else if (lasttime + 35 == timer)
            {
                spawnCreature(19551, _unit->GetPosition());
                //_unit->CastSpell(_unit, spells[2].info, spells[2].instant);
                lasttime = timer + RandomUInt(9);
                _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
                setAIAgent(AGENT_NULL);
                _unit->GetAIInterface()->StopMovement(0);
                _unit->GetAIInterface()->m_canMove = true;

                _unit->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                _unit->GetAIInterface()->setWayPointToMove(NextWP);
                // ugly code, trows compile error if left just null, this should do it ~ azolex
                uint32 nullfix = 0;
                _unit->GetAIInterface()->setNextTarget(nullfix);
                Flying = true;
            }
        }

        void PhaseTwo()
        {
            CastSpell(5);
            CastSpell(4);
            if ((RandomUInt(1, 100)) < 2)
            {
                Unit* target = GetRandomTarget();
                Creature* patch;
                if (target != NULL)
                {
                    auto pos = target->GetPosition();
                    patch = spawnCreature(20602, pos.x, pos.y, pos.z, 0);
                    if (patch != NULL)
                    {
                        patch->SetDisplayId(16946);
                        patch->SetNativeDisplayId(16946);
                        patch->SetFaction(_unit->GetFaction());
                    }
                }
            }
            if (Meteor == true)
            {
            }
            if (lasttime + 35 == timer)
            {
                spawnCreature(19551, _unit->GetPosition());
                spawnCreature(19551, _unit->GetPosition());
                lasttime = timer;
            }
        }

        Unit* GetRandomTarget()
        {
            for (Object::InRangeSet::iterator i = _unit->GetInRangeSetBegin(); i != _unit->GetInRangeSetEnd(); ++i)
            {
                if (isHostile(_unit, (*i)) && (*i)->GetInstanceID() == _unit->GetInstanceID())
                {
                    Unit* RandomTarget = static_cast<Unit*>(*i);

                    if (RandomTarget->isAlive())
                        return RandomTarget;
                }
            }
            return NULL;
        }

        void CastSpell(int id)
        {
            if (spells[id].casttime > 0) return;
            if (_unit->GetCurrentSpell() != NULL) return;

            Unit* target = NULL;
            target = _unit->GetAIInterface()->getNextTarget();
            switch (spells[id].targettype)
            {
                case TARGET_SELF:
                case TARGET_VARIOUS:
                    _unit->CastSpell(_unit, spells[id].info, spells[id].instant);
                    break;
                case TARGET_ATTACKING:
                    _unit->CastSpell(target, spells[id].info, spells[id].instant);
                    break;
                case TARGET_DESTINATION:
                    _unit->CastSpellAoF(target->GetPosition(), spells[id].info, spells[id].instant);
                    break;
                case TARGET_RANDOM_SINGLE:
                    {
                        target = GetRandomTarget();
                        if (target != NULL)
                            _unit->CastSpell(target, spells[id].info, spells[id].instant);
                    }
                    break;
            }
            spells[id].casttime = spells[id].cooldown;
        }

        void SetNextWP(uint32 wp)
        {
            _unit->GetAIInterface()->setAiState(AI_STATE_ATTACKING);//STATE_IDLE
            NextWP = wp;
            Flying = false;
            _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
            _unit->GetAIInterface()->m_canMove = false;
            lasttime = timer;
        }

        void OnReachWP(uint32 iWaypointId, bool bForwards)
        {

            if (Phase == 0)
            {
                if (NextWP == 6) Phase = 1;
                else NextWP = iWaypointId % 5 + 1;
                _unit->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                _unit->GetAIInterface()->setWayPointToMove(NextWP);
            }

            switch (iWaypointId)
            {
                case 6:
                    {
                        SetNextWP(7);
                    }
                    break;

                case 7:
                    {
                        SetNextWP(8);
                    }
                    break;
                case 8:
                    {
                        SetNextWP(9);
                    }
                    break;
                case 9:
                    {
                        SetNextWP(10);
                    }
                    break;

                case 10:
                    {
                        if (Phase == 1)
                        {
                            FlameQuills = true;
                            QuillsCount = 0;
                            Flying = false;
                            _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                            _unit->GetAIInterface()->m_canMove = false;
                            NextWP = 6;
                            lasttime = timer;
                            //_unit->Emote(EMOTE_STATE_WHIRLWIND);
                            _unit->CastSpell(_unit, sSpellCustomizations.GetSpellInfo(34229), true);
                        }

                        if (Phase == 2)
                        {
                            Meteor = true;
                        }

                    }
                    break;
                default:
                    break;
            }
        }

    protected:

        bool FlameQuills;
        uint32 QuillsCount;
        bool Meteor;
        uint32 NextWP;
        uint32 m_entry;
        uint32 FlyWay;
        uint32 Phase;
        uint32 nDeath;
        uint8 nrspells;
        uint32 timer, lasttime;
        bool Flying;
};

const uint32 CN_EMBEROFALAR = 19551;

class EmberAlarAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(EmberAlarAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        EmberAlarAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(34341);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        Unit* GetAlar()
        {
            for (Object::InRangeSet::iterator i = _unit->GetInRangeSetBegin(); i != _unit->GetInRangeSetEnd(); ++i)
            {
                if ((*i)->IsCreature() && _unit->GetDistance2dSq((*i)) < 400000)
                {
                    Creature* creature = static_cast<Creature*>((*i));
                    if (creature->GetEntry() == 19514 && (*i)->GetInstanceID() == _unit->GetInstanceID())
                    {
                        return creature;
                    }
                }
            }
            return NULL;
        }

        void OnDied(Unit* mKiller)
        {

            _unit->CastSpell(_unit, spells[0].info, spells[0].instant);
            CastTime();

            Unit* Alar = NULL;
            //Alar=getNearestCreature(_unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), 19514);

            Alar = GetAlar();
            if (Alar == NULL)
            {
                return;
            }

            uint32 Phase = Alar->GetBaseMana();
            if (Phase == 2)
            {
                uint32 maxhp = Alar->getUInt32Value(UNIT_FIELD_MAXHEALTH);
                Alar->DealDamage(Alar, (maxhp * 3) / 100, 0, 0, 0);
            }
        }

    protected:
        uint8 nrspells;
};


const uint32 CN_PATCHALAR = 20602;

class PatchAlarAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(PatchAlarAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        PatchAlarAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(35380);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 1000;

            //_unit->GetAIInterface()->SetAllowedToEnterCombat(false);
            //_unit->GetAIInterface()->SetAIState(STATE_IDLE);
            //_unit->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            _unit->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
            _unit->GetAIInterface()->m_canMove = false;
            _unit->Despawn(120000, 0);
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            _unit->CastSpell(_unit, spells[0].info, true);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }


        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

    protected:
        uint8 nrspells;
};

/*
    AlarAuxClass::AlarAuxClass (CreatureAIScript *al)
    {
        alar=al;
        sEventMgr.AddEvent(al, &AlarAI::Rebirth, EVENT_CORPSE_DESPAWN, 3000, 1,0);
    }

    AlarAuxClass::~AlarAuxClass ()
    {
        sEventMgr.RemoveEvents(alar);
    }

    void AlarAuxClass::Rebirth()
    {
        //((AlarAI *)alar)->Rebirth();
        //delete this;
    }
*/
//-----------------------------------------------------------//
//---------------Kael'thas Encounter Script------------------//
//--------------Rewritten/optimized by M4ksiu----------------//
//-------------------Scripted by Plexor----------------------//
//-----------------------------------------------------------//

// Thaladred the Darkener AI(1st advisor)
const uint32 CN_DARKENER = 20064;
const uint32 DARKENER_PSYCHIC_BLOW = 36966;
const uint32 DARKENER_SILENCE = 29943;

class DarkenerAI : public MoonScriptCreatureAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(DarkenerAI, MoonScriptCreatureAI)
        DarkenerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(DARKENER_PSYCHIC_BLOW, Target_Current, 10, 0, 20);
            AddSpell(DARKENER_SILENCE, Target_Current, 10, 0, 15);
            AddEmote(Event_OnCombatStart, "Prepare yourselves!", CHAT_MSG_MONSTER_YELL, 11203);
            AddEmote(Event_OnDied, "Forgive me, my prince! I have... failed.", CHAT_MSG_MONSTER_YELL, 11204);
            setCanEnterCombat(false);
            mCurrentTarget = NULL;
            mGazeSwitchTimer = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            ParentClass::OnCombatStart(mTarget);
            setCanEnterCombat(true);
            SwitchTarget();

            mGazeSwitchTimer = _addTimer((RandomUInt(4) + 8) * 1000);
        }

        void OnCombatStop(Unit* mTarget)
        {
            ParentClass::OnCombatStop(mTarget);
            mCurrentTarget = NULL;

            if (isAlive())
            {
                setCanEnterCombat(false);
            }
        }

        void OnTargetDied(Unit* mTarget)
        {
            SwitchTarget();
        }

        void AIUpdate()
        {
            if (_isTimerFinished(mGazeSwitchTimer))
            {
                _resetTimer(mGazeSwitchTimer, (RandomUInt(4) + 8) * 1000);
                if (!SwitchTarget())
                    return;
            }

            ParentClass::AIUpdate();
        }

        bool SwitchTarget()
        {
            mCurrentTarget = GetBestPlayerTarget();
            if (mCurrentTarget == _unit->GetAIInterface()->getNextTarget())
                return true;

            if (mCurrentTarget != NULL)
            {
                _unit->GetAIInterface()->modThreatByPtr(mCurrentTarget, 1000000);
                Player* pPlayer = static_cast<Player*>(mCurrentTarget);
                char msg[256];
                snprintf((char*)msg, 256, "%s sets eyes on %s", _unit->GetCreatureProperties()->Name.c_str(), pPlayer->GetName());
                _unit->SendChatMessageAlternateEntry(CN_DARKENER, CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, msg);
                return true;
            }

            _wipeHateList();
            return false;
        }

        int32 mGazeSwitchTimer;
        Unit* mCurrentTarget;
};

// Lord Sanguinar AI (2nd advisor)
const uint32 CN_SANGUINAR = 20060;
const uint32 SANGUINAR_BELLOWING = 36922;

class SanguinarAI : public MoonScriptCreatureAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(SanguinarAI, MoonScriptCreatureAI)
        SanguinarAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(SANGUINAR_BELLOWING, Target_Self, 100, 0, 30);
            AddEmote(Event_OnCombatStart, "Blood for blood!", CHAT_MSG_MONSTER_YELL, 11152);
            AddEmote(Event_OnDied, "NO! I will... not.", CHAT_MSG_MONSTER_YELL, 11153);
            setCanEnterCombat(false);
        }

        void OnCombatStart(Unit* mTarget)
        {
            ParentClass::OnCombatStart(mTarget);
            setCanEnterCombat(true);
        }

        void OnCombatStop(Unit* mTarget)
        {
            ParentClass::OnCombatStop(mTarget);

            if (isAlive())
            {
                setCanEnterCombat(false);
            }
        }
};

// Grand Astromancer Capernian AI (3rd advisor)
const uint32 CN_CAPERNIAN = 20062;
const uint32 CAPERNIAN_CONFLAGRATION = 37018;
const uint32 CAPERNIAN_FIREBALL = 36971;
const uint32 CAPERNIAN_ARCANE_BURST = 36970;

class CapernianAI : public MoonScriptCreatureAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(CapernianAI, MoonScriptCreatureAI);
        CapernianAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(CAPERNIAN_CONFLAGRATION, Target_RandomPlayer, 7, 0, 10, 0, 30, true);
            AddSpell(CAPERNIAN_FIREBALL, Target_Current, 73, 2, 0);
            mArcaneBurst = AddSpell(CAPERNIAN_ARCANE_BURST, Target_Self, 0, 1, 15);
            AddEmote(Event_OnCombatStart, "The sin'dorei reign supreme!", CHAT_MSG_MONSTER_YELL, 11117);
            AddEmote(Event_OnDied, "This is not over!", CHAT_MSG_MONSTER_YELL, 11118);
            setCanEnterCombat(false);
        }

        void OnCombatStart(Unit* mTarget)
        {
            ParentClass::OnCombatStart(mTarget);
            setCanEnterCombat(true);

            if (getRangeToObject(mTarget) <= 30.0f)
            {
                setAIAgent(AGENT_SPELL);
                setRooted(true);
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            ParentClass::OnCombatStop(mTarget);

            if (isAlive())
            {
                setCanEnterCombat(false);
            }
        }

        void AIUpdate()
        {
            setAIAgent(AGENT_NULL);
            setRooted(false);
            Unit* pClosestTarget = GetBestPlayerTarget(TargetFilter_Closest);
            if (pClosestTarget != NULL && getRangeToObject(pClosestTarget) <= 6.0f)
            {
                CastSpellNowNoScheduling(mArcaneBurst);
            }

            Unit* pTarget = _unit->GetAIInterface()->getNextTarget();
            if (pTarget != NULL && getRangeToObject(pTarget) <= 30.0f)
            {
                ParentClass::AIUpdate();
                if (getAIAgent() != AGENT_SPELL)
                {
                    setAIAgent(AGENT_SPELL);
                    setRooted(true);
                }
            }
        }

        SpellDesc* mArcaneBurst;
};

// Master Engineer Telonicus AI (4th advisor)
const uint32 CN_TELONICUS = 20063;
const uint32 TELONICUS_BOMB = 37036;
const uint32 TELONICUS_REMOTE_TOY = 37027;    // doesn't seems to work like it should

class TelonicusAI : public MoonScriptCreatureAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(TelonicusAI, MoonScriptCreatureAI);
        TelonicusAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(TELONICUS_BOMB, Target_RandomPlayerDestination, 10, 1.5f, 15, 0, 30);
            AddSpell(TELONICUS_REMOTE_TOY, Target_RandomPlayer, 10, 0, 15, 0, 30);
            AddEmote(Event_OnCombatStart, "Anar'alah belore!", CHAT_MSG_MONSTER_YELL, 11157);
            AddEmote(Event_OnDied, "More perlis... await.", CHAT_MSG_MONSTER_YELL, 11158);            // not sure
            setCanEnterCombat(false);
        }

        void OnCombatStart(Unit* mTarget)
        {
            ParentClass::OnCombatStart(mTarget);
            setCanEnterCombat(true);
        }

        void OnCombatStop(Unit* mTarget)
        {
            ParentClass::OnCombatStop(mTarget);

            if (isAlive())
            {
                setCanEnterCombat(false);
            }
        }
};

// Flame Strike AI
const uint32 CN_FLAME_STRIKE_TRIGGER = 21369;
const uint32 FLAME_STRIKE_TRIGGER_FLAME_STRIKE = 36731;
const uint32 FLAME_STRIKE_TRIGGER_FLAME_STRIKE_EFFECT = 36730;

class FlameStrikeAI : public MoonScriptCreatureAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(FlameStrikeAI, MoonScriptCreatureAI);
        FlameStrikeAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            _applyAura(FLAME_STRIKE_TRIGGER_FLAME_STRIKE_EFFECT);
            RegisterAIUpdateEvent(5000);
            setCanEnterCombat(false);
            _setMeleeDisabled(false);
            setRooted(true);
            _unit->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void OnDied(Unit* mKiller)
        {
            ParentClass::OnDied(mKiller);
            _removeAura(FLAME_STRIKE_TRIGGER_FLAME_STRIKE_EFFECT);
            despawn(500);
        }

        void AIUpdate()
        {
            _unit->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
            _applyAura(FLAME_STRIKE_TRIGGER_FLAME_STRIKE);
            RemoveAIUpdateEvent();
            despawn(8500);
        }
};

// Phoenix AI
const uint32 CN_PHOENIX = 21362;
const uint32 PHOENIX_BURN = 36721;
const uint32 PHOENIX_REBIRTH = 35369;        // used as instant cast - but it does not show animation now (maybe it would be good to move it to trigger?)

class PhoenixAI : public MoonScriptCreatureAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(PhoenixAI, MoonScriptCreatureAI);
        PhoenixAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            RegisterAIUpdateEvent(1000);
            Unit* pTarget = GetBestPlayerTarget();
            if (pTarget != NULL)
            {
                _unit->GetAIInterface()->AttackReaction(pTarget, 500, 0);
            }

            mBurnTimer = _addTimer(3000);
        }

        void OnCombatStart(Unit* mTarget) {}

        void OnCombatStop(Unit* pTarget) {}

        void OnTargetDied(Unit* mTarget)
        {
            ParentClass::OnTargetDied(mTarget);
            Unit* pTarget = GetBestPlayerTarget(TargetFilter_Closest);
            if (pTarget != NULL)
            {
                _unit->GetAIInterface()->AttackReaction(pTarget, 500);
                ParentClass::OnCombatStart(pTarget);
            }
            else
            {
                despawn(1, 0);
            }
        }

        void OnDied(Unit* mKiller)
        {
            ParentClass::OnDied(mKiller);
            _applyAura(PHOENIX_REBIRTH);
            SpawnCreature(21364, _unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), _unit->GetOrientation(), false);
            despawn(500);
        }

        void AIUpdate()
        {
            double CurrentHP = (double)_unit->getUInt32Value(UNIT_FIELD_HEALTH);
            double PercMaxHP = (double)_unit->getUInt32Value(UNIT_FIELD_MAXHEALTH) * 0.05;
            if (CurrentHP > PercMaxHP && _isTimerFinished(mBurnTimer))
            {
                _unit->SetHealth((uint32)(CurrentHP - PercMaxHP));
                _resetTimer(mBurnTimer, 3000);
                _applyAura(PHOENIX_BURN);
            }
            else if (CurrentHP <= PercMaxHP)
            {
                SpawnCreature(21364, _unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), _unit->GetOrientation(), false);
                despawn(500);
                return;
            }

            ParentClass::AIUpdate();
        }

        int32    mBurnTimer;
};

//Phoenix Egg AI
const uint32 CN_PHOENIX_EGG = 21364;

class PhoenixEggAI : public MoonScriptCreatureAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(PhoenixEggAI, MoonScriptCreatureAI);
        PhoenixEggAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            RegisterAIUpdateEvent(15000);
            setCanEnterCombat(false);
            _setMeleeDisabled(false);
            setRooted(true);
        }

        void OnDied(Unit* mKiller)
        {
            ParentClass::OnDied(mKiller);
            despawn(500);
        }

        void AIUpdate()
        {
            SpawnCreature(CN_PHOENIX, _unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), _unit->GetOrientation(), false);
            despawn(0);
        }
};

//-------------------Kael'thas Weapons------------------//

const uint32 NETHERSTRAND_LONGBOW = 21268;
const uint32 DEVASTATION = 21269;
const uint32 COSMIC_INFUSER = 21270;
const uint32 INFINITY_BLADE = 21271;
const uint32 WARP_SLICER = 21272;
const uint32 PHASESHIFT_BULWARK = 21273;
const uint32 STAFF_OF_DISINTEGRATION = 21274;

class WeaponsAI : public MoonScriptCreatureAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(WeaponsAI, MoonScriptCreatureAI);
        WeaponsAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            _unit->m_noRespawn = true;

            Unit* pTarget = GetBestPlayerTarget();
            if (pTarget != NULL)
            {
                _unit->GetAIInterface()->AttackReaction(pTarget, 200, 0);
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            Unit* pTarget = GetBestPlayerTarget();
            if (pTarget != NULL)
            {
                _unit->GetAIInterface()->AttackReaction(pTarget, 500);
            }
            else
            {
                despawn(1);
            }
        }
};

//\todo Add weapon summon effect
// Check why some features block (like melee, movement and so on) if there's only 1 target and spell req. NotCurrent one

//Prince Kael'Thas
const uint32 CN_KAELTHAS = 19622;

// Common spells
const uint32 KAELTHAS_FIREBALL = 36805;    // prolly wrong id
const uint32 KAELTHAS_ARCANE_DISRUPTION = 36834;
const uint32 KAELTHAS_SHOCK_BARRIER = 36815;    // timed

// Phase 4 spells
const uint32 KAELTHAS_FLAME_STRIKE_SUMMON = 36735;
const uint32 KAELTHAS_PHOENIX = 36723;
const uint32 KAELTHAS_PYROBLAST = 36819;    // timed
const uint32 KAELTHAS_MIND_CONTROL = 36797;    // timed

// Phase 5 spells
const uint32 KAELTHAS_GRAVITY_LAPSE = 35966;    // timed
const uint32 KAELTHAS_NETHER_VAPOR = 35859;
const uint32 KAELTHAS_NETHER_BEAM = 35873;    // timed along with lapse

//const uint32 KAELTHAS_GRAVITY1 = 34480;    // knockback + aura
//const uint32 KAELTHAS_GRAVITY2 = 35941;    // explosion effect

const uint32 KAELTHAS_SUMMON_WEAPONS = 36976;    //casting effect

const uint32 REMOVE_INFUSER = 39498;
const uint32 REMOVE_DEVASTATION = 39499;
const uint32 REMOVE_INFINITY = 39500;
const uint32 REMOVE_LONGBOW = 39501;
const uint32 REMOVE_BULWARK = 39502;
const uint32 REMOVE_STAFF = 39503;
const uint32 REMOVE_SLICER = 39504;

const Movement::Location Triggers[] =
{
    { 789.719543f, 24.627499f, 52.728550f },
    { 791.931152f, -24.925735f, 52.728550f },
};

const LocationExtra Advisors[] =
{
    { 785.807007f,  19.486200f, 48.911800f, 3.979350f, 20064 },
    { 785.781982f, -20.399500f, 48.911800f, 2.303830f, 20060 },
    { 792.408020f, -13.241500f, 48.911800f, 2.687810f, 20062 },
    { 792.724976f,  12.775400f, 48.911800f, 3.595380f, 20063 }
};

const LocationExtra Gates[] =
{
    { 676.777283f, -44.468628f, 46.780785f, 0.932028f, 184325 },
    { 676.812500f,  43.073757f, 46.781292f, 5.312979f, 184324 }
};

const Movement::LocationWithFlag Waypoints[] =
{
    {  },
    { 794.072998f,  0.214634f, 48.728500f, 0.0f, Movement::WP_MOVE_TYPE_RUN },
    { 794.052998f,  0.214634f, 75.728500f, 0.0f, Movement::WP_MOVE_TYPE_FLY },
    { 794.032998f,  0.214634f, 48.728500f, 0.0f, Movement::WP_MOVE_TYPE_FLY }
};

const LocationExtra KaelthasWeapons[] =
{
    { 794.38f, 15.00f, 48.72f, 2.9f, 21270 },        // [Cosmic Infuser]
    { 785.47f, 12.12f, 48.72f, 3.14f, 21269 },        // [Devastation]
    { 781.25f, 4.39f, 48.72f, 3.14f, 21271 },        // [Infinity Blade]
    { 777.38f, -0.81f, 48.72f, 3.06f, 21273 },        // [Phaseshift Bulwark]
    { 781.48f, -6.08f, 48.72f, 3.9f, 21274 },        // [Staff of Disintegration]
    { 785.42f, -13.59f, 48.72f, 3.4f, 21272 },        // [Warp Slicer]
    { 793.06f, -16.61f, 48.72f, 3.10f, 21268 }        // [Netherstrand Longbow]
};

enum AdvisorPhase
{
    PHASE_SPEECH,
    PHASE_ATTACK_COMMAND,
    PHASE_ADV_FIGHT,
};

void SpellFunc_KaelThasArcaneDisruption(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType);
void SpellFunc_KaelThasFlameStrike(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType);

class KaelThasAI : public MoonScriptBossAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(KaelThasAI, MoonScriptBossAI);
        KaelThasAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            for (uint8 i = 1; i < 4; ++i)
            {
                AddWaypoint(CreateWaypoint(1, 0, Waypoints[i].wp_flag, Waypoints[i].wp_location));
            }

            setCanEnterCombat(true);
            SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
            setRooted(false);

            // Other spells
            mSummonWeapons = AddSpell(KAELTHAS_SUMMON_WEAPONS, Target_Self, 0, 3, 0);
            mSummonWeapons->AddEmote("As you see, I have many weapons in my arsenal...", CHAT_MSG_MONSTER_YELL, 11261);

            // Common spells
            mArcaneDisruption = AddSpell(KAELTHAS_ARCANE_DISRUPTION, Target_Self, 0, 0, 0);
            mArcaneDisruptionFunc = AddSpellFunc(&SpellFunc_KaelThasArcaneDisruption, Target_RandomPlayerNotCurrent, 0, 0, 0);
            mShockBarrier = AddSpell(KAELTHAS_SHOCK_BARRIER, Target_Self, 0, 0, 60);
            AddPhaseSpell(7, AddSpell(KAELTHAS_FIREBALL, Target_Current, 10, 2, 15));
            AddPhaseSpell(8, AddSpell(KAELTHAS_FIREBALL, Target_Current, 10, 2, 15));

            // 1st phase
            mPyroblast = AddSpell(KAELTHAS_PYROBLAST, Target_Current, 0, 4, 0);
            SpellDesc* mMindControl = AddPhaseSpell(7, AddSpell(KAELTHAS_MIND_CONTROL, Target_Self, 100, 0, 30));
            mMindControl->AddEmote("Obey me.", CHAT_MSG_MONSTER_YELL, 11268);
            mMindControl->AddEmote("Bow to my will.", CHAT_MSG_MONSTER_YELL, 11269);
            mFlameStrike = AddSpell(KAELTHAS_FLAME_STRIKE_SUMMON, Target_RandomPlayerNotCurrent, 0, 0, 0);
            mFlameStrikeFunc = AddSpellFunc(&SpellFunc_KaelThasFlameStrike, Target_RandomPlayerNotCurrent, 0, 0, 0);
            mPhoenix = AddSpell(KAELTHAS_PHOENIX, Target_Self, 0, 0, 0);
            mPhoenix->AddEmote("Anara'nel belore!", CHAT_MSG_MONSTER_YELL, 11267);
            mPhoenix->AddEmote("By the power of the sun!", CHAT_MSG_MONSTER_YELL, 11266);

            // After powering up + Nether Vapor + Additional spells
            mNetherBeam = AddPhaseSpell(8, AddSpell(KAELTHAS_NETHER_BEAM, Target_RandomPlayer, 0, 0, 0));

            AddEmote(Event_OnCombatStart, "Energy. Power. My people are addicted to it. Their dependence made manifest after the Sunwell was destroyed. Welcome to the future...a pity you're too late to stop it. No one can stop me now. Selama ashal'anore.", CHAT_MSG_MONSTER_YELL, 11256);
            AddEmote(Event_OnTargetDied, "You will not prevail.", CHAT_MSG_MONSTER_YELL, 11270);
            AddEmote(Event_OnTargetDied, "You gambled...and lost.", CHAT_MSG_MONSTER_YELL, 11271);
            AddEmote(Event_OnTargetDied, "This was child's play.", CHAT_MSG_MONSTER_YELL, 11272);
            AddEmote(Event_OnDied, "For...Quel...thalas!", CHAT_MSG_MONSTER_YELL, 11274);
            mAIUpdateFrequency = 30000;

            mArcaneDisruptionTimer = 0;
            mShockBarrierTimer = 0;
            mFlameStrikeTimer = 0;
            mPhoenixTimer = 0;
            mEventTimer = 0;
            mAdvisorPhase = PHASE_SPEECH;

            for (uint8 i = 0; i < 4; ++i)
            {
                Creature* pCreature = getNearestCreature(Advisors[i].x, Advisors[i].y, Advisors[i].z, Advisors[i].addition);
                if (pCreature != NULL)
                {
                    pCreature->Despawn(0, 0);
                }

                SpawnCreature(Advisors[i].addition, Advisors[i].x, Advisors[i].y, Advisors[i].z, Advisors[i].o);
            }

        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
            SetAIUpdateFreq(24000);
            ParentClass::OnCombatStart(mTarget);
            setAIAgent(AGENT_SPELL);
            setRooted(true);

            for (uint8 i = 0; i < 2; ++i)
            {
                GameObject* pGameobject = getNearestGameObject(Gates[i].x, Gates[i].y, Gates[i].z, Gates[i].addition);
                if (pGameobject != NULL && pGameobject->GetState() == 0)
                {
                    pGameobject->SetState(GO_STATE_CLOSED);
                }
            }

            mEventTimer = mArcaneDisruptionTimer = mShockBarrierTimer = mFlameStrikeTimer = mPhoenixTimer = -1;
            mAdvisorPhase = PHASE_SPEECH;
            mAdvCoords.clear();
        }

        void OnCombatStop(Unit* mTarget)
        {
            setRooted(false);
            ParentClass::OnCombatStop(mTarget);
            _unit->setUInt64Value(UNIT_FIELD_FLAGS, 0);

            if (isAlive())
            {
                for (uint8 i = 0; i < 4; ++i)
                {
                    Creature* pCreature = getNearestCreature(Advisors[i].x, Advisors[i].y, Advisors[i].z, Advisors[i].addition);
                    if (pCreature != NULL)
                    {
                        pCreature->Despawn(0, 0);
                    }

                    SpawnCreature(Advisors[i].addition, Advisors[i].x, Advisors[i].y, Advisors[i].z, Advisors[i].o);
                }
            }

            for (uint8 i = 0; i < 2; ++i)
            {
                GameObject* pGameobject = getNearestGameObject(Gates[i].x, Gates[i].y, Gates[i].z, Gates[i].addition);
                if (pGameobject != NULL && pGameobject->GetState() == 1)
                {
                    pGameobject->SetState(GO_STATE_OPEN);
                }
            }
        }

        void SendAdvisorEmote()
        {
            switch (GetPhase())
            {
                case 1:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11259, "Let us see how your nerves hold up against the Darkener, Thaladred.");
                    SetAIUpdateFreq(5000);
                    break;
                case 2:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11260, "You have persevered against some of my best advisors. But none can withstand the might of the Bloodhammer. Behold, Lord Sanguinar.");
                    SetAIUpdateFreq(12000);
                    break;
                case 3:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11257, "Capernian will see to it that your stay here is a short one.");
                    SetAIUpdateFreq(5000);
                    break;
                case 4:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11258, "Well done. You have proven worthy to test your skills against my Master Engineer, Telonicus.");
                    SetAIUpdateFreq(8000);
                    break;
            }

            mAdvisorPhase = PHASE_ATTACK_COMMAND;
        }

        void SendAdvisorToFight(Creature* pCreature)
        {
            pCreature->GetAIInterface()->SetAllowedToEnterCombat(true);
            pCreature->setUInt64Value(UNIT_FIELD_FLAGS, 0);

            Unit* pTarget = GetBestPlayerTarget();
            if (pTarget != NULL)
            {
                pCreature->GetAIInterface()->AttackReaction(pTarget, 200, 0);
            }

            SetAIUpdateFreq(1000);
            mAdvisorPhase = PHASE_ADV_FIGHT;
        }

        void CheckAdvisorState(Creature* pCreature)
        {
            if (!pCreature->isAlive())
            {
                LocationExtra pCoords;
                pCoords.x = pCreature->GetPositionX();
                pCoords.y = pCreature->GetPositionY();
                pCoords.z = pCreature->GetPositionZ();
                pCoords.o = pCreature->GetOrientation();
                pCoords.addition = pCreature->GetEntry();

                SetAIUpdateFreq(5000);
                SetPhase(GetPhase() + 1);
                mAdvCoords.push_back(pCoords);
                mAdvisorPhase = PHASE_SPEECH;
            }
        }

        void AIUpdate()
        {
            if (GetPhase() < 5)
            {
                uint32 i = GetPhase() > 0 ? GetPhase() - 1 : 0;
                Creature* pCreature = getNearestCreature(Advisors[i].x, Advisors[i].y, Advisors[i].z, Advisors[i].addition);
                if (pCreature == NULL || (!pCreature->isAlive() && mAdvisorPhase != PHASE_ADV_FIGHT))
                {
                    LocationExtra pCoords;
                    pCoords.x = Advisors[i].x;
                    pCoords.y = Advisors[i].y;
                    pCoords.z = Advisors[i].z;
                    pCoords.o = Advisors[i].o;
                    pCoords.addition = Advisors[i].addition;
                    mAdvCoords.push_back(pCoords);

                    SetPhase(GetPhase() + 1);
                    mAdvisorPhase = PHASE_SPEECH;
                    return;
                }
                switch (mAdvisorPhase)
                {
                    case PHASE_SPEECH:
                        SendAdvisorEmote();
                        break;
                    case PHASE_ATTACK_COMMAND:
                        SendAdvisorToFight(pCreature);
                        break;
                    case PHASE_ADV_FIGHT:
                        CheckAdvisorState(pCreature);
                        break;
                }
            }
            if (GetPhase() == 5)
            {
                if (mEventTimer == -1)
                {
                    CastSpellNowNoScheduling(mSummonWeapons);
                    SetAIUpdateFreq(3000);
                    mEventTimer = -2;
                    return;
                }
                else if (mEventTimer == -2)
                {
                    for (uint8 i = 0; i < 7; ++i)
                    {
                        SpawnCreature(KaelthasWeapons[i].addition, KaelthasWeapons[i].x, KaelthasWeapons[i].y, KaelthasWeapons[i].z, KaelthasWeapons[i].o);
                    }

                    SetAIUpdateFreq(1000);
                    mEventTimer = _addTimer(125000);
                    return;
                }
                else if (_isTimerFinished(mEventTimer))
                {
                    for (uint8 i = 0; i < 4; ++i)
                    {
                        if (mAdvCoords.size() <= (size_t)i)
                            break;

                        Creature* pCreature = getNearestCreature(mAdvCoords[i].x, mAdvCoords[i].y, mAdvCoords[i].z, mAdvCoords[i].addition);
                        if (pCreature != NULL && !pCreature->isAlive())
                        {
                            pCreature->Despawn(0, 0);
                        }

                        MoonScriptCreatureAI* pAI = SpawnCreature(mAdvCoords[i].addition, mAdvCoords[i].x, mAdvCoords[i].y, mAdvCoords[i].z, false);
                        if (pAI != NULL)
                        {
                            pCreature = pAI->GetUnit();
                        }
                        else continue;

                        if (pCreature != NULL)
                        {
                            pCreature->GetAIInterface()->SetAllowedToEnterCombat(true);
                            pCreature->setUInt64Value(UNIT_FIELD_FLAGS, 0);
                            if (pCreature->GetScript() != NULL)
                            {
                                static_cast< MoonScriptCreatureAI* >(pCreature->GetScript())->AggroNearestUnit(200);
                            }
                        }
                    }

                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11262, "Perhaps I underestimated you. It would be unfair to make you fight all four Advisors at once, but...fair treatment was never shown to my people. I'm just returning the favor.");
                    _resetTimer(mEventTimer, 180000);
                    SetPhase(6);
                    mAdvCoords.clear();
                }

                ParentClass::AIUpdate();
                setAIAgent(AGENT_SPELL);
                setRooted(true);
            }
            if (GetPhase() == 6)
            {
                ParentClass::AIUpdate();
                if (_isTimerFinished(mEventTimer))
                {
                    mArcaneDisruptionTimer = _addTimer(20000);
                    mShockBarrierTimer = _addTimer(60000);
                    mFlameStrikeTimer = _addTimer(40000);
                    setCanEnterCombat(true);
                    setAIAgent(AGENT_NULL);
                    setRooted(false);
                    SetPhase(7);
                }
                else
                {
                    setAIAgent(AGENT_SPELL);
                    setRooted(true);
                }

                return;
            }
            if (GetPhase() == 7)
            {
                if (!_isCasting())
                {
                    if (getAIAgent() == AGENT_SPELL)
                    {
                        setAIAgent(AGENT_NULL);
                        setRooted(false);
                    }
                    if (_isTimerFinished(mShockBarrierTimer))
                    {
                        CastSpellNowNoScheduling(mShockBarrier);
                        _resetTimer(mShockBarrierTimer, 70000);
                    }
                    else if (_isTimerFinished(mArcaneDisruptionTimer))
                    {
                        CastSpellNowNoScheduling(mArcaneDisruptionFunc);
                        _resetTimer(mArcaneDisruptionTimer, 30000);
                    }
                    else if (_isTimerFinished(mFlameStrikeTimer))
                    {
                        CastSpellNowNoScheduling(mFlameStrikeFunc);
                    }
                }
                if (_unit->HasAura(KAELTHAS_SHOCK_BARRIER))
                {
                    CastSpellNowNoScheduling(mPyroblast);
                    setAIAgent(AGENT_SPELL);
                    setRooted(true);
                }
                else if (_isTimerFinished(mPhoenixTimer))        // it spawns on caster's place, but should in 20y from him
                {
                    // also it seems to not work (not always)
                    CastSpell(mPhoenix);
                    _removeTimer(mPhoenixTimer);
                }

                ParentClass::AIUpdate();
            }
        }

        Unit* GetRandomPlayer()
        {
            return GetBestPlayerTarget(TargetFilter_NotCurrent);
        }

        SpellDesc* mSummonWeapons;
        SpellDesc* mArcaneDisruption;
        SpellDesc* mArcaneDisruptionFunc;
        SpellDesc* mShockBarrier;
        SpellDesc* mPyroblast;
        SpellDesc* mFlameStrike;
        SpellDesc* mFlameStrikeFunc;
        SpellDesc* mPhoenix;
        SpellDesc* mNetherBeam;

        AdvisorPhase mAdvisorPhase;
        int32 mArcaneDisruptionTimer;
        int32 mShockBarrierTimer;
        int32 mFlameStrikeTimer;
        uint32 mPhoenixTimer;
        int32 mEventTimer;

        std::vector<LocationExtra> mAdvCoords;
};

void SpellFunc_KaelThasArcaneDisruption(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType)
{
    KaelThasAI* KaelThas = (pCreatureAI) ? static_cast< KaelThasAI* >(pCreatureAI) : NULL;
    if (KaelThas != NULL)
    {
        KaelThas->CastSpellNowNoScheduling(KaelThas->mArcaneDisruption);
        Unit* pMainTarget = KaelThas->GetUnit()->GetAIInterface()->getNextTarget();
        if (pTarget != NULL && pMainTarget != NULL)
        {
            KaelThas->GetUnit()->GetAIInterface()->setNextTarget(pTarget);
            KaelThas->GetUnit()->GetAIInterface()->AttackReaction(pTarget, 1000);
            KaelThas->GetUnit()->GetAIInterface()->RemoveThreatByPtr(pMainTarget);
        }
    }
}

void SpellFunc_KaelThasFlameStrike(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType)
{
    KaelThasAI* KaelThas = (pCreatureAI) ? static_cast< KaelThasAI* >(pCreatureAI) : NULL;
    if (KaelThas != NULL)
    {
        if (pTarget != NULL)
        {
            KaelThas->GetUnit()->CastSpell(pTarget, KAELTHAS_FLAME_STRIKE_SUMMON, true);
            Creature* pFriendlyTrigger = KaelThas->getNearestCreature(CN_FLAME_STRIKE_TRIGGER);
            if (pFriendlyTrigger != NULL && pFriendlyTrigger->IsPet())
            {
                pFriendlyTrigger->Despawn(0, 0);
            }

            KaelThas->spawnCreature(CN_FLAME_STRIKE_TRIGGER, pTarget->GetPosition());
            uint32 TimerAdd = (3 + RandomUInt(5)) * 1000;
            KaelThas->mPhoenixTimer = KaelThas->_addTimer(TimerAdd);
            KaelThas->_resetTimer(KaelThas->mFlameStrikeTimer, TimerAdd + 40000);
            return;
        }

        KaelThas->_resetTimer(KaelThas->mFlameStrikeTimer, 40000);
    }
}

void SetupTheEye(ScriptMgr* mgr)
{
    //Trash mobs
    /*mgr->register_creature_script(CN_A_STAR_SCRYER, &AStarScryerAI::Create);
    mgr->register_creature_script(CN_ASTROMANCER, &AstromancerAI::Create);
    mgr->register_creature_script(CN_STAR_SCRYER, &StarScryerAI::Create);
    mgr->register_creature_script(CN_ASTROMANCER_LORD, &AstromancerLordAI::Create);
    mgr->register_creature_script(CN_BLOOD_VINDICATOR, &BloodVindicatorAI::Create);
    mgr->register_creature_script(CN_BLOOD_LEGIONNARE, &BloodLegionnareAI::Create);
    mgr->register_creature_script(CN_BLOOD_MARSHAL, &BloodMarshalAI::Create);
    mgr->register_creature_script(CN_PHOENIX_HAWK, &PhoenixHawkAI::Create);
    mgr->register_creature_script(CN_CRYSTAL_SENTINEL, &CrystalSentinelAI::Create);
    mgr->register_creature_script(CN_CRYSTAL_MECHANIC, &CrystalMechanicAI::Create);*/

    //Void Reaver event
    mgr->register_creature_script(CN_VOID_REAVER, &VoidReaverAI::Create);

    //High Astromancer Solarian
    mgr->register_creature_script(CN_SOLARIAN, &HighAstromancerSolarianAI::Create);
    mgr->register_creature_script(CN_SOLARIUMAGENT, &SolariumAgentAI::Create);
    mgr->register_creature_script(CN_SOLARIUMPRIEST, &SolariumPriestAI::Create);
    mgr->register_creature_script(CN_SOLARIUM_SPOT_LIGHT, &SolariumSpotLight::Create);

    //Al'ar event
    /*mgr->register_creature_script(CN_ALAR, &AlarAI::Create);
    mgr->register_creature_script(CN_EMBEROFALAR, &EmberAlarAI::Create);
    mgr->register_creature_script(CN_PATCHALAR, &PatchAlarAI::Create);*/

    //Kael'Thas Encounter
    mgr->register_creature_script(CN_PHOENIX, &PhoenixAI::Create);
    mgr->register_creature_script(CN_PHOENIX_EGG, &PhoenixEggAI::Create);
    mgr->register_creature_script(CN_FLAME_STRIKE_TRIGGER, &FlameStrikeAI::Create);
    mgr->register_creature_script(CN_DARKENER, &DarkenerAI::Create);
    mgr->register_creature_script(CN_SANGUINAR, &SanguinarAI::Create);
    mgr->register_creature_script(CN_CAPERNIAN, &CapernianAI::Create);
    mgr->register_creature_script(CN_TELONICUS, &TelonicusAI::Create);
    mgr->register_creature_script(CN_KAELTHAS, &KaelThasAI::Create);

    // Kael'thas Weapons
    mgr->register_creature_script(NETHERSTRAND_LONGBOW, &WeaponsAI::Create);
    mgr->register_creature_script(DEVASTATION, &WeaponsAI::Create);
    mgr->register_creature_script(COSMIC_INFUSER, &WeaponsAI::Create);
    mgr->register_creature_script(INFINITY_BLADE, &WeaponsAI::Create);
    mgr->register_creature_script(WARP_SLICER, &WeaponsAI::Create);
    mgr->register_creature_script(PHASESHIFT_BULWARK, &WeaponsAI::Create);
    mgr->register_creature_script(STAFF_OF_DISINTEGRATION, &WeaponsAI::Create);
}
