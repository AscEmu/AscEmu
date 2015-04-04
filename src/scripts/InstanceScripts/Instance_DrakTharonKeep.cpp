/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Instance_DrakTharonKeep.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Drak'Tharon Keep
class InstanceDrakTharonKeepScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceDrakTharonKeepScript, MoonInstanceScript);
        InstanceDrakTharonKeepScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
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

/*
 Trollgore - TOO EASY!!
 \todo Whole corpses/consume thingo is wrong
 NOTES:
 Core doesn't support auras on corpses, we are currently unable to script this blizzlike
 */

//TrollgoreAI
#define INVASION_INTERVAL 20000
#define INVADERS_PER_INVASION 1
//two mobs per 10s

class TrollgoreAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(TrollgoreAI);
        TrollgoreAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            heroic = (_unit->GetMapMgr()->iInstanceMode == MODE_HEROIC);
            invastion_timer = 0;
            spells.clear();
            /* SPELLS INIT */
            ScriptSpell* Crush = new ScriptSpell;
            Crush->normal_spellid = 49639;
            Crush->heroic_spellid = 49639;
            Crush->chance = 20;
            Crush->timer = 1000;
            Crush->time = 0;
            Crush->target = SPELL_TARGET_CURRENT_ENEMY;
            spells.push_back(Crush);

            ScriptSpell* Infected_Wound = new ScriptSpell;
            Infected_Wound->normal_spellid = 49637;
            Infected_Wound->heroic_spellid = 49637;
            Infected_Wound->chance = 50;
            Infected_Wound->timer = 8000;
            Infected_Wound->time = 0;
            Infected_Wound->target = SPELL_TARGET_CURRENT_ENEMY;
            spells.push_back(Infected_Wound);

            ScriptSpell* Consume = new ScriptSpell;
            Consume->normal_spellid = 49381;
            Consume->heroic_spellid = 59805;
            Consume->chance = 100;
            Consume->timer = 10000;
            Consume->time = 0;
            Consume->target = SPELL_TARGET_SELF;
            spells.push_back(Consume);
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit*  mKiller)
        {
            RemoveAIUpdateEvent();
        }

        void OnDamageTaken(Unit* mAttacker, uint32 fAmount)
        {
            /*if (mAttacker->IsCreature() && TO_CREATURE(mAttacker)->GetProto()->Id == DRAKKARI_INVADER_ENTRY)
            {
            uint32 spellid = heroic ? 59809 : 49618;
            //corpse cannot have aura, cannot cast spell, so we have to change this
            //also if he cast that spells, no players are being affected, target any player
            _unit->CastSpell(GetRandomPlayerTarget(), spellid, true);
            }*/
        }

        void AIUpdate()
        {
            if (spells.size() > 0)
            {
                for (uint8 i = 0; i < spells.size(); i++)
                {
                    if (spells[i]->time < getMSTime())
                    {
                        if (Rand(spells[i]->chance))
                        {
                            CastScriptSpell(spells[i]);
                            spells[i]->time = getMSTime() + spells[i]->timer;
                        }
                    }
                }
            }
            if (invastion_timer < getMSTime())
            {
                invastion_timer = getMSTime() + INVASION_INTERVAL;
                //spawn invaders ;)
                for (uint8 i = 0; i < INVADERS_PER_INVASION; i++)
                {
                    CreatureProto* cp = CreatureProtoStorage.LookupEntry(CN_DRAKKARI_INVADER);
                    CreatureInfo* ci = CreatureNameStorage.LookupEntry(CN_DRAKKARI_INVADER);
                    Creature* c = NULL;
                    if (cp && ci)
                    {
                        c = _unit->GetMapMgr()->CreateCreature(CN_DRAKKARI_INVADER);
                        if (c)
                        {
                            //position is guessed
                            c->Load(cp, -259.532f, -618.976f, 26.669f, 0.0f);
                            c->PushToWorld(_unit->GetMapMgr());
                            //path finding would be usefull :)
                            //c->GetAIInterface()->SetRun();
                            c->GetAIInterface()->MoveTo(_unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), _unit->GetOrientation());
                        }
                    }
                }
            }
        }

        Player* GetRandomPlayerTarget()
        {
            vector< uint32 > possible_targets;
            for (set< Object* >::iterator iter = _unit->GetInRangePlayerSetBegin(); iter != _unit->GetInRangePlayerSetEnd(); ++iter)
            {
                if ((*iter) && (TO< Player* >(*iter))->isAlive())
                    possible_targets.push_back((uint32)(*iter)->GetGUID());
            }
            if (possible_targets.size() > 0)
            {
                uint32 random_player = possible_targets[Rand(uint32(possible_targets.size() - 1))];
                return _unit->GetMapMgr()->GetPlayer(random_player);
            }
            return NULL;
        }

        void CastScriptSpell(ScriptSpell* spell)
        {
            _unit->Root();
            uint32 spellid = heroic ? spell->heroic_spellid : spell->normal_spellid;
            Unit* spelltarget = NULL;
            switch (spell->target)
            {
                case SPELL_TARGET_SELF:
                {
                    spelltarget = _unit;
                }
                break;
                case SPELL_TARGET_GENERATE:
                {
                    spelltarget = NULL;
                }
                break;
                case SPELL_TARGET_CURRENT_ENEMY:
                {
                    spelltarget = _unit->GetAIInterface()->getNextTarget();
                }
                break;
                case SPELL_TARGET_RANDOM_PLAYER:
                {
                    spelltarget = GetRandomPlayerTarget();
                }
                break;
            }
            _unit->CastSpell(spelltarget, spellid, false);
            _unit->Unroot();
        }

        void Destroy()
        {
            for (uint32 i = 0; i < spells.size(); ++i)
            {
                if (spells[i] != NULL)
                    delete spells[i];
            };

            spells.clear();

            delete this;
        };

    protected:

        bool heroic;
        uint32 invastion_timer;
        vector< ScriptSpell* > spells;
};

