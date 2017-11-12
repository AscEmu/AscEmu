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

// \todo Ingvar the Plunderer - Ressurection Event, Fix despawn(i think all bosses), Add sound ID's
// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Instance_UtgardeKeep.h"

//////////////////////////////////////////////////////////////////////////
//////// Utgarde Keep Instance script

enum UtgardeData
{
    UTGARDE_FORGE_MASTER,

    // Bosses
    UTGARDE_KELESETH,
    UTGARDE_SKARVALD_AND_DALRONN,
    UTGARDE_INGVAR,

    UTGARDE_DATA_END
};

struct ForgeMasterData
{
    ForgeMasterData(uint32 pB = 0, uint32 pF = 0, uint32 pA = 0)
    {
        mBellow = pB;
        mFire = pF;
        mAnvil = pA;
    }

    uint32 mBellow;
    uint32 mFire;
    uint32 mAnvil;
};

class UtgardeKeepScript : public InstanceScript
{
    public:
        uint32        mKelesethGUID;
        uint32        mSkarvaldGUID;
        uint32        mDalronnGUID;
        uint32        mIngvarGUID;

        ForgeMasterData m_fmData[3];
        uint32        mDalronnDoorsGUID;
        uint32        mIngvarDoors[2];

        uint8        mUtgardeData[UTGARDE_DATA_END];

        UtgardeKeepScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
        {
            mKelesethGUID = 0;
            mSkarvaldGUID = 0;
            mDalronnGUID = 0;
            mIngvarGUID = 0;

            for (uint8 i = 0; i < 3; ++i)
            {
                m_fmData[i] = ForgeMasterData();
            }

            mDalronnDoorsGUID = 0;

            for (uint8 i = 0; i < 2; ++i)
            {
                mIngvarDoors[i] = 0;
            }

            for (uint8 i = 0; i < UTGARDE_DATA_END; ++i)
            {
                mUtgardeData[i] = 0;
            }
        }

        static InstanceScript* Create(MapMgr* pMapMgr) { return new UtgardeKeepScript(pMapMgr); }

        void OnCreaturePushToWorld(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
                case CN_PRINCE_KELESETH:
                    mKelesethGUID = pCreature->GetLowGUID();
                    break;
                case CN_SKARVALD:
                    mSkarvaldGUID = pCreature->GetLowGUID();
                    break;
                case CN_DALRONN:
                    mDalronnGUID = pCreature->GetLowGUID();
                    break;
                case CN_INGVAR:
                    mIngvarGUID = pCreature->GetLowGUID();
                    break;
            }
        }

        void OnGameObjectPushToWorld(GameObject* pGameObject)
        {
            switch (pGameObject->GetEntry())
            {
                case BELLOW_1:
                    m_fmData[0].mBellow = pGameObject->GetLowGUID();
                    break;
                case BELLOW_2:
                    m_fmData[1].mBellow = pGameObject->GetLowGUID();
                    break;
                case BELLOW_3:
                    m_fmData[2].mBellow = pGameObject->GetLowGUID();
                    break;
                case FORGEFIRE_1:
                    m_fmData[0].mFire = pGameObject->GetLowGUID();
                    break;
                case FORGEFIRE_2:
                    m_fmData[1].mFire = pGameObject->GetLowGUID();
                    break;
                case FORGEFIRE_3:
                    m_fmData[2].mFire = pGameObject->GetLowGUID();
                    break;
                case GLOWING_ANVIL_1:
                    m_fmData[0].mAnvil = pGameObject->GetLowGUID();
                    break;
                case GLOWING_ANVIL_2:
                    m_fmData[1].mAnvil = pGameObject->GetLowGUID();
                    break;
                case GLOWING_ANVIL_3:
                    m_fmData[2].mAnvil = pGameObject->GetLowGUID();
                    break;
                case DALRONN_DOORS:
                    mDalronnDoorsGUID = pGameObject->GetLowGUID();
                    break;
                case INGVAR_DOORS_1:
                    mIngvarDoors[0] = pGameObject->GetLowGUID();
                    break;
                case INGVAR_DOORS_2:
                    mIngvarDoors[1] = pGameObject->GetLowGUID();
                    break;
            }
        }

        void SetLocaleInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
        {
            switch (pIndex)
            {
                case UTGARDE_FORGE_MASTER:
                {
                    mUtgardeData[UTGARDE_FORGE_MASTER]++;
                    HandleForge();
                } break;
                case UTGARDE_INGVAR:
                {
                    mUtgardeData[UTGARDE_INGVAR] = pData;

                    if (pData == Finished)
                    {
                        GameObject* pGO = nullptr;
                        for (uint8 i = 0; i < 2; ++i)
                        {
                            pGO = GetGameObjectByGuid(mIngvarDoors[i]);
                            if (pGO)
                            {
                                pGO->SetState(pGO->GetState() == 1 ? 0 : 1);
                            }
                        }
                    }
                } break;
            }
        }

        void HandleForge()
        {
            GameObject* pGO = nullptr;
            pGO = GetGameObjectByGuid(m_fmData[mUtgardeData[UTGARDE_FORGE_MASTER] - 1].mBellow);
            if (pGO)
            {
                pGO->SetState(pGO->GetState() == 1 ? 0 : 1);
            }

            pGO = GetGameObjectByGuid(m_fmData[mUtgardeData[UTGARDE_FORGE_MASTER] - 1].mFire);
            if (pGO)
            {
                pGO->SetState(pGO->GetState() == 1 ? 0 : 1);
            }

            pGO = GetGameObjectByGuid(m_fmData[mUtgardeData[UTGARDE_FORGE_MASTER] - 1].mAnvil);
            if (pGO)
            {
                pGO->SetState(pGO->GetState() == 1 ? 0 : 1);
            }
        }
};

