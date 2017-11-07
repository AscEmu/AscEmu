/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
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
#include "Instance_TheSlavePens.h"
#include "Objects/Faction.h"


// CoilfangChampionAI
// It should also have effect on other allies of target, but somehow it affects caster too (we can use only this: 38946 as workaround
// But I think it's better to leave it as it is to not change it in a future as this effect will be repaired)
// In Heroid Mode is immune to Mind Control and Seduction (same for Bogstrok)
class CoilfangChampionAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangChampionAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        CoilfangChampionAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(INTIMIDATING_SHOUT);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
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
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
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

// CoilfangObserverAI
// In Heroic mode, it becomes immune to Mind Control and Seduction, but can still be feared and frozen by traps.
class CoilfangObserverAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangObserverAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        CoilfangObserverAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(IMMOLATE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].cooldown = 35;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
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
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
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


// CoilfangDefenderAI
// Cannot be trapped or feared, but can be stunned, polymorphed, cycloned and disarmed.
// In Heroic, immune to mind control, seduce and sheep, but still vulnerable to stun and kiting.
// Stealth Detectors, thus cannot be sapped.
class CoilfangDefenderAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangDefenderAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        CoilfangDefenderAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(REFLECTIVE_SHIELD);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true; // for other spells than 41475 false!
            spells[0].cooldown = 40; // 20
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
            getCreature()->CastSpell(getCreature(), spells[0].info, spells[0].instant);
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
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
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


// CoilfangScaleHealerAI
// Priests can Mind Control these; the surrounding mobs will kill them quickly.
// Note: Idk if it casts those spells, but it has them when it's mind controlled.
class CoilfangScaleHealerAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangScaleHealerAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        CoilfangScaleHealerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(HOLY_NOVA);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = 20;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(POWER_WORD_SHIELD);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].cooldown = 45;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(GREATER_HEAL);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = false;
            spells[2].cooldown = 50;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
            getCreature()->CastSpell(getCreature(), spells[1].info, spells[1].instant);

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
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
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


// CoilfangSoothsayerAI
// All forms of crowd control work on it.
class CoilfangSoothsayerAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangSoothsayerAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        CoilfangSoothsayerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(MIND_CONTROL);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 40;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
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
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
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


// CoilfangTechnicianAI
// Can be seduced and mind controlled in heroic mode.
// Note: Idk if it casts those spells, but it has them when it's mind controlled.
class CoilfangTechnicianAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangTechnicianAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        CoilfangTechnicianAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(RAIN_OF_FIRE);
            spells[0].targettype = TARGET_DESTINATION;
            spells[0].instant = true;
            spells[0].cooldown = 35;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(BLIZZARD);
            spells[1].targettype = TARGET_DESTINATION;
            spells[1].instant = false;
            spells[1].cooldown = 20;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
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
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
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

// CoilfangRayAI
// All forms of Beast crowd control work.
class CoilfangRayAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangRayAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        CoilfangRayAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(HOWL_OF_TERROR);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].cooldown = 20;
            spells[0].perctrigger = 2.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
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
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
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


//////////////////////////////////////////////////////////////
// Boss AIs
//////////////////////////////////////////////////////////////

// TotemsAI
class TotemsAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(TotemsAI);

        TotemsAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            uint32 Despawn = 30000;
            uint32 AIUpdate = 1000;

            SpellID = 1;
            switch (getCreature()->GetEntry())
            {
                case CN_MENNUS_HEALING_WARD:
                    SpellID = 34977;
                    break;
                case CN_TAINED_EARTHGRAB_TOTEM:
                    SpellID = 20654;
                    AIUpdate = 5000;
                    break;
                case CN_TAINED_STONESKIN_TOTEM:
                    Despawn = 60000;
                    SpellID = 25509;    // temporary spell
                    AIUpdate = 0;
                    break;
                default:    // for Corrupted Nova Totem and it's also safe case
                    {
                        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);    // hax
                        Despawn = 6000;
                        SpellID = 33132;
                        AIUpdate = 5000;
                    }
            }

            if (AIUpdate != 0)
                RegisterAIUpdateEvent(AIUpdate);

            setAIAgent(AGENT_SPELL);
            getCreature()->Despawn(Despawn, 0);
            getCreature()->m_noRespawn = true;

            getCreature()->CastSpell(getCreature(), SpellID, true);
        }

        void AIUpdate()
        {
            if (getCreature()->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2))
                getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);

            getCreature()->CastSpell(getCreature(), SpellID, true);
        }

    protected:

        uint32 SpellID;
};

