/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_IceCrownCitadel.h"
#include "Objects/Faction.h"

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
enum IceCrown_Encounters
{
    DATA_LORD_MARROWGAR,
    DATA_COLDFLAME,
    DATA_BONE_SPIKE,
    DATA_LADY_DEATHWHISPER,
    DATA_VALITHRIA_DREAM,

    ICC_DATA_END
};

//////////////////////////////////////////////////////////////////////////////////////////
//IceCrownCitadel Instance
class IceCrownCitadelScript : public InstanceScript
{
public:

    explicit IceCrownCitadelScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        if (getData(CN_LORD_MARROWGAR) == Finished)
        {
            setGameObjectStateForEntry(GO_MARROWGAR_ICEWALL_1, GO_STATE_CLOSED);    // Icewall 1
            setGameObjectStateForEntry(GO_MARROWGAR_ICEWALL_2, GO_STATE_CLOSED);    // Icewall 2
            setGameObjectStateForEntry(GO_MARROWGAR_DOOR, GO_STATE_CLOSED);         // Door
        }

        // test timers
        addTimer(75000);
        addTimer(5000);
        addTimer(35000);
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new IceCrownCitadelScript(pMapMgr); }

    void UpdateEvent() override
    {
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        // Gos which are not visible by killing a boss needs a second check...
        if (getData(CN_LORD_MARROWGAR) == Finished)
        {
            setGameObjectStateForEntry(GO_MARROWGAR_ICEWALL_1, GO_STATE_OPEN);    // Icewall 1
            setGameObjectStateForEntry(GO_MARROWGAR_ICEWALL_2, GO_STATE_OPEN);    // Icewall 2
            setGameObjectStateForEntry(GO_MARROWGAR_DOOR, GO_STATE_OPEN);         // Door
        }

        switch (pGameObject->getEntry())
        {
            case GO_TELE_1:
            case GO_TELE_2:
            case GO_TELE_3:
            case GO_TELE_4:
            case GO_TELE_5:
            {
                pGameObject->setFlags(GO_FLAG_NONE);
            } break;
        }
    }

    void OnCreatureDeath(Creature* pCreature, Unit* /*pUnit*/) override
    {
        switch (pCreature->getEntry())
        {
            case CN_LORD_MARROWGAR:
            {
                setGameObjectStateForEntry(GO_MARROWGAR_ICEWALL_1, GO_STATE_OPEN);    // Icewall 1
                setGameObjectStateForEntry(GO_MARROWGAR_ICEWALL_2, GO_STATE_OPEN);    // Icewall 2
                setGameObjectStateForEntry(GO_MARROWGAR_DOOR, GO_STATE_OPEN);         // Door
            }break;
            default:
            break;
        }
    }

    void OnPlayerEnter(Player* player) override
    {
        if (!spawnsCreated())
        {
            // setup only the npcs with the correct team...
            switch (player->getTeam())
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
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// IceCrown Teleporter
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
        //menu.addItem(GOSSIP_ICON_CHAT, (518), 3);        // Teleport to Deathbringer's Rise.

        if (pInstance->getData(CN_VALITHRIA_DREAMWALKER) == Finished)
            menu.addItem(GOSSIP_ICON_CHAT, 519, 4);      // Teleport to the Upper Spire.

        if (pInstance->getData(CN_COLDFLAME) == Finished)
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
// Boss: Lord Marrowgar
const uint32_t LM_BERSERK = 47008;
const uint32_t BONE_SLICE = 69055;
const uint32_t BONE_SPIKE = 69057;       // Not shure about this
const uint32_t BONE_STORM = 69076;
const uint32_t SOUL_FEAST = 71203;       // Needs a script

class LordMarrowgarAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LordMarrowgarAI)
    explicit LordMarrowgarAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        sendDBChatMessage(922);      // This is the beginning AND the end, mortals. None may enter the master's sanctum!

        // examplecode for spell setup
        auto boneslice = addAISpell(BONE_SLICE, 60.0f, TARGET_ATTACKING, 0, 120);
        boneslice->addEmote("boneslice", CHAT_MSG_MONSTER_YELL, 0);
        boneslice->setAvailableForScriptPhase({ 2 });

        auto bonestorm = addAISpell(BONE_STORM, 30.0f, TARGET_DESTINATION, 30, 300, true);
        bonestorm->addEmote("bonestorm", CHAT_MSG_MONSTER_YELL, 0);
        bonestorm->setAvailableForScriptPhase({ 4 });
        bonestorm->setAttackStopTimer(3000);

        auto berserk = addAISpell(LM_BERSERK, 50.0f, TARGET_SELF, 30, 240);
        berserk->addEmote("berserk", CHAT_MSG_MONSTER_YELL, 0);
        berserk->setMaxStackCount(1);
        berserk->setMinMaxPercentHp(0, 50);
        berserk->setAvailableForScriptPhase({ 3, 5 });

        auto souldFest = addAISpell(SOUL_FEAST, 50.0f, TARGET_RANDOM_SINGLE, 0, 20);
        souldFest->addEmote("Your soul is fest", CHAT_MSG_MONSTER_YELL, 0);
        souldFest->setAvailableForScriptPhase({ 2 });

        auto bonespike = addAISpell(BONE_SPIKE, 80.0f, TARGET_RANDOM_SINGLE, 10, 30);
        bonespike->setAnnouncement("Lord Marrowgar is preparing BoneSpike");
        bonespike->addEmote("bonespike", CHAT_MSG_MONSTER_YELL, 0);
        bonespike->addDBEmote(925);      // Bound by bone!
        bonespike->addDBEmote(926);      // Stick around!
        bonespike->addDBEmote(927);      // The only escape is death!
        bonespike->setAvailableForScriptPhase({ 2 });
        bonespike->setMinMaxDistance(10.0f, 500.0f);

        // example for random message on event
        addEmoteForEvent(Event_OnCombatStart, 923);     // The Scourge will wash over this world as a swarm of death and destruction!
        addEmoteForEvent(Event_OnTargetDied, 928);      // More bones for the offering!
        addEmoteForEvent(Event_OnTargetDied, 929);      // Languish in damnation!
        addEmoteForEvent(Event_OnDied, 930);            // I see... Only darkness.
    }

    void AIUpdate() override
    {
    }

    void OnCastSpell(uint32_t /*spellId*/) override
    {
    }

    void OnHitBySpell(uint32_t pSpellId, Unit* pUnitCaster) override
    {
        switch (pSpellId)
        {
            case 49233:
            {
                if (pUnitCaster != nullptr && pUnitCaster->isPlayer())
                {
                    std::stringstream ss;
                    ss << "Player " << static_cast<Player*>(pUnitCaster)->getName().c_str();
                    sendAnnouncement(ss.str());
                }
            } break;
            case 9770:
            {
                if (pUnitCaster != nullptr && pUnitCaster->isPlayer())
                {
                    std::stringstream ss;
                    ss << static_cast<Player*>(pUnitCaster)->getName().c_str() << " hits me with aura 9770... damnit! ";
                    sendAnnouncement(ss.str());
                }
            } break;
            case 9798:
            {
                std::stringstream ss;
                ss << "9798 hit me..... damnit! ";
                sendAnnouncement(ss.str());
            } break;
        }
    }

    // Testcode - remove me please
    void OnScriptPhaseChange(uint32_t scriptPhase) override
    {
        // Testcode - remove me please
        std::stringstream ss;
        ss << "My scriptPhase is now " << scriptPhase;

        sendAnnouncement(ss.str());

        switch (scriptPhase)
        {
            case 1:
                despawn(60000, 1000);
                break;
            case 2:
                despawn(2000, 10000);
                break;
        }
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
    }

    void OnDied(Unit* /*pTarget*/) override
    {
    }
};

const uint32_t IMPALED = 69065;

class BoneSpikeAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BoneSpikeAI)
    explicit BoneSpikeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);  // On wowhead they said "kill them not just looking at them".
        getCreature()->Despawn(8000, 0);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
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
#ifndef UseNewMapScriptsProject
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
    //mgr->register_creature_script(CN_LADY_DEATHWHISPER, &LadyDeathwhisperAI::Create);
    //mgr->register_creature_script(CN_VALITHRIA_DREAMWALKER, &ValithriaDreamwalkerAI::Create);
    //mgr->register_creature_script(CN_COLDFLAME, &ColdFlameAI::Create);

    //Misc
    mgr->register_creature_script(CN_BONE_SPIKE, &BoneSpikeAI::Create);
#endif
}
