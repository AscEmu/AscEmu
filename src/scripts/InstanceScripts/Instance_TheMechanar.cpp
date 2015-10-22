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
#include "Instance_TheMechanar.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Tempest Keep: The Mechanar
class InstanceTheMechanarScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceTheMechanarScript, MoonInstanceScript);
        InstanceTheMechanarScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
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

// Arcane ServantAI
class ArcaneServantAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ArcaneServantAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        ArcaneServantAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = dbcSpell.LookupEntry(SP_ARCANE_VOLLEY);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_ARCANE_EXPLOSION);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 15.0f;
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

// Bloodwarder CenturionAI
class BloodwarderCenturionAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(BloodwarderCenturionAI);
        SP_AI_Spell spells[5];
        bool m_spellcheck[5];

        BloodwarderCenturionAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 5;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = dbcSpell.LookupEntry(SP_CENTURION_SHIELD_BASH);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 5.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_CENTURION_UNSTABLE_AFFLICTION);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 6.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_CENTURION_MELT_ARMOR);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 6.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(SP_CENTURION_CHILLING_TOUCH);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 8.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = dbcSpell.LookupEntry(SP_CENTURION_ETHEREAL_TELEPORT);
            spells[4].targettype = TARGET_SELF;
            spells[4].instant = true;
            spells[4].cooldown = -1;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->CastSpell(_unit, spells[4].info, spells[4].instant);
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

// Bloodwarder PhysicianAI
class BloodwarderPhysicianAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(BloodwarderPhysicianAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        BloodwarderPhysicianAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = dbcSpell.LookupEntry(SP_PHYSICIAN_HOLY_SHOCK);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_PHYSICIAN_ANESTHETIC);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 6.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_PHYSICIAN_BANDAGE);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = false;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 6.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(SP_PHYSICIAN_ETHEREAL_TELEPORT_PHYS);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->CastSpell(_unit, spells[3].info, spells[3].instant);
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

// Bloodwarder SlayerAI
class BloodwarderSlayerAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(BloodwarderSlayerAI);
        SP_AI_Spell spells[5];
        bool m_spellcheck[5];

        BloodwarderSlayerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 5;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = dbcSpell.LookupEntry(SP_SLAYER_WHIRLWIND);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 15.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_SLAYER_SOLAR_STRIKE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 8.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_SLAYER_MELT_ARMOR);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 5.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(SP_SLAYER_CHILLING_TOUCH);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 5.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = dbcSpell.LookupEntry(SP_SLAYER_MORTAL_STRIKE);
            spells[4].targettype = TARGET_ATTACKING;
            spells[4].instant = true;
            spells[4].cooldown = -1;
            spells[4].perctrigger = 10.0f;
            spells[4].attackstoptimer = 1000;

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

// Mechanar CrusherAI
class MechanarCrusherAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(MechanarCrusherAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        MechanarCrusherAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = dbcSpell.LookupEntry(SP_MECH_CRUSHER_DISARM);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 8.0f;
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

// Mechanar DrillerAI
class MechanarDrillerAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(MechanarDrillerAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        MechanarDrillerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = dbcSpell.LookupEntry(SP_MECH_DRILLER_GLOB_OF_MACHINE_FLUID);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 5.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_MECH_DRILLER_ARMOR);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 8.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_MECH_DRILLER_CRIPPLING_POISON);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 10.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(SP_MECH_DRILLER_POUND);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 10.0f;
            spells[3].attackstoptimer = 1000;

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

