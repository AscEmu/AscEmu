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
#include "Instance_Scholomance.h"


// Doctor Theolen KrastinovAI
class DoctorTheolenKrastinovAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(DoctorTheolenKrastinovAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        DoctorTheolenKrastinovAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            FRENZY_LIMITER = 0;
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_DR_THEOL_REND);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 20.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_DR_THEOL_KRASTINOVCLEAVE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 9.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_DR_THEOL_FRENZY);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget) override
        {
            FRENZY_LIMITER = 0;
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget) override
        {
            FRENZY_LIMITER = 0;
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller) override
        {
            FRENZY_LIMITER = 0;
        }

        void AIUpdate() override
        {
            if (getCreature()->GetHealthPct() <= 50 && FRENZY_LIMITER == 0)
            {
                // FRENZY
                getCreature()->CastSpell(getCreature(), spells[2].info, spells[2].instant);
                FRENZY_LIMITER = 1;
            }
            else
            {
                float val = RandomFloat(100.0f);
                SpellCast(val);
            }
        }

        void SpellCast(float val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            target = getCreature()->GetAIInterface()->getNextTarget();
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint32 FRENZY_LIMITER;
        uint8 nrspells;
};

// Instructor MaliciaAI
class InstructorMaliciaAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(InstructorMaliciaAI);
        SP_AI_Spell spells[5];
        bool m_spellcheck[5];

        InstructorMaliciaAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 5;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_MALICIA_CALL_OF_GRAVE);
            spells[0].targettype = TARGET_DESTINATION; // VARIOUS, DESINATION or ATTACKING?
            spells[0].instant = true;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_MALICIA_CORRUPTION);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;    // should be false, but doesn't work then
            spells[1].perctrigger = 9.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_MALICIA_FLASH_HEAL);
            spells[2].targettype = TARGET_SELF;    // works for caster and his enemy :o
            spells[2].instant = true;    // should be false, but doesn't work then
            spells[2].perctrigger = 5.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(SP_MALICIA_RENEW);
            spells[3].targettype = TARGET_SELF;    // w00t, heals player too ? :|
            spells[3].instant = true;
            spells[3].perctrigger = 4.0f;    // why 0?:|
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(SP_MALICIA_HEAL);
            spells[4].targettype = TARGET_SELF;
            spells[4].instant = true;    // should be false, but doesn't work then
            spells[4].perctrigger = 5.0f;
            spells[4].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget) override
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            target = getCreature()->GetAIInterface()->getNextTarget();
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint8 nrspells;
};

// The RavenianAI
class TheRavenianAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TheRavenianAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        TheRavenianAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_RAVENIAN_TRAMPLE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_RAVENIAN_RAVENIANCLEAVE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 9.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_RAVENIAN_SUNDERINCLEAVE);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].perctrigger = 20.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(SP_RAVENIAN_KNOCKAWAY);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = true;
            spells[3].perctrigger = 11.0f;
            spells[3].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget) override
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            target = getCreature()->GetAIInterface()->getNextTarget();
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint8 nrspells;
};

// Lady Illucia BarovAI
class LadyIlluciaBarovAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LadyIlluciaBarovAI);
        SP_AI_Spell spells[5];
        bool m_spellcheck[5];

        LadyIlluciaBarovAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 5;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_ILLUCIA_CURSE_OF_AGONY);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_ILLUCIA_SHADOW_SHOCK);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].perctrigger = 12.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_ILLUCIA_SILENCE);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].perctrigger = 8.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(SP_ILLUCIA_FEAR);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = true;
            spells[3].perctrigger = 4.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(SP_ILLUCIA_DOMINATE_MIND);
            spells[4].targettype = TARGET_ATTACKING;
            spells[4].instant = true;
            spells[4].perctrigger = 4.0f;
            spells[4].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget) override
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            target = getCreature()->GetAIInterface()->getNextTarget();
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint8 nrspells;
};

// Ras ForstwhisperAI
class RasForstwhisperAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(RasForstwhisperAI);
        SP_AI_Spell spells[6];
        bool m_spellcheck[6];

        RasForstwhisperAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 6;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_RAS_FORTH_FROSTBOLT);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].perctrigger = 14.0f;
            spells[0].attackstoptimer = 2000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_RAS_FORTH_ICE_ARMOR);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_RAS_FORTH_FREEZE);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].perctrigger = 11.0f;
            spells[2].attackstoptimer = 4000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(SP_RAS_FORTH_FEAR);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = true;    // should be false, but doesn't work then
            spells[3].perctrigger = 9.0f;
            spells[3].attackstoptimer = 2000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(SP_RAS_FORTH_CHILL_NOVA);
            spells[4].targettype = TARGET_VARIOUS;
            spells[4].instant = true;
            spells[4].perctrigger = 8.0f;
            spells[4].attackstoptimer = 1000;

            spells[5].info = sSpellCustomizations.GetSpellInfo(SP_RAS_FORTH_FROSTB_VOLLEY);
            spells[5].targettype = TARGET_VARIOUS;
            spells[5].instant = true;    // should be false, but doesn't work then
            spells[5].perctrigger = 13.0f;
            spells[5].attackstoptimer = 2000;
        }

        void OnCombatStart(Unit* mTarget) override
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
            getCreature()->CastSpell(getCreature(), spells[1].info, spells[1].instant);
        }

        void OnCombatStop(Unit* mTarget) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            target = getCreature()->GetAIInterface()->getNextTarget();
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint8 nrspells;
};

