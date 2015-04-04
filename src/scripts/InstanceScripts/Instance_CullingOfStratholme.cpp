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
#include "Instance_CullingOfStratholme.h"

//MeathookAA
class MeathookAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(MeathookAI);
        MeathookAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            heroic = (_unit->GetMapMgr()->iInstanceMode == MODE_HEROIC);
            spells.clear();
            /* SPELLS INIT */
            ScriptSpell* ConstrictingChains = new ScriptSpell;
            ConstrictingChains->normal_spellid = 52696;
            ConstrictingChains->heroic_spellid = 58823;
            ConstrictingChains->chance = 100;
            ConstrictingChains->timer = 8000;
            ConstrictingChains->time = 0;
            ConstrictingChains->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(ConstrictingChains);

            ScriptSpell* DiseaseExpulsion = new ScriptSpell;
            DiseaseExpulsion->normal_spellid = 52666;
            DiseaseExpulsion->heroic_spellid = 58824;
            DiseaseExpulsion->chance = 50;
            DiseaseExpulsion->timer = 5000;
            DiseaseExpulsion->time = 0;
            DiseaseExpulsion->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(DiseaseExpulsion);

            ScriptSpell* Frenzy = new ScriptSpell;
            Frenzy->normal_spellid = 58841;
            Frenzy->heroic_spellid = 58841;
            Frenzy->chance = 20;
            Frenzy->timer = 1000;
            Frenzy->time = 0;
            Frenzy->target = SPELL_TARGET_SELF;
            spells.push_back(Frenzy);
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(SAY_MEATHOOK_01);
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
            _unit->RemoveAllAuras();
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(SAY_MEATHOOK_06);
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            switch (rand() % 3)
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_MEATHOOK_02);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_MEATHOOK_03);
                    break;
                case 2:
                    _unit->SendScriptTextChatMessage(SAY_MEATHOOK_04);
                    break;
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
                Player* p = TO< Player* >(*iter);
                if (p->isAlive())
                    possible_targets.push_back(p->GetLowGUID());
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

//SalramTheFleshcrafterAI
class SalramTheFleshcrafterAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(SalramTheFleshcrafterAI);

        SalramTheFleshcrafterAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            heroic = (_unit->GetMapMgr()->iInstanceMode == MODE_HEROIC);
            spells.clear();
            /* SPELLS INIT */
            ScriptSpell* ShadowBolt = new ScriptSpell;
            ShadowBolt->normal_spellid = 57725;
            ShadowBolt->heroic_spellid = 58827;
            ShadowBolt->chance = 50;
            ShadowBolt->timer = 4000;
            ShadowBolt->time = 0;
            ShadowBolt->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(ShadowBolt);

            ScriptSpell* CurseOfTwistedFlesh = new ScriptSpell;
            CurseOfTwistedFlesh->normal_spellid = 0;
            CurseOfTwistedFlesh->heroic_spellid = 58845;
            CurseOfTwistedFlesh->chance = 50;
            CurseOfTwistedFlesh->timer = 4000;
            CurseOfTwistedFlesh->time = 0;
            CurseOfTwistedFlesh->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(CurseOfTwistedFlesh);

            ScriptSpell* StealFlesh = new ScriptSpell;
            StealFlesh->normal_spellid = 52708;
            StealFlesh->heroic_spellid = 52708;
            StealFlesh->chance = 30;
            StealFlesh->timer = 5000;
            StealFlesh->time = 0;
            StealFlesh->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(StealFlesh);

            ScriptSpell* SummonGhouls = new ScriptSpell;
            SummonGhouls->normal_spellid = 52451;
            SummonGhouls->heroic_spellid = 52451;
            SummonGhouls->chance = 50;
            SummonGhouls->timer = 4000;
            SummonGhouls->time = 0;
            SummonGhouls->target = SPELL_TARGET_SELF;
            spells.push_back(SummonGhouls);

            ScriptSpell* ExplodeGhoul = new ScriptSpell;
            ExplodeGhoul->normal_spellid = 52480;
            ExplodeGhoul->heroic_spellid = 58825;
            ExplodeGhoul->chance = 80;
            ExplodeGhoul->timer = 6000;
            ExplodeGhoul->time = 0;
            ExplodeGhoul->target = SPELL_TARGET_CUSTOM;
            spells.push_back(ExplodeGhoul);
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(SAY_SALRAM_FLESH_01);
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
            _unit->RemoveAllAuras();
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(SAY_SALRAM_FLESH_06);
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            switch (rand() % 3)
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_SALRAM_FLESH_03);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_SALRAM_FLESH_04);
                    break;
                case 2:
                    _unit->SendScriptTextChatMessage(SAY_SALRAM_FLESH_05);
                    break;
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
                Player* p = TO< Player* >(*iter);

                if (p->isAlive())
                    possible_targets.push_back(p->GetLowGUID());
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
            if (spellid == 0)
                return;
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
                case SPELL_TARGET_CUSTOM:
                {
                    spelltarget = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), 27733);
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

