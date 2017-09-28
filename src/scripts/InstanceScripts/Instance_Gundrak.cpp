/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2010 ArcEmu Team <http://www.arcemu.org/>
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
#include "Instance_Gundrak.h"


#define GO_FLAG_UNCLICKABLE            0x00000010


/////////////////////////////////////////////////////////////////////////////////
/// Gundrak Instance Script
// Status: Finished
class GundrakScript : public MoonInstanceScript
{
    public:
        uint32        mSkadranGUID;
        uint32        mColossusGUID;
        uint32        mMoorabiGUID;
        uint32        mEckGUID;
        uint32        mGalDarahGUID;

        uint32        mSladranAltarGUID;
        uint32        mSladranStatueGUID;
        uint32        mColossusAltarGUID;
        uint32        mColossusStatueGUID;
        uint32        mMoorabiAltarGUID;
        uint32        mMoorabiStatueGUID;

        uint32        mEckDoorsGUID;

        uint32        mTrapDoorGUID;
        uint32      mCoilisionGUID;

        uint32      mCombatDoorsGUID;
        uint32        mDoor1GUID;
        uint32        mDoor2GUID;

        uint8        mStatueCount;

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(GundrakScript, MoonInstanceScript);
        GundrakScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
        {
            mSkadranGUID = 0;
            mColossusGUID = 0;
            mMoorabiGUID = 0;
            mEckGUID = 0;
            mGalDarahGUID = 0;

            mSladranAltarGUID = 0;
            mSladranStatueGUID = 0;
            mColossusAltarGUID = 0;
            mColossusStatueGUID = 0;
            mMoorabiAltarGUID = 0;
            mMoorabiStatueGUID = 0;

            mEckDoorsGUID = 0;

            mTrapDoorGUID = 0;
            mCoilisionGUID = 0;

            mCombatDoorsGUID = 0;
            mDoor1GUID = 0;
            mDoor2GUID = 0;

            mStatueCount = 0;
        };

