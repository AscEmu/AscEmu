/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
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
#include "Instance_TheShatteredHalls.h"
#include "Objects/Faction.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Hellfire Citadel: The Shattered Halls
class InstanceTheShatteredHallsScript : public InstanceScript
{
    public:

        InstanceTheShatteredHallsScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
        {}

        static InstanceScript* Create(MapMgr* pMapMgr) { return new InstanceTheShatteredHallsScript(pMapMgr); }
};

// FelOrcConvertAI
class FelOrcConvertAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(FelOrcConvertAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        FelOrcConvertAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_FEL_ORC_CONVERTER_HEMORRHAGE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 25;
            spells[0].perctrigger = 0.0f;
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
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


// ShatteredHandHeathenAI
class ShatteredHandHeathenAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandHeathenAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        ShatteredHandHeathenAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_HAND_HEATHEN_BLOODTHIRST);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 25;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_HAND_HEATHEN_ENRAGE);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].cooldown = 70;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            _unit->CastSpell(_unit, spells[1].info, spells[1].instant);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

// ShatteredHandLegionnaireAI
class ShatteredHandLegionnaireAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandLegionnaireAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        ShatteredHandLegionnaireAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_HAND_LEGI_AURA_OF_DISCIPLINE);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = -1;    // no idea if this should be like that
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_HAND_LEGI_PUMMEL);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = 25;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_HAND_LEGI_ENRAGE);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].cooldown = 70;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            _unit->CastSpell(_unit, spells[0].info, spells[0].instant);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

// ShatteredHandSavageAI
class ShatteredHandSavageAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandSavageAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        ShatteredHandSavageAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_HAND_SAVAGE_SLICE_AND_DICE);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = true;
            spells[0].cooldown = 35;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_HAND_SAVAGE_ENRAGE);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].cooldown = 70;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_HAND_SAVAGE_DEATHBLOW);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = 25;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            _unit->CastSpell(_unit, spells[1].info, spells[1].instant);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


// ShadowmoonAcolyteAI
class ShadowmoonAcolyteAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ShadowmoonAcolyteAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        ShadowmoonAcolyteAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SHADOWMOON_ACOLYTE_HEAL);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = false;
            spells[0].cooldown = 35;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_SHADOWMOON_ACOLYTE_PW_SHIELD);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].cooldown = 45;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_SHADOWMOON_ACOLYTE_MIND_BLAST);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].cooldown = 10;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(SP_SHADOWMOON_ACOLYTE_RESIST_SHADOW);
            spells[3].targettype = TARGET_SELF; // should be ally
            spells[3].instant = true;
            spells[3].cooldown = 65;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            if (RandomUInt(4) == 1)
                _unit->CastSpell(_unit, spells[3].info, spells[3].instant);
            else
                spells[3].casttime = RandomUInt(20) + 10;
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

// ShatteredHandAssassinAI
class ShatteredHandAssassinAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandAssassinAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        ShatteredHandAssassinAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_ASSASSIN_SAP);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_ASSASSIN_STEALTH);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_ASSASSIN_CHEAP_SHOT);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = 25;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

            _unit->CastSpell(_unit, spells[1].info, spells[1].instant);

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            if (_unit->GetCurrentSpell() && mTarget)
                _unit->CastSpell(mTarget, spells[0].info, spells[0].instant);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            _unit->CastSpell(_unit, spells[1].info, spells[1].instant);
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
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
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

// ShatteredHandGladiatorAI
class ShatteredHandGladiatorAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandGladiatorAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        ShatteredHandGladiatorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_GLADI_MORTAL_STRIKE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 0.0f;
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
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

// ShatteredHandHoundmasterAI
// he patrols with Rabid Warhounds
class ShatteredHandHoundmasterAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandHoundmasterAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        ShatteredHandHoundmasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_HOUNDMASTER_VOLLEY);
            spells[0].targettype = TARGET_DESTINATION;
            spells[0].instant = false;
            spells[0].cooldown = 30;
            spells[0].perctrigger = 0.0f;
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
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