// Mechanar TinkererAI
class MechanarTinkererAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(MechanarTinkererAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        MechanarTinkererAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = dbcSpell.LookupEntry(SP_MECH_TINKERER_NETHERBOMB);
            spells[0].targettype = TARGET_DESTINATION;
            spells[0].instant = false;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 5.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_MECH_TINKERER_PRAYER_OF_MENDING);
            spells[1].targettype = TARGET_VARIOUS;    // ?
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 8.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_MECH_TINKERER_MANIACAL_CHARGE);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 5.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(SP_MECH_TINKERER_NETHER_EXPLOSION);
            spells[3].targettype = TARGET_VARIOUS;
            spells[3].instant = false;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

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
            _unit->CastSpell(_unit, spells[3].info, spells[3].instant);
            CastTime();
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            /*    // not working yet, I must found out why you can't loot after death (hmm, sth is wrong? :P)
            if (_unit->GetHealthPct() < 15 && _unit->GetAIInterface()->GetNextTarget())
            {
            uint32 val = sRand.rand(100);
            if (val > 0 && val < 20)
            {
            _unit->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
            _unit->setDeathState(JUST_DIED);
            _unit->GetAIInterface()->HandleEvent(EVENT_UNITDIED, _unit, 0);
            //_unit->setDeathState(DEATH);
            //CALL_SCRIPT_EVENT(m_Unit, OnDied)(_unit);
            //_unit->GetAIInterface()->HandleEvent(EVENT_UNITDIED, _unit, misc1);    //, uint32 misc1
            //CastTime();
            //RemoveAIUpdateEvent();
            }
            }
            */
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

// Mechanar WreckerAI
class MechanarWreckerAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(MechanarWreckerAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        MechanarWreckerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = dbcSpell.LookupEntry(SP_MECH_WRECKER_POUND);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 12.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_MECH_WRECKER_GLOB_OF_MACHINE_FLUID);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 7.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_MECH_WRECKER_PRAYER_OF_MENDING);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 6.0f;
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

// Raging FlamesAI
class RagingFlamesAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(RagingFlamesAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        RagingFlamesAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = dbcSpell.LookupEntry(SP_RAGING_FLAMES);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 0.0f;    // 8// disabled to prevent crashes
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_RAGING_FLAMES_INFERNO);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 9.0f;
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

// Sunseeker AstromageAI
class SunseekerAstromageAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(SunseekerAstromageAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        SunseekerAstromageAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = dbcSpell.LookupEntry(SP_SS_ASTROMAGE_SCORCH);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 12.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_SS_ASTROMAGE_SOLARBURN);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 10.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_SS_ASTROMAGE_FIRE_SHIELD);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 8.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(SP_SS_ASTROMAGE_ETHEREAL_TELEPORT);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->CastSpell(_unit, spells[3].info, spells[3].instant);
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

// Sunseeker EngineerAI
class SunseekerEngineerAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(SunseekerEngineerAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        SunseekerEngineerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = dbcSpell.LookupEntry(SP_SS_ENGINEER_SUPER_SHRINK_RAY);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_SS_ENGINEER_DEATH_RAY);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 13.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_SS_ENGINEER_GROWTH_RAY);
            spells[2].targettype = TARGET_SELF;    // ?
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 7.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(SP_SS_ENGINEER_ETHEREAL_TELEPORT);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->CastSpell(_unit, spells[3].info, spells[3].instant);
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

// Sunseeker NetherbinderAI
class SunseekerNetherbinderAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(SunseekerNetherbinderAI);
        SP_AI_Spell spells[5];
        bool m_spellcheck[5];

        SunseekerNetherbinderAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 5;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = dbcSpell.LookupEntry(SP_SS_NETHERBINDER_ARCANE_NOVA);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_SS_NETHERBINDER_STARFIRE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 13.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_SS_NETHERBINDER_SUMMON_ARCANE_GOLEM1);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 5.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(SP_SS_NETHERBINDER_SUMMON_ARCANE_GOLEM2);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 5.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = dbcSpell.LookupEntry(SP_SS_NETHERBINDER_DISPEL_MAGIC);
            spells[4].targettype = TARGET_SELF;
            spells[4].instant = true;
            spells[4].cooldown = -1;
            spells[4].perctrigger = 8.0f;
            spells[4].attackstoptimer = 1000;
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

// Tempest-Forge DestroyerAI
class TempestForgeDestroyerAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(TempestForgeDestroyerAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        TempestForgeDestroyerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = dbcSpell.LookupEntry(SP_TEMPEST_DESTROYER_KNOCKDOWN);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_TEMPEST_DESTROYER_CHARGED_FIST);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 12.0f;
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

