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
#include "Instance_BlackrockDepths.h"


class AmbassadorFlamelash : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AmbassadorFlamelash);
        SP_AI_Spell spell;
        bool m_spellcheck;

        AmbassadorFlamelash(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            spell.info = sSpellCustomizations.GetSpellInfo(15573);
            spell.targettype = TARGET_ATTACKING;
            spell.instant = true;
            spell.cooldown = 10;
            spell.perctrigger = RandomFloat(20.0f);
            spell.attackstoptimer = 1000;
            m_spellcheck = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            spell.casttime = spell.cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                spell.casttime--;

                if (m_spellcheck)
                {
                    spell.casttime = spell.cooldown;
                    target = getCreature()->GetAIInterface()->getNextTarget();
                    getCreature()->CastSpell(target, spell.info, spell.instant);

                    if (spell.speech != "")
                    {
                        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spell.speech.c_str());
                        getCreature()->PlaySoundToSet(spell.soundid);
                    }

                    m_spellcheck = false;
                    return;
                }

                if ((val > comulativeperc && val <= (comulativeperc + spell.perctrigger)) || !spell.casttime)
                {
                    getCreature()->setAttackTimer(spell.attackstoptimer, false);
                    m_spellcheck = true;
                }
                comulativeperc += spell.perctrigger;
            }
        }
};


class AnubShiah : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AnubShiah);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        AnubShiah(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(11661);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;


            spells[1].info = sSpellCustomizations.GetSpellInfo(15471);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;

        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


class BaelGar : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(BaelGar);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        BaelGar(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(13879);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;


            spells[1].info = sSpellCustomizations.GetSpellInfo(13895);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;

        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


class EmperorDagranThaurissan : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(EmperorDagranThaurissan);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        EmperorDagranThaurissan(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(17492);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;


            spells[1].info = sSpellCustomizations.GetSpellInfo(24573);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;


            spells[2].info = sSpellCustomizations.GetSpellInfo(20691);
            spells[2].cooldown = 10;
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].perctrigger = RandomFloat(20.0f);
            spells[2].attackstoptimer = 1000;
            m_spellcheck[2] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


class Eviscerator : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(Eviscerator);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        Eviscerator(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(14331);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;


            spells[1].info = sSpellCustomizations.GetSpellInfo(20741);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


class FineousDarkvire : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(FineousDarkvire);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        FineousDarkvire(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(15614);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;


            spells[1].info = sSpellCustomizations.GetSpellInfo(13953);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


class GeneralAngerforge : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GeneralAngerforge);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        GeneralAngerforge(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(14099);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(9080);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;

            spells[2].info = sSpellCustomizations.GetSpellInfo(20691);
            spells[2].cooldown = 10;
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].perctrigger = RandomFloat(20.0f);
            spells[2].attackstoptimer = 1000;
            m_spellcheck[2] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


class GolemLordArgelmach : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GolemLordArgelmach);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        GolemLordArgelmach(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(16033);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(16034);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;

            spells[2].info = sSpellCustomizations.GetSpellInfo(10432);
            spells[2].cooldown = 10;
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].perctrigger = RandomFloat(20.0f);
            spells[2].attackstoptimer = 1000;
            m_spellcheck[2] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


class GoroshTheDervish : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GoroshTheDervish);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        GoroshTheDervish(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(15589);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(24573);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


class Grizzle : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(Grizzle);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        Grizzle(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(6524);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(20691);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


class HedrumTheCreeper : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HedrumTheCreeper);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        HedrumTheCreeper(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(15475);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(15474);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;

            spells[2].info = sSpellCustomizations.GetSpellInfo(3609);
            spells[2].cooldown = 10;
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].perctrigger = RandomFloat(20.0f);
            spells[2].attackstoptimer = 1000;
            m_spellcheck[2] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

class HighInterrogatorGerstahn : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HighInterrogatorGerstahn);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        HighInterrogatorGerstahn(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(10894);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(8122);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;

            spells[2].info = sSpellCustomizations.GetSpellInfo(10876);
            spells[2].cooldown = 10;
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].perctrigger = RandomFloat(20.0f);
            spells[2].attackstoptimer = 1000;
            m_spellcheck[2] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

class HoundmasterGrebmar : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HoundmasterGrebmar);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        HoundmasterGrebmar(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(23511);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(17153);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


class HurleyBlackbreath : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HurleyBlackbreath);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        HurleyBlackbreath(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(17294);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(15583);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;

            spells[2].info = sSpellCustomizations.GetSpellInfo(14099);
            spells[2].cooldown = 10;
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].perctrigger = RandomFloat(20.0f);
            spells[2].attackstoptimer = 1000;
            m_spellcheck[2] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


