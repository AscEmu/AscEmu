/*
 Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 This file is released under the MIT license. See README-MIT for more information.
 */

#include "Setup.h"
#include "Event_Darkmoon_Faire.h"

 //////////////////////////////////////////////////////////////////////////////////////////
 //\details <b>Darkmoon Faire (Elwynn Forest)</b>\n
 // event_properties entry: 4 \n
 // event_properties holiday: 374 \n
 //\todo Check all Darkmoon Faire events


 //////////////////////////////////////////////////////////////////////////////////////////
 //\details <b>Darkmoon Faire (Mulgore)</b>\n
 // event_properties entry: 5 \n
 // event_properties holiday: 375 \n


 //////////////////////////////////////////////////////////////////////////////////////////
 //\details <b>Darkmoon Faire (Terokkar Forest)</b>\n
 // event_properties entry: 3 \n
 // event_properties holiday: 376 \n


 //////////////////////////////////////////////////////////////////////////////////////////
 //\details <b>Darkmoon Faire Gameobjects</b>\n

 // Blastenheimer 5000 Ultra Cannon
class Blastenheimer5000 : public GameObjectAIScript
{
public:
    explicit Blastenheimer5000(GameObject* pGameObject) : GameObjectAIScript(pGameObject)
    {
        mPlayerGuid = 0;
    }

    static GameObjectAIScript* Create(GameObject* pGameObject)
    {
        return new Blastenheimer5000(pGameObject);
    }

    void OnActivate(Player* pPlayer) override
    {
        if (mPlayerGuid != 0)
            return;

        pPlayer->CastSpell(pPlayer, 24832, true);
        pPlayer->setMoveRoot(true);
        _gameobject->PlaySoundToSet(8476);
        mPlayerGuid = static_cast<uint32>(pPlayer->GetGUID());
        RegisterAIUpdateEvent(2200);
    }

    void AIUpdate() override
    {
        auto CurrentPlayer = objmgr.GetPlayer(mPlayerGuid);
        if (CurrentPlayer == nullptr)
        {
            RemoveAIUpdateEvent();
            mPlayerGuid = 0;
            return;
        }

        if (CurrentPlayer->GetMapId() == 530)           /// Shattrath
        {
            CurrentPlayer->SafeTeleport(530, 0, -1742.640869f, 5454.712402f, -7.928009f, 4.606363f);
        }
        else if (CurrentPlayer->GetMapId() == 0)        /// Elwynn Forest
        {
            CurrentPlayer->SafeTeleport(0, 0, -9569.150391f, -14.753426f, 68.051422f, 4.874008f);
        }
        else if (CurrentPlayer->GetMapId() == 1)        /// Mulgore
        {
            CurrentPlayer->SafeTeleport(1, 0, -1326.711914f, 86.301125f, 133.093918f, 3.510725f);
        }

        CurrentPlayer->setMoveRoot(false);
        CurrentPlayer->CastSpell(CurrentPlayer, 42867, true);   // 24742
        _gameobject->SetFlags(0);
        mPlayerGuid = 0;
        RemoveAIUpdateEvent();
    }

protected:

    uint32 mPlayerGuid;
};

