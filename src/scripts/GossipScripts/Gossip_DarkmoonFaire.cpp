/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) Nexis-The Darkmoon Faire Project <http://www.sunplusplus.info>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../world/Gossip.h"
#include "../scripts/eventscripts/Event_Darkmoon_Faire.h"
#include "Setup.h"

// Setup Carnies
/// \todo Carnie use other text for gossip when Darkmoon Faire "is coming".
// move BARK_SETUP_CARNIES_1 - 4 from npc_script_text to npc_text (with prob) one entry.
class SetupCarnies_Gossip : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            int randGossip;
            randGossip = rand() % 4;
            switch(randGossip)
            {
                case 0:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), BARK_SETUP_CARNIES_1, plr);
                    break;

                case 1:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), BARK_SETUP_CARNIES_2, plr);
                    break;

                case 2:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), BARK_SETUP_CARNIES_3, plr);
                    break;

                case 3:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), BARK_SETUP_CARNIES_4, plr);
                    break;
            }

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char* Code)
        {
            if (!pObject->IsCreature())
                return;

            switch (IntId)
            {
                case 1:
                    GossipHello(pObject, plr);
                    break;
            }
        }
};


// Flik
class Flik_Bark : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(Flik_Bark);
        Flik_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            RegisterAIUpdateEvent(180000);             // Start initial update after: 3mins
        }

        void AIUpdate()
        {
            int randGossip;
            randGossip = rand() % 4;
            switch(randGossip)
            {
                case 0:
                    _unit->SendScriptTextChatMessage(BARK_FLIK_1);
                    break;

                case 1:
                    _unit->SendScriptTextChatMessage(BARK_FLIK_2);
                    break;

                case 2:
                    _unit->SendScriptTextChatMessage(BARK_FLIK_3);
                    break;

                case 3:
                    _unit->SendScriptTextChatMessage(BARK_FLIK_4);
                    break;
            }

            int rndTimer;
            rndTimer = rand() % 240 + 120;              // Generate a random value between: 2-4mins
            rndTimer = rndTimer * 1000;                 // Convert to milliseconds
            ModifyAIUpdateEvent(rndTimer);              // Modify timer to new random value
        }
};


// Flik's Frog
class FliksFrog_Gossip : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;

            int randGossip;
            randGossip = rand() % 2;
            switch(randGossip)
            {
                case 0:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60011, plr);
                    break;
                case 1:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60012, plr);
                    break;
            }

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char* Code)
        {
            if (!pObject->IsCreature())
                return;

            switch (IntId)
            {
                case 1:
                    GossipHello(pObject, plr);
                    break;
            }
        }

};


/// Gevas Grimegate
class GevasGrimegate_Bark : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(GevasGrimegate_Bark);
        GevasGrimegate_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            RegisterAIUpdateEvent(60000);             // Start initial update after: 1mins
        }

        void AIUpdate()
        {
            int randGossip;
            randGossip = rand() % 4;
            switch(randGossip)
            {
                case 0:
                    _unit->SendScriptTextChatMessage(BARK_GEVAS_GRIMEGATE_1);
                    break;

                case 1:
                    _unit->SendScriptTextChatMessage(BARK_GEVAS_GRIMEGATE_2);
                    break;

                case 2:
                    _unit->SendScriptTextChatMessage(BARK_GEVAS_GRIMEGATE_3);
                    break;

                case 3:
                    _unit->SendScriptTextChatMessage(BARK_GEVAS_GRIMEGATE_4);
                    break;
            }

            int rndTimer;
            rndTimer = rand() % 300 + 180;             // Generate a random value between: 3-5mins
            rndTimer = rndTimer * 1000;             // Convert to milliseconds
            ModifyAIUpdateEvent(rndTimer);             // Modify timer to new random value
        }
};


// Lhara
class Lhara_Bark : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(Lhara_Bark);
        Lhara_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            RegisterAIUpdateEvent(90000);             // Start initial update after: 1.5mins
        }

        void AIUpdate()
        {
            int randGossip;
            randGossip = rand() % 4;
            switch(randGossip)
            {
                case 0:
                    _unit->SendScriptTextChatMessage(BARK_LHARA_1);
                    break;

                case 1:
                    _unit->SendScriptTextChatMessage(BARK_LHARA_2);
                    break;

                case 2:
                    _unit->SendScriptTextChatMessage(BARK_LHARA_3);
                    break;

                case 3:
                    _unit->SendScriptTextChatMessage(BARK_LHARA_4);
                    break;
            }

            int rndTimer;
            rndTimer = rand() % 360 + 240;             // Generate a random value between: 4-6mins
            rndTimer = rndTimer * 1000;             // Convert to milliseconds
            ModifyAIUpdateEvent(rndTimer);             // Modify timer to new random value
        }
};