//ChronoLordEpochAI
class ChronoLordEpochAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ChronoLordEpochAI);
        ChronoLordEpochAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            heroic = (_unit->GetMapMgr()->iInstanceMode == MODE_HEROIC);
            spells.clear();
            /* SPELLS INIT */
            ScriptSpell* WoundingStrike = new ScriptSpell;
            WoundingStrike->normal_spellid = 52771;
            WoundingStrike->heroic_spellid = 58830;
            WoundingStrike->chance = 50;
            WoundingStrike->timer = 3000;
            WoundingStrike->time = 0;
            WoundingStrike->target = SPELL_TARGET_CURRENT_ENEMY;
            spells.push_back(WoundingStrike);

            ScriptSpell* CurseOfExertion = new ScriptSpell;
            CurseOfExertion->normal_spellid = 52772;
            CurseOfExertion->heroic_spellid = 52772;
            CurseOfExertion->chance = 100;
            CurseOfExertion->timer = 5000;
            CurseOfExertion->time = 0;
            CurseOfExertion->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(CurseOfExertion);

            ScriptSpell* TimeWarp = new ScriptSpell;
            TimeWarp->normal_spellid = 52766;
            TimeWarp->heroic_spellid = 52766;
            TimeWarp->chance = 50;
            TimeWarp->timer = 6000;
            TimeWarp->time = 0;
            TimeWarp->target = SPELL_TARGET_CURRENT_ENEMY;
            spells.push_back(TimeWarp);

            ScriptSpell* TimeStop = new ScriptSpell;
            TimeStop->normal_spellid = 0;
            TimeStop->heroic_spellid = 58848;
            TimeStop->chance = 20;
            TimeStop->timer = 2000;
            TimeStop->time = 0;
            TimeStop->target = SPELL_TARGET_CURRENT_ENEMY;
            spells.push_back(TimeStop);
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(SAY_CHRONOLORD_EPOCH_02);
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
            _unit->RemoveAllAuras();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            switch (rand() % 3)
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_CHRONOLORD_EPOCH_06);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_CHRONOLORD_EPOCH_07);
                    break;
                case 2:
                    _unit->SendScriptTextChatMessage(SAY_CHRONOLORD_EPOCH_08);
                    break;
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
                Player* p = TO< Player* >(*iter);
                if (p->isAlive())
                    possible_targets.push_back(p->GetLowGUID());
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
            if (spellid == 0)
                return;
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

