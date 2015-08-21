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
#include "Instance_TheStockade.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Stormwind Stockade
class InstanceStormwindStockadeScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceStormwindStockadeScript, MoonInstanceScript);
        InstanceStormwindStockadeScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
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

// DeepfuryAI
class DeepfuryAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(DeepfuryAI);
        SP_AI_Spell spell;
        bool m_spellcheck;

        DeepfuryAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            m_spellcheck = false;
            spell.info = dbcSpell.LookupEntry(KAM_SHIELDSLAM);
            spell.targettype = TARGET_ATTACKING;
            spell.instant = true;
            spell.perctrigger = 0.0f;
            spell.cooldown = 10;
            spell.attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(1000);
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

        void CastTime()
        {
            spell.casttime = spell.cooldown;
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                spell.casttime--;
                if (m_spellcheck)
                {
                    if (!spell.instant)
                        _unit->GetAIInterface()->StopMovement(1);

                    spell.casttime = spell.cooldown;
                    target = _unit->GetAIInterface()->getNextTarget();
                    _unit->CastSpell(target, spell.info, spell.instant);

                    if (spell.speech != "")
                    {
                        _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spell.speech.c_str());
                        _unit->PlaySoundToSet(spell.soundid);
                    }

                    m_spellcheck = false;
                    return;
                }

                if ((val > comulativeperc && val <= (comulativeperc + spell.perctrigger)) || !spell.casttime)
                {
                    _unit->setAttackTimer(spell.attackstoptimer, false);
                    m_spellcheck = true;
                }
                comulativeperc += spell.perctrigger;
            }
        }

};

// HamhockAI
class HamhockAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(HamhockAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        HamhockAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(CHAINLIGHT);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].perctrigger = 0.0f;
            spells[0].cooldown = 10;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(DEMORALIZO);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 0.0f;
            spells[1].cooldown = 20;
            spells[1].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(1000);
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

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
                        if (!spells[i].instant)
                            _unit->GetAIInterface()->StopMovement(1);

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

// BazilAI
class BazilAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(BazilAI);
        SP_AI_Spell spell;
        bool m_spellcheck;

        BazilAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            m_spellcheck = false;
            spell.info = dbcSpell.LookupEntry(BAZILBOMB);
            spell.targettype = TARGET_RANDOM_SINGLE;
            spell.instant = true;
            spell.perctrigger = 0.0f;
            spell.cooldown = 12;
            spell.attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(1000);
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

        void CastTime()
        {
            spell.casttime = spell.cooldown;
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                spell.casttime--;

                if (m_spellcheck)
                {
                    if (!spell.instant)
                        _unit->GetAIInterface()->StopMovement(1);

                    spell.casttime = spell.cooldown;
                    target = _unit->GetAIInterface()->getNextTarget();
                    std::vector<Unit* > target_list;
                    for (set< Object* >::iterator itr = _unit->GetInRangePlayerSetBegin(); itr != _unit->GetInRangePlayerSetEnd(); ++itr)
                    {
                        target = TO< Unit* >(*itr);
                        if (target)
                            target_list.push_back(target);

                        target = NULL;
                    }

                    if (!target_list.size())
                        return;

                    auto random_index = RandomUInt(0, target_list.size() - 1);
                    auto random_target = target_list[random_index];

                    if (random_target == nullptr)
                        return;

                    _unit->CastSpell(random_target, spell.info, spell.instant);

                    random_target = nullptr;

                    if (spell.speech != "")
                    {
                        _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spell.speech.c_str());
                        _unit->PlaySoundToSet(spell.soundid);
                    }

                    m_spellcheck = false;
                    return;
                }

                if ((val > comulativeperc && val <= (comulativeperc + spell.perctrigger)) || !spell.casttime)
                {
                    _unit->setAttackTimer(spell.attackstoptimer, false);
                    m_spellcheck = true;
                }
                comulativeperc += spell.perctrigger;
            }
        }

};

// DextrenAI
class DextrenAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(DextrenAI);
        SP_AI_Spell spell;
        bool m_spellcheck;

        DextrenAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            m_spellcheck = false;
            spell.info = dbcSpell.LookupEntry(INTIMIDATING);
            spell.targettype = TARGET_ATTACKING;
            spell.instant = true;
            spell.perctrigger = 0.0f;
            spell.cooldown = 15;
            spell.attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(1000);
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

        void CastTime()
        {
            spell.casttime = spell.cooldown;
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                spell.casttime--;
                if (m_spellcheck)
                {
                    if (!spell.instant)
                        _unit->GetAIInterface()->StopMovement(1);

                    spell.casttime = spell.cooldown;
                    target = _unit->GetAIInterface()->getNextTarget();
                    _unit->CastSpell(target, spell.info, spell.instant);

                    if (spell.speech != "")
                    {
                        _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spell.speech.c_str());
                        _unit->PlaySoundToSet(spell.soundid);
                    }

                    m_spellcheck = false;
                    return;
                }

                if ((val > comulativeperc && val <= (comulativeperc + spell.perctrigger)) || !spell.casttime)
                {
                    _unit->setAttackTimer(spell.attackstoptimer, false);
                    m_spellcheck = true;
                }
                comulativeperc += spell.perctrigger;
            }
        }

};