// ShatteredHandReaverAI
class ShatteredHandReaverAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandReaverAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        ShatteredHandReaverAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_REAVER_CLEAVE);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = 10;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_REAVER_UPPERCUT);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = 35;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_REAVER_ENRAGE);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].cooldown = 70;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            if (RandomUInt(4) == 1)
                _unit->CastSpell(_unit, spells[2].info, spells[2].instant);
            else
                spells[2].casttime = RandomUInt(30) + 20;
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

// ShatteredHandSentryAI
class ShatteredHandSentryAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandSentryAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        ShatteredHandSentryAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_SENTRY_HAMSTERING);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 20;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_SENTRY_CHARGE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 0.0f;
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
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            if (_unit->GetAIInterface()->getNextTarget() && _unit->GetCurrentSpell())
            {
                Unit* target = NULL;
                target = _unit->GetAIInterface()->getNextTarget();
                if (_unit->GetDistance2dSq(target) > 225.0f && RandomUInt(4) == 1)
                {
                    _unit->CastSpell(target, spells[1].info, spells[1].instant);
                    return;
                }
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
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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

// ShatteredHandSharpshooterAI
class ShatteredHandSharpshooterAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandSharpshooterAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        ShatteredHandSharpshooterAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_SHARP_SCATTER_SHOT);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 20;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_SHARP_IMMO_ARROW);
            spells[1].targettype = TARGET_ATTACKING;    // no idea why fire stays under caster instead of target
            spells[1].instant = false;
            spells[1].cooldown = 5;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_SHARP_SHOT);    // disabled for now
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_SHARP_INCENDIARY_SHOT);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = false;
            spells[3].cooldown = 35;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            setAIAgent(AGENT_NULL);
            if (_unit->GetAIInterface()->getNextTarget() && _unit->GetDistance2dSq(_unit->GetAIInterface()->getNextTarget()) <= 900.0f)
            {
                setAIAgent(AGENT_SPELL);
                if (_unit->GetCurrentSpell() == NULL)
                {
                    uint32 Chance = RandomUInt(100);
                    if (Chance <= 70)
                    {
                        _unit->CastSpell(_unit->GetAIInterface()->getNextTarget(), spells[2].info, spells[2].instant);
                    }

                    else if (Chance > 70 && Chance <= 78)
                    {
                        _unit->CastSpell(_unit->GetAIInterface()->getNextTarget(), spells[1].info, spells[1].instant);
                    }

                    else if (Chance > 78 && Chance <= 82)
                    {
                        _unit->CastSpell(_unit->GetAIInterface()->getNextTarget(), spells[3].info, spells[3].instant);
                    }

                    else if (Chance > 82 && Chance <= 86)
                    {
                        _unit->CastSpell(_unit->GetAIInterface()->getNextTarget(), spells[0].info, spells[0].instant);
                    }
                }
            }
        }

    protected:

        uint8 nrspells;
};

// ShatteredHandBrawlerAI
// Self Visual - Sleep Until Cancelled (DND) 16093 ?
class ShatteredHandBrawlerAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandBrawlerAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        ShatteredHandBrawlerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_CURSE_OF_THE_SHATTERED_HAND);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 35;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_BRAWLER_KICK);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = 25;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_SHATT_HAND_BRAWLER_TRASH);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].cooldown = 20;
            spells[2].perctrigger = 0.0f;
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
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
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


//Grand Warlock Nethekurse Encounter

static Movement::Location Darkcasters[] =
{
    { 160.563004f, 272.989014f, -13.189000f },
    { 176.201004f, 264.669006f, -13.141600f },
    { 194.951004f, 265.657990f, -13.181700f }
};