// Tempest-Forge PatrollerAI
class TempestForgePatrollerAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(TempestForgePatrollerAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        TempestForgePatrollerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            spells[0].info = dbcSpell.LookupEntry(SP_TEMPEST_PAT_CHARGED_ARCANE_MISSILE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 12.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_TEMPEST_PAT_KNOCKDOWN);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 8.0f;
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

///////////////////////////////////////////////////////////////////////////////////
// Boss AIs
//////////////////////////////////////////////////////////////////////////////////

// Gatewatcher Gyro-Kill AI
class GatewatcherGyroKillAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(GatewatcherGyroKillAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        GatewatcherGyroKillAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_GW_GYRO_KILL_SAW_BLADE);
            spells[0].targettype = TARGET_ATTACKING;    // to prevent crashes when used VARIOUS
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 13.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_GW_GYRO_KILL_SHADOW_POWER);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 7.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_GW_GYRO_KILL_STEAM_OF_MACHINE_FLUID);
            spells[2].targettype = TARGET_VARIOUS;    // VARIOUS doesn't work somehow :S (sometimes yes, sometimes no)
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 9.0f;
            spells[2].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            _unit->SendScriptTextChatMessage(SAY_GW_GYRO_KILL_05);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_GW_GYRO_KILL_06);
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(SAY_GW_GYRO_KILL_04);
                        break;
                }
            }
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
            RemoveAIUpdateEvent();
            _unit->SendScriptTextChatMessage(SAY_GW_GYRO_KILL_01);
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SawBladesSound()
        {
            switch (RandomUInt(5))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_GW_GYRO_KILL_02);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_GW_GYRO_KILL_03);
                    break;
                default:
                    break;
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

                        if (m_spellcheck[0] == true)
                            SawBladesSound();

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

// Gatewatcher Iron-Hand AI
class GatewatcherIronHandAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(GatewatcherIronHandAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        GatewatcherIronHandAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_GW_IRON_HAND_JACK_HAMMER);
            spells[0].targettype = TARGET_VARIOUS;    // why this is spammed when casted ? :| maybe core bug? :|
            spells[0].instant = false;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 7.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_GW_IRON_HAND_HAMMER_PUNCH);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 9.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_GW_IRON_HAND_STREAM_OF_MACHINE_FLUID);
            spells[2].targettype = TARGET_VARIOUS;    // VARIOUS doesn't work somehow (sometimes yes, sometimes no)
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 7.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(SP_GW_IRON_HAND_SHADOW_POWER);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = false;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 5.0f;
            spells[3].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            _unit->SendScriptTextChatMessage(SAY_GW_GYRO_KILL_01);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_GW_GYRO_KILL_04);
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(SAY_GW_GYRO_KILL_05);
                        break;
                }
            }
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
            RemoveAIUpdateEvent();
            _unit->SendScriptTextChatMessage(SAY_GW_GYRO_KILL_06);
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void HammerSound()
        {
            switch (RandomUInt(8))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_GW_GYRO_KILL_02);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_GW_GYRO_KILL_03);
                    break;
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

                        if (m_spellcheck[1] == true)
                        {
                            HammerSound();
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

// Mechano-Lord Capacitus AI
class MechanoLordCapacitusAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(MechanoLordCapacitusAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        MechanoLordCapacitusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_MECH_LORD_HEAD_CRACK);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_MECH_LORD_REFLECTIVE_DAMAGE_SHIELD);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 7.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].speech = "Think you can hurt me, huh? Think I'm afraid a' you?";
            spells[1].soundid = 11165;

            spells[2].info = dbcSpell.LookupEntry(SP_MECH_LORD_REFLECTIVE_MAGIC_SHIELD);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = false;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 7.0f;
            spells[2].attackstoptimer = 1000;
            spells[2].speech = "Go ahead, gimme your best shot. I can take it!";
            spells[2].soundid = 11166;

            spells[3].info = dbcSpell.LookupEntry(SP_MECH_LORD_SEED_OF_CORRUPTION);    // it won't work anyway
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 5.0f;
            spells[3].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            _unit->SendScriptTextChatMessage(SAY_MECH_LORD_06);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_MECH_LORD_03);
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(SAY_MECH_LORD_02);
                        break;
                }
            }
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
            RemoveAIUpdateEvent();
            _unit->SendScriptTextChatMessage(SAY_MECH_LORD_01);
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

