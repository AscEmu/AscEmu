/*
* ArcScripts for ArcEmu MMORPG Server
* Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
* Copyright (C) 2008-2009 Sun++ Team <http://www.sunplusplus.info/>
* Copyright (C) 2005-2007 Ascent Team
* Copyright (C) 2007-2008 Moon++ Team <http://www.moonplusplus.info/>
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
#include "Instance_BlackMorass.h"


// ChronoLordAI
class ChronoLordAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ChronoLordAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        ChronoLordAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(ARCANE_BLAST);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 10;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(TIME_LAPSE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = 8;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            // new
            addEmoteForEvent(Event_OnCombatStart, SAY_CHRONOLORD_01);
            addEmoteForEvent(Event_OnTargetDied, SAY_CHRONOLORD_02);
            addEmoteForEvent(Event_OnTargetDied, SAY_CHRONOLORD_03);
            addEmoteForEvent(Event_OnDied, SAY_CHRONOLORD_04);
        }

        void OnCombatStart(Unit* mTarget) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (int i = 0; i < nrspells; ++i)
            {
                spells[i].casttime = spells[i].cooldown;
            }
        }

        void OnCombatStop(Unit* mTarget) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller) override
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
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
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


// TemporusAI
class TemporusAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TemporusAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        TemporusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(HASTEN);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = true;
            spells[0].cooldown = 10;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(MORTAL_WOUND);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = 5;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            // new
            addEmoteForEvent(Event_OnCombatStart, SAY_TEMPORUS_01);
            addEmoteForEvent(Event_OnTargetDied, SAY_TEMPORUS_02);
            addEmoteForEvent(Event_OnTargetDied, SAY_TEMPORUS_03);
            addEmoteForEvent(Event_OnDied, SAY_TEMPORUS_04);
        }

        void OnCombatStart(Unit* mTarget) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* mTarget) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller) override
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
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
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


//AenusAI
class AenusAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AenusAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        AenusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(SAND_BREATH);
            spells[0].targettype = TARGET_DESTINATION;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(TIME_STOP);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 15;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(FRENZY);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].cooldown = 8;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

            // new
            addEmoteForEvent(Event_OnCombatStart, SAY_AENUS_01);
            addEmoteForEvent(Event_OnTargetDied, SAY_AENUS_02);
            addEmoteForEvent(Event_OnTargetDied, SAY_AENUS_03);
            addEmoteForEvent(Event_OnDied, SAY_AENUS_04);
        }

        void OnCombatStart(Unit* mTarget) override
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* mTarget) override
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller) override
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
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
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

void SetupTheBlackMorass(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_CHRONO_LORD_DEJA, &ChronoLordAI::Create);
    mgr->register_creature_script(CN_TEMPORUS, &TemporusAI::Create);
    mgr->register_creature_script(CN_AEONUS, &AenusAI::Create);
}