//InfiniteCorruptorAI
class InfiniteCorruptorAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(InfiniteCorruptorAI);
        InfiniteCorruptorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            heroic = (_unit->GetMapMgr()->iInstanceMode == MODE_HEROIC);
            spells.clear();
            ScriptSpell* VoidStrike = new ScriptSpell;
            VoidStrike->normal_spellid = 60590;
            VoidStrike->heroic_spellid = 60590;
            VoidStrike->chance = 50;
            VoidStrike->timer = 1000;
            VoidStrike->time = 0;
            VoidStrike->target = SPELL_TARGET_CURRENT_ENEMY;
            spells.push_back(VoidStrike);

            ScriptSpell* CorruptingBlight = new ScriptSpell;
            CorruptingBlight->normal_spellid = 60588;
            CorruptingBlight->heroic_spellid = 60588;
            CorruptingBlight->chance = 50;
            CorruptingBlight->timer = 3000;
            CorruptingBlight->time = 0;
            CorruptingBlight->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(CorruptingBlight);
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(SAY_INFINITE_CORRUP_01);
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
            _unit->RemoveAllAuras();
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(SAY_INFINITE_CORRUP_02);
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
                Player* p = TO< Player* >(*iter);
                if (p->isAlive())
                    possible_targets.push_back(p->GetLowGUID());
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
            if (spellid == 0)
                return;
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