// InmateAI
class InmateAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(InmateAI);
        SP_AI_Spell spell;
        bool m_spellcheck;

        InmateAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            m_spellcheck = false;
            spell.info = dbcSpell.LookupEntry(CONVICTREND);
            spell.targettype = TARGET_ATTACKING;
            spell.instant = true;
            spell.perctrigger = 0.0f;
            spell.cooldown = 12;
            spell.attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(1000);
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

        void CastTime()
        {
            spell.casttime = spell.cooldown;
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                spell.casttime--;
                if (m_spellcheck)
                {
                    if (!spell.instant)
                        _unit->GetAIInterface()->StopMovement(1);

                    spell.casttime = spell.cooldown;
                    target = _unit->GetAIInterface()->getNextTarget();
                    _unit->CastSpell(target, spell.info, spell.instant);

                    if (spell.speech != "")
                    {
                        _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spell.speech.c_str());
                        _unit->PlaySoundToSet(spell.soundid);
                    }

                    m_spellcheck = false;
                    return;
                }

                if ((val > comulativeperc && val <= (comulativeperc + spell.perctrigger)) || !spell.casttime)
                {
                    _unit->setAttackTimer(spell.attackstoptimer, false);
                    m_spellcheck = true;
                }
                comulativeperc += spell.perctrigger;
            }
        }

};

// InsurgentAI
class InsurgentAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(InsurgentAI);
        SP_AI_Spell spell;
        bool m_spellcheck;

        InsurgentAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            m_spellcheck = false;
            spell.info = dbcSpell.LookupEntry(INSURGENTDEMORALIZO);
            spell.targettype = TARGET_SELF;
            spell.instant = true;
            spell.perctrigger = 0.0f;
            spell.cooldown = 15;
            spell.attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(1000);
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

        void CastTime()
        {
            spell.casttime = spell.cooldown;
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                spell.casttime--;
                if (m_spellcheck)
                {
                    if (!spell.instant)
                        _unit->GetAIInterface()->StopMovement(1);

                    spell.casttime = spell.cooldown;
                    target = _unit->GetAIInterface()->getNextTarget();
                    _unit->CastSpell(_unit, spell.info, spell.instant);

                    if (spell.speech != "")
                    {
                        _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spell.speech.c_str());
                        _unit->PlaySoundToSet(spell.soundid);
                    }

                    m_spellcheck = false;
                    return;
                }

                if ((val > comulativeperc && val <= (comulativeperc + spell.perctrigger)) || !spell.casttime)
                {
                    _unit->setAttackTimer(spell.attackstoptimer, false);
                    m_spellcheck = true;
                }
                comulativeperc += spell.perctrigger;
            }
        }

};

// PrisonerAI
class PrisonerAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(PrisonerAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        PrisonerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(PRISONKICK);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 0.0f;
            spells[0].cooldown = 11;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(PRISONDISARM);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 0.0f;
            spells[1].cooldown = 15;
            spells[1].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(1000);
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

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
                        if (!spells[i].instant)
                            _unit->GetAIInterface()->StopMovement(1);

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

// ConvictAI
class ConvictAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(ConvictAI);
        SP_AI_Spell spell;
        bool m_spellcheck;

        ConvictAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            m_spellcheck = false;
            spell.info = dbcSpell.LookupEntry(CONVICTBACKHAND);
            spell.targettype = TARGET_ATTACKING;
            spell.instant = true;
            spell.perctrigger = 0.0f;
            spell.cooldown = 10;
            spell.attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(1000);
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

        void CastTime()
        {
            spell.casttime = spell.cooldown;
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                spell.casttime--;
                if (m_spellcheck)
                {
                    if (!spell.instant)
                        _unit->GetAIInterface()->StopMovement(1);

                    spell.casttime = spell.cooldown;
                    target = _unit->GetAIInterface()->getNextTarget();
                    _unit->CastSpell(target, spell.info, spell.instant);

                    if (spell.speech != "")
                    {
                        _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spell.speech.c_str());
                        _unit->PlaySoundToSet(spell.soundid);
                    }

                    m_spellcheck = false;
                    return;
                }

                if ((val > comulativeperc && val <= (comulativeperc + spell.perctrigger)) || !spell.casttime)
                {
                    _unit->setAttackTimer(spell.attackstoptimer, false);
                    m_spellcheck = true;
                }
                comulativeperc += spell.perctrigger;
            }
        }

};

void SetupTheStockade(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_THE_STOCKADE, &InstanceStormwindStockadeScript::Create);

    //Creatures
    mgr->register_creature_script(CN_KAMDEEPFURY, &DeepfuryAI::Create);
    mgr->register_creature_script(CN_HAMHOCK, &HamhockAI::Create);
    mgr->register_creature_script(CN_BAZILTHREDD, &BazilAI::Create);
    mgr->register_creature_script(CN_DEXTRENWARD, &DextrenAI::Create);
    mgr->register_creature_script(CN_DEFINMATE, &InmateAI::Create);
    mgr->register_creature_script(CN_DEFINSURGENT, &InsurgentAI::Create);
    mgr->register_creature_script(CN_DEFPRISONER, &PrisonerAI::Create);
    mgr->register_creature_script(CN_DEFCONVICT, &ConvictAI::Create);
}