// ShadowmoonDarkcasterAI
class ShadowmoonDarkcasterAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ShadowmoonDarkcasterAI);

        ShadowmoonDarkcasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            Unit* GrandWarlock = NULL;
            GrandWarlock = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(178.811996f, 292.377991f, -8.190210f, CN_GRAND_WARLOCK_NETHEKURSE);
            if (GrandWarlock)
            {
                GrandWarlock->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
                GrandWarlock->GetAIInterface()->SetAllowedToEnterCombat(false);
            }
        }

        void OnCombatStart(Creature* mTarget)
        {
            Creature* GrandWarlock = NULL;
            GrandWarlock = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(178.811996f, 292.377991f, -8.190210f, CN_GRAND_WARLOCK_NETHEKURSE);
            if (GrandWarlock)
            {
                switch (RandomUInt(3))        // must be verified + emotes?
                {
                    case 0:
                        GrandWarlock->SendScriptTextChatMessage(SAY_GRAND_WARLOCK_02);
                        break;
                    case 1:
                        GrandWarlock->SendScriptTextChatMessage(SAY_GRAND_WARLOCK_03);
                        break;
                    case 2:
                        GrandWarlock->SendScriptTextChatMessage(SAY_GRAND_WARLOCK_04);
                        break;
                    case 3:
                        GrandWarlock->SendScriptTextChatMessage(SAY_GRAND_WARLOCK_05);
                        break;
                }
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void OnDied(Creature* mKiller)
        {
            Creature* GrandWarlock = NULL;
            GrandWarlock = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(178.811996f, 292.377991f, -8.190210f, CN_GRAND_WARLOCK_NETHEKURSE);
            if (GrandWarlock)    // any emotes needed?
            {
                uint32 Counter = 0;
                for (uint8 i = 0; i < 3; i++)
                {
                    Creature* Servant = NULL;
                    Servant = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(Darkcasters[i].x, Darkcasters[i].y, Darkcasters[i].z, CN_SHADOWMOON_DARKCASTER);
                    if (!Servant)
                        continue;
                    if (!Servant->isAlive())
                        continue;
                    Counter++;
                }

                if (Counter == 0)
                {
                    GrandWarlock->GetAIInterface()->HandleEvent(EVENT_ENTERCOMBAT, GrandWarlock, 0);
                }

                switch (RandomUInt(2))    // those need to be verified too
                {
                    case 0:
                        GrandWarlock->SendScriptTextChatMessage(SAY_GRAND_WARLOCK_06);
                        break;
                    case 1:
                        GrandWarlock->SendScriptTextChatMessage(SAY_GRAND_WARLOCK_07);
                        break;
                    case 2:
                        GrandWarlock->SendScriptTextChatMessage(SAY_GRAND_WARLOCK_08);
                        break;
                }
            }
        }

};


// Grand Warlock NethekurseAI
/// \todo It has much more sounds (like for servant dies etc.). For future makes researches on them.
class GrandWarlockNethekurseAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(GrandWarlockNethekurseAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        GrandWarlockNethekurseAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_GRAND_WARLOCK_NETH_DEATH_COIL);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].mindist2cast = 0.0f;
            spells[0].maxdist2cast = 40.0f;
            // disabled for now
            spells[1].info = sSpellCustomizations.GetSpellInfo(SP_LESSER_SHADOW_FISSURE);
            spells[1].targettype = TARGET_RANDOM_SINGLE;
            spells[1].instant = true;    // doesn't work, because of lack of core support (so to prevent channeling I changed false to true)
            spells[1].cooldown = 25;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].mindist2cast = 0.0f;
            spells[1].maxdist2cast = 40.0f;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SP_GRAND_WARLOCK_NETH_DARK_SPIN);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = false;
            spells[2].cooldown = 60;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

            Started = false;
        }

        void OnCombatStart(Unit* mTarget)
        {
            for (uint8 i = 0; i < 3; i++)
                spells[i].casttime = 0;

            if (Started)
            {
                switch (RandomUInt(2))
                {
                    case 0:
                        sendDBChatMessage(SAY_GRAND_WARLOCK_13);
                        break;
                    case 1:
                        sendDBChatMessage(SAY_GRAND_WARLOCK_14);
                        break;
                    case 2:
                        sendDBChatMessage(SAY_GRAND_WARLOCK_15);
                        break;
                }

                RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            }

            else
                RegisterAIUpdateEvent(4000);
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        sendDBChatMessage(SAY_GRAND_WARLOCK_16);
                        break;
                    case 1:
                        sendDBChatMessage(SAY_GRAND_WARLOCK_17);
                        break;
                }
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);

            if (Started)
                RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            sendDBChatMessage(SAY_GRAND_WARLOCK_18);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            if (!Started)
            {
                _unit->GetAIInterface()->SetAllowedToEnterCombat(true);
                _unit->SetEmoteState(EMOTE_ONESHOT_NONE);
                _unit->setUInt64Value(UNIT_FIELD_FLAGS, 0);

                setAIAgent(AGENT_NULL);
                _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
                RemoveAIUpdateEvent();

                Started = true;

                Unit* target = NULL;
                target = FindTarget();
                if (target)
                {
                    _unit->GetAIInterface()->AttackReaction(target, 1, 0);
                }
            }

            if (_unit->getAuraWithId(SP_GRAND_WARLOCK_NETH_DARK_SPIN))
            {
                _unit->setAttackTimer(2500, false);
                return;
            }

            uint32 t = (uint32)time(NULL);
            // not sure if this should work like that
            if (t > spells[2].casttime && _unit->GetHealthPct() <= 20 && _unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                _unit->setAttackTimer(2500, false);

                _unit->CastSpell(_unit, spells[2].info, spells[2].instant);

                spells[2].casttime = t + 120;
                return;
            }

            else
            {
                float val = RandomFloat(100.0f);
                SpellCast(val);
            }
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
                std::vector<Unit*> TargetTable;        // From M4ksiu - Big THX to Capt
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

        // A bit rewritten FindTarget function
        Unit* FindTarget()
        {
            Unit* target = NULL;
            float distance = 50.0f;

            Unit* pUnit;
            float dist;

            for (std::set<Object*>::iterator itr = _unit->GetInRangeOppFactsSetBegin(); itr != _unit->GetInRangeOppFactsSetEnd(); ++itr)
            {
                if (!(*itr)->IsUnit())
                    continue;

                pUnit = static_cast<Unit*>((*itr));

                if (pUnit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH))
                    continue;

                if (!pUnit->isAlive() || _unit == pUnit)
                    continue;

                dist = _unit->GetDistance2dSq(pUnit);

                if (dist > distance * distance)
                    continue;

                target = pUnit;
                break;
            }

            return target;
        }

    protected:

        bool Started;
        uint8 nrspells;
};


