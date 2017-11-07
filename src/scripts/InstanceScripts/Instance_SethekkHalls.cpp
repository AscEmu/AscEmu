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
#include "Instance_SethekkHalls.h"
#include "Objects/Faction.h"


// Avian Darkhawk AI
class AvianDarkhawkAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(AvianDarkhawkAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        AvianDarkhawkAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_AVIAN_DARKHAWK_CHARGE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 5.0f;
            spells[0].attackstoptimer = 1000;

            target = nullptr;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
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

        Unit* target;
        uint8 nrspells;
};

// Avian Ripper AI
class AvianRipperAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(AvianRipperAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        AvianRipperAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_AVIAN_RIPPER_FLESH_RIP);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 15.0f;
            spells[0].attackstoptimer = 3000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
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

// Avian Warhawk AI
class AvianWarhawkAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(AvianWarhawkAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        AvianWarhawkAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_AVIAN_WARHAWK_CLEAVE);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].perctrigger = 12.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_AVIAN_WARHAWK_CHARGE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 5.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_AVIAN_WARHAWK_BITE);
            spells[2].targettype = TARGET_ATTACKING; // check targeting!
            spells[2].instant = true;
            spells[2].perctrigger = 12.0f;
            spells[2].attackstoptimer = 1000;

            target = nullptr;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
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

        Unit* target;
        uint8 nrspells;
};

// Cobalt Serpent AI
class CobaltSerpentAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(CobaltSerpentAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        CobaltSerpentAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_COBALT_SERPENT_WING_BUFFET);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 7.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_COBALT_SERPENT_FROSTBOLT);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 15.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_COBALT_SERPENT_CHAIN_LIGHTNING);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].perctrigger = 9.0f;
            spells[2].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
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

// Time-Lost Controller AI
class TimeLostControllerAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(TimeLostControllerAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        TimeLostControllerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_TL_CONTROLLER_SHIRNK);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;
            /*
                    spells[1].info = sSpellCustomizations.GetSpellInfo(CONTROL_TOTEM);
                    spells[1].targettype = TARGET_;
                    spells[1].instant = false;
                    spells[1].perctrigger = 0.0f;
                    spells[1].attackstoptimer = 1000;
                    */
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
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

// Time-Lost Scryer AI
class TimeLostScryerAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(TimeLostScryerAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        TimeLostScryerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_TL_SCRYER_FLASH_HEAL);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = true;
            spells[0].perctrigger = 5.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_TL_SCRYER_ARCANE_MISSILES);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 12.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
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


// Time-Lost Shadowmage AI
class TimeLostShadowmageAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(TimeLostShadowmageAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        TimeLostShadowmageAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_TL_CURSE_OF_THE_DARK_TALON);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
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

// Sethekk Guard AI
class SethekkGuardAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(SethekkGuardAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        SethekkGuardAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SETHEKK_GUARD_THUNDERCLAP);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].perctrigger = 12.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_SETHEKK_GUARD_SUNDER_ARMOR);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 8.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
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

// Sethekk Initiate AI
class SethekkInitiateAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(SethekkInitiateAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        SethekkInitiateAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SETHEKK_INIT_MAGIC_REFLECTION);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = true;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
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

// Sethekk Oracle AI
class SethekkOracleAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(SethekkOracleAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        SethekkOracleAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SETHEKK_ORACLE_FAERIE_FIRE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_SETHEKK_ORACLE_ARCANE_LIGHTNING);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 15.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
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

// Sethekk Prophet AI
class SethekkProphetAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(SethekkProphetAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        SethekkProphetAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SETHEKK_PROPHET_FEAR);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;
            /*
                    spells[1].info = sSpellCustomizations.GetSpellInfo();
                    spells[1].targettype = TARGET_;
                    spells[1].instant = true;
                    spells[1].perctrigger = 0.0f;
                    spells[1].attackstoptimer = 1000;
                    */
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
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

// Sethekk Ravenguard AI
class SethekkRavenguardAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(SethekkRavenguardAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        SethekkRavenguardAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SETHEKK_RAVENG_BLOODTHIRST);
            spells[0].targettype = TARGET_ATTACKING;    //?
            spells[0].instant = true;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_SETHEKK_RAVENG_HOWLING_SCREECH);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].perctrigger = 8.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
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

// Sethekk Shaman AI
class SethekkShamanAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(SethekkShamanAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        SethekkShamanAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SETHEKK_SHAMAN_SUM_DARK_VORTEX);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = true;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
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

// Sethekk Talon Lord AI
class SethekkTalonLordAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(SethekkTalonLordAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        SethekkTalonLordAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SETHEKK_TALON_OF_JUSTICE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_SETHEKK_TALON_AVENGERS_SHIELD);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 7.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
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

