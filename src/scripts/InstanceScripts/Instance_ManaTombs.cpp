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

#include "Setup.h"
#include "Instance_ManaTombs.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Auchindoun: Mana-Tombs
class InstanceAuchindounManaTombsScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceAuchindounManaTombsScript, MoonInstanceScript);
        InstanceAuchindounManaTombsScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
        {
            // Way to select bosses
            BuildEncounterMap();
            if (mEncounters.size() == 0)
                return;

            for (EncounterMap::iterator Iter = mEncounters.begin(); Iter != mEncounters.end(); ++Iter)
            {
                if ((*Iter).second.mState != State_Finished)
                    continue;
            }
        }

        void OnGameObjectPushToWorld(GameObject* pGameObject) { }

        void SetInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = (EncounterState)pData;
        }

        uint32 GetInstanceData(uint32 pType, uint32 pIndex)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return 0;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return 0;

            return (*Iter).second.mState;
        }

        void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
        {
            EncounterMap::iterator Iter = mEncounters.find(pCreature->GetEntry());
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = State_Finished;

            return;
        }
};

// EtherealDarkcasterAI
class EtherealDarkcasterAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(EtherealDarkcasterAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        EtherealDarkcasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(MANA_BURN);
            spells[0].targettype = TARGET_DESTINATION;
            spells[0].instant = true;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SHADOW_WORD_PAIN);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 10.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
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
                for (int i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        int nrspells;
};


// EtherealPriestAI
class EtherealPriestAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(EtherealPriestAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        EtherealPriestAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(HEAL);
            spells[0].targettype = TARGET_SELF; // until function to target wounded friendly unit will be done
            spells[0].instant = false;
            spells[0].perctrigger = 7.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(POWER_WORD_SHIELD);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].perctrigger = 7.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
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
                for (int i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        int nrspells;
};

// EtherealTheurgistAI
class EtherealTheurgistAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(EtherealTheurgistAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        EtherealTheurgistAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(POLYMORPH);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].perctrigger = 7.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(BLAST_WAVE);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].perctrigger = 10.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
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
                for (int i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        int nrspells;
};


