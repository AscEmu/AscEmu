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
#include "Instance_BlackrockSpire.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Blackrock Spire
class InstanceBlackrockSpireScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceBlackrockSpireScript, MoonInstanceScript);
        InstanceBlackrockSpireScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
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

// General Drakkisath AI by Soulshifter
class GeneralDrakkisathAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(GeneralDrakkisathAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        GeneralDrakkisathAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 4;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = dbcSpell.LookupEntry(SPELL_FIRENOVA);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 15.0f;
            spells[0].attackstoptimer = 1000; // 1sec

            spells[1].info = dbcSpell.LookupEntry(SPELL_CLEAVE1);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 20.0f;
            spells[1].attackstoptimer = 1000; // 1sec

            spells[2].info = dbcSpell.LookupEntry(SPELL_CONFLAGRATION);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].perctrigger = 20.0f;
            spells[2].attackstoptimer = 1000; // 1sec

            spells[3].info = dbcSpell.LookupEntry(SPELL_THUNDERCLAP);
            spells[3].targettype = TARGET_VARIOUS;
            spells[3].instant = false;
            spells[3].perctrigger = 15.0f;
            spells[3].attackstoptimer = 1000; // 1sec
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
                for (uint8 i = 0; i < nrspells; i++)
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

        uint8 nrspells;
};


// Pyroguard Embersser AI by Soulshifter
class PyroguardEmbersserAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(PyroguardEmbersserAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        PyroguardEmbersserAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 3;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = dbcSpell.LookupEntry(SPELL_FIRENOVA);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 15.0f;
            spells[0].attackstoptimer = 1000; // 1sec

            spells[1].info = dbcSpell.LookupEntry(SPELL_FLAMEBUFFET);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = false;
            spells[1].perctrigger = 25.0f;
            spells[1].attackstoptimer = 1000; // 1sec

            spells[2].info = dbcSpell.LookupEntry(SPELL_PYROBLAST);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].perctrigger = 30.0f;
            spells[2].attackstoptimer = 1000; // 1sec
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
                for (uint8 i = 0; i < nrspells; i++)
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

        uint8 nrspells;
};

// Warchief Rend Blackhand AI by Soulshifter
///\todo  PHASES. D:
class RendBlackhandAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(RendBlackhandAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        RendBlackhandAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 3;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = dbcSpell.LookupEntry(SPELL_WHIRLWIND);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 30.0f;
            spells[0].attackstoptimer = 1000; // 1sec

            spells[1].info = dbcSpell.LookupEntry(SPELL_CLEAVE2);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = false;
            spells[1].perctrigger = 30.0f;
            spells[1].attackstoptimer = 1000; // 1sec

            spells[2].info = dbcSpell.LookupEntry(SPELL_THUNDERCLAP_WR);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = false;
            spells[2].perctrigger = 30.0f;
            spells[2].attackstoptimer = 1000; // 1sec
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
                for (uint8 i = 0; i < nrspells; i++)
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

        uint8 nrspells;
};


// Gyth AI by Soulshifter
class GythAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(GythAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        GythAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 3;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = dbcSpell.LookupEntry(SPELL_CORROSIVEACID);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 24.0f;
            spells[0].attackstoptimer = 1000; // 1sec

            spells[1].info = dbcSpell.LookupEntry(SPELL_FREEZE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 30.0f;
            spells[1].attackstoptimer = 1000; // 1sec

            spells[2].info = dbcSpell.LookupEntry(SPELL_FLAMEBREATH);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = false;
            spells[2].perctrigger = 20.0f;
            spells[2].attackstoptimer = 1000; // 1sec

            HasSummoned = false;
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));

            HasSummoned = false;
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
            if (!HasSummoned && _unit->GetHealthPct() <= 8)
            {
                Unit* Warchief = NULL;
                Warchief = _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_REND_BLACKHAND, 157.366516f, -419.779358f, 110.472336f, 3.056772f, true, false, 0, 0);
                if (Warchief != NULL)
                {
                    if (_unit->GetAIInterface()->getNextTarget() != NULL)
                    {
                        Warchief->GetAIInterface()->AttackReaction(_unit->GetAIInterface()->getNextTarget(), 1, 0);
                    }
                }

                HasSummoned = true;
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
                for (uint8 i = 0; i < nrspells; i++)
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

        bool HasSummoned;
        uint8 nrspells;
};


