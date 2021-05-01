/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_IceCrownCitadel.h"
#include "Objects/Faction.h"
#include "Units/Summons/Summon.h"
#include "../world/Objects/ObjectMgr.h"
#include "../world/Management/TransporterHandler.h"
#include "../world/Objects/Transporter.h"

#if VERSION_STRING >= WotLK

//////////////////////////////////////////////////////////////////////////////////////////
//ICC zone: 4812
//Prepared creature entry:
//
//CN_DEATHBRINGER_SAURFANG    37813
//CN_FESTERGUT                36626
//CN_ROTFACE                  36627
//CN_PROFESSOR_PUTRICIDE      36678
//CN_PRINCE_VALANAR           37970
//N_BLOOD_QUEEN_LANATHEL     37955
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
        MarrowgarIcewall1GUID = 0;
        MarrowgarIcewall2GUID = 0;
        MarrowgarEntranceDoorGUID = 0;

        // Lady Deathwhisper
        LadyDeathwisperElevatorGUID = 0;
        LadyDeathwisperEntranceDoorGUID = 0;

        // Gunship Battle
        skybreaker = nullptr;
        orgrimmar = nullptr;
        SkybreakerBossGUID = 0;
        OrgrimmarBossGUID = 0;
        DeathbringerSaurfangGbGUID = 0;
        MuradinBronzebeardGbGUID = 0;
        DeathbringerSaurfangNotVisualGUID = 0;
        MuradinBronzebeardNotVisualGUID = 0;
        GbBattleMageGUID = 0;
        isPrepared = false;
        addData(DATA_GUNSHIP_EVENT, NotStarted);

        // Deathbringer Saurfang
        DeathbringerDoorGUID = 0;
        DeathbringerSaurfangSpawnID = 0;
        
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
        case DATA_DEATHBRINGER_SAURFANG:
            return DeathbringerSaurfangSpawnID;
        }
        return 0;
    }

    uint64_t getLocalData64(uint32_t type) const
    {
        switch (type)
        {
            // Gunship battle	
        case DATA_SKYBREAKER_BOSS:
            return SkybreakerBossGUID;
        case DATA_ORGRIMMAR_HAMMER_BOSS:
            return OrgrimmarBossGUID;
        case DATA_GB_HIGH_OVERLORD_SAURFANG:
            return DeathbringerSaurfangGbGUID;
        case DATA_GB_MURADIN_BRONZEBEARD:
            return MuradinBronzebeardGbGUID;
        case DATA_HIGH_OVERLORD_SAURFANG_NOT_VISUAL:
            return DeathbringerSaurfangNotVisualGUID;
        case DATA_MURADIN_BRONZEBEARD_NOT_VISUAL:
            return MuradinBronzebeardNotVisualGUID;
        case DATA_GB_BATTLE_MAGE:
            return GbBattleMageGUID;
        }
        return 0;
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            // Gunship
        case NPC_GB_SKYBREAKER:
            SkybreakerBossGUID = pCreature->getGuid();
            break;
        case NPC_GB_ORGRIMS_HAMMER:
            OrgrimmarBossGUID = pCreature->getGuid();
            break;
        case NPC_GB_HIGH_OVERLORD_SAURFANG:
            DeathbringerSaurfangGbGUID = pCreature->getGuid();
            break;
        case NPC_GB_MURADIN_BRONZEBEARD:
            MuradinBronzebeardGbGUID = pCreature->getGuid();
            break;
        case NPC_GB_HIGH_OVERLORD_SAURFANG_NV:
            DeathbringerSaurfangNotVisualGUID = pCreature->getGuid();
            break;
        case NPC_GB_MURADIN_BRONZEBEARD_NV:
            MuradinBronzebeardNotVisualGUID = pCreature->getGuid();
            break;
        case NPC_GB_SKYBREAKER_SORCERERS:
        case NPC_GB_KORKRON_BATTLE_MAGE:
            GbBattleMageGUID = pCreature->getGuid();
            break;

            // Deathbringer Suarfang
        case CN_DEATHBRINGER_SAURFANG:
            DeathbringerSaurfangSpawnID = pCreature->spawnid;
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

    void SetGameobjectStates(GameObject* pGameObject)
    {
        // Gos which are not visible by killing a boss needs a second check...
        if (getData(CN_LORD_MARROWGAR) == Finished)
        {
            if (MarrowgarIcewall1GUID)
                GetGameObjectByGuid(MarrowgarIcewall1GUID)->setState(GO_STATE_OPEN);        // Icewall 1

            if (MarrowgarIcewall2GUID)
                GetGameObjectByGuid(MarrowgarIcewall2GUID)->setState(GO_STATE_OPEN);        // Icewall 2

            if (MarrowgarEntranceDoorGUID)
                GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_OPEN);    // Door  
        }

        if (getData(CN_LADY_DEATHWHISPER) == Finished)
        {
            if (LadyDeathwisperEntranceDoorGUID)
                GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_OPEN);

            if (LadyDeathwisperElevatorGUID)
                GetGameObjectByGuid(LadyDeathwisperElevatorGUID)->setState(GO_STATE_OPEN);
        }

        if (getData(CN_LADY_DEATHWHISPER) == NotStarted)
        {
            if (LadyDeathwisperEntranceDoorGUID)
                GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_OPEN);
        }

        if (getData(CN_DEATHBRINGER_SAURFANG) == NotStarted)
        {
            if (DeathbringerDoorGUID)
                GetGameObjectByGuid(DeathbringerDoorGUID)->setState(GO_STATE_CLOSED);
        }

        if (getData(CN_DEATHBRINGER_SAURFANG) == Finished)
        {
            if (DeathbringerDoorGUID)
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
                    GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_CLOSED);
            }
            if (state == NotStarted)
            {
                if (MarrowgarEntranceDoorGUID)
                    GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_OPEN);
            }
            if (state == Finished)
            {
                if (MarrowgarIcewall1GUID)
                    GetGameObjectByGuid(MarrowgarIcewall1GUID)->setState(GO_STATE_OPEN);        // Icewall 1

                if (MarrowgarIcewall2GUID)
                    GetGameObjectByGuid(MarrowgarIcewall2GUID)->setState(GO_STATE_OPEN);        // Icewall 2

                if (MarrowgarEntranceDoorGUID)
                    GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_OPEN);    // Door  
            }
            break;
        case CN_LADY_DEATHWHISPER:
            if (state == InProgress)
            {
                if (LadyDeathwisperEntranceDoorGUID)
                    GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_CLOSED);
            }
            if (state == NotStarted)
            {
                if (LadyDeathwisperEntranceDoorGUID)
                    GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_CLOSED);
            }
            if (state == Finished)
            {
                if (LadyDeathwisperEntranceDoorGUID)
                    GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_OPEN);

                if (LadyDeathwisperElevatorGUID)
                    GetGameObjectByGuid(LadyDeathwisperElevatorGUID)->setState(GO_STATE_OPEN);
            }
            break;
        case CN_DEATHBRINGER_SAURFANG:
            {
            if (state == InProgress)
            {
                if (DeathbringerDoorGUID)
                    GetGameObjectByGuid(DeathbringerDoorGUID)->setState(GO_STATE_CLOSED);
            }

            if (state == NotStarted)
            {
                if (DeathbringerDoorGUID)
                    GetGameObjectByGuid(DeathbringerDoorGUID)->setState(GO_STATE_CLOSED);
            }

            if (state == Finished)
            {
                if (DeathbringerDoorGUID)
                    GetGameObjectByGuid(DeathbringerDoorGUID)->setState(GO_STATE_OPEN);
            }
                break;
            }
        }      
    }

    void PrepareGunshipEvent(Player* player)
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
        else
            sTransportHandler.loadTransportForPlayers(player);
    }

    void SpawnEnemyGunship()
    {
        if (TeamInInstance == TEAM_ALLIANCE)
            orgrimmar = sTransportHandler.createTransport(GO_ORGRIM_S_HAMMER_ALLIANCE_ICC, mInstance);

        if (TeamInInstance == TEAM_HORDE)   
           skybreaker = sTransportHandler.createTransport(GO_THE_SKYBREAKER_HORDE_ICC, mInstance);
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
                    for (uint8_t i = 0; i < 13; i++)
                        spawnCreature(AllySpawns[i].entry, AllySpawns[i].x, AllySpawns[i].y, AllySpawns[i].z, AllySpawns[i].o, AllySpawns[i].faction);
                    break;
                case TEAM_HORDE:
                    for (uint8_t i = 0; i < 13; i++)
                        spawnCreature(HordeSpawns[i].entry, HordeSpawns[i].x, HordeSpawns[i].y, HordeSpawns[i].z, HordeSpawns[i].o, HordeSpawns[i].faction);
                    break;
            }
            setSpawnsCreated();
        }
        PrepareGunshipEvent(player); // Spawn Gunship Event
    }    

    void UpdateEvent() override
    {
        scriptEvents.updateEvents(getUpdateFrequency(), 0);

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_WIPE_CHECK:
                if (TeamInInstance = TEAM_ALLIANCE)
                {
                    DoCheckFallingPlayer(mInstance->GetCreature(static_cast<uint32_t>(MuradinBronzebeardGbGUID)));
                    if (DoWipeCheck(skybreaker))
                        scriptEvents.addEvent(EVENT_WIPE_CHECK, 3000);
                    else
                        DoAction(ACTION_FAIL);
                }
                else
                {
                    DoCheckFallingPlayer(mInstance->GetCreature(static_cast<uint32_t>(DeathbringerSaurfangGbGUID)));
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

    void SendMusicToPlayers(uint32 musicId) const
    {
        WorldPacket data(SMSG_PLAY_MUSIC, 4);
        data << uint32(musicId);
        SendPacketToPlayers(&data);
    }

    // Send packet to all players
    void SendPacketToPlayers(WorldPacket* data) const
    {
        auto players = mInstance->m_PlayerStorage;
        if (players.size())
            for (auto itr = players.begin(); itr != players.end(); ++itr)
                if (Player* player = itr->second)
                    player->GetSession()->SendPacket(data);
    }

    void DoAction(int32 const action)
    {
        switch (action)
        {
        case ACTION_INTRO_START:
            scriptEvents.addEvent(EVENT_START_FLY, 2500);
            break;
        case ACTION_BATTLE_EVENT:
            scriptEvents.addEvent(EVENT_WIPE_CHECK, 5000);
            setData(DATA_GUNSHIP_EVENT, InProgress);
            SendMusicToPlayers(17289);
            sEventMgr.AddEvent(static_cast<Object*>(skybreaker), &Object::PlaySoundToSet, (uint32_t)17289, EVENT_UNK, 41000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            break;
        }
    }

    void TransporterEvents(Transporter* transport, uint32_t eventId) override
    {
        switch (eventId)
        {
        case EVENT_ENEMY_GUNSHIP_DESPAWN:
            if (getData(DATA_GUNSHIP_EVENT) == Finished)
                transport->GetMapMgr()->RemoveFromMapMgr(transport, true);
            break;
        case EVENT_ENEMY_GUNSHIP_COMBAT:
            if (Creature* captain = TeamInInstance == TEAM_HORDE ? mInstance->GetCreature(static_cast<uint32_t>(DeathbringerSaurfangGbGUID)) : mInstance->GetCreature(static_cast<uint32_t>(MuradinBronzebeardGbGUID)))
                captain->GetScript()->DoAction(ACTION_BATTLE_EVENT);
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
    bool DoWipeCheck(Transporter* t)
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

public:
    Transporter* skybreaker;
    Transporter* orgrimmar;

protected:
    IceCrownCitadelScript* Instance;
    uint8_t TeamInInstance;

    // Marrowgar
    uint32_t MarrowgarIcewall1GUID;
    uint32_t MarrowgarIcewall2GUID;
    uint32_t MarrowgarEntranceDoorGUID;

    // Lady Deathwhisper
    uint32_t LadyDeathwisperElevatorGUID;
    uint32_t LadyDeathwisperEntranceDoorGUID;

    // Gunship Event			
    uint64_t SkybreakerBossGUID;
    uint64_t OrgrimmarBossGUID;
    uint64_t DeathbringerSaurfangGbGUID;
    uint64_t MuradinBronzebeardGbGUID;
    uint64_t DeathbringerSaurfangNotVisualGUID;
    uint64_t MuradinBronzebeardNotVisualGUID;
    uint64_t GbBattleMageGUID;
    bool isPrepared;

    // Deathbringer Saurfang
    uint32_t DeathbringerDoorGUID;
    uint32_t DeathbringerSaurfangSpawnID;
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
    ADD_CREATURE_FACTORY_FUNCTION(LordMarrowgarAI)
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

    void OnLoad() override
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
                if (_isHeroic() || !getCreature()->HasAura(SPELL_BONE_STORM))
                    _castAISpell(boneSpikeGraveyardSpell);

                scriptEvents.addEvent(EVENT_BONE_SPIKE_GRAVEYARD, Util::getRandomInt(15000, 20000));
                break;
            case EVENT_COLDFLAME:
                coldflameLastPos = getCreature()->GetPosition();

                if (!getCreature()->HasAura(SPELL_BONE_STORM))
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
                    boneStormtarget = getBestPlayerTarget(TargetFilter_Aggroed);
                
                if (boneStormtarget)
                    getCreature()->GetAIInterface()->MoveTo(boneStormtarget->GetPositionX(), boneStormtarget->GetPositionY(), boneStormtarget->GetPositionZ());

                break;
            }
            case EVENT_BONE_STORM_END:  
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
        if (getCreature()->HasAura(SPELL_BONE_STORM))
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

    void DoAction(int32 const action) override
    {
        if (action != ACTION_CLEAR_SPIKE_IMMUNITIES)
            return;

        boneSpikeImmune.clear();
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
    ADD_CREATURE_FACTORY_FUNCTION(ColdflameAI)
    explicit ColdflameAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();     
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

        if (summoner->HasAura(SPELL_BONE_STORM))
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

    // Unit
    Unit* summoner;

    //Spells
    CreatureAISpells* coldflameTriggerSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Bone Spike
class BoneSpikeAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BoneSpikeAI)
    explicit BoneSpikeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        // Common
        hasTrappedUnit = false;
        summon = nullptr;

        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
    }

    void OnSummon(Unit* summoner) override
    {
        summon = summoner;
        // Make our Creature in Combat otherwise on Died Script wont trigger
        getCreature()->GetAIInterface()->setAiScriptType(AI_SCRIPT_AGRO);

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

    void OnDied(Unit* pTarget) override
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

            marrowgarAI->sendRandomDBChatMessage(emoteVector);  
        }
    }

    SpellCastResult onCanCast(Spell* spell, uint32_t* parameter1, uint32_t* parameter2) override
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

        Unit* mt = caster->GetAIInterface()->GetMostHated();
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

        if (target->HasAura(SPELL_IMPALED))
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
                coldflametarget = pCreature->GetScript()->getBestPlayerTarget(TargetFilter_Aggroed);

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

        Unit* target = nullptr;
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
        if (target->HasAura(SPELL_IMPALED))
            return false;

        if (target->GetDistance2dSq(spell->getUnitCaster()) > static_cast<float>(spell->getSpellInfo()->getEffectRadiusIndex(EFF_INDEX_0)) )
            return false;

        return true;
    }

    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effectIndex) override
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

    SpellCastResult onCanCast(Spell* spell, uint32_t* parameter1, uint32_t* parameter2) override
    {
        targetCount = 0;
        static_cast<Creature*>(spell->getUnitCaster())->GetScript()->DoAction(ACTION_CLEAR_SPIKE_IMMUNITIES);

        return SpellCastResult::SPELL_CAST_SUCCESS;
    }

    void filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets) override
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
    ADD_CREATURE_FACTORY_FUNCTION(LadyDeathwhisperAI)
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

    void OnLoad() override
    {
        if (!_isInCombat() && !getScriptPhase() == PHASE_INTRO)
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

        if (mInstance->GetDifficulty() != MODE_NORMAL_10MEN)
            scriptEvents.addEvent(EVENT_DOMINATE_MIND_H, 27000);

        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        _setMeleeDisabled(true);
        setRooted(true);

        getCreature()->RemoveAura(SPELL_SHADOW_CHANNELING);
        _castAISpell(manaBarrierSpell);
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        Reset();
    }

    ///\ todo Health Decreases visualy
    void DamageTaken(Unit* _attacker, uint32* damage) override
    {
        uint32_t currentMana = getCreature()->getPower(POWER_TYPE_MANA);
        // When Lady Deathwhsiper has her mana Barrier dont deal damage to her instead reduce her mana.      
        if (getCreature()->HasAura(SPELL_MANA_BARRIER) && *damage < currentMana)
        {
            uint32_t manareduction = 0;
            uint32_t maxHealth = getCreature()->getMaxHealth();

            getCreature()->setHealth(maxHealth);

            if (*damage < currentMana)
                manareduction = (currentMana - *damage);

            getCreature()->setPower(POWER_TYPE_MANA, manareduction);
            *damage = 1; // Hackfix health reduces visualy and by setting maxhealth when it has maxhealth dont updates healthbar
        }
    }

    void OnDamageTaken(Unit* _attacker, uint32_t damage) override
    {
        uint32_t currentMana = getCreature()->getPower(POWER_TYPE_MANA);
        // When Lady Deathwhsiper has her mana Barrier dont deal damage to her instead reduce her mana.
        // phase transition
        if (getScriptPhase() == PHASE_ONE && damage > currentMana)
        {
            sendDBChatMessage(SAY_LADY_PHASE_2);
            sendDBChatMessage(SAY_LADY_PHASE_2_EMOTE);
            setRooted(false);
            getCreature()->setPower(POWER_TYPE_MANA, 0);
            getCreature()->RemoveAura(SPELL_MANA_BARRIER);
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

        if (getCreature()->GetAIInterface()->getAiState() == AI_STATE_CASTING)
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
        if (getCreature()->HasAura(SPELL_MANA_BARRIER))
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

        if (mInstance->GetDifficulty() == MODE_NORMAL_25MEN || mInstance->GetDifficulty() == MODE_HEROIC_25MEN)
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
        if (mInstance->GetDifficulty() == MODE_NORMAL_25MEN || mInstance->GetDifficulty() == MODE_HEROIC_25MEN)
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
        if (summons.size())
            for (auto itr = summons.begin(); itr != summons.end();)
                (*itr)->Despawn(100, 0);
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

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Cult Adherent
class CultAdherentAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CultAdherentAI)
    explicit CultAdherentAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        temporalVisualSpell         = addAISpell(SPELL_TELEPORT_VISUAL, 0.0f, TARGET_SELF);
        frostFeverSpell             = addAISpell(SPELL_FROST_FEVER, 100.0f, TARGET_ATTACKING, 0, Util::getRandomUInt(10000, 12000));
        deathchillSpell             = addAISpell(SPELL_DEATHCHILL_BLAST, 100.0f, TARGET_ATTACKING, 0, Util::getRandomUInt(14000, 16000));
        curseOfTorporSpell          = addAISpell(SPELL_CURSE_OF_TORPOR, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(14000, 16000));
        shroudOfTheOccultSpell      = addAISpell(SPELL_SHORUD_OF_THE_OCCULT, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(32000, 39000));
        cultistDarkMartyrdomSpell   = addAISpell(SPELL_DARK_MARTYRDOM_ADHERENT, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(18000, 32000));
    }

    void OnLoad()
    {
        _castAISpell(temporalVisualSpell);
        auto NewTarget = getBestPlayerTarget(TargetFilter_Closest);
        getCreature()->GetAIInterface()->setNextTarget(NewTarget);
        getCreature()->GetAIInterface()->AttackReaction(NewTarget, 200);
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
    ADD_CREATURE_FACTORY_FUNCTION(CultFanaticAI)
    explicit CultFanaticAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        temporalVisualSpell         = addAISpell(SPELL_TELEPORT_VISUAL, 0.0f, TARGET_SELF);
        necroticStrikeSpell         = addAISpell(SPELL_NECROTIC_STRIKE, 100.0f, TARGET_ATTACKING, 0, Util::getRandomUInt(10000, 12000));
        shadowCleaveSpell           = addAISpell(SPELL_SHADOW_CLEAVE, 100.0f, TARGET_ATTACKING, 0 , Util::getRandomUInt(14000, 16000));
        vampireMightSpell           = addAISpell(SPELL_VAMPIRIC_MIGHT, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(20000, 27000));
        darkMartyrdomSpell          = addAISpell(SPELL_DARK_MARTYRDOM_ADHERENT, 100.0f, TARGET_SELF, 0 , Util::getRandomUInt(18000, 32000));
    }

    void OnLoad()
    {
        _castAISpell(temporalVisualSpell);
        auto NewTarget = getBestPlayerTarget(TargetFilter_Closest);
        getCreature()->GetAIInterface()->setNextTarget(NewTarget);
        getCreature()->GetAIInterface()->AttackReaction(NewTarget, 200);
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
//
//
//
//

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
    ADD_CREATURE_FACTORY_FUNCTION(MuradinAI)
    explicit MuradinAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();

        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    }

    void AIUpdate() override
    {
        if (getCreature()->GetAIInterface()->getAiState() == AI_STATE_CASTING)
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
                break;
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
                if (Creature* pSaurfang = getNearestCreature(NPC_GB_HIGH_OVERLORD_SAURFANG))
                    pSaurfang->GetScript()->sendDBChatMessage(SAY_BOARDING_SKYBREAKER_0);
                break;
            case EVENT_INTRO_ALLIANCE_8:
                sendDBChatMessage(SAY_INTRO_ALLIANCE_7);
                break;
            }
        }
    }

    void DoAction(int32 const action)
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
    ADD_CREATURE_FACTORY_FUNCTION(SaurfangAI)
        explicit SaurfangAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();

        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    }

    void AIUpdate() override
    {
        if (getCreature()->GetAIInterface()->getAiState() == AI_STATE_CASTING)
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
                break;
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
                if (Creature* pSaurfang = getNearestCreature(NPC_GB_MURADIN_BRONZEBEARD))
                    pSaurfang->GetScript()->sendDBChatMessage(SAY_INTRO_ALLIANCE_7);
                break;
            case EVENT_INTRO_HORDE_5:
                sendDBChatMessage(SAY_INTRO_HORDE_4);
                break;
            }
        }
    }

    void DoAction(int32 const action)
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
            break;
        }
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;
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
    ADD_CREATURE_FACTORY_FUNCTION(MuradinSaurfangEvent)
        explicit MuradinSaurfangEvent(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();

        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
 
        AddWaypoint(CreateWaypoint(POINT_FIRST_STEP, 0, Movement::WP_MOVE_TYPE_WALK, firstStepPos));
        AddWaypoint(CreateWaypoint(POINT_CHARGE, 0, Movement::WP_MOVE_TYPE_RUN, chargePos[0]));
        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
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
                getCreature()->GetAIInterface()->StopMovement(0);

                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(POINT_FIRST_STEP);

                getCreature()->GetAIInterface()->setCreatureState(MOVING);
            break;
            }
            case EVENT_INTRO_ALLIANCE_5_SE:
            {
                sendDBChatMessage(SAY_INTRO_ALLIANCE_5_SE);
                
                SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
                getCreature()->GetAIInterface()->setWayPointToMove(POINT_CHARGE);            
            break;
            }
            }
        }
    }

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        switch (iWaypointId)
        {
        case POINT_FIRST_STEP:
        {
            sendDBChatMessage(SAY_INTRO_ALLIANCE_4_SE);

            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_5_SE, 5000, PHASE_INTRO_A);

            if (Creature* deathbringer = mInstance->getCreatureBySpawnId(mInstance->getLocalData(DATA_DEATHBRINGER_SAURFANG)))
                deathbringer->GetScript()->DoAction(ACTION_CONTINUE_INTRO);
            break;
        }
        }
    }

    void DoAction(int32 const action) override
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
            for (auto itr = _guardList.begin(); itr != _guardList.end(); ++x, ++itr) //37830
            {
                ;
            }
            //

            setScriptPhase(PHASE_INTRO_A);
            sendDBChatMessage(SAY_INTRO_ALLIANCE_1_SE);

            // Start Intro
            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_4_SE, 29500, PHASE_INTRO_A);

            // Open Suarfangs Door
            if (GameObject* Door = mInstance->GetGameObjectByGuid(mInstance->getLocalData(DATA_SAURFANG_DOOR)))
                Door->setState(GO_STATE_OPEN);

            // Start Intro on Suarfang        
            if (Creature* deathbringer = mInstance->getCreatureBySpawnId(mInstance->getLocalData(DATA_DEATHBRINGER_SAURFANG)))
                deathbringer->GetScript()->DoAction(PHASE_INTRO_A);

            // Clear NPC FLAGS
            getCreature()->removeNpcFlags(UNIT_NPC_FLAG_GOSSIP);
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
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
    ADD_CREATURE_FACTORY_FUNCTION(OverlordSaurfangEvent)
        explicit OverlordSaurfangEvent(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();

        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

        AddWaypoint(CreateWaypoint(POINT_FIRST_STEP, 0, Movement::WP_MOVE_TYPE_WALK, firstStepPos));
        AddWaypoint(CreateWaypoint(POINT_CHARGE, 0, Movement::WP_MOVE_TYPE_RUN, chargePos[0]));
        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
    }

    void AIUpdate() override
    {
        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_INTRO_HORDE_3_SE:
                getCreature()->GetAIInterface()->StopMovement(0);

                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(POINT_FIRST_STEP);

                getCreature()->GetAIInterface()->setCreatureState(MOVING);
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
                SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
                getCreature()->GetAIInterface()->setWayPointToMove(POINT_CHARGE);
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

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        switch (iWaypointId)
        {
        case POINT_FIRST_STEP:
        {
            sendDBChatMessage(SAY_INTRO_HORDE_3_SE);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_5_SE, 15500, PHASE_INTRO_H);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_6_SE, 29500, PHASE_INTRO_H);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_7_SE, 43800, PHASE_INTRO_H);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_8_SE, 47000, PHASE_INTRO_H);

            if (Creature* deathbringer = mInstance->getCreatureBySpawnId(mInstance->getLocalData(DATA_DEATHBRINGER_SAURFANG)))
                deathbringer->GetScript()->DoAction(ACTION_CONTINUE_INTRO);
            break;
        }
        }
    }

    void DoAction(int32 const action)
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
            for (auto itr = _guardList.begin(); itr != _guardList.end(); ++x, ++itr) // 37920
            {
                ;
            }
            //

            sendDBChatMessage(SAY_INTRO_HORDE_1_SE);
            setScriptPhase(PHASE_INTRO_H);

            // Start Intro
            scriptEvents.addEvent(EVENT_INTRO_HORDE_3_SE, 18500, PHASE_INTRO_H);

            // Open Suarfangs Door
            if (GameObject* Door = mInstance->GetGameObjectByGuid(mInstance->getLocalData(DATA_SAURFANG_DOOR)))
                Door->setState(GO_STATE_OPEN);

            // Start Intro on Suarfang        
            if (Creature* deathbringer = mInstance->getCreatureBySpawnId(mInstance->getLocalData(DATA_DEATHBRINGER_SAURFANG)))
                deathbringer->GetScript()->DoAction(PHASE_INTRO_H);

            // Clear NPC FLAGS
            getCreature()->removeUnitFlags(UNIT_NPC_FLAG_GOSSIP);
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);

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
    ADD_CREATURE_FACTORY_FUNCTION(DeathbringerSaurfangAI)
        explicit DeathbringerSaurfangAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();
        getCreature()->EnableAI();
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

        AddWaypoint(CreateWaypoint(POINT_SAURFANG, 0, Movement::WP_MOVE_TYPE_WALK, deathbringerPos));
        AddWaypoint(CreateWaypoint(POINT_FIRST_STEP, 0, Movement::WP_MOVE_TYPE_WALK, deathbringerPos));
        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
        _introDone = false;
    }

    void AIUpdate() override
    {
        if (getCreature()->GetAIInterface()->getAiState() == AI_STATE_CASTING)
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_INTRO_ALLIANCE_2_SE:
                getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                getCreature()->SetFaction(974);
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_ALLIANCE_2);
                break;
            case EVENT_INTRO_ALLIANCE_3_SE:
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_ALLIANCE_3);
                break;
            case EVENT_INTRO_ALLIANCE_6_SE:
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_ALLIANCE_6);
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_ALLIANCE_7);
                // \Todo Cast SPELL_GRIP_OF_AGONY
                break;
            case EVENT_INTRO_HORDE_2_SE:
                getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                getCreature()->SetFaction(974);
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_HORDE_2);
                break;
            case EVENT_INTRO_HORDE_4_SE:
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_HORDE_4);
                break;
            case EVENT_INTRO_HORDE_9_SE:
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_HORDE_9);
                // \Todo Cast SPELL_GRIP_OF_AGONY
                break;
            case EVENT_INTRO_FINISH_SE:
                setScriptPhase(PHASE_COMBAT);
                _introDone = true;
                break;
            }
        }
    }

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        switch (iWaypointId)
        {
        case POINT_SAURFANG:
        {
            // Close Suarfangs Door
            if (GameObject* Door = mInstance->GetGameObjectByGuid(mInstance->getLocalData(DATA_SAURFANG_DOOR)))
                Door->setState(GO_STATE_CLOSED);
            break;
        }
        }
    }

    void DoAction(int32 const action)
    {
        switch (action)
        {
        case PHASE_INTRO_A:
        case PHASE_INTRO_H:
        {     
            setScriptPhase(uint32(action));

            // Move
            getCreature()->GetAIInterface()->StopMovement(0);

            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
            getCreature()->GetAIInterface()->setWayPointToMove(POINT_SAURFANG);

            getCreature()->GetAIInterface()->setCreatureState(MOVING);

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
            scriptEvents.addEvent(EVENT_INTRO_FINISH_SE, 8000, PHASE_INTRO_A);

            scriptEvents.addEvent(EVENT_INTRO_HORDE_4_SE, 6500, PHASE_INTRO_H);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_9_SE, 48200, PHASE_INTRO_H);
            scriptEvents.addEvent(EVENT_INTRO_FINISH_SE, 55700, PHASE_INTRO_H);
            break;
        }
        }
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;
    bool _introDone;
};
#endif
void SetupICC(ScriptMgr* mgr)
{
#if VERSION_STRING >= WotLK
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

    // Spell Cultist Dark Martyrdom
    mgr->register_spell_script(SPELL_DARK_MARTYRDOM_ADHERENT, new DarkMartyrdom);

    //Gossips
    GossipScript* MuradinGossipScript = new MuradinGossip();
    mgr->register_creature_gossip(NPC_GB_MURADIN_BRONZEBEARD, MuradinGossipScript);
    GossipScript* SaurfangGossipScript = new SaurfangGossip();
    mgr->register_creature_gossip(NPC_GB_HIGH_OVERLORD_SAURFANG, SaurfangGossipScript);
    GossipScript* MuradinSaurfangEventGossipScript = new MuradinSeGossip();
    mgr->register_creature_gossip(NPC_SE_MURADIN_BRONZEBEARD, MuradinSaurfangEventGossipScript);
    GossipScript* OverlordSaurfangEventGossipScript = new OverlordSeGossip();
    mgr->register_creature_gossip(NPC_SE_HIGH_OVERLORD_SAURFANG, OverlordSaurfangEventGossipScript);

    //Misc
    mgr->register_creature_script(NPC_COLDFLAME, &ColdflameAI::Create);
    mgr->register_creature_script(NPC_BONE_SPIKE, &BoneSpikeAI::Create);

    mgr->register_creature_script(NPC_CULT_FANATIC, CultFanaticAI::Create);
    mgr->register_creature_script(NPC_CULT_ADHERENT, CultAdherentAI::Create);
#endif
}