// Nethermancer Sepethrea AI
class NethermancerSepethreaAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(NethermancerSepethreaAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        NethermancerSepethreaAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            SummonTimer = 4;
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_NETH_SEPETHREA_SUMMON_RAGIN_FLAMES);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_NETH_SEPETHREA_FROST_ATTACK);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 9.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_NETH_SEPETHREA_ARCANE_BLAST);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 3.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(SP_NETH_SEPETHREA_DRAGONS_BREATH);
            spells[3].targettype = TARGET_VARIOUS;    // doesn't afffect when VARIOUS? WTF? :|  Sometimes works, sometimes not? :|
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 8.0f;
            spells[3].attackstoptimer = 1000;
            /*
                    spells[4].info = dbcSpell.LookupEntry(KNOCKBAC);
                    spells[4].targettype = TARGET_ATTACKING;
                    spells[4].instant = true;
                    spells[4].cooldown = -1;
                    spells[4].perctrigger = 2.0f;
                    spells[4].attackstoptimer = 1000;
                    */
        }

        void OnCombatStart(Unit* mTarget)
        {
            SummonTimer = 4;
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            //_unit->CastSpell(_unit, spells[0].info, spells[0].instant);
            _unit->SendScriptTextChatMessage(SAY_NETH_SEPETHREA_01);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* mTarget)
        {
            SummonTimer = 4;
            CastTime();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_NETH_SEPETHREA_05);
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(SAY_NETH_SEPETHREA_06);
                        break;
                }
            }
        }

        void OnDied(Unit* mKiller)
        {
            SummonTimer = 4;
            CastTime();
            RemoveAIUpdateEvent();
            _unit->SendScriptTextChatMessage(SAY_NETH_SEPETHREA_07);
        }

        void AIUpdate()
        {
            SummonTimer--;

            if (!SummonTimer)
            {
                _unit->CastSpell(_unit, spells[0].info, spells[0].instant);
                _unit->SendScriptTextChatMessage(SAY_NETH_SEPETHREA_02);
            }

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void DragonsBreathSound()
        {
            switch (RandomUInt(8))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_NETH_SEPETHREA_03);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_NETH_SEPETHREA_04);
                    break;
                default:
                    break;
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

                        if (m_spellcheck[3] == true)
                            DragonsBreathSound();

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

        int SummonTimer;
        uint8 nrspells;
};


// Pathaleon the Calculator AI
// hmm... he switches weapons and there is sound for it, but I must know when he does that, how it looks like etc.
// before adding weapon switching =/    (Sound: 11199; speech: "I prefer to be hands-on...";)
class PathaleonTheCalculatorAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(PathaleonTheCalculatorAI);
        SP_AI_Spell spells[7];
        bool m_spellcheck[7];

        PathaleonTheCalculatorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            SummonTimer = RandomUInt(30, 45);   // 30 - 45 sec
            nrspells = 7;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_PATHALEON_MANA_TRAP);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_PATHALEON_DOMINATION);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 4.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_PATHALEON_SILENCE);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 6.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(SP_PATHALEON_SUMMON_NETHER_WRAITH1);
            spells[3].targettype = TARGET_SELF;    // hmm
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = dbcSpell.LookupEntry(SP_PATHALEON_SUMMON_NETHER_WRAITH2);
            spells[4].targettype = TARGET_SELF;    // hmm
            spells[4].instant = true;
            spells[4].cooldown = -1;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;

            spells[5].info = dbcSpell.LookupEntry(SP_PATHALEON_SUMMON_NETHER_WRAITH3);
            spells[5].targettype = TARGET_SELF;    // hmm
            spells[5].instant = true;
            spells[5].cooldown = -1;
            spells[5].perctrigger = 0.0f;
            spells[5].attackstoptimer = 1000;

            spells[6].info = dbcSpell.LookupEntry(SP_PATHALEON_SUMMON_NETHER_WRAITH4);
            spells[6].targettype = TARGET_SELF;    // hmm
            spells[6].instant = true;
            spells[6].cooldown = -1;
            spells[6].perctrigger = 0.0f;
            spells[6].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            SummonTimer = RandomUInt(30, 45);
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            _unit->SendScriptTextChatMessage(SAY_PATHALEON_01);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* mTarget)
        {
            SummonTimer = RandomUInt(30, 45);
            CastTime();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_PATHALEON_06);
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(SAY_PATHALEON_07);
                        break;
                }
            }
        }

        void OnDied(Unit* mKiller)
        {
            SummonTimer = RandomUInt(30, 45);
            CastTime();
            RemoveAIUpdateEvent();
            _unit->SendScriptTextChatMessage(SAY_PATHALEON_08);
        }

        void AIUpdate()
        {
            SummonTimer--;

            if (!SummonTimer)
            {
                _unit->CastSpell(_unit, spells[3].info, spells[3].instant);
                _unit->CastSpell(_unit, spells[4].info, spells[4].instant);
                _unit->CastSpell(_unit, spells[5].info, spells[5].instant);
                _unit->CastSpell(_unit, spells[6].info, spells[6].instant);
                SummonTimer = RandomUInt(30, 45);    // 30 - 45
                _unit->SendScriptTextChatMessage(SAY_PATHALEON_04);
            }

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void DominationSound()
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_PATHALEON_02);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_PATHALEON_03);
                    break;
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

                        if (m_spellcheck[1] == true)
                        {
                            DominationSound();
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

        uint32 SummonTimer;
        uint8 nrspells;
};