class LordIncendius : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LordIncendius);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        LordIncendius(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(13899);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(13900);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;

            spells[2].info = sSpellCustomizations.GetSpellInfo(14099);
            spells[2].cooldown = 10;
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].perctrigger = RandomFloat(20.0f);
            spells[2].attackstoptimer = 1000;
            m_spellcheck[2] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


class LordRoccor : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LordRoccor);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        LordRoccor(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(10448);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(6524);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;

            spells[2].info = sSpellCustomizations.GetSpellInfo(10414);
            spells[2].cooldown = 10;
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].perctrigger = RandomFloat(20.0f);
            spells[2].attackstoptimer = 1000;
            m_spellcheck[2] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


class Magmus : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(Magmus);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        Magmus(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(13900);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(24375);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

class OkThorTheBreaker : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(OkThorTheBreaker);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        OkThorTheBreaker(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(15453);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(15451);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

class Phalanx : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(Phalanx);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        Phalanx(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(8732);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(22425);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;

            spells[2].info = sSpellCustomizations.GetSpellInfo(14099);
            spells[2].cooldown = 10;
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].perctrigger = RandomFloat(20.0f);
            spells[2].attackstoptimer = 1000;
            m_spellcheck[2] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

class PrincessMoiraBronzebeard : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(PrincessMoiraBronzebeard);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        PrincessMoiraBronzebeard(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(10947);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(22645);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

class PyromancerLoregrain : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(PyromancerLoregrain);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        PyromancerLoregrain(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(10448);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = true;

            spells[1].info = sSpellCustomizations.GetSpellInfo(15095);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            CastTime();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

void SetupBlackrockDepths(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_AMBASSADOR_FLAMELASH, &AmbassadorFlamelash::Create);
    mgr->register_creature_script(CN_ANUB_SHIAH, &AnubShiah::Create);
    mgr->register_creature_script(CN_BAEL_GAR, &BaelGar::Create);
    mgr->register_creature_script(CN_DRAGRAN_THAURISSAN, &EmperorDagranThaurissan::Create);
    mgr->register_creature_script(CN_EVISCERATOR, &Eviscerator::Create);
    mgr->register_creature_script(CN_FINEOUS_DARKVIRE, &FineousDarkvire::Create);
    mgr->register_creature_script(CN_GENERAL_ANGERFORGE, &GeneralAngerforge::Create);
    mgr->register_creature_script(CN_GOLEM_LORD_ARGELMACH, &GolemLordArgelmach::Create);
    //mgr->register_creature_script(CN_GOROSH_THE_DERVISH, &GoroshtheDervish::Create); - AI must be added ;)
    mgr->register_creature_script(CN_GRIZZLE, &Grizzle::Create);
    mgr->register_creature_script(CN_HEDRUM_THE_CREEPER, &HedrumTheCreeper::Create);
    mgr->register_creature_script(CN_INTERROGATOR_GERSTAHN, &HighInterrogatorGerstahn::Create);
    mgr->register_creature_script(CN_HOUNDMASTER_GREBMAR, &HoundmasterGrebmar::Create);
    mgr->register_creature_script(CN_HURLEY_BLACKBREATH, &HurleyBlackbreath::Create);
    mgr->register_creature_script(CN_LORD_INCENDIUS, &LordIncendius::Create);
    mgr->register_creature_script(CN_LORD_ROCCOR, &LordRoccor::Create);
    mgr->register_creature_script(CN_MAGMUS, &Magmus::Create);
    mgr->register_creature_script(CN_OKTHOR_THE_BREAKER, &OkThorTheBreaker::Create);
    mgr->register_creature_script(CN_PHALANX, &Phalanx::Create);
    mgr->register_creature_script(CN_MOIRA_BRONZEBART, &PrincessMoiraBronzebeard::Create);
    mgr->register_creature_script(CN_PYROMANCER_LOREGRAIN, &PyromancerLoregrain::Create);
}