// The Beast AI by Soulshifter
class TheBeastAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(TheBeastAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        TheBeastAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 3;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = dbcSpell.LookupEntry(SPELL_FLAMEBREAK);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 20.0f;
            spells[0].attackstoptimer = 1000; // 1sec

            spells[1].info = dbcSpell.LookupEntry(SPELL_IMMOLATE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 15.0f;
            spells[1].attackstoptimer = 1000; // 1sec

            spells[2].info = dbcSpell.LookupEntry(SPELL_TERRIFYINGROAR);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = true;
            spells[2].perctrigger = 20.0f;
            spells[2].attackstoptimer = 1000; // 1sec
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
                for (uint8 i = 0; i < nrspells; i++)
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

        uint8 nrspells;
};


// Highlord Omokk AI by Soulshifter
class HighlordOmokkAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(HighlordOmokkAI);
        SP_AI_Spell spells[7];
        bool m_spellcheck[7];

        HighlordOmokkAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 7;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = dbcSpell.LookupEntry(SPELL_WARSTOMP);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 20.0f;
            spells[0].attackstoptimer = 1000; // 1sec

            spells[1].info = dbcSpell.LookupEntry(SPELL_CLEAVE3);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 10.0f;
            spells[1].attackstoptimer = 1000; // 1sec

            spells[2].info = dbcSpell.LookupEntry(SPELL_STRIKE);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].perctrigger = 30.0f;
            spells[2].attackstoptimer = 1000; // 1sec

            spells[3].info = dbcSpell.LookupEntry(SPELL_REND);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = false;
            spells[3].perctrigger = 25.0f;
            spells[3].attackstoptimer = 1000; // 1sec

            spells[4].info = dbcSpell.LookupEntry(SPELL_SUNDERARMOR);
            spells[4].targettype = TARGET_ATTACKING;
            spells[4].instant = false;
            spells[4].perctrigger = 20.0f;
            spells[4].attackstoptimer = 1000; // 1sec

            spells[5].info = dbcSpell.LookupEntry(SPELL_KNOCKAWAY);
            spells[5].targettype = TARGET_VARIOUS;
            spells[5].instant = false;
            spells[5].perctrigger = 20.0f;
            spells[5].attackstoptimer = 1000; // 1sec

            spells[6].info = dbcSpell.LookupEntry(SPELL_SLOW);
            spells[6].targettype = TARGET_VARIOUS;
            spells[6].instant = false;
            spells[6].perctrigger = 20.0f;
            spells[6].attackstoptimer = 1000; // 1sec
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
                for (uint8 i = 0; i < nrspells; i++)
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

        uint8 nrspells;
};


// Shadow Hunter Vosh'gajin AI by Soulshifter
class ShadowHunterVoshAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ShadowHunterVoshAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        ShadowHunterVoshAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 3;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = dbcSpell.LookupEntry(SPELL_CURSEOFBLOOD);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 15.0f;
            spells[0].attackstoptimer = 1000; // 1sec

            spells[1].info = dbcSpell.LookupEntry(SPELL_HEX);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 20.0f;
            spells[1].attackstoptimer = 1000; // 1sec

            spells[2].info = dbcSpell.LookupEntry(SPELL_CLEAVE4);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].perctrigger = 20.0f;
            spells[2].attackstoptimer = 1000; // 1sec
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
                for (uint8 i = 0; i < nrspells; i++)
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

        uint8 nrspells;
};


// War Master Voone AI by Soulshifter
class WarMasterVooneAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(WarMasterVooneAI);
        SP_AI_Spell spells[6];
        bool m_spellcheck[6];

        WarMasterVooneAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 6;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = dbcSpell.LookupEntry(SPELL_SNAPKICK);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 20.0f;
            spells[0].attackstoptimer = 1000; // 1sec

            spells[1].info = dbcSpell.LookupEntry(SPELL_CLEAVE_WM);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 10.0f;
            spells[1].attackstoptimer = 1000; // 1sec

            spells[2].info = dbcSpell.LookupEntry(SPELL_UPPERCUT);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].perctrigger = 20.0f;
            spells[2].attackstoptimer = 1000; // 1sec

            spells[3].info = dbcSpell.LookupEntry(SPELL_MORTALSTRIKE);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = false;
            spells[3].perctrigger = 20.0f;
            spells[3].attackstoptimer = 1000; // 1sec

            spells[4].info = dbcSpell.LookupEntry(SPELL_PUMMEL);
            spells[4].targettype = TARGET_ATTACKING;
            spells[4].instant = false;
            spells[4].perctrigger = 20.0f;
            spells[4].attackstoptimer = 1000; // 1sec

            spells[5].info = dbcSpell.LookupEntry(SPELL_THROWAXE);
            spells[5].targettype = TARGET_ATTACKING;
            spells[5].instant = false;
            spells[5].perctrigger = 30.0f;
            spells[5].attackstoptimer = 1000; // 1sec
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
                for (uint8 i = 0; i < nrspells; i++)
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

        uint8 nrspells;
};