//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Forge Master
class DragonflayerForgeMasterAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(DragonflayerForgeMasterAI, MoonScriptCreatureAI);
        DragonflayerForgeMasterAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            pInstance = (UtgardeKeepScript*)getCreature()->GetMapMgr()->GetScript();

            AddSpell(DRAGONFLAYER_FORGE_MASTER_BURNING_BRAND, Target_Current, 8, 0, 40, 0, 30);
        }

        void OnDied(Unit* pKiller)
        {
            if (pInstance)
                pInstance->SetLocaleInstanceData(0, UTGARDE_FORGE_MASTER, 0);

            ParentClass::OnDied(pKiller);
        }

        UtgardeKeepScript* pInstance;
};

//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer HeartSplitter
class DragonflayerHeartSplitterAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(DragonflayerHeartSplitterAI, MoonScriptCreatureAI);
        DragonflayerHeartSplitterAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(DRAGONFLAYER_HEARTSPLITTER_PIERCING_JAB, Target_Current, 8, 0, 40, 0, 30);
            AddSpell(DRAGONFLAYER_HEARTSPLITTER_THROW, Target_Current, 8, 0, 40, 0, 30);
            AddSpell(DRAGONFLAYER_HEARTSPLITTER_WING_CLIP, Target_Current, 8, 0, 40, 0, 30);
        }
};


//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Ironhelm
class DragonflayerIronhelmAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(DragonflayerIronhelmAI, MoonScriptCreatureAI);
        DragonflayerIronhelmAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(DRAGONFLAYER_IRONHELM_HEROIC_STRIKE, Target_Current, 8, 0, 40, 0, 30);
            AddSpell(DRAGONFLAYER_IRONHELM_RINGING_SLAP, Target_Current, 8, 0, 40, 0, 30);
        }
};


//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Metalworker
class DragonflayerMetalworkerAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(DragonflayerMetalworkerAI, MoonScriptCreatureAI);
        DragonflayerMetalworkerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(DRAGONFLAYER_METALWORKER_SUNDER_ARMOR, Target_Current, 8, 0, 40, 0, 30);
            mDfEnrage = AddSpell(DRAGONFLAYER_METALWORKER_ENRAGE, Target_Self, 0, 0, 0);
            Enrage = true;
        }

        void AIUpdate()
        {
            if (_getHealthPercent() <= 20 && Enrage)
            {
                CastSpell(mDfEnrage);
                Enrage = false;
            }

            ParentClass::AIUpdate();
        }

        bool Enrage;
        SpellDesc* mDfEnrage;
};

//////////////////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Overseer
class DragonflayerOverseerAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(DragonflayerOverseerAI, MoonScriptCreatureAI);
        DragonflayerOverseerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(DRAGONFLAYER_OVERSEER_BATTLE_SHOUT, Target_Current, 8, 0, 40, 0, 30);
            AddSpell(DRAGONFLAYER_OVERSEER_CHARGE, Target_Current, 8, 0, 40, 0, 30);
            AddSpell(DRAGONFLAYER_OVERSEER_DEMORALIZING_SHOUT, Target_Current, 8, 0, 40, 0, 30);
        }
};

