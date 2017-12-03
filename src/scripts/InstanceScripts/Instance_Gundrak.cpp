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

#define GO_FLAG_UNCLICKABLE 0x00000010

class GundrakScript : public InstanceScript
{
    public:
        uint32 mSladranAltarGUID;
        uint32 mSladranStatueGUID;
        uint32 mColossusAltarGUID;
        uint32 mColossusStatueGUID;
        uint32 mMoorabiAltarGUID;
        uint32 mMoorabiStatueGUID;

        uint32 mEckDoorsGUID;

        uint32 mTrapDoorGUID;
        uint32 mCoilisionGUID;

        uint32 mCombatDoorsGUID;
        uint32 mDoor1GUID;
        uint32 mDoor2GUID;

        uint8 mStatueCount;

        GundrakScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
        {
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
        }

        static InstanceScript* Create(MapMgr* pMapMgr) { return new GundrakScript(pMapMgr); }

        void OnGameObjectPushToWorld(GameObject* pGameObject) override
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
        }

        void OnGameObjectActivate(GameObject* pGameObject, Player* /*pPlayer*/) override
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
            }

            if (mStatueCount < 3)
                return;

            GameObject* pTrapDoors = GetGameObjectByGuid(mTrapDoorGUID);
            GameObject* pCoilision = GetGameObjectByGuid(mCoilisionGUID);
            if (pTrapDoors)
                pTrapDoors->SetState(pTrapDoors->GetState() == 1 ? 0 : 1);
            if (pCoilision)
                pCoilision->SetState(pCoilision->GetState() == 1 ? 0 : 1);
        }

        void OnCreatureDeath(Creature* pVictim, Unit* /*pKiller*/) override
        {
            GameObject* pDoors = NULL;
            GameObject* pAltar = NULL;
            switch (pVictim->GetEntry())
            {
                case CN_MOORABI:
                {
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
                    pAltar = GetGameObjectByGuid(mSladranAltarGUID);
                    if (pAltar)
                        pAltar->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNCLICKABLE);
                }
                break;
                case CN_DRAKKARI_COLOSSUS:
                {
                    pAltar = GetGameObjectByGuid(mColossusAltarGUID);
                    if (pAltar)
                        pAltar->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNCLICKABLE);
                }
                break;
            }
        }
};


// Status: 50% done, missing add related stuff and maybe correct timers
class SladranAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SladranAI);
    SladranAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        CreatureAISpells* sdPoisonNova = nullptr;
        if (_isHeroic())
        {
            addAISpell(59840, 25.0f, TARGET_ATTACKING, 0, 6);
            addAISpell(59839, 18.0f, TARGET_RANDOM_SINGLE, 2, 8);

            sdPoisonNova = addAISpell(59842, 10.0f, TARGET_SELF, 4, 16);
        }
        else
        {
            addAISpell(48287, 25.0f, TARGET_ATTACKING, 0, 6);
            addAISpell(54970, 18.0f, TARGET_RANDOM_SINGLE, 2, 8);

            sdPoisonNova = addAISpell(55081, 10.0f, TARGET_SELF, 4, 16);
        }

        if (sdPoisonNova != nullptr)
            sdPoisonNova->setAnnouncement("Slad'ran begins to cast Poison Nova!");

        addEmoteForEvent(Event_OnCombatStart, 8754);     // Drakkari gonna kill anybody who trespass on these lands!
        addEmoteForEvent(Event_OnTargetDied, 4217);     // You not breathin'? Good.
        addEmoteForEvent(Event_OnTargetDied, 4218);     // Ssscared now?
        addEmoteForEvent(Event_OnTargetDied, 4219);     // I eat you next, mon.
        addEmoteForEvent(Event_OnDied, 4220);     // I sssee now... Ssscourge wasss not... our greatessst enemy....
    }
};


// Status: 20% done, missing rihno part, need vehicle support for that, missing stampade script
class GalDarahAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GalDarahAI);
    GalDarahAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (_isHeroic())
            addAISpell(59824, 20.0f, TARGET_SELF, 0, 12);
        else
            addAISpell(55250, 20.0f, TARGET_SELF, 0, 12);

        addEmoteForEvent(Event_OnCombatStart, 4199);     // I'm gonna spill your guts, mon!
        addEmoteForEvent(Event_OnTargetDied, 4200);     // What a rush!
        addEmoteForEvent(Event_OnTargetDied, 4201);     // Who needs gods when we ARE gods?
        addEmoteForEvent(Event_OnTargetDied, 4202);     // I told ya so!
        addEmoteForEvent(Event_OnDied, 4203);     // Even the mighty... can fall.
    }
};


void SetupGundrak(ScriptMgr* mgr)
{
#ifndef UseNewMapScriptsProject
    mgr->register_instance_script(MAP_GUNDRAK, &GundrakScript::Create);
#endif
    mgr->register_creature_script(CN_SLADRAN, &SladranAI::Create);
    mgr->register_creature_script(CN_GAL_DARAH, &GalDarahAI::Create);
};
