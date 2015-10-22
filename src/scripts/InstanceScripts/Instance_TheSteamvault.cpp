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
#include "Instance_TheSteamvault.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Coilfang: The Steamvault
class InstanceTheSteamvaultScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceTheSteamvaultScript, MoonInstanceScript);
        InstanceTheSteamvaultScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
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

// CoilfangEngineerAI
class CoilfangEngineerAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangEngineerAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        CoilfangEngineerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(BOMB);
            spells[0].targettype = TARGET_DESTINATION;
            spells[0].instant = false;
            spells[0].cooldown = 10;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(NET);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = 25;
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


// CoilfangOracleAI
class CoilfangOracleAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangOracleAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        CoilfangOracleAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(FROST_SHOCK);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SONIC_BURST);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 25;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(HEAL);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = false;
            spells[2].cooldown = 45;
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

// CoilfangWarriorAI
class CoilfangWarriorAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangWarriorAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        CoilfangWarriorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            /*
                    spells[0].info = dbcSpell.LookupEntry(MORTAL_STRIKE);
                    spells[0].targettype = TARGET_ATTACKING;
                    spells[0].instant = false;
                    spells[0].cooldown = ;
                    spells[0].perctrigger = 0.0f;
                    spells[0].attackstoptimer = 1000;
            */
            spells[0].info = dbcSpell.LookupEntry(MORTAL_BLOW);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(BATTLE_SHOUT);    // should affect friends not enemies :S
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 35;
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


// CoilfangSirenAI
class CoilfangSirenAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangSirenAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        CoilfangSirenAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(AOE_FEAR);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = 35;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(LIGHTNING_BOLT);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = 25;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(ARCANE_FLARE);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = 15;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;
            /*
                    spells[2].info = dbcSpell.LookupEntry(MOONFIRE);
                    spells[2].targettype = TARGET_ATTACKING;
                    spells[2].instant = true;
                    spells[2].cooldown = ;
                    spells[2].perctrigger = 0.0f;
                    spells[2].attackstoptimer = 1000;
            */
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



// BogOverlordAI
// Should be immunite on CC's and slowing down effects.
// Also should detect players in stealth mode. (Invisibility and Stealth Detection 18950 ?)

class BogOverlordAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(BogOverlordAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        BogOverlordAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {

            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(FUNGAL_DECAY);
            spells[0].targettype = TARGET_ATTACKING;    // should be random (as in many other spells)
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(TRAMPLE);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 10;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(ENRAGE_BOG_OVERLORD);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = false;
            spells[2].cooldown = 55;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(DISEASE_CLOUD);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = -1;    // not sure to this and...
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            _unit->CastSpell(_unit, spells[3].info, spells[3].instant); // ...and this
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

// CoilfangSorceressAI
class CoilfangSorceressAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangSorceressAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        CoilfangSorceressAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(FROSTBOLT);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].cooldown = 10;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(BLIZZARD);
            spells[1].targettype = TARGET_DESTINATION;
            spells[1].instant = false;
            spells[1].cooldown = 35;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(FROST_NOVA);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = true;
            spells[2].cooldown = 25;
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

// CoilfangLeperAI
class CoilfangLeperAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangLeperAI);
        SP_AI_Spell spells[8];
        bool m_spellcheck[8];

        CoilfangLeperAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 8;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SHADOW_BOLT);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(FIRE_BLAST_LEPER);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = 25;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(STRIKE);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = 35;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(FROST_NOVA_LEPER);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = true;
            spells[3].cooldown = 55;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = dbcSpell.LookupEntry(CLEAVE_LEPER);
            spells[4].targettype = TARGET_VARIOUS;    // ?
            spells[4].instant = true;
            spells[4].cooldown = 45;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;

            spells[5].info = dbcSpell.LookupEntry(HEAL_LEPER);
            spells[5].targettype = TARGET_SELF;
            spells[5].instant = false;
            spells[5].cooldown = 85;
            spells[5].perctrigger = 0.0f;
            spells[5].attackstoptimer = 1000;

            spells[6].info = dbcSpell.LookupEntry(SUNDER_ARMOR_LEPER);
            spells[6].targettype = TARGET_ATTACKING;
            spells[6].instant = true;
            spells[6].cooldown = 95;
            spells[6].perctrigger = 0.0f;
            spells[6].attackstoptimer = 1000;

            spells[7].info = dbcSpell.LookupEntry(SHOOT);
            spells[7].targettype = TARGET_ATTACKING;
            spells[7].instant = true;
            spells[7].cooldown = -1;    // disabled for now
            spells[7].perctrigger = 0.0f;
            spells[7].attackstoptimer = 1000;

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