/*
Spells:
=====================
Cannon - 24933
Mortor - 25003
Drop Mine - 39685, 25024
Nitrous Boost - 27746


const uint32 CANNON = 24933            //39692, 34154
const uint32 MORTAR = 25003            //33861 -- Triggers Explosion, 39695 --- Summons Mortar
const uint32 NITROUS = 27746           //Needs Scripting
const uint32 FLAMETHROWER = 39693      //25027
const uint32 MACHINEGUN = 25026
const uint32 DROPMINE = 25024
const uint32 SHIELD = 27759

static uint32 TonkSpecials[4] = { FLAMETHROWER, MACHINEGUN, DROPMINE, SHIELD };

/// Tonk Control Consoles
class TonkControlConsole : public GameObjectAIScript
{
public:
TonkControlConsole(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
static GameObjectAIScript *Create(GameObject* GO) { return new TonkControlConsole(GO); }

// Click the Console
void OnActivate(Player* pPlayer)
{
// Pre-flight checks
GameObject* tonkConsole = NULL;
tonkConsole = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 180524);

// Open and disable the Tonk Console
tonkConsole->SetFlags(GO_FLAG_NONSELECTABLE);
tonkConsole->SetState(GO_STATE_OPEN);

// Spawn Steam Tonk
pPlayer->GetMapMgr()->GetInterface()->SpawnCreature(19405, pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), pPlayer->GetOrientation(), true, false, 0, 0)->Despawn(310000, 0);;

// Store the tonk just spawned
Creature* pTonk = NULL;
pTonk = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 19405);

// Cast the tonk control spell on the tonk
pPlayer->CastSpell(pTonk, 33849, false);

// Start checks to see if player still has aura
RegisterAIUpdateEvent(1000);

Plr = pPlayer;
Tonk = pTonk;
Console = tonkConsole;
}

void AIUpdate()
{
if (!Plr->HasAura(33849) || Tonk->isDead())
{
// Kill then Despawn Tonk after 10 seconds
Plr->CastSpell(Tonk, 5, false); // Kill spell
Plr->CastSpell(Plr, 2880, false); // Stun Player
Plr->RemoveAura(33849);
Tonk->Despawn(10000,0);

// Close the console so others can access it
Console->SetFlags(0);
Console->SetState(GO_STATE_CLOSED);
RemoveAIUpdateEvent();
}
}

protected:
Player* Plr;
Creature* Tonk;
GameObject* Console;
};
*/

// Setup Carnies
/// \todo Carnie use other text for gossip when Darkmoon Faire "is coming".
// move BARK_SETUP_CARNIES_1 - 4 from npc_script_text to npc_text (with prob) one entry.
class SetupCarnies_Gossip : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* plr) override
    {
        uint32 textId = 0;
        uint32 randomNumber = RandomUInt(3);

        switch (randomNumber)
        {
            case 0:
                textId = BARK_SETUP_CARNIES_1;
                break;
            case 1:
                textId = BARK_SETUP_CARNIES_2;
                break;
            case 2:
                textId = BARK_SETUP_CARNIES_3;
                break;
            case 3:
                textId = BARK_SETUP_CARNIES_4;
                break;
        }

        Arcemu::Gossip::Menu menu(pObject->GetGUID(), textId, plr->GetSession()->language);
        menu.Send(plr);
    }
};


class Flik_Bark : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(Flik_Bark);
    Flik_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(1000);

        addEmoteForEvent(Event_OnIdle, BARK_FLIK_1);
        addEmoteForEvent(Event_OnIdle, BARK_FLIK_2);
        addEmoteForEvent(Event_OnIdle, BARK_FLIK_3);
        addEmoteForEvent(Event_OnIdle, BARK_FLIK_4);

        enableOnIdleEmote(true, 180000);
        setRandomIdleEmoteTime(120000, 240000);
    }
};


// Flik's Frog
class FliksFrog_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        uint32 textId = 0;
        uint32 randomNumber = RandomUInt(1);

        switch (randomNumber)
        {
            case 0:
                textId = 60011;
                break;
            case 1:
                textId = 60012;
                break;
        }

        Arcemu::Gossip::Menu menu(pObject->GetGUID(), textId, plr->GetSession()->language);
        menu.Send(plr);
    }
};


class GelvasGrimegate_Bark : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(GelvasGrimegate_Bark);
    GelvasGrimegate_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(1000);

        addEmoteForEvent(Event_OnIdle, BARK_GEVAS_GRIMEGATE_1);
        addEmoteForEvent(Event_OnIdle, BARK_GEVAS_GRIMEGATE_2);
        addEmoteForEvent(Event_OnIdle, BARK_GEVAS_GRIMEGATE_3);
        addEmoteForEvent(Event_OnIdle, BARK_GEVAS_GRIMEGATE_4);

        enableOnIdleEmote(true, 60000);
        setRandomIdleEmoteTime(180000, 300000);
    }
};


class Lhara_Bark : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(Lhara_Bark);
    Lhara_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(1000);

        addEmoteForEvent(Event_OnIdle, BARK_LHARA_1);
        addEmoteForEvent(Event_OnIdle, BARK_LHARA_2);
        addEmoteForEvent(Event_OnIdle, BARK_LHARA_3);
        addEmoteForEvent(Event_OnIdle, BARK_LHARA_4);

        enableOnIdleEmote(true, 90000);
        setRandomIdleEmoteTime(240000, 360000);
    }
};


