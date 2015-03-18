/*
  * AscEmu Framework based on ArcEmu MMORPG Server
  * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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
#include "Raid_IceCrownCitadel.h"

////////////////////////////////////////////////////////
//ICC zone: 4812
//Prepared creature entry:
//
//#define CN_DEATHBRINGER_SAURFANG    37813
//#define CN_FESTERGUT                36626
//#define CN_ROTFACE                  36627
//#define CN_PROFESSOR_PUTRICIDE      36678
//#define CN_PRINCE_VALANAR           37970
//#define CN_BLOOD_QUEEN_LANATHEL     37955
//#define CN_SINDRAGOSA               36853
//#define CN_THE_LICHKING             36597
//
///\todo  start boss scripts
////////////////////////////////////////////////////////
//Event: GunshipBattle
//
//Affects:
//Available teleports. If GunshipBattle done -> Teleportlocation 4 available.
//
//Devnotes:
//Far away from implementing this :(
///////////////////////////////////////////////////////


///////////////////////////////////////////////////////
//IceCrownCitadel Instance

class IceCrownCitadelScript : public MoonInstanceScript
{
    friend class ICCTeleporterAI; // Friendship forever ;-)

public:

    uint32 mMarrowgarGUID;
    uint32 mColdflameGUID;
    uint32 mDeathwhisperGUID;
    uint32 mDreamwalkerGUID;

    MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(IceCrownCitadelScript, MoonInstanceScript);
    IceCrownCitadelScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr) 
    {
        //NPC states
        mMarrowgarGUID = 0;
        mColdflameGUID = 0;
        mDeathwhisperGUID = 0;
        mDreamwalkerGUID = 0;
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject)
    {
        // Gos which are not visible by killing a boss needs a second check...
        if (GetInstanceData(Data_EncounterState, CN_LORD_MARROWGAR) == State_Finished)
        {
            AddGameObjectStateByEntry(GO_MARROWGAR_ICEWALL_1, State_Active);    // Icewall 1
            AddGameObjectStateByEntry(GO_MARROWGAR_ICEWALL_2, State_Active);    // Icewall 2
            AddGameObjectStateByEntry(GO_MARROWGAR_DOOR, State_Active);         // Door
        }
    }

    void OnCreaturePushToWorld(Creature* pCreature)
    {
        switch (pCreature->GetEntry())
        {
            // First set state
            case CN_LORD_MARROWGAR:
                {
                    mMarrowgarGUID = pCreature->GetLowGUID();
                    mEncounters.insert(EncounterMap::value_type(CN_LORD_MARROWGAR, BossData(0, mMarrowgarGUID)));

                    AddGameObjectStateByEntry(GO_MARROWGAR_ICEWALL_1, State_Inactive);    // Icewall 1
                    AddGameObjectStateByEntry(GO_MARROWGAR_ICEWALL_2, State_Inactive);    // Icewall 2
                    AddGameObjectStateByEntry(GO_MARROWGAR_DOOR, State_Inactive);         // Door

                }break;

            case CN_LADY_DEATHWHISPER:
                {
                    mDeathwhisperGUID = pCreature->GetLowGUID();
                    mEncounters.insert(EncounterMap::value_type(CN_LADY_DEATHWHISPER, BossData(0, mDeathwhisperGUID)));

                }break;

            case CN_VALITHRIA_DREAMWALKER:
                {
                    mDreamwalkerGUID = pCreature->GetLowGUID();
                    mEncounters.insert(EncounterMap::value_type(CN_VALITHRIA_DREAMWALKER, BossData(0, mDreamwalkerGUID)));

                }break;

            case CN_COLDFLAME:
                {
                    mColdflameGUID = pCreature->GetLowGUID();
                    mEncounters.insert(EncounterMap::value_type(CN_COLDFLAME, BossData(0, mColdflameGUID)));

                }break;

        }
    };


    void SetInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
    {
        if (pType != Data_EncounterState || pIndex == 0)
            return;

        EncounterMap::iterator Iter = mEncounters.find(pIndex);
        if (Iter == mEncounters.end())
            return;

        (*Iter).second.mState = (EncounterState)pData;
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

    void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
    {
        EncounterMap::iterator Iter = mEncounters.find(pCreature->GetEntry());
        if (Iter == mEncounters.end())
            return;

        (*Iter).second.mState = State_Finished;

        switch (pCreature->GetEntry())
        {
            case CN_LORD_MARROWGAR:
            {
                SetInstanceData(Data_EncounterState, CN_LORD_MARROWGAR, State_Finished);

                AddGameObjectStateByEntry(GO_MARROWGAR_ICEWALL_1, State_Active);    // Icewall 1
                AddGameObjectStateByEntry(GO_MARROWGAR_ICEWALL_2, State_Active);    // Icewall 2
                AddGameObjectStateByEntry(GO_MARROWGAR_DOOR, State_Active);         // Door
            }break;

            case CN_LADY_DEATHWHISPER:
                SetInstanceData(Data_EncounterState, CN_LADY_DEATHWHISPER, State_Finished);
                break;

            case CN_VALITHRIA_DREAMWALKER:
                SetInstanceData(Data_EncounterState, CN_VALITHRIA_DREAMWALKER, State_Finished);
                break;

            case CN_COLDFLAME:
                SetInstanceData(Data_EncounterState, CN_COLDFLAME, State_Finished);
                break;

            default:
                break;
        }
        return;
    }

    void OnPlayerEnter(Player* player)
        {
            // setup only the npcs with the correct team...
            switch (player->GetTeam())
            {
                case TEAM_ALLIANCE:
                    sChatHandler.SystemMessage(player->GetSession(), "Team = Alliance");
                    break;
                case TEAM_HORDE:
                    sChatHandler.SystemMessage(player->GetSession(), "Team = Horde");
                    break;
            }
        }

};

///////////////////////////////////////////////////////
// IceCrown Teleporter
class ICCTeleporterAI : public GameObjectAIScript
{
public:

    ICCTeleporterAI(GameObject* go) : GameObjectAIScript(go){}

    ~ICCTeleporterAI(){}

    static GameObjectAIScript* Create(GameObject* go) { return new ICCTeleporterAI(go); }

    void OnActivate(Player* player)
    {
        IceCrownCitadelScript* pInstance = (IceCrownCitadelScript*)player->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        GossipMenu* menu = NULL;
        objmgr.CreateGossipMenuForPlayer(&menu, _gameobject->GetGUID(), 1/*Its not one... need to be checked*/, player);
        menu->AddItem(ICON_CHAT, "Teleport to Light's Hammer.", 0);

        if (pInstance->GetInstanceData(Data_EncounterState, CN_LORD_MARROWGAR) == State_Finished)
            menu->AddItem(ICON_CHAT, "Teleport to Oratory of The Damned.", 1);

        if (pInstance->GetInstanceData(Data_EncounterState, CN_LADY_DEATHWHISPER) == State_Finished)
            menu->AddItem(ICON_CHAT, "Teleport to Rampart of Skulls.", 2);

        // GunshipBattle has to be finished...
        //menu->AddItem(ICON_CHAT, "Teleport to Deathbringer's Rise.", 3);

        if (pInstance->GetInstanceData(Data_EncounterState, CN_VALITHRIA_DREAMWALKER) == State_Finished)
            menu->AddItem(ICON_CHAT, "Teleport to the Upper Spire.", 4);

        if (pInstance->GetInstanceData(Data_EncounterState, CN_COLDFLAME) == State_Finished)
            menu->AddItem(ICON_CHAT, "Teleport to Sindragosa's Lair.", 5);

        menu->SendTo(player);
    }

};

