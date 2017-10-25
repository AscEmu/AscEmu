/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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

// \todo Last boss needs to be finished
#include "Setup.h"
#include "Instance_Nexus.h"

#define GO_FLAG_UNCLICKABLE 0x00000010

//////////////////////////////////////////////////////////////////////////////////////////
///Anomalus
class AnomalusAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(AnomalusAI, MoonScriptBossAI);
        AnomalusAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            mInstance = GetInstanceScript();

            if (IsHeroic())
                AddSpell(SPARK_HC, Target_RandomPlayer, 80, 0, 3);
            else
                AddSpell(SPARK, Target_RandomPlayer, 80, 0, 3);

            mSummon = 0;
            mRift = false;
            mSummonTimer = 0;

        };

        void OnCombatStart(Unit* mTarget)
        {
            sendDBChatMessage(4317);     // Chaos beckons.
            mSummon = 0;
            mRift = false;
            mSummonTimer = AddTimer(IsHeroic() ? 14000 : 18000);   // check heroic

            ParentClass::OnCombatStart(mTarget);

            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, NEXUS_ANOMALUS, State_InProgress);
        };

        void AIUpdate()
        {
            if ((_getHealthPercent() <= 50 && mSummon == 0))
                mSummon += 1;

            if (mSummon == 1)
                ChargeRift();

            if (IsTimerFinished(mSummonTimer) && mRift == false)
            {
                SummonRift(false);
                ResetTimer(mSummonTimer, IsHeroic() ? 14000 : 18000);
            };

            if (mRift == true && (GetLinkedCreature() == NULL || !GetLinkedCreature()->isAlive()))
            {
                RemoveAura(47748);
                mRift = false;
            };

            ParentClass::AIUpdate();
        };

        void SummonRift(bool bToCharge)
        {
            if (!bToCharge)
                sendDBChatMessage(4319);     // Reality... unwoven.

            Announce("Anomalus opens a Chaotic Rift!");
            //we are linked with CN_CHAOTIC_RIFT.
            CreatureAIScript* chaoticRift = SpawnCreature(CN_CHAOTIC_RIFT, _unit->GetPositionX() + 13.5f, _unit->GetPositionY(), _unit->GetPositionZ(), _unit->GetOrientation(), false);
            if (chaoticRift != NULL)
            {
                SetLinkedCreature(chaoticRift);
                chaoticRift->SetLinkedCreature(this);
            }
        };

        void ChargeRift()
        {
            SummonRift(true);
            sendDBChatMessage(4320);     // Indestructible.
            Announce("Anomalus shields himself and diverts his power to the rifts!");
            ApplyAura(47748);   // me immune
            setRooted(true);

            mRift = true;
            mSummon += 1;
        };

        void OnDied(Unit* pKiller)
        {
            sendDBChatMessage(4318);     // Of course.

            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, NEXUS_ANOMALUS, State_Finished);

            ParentClass::OnDied(pKiller);
        };

        void OnCombatStop(Unit* pTarget)
        {
            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, NEXUS_ANOMALUS, State_Performed);

            ParentClass::OnCombatStop(pTarget);
        };

    private:

        int32 mSummonTimer;
        uint8 mSummon;
        bool mRift;
        MoonInstanceScript* mInstance;
};

class ChaoticRiftAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(ChaoticRiftAI, MoonScriptBossAI);
        ChaoticRiftAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
            auto spell_mana_wrath = sSpellCustomizations.GetSpellInfo(SUMMON_MANA_WRAITH);
            if (spell_mana_wrath != nullptr)
                AddSpell(SUMMON_MANA_WRAITH, Target_Self, 30, 0, spell_mana_wrath->getRecoveryTime());

            auto spell_energy_burst = sSpellCustomizations.GetSpellInfo(CHAOTIC_ENERGY_BURST);
            if (spell_energy_burst != nullptr)
                AddSpell(CHAOTIC_ENERGY_BURST, Target_RandomPlayer, 30, 0, spell_energy_burst->getRecoveryTime());
        };

        void OnLoad()
        {
            ApplyAura(CHAOTIC_RIFT_AURA);
            despawn(40000, 0);
            ParentClass::OnLoad();
        };

        void OnDied(Unit* mKiller)
        {
            despawn(2000, 0);
            ParentClass::OnDied(mKiller);
        };

        void OnCombatStop(Unit* pTarget)
        {
            despawn(2000, 0);
            ParentClass::OnCombatStop(pTarget);
        };
};

class CraziedManaWrathAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(CraziedManaWrathAI, MoonScriptBossAI);
        CraziedManaWrathAI(Creature* pCreature) : MoonScriptBossAI(pCreature) {};

        void OnCombatStop(Unit* pTarget)
        {
            despawn(2000, 0);
            ParentClass::OnCombatStop(pTarget);
        };

        void OnDied(Unit* mKiller)
        {
            despawn(2000, 0);
            ParentClass::OnDied(mKiller);
        };
};


//////////////////////////////////////////////////////////////////////////////////////////
/// Grand Magus Telestra
static Movement::Location FormSpawns[] =
{
    { 494.726990f, 89.128799f, -15.941300f, 6.021390f },
    { 495.006012f, 89.328102f, -16.124609f, 0.027486f },
    { 504.798431f, 102.248375f, -16.124609f, 4.629921f }
};


class TelestraBossAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(TelestraBossAI, MoonScriptBossAI);
        TelestraBossAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            mInstance = GetInstanceScript();

            mHeroic = IsHeroic();
            if (mHeroic)
            {
                AddSpell(ICE_NOVA_HC, Target_Self, 25, 2.0, 15);
                AddSpell(FIREBOMB_HC, Target_RandomPlayer, 35, 1.5, 5);
                AddSpell(GRAVITY_WELL, Target_Self, 15, 0.5, 20);
            }
            else
            {
                AddSpell(ICE_NOVA, Target_Self, 25, 2.0, 15);
                AddSpell(FIREBOMB, Target_RandomPlayer, 35, 1.5, 5);
                AddSpell(GRAVITY_WELL, Target_Self, 15, 0.5, 20);
            };

            SetAIUpdateFreq(1000);

            mAddCount = 0;
            mPhaseRepeat = 2;

            mAddArray[0] = mAddArray[1] = mAddArray[2] = NULL;
        }

        void AIUpdate()
        {
            if (GetPhase() == 1 && _getHealthPercent() <= (mPhaseRepeat * 25))
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        sendDBChatMessage(4330);      // There's plenty of me to go around.
                        break;
                    case 1:
                        sendDBChatMessage(4331);      // I'll give you more than you can handle!
                        break;
                }

                SetPhase(2);
                setRooted(true);
                _setRangedDisabled(true);
                _setCastDisabled(true);
                _setTargetingDisabled(true);
                ApplyAura(60191);

                for (uint8 i = 0; i < 3; ++i)
                {
                    mAddArray[i] = _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_TELESTRA_FIRE + i, FormSpawns[i].x, FormSpawns[i].y, FormSpawns[i].z, FormSpawns[i].o, true, true, 0, 0);
                    if (mAddArray[i] != NULL)
                        ++mAddCount;
                }

            }

            if (GetPhase() == 2)
            {
                for (uint8 i = 0; i < 3; ++i)
                {
                    if (mAddArray[i] != NULL)
                    {
                        mAddArray[i]->Despawn(1000, 0);
                        mAddArray[i] = NULL;
                        --mAddCount;
                    }
                }

                if (mAddCount != 0)
                    return;

                sendChatMessage(CHAT_MSG_MONSTER_YELL, 13323, "Now to finish the job!");

                RemoveAura(60191);
                setRooted(false);
                mPhaseRepeat = 1;
                SetPhase(mHeroic ? 1 : 3);   //3 disables p2
            };

            ParentClass::AIUpdate();
        }

        void OnCombatStart(Unit* pTarget)
        {
            sendDBChatMessage(4326);      // You know what they say about curiosity....

            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, NEXUS_TELESTRA, State_InProgress);

            ParentClass::OnCombatStart(pTarget);
        }

        void OnTargetDied(Unit* pTarget)
        {
            sendDBChatMessage(4327);      // Death becomes you.
        }

        void OnCombatStop(Unit* pTarget)
        {
            for (uint8 i = 0; i < 3; ++i)
            {
                if (mAddArray[i] != NULL)
                {
                    mAddArray[i]->Despawn(1000, 0);
                    mAddArray[i] = NULL;
                }
            }

            ParentClass::OnCombatStop(pTarget);

            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, NEXUS_TELESTRA, State_Performed);
        }

        void OnDied(Unit* pKiller)
        {
            sendDBChatMessage(4328);      // Damn the... luck.

            for (uint8 i = 0; i < 3; ++i)
            {
                if (mAddArray[i] != NULL)
                {
                    mAddArray[i]->Despawn(1000, 0);
                    mAddArray[i] = NULL;
                }
            }

            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, NEXUS_TELESTRA, State_Finished);

            ParentClass::OnDied(pKiller);
        }

    private:

        Creature* mAddArray[3];
        bool mHeroic;

        int32 mPhaseRepeat;
        int32 mAddCount;

        MoonInstanceScript* mInstance;
};

class TelestraFireAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(TelestraFireAI, MoonScriptBossAI);
        TelestraFireAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            if (IsHeroic())
            {
                AddSpell(FIRE_BLAST_HC, Target_RandomPlayer, 30, 0, 14);
                AddSpell(SCORCH_HC, Target_Current, 100, 1, 3);
            }
            else
            {
                AddSpell(FIRE_BLAST, Target_RandomPlayer, 30, 0, 14);
                AddSpell(SCORCH, Target_Current, 100, 1, 3);
            };
        };

        void OnLoad()
        {
            AggroNearestUnit();
            ParentClass::OnLoad();
        };
};

class TelestraFrostAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(TelestraFrostAI, MoonScriptBossAI);
        TelestraFrostAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            if (IsHeroic())
            {
                AddSpell(BLIZZARD_HC, Target_RandomPlayerDestination, 20, 0, 20);
                AddSpell(ICE_BARB_HC, Target_RandomPlayer, 25, 0.5, 6);
            }
            else
            {
                AddSpell(BLIZZARD, Target_RandomPlayerDestination, 20, 0, 20);
                AddSpell(ICE_BARB, Target_RandomPlayer, 25, 0.5, 6);
            };
        };

        void OnLoad()
        {
            AggroNearestUnit();
            ParentClass::OnLoad();
        };
};

class TelestraArcaneAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(TelestraArcaneAI, MoonScriptBossAI);
        TelestraArcaneAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            AddSpell(TIME_STOP, Target_Self, 30, 1.5, 30);
            AddSpell(CRITTER, Target_RandomPlayer, 25, 0, 20);
        };

        void OnLoad()
        {
            AggroNearestUnit();
            ParentClass::OnLoad();
        };
};


//////////////////////////////////////////////////////////////////////////////////////////
/// Ormorok the Tree-Shaper
class OrmorokAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(OrmorokAI, MoonScriptBossAI);
    OrmorokAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        mInstance = GetInstanceScript();

        if (IsHeroic())
            AddSpell(TRAMPLE_H, Target_Current, 30, 0, 9);
        else
            AddSpell(TRAMPLE, Target_Current, 30, 0, 9);

        AddSpell(SPELL_REFLECTION, Target_Self, 35, 2.0f, 15);
        mCrystalSpikes = AddSpell(CRYSTAL_SPIKES, Target_Self, 25, 0, 12);
        mCrystalSpikes->AddEmote("Bleed!", CHAT_MSG_MONSTER_YELL, 13332);

        mEnraged = false;
    };

    void OnCombatStart(Unit* pTarget)
    {
        sendDBChatMessage(1943);     // Noo!
        mEnraged = false;
        ParentClass::OnCombatStart(pTarget);
    };

    void AIUpdate()
    {
        if (_getHealthPercent() <= 25 && mEnraged == false)
        {
            ApplyAura(FRENZY);
            Announce("Ormorok the Tree-Shaper goes into a frenzy!");
            mEnraged = true;
        };

        ParentClass::AIUpdate();
    };

    void OnDied(Unit* pKiller)
    {
        sendDBChatMessage(1944);     // Aaggh!

        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, NEXUS_ORMOROK, State_Finished);

        ParentClass::OnDied(pKiller);
    };

    private:

        SpellDesc* mCrystalSpikes;
        bool mEnraged;

        MoonInstanceScript* mInstance;
};

class CrystalSpikeAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(CrystalSpikeAI, MoonScriptBossAI);
    CrystalSpikeAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        m_part = 0;
    }

    void OnLoad()
    {
        setCanEnterCombat(false);
        setRooted(true);
        _unit->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        despawn(4500, 0);
        RegisterAIUpdateEvent(500);

        ParentClass::OnLoad();
    };

    void AIUpdate()
    {
        m_part += 1;

        if (m_part == 1)
        {
            _unit->CastSpell(_unit, SPELL_CRYSTAL_SPIKE_VISUAL, true);
        }
        else if (m_part == 5)
        {
            if (IsHeroic())
            {
                _unit->CastSpell(_unit, sSpellCustomizations.GetSpellInfo(SPELL_CRYSTAL_SPIKE_H), true);
            }
            else
            {
                _unit->CastSpell(_unit, sSpellCustomizations.GetSpellInfo(SPELL_CRYSTAL_SPIKE), true);
            };
        };
    };

    private:

        int m_part;
};