// Blood Guard PorungAI
// Note: This boss appears only in Heroic mode and I don't have much infos about it =/
class BloodGuardPorungAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(BloodGuardPorungAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        BloodGuardPorungAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_BLOOD_GUARD_PORUNG_CLEAVE);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 10.0f;
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
                spells[i].casttime = 0;
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

        uint8 nrspells;
};

// WarbringerOmroggAI
// Maybe timer for 'afterspeech' should be added too?
void SpellFunc_Warbringer_BurningMaul(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType);

class WarbringerOmroggAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(WarbringerOmroggAI, MoonScriptCreatureAI);
        WarbringerOmroggAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(SP_WARBRINGER_OMROGG_THUNDERCLAP, Target_Self, 25, 1, 12);
            AddSpell(SP_WARBRINGER_OMROGG_FEAR, Target_Self, 7, 0, 20);
            AddSpellFunc(&SpellFunc_Warbringer_BurningMaul, Target_Self, 100, 0, 30);
            mBlastWave = AddSpell(SP_WARBRINGER_OMROGG_BLAST_WAVE, Target_Self, 0, 1, 0);
            mBlastWaveTimer = mSpeechTimer = mSpeechId = mAggroShiftTimer = INVALIDATE_TIMER;
            mRightHead = mLeftHead = NULL;
        }

        void OnCombatStart(Unit* pTarget)
        {
            ParentClass::OnCombatStart(pTarget);
            mAggroShiftTimer = _addTimer(20000 + RandomUInt(10) * 1000);
            mBlastWaveTimer = mSpeechTimer = mSpeechId = INVALIDATE_TIMER;
            mLeftHead = SpawnCreature(19523, _unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), _unit->GetOrientation(), false);
            mRightHead = SpawnCreature(19524, _unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), _unit->GetOrientation(), false);
            if (mLeftHead != NULL)
            {
                mLeftHead->GetUnit()->GetAIInterface()->SetUnitToFollow(_unit);
            }
            if (mRightHead != NULL)
            {
                mRightHead->GetUnit()->GetAIInterface()->SetUnitToFollow(_unit);
            }

            if (mLeftHead == NULL || mRightHead == NULL)
                return;

            switch (RandomUInt(2))
            {
                case 0:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 10308, "If you nice me let you live.");
                    mSpeechTimer = _addTimer(4000);
                    mSpeechId = 1;
                    break;
                case 1:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 10309, "Me hungry!");
                    mSpeechTimer = _addTimer(2500);
                    mSpeechId = 2;
                    break;
                case 2:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 10306, "Smash!");
                    mSpeechTimer = _addTimer(2000);
                    mSpeechId = 3;
                    break;
            }
        }

        void OnCombatStop(Unit* pTarget)
        {
            ParentClass::OnCombatStop(pTarget);
            if (isAlive())
            {
                if (mLeftHead != NULL)
                {
                    mLeftHead->despawn(1000);
                    mLeftHead = NULL;
                }
                if (mRightHead != NULL)
                {
                    mRightHead->despawn(1000);
                    mRightHead = NULL;
                }
            }
        }

        void OnTargetDied(Unit* pTarget)
        {
            if (mLeftHead == NULL || mRightHead == NULL || mSpeechTimer != INVALIDATE_TIMER)
                return;

            switch (RandomUInt(1))
            {
                case 0:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 10320, "I'm tired. You kill the next one!");
                    break;
                case 1:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 10310, "This one die easy!");
                    mSpeechTimer = _addTimer(3000);
                    mSpeechId = 0;
                    break;
            }
        }

        void OnDied(Unit* pKiller)
        {
            ParentClass::OnDied(pKiller);
            if (mLeftHead == NULL || mRightHead == NULL)
                return;

            sendChatMessage(CHAT_MSG_MONSTER_YELL, 10311, "This all... your fault!");
            mLeftHead->despawn(1000);
            mLeftHead = NULL;
            mRightHead->RegisterAIUpdateEvent(3000);
            mRightHead->despawn(4000);
            mRightHead = NULL;
        }

        void AIUpdate()
        {
            ParentClass::AIUpdate();

            if (mSpeechTimer != INVALIDATE_TIMER && _isTimerFinished(mSpeechTimer))
            {
                bool ResetSpeech = true;
                _removeTimer(mSpeechTimer);
                if (mLeftHead != NULL && mRightHead != NULL)
                {
                    switch (mSpeechId)
                    {
                        case 0:
                            mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10321, "That's because I do all the hard work!");
                            break;
                        case 1:
                            mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10318, "No, we will NOT let you live!");
                            break;
                        case 2:
                            mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10319, "You always hungry. That why we so fat!");
                            break;
                        case 3:
                            mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10317, "Why don't you let me do the talking!");
                            break;
                        case 4:
                            mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10313, "I'm not done yet, idiot!");
                            break;
                        case 5:
                            mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10316, "Bored, he's almost dead!");
                            break;
                        case 6:
                            mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10314, "That's not funny!");
                            mSpeechTimer = _addTimer(6000);
                            mSpeechId = 8;
                            ResetSpeech = false;
                            break;
                        case 7:
                            mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10315, "What are you doing!?");
                            break;
                        case 8:
                            mLeftHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10304, "Ha ha ha!");
                            break;
                    }
                }

                if (ResetSpeech)
                    mSpeechId = -1;
            }
            else if (_isTimerFinished(mAggroShiftTimer))
            {
                _resetTimer(mAggroShiftTimer, 20000 + RandomUInt(10) * 1000);
                ShiftAggro();
            }

            if (mBlastWaveTimer != INVALIDATE_TIMER && _isTimerFinished(mBlastWaveTimer))
            {
                _removeTimer(mBlastWaveTimer);
                CastSpell(mBlastWave);
            }
        }

        void ShiftAggro()
        {
            Unit* pTarget = GetBestPlayerTarget(TargetFilter_NotCurrent);
            if (pTarget != NULL)
            {
                _clearHateList();
                _unit->GetAIInterface()->setNextTarget(pTarget);
                _unit->GetAIInterface()->modThreatByPtr(pTarget, 1000);

                if (mLeftHead == NULL || mRightHead == NULL || mSpeechTimer != INVALIDATE_TIMER)
                    return;

                switch (RandomUInt(6))
                {
                    case 0:
                        mLeftHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10301, "We kill his friend!");
                        break;
                    case 1:
                        mLeftHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10300, "Me not like this one...");
                        mSpeechTimer = _addTimer(3000);
                        mSpeechId = 4;
                        break;
                    case 2:
                        mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10312, "Hey, you numbskull!");
                        break;
                    case 3:
                        mLeftHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10305, "Me get bored.");
                        mSpeechTimer = _addTimer(3000);
                        mSpeechId = 5;
                        break;
                    case 4:
                        mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10315, "What are you doing!?");
                        break;
                    case 5:
                        mLeftHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10303, "You stay here. Me go kill someone else!");
                        mSpeechTimer = _addTimer(4000);
                        mSpeechId = 6;
                        break;
                    case 6:
                        mLeftHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10302, "Me kill someone else!");
                        mSpeechTimer = _addTimer(3000);
                        mSpeechId = 7;
                        break;
                }
            }
        }

        MoonScriptCreatureAI* mLeftHead;
        MoonScriptCreatureAI* mRightHead;
        int32 mAggroShiftTimer;
        uint32 mBlastWaveTimer;
        uint32 mSpeechTimer;
        int32 mSpeechId;
        SpellDesc* mBlastWave;
};