        void OnCreaturePushToWorld(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
                case CN_SLADRAN:
                {
                    mSkadranGUID = pCreature->GetLowGUID();
                    mEncounters.insert(EncounterMap::value_type(CN_SLADRAN, BossData(0, mSkadranGUID)));
                }
                break;
                case CN_MOORABI:
                {
                    mMoorabiGUID = pCreature->GetLowGUID();
                    mEncounters.insert(EncounterMap::value_type(CN_MOORABI, BossData(0, mMoorabiGUID)));
                }
                break;
                case CN_GAL_DARAH:
                {
                    mGalDarahGUID = pCreature->GetLowGUID();
                    mEncounters.insert(EncounterMap::value_type(CN_GAL_DARAH, BossData(0, mGalDarahGUID)));
                }
                break;
                case CN_DRAKKARI_COLOSSUS:
                {
                    mColossusGUID = pCreature->GetLowGUID();
                    mEncounters.insert(EncounterMap::value_type(CN_DRAKKARI_COLOSSUS, BossData(0, mColossusGUID)));
                }
                break;
                case CN_ECK:
                {
                    if (mInstance->iInstanceMode == MODE_HEROIC)
                    {
                        mEckGUID = pCreature->GetLowGUID();
                        mEncounters.insert(EncounterMap::value_type(CN_ECK, BossData(0, mEckGUID)));
                    };
                }
                break;
            };
        };

        void OnGameObjectPushToWorld(GameObject* pGameObject)
        {
            switch (pGameObject->GetEntry())
            {

                case GO_ALTAR1_SLADRAN:
                {
                    mSladranAltarGUID = pGameObject->GetLowGUID();
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                }
                break;
                case GO_STATUE1_SLADRAN:
                {
                    mSladranStatueGUID = pGameObject->GetLowGUID();
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                }
                break;
                case GO_ALTAR2_COLOSSUS:
                {
                    mColossusAltarGUID = pGameObject->GetLowGUID();
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                }
                break;
                case GO_STATUE2_COLOSSUS:
                {
                    mColossusStatueGUID = pGameObject->GetLowGUID();
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                }
                break;
                case GO_ALTAR3_MOORABI:
                {
                    mMoorabiAltarGUID = pGameObject->GetLowGUID();
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                }
                break;
                case GO_STATUE3_MOORABI:
                {
                    mMoorabiStatueGUID = pGameObject->GetLowGUID();
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                }
                break;
                case GO_ECKDOOR:
                {
                    mEckDoorsGUID = pGameObject->GetLowGUID();
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                }
                break;
                case GO_TRAPDOOR:
                {
                    mTrapDoorGUID = pGameObject->GetLowGUID();
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                }
                break;
                case GO_COILISION:
                {
                    mCoilisionGUID = pGameObject->GetLowGUID();
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                }
                break;
                case GO_GAL_DARAH_DOOR1:
                {
                    mDoor1GUID = pGameObject->GetLowGUID();
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                }
                break;
                case GO_GAL_DARAH_DOOR2:
                {
                    mDoor2GUID = pGameObject->GetLowGUID();
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                }
                break;
                case 192568:
                    mCombatDoorsGUID = pGameObject->GetLowGUID();
                    break;
            }
        };

        void OnGameObjectActivate(GameObject* pGameObject, Player* pPlayer)
        {
            switch (pGameObject->GetEntry())
            {
                case GO_ALTAR1_SLADRAN:
                {
                    GameObject* pStatue = GetGameObjectByGuid(mSladranStatueGUID);
                    if (pStatue)
                        pStatue->SetState(pStatue->GetState() == 1 ? 0 : 1);

                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    mStatueCount++;
                }
                break;
                case GO_ALTAR2_COLOSSUS:
                {
                    GameObject* pStatue = GetGameObjectByGuid(mColossusStatueGUID);
                    if (pStatue)
                        pStatue->SetState(pStatue->GetState() == 1 ? 0 : 1);

                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    mStatueCount++;
                }
                break;
                case GO_ALTAR3_MOORABI:
                {
                    GameObject* pStatue = GetGameObjectByGuid(mMoorabiStatueGUID);
                    if (pStatue)
                        pStatue->SetState(pStatue->GetState() == 1 ? 0 : 1);

                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    mStatueCount++;
                }
                break;
            };

            if (mStatueCount < 3)
                return;

            GameObject* pTrapDoors = GetGameObjectByGuid(mTrapDoorGUID);
            GameObject* pCoilision = GetGameObjectByGuid(mCoilisionGUID);
            if (pTrapDoors)
                pTrapDoors->SetState(pTrapDoors->GetState() == 1 ? 0 : 1);
            if (pCoilision)
                pCoilision->SetState(pCoilision->GetState() == 1 ? 0 : 1);
        };

        void SetInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = (EncounterState)pData;

            if (pIndex != CN_GAL_DARAH)
                return;

            GameObject* pDoors = GetGameObjectByGuid(mCombatDoorsGUID);
            if (!pDoors)
                return;

            switch (pData)
            {
                case State_InProgress:
                    pDoors->SetState(pDoors->GetState() == 1 ? 0 : 1);
                    break;
                case State_NotStarted:
                case State_Performed:
                case State_Finished:
                    pDoors->SetState(pDoors->GetState() == 1 ? 0 : 1);
                    break;
            }
        };

        uint32 GetInstanceData(uint32 pType, uint32 pIndex)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return 0;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return 0;

            return (*Iter).second.mState;
        };

        void OnCreatureDeath(Creature* pVictim, Unit* pKiller)
        {
            EncounterMap::iterator Iter = mEncounters.find(pVictim->GetEntry());
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = State_Finished;

            GameObject* pDoors = NULL;
            GameObject* pAltar = NULL;
            switch (pVictim->GetEntry())
            {
                case CN_MOORABI:
                {
                    SetInstanceData(Data_EncounterState, CN_MOORABI, State_Finished);

                    pAltar = GetGameObjectByGuid(mMoorabiAltarGUID);
                    if (pAltar)
                        pAltar->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNCLICKABLE);

                    if (mInstance->iInstanceMode != MODE_HEROIC)
                        return;

                    pDoors = GetGameObjectByGuid(mEckDoorsGUID);
                    if (pDoors)
                        pDoors->SetState(GO_STATE_OPEN);
                }
                break;
                case CN_GAL_DARAH:
                {
                    SetInstanceData(Data_EncounterState, CN_GAL_DARAH, State_Finished);
                    pDoors = GetGameObjectByGuid(mDoor1GUID);
                    if (pDoors)
                        pDoors->SetState(GO_STATE_OPEN);

                    pDoors = GetGameObjectByGuid(mDoor2GUID);
                    if (pDoors)
                        pDoors->SetState(GO_STATE_OPEN);
                }
                break;
                case CN_SLADRAN:
                {
                    SetInstanceData(Data_EncounterState, CN_SLADRAN, State_Finished);
                    pAltar = GetGameObjectByGuid(mSladranAltarGUID);
                    if (pAltar)
                        pAltar->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNCLICKABLE);
                }
                break;
                case CN_DRAKKARI_COLOSSUS:
                {
                    SetInstanceData(Data_EncounterState, CN_DRAKKARI_COLOSSUS, State_Finished);
                    pAltar = GetGameObjectByGuid(mColossusAltarGUID);
                    if (pAltar)
                        pAltar->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNCLICKABLE);
                }
                break;
                case CN_ECK:
                {
                    SetInstanceData(Data_EncounterState, CN_ECK, State_Finished);
                }
                break;
            };
        };
};