//////////////////////////////////////////////////////////////////////////////////////
//////// Tunneling Ghoul
class TunnelingGhoulAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(TunnelingGhoulAI, MoonScriptCreatureAI);
    TunnelingGhoulAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(TUNNELING_GHOUL_DECREPIFY, Target_Current, 8, 0, 40, 0, 30);
        AddSpell(TUNNELING_GHOUL_STRIKE, Target_Current, 8, 0, 40, 0, 30);
    }
};

//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Runecaster
class DragonflayerRunecasterAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(DragonflayerRunecasterAI, MoonScriptCreatureAI);
        DragonflayerRunecasterAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(DRAGONFLAYER_RUNECASTER_BOLTHORNS_RUNE_OF_FLAME, Target_Self , 100, 0, 0);
            AddSpell(DRAGONFLAYER_RUNECASTER_NJORDS_RUNE_OF_PROTECTION, Target_Self , 100, 0, 0);
        }
};

//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Spiritualist
class DragonflayerSpiritualistAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(DragonflayerSpiritualistAI, MoonScriptCreatureAI);
        DragonflayerSpiritualistAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(DRAGONFLAYER_SPIRITUALIST_FLAME_SHOCK, Target_Current, 8, 0, 40, 0, 30);
            AddSpell(DRAGONFLAYER_SPIRITUALIST_LIGHTNING_BOLT, Target_RandomPlayerNotCurrent, 8, 0, 40, 0, 30);
            mHealDf = AddSpell(DRAGONFLAYER_SPIRITUALIST_HEALING_WAVE, Target_Self, 0, 0, 2);
            Heal = true;
        }

        void AIUpdate()
        {
            if (_getHealthPercent() <= 42 && Heal)
            {
                CastSpell(mHealDf);
                Heal = false;
            }

            ParentClass::AIUpdate();
        }

        bool Heal;
        SpellDesc* mHealDf;
};

//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Strategist
class DragonflayerStrategistAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(DragonflayerStrategistAI, MoonScriptCreatureAI);
        DragonflayerStrategistAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(DRAGONFLAYER_STRATEGIST_BLIND, Target_Current, 8, 0, 40, 0, 30);
            AddSpell(DRAGONFLAYER_STRATEGIST_HURL_DAGGER, Target_Current, 8, 0, 40, 0, 30);
            AddSpell(DRAGONFLAYER_STRATEGIST_TICKING_BOMB, Target_Current, 8, 0, 40, 0, 30);
        }
};

//////////////////////////////////////////////////////////////////////////
//////// Proto-Drake Handler
class ProtoDrake_HandlerAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(ProtoDrake_HandlerAI, MoonScriptCreatureAI);
    ProtoDrake_HandlerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(PROTO_DRAKE_HANDLER_DEBILITATING_STRIKE, Target_Current, 8, 0, 40, 0, 30);
        AddSpell(PROTO_DRAKE_HANDLER_THROW, Target_Current, 8, 0, 40, 0, 30);
        AddSpell(PROTO_DRAKE_HANDLER_UNHOLY_RAGE, Target_Current, 8, 0, 40, 0, 30);
    }
};

//////////////////////////////////////////////////////////////////////////
//////// Frenzied Geist
class FrenziedGeistAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(FrenziedGeistAI, MoonScriptCreatureAI);
        FrenziedGeistAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(FRENZIED_GEIST_FIXATE, Target_Current, 8, 0, 40, 0, 30);
        }
};

//////////////////////////////////////////////////////////////////////////
//////// Savage Worg
class SavageWorgAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SavageWorgAI, MoonScriptCreatureAI);
    SavageWorgAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SAVAGE_WORG_ENRAGE, Target_Self, 8, 0, 40, 0, 30);
        AddSpell(SAVAGE_WORG_POUNCE, Target_Current, 8, 0, 40, 0, 30);
    }
};

//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Bonecrusher
class DragonflayerBonecrusherAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DragonflayerBonecrusherAI, MoonScriptCreatureAI);
    DragonflayerBonecrusherAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(DRAGONFLAYER_BONECRUSHER_HEAD_CRACK, Target_Current, 8, 0, 40, 0, 30);
        AddSpell(DRAGONFLAYER_BONECRUSHER_KNOCKDOWNSPIN, Target_Self, 8, 0, 40, 0, 30);
    }
};