//MalganisAI
class MalganisAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(MalganisAI);
        MalganisAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            heroic = (_unit->GetMapMgr()->iInstanceMode == MODE_HEROIC);
            scene = true;
            spells.clear();
            ScriptSpell* CarrionSwarm = new ScriptSpell;
            CarrionSwarm->normal_spellid = 52720;
            CarrionSwarm->heroic_spellid = 58852;
            CarrionSwarm->chance = 60;
            CarrionSwarm->timer = 10000;
            CarrionSwarm->time = 0;
            CarrionSwarm->target = SPELL_TARGET_CURRENT_ENEMY;
            spells.push_back(CarrionSwarm);

            ScriptSpell* MindBlast = new ScriptSpell;
            MindBlast->normal_spellid = 52722;
            MindBlast->heroic_spellid = 58850;
            MindBlast->chance = 50;
            MindBlast->timer = 5000;
            MindBlast->time = 0;
            MindBlast->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(MindBlast);

            ScriptSpell* Sleep = new ScriptSpell;
            Sleep->normal_spellid = 52721;
            Sleep->heroic_spellid = 58849;
            Sleep->chance = 40;
            Sleep->timer = 7000;
            Sleep->time = 0;
            Sleep->target = SPELL_TARGET_RANDOM_PLAYER;
            spells.push_back(Sleep);

            ScriptSpell* VampiricTouch = new ScriptSpell;
            VampiricTouch->normal_spellid = 52723;
            VampiricTouch->heroic_spellid = 52723;
            VampiricTouch->chance = 90;
            VampiricTouch->timer = 30000;
            VampiricTouch->time = 0;
            VampiricTouch->target = SPELL_TARGET_SELF;
            spells.push_back(VampiricTouch);
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(SAY_MALGANIS_03);
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
            _unit->RemoveAllAuras();
        }

        void OnTargetDied(Unit* mTarget)
        {
            switch (rand() % 3)
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_MALGANIS_04);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_MALGANIS_05);
                    break;
                case 2:
                    _unit->SendScriptTextChatMessage(SAY_MALGANIS_06);
                    break;
            }
        }

        void OnDamageTaken(Unit* mAttacker, uint32 fAmount)
        {
            if (_unit->HasAura(52723))    //handling a dummy :)
            {
                _unit->Heal(_unit, 52723, fAmount / 2);
            }
            if (_unit->GetHealthPct() < 2)
            {
                //lala
                _unit->Root();
                _unit->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                for (uint8 i = 0; i < 7; i++)
                    _unit->SchoolImmunityList[i] = 1;
                RemoveAIUpdateEvent();
                _unit->SendScriptTextChatMessage(SAY_MALGANIS_17);
                //spawn a chest and go
                GameObject* go = _unit->GetMapMgr()->CreateGameObject(190663);
                go->CreateFromProto(190663, _unit->GetMapMgr()->GetMapId(), _unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), _unit->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f);
                go->PushToWorld(_unit->GetMapMgr());
                _unit->Despawn(1, 0);
            }
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(SAY_MALGANIS_16);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            if (!scene && spells.size() > 0)
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
            else
            {
                //this is ugly, better ideas?
                scene = false;
                Creature* citizen = NULL;
                uint32 entry = 0;
                for (uint32 i = 0; i != _unit->GetMapMgr()->m_CreatureHighGuid; ++i)
                {
                    if (_unit->GetMapMgr()->CreatureStorage[i] != NULL)
                    {
                        entry = _unit->GetMapMgr()->CreatureStorage[i]->GetEntry();
                        if (entry == 31126 || entry == 31127 || entry == 28167 || entry == 28169)
                        {
                            citizen = _unit->GetMapMgr()->CreatureStorage[i];
                            CreatureProto* cp = CreatureProtoStorage.LookupEntry(27737);//risen zombie
                            CreatureInfo* ci = CreatureNameStorage.LookupEntry(27737);
                            Creature* c = NULL;
                            if (cp && ci)
                            {
                                c = _unit->GetMapMgr()->CreateCreature(27737);
                                if (c)
                                {
                                    //position is guessed
                                    c->Load(cp, citizen->GetPositionX(), citizen->GetPositionY(), citizen->GetPositionZ(), citizen->GetOrientation());
                                    c->PushToWorld(_unit->GetMapMgr());
                                }
                            }
                            citizen->Despawn(0, 0);
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
                Player* p = TO< Player* >(*iter);
                if (p->isAlive())
                    possible_targets.push_back(p->GetLowGUID());
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
            if (spellid == 0)
                return;
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
        bool scene;
        vector< ScriptSpell* > spells;
};


class Quest_Dispelling_Illusions : public QuestScript
{
    public:

        void OnQuestStart(Player* mTarget, QuestLogEntry* qLogEntry)
        {
            for (uint8 i = 0; i < 5; i++)
                SpawnCrates(i, mTarget->GetMapMgr());
        }
        void SpawnCrates(uint32 id, MapMgr* pMapMgr)
        {
            uint32 entry = 190094;
            float x = 0.0f, y = 0.0f, z = 0.0f, o = 0.0f;
            switch (id)
            {
                case 0:
                {
                    x = 1570.92f;
                    y = 669.933f;
                    z = 102.309f;
                    o = -1.64061f;
                }
                break;
                case 1:
                {
                    x = 1579.42f;
                    y = 621.446f;
                    z = 99.7329f;
                    o = 2.9845f;
                }
                break;
                case 2:
                {
                    x = 1629.68f;
                    y = 731.367f;
                    z = 112.847f;
                    o = -0.837757f;
                }
                break;
                case 3:
                {
                    x = 1674.39f;
                    y = 872.307f;
                    z = 120.394f;
                    o = -1.11701f;
                }
                break;
                case 4:
                {
                    x = 1628.98f;
                    y = 812.142f;
                    z = 120.689f;
                    o = 0.436332f;
                }
                break;
            }
            GameObject* crate = pMapMgr->GetInterface()->GetGameObjectNearestCoords(x, y, z, 190094);
            if (crate)
                crate->Despawn(0, 0);
            GameObject* go = pMapMgr->CreateGameObject(entry);
            go->CreateFromProto(entry, pMapMgr->GetMapId(), x, y, z, o, 0.0f, 0.0f, 0.0f, 0.0f);
            go->PushToWorld(pMapMgr);
        }
};


static Location walk[] =
{
    { 0, 0, 0, 0 },
    { 1811.2177f, 1276.5729f, 141.9048f, 0.098f },
    { 1884.9484f, 1284.9110f, 143.7776f, 6.2810f },
    { 1897.4763f, 1291.1870f, 143.5821f, 1.4194f }
};

//UtherAI
class UtherAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(UtherAI);
        UtherAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(1, 0, RUN));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(2, 0, RUN));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(3, 90000, RUN));
            check = true;
        }

        void OnReachWP(uint32 i, bool usl)
        {
            if (i == 3 && check)
            {
                check = false;
                Creature* Arthas = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), CN_ARTHAS);
                Creature* Jaina = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), CN_JAINA);
                if (Arthas && Jaina)  //Show must go on!
                {
                    //we add 0,5s per speech
                    //1 = 0s
                    Arthas->SendScriptTextChatMessage(SAY_ARTHAS_01);
                    //2 = 2,5s
                    _unit->SendTimedScriptTextChatMessage(SAY_UTHER_01, 2500);
                    //3 = 9s
                    Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_02, 9000);
                    //4 = 14,5s
                    Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_03, 14500);
                    //5 = 25s
                    _unit->SendTimedScriptTextChatMessage(SAY_UTHER_02, 25000);
                    //6 = 26,5s
                    Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_04, 26500);
                    //7 = 29s
                    _unit->SendTimedScriptTextChatMessage(SAY_UTHER_03, 29000);
                    //8 = 33,5s
                    Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_05, 33500);
                    //9 = 38s
                    _unit->SendTimedScriptTextChatMessage(SAY_UTHER_04, 38000);
                    //10 = 44,5s
                    Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_06, 44500);
                    //11 = 49s
                    _unit->SendTimedScriptTextChatMessage(SAY_UTHER_05, 49000);
                    //12 = 53,5s
                    Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_07, 53500);
                    //13 = 65s
                    Jaina->SendTimedScriptTextChatMessage(SAY_JAINA_01, 65000);
                    //14 = 67,5s
                    Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_08, 67500);
                    //15 = 77s
                    //here few knights should leave, after speech, Uther should leave also
                    _unit->SendTimedScriptTextChatMessage(SAY_UTHER_06, 77000);
                    //16 = 80,5s
                    //Jaina begins leaving
                    Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_09, 80500);
                    //17 = 82s
                    Jaina->SendTimedScriptTextChatMessage(SAY_JAINA_02, 82000);
                    //trigger Arthas actions = 86,5s
                    sEventMgr.AddEvent(Arthas, &Creature::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, 86500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                    _unit->Despawn(100000, 0);
                }
            }
        }

        inline WayPoint* CreateWaypoint(int id, uint32 waittime, uint32 flags)
        {
            WayPoint* wp = _unit->CreateWaypointStruct();
            wp->id = id;
            wp->x = walk[id].x;
            wp->y = walk[id].y;
            wp->z = walk[id].z;
            wp->o = walk[id].o;
            wp->waittime = waittime;
            wp->flags = flags;
            wp->forwardemoteoneshot = false;
            wp->forwardemoteid = 0;
            wp->backwardemoteoneshot = false;
            wp->backwardemoteid = 0;
            wp->forwardskinid = 0;
            wp->backwardskinid = 0;
            return wp;
        }

    private:

        bool check;
};

