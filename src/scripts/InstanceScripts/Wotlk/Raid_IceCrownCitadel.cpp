/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_IceCrownCitadel.h"
#include "Objects/Faction.h"
#include "Units/Summons/Summon.h"
#include <Objects/ObjectMgr.h>
#include <Management/TransporterHandler.h>
#include <Objects/Transporter.h>
#include "Movement/MovementGenerators/PointMovementGenerator.h"
#include "Server/Script/CreatureAIScript.h"

//////////////////////////////////////////////////////////////////////////////////////////
//ICC zone: 4812
//Prepared creature entry:
//
//CN_DEATHBRINGER_SAURFANG    37813
//CN_FESTERGUT                36626
//CN_ROTFACE                  36627
//CN_PROFESSOR_PUTRICIDE      36678
//CN_PRINCE_VALANAR           37970
//N_BLOOD_QUEEN_LANATHEL      37955
//CN_SINDRAGOSA               36853
//CN_THE_LICHKING             36597
//
///\todo  start boss scripts
//////////////////////////////////////////////////////////////////////////////////////////
//Event: GunshipBattle
//
//Affects:
//Available teleports. If GunshipBattle done -> Teleportlocation 4 available.
//
//Devnotes:
//Far away from implementing this :(
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//IceCrownCitadel Instance
class IceCrownCitadelScript : public InstanceScript
{
public:
    explicit IceCrownCitadelScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        Instance = (IceCrownCitadelScript*)pMapMgr->GetScript();
        TeamInInstance = 3;

        // Lord Marrowgar
        LordMarrowgar = nullptr;
        MarrowgarIcewall1GUID = 0;
        MarrowgarIcewall2GUID = 0;
        MarrowgarEntranceDoorGUID = 0;

        // Lady Deathwhisper
        LadyDeathwisper = nullptr;
        LadyDeathwisperElevatorGUID = 0;
        LadyDeathwisperEntranceDoorGUID = 0;

        // Gunship Battle
        skybreaker = nullptr;
        orgrimmar = nullptr;
        SkybreakerBoss = nullptr;
        OrgrimmarBoss = nullptr;
        DeathbringerSaurfangGb = nullptr;
        MuradinBronzebeardGb = nullptr;
        GbBattleMage = nullptr;
        isPrepared = false;
        addData(DATA_GUNSHIP_EVENT, NotStarted);

        // Deathbringer Saurfang
        DeathbringerDoorGUID = 0;
        DeathbringerSaurfang = nullptr;
        
        // Misc Data
        setLocalData(DATA_BONED_ACHIEVEMENT, uint32_t(true));
        setLocalData(DATA_TEAM_IN_INSTANCE, TeamInInstance);
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new IceCrownCitadelScript(pMapMgr); }

    uint32_t getLocalData(uint32_t type) const
    {
        switch (type)
        {
            // Deathbringer Suarfang
        case DATA_SAURFANG_DOOR:
            return DeathbringerDoorGUID;
        case DATA_TEAM_IN_INSTANCE:
            return TeamInInstance;
        }
        return 0;
    }

    Creature* getLocalCreatureData(uint32_t type) const
    {
        switch (type)
        {
            // Gunshipbattle
        case DATA_SKYBREAKER_BOSS:
            return SkybreakerBoss;
        case DATA_ORGRIMMAR_HAMMER_BOSS:
            return OrgrimmarBoss;
        case DATA_GB_HIGH_OVERLORD_SAURFANG:
            return DeathbringerSaurfangGb;
        case DATA_GB_MURADIN_BRONZEBEARD:
            return MuradinBronzebeardGb;
        case DATA_GB_BATTLE_MAGE:
            return GbBattleMage;
            // Deathbringer Saurfang
        case DATA_DEATHBRINGER_SAURFANG:
            return DeathbringerSaurfang;
        }
        return nullptr;
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            // Lord Marrowgar
        case CN_LORD_MARROWGAR:
            LordMarrowgar = pCreature;
            // Lady Deathwisper
        case CN_LADY_DEATHWHISPER:
            LadyDeathwisper = pCreature;
            // Gunship
        case NPC_GB_SKYBREAKER:
            SkybreakerBoss = pCreature;
            break;
        case NPC_GB_ORGRIMS_HAMMER:
            OrgrimmarBoss = pCreature;
            break;
        case NPC_GB_HIGH_OVERLORD_SAURFANG:
            DeathbringerSaurfangGb = pCreature;
            break;
        case NPC_GB_MURADIN_BRONZEBEARD:
            MuradinBronzebeardGb = pCreature;
            break;
        case NPC_GB_SKYBREAKER_SORCERERS:
        case NPC_GB_KORKRON_BATTLE_MAGE:
            GbBattleMage = pCreature;
            break;

            // Deathbringer Suarfang
        case CN_DEATHBRINGER_SAURFANG:
            DeathbringerSaurfang = pCreature;
            break;
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
        case GO_MARROWGAR_ICEWALL_1:
            MarrowgarIcewall1GUID = pGameObject->getGuidLow();
            break;
        case GO_MARROWGAR_ICEWALL_2:
            MarrowgarIcewall2GUID = pGameObject->getGuidLow();
            break;
        case GO_MARROWGAR_DOOR:
            MarrowgarEntranceDoorGUID = pGameObject->getGuidLow();
            break;
        case GO_ORATORY_OF_THE_DAMNED_ENTRANCE:
            LadyDeathwisperEntranceDoorGUID = pGameObject->getGuidLow();
            break;
        case GO_LADY_DEATHWHISPER_ELEVATOR:
            LadyDeathwisperElevatorGUID = pGameObject->getGuidLow();
            break;
        case GO_TELE_1:
        case GO_TELE_2:
        case GO_TELE_3:
        case GO_TELE_4:
        case GO_TELE_5:
            pGameObject->setFlags(GO_FLAG_NONE);
            break;
        case GO_SAURFANG_S_DOOR:
            DeathbringerDoorGUID = pGameObject->getGuidLow();
            break;
        }

        // State also must changes for Objects out of range so change state on spawning them :)
        SetGameobjectStates(pGameObject);
    }

    void SetGameobjectStates(GameObject* /*pGameObject*/)
    {
        // Gos which are not visible by killing a boss needs a second check...
        if (getData(CN_LORD_MARROWGAR) == Finished)
        {
            if (MarrowgarIcewall1GUID)
                if(GetGameObjectByGuid(MarrowgarIcewall1GUID))
                    GetGameObjectByGuid(MarrowgarIcewall1GUID)->setState(GO_STATE_OPEN);        // Icewall 1

            if (MarrowgarIcewall2GUID)
                if(GetGameObjectByGuid(MarrowgarIcewall2GUID))
                    GetGameObjectByGuid(MarrowgarIcewall2GUID)->setState(GO_STATE_OPEN);        // Icewall 2

            if (MarrowgarEntranceDoorGUID)
                if(GetGameObjectByGuid(MarrowgarEntranceDoorGUID))
                    GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_OPEN);    // Door  
        }

        if (getData(CN_LADY_DEATHWHISPER) == Finished)
        {
            if (LadyDeathwisperEntranceDoorGUID)
                if(GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID))
                    GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_OPEN);

            if (LadyDeathwisperElevatorGUID)
                if(GetGameObjectByGuid(LadyDeathwisperElevatorGUID))
                GetGameObjectByGuid(LadyDeathwisperElevatorGUID)->setState(GO_STATE_OPEN);
        }

        if (getData(CN_LADY_DEATHWHISPER) == NotStarted)
        {
            if (LadyDeathwisperEntranceDoorGUID)
                if(GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID))
                GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_OPEN);
        }

        if (getData(CN_DEATHBRINGER_SAURFANG) == NotStarted)
        {
            if (DeathbringerDoorGUID)
                if(GetGameObjectByGuid(DeathbringerDoorGUID))
                GetGameObjectByGuid(DeathbringerDoorGUID)->setState(GO_STATE_CLOSED);
        }

        if (getData(CN_DEATHBRINGER_SAURFANG) == Finished)
        {
            if (DeathbringerDoorGUID)
                if(GetGameObjectByGuid(DeathbringerDoorGUID))
                GetGameObjectByGuid(DeathbringerDoorGUID)->setState(GO_STATE_OPEN);
        }
    }

    void OnEncounterStateChange(uint32_t entry, uint32_t state) override
    {
        switch (entry)
        {
        case CN_LORD_MARROWGAR:
            if (state == InProgress)
            {
                if (MarrowgarEntranceDoorGUID)
                    if(GetGameObjectByGuid(MarrowgarEntranceDoorGUID))
                    GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_CLOSED);
            }
            if (state == NotStarted)
            {
                if (MarrowgarEntranceDoorGUID)
                    if(GetGameObjectByGuid(MarrowgarEntranceDoorGUID))
                    GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_OPEN);
            }
            if (state == Finished)
            {
                if (MarrowgarIcewall1GUID)
                    if(GetGameObjectByGuid(MarrowgarIcewall1GUID))
                    GetGameObjectByGuid(MarrowgarIcewall1GUID)->setState(GO_STATE_OPEN);        // Icewall 1

                if (MarrowgarIcewall2GUID)
                    if(GetGameObjectByGuid(MarrowgarIcewall2GUID))
                    GetGameObjectByGuid(MarrowgarIcewall2GUID)->setState(GO_STATE_OPEN);        // Icewall 2

                if (MarrowgarEntranceDoorGUID)
                    if(GetGameObjectByGuid(MarrowgarEntranceDoorGUID))
                    GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_OPEN);    // Door  
            }
            break;
        case CN_LADY_DEATHWHISPER:
            if (state == InProgress)
            {
                if (LadyDeathwisperEntranceDoorGUID)
                    if(GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID))
                    GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_CLOSED);
            }
            if (state == NotStarted)
            {
                if (LadyDeathwisperEntranceDoorGUID)
                    if(GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID))
                    GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_CLOSED);
            }
            if (state == Finished)
            {
                if (LadyDeathwisperEntranceDoorGUID)
                    if (GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID))
                    GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_OPEN);

                if (LadyDeathwisperElevatorGUID)
                    if (GetGameObjectByGuid(LadyDeathwisperElevatorGUID))
                    GetGameObjectByGuid(LadyDeathwisperElevatorGUID)->setState(GO_STATE_OPEN);
            }
            break;
        case CN_DEATHBRINGER_SAURFANG:
            {
            if (state == InProgress)
            {
                if (DeathbringerDoorGUID)
                    if (GetGameObjectByGuid(DeathbringerDoorGUID))
                        if(GetGameObjectByGuid(DeathbringerDoorGUID))
                    GetGameObjectByGuid(DeathbringerDoorGUID)->setState(GO_STATE_CLOSED);
            }

            if (state == NotStarted)
            {
                if (DeathbringerDoorGUID)
                    if (GetGameObjectByGuid(DeathbringerDoorGUID))
                    GetGameObjectByGuid(DeathbringerDoorGUID)->setState(GO_STATE_CLOSED);
            }

            if (state == Finished)
            {
                if (DeathbringerDoorGUID)
                    if (GetGameObjectByGuid(DeathbringerDoorGUID))
                    GetGameObjectByGuid(DeathbringerDoorGUID)->setState(GO_STATE_OPEN);
            }
                break;
            }
        }      
    }

    void OnAreaTrigger(Player* /*pPlayer*/, uint32 pAreaId)
    {
        switch(pAreaId)
        {
        case ICC_ENTRANCE:
            break;
        case ICC_LORD_MARROWGAR_ENTRANCE:
            if (LordMarrowgar)
                LordMarrowgar->GetScript()->DoAction(ACTION_MARROWGAR_INTRO_START);
            break;
        case ICC_LADY_DEATHWHISPER_ENTRANCE:
            if (LadyDeathwisper)
                LadyDeathwisper->GetScript()->DoAction(ACTION_LADY_INTRO_START);
            break;
        case ICC_DRAGON_HORDE:
            break;
        }
    }

    void SpawnEnemyGunship()
    {
        if (TeamInInstance == TEAM_ALLIANCE)
            orgrimmar = sTransportHandler.createTransport(GO_ORGRIM_S_HAMMER_ALLIANCE_ICC, mInstance);

        if (TeamInInstance == TEAM_HORDE)   
           skybreaker = sTransportHandler.createTransport(GO_THE_SKYBREAKER_HORDE_ICC, mInstance);
    }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }

    void OnPlayerEnter(Player* player) override
    {
        if (TeamInInstance == 3)
        {
            TeamInInstance = player->getTeam();
            setLocalData(DATA_TEAM_IN_INSTANCE, TeamInInstance);
        }

        if (!spawnsCreated())
        {
            // setup only the npcs with the correct team...
            switch (TeamInInstance)
            {
                case TEAM_ALLIANCE:
                    for (uint8_t i = 0; i < 18; i++)
                        spawnCreature(AllySpawns[i].entry, AllySpawns[i].x, AllySpawns[i].y, AllySpawns[i].z, AllySpawns[i].o, AllySpawns[i].faction);
                    break;
                case TEAM_HORDE:
                    for (uint8_t i = 0; i < 18; i++)
                        spawnCreature(HordeSpawns[i].entry, HordeSpawns[i].x, HordeSpawns[i].y, HordeSpawns[i].z, HordeSpawns[i].o, HordeSpawns[i].faction);
                    break;
            }
            setSpawnsCreated();
        }

        // Spawning the Gunships at the same moment a player enters causes them to bug the npcs sometimes
        if(!isPrepared)
            scriptEvents.addEvent(EVENT_SPAWN_GUNSHIPS, 5000);
    }    

    void UpdateEvent() override
    {
        scriptEvents.updateEvents(getUpdateFrequency(), 0);

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_SPAWN_GUNSHIPS:
            {
                if (getData(DATA_GUNSHIP_EVENT) == Finished)
                    return;

                if (!isPrepared)
                {
                    if (TeamInInstance == TEAM_ALLIANCE)
                        skybreaker = sTransportHandler.createTransport(GO_THE_SKYBREAKER_ALLIANCE_ICC, mInstance);

                    if (TeamInInstance == TEAM_HORDE)
                        orgrimmar = sTransportHandler.createTransport(GO_ORGRIM_S_HAMMER_HORDE_ICC, mInstance);

                    isPrepared = true;
                }
            }
                break;
            case EVENT_WIPE_CHECK:
                if (TeamInInstance == TEAM_ALLIANCE)
                {
                    DoCheckFallingPlayer(MuradinBronzebeardGb);
                    if (DoWipeCheck(skybreaker))
                        scriptEvents.addEvent(EVENT_WIPE_CHECK, 3000);
                    else
                        DoAction(ACTION_FAIL);
                }
                else
                {
                    DoCheckFallingPlayer(DeathbringerSaurfang);
                    if (DoWipeCheck(orgrimmar))
                        scriptEvents.addEvent(EVENT_WIPE_CHECK, 3000);
                    else
                        DoAction(ACTION_FAIL);
                }
                break;
            case EVENT_START_FLY:
                if (TeamInInstance == TEAM_ALLIANCE && skybreaker)
                    skybreaker->EnableMovement(true, mInstance);
                
                if (TeamInInstance == TEAM_HORDE && orgrimmar)
                    orgrimmar->EnableMovement(true, mInstance);
                break;
            }
        }
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
        case ACTION_INTRO_START:
            scriptEvents.addEvent(EVENT_START_FLY, 2500);
            break;
        case ACTION_BATTLE_EVENT:
            scriptEvents.addEvent(EVENT_WIPE_CHECK, 5000);
            setData(DATA_GUNSHIP_EVENT, InProgress);
            break;
        case ACTION_BATTLE_DONE:
            if (getData(DATA_GUNSHIP_EVENT) == Finished)
            {
                if (TeamInInstance == TEAM_ALLIANCE && skybreaker)
                {
                    skybreaker->EnableMovement(true, mInstance);

                    if (orgrimmar)
                        orgrimmar->EnableMovement(true, mInstance);
                }

                if (TeamInInstance == TEAM_HORDE && orgrimmar)
                {
                    orgrimmar->EnableMovement(true, mInstance);

                    if (skybreaker)
                        skybreaker->EnableMovement(true, mInstance);
                }
            }
            break;
        case ACTION_FAIL:
            // Handle Wipe Here
            break;
        }
    }

    void TransporterEvents(Transporter* transport, uint32_t eventId) override
    {
        switch (eventId)
        {
        case EVENT_ENEMY_GUNSHIP_DESPAWN:
            if (getData(DATA_GUNSHIP_EVENT) == Finished)
            {
                transport->UnloadStaticPassengers();
                transport->GetMapMgr()->RemoveFromMapMgr(transport, true);
            }
            break;
        case EVENT_ENEMY_GUNSHIP_COMBAT:
            if (Creature* captain = getLocalData(DATA_TEAM_IN_INSTANCE) == TEAM_HORDE ? getLocalCreatureData(DATA_GB_HIGH_OVERLORD_SAURFANG) : getLocalCreatureData(DATA_GB_MURADIN_BRONZEBEARD))
                captain->GetScript()->DoAction(ACTION_BATTLE_EVENT);
            // Instance
            transport->GetMapMgr()->GetScript()->DoAction(ACTION_BATTLE_EVENT);
            [[fallthrough]];
        case EVENT_PLAYERS_GUNSHIP_SPAWN:
        case EVENT_PLAYERS_GUNSHIP_COMBAT:
            transport->EnableMovement(false, mInstance);
            break;
        case EVENT_PLAYERS_GUNSHIP_SAURFANG:
            transport->EnableMovement(false, mInstance);
            break;
        }
    }

    //Wipe check
    bool DoWipeCheck(Transporter* /*t*/)
    {
        // todo
        return true;
    }

    //Check falling players
    void DoCheckFallingPlayer(Creature* pCreature)
    {
        if (pCreature)
        {
            auto const Players = mInstance->m_PlayerStorage;
            if (Players.size())
                for (auto itr = Players.begin(); itr != Players.end(); ++itr)
                    if (Player* pPlayer = itr->second)
                        if (pPlayer->GetPositionZ() < 420.0f && pPlayer->IsWithinDistInMap(pCreature, 300.0f))
                            pPlayer->Teleport(pCreature->GetPosition(), mInstance);
        }
    }

    void TransportBoarded(Unit* pUnit, Transporter* transport)
    {
        if (transport->getEntry() == GO_THE_SKYBREAKER_ALLIANCE_ICC)
            pUnit->castSpell(pUnit, SPELL_ON_SKYBREAKER_DECK, false);

        if (transport->getEntry() == GO_THE_SKYBREAKER_HORDE_ICC)
            pUnit->castSpell(pUnit, SPELL_ON_SKYBREAKER_DECK, false);

        if (transport->getEntry() == GO_ORGRIM_S_HAMMER_HORDE_ICC)
            pUnit->castSpell(pUnit, SPELL_ON_ORGRIMS_HAMMER_DECK, false);

        if (transport->getEntry() == GO_ORGRIM_S_HAMMER_ALLIANCE_ICC)
            pUnit->castSpell(pUnit, SPELL_ON_ORGRIMS_HAMMER_DECK, false);
    }

    void TransportUnboarded(Unit* pUnit, Transporter* transport)
    {
        if (transport->getEntry() == GO_THE_SKYBREAKER_ALLIANCE_ICC)
            pUnit->RemoveAura(SPELL_ON_SKYBREAKER_DECK);

        if (transport->getEntry() == GO_THE_SKYBREAKER_HORDE_ICC)
            pUnit->RemoveAura(SPELL_ON_SKYBREAKER_DECK);

        if (transport->getEntry() == GO_ORGRIM_S_HAMMER_HORDE_ICC)
            pUnit->RemoveAura(SPELL_ON_ORGRIMS_HAMMER_DECK);

        if (transport->getEntry() == GO_ORGRIM_S_HAMMER_ALLIANCE_ICC)
            pUnit->RemoveAura(SPELL_ON_ORGRIMS_HAMMER_DECK);
    }

