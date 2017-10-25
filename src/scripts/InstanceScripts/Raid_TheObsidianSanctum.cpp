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

#include "Setup.h"
#include "Raid_TheObsidianSanctum.h"

class ObsidianSanctumScript : public MoonInstanceScript
{
    public:
        uint32 m_creatureGuid[OS_DATA_END];

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(ObsidianSanctumScript, MoonInstanceScript);
        ObsidianSanctumScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
        {
            memset(m_creatureGuid, 0, sizeof(m_creatureGuid));
        }

        void OnCreaturePushToWorld(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
                case CN_DRAKE_TENEBRON:
                    m_creatureGuid[DRAKE_TENEBRON] = pCreature->GetLowGUID();
                    break;
                case CN_DRAKE_VESPERON:
                    m_creatureGuid[DRAKE_VESPERON] = pCreature->GetLowGUID();
                    break;
                case CN_DRAKE_SHADRON:
                    m_creatureGuid[DRAKE_SHADRON] = pCreature->GetLowGUID();
                    break;
                case CN_SARTHARION:
                    m_creatureGuid[BOSS_SARTHARION] = pCreature->GetLowGUID();
                    break;
                default:
                    break;
            }
        }

        void OnCreatureDeath(Creature* pVictim, Unit* pKiller)
        {
            switch (pVictim->GetEntry())
            {
                case CN_SARTHARION:
                    m_creatureGuid[BOSS_SARTHARION] = 0;
                    break;
                default:
                    break;
            }
        }

        void DoDrakeAura(uint8 pData)
        {
            uint32 pSpellEntry = 0;
            switch (pData)
            {
                case DRAKE_TENEBRON:
                    pSpellEntry = TENEBRON_AURA;
                    break;
                case DRAKE_SHADRON:
                    pSpellEntry = SHADRON_AURA;
                    break;
                case DRAKE_VESPERON:
                    pSpellEntry = VESPERON_AURA;
                    break;
                default:
                    break;
            }

            Creature* pSartharion = GetCreature(BOSS_SARTHARION);
            if (pSartharion == NULL)
                return;

            pSartharion->CastSpell(pSartharion, pSpellEntry, true);
            pSartharion->RemoveAura(pSpellEntry);   // unproper hackfix
        }

        Creature* GetCreature(uint8 pData)
        {
            if (pData >= OS_DATA_END)   // impossible tho
                return NULL;

            return GetCreatureByGuid(m_creatureGuid[pData]);
        }
};

void SpellFunc_FlameTsunami(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType)
{
    if (pCreatureAI != NULL)
    {
        pCreatureAI->GetUnit()->SendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, "The lava surrounding Sartharion churns!");

        switch (RandomUInt(3))
        {
            case 0:
                pCreatureAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 14100, "Such flammable little insects....");
                break;
            case 1:
                pCreatureAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 14101, "Your charred bones will litter the floor!");
                break;
            case 2:
                pCreatureAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 14102, "How much heat can you take?");
                break;
            case 3:
                pCreatureAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 14103, "All will be reduced to ash!");
                break;
        }

        Creature* Tsunami = NULL;

        for (uint8 i = 0; i < 3; ++i)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    Tsunami = pCreatureAI->GetUnit()->GetMapMgr()->GetInterface()->SpawnCreature(CN_FLAME_TSUNAMI, TSUNAMI_SPAWN[i].x, TSUNAMI_SPAWN[i].y, TSUNAMI_SPAWN[i].z, TSUNAMI_SPAWN[i].o, true, true, 0, 0);

                    if (Tsunami != NULL)
                        Tsunami->GetAIInterface()->MoveTo(TSUNAMI_MOVE[i].x, TSUNAMI_MOVE[i].y, TSUNAMI_MOVE[i].z);
                    break;
                case 1:
                    Tsunami = pCreatureAI->GetUnit()->GetMapMgr()->GetInterface()->SpawnCreature(CN_FLAME_TSUNAMI, TSUNAMI_SPAWN[i + 3].x, TSUNAMI_SPAWN[i + 3].y, TSUNAMI_SPAWN[i + 3].z, TSUNAMI_SPAWN[i + 3].o, true, true, 0, 0);

                    if (Tsunami != NULL)
                        Tsunami->GetAIInterface()->MoveTo(TSUNAMI_MOVE[i + 3].x, TSUNAMI_MOVE[i + 3].y, TSUNAMI_MOVE[i + 3].z);
            }

            Tsunami = NULL;
        }
    }
};