/*
 Novos the Summoner
 \todo
 - Crystal should be actived/deactived instead of created/deleted, minor
 - Create waypoints for summons, we need them coz Core doesn't not have path finding
 */

// NovosTheSummonerAI
#define INVADE_INTERVAL 30000//4 mobs per 30s
#define INVADERS_COUNT 3
#define HANDLER_INTERVAL 60000//one handler per 60s
#define ELITE_CHANCE 20//how much chance for elite we've got each invasion?

class NovosTheSummonerAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(NovosTheSummonerAI);
        NovosTheSummonerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            heroic = (_unit->GetMapMgr()->iInstanceMode == MODE_HEROIC);
            phase = 0;
            invasion_timer = 0;
            handler_timer = 0;
            spells.clear();
            /* SPELLS INIT */
            ScriptSpell* ArcaneBlast = new ScriptSpell;
            ArcaneBlast->normal_spellid = 49198;
            ArcaneBlast->heroic_spellid = 59909;
            ArcaneBlast->chance = 70;
            ArcaneBlast->timer = 4000;
            ArcaneBlast->time = 0;
            ArcaneBlast->target = SPELL_TARGET_CURRENT_ENEMY;
            spells.push_back(ArcaneBlast);

            ScriptSpell* Blizzard = new ScriptSpell;
            Blizzard->normal_spellid = 49034;
            Blizzard->heroic_spellid = 59854;
            Blizzard->chance = 50;
            Blizzard->timer = 6000;
            Blizzard->time = 0;
            Blizzard->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(Blizzard);

            ScriptSpell* Frostbolt = new ScriptSpell;
            Frostbolt->normal_spellid = 49037;
            Frostbolt->heroic_spellid = 59855;
            Frostbolt->chance = 40;
            Frostbolt->timer = 2000;
            Frostbolt->time = 0;
            Frostbolt->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(Frostbolt);

            ScriptSpell* WrathOfMisery = new ScriptSpell;
            WrathOfMisery->normal_spellid = 50089;
            WrathOfMisery->heroic_spellid = 59856;
            WrathOfMisery->chance = 10;
            WrathOfMisery->timer = 5000;
            WrathOfMisery->time = 0;
            WrathOfMisery->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(WrathOfMisery);
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(SAY_NOVOS_SUMMONER_01);
            _unit->CastSpell(_unit, 47346, false);
            //spawn 4 Ritual Crystal
            for (uint8 i = 0; i < 4; i++)
                SpawnCrystal(i);
            handler_timer = getMSTime() + HANDLER_INTERVAL;
            _unit->GetAIInterface()->disable_melee = true;
            phase = 1;
            for (uint8 i = 0; i < 7; i++)
                _unit->SchoolImmunityList[i] = 1;
            _unit->SendScriptTextChatMessage(SAY_NOVOS_SUMMONER_05);
            _unit->SendScriptTextChatMessage(SAY_NOVOS_SUMMONER_06);
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnLoad()
        {
            //root him and disable melee for him ;)
            _unit->GetAIInterface()->disable_melee = true;
            _unit->Root();
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
            for (uint8 i = 0; i < 4; i++)
            {
                if (_unit->m_ObjectSlots[i])
                {
                    GameObject* Crystal = _unit->GetMapMgr()->GetGameObject(_unit->m_ObjectSlots[i]);
                    if (Crystal && Crystal->IsInWorld())
                        Crystal->Despawn(0, 0);
                }
            }
            _unit->Root();
            _unit->InterruptSpell();
            _unit->RemoveAllAuras();
        }

        void OnDied(Unit*  mKiller)
        {
            _unit->SendScriptTextChatMessage(SAY_NOVOS_SUMMONER_03);
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            //BUAHAHAHAH
            _unit->SendScriptTextChatMessage(SAY_NOVOS_SUMMONER_02);
        }

        void AIUpdate()
        {
            //we are not using any abilities in first phase
            if (phase == 2 && spells.size() > 0)
            {
                for (uint8 i = 0; i < spells.size(); i++)
                {
                    if (spells[i]->time < getMSTime())
                    {
                        if (Rand(spells[i]->chance))
                        {
                            CastScriptSpell(spells[i]);
                            spells[i]->time = getMSTime() + spells[i]->timer;
                        }
                    }
                }
            }
            if (phase == 1)
            {
                if (invasion_timer < getMSTime())
                {
                    invasion_timer = getMSTime() + INVADE_INTERVAL;
                    SpawnInvader(0);
                }
                if (handler_timer < getMSTime())
                {
                    handler_timer = getMSTime() + HANDLER_INTERVAL;
                    SpawnInvader(1);
                }
                bool new_phase = true;
                for (uint8 i = 0; i < 4; i++)
                {
                    if (_unit->m_ObjectSlots[i])
                    {
                        GameObject* Crystal = _unit->GetMapMgr()->GetGameObject(_unit->m_ObjectSlots[i]);
                        if (Crystal && Crystal->IsInWorld())
                            new_phase = false;
                    }
                }
                if (new_phase)
                {
                    _unit->InterruptSpell();
                    _unit->RemoveAllAuras();
                    _unit->Unroot();
                    _unit->GetAIInterface()->disable_melee = false;
                    phase = 2;
                    for (uint8 i = 0; i < 7; i++)
                        _unit->SchoolImmunityList[i] = 0;
                }
            }
        }

        Player* GetRandomPlayerTarget()
        {

            vector< uint32 > possible_targets;
            for (set< Object* >::iterator iter = _unit->GetInRangePlayerSetBegin(); iter != _unit->GetInRangePlayerSetEnd(); ++iter)
            {
                if ((*iter) && (TO< Player* >(*iter))->isAlive())
                    possible_targets.push_back((uint32)(*iter)->GetGUID());
            }
            if (possible_targets.size() > 0)
            {
                uint32 random_player = possible_targets[Rand(uint32(possible_targets.size() - 1))];
                return _unit->GetMapMgr()->GetPlayer(random_player);
            }
            return NULL;
        }

        void CastScriptSpell(ScriptSpell* spell)
        {
            _unit->Root();
            uint32 spellid = heroic ? spell->heroic_spellid : spell->normal_spellid;
            Unit* spelltarget = NULL;
            switch (spell->target)
            {
                case SPELL_TARGET_SELF:
                {
                    spelltarget = _unit;
                }
                break;
                case SPELL_TARGET_GENERATE:
                {
                    spelltarget = NULL;
                }
                break;
                case SPELL_TARGET_CURRENT_ENEMY:
                {
                    spelltarget = _unit->GetAIInterface()->getNextTarget();
                }
                break;
                case SPELL_TARGET_RANDOM_PLAYER:
                {
                    spelltarget = GetRandomPlayerTarget();
                }
                break;
            }
            _unit->CastSpell(spelltarget, spellid, false);
            if (phase == 2)
                _unit->Unroot();
        }
        //type: 1 - normal, 0 - handler
        void SpawnInvader(uint32 type)
        {
            _unit->SendScriptTextChatMessage(SAY_NOVOS_SUMMONER_04);
            //x                y                z
            //-379.101227f    -824.835449f    60.0f
            uint32 mob_entry = 0;
            if (type)
            {
                mob_entry = 26627;
                CreatureProto* cp = CreatureProtoStorage.LookupEntry(mob_entry);
                CreatureInfo* ci = CreatureNameStorage.LookupEntry(mob_entry);
                Creature* c = NULL;
                if (cp && ci)
                {
                    c = _unit->GetMapMgr()->CreateCreature(mob_entry);
                    if (c)
                    {
                        //position is guessed
                        c->Load(cp, -379.101227f, -824.835449f, 60.0f, 0.0f);
                        c->PushToWorld(_unit->GetMapMgr());
                        c->SetSummonedByGUID(_unit->GetGUID());
                        //path finding would be usefull :)
                        Player* p_target = GetRandomPlayerTarget();
                        if (p_target)
                        {
                            c->GetAIInterface()->SetRun();
                            c->GetAIInterface()->MoveTo(p_target->GetPositionX(), p_target->GetPositionY(), p_target->GetPositionZ(), p_target->GetOrientation());
                        }
                    }
                }
            }
            else
            {
                for (uint8 i = 0; i < INVADERS_COUNT; i++)
                {
                    mob_entry = 0;
                    if (Rand(ELITE_CHANCE))
                        mob_entry = 27597;
                    else
                    {
                        uint32 mobs[2] = { 27598, 27600 };
                        mob_entry = mobs[Rand(1)];
                    }
                    CreatureProto* cp = CreatureProtoStorage.LookupEntry(mob_entry);
                    CreatureInfo* ci = CreatureNameStorage.LookupEntry(mob_entry);
                    Creature* c = NULL;
                    if (cp && ci)
                    {
                        c = _unit->GetMapMgr()->CreateCreature(mob_entry);
                        if (c)
                        {
                            //position is guessed
                            c->Load(cp, -379.101227f, -824.835449f, 60.0f, 0.0f);
                            c->PushToWorld(_unit->GetMapMgr());
                            //path finding would be usefull :)
                            Player* p_target = GetRandomPlayerTarget();
                            if (p_target)
                            {
                                c->GetAIInterface()->SetRun();
                                c->GetAIInterface()->MoveTo(p_target->GetPositionX(), p_target->GetPositionY(), p_target->GetPositionZ(), p_target->GetOrientation());
                            }
                        }
                    }
                }
            }
        }
        void SpawnCrystal(uint32 id)
        {
            uint32 entry = 0;
            float x = 0.0f, y = 0.0f, z = 0.0f, o = 0.0f;
            switch (id)
            {
                case 0:
                {
                    entry = GO_RITUAL_CRYSTAL_ENTRY_1;
                    x = -392.416f;
                    y = -724.865f;
                    z = 29.4156f;
                    o = M_PI_FLOAT;
                }
                break;
                case 1:
                {
                    entry = GO_RITUAL_CRYSTAL_ENTRY_2;
                    x = -365.279f;
                    y = -751.087f;
                    z = 29.4156f;
                    o = M_PI_FLOAT;
                }
                break;
                case 2:
                {
                    entry = GO_RITUAL_CRYSTAL_ENTRY_3;
                    x = -365.41f;
                    y = -724.865f;
                    z = 29.4156f;
                    o = M_PI_FLOAT;
                }
                break;
                case 3:
                {
                    entry = GO_RITUAL_CRYSTAL_ENTRY_4;
                    x = -392.286f;
                    y = -751.087f;
                    z = 29.4156f;
                    o = M_PI_FLOAT;
                }
                break;
            }
            GameObject* go = _unit->GetMapMgr()->CreateGameObject(entry);
            go->CreateFromProto(entry, _unit->GetMapMgr()->GetMapId(), x, y, z, o, 0.0f, 0.0f, 0.0f, 0.0f);
            go->PushToWorld(_unit->GetMapMgr());
            _unit->m_ObjectSlots[id] = go->GetUIdFromGUID();
        }

        void Destroy()
        {
            for (uint32 i = 0; i < spells.size(); ++i)
            {
                if (spells[i] != NULL)
                    delete spells[i];
            };

            spells.clear();

            delete this;
        };

    protected:

        bool heroic;
        vector< ScriptSpell* > spells;
        uint32 invasion_timer;
        uint32 handler_timer;
        uint32 phase;
};