class ICCTeleporterGossip : public GossipScript
{
public:
    ICCTeleporterGossip() : GossipScript(){}

    void OnSelectOption(Object* object, Player* player, uint32 Id, const char* enteredcode)
    {
        Arcemu::Gossip::Menu::Complete(player);

        if (Id >= 7)
            return;
        else
            player->SafeTeleport(ICCTeleCoords[Id][0], player->GetInstanceID(), ICCTeleCoords[Id][1], ICCTeleCoords[Id][2], ICCTeleCoords[Id][3], ICCTeleCoords[Id][4]);
    }
};

///////////////////////////////////////////////////////
// Boss: Lord Marrowgar
// LM_BERSERK    = 47008
// BONE_SLICE    = 69055
// BONE_SPIKE    = 69057
// BONE_STORM    = 69076
// SOUL_FEAST    = 71203
// ...

///////////////////////////////////////////////////////
// Boss: Lady Deathwhisper
// MANA_BARRIER = 70842
// DEATH_AND_DECAY = 71001
// TOUCH_OF_INSIGNIFICANCE = 71204
// SHADOW_BOLT = 71254
// DOMINATE_MIND_H = 71289
// SUMMON_SHADE = 71363
// FROSTBOLT = 71420
// FROSTBOLT_VOLLEY = 72905
// ...

///////////////////////////////////////////////////////
// Boss: Valithria Dreamwalker
// ...
//
//
//
//

///////////////////////////////////////////////////////
// Boss: Cold Flame
// ...
//
//
//
//

void SetupICC(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_ICECROWNCITADEL, &IceCrownCitadelScript::Create);

    //Teleporters
    mgr->register_gameobject_script(GO_TELE_1, &ICCTeleporterAI::Create);
    mgr->register_go_gossip_script(GO_TELE_1, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_2, &ICCTeleporterAI::Create);
    mgr->register_go_gossip_script(GO_TELE_2, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_3, &ICCTeleporterAI::Create);
    mgr->register_go_gossip_script(GO_TELE_3, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_4, &ICCTeleporterAI::Create);
    mgr->register_go_gossip_script(GO_TELE_4, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_5, &ICCTeleporterAI::Create);
    mgr->register_go_gossip_script(GO_TELE_5, new ICCTeleporterGossip());

    //Bosses
    //mgr->register_creature_script(CN_LORD_MARROWGAR, &LordMarrowgarAI::Create);
    //mgr->register_creature_script(CN_LADY_DEATHWHISPER, &LadyDeathwhisperAI::Create);
    //mgr->register_creature_script(CN_VALITHRIA_DREAMWALKER, &ValithriaDreamwalkerAI::Create);
    //mgr->register_creature_script(CN_COLDFLAME, &ColdFlameAI::Create);
}
