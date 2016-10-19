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
#include "Instance_RazorfenDowns.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Azjol-Nerub
class InstanceRazorfenDownsScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceRazorfenDownsScript, MoonInstanceScript);
        InstanceRazorfenDownsScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
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

class AmnennarTheColdbringerAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(AmnennarTheColdbringerAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        AmnennarTheColdbringerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(10179);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 3000;
            m_spellcheck[0] = true;


            spells[1].info = sSpellCustomizations.GetSpellInfo(22645);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 3000;
            m_spellcheck[1] = true;


            spells[2].info = sSpellCustomizations.GetSpellInfo(13009);
            spells[2].cooldown = 10;
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].perctrigger = RandomFloat(20.0f);
            spells[2].attackstoptimer = 3000;
            m_spellcheck[2] = true;

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
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
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


class GluttonAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(GluttonAI, MoonScriptCreatureAI);
    GluttonAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        //spells
        mDiseaseCloud = AddSpell(SP_GLUTTON_DISEASE_CLOUD, Target_Self, 0, 0, 0, 0, 0);
        mFrenzy = AddSpell(SP_GLUTTON_FRENZY, Target_Self, 10, 0, 20, 0, 0);
        mFrenzy->AddEmote("Glutton is getting hungry!", Text_Yell);
    }

    void OnCombatStart(Unit* pTarget)
    {
        CastSpellNowNoScheduling(mDiseaseCloud);

        ParentClass::OnCombatStart(pTarget);
    }

    SpellDesc* mDiseaseCloud;
    SpellDesc* mFrenzy;
};


class MordreshFireEyeAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(MordreshFireEyeAI, MoonScriptCreatureAI);
    MordreshFireEyeAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        //spells
        AddSpell(SP_MORDRESH_FIRE_NOVA, Target_Self, 10, 2, 0);
        AddSpell(SP_MORDRESH_FIREBALL, Target_Current, 10, 3, 0, 0, 40);
    }
};

class PlaguemawTheRottingAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(PlaguemawTheRottingAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        PlaguemawTheRottingAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(12947);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 3000;
            m_spellcheck[0] = true;


            spells[1].info = sSpellCustomizations.GetSpellInfo(12946);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 3000;
            m_spellcheck[1] = true;

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
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
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

class RagglesnoutAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(RagglesnoutAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        RagglesnoutAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(10892);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 3000;
            m_spellcheck[0] = true;


            spells[1].info = sSpellCustomizations.GetSpellInfo(11659);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 3000;
            m_spellcheck[1] = true;

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
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
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


class TutenKashAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(TutenKashAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        TutenKashAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(12255);
            spells[0].cooldown = 10;
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = RandomFloat(20.0f);
            spells[0].attackstoptimer = 3000;
            m_spellcheck[0] = true;


            spells[1].info = sSpellCustomizations.GetSpellInfo(12252);
            spells[1].cooldown = 10;
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = RandomFloat(20.0f);
            spells[1].attackstoptimer = 3000;
            m_spellcheck[1] = true;

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
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
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

void SetupRazorfenDowns(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_RAZORFEN_DOWNS, &InstanceRazorfenDownsScript::Create);

    mgr->register_creature_script(CN_AMNENNAR_GOLDBRINGER, &AmnennarTheColdbringerAI::Create);
    mgr->register_creature_script(CN_GLUTTON, &GluttonAI::Create);
    mgr->register_creature_script(CN_MORDRESH_FIRE_EYE, &MordreshFireEyeAI::Create);
    mgr->register_creature_script(CN_PLAGUEMAW_THE_ROTTING, &PlaguemawTheRottingAI::Create);
    mgr->register_creature_script(CN_RAGGLESNOUT, &RagglesnoutAI::Create);
    mgr->register_creature_script(CN_TUTEN_KASH, &TutenKashAI::Create);
}
