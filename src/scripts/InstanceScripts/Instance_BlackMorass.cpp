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

//////////////////////////////////////////////////////////////////////////////////////////
//Caverns of Time: Black Morass
class InstanceBlackMorassScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceBlackMorassScript, MoonInstanceScript);
        InstanceBlackMorassScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
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

// ChronoLordAI
class ChronoLordAI : public CreatureAIScript
{
    public:

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
            spells[0].info = dbcSpell.LookupEntry(ARCANE_BLAST);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 10;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(TIME_LAPSE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = 8;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            /*spells[2].info = dbcSpell.LookupEntry(MAGNETIC_PULL);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = true;
            spells[2].cooldown = 15;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;*/
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            _unit->SendScriptTextChatMessage(SAY_CHRONOLORD_01);
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_CHRONOLORD_02);
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(SAY_CHRONOLORD_03);
                        break;
                }
            }
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
            _unit->SendScriptTextChatMessage(SAY_CHRONOLORD_04);
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


// TemporusAI
class TemporusAI : public CreatureAIScript
{
    public:

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
            spells[0].info = dbcSpell.LookupEntry(HASTEN);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = true;
            spells[0].cooldown = 10;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(MORTAL_WOUND);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = 5;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            /*spells[2].info = dbcSpell.LookupEntry(SPELL_REFLECTION);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = true;
            spells[2].cooldown = 15;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;*/
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            _unit->SendScriptTextChatMessage(SAY_TEMPORUS_01);
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_TEMPORUS_02);
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(SAY_TEMPORUS_03);
                        break;
                }
            }
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
            _unit->SendScriptTextChatMessage(SAY_TEMPORUS_04);
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


//AenusAI
class AenusAI : public CreatureAIScript
{
    public:

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
            spells[0].info = dbcSpell.LookupEntry(SAND_BREATH);
            spells[0].targettype = TARGET_DESTINATION;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(TIME_STOP);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 15;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(FRENZY);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].cooldown = 8;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            _unit->SendScriptTextChatMessage(SAY_AENUS_01);
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_AENUS_02);
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(SAY_AENUS_03);
                        break;
                }
            }
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
            _unit->SendScriptTextChatMessage(SAY_AENUS_04);
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

void SetupTheBlackMorass(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_COT_BLACK_MORASS, &InstanceBlackMorassScript::Create);

    mgr->register_creature_script(CN_CHRONO_LORD_DEJA, &ChronoLordAI::Create);
    mgr->register_creature_script(CN_TEMPORUS, &TemporusAI::Create);
    mgr->register_creature_script(CN_AEONUS, &AenusAI::Create);
}