// Maxima Blastenheimer
class MaximaBlastenheimer_Gossip : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;

            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), BARK_MAXIMA_1, plr);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_ULTRA_CANNON), 1);
            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char* Code)
        {
            if (!pObject->IsCreature())
                return;

            GossipMenu* Menu;

            switch (IntId)
            {
                case 0:
                    GossipHello(pObject, plr);
                    break;

                case 1:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), BARK_MAXIMA_2, plr);
                    Menu->SendTo(plr);
                    break;
            }
        }
};


// Morja
class Morja_Bark : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(Morja_Bark);
        Morja_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            RegisterAIUpdateEvent(240000);              // Start initial update after: 4mins
        }

        void AIUpdate()
        {
            _unit->SendScriptTextChatMessage(BARK_MORJA_1);

            int rndTimer;
            rndTimer = rand() % 360 + 240;              // Generate a random value between: 4-6mins
            rndTimer = rndTimer * 1000;                 // Convert to milliseconds
            ModifyAIUpdateEvent(rndTimer);              // Modify timer to new random value
        }
};


// Professor Thaddeus Paleo
class ProfessorThaddeusPaleo_Gossip : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60016, plr);

            if(pObject->GetUInt32Value(UNIT_NPC_FLAGS) & UNIT_NPC_FLAG_VENDOR)
                Menu->AddItem(Arcemu::Gossip::ICON_VENDOR, plr->GetSession()->LocalizedGossipOption(GI_BROWS_GOODS), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_ME_DARKMOON_CARDS), 2);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            Creature* pCreature = (pObject->IsCreature()) ? (TO_CREATURE(pObject)) : NULL;
            if(!pObject->IsCreature())
                return;

            switch(IntId)
            {
                case 1:
                    plr->GetSession()->SendInventoryList(pCreature);
                    break;

                case 2:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60017, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_BEAST_DECK), 5);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_PORTAL_DECK), 6);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_ELEMENTALS_DECK), 7);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_WARLORDS_DECK), 8);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_FURIES_DECK), 9);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_LUNACY_DECK), 10);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_BLESSINGS_DECK), 11);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELL_STORMS_DECK), 12);

                    Menu->SendTo(plr);
                    break;

                case 5:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60018, plr);
                    Menu->SendTo(plr);
                    break;

                case 6:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60019, plr);
                    Menu->SendTo(plr);
                    break;

                case 7:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60020, plr);
                    Menu->SendTo(plr);
                    break;

                case 8:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60021, plr);
                    Menu->SendTo(plr);
                    break;

                case 9:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60022, plr);
                    Menu->SendTo(plr);
                    break;

                case 10:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60023, plr);
                    Menu->SendTo(plr);
                    break;

                case 11:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60024, plr);
                    Menu->SendTo(plr);
                    break;

                case 12:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60025, plr);
                    Menu->SendTo(plr);
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
            RegisterAIUpdateEvent(210000);             // Start initial update after: 3.5mins
        }

        void AIUpdate()
        {
            int randGossip;
            randGossip = rand() % 4;
            switch(randGossip)
            {
                case 0:
                    _unit->SendScriptTextChatMessage(BARK_PROFESSOR_THADDEUS_PALEO_1);
                    break;

                case 1:
                    _unit->SendScriptTextChatMessage(BARK_PROFESSOR_THADDEUS_PALEO_2);
                    break;

                case 2:
                    _unit->SendScriptTextChatMessage(BARK_PROFESSOR_THADDEUS_PALEO_3);
                    break;

                case 3:
                    _unit->SendScriptTextChatMessage(BARK_PROFESSOR_THADDEUS_PALEO_4);
                    break;
            }

            int rndTimer;
            rndTimer = rand() % 180 + 300;             // Generate a random value between: 3-5mins
            rndTimer = rndTimer * 1000;             // Convert to milliseconds
            ModifyAIUpdateEvent(rndTimer);             // Modify timer to new random value
        }
};