static Location ArthasWalk[] =
{
    { 0, 0, 0, 0 },
    { 1908.9722f, 1312.8898f, 149.9889f, 0.6858f },
    { 1884.9483f, 1284.9110f, 143.7776f, -2.2802f },
    { 1991.4326f, 1286.5925f, 145.4636f, 1.2591f },
    { 2007.2526f, 1327.5848f, 142.9831f, 5.5553f },
    { 2024.4555f, 1308.2036f, 143.4564f, 4.8492f },
    { 2028.9012f, 1285.9205f, 143.6552f, 0.0661f },
    { 2078.9479f, 1287.9812f, 141.4830f, 0.0308f }
};


//ArthasAI
class ArthasAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ArthasAI);
        ArthasAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(1, 10500, RUN));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(2, 0, RUN));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(3, 0, RUN));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(4, 0, RUN));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(5, 0, RUN));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(6, 0, RUN));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(7, 0, RUN));

            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_SCRIPTIDLE);
            _unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_DONTMOVEWP);
            phase = 0;
        }

        void OnReachWP(uint32 i, bool usl)
        {
            switch (i)
            {
                case 1:
                {
                    _unit->SendScriptTextChatMessage(SAY_ARTHAS_10);
                    _unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_WANTEDWP);
                    _unit->GetAIInterface()->setWaypointToMove(2);
                }
                break;
                case 7:
                {
                    _unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_DONTMOVEWP);
                    _unit->GetAIInterface()->SetAIState(STATE_SCRIPTIDLE);
                    _unit->GetAIInterface()->m_canMove = false;
                    _unit->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                }
                break;
                case 1000://haxxed ;)
                {
                    _unit->SendScriptTextChatMessage(SAY_ARTHAS_11);
                    phase++;
                    sEventMgr.AddEvent(_unit, &Creature::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, 12500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                }
                break;
                default:
                {
                    if (i > 1 && i < 7)
                    {
                        _unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_WANTEDWP);
                        _unit->GetAIInterface()->setWaypointToMove(i + 1);
                    }
                }
                break;
            }
        }

        void AIUpdate()
        {
            switch (phase)
            {
                case 0:
                {
                    _unit->GetAIInterface()->StopMovement(0);
                    _unit->GetAIInterface()->SetAIState(STATE_SCRIPTMOVE);
                    _unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_WANTEDWP);
                    _unit->GetAIInterface()->setWaypointToMove(1);
                }
                break;
                case 1:
                {
                    _unit->SendTimedScriptTextChatMessage(SAY_ARTHAS_12, 300);
                    Creature* citizen = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), 28167);
                    if (citizen)
                    {
                        _unit->GetAIInterface()->MoveTo(citizen->GetPositionX(), citizen->GetPositionY(), citizen->GetPositionZ(), citizen->GetOrientation());
                        _unit->DealDamage(citizen, citizen->GetUInt32Value(UNIT_FIELD_HEALTH), 0, 0, 0);
                    }
                    citizen = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), 28169);
                    if (citizen)
                    {
                        _unit->GetAIInterface()->MoveTo(citizen->GetPositionX(), citizen->GetPositionY(), citizen->GetPositionZ(), citizen->GetOrientation());
                        _unit->DealDamage(citizen, citizen->GetUInt32Value(UNIT_FIELD_HEALTH), 0, 0, 0);
                    }
                    _unit->SendTimedScriptTextChatMessage(SAY_ARTHAS_13, 1000);
                    phase++;
                    sEventMgr.AddEvent(_unit, &Creature::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, 1500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                }
                break;
                case 2:
                {
                    //we need that tricky animation here
                    //spawn Mal'Ganis
                    CreatureProto* cp = CreatureProtoStorage.LookupEntry(26533);
                    CreatureInfo* ci = CreatureNameStorage.LookupEntry(26533);
                    Creature* c = NULL;
                    if (cp && ci)
                    {
                        c = _unit->GetMapMgr()->CreateCreature(26533);
                        if (c)
                        {
                            //position is guessed
                            c->Load(cp, 2113.52f, 1288.01f, 136.382f, 2.30383f);
                            c->PushToWorld(_unit->GetMapMgr());
                        }
                    }
                    if (c)
                    {
                        c->bInvincible = true;
                        c->GetAIInterface()->m_canMove = false;
                        c->GetAIInterface()->SetAllowedToEnterCombat(false);
                        for (uint8 i = 0; i < 7; i++)
                            c->SchoolImmunityList[i] = 1;
                        c->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        //1 = 0s
                        c->SendScriptTextChatMessage(SAY_MALGANIS_01);
                        //2 = 13s
                        //change all citizens to undeads...
                        sEventMgr.AddEvent(c, &Creature::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, 13000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                        c->SendTimedScriptTextChatMessage(SAY_MALGANIS_02, 13000);
                        //2 = 32s
                        _unit->SendTimedScriptTextChatMessage(SAY_ARTHAS_14, 32000);
                        c->Despawn(38500, 0);
                        //3 = 37s
                        /*sEventMgr.AddEvent(TO_UNIT(_unit), &Unit::SendChatMessage, (uint8)CHAT_MSG_MONSTER_SAY, (uint32)LANG_UNIVERSAL,
                        "Mal'Ganis will send out some of his blablabla - write that txt",
                        EVENT_UNIT_CHAT_MSG, 37000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);*/
                        sEventMgr.AddEvent(TO_OBJECT(_unit), &Object::PlaySoundToSet, (uint32)14885, EVENT_UNK, 39000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                    }
                }
                break;
            }
        }

        inline WayPoint* CreateWaypoint(int id, uint32 waittime, uint32 flags)
        {
            WayPoint* wp = _unit->CreateWaypointStruct();
            wp->id = id;
            wp->x = ArthasWalk[id].x;
            wp->y = ArthasWalk[id].y;
            wp->z = ArthasWalk[id].z;
            wp->o = ArthasWalk[id].o;
            wp->waittime = waittime;
            wp->flags = flags;
            wp->forwardemoteoneshot = false;
            wp->forwardemoteid = 0;
            wp->backwardemoteoneshot = false;
            wp->backwardemoteid = 0;
            wp->forwardskinid = 0;
            wp->backwardskinid = 0;
            return wp;
        }

    protected:

        uint32 phase;
};