//////////////////////////////////////////////////////////////////////////////////////////
/// Keristrasza
/// \todo currently unfinished
class KeristraszaAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(KeristraszaAI, MoonScriptBossAI);
    KeristraszaAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        if (IsHeroic())
            AddSpell(CRYSTALFIRE_BREATH_HC, Target_Self, 30, 1, 14);
        else
            AddSpell(CRYSTALFIRE_BREATH, Target_Self, 30, 1, 14);

        AddSpell(CRYSTAL_CHAINS, Target_RandomPlayer, 30, 0, 12);
        AddSpell(TAIL_SWEEP, Target_Self, 40, 0, 8);

        mCrystalize = AddSpell(CRYSTALLIZE, Target_Self, 25, 0, 22);
        mCrystalize->AddEmote("Stay. Enjoy your final moments.", CHAT_MSG_MONSTER_YELL, 13451);

        mEnraged = false;
        setCanEnterCombat(false);
    }

    void OnLoad()
    {
        ApplyAura(47543);   // frozen prison
        ParentClass::OnLoad();
    }

    void OnCombatStart(Unit* pTarget)
    {
        sendDBChatMessage(4321);     // Preserve? Why? There's no truth in it. No no no... only in the taking! I see that now!
    }

    void OnTargetDied(Unit* pTarget)
    {
        sendDBChatMessage(4322);     // Now we've come to the truth! 
    }

    void OnDied(Unit* pKiller)
    {
        sendDBChatMessage(4324);     // Dragonqueen... Life-Binder... preserve... me.
    }

    void AIUpdate()
    {
        if (mEnraged == false && _getHealthPercent() <= 25)
        {
            ApplyAura(ENRAGE);
            mEnraged = true;
        }
    }

    void Release()
    {
        setCanEnterCombat(true);
        RemoveAura(47543);
        ApplyAura(INTENSE_COLD);
    }

    private:

        bool mEnraged;
        SpellDesc* mCrystalize;
};

// Nexus Instance script
class NexusScript : public MoonInstanceScript
{
    public:

        uint32 mAnomalusGUID;
        uint32 mTelestraGUID;
        uint32 mOrmorokGUID;
        uint32 mKeristraszaGUID;

        uint8 mCSCount;

        uint32 m_uiEncounters[NEXUS_END];

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(NexusScript, MoonInstanceScript);
        NexusScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
        {
            mAnomalusGUID = 0;
            mTelestraGUID = 0;
            mOrmorokGUID = 0;
            mKeristraszaGUID = 0;

            mCSCount = 0;

            for (uint8 i = 0; i < NEXUS_END; ++i)
                m_uiEncounters[i] = State_NotStarted;
        };