public:
    Transporter* skybreaker;
    Transporter* orgrimmar;

protected:
    IceCrownCitadelScript* Instance;
    uint8_t TeamInInstance;

    // Marrowgar
    Creature* LordMarrowgar;
    uint32_t MarrowgarIcewall1GUID;
    uint32_t MarrowgarIcewall2GUID;
    uint32_t MarrowgarEntranceDoorGUID;

    // Lady Deathwhisper
    Creature* LadyDeathwisper;
    uint32_t LadyDeathwisperElevatorGUID;
    uint32_t LadyDeathwisperEntranceDoorGUID;

    // Gunship Event			
    Creature* SkybreakerBoss;
    Creature* OrgrimmarBoss;
    Creature* DeathbringerSaurfangGb;
    Creature* MuradinBronzebeardGb;
    Creature* GbBattleMage;
    bool isPrepared;

    // Deathbringer Saurfang
    uint32_t DeathbringerDoorGUID;
    Creature* DeathbringerSaurfang;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// IceCrown Teleporter
class ICCTeleporterGossip : public GossipScript
{
public:
    void onHello(Object* object, Player* player) override
    {
        InstanceScript* pInstance = player->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        GossipMenu menu(object->getGuid(), 15221, player->GetSession()->language);
        menu.addItem(GOSSIP_ICON_CHAT, 515, 0);     // Teleport to Light's Hammer.

        if (pInstance->getData(CN_LORD_MARROWGAR) == Finished)
            menu.addItem(GOSSIP_ICON_CHAT, 516, 1);      // Teleport to Oratory of The Damned.

        if (pInstance->getData(CN_LADY_DEATHWHISPER) == Finished)
            menu.addItem(GOSSIP_ICON_CHAT, 517, 2);      // Teleport to Rampart of Skulls.

        // GunshipBattle has to be finished...
        if (pInstance->getData(DATA_GUNSHIP_EVENT) == Finished)
        menu.addItem(GOSSIP_ICON_CHAT, (518), 3);        // Teleport to Deathbringer's Rise.

        if (pInstance->getData(CN_VALITHRIA_DREAMWALKER) == Finished)
            menu.addItem(GOSSIP_ICON_CHAT, 519, 4);      // Teleport to the Upper Spire.

        if (pInstance->getData(NPC_COLDFLAME) == Finished)
            menu.addItem(GOSSIP_ICON_CHAT, 520, 5);      // Teleport to Sindragosa's Lair.

        menu.sendGossipPacket(player);
    }