//////////////////////////////////////////////////////////////////////////
//////// Proto-Drake Rider
class ProtoDrake_RiderAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(ProtoDrake_RiderAI, MoonScriptCreatureAI);
    ProtoDrake_RiderAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(PROTO_DRAKE_RIDER_PIERCING_JAB, Target_Current, 8, 0, 40, 0, 30);
        AddSpell(PROTO_DRAKE_RIDER_THROW, Target_Current, 8, 0, 40, 0, 30);
        AddSpell(PROTO_DRAKE_RIDER_WING_CLIP, Target_Current, 8, 0, 40, 0, 30);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////
/////////////////// Skarvald the Constructor  & Dalronn the Controller  ///////////////////
//////////////////////////////////////////////////////////////////////////////////////////

// Skarvald the Constructor
class SkarvaldTheConstructorAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(SkarvaldTheConstructorAI, MoonScriptCreatureAI);
        SkarvaldTheConstructorAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(SKARVALD_CHARGE, Target_RandomPlayerNotCurrent, 35, 0, 8);
            AddSpell(STONE_STRIKE, Target_ClosestPlayer, 25, 0, 10);

            mReplyTimer = INVALIDATE_TIMER;
            pDalronn = NULL;
            pDalronnGhost = NULL;
        }

        void OnCombatStart(Unit* pTarget)
        {
            sendDBChatMessage(4471);     // Dalronn! See if you can muster the nerve to join my attack!

            pDalronn = getNearestCreatureAI(CN_DALRONN);

            mReplyTimer = _addTimer(2500);

            ParentClass::OnCombatStart(pTarget);
        }

        void AIUpdate()
        {
            if (_isTimerFinished(mReplyTimer) && pDalronn != nullptr)
            {
                pDalronn->sendChatMessage(CHAT_MSG_MONSTER_YELL, 13199, "By all means, don't assess the situation, you halfwit! Just jump into the fray!");
                _removeTimer(mReplyTimer);
            }

            ParentClass::AIUpdate();
        }

        void OnDied(Unit* pKiller)
        {
            if (pDalronn != nullptr && pDalronn->isAlive())
            {
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Not... over... yet.");
                pDalronn->sendChatMessage(CHAT_MSG_MONSTER_YELL, 13203, "Skarvald, you incompetent slug! Return and make yourself useful!");
                spawnCreature(CN_SKARVALD_GHOST, getCreature()->GetPosition());
                getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

            }
            else if (pDalronn != nullptr && !pDalronn->isAlive())
            {
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 13231, "A warrior's death.");

                pDalronnGhost = getNearestCreatureAI(CN_DALRONN_GHOST);

                if (pDalronnGhost != nullptr)
                {
                    pDalronnGhost->despawn(1000, 0);
                    pDalronnGhost = nullptr;
                }
            }

            ParentClass::OnDied(pKiller);
        }

        void OnCombatStop(Unit* pTarget)
        {
            if (pDalronn != nullptr)
            {
                if (pDalronn->isAlive())
                    moveToSpawn();
                else
                    spawnCreature(CN_DALRONN, pDalronn->getCreature()->GetSpawnPosition());
            }

            if (pDalronnGhost != nullptr && pDalronnGhost->isAlive())
            {
                pDalronnGhost->despawn();
                pDalronnGhost = nullptr;
            }
        }

    private:
        uint32 mReplyTimer;
        CreatureAIScript* pDalronn;
        CreatureAIScript* pDalronnGhost;
};