void SpellFunc_Warbringer_BurningMaul(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType)
{
    WarbringerOmroggAI* Warbringer = (pCreatureAI) ? static_cast< WarbringerOmroggAI* >(pCreatureAI) : NULL;
    if (Warbringer != NULL)
    {
        Warbringer->CastSpell(Warbringer->mBlastWave);
        Warbringer->mBlastWaveTimer = Warbringer->_addTimer(RandomUInt(5) + 5);
    }
}

class HeadAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(HeadAI, MoonScriptCreatureAI);
    HeadAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        _setScale(4.0f);
        _unit->setUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        _unit->m_noRespawn = true;
    }

    void AIUpdate()
    {
        if (_unit->GetEntry() != CN_RIGHT_HEAD)
            return;

        sendChatMessage(CHAT_MSG_MONSTER_YELL, 10322, "I... hate... you!");
        RemoveAIUpdateEvent();                                // Dangerous!
    }

    void Destroy()
    {
        Creature* pUnit = getNearestCreature(CN_WARBRINGER_OMROGG);
        if (pUnit != NULL && pUnit->GetScript() != NULL)
        {
            WarbringerOmroggAI* pAI = static_cast< WarbringerOmroggAI* >(pUnit->GetScript());
            if (pAI->mLeftHead == (MoonScriptCreatureAI*)(this))
                pAI->mLeftHead = NULL;
            if (pAI->mRightHead == (MoonScriptCreatureAI*)(this))
                pAI->mRightHead = NULL;
        }
    }
};