/////////////////////////////////////////////////////////////////////////////////
// Slad'ran encounter
// Status: 50% done, missing add related stuff and maybe correct timers
class SladranAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SladranAI, MoonScriptCreatureAI);
    SladranAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        mInstance = GetInstanceScript();

        SpellDesc* sdPoisonNova = AddSpell(HeroicInt(55081, 59842), Target_Self, 10, 3.5f, 16);
        sdPoisonNova->AddAnnouncement("Slad'ran begins to cast Poison Nova!");

        AddSpell(HeroicInt(48287, 59840), Target_Current, 25, 0, 6);
        AddSpell(HeroicInt(54970, 59839), Target_RandomPlayerNotCurrent, 18, 1.5f, 8);
    }

    void OnCombatStart(Unit* pTarget)
    {
        _unit->SendScriptTextChatMessage(8754);     // Drakkari gonna kill anybody who trespass on these lands!
        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_InProgress);

        ParentClass::OnCombatStart(pTarget);
    }

    void OnTargetDied(Unit* pTarget)
    {
        switch (RandomUInt(2))
        {
            case 0:
                _unit->SendScriptTextChatMessage(4217);     // You not breathin'? Good.
            break;
            case 1:
                _unit->SendScriptTextChatMessage(4218);     // Ssscared now?
            break;
            case 2:
                _unit->SendScriptTextChatMessage(4219);     // I eat you next, mon.
            break;
        }
    }

    void OnCombatStop(Unit* pTarget)
    {
        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Performed);

        ParentClass::OnCombatStop(pTarget);
    }

    void OnDied(Unit* pKiller)
    {
        _unit->SendScriptTextChatMessage(4220);     // I sssee now... Ssscourge wasss not... our greatessst enemy....
    }

    MoonInstanceScript* mInstance;
};


/////////////////////////////////////////////////////////////////////////////////
// Gal'darah encounter
// Status: 20% done, missing rihno part, need vehicle support for that, missing stampade script
class GalDarahAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(GalDarahAI, MoonScriptCreatureAI);
    GalDarahAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        mInstance = GetInstanceScript();

        AddSpell(HeroicInt(55250, 59824), Target_Self, 20, 0, 12);
    }

    void OnCombatStart(Unit* pTarget)
    {
        _unit->SendScriptTextChatMessage(4199);     // I'm gonna spill your guts, mon!

        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_InProgress);

        ParentClass::OnCombatStart(pTarget);
    }

    void OnTargetDied(Unit* pTarget)
    {
        switch (RandomUInt(2))
        {
            case 0:
                _unit->SendScriptTextChatMessage(4200);     // What a rush!
            break;
            case 1:
                _unit->SendScriptTextChatMessage(4201);     // Who needs gods when we ARE gods?
            break;
            case 2:
                _unit->SendScriptTextChatMessage(4202);     // I told ya so!
            break;
        }
    }

    void OnCombatStop(Unit* pTarget)
    {
        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Performed);

        ParentClass::OnCombatStop(pTarget);
    }

    void OnDied(Unit* pKiller)
    {
        _unit->SendScriptTextChatMessage(4203);     // Even the mighty... can fall.
    }

    MoonInstanceScript* mInstance;
};


void SetupGundrak(ScriptMgr* mgr)
{
#ifndef UseNewMapScriptsProject
    mgr->register_instance_script(MAP_GUNDRAK, &GundrakScript::Create);
#endif
    mgr->register_creature_script(CN_SLADRAN, &SladranAI::Create);
    mgr->register_creature_script(CN_GAL_DARAH, &GalDarahAI::Create);
};