// Sayge
/// \todo find correct text in npc_text.
class Sayge_Gossip : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            // Check to see if the player already has a buff from Sayge.
            if (plr->HasAura(23768) || plr->HasAura(23769) || plr->HasAura(23767) || plr->HasAura(23738) || plr->HasAura(23766) || plr->HasAura(23737) || plr->HasAura(23735) || plr->HasAura(23736))
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60034, plr);             // Player has buff, denied.
            }
            else
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60026, plr);            // Player doesn't have any buff
                Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_READY_DISC_FORTUNE), 1);
            }

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            if (!pObject->IsCreature())
                return;
            Creature* pCreature = TO_CREATURE(pObject);

            switch (IntId)
            {
                case 1:        // Question 1 (Initial question, always the same)
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60027, plr);
                    // plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_1_1)
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_1_1), 10);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_1_2), 11);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_1_3), 12);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_1_4), 13);

                    Menu->SendTo(plr);
                    break;

                case 10:    // Question 2 (First Answer = 1)
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60028, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_2_1), 14);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_2_2), 15);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_2_3), 16);

                    Menu->SendTo(plr);
                    break;

                case 11:     // Question 2 (First Answer = 2)
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60029, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_3_1), 17);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_3_2), 18);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_3_3), 19);

                    Menu->SendTo(plr);
                    break;

                case 12:     // Question 2 (First Answer = 3)
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60030, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_4_1), 20);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_4_2), 21);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_4_3), 22);

                    Menu->SendTo(plr);
                    break;

                case 13:     // Question 2 (First Answer = 4)
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60031, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_5_1), 23);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_5_2), 24);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ANSWER_5_3), 25);

                    Menu->SendTo(plr);
                    break;

                    // Answers 1-#
                case 14:     // Answers: 1-1
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60032, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                    Menu->SendTo(plr);

                    pCreature->CastSpell(plr, 23768, true);
                    break;

                case 15:     // Answers: 1-2
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60032, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                    Menu->SendTo(plr);

                    pCreature->CastSpell(plr, 23769, true);
                    break;

                case 16:     // Answers: 1-3
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60032, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                    Menu->SendTo(plr);

                    pCreature->CastSpell(plr, 23767, true);
                    break;

                    // Answers 2-#
                case 17:     // Answers: 2-1
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60032, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                    Menu->SendTo(plr);

                    pCreature->CastSpell(plr, 23738, true);
                    break;

                case 18:     // Answers: 2-2
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60032, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                    Menu->SendTo(plr);

                    pCreature->CastSpell(plr, 23766, true);
                    break;

                case 19:     // Answers: 2-3
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60032, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                    Menu->SendTo(plr);

                    pCreature->CastSpell(plr, 23769, true);
                    break;

                    // Answers 3-#
                case 20:     // Answers: 3-1
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60032, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                    Menu->SendTo(plr);

                    pCreature->CastSpell(plr, 23737, true);
                    break;

                case 21:     // Answers: 3-2
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60032, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                    Menu->SendTo(plr);

                    pCreature->CastSpell(plr, 23735, true);
                    break;

                case 22:     // Answers: 3-3
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60032, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                    Menu->SendTo(plr);

                    pCreature->CastSpell(plr, 23736, true);
                    break;

                    // Answers 4-#
                case 23:     // Answers: 4-1
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60032, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                    Menu->SendTo(plr);

                    pCreature->CastSpell(plr, 23766, true);
                    break;

                case 24:     // Answers: 4-2
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60032, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                    Menu->SendTo(plr);

                    pCreature->CastSpell(plr, 23738, true);
                    break;

                case 25:     // Answers: 4-3
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60032, plr);

                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WRITTEN_FORTUNES), 30);
                    Menu->SendTo(plr);

                    pCreature->CastSpell(plr, 23737, true);
                    break;

                case 30:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60033, plr);
                    Menu->SendTo(plr);

                    // Cast the fortune into the player's inventory - Not working?
                    pCreature->CastSpell(plr, 23765, true);

                    // TEMP fix for spell not adding item to  player's inventory.
                    if (plr->GetItemInterface()->CalculateFreeSlots(ItemPrototypeStorage.LookupEntry(19422)))
                    {
                        plr->GetItemInterface()->AddItemToFreeSlot(objmgr.CreateItem(19422, plr)); //Darkmoon Faire Fortune
                    }
                    else
                    {
                        sChatHandler.SystemMessage(plr->GetSession(), plr->GetSession()->LocalizedGossipOption(GI_DF_NOT_ENOUGH_SLOTS));
                    }
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
            RegisterAIUpdateEvent(135000);             // Start initial update after: 2.25mins
        }

        void AIUpdate()
        {
            int randGossip;
            randGossip = rand() % 4;
            switch(randGossip)
            {
                case 0:
                    _unit->SendScriptTextChatMessage(BARK_SAYGE_1);
                    break;

                case 1:
                    _unit->SendScriptTextChatMessage(BARK_SAYGE_2);
                    break;

                case 2:
                    _unit->SendScriptTextChatMessage(BARK_SAYGE_3);
                    break;

                case 3:
                    _unit->SendScriptTextChatMessage(BARK_SAYGE_4);
                    break;
            }

            int rndTimer;
            rndTimer = rand() % 180 + 300;             // Generate a random value between: 3-5mins
            rndTimer = rndTimer * 1000;             // Convert to milliseconds
            ModifyAIUpdateEvent(rndTimer);             // Modify timer to new random value
        }
};