// Warchief Kargath BladefistAI

// Should call for support?
// does he use only one ability?
class WarchiefKargathBladefistAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(WarchiefKargathBladefistAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        WarchiefKargathBladefistAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SP_WARCHIEF_LARAGATH_BLADE_DANCE);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = 30;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            switch (RandomUInt(2))
            {
                case 0:
                    sendDBChatMessage(SAY_WARCHIEF_KARGATH_01);
                    break;
                case 1:
                    sendDBChatMessage(SAY_WARCHIEF_KARGATH_02);
                    break;
                case 2:
                    sendDBChatMessage(SAY_WARCHIEF_KARGATH_03);
                    break;
            }

            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));

            spells[0].casttime = (uint32)time(NULL) + 30;
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        sendDBChatMessage(SAY_WARCHIEF_KARGATH_04);
                        break;
                    case 1:
                        sendDBChatMessage(SAY_WARCHIEF_KARGATH_05);
                        break;
                }
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            sendDBChatMessage(SAY_WARCHIEF_KARGATH_06);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            uint32 t = (uint32)time(NULL);
            if (t > spells[0].casttime && _unit->GetAIInterface()->getNextTarget() && _unit->GetCurrentSpell() == NULL)
            {
                _unit->setAttackTimer(1500, false);

                _unit->CastSpell(_unit->GetAIInterface()->getNextTarget(), spells[0].info, spells[0].instant);

                spells[0].casttime = t + spells[0].cooldown;
            }
        }

    protected:

        uint8 nrspells;
};