// CoilfangSlavemasterAI
// Slaves should run after killing slavemaster
class CoilfangSlavemasterAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangSlavemasterAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        CoilfangSlavemasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(GEYSER);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = 20;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(ENRAGE_SlAVEMASTER);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 55;
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

// CoilfangWaterElementalAI
class CoilfangWaterElementalAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangWaterElementalAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        CoilfangWaterElementalAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            /*
                    spells[0].info = dbcSpell.LookupEntry(FROSTBOLT_VOLLEY);
                    spells[0].targettype = TARGET_ATTACKING;
                    spells[0].instant = true;        // Should be false, but doesn't work then
                    spells[0].cooldown = ;
                    spells[0].perctrigger = 0.0f;
                    spells[0].attackstoptimer = 1000;
            */
            spells[0].info = dbcSpell.LookupEntry(WATER_BOLT_WOLLEY);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].cooldown = 10;
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

// CoilfangMyrmidonAI
class CoilfangMyrmidonAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangMyrmidonAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        CoilfangMyrmidonAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SWEEPING_STRIKES);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = true;
            spells[0].cooldown = 25;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(CLEAVE);
            spells[1].targettype = TARGET_VARIOUS;    // not sure
            spells[1].instant = true;
            spells[1].cooldown = 15;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(EXECUTE);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = 35;
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