// Mennu the BetrayerAI
uint32 Totems[4] = { 20208, 18176, 18177, 14662 };

class MennuTheBetrayerAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(MennuTheBetrayerAI);
        SP_AI_Spell spells[5];
        bool m_spellcheck[5];
        bool SummonedTotems[4];

        MennuTheBetrayerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            for (uint8 i = 0; i < 4; i++)
                SummonedTotems[i] = false;

            spells[0].info = sSpellCustomizations.GetSpellInfo(LIGHTNING_BOLT);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 5000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(MENNUS_HEALING_WARD);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].cooldown = 20;
            spells[1].perctrigger = 10.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(TAINTED_EARTHGRAB_TOTEM);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].cooldown = 0;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(TAINTED_STONESKIN_TOTEM);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = 0;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(CORRUPTED_NOVA_TOTEM);
            spells[4].targettype = TARGET_SELF;
            spells[4].instant = true;
            spells[4].cooldown = 0;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;

            TotemCounter = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            for (uint8 i = 0; i < 4; i++)
                SummonedTotems[i] = false;

            TotemCounter = 0;

            switch (RandomUInt(2))
            {
                case 0:
                    sendDBChatMessage(SAY_MENNU_BETRAYER_01);
                    break;
                case 1:
                    sendDBChatMessage(SAY_MENNU_BETRAYER_02);
                    break;
                case 2:
                    sendDBChatMessage(SAY_MENNU_BETRAYER_03);
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
                        sendDBChatMessage(SAY_MENNU_BETRAYER_04);
                        break;
                    case 1:
                        sendDBChatMessage(SAY_MENNU_BETRAYER_05);
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
            sendDBChatMessage(SAY_MENNU_BETRAYER_06);
        }

        void AIUpdate()
        {
            if (TotemCounter != 0 && getCreature()->GetCurrentSpell() == NULL)
            {
                TotemSpawning();
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

                        if (i == 1)
                        {
                            spells[1].casttime = (uint32)time(NULL) + spells[1].cooldown;
                            TotemSpawning();
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
        // Random totem spawning
        void TotemSpawning()
        {
            getCreature()->setAttackTimer(1500, false);

            bool Spawned = false;
            uint32 Counter = 0;
            while(!Spawned)
            {
                if (Counter >= 2)
                {
                    for (uint8 i = 0; i < 4; i++)
                    {
                        if (!SummonedTotems[i])
                        {
                            spawnCreature(Totems[i], getCreature()->GetPosition());
                            getCreature()->CastSpell(getCreature(), spells[i + 1].info, spells[i + 1].instant);

                            SummonedTotems[i] = true;
                            TotemCounter++;
                            break;
                        }
                    }

                    Spawned = true;
                }

                uint32 i = RandomUInt(3);
                if (SummonedTotems[i])
                    Counter++;
                else
                {
                    spawnCreature(Totems[i], getCreature()->GetPosition());
                    getCreature()->CastSpell(getCreature(), spells[i + 1].info, spells[i + 1].instant);

                    SummonedTotems[i] = true;
                    TotemCounter++;
                    Spawned = true;
                }
            }

            if (TotemCounter == 4)
            {
                for (uint8 i = 0; i < 4; i++)
                    SummonedTotems[i] = false;

                TotemCounter = 0;
            }
        }

    protected:

        uint32 TotemCounter;
        uint8 nrspells;
};

// Rokmar the CracklerAI
class RokmarTheCracklerAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(RokmarTheCracklerAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        RokmarTheCracklerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;

            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(GRIEVOUS_WOUND);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 20;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(WATER_SPIT);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = false;
            spells[1].cooldown = 10;
            spells[1].perctrigger = 16.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(ENSNARING_MOSS);
            spells[2].targettype = TARGET_RANDOM_SINGLE;
            spells[2].instant = false;
            spells[2].cooldown = 35;
            spells[2].perctrigger = 8.0f;
            spells[2].attackstoptimer = 1000;
            spells[2].mindist2cast = 0.0f;
            spells[2].maxdist2cast = 30.0f;

            spells[3].info = sSpellCustomizations.GetSpellInfo(ENRAGE);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = 0;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            Enraged = false;
        }

        void OnCombatStart(Unit* mTarget)
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));

            Enraged = false;
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            if (getCreature()->GetHealthPct() <= 20 && !Enraged)
            {
                getCreature()->CastSpell(getCreature(), spells[3].info, spells[3].instant);

                Enraged = true;
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

                        target = getCreature()->GetAIInterface()->getNextTarget();
                        if ((i == 0 && getCreature()->GetDistance2dSq(target) <= 100.0f) || i != 0)
                        {
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

    protected:

        bool Enraged;
        uint8 nrspells;
};


// QuagmirranAI
class QuagmirranAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(QuagmirranAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        QuagmirranAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(ACID_GEYSER);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = false;
            spells[0].cooldown = 20;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].mindist2cast = 0.0f;
            spells[0].maxdist2cast = 40.0f;

            spells[1].info = sSpellCustomizations.GetSpellInfo(POISON_BOLT_VOLLEY);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 10;
            spells[1].perctrigger = 15.0f;
            spells[1].attackstoptimer = 2000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(CLEAVE);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = 15;
            spells[2].perctrigger = 6.0f;
            spells[2].attackstoptimer = 2000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

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
                        if (!spells[i].instant)
                            getCreature()->GetAIInterface()->StopMovement(1);

                        target = getCreature()->GetAIInterface()->getNextTarget();
                        if ((i == 2 && getCreature()->GetDistance2dSq(target) <= 100.0f) || i != 2)
                        {
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

                if (i == 0)
                {
                    getCreature()->GetAIInterface()->StopMovement(9000);
                    getCreature()->setAttackTimer(9000, false);
                }

                TargetTable.clear();
            }
        }

    protected:

        uint8 nrspells;
};