/// \todo Shattered Hand Executioner 17301, Shattered Hand Champion 17671,
// Shattered Hand Centurion 17465, Shattered Hand Blood Guard 17461,
// hattered Hand Archer 17427, Sharpshooter Guard 17622, Shattered Hand Zealot 17462
// (lack of infos or don't have any spells!) more?
void SetupTheShatteredHalls(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_HC_SHATTERED_HALLS, &InstanceTheShatteredHallsScript::Create);

    //Creatures
    mgr->register_creature_script(CN_FEL_ORC_CONVERT, &FelOrcConvertAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_HEATHEN, &ShatteredHandHeathenAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_LEGIONNAIRE, &ShatteredHandLegionnaireAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_SAVAGE, &ShatteredHandSavageAI::Create);
    mgr->register_creature_script(CN_SHADOWMOON_ACOLYTE, &ShadowmoonAcolyteAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_ASSASSIN, &ShatteredHandAssassinAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_GLADIATOR, &ShatteredHandGladiatorAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_HOUNDMASTER, &ShatteredHandHoundmasterAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_REAVER, &ShatteredHandReaverAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_SENTRY, &ShatteredHandSentryAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_SHARPSHOOTER, &ShatteredHandSharpshooterAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_BRAWLER, &ShatteredHandBrawlerAI::Create);
    mgr->register_creature_script(CN_SHADOWMOON_DARKCASTER, &ShadowmoonDarkcasterAI::Create);
    mgr->register_creature_script(CN_GRAND_WARLOCK_NETHEKURSE, &GrandWarlockNethekurseAI::Create);
    mgr->register_creature_script(CN_BLOOD_GUARD_PORUNG, &BloodGuardPorungAI::Create);
    mgr->register_creature_script(CN_WARBRINGER_OMROGG, &WarbringerOmroggAI::Create);
    mgr->register_creature_script(CN_LEFT_HEAD, &HeadAI::Create);
    mgr->register_creature_script(CN_RIGHT_HEAD, &HeadAI::Create);
    mgr->register_creature_script(CN_WARCHIEF_KARGATH_BLADEFIST, &WarchiefKargathBladefistAI::Create);
}