////////////////////////////////////////////////////
// Lakka AI
static Movement::LocationWithFlag LakkaWaypoint[] =
{
    {},
    { -157.200f, 159.922f, 0.010f, 0.104f, Movement::WP_MOVE_TYPE_WALK },
    { -128.318f, 172.483f, 0.009f, 0.222f, Movement::WP_MOVE_TYPE_WALK },
    { -73.749f, 173.171f, 0.009f, 6.234f, Movement::WP_MOVE_TYPE_WALK },
};

class LakkaAI : public MoonScriptCreatureAI
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(LakkaAI);
        LakkaAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);

            //WPs
            for (uint8 i = 1; i < 4; ++i)
            {
                AddWaypoint(CreateWaypoint(i, 0, LakkaWaypoint[i].wp_flag, LakkaWaypoint[i].wp_location));
            }
        }

        void OnReachWP(uint32 iWaypointId, bool bForwards)
        {
            switch (iWaypointId)
            {
                case 1:
                {
                    SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    SetWaypointToMove(2);
                    Player* pPlayer = NULL;
                    QuestLogEntry* pQuest = NULL;
                    for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                    {
                        if ((*itr)->IsPlayer())
                        {
                            pPlayer = static_cast<Player*>((*itr));
                            if (pPlayer != NULL)
                            {
                                pQuest = pPlayer->GetQuestLogForEntry(10097);
                                if (pQuest != NULL && pQuest->GetMobCount(1) < 1)
                                {
                                    pQuest->SetMobCount(1, 1);
                                    pQuest->SendUpdateAddKill(1);
                                    pQuest->UpdatePlayerFields();
                                }
                            }
                        }
                    }
                }
                break;
                case 3:
                {
                    despawn(100, 0);
                }
                break;
                default:
                {
                    SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    SetWaypointToMove(1);
                }
            }
        }
};

////////////////////////////////////////////////////
// Boss AIs