// Mother Smolderweb AI by Soulshifter
class MotherSmolderwebAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(MotherSmolderwebAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        MotherSmolderwebAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 4;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = dbcSpell.LookupEntry(SPELL_CRYSTALIZE);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 25.0f;
            spells[0].attackstoptimer = 1000; // 1sec

            spells[1].info = dbcSpell.LookupEntry(SPELL_MOTHERSMILK);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 20.0f;
            spells[1].attackstoptimer = 1000; // 1sec

            spells[2].info = dbcSpell.LookupEntry(SPELL_POISON);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].perctrigger = 20.0f;
            spells[2].attackstoptimer = 1000; // 1sec

            spells[3].info = dbcSpell.LookupEntry(SPELL_WEBEXPLOSION);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = false;
            spells[3].perctrigger = 20.0f;
            spells[3].attackstoptimer = 1000; // 1sec
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
                for (uint8 i = 0; i < nrspells; i++)
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

        uint8 nrspells;
};


// Urok Doomhowl AI by Soulshifter
class UrokDoomhowlAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(UrokDoomhowlAI);
        SP_AI_Spell spells[7];
        bool m_spellcheck[7];

        UrokDoomhowlAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 7;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = dbcSpell.LookupEntry(SPELL_WARSTOMP_UD);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 20.0f;
            spells[0].attackstoptimer = 1000; // 1sec

            spells[1].info = dbcSpell.LookupEntry(SPELL_CLEAVE_UD);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 10.0f;
            spells[1].attackstoptimer = 1000; // 1sec

            spells[2].info = dbcSpell.LookupEntry(SPELL_STRIKE_UD);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].perctrigger = 20.0f;
            spells[2].attackstoptimer = 1000; // 1sec

            spells[3].info = dbcSpell.LookupEntry(SPELL_REND_UD);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = false;
            spells[3].perctrigger = 20.0f;
            spells[3].attackstoptimer = 1000; // 1sec

            spells[4].info = dbcSpell.LookupEntry(SPELL_SUNDERARMOR_UD);
            spells[4].targettype = TARGET_ATTACKING;
            spells[4].instant = false;
            spells[4].perctrigger = 20.0f;
            spells[4].attackstoptimer = 1000; // 1sec

            spells[5].info = dbcSpell.LookupEntry(SPELL_KNOCKAWAY_UD);
            spells[5].targettype = TARGET_VARIOUS;
            spells[5].instant = false;
            spells[5].perctrigger = 20.0f;
            spells[5].attackstoptimer = 1000; // 1sec

            spells[6].info = dbcSpell.LookupEntry(SPELL_SLOW_UD);
            spells[6].targettype = TARGET_VARIOUS;
            spells[6].instant = false;
            spells[6].perctrigger = 10.0f;
            spells[6].attackstoptimer = 1000; // 1sec
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
                for (uint8 i = 0; i < nrspells; i++)
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

        uint8 nrspells;
};


// Quartermaster Zigris AI by Soulshifter
class QuartermasterZigrisAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(QuartermasterZigrisAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        QuartermasterZigrisAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 3;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = dbcSpell.LookupEntry(SPELL_SHOOT);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 40.0f;
            spells[0].attackstoptimer = 1000; // 1sec

            spells[1].info = dbcSpell.LookupEntry(SPELL_STUNBOMB);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 20.0f;
            spells[1].attackstoptimer = 1000; // 1sec

            spells[2].info = dbcSpell.LookupEntry(SPELL_HOOKEDNET);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].perctrigger = 20.0f;
            spells[2].attackstoptimer = 1000; // 1sec
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
                for (uint8 i = 0; i < nrspells; i++)
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

        uint8 nrspells;
};