//CrystalHandlerAI
class CrystalHandlerAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(CrystalHandlerAI);
        CrystalHandlerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            heroic = (_unit->GetMapMgr()->iInstanceMode == MODE_HEROIC);
            spells.clear();
            /* SPELLS INIT */
            ScriptSpell* FlashofDarkness = new ScriptSpell;
            FlashofDarkness->normal_spellid = 49668;
            FlashofDarkness->heroic_spellid = 59004;
            FlashofDarkness->chance = 50;
            FlashofDarkness->timer = 4000;
            FlashofDarkness->time = 0;
            FlashofDarkness->target = SPELL_TARGET_CURRENT_ENEMY;
            spells.push_back(FlashofDarkness);
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit*  mKiller)
        {
            RemoveAIUpdateEvent();
            Unit* Novos = _unit->GetMapMgr()->GetUnit(_unit->GetSummonedByGUID());
            if (Novos)
                for (uint8 i = 0; i < 4; i++)
                    if (Novos->m_ObjectSlots[i])
                    {
                        GameObject* Crystal = Novos->GetMapMgr()->GetGameObject(Novos->m_ObjectSlots[i]);
                        if (Crystal && Crystal->IsInWorld())
                        {
                            Crystal->Despawn(0, 0);
                            return;
                        }
                    }
        }

        void AIUpdate()
        {
            if (spells.size() > 0)
            {
                for (uint8 i = 0; i < spells.size(); i++)
                {
                    if (spells[i]->time < getMSTime())
                    {
                        if (Rand(spells[i]->chance))
                        {
                            CastScriptSpell(spells[i]);
                            spells[i]->time = getMSTime() + spells[i]->timer;
                        }
                    }
                }
            }
        }

        Player* GetRandomPlayerTarget()
        {

            vector< uint32 > possible_targets;
            for (set< Object* >::iterator iter = _unit->GetInRangePlayerSetBegin(); iter != _unit->GetInRangePlayerSetEnd(); ++iter)
            {
                if ((*iter) && (TO< Player* >(*iter))->isAlive())
                    possible_targets.push_back((uint32)(*iter)->GetGUID());
            }
            if (possible_targets.size() > 0)
            {
                uint32 random_player = possible_targets[Rand(uint32(possible_targets.size() - 1))];
                return _unit->GetMapMgr()->GetPlayer(random_player);
            }
            return NULL;
        }

        void CastScriptSpell(ScriptSpell* spell)
        {
            _unit->Root();
            uint32 spellid = heroic ? spell->heroic_spellid : spell->normal_spellid;
            Unit* spelltarget = NULL;
            switch (spell->target)
            {
                case SPELL_TARGET_SELF:
                {
                    spelltarget = _unit;
                }
                break;
                case SPELL_TARGET_GENERATE:
                {
                    spelltarget = NULL;
                }
                break;
                case SPELL_TARGET_CURRENT_ENEMY:
                {
                    spelltarget = _unit->GetAIInterface()->getNextTarget();
                }
                break;
                case SPELL_TARGET_RANDOM_PLAYER:
                {
                    spelltarget = GetRandomPlayerTarget();
                }
                break;
            }
            _unit->CastSpell(spelltarget, spellid, false);
            _unit->Unroot();
        }

        void Destroy()
        {
            for (uint32 i = 0; i < spells.size(); ++i)
            {
                if (spells[i] != NULL)
                    delete spells[i];
            };

            spells.clear();

            delete this;
        };

    protected:

        bool heroic;
        vector< ScriptSpell* > spells;
};