    void onSelectOption(Object* /*object*/, Player* player, uint32_t Id, const char* /*enteredcode*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 0:
                player->castSpell(player, 70781, true);     // Light's Hammer
                break;
            case 1:
                player->castSpell(player, 70856, true);     // Oratory of The Damned
                break;
            case 2:
                player->castSpell(player, 70857, true);     // Rampart of Skulls
                break;
            case 3:
                player->castSpell(player, 70858, true);     // Deathbringer's Rise
                break;
            case 4:
                player->castSpell(player, 70859, true);     // Upper Spire
                break;
            case 5:
                player->castSpell(player, 70861, true);     // Sindragosa's Lair
                break;
        }
        GossipMenu::senGossipComplete(player);
    }
};

class ICCTeleporterAI : public GameObjectAIScript
{
public:
    explicit ICCTeleporterAI(GameObject* go) : GameObjectAIScript(go) {}

    ~ICCTeleporterAI() {}

    static GameObjectAIScript* Create(GameObject* go) { return new ICCTeleporterAI(go); }

    void OnActivate(Player* player) override
    {
        ICCTeleporterGossip gossip;
        gossip.onHello(_gameobject, player);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Lord Marrowgar
class LordMarrowgarAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LordMarrowgarAI(c); }
    explicit LordMarrowgarAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        boneStormDuration = RAID_MODE<uint32_t>(20000, 30000, 20000, 30000);
        baseSpeed = pCreature->getSpeedRate(TYPE_RUN, false);
        introDone = false;
        boneSlice = false;
        boneStormtarget = nullptr;
        coldflameLastPos = getCreature()->GetPosition();

        // Scripted Spells not autocastet
        boneSliceSpell = addAISpell(SPELL_BONE_SLICE, 0.0f, TARGET_ATTACKING);
        boneStormSpell = addAISpell(SPELL_BONE_STORM, 0.0f, TARGET_SELF);
        boneStormSpell->addDBEmote(SAY_MARR_BONE_STORM_EMOTE);

        boneSpikeGraveyardSpell = addAISpell(SPELL_BONE_SPIKE_GRAVEYARD, 0.0f, TARGET_SELF);
        coldflameNormalSpell = addAISpell(SPELL_COLDFLAME_NORMAL, 0.0f, TARGET_SELF);
        coldflameBoneStormSpell = addAISpell(SPELL_COLDFLAME_BONE_STORM, 0.0f, TARGET_SELF);

        berserkSpell = addAISpell(SPELL_BERSERK, 0.0f, TARGET_SELF);
        berserkSpell->addDBEmote(SAY_MARR_BERSERK);                  // THE MASTER'S RAGE COURSES THROUGH ME!
        berserkSpell->mIsTriggered = true;

        // Messages
        addEmoteForEvent(Event_OnCombatStart, SAY_MARR_AGGRO);     // The Scourge will wash over this world as a swarm of death and destruction!
        addEmoteForEvent(Event_OnTargetDied, SAY_MARR_KILL_1);      // More bones for the offering!
        addEmoteForEvent(Event_OnTargetDied, SAY_MARR_KILL_2);      // Languish in damnation!
        addEmoteForEvent(Event_OnDied, SAY_MARR_DEATH);            // I see... Only darkness.
    }

    void IntroStart()
    {
        sendDBChatMessage(SAY_MARR_ENTER_ZONE);      // This is the beginning AND the end, mortals. None may enter the master's sanctum!
        introDone = true;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        // common events
        scriptEvents.addEvent(EVENT_ENABLE_BONE_SLICE, 10000);
        scriptEvents.addEvent(EVENT_BONE_SPIKE_GRAVEYARD, Util::getRandomInt(10000, 15000));
        scriptEvents.addEvent(EVENT_COLDFLAME, 5000);
        scriptEvents.addEvent(EVENT_WARN_BONE_STORM, Util::getRandomInt(45000, 50000));
        scriptEvents.addEvent(EVENT_ENRAGE, 600000);
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        Reset();
    }

    void Reset()
    {
        scriptEvents.resetEvents();

        getCreature()->setSpeedRate(TYPE_RUN, baseSpeed, true);
        getCreature()->RemoveAura(SPELL_BONE_STORM);
        getCreature()->RemoveAura(SPELL_BERSERK);

        boneSlice = false;
        boneSpikeImmune.clear();
    }

    void AIUpdate() override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        if (_isCasting())
            return;

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_BONE_SPIKE_GRAVEYARD:
                if (_isHeroic() || !getCreature()->hasAurasWithId(SPELL_BONE_STORM))
                    _castAISpell(boneSpikeGraveyardSpell);

                scriptEvents.addEvent(EVENT_BONE_SPIKE_GRAVEYARD, Util::getRandomInt(15000, 20000));
                break;
            case EVENT_COLDFLAME:
                coldflameLastPos = getCreature()->GetPosition();

                if (!getCreature()->hasAurasWithId(SPELL_BONE_STORM))
                    _castAISpell(coldflameNormalSpell);
                else
                    _castAISpell(coldflameBoneStormSpell);
                
                scriptEvents.addEvent(EVENT_COLDFLAME, 5000);
                break;
            case EVENT_WARN_BONE_STORM:
                boneSlice = false;
                _castAISpell(boneStormSpell);

                scriptEvents.delayEvent(EVENT_BONE_SPIKE_GRAVEYARD, 3000);
                scriptEvents.delayEvent(EVENT_COLDFLAME, 3000);

                scriptEvents.addEvent(EVENT_BONE_STORM_BEGIN, 3050);
                scriptEvents.addEvent(EVENT_WARN_BONE_STORM, Util::getRandomInt(90000, 95000));
            case EVENT_BONE_STORM_BEGIN:
                getCreature()->setSpeedRate(TYPE_RUN, baseSpeed*3.0f, true);
                sendDBChatMessage(SAY_MARR_BONE_STORM); // BONE STORM!
                
                scriptEvents.addEvent(EVENT_BONE_STORM_END, boneStormDuration + 1);
            case EVENT_BONE_STORM_MOVE:
            {
                scriptEvents.addEvent(EVENT_BONE_STORM_MOVE, boneStormDuration / 3);

                boneStormtarget= getBestPlayerTarget(TargetFilter_NotCurrent);
                if (!boneStormtarget)
                    boneStormtarget = getBestPlayerTarget(TargetFilter_Current);
                
                if (boneStormtarget)
                    getCreature()->getMovementManager()->movePoint(POINT_TARGET_BONESTORM_PLAYER, boneStormtarget->GetPosition());

                break;
            }
            case EVENT_BONE_STORM_END:
                if (MovementGenerator* movement = getCreature()->getMovementManager()->getMovementGenerator([](MovementGenerator const* a) -> bool
                {
                    if (a->getMovementGeneratorType() == POINT_MOTION_TYPE)
                    {
                        PointMovementGenerator<Creature> const* pointMovement = dynamic_cast<PointMovementGenerator<Creature> const*>(a);
                        return pointMovement && pointMovement->getId() == POINT_TARGET_BONESTORM_PLAYER;
                    }
                    return false;
                }))
                    getCreature()->getMovementManager()->remove(movement);

                getCreature()->getMovementManager()->moveChase(getCreature()->getAIInterface()->getCurrentTarget());

                getCreature()->setSpeedRate(TYPE_RUN, baseSpeed, true);
                scriptEvents.removeEvent(EVENT_BONE_STORM_MOVE);
                scriptEvents.addEvent(EVENT_ENABLE_BONE_SLICE, 10000);

                if (!_isHeroic())
                    scriptEvents.addEvent(EVENT_BONE_SPIKE_GRAVEYARD, Util::getRandomInt(15000, 20000));
                break;
            case EVENT_ENABLE_BONE_SLICE:
                boneSlice = true;
                break;
            case EVENT_ENRAGE:
                _castAISpell(berserkSpell);
                break;
            }
        }
     
        // We should not melee attack when storming
        if (getCreature()->hasAurasWithId(SPELL_BONE_STORM))
        {
            _setMeleeDisabled(true);
            return;
        }

        _setMeleeDisabled(false);

        // 10 seconds since encounter start Bone Slice replaces melee attacks
        if (boneSlice)
        {
            _castAISpell(boneSliceSpell);
        }

    }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != POINT_MOTION_TYPE || iWaypointId != POINT_TARGET_BONESTORM_PLAYER)
            return;

        // lock movement
        getCreature()->getMovementManager()->moveIdle();
    }

    LocationVector const* GetLastColdflamePosition() const
    {
        return &coldflameLastPos;
    }

    void SetLastColdflamePosition(LocationVector pos)
    {
        coldflameLastPos = pos;
    }

    void SetCreatureData64(uint32 Type, uint64 Data) override
    {
        switch (Type)
        {
        case DATA_COLDFLAME_GUID:
            coldflameTarget = Data;
            break;
        case DATA_SPIKE_IMMUNE:
            boneSpikeImmune.push_back(Data);
            break;
        }
    }

    uint64 GetCreatureData64(uint32 Type) const
    { 
        switch (Type)
        {
        case DATA_COLDFLAME_GUID:
            return coldflameTarget;
        case DATA_SPIKE_IMMUNE + 0:
        case DATA_SPIKE_IMMUNE + 1:
        case DATA_SPIKE_IMMUNE + 2:
        {
            uint32_t index = Type - DATA_SPIKE_IMMUNE;
            if (index < boneSpikeImmune.size())
                return boneSpikeImmune[index];

            break;
        }
        }

        return 0;
    }

    void DoAction(int32_t const action) override
    {
        if (action == ACTION_CLEAR_SPIKE_IMMUNITIES)
            boneSpikeImmune.clear();

        if (action == ACTION_MARROWGAR_INTRO_START)
            IntroStart();
    }

protected:
    // Common
    InstanceScript* mInstance;
    float baseSpeed;
    bool introDone;
    bool boneSlice;

    Unit* boneStormtarget;
    LocationVector coldflameLastPos;
    uint64_t coldflameTarget;
    std::vector<uint64_t> boneSpikeImmune;

    // Spells
    CreatureAISpells* boneSliceSpell;
    CreatureAISpells* boneStormSpell;
    CreatureAISpells* boneSpikeGraveyardSpell;
    CreatureAISpells* coldflameNormalSpell;
    CreatureAISpells* coldflameBoneStormSpell;
    CreatureAISpells* berserkSpell;

    uint32_t boneStormDuration;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Cold Flame
class ColdflameAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ColdflameAI(c); }
    explicit ColdflameAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();     
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->getAIInterface()->setAiScriptType(AI_SCRIPT_PASSIVE);
        coldflameTriggerSpell = addAISpell(SPELL_COLDFLAME_SUMMON, 0.0f, TARGET_SOURCE);
        coldflameTriggerSpell->mIsTriggered = true;
    }

    void OnLoad() override
    {
        getCreature()->setVisible(false);
    }

    void OnSummon(Unit* summoner) override
    {
        if (!mInstance || !summoner->isCreature())
            return;

        if (summoner->hasAurasWithId(SPELL_BONE_STORM))
        {
            // Bonestorm X Pattern
            if (LordMarrowgarAI* marrowgarAI = static_cast<LordMarrowgarAI*>(static_cast<Creature*>(summoner)->GetScript()))
            {
                LocationVector const* ownerPos = marrowgarAI->GetLastColdflamePosition();
                float angle = ownerPos->o / M_PI_FLOAT * 180.0f;
                MoveTeleport(ownerPos->x , ownerPos->y, ownerPos->z, ownerPos->o);
                // Store last Coldflame Position and add 90 degree to create x pattern
                LocationVector nextPos;
                nextPos.x = ownerPos->x;
                nextPos.y = ownerPos->y;
                nextPos.z = ownerPos->z;
                nextPos.o = angle + 90 * M_PI_FLOAT / 180.0f;

                marrowgarAI->SetLastColdflamePosition(nextPos);
            }
        }
        // Random target Case
        else
        {
            Unit* target = mInstance->GetInstance()->GetUnit(static_cast<Creature*>(summoner)->GetScript()->GetCreatureData64(DATA_COLDFLAME_GUID));
            if (!target)
            {
                getCreature()->Despawn(100, 0);
                return;
            }        
            MoveTeleport(summoner->GetPosition());

            getCreature()->SetOrientation(getCreature()->calcAngle(summoner->GetPositionX(), summoner->GetPositionY(), target->GetPositionX(), target->GetPositionY()) * M_PI_FLOAT / 180.0f);
        }        
        MoveTeleport(summoner->GetPositionX(), summoner->GetPositionY(),summoner->GetPositionZ(), getCreature()->GetOrientation());
        scriptEvents.addEvent(EVENT_COLDFLAME_TRIGGER, 500);
    }

    void AIUpdate() override
    {
        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        if (scriptEvents.getFinishedEvent() == EVENT_COLDFLAME_TRIGGER)
        {
            LocationVector newPos;
            newPos = getCreature()->GetPosition();

            float angle = newPos.o;
            float destx, desty, destz;
            destx = newPos.x + 5.0f * std::cos(angle);
            desty = newPos.y + 5.0f * std::sin(angle);
            destz = newPos.z;

            newPos.x = destx;
            newPos.y = desty;
            newPos.z = destz;

            MoveTeleport(newPos.x, newPos.y, newPos.z, newPos.o);
            _castAISpell(coldflameTriggerSpell);         
            scriptEvents.addEvent(EVENT_COLDFLAME_TRIGGER, 500);
        }
    }

protected:
    // Common
    InstanceScript* mInstance;

    //Spells
    CreatureAISpells* coldflameTriggerSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Bone Spike
class BoneSpikeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BoneSpikeAI(c); }
    explicit BoneSpikeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        // Common
        hasTrappedUnit = false;
        summon = nullptr;

        getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
    }

    void OnSummon(Unit* summoner) override
    {
        summon = summoner;
        // Make our Creature in Combat otherwise on Died Script wont trigger
        getCreature()->getAIInterface()->setAiScriptType(AI_SCRIPT_AGRO);

        getCreature()->castSpell(summoner, SPELL_IMPALED, false);
        summoner->castSpell(getCreature(), SPELL_RIDE_VEHICLE, true);
        scriptEvents.addEvent(EVENT_FAIL_BONED, 8000);
        hasTrappedUnit = true;
    }

    void OnTargetDied(Unit* pTarget) override
    {
        getCreature()->Despawn(100, 0);
        pTarget->RemoveAura(SPELL_IMPALED);
    }

    void OnDied(Unit* /*pTarget*/) override
    {       
        if (summon)
            summon->RemoveAura(SPELL_IMPALED);
         
        getCreature()->Despawn(100, 0);
    }

    void AIUpdate() override
    {
        if (!hasTrappedUnit)
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        if (scriptEvents.getFinishedEvent() == EVENT_FAIL_BONED)
            if (mInstance)
                mInstance->setLocalData(DATA_BONED_ACHIEVEMENT, uint32_t(false));
    }

protected:
    // Common
    InstanceScript* mInstance;

    // Summon
    Unit* summon;

