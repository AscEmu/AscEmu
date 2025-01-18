/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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
#include "Management/QuestLogEntry.hpp"
#include "Management/QuestMgr.h"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Spell/Spell.hpp"
#include "Storage/MySQLDataStore.hpp"

enum 
{
    // Hunt Is On (Quest: 11794)
    QUEST_HUNT_IS_ON = 11794,

    NPC_SURRISTRASZ = 24795,
    GI_SURRISTRASZ = 191,   // "May I use a drake to fly elsewhere?"   

    //SPELL_ABMER_TO_COLDARRA = 46064

    // Neutralizing the Cauldrons
    CN_PURIFYING_TOTEM = 25494
};

//////////////////////////////////////////////////////////////////////////////////////////
// Call to Arms!
class BellRope : public GameObjectAIScript
{
public:
    explicit BellRope(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new BellRope(GO); };

    void OnActivate(Player* pPlayer) override
    {
        pPlayer->addQuestKill(11965, 0, 0);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Reading the Meters
class ColdarraGeoMonitorNexus : public GameObjectAIScript
{
public:
    explicit ColdarraGeoMonitorNexus(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new ColdarraGeoMonitorNexus(GO); };

    void OnActivate(Player* pPlayer) override
    {
        pPlayer->addQuestKill(11900, 0, 0);
    }
};

class ColdarraGeoMonitorSouth : public GameObjectAIScript
{
public:
    explicit ColdarraGeoMonitorSouth(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new ColdarraGeoMonitorSouth(GO); };

    void OnActivate(Player* pPlayer) override
    {
        pPlayer->addQuestKill(11900, 1, 0);
    }
};

class ColdarraGeoMonitorNorth : public GameObjectAIScript
{
public:
    explicit ColdarraGeoMonitorNorth(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new ColdarraGeoMonitorNorth(GO); };

    void OnActivate(Player* pPlayer) override
    {
        pPlayer->addQuestKill(11900, 2, 0);
    }
};

class ColdarraGeoMonitorWest : public GameObjectAIScript
{
public:
    explicit ColdarraGeoMonitorWest(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new ColdarraGeoMonitorWest(GO); };

    void OnActivate(Player* pPlayer) override
    {
        pPlayer->addQuestKill(11900, 3, 0);
    }
};

class PurifyingTotemAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PurifyingTotemAI(c); }
    explicit PurifyingTotemAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        setCanEnterCombat(false);
        setRooted(true);
        despawn(8000, 0);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Cutting Off the Source
class NerubarEggSac : public GameObjectAIScript
{
public:
    explicit NerubarEggSac(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new NerubarEggSac(GO); };