// Darkweaver SythAI
class DarkweaverSythAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(DarkweaverSythAI);
        SP_AI_Spell spells[9];
        bool m_spellcheck[9];

        DarkweaverSythAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 5;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;

            }
            // Not sure in any way about target types
            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_DARKW_SYNTH_FROST_SHOCK);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 2000;
            spells[0].cooldown = 15;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_DARKW_SYNTH_FLAME_SHOCK);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 8.0f;
            spells[1].attackstoptimer = 2000;
            spells[1].cooldown = 15;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_DARKW_SYNTH_SHADOW_SHOCK);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].perctrigger = 8.0f;
            spells[2].attackstoptimer = 2000;
            spells[2].cooldown = 15;

            spells[3].info = sSpellCustomizations.GetSpellInfo(SP_DARKW_SYNTH_ARCANE_SHOCK);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = true;
            spells[3].perctrigger = 8.0f;
            spells[3].attackstoptimer = 2000;
            spells[3].cooldown = 15;

            spells[4].info = sSpellCustomizations.GetSpellInfo(SP_DARKW_SYNTH_CHAIN_LIGHTNING);
            spells[4].targettype = TARGET_ATTACKING;
            spells[4].instant = true;
            spells[4].perctrigger = 10.0f;
            spells[4].attackstoptimer = 1000;
            spells[4].cooldown = 15;

            spells[5].info = sSpellCustomizations.GetSpellInfo(SP_DARKW_SYNTH_SUM_FIRE_ELEMENTAL);
            spells[5].targettype = TARGET_SELF;
            spells[5].instant = true;
            spells[5].perctrigger = 0.0f;
            spells[5].attackstoptimer = 1000;
            spells[5].cooldown = 10;

            spells[6].info = sSpellCustomizations.GetSpellInfo(SP_DARKW_SYNTH_SUM_FROST_ELEMENTAL);
            spells[6].targettype = TARGET_SELF;
            spells[6].instant = true;
            spells[6].perctrigger = 0.0f;
            spells[6].attackstoptimer = 1000;
            spells[6].cooldown = -1;

            spells[7].info = sSpellCustomizations.GetSpellInfo(SP_DARKW_SYNTH_SUM_ARCANE_ELEMENTAL);
            spells[7].targettype = TARGET_SELF;
            spells[7].instant = true;
            spells[7].perctrigger = 0.0f;
            spells[7].attackstoptimer = 1000;
            spells[7].cooldown = -1;

            spells[8].info = sSpellCustomizations.GetSpellInfo(SP_DARKW_SYNTH_SUM_SHADOW_ELEMENTAL);
            spells[8].targettype = TARGET_SELF;
            spells[8].instant = true;
            spells[8].perctrigger = 0.0f;
            spells[8].attackstoptimer = 1000;
            spells[8].cooldown = -1;

            Summons = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            for (uint8 i = 0; i < 9; i++)
                spells[i].casttime = 0;

            Summons = 0;

            switch (RandomUInt(2))
            {
                case 0:
                    sendDBChatMessage(SAY_DARKW_SYNTH_02);
                    break;
                case 1:
                    sendDBChatMessage(SAY_DARKW_SYNTH_03);
                    break;
                case 2:
                    sendDBChatMessage(SAY_DARKW_SYNTH_04);
                    break;
            }

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (getCreature()->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        sendDBChatMessage(SAY_DARKW_SYNTH_05);
                        break;
                    case 1:
                        sendDBChatMessage(SAY_DARKW_SYNTH_06);
                        break;
                }
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            RemoveAIUpdateEvent();

            Summons = 0;
        }

        void OnDied(Unit* mKiller)
        {
            sendDBChatMessage(SAY_DARKW_SYNTH_07);

            GameObject* LakkasCage = getNearestGameObject(-160.813f, 157.043f, 0.194095f, 183051);
            Creature* mLakka = getNearestCreature(-160.813f, 157.043f, 0.194095f, 18956);

            if (LakkasCage != NULL)
            {
                LakkasCage->SetState(GO_STATE_OPEN);
                LakkasCage->SetFlags(LakkasCage->GetFlags() - 1);
            }

            if (mLakka != NULL && mLakka->GetScript())
            {
                MoonScriptCreatureAI* pLakkaAI = static_cast< MoonScriptCreatureAI* >(mLakka->GetScript());
                mLakka->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                pLakkaAI->SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                pLakkaAI->SetWaypointToMove(1);
                pLakkaAI->setAIAgent(AGENT_NULL);
            }
        }

        void AIUpdate()
        {
            uint32 t = (uint32)time(NULL);
            if (t > spells[4].casttime && ((getCreature()->GetHealthPct() <= 75 && Summons == 0) || (getCreature()->GetHealthPct() <= 50 && Summons == 1) || (getCreature()->GetHealthPct() <= 25 && Summons == 2)))
            {
                getCreature()->setAttackTimer(1500, false);
                getCreature()->GetAIInterface()->StopMovement(1000);    // really?

                SummonElementalWave();

                spells[4].casttime = t + spells[4].cooldown;
                Summons++;
                return;
            }

            else
            {
                float val = RandomFloat(100.0f);
                SpellCast(val);
            }
        }

        void SummonElementalWave()
        {
            sendDBChatMessage(SAY_DARKW_SYNTH_01);

            getCreature()->CastSpell(getCreature(), spells[5].info, spells[5].instant);
            getCreature()->CastSpell(getCreature(), spells[6].info, spells[6].instant);
            getCreature()->CastSpell(getCreature(), spells[7].info, spells[7].instant);
            getCreature()->CastSpell(getCreature(), spells[8].info, spells[8].instant);
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
                        if (!spells[i].instant)
                            getCreature()->GetAIInterface()->StopMovement(1);

                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
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

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint32 Summons;
        uint8 nrspells;
};