    bool hasTrappedUnit;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Storm
class BoneStorm : public SpellScript
{
public:
    void onAuraCreate(Aura* aur) override
    {
        // set duration here
        int32_t duration = 20000;
        if(aur->GetUnitCaster()->isCreature())
            duration = static_cast<Creature*>(aur->GetUnitCaster())->GetScript()->RAID_MODE<uint32_t>(20000, 30000, 20000, 30000);

        aur->setOriginalDuration(duration);
        aur->setMaxDuration(duration);
        aur->setTimeLeft(duration);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Storm Damage
class BoneStormDamage : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg) override
    {
        if (effIndex != EFF_INDEX_0 || spell->GetUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        auto distance = spell->GetUnitTarget()->GetDistance2dSq(spell->getCaster());
        // If target is closer than 5 yards, do full damage
        if (distance <= 5.0f)
            distance = 1.0f;

        *dmg = float2int32(*dmg / distance);
        return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Spike Graveyard
class BoneSpikeGraveyard : public SpellScript
{
public:
    void onAuraApply(Aura* aur) override
    {
        aur->removeAuraEffect(EFF_INDEX_1);

        if (Creature* marrowgar = static_cast<Creature*>(aur->GetUnitCaster()))
        {
            CreatureAIScript* marrowgarAI = marrowgar->GetScript();
            if (!marrowgarAI)
                return;

            uint8_t boneSpikeCount = uint8_t(aur->GetUnitCaster()->GetMapMgr()->pInstance->m_difficulty & 1 ? 3 : 1);
            std::list<Unit*> targets;

            targets.clear();

            for (uint8_t target = 0; target < boneSpikeCount; ++target)
                targets.push_back(GetRandomTargetNotMainTank(marrowgar));

            if (targets.empty())
                return;

            uint32 i = 0;
            for (std::list<Unit*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr, ++i)
            {
                Unit* target = *itr;
                target->castSpell(target, BoneSpikeSummonId[i], true);
            }


            std::vector<uint32_t> emoteVector;
            emoteVector.clear();
            emoteVector.push_back(SAY_MARR_BONESPIKE_1);// Stick Around
            emoteVector.push_back(SAY_MARR_BONESPIKE_2);// The only Escape is Darkness 
            emoteVector.push_back(SAY_MARR_BONESPIKE_3);// More Bones for the offering

            marrowgarAI->sendRandomDBChatMessage(emoteVector, nullptr);
        }
    }

    SpellCastResult onCanCast(Spell* spell, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/) override
    {          
        if (Creature* marrowgar = static_cast<Creature*>(spell->getUnitCaster()))

            if (GetRandomTargetNotMainTank(marrowgar))
                return SpellCastResult::SPELL_CAST_SUCCESS;

        return SpellCastResult::SPELL_FAILED_DONT_REPORT;
    }

    Unit* GetRandomTargetNotMainTank(Creature* caster)
    {
        Unit* target = nullptr;
        std::vector<Player*> players;

        Unit* mt = caster->getAIInterface()->getCurrentTarget();
        if (mt == nullptr || !mt->isPlayer())
            return 0;

        for (const auto& itr : caster->getInRangePlayersSet())
        {
            Player* obj = static_cast<Player*>(itr);
            if (obj != mt && CheckTarget(obj, caster->GetScript()))
                players.push_back(obj);
        }

        if (players.size())
            target = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];

        return target;
    }

    bool CheckTarget(Unit* target, CreatureAIScript* creatureAI)
    {
        if (target->getObjectTypeId() != TYPEID_PLAYER)
            return false;

        if (target->hasAurasWithId(SPELL_IMPALED))
            return false;

        // Check if it is one of the tanks soaking Bone Slice
        for (uint32 i = 0; i < MAX_BONE_SPIKE_IMMUNE; ++i)
            if (target->getGuid() == creatureAI->GetCreatureData64(DATA_SPIKE_IMMUNE + i))
                return false;

        return true;
    }        
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame
class Coldflame : public SpellScript
{
public:
    void filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets) override
    {
        if (effectIndex != EFF_INDEX_0)
            return;

        effectTargets->clear();

        Unit* coldflametarget = nullptr;
        Creature* pCreature = nullptr;

        if (spell->getUnitCaster()->isCreature())
            pCreature = static_cast<Creature*>(spell->getUnitCaster());

        // select any unit but not the tank 
        if (pCreature)
        {
            coldflametarget = pCreature->GetScript()->getBestPlayerTarget(TargetFilter_NotCurrent);
            if (!coldflametarget)
                coldflametarget = pCreature->GetScript()->getBestPlayerTarget(TargetFilter_Current);

            if (coldflametarget)
            {
                pCreature->GetScript()->SetCreatureData64(DATA_COLDFLAME_GUID, coldflametarget->getGuid());
                effectTargets->push_back(coldflametarget->getGuid());
            }
        }
    }

    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effectIndex) override
    {
        if (effectIndex != EFF_INDEX_0)
            return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

        spell->getUnitCaster()->castSpell(spell->GetUnitTarget(), spell->damage, true);

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame Bone Storm
class ColdflameBonestorm : public SpellScript
{
public:
    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effectIndex) override
    {
        if (effectIndex != EFF_INDEX_0)
            return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

        for (uint8 i = 0; i < 4; ++i)
            spell->getUnitCaster()->castSpell(spell->GetUnitTarget(), (spell->damage + i), true);

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame Damage
class ColdflameDamage: public SpellScript
{
public:
    void filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets) override
    {
        if (effectIndex != EFF_INDEX_0)
            return;

        effectTargets->clear();

        std::vector<Player*> players;
        for (const auto& itr : spell->getUnitCaster()->getInRangePlayersSet())
        {
            auto target = static_cast<Player*>(itr);

            if (CanBeAppliedOn(target, spell))
                effectTargets->push_back(target->getGuid());
        }
    }

    bool CanBeAppliedOn(Unit* target, Spell* spell)
    {
        if (target->hasAurasWithId(SPELL_IMPALED))
            return false;

        if (target->GetDistance2dSq(spell->getUnitCaster()) > static_cast<float>(spell->getSpellInfo()->getEffectRadiusIndex(EFF_INDEX_0)) )
            return false;

        return true;
    }

    SpellScriptExecuteState beforeSpellEffect(Spell* /*spell*/, uint8_t effectIndex) override
    {
        if (effectIndex == EFF_INDEX_2)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        return SpellScriptExecuteState::EXECUTE_OK;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Slice
class BoneSlice : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg) override
    {
        if (effIndex != EFF_INDEX_0 || spell->GetUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        if (!targetCount)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;
        
        *dmg = float2int32(*dmg / (float)targetCount);
        return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
    }

    SpellCastResult onCanCast(Spell* spell, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/) override
    {
        targetCount = 0;
        static_cast<Creature*>(spell->getUnitCaster())->GetScript()->DoAction(ACTION_CLEAR_SPIKE_IMMUNITIES);

        return SpellCastResult::SPELL_CAST_SUCCESS;
    }

    void filterEffectTargets(Spell* /*spell*/, uint8_t effectIndex, std::vector<uint64_t>* effectTargets) override
    {
        if (effectIndex != EFF_INDEX_0)
            return;

        targetCount = static_cast<uint32_t>(effectTargets->size());
    }

    void afterSpellEffect(Spell* spell, uint8_t effIndex) override
    {
        if (effIndex != EFF_INDEX_0)
            return;

        if (spell->GetUnitTarget())
        {
            static_cast<Creature*>(spell->getUnitCaster())->GetScript()->SetCreatureData64(DATA_SPIKE_IMMUNE, spell->GetUnitTarget()->getGuid());
        }
    }

    uint32_t targetCount;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Lady Deathwhisper
class LadyDeathwhisperAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LadyDeathwhisperAI(c); }
    explicit LadyDeathwhisperAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();
       
        dominateMindCount = RAID_MODE<uint8_t>(0, 1, 1, 3);
        introDone = false;

        // Scripted Spells not autocastet
        shadowChannelingSpell           = addAISpell(SPELL_SHADOW_CHANNELING, 0.0f, TARGET_SELF);
        manaBarrierSpell                = addAISpell(SPELL_MANA_BARRIER, 0.0f, TARGET_SELF);
        manaBarrierSpell->mIsTriggered = true;
        deathAndDecaySpell              = addAISpell(SPELL_DEATH_AND_DECAY, 0.0f, TARGET_CUSTOM);
        dominateMindHeroSpell           = addAISpell(SPELL_DOMINATE_MIND_H, 0.0f, TARGET_CUSTOM);
        shadowBoltSpell                 = addAISpell(SPELL_SHADOW_BOLT, 0.0f, TARGET_CUSTOM);
        frostBoltSpell                  = addAISpell(SPELL_FROSTBOLT, 0.0f, TARGET_RANDOM_SINGLE);
        frostBoltVolleySpell            = addAISpell(SPELL_FROSTBOLT_VOLLEY, 0.0f, TARGET_ATTACKING);
        touchOfInsignifcanceSpell       = addAISpell(SPELL_TOUCH_OF_INSIGNIFICANCE, 0.0f, TARGET_ATTACKING);
        summonShadeSpell                = addAISpell(SPELL_SUMMON_SHADE, 0.0f, TARGET_CUSTOM);
        berserkSpell                    = addAISpell(SPELL_BERSERK, 0.0f, TARGET_SELF);
        darkMartydromSpell              = addAISpell(SPELL_DARK_MARTYRDOM_T, 0.0f, TARGET_CUSTOM);
        darkMartydromSpell->addDBEmote(SAY_LADY_DEAD);
        darkTransformationSpell         = addAISpell(SPELL_DARK_TRANSFORMATION_T, 0.0f, TARGET_CUSTOM);
        darkTransformationSpell->mIsTriggered = true;
        darkTransformationSpell->addDBEmote(SAY_LADY_TRANSFORMATION);
        darkEmpowermentSpell            = addAISpell(SPELL_DARK_EMPOWERMENT_T, 0.0f, TARGET_CUSTOM);
        darkEmpowermentSpell->mIsTriggered = true;
        darkEmpowermentSpell->addDBEmote(SAY_LADY_EMPOWERMENT);
              

        // Messages
        addEmoteForEvent(Event_OnCombatStart, SAY_LADY_AGGRO);     
        addEmoteForEvent(Event_OnTargetDied, SAY_LADY_DEAD);            
        addEmoteForEvent(Event_OnDied, SAY_LADY_DEATH);            
    }

    void IntroStart()
    {
        if (!_isInCombat() && getScriptPhase() >= PHASE_INTRO)
            return;

        ///\todo Add SpellImmunities
        sendDBChatMessage(SAY_LADY_INTRO_1);
        setScriptPhase(PHASE_INTRO);
        scriptEvents.addEvent(EVENT_INTRO_2, 11000, PHASE_INTRO);
        scriptEvents.addEvent(EVENT_INTRO_3, 21000, PHASE_INTRO);
        scriptEvents.addEvent(EVENT_INTRO_4, 31500, PHASE_INTRO);
        scriptEvents.addEvent(EVENT_INTRO_5, 39500, PHASE_INTRO);
        scriptEvents.addEvent(EVENT_INTRO_6, 48500, PHASE_INTRO);
        scriptEvents.addEvent(EVENT_INTRO_7, 58000, PHASE_INTRO);
        _castAISpell(shadowChannelingSpell);
        introDone = true;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();       
        setScriptPhase(PHASE_ONE);

        // common events
        scriptEvents.addEvent(EVENT_BERSERK, 600000);
        scriptEvents.addEvent(EVENT_DEATH_AND_DECAY, 10000);

        // phase one events
        scriptEvents.addEvent(EVENT_P1_SUMMON_WAVE, 5000, PHASE_ONE);
        scriptEvents.addEvent(EVENT_P1_SHADOW_BOLT, Util::getRandomUInt(5500, 6000), PHASE_ONE);
        scriptEvents.addEvent(EVENT_P1_EMPOWER_CULTIST, Util::getRandomUInt(20000, 30000), PHASE_ONE);

        if (mInstance->GetDifficulty() != InstanceDifficulty::RAID_10MAN_NORMAL)
            scriptEvents.addEvent(EVENT_DOMINATE_MIND_H, 27000);

        getCreature()->getAIInterface()->setAiState(AI_STATE_IDLE);
        _setMeleeDisabled(true);
        setRooted(true);

        getCreature()->RemoveAura(SPELL_SHADOW_CHANNELING);
        _castAISpell(manaBarrierSpell);
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        Reset();
    }

    void DoAction(int32_t const action) override
    {
        if (action == ACTION_LADY_INTRO_START)
            IntroStart();

        if (action == ACTION_MANABARRIER_DOWN)
        {
            // When Lady Deathwhsiper has her mana Barrier dont deal damage to her instead reduce her mana.
            // phase transition
            if (getScriptPhase() == PHASE_ONE)
            {
                sendDBChatMessage(SAY_LADY_PHASE_2);
                sendDBChatMessage(SAY_LADY_PHASE_2_EMOTE);
                setRooted(false);
                getCreature()->setPower(POWER_TYPE_MANA, 0);
                getCreature()->getMovementManager()->moveChase(getCreature()->getAIInterface()->getCurrentTarget());
                setScriptPhase(PHASE_TWO);
                scriptEvents.addEvent(EVENT_P2_FROSTBOLT, Util::getRandomUInt(10000, 12000), PHASE_TWO);
                scriptEvents.addEvent(EVENT_P2_FROSTBOLT_VOLLEY, Util::getRandomUInt(19000, 21000), PHASE_TWO);
                scriptEvents.addEvent(EVENT_P2_TOUCH_OF_INSIGNIFICANCE, Util::getRandomUInt(6000, 9000), PHASE_TWO);
                scriptEvents.addEvent(EVENT_P2_SUMMON_SHADE, Util::getRandomUInt(12000, 15000), PHASE_TWO);
                // on heroic mode Lady Deathwhisper is immune to taunt effects in phase 2 and continues summoning adds
                if (_isHeroic())
                {
                    ///\todo Add SpellImmunities
                    scriptEvents.addEvent(EVENT_P2_SUMMON_WAVE, 45000, PHASE_TWO);
                }
            }
        }
    }

    void Reset()
    {
        getCreature()->setPower(POWER_TYPE_MANA, getCreature()->getMaxPower(POWER_TYPE_MANA));

        ///\todo Add SpellImmunities
        setScriptPhase(PHASE_ONE);
        scriptEvents.resetEvents();

        waveCounter = 0;
        nextVengefulShadeTargetGUID = 0;

        DeleteSummons();

        _castAISpell(shadowChannelingSpell);
        getCreature()->RemoveAllAuras();
    }

    void AIUpdate() override
    {
        if (!_isInCombat() && !getScriptPhase() == PHASE_INTRO)
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        if (getCreature()->getAIInterface()->getAiState() == AI_STATE_CASTING)
            return;

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_INTRO_2:
                sendDBChatMessage(SAY_LADY_INTRO_2);
                break;
            case EVENT_INTRO_3:
                sendDBChatMessage(SAY_LADY_INTRO_3);
                break;
            case EVENT_INTRO_4:
                sendDBChatMessage(SAY_LADY_INTRO_4);
                break;
            case EVENT_INTRO_5:
                sendDBChatMessage(SAY_LADY_INTRO_5);
                break;
            case EVENT_INTRO_6:
                sendDBChatMessage(SAY_LADY_INTRO_6);
                break;
            case EVENT_INTRO_7:
                sendDBChatMessage(SAY_LADY_INTRO_7);
                setScriptPhase(PHASE_ONE);
                break;
            case EVENT_DEATH_AND_DECAY:
                if (Unit* target = getBestPlayerTarget())
                {
                    deathAndDecaySpell->setCustomTarget(target);
                    _castAISpell(deathAndDecaySpell);
                }
                scriptEvents.addEvent(EVENT_DEATH_AND_DECAY, Util::getRandomUInt(22000, 30000));
                break;
            case EVENT_DOMINATE_MIND_H:
                sendDBChatMessage(SAY_LADY_DOMINATE_MIND);
                for (uint8 i = 0; i < dominateMindCount; i++)
                    if (Unit* target = getBestPlayerTarget(TargetFilter_NotCurrent))
                    {
                        dominateMindHeroSpell->setCustomTarget(target);
                        _castAISpell(dominateMindHeroSpell);
                    }
                scriptEvents.addEvent(EVENT_DOMINATE_MIND_H, Util::getRandomUInt(40000, 45000));
                break;
            case EVENT_P1_SUMMON_WAVE:
                SummonWavePhaseOne();
                scriptEvents.addEvent(EVENT_P1_SUMMON_WAVE, _isHeroic() ? 45000 : 60000, PHASE_ONE);
                break;
            case EVENT_P1_SHADOW_BOLT:
                if (Unit* target = getBestPlayerTarget())
                {
                    shadowBoltSpell->setCustomTarget(target);
                    _castAISpell(shadowBoltSpell);
                }
                scriptEvents.addEvent(EVENT_P1_SHADOW_BOLT, Util::getRandomUInt(5000, 8000), PHASE_ONE);
                break;
            case EVENT_P1_REANIMATE_CULTIST:
                ReanimateCultist();
                break;
            case EVENT_P1_EMPOWER_CULTIST:
                EmpowerCultist();
                scriptEvents.addEvent(EVENT_P1_EMPOWER_CULTIST, Util::getRandomUInt(18000, 25000));
                break;
            case EVENT_P2_FROSTBOLT:
                _castAISpell(frostBoltSpell);
                scriptEvents.addEvent(EVENT_P2_FROSTBOLT, Util::getRandomUInt(10000, 11000), PHASE_TWO);
                break;
            case EVENT_P2_FROSTBOLT_VOLLEY:
                _castAISpell(frostBoltVolleySpell);
                scriptEvents.addEvent(EVENT_P2_FROSTBOLT_VOLLEY, Util::getRandomUInt(13000, 15000), PHASE_TWO);
                break;
            case EVENT_P2_TOUCH_OF_INSIGNIFICANCE:
                _castAISpell(touchOfInsignifcanceSpell);
                scriptEvents.addEvent(EVENT_P2_TOUCH_OF_INSIGNIFICANCE, Util::getRandomUInt(9000, 13000), PHASE_TWO);
                break;
            case EVENT_P2_SUMMON_SHADE:
                if (Unit* shadeTarget = getBestPlayerTarget(TargetFilter_NotCurrent))
                {
                    summonShadeSpell->setCustomTarget(shadeTarget);
                    nextVengefulShadeTargetGUID = shadeTarget->getGuid();
                    _castAISpell(summonShadeSpell);
                }
                scriptEvents.addEvent(EVENT_P2_SUMMON_SHADE, Util::getRandomUInt(18000, 23000), PHASE_TWO);
                break;
            case EVENT_P2_SUMMON_WAVE:
                SummonWavePhaseTwo();
                scriptEvents.addEvent(EVENT_P2_SUMMON_WAVE, 45000, PHASE_TWO);
                break;
            case EVENT_BERSERK:
                _castAISpell(berserkSpell);
                sendDBChatMessage(SAY_LADY_BERSERK);
                break;
            }
        }