// TidalSurgerAI
// Should has immunite to frost spells (as adds) + should has
// 3-4 minons (Idk if they should be spawned by script)
class TidalSurgerAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(TidalSurgerAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        TidalSurgerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            /*
                    spells[0].info = dbcSpell.LookupEntry(KNOCKBACK);
                    spells[0].targettype = TARGET_ATTACKING; // should be random
                    spells[0].instant = true;
                    spells[0].cooldown = ;
                    spells[0].perctrigger = 0.0f;
                    spells[0].attackstoptimer = 1000;
            */
            spells[0].info = dbcSpell.LookupEntry(WATER_SPOUT);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 25;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(FROST_NOVA_SURGER);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 15;
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

// SteamSurgerAI
class SteamSurgerAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(SteamSurgerAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        SteamSurgerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(WATER_BOLT);
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

/////////////////////////////////////////////////////////////
// Boss AIs
/////////////////////////////////////////////////////////////

// Hydromancer ThespiaAI
class HydromancerThespiaAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(HydromancerThespiaAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        HydromancerThespiaAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;

            }

            spells[0].info = dbcSpell.LookupEntry(ENVELOPING_WINDS);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = true;
            spells[0].cooldown = 10;
            spells[0].perctrigger = 9.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].mindist2cast = 0.0f;
            spells[0].maxdist2cast = 35.0f;

            spells[1].info = dbcSpell.LookupEntry(LIGHTNING_CLOUD);
            spells[1].targettype = TARGET_RANDOM_DESTINATION;
            spells[1].instant = true;
            spells[1].cooldown = 25;
            spells[1].perctrigger = 8.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].mindist2cast = 0.0f;
            spells[1].maxdist2cast = 30.0f;

            spells[2].info = dbcSpell.LookupEntry(LUNG_BURST);
            spells[2].targettype = TARGET_RANDOM_SINGLE;
            spells[2].instant = true;
            spells[2].cooldown = 15;
            spells[2].perctrigger = 9.0f;
            spells[2].attackstoptimer = 1000;
            spells[2].mindist2cast = 0.0f;
            spells[2].maxdist2cast = 40.0f;
        }

        void OnCombatStart(Unit* mTarget)
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            switch (RandomUInt(2))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_HYDROMACER_THESPIA_02);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_HYDROMACER_THESPIA_03);
                    break;
                case 2:
                    _unit->SendScriptTextChatMessage(SAY_HYDROMACER_THESPIA_04);
                    break;
            }

            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_HYDROMACER_THESPIA_05);
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(SAY_HYDROMACER_THESPIA_06);
                        break;
                }
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(SAY_HYDROMACER_THESPIA_07);
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

                auto random_index = RandomUInt(0, TargetTable.size() - 1);
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
                        _unit->CastSpellAoF(random_target->GetPositionX(), random_target->GetPositionY(), random_target->GetPositionZ(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:

        uint8 nrspells;
};

//---- Steamrigger encounter ----//

static Location SpawnCoords[] =
{
    { -300.037842f, -115.296227f, -7.865229f, 4.197916f },
    { -330.083008f, -121.505997f, -7.985120f, 5.061450f },
    { -346.530273f, -147.167892f, -6.703687f, 0.010135f }
};

// Steamrigger MechanicAI
// Should they really fight?
class SteamriggerMechanicAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(SteamriggerMechanicAI);

        SteamriggerMechanicAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
            _unit->m_noRespawn = true;
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->GetAIInterface()->ResetUnitToFollow();
            _unit->GetAIInterface()->SetUnitToFollowAngle(0.0f);

            if (_unit->GetCurrentSpell() != NULL)
                _unit->GetCurrentSpell()->cancel();
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
        }

        void OnTargetDied(Unit* mTarget)
        {
            _unit->GetAIInterface()->RemoveThreatByPtr(mTarget);
        }

        void OnDied(Unit* mKiller)
        {
            _unit->GetAIInterface()->ResetUnitToFollow();
            _unit->GetAIInterface()->SetUnitToFollowAngle(0.0f);

            RemoveAIUpdateEvent();
        }

        void OnDamageTaken(Unit* mAttacker, uint32 fAmount)
        {
            _unit->GetAIInterface()->SetAllowedToEnterCombat(true);
        }

};