void SpellFunc_LavaSpawn(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType)
{
    if (pCreatureAI == NULL)
        return;

    for (uint8 i = 0; i < 2; ++i)
    {
        uint32 j = RandomUInt(5);
        pCreatureAI->SpawnCreature(CN_LAVA_BLAZE, pTarget->GetPositionX() + j, pTarget->GetPositionY() + j, pTarget->GetPositionZ(), pTarget->GetOrientation(), true);
    };
};

class SartharionAI : public MoonScriptBossAI
{
        MOONSCRIPT_FACTORY_FUNCTION(SartharionAI, MoonScriptBossAI);
        SartharionAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            mInstance = dynamic_cast<ObsidianSanctumScript*>(GetInstanceScript());

            AddSpell(SARTHARION_CLEAVE, Target_Current, 24, 0, 8);

            SpellDesc* mFlame = AddSpell(HeroicInt(56908, 58956), Target_Self, 18, 2, 16);
            mFlame->AddEmote("Burn, you miserable wretches!", CHAT_MSG_MONSTER_YELL, 14098);

            AddSpell(HeroicInt(56910, 58957), Target_Self, 40, 0, 12);
            mFlameTsunami = AddSpellFunc(&SpellFunc_FlameTsunami, Target_Self, 99, 0, 25);
            mSummonLava = AddSpellFunc(&SpellFunc_LavaSpawn, Target_RandomUnitNotCurrent, 25, 0, 8);

            AddEmote(Event_OnTargetDied, "You will make a fine meal for the hatchlings.", CHAT_MSG_MONSTER_YELL, 14094);
            AddEmote(Event_OnTargetDied, "You are at a grave disadvantage!", CHAT_MSG_MONSTER_YELL, 14096);
            AddEmote(Event_OnTargetDied, "This is why we call you lesser beings.", CHAT_MSG_MONSTER_YELL, 14097);
            AddEmote(Event_OnCombatStart, "It is my charge to watch over these eggs. I will see you burn before any harm comes to them!", CHAT_MSG_MONSTER_YELL, 14093);

            for (uint8 i = 0; i < OS_DATA_END - 1; i++)
            {
                m_bDrakes[i] = false;
                m_cDrakes[i] = NULL;
            }