        // We should not melee attack when barrier is up
        if (getCreature()->hasAurasWithId(SPELL_MANA_BARRIER))
        {
            _setMeleeDisabled(true);
            return;
        }

        _setMeleeDisabled(false);
    }

    // summoning function for first phase
    void SummonWavePhaseOne()
    {    
        uint8 addIndex1 = waveCounter & 1;
        uint8 addIndex2 = uint8(addIndex1 ^ 1);

        // Todo summon Darnavan when weekly quest is active
        if (waveCounter)
            Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex1 * 3]);

        Summon(SummonEntries[addIndex2], LadyDeathwhisperSummonPositions[addIndex1 * 3 + 1]);
        Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex1 * 3 + 2]);

        if (mInstance->GetDifficulty() == InstanceDifficulty::RAID_25MAN_NORMAL || mInstance->GetDifficulty() == InstanceDifficulty::RAID_25MAN_HEROIC)
        {
            Summon(SummonEntries[addIndex2], LadyDeathwhisperSummonPositions[addIndex2 * 3]);
            Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex2 * 3 + 1]);
            Summon(SummonEntries[addIndex2], LadyDeathwhisperSummonPositions[addIndex2 * 3 + 2]);
            Summon(SummonEntries[Util::getRandomUInt(0, 1)], LadyDeathwhisperSummonPositions[6]);
        }
        ++waveCounter;
    }

    // summoning function for second phase
    void SummonWavePhaseTwo()
    {       
        if (mInstance->GetDifficulty() == InstanceDifficulty::RAID_25MAN_NORMAL || mInstance->GetDifficulty() == InstanceDifficulty::RAID_25MAN_HEROIC)
        {
            uint8 addIndex1 = waveCounter & 1;
            Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex1 * 3]);
            Summon(SummonEntries[addIndex1 ^ 1], LadyDeathwhisperSummonPositions[addIndex1 * 3 + 1]);
            Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex1 * 3 + 2]);
        }
        else
            Summon(SummonEntries[Util::getRandomUInt(0, 1)], LadyDeathwhisperSummonPositions[6]);
        ++waveCounter;
    }

    void Summon(uint32 entry, const LocationVector& pos)
    {
        Creature* summon = spawnCreature(entry, pos);

        if (summon)
        {
            summon->setSummonedByGuid(getCreature()->getGuid());
            summons.push_back(summon);
        }           
    }

    void DeleteSummons()
    {
        if (summons.empty())
            return;

        for (const auto& summon : summons)
        {
            if (summon->IsInWorld())
                summon->Despawn(100, 0);
        }

        summons.clear();
    }

    void ReanimateCultist()
    {
        if (reanimationQueue.empty())
            return;

        uint64 cultistGUID = reanimationQueue.front();
        Creature* cultist = mInstance->GetCreatureByGuid(static_cast<uint32_t>(cultistGUID));
        reanimationQueue.pop_front();
        if (!cultist)
            return;

        darkMartydromSpell->setCustomTarget(cultist);
        _castAISpell(darkMartydromSpell);
    }

    void EmpowerCultist()
    {
        if (summons.empty())
            return;

        std::list<Creature*> temp;
        for (auto itr = summons.begin(); itr != summons.end(); ++itr)
            if ((*itr)->isAlive() && ((*itr)->getEntry() == NPC_CULT_FANATIC || (*itr)->getEntry() == NPC_CULT_ADHERENT))
                temp.push_back((*itr));
        
        if (temp.empty())
            return;

        // select random cultist
        uint8_t i = static_cast<uint8_t>(Util::getRandomUInt(0, static_cast<uint32_t>(temp.size() - 1)));
        auto it = temp.begin();
        std::advance(it, i);

        Creature* cultist = (*it);
        if (cultist->getEntry() == NPC_CULT_FANATIC)
        {
            darkTransformationSpell->setCustomTarget(cultist);
            _castAISpell(darkTransformationSpell);
        }
        else
        {
            darkEmpowermentSpell->setCustomTarget(cultist);
            _castAISpell(darkEmpowermentSpell);
        }
    }

    void SetCreatureData64(uint32 Type, uint64 Data) override
    {
        switch (Type)
        {
        case DATA_CULTIST_GUID:
            reanimationQueue.push_back(Data);
            scriptEvents.addEvent(EVENT_P1_REANIMATE_CULTIST, 3000, PHASE_ONE);
            break;
        }
    }

protected:
    // Common
    InstanceScript* mInstance;
    uint64_t nextVengefulShadeTargetGUID;
    std::deque<uint64_t> reanimationQueue;
    uint32_t waveCounter;
    uint8_t dominateMindCount;
    bool introDone;
    std::list<Creature*> summons;

    // Spells
    CreatureAISpells* shadowChannelingSpell;
    CreatureAISpells* manaBarrierSpell;
    CreatureAISpells* deathAndDecaySpell;
    CreatureAISpells* dominateMindHeroSpell;
    CreatureAISpells* shadowBoltSpell;
    CreatureAISpells* frostBoltSpell;
    CreatureAISpells* frostBoltVolleySpell;
    CreatureAISpells* touchOfInsignifcanceSpell;
    CreatureAISpells* summonShadeSpell;
    CreatureAISpells* berserkSpell;
    CreatureAISpells* darkMartydromSpell;
    CreatureAISpells* darkTransformationSpell;
    CreatureAISpells* darkEmpowermentSpell;
};