// KingDreadAI
// \todo King Dred Call nearby friends
class KingDreadAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(KingDreadAI);
        KingDreadAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            heroic = (_unit->GetMapMgr()->iInstanceMode == MODE_HEROIC);
            spells.clear();
            /* SPELLS INIT */
            ScriptSpell* BellowingRoar = new ScriptSpell;
            BellowingRoar->normal_spellid = 22686;
            BellowingRoar->heroic_spellid = 22686;
            BellowingRoar->chance = 30;
            BellowingRoar->timer = 5000;
            BellowingRoar->time = 0;
            BellowingRoar->target = SPELL_TARGET_CURRENT_ENEMY;
            spells.push_back(BellowingRoar);

            ScriptSpell* GrievousBite = new ScriptSpell;
            GrievousBite->normal_spellid = 48849;
            GrievousBite->heroic_spellid = 59422;
            GrievousBite->chance = 80;
            GrievousBite->timer = 3000;
            GrievousBite->time = 0;
            GrievousBite->target = SPELL_TARGET_CURRENT_ENEMY;
            spells.push_back(GrievousBite);

            ScriptSpell* ManglingSlash = new ScriptSpell;
            ManglingSlash->normal_spellid = 48873;
            ManglingSlash->heroic_spellid = 48873;
            ManglingSlash->chance = 40;
            ManglingSlash->timer = 2000;
            ManglingSlash->time = 0;
            ManglingSlash->target = SPELL_TARGET_CURRENT_ENEMY;
            spells.push_back(ManglingSlash);

            ScriptSpell* PiercingSlash = new ScriptSpell;
            PiercingSlash->normal_spellid = 48878;
            PiercingSlash->heroic_spellid = 48878;
            PiercingSlash->chance = 40;
            PiercingSlash->timer = 5000;
            PiercingSlash->time = 0;
            PiercingSlash->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(PiercingSlash);
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit*  mKiller)
        {
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            if (spells.size() > 0)
            {
                for (uint8 i = 0; i < spells.size(); i++)
                {
                    if (spells[i]->time < getMSTime())
                    {
                        if (Rand(spells[i]->chance))
                        {
                            CastScriptSpell(spells[i]);
                            spells[i]->time = getMSTime() + spells[i]->timer;
                        }
                    }
                }
            }
        }

        Player* GetRandomPlayerTarget()
        {

            vector< uint32 > possible_targets;
            for (set< Object* >::iterator iter = _unit->GetInRangePlayerSetBegin(); iter != _unit->GetInRangePlayerSetEnd(); ++iter)
            {
                if ((*iter) && (TO< Player* >(*iter))->isAlive())
                    possible_targets.push_back((uint32)(*iter)->GetGUID());
            }
            if (possible_targets.size() > 0)
            {
                uint32 random_player = possible_targets[Rand(uint32(possible_targets.size() - 1))];
                return _unit->GetMapMgr()->GetPlayer(random_player);
            }
            return NULL;
        }

        void CastScriptSpell(ScriptSpell* spell)
        {
            _unit->Root();
            uint32 spellid = heroic ? spell->heroic_spellid : spell->normal_spellid;
            Unit* spelltarget = NULL;
            switch (spell->target)
            {
                case SPELL_TARGET_SELF:
                {
                    spelltarget = _unit;
                }
                break;
                case SPELL_TARGET_GENERATE:
                {
                    spelltarget = NULL;
                }
                break;
                case SPELL_TARGET_CURRENT_ENEMY:
                {
                    spelltarget = _unit->GetAIInterface()->getNextTarget();
                }
                break;
                case SPELL_TARGET_RANDOM_PLAYER:
                {
                    spelltarget = GetRandomPlayerTarget();
                }
                break;
            }
            _unit->CastSpell(spelltarget, spellid, false);
            _unit->Unroot();
        }

        void Destroy()
        {
            for (uint32 i = 0; i < spells.size(); ++i)
            {
                if (spells[i] != NULL)
                    delete spells[i];
            };

            spells.clear();

            delete this;
        };

    protected:

        bool heroic;
        vector< ScriptSpell* > spells;
};