// Selina Dourman
/// \ todo find right gossip text in npc_text.
class SelinaDourman_Gossip : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;

            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60035, plr);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WHAT_PURCHASE), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_FAIRE_PRIZE), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_WHAT_ARE_DARKMOON), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_THINGS_FAIRE), 4);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            if (!pObject->IsCreature())
                return;
            Creature* pCreature = TO_CREATURE(pObject);

            switch (IntId)

            {
                case 1:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60036, plr);            // What can I purchase?
                    Menu->SendTo(plr);
                    break;

                case 2:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60037, plr);            // What are Darkmoon Faire Prize Tickets and how do I get them?
                    Menu->SendTo(plr);
                    break;

                case 3:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60038, plr);            // What are Darkmoon Cards?
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_MORE), 10);
                    Menu->SendTo(plr);
                    break;

                case 4:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60040, plr);            // What other things can I do at the faire?
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_TONK_CONTROLS), 20);
                    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ABOUT_CANON), 21);
                    Menu->SendTo(plr);
                    break;

                case 10:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60039, plr);            // What are Darkmoon Cards? <more>
                    Menu->SendTo(plr);
                    break;

                case 20:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60041, plr);            // What are these Tonk Control Consoles?
                    Menu->SendTo(plr);
                    break;

                case 21:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 60042, plr);            // Tell me about the cannon.
                    Menu->SendTo(plr);
                    break;
            }
        }

};


// Silas Darkmoon
class SilasDarkmoon_Gossip : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;

            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 60013, plr);                    /// \todo find right text
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DF_ASK_PROFIT), 1);    // Silas, why is most everything at the fair free? How do you make a profit?

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            if (!pObject->IsCreature())
                return;
            Creature* pCreature = TO_CREATURE(pObject);

            switch (IntId)
            {
                case 1:
                    objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 7336, plr);           // It's not always about money!
                    Menu->SendTo(plr);
                    break;
            }
        }

};

class SilasDarkmoon_Bark : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(SilasDarkmoon_Bark);
        SilasDarkmoon_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            RegisterAIUpdateEvent(180000);             // Start initial update after: 3mins
        }

        void AIUpdate()
        {
            int randGossip;
            randGossip = rand() % 6;
            switch (randGossip)
            {
                case 0:
                    _unit->SendScriptTextChatMessage(BARK_SILAS_DARKMOON_1);
                    break;

                case 1:
                    _unit->SendScriptTextChatMessage(BARK_SILAS_DARKMOON_2);
                    break;

                case 2:
                    _unit->SendScriptTextChatMessage(BARK_SILAS_DARKMOON_3);
                    break;

                case 3:
                    _unit->SendScriptTextChatMessage(BARK_SILAS_DARKMOON_4);
                    break;

                case 4:
                    _unit->SendScriptTextChatMessage(BARK_SILAS_DARKMOON_5);
                    break;

                case 5:
                    _unit->SendScriptTextChatMessage(BARK_SILAS_DARKMOON_6);
                    break;
            }

            int rndTimer;
            rndTimer = rand() % 300 + 240;             // Generate a random value between: 3-5mins
            rndTimer = rndTimer * 1000;             // Convert to milliseconds
            ModifyAIUpdateEvent(rndTimer);             // Modify timer to new random value
        }
};