class ManaBarrier : public SpellScript
{
    SpellScriptExecuteState onAuraPeriodicTick(Aura* aur, AuraEffectModifier* /*aurEff*/, float_t* /*damage*/) override
    {
        // Aura should periodically trigger spell every 1 sec but that spell is serverside so we don't have it in DBC
        // Overwrite default periodic tick with converting mana to hp

        const auto auraOwner = aur->getOwner();
        // If aura owner doesn't use mana, remove aura
        if (auraOwner->getMaxPower(POWER_TYPE_MANA) == 0)
        {
            aur->removeAura();
            return SpellScriptExecuteState::EXECUTE_PREVENT;
        }

        const auto healthDelta = auraOwner->getMaxHealth() - auraOwner->getHealth();
        if (healthDelta == 0)
        {
            // Unit is already at max health, so do nothing
            return SpellScriptExecuteState::EXECUTE_PREVENT;
        }

        const auto currentMana = auraOwner->getPower(POWER_TYPE_MANA);
        if (healthDelta < currentMana)
        {
            // Restore health to max and remove equal amount of mana
            auraOwner->setPower(POWER_TYPE_MANA, currentMana - healthDelta);
            auraOwner->setHealth(auraOwner->getMaxHealth());
            return SpellScriptExecuteState::EXECUTE_PREVENT;
        }

        // Unit takes more damage than it has mana left => remove aura
        aur->removeAura();

        // Aura is used by Lady Deathwhisper, change phase when all mana is drained
        if (auraOwner->isCreature())
        {
            const auto creatureOwner = static_cast<Creature*>(auraOwner);
            if (creatureOwner->GetScript() != nullptr)
                creatureOwner->GetScript()->DoAction(ACTION_MANABARRIER_DOWN);
        }

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Cult Adherent
class CultAdherentAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CultAdherentAI(c); }
    explicit CultAdherentAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        temporalVisualSpell         = addAISpell(SPELL_TELEPORT_VISUAL, 0.0f, TARGET_SELF);
        frostFeverSpell             = addAISpell(SPELL_FROST_FEVER, 100.0f, TARGET_ATTACKING, 0, Util::getRandomUInt(10, 12));
        deathchillSpell             = addAISpell(SPELL_DEATHCHILL_BLAST, 100.0f, TARGET_ATTACKING, 0, Util::getRandomUInt(14, 16));
        curseOfTorporSpell          = addAISpell(SPELL_CURSE_OF_TORPOR, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(14, 16));
        shroudOfTheOccultSpell      = addAISpell(SPELL_SHORUD_OF_THE_OCCULT, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(32, 39));
        cultistDarkMartyrdomSpell   = addAISpell(SPELL_DARK_MARTYRDOM_ADHERENT, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(18, 32));
    }

    void OnLoad()
    {
        _castAISpell(temporalVisualSpell);
        auto NewTarget = getBestPlayerTarget(TargetFilter_Closest);
        if (NewTarget)
        {
            getCreature()->getAIInterface()->setCurrentTarget(NewTarget);
            getCreature()->getAIInterface()->onHostileAction(NewTarget);
        }
    }

protected:
    // Common
    InstanceScript* mInstance;

    // Spells
    CreatureAISpells* temporalVisualSpell;
    CreatureAISpells* frostFeverSpell;
    CreatureAISpells* deathchillSpell;
    CreatureAISpells* curseOfTorporSpell;
    CreatureAISpells* shroudOfTheOccultSpell;
    CreatureAISpells* cultistDarkMartyrdomSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Cult Fanatic
class CultFanaticAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CultFanaticAI(c); }
    explicit CultFanaticAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        temporalVisualSpell         = addAISpell(SPELL_TELEPORT_VISUAL, 0.0f, TARGET_SELF);
        necroticStrikeSpell         = addAISpell(SPELL_NECROTIC_STRIKE, 100.0f, TARGET_ATTACKING, 0, Util::getRandomUInt(10, 12));
        shadowCleaveSpell           = addAISpell(SPELL_SHADOW_CLEAVE, 100.0f, TARGET_ATTACKING, 0 , Util::getRandomUInt(14, 16));
        vampireMightSpell           = addAISpell(SPELL_VAMPIRIC_MIGHT, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(20, 27));
        darkMartyrdomSpell          = addAISpell(SPELL_DARK_MARTYRDOM_ADHERENT, 100.0f, TARGET_SELF, 0 , Util::getRandomUInt(18, 32));
    }

    void OnLoad()
    {
        _castAISpell(temporalVisualSpell);
        auto NewTarget = getBestPlayerTarget(TargetFilter_Closest);
        if (NewTarget)
        {
            getCreature()->getAIInterface()->setCurrentTarget(NewTarget);
            getCreature()->getAIInterface()->onHostileAction(NewTarget);
        }
    }

protected:
    // Common
    InstanceScript* mInstance;

    // Spells
    CreatureAISpells* temporalVisualSpell;
    CreatureAISpells* necroticStrikeSpell;
    CreatureAISpells* shadowCleaveSpell;
    CreatureAISpells* vampireMightSpell;
    CreatureAISpells* darkMartyrdomSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Cultist Dark Martyrdom
class DarkMartyrdom : public SpellScript
{
public:
    void afterSpellEffect(Spell* spell, uint8_t effIndex) override
    {
        if (effIndex != EFF_INDEX_0)
            return;

        if (Creature* owner = spell->getCaster()->GetMapMgrCreature(spell->getUnitCaster()->getSummonedByGuid()))
            owner->GetScript()->SetCreatureData64(DATA_CULTIST_GUID, spell->getUnitCaster()->getGuidLow());
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Boss: Valithria Dreamwalker
// ...

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Gunship Battle Alliance
class MuradinGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 14500);
        if (!plr->isGroupLeader())
            menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_NOT_LEADER, 1);

        menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_ALLIANCE_RDY, 2);
        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        IceCrownCitadelScript* pInstance = (IceCrownCitadelScript*)pPlayer->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        switch (Id)
        {
        case 1:
            
            break;
        case 2:
            // Instance Start Gunship
            pInstance->DoAction(ACTION_INTRO_START);
            // Muradin Intro
            static_cast<Creature*>(pObject)->GetScript()->DoAction(ACTION_INTRO_START);
            // Clear NPC FLAGS
            static_cast<Creature*>(pObject)->removeUnitFlags(UNIT_NPC_FLAG_GOSSIP);
            break;
        }
        GossipMenu::senGossipComplete(pPlayer);
    }
};

class MuradinAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MuradinAI(c); }
    explicit MuradinAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();

        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    }

    void AIUpdate() override
    {
        if (getCreature()->getAIInterface()->getAiState() == AI_STATE_CASTING)
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_INTRO_ALLIANCE_1:
                sendDBChatMessage(SAY_INTRO_ALLIANCE_0);
                break;
            case EVENT_INTRO_ALLIANCE_2:
                sendDBChatMessage(SAY_INTRO_ALLIANCE_1);
                break;
            case EVENT_INTRO_ALLIANCE_3:
                sendDBChatMessage(SAY_INTRO_ALLIANCE_2);
                mInstance->SpawnEnemyGunship();
                break;
            case EVENT_INTRO_ALLIANCE_4:
                sendDBChatMessage(SAY_INTRO_ALLIANCE_3);
                break;
            case EVENT_INTRO_ALLIANCE_5:
                sendDBChatMessage(SAY_INTRO_ALLIANCE_4);
                break;
            case EVENT_INTRO_ALLIANCE_6:
                sendDBChatMessage(SAY_INTRO_ALLIANCE_5);
                break;
            case EVENT_INTRO_ALLIANCE_7:
                if (Creature* pSaurfang = mInstance->getLocalCreatureData(DATA_GB_HIGH_OVERLORD_SAURFANG))
                    pSaurfang->GetScript()->sendDBChatMessage(SAY_BOARDING_SKYBREAKER_0);
                break;
            case EVENT_INTRO_ALLIANCE_8:
                sendDBChatMessage(SAY_INTRO_ALLIANCE_7);
                break;
            case EVENT_KEEP_PLAYER_IN_COMBAT:
                if (mInstance->getData(DATA_GUNSHIP_EVENT) == InProgress)
                {
                    //SPELL_LOCK_PLAYERS_AND_TAP_CHEST maybe not needed to cast it but prepared
                    scriptEvents.addEvent(EVENT_KEEP_PLAYER_IN_COMBAT, 5000);
                }
                break;
            case EVENT_SUMMON_MAGE:
                break;
            case EVENT_ADDS:
                break;
            case EVENT_ADDS_BOARD_YELL:
                break;
            case EVENT_CHECK_RIFLEMAN:
                break;
            case EVENT_CHECK_MORTAR:
                break;
            case EVENT_CLEAVE:
                break;
            }
        }
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
        case ACTION_INTRO_START:
            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_1, 5000);
            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_2, 10000);
            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_3, 28000);
            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_4, 33000);
            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_5, 39000);
            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_6, 45000);
            break;
        case ACTION_BATTLE_EVENT:
            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_7, 5000);
            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_8, 11000);
            scriptEvents.addEvent(EVENT_KEEP_PLAYER_IN_COMBAT, 1);

            mInstance->setZoneMusic(4812, 17289);

            // Combat starts now
            if (Creature* orgrimsHammer = mInstance->getLocalCreatureData(DATA_ORGRIMMAR_HAMMER_BOSS))
                mInstance->sendUnitEncounter(EncounterFrameEngage, orgrimsHammer, 1);

            if (Creature* skybreaker = mInstance->getLocalCreatureData(DATA_SKYBREAKER_BOSS))
                mInstance->sendUnitEncounter(EncounterFrameEngage, skybreaker, 2);

            break;
        case ACTION_SPAWN_MAGE:
            break;
        }     
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Gunship Battle Horde
class SaurfangGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 14500);
        if (!plr->isGroupLeader())
            menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_NOT_LEADER, 1);

        menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_HORDE_RDY, 2);
        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        IceCrownCitadelScript* pInstance = (IceCrownCitadelScript*)pPlayer->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        switch (Id)
        {
        case 1:

            break;
        case 2:
            // Instance Start Gunship
            pInstance->DoAction(ACTION_INTRO_START);
            // Saurfang Intro
            static_cast<Creature*>(pObject)->GetScript()->DoAction(ACTION_INTRO_START);
            // Clear NPC FLAGS
            static_cast<Creature*>(pObject)->removeUnitFlags(UNIT_NPC_FLAG_GOSSIP);
            break;
        }
        GossipMenu::senGossipComplete(pPlayer);
    }
};

class SaurfangAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SaurfangAI(c); }
    explicit SaurfangAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();

        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    }

    void AIUpdate() override
    {
        if (getCreature()->getAIInterface()->getAiState() == AI_STATE_CASTING)
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_INTRO_HORDE_1:
                sendDBChatMessage(SAY_INTRO_HORDE_0);
                break;
            case EVENT_INTRO_HORDE_1_1:
                sendDBChatMessage(SAY_INTRO_HORDE_0_1);
                break;
            case EVENT_INTRO_HORDE_2:
                sendDBChatMessage(SAY_INTRO_HORDE_1);
                mInstance->SpawnEnemyGunship();
                break;
            case EVENT_INTRO_HORDE_3:
                sendDBChatMessage(SAY_INTRO_HORDE_2);
                break;
            case EVENT_INTRO_HORDE_4:
                sendDBChatMessage(SAY_BOARDING_SKYBREAKER_0);
                if (Creature* pMuradin = mInstance->getLocalCreatureData(DATA_GB_MURADIN_BRONZEBEARD))
                    pMuradin->GetScript()->sendDBChatMessage(SAY_INTRO_ALLIANCE_7);
                break;
            case EVENT_INTRO_HORDE_5:
                sendDBChatMessage(SAY_INTRO_HORDE_4);
                break;
            case EVENT_KEEP_PLAYER_IN_COMBAT:
                if (mInstance->getData(DATA_GUNSHIP_EVENT) == InProgress)
                {
                    //SPELL_LOCK_PLAYERS_AND_TAP_CHEST maybe not needed to cast it but prepared
                    scriptEvents.addEvent(EVENT_KEEP_PLAYER_IN_COMBAT, 5000);
                }
                break;
            case EVENT_SUMMON_MAGE:
                break;
            case EVENT_ADDS:
                break;
            case EVENT_ADDS_BOARD_YELL:
                break;
            case EVENT_CHECK_RIFLEMAN:
                break;
            case EVENT_CHECK_MORTAR:
                break;
            case EVENT_CLEAVE:
                break;
            }
        }
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
        case ACTION_INTRO_START:
            scriptEvents.addEvent(EVENT_INTRO_HORDE_1, 5000);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_1_1, 16000);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_2, 24600);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_3, 29600);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_4, 39200);
            break;
        case ACTION_BATTLE_EVENT:
            scriptEvents.addEvent(EVENT_INTRO_HORDE_5, 5000);

            mInstance->setZoneMusic(4812, 17289);

            // Combat starts now
            if (Creature* skybreaker = mInstance->getLocalCreatureData(DATA_SKYBREAKER_BOSS))
                mInstance->sendUnitEncounter(EncounterFrameEngage, skybreaker, 1);

            if (Creature* orgrimsHammer = mInstance->getLocalCreatureData(DATA_ORGRIMMAR_HAMMER_BOSS))
                mInstance->sendUnitEncounter(EncounterFrameEngage, orgrimsHammer, 2);

            break;
        case ACTION_SPAWN_MAGE:
            break;
        }
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;
};

class ZafodBoomboxGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        pInstance = (IceCrownCitadelScript*)plr->GetMapMgr()->GetScript();

        GossipMenu menu(pObject->getGuid(), 14500);
        menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_JETPACK, 1);
        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* /*pObject*/, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
        case 1:
            pPlayer->getItemInterface()->AddItemById(ITEM_GOBLIN_ROCKET_PACK, 1, 0);
            break;
        }
        GossipMenu::senGossipComplete(pPlayer);
    }