// Dalronn the Controller
class DalronnTheControllerAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(DalronnTheControllerAI, MoonScriptCreatureAI);
        DalronnTheControllerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            if (_isHeroic())
            {
                AddSpell(SHADOW_BOLT_HC, Target_RandomPlayer, 85, 2, 3);
                AddSpell(DEBILITATE, Target_RandomPlayer, 25, 0, 12);
                mSummonTimer = _addTimer(15000);
            }
            else
            {
                AddSpell(SHADOW_BOLT, Target_RandomPlayer, 35, 2, 8);
                AddSpell(DEBILITATE, Target_RandomPlayer, 25, 0, 12);
                mSummonTimer = _addTimer(15000);
            }
            pSkarvald = nullptr;
            pSkarvaldGhost = nullptr;
        }

        void OnCombatStart(Unit* pTarget)
        {
            pSkarvald = getNearestCreatureAI(CN_SKARVALD);

            ParentClass::OnCombatStart(pTarget);
        }

        void AIUpdate()
        {
            if (_isTimerFinished(mSummonTimer))
            {
                spawnCreature(SKELETON_ADD, getCreature()->GetPositionX() + 6, getCreature()->GetPositionY() + 4, getCreature()->GetPositionZ(), 0);
                spawnCreature(SKELETON_ADD, getCreature()->GetPositionX() - 6, getCreature()->GetPositionY() + 4, getCreature()->GetPositionZ(), 0);
                _resetTimer(mSummonTimer, 15000);
            }

            ParentClass::AIUpdate();
        }

        void OnDied(Unit* pKiller)
        {
            if (pSkarvald != nullptr && pSkarvald->isAlive())
            {
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "See... you... soon.");
                pSkarvald->sendChatMessage(CHAT_MSG_MONSTER_YELL, 13233, "Pagh! What sort of necromancer lets death stop him? I knew you were worthless!");
                spawnCreature(CN_DALRONN_GHOST, getCreature()->GetPosition());
                getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }
            else if (pSkarvald != nullptr && !pSkarvald->isAlive())
            {
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 13201, "There's no... greater... glory.");

                pSkarvaldGhost = getNearestCreatureAI(CN_SKARVALD_GHOST);

                if (pSkarvaldGhost != nullptr)
                {
                    pSkarvaldGhost->despawn(1000, 0);
                    pSkarvaldGhost = nullptr;
                }
            }

            ParentClass::OnDied(pKiller);
        }

        void OnCombatStop(Unit* pTarget)
        {
            if (pSkarvald != nullptr)
            {
                if (pSkarvald->isAlive())
                    moveToSpawn();
                else
                    spawnCreature(CN_DALRONN, pSkarvald->getCreature()->GetSpawnPosition());
            }

            if (pSkarvaldGhost != nullptr && pSkarvaldGhost->isAlive())
            {
                pSkarvaldGhost->despawn();
                pSkarvaldGhost = nullptr;
            }
        }

    private:
        int32 mSummonTimer;
        CreatureAIScript* pSkarvald;
        CreatureAIScript* pSkarvaldGhost;
};

class SkarvaldTheConstructorGhostAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(SkarvaldTheConstructorGhostAI, MoonScriptCreatureAI);
        SkarvaldTheConstructorGhostAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(SKARVALD_CHARGE, Target_RandomPlayerNotCurrent, 35, 0, 8);
            AddSpell(STONE_STRIKE, Target_ClosestPlayer, 25, 0, 10);
        }

        void OnLoad()
        {
            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);

            Player* pTarget = getNearestPlayer();
            if (pTarget != nullptr)
                getCreature()->GetAIInterface()->AttackReaction(pTarget, 50, 0);

            ParentClass::OnLoad();
        }
};

class DalronnTheControllerGhostAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(DalronnTheControllerGhostAI, MoonScriptCreatureAI);
        DalronnTheControllerGhostAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            if (_isHeroic())
            {
                AddSpell(SHADOW_BOLT_HC, Target_RandomPlayer, 85, 2, 3);
                AddSpell(DEBILITATE, Target_RandomPlayer, 25, 0, 12);
            }
            else
            {
                AddSpell(SHADOW_BOLT, Target_RandomPlayer, 35, 2, 8);
                AddSpell(DEBILITATE, Target_RandomPlayer, 25, 0, 12);
            }
        }

        void OnLoad()
        {
            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);

            Player* pTarget = getNearestPlayer();
            if (pTarget != nullptr)
                getCreature()->GetAIInterface()->AttackReaction(pTarget, 50, 0);

            ParentClass::OnLoad();
        }
};

////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// Prince Keleseth ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void SpellFunc_KelesethFrostTomb(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType)
{
    if (pCreatureAI != NULL)
    {
        if (pTarget == NULL || !pTarget->IsPlayer() || pTarget->IsDead())
            return;

        pCreatureAI->getCreature()->CastSpell(pTarget, FROST_TOMB_SPELL, true);
        pCreatureAI->spawnCreature(CN_FROST_TOMB, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), pTarget->GetOrientation());
        pCreatureAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Not so fast.");
    }
}

void SpellFunc_KelesethAddSummon(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType)
{
    if (pCreatureAI != NULL)
    {
        for (uint8 i = 0; i < 5; ++i)
            pCreatureAI->spawnCreature(KELESETH_SKELETON_ADD, 163.376f + i + 4, 252.901f - i + 5, 42.868f, 0, pCreatureAI->getCreature()->GetFaction());
    }
}

class PrinceKelesethAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(PrinceKelesethAI, MoonScriptCreatureAI);
        PrinceKelesethAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            mFrostTomb = AddSpellFunc(&SpellFunc_KelesethFrostTomb, Target_RandomPlayer, 25, 0, 15, 0, 20);
            mAddSummon = AddSpellFunc(&SpellFunc_KelesethAddSummon, Target_Self, 0, 0, 0);

            if (_isHeroic())
                mShadowBolt = AddSpell(KELESETH_SHADOW_BOLT_HC, Target_Current, 100, 2, 2);
            else
                mShadowBolt = AddSpell(KELESETH_SHADOW_BOLT, Target_Current, 100, 2, 2);

        }

        void OnCombatStart(Unit* pTarget)
        {
            sendDBChatMessage(500);      // Your blood is mine!
            CastSpellNowNoScheduling(mAddSummon);

            ParentClass::OnCombatStart(pTarget);
        }

        void OnTargetDied(Unit* pTarget)
        {
            sendDBChatMessage(504);      // I join... the night.
        }

        SpellDesc* mAddSummon;
        SpellDesc* mShadowBolt;
        SpellDesc* mFrostTomb;
};

// FrostTombAI
class FrostTombAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(FrostTombAI, MoonScriptCreatureAI);
        FrostTombAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            SetAIUpdateFreq(1000);
            plr = nullptr;
        }

        void OnLoad()
        {
            setRooted(true);
            plr = getNearestPlayer();
            ParentClass::OnLoad();
        }

        void AIUpdate()
        {
            ParentClass::AIUpdate();
            if (plr == nullptr || plr->IsDead() || !plr->HasAura(FROST_TOMB_SPELL))
                despawn();
        }

        void OnDied(Unit* pKilled)
        {
            if (plr != nullptr && plr->HasAura(FROST_TOMB_SPELL))
            {
                plr->RemoveAura(FROST_TOMB_SPELL);
            }

            ParentClass::OnDied(pKilled);

            despawn(1);
        }

    private:
        Player* plr;
};

class SkeletonAddAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(SkeletonAddAI, MoonScriptCreatureAI);
        SkeletonAddAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            if (_isHeroic())
                AddSpell(DECREPIFY_HC, Target_Current, 8, 0, 40);
            else
                AddSpell(DECREPIFY, Target_Current, 8, 0, 40);
        }

        void OnLoad()
        {
            Player* pTarget = getNearestPlayer();
            if (pTarget != nullptr)
                getCreature()->GetAIInterface()->AttackReaction(pTarget, 50, 0);

            ParentClass::OnLoad();
        }

        void OnCombatStop(Unit* pTarget)
        {
            despawn(1);
        }

        void OnDied(Unit* pKiller)
        {
            despawn(1);
        }

};

////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Ingvar the Plunderer //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void SpellFunc_ShadowAxe(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType)
{
    if (pCreatureAI != NULL)
    {
        if (pTarget == NULL || !pTarget->IsPlayer() || pTarget->IsDead())
            return;

        Creature* pShadowAxe = pCreatureAI->spawnCreature(CN_SHADOW_AXE, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), pTarget->GetOrientation());
        if (pShadowAxe == nullptr)
            return;

        pShadowAxe->CastSpell(pShadowAxe, SHADOW_AXE_SPELL, true);
        pShadowAxe->Despawn(10000, 0);
    }
};

class IngvarThePlundererAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(IngvarThePlundererAI, MoonScriptCreatureAI);
        IngvarThePlundererAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(INGVAR_CLEAVE, Target_Current, 24, 0, 6);

            if (_isHeroic())
            {
                AddSpell(INGVAR_ENRAGE_HC, Target_Self, 45, 0, 4);
                AddSpell(INGVAR_SMASH_HC, Target_Self, 25, 3, 18);
                AddSpell(INGVAR_ROAR_HC, Target_Self, 25, 2, 10);
            }
            else
            {
                AddSpell(INGVAR_ENRAGE, Target_Self, 45, 0, 4);
                AddSpell(INGVAR_SMASH, Target_Self, 25, 3, 18);
                AddSpell(INGVAR_ROAR, Target_Self, 25, 2, 10);
            }

            SetAIUpdateFreq(1000);
        }

        void OnCombatStart(Unit* pTarget)
        {
            sendDBChatMessage(4468);     // I'll paint my face with your blood!
        }

        void OnTargetDied(Unit* pTarget)
        {
            sendDBChatMessage(4469);     // Mjul orm agn gjor!
        }

        void OnDied(Unit* pKiller)
        {
            sendDBChatMessage(4470);     // My life for the... death god!

            //Ressurect event
            spawnCreature(CN_INGVAR_UNDEAD, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), getCreature()->GetFaction());
            despawn(1000, 0);
        }
};

class IngvarUndeadAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(IngvarUndeadAI, MoonScriptCreatureAI);
        IngvarUndeadAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            mInstance = getInstanceScript();

            AddSpellFunc(&SpellFunc_ShadowAxe, Target_RandomPlayerNotCurrent, 15, 0, 21);
            AddSpell(INGVAR_DARK_SMASH, Target_Self, 12, 3, 16);

            if (_isHeroic())
            {
                AddSpell(INGVAR_DREADFUL_ROAR, Target_Self, 25, 2, 10);
                AddSpell(INGVAR_WOE_STRIKE, Target_ClosestUnit, 18, 0, 16);
            }

        }

        void OnLoad()
        {
            Player* pTarget = getNearestPlayer();
            if (pTarget != nullptr)
                getCreature()->GetAIInterface()->AttackReaction(pTarget, 50, 0);
        }

        void OnDied(Unit* pKiller)
        {
            sendDBChatMessage(6986);     // No! I can do... better! I can...

            if (mInstance)
                mInstance->setData(getCreature()->GetEntry(), Finished);

            ParentClass::OnDied(pKiller);
        }

    private:
        InstanceScript* mInstance;
};

void SetupUtgardeKeep(ScriptMgr* mgr)
{
#ifndef UseNewMapScriptsProject
    mgr->register_instance_script(MAP_UTGARDE_KEEP, &UtgardeKeepScript::Create);
#endif
    //////////////////////////////////////////////////////////////////////////////////////////
    ///////// Mobs
    mgr->register_creature_script(CN_DRAGONFLAYER_FORGE_MASTER, &DragonflayerForgeMasterAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_HEARTSPLITTER, &DragonflayerHeartSplitterAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_IRONHELM, &DragonflayerIronhelmAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_METALWORKER, &DragonflayerIronhelmAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_OVERSEER, &DragonflayerOverseerAI::Create);
    mgr->register_creature_script(CN_TUNNELING_GHOUL, &TunnelingGhoulAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_RUNECASTER, &DragonflayerRunecasterAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_SPIRITUALIST, &DragonflayerSpiritualistAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_STRATEGIST, &DragonflayerStrategistAI::Create);
    mgr->register_creature_script(CN_PROTO_DRAKE_HANDLER, &ProtoDrake_HandlerAI::Create);
    mgr->register_creature_script(CN_FRENZIED_GEIST, &FrenziedGeistAI::Create);
    mgr->register_creature_script(CN_SAVAGE_WORG, &SavageWorgAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_BONECRUSHER, &DragonflayerBonecrusherAI::Create);
    mgr->register_creature_script(CN_PROTO_DRAKE_RIDER, &ProtoDrake_RiderAI::Create);
    //////////////////////////////////////////////////////////////////////////////////////////
    ///////// Bosses

    // Skarvald & Dalronn Encounter
    mgr->register_creature_script(CN_SKARVALD, &SkarvaldTheConstructorAI::Create);
    mgr->register_creature_script(CN_DALRONN, &DalronnTheControllerAI::Create);
    mgr->register_creature_script(CN_SKARVALD_GHOST, &SkarvaldTheConstructorGhostAI::Create);
    mgr->register_creature_script(CN_DALRONN_GHOST, &DalronnTheControllerGhostAI::Create);

    // Prince Keleseth Encounter
    mgr->register_creature_script(CN_PRINCE_KELESETH, &PrinceKelesethAI::Create);
    mgr->register_creature_script(CN_FROST_TOMB, &FrostTombAI::Create);
    mgr->register_creature_script(KELESETH_SKELETON_ADD, &SkeletonAddAI::Create);

    // Ingvar the Plunderer Encounter
    mgr->register_creature_script(CN_INGVAR, &IngvarThePlundererAI::Create);
    mgr->register_creature_script(CN_INGVAR_UNDEAD, &IngvarUndeadAI::Create);
}