// EtherealSorcererAI
class EtherealSorcererAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(EtherealSorcererAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        EtherealSorcererAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(ARCANE_MISSILES);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
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
                for (int i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        int nrspells;
};

// NexusStalkerAI
class NexusStalkerAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(NexusStalkerAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        NexusStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(GOUGE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(POISON);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 7.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(STEALTH);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].perctrigger = 5.0f;
            spells[2].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
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
                for (int i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        int nrspells;
};

// NexusTerrorAI
class NexusTerrorAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(NexusTerrorAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        NexusTerrorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(PSYCHIC_SCREAM);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].perctrigger = 3.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
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
                for (int i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        int nrspells;
};


// ManaLeechAI
class ManaLeechAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ManaLeechAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        ManaLeechAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(ARCANE_EXPLOSION);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            _unit->CastSpell(_unit, spells[0].info, spells[0].instant);
            RemoveAIUpdateEvent();
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
                for (int i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        int nrspells;
};

// EtherealSpellbinderAI
class EtherealSpellbinderAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(EtherealSpellbinderAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        EtherealSpellbinderAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(CORRUPTION);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 7.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(IMMOLATE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 7.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(UNSTABLE_AFFLICTION);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].perctrigger = 7.0f;
            spells[2].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SUMMON_ETHEREAL_WRAITH);
            spells[2].targettype = TARGET_SELF;  // ?
            spells[2].instant = true;
            spells[2].perctrigger = 5.0f;
            spells[2].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
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
                for (int i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        int nrspells;
};


// EtherealWraithAI
class EtherealWraithAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(EtherealWraithAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        EtherealWraithAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SHADOW_BOLT_VOLLEY);
            spells[0].targettype = TARGET_VARIOUS; // Haven't tested on groups, but should work correctly.
            spells[0].instant = false;
            spells[0].perctrigger = 15.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
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
                for (int i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        int nrspells;
};


/// \todo Needs to add Yor in a future

// PandemoniusAI
class PandemoniusAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(PandemoniusAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        PandemoniusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;

            }

            spells[0].info = dbcSpell.LookupEntry(DARK_SHELL);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = false;
            spells[0].perctrigger = 20.0f;
            spells[0].attackstoptimer = 2000;
            spells[0].cooldown = 20;

            spells[1].info = dbcSpell.LookupEntry(VOID_BLAST);
            spells[1].targettype = TARGET_RANDOM_SINGLE;
            spells[1].instant = false;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1500;
            spells[1].cooldown = 5;
            spells[1].mindist2cast = 0.0f;
            spells[1].maxdist2cast = 40.0f;
        }

        void OnCombatStart(Unit* mTarget)
        {
            for (int i = 0; i < 2; i++)
                spells[i].casttime = 0;

            int RandomSpeach = rand() % 3;
            switch (RandomSpeach)
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_PANDEMONIUS_01);
                    _unit->PlaySoundToSet(10561);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_PANDEMONIUS_02);
                    _unit->PlaySoundToSet(10562);
                    break;
                case 2:
                    _unit->SendScriptTextChatMessage(SAY_PANDEMONIUS_03);
                    _unit->PlaySoundToSet(10563);
                    break;
            }

            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                int RandomSpeach = rand() % 2;
                switch (RandomSpeach)
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_PANDEMONIUS_04);
                        _unit->PlaySoundToSet(10564);
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(SAY_PANDEMONIUS_05);
                        _unit->PlaySoundToSet(10565);
                        break;
                }
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(SAY_PANDEMONIUS_06);
            _unit->PlaySoundToSet(10566);

            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            uint32 t = (uint32)time(NULL);
            if (t > spells[1].casttime && _unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                CastSpellOnRandomTarget(1, spells[1].mindist2cast, spells[1].maxdist2cast, 0, 100);

                spells[1].casttime = t + spells[1].cooldown;
                return;
            }

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (int i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
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
                std::vector<Unit*> TargetTable;        /* From M4ksiu - Big THX to Capt who helped me with std stuff to make it simple and fully working <3 */
                /* If anyone wants to use this function, then leave this note!                                         */
                for (set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(_unit, (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(_unit, (*itr)) && (*itr) != _unit)) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = TO_UNIT(*itr);

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

                size_t RandTarget = rand() % TargetTable.size();

                Unit*  RTarget = TargetTable[RandTarget];

                if (!RTarget)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(RTarget, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(RTarget->GetPositionX(), RTarget->GetPositionY(), RTarget->GetPositionZ(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:

        int nrspells;
};

// TavarokAI
/// \todo Strange... I couldn't find any sounds for this boss in DBC and in extracted from client sounds O_O
class TavarokAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(TavarokAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        TavarokAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(EARTHQUAKE);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 2000;
            spells[0].cooldown = 20;

            spells[1].info = dbcSpell.LookupEntry(CRYSTAL_PRISON);
            spells[1].targettype = TARGET_RANDOM_SINGLE;
            spells[1].instant = true;
            spells[1].perctrigger = 8.0f;
            spells[1].attackstoptimer = 2000;
            spells[1].cooldown = 20;
            spells[1].mindist2cast = 0.0f;
            spells[1].maxdist2cast = 40.0f;

            spells[2].info = dbcSpell.LookupEntry(ARCING_SMASH);
            spells[2].targettype = TARGET_VARIOUS;  // Should affect only party/raid member in front of caster (I think it works, but needs tests anyway)
            spells[2].instant = true;
            spells[2].perctrigger = 12.0f;
            spells[2].attackstoptimer = 1000;
            spells[2].cooldown = 10;
        }

        void OnCombatStart(Unit* mTarget)
        {
            for (int i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
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
                for (int i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
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
                std::vector<Unit*> TargetTable;        // From M4ksiu - Big THX to Capt
                for (set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(_unit, (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(_unit, (*itr)) && (*itr) != _unit)) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = TO_UNIT(*itr);

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

                size_t RandTarget = rand() % TargetTable.size();

                Unit*  RTarget = TargetTable[RandTarget];

                if (!RTarget)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(RTarget, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(RTarget->GetPositionX(), RTarget->GetPositionY(), RTarget->GetPositionZ(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:

        int nrspells;
};


// NexusPrinceShaffarAI
/// \todo Work on beacons and find out if my current way of spawning them is correct
class NexusPrinceShaffarAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(NexusPrinceShaffarAI);
        SP_AI_Spell spells[5];
        bool m_spellcheck[5];

        NexusPrinceShaffarAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(FIREBALL);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = false;
            spells[0].perctrigger = 35.0f;
            spells[0].attackstoptimer = 2000;
            spells[0].cooldown = 5;
            spells[0].mindist2cast = 0.0f;
            spells[0].maxdist2cast = 40.0f;

            spells[1].info = dbcSpell.LookupEntry(FROSTBOLT);
            spells[1].targettype = TARGET_RANDOM_SINGLE;
            spells[1].instant = false;
            spells[1].perctrigger = 35.0f;
            spells[1].attackstoptimer = 2000;
            spells[1].cooldown = 5;
            spells[1].mindist2cast = 0.0f;
            spells[1].maxdist2cast = 40.0f;

            spells[2].info = dbcSpell.LookupEntry(FROST_NOVA);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = true;
            spells[2].perctrigger = 15.0f;
            spells[2].attackstoptimer = 1000;
            spells[2].cooldown = 15;

            spells[3].info = dbcSpell.LookupEntry(BLINK);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].perctrigger = 5.0f;
            spells[3].attackstoptimer = 1000;
            spells[3].cooldown = 20;

            spells[4].info = dbcSpell.LookupEntry(SUMMON_ETEREAL_BECON);
            spells[4].targettype = TARGET_SELF;
            spells[4].instant = true;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;
            spells[4].cooldown = 10;
        }

        void OnCombatStart(Unit* mTarget)
        {
            for (int i = 0; i < 3; i++)
                spells[i].casttime = 0;

            uint32 t = (uint32)time(NULL);
            spells[3].casttime = t + RandomUInt(10);
            spells[4].casttime = t + spells[4].cooldown;

            int RandomSpeach = rand() % 3;
            switch (RandomSpeach)
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_NEXUSPRINCE_02);
                    _unit->PlaySoundToSet(10541);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_NEXUSPRINCE_03);
                    _unit->PlaySoundToSet(10542);
                    break;
                case 2:
                    _unit->SendScriptTextChatMessage(SAY_NEXUSPRINCE_04);
                    _unit->PlaySoundToSet(10543);
                    break;
            }
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                int RandomSpeach = rand() % 2;
                switch (RandomSpeach)
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_NEXUSPRINCE_06);
                        _unit->PlaySoundToSet(10544);
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(SAY_NEXUSPRINCE_05);
                        _unit->PlaySoundToSet(10545);
                        break;
                }
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(SAY_NEXUSPRINCE_08);
            _unit->PlaySoundToSet(10546);

            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            // not sure if it should be like that
            uint32 t = (uint32)time(NULL);
            if (t > spells[4].casttime && _unit->GetCurrentSpell() == NULL)
            {
                _unit->CastSpell(_unit, spells[4].info, spells[4].instant);

                spells[4].casttime = t + 10;
                return;
            }

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (int i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        if (!spells[i].instant)
                            _unit->GetAIInterface()->StopMovement(1);


                        if (i == 3)
                        {
                            uint32 t = (uint32)time(NULL);
                            if (t > spells[2].casttime && RandomUInt(2) == 1)
                            {
                                _unit->CastSpell(_unit, spells[2].info, spells[2].instant);

                                spells[2].casttime = t + spells[2].cooldown;
                            }
                        }

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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
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
                std::vector<Unit*> TargetTable;        // From M4ksiu - Big THX to Capt
                for (set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(_unit, (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(_unit, (*itr)) && (*itr) != _unit)) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = TO_UNIT(*itr);

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

                size_t RandTarget = rand() % TargetTable.size();

                Unit*  RTarget = TargetTable[RandTarget];

                if (!RTarget)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(RTarget, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(RTarget->GetPositionX(), RTarget->GetPositionY(), RTarget->GetPositionZ(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:

        int nrspells;
};

// YorAI
class YorAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(YorAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        YorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(DOUBLE_BREATH);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].perctrigger = 20.0f;
            spells[0].attackstoptimer = 2000;
            spells[0].cooldown = 15;

            spells[1].info = dbcSpell.LookupEntry(STOMP);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].perctrigger = 7.0f;
            spells[1].attackstoptimer = 2000;
            spells[1].cooldown = 25;
        }

        void OnCombatStart(Unit* mTarget)
        {
            uint32 t = (uint32)time(NULL);
            spells[0].casttime = 0;
            spells[1].casttime = t + RandomUInt(10);

            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
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
                for (int i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        if (!spells[i].instant)
                            _unit->GetAIInterface()->StopMovement(1);

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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
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

    protected:

        int nrspells;
};

void SetupManaTombs(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_AUCHENAI_MANA_TOMBS, &InstanceAuchindounManaTombsScript::Create);

    mgr->register_creature_script(CN_ETHEREAL_DARKCASTER, &EtherealDarkcasterAI::Create);
    mgr->register_creature_script(CN_ETHEREAL_PRIEST, &EtherealPriestAI::Create);
    mgr->register_creature_script(CN_ETHEREAL_SPELLBINDER, &EtherealSpellbinderAI::Create);
    mgr->register_creature_script(CN_ETHEREAL_THEURGIST, &EtherealTheurgistAI::Create);
    mgr->register_creature_script(CN_ETHEREAL_SORCERER, &EtherealSorcererAI::Create);
    mgr->register_creature_script(CN_ETHEREAL_WRAITH, &EtherealWraithAI::Create);
    mgr->register_creature_script(CN_NEXUS_STALKER, &NexusStalkerAI::Create);
    mgr->register_creature_script(CN_NEXUS_TERROR, &NexusTerrorAI::Create);
    mgr->register_creature_script(CN_MANA_LEECH, &ManaLeechAI::Create);
    mgr->register_creature_script(CN_PANDEMONIUS, &PandemoniusAI::Create);
    mgr->register_creature_script(CN_TAVAROK, &TavarokAI::Create);
    mgr->register_creature_script(CN_NEXUS_PRINCE_SHAFFAR, &NexusPrinceShaffarAI::Create);
    mgr->register_creature_script(CN_YOR, &YorAI::Create);
}