protected:
    IceCrownCitadelScript* pInstance;
};

class ZafodBoomboxAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ZafodBoomboxAI(c); }
    explicit ZafodBoomboxAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();
        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;
};

class GunshipAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GunshipAI(c); }
    explicit GunshipAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();

        _teamInInstance = mInstance->getLocalData(DATA_TEAM_IN_INSTANCE);
        _summonedFirstMage = false;
        _died = false;
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
    }

    void DamageTaken(Unit* /*_attacker*/, uint32_t* damage) override
    {
        if (*damage >= getCreature()->getHealth())
        {
            OnDied(nullptr);
            *damage = getCreature()->getHealth() - 1;
            return;
        }

        if (_summonedFirstMage)
            return;

        if (getCreature()->GetTransport()->getEntry() != uint32_t(_teamInInstance == TEAM_HORDE ? GO_THE_SKYBREAKER_HORDE_ICC : GO_ORGRIM_S_HAMMER_ALLIANCE_ICC))
            return;

        if (getCreature()->getHealthPct() > 90)
            return;

        _summonedFirstMage = true;
        if (Creature* captain = mInstance->getLocalCreatureData(_teamInInstance == TEAM_HORDE ? DATA_GB_MURADIN_BRONZEBEARD : DATA_GB_HIGH_OVERLORD_SAURFANG))
            captain->GetScript()->DoAction(ACTION_SPAWN_MAGE);
    }

    void OnDied(Unit* /*pTarget*/) override
    {
        if (_died)
            return;

        _died = true;

        // Victory??
        bool isVictory = getCreature()->GetTransport()->getEntry() == GO_THE_SKYBREAKER_HORDE_ICC || getCreature()->GetTransport()->getEntry() == GO_ORGRIM_S_HAMMER_ALLIANCE_ICC;
        mInstance->setData(DATA_GUNSHIP_EVENT, isVictory ? Finished : InvalidState);

        // Disangege
        if (Creature* creature = mInstance->getLocalCreatureData(getCreature()->getEntry() == NPC_GB_ORGRIMS_HAMMER ? DATA_SKYBREAKER_BOSS : DATA_ORGRIMMAR_HAMMER_BOSS))
        {
            mInstance->sendUnitEncounter(EncounterFrameDisengaged, creature);
        }

        mInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        // Stopp Playing Music
        mInstance->setZoneMusic(4812, 0);

        std::list<Creature*> creatures;
        // Eject All Passengers
        GetCreatureListWithEntryInGrid(creatures, _teamInInstance == TEAM_HORDE ? NPC_GB_HORDE_CANON : NPC_GB_ALLIANCE_CANON, 900.0f);
        for (auto itr = creatures.begin(); itr != creatures.end(); ++itr)
        {
            Creature* cannon = *itr;
            if (isVictory)
                cannon->GetScript()->DoAction(ACTION_BATTLE_DONE);
            else
                cannon->GetScript()->DoAction(ACTION_FAIL);
        }
        creatures.clear();
        // Explosion
        GetCreatureListWithEntryInGrid(creatures, NPC_GB_GUNSHIP_HULL, 900.0f);
        for (auto itr = creatures.begin(); itr != creatures.end(); ++itr)
        {
            Creature* hull = *itr;
            if (hull->GetTransport() != getCreature()->GetTransport())
                continue;

            if (isVictory)
                hull->GetScript()->DoAction(ACTION_BATTLE_DONE);
            else
                hull->GetScript()->DoAction(ACTION_FAIL);
        }
        creatures.clear();

        if (isVictory)
            mInstance->DoAction(ACTION_BATTLE_DONE);
        else
            mInstance->DoAction(ACTION_FAIL);
    }

private:
    uint32_t _teamInInstance;
    std::map<uint64_t, uint32_t> _shipVisits;
    bool _summonedFirstMage;
    bool _died;

protected:
    // Common
    IceCrownCitadelScript* mInstance;
};

class GunshipHullAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GunshipHullAI(c); }
    explicit GunshipHullAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);

        // Scripted Spells not autocastet
        ExplosionVictory = addAISpell(SPELL_EXPLOSION_VICTORY, 0.0f, TARGET_SELF);
        ExplosionWipe = addAISpell(SPELL_EXPLOSION_WIPE, 0.0f, TARGET_SELF);
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
        case ACTION_BATTLE_DONE:    // Enemy Ship Explodes
            _castAISpell(ExplosionVictory);
            break;
        case ACTION_FAIL:   // Our Ship Explodes
            _castAISpell(ExplosionWipe);
            break;
        }
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;

    // Spells
    CreatureAISpells* ExplosionVictory;
    CreatureAISpells* ExplosionWipe;
};

class GunshipCanonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GunshipCanonAI(c); }
    explicit GunshipCanonAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();

        getCreature()->setControlled(true, UNIT_STATE_ROOTED);

        EjectBelowZero = addAISpell(SPELL_EJECT_ALL_PASSENGERS_BELOW_ZERO, 0.0f, TARGET_SELF, 0, 0, 0, true);
        EcectWipe = addAISpell(SPELL_EJECT_ALL_PASSENGERS_WIPE, 0.0f, TARGET_SELF, 0, 0, 0, true);
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
        case ACTION_BATTLE_DONE:    // Enemy Ship Explodes
            _castAISpell(EjectBelowZero);
            break;
        case ACTION_FAIL:   // Our Ship Explodes
            _castAISpell(EcectWipe);
            break;
        }
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;

    // Spells
    CreatureAISpells* EjectBelowZero;
    CreatureAISpells* EcectWipe;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Deathbringer Suarfang
class MuradinSeGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        pInstance = (IceCrownCitadelScript*)plr->GetMapMgr()->GetScript();

        if (pInstance && pInstance->getData(CN_DEATHBRINGER_SAURFANG) != Finished)
        {
            GossipMenu menu(pObject->getGuid(), 14500);
            menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_MURADIN_START, 1);
            menu.sendGossipPacket(plr);
        }
    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
        case 1:
            // Start Intro
            static_cast<Creature*>(pObject)->GetScript()->DoAction(ACTION_START_EVENT);
            break;
        }
        GossipMenu::senGossipComplete(pPlayer);
    }

protected:
    IceCrownCitadelScript* pInstance;
};

class MuradinSaurfangEvent : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MuradinSaurfangEvent(c); }
    explicit MuradinSaurfangEvent(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();
        
        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    }

    void AIUpdate() override
    {
        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_INTRO_ALLIANCE_4_SE:
            {
                getCreature()->getMovementManager()->movePoint(POINT_FIRST_STEP, firstStepPos.getPositionX(), firstStepPos.getPositionY(), firstStepPos.getPositionZ());
            break;
            }
            case EVENT_INTRO_ALLIANCE_5_SE:
            {
                sendDBChatMessage(SAY_INTRO_ALLIANCE_5_SE);

                // Charge
                getCreature()->getMovementManager()->moveCharge(chargePos[0].getPositionX(), chargePos[0].getPositionY(), chargePos[0].getPositionZ(), 8.5f, POINT_CHARGE); 

                for (auto itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                    (*itr)->GetScript()->DoAction(ACTION_CHARGE);
            break;
            }
            }
        }
    }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type == POINT_MOTION_TYPE && iWaypointId == POINT_FIRST_STEP)
        {
            sendDBChatMessage(SAY_INTRO_ALLIANCE_4_SE);

            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_5_SE, 5000, PHASE_INTRO_A);

            if (Creature* deathbringer = mInstance->getLocalCreatureData(DATA_DEATHBRINGER_SAURFANG))
                deathbringer->GetScript()->DoAction(ACTION_CONTINUE_INTRO);
        }
    }

    void OnHitBySpell(uint32_t _spellId, Unit* /*_caster*/) override
    {
        if (_spellId == SPELL_GRIP_OF_AGONY)
        {
            getCreature()->setMoveDisableGravity(true);
            getCreature()->getMovementManager()->movePoint(POINT_CHOKE, chokePos[0]);
        }
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
        case ACTION_START_EVENT:
        {            
            // Prevent crashes
            if (getScriptPhase() == PHASE_INTRO_A)
                return;

            // Guards
            uint32 x = 1;
            GetCreatureListWithEntryInGrid(_guardList, NPC_SE_SKYBREAKER_MARINE, 20.0f);
            for (auto itr = _guardList.begin(); itr != _guardList.end(); ++x, ++itr)
                (*itr)->GetScript()->SetCreatureData(0, x);
            //

            setScriptPhase(PHASE_INTRO_A);
            sendDBChatMessage(SAY_INTRO_ALLIANCE_1_SE);

            // Start Intro
            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_4_SE, 29500, PHASE_INTRO_A);

            // Open Suarfangs Door
            if (GameObject* Door = mInstance->GetGameObjectByGuid(mInstance->getLocalData(DATA_SAURFANG_DOOR)))
                Door->setState(GO_STATE_OPEN);

            // Start Intro on Suarfang        
            if (Creature* deathbringer = mInstance->getLocalCreatureData(DATA_DEATHBRINGER_SAURFANG))
                deathbringer->GetScript()->DoAction(PHASE_INTRO_A);

            // Clear NPC FLAGS
            getCreature()->removeNpcFlags(UNIT_NPC_FLAG_GOSSIP);
            getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
            break;
        }
        case ACTION_START_OUTRO:
        {
            sendDBChatMessage(SAY_OUTRO_ALLIANCE_1_SE);
            getCreature()->setMoveDisableGravity(false);
            break;
        }
        case ACTION_INTERRUPT_INTRO:
            scriptEvents.resetEvents();
            resetScriptPhase();
            break;
        }
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;
    std::list<Creature*> _guardList;
};

class OverlordSeGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 14500);
        menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_SAURFANG_START, 1);
        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        IceCrownCitadelScript* pInstance = (IceCrownCitadelScript*)pPlayer->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        switch (Id)
        {
        case 1:
            // Start Intro
            static_cast<Creature*>(pObject)->GetScript()->DoAction(ACTION_START_EVENT);
            break;
        }
        GossipMenu::senGossipComplete(pPlayer);
    }
};