/// \note Coilfang Slavemaster was already scripted in SteamVaults, so I haven't
// copied/pasted it here.
// Still many NPCs left and I don't have infos if any of those use any spell
void SetupTheSlavePens(ScriptMgr* mgr)
{
    //Creatures
    /*mgr->register_creature_script(CN_COILFANG_CHAMPION, &CoilfangChampionAI::Create);
    mgr->register_creature_script(CN_COILFANG_OBSERVER, &CoilfangObserverAI::Create);
    mgr->register_creature_script(CN_COILFANG_DEFENDER, &CoilfangDefenderAI::Create);
    mgr->register_creature_script(CN_COILFANG_SCALE_HEALER, &CoilfangScaleHealerAI::Create);
    mgr->register_creature_script(CN_COILFANG_SOOTHSAYER, &CoilfangSoothsayerAI::Create);
    mgr->register_creature_script(CN_COILFANG_TECHNICIAN, &CoilfangTechnicianAI::Create);
    mgr->register_creature_script(CN_COILFANG_RAY, &CoilfangRayAI::Create);*/
    mgr->register_creature_script(CN_MENNUS_HEALING_WARD, &TotemsAI::Create);
    mgr->register_creature_script(CN_TAINED_EARTHGRAB_TOTEM, &TotemsAI::Create);
    mgr->register_creature_script(CN_TAINED_STONESKIN_TOTEM, &TotemsAI::Create);
    mgr->register_creature_script(CN_CORRUPTED_NOVA_TOTEM, &TotemsAI::Create);
    mgr->register_creature_script(CN_MENNU_THE_BETRAYER, &MennuTheBetrayerAI::Create);
    mgr->register_creature_script(CN_ROKMAR_THE_CRACKLER, &RokmarTheCracklerAI::Create);
    mgr->register_creature_script(CN_QUAGMIRRAN, &QuagmirranAI::Create);
}