//ArthasGossip
class ArthasGossip : public GossipScript
{
    public:

        void GossipHello(Object* pObject, Player* Plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 1, Plr);

            Menu->AddItem(0, "We're ready to go!", 1);  //find correct txt

            Menu->SendTo(Plr);
        }

        void GossipSelectOption(Object* pObject, Player*  Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            switch (IntId)
            {
                case 0:
                {
                    GossipHello(pObject, Plr);
                }
                break;
                case 1:
                {
                    TO_CREATURE(pObject)->SetUInt32Value(UNIT_NPC_FLAGS, 0);
                    TO_CREATURE(pObject)->GetScript()->OnReachWP(1000, 0);
                }
                break;
            }
        }
};

// \todo check the unregistered scripts (broeken? outdated?)
void SetupCullingOfStratholme(ScriptMgr* mgr)
{
    //////////////////////////////////////////
    // TRASH MOBS
    //////////////////////////////////////////

    //////////////////////////////////////////
    // BOSSES
    //////////////////////////////////////////
    //mgr->register_creature_script(CN_MEATHOOK, &MeathookAI::Create);
    //mgr->register_creature_script(CN_SALRAMM_THE_FLESHCRAFTER, &SalramTheFleshcrafterAI::Create);
    //mgr->register_creature_script(CN_CHRONO_LORD_EPOCH, &ChronoLordEpochAI::Create);
    //mgr->register_creature_script(CN_INFINITE_CORRUPTOR, &InfiniteCorruptorAI::Create);
    //mgr->register_creature_script(CN_MALGANIS, &MalganisAI::Create);
    //////////////////////////////////////////
    // QUESTS & STUFF
    //////////////////////////////////////////
    //UPDATE `quests` SET `ReqKillMobOrGOCount1`='5' WHERE (`entry`='13149');
    QuestScript* Dispelling_Illusions = new Quest_Dispelling_Illusions();
    mgr->register_quest_script(13149, Dispelling_Illusions);
    //mgr->register_creature_script(CN_UTHER, &UtherAI::Create);
    //mgr->register_creature_script(CN_ARTHAS, &ArthasAI::Create);
    //GossipScript * AG = new ArthasGossip();
    //mgr->register_gossip_script(CN_ARTHAS, AG);
}
