/*
 * ArcScripts for ArcEmu MMORPG Server
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

// Call to Arms!
class BellRope : public GameObjectAIScript
{
    public:

        ADD_GAMEOBJECT_FACTORY_FUNCTION(BellRope);
        BellRope(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

        void OnActivate(Player* pPlayer)
        {
            if (sEAS.GetQuest(pPlayer, 11965))
                sEAS.KillMobForQuest(pPlayer, 11965, 0);
        }
};

// Reading the Meters
class ColdarraGeoMonitorNexus : public GameObjectAIScript
{
    public:

        ADD_GAMEOBJECT_FACTORY_FUNCTION(ColdarraGeoMonitorNexus);
        ColdarraGeoMonitorNexus(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

        void OnActivate(Player* pPlayer)
        {
            if (sEAS.GetQuest(pPlayer, 11900))
                sEAS.KillMobForQuest(pPlayer, 11900, 0);
        }
};

class ColdarraGeoMonitorSouth : public GameObjectAIScript
{
    public:

        ADD_GAMEOBJECT_FACTORY_FUNCTION(ColdarraGeoMonitorSouth);
        ColdarraGeoMonitorSouth(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

        void OnActivate(Player* pPlayer)
        {
            if (sEAS.GetQuest(pPlayer, 11900))
                sEAS.KillMobForQuest(pPlayer, 11900, 1);
        }
};

class ColdarraGeoMonitorNorth : public GameObjectAIScript
{
    public:

        ADD_GAMEOBJECT_FACTORY_FUNCTION(ColdarraGeoMonitorNorth);
        ColdarraGeoMonitorNorth(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

        void OnActivate(Player* pPlayer)
        {
            if (sEAS.GetQuest(pPlayer, 11900))
                sEAS.KillMobForQuest(pPlayer, 11900, 2);
        }
};

class ColdarraGeoMonitorWest : public GameObjectAIScript
{
    public:

        ADD_GAMEOBJECT_FACTORY_FUNCTION(ColdarraGeoMonitorWest);
        ColdarraGeoMonitorWest(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

        void OnActivate(Player* pPlayer)
        {
            if (sEAS.GetQuest(pPlayer, 11900))
                sEAS.KillMobForQuest(pPlayer, 11900, 3);
        }
};

// Neutralizing the Cauldrons
#define CN_PURIFYING_TOTEM    25494

class PurifyingTotemAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(PurifyingTotemAI, MoonScriptCreatureAI);
    PurifyingTotemAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        SetCanEnterCombat(false);
        SetCanMove(false);
        Despawn(8000, 0);
    }
};


// Cutting Off the Source
class NerubarEggSac : public GameObjectAIScript
{
    public:

        ADD_GAMEOBJECT_FACTORY_FUNCTION(NerubarEggSac);
        NerubarEggSac(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

        void OnActivate(Player* pPlayer)
        {
            if (sEAS.GetQuest(pPlayer, 11602))
                return;

            sEAS.KillMobForQuest(pPlayer, 11602, 0);
            _gameobject->SetState(GAMEOBJECT_STATE_CLOSED);
            _gameobject->SetState(GAMEOBJECT_STATE_OPEN);
            _gameobject->Despawn(500, 60000);
        }
};


// Bury Those Cockroaches!
class SeaforiumDepthCharge : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SeaforiumDepthCharge, MoonScriptCreatureAI);
    SeaforiumDepthCharge(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        SetCanMove(false);
        SetCanEnterCombat(false);
        _unit->SetFaction(21);
    }

    void OnLoad()
    {
        if (!_unit->IsSummon())
            return;

        Unit* summoner = static_cast< Summon* >(_unit)->GetOwner();

        if (summoner != NULL)
        {
            if (summoner->IsPlayer())
            {
                Player* p = static_cast<Player*>(summoner);
                if (p->HasQuest(11608))
                {
                    GameObject* pSinkhole = p->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), 300171);
                    if (pSinkhole != NULL)
                    {
                        _unit->CastSpell(_unit, 45502, true);

                        float posX = pSinkhole->GetPositionX();
                        if (posX == 2657.13f)
                            sEAS.KillMobForQuest(p, 11608, 0);

                        if (posX == 2716.02f)
                            sEAS.KillMobForQuest(p, 11608, 1);

                        if (posX == 2877.96f)
                            sEAS.KillMobForQuest(p, 11608, 2);

                        if (posX == 2962.16f)
                            sEAS.KillMobForQuest(p, 11608, 3);

                    }
                }
            }
        }
        _unit->Despawn(500, 0);
    }
};


// Hatching a Plan
class BlueDragonEgg : public GameObjectAIScript
{
    public:

        ADD_GAMEOBJECT_FACTORY_FUNCTION(BlueDragonEgg);
        BlueDragonEgg(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

        void OnActivate(Player* pPlayer)
        {
            if (!sEAS.GetQuest(pPlayer, 11936))
                return;

            sEAS.KillMobForQuest(pPlayer, 11936, 0);
            ///\todo why setting gameobject state 1 and 0?!?!
            _gameobject->SetState(GAMEOBJECT_STATE_CLOSED);
            _gameobject->SetState(GAMEOBJECT_STATE_OPEN);
            _gameobject->Despawn(500, 60000);
        }
};


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

        void GossipHello(Object* pObject, Player* pPlayer)
        {
            GossipMenu* Menu;

            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 12435, pPlayer);

            if (sEAS.GetQuest(pPlayer, QUEST_THE_MECHAGNOMES))
                Menu->AddItem(ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(GOSSIP_OPTION_FIZZCRANK_1), 1);

            Menu->SendTo(pPlayer);
        }

        void GossipSelectOption(Object* pObject, Player* pPlayer, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch (IntId)
            {
                case 1:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), GOSSIP_TEXTID_FIZZCRANK1, pPlayer);
                    Menu->AddItem(ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(GOSSIP_OPTION_FIZZCRANK_2), 2);
                    Menu->SendTo(pPlayer);
                    break;
                case 2:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), GOSSIP_TEXTID_FIZZCRANK2, pPlayer);
                    Menu->AddItem(ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(GOSSIP_OPTION_FIZZCRANK_2), 3);
                    Menu->SendTo(pPlayer);
                    break;
                case 3:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), GOSSIP_TEXTID_FIZZCRANK3, pPlayer);
                    Menu->AddItem(ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(GOSSIP_OPTION_FIZZCRANK_2), 4);
                    Menu->SendTo(pPlayer);
                    break;
                case 4:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), GOSSIP_TEXTID_FIZZCRANK4, pPlayer);
                    Menu->AddItem(ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(GOSSIP_OPTION_FIZZCRANK_2), 5);
                    Menu->SendTo(pPlayer);
                    break;
                case 5:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), GOSSIP_TEXTID_FIZZCRANK5, pPlayer);
                    Menu->AddItem(ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(GOSSIP_OPTION_FIZZCRANK_2), 6);
                    Menu->SendTo(pPlayer);
                    break;
                case 6:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), GOSSIP_TEXTID_FIZZCRANK6, pPlayer);
                    Menu->AddItem(ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(GOSSIP_OPTION_FIZZCRANK_2), 7);
                    Menu->SendTo(pPlayer);
                    break;
                case 7:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), GOSSIP_TEXTID_FIZZCRANK7, pPlayer);
                    Menu->AddItem(ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(GOSSIP_OPTION_FIZZCRANK_2), 8);
                    Menu->SendTo(pPlayer);
                    break;
                case 8:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), GOSSIP_TEXTID_FIZZCRANK8, pPlayer);
                    Menu->AddItem(ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(GOSSIP_OPTION_FIZZCRANK_2), 9);
                    Menu->SendTo(pPlayer);
                    break;
                case 9:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), GOSSIP_TEXTID_FIZZCRANK9, pPlayer);
                    Menu->SendTo(pPlayer);
                    break;
            }
        }
};


//#define GOSSIP_ITEM_FREE_FLIGHT "I'd like passage to the Transitus Shield." this is not blizzlike...
enum eSurristrasz
{
    NPC_SURRISTRASZ = 24795,
    GI_SURRISTRASZ = 191,   // "May I use a drake to fly elsewhere?"   

    SPELL_ABMER_TO_COLDARRA = 46064
};

class SurristraszGossip : public GossipScript
{
    public:

        void GossipHello(Object* pObject, Player* pPlayer)
        {
            uint32 Text = objmgr.GetGossipTextForNpc(static_cast<Creature*>(pObject)->GetEntry());

            // check if there is a entry in the db
            if (NpcTextStorage.LookupEntry(Text) == NULL) { return; }

            Arcemu::Gossip::Menu menu(pObject->GetGUID(), Text, pPlayer->GetSession()->language);
            sQuestMgr.FillQuestMenu(static_cast<Creature*>(pObject), pPlayer, menu);

            menu.AddItem(ICON_FLIGHTMASTER, pPlayer->GetSession()->LocalizedGossipOption(GI_SURRISTRASZ), 1);

            menu.Send(pPlayer);
        }

        void GossipSelectOption(Object* pObject, Player* pPlayer, uint32 Id, uint32 IntId, const char* Code)
        {
            if (!pObject->IsCreature())
                return;

            switch (IntId)
            {
                case 1:
                    pPlayer->GetSession()->SendTaxiList(static_cast<Creature*>(pObject));
                    break;
            }
        }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Quest: Lefty Loosey, Righty Tighty ID: 11788
// West Point Station Valve
class WestPointStationValve : public GameObjectAIScript
{
    public:

        ADD_GAMEOBJECT_FACTORY_FUNCTION(WestPointStationValve);
        WestPointStationValve(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

        void OnActivate(Player* pPlayer)
        {
            if (!pPlayer->HasQuest(11788) || pPlayer->HasFinishedQuest(11788))
                return;

            auto quest_entry = pPlayer->GetQuestLogForEntry(11788);
            if (quest_entry == nullptr)
                return;

            if (quest_entry->GetMobCount(0) != 0)
                return;

            Creature* Twonky = sEAS.SpawnCreature(pPlayer, 25830, 4117.513672f, 5089.670898f, -1.506265f, 2.043593f, 0);
            if (Twonky->isAlive())
                _gameobject->SetState(GAMEOBJECT_STATE_OPEN);
            else
                _gameobject->SetState(GAMEOBJECT_STATE_CLOSED);
        }
};

// North Point Station Valve
class NorthPointStationValve : public GameObjectAIScript
{
    public:

        ADD_GAMEOBJECT_FACTORY_FUNCTION(NorthPointStationValve);
        NorthPointStationValve(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

        void OnActivate(Player* pPlayer)
        {
            if (!pPlayer->HasQuest(11788) || pPlayer->HasFinishedQuest(11788))
                return;

            auto quest_entry = pPlayer->GetQuestLogForEntry(11788);
            if (quest_entry == nullptr)
                return;

            if (quest_entry->GetMobCount(1) != 0)
                return;

            Creature* Ed210 = sEAS.SpawnCreature(pPlayer, 25831, 4218.529785f, 4802.284668f, -12.975346f, 5.833142f, 0);
            if (Ed210->isAlive())
                _gameobject->SetState(GAMEOBJECT_STATE_OPEN);
            else
                _gameobject->SetState(GAMEOBJECT_STATE_CLOSED);
        }
};

// Fizzcrank Pumping Station Valve
class FizzcrankPumpingStationValve : public GameObjectAIScript
{
    public:

        ADD_GAMEOBJECT_FACTORY_FUNCTION(FizzcrankPumpingStationValve);
        FizzcrankPumpingStationValve(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

        void OnActivate(Player* pPlayer)
        {
            if (!pPlayer->HasQuest(11788) || pPlayer->HasFinishedQuest(11788))
                return;

            auto quest_entry = pPlayer->GetQuestLogForEntry(11788);
            if (quest_entry == nullptr)
                return;

            if (quest_entry->GetMobCount(2) != 0)
                return;

            Creature* MaxBlasto = sEAS.SpawnCreature(pPlayer, 25832, 4029.974609f, 4890.195313f, -12.775084f, 1.081481f, 0);
            if (MaxBlasto->isAlive())
                _gameobject->SetState(GAMEOBJECT_STATE_OPEN);
            else
                _gameobject->SetState(GAMEOBJECT_STATE_CLOSED);
        }
};

// South Point Station Valve
class SouthPointStationValve : public GameObjectAIScript
{
    public:

        ADD_GAMEOBJECT_FACTORY_FUNCTION(SouthPointStationValve);
        SouthPointStationValve(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

        void OnActivate(Player* pPlayer)
        {
            if (!pPlayer->HasQuest(11788) || pPlayer->HasFinishedQuest(11788))
                return;

            auto quest_entry = pPlayer->GetQuestLogForEntry(11788);
            if (quest_entry == nullptr)
                return;

            if (quest_entry->GetMobCount(3) != 0)
                return;

            Creature* TheGrinder = sEAS.SpawnCreature(pPlayer, 25833, 3787.021484f, 4821.941895f, -12.967110f, 5.097224f, 0);
            if (TheGrinder->isAlive())
                _gameobject->SetState(GAMEOBJECT_STATE_OPEN);
            else
                _gameobject->SetState(GAMEOBJECT_STATE_CLOSED);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Quest: The Gearmaster 11798
// The Gearmaster's Manual
class TheGearmastersManual : public GameObjectAIScript
{
    public:

        ADD_GAMEOBJECT_FACTORY_FUNCTION(TheGearmastersManual);
        TheGearmastersManual(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

        void OnActivate(Player* pPlayer)
        {
            if (!pPlayer->HasQuest(11798) || pPlayer->HasFinishedQuest(11798))
                return;

            auto quest_entry = pPlayer->GetQuestLogForEntry(11798);
            if (quest_entry == nullptr)
                return;

            if (quest_entry->GetMobCount(0) == 0)
            {
                quest_entry->SetMobCount(0, 1);
                quest_entry->SendUpdateAddKill(0);
                quest_entry->UpdatePlayerFields();
            }

            Creature* GearmasterMechazod = sEAS.SpawnCreature(pPlayer, 25834, 4006.289551f, 4848.437500f, 25.957747f, 2.459837f, 0);
            GearmasterMechazod->SetTargetGUID(pPlayer->GetGUID());
            if (GearmasterMechazod->isAlive())
                _gameobject->SetState(GAMEOBJECT_STATE_OPEN);
            else
                _gameobject->SetState(GAMEOBJECT_STATE_CLOSED);
        }
};

///\todo: Change to spellevent (target player), npc say is not ready yet. Add Visual Aura on Spawn.
class GearmasterMechazodAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(GearmasterMechazodAI);
        GearmasterMechazodAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            _unit->SetEquippedItem(0, 28487);       // Mainhand
            _unit->SetEquippedItem(1, 11587);       // Offhand
            _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
            RegisterAIUpdateEvent(100);
            phase = 0;
        }

        void AIUpdate()
        {
            switch (phase)
            {
                case 0:
                {
                    RemoveAIUpdateEvent();
                    _unit->SendScriptTextChatMessage(8736);
                    RegisterAIUpdateEvent(7000);
                    phase = 1;
                }break;
                case 1:
                {
                    _unit->SendScriptTextChatMessage(8737);

                    phase = 2;
                }break;
                case 2:
                {
                    _unit->SendScriptTextChatMessage(8738);

                    phase = 3;
                }break;
                case 3:
                {
                    _unit->SendScriptTextChatMessage(8739);

                    phase = 4;
                }break;
                case 4:
                {
                    _unit->GetAIInterface()->SetAllowedToEnterCombat(true);
                    _unit->GetAIInterface()->m_canMove = true;
                    RemoveAIUpdateEvent();          // Remove Update, now we are in OnCombatStart
                }break;
                default:
                    break;
            }
        }

    protected:

        uint32 phase;
};

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
    mgr->register_gossip_script(NPC_FIZZCRANK, new FizzcrankGossip);

    // Surristrasz
    mgr->register_gossip_script(NPC_SURRISTRASZ, new SurristraszGossip);

    // Quest: Lefty Loosey, Righty Tighty ID: 11788
    mgr->register_gameobject_script(187984, &WestPointStationValve::Create);
    mgr->register_gameobject_script(187985, &NorthPointStationValve::Create);
    mgr->register_gameobject_script(187986, &FizzcrankPumpingStationValve::Create);
    mgr->register_gameobject_script(187987, &SouthPointStationValve::Create);

    // Quest: The Gearmaster 11798
    mgr->register_gameobject_script(190334, &TheGearmastersManual::Create);
    mgr->register_creature_script(25834, &GearmasterMechazodAI::Create);
}