class OverlordSaurfangEvent : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OverlordSaurfangEvent(c); }
    explicit OverlordSaurfangEvent(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();

        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
        getCreature()->getAIInterface()->setAiState(AI_STATE_IDLE);
    }

    void AIUpdate() override
    {
        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_INTRO_HORDE_3_SE:
                getCreature()->getMovementManager()->movePoint(POINT_FIRST_STEP, firstStepPos.getPositionX(), firstStepPos.getPositionY(), firstStepPos.getPositionZ());
                break;
            case EVENT_INTRO_HORDE_5_SE:
                sendDBChatMessage(SAY_INTRO_HORDE_5_SE);
                break;
            case EVENT_INTRO_HORDE_6_SE:
                sendDBChatMessage(SAY_INTRO_HORDE_6_SE);
                break;
            case EVENT_INTRO_HORDE_7_SE:
                sendDBChatMessage(SAY_INTRO_HORDE_7_SE);
                break;
            case EVENT_INTRO_HORDE_8_SE:
                sendDBChatMessage(SAY_INTRO_HORDE_8_SE);

                // Charge
                getCreature()->getMovementManager()->moveCharge(chargePos[0].getPositionX(), chargePos[0].getPositionY(), chargePos[0].getPositionZ(), 8.5f, POINT_CHARGE);
                break;
            case EVENT_OUTRO_HORDE_2_SE:   // say
                sendDBChatMessage(SAY_OUTRO_HORDE_2_SE);
                break;
            case EVENT_OUTRO_HORDE_3_SE:   // say
                sendDBChatMessage(SAY_OUTRO_HORDE_3_SE);
                break;
            case EVENT_OUTRO_HORDE_4_SE:   // move
                break;
            case EVENT_OUTRO_HORDE_5_SE:   // move
                break;
            case EVENT_OUTRO_HORDE_6_SE:   // say
                sendDBChatMessage(SAY_OUTRO_HORDE_4_SE);
                break;
            }
        }
    }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type == POINT_MOTION_TYPE && iWaypointId == POINT_FIRST_STEP)
        {
            sendDBChatMessage(SAY_INTRO_HORDE_3_SE);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_5_SE, 15500, PHASE_INTRO_H);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_6_SE, 29500, PHASE_INTRO_H);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_7_SE, 43800, PHASE_INTRO_H);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_8_SE, 47000, PHASE_INTRO_H);

            if (Creature* deathbringer = mInstance->getLocalCreatureData(DATA_DEATHBRINGER_SAURFANG))
                deathbringer->GetScript()->DoAction(ACTION_CONTINUE_INTRO);
        }
    }

    void OnHitBySpell(uint32_t _spellId, Unit* /*_caster*/) override
    {
        if (_spellId == SPELL_GRIP_OF_AGONY)
        {
            getCreature()->setMoveDisableGravity(true);
            getCreature()->getMovementManager()->movePoint(POINT_CHOKE, chokePos[0]);
        }
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
        case ACTION_START_EVENT:
        {
            // Prevent crashes
            if (getScriptPhase() == PHASE_INTRO_H)
                return;

            // Guards
            uint32 x = 1;
            GetCreatureListWithEntryInGrid(_guardList, NPC_SE_KOR_KRON_REAVER, 20.0f);
            for (auto itr = _guardList.begin(); itr != _guardList.end(); ++x, ++itr)
                (*itr)->GetScript()->SetCreatureData(0, x);
            //

            sendDBChatMessage(SAY_INTRO_HORDE_1_SE);
            setScriptPhase(PHASE_INTRO_H);

            // Start Intro
            scriptEvents.addEvent(EVENT_INTRO_HORDE_3_SE, 18500, PHASE_INTRO_H);

            // Open Suarfangs Door
            if (GameObject* Door = mInstance->GetGameObjectByGuid(mInstance->getLocalData(DATA_SAURFANG_DOOR)))
                Door->setState(GO_STATE_OPEN);

            // Start Intro on Suarfang        
            if (Creature* deathbringer = mInstance->getLocalCreatureData(DATA_DEATHBRINGER_SAURFANG))
                deathbringer->GetScript()->DoAction(PHASE_INTRO_H);

            // Clear NPC FLAGS
            getCreature()->removeUnitFlags(UNIT_NPC_FLAG_GOSSIP);
            getCreature()->getAIInterface()->setAllowedToEnterCombat(false);

            break;
        }
        case ACTION_START_OUTRO:
        {
            sendDBChatMessage(SAY_OUTRO_HORDE_1_SE);
            scriptEvents.addEvent(EVENT_OUTRO_HORDE_2_SE, 10000);   // say
            scriptEvents.addEvent(EVENT_OUTRO_HORDE_3_SE, 18000);   // say
            scriptEvents.addEvent(EVENT_OUTRO_HORDE_4_SE, 24000);   // cast
            scriptEvents.addEvent(EVENT_OUTRO_HORDE_5_SE, 30000);   // move
            getCreature()->setMoveDisableGravity(false);
            break;
        }
        case ACTION_INTERRUPT_INTRO:
            scriptEvents.resetEvents();
            resetScriptPhase();
            break;
        }
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;
    std::list<Creature*> _guardList;
};

class DeathbringerSaurfangAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DeathbringerSaurfangAI(c); }
    explicit DeathbringerSaurfangAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();
        getCreature()->setAItoUse(true);
        getCreature()->getAIInterface()->setAiState(AI_STATE_IDLE);

        _introDone = false;

        // Scripted Spells not autocastet
        GripOfAgonySpell = addAISpell(SPELL_GRIP_OF_AGONY, 0.0f, TARGET_SELF);
    }

    void AIUpdate() override
    {
        if (getCreature()->getAIInterface()->getAiState() == AI_STATE_CASTING)
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_INTRO_ALLIANCE_2_SE:
                getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                getCreature()->setFaction(974);
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_ALLIANCE_2);
                break;
            case EVENT_INTRO_ALLIANCE_3_SE:
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_ALLIANCE_3);
                break;
            case EVENT_INTRO_ALLIANCE_6_SE:
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_ALLIANCE_6);
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_ALLIANCE_7);
                _castAISpell(GripOfAgonySpell);
                setCanEnterCombat(true);
                getCreature()->removeUnitFlags(UNIT_FLAG_IGNORE_PLAYER_NPC);
                getCreature()->getAIInterface()->setAiScriptType(AI_SCRIPT_LONER);
                break;
            case EVENT_INTRO_HORDE_2_SE:
                getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                getCreature()->setFaction(974);
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_HORDE_2);
                break;
            case EVENT_INTRO_HORDE_4_SE:
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_HORDE_4);
                break;
            case EVENT_INTRO_HORDE_9_SE:
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_HORDE_9);
                _castAISpell(GripOfAgonySpell);
                setCanEnterCombat(true);
                getCreature()->removeUnitFlags(UNIT_FLAG_IGNORE_PLAYER_NPC);
                getCreature()->getAIInterface()->setAiScriptType(AI_SCRIPT_LONER);
                break;
            case EVENT_INTRO_FINISH_SE:
                setScriptPhase(PHASE_COMBAT);
                _introDone = true;
                break;
            }
        }
    }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != POINT_MOTION_TYPE && iWaypointId != POINT_SAURFANG)
            return;

        // Close Suarfangs Door
        if (GameObject* Door = mInstance->GetGameObjectByGuid(mInstance->getLocalData(DATA_SAURFANG_DOOR)))
            Door->setState(GO_STATE_CLOSED);
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
        case PHASE_INTRO_A:
        case PHASE_INTRO_H:
        {     
            setScriptPhase(uint32(action));

            // Move
            getCreature()->getMovementManager()->movePoint(POINT_SAURFANG, deathbringerPos.getPositionX(), deathbringerPos.getPositionY(), deathbringerPos.getPositionZ());

            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_2_SE, 2500, PHASE_INTRO_A);
            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_3_SE, 20000, PHASE_INTRO_A);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_2_SE, 5000, PHASE_INTRO_H);
            break;
        }
        case ACTION_CONTINUE_INTRO:
        {
            if (_introDone)
                return;

            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_6_SE, 7000, PHASE_INTRO_A);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_4_SE, 6500, PHASE_INTRO_H);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_9_SE, 48200, PHASE_INTRO_H);

            if (mInstance->getLocalData(DATA_TEAM_IN_INSTANCE) == TEAM_ALLIANCE)
                scriptEvents.addEvent(EVENT_INTRO_FINISH_SE, 8000, PHASE_INTRO_A);
            else
                scriptEvents.addEvent(EVENT_INTRO_FINISH_SE, 55700, PHASE_INTRO_H);
            break;
        }
        }
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;
    bool _introDone;

    // Spells
    CreatureAISpells* GripOfAgonySpell;
};

class NpcSaurfangEventAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NpcSaurfangEventAI(c); }
    explicit NpcSaurfangEventAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();
        getCreature()->getAIInterface()->setAiState(AI_STATE_IDLE);
        _index = 0;
    }

    void SetCreatureData(uint32_t type, uint32_t data) override
    {
        if (!(!type && data && data < 6))
            return;
        _index = data;
    }

    void OnHitBySpell(uint32_t _spellId, Unit* /*_caster*/) override
    {
        if (_spellId == SPELL_GRIP_OF_AGONY)
        {
            getCreature()->setMoveDisableGravity(true);
            getCreature()->getMovementManager()->movePoint(POINT_CHOKE, chokePos[_index]);
        }
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
        case ACTION_CHARGE:
        {
            if (action == ACTION_CHARGE && _index)
            {
                getCreature()->getMovementManager()->moveCharge(chargePos[_index].getPositionX(), chargePos[_index].getPositionY(), chargePos[_index].getPositionZ(), 8.5f, POINT_CHARGE);
            }
            else if (action == ACTION_DESPAWN)
                getCreature()->Despawn(100, 0);
            break;
        }
        }
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;
    uint32_t _index;
};

class GripOfAgony : public SpellScript
{
public:
    void filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets) override
    {
        if (effectIndex != EFF_INDEX_0)
            return;

        // Hackfix shouldnt cast on self
        effectTargets->clear();

        std::vector<Player*> players;
        for (const auto& itr : spell->getUnitCaster()->getInRangeObjectsSet())
        {
            float distance = spell->getUnitCaster()->CalcDistance(itr);
            if (itr->isCreature() && itr->getEntry() != CN_DEATHBRINGER_SAURFANG && distance <= 100.0f)
            {
                effectTargets->push_back(itr->getGuid());
            }
        }
    }
};

void SetupICC(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_ICECROWNCITADEL, &IceCrownCitadelScript::Create);

    //Teleporters
    mgr->register_gameobject_script(GO_TELE_1, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_1, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_2, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_2, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_3, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_3, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_4, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_4, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_5, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_5, new ICCTeleporterGossip());

    //Bosses
    mgr->register_creature_script(CN_LORD_MARROWGAR, &LordMarrowgarAI::Create);
    mgr->register_creature_script(CN_LADY_DEATHWHISPER, &LadyDeathwhisperAI::Create);
    mgr->register_creature_script(NPC_GB_MURADIN_BRONZEBEARD, &MuradinAI::Create);
    mgr->register_creature_script(NPC_GB_HIGH_OVERLORD_SAURFANG, &SaurfangAI::Create);
    mgr->register_creature_script(NPC_SE_MURADIN_BRONZEBEARD, &MuradinSaurfangEvent::Create);
    mgr->register_creature_script(NPC_SE_HIGH_OVERLORD_SAURFANG, &OverlordSaurfangEvent::Create);
    mgr->register_creature_script(CN_DEATHBRINGER_SAURFANG, &DeathbringerSaurfangAI::Create);
    //mgr->register_creature_script(CN_VALITHRIA_DREAMWALKER, &ValithriaDreamwalkerAI::Create);
    mgr->register_creature_script(gunshipIds, &GunshipAI::Create);

    //Spell Bone Storm
    mgr->register_spell_script(boneStormIds, new BoneStormDamage);
    mgr->register_spell_script(SPELL_BONE_STORM, new BoneStorm);

    // Spell Bone Spike Graveyard
    mgr->register_spell_script(SPELL_BONE_SPIKE_GRAVEYARD, new BoneSpikeGraveyard);

    // Spell Coldflame
    mgr->register_spell_script(SPELL_COLDFLAME_NORMAL, new Coldflame);
    mgr->register_spell_script(SPELL_COLDFLAME_DAMAGE, new ColdflameDamage);
    mgr->register_spell_script(SPELL_COLDFLAME_BONE_STORM, new ColdflameBonestorm);

    // Spell Bone Slice
    mgr->register_spell_script(SPELL_BONE_SLICE, new BoneSlice);

    // Spell Mana Barrier
    mgr->register_spell_script(SPELL_MANA_BARRIER, new ManaBarrier);

    // Spell Cultist Dark Martyrdom
    mgr->register_spell_script(SPELL_DARK_MARTYRDOM_ADHERENT, new DarkMartyrdom);

    // Spell Grip Of Agony
    mgr->register_spell_script(SPELL_GRIP_OF_AGONY, new GripOfAgony);

    //Gossips
    GossipScript* MuradinGossipScript = new MuradinGossip();
    mgr->register_creature_gossip(NPC_GB_MURADIN_BRONZEBEARD, MuradinGossipScript);
    GossipScript* SaurfangGossipScript = new SaurfangGossip();
    mgr->register_creature_gossip(NPC_GB_HIGH_OVERLORD_SAURFANG, SaurfangGossipScript);
    GossipScript* MuradinSaurfangEventGossipScript = new MuradinSeGossip();
    mgr->register_creature_gossip(NPC_SE_MURADIN_BRONZEBEARD, MuradinSaurfangEventGossipScript);
    GossipScript* OverlordSaurfangEventGossipScript = new OverlordSeGossip();
    mgr->register_creature_gossip(NPC_SE_HIGH_OVERLORD_SAURFANG, OverlordSaurfangEventGossipScript);
    GossipScript* ZafodBoomboxGossipScript = new ZafodBoomboxGossip();
    mgr->register_creature_gossip(NPC_GB_ZAFOD_BOOMBOX, ZafodBoomboxGossipScript);

    //Misc
    mgr->register_creature_script(NPC_COLDFLAME, &ColdflameAI::Create);
    mgr->register_creature_script(NPC_BONE_SPIKE, &BoneSpikeAI::Create);

    mgr->register_creature_script(NPC_CULT_FANATIC, CultFanaticAI::Create);
    mgr->register_creature_script(NPC_CULT_ADHERENT, CultAdherentAI::Create);

    mgr->register_creature_script(NPC_SE_SKYBREAKER_MARINE, NpcSaurfangEventAI::Create);
    mgr->register_creature_script(NPC_SE_KOR_KRON_REAVER, NpcSaurfangEventAI::Create);

    mgr->register_creature_script(NPC_GB_ZAFOD_BOOMBOX, ZafodBoomboxAI::Create);

    mgr->register_creature_script(NPC_GB_GUNSHIP_HULL, GunshipHullAI::Create);

    mgr->register_creature_script(canonIds, GunshipCanonAI::Create);
}