// Talon King IkissAI
class TalonKingIkissAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(TalonKingIkissAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        TalonKingIkissAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_TALRON_K_IKISS_ARCANE_VOLLEY);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 12.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].cooldown = 10;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_TALRON_K_IKISS_BLINK);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].perctrigger = 7.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].cooldown = 25;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_TALRON_K_IKISS_POLYMORPH);
            spells[2].targettype = TARGET_RANDOM_SINGLE;
            spells[2].instant = false;
            spells[2].perctrigger = 9.0f;
            spells[2].attackstoptimer = 1000;
            spells[2].cooldown = 15;
            spells[2].mindist2cast = 0.0f;
            spells[2].maxdist2cast = 40.0f;

            spells[3].info = sSpellCustomizations.GetSpellInfo(SP_TALRON_K_IKISS_ARCANE_EXPLOSION);
            spells[3].targettype = TARGET_VARIOUS;
            spells[3].instant = false;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;
            spells[3].cooldown = -1;

            Blink = false;
        }

        void OnCombatStart(Unit* mTarget)
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;
            uint32 t = (uint32)time(NULL);
            spells[1].casttime = t + RandomUInt(5) + 10;

            if (getCreature()->GetCurrentSpell() == NULL)
            {
                getCreature()->GetAIInterface()->StopMovement(1);
                getCreature()->setAttackTimer(3000, false);
                getCreature()->CastSpell(getCreature(), spells[0].info, spells[0].instant);

                spells[0].casttime = t + spells[0].cooldown;
            }

            Blink = false;

            switch (RandomUInt(2))
            {
                case 0:
                    sendDBChatMessage(SAY_TALRON_K_IKISS_02);
                    break;
                case 1:
                    sendDBChatMessage(SAY_TALRON_K_IKISS_03);
                    break;
                case 2:
                    sendDBChatMessage(SAY_TALRON_K_IKISS_04);
                    break;
            }

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnTargetDied(Unit* mTarget) // left to keep it easy to add needed data.
        {
            if (getCreature()->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        sendDBChatMessage(SAY_TALRON_K_IKISS_05);
                        break;
                    case 1:
                        sendDBChatMessage(SAY_TALRON_K_IKISS_06);
                        break;
                }
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            sendDBChatMessage(SAY_TALRON_K_IKISS_07);

            GameObject* IkissDoor = getNearestGameObject(43.079f, 149.505f, 0.034f, 183398);
            if (IkissDoor != NULL)
                IkissDoor->SetState(GO_STATE_OPEN);
        }

        void AIUpdate()
        {
            if (Blink)
            {
                getCreature()->GetAIInterface()->StopMovement(2000);
                getCreature()->setAttackTimer(6500, false);

                if (getCreature()->GetCurrentSpell())
                    getCreature()->GetCurrentSpell()->cancel();

                getCreature()->CastSpell(getCreature(), spells[3].info, spells[3].instant);

                Blink = false;
                return;
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
                        if (!spells[i].instant)
                            getCreature()->GetAIInterface()->StopMovement(1);

                        if (i == 1)
                            BlinkCast();
                        else
                        {
                            target = getCreature()->GetAIInterface()->getNextTarget();
                            switch (spells[i].targettype)
                            {
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
                                case TARGET_RANDOM_FRIEND:
                                case TARGET_RANDOM_SINGLE:
                                case TARGET_RANDOM_DESTINATION:
                                    CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                    break;
                            }
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
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

            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;        // From M4ksiu - Big THX to Capt
                for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(getCreature(), (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(getCreature(), (*itr)) && (*itr) != getCreature())) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && getCreature()->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && getCreature()->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (getCreature()->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(getCreature(), RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (getCreature()->GetHealthPct() >= minhp2cast && getCreature()->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(getCreature());

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
                        getCreature()->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        getCreature()->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

        void BlinkCast()
        {
            std::vector<Unit*> TargetTable;        // From M4ksiu - Big THX to Capt
            for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
            {
                if (isHostile(getCreature(), (*itr)) && (*itr) != getCreature() && (*itr)->IsUnit())
                {
                    Unit* RandomTarget = NULL;
                    RandomTarget = static_cast<Unit*>(*itr);
                    if (RandomTarget->isAlive() && getCreature()->GetDistance2dSq(RandomTarget) >= 0.0f && getCreature()->GetDistance2dSq(RandomTarget) <= 900.0f && getCreature()->GetAIInterface()->getThreatByPtr(RandomTarget) > 0)
                    {
                        TargetTable.push_back(RandomTarget);
                    }
                }
            }

            if (!TargetTable.size())
            {
                TargetTable.clear();
                return;
            }

            auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
            auto random_target = TargetTable[random_index];

            if (random_target == nullptr)
                return;

            getCreature()->GetAIInterface()->setNextTarget(random_target);
            getCreature()->CastSpell(getCreature(), spells[1].info, spells[1].instant);

            TargetTable.clear();

            getCreature()->setAttackTimer(2500, false);
            getCreature()->GetAIInterface()->StopMovement(2500);

            Blink = true;
        }

    protected:

        bool Blink;
        uint8 nrspells;
};

// AnzuAI
class ANZUAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ANZUAI);
        SP_AI_Spell spells[5];
        bool m_spellcheck[5];

        ANZUAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_ANZU_SPELL_BOMB);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_ANZU_CYCLONE_OF_FEATHERS);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 10.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_ANZU_PARALYZING_SCREECH);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = false;
            spells[2].perctrigger = 10.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(SP_ANZU_BANISH);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(SP_ANZU_SUMMON_RAVEN_GOD);
            spells[4].targettype = TARGET_SELF;
            spells[4].instant = true;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;

            getCreature()->CastSpell(getCreature(), spells[4].info, spells[4].instant);

            Banished = false;
            Summon = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));

            Banished = false;
            Summon = 0;
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            getCreature()->RemoveAura(SP_ANZU_BANISH);

            RemoveAIUpdateEvent();

            Banished = false;
            Summon = 0;
        }

        void AIUpdate()
        {
            if ((getCreature()->GetHealthPct() <= 66 && Summon == 0) || (getCreature()->GetHealthPct() <= 33 && Summon == 1))
            {
                SummonPhase();
                Summon++;
            }

            if (Banished)
            {
                uint32 t = (uint32)time(NULL);
                if (t > spells[4].casttime)
                {
                    getCreature()->RemoveAura(SP_ANZU_BANISH);

                    Banished = false;
                }

                else
                {
                    for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                    {
                        if ((*itr) != getCreature() && (*itr)->IsCreature())
                        {
                            Creature* Check = NULL;
                            Check = static_cast<Creature*>(*itr);

                            if (Check->GetEntry() != 23132)
                                continue;

                            if (Check->isAlive())
                                break;

                            getCreature()->RemoveAura(SP_ANZU_BANISH);
                        }
                    }
                }
                return;
            }

            else
            {
                float val = RandomFloat(100.0f);
                SpellCast(val);
            }
        }

        void SummonPhase()
        {
            getCreature()->CastSpell(getCreature(), spells[4].info, spells[4].instant);

            spells[4].casttime = (uint32)time(NULL) + 60;

            Banished = true;
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

                        if (spells[i].speech != "")
                        {
                            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            getCreature()->PlaySoundToSet(spells[i].soundid);
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

        // A bit rewritten FindTarget function
        Unit* FindTarget()
        {
            Unit* target = NULL;
            float distance = 80.0f;
            float z_diff;

            Unit* pUnit;
            float dist;

            for (std::set<Object*>::iterator itr = getCreature()->GetInRangeOppFactsSetBegin(); itr != getCreature()->GetInRangeOppFactsSetEnd(); ++itr)
            {
                if (!(*itr)->IsUnit())
                    continue;

                pUnit = static_cast<Unit*>((*itr));

                if (pUnit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH))
                    continue;

                z_diff = fabs(getCreature()->GetPositionZ() - pUnit->GetPositionZ());
                if (z_diff > 5.0f)
                    continue;

                if (!pUnit->isAlive() || getCreature() == pUnit)
                    continue;

                dist = getCreature()->GetDistance2dSq(pUnit);

                if (dist > distance * distance)
                    continue;

                distance = dist;
                target = pUnit;
            }

            return target;
        }

    protected:

        bool Banished;
        uint32 Summon;
        uint8 nrspells;
};