// Mekgineer Steamrigger
// Must spawn 3 Steamrigger Mechanics when his health is on 75%, 50% and 25%
class MekgineerSteamriggerAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(MekgineerSteamriggerAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];
        std::vector <Unit*> Gnomes;

        MekgineerSteamriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SUPER_SHRINK_RAY);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = true;
            spells[0].cooldown = 20;
            spells[0].perctrigger = 9.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].mindist2cast = 0.0f;
            spells[0].maxdist2cast = 40.0f;

            spells[1].info = dbcSpell.LookupEntry(SAW_BLADE);
            spells[1].targettype = TARGET_RANDOM_SINGLE; // when killed with VARIOUS (because with that caster attacks also himself) server crashes
            spells[1].instant = true;
            spells[1].cooldown = 15;
            spells[1].perctrigger = 10.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].mindist2cast = 0.0f;
            spells[1].maxdist2cast = 40.0f;

            spells[2].info = dbcSpell.LookupEntry(ELECTRIFIED_NET);
            spells[2].targettype = TARGET_RANDOM_SINGLE;
            spells[2].instant = true;
            spells[2].cooldown = 15;
            spells[2].perctrigger = 8.0f;
            spells[2].attackstoptimer = 1000;
            spells[2].mindist2cast = 0.0f;
            spells[2].maxdist2cast = 40.0f;

            spells[3].info = dbcSpell.LookupEntry(ENRAGE);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = 300;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            GnomeCounter = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;
            spells[3].casttime = (uint32)time(NULL) + spells[3].cooldown;

            GnomeCounter = 0;

            Gnomes.clear();

            switch (RandomUInt(3))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_MEKGINEER_STEAMRIGGER_02);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_MEKGINEER_STEAMRIGGER_03);
                    break;
                case 2:
                    _unit->SendScriptTextChatMessage(SAY_MEKGINEER_STEAMRIGGER_04);
                    break;
                case 3:
                    _unit->SendScriptTextChatMessage(SAY_MEKGINEER_STEAMRIGGER_05);
                    break;
            }

            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_MEKGINEER_STEAMRIGGER_06);
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(SAY_MEKGINEER_STEAMRIGGER_07);
                        break;
                }
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            for (size_t i = 0; i < Gnomes.size(); i++)
            {
                Creature* Gnome = NULL;
                Gnome = static_cast<Creature*>(Gnomes[i]);
                if (!Gnome)
                    continue;

                if (!Gnome->IsInWorld() || !Gnome->isAlive())
                    continue;

                Gnome->Despawn(0, 0);
            }

            Gnomes.clear();

            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(SAY_MEKGINEER_STEAMRIGGER_09);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            if (Gnomes.size())
            {
                Unit* Gnome = NULL;
                for (std::vector<Unit*>::iterator itr = Gnomes.begin(); itr < Gnomes.end(); ++itr)
                {
                    Gnome = *itr;
                    if (!Gnome->isAlive() || !Gnome->IsInWorld())
                    {
                        itr = Gnomes.erase(itr);
                        continue;
                    }

                    if (Gnome->GetAIInterface()->getNextTarget())
                        continue;

                    if (Gnome->GetAIInterface()->getUnitToFollow() == NULL)
                    {
                        Gnome->GetAIInterface()->SetUnitToFollow(_unit);
                        Gnome->GetAIInterface()->SetFollowDistance(15.0f);
                    }

                    if (_unit->GetDistance2dSq(Gnome) > 250.0f)
                    {
                        if (Gnome->GetCurrentSpell() != NULL)
                            Gnome->GetCurrentSpell()->cancel();

                        continue;
                    }

                    if (Gnome->GetCurrentSpell() == NULL)
                    {
                        Gnome->GetAIInterface()->StopMovement(1);
                        Gnome->CastSpell(_unit, REPAIR, false);    // core problem? casted on self (and effect is applied on caster instead of _unit)
                    }
                }
            }

            uint32 t = (uint32)time(NULL);
            if (t > spells[3].casttime)
            {
                _unit->SendScriptTextChatMessage(SAY_MEKGINEER_STEAMRIGGER_08);

                _unit->CastSpell(_unit, spells[3].info, spells[3].instant);

                spells[3].casttime = t + spells[3].cooldown;
            }

            if ((_unit->GetHealthPct() <= 75 && GnomeCounter == 0) || (_unit->GetHealthPct() <= 50 && GnomeCounter == 1) || (_unit->GetHealthPct() <= 25 && GnomeCounter == 2))
            {
                Unit* Gnome = NULL;
                for (uint8 i = 0; i < 3; i++)
                {
                    Gnome = _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_STEAMRIGGER_MECHANIC, SpawnCoords[i].x, SpawnCoords[i].y, SpawnCoords[i].z, SpawnCoords[i].o, true, false, _unit->GetFaction(), 50);
                    if (Gnome)
                    {
                        Gnome->GetAIInterface()->SetUnitToFollow(_unit);
                        Gnome->GetAIInterface()->SetFollowDistance(15.0f);
                        Gnomes.push_back(Gnome);
                    }
                }

                _unit->SendScriptTextChatMessage(SAY_MEKGINEER_STEAMRIGGER_01);

                GnomeCounter++;
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

                auto random_index = RandomUInt(0, TargetTable.size() - 1);
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
                        _unit->CastSpellAoF(random_target->GetPositionX(), random_target->GetPositionY(), random_target->GetPositionZ(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:

        uint8 GnomeCounter;
        uint8 nrspells;
};

//---- Warlord Kalitresh Encounter ----//

static Location Distiller[] =
{
    {  },
    { -113.183952f, -488.599335f, 8.196310f, 6.134734f },
    {  -76.880989f, -489.164673f, 8.236189f, 2.103285f },
    {  -74.309753f, -520.418884f, 8.255078f, 4.612630f },
    { -116.220764f, -520.139771f, 8.198921f, 5.127069f }
};

static Location DistillerMoveTo[] =
{
    {  },
    { -108.092949f, -491.747803f, 8.198845f,  0.621336f },
    {  -81.165871f, -492.459869f, 8.255936f,  6.531955f },
    {  -79.170982f, -518.544800f, 8.241381f, -2.281880f },
    { -112.033188f, -517.945190f, 8.205022f, -0.949258f }
};

// Naga DistillerAI
class NagaDistillerAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(NagaDistillerAI);

        NagaDistillerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            _unit->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
            _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
            _unit->GetAIInterface()->disable_melee = true;
            _unit->GetAIInterface()->m_canMove = false;
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SetChannelSpellTargetGUID(0);
            _unit->SetChannelSpellId(0);
        }
};