/// \todo Data needed for: Nether Wraith, Mechanar Crusher (maybe not enough?)
void SetupTheMechanar(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_TK_THE_MECHANAR, &InstanceTheMechanarScript::Create);

    //Creatures
    mgr->register_creature_script(CN_ARCANE_SERVANT, &ArcaneServantAI::Create);
    mgr->register_creature_script(CN_BLOODWARDER_CENTURION, &BloodwarderCenturionAI::Create);
    mgr->register_creature_script(CN_BLOODWARDER_PHYSICIAN, &BloodwarderPhysicianAI::Create);
    mgr->register_creature_script(CN_BLOODWARDER_SLAYER, &BloodwarderSlayerAI::Create);
    mgr->register_creature_script(CN_MECHANAR_CRUSHER, &MechanarCrusherAI::Create);
    mgr->register_creature_script(CN_MECHANAR_DRILLER, &MechanarDrillerAI::Create);
    mgr->register_creature_script(CN_MECHANAR_TINKERER, &MechanarTinkererAI::Create);
    mgr->register_creature_script(CN_MECHANAR_WRECKER, &MechanarWreckerAI::Create);
    mgr->register_creature_script(CN_RAGING_FLAMES, &RagingFlamesAI::Create);
    mgr->register_creature_script(CN_SUNSEEKER_ASTROMAGE, &SunseekerAstromageAI::Create);
    mgr->register_creature_script(CN_SUNSEEKER_ENGINEER, &SunseekerEngineerAI::Create);
    mgr->register_creature_script(CN_SUNSEEKER_NETHERBINDER, &SunseekerNetherbinderAI::Create);
    mgr->register_creature_script(CN_TEMPEST_FORGE_DESTROYER, &TempestForgeDestroyerAI::Create);
    mgr->register_creature_script(CN_TEMPEST_FORGE_PATROLLER, &TempestForgePatrollerAI::Create);
    mgr->register_creature_script(CN_GATEWATCHER_GYRO_KILL, &GatewatcherGyroKillAI::Create);
    mgr->register_creature_script(CN_GATEWATCHER_IRON_HAND, &GatewatcherIronHandAI::Create);
    mgr->register_creature_script(CN_MECHANO_LORD_CAPACITUS, &MechanoLordCapacitusAI::Create);
    mgr->register_creature_script(CN_NETHERMANCER_SEPETHREA, &NethermancerSepethreaAI::Create);
    mgr->register_creature_script(CN_PATHALEON_THE_CALCULATOR, &PathaleonTheCalculatorAI::Create);
}