/*
 The Prophet Tharon'ja
 \todo
 - Skeleton/Normal phases should be based on boss health instead of time
 - Figure out why players are not always changed to skeletons while chaning phases
 */

// TheProphetTaronjaAI
#define WINDSERPENT_PHASE_INTERVAL 60000//change phase each 60s
#define WINDSERPENT_PHASE_LENGTH 30000//30s
#define PHASES_COUNT 2

class TheProphetTaronjaAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(TheProphetTaronjaAI);
        TheProphetTaronjaAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            heroic = (_unit->GetMapMgr()->iInstanceMode == MODE_HEROIC);
            spells.clear();
            phase_timer = 0;
            phase_length = 0;
            phase = 0;
            /* SPELLS INIT */
            ScriptSpell* CurseOfLife = new ScriptSpell;
            CurseOfLife->normal_spellid = 49527;
            CurseOfLife->heroic_spellid = 59972;
            CurseOfLife->chance = 80;
            CurseOfLife->timer = 7000;
            CurseOfLife->time = 0;
            CurseOfLife->phase = PHASES_COUNT + 1; //all phases
            CurseOfLife->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(CurseOfLife);

            ScriptSpell* LightningBreath = new ScriptSpell;
            LightningBreath->normal_spellid = 49537;
            LightningBreath->heroic_spellid = 59963;
            LightningBreath->chance = 60;
            LightningBreath->timer = 4000;
            LightningBreath->time = 0;
            LightningBreath->phase = PHASES_COUNT + 1; //all phases
            LightningBreath->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(LightningBreath);

            ScriptSpell* PoisonCloud = new ScriptSpell;
            PoisonCloud->normal_spellid = 49548;
            PoisonCloud->heroic_spellid = 59969;
            PoisonCloud->chance = 30;
            PoisonCloud->timer = 6000;
            PoisonCloud->time = 0;
            PoisonCloud->phase = PHASES_COUNT + 1; //all phases
            PoisonCloud->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(PoisonCloud);

            ScriptSpell* RainofFire = new ScriptSpell;
            RainofFire->normal_spellid = 49518;
            RainofFire->heroic_spellid = 59971;
            RainofFire->chance = 70;
            RainofFire->timer = 10000;
            RainofFire->time = 0;
            RainofFire->phase = PHASES_COUNT + 1; //all phases
            RainofFire->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(RainofFire);

            ScriptSpell* ShadowVolley = new ScriptSpell;
            ShadowVolley->normal_spellid = 49528;
            ShadowVolley->heroic_spellid = 59973;
            ShadowVolley->chance = 60;
            ShadowVolley->timer = 5000;
            ShadowVolley->time = 0;
            ShadowVolley->phase = PHASES_COUNT + 1; //all phases
            ShadowVolley->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(ShadowVolley);

            ScriptSpell* EyeBeam = new ScriptSpell;
            EyeBeam->normal_spellid = 49544;
            EyeBeam->heroic_spellid = 59965;
            EyeBeam->chance = 50;
            EyeBeam->timer = 3000;
            EyeBeam->time = 0;
            EyeBeam->phase = 2;
            EyeBeam->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(EyeBeam);
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            phase = 1;
            phase_length = 0;
            phase_timer = getMSTime() + WINDSERPENT_PHASE_INTERVAL;
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
            phase = 1;
            phase_timer = 0;
            phase_length = 0;
            _unit->SetDisplayId(_unit->GetNativeDisplayId());
        }

        void OnDied(Unit*  mKiller)
        {
            RemoveAIUpdateEvent();
        }

        void OnDamageTaken(Unit* mAttacker, uint32 fAmount)
        {
            if (_unit->GetHealthPct() < 2 && phase == 2)
            {
                phase = 1;
                phase_timer = getMSTime() + WINDSERPENT_PHASE_INTERVAL;
                _unit->SetDisplayId(_unit->GetNativeDisplayId());
                _unit->CastSpell(_unit, 53463, false);
            }
        }

        void AIUpdate()
        {
            if (phase == 1 && phase_timer < getMSTime())
            {
                phase = 2;
                phase_length = getMSTime() + WINDSERPENT_PHASE_LENGTH;
                _unit->SetDisplayId(27073);
                _unit->RemoveAllAuras();
                _unit->CastSpell(_unit, 49356, false);
            }
            if (phase == 2 && phase_length < getMSTime())
            {
                phase = 1;
                phase_timer = getMSTime() + WINDSERPENT_PHASE_INTERVAL;
                _unit->SetDisplayId(_unit->GetNativeDisplayId());
                _unit->RemoveAllAuras();
                _unit->CastSpell(_unit, 53463, false);
            }
            if (spells.size() > 0)
            {
                for (uint8 i = 0; i < spells.size(); i++)
                {
                    if (spells[i]->time < getMSTime() && (spells[i]->phase == phase || spells[i]->phase > PHASES_COUNT))
                    {
                        if (Rand(spells[i]->chance))
                        {
                            CastScriptSpell(spells[i]);
                            spells[i]->time = getMSTime() + spells[i]->timer;
                        }
                    }
                }
            }
        }

        Player* GetRandomPlayerTarget()
        {

            vector< uint32 > possible_targets;
            for (set< Object* >::iterator iter = _unit->GetInRangePlayerSetBegin(); iter != _unit->GetInRangePlayerSetEnd(); ++iter)
            {
                if ((*iter) && (TO< Player* >(*iter))->isAlive())
                    possible_targets.push_back((uint32)(*iter)->GetGUID());
            }
            if (possible_targets.size() > 0)
            {
                uint32 random_player = possible_targets[Rand(uint32(possible_targets.size() - 1))];
                return _unit->GetMapMgr()->GetPlayer(random_player);
            }
            return NULL;
        }

        void CastScriptSpell(ScriptSpell* spell)
        {
            _unit->Root();
            uint32 spellid = heroic ? spell->heroic_spellid : spell->normal_spellid;
            Unit* spelltarget = NULL;
            switch (spell->target)
            {
                case SPELL_TARGET_SELF:
                {
                    spelltarget = _unit;
                }
                break;
                case SPELL_TARGET_GENERATE:
                {
                    spelltarget = NULL;
                }
                break;
                case SPELL_TARGET_CURRENT_ENEMY:
                {
                    spelltarget = _unit->GetAIInterface()->getNextTarget();
                }
                break;
                case SPELL_TARGET_RANDOM_PLAYER:
                {
                    spelltarget = GetRandomPlayerTarget();
                }
                break;
            }
            _unit->CastSpell(spelltarget, spellid, false);
            _unit->Unroot();
        }

        void Destroy()
        {
            for (uint32 i = 0; i < spells.size(); ++i)
            {
                if (spells[i] != NULL)
                    delete spells[i];
            };

            spells.clear();

            delete this;
        };

    protected:

        bool heroic;
        vector< ScriptSpell* > spells;
        uint32 phase_timer;
        uint32 phase_length;
        uint32 phase;
};

void SetupDrakTharonKeep(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_DRAK_THARON_KEEP, &InstanceDrakTharonKeepScript::Create);

    //Trash Mobs

    //Bosses
    mgr->register_creature_script(CN_TROLLGORE, &TrollgoreAI::Create);
    mgr->register_creature_script(CN_NOVOS_THE_SUMMONER, &NovosTheSummonerAI::Create);
    mgr->register_creature_script(CN_CRYSTAL_HANDLER, &CrystalHandlerAI::Create);
    mgr->register_creature_script(CN_KING_DRED, &KingDreadAI::Create);
    mgr->register_creature_script(CN_THE_PROPHET_THARONJA, &TheProphetTaronjaAI::Create);
}