            mDrakeTimer = 0;
            m_bEnraged = false;
            m_iDrakeCount = 0;
        };

        void OnCombatStart(Unit* pTarget)
        {
            m_bEnraged = false;
            mFlameTsunami->TriggerCooldown();
            CheckDrakes();

            if (m_iDrakeCount >= 1)   //HardMode!
            {
                mDrakeTimer = AddTimer(30000);
                ApplyAura(SARTHARION_AURA);
                RemoveAuraOnPlayers(SARTHARION_AURA);   // unproper hackfix
                _regenerateHealth();// Lets heal him as aura increase his hp for 25%
            };

            ParentClass::OnCombatStart(pTarget);
        };

        void AIUpdate()
        {
            if (m_iDrakeCount >= 1)
            {
                if (IsTimerFinished(mDrakeTimer))
                {
                    if (m_bDrakes[DRAKE_TENEBRON] == true)
                        CallTenebron();
                    else if (m_bDrakes[DRAKE_SHADRON] == true)
                        CallShadron();
                    else if (m_bDrakes[DRAKE_VESPERON] == true)
                        CallVesperon();

                    ResetTimer(mDrakeTimer, 45000);
                };
            };

            if (_getHealthPercent() <= 10 && m_bEnraged == false)   // enrage phase
            {
                for (uint8 i = 0; i < 3; ++i)
                    CastSpellNowNoScheduling(mSummonLava);

                m_bEnraged = true;
            };

            ParentClass::AIUpdate();
        };

        void CheckDrakes()
        {
            if (mInstance == NULL)
                return;

            m_iDrakeCount = 0;

            for (uint8 i = 0; i < (OS_DATA_END - 1); ++i)
            {
                m_cDrakes[i] = mInstance->GetCreature(i);
                if (m_cDrakes[i] != NULL && m_cDrakes[i]->isAlive())
                {
                    m_bDrakes[i] = true;
                    m_iDrakeCount++;
                    mInstance->DoDrakeAura(i);
                };
            };
        };

        void CallTenebron()
        {
            if (m_cDrakes[DRAKE_TENEBRON] != NULL && m_cDrakes[DRAKE_TENEBRON]->isAlive())
            {
                sendDBChatMessage(3982);     //Tenebron!The eggs are yours to protect as well!
                m_cDrakes[DRAKE_TENEBRON]->GetAIInterface()->MoveTo(3254.606689f, 531.867859f, 66.898163f);
                m_cDrakes[DRAKE_TENEBRON]->SetOrientation(4.215994f);
            };
            m_bDrakes[DRAKE_TENEBRON] = false;
        };

        void CallShadron()
        {
            if (m_cDrakes[DRAKE_SHADRON] != NULL && m_cDrakes[DRAKE_SHADRON]->isAlive())
            {
                sendDBChatMessage(3981);     //Shadron! Come to me! All is at risk!
                m_cDrakes[DRAKE_SHADRON]->GetAIInterface()->MoveTo(3254.606689f, 531.867859f, 66.898163f);
                m_cDrakes[DRAKE_SHADRON]->SetOrientation(4.215994f);
            };
            m_bDrakes[DRAKE_SHADRON] = false;
        };

        void CallVesperon()
        {
            if (m_cDrakes[DRAKE_VESPERON] != NULL && m_cDrakes[DRAKE_VESPERON]->isAlive())
            {
                sendDBChatMessage(3983);     //Vesperon, the clutch is in danger! Assist me!
                m_cDrakes[DRAKE_VESPERON]->GetAIInterface()->MoveTo(3254.606689f, 531.867859f, 66.898163f);
                m_cDrakes[DRAKE_VESPERON]->SetOrientation(4.215994f);
            };
            m_bDrakes[DRAKE_VESPERON] = false;
        };

        void OnDied(Unit* pKiller)
        {
            sendDBChatMessage(3984);         //Such is the price... of failure...

            RemoveAIUpdateEvent();
            ParentClass::OnDied(pKiller);
        };

    private:
        bool m_bDrakes[OS_DATA_END - 1];
        int32 mDrakeTimer;
        bool m_bEnraged;
        int    m_iDrakeCount;

        ObsidianSanctumScript* mInstance;

        Creature* m_cDrakes[OS_DATA_END - 1]; // exclude boss
        SpellDesc* mFlameTsunami, *mSummonLava;
};

class TsunamiAI : public MoonScriptBossAI
{
        MOONSCRIPT_FACTORY_FUNCTION(TsunamiAI, MoonScriptBossAI);
        TsunamiAI(Creature* pCreature) : MoonScriptBossAI(pCreature) {};

        void OnLoad()
        {
            RegisterAIUpdateEvent(1000);
            setFlyMode(true);
            setCanEnterCombat(false);
            _unit->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            despawn(11500, 0);

            ParentClass::OnLoad();
        };

        void AIUpdate()
        {
            ApplyAura(TSUNAMI);
            ApplyAura(TSUNAMI_VISUAL);
            RegisterAIUpdateEvent(11000);

            ParentClass::OnLoad();
        };

};

class CyclonAI : public MoonScriptBossAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(CyclonAI, MoonScriptBossAI);
        CyclonAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {};

        void OnLoad()
        {
            setRooted(true);
            setCanEnterCombat(false);
            _unit->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            ApplyAura(CYCLON_SPELL);
            ApplyAura(CYCLON_AURA);

            ParentClass::OnLoad();
        };

};

class LavaBlazeAI : public MoonScriptBossAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(LavaBlazeAI, MoonScriptBossAI);
        LavaBlazeAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {};

        void OnLoad()
        {
            AggroNearestPlayer(1);
            ParentClass::OnLoad();
        };

        void OnCombatStop(Unit* pTarget)
        {
            despawn(1000, 0);
        };

        void OnDied(Unit* pKiller)
        {
            despawn(1000, 0);
        };

};

void SetupTheObsidianSanctum(ScriptMgr* mgr)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    ///////// Bosses
    mgr->register_creature_script(CN_SARTHARION, &SartharionAI::Create);
    mgr->register_creature_script(CN_FLAME_TSUNAMI, &TsunamiAI::Create);
    mgr->register_creature_script(CN_CYCLON, &CyclonAI::Create);
    mgr->register_creature_script(CN_LAVA_BLAZE, &LavaBlazeAI::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    ///////// Instance
    mgr->register_instance_script(MAP_OBSIDIAN_SANCTUM, &ObsidianSanctumScript::Create);
};