class MaximaBlastenheimer_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), BARK_MAXIMA_1, plr->GetSession()->language);
        menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_ULTRA_CANNON), 1);
        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), BARK_MAXIMA_2, plr->GetSession()->language);
        menu.Send(plr);
    }
};


class Morja_Bark : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(Morja_Bark);
    Morja_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(1000);

        addEmoteForEvent(Event_OnIdle, BARK_MORJA_1);

        enableOnIdleEmote(true, 240000);
        setRandomIdleEmoteTime(240000, 360000);
    }
};


class ProfessorThaddeusPaleo_Gossip : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60016, plr->GetSession()->language);

        if (pObject->getUInt32Value(UNIT_NPC_FLAGS) & UNIT_NPC_FLAG_VENDOR)
            menu.AddItem(GOSSIP_ICON_VENDOR, plr->GetSession()->LocalizedGossipOption(GI_BROWS_GOODS), 1);

        menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_ME_DARKMOON_CARDS), 2);
        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Creature* pCreature = static_cast<Creature*>(pObject);

        switch (Id)
        {
            case 1:
                plr->GetSession()->SendInventoryList(pCreature);
                break;
            case 2:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60017, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_BEAST_DECK), 5);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_PORTAL_DECK), 6);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_ELEMENTALS_DECK), 7);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_WARLORDS_DECK), 8);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_FURIES_DECK), 9);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_LUNACY_DECK), 10);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_BLESSINGS_DECK), 11);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_STORMS_DECK), 12);
                menu.Send(plr);
            } break;
            case 5:
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 60018, plr);
                break;
            case 6:
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 60019, plr);
                break;
            case 7:
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 60020, plr);
                break;
            case 8:
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 60021, plr);
                break;
            case 9:
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 60022, plr);
                break;
            case 10:
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 60023, plr);
                break;
            case 11:
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 60024, plr);
                break;
            case 12:
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 60025, plr);
                break;
            default:
                break;
        }
    }
};


class ProfessorThaddeusPaleo_Bark : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(ProfessorThaddeusPaleo_Bark);
    ProfessorThaddeusPaleo_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(1000);

        addEmoteForEvent(Event_OnIdle, BARK_PROFESSOR_THADDEUS_PALEO_1);
        addEmoteForEvent(Event_OnIdle, BARK_PROFESSOR_THADDEUS_PALEO_2);
        addEmoteForEvent(Event_OnIdle, BARK_PROFESSOR_THADDEUS_PALEO_3);
        addEmoteForEvent(Event_OnIdle, BARK_PROFESSOR_THADDEUS_PALEO_4);

        enableOnIdleEmote(true, 210000);
        setRandomIdleEmoteTime(180000, 360000);
    }
};