// Jandice BarovAI
class JandiceBarovAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(JandiceBarovAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        JandiceBarovAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_JANDICE_CURSE_OF_BLOOD);
            spells[0].targettype = TARGET_DESTINATION;
            spells[0].instant = true;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_JANDICE_BANISH);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 8.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_JANDICE_SUMMON_ILLUSION);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].perctrigger = 5.0f;
            spells[2].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget) override
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            target = getCreature()->GetAIInterface()->getNextTarget();
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint8 nrspells;
};

// KormokAI
class KormokAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(KormokAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        KormokAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_KORMOK_SHADOW_B_VOLLEY);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].perctrigger = 11.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_KORMOK_BONE_SHIELD);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_KORMOK_SUM_RISEY_LACKEY);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = false;
            spells[2].perctrigger = 4.0f;
            spells[2].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget) override
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
            getCreature()->CastSpell(getCreature(), spells[1].info, spells[1].instant);
        }

        void OnCombatStop(Unit* mTarget) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            target = getCreature()->GetAIInterface()->getNextTarget();
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint8 nrspells;
};

// VectusAI
class VectusAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(VectusAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        VectusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            FRENZY_LIMITER = 0;
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_VECTUS_BLAST_WAVE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 18.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_VECTUS_FIRE_SHIELD);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].perctrigger = 5.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_VECTUS_FRENZY);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget) override
        {
            FRENZY_LIMITER = 0;
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
            getCreature()->CastSpell(getCreature(), spells[1].info, spells[1].instant);
        }

        void OnCombatStop(Unit* mTarget) override
        {
            FRENZY_LIMITER = 0;
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller) override
        {
            FRENZY_LIMITER = 0;
        }

        void AIUpdate() override
        {
            if (getCreature()->GetHealthPct() <= 25 && !FRENZY_LIMITER)
            {
                getCreature()->CastSpell(getCreature(), spells[2].info, spells[2].instant);
                FRENZY_LIMITER = 1;
            }
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            target = getCreature()->GetAIInterface()->getNextTarget();
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint32 FRENZY_LIMITER;
        uint8 nrspells;
};

// Lord Alexei BarovAI
class LordAlexeiBarovAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LordAlexeiBarovAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        LordAlexeiBarovAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_ALEXEI_UNHOLY_AURA);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = true;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_ALEXEI_IMMOLATE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 10.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_ALEXEI_VEIL_OF_SHADOW);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = false;
            spells[2].perctrigger = 8.0f;
            spells[2].attackstoptimer = 2000;
        }

        void OnCombatStart(Unit* mTarget) override
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
            getCreature()->CastSpell(getCreature(), spells[0].info, spells[0].instant);
        }

        void OnCombatStop(Unit* mTarget) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            target = getCreature()->GetAIInterface()->getNextTarget();
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint8 nrspells;
};

// Lorekeeper PolkeltAI
class LorekeeperPolkeltAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LorekeeperPolkeltAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        LorekeeperPolkeltAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_LORE_VOLATILE_INFECTION);
            spells[0].targettype = TARGET_ATTACKING;    // various affects caster too
            spells[0].instant = true;
            spells[0].perctrigger = 6.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_LORE_DARK_PLAGUE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 8.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_LORE_CORROSIVE_ACID);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;    // should be false, but doesn't work then =/
            spells[2].perctrigger = 12.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(SP_LORE_NOXIOUS_CATALYST);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = true;
            spells[3].perctrigger = 10.0f;
            spells[3].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget) override
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            target = getCreature()->GetAIInterface()->getNextTarget();
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint8 nrspells;
};

// Darkmaster GandlingAI
class DarkmasterGandlingAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(DarkmasterGandlingAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        DarkmasterGandlingAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3; // 4 ?
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_GANDLING_ARCANE_MISSILES); //VOLATILEINFECTION ???????? :|
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 12.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_GANDLING_COT_DARKMASTER);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 7.0f;
            spells[1].attackstoptimer = 2000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_GANDLING_SHADOW_SHIELD);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget) override
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
            getCreature()->CastSpell(getCreature(), spells[2].info, spells[2].instant);
        }

        void OnCombatStop(Unit* mTarget) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            target = getCreature()->GetAIInterface()->getNextTarget();
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint8 nrspells;
};

void SetupScholomance(ScriptMgr* mgr)
{
    //Creatures
    mgr->register_creature_script(CN_DOCTOR_THEOLEN_KRASTINOV, &DoctorTheolenKrastinovAI::Create);
    mgr->register_creature_script(CN_INSTRUCTOR_MALICIA, &InstructorMaliciaAI::Create);
    mgr->register_creature_script(CN_THE_RAVENIAN, &TheRavenianAI::Create);
    mgr->register_creature_script(CN_LADY_ILLUCIA_BAROV, &LadyIlluciaBarovAI::Create);
    mgr->register_creature_script(CN_RAS_FORSTWHISPER, &RasForstwhisperAI::Create);
    mgr->register_creature_script(CN_JANDICE_BAROV, &JandiceBarovAI::Create);
    mgr->register_creature_script(CN_KORMOK, &KormokAI::Create);
    mgr->register_creature_script(CN_VECTUS, &VectusAI::Create);
    mgr->register_creature_script(CN_LORD_ALEXEI_BAROV, &LordAlexeiBarovAI::Create);
    mgr->register_creature_script(CN_LOREKEEPER_POLKELT, &LorekeeperPolkeltAI::Create);
    mgr->register_creature_script(CN_DARKMASTER_GANDLING, &DarkmasterGandlingAI::Create);
}

// Who should be added ?
//    *Kirtonos the Herald
//    *Death Knight Darkreaver
//    *Lord Blackwood
//    *Marduk Blackpool
//    *Rattlegore