// Warlord Kalitresh AI
class WarlordKalitreshAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(WarlordKalitreshAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        WarlordKalitreshAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(IMPALE);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = true;
            spells[0].cooldown = 10;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].mindist2cast = 0.0f;
            spells[0].maxdist2cast = 40.0f;

            spells[1].info = dbcSpell.LookupEntry(HEAD_CRACK);
            spells[1].targettype = TARGET_RANDOM_SINGLE;
            spells[1].instant = true;
            spells[1].cooldown = 10;
            spells[1].perctrigger = 8.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].mindist2cast = 0.0f;
            spells[1].maxdist2cast = 40.0f;

            spells[2].info = dbcSpell.LookupEntry(SPELL_REFLECTION);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].cooldown = 25;
            spells[2].perctrigger = 8.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(WARLORDS_RAGE);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = 90;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;
            spells[3].soundid = 10391;
            spells[3].speech = "This is not nearly over!";

            DistillerNumber = 0;
            RagePhaseTimer = 0;
            EnrageTimer = 0;
            RagePhase = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            RagePhaseTimer = (uint32)time(NULL) + RandomUInt(15) + 10;
            DistillerNumber = 0;
            EnrageTimer = 0;
            RagePhase = 0;

            GameObject* Gate = NULL;
            Gate = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-95.774361f, -439.608612f, 3.382976f, 183049);
            if (Gate)
                Gate->SetState(GAMEOBJECT_STATE_CLOSED);

            switch (RandomUInt(2))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_WARLORD_KALITRESH_03);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_WARLORD_KALITRESH_04);
                    break;
                case 2:
                    _unit->SendScriptTextChatMessage(SAY_WARLORD_KALITRESH_05);
                    break;
            }

            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_WARLORD_KALITRESH_06);
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(SAY_WARLORD_KALITRESH_07);
                        break;
                }
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            GameObject* Gate = NULL;
            Gate = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-95.774361f, -439.608612f, 3.382976f, 183049);
            if (Gate)
                Gate->SetState(GAMEOBJECT_STATE_OPEN);

            _unit->GetAIInterface()->SetAllowedToEnterCombat(true);
            _unit->GetAIInterface()->m_canMove = true;
            _unit->GetAIInterface()->ResetUnitToFollow();
            _unit->GetAIInterface()->SetFollowDistance(0.0f);
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);

            if (_unit->FindAura(37076))
                _unit->RemoveAura(37076);
            if (_unit->FindAura(36453))
                _unit->RemoveAura(36453);

            Unit* pDistiller = NULL;
            pDistiller = GetClosestDistiller();
            if (pDistiller)
            {
                pDistiller->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
                pDistiller->SetChannelSpellTargetGUID(0);
                pDistiller->SetChannelSpellId(0);
                pDistiller->GetAIInterface()->WipeTargetList();
                pDistiller->GetAIInterface()->WipeHateList();
            }

            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(SAY_WARLORD_KALITRESH_08);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            uint32 t = (uint32)time(NULL);
            if (t > RagePhaseTimer)
            {
                if (EnrageTimer != 0)
                    EnrageTimer++;

                Unit* pDistiller = NULL;
                pDistiller = GetClosestDistiller();
                if (!pDistiller || (pDistiller->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9) && RagePhase != 0))
                {
                    _unit->GetAIInterface()->SetAllowedToEnterCombat(true);
                    _unit->GetAIInterface()->m_canMove = true;
                    _unit->GetAIInterface()->ResetUnitToFollow();
                    _unit->GetAIInterface()->SetFollowDistance(0.0f);

                    RagePhaseTimer = t + RandomUInt(15) + 20;
                    EnrageTimer = 0;
                    RagePhase = 0;

                    if (_unit->FindAura(31543))
                        _unit->RemoveAura(31543);
                    if (_unit->FindAura(37076))
                        _unit->RemoveAura(37076);
                }

                else
                {
                    if (EnrageTimer == 0 && RagePhase == 0)
                    {
                        _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
                        _unit->GetAIInterface()->SetUnitToFollow(pDistiller);
                        _unit->GetAIInterface()->SetFollowDistance(10.0f);

                        _unit->GetAIInterface()->StopMovement(0);
                        _unit->GetAIInterface()->SetAIState(STATE_SCRIPTMOVE);
                        _unit->GetAIInterface()->MoveTo(DistillerMoveTo[DistillerNumber].x, DistillerMoveTo[DistillerNumber].y, DistillerMoveTo[DistillerNumber].z, DistillerMoveTo[DistillerNumber].o);

                        if (_unit->GetDistance2dSq(pDistiller) <= 100.0f)
                        {
                            pDistiller->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
                            pDistiller->SetChannelSpellTargetGUID(_unit->GetGUID());
                            pDistiller->SetChannelSpellId(31543);

                            _unit->GetAIInterface()->StopMovement(0);
                            _unit->GetAIInterface()->SetAIState(STATE_SCRIPTIDLE);
                            _unit->GetAIInterface()->m_canMove = false;
                            _unit->SendScriptTextChatMessage(SAY_WARLORD_KALITRESH_02);

                            if (!_unit->FindAura(36453))
                                _unit->CastSpell(_unit, 31543, true);

                            EnrageTimer = t + 3;
                            RagePhase = 1;
                        }
                    }

                    else if (t > EnrageTimer && RagePhase == 1)
                    {
                        EnrageTimer = t + 6;    // 9
                        RagePhase = 2;
                    }

                    else if (t > EnrageTimer && RagePhase == 2)
                    {
                        pDistiller->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
                        pDistiller->SetChannelSpellTargetGUID(0);
                        pDistiller->SetChannelSpellId(0);
                        pDistiller->GetAIInterface()->WipeTargetList();
                        pDistiller->GetAIInterface()->WipeHateList();

                        _unit->GetAIInterface()->SetAllowedToEnterCombat(true);
                        _unit->GetAIInterface()->m_canMove = true;
                        _unit->GetAIInterface()->ResetUnitToFollow();
                        _unit->GetAIInterface()->SetFollowDistance(0.0f);
                        _unit->CastSpell(_unit, 36453, true);

                        RagePhaseTimer = t + RandomUInt(15) + 20;
                        DistillerNumber = 0;
                        EnrageTimer = 0;
                        RagePhase = 0;
                    }
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

                auto random_index = RandomUInt(0, TargetTable.size() - 1);
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
                        _unit->CastSpellAoF(random_target->GetPositionX(), random_target->GetPositionY(), random_target->GetPositionZ(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

        Unit* GetClosestDistiller()
        {
            float distance = 50.0f;
            Unit* pDistiller = NULL;
            Unit* Unit2Check = NULL;

            for (uint8 i = 1; i < 5; i++)
            {
                Unit2Check = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(Distiller[i].x, Distiller[i].y, Distiller[i].z, 17954);
                if (!Unit2Check)
                    continue;

                if (!Unit2Check->isAlive())
                    continue;

                float Dist2Unit = _unit->GetDistance2dSq(Unit2Check);
                if (Dist2Unit > distance * distance)
                    continue;

                pDistiller = Unit2Check;
                distance = sqrt(Dist2Unit);
                DistillerNumber = i;
            }

            return pDistiller;
        }

    protected:

        uint32 DistillerNumber;
        uint32 RagePhaseTimer;
        uint32 EnrageTimer;
        uint32 RagePhase;
        uint8 nrspells;
};

/** \todo Check all spells/creatures and evenatually balance them (if needed!)
   also add spawns correctly (no core support for now and hacky Onyxia way does
   not work (in Onyxia also). Change target type when there will be more core
   support. Also very imporant thing is to force spawns to heal random or target
   you choosed. When that will work, I will add AI for mechanics who should
   heal one of instance bosses.*/
// Don't have infos about: Second Fragment Guardian (22891) | Dreghood Slave (17799 -
// should they really have enrage ? [8269]), Driller should use Warlord's Rage spell
// (31543) to force Warlord to enrage, but I need more infos about targeting target
// you want to.
void SetupTheSteamvault(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_CF_STEAMVAULT, &InstanceTheSteamvaultScript::Create);

    //Creatures
    /*mgr->register_creature_script(CN_COILFANG_ORACLE, &CoilfangOracleAI::Create);
    mgr->register_creature_script(CN_COILFANG_ENGINEER, &CoilfangEngineerAI::Create);
    mgr->register_creature_script(CN_COILFANG_WARRIOR, &CoilfangWarriorAI::Create);
    mgr->register_creature_script(CN_COILFANG_SIREN, &CoilfangSirenAI::Create);
    mgr->register_creature_script(CN_COILFANG_SORCERESS, &CoilfangSorceressAI::Create);
    mgr->register_creature_script(CN_COILFANG_LEPER, &CoilfangLeperAI::Create);
    mgr->register_creature_script(CN_COILFANG_SLAVEMASTER, &CoilfangSlavemasterAI::Create);
    mgr->register_creature_script(CN_COILFANG_MYRMIDON, &CoilfangMyrmidonAI::Create);
    mgr->register_creature_script(CN_COILFANG_WATER_ELEMENTAL, &CoilfangWaterElementalAI::Create);
    mgr->register_creature_script(CN_BOG_OVERLORD, &BogOverlordAI::Create);
    mgr->register_creature_script(CN_TIDAL_SURGER, &TidalSurgerAI::Create);
    mgr->register_creature_script(CN_STEAM_SURGER, &SteamSurgerAI::Create);*/
    mgr->register_creature_script(CN_HYDROMANCER_THESPIA, &HydromancerThespiaAI::Create);
    mgr->register_creature_script(CN_STEAMRIGGER_MECHANIC, &SteamriggerMechanicAI::Create);
    mgr->register_creature_script(CN_MEKGINEER_STEAMRIGGER, &MekgineerSteamriggerAI::Create);
    mgr->register_creature_script(CN_NAGA_DISTILLER, &NagaDistillerAI::Create);
    mgr->register_creature_script(CN_WARLORD_KALITRESH, &WarlordKalitreshAI::Create);
}