// Sayge
/// \todo find correct text in npc_text.
class Sayge_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        // Check to see if the player already has a buff from Sayge.
        if (plr->HasAura(23768) || plr->HasAura(23769) || plr->HasAura(23767) || plr->HasAura(23738) || plr->HasAura(23766) || plr->HasAura(23737) || plr->HasAura(23735) || plr->HasAura(23736))
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60034, plr->GetSession()->language);
            menu.Send(plr);
        }
        else
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60026, plr->GetSession()->language);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_READY_DISC_FORTUNE), 1);
            menu.Send(plr);
        }
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Creature* pCreature = static_cast<Creature*>(pObject);

        switch (Id)
        {
            case 1:        // Question 1 (Initial question, always the same)
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60027, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_1_1), 10);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_1_2), 11);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_1_3), 12);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_1_4), 13);
                menu.Send(plr);
            }break;
            case 10:    // Question 2 (First Answer = 1)
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60028, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_2_1), 14);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_2_2), 15);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_2_3), 16);
                menu.Send(plr);
            }break;
            case 11:     // Question 2 (First Answer = 2)
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60029, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_3_1), 17);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_3_2), 18);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_3_3), 19);
                menu.Send(plr);
            }break;
            case 12:     // Question 2 (First Answer = 3)
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60030, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_4_1), 20);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_4_2), 21);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_4_3), 22);
                menu.Send(plr);
            }break;
            case 13:     // Question 2 (First Answer = 4)
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60031, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_5_1), 23);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_5_2), 24);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_5_3), 25);
                menu.Send(plr);
            }break;
            // Answers 1-#
            case 14:     // Answers: 1-1
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60032, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                pCreature->CastSpell(plr, 23768, true);
                menu.Send(plr);
            }break;
            case 15:     // Answers: 1-2
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60032, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                pCreature->CastSpell(plr, 23769, true);
                menu.Send(plr);
            }break;
            case 16:     // Answers: 1-3
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60032, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                pCreature->CastSpell(plr, 23767, true);
                menu.Send(plr);
            }break;
            // Answers 2-#
            case 17:     // Answers: 2-1
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60032, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                pCreature->CastSpell(plr, 23738, true);
                menu.Send(plr);
            }break;
            case 18:     // Answers: 2-2
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60032, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                pCreature->CastSpell(plr, 23766, true);
                menu.Send(plr);
            }break;
            case 19:     // Answers: 2-3
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60032, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                pCreature->CastSpell(plr, 23769, true);
                menu.Send(plr);
            }break;
            // Answers 3-#
            case 20:     // Answers: 3-1
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60032, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                pCreature->CastSpell(plr, 23737, true);
                menu.Send(plr);
            }break;
            case 21:     // Answers: 3-2
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60032, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                pCreature->CastSpell(plr, 23735, true);
                menu.Send(plr);
            }break;
            case 22:     // Answers: 3-3
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60032, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                pCreature->CastSpell(plr, 23736, true);
                menu.Send(plr);
            }break;
            // Answers 4-#
            case 23:     // Answers: 4-1
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60032, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                pCreature->CastSpell(plr, 23766, true);
                menu.Send(plr);
            }break;
            case 24:     // Answers: 4-2
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60032, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                pCreature->CastSpell(plr, 23738, true);
                menu.Send(plr);
            }break;
            case 25:     // Answers: 4-3
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60032, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                pCreature->CastSpell(plr, 23737, true);
                menu.Send(plr);
            }break;
            case 30:
            {

                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60033, plr->GetSession()->language);
                menu.Send(plr);
                // Cast the fortune into the player's inventory - Not working?
                pCreature->CastSpell(plr, 23765, true);
                // TEMP fix for spell not adding item to  player's inventory.
                auto proto = sMySQLStore.getItemProperties(19422);
                if (proto == nullptr)
                    return;

                auto slotresult = plr->GetItemInterface()->FindFreeInventorySlot(proto);
                if (!slotresult.Result)
                {
                    plr->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
                    return;
                }
                else
                {
                    auto item = objmgr.CreateItem(19422, plr);
                    if (item == nullptr)
                        return;

                    auto result = plr->GetItemInterface()->SafeAddItem(item, slotresult.ContainerSlot, slotresult.Slot);
                    if (!result)
                    {
                        LOG_ERROR("Error while adding item %u to player %s", item->GetEntry(), plr->GetNameString());
                        item->DeleteMe();
                        return;
                    }
                }
                break;
            }
            default:
                break;
        }
    }
};

class Sayge_Bark : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(Sayge_Bark);
    Sayge_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(1000);

        addEmoteForEvent(Event_OnIdle, BARK_SAYGE_1);
        addEmoteForEvent(Event_OnIdle, BARK_SAYGE_2);
        addEmoteForEvent(Event_OnIdle, BARK_SAYGE_3);
        addEmoteForEvent(Event_OnIdle, BARK_SAYGE_4);

        enableOnIdleEmote(true, 135000);
        setRandomIdleEmoteTime(180000, 360000);
    }
};


// Selina Dourman
/// \ todo find right gossip text in npc_text.
class SelinaDourman_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60035, plr->GetSession()->language);
        menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WHAT_PURCHASE), 1);
        menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_FAIRE_PRIZE), 2);
        menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WHAT_ARE_DARKMOON), 3);
        menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_THINGS_FAIRE), 4);
        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 IntId, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        //Creature* pCreature = static_cast<Creature*>(pObject);

        switch (IntId)
        {
            case 1:
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 60036, plr);           // What can I purchase?
                break;
            case 2:
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 60037, plr);           // What are Darkmoon Faire Prize Tickets and how do I get them?
                break;
            case 3:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60038, plr->GetSession()->language);          // What are Darkmoon Cards?
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_MORE), 10);
                menu.Send(plr);
            }break;
            case 4:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60040, plr->GetSession()->language);          // What other things can I do at the faire?
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_TONK_CONTROLS), 20);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ABOUT_CANON), 21);
                menu.Send(plr);
            }break;
            case 10:
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 60039, plr);            // What are Darkmoon Cards? <more>
                break;
            case 20:
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 60041, plr);           // What are these Tonk Control Consoles?
                break;
            case 21:
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 60042, plr);           // Tell me about the cannon.
                break;
            default:
                break;
        }
    }
};