// Stamp Thunderhorn
class StampThunderhorn_Bark : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(StampThunderhorn_Bark);
        StampThunderhorn_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            RegisterAIUpdateEvent(180000);             // Start initial update after: 3mins
        }

        void AIUpdate()
        {
            int randGossip;
            randGossip = rand() % 5;
            switch(randGossip)
            {
                case 0:
                    _unit->SendScriptTextChatMessage(BARK_STAMP_THUNDERHORN_1);
                    break;

                case 1:
                    _unit->SendScriptTextChatMessage(BARK_STAMP_THUNDERHORN_2);
                    break;

                case 2:
                    _unit->SendScriptTextChatMessage(BARK_STAMP_THUNDERHORN_3);
                    break;

                case 3:
                    _unit->SendScriptTextChatMessage(BARK_STAMP_THUNDERHORN_4);
                    break;

                case 4:
                    _unit->SendScriptTextChatMessage(BARK_STAMP_THUNDERHORN_5);
                    break;
            }

            int rndTimer;
            rndTimer = rand() % 300 + 180;             // Generate a random value between: 3-5mins
            rndTimer = rndTimer * 1000;             // Convert to milliseconds
            ModifyAIUpdateEvent(rndTimer);             // Modify timer to new random value
        }
};


// Sylannia
class Sylannia_Bark : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(Sylannia_Bark);
        Sylannia_Bark(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            RegisterAIUpdateEvent(120000);             // Start initial update after: 2mins
        }

        void AIUpdate()
        {
            int randGossip;
            randGossip = rand() % 4;
            switch(randGossip)
            {
                case 0:
                    _unit->SendScriptTextChatMessage(BARK_SYLANNIA_1);
                    break;

                case 1:
                    _unit->SendScriptTextChatMessage(BARK_SYLANNIA_2);
                    break;

                case 2:
                    _unit->SendScriptTextChatMessage(BARK_SYLANNIA_3);
                    break;

                case 3:
                    _unit->SendScriptTextChatMessage(BARK_SYLANNIA_4);
                    break;
            }

            int rndTimer;
            rndTimer = rand() % 360 + 180;             // Generate a random value between: 3-6mins
            rndTimer = rndTimer * 1000;             // Convert to milliseconds
            ModifyAIUpdateEvent(rndTimer);             // Modify timer to new random value
        }
};


void SetupDarkmoonFaireGossip(ScriptMgr* mgr)
{
    // Event DF
    GossipScript* FliksFrogGossip = new FliksFrog_Gossip;
    GossipScript* MaximaBlastenheimerGossip = new MaximaBlastenheimer_Gossip;
    GossipScript* ProfessorThaddeusPaleoGossip = new ProfessorThaddeusPaleo_Gossip;
    GossipScript* SaygeGossip = new Sayge_Gossip;
    GossipScript* SelinaDourmanGossip = new SelinaDourman_Gossip;
    GossipScript* SilasDarkmoonGossip = new SilasDarkmoon_Gossip;

    mgr->register_gossip_script(14866, FliksFrogGossip);
    mgr->register_gossip_script(15303, MaximaBlastenheimerGossip);
    mgr->register_gossip_script(14847, ProfessorThaddeusPaleoGossip);
    mgr->register_gossip_script(14822, SaygeGossip);
    mgr->register_gossip_script(10445, SelinaDourmanGossip);
    mgr->register_gossip_script(14823, SilasDarkmoonGossip);

    // Event DF prepare
    //mgr->register_gossip_script(14849, SetupCarniesGossip);
    //GossipScript* SetupCarniesGossip = new SetupCarnies_Gossip;
}

void SetupDarkmoonFaireBarker(ScriptMgr* mgr)
{
    // Event DF
    mgr->register_creature_script(14860, &Flik_Bark::Create);
    mgr->register_creature_script(14828, &GevasGrimegate_Bark::Create);
    mgr->register_creature_script(14846, &Lhara_Bark::Create);
    mgr->register_creature_script(14871, &Morja_Bark::Create);

    mgr->register_creature_script(14847, &ProfessorThaddeusPaleo_Bark::Create);
    mgr->register_creature_script(14822, &Sayge_Bark::Create);
    mgr->register_creature_script(14823, &SilasDarkmoon_Bark::Create);
    mgr->register_creature_script(14845, &StampThunderhorn_Bark::Create);
    mgr->register_creature_script(14844, &Sylannia_Bark::Create);

    // Event DF prepare
    //mgr->register_creature_script(14849, &SetupCarnies_Bark::Create);
}