void SetupSethekkHalls(ScriptMgr* mgr)
{
    //Creatures
    mgr->register_creature_script(CN_AVIAN_DARKHAWK, &AvianDarkhawkAI::Create);
    mgr->register_creature_script(CN_AVIAN_RIPPER, &AvianRipperAI::Create);
    mgr->register_creature_script(CN_AVIAN_WARHAWK, &AvianWarhawkAI::Create);
    mgr->register_creature_script(CN_COBALT_SERPENT, &CobaltSerpentAI::Create);
    mgr->register_creature_script(CN_TIME_LOST_CONTROLLER, &TimeLostControllerAI::Create);
    mgr->register_creature_script(CN_TIME_LOST_SCRYER, &TimeLostScryerAI::Create);
    mgr->register_creature_script(CN_TIME_LOST_SHADOWMAGE, &TimeLostShadowmageAI::Create);
    mgr->register_creature_script(CN_SETHEKK_GUARD, &SethekkGuardAI::Create);
    mgr->register_creature_script(CN_SETHEKK_INITIATE, &SethekkInitiateAI::Create);
    mgr->register_creature_script(CN_SETHEKK_ORACLE, &SethekkOracleAI::Create);
    mgr->register_creature_script(CN_SETHEKK_PROPHET, &SethekkProphetAI::Create);
    mgr->register_creature_script(CN_SETHEKK_RAVENGUARD, &SethekkRavenguardAI::Create);
    mgr->register_creature_script(CN_SETHEKK_SHAMAN, &SethekkShamanAI::Create);
    mgr->register_creature_script(CN_SETHEKK_TALON_LORD, &SethekkTalonLordAI::Create);
    mgr->register_creature_script(CN_DARKWEAVER_SYTH, &DarkweaverSythAI::Create);
    mgr->register_creature_script(CN_TALON_KING_IKISS, &TalonKingIkissAI::Create);
    mgr->register_creature_script(CN_LAKKA, &LakkaAI::Create);
    //mgr->register_creature_script(CN_ANZU, &AnzuAI::Create); /// \todo Can't check Anzu he is in the DB right now
}