    void OnActivate(Player* pPlayer) override
    {
        pPlayer->addQuestKill(11602, 0, 0);

        _gameobject->setState(GO_STATE_CLOSED);
        _gameobject->setState(GO_STATE_OPEN);
        _gameobject->despawn(500, 60000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Bury Those Cockroaches!
class SeaforiumDepthCharge : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SeaforiumDepthCharge(c); }
    explicit SeaforiumDepthCharge(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        setRooted(true);
        setCanEnterCombat(false);
        getCreature()->setFaction(21);
    }

    void OnLoad() override
    {
        if (!getCreature()->isSummon())
            return;

        Unit* summoner = static_cast<Summon*>(getCreature())->getUnitOwner();

        if (summoner != nullptr)
        {
            if (summoner->isPlayer())
            {
                Player* p = static_cast<Player*>(summoner);
                if (p->hasQuestInQuestLog(11608))
                {
                    GameObject* pSinkhole = p->getWorldMap()->getInterface()->getGameObjectNearestCoords(p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), 300171);
                    if (pSinkhole != nullptr)
                    {
                        getCreature()->castSpell(getCreature(), 45502, true);

                        float posX = pSinkhole->GetPositionX();
                        if (posX == 2657.13f)
                        {
                            p->addQuestKill(11608, 0, 0);
                        }

                        if (posX == 2716.02f)
                        {
                            p->addQuestKill(11608, 1, 0);
                        }

                        if (posX == 2877.96f)
                        {
                            p->addQuestKill(11608, 2, 0);
                        }

                        if (posX == 2962.16f)
                        {
                            p->addQuestKill(11608, 3, 0);
                        }
                    }
                }
            }
        }
        getCreature()->Despawn(500, 0);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hatching a Plan
class BlueDragonEgg : public GameObjectAIScript
{
public:
    explicit BlueDragonEgg(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new BlueDragonEgg(GO); };

    void OnActivate(Player* pPlayer) override
    {
        pPlayer->addQuestKill(11936, 0, 0);

        //\todo why setting gameobject state 1 and 0?!?!
        _gameobject->setState(GO_STATE_CLOSED);
        _gameobject->setState(GO_STATE_OPEN);
        _gameobject->despawn(500, 60000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Quest: The Machagnomes
enum eFizzcrank
{
    NPC_FIZZCRANK = 25590,

    GOSSIP_TEXTID_FIZZCRANK1 = 12456,
    GOSSIP_TEXTID_FIZZCRANK2 = 12457,
    GOSSIP_TEXTID_FIZZCRANK3 = 12458,
    GOSSIP_TEXTID_FIZZCRANK4 = 12459,
    GOSSIP_TEXTID_FIZZCRANK5 = 12460,
    GOSSIP_TEXTID_FIZZCRANK6 = 12461,
    GOSSIP_TEXTID_FIZZCRANK7 = 12462,
    GOSSIP_TEXTID_FIZZCRANK8 = 12463,
    GOSSIP_TEXTID_FIZZCRANK9 = 12464,

    GOSSIP_OPTION_FIZZCRANK_1 = 190,      // Tell me what's going on out here, Fizzcrank.
    GOSSIP_OPTION_FIZZCRANK_2 = 189,      // Go on.

    QUEST_THE_MECHAGNOMES = 11708
};

class FizzcrankGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* pPlayer) override
    {
        GossipMenu menu(pObject->getGuid(), 12435, pPlayer->getSession()->language);
        if (pPlayer->hasQuestInQuestLog(QUEST_THE_MECHAGNOMES))
            menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_FIZZCRANK_1, 1);

        menu.sendGossipPacket(pPlayer);
    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                GossipMenu menu(pObject->getGuid(), GOSSIP_TEXTID_FIZZCRANK1, pPlayer->getSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_FIZZCRANK_2, 2);
                menu.sendGossipPacket(pPlayer);
            }break;
            case 2:
            {
                GossipMenu menu(pObject->getGuid(), GOSSIP_TEXTID_FIZZCRANK2, pPlayer->getSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_FIZZCRANK_2, 3);
                menu.sendGossipPacket(pPlayer);
            }break;
            case 3:
            {
                GossipMenu menu(pObject->getGuid(), GOSSIP_TEXTID_FIZZCRANK3, pPlayer->getSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_FIZZCRANK_2, 4);
                menu.sendGossipPacket(pPlayer);
            }break;
            case 4:
            {
                GossipMenu menu(pObject->getGuid(), GOSSIP_TEXTID_FIZZCRANK4, pPlayer->getSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_FIZZCRANK_2, 5);
                menu.sendGossipPacket(pPlayer);
            }break;
            case 5:
            {
                GossipMenu menu(pObject->getGuid(), GOSSIP_TEXTID_FIZZCRANK5, pPlayer->getSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_FIZZCRANK_2, 6);
                menu.sendGossipPacket(pPlayer);
            }break;
            case 6:
            {
                GossipMenu menu(pObject->getGuid(), GOSSIP_TEXTID_FIZZCRANK6, pPlayer->getSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_FIZZCRANK_2, 7);
                menu.sendGossipPacket(pPlayer);
            }break;
            case 7:
            {
                GossipMenu menu(pObject->getGuid(), GOSSIP_TEXTID_FIZZCRANK7, pPlayer->getSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_FIZZCRANK_2, 8);
                menu.sendGossipPacket(pPlayer);
            }break;
            case 8:
            {
                GossipMenu menu(pObject->getGuid(), GOSSIP_TEXTID_FIZZCRANK8, pPlayer->getSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_FIZZCRANK_2, 9);
                menu.sendGossipPacket(pPlayer);
            }break;
            case 9:
            {
                GossipMenu::sendSimpleMenu(pObject->getGuid(), GOSSIP_TEXTID_FIZZCRANK9, pPlayer);
            }break;
        }
    }
};

///\todo: GOSSIP_ITEM_FREE_FLIGHT "I'd like passage to the Transitus Shield." this is not blizzlike...
class SurristraszGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* pPlayer) override
    {
        uint32_t Text = sMySQLStore.getGossipTextIdForNpc(pObject->getEntry());

        // check if there is a entry in the db
        if (sMySQLStore.getNpcGossipText(Text) == nullptr)
            Text = DefaultGossipTextId;

        GossipMenu menu(pObject->getGuid(), Text, pPlayer->getSession()->language);
        sQuestMgr.FillQuestMenu(static_cast<Creature*>(pObject), pPlayer, menu);

        menu.addItem(GOSSIP_ICON_FLIGHTMASTER, GI_SURRISTRASZ, 1);

        menu.sendGossipPacket(pPlayer);
    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        pPlayer->getSession()->sendTaxiMenu(static_cast<Creature*>(pObject));
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Quest: Lefty Loosey, Righty Tighty ID: 11788
// West Point Station Valve
class WestPointStationValve : public GameObjectAIScript
{
public:
    explicit WestPointStationValve(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new WestPointStationValve(GO); };

    void OnActivate(Player* pPlayer) override
    {
        if (!pPlayer->hasQuestInQuestLog(11788) || pPlayer->hasQuestFinished(11788))
            return;

        auto* questLog = pPlayer->getQuestLogByQuestId(11788);
        if (questLog == nullptr)
            return;

        if (questLog->getMobCountByIndex(0) != 0)
            return;

        Creature* Twonky = pPlayer->getWorldMap()->createAndSpawnCreature(25830, LocationVector(4117.513672f, 5089.670898f, -1.506265f, 2.043593f));
        if (Twonky->isAlive())
            _gameobject->setState(GO_STATE_OPEN);
        else
            _gameobject->setState(GO_STATE_CLOSED);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// North Point Station Valve
class NorthPointStationValve : public GameObjectAIScript
{
public:
    explicit NorthPointStationValve(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new NorthPointStationValve(GO); };

    void OnActivate(Player* pPlayer) override
    {
        if (!pPlayer->hasQuestInQuestLog(11788) || pPlayer->hasQuestFinished(11788))
            return;

        auto* questLog = pPlayer->getQuestLogByQuestId(11788);
        if (questLog == nullptr)
            return;

        if (questLog->getMobCountByIndex(1) != 0)
            return;

        Creature* Ed210 = pPlayer->getWorldMap()->createAndSpawnCreature(25831, LocationVector(4218.529785f, 4802.284668f, -12.975346f, 5.833142f));
        if (Ed210->isAlive())
            _gameobject->setState(GO_STATE_OPEN);
        else
            _gameobject->setState(GO_STATE_CLOSED);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Fizzcrank Pumping Station Valve
class FizzcrankPumpingStationValve : public GameObjectAIScript
{
public:
    explicit FizzcrankPumpingStationValve(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new FizzcrankPumpingStationValve(GO); };

    void OnActivate(Player* pPlayer) override
    {
        if (!pPlayer->hasQuestInQuestLog(11788) || pPlayer->hasQuestFinished(11788))
            return;

        auto* questLog = pPlayer->getQuestLogByQuestId(11788);
        if (questLog == nullptr)
            return;

        if (questLog->getMobCountByIndex(2) != 0)
            return;

        Creature* MaxBlasto = pPlayer->getWorldMap()->createAndSpawnCreature(25832, LocationVector(4029.974609f, 4890.195313f, -12.775084f, 1.081481f));
        if (MaxBlasto->isAlive())
            _gameobject->setState(GO_STATE_OPEN);
        else
            _gameobject->setState(GO_STATE_CLOSED);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// South Point Station Valve
class SouthPointStationValve : public GameObjectAIScript
{
public:
    explicit SouthPointStationValve(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new SouthPointStationValve(GO); };

    void OnActivate(Player* pPlayer) override
    {
        if (!pPlayer->hasQuestInQuestLog(11788) || pPlayer->hasQuestFinished(11788))
            return;

        auto* questLog = pPlayer->getQuestLogByQuestId(11788);
        if (questLog == nullptr)
            return;

        if (questLog->getMobCountByIndex(3) != 0)
            return;

        Creature* TheGrinder = pPlayer->getWorldMap()->createAndSpawnCreature(25833, LocationVector(3787.021484f, 4821.941895f, -12.967110f, 5.097224f));
        if (TheGrinder->isAlive())
            _gameobject->setState(GO_STATE_OPEN);
        else
            _gameobject->setState(GO_STATE_CLOSED);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Quest: The Gearmaster 11798
// The Gearmaster's Manual
class TheGearmastersManual : public GameObjectAIScript
{
public:
    explicit TheGearmastersManual(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new TheGearmastersManual(GO); };

    void OnActivate(Player* pPlayer) override
    {
        if (!pPlayer->hasQuestInQuestLog(11798) || pPlayer->hasQuestFinished(11798))
            return;

        auto* questLog = pPlayer->getQuestLogByQuestId(11798);
        if (questLog == nullptr)
            return;

        if (questLog->getMobCountByIndex(0) == 0)
        {
            questLog->setMobCountForIndex(0, 1);
            questLog->sendUpdateAddKill(0);
            questLog->updatePlayerFields();
        }

        Creature* GearmasterMechazod = pPlayer->getWorldMap()->createAndSpawnCreature(25834, LocationVector(4006.289551f, 4848.437500f, 25.957747f, 2.459837f));
        GearmasterMechazod->setTargetGuid(pPlayer->getGuid());
        if (GearmasterMechazod->isAlive())
            _gameobject->setState(GO_STATE_OPEN);
        else
            _gameobject->setState(GO_STATE_CLOSED);
    }
};

///\todo: Change to spellevent (target player), npc say is not ready yet. Add Visual Aura on Spawn.
class GearmasterMechazodAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GearmasterMechazodAI(c); }
    explicit GearmasterMechazodAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->setVirtualItemSlotId(MELEE, 28487);
        getCreature()->setVirtualItemSlotId(OFFHAND, 11587);
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        RegisterAIUpdateEvent(100);
        phase = 0;
    }

    void AIUpdate() override
    {
        switch (phase)
        {
            case 0:
            {
                RemoveAIUpdateEvent();
                getCreature()->SendScriptTextChatMessage(8736);
                RegisterAIUpdateEvent(7000);
                phase = 1;
            }break;
            case 1:
            {
                getCreature()->SendScriptTextChatMessage(8737);

                phase = 2;
            }break;
            case 2:
            {
                getCreature()->SendScriptTextChatMessage(8738);

                phase = 3;
            }break;
            case 3:
            {
                getCreature()->SendScriptTextChatMessage(8739);

                phase = 4;
            }break;
            case 4:
            {
                getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
                getCreature()->setControlled(true, UNIT_STATE_ROOTED);
                RemoveAIUpdateEvent();          // Remove Update, now we are in OnCombatStart
            }break;
            default:
                break;
        }
    }

protected:
    uint32_t phase;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hunt Is On (Quest: 11794)
class SaltyJohnGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* pPlayer) override
    {
        if (pPlayer->hasQuestInQuestLog(QUEST_HUNT_IS_ON) && pPlayer->hasAurasWithId(46078))
        {
            GossipMenu menu(pObject->getGuid(), 12435, pPlayer->getSession()->language);
            menu.addItem(GOSSIP_ICON_CHAT, 603, 1);
            menu.sendGossipPacket(pPlayer);
        }
    }

    void onSelectOption(Object* pObject, Player* /*pPlayer*/, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Creature* SaltyJohn = static_cast<Creature*>(pObject);
        SaltyJohn->setFaction(14);
        SaltyJohn->sendChatMessage(12, 0, "I suppose this is it.. then? I won't go down quietly!");
    }
};

class TomHeggerGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* pPlayer) override
    {
        if (pPlayer->hasQuestInQuestLog(QUEST_HUNT_IS_ON) && pPlayer->hasAurasWithId(46078))
        {
            GossipMenu menu(pObject->getGuid(), 12435, pPlayer->getSession()->language);
            menu.addItem(GOSSIP_ICON_CHAT, 604, 1);
            menu.sendGossipPacket(pPlayer);
        }
    }

    void onSelectOption(Object* pObject, Player* /*pPlayer*/, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Creature* TomHegger = static_cast<Creature*>(pObject);
        TomHegger->setFaction(14);
        TomHegger->sendChatMessage(12, 0, "You don't know who you're messing with, ! Death beckons!");
    }
};

class GuardMitchGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* pPlayer) override
    {
        if (pPlayer->hasQuestInQuestLog(QUEST_HUNT_IS_ON) && pPlayer->hasAurasWithId(46078))
        {
            GossipMenu menu(pObject->getGuid(), 12435, pPlayer->getSession()->language);
            menu.addItem(GOSSIP_ICON_CHAT, 605, 1);
            menu.sendGossipPacket(pPlayer);
        }
    }

    void onSelectOption(Object* pObject, Player* /*pPlayer*/, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Creature* GuardMitch = static_cast<Creature*>(pObject);
        GuardMitch->setFaction(14);
        GuardMitch->sendChatMessage(12, 0, "Finally! This charade is over... Arthas give me strength!");
    }
};

bool PlaceCart(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    Creature* pCreature = pSpell->getTargetConstraintCreature();
    auto* questLog = pPlayer->getQuestLogByQuestId(11897);
    if (questLog == nullptr)
        return true;

    if (pCreature->getEntry() == 26248)
    {
        if (questLog->getMobCountByIndex(2) == 0)
        {
            pCreature->castSpell(pCreature, 46798, true);
            pCreature->castSpell(pCreature, 46799, true);
            pCreature->castSpell(pCreature, 46800, true);
        }

        pPlayer->addQuestKill(11897, 2, 0);
    }

    if (pCreature->getEntry() == 26249)
    {
        if (questLog->getMobCountByIndex(1) == 0)
        {
            pCreature->castSpell(pCreature, 46798, true);
            pCreature->castSpell(pCreature, 46799, true);
            pCreature->castSpell(pCreature, 46800, true);
        }

        pPlayer->addQuestKill(11897, 1, 0);
    }

    return true;
}

class Worm : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Worm(c); }
    explicit Worm(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setMoveRoot(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        getCreature()->die(getCreature(), getCreature()->getHealth(), 0);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Fuelling the Project (Quest: 11715)
bool PlaceOil(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    Creature* pCreature = pSpell->getTargetConstraintCreature();
    auto* questLog = pPlayer->getQuestLogByQuestId(11715);
    if (questLog == nullptr)
        return true;

    if (pCreature->getEntry() == 25781)
    {
        if (questLog->getMobCountByIndex(0) < questLog->getQuestProperties()->required_mob_or_go_count[0])
        {
            pCreature->castSpell(pCreature, 45991,false);
            pCreature->Despawn(10000, pCreature->GetCreatureProperties()->RespawnTime);
            pPlayer->addQuestKill(11715, 0, 0);
        }
    }

    return true;
}

void SetupBoreanTundra(ScriptMgr* mgr)
{
    // Call to Arms!
    mgr->register_gameobject_script(188163, &BellRope::Create);
    // Reading the Meters
    mgr->register_gameobject_script(188100, &ColdarraGeoMonitorNexus::Create);
    mgr->register_gameobject_script(188101, &ColdarraGeoMonitorSouth::Create);
    mgr->register_gameobject_script(188102, &ColdarraGeoMonitorNorth::Create);
    mgr->register_gameobject_script(188103, &ColdarraGeoMonitorWest::Create);

    // Cutting Off the Source
    mgr->register_gameobject_script(187655, &NerubarEggSac::Create);
    // Bury Those Cockroaches!
    mgr->register_creature_script(25401, &SeaforiumDepthCharge::Create);
    // Hatching a Plan
    mgr->register_gameobject_script(188133, &BlueDragonEgg::Create);

    // Neutralizing the Cauldrons
    mgr->register_creature_script(CN_PURIFYING_TOTEM, &PurifyingTotemAI::Create);

    // Mechagnomes
    // Fizzcrank Fullthrottle
    mgr->register_creature_gossip(NPC_FIZZCRANK, new FizzcrankGossip());

    // Surristrasz
    mgr->register_creature_gossip(NPC_SURRISTRASZ, new SurristraszGossip());

    // Quest: Lefty Loosey, Righty Tighty ID: 11788
    mgr->register_gameobject_script(187984, &WestPointStationValve::Create);
    mgr->register_gameobject_script(187985, &NorthPointStationValve::Create);
    mgr->register_gameobject_script(187986, &FizzcrankPumpingStationValve::Create);
    mgr->register_gameobject_script(187987, &SouthPointStationValve::Create);

    // Quest: The Gearmaster 11798
    mgr->register_gameobject_script(190334, &TheGearmastersManual::Create);
    mgr->register_creature_script(25834, &GearmasterMechazodAI::Create);

    // Hunt Is On
    mgr->register_creature_gossip(25248, new SaltyJohnGossip());
    mgr->register_creature_gossip(25827, new TomHeggerGossip());
    mgr->register_creature_gossip(25828, new GuardMitchGossip());

    // Quest: Plug the Sinkholes
    mgr->register_dummy_spell(46797, &PlaceCart);
    mgr->register_creature_script(26250, &Worm::Create);

    // Quest: Fueling the Project
    mgr->register_dummy_spell(45990, &PlaceOil);
}