// Halycon AI by Soulshifter
class HalyconAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(HalyconAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        HalyconAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 2;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = dbcSpell.LookupEntry(SPELL_CROWDPUMMEL);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 25.0f;
            spells[0].attackstoptimer = 1000; // 1sec

            spells[1].info = dbcSpell.LookupEntry(SPELL_MIGHTYBLOW);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 25.0f;
            spells[1].attackstoptimer = 1000; // 1sec

            HasSummoned = false;
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));

            HasSummoned = false;
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
            if (!HasSummoned && _unit->GetHealthPct() <= 25)
            {
                Unit* cGizrul = NULL;
                cGizrul = _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_GIZRUL, -195.100006f, -321.970001f, 65.424400f, 0.016500f, true, false, 0, 0);
                if (cGizrul != NULL)
                {
                    if (_unit->GetAIInterface()->getNextTarget() != NULL)
                    {
                        cGizrul->GetAIInterface()->AttackReaction(_unit->GetAIInterface()->getNextTarget(), 1, 0);
                    }
                }

                HasSummoned = true;
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
                for (uint8 i = 0; i < nrspells; i++)
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

        bool HasSummoned;
        uint8 nrspells;
};


// Overlord Wyrmthalak AI by Soulshifter
class OverlordWyrmthalakAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(OverlordWyrmthalakAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        OverlordWyrmthalakAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 4;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = dbcSpell.LookupEntry(SPELL_BLASTWAVE);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 25.0f;
            spells[0].attackstoptimer = 1000; // 1sec

            spells[1].info = dbcSpell.LookupEntry(SPELL_SHOUT);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 20.0f;
            spells[1].attackstoptimer = 1000; // 1sec

            spells[2].info = dbcSpell.LookupEntry(SPELL_CLEAVE5);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].perctrigger = 25.0f;
            spells[2].attackstoptimer = 1000; // 1sec

            spells[3].info = dbcSpell.LookupEntry(SPELL_KNOCKAWAY_OW);
            spells[3].targettype = TARGET_VARIOUS;
            spells[3].instant = false;
            spells[3].perctrigger = 15.0f;
            spells[3].attackstoptimer = 1000; // 1sec

            HasSummoned = false;
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));

            HasSummoned = false;
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
            if (!HasSummoned && _unit->GetHealthPct() <= 50)
            {
                Unit* Warlord1 = NULL;
                Unit* Warlord2 = NULL;
                Warlord1 = _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_SPIRESTONE_WARLORD, -30.675352f, -493.231750f, 90.610725f, 3.123542f, true, false, 0, 0); //This line makes the unit spawn a creature
                Warlord2 = _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_SPIRESTONE_WARLORD, -30.433489f, -479.833923f, 90.535606f, 3.123542f, true, false, 0, 0); //This line makes the unit spawn a creature
                if (_unit->GetAIInterface()->getNextTarget() != NULL)
                {
                    if (Warlord1 != NULL)
                    {
                        Warlord1->GetAIInterface()->AttackReaction(_unit->GetAIInterface()->getNextTarget(), 1, 0);
                    }
                    if (Warlord2 != NULL)
                    {
                        Warlord2->GetAIInterface()->AttackReaction(_unit->GetAIInterface()->getNextTarget(), 1, 0);
                    }
                }

                HasSummoned = true;    //Indicates that the spawns have been summoned
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
                for (uint8 i = 0; i < nrspells; i++)
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

        bool HasSummoned;
        uint8 nrspells;
};

void SetupBlackrockSpire(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_BLACKROCK_SPIRE, &InstanceBlackrockSpireScript::Create);

    mgr->register_creature_script(CN_GENERAL_DRAKKISATH, &GeneralDrakkisathAI::Create);
    mgr->register_creature_script(CN_PYROGUARD_EMBERSSER, &PyroguardEmbersserAI::Create);
    mgr->register_creature_script(CN_REND_BLACKHAND, &RendBlackhandAI::Create);
    mgr->register_creature_script(CN_GYTH, &GythAI::Create);
    mgr->register_creature_script(CN_THE_BEAST, &TheBeastAI::Create);
    mgr->register_creature_script(CN_HIGHLORD_OMOKK, &HighlordOmokkAI::Create);
    mgr->register_creature_script(CN_SHADOW_HUNTER_VOSH, &ShadowHunterVoshAI::Create);
    mgr->register_creature_script(CN_WAR_MASTER_VOONE, &WarMasterVooneAI::Create);
    mgr->register_creature_script(CN_MOTHER_SMOLDERWEB, &MotherSmolderwebAI::Create);
    mgr->register_creature_script(CN_UROK_DOOMHOWL, &UrokDoomhowlAI::Create);
    mgr->register_creature_script(CN_QUARTERMASTER_ZIGRIS, &QuartermasterZigrisAI::Create);
    mgr->register_creature_script(CN_HALYCON, &HalyconAI::Create);
    mgr->register_creature_script(CN_OVERLORD_WYRMTHALAK, &OverlordWyrmthalakAI::Create);
}