class SilasDarkmoon_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 60013, plr->GetSession()->language);                // \todo find right text
        menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ASK_PROFIT), 1);    // Silas, why is most everything at the fair free? How do you make a profit?
        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 7336, plr);
    }
};


class SilasDarkmoon_Bark : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(SilasDarkmoon_Bark);
    SilasDarkmoon_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(1000);

        addEmoteForEvent(Event_OnIdle, BARK_SILAS_DARKMOON_1);
        addEmoteForEvent(Event_OnIdle, BARK_SILAS_DARKMOON_2);
        addEmoteForEvent(Event_OnIdle, BARK_SILAS_DARKMOON_3);
        addEmoteForEvent(Event_OnIdle, BARK_SILAS_DARKMOON_4);
        addEmoteForEvent(Event_OnIdle, BARK_SILAS_DARKMOON_5);
        addEmoteForEvent(Event_OnIdle, BARK_SILAS_DARKMOON_6);

        enableOnIdleEmote(true, 180000);
        setRandomIdleEmoteTime(240000, 360000);
    }
};


class StampThunderhorn_Bark : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(StampThunderhorn_Bark);
    StampThunderhorn_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(1000);

        addEmoteForEvent(Event_OnIdle, BARK_STAMP_THUNDERHORN_1);
        addEmoteForEvent(Event_OnIdle, BARK_STAMP_THUNDERHORN_2);
        addEmoteForEvent(Event_OnIdle, BARK_STAMP_THUNDERHORN_3);
        addEmoteForEvent(Event_OnIdle, BARK_STAMP_THUNDERHORN_4);
        addEmoteForEvent(Event_OnIdle, BARK_STAMP_THUNDERHORN_5);

        enableOnIdleEmote(true, 180000);
        setRandomIdleEmoteTime(180000, 360000);
    }
};


class Sylannia_Bark : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(Sylannia_Bark);
    Sylannia_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(1000);

        addEmoteForEvent(Event_OnIdle, BARK_SYLANNIA_1);
        addEmoteForEvent(Event_OnIdle, BARK_SYLANNIA_2);
        addEmoteForEvent(Event_OnIdle, BARK_SYLANNIA_3);
        addEmoteForEvent(Event_OnIdle, BARK_SYLANNIA_4);

        enableOnIdleEmote(true, 120000);
        setRandomIdleEmoteTime(180000, 360000);
    }
};


void SetupDarkmoonFaire(ScriptMgr* mgr)
{
    //GossipScripts
    mgr->register_creature_gossip(14866, new FliksFrog_Gossip());
    mgr->register_creature_gossip(15303, new MaximaBlastenheimer_Gossip());
    mgr->register_creature_gossip(14847, new ProfessorThaddeusPaleo_Gossip());
    mgr->register_creature_gossip(14822, new Sayge_Gossip());
    mgr->register_creature_gossip(10445, new SelinaDourman_Gossip());
    mgr->register_creature_gossip(14823, new SilasDarkmoon_Gossip());

    // GameobjectScripts
    mgr->register_gameobject_script(180515, &Blastenheimer5000::Create);            // Blastenheimer 5000 Ultra Cannon
    // mgr->register_gameobject_script(180524, &TonkControlConsole::Create);        // Tonk Control Console

    //CreatureScripts
    mgr->register_creature_script(14860, &Flik_Bark::Create);
    mgr->register_creature_script(14828, &GelvasGrimegate_Bark::Create);
    mgr->register_creature_script(14846, &Lhara_Bark::Create);
    mgr->register_creature_script(14871, &Morja_Bark::Create);

    mgr->register_creature_script(14847, &ProfessorThaddeusPaleo_Bark::Create);
    mgr->register_creature_script(14822, &Sayge_Bark::Create);
    mgr->register_creature_script(14823, &SilasDarkmoon_Bark::Create);
    mgr->register_creature_script(14845, &StampThunderhorn_Bark::Create);
    mgr->register_creature_script(14844, &Sylannia_Bark::Create);
}