        void SetInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
        {
            if (pType != Data_EncounterState)
                return;

            if (pIndex >= NEXUS_END)
                return;

            m_uiEncounters[pIndex] = pData;

            if (pData == State_Finished)
            {
                switch (pIndex)
                {
                    case NEXUS_ANOMALUS:
                    {
                        GameObjectSet sphereSet = getGameObjectsSetForEntry(ANOMALUS_CS);
                        for (auto goSphere : sphereSet)
                        {
                            if (goSphere != nullptr)
                                goSphere->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNCLICKABLE);
                        }
                    } break;
                    case NEXUS_TELESTRA:
                    {
                        GameObjectSet sphereSet = getGameObjectsSetForEntry(TELESTRA_CS);
                        for (auto goSphere : sphereSet)
                        {
                            if (goSphere != nullptr)
                                goSphere->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNCLICKABLE);
                        }
                    } break;
                    case NEXUS_ORMOROK:
                    {
                        GameObjectSet sphereSet = getGameObjectsSetForEntry(ORMOROK_CS);
                        for (auto goSphere : sphereSet)
                        {
                            if (goSphere != nullptr)
                                goSphere->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNCLICKABLE);
                        }
                    } break;
                    default:
                        break;
                }
            }
        }

        uint32 GetInstanceData(uint32 pType, uint32 pIndex)
        {
            return m_uiEncounters[pIndex];
        };

        void OnCreaturePushToWorld(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
                case CN_KERISTRASZA:
                    mKeristraszaGUID = pCreature->GetLowGUID();
                    break;
                case CN_ANOMALUS:
                    mAnomalusGUID = pCreature->GetLowGUID();
                    break;
                case CN_TELESTRA:
                    mTelestraGUID = pCreature->GetLowGUID();
                    break;
                case CN_ORMOROK:
                    mOrmorokGUID = pCreature->GetLowGUID();
                    break;
            }
        };

        void OnGameObjectPushToWorld(GameObject* pGameObject)
        {
            switch (pGameObject->GetEntry())
            {
                case ANOMALUS_CS:
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    break;
                case TELESTRA_CS:
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    break;
                case ORMOROK_CS:
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    break;
            };
        };

        void OnGameObjectActivate(GameObject* pGameObject, Player* pPlayer)
        {
            switch (pGameObject->GetEntry())
            {
                case ANOMALUS_CS:
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    ++mCSCount;
                    break;
                case TELESTRA_CS:
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    ++mCSCount;
                    break;
                case ORMOROK_CS:
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    ++mCSCount;
                    break;
                default:
                    return;
            }

            if (mCSCount == 3)   // release last boss
            {
                Creature* pKeristrasza = GetCreatureByGuid(mKeristraszaGUID);
                if (pKeristrasza == NULL)
                    return;

                KeristraszaAI* pKeristraszaAI = static_cast< KeristraszaAI* >(pKeristrasza->GetScript());
                if (pKeristraszaAI == NULL)
                    return;

                pKeristraszaAI->Release();
            };
        };

        void OnPlayerEnter(Player* player)
        {
            if (!mSpawnsCreated)
            {
                // team spawns
                if (player->GetTeam() == TEAM_ALLIANCE)
                {
                    for (uint8 i = 0; i < 18; i++)
                        spawnCreature(TrashHordeSpawns[i].entry, TrashHordeSpawns[i].x, TrashHordeSpawns[i].y, TrashHordeSpawns[i].z, TrashHordeSpawns[i].o, TrashHordeSpawns[i].faction);
                }
                else
                {
                    for (uint8 i = 0; i < 18; i++)
                        spawnCreature(TrashAllySpawns[i].entry, TrashAllySpawns[i].x, TrashAllySpawns[i].y, TrashAllySpawns[i].z, TrashAllySpawns[i].o, TrashAllySpawns[i].faction);
                }

                // difficulty spawns
                if (player->GetDungeonDifficulty() == MODE_NORMAL)
                {
                    switch (player->GetTeam())
                    {
                        case TEAM_ALLIANCE:
                            spawnCreature(CN_HORDE_COMMANDER, 425.39f, 185.82f, -35.01f, -1.57f, 14);
                            break;
                        case TEAM_HORDE:
                            spawnCreature(CN_ALLIANCE_COMMANDER, 425.39f, 185.82f, -35.01f, -1.57f, 14);
                            break;
                    }
                }
                else    // MODE_HEROIC
                {
                    switch (player->GetTeam())
                    {
                        case TEAM_ALLIANCE:
                            spawnCreature(H_CN_HORDE_COMMANDER, 425.39f, 185.82f, -35.01f, -1.57f, 14);
                            break;
                        case TEAM_HORDE:
                            spawnCreature(H_CN_ALLIANCE_COMMANDER, 425.39f, 185.82f, -35.01f, -1.57f, 14);
                            break;
                    }
                }
                mSpawnsCreated = true;
            }
        };
};

void SetupNexus(ScriptMgr* mgr)
{
    // Anomalus Encounter
    mgr->register_creature_script(CN_ANOMALUS, &AnomalusAI::Create);
    mgr->register_creature_script(CN_CHAOTIC_RIFT, &ChaoticRiftAI::Create);
    mgr->register_creature_script(CN_CRAZED_MANA_WRAITH, &CraziedManaWrathAI::Create);

    // Grand Magus Telestra Encounter
    mgr->register_creature_script(CN_TELESTRA, &TelestraBossAI::Create);
    mgr->register_creature_script(CN_TELESTRA_ARCANE, &TelestraArcaneAI::Create);
    mgr->register_creature_script(CN_TELESTRA_FROST, &TelestraFrostAI::Create);
    mgr->register_creature_script(CN_TELESTRA_FIRE, &TelestraFireAI::Create);

    // Ormorok the Tree-Shaper Encounter
    mgr->register_creature_script(CN_ORMOROK, &OrmorokAI::Create);
    mgr->register_creature_script(CN_CRYSTAL_SPIKE, &CrystalSpikeAI::Create);


    // Keristrasza Encounter
    mgr->register_creature_script(CN_KERISTRASZA, &KeristraszaAI::Create);

#ifndef UseNewMapScriptsProject
    mgr->register_instance_script(MAP_NEXUS, &NexusScript::Create);
#endif
}
