/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
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

#include "../world/Gossip.h"
#include "Setup.h"

/************************************************************************/
/* GENERAL GUARD SCRIPT                                                 */
/************************************************************************/

// Covers *all* guard types, scripting their texts to guide players around.
// Enable this define to make all gossip texts have a "back" / "I was looking
// for somethinge else" button added.

#define BACK_BUTTON

#ifdef BACK_BUTTON

// Make code neater with this define.
#define SendQuickMenu(textid) objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), textid, Plr); \
    Menu->SendTo(Plr);

#else

// Make code neater with this define.
#define SendQuickMenu(textid) objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), textid, Plr); \
    Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_LOOKING_SOMTH_ELSE), 0); \
    Menu->SendTo(Plr);

#endif

/************************************************************************/
/* Stormwind CITY Guards                                                */
/************************************************************************/

class StormwindGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 933, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_BANK_OF_STORMWIND), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_STORMWIND_HARBOUR), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DEEPRUN_TRAM), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GRYPHON_M), 6);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD2_M), 7);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MAILBOX), 8);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE2_M), 9);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_WEAPON3_M), 10);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_OFFICERS_LOUNGE), 11);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BATTLE3_M), 12);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BARBER), 13);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS2_T), 14);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION2_T), 15);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     // Auction House
                    {
                        SendQuickMenu(3834);
                        Plr->Gossip_SendPOI(1);
                    }
                    break;

                case 2:     // Bank of Stormwind
                    {
                        SendQuickMenu(764);
                        Plr->Gossip_SendPOI(2);
                    }
                    break;

                case 3:     // Stormwind Harbor
                    {
                        SendQuickMenu(13439);
                        Plr->Gossip_SendPOI(3);
                    }
                    break;

                case 4:     // Deeprun Tram
                    {
                        SendQuickMenu(3813);
                        Plr->Gossip_SendPOI(4);
                    }
                    break;

                case 5:     // The Inn
                    {
                        SendQuickMenu(3860);
                        Plr->Gossip_SendPOI(5);
                    }
                    break;

                case 6:     // Gryphon Master
                    {
                        SendQuickMenu(879);
                        Plr->Gossip_SendPOI(6);
                    }
                    break;

                case 7:     // Guild Master
                    {
                        SendQuickMenu(882);
                        Plr->Gossip_SendPOI(7);
                    }
                    break;

                case 8:     // Mailbox
                    {
                        SendQuickMenu(3861);
                        Plr->Gossip_SendPOI(8);
                    }
                    break;

                case 9:     // Stable Master
                    {
                        SendQuickMenu(5984);
                        Plr->Gossip_SendPOI(9);
                    }
                    break;

                case 10:     // Weapons Master
                    {
                        SendQuickMenu(4516);
                        Plr->Gossip_SendPOI(10);
                    }
                    break;

                case 11:    // Officers' Lounge
                    {
                        SendQuickMenu(7047);
                        Plr->Gossip_SendPOI(11);
                    }
                    break;

                case 12:    // Battlemaster
                    {
                        SendQuickMenu(10218);
                        Plr->Gossip_SendPOI(12);
                    }
                    break;

                case 13:     // Barber
                    {
                        SendQuickMenu(13882);
                        Plr->Gossip_SendPOI(13);
                    }
                    break;

                case 14:    // Class Trainers
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 898, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_DRUID), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HUNTER), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MAGE), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PALADIN), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ROGUE), 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SHAMAN), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARLOCK), 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR), 24);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 15:    // Profession Trainers
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 918, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 25);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 26);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 27);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 28);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING), 29);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 30);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 31);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 32);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_INSCRIPTION), 33);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 34);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 35);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 36);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 37);
                        Menu->SendTo(Plr);
                    }
                    break;

                    ////////////////
                    // Class trainer submenu
                    ////////

                case 16: //Druid
                    {
                        Plr->Gossip_SendPOI(14);
                        SendQuickMenu(902);
                    }
                    break;

                case 17: //Hunter
                    {
                        Plr->Gossip_SendPOI(15);
                        SendQuickMenu(905);
                    }
                    break;

                case 18: //Mage
                    {
                        Plr->Gossip_SendPOI(16);
                        SendQuickMenu(899);
                    }
                    break;

                case 19: //Paladin
                    {
                        Plr->Gossip_SendPOI(17);
                        SendQuickMenu(904);
                    }
                    break;

                case 20: //Priest
                    {
                        Plr->Gossip_SendPOI(18);
                        SendQuickMenu(903);
                    }
                    break;

                case 21: //Rogue
                    {
                        Plr->Gossip_SendPOI(19);
                        SendQuickMenu(900);
                    }
                    break;

                case 22: //Shaman
                    {
                        Plr->Gossip_SendPOI(20);
                        SendQuickMenu(10106);
                    }
                    break;

                case 23: //Warlock
                    {
                        Plr->Gossip_SendPOI(21);
                        SendQuickMenu(906);
                    }
                    break;

                case 24: //Warrior
                    {
                        Plr->Gossip_SendPOI(22);
                        SendQuickMenu(901);
                    }
                    break;

                case 25: //Alchemy
                    {
                        Plr->Gossip_SendPOI(23);
                        SendQuickMenu(919);
                    }
                    break;

                case 26: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(24);
                        SendQuickMenu(920);
                    }
                    break;

                case 27: //Cooking
                    {
                        Plr->Gossip_SendPOI(25);
                        SendQuickMenu(921);
                    }
                    break;

                case 28: //Enchanting
                    {
                        Plr->Gossip_SendPOI(26);
                        SendQuickMenu(941);
                    }
                    break;

                case 29: //Engineering
                    {
                        Plr->Gossip_SendPOI(27);
                        SendQuickMenu(922);
                    }
                    break;

                case 30: //First Aid
                    {
                        Plr->Gossip_SendPOI(28);
                        SendQuickMenu(923);
                    }
                    break;

                case 31: //Fishing
                    {
                        Plr->Gossip_SendPOI(29);
                        SendQuickMenu(940);
                    }
                    break;

                case 32: //Herbalism
                    {
                        Plr->Gossip_SendPOI(30);
                        SendQuickMenu(924);
                    }
                    break;

                case 33: //Inscription
                    {
                        Plr->Gossip_SendPOI(31);
                        SendQuickMenu(13881);
                    }
                    break;

                case 34: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(32);
                        SendQuickMenu(925);
                    }
                    break;

                case 35: //Mining
                    {
                        Plr->Gossip_SendPOI(33);
                        SendQuickMenu(927);
                    }
                    break;

                case 36: //Skinning
                    {
                        Plr->Gossip_SendPOI(34);
                        SendQuickMenu(928);
                    }
                    break;

                case 37: //Tailoring
                    {
                        Plr->Gossip_SendPOI(35);
                        SendQuickMenu(929);
                    }
                    break;
            }
        }
};

class DarnassusGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3016, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_HYPPOGRYPH_M), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD2_M), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MAILBOX), 6);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE2_M), 7);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_WEAPON3_M), 8);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BATTLE3_M), 9);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS2_T), 10);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION2_T), 11);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_LEXICON_OF_POWER), 27);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     // Auction House
                    {
                        SendQuickMenu(3833);
                        Plr->Gossip_SendPOI(36);
                    }
                    break;

                case 2:        // The Bank
                    {
                        SendQuickMenu(3017);
                        Plr->Gossip_SendPOI(37);
                    }
                    break;

                case 3:        // Hippogryph Master
                    {
                        SendQuickMenu(3018);
                        Plr->Gossip_SendPOI(38);
                    }
                    break;

                case 4:        // Guild Master
                    {
                        SendQuickMenu(3019);
                        Plr->Gossip_SendPOI(39);
                    }
                    break;

                case 5:        // The Inn
                    {
                        SendQuickMenu(3020);
                        Plr->Gossip_SendPOI(40);
                    }
                    break;

                case 6:        // Mailbox
                    {
                        SendQuickMenu(3021);
                        Plr->Gossip_SendPOI(41);
                    }
                    break;

                case 7:        // Stable Master
                    {
                        SendQuickMenu(5980);
                        Plr->Gossip_SendPOI(42);
                    }
                    break;

                case 8:        // Weapons Trainer
                    {
                        SendQuickMenu(4517);
                        Plr->Gossip_SendPOI(43);
                    }
                    break;

                case 9:    // Battlemaster
                    {
                        SendQuickMenu(7519);
                        Plr->Gossip_SendPOI(44);
                    }
                    break;

                case 10:    // Class Trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4264, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_DRUID), 12);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HUNTER), 13);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 14);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ROGUE), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR), 16);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 11:    // Profession Trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4273, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_INSCRIPTION), 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 25);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 26);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 12:    // Druid
                    {
                        Plr->Gossip_SendPOI(45);
                        SendQuickMenu(3024);
                    }
                    break;

                case 13:    // Hunter
                    {
                        Plr->Gossip_SendPOI(46);
                        SendQuickMenu(3023);
                    }
                    break;

                case 14:    // Priest
                    {
                        Plr->Gossip_SendPOI(47);
                        SendQuickMenu(3025);
                    }
                    break;

                case 15:    // Rogue
                    {
                        Plr->Gossip_SendPOI(48);
                        SendQuickMenu(3026);
                    }
                    break;

                case 16:    // Warrior
                    {
                        Plr->Gossip_SendPOI(49);
                        SendQuickMenu(3033);
                    }
                    break;

                case 17: //Alchemy
                    {
                        Plr->Gossip_SendPOI(50);
                        SendQuickMenu(3035);
                    }
                    break;

                case 18: //Cooking
                    {
                        Plr->Gossip_SendPOI(51);
                        SendQuickMenu(3036);
                    }
                    break;

                case 19: //Enchanting
                    {
                        Plr->Gossip_SendPOI(52);
                        SendQuickMenu(3337);
                    }
                    break;

                case 20: //First Aid
                    {
                        Plr->Gossip_SendPOI(53);
                        SendQuickMenu(3037);
                    }
                    break;

                case 21: //Fishing
                    {
                        Plr->Gossip_SendPOI(54);
                        SendQuickMenu(3038);
                    }
                    break;

                case 22: //Herbalism
                    {
                        Plr->Gossip_SendPOI(55);
                        SendQuickMenu(3039);
                    }
                    break;
                case 23: //Inscription
                    {
                        Plr->Gossip_SendPOI(56);
                        SendQuickMenu(13886);
                    }
                    break;

                case 24: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(57);
                        SendQuickMenu(3040);
                    }
                    break;

                case 25: //Skinning
                    {
                        Plr->Gossip_SendPOI(58);
                        SendQuickMenu(3042);
                    }
                    break;

                case 26: //Tailoring
                    {
                        Plr->Gossip_SendPOI(59);
                        SendQuickMenu(3044);
                    }
                    break;

                case 27: //Lexicon of Power
                    {
                        Plr->Gossip_SendPOI(60);
                        SendQuickMenu(14174);
                    }
                    break;
            }
        }
        void GossipEnd(Object* pObject, Player* Plr)
        {

        }
};

class GoldshireGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4259, plr);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK2), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GRYPHON_M), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD2_M), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN2), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE2_M), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS2_T), 6);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION2_T), 7);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {

                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     //Bank
                    {
                        SendQuickMenu(4260);
                    }
                    break;

                case 2:     //Gryphon Master
                    {
                        SendQuickMenu(4261);
                    }
                    break;

                case 3:     //Guild Master
                    {
                        SendQuickMenu(4262);
                    }
                    break;

                case 4:     //Inn
                    {
                        SendQuickMenu(4263);
                        Plr->Gossip_SendPOI(61);
                    }
                    break;

                case 5:     //Stable Master
                    {
                        SendQuickMenu(5983);
                        Plr->Gossip_SendPOI(62);
                    }
                    break;

                case 6:     //Class Trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4264, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_DRUID), 8);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HUNTER), 9);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MAGE), 10);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PALADIN), 11);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 12);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ROGUE), 13);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SHAMAN), 14);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARLOCK), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR), 16);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 7:        //Profession Trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4273, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING), 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_INSCRIPTION), 25);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 26);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 27);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 28);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 29);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 8: //Druid
                    {
                        SendQuickMenu(4265);
                    }
                    break;

                case 9: //Hunter
                    {
                        SendQuickMenu(4266);
                    }
                    break;

                case 10: //Mage
                    {
                        Plr->Gossip_SendPOI(63);
                        SendQuickMenu(4268);
                    }
                    break;

                case 11: //Paladin
                    {
                        Plr->Gossip_SendPOI(64);
                        SendQuickMenu(4269);
                    }
                    break;

                case 12: //Priest
                    {
                        Plr->Gossip_SendPOI(65);
                        SendQuickMenu(4267);
                    }
                    break;

                case 13: //Rogue
                    {
                        Plr->Gossip_SendPOI(66);
                        SendQuickMenu(4270);
                    }
                    break;

                case 14: //Shaman
                    {
                        SendQuickMenu(10101);
                    }
                    break;

                case 15: //Warlock
                    {
                        Plr->Gossip_SendPOI(67);
                        SendQuickMenu(4272);
                    }
                    break;

                case 16: //Warrior
                    {
                        Plr->Gossip_SendPOI(68);
                        SendQuickMenu(4271);
                    }
                    break;

                case 17: //Alchemy
                    {
                        Plr->Gossip_SendPOI(69);
                        SendQuickMenu(4274);
                    }
                    break;

                case 18: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(70);
                        SendQuickMenu(4275);
                    }
                    break;

                case 19: //Cooking
                    {
                        Plr->Gossip_SendPOI(71);
                        SendQuickMenu(4276);
                    }
                    break;

                case 20: //Enchanting
                    {
                        SendQuickMenu(4277);
                    }
                    break;

                case 21: //Engineering
                    {
                        SendQuickMenu(4278);
                    }
                    break;

                case 22: //First Aid
                    {
                        Plr->Gossip_SendPOI(72);
                        SendQuickMenu(4279);
                    }
                    break;

                case 23: //Fishing
                    {
                        Plr->Gossip_SendPOI(73);
                        SendQuickMenu(4280);
                    }
                    break;

                case 24: //Herbalism
                    {
                        Plr->Gossip_SendPOI(74);
                        SendQuickMenu(4281);
                    }
                    break;

                case 25: //Inscription
                    {
                        Plr->Gossip_SendPOI(75);
                        SendQuickMenu(13881);
                    }
                    break;

                case 26: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(76);
                        SendQuickMenu(4282);
                    }
                    break;

                case 27: //Mining
                    {
                        SendQuickMenu(4283);
                    }
                    break;

                case 28: //Skinning
                    {
                        Plr->Gossip_SendPOI(77);
                        SendQuickMenu(4284);
                    }
                    break;

                case 29: //Tailoring
                    {
                        Plr->Gossip_SendPOI(78);
                        SendQuickMenu(4285);
                    }
                    break;
            }
        }
};

class UndercityGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3543, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BAT_H), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD_M), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_MAILBOX), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_AH), 6);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_ZEPPELIN_M), 7);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_WEAPON_M), 8);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE_M), 9);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BATTLE_M), 10);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS_T), 11);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION_T), 12);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_LOCKSMITH), 32);
            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     // The bank
                    {
                        SendQuickMenu(3514);
                        Plr->Gossip_SendPOI(79);
                    }
                    break;

                case 2:     // The bat handler
                    {
                        SendQuickMenu(3515);
                        Plr->Gossip_SendPOI(80);
                    }
                    break;

                case 3:     // The guild master
                    {
                        SendQuickMenu(3516);
                        Plr->Gossip_SendPOI(81);
                    }
                    break;

                case 4:     // The inn
                    {
                        SendQuickMenu(3517);
                        Plr->Gossip_SendPOI(82);
                    }
                    break;

                case 5:     // The mailbox
                    {
                        SendQuickMenu(3518);
                        Plr->Gossip_SendPOI(83);
                    }
                    break;

                case 6:     // The auction house
                    {
                        SendQuickMenu(3520);
                        Plr->Gossip_SendPOI(84);
                    }
                    break;

                case 7:     // The zeppelin master
                    {
                        SendQuickMenu(3519);
                        Plr->Gossip_SendPOI(85);
                    }
                    break;

                case 8:     // The weapon master
                    {
                        SendQuickMenu(4521);
                        Plr->Gossip_SendPOI(86);
                    }
                    break;

                case 9:     // The stable master
                    {
                        SendQuickMenu(5979);
                        Plr->Gossip_SendPOI(87);
                    }
                    break;

                case 10:    // The battlemaster
                    {
                        SendQuickMenu(7527);
                        Plr->Gossip_SendPOI(88);
                    }
                    break;

                case 11:    // A class trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3542, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MAGE)         , 13);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PALADIN)      , 14);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST)       , 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ROGUE)        , 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARLOCK)      , 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR)      , 18);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 12:    // A profession trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3541, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY)           , 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING)     , 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING)           , 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING)       , 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING)       , 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID)         , 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING)          , 25);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM)         , 26);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W)    , 27);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING)            , 28);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING)          , 29);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING)         , 30);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 13: //Mage
                    {
                        Plr->Gossip_SendPOI(89);
                        SendQuickMenu(3513);
                    }
                    break;

                case 14: //Paladin
                    {
                        Plr->Gossip_SendPOI(90);
                        SendQuickMenu(3521);
                    }
                    break;

                case 15: //Priest
                    {
                        Plr->Gossip_SendPOI(91);
                        SendQuickMenu(3521);
                    }
                    break;

                case 16: //Rogue
                    {
                        Plr->Gossip_SendPOI(92);
                        SendQuickMenu(3526);
                    }
                    break;

                case 17: //Warlock
                    {
                        Plr->Gossip_SendPOI(93);
                        SendQuickMenu(3526);
                    }
                    break;

                case 18: //Warrior
                    {
                        Plr->Gossip_SendPOI(94);
                        SendQuickMenu(3527);
                    }
                    break;

                case 19: //Alchemy
                    {
                        Plr->Gossip_SendPOI(95);
                        SendQuickMenu(3528);
                    }
                    break;

                case 20: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(96);
                        SendQuickMenu(3529);
                    }
                    break;

                case 21: //Cooking
                    {
                        Plr->Gossip_SendPOI(97);
                        SendQuickMenu(3530);
                    }
                    break;

                case 22: //Enchanting
                    {
                        Plr->Gossip_SendPOI(98);
                        SendQuickMenu(3531);
                    }
                    break;

                case 23: //Engineering
                    {
                        Plr->Gossip_SendPOI(99);
                        SendQuickMenu(3532);
                    }
                    break;

                case 24: //First Aid
                    {
                        Plr->Gossip_SendPOI(100);
                        SendQuickMenu(3533);
                    }
                    break;

                case 25: //Fishing
                    {
                        Plr->Gossip_SendPOI(101);
                        SendQuickMenu(3534);
                    }
                    break;

                case 26: //Herbalism
                    {
                        Plr->Gossip_SendPOI(102);
                        SendQuickMenu(3535);
                    }
                    break;

                case 27: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(103);
                        SendQuickMenu(3536);
                    }
                    break;

                case 28: //Mining
                    {
                        Plr->Gossip_SendPOI(104);
                        SendQuickMenu(3537);
                    }
                    break;

                case 29: //Skinning
                    {
                        Plr->Gossip_SendPOI(105);
                        SendQuickMenu(3538);
                    }
                    break;

                case 30: //Tailoring
                    {
                        Plr->Gossip_SendPOI(106);
                        SendQuickMenu(3539);
                    }
                    break;
                case 32:     // Locksmith
                    {
                        Plr->Gossip_SendPOI(107);
                        SendQuickMenu(14916);
                    }break;
            }
        }
};

class UndercityGuardOverseer : public GossipScript
{
    public:
        void OnHello(Object* pObject, Player* Plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 15321, Plr);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_AH), 1);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 2);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BARBER), 3);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BAT_H), 4);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BATTLE_M), 5);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD_M), 6);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 7);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_LOCKSMITH), 8);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_MAILBOX), 9);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE_M), 10);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_WEAPON_M), 11);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_ZEPPELIN_M), 12);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_A_CLASS_T), 13);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION_T), 14);
            Menu->SendTo(Plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:    
                    OnHello(pObject, Plr);
                    break;

                case 1:     // The auction house
                    {
                        SendQuickMenu(14900);
                        Plr->Gossip_SendPOI(108);
                    }
                    break;

                case 2:     // The bank
                    {
                        SendQuickMenu(14901);
                        Plr->Gossip_SendPOI(109);
                    }
                    break;

                case 3:     // Barber
                    {
                        SendQuickMenu(14902);
                        Plr->Gossip_SendPOI(110);
                    }
                    break;

                case 4:     // The bat handler
                    {
                        SendQuickMenu(14903);
                        Plr->Gossip_SendPOI(111);
                    }
                    break;

                case 5:    // The battlemaster
                    {
                        SendQuickMenu(14904);
                        Plr->Gossip_SendPOI(112);
                    }
                    break;

                case 6:     // The guild master
                    {
                        SendQuickMenu(14911);
                        Plr->Gossip_SendPOI(113);
                    }
                    break;

                case 7:     // The inn
                    {
                        SendQuickMenu(14913);
                        Plr->Gossip_SendPOI(114);
                    }
                    break;

                case 8:     // Locksmith
                    {
                        SendQuickMenu(14916);
                        Plr->Gossip_SendPOI(115);
                    }
                    break;

                case 9:     // The mailbox
                    {
                        SendQuickMenu(14918);
                        Plr->Gossip_SendPOI(116);
                    }
                    break;

                case 10:     // The stable master
                    {
                        SendQuickMenu(14924);
                        Plr->Gossip_SendPOI(117);
                    }
                    break;

                case 11:     // The weapon master
                    {
                        SendQuickMenu(14928);
                        Plr->Gossip_SendPOI(118);
                    }
                    break;

                case 12:     // The zeppelin master
                    {
                        SendQuickMenu(14929);
                        Plr->Gossip_SendPOI(119);
                    }
                    break;

                case 13:    // A class trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3542, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MAGE), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PALADIN), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ROGUE), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARLOCK), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR), 20);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 14:    // A profession trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3541, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING), 25);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 26);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 27);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 28);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_INSCRIPTION) , 29);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 30);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 31);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 32);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 33);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 15: //Mage
                    {
                        SendQuickMenu(3513);
                        Plr->Gossip_SendPOI(120);
                    }
                    break;

                case 16: //Paladin
                    {
                        SendQuickMenu(3521);
                        Plr->Gossip_SendPOI(121);
                    }
                    break;

                case 17: //Priest
                    {
                        SendQuickMenu(3521);
                        Plr->Gossip_SendPOI(122);
                    }
                    break;

                case 18: //Rogue
                    {
                        SendQuickMenu(3526);
                        Plr->Gossip_SendPOI(123);
                    }
                    break;

                case 19: //Warlock
                    {
                        SendQuickMenu(3526);
                        Plr->Gossip_SendPOI(124);
                    }
                    break;

                case 20: //Warrior
                    {
                        SendQuickMenu(3527);
                        Plr->Gossip_SendPOI(125);
                    }
                    break;

                case 21: //Alchemy
                    {
                        SendQuickMenu(3528);
                        Plr->Gossip_SendPOI(126);
                    }
                    break;

                case 22: //Blacksmithing
                    {
                        SendQuickMenu(3529);
                        Plr->Gossip_SendPOI(127);
                    }
                    break;

                case 23: //Cooking
                    {
                        SendQuickMenu(3530);
                        Plr->Gossip_SendPOI(128);
                    }
                    break;

                case 24: //Enchanting
                    {
                        SendQuickMenu(3531);
                        Plr->Gossip_SendPOI(129);
                    }
                    break;

                case 25: //Engineering
                    {
                        SendQuickMenu(3532);
                        Plr->Gossip_SendPOI(130);
                    }
                    break;

                case 26: //First Aid
                    {
                        SendQuickMenu(3533);
                        Plr->Gossip_SendPOI(131);
                    }
                    break;

                case 27: //Fishing
                    {
                        SendQuickMenu(3534);
                        Plr->Gossip_SendPOI(132);
                    }
                    break;

                case 28: //Herbalism
                    {
                        SendQuickMenu(3535);
                        Plr->Gossip_SendPOI(133);
                    }
                    break;

                case 29: //Inscription
                    {
                        SendQuickMenu(14914);
                        Plr->Gossip_SendPOI(134);
                    }
                    break;

                case 30: //Leatherworking
                    {
                        SendQuickMenu(3536);
                        Plr->Gossip_SendPOI(135);
                    }
                    break;

                case 31: //Mining
                    {
                        SendQuickMenu(3537);
                        Plr->Gossip_SendPOI(136);
                    }
                    break;

                case 32: //Skinning
                    {
                        SendQuickMenu(3538);
                        Plr->Gossip_SendPOI(137);
                    }
                    break;

                case 33: //Tailoring
                    {
                        SendQuickMenu(3539);
                        Plr->Gossip_SendPOI(138);
                    }
                    break;

            }
        }
};

class TeldrassilGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4316, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(530), 2);                 // Rut'Theran Ferry
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD_M), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE2_M), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS2_T), 6);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION2_T), 7);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     // The Bank
                    {
                        SendQuickMenu(4317);
                    }
                    break;

                case 2:        // Rut'theran erry
                    {
                        SendQuickMenu(4318);
                    }
                    break;

                case 3:        // The Guild Master
                    {
                        SendQuickMenu(4319);
                    }
                    break;

                case 4:        // The Inn
                    {
                        Plr->Gossip_SendPOI(139);
                        SendQuickMenu(4320);
                    }
                    break;

                case 5:        // Stable Master
                    {
                        Plr->Gossip_SendPOI(140);
                        SendQuickMenu(5982);
                    }
                    break;

                case 6:    // Class Trainers
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4264, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_DRUID), 8);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HUNTER), 9);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 10);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ROGUE), 11);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR), 12);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 7:    // Profession Trainers
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4273, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 13);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 14);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_INSCRIPTION), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 22);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 8: //Druid
                    {
                        Plr->Gossip_SendPOI(141);
                        SendQuickMenu(4323);
                    }
                    break;

                case 9: // Hunter
                    {
                        Plr->Gossip_SendPOI(142);
                        SendQuickMenu(4324);
                    }
                    break;

                case 10: // Priest
                    {
                        Plr->Gossip_SendPOI(143);
                        SendQuickMenu(4325);
                    }
                    break;

                case 11: // Rogue
                    {
                        Plr->Gossip_SendPOI(144);
                        SendQuickMenu(4326);
                    }
                    break;

                case 12: // Warrior
                    {
                        Plr->Gossip_SendPOI(145);
                        SendQuickMenu(4327);
                    }
                    break;

                case 13: // Alchemy
                    {
                        Plr->Gossip_SendPOI(146);
                        SendQuickMenu(4329);
                    }
                    break;

                case 14: // Cooking
                    {
                        Plr->Gossip_SendPOI(147);
                        SendQuickMenu(4330);
                    }
                    break;

                case 15: // Enchanting
                    {
                        Plr->Gossip_SendPOI(148);
                        SendQuickMenu(4331);
                    }
                    break;

                case 16: // First Aid
                    {
                        Plr->Gossip_SendPOI(149);
                        SendQuickMenu(4332);
                    }
                    break;

                case 17: // Fishing
                    {
                        SendQuickMenu(4333);
                    }
                    break;

                case 18: // Herbalism
                    {
                        Plr->Gossip_SendPOI(150);
                        SendQuickMenu(4334);
                    }
                    break;

                case 19: // Inscription
                    {
                        Plr->Gossip_SendPOI(151);
                        SendQuickMenu(13886);
                    }
                    break;

                case 20: // Leatherworking
                    {
                        Plr->Gossip_SendPOI(152);
                        SendQuickMenu(4335);
                    }
                    break;

                case 21: // Skinning
                    {
                        Plr->Gossip_SendPOI(153);
                        SendQuickMenu(4336);
                    }
                    break;

                case 22: // Tailoring
                    {
                        SendQuickMenu(4337);
                    }
                    break;
            }
        }
};

class SilvermoonGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9316, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DRAGONHAWK_M), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD2_M), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MAILBOX), 6);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE2_M), 7);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_WEAPON2_M), 8);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BATTLE3_M), 9);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS2_T), 10);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION2_T), 11);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MANA_LOOM), 12);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_LEXICON_OF_POWER), 40);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     // Auction House
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9317, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_TO_THE_WEST), 13);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_TO_THE_EAST), 14);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 2:     // The Bank
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9320, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_WEST), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_EAST), 16);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 3:     // Dragonhawk Master
                    {
                        SendQuickMenu(9323);
                        Plr->Gossip_SendPOI(154);
                    }
                    break;

                case 4:     // Guild Master
                    {
                        SendQuickMenu(9324);
                        Plr->Gossip_SendPOI(155);
                    }
                    break;

                case 5:     // The Inn
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9325, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SILVERMOON_C_INN), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_WAYFARERS_TAV), 18);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 6:     // Mailbox
                    {
                        SendQuickMenu(9326);
                        Plr->Gossip_SendPOI(156);
                    }
                    break;

                case 7:     // Stable Master
                    {
                        SendQuickMenu(9327);
                        Plr->Gossip_SendPOI(157);
                    }
                    break;

                case 8:     // Weapon Master
                    {
                        SendQuickMenu(9328);
                        Plr->Gossip_SendPOI(158);
                    }
                    break;

                case 9:     // Battlemasters
                    {
                        SendQuickMenu(9329);
                        Plr->Gossip_SendPOI(159);
                    }
                    break;

                case 10:    // Class Trainers
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9331, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_DRUID), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HUNTER), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MAGE), 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PALADIN), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ROGUE), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARLOCK), 25);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 11:    // Profession Trainers
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9338, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 26);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 27);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 28);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 29);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING), 30);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 31);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 32);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 33);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_INSCRIPTION), 34);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_JUWELCRAFTING), 35);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 36);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 37);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 38);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 39);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 12: //Mana Loom
                    {
                        Plr->Gossip_SendPOI(160);
                        SendQuickMenu(10502);
                    }
                    break;

                case 13: //To the west - Auction House no. 1
                    {
                        Plr->Gossip_SendPOI(161);
                        SendQuickMenu(9318);
                    }
                    break;

                case 14: //To the east - Auction House no. 2
                    {
                        Plr->Gossip_SendPOI(162);
                        SendQuickMenu(9319);
                    }
                    break;

                case 15:     // The bank - The west.
                    {
                        SendQuickMenu(9321);
                        Plr->Gossip_SendPOI(163);
                    }
                    break;

                case 16:     // The bank - The east.
                    {
                        SendQuickMenu(9322);
                        Plr->Gossip_SendPOI(164);
                    }
                    break;

                case 17: //The Silvermoon City Inn
                    {
                        Plr->Gossip_SendPOI(165);
                        SendQuickMenu(9325);
                    }
                    break;

                case 18: //The Wayfarer's Rest tavern
                    {
                        Plr->Gossip_SendPOI(166);
                        SendQuickMenu(9603);
                    }
                    break;

                case 19: //Druid
                    {
                        Plr->Gossip_SendPOI(167);
                        SendQuickMenu(9330);
                    }
                    break;

                case 20: //Hunter
                    {
                        Plr->Gossip_SendPOI(168);
                        SendQuickMenu(9332);
                    }
                    break;

                case 21: //Mage
                    {
                        Plr->Gossip_SendPOI(169);
                        SendQuickMenu(9333);
                    }
                    break;

                case 22: //Paladin
                    {
                        Plr->Gossip_SendPOI(170);
                        SendQuickMenu(9334);
                    }
                    break;

                case 23: //Priest
                    {
                        Plr->Gossip_SendPOI(171);
                        SendQuickMenu(9335);
                    }
                    break;

                case 24: //Rogue
                    {
                        Plr->Gossip_SendPOI(172);
                        SendQuickMenu(9336);
                    }
                    break;

                case 25: //Warlock
                    {
                        Plr->Gossip_SendPOI(173);
                        SendQuickMenu(9337);
                    }
                    break;

                case 26: //Alchemy
                    {
                        Plr->Gossip_SendPOI(174);
                        SendQuickMenu(9339);
                    }
                    break;

                case 27: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(175);
                        SendQuickMenu(9340);
                    }
                    break;

                case 28: //Cooking
                    {
                        Plr->Gossip_SendPOI(176);
                        SendQuickMenu(9624);
                    }
                    break;

                case 29: //Enchanting
                    {
                        Plr->Gossip_SendPOI(177);
                        SendQuickMenu(9341);
                    }
                    break;

                case 30: //Engineering
                    {
                        Plr->Gossip_SendPOI(178);
                        SendQuickMenu(9342);
                    }
                    break;

                case 31: //First Aid
                    {
                        Plr->Gossip_SendPOI(179);
                        SendQuickMenu(9343);
                    }
                    break;

                case 32: //Fishing
                    {
                        Plr->Gossip_SendPOI(180);
                        SendQuickMenu(9344);
                    }
                    break;

                case 33: //Herbalism
                    {
                        Plr->Gossip_SendPOI(181);
                        SendQuickMenu(9345);
                    }
                    break;
                case 34: //Inscription
                    {
                        Plr->Gossip_SendPOI(182);
                        SendQuickMenu(13893);
                    }
                    break;

                case 35: //Jewelcrafting
                    {
                        Plr->Gossip_SendPOI(183);
                        SendQuickMenu(9346);
                    }
                    break;

                case 36: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(184);
                        SendQuickMenu(9347);
                    }
                    break;

                case 37: //Mining
                    {
                        Plr->Gossip_SendPOI(185);
                        SendQuickMenu(9348);
                    }
                    break;

                case 38: //Skinning
                    {
                        Plr->Gossip_SendPOI(186);
                        SendQuickMenu(9349);
                    }
                    break;

                case 39: //Tailoring
                    {
                        Plr->Gossip_SendPOI(187);
                        SendQuickMenu(9350);
                    }
                    break;
                case 40: //Lexicon of Power
                    {
                        Plr->Gossip_SendPOI(188);
                        SendQuickMenu(14174);
                    }
                    break;
            }
        }
};

class ExodarGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9551, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_HYPPOGRYPH_M), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD2_M), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MAILBOX), 6);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE2_M), 7);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_WEAPON2_M), 8);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BATTLE2_M), 9);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS2_T), 10);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION2_T), 11);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_LEXICON_OF_POWER), 34);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     // Auction House
                    {
                        SendQuickMenu(9528);
                        Plr->Gossip_SendPOI(189);
                    }
                    break;

                case 2:     // The Bank
                    {
                        SendQuickMenu(9529);
                        Plr->Gossip_SendPOI(190);
                    }
                    break;

                case 3:     // Hippogryph Master
                    {
                        SendQuickMenu(9530);
                        Plr->Gossip_SendPOI(191);
                    }
                    break;

                case 4:     // Guild Master
                    {
                        SendQuickMenu(9539);
                        Plr->Gossip_SendPOI(192);
                    }
                    break;

                case 5:     // The Inn
                    {
                        SendQuickMenu(9545);
                        Plr->Gossip_SendPOI(193);
                    }
                    break;

                case 6:     // Mailbox
                    {
                        SendQuickMenu(10254);
                        Plr->Gossip_SendPOI(194);
                    }
                    break;

                case 7:     // Stable Master
                    {
                        SendQuickMenu(9558);
                        Plr->Gossip_SendPOI(195);
                    }
                    break;

                case 8:     // Weapon Master
                    {
                        SendQuickMenu(9565);
                        Plr->Gossip_SendPOI(196);
                    }
                    break;

                case 9:     // Battlemasters
                    {
                    Plr->Gossip_SendPOI(197);
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9531, Plr);
                    Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BATTLE3_M), 12);
                    Menu->SendTo(Plr);
                    }
                    break;

                case 10:    // Class Trainers
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9533, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_DRUID), 13);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HUNTER), 14);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MAGE), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PALADIN), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SHAMAN), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR), 19);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 11:    // Profession Trainers
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9555, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING), 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 25);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 26);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_INSCRIPTION), 27);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_JUWELCRAFTING), 28);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 29);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 30);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 31);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 32);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 33);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 12://Arena Battlemaster Exodar
                    {
                        Plr->Gossip_SendPOI(198);
                        SendQuickMenu(10223);
                    }
                    break;

                case 13: //Druid
                    {
                        Plr->Gossip_SendPOI(199);
                        SendQuickMenu(9534);
                    }
                    break;

                case 14: //Hunter
                    {
                        Plr->Gossip_SendPOI(200);
                        SendQuickMenu(9544);
                    }
                    break;

                case 15: //Mage
                    {
                        Plr->Gossip_SendPOI(201);
                        SendQuickMenu(9550);
                    }
                    break;

                case 16: //Paladin
                    {
                        Plr->Gossip_SendPOI(202);
                        SendQuickMenu(9553);
                    }
                    break;

                case 17: //Priest
                    {
                        Plr->Gossip_SendPOI(203);
                        SendQuickMenu(9554);
                    }
                    break;

                case 18: //Shaman
                    {
                        Plr->Gossip_SendPOI(204);
                        SendQuickMenu(9556);
                    }
                    break;

                case 19: //Warrior
                    {
                        Plr->Gossip_SendPOI(205);
                        SendQuickMenu(9562);
                    }
                    break;

                case 20: //Alchemy
                    {
                        Plr->Gossip_SendPOI(206);
                        SendQuickMenu(9527);
                    }
                    break;

                case 21: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(207);
                        SendQuickMenu(9532);
                    }
                    break;

                case 22: //Enchanting
                    {
                        Plr->Gossip_SendPOI(208);
                        SendQuickMenu(9535);
                    }
                    break;

                case 23: //Engineering
                    {
                        Plr->Gossip_SendPOI(209);
                        SendQuickMenu(9536);
                    }
                    break;

                case 24: //First Aid
                    {
                        Plr->Gossip_SendPOI(210);
                        SendQuickMenu(9537);
                    }
                    break;

                case 25: //Fishing
                    {
                        Plr->Gossip_SendPOI(211);
                        SendQuickMenu(9538);
                    }
                    break;

                case 26: //Herbalism
                    {
                        Plr->Gossip_SendPOI(212);
                        SendQuickMenu(9543);
                    }
                    break;

                case 27: //Inscription
                    {
                        Plr->Gossip_SendPOI(213);
                        SendQuickMenu(13887);
                    }
                    break;

                case 28: //Jewelcrafting
                    {
                        Plr->Gossip_SendPOI(214);
                        SendQuickMenu(9547);
                    }
                    break;

                case 29: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(215);
                        SendQuickMenu(9549);
                    }
                    break;

                case 30: //Mining
                    {
                        Plr->Gossip_SendPOI(216);
                        SendQuickMenu(9552);
                    }
                    break;

                case 31: //Skinning
                    {
                        Plr->Gossip_SendPOI(217);
                        SendQuickMenu(9557);
                    }
                    break;

                case 32: //Tailoring
                    {
                        Plr->Gossip_SendPOI(218);
                        SendQuickMenu(9350);
                    }
                    break;

                case 33: //Cooking
                    {
                        Plr->Gossip_SendPOI(219);
                        SendQuickMenu(9559);
                    }
                    break;

                case 34: //Lexicon of Power
                    {
                        Plr->Gossip_SendPOI(220);
                        SendQuickMenu(14174);
                    }
                    break;
            }
        }
};

class OrgrimmarGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2593, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_WIND_R_M), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD_M), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_MAILBOX), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_AH), 6);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_ZEPPELIN_M), 7);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_WEAPON_M), 8);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE_M), 9);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_OFFICERS), 10);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BATTLE_M), 11);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS_T), 12);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION_T), 13);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     // The bank
                    {
                        SendQuickMenu(2554);
                        Plr->Gossip_SendPOI(221);
                    }
                    break;

                case 2:     // The wind rider master
                    {
                        SendQuickMenu(2555);
                        Plr->Gossip_SendPOI(222);
                    }
                    break;

                case 3:     // The guild master
                    {
                        SendQuickMenu(2556);
                        Plr->Gossip_SendPOI(223);
                    }
                    break;

                case 4:     // The inn
                    {
                        SendQuickMenu(2557);
                        Plr->Gossip_SendPOI(224);
                    }
                    break;

                case 5:     // The mailbox
                    {
                        SendQuickMenu(2558);
                        Plr->Gossip_SendPOI(225);
                    }
                    break;

                case 6:     // The auction house
                    {
                        SendQuickMenu(3075);
                        Plr->Gossip_SendPOI(226);
                    }
                    break;

                case 7:     // The zeppelin master
                    {
                        SendQuickMenu(3173);
                        Plr->Gossip_SendPOI(227);
                    }
                    break;

                case 8:     // The weapon master
                    {
                        SendQuickMenu(4519);
                        Plr->Gossip_SendPOI(228);
                    }
                    break;

                case 9:     // The stable master
                    {
                        SendQuickMenu(5974);
                        Plr->Gossip_SendPOI(229);
                    }
                    break;

                case 10:    // The officers' lounge
                    {
                        SendQuickMenu(7046);
                        Plr->Gossip_SendPOI(230);
                    }
                    break;

                case 11:    // The battlemaster
                    {
                        SendQuickMenu(7521);
                        Plr->Gossip_SendPOI(231);
                    }
                    break;

                case 12:    // A class trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2599, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HUNTER), 14);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MAGE), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SHAMAN), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ROGUE), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARLOCK), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PALADIN), 21);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 13:    // A profession trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2594, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 25);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING), 26);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 27);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 28);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 29);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 30);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 31);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 32);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 33);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 14: //Hunter
                    {
                        Plr->Gossip_SendPOI(232);
                        SendQuickMenu(2559);
                    }
                    break;

                case 15: //Mage
                    {
                        Plr->Gossip_SendPOI(233);
                        SendQuickMenu(2560);
                    }
                    break;

                case 16: //Priest
                    {
                        Plr->Gossip_SendPOI(234);
                        SendQuickMenu(2561);
                    }
                    break;

                case 17: //Shaman
                    {
                        Plr->Gossip_SendPOI(235);
                        SendQuickMenu(2562);
                    }
                    break;

                case 18: //Rogue
                    {
                        Plr->Gossip_SendPOI(236);
                        SendQuickMenu(2563);
                    }
                    break;

                case 19: //Warlock
                    {
                        Plr->Gossip_SendPOI(237);
                        SendQuickMenu(2564);
                    }
                    break;

                case 20: //Warrior
                    {
                        Plr->Gossip_SendPOI(238);
                        SendQuickMenu(2565);
                    }
                    break;

                case 21: //Paladin
                    {
                        Plr->Gossip_SendPOI(239);
                        SendQuickMenu(2566);
                    }
                    break;

                case 22: //Alchemy
                    {
                        Plr->Gossip_SendPOI(340);
                        SendQuickMenu(2497);
                    }
                    break;

                case 23: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(241);
                        SendQuickMenu(2499);
                    }
                    break;

                case 24: //Cooking
                    {
                        Plr->Gossip_SendPOI(242);
                        SendQuickMenu(2500);
                    }
                    break;

                case 25: //Enchanting
                    {
                        Plr->Gossip_SendPOI(243);
                        SendQuickMenu(2501);
                    }
                    break;

                case 26: //Engineering
                    {
                        Plr->Gossip_SendPOI(244);
                        SendQuickMenu(2653);
                    }
                    break;

                case 27: //First Aid
                    {
                        Plr->Gossip_SendPOI(245);
                        SendQuickMenu(2502);
                    }
                    break;

                case 28: //Fishing
                    {
                        Plr->Gossip_SendPOI(246);
                        SendQuickMenu(2503);
                    }
                    break;

                case 29: //Herbalism
                    {
                        Plr->Gossip_SendPOI(247);
                        SendQuickMenu(2504);
                    }
                    break;

                case 30: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(248);
                        SendQuickMenu(2513);
                    }
                    break;

                case 31: //Mining
                    {
                        Plr->Gossip_SendPOI(249);
                        SendQuickMenu(2515);
                    }
                    break;

                case 32: //Skinning
                    {
                        Plr->Gossip_SendPOI(250);
                        SendQuickMenu(2516);
                    }
                    break;

                case 33: //Tailoring
                    {
                        Plr->Gossip_SendPOI(251);
                        SendQuickMenu(2518);
                    }
                    break;
            }
        }
};

class ThunderbluffGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3543, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_WIND_R_M), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD_M), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_MAILBOX), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_AH), 6);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_WEAPON_M), 7);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE_M), 8);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BATTLE_M), 9);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS_T), 10);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION_T), 11);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     // The bank
                    {
                        SendQuickMenu(1292);
                        Plr->Gossip_SendPOI(252);
                    }
                    break;

                case 2:     // The wind rider master
                    {
                        SendQuickMenu(1293);
                        Plr->Gossip_SendPOI(253);
                    }
                    break;

                case 3:     // The guild master
                    {
                        SendQuickMenu(1291);
                        Plr->Gossip_SendPOI(254);
                    }
                    break;

                case 4:     // The inn
                    {
                        SendQuickMenu(3153);
                        Plr->Gossip_SendPOI(255);
                    }
                    break;

                case 5:     // The mailbox
                    {
                        SendQuickMenu(3154);
                        Plr->Gossip_SendPOI(256);
                    }
                    break;

                case 6:     // The auction house
                    {
                        SendQuickMenu(3155);
                        Plr->Gossip_SendPOI(257);
                    }
                    break;

                case 7:     // The weapon master
                    {
                        SendQuickMenu(4520);
                        Plr->Gossip_SendPOI(258);
                    }
                    break;

                case 8:     // The stable master
                    {
                        SendQuickMenu(5977);
                        Plr->Gossip_SendPOI(259);
                    }
                    break;

                case 9:    // The battlemaster
                    {
                        SendQuickMenu(7527);
                        Plr->Gossip_SendPOI(260);
                    }
                    break;

                case 10:    // A class trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3542, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_DRUID), 12);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HUNTER), 13);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MAGE), 14);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SHAMAN), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR), 17);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 11:    // A profession trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3541, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 25);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 26);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 27);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 28);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 12: //Druid
                    {
                        Plr->Gossip_SendPOI(261);
                        SendQuickMenu(1294);
                    }
                    break;

                case 13: //Hunter
                    {
                        Plr->Gossip_SendPOI(262);
                        SendQuickMenu(1295);
                    }
                    break;

                case 14: //Mage
                    {
                        Plr->Gossip_SendPOI(263);
                        SendQuickMenu(1296);
                    }
                    break;

                case 15: //Priest
                    {
                        Plr->Gossip_SendPOI(264);
                        SendQuickMenu(1297);
                    }
                    break;

                case 16: //Shaman
                    {
                        Plr->Gossip_SendPOI(265);
                        SendQuickMenu(1298);
                    }
                    break;

                case 17: //Warrior
                    {
                        Plr->Gossip_SendPOI(266);
                        SendQuickMenu(1299);
                    }
                    break;

                case 18: //Alchemy
                    {
                        Plr->Gossip_SendPOI(267);
                        SendQuickMenu(1332);
                    }
                    break;

                case 19: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(268);
                        SendQuickMenu(1333);
                    }
                    break;

                case 20: //Cooking
                    {
                        Plr->Gossip_SendPOI(269);
                        SendQuickMenu(1334);
                    }
                    break;

                case 21: //Enchanting
                    {
                        Plr->Gossip_SendPOI(270);
                        SendQuickMenu(1335);
                    }
                    break;

                case 22: //First Aid
                    {
                        Plr->Gossip_SendPOI(271);
                        SendQuickMenu(1336);
                    }
                    break;

                case 23: //Fishing
                    {
                        Plr->Gossip_SendPOI(272);
                        SendQuickMenu(1337);
                    }
                    break;

                case 24: //Herbalism
                    {
                        Plr->Gossip_SendPOI(273);
                        SendQuickMenu(1338);
                    }
                    break;

                case 25: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(274);
                        SendQuickMenu(1339);
                    }
                    break;

                case 26: //Mining
                    {
                        Plr->Gossip_SendPOI(275);
                        SendQuickMenu(1340);
                    }
                    break;

                case 27: //Skinning
                    {
                        Plr->Gossip_SendPOI(276);
                        SendQuickMenu(1343);
                    }
                    break;

                case 28: //Tailoring
                    {
                        Plr->Gossip_SendPOI(277);
                        SendQuickMenu(1341);
                    }
                    break;
            }
        }
};

class BloodhoofGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3543, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_WIND_R_M), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE_M), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS_T), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION_T), 6);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     // The bank
                    {
                        SendQuickMenu(4051);
                    }
                    break;

                case 2:     // The wind rider master
                    {
                        SendQuickMenu(4052);
                    }
                    break;

                case 3:     // The inn
                    {
                        SendQuickMenu(4053);
                        Plr->Gossip_SendPOI(278);
                    }
                    break;

                case 4:     // The stable master
                    {
                        SendQuickMenu(5976);
                        Plr->Gossip_SendPOI(279);
                    }
                    break;

                case 5:     // A class trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4069, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_DRUID), 7);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HUNTER), 8);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SHAMAN), 9);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR), 10);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 6:     // A profession trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3541, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 11);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 12);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 13);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 14);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 21);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 7: //Druid
                    {
                        Plr->Gossip_SendPOI(280);
                        SendQuickMenu(4054);
                    }
                    break;

                case 8: //Hunter
                    {
                        Plr->Gossip_SendPOI(281);
                        SendQuickMenu(4055);
                    }
                    break;

                case 9: //Shaman
                    {
                        Plr->Gossip_SendPOI(282);
                        SendQuickMenu(4056);
                    }
                    break;

                case 10: //Warrior
                    {
                        Plr->Gossip_SendPOI(283);
                        SendQuickMenu(4057);
                    }
                    break;

                case 11: //Alchemy
                    {
                        SendQuickMenu(4058);
                    }
                    break;

                case 12: //Blacksmithing
                    {
                        SendQuickMenu(4059);
                    }
                    break;

                case 13: //Cooking
                    {
                        Plr->Gossip_SendPOI(284);
                        SendQuickMenu(4060);
                    }
                    break;

                case 14: //Enchanting
                    {
                        SendQuickMenu(4061);
                    }
                    break;

                case 15: //First Aid
                    {
                        Plr->Gossip_SendPOI(285);
                        SendQuickMenu(4062);
                    }
                    break;

                case 16: //Fishing
                    {
                        Plr->Gossip_SendPOI(286);
                        SendQuickMenu(4063);
                    }
                    break;

                case 17: //Herbalism
                    {
                        SendQuickMenu(4064);
                    }
                    break;

                case 18: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(287);
                        SendQuickMenu(4065);
                    }
                    break;

                case 19: //Mining
                    {
                        SendQuickMenu(4066);
                    }
                    break;

                case 20: //Skinning
                    {
                        Plr->Gossip_SendPOI(288);
                        SendQuickMenu(4067);
                    }
                    break;

                case 21: //Tailoring
                    {
                        SendQuickMenu(4068);
                    }
                    break;
            }
        }
};

class RazorHillGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4037, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_WIND_R_M), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE_M), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS_T), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION_T), 6);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     // The bank
                    {
                        SendQuickMenu(4032);
                    }
                    break;

                case 2:     // The wind rider master
                    {
                        SendQuickMenu(4033);
                    }
                    break;

                case 3:     // The inn
                    {
                        SendQuickMenu(4034);
                        Plr->Gossip_SendPOI(289);
                    }
                    break;

                case 4:     // The stable master
                    {
                        SendQuickMenu(5973);
                        Plr->Gossip_SendPOI(290);
                    }
                    break;

                case 5:     // A class trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4035, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HUNTER), 7);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MAGE), 8);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 9);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ROGUE), 10);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SHAMAN), 11);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARLOCK), 12);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR), 13);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 6:     // A profession trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3541, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 14);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 25);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 7: //Hunter
                    {
                        Plr->Gossip_SendPOI(291);
                        SendQuickMenu(4013);
                    }
                    break;

                case 8: //Mage
                    {
                        Plr->Gossip_SendPOI(292);
                        SendQuickMenu(4014);
                    }
                    break;

                case 9: //Priest
                    {
                        Plr->Gossip_SendPOI(293);
                        SendQuickMenu(4015);
                    }
                    break;

                case 10: //Rogue
                    {
                        Plr->Gossip_SendPOI(294);
                        SendQuickMenu(4016);
                    }
                    break;

                case 11: //Shaman
                    {
                        Plr->Gossip_SendPOI(295);
                        SendQuickMenu(4017);
                    }
                    break;

                case 12: //Warlock
                    {
                        Plr->Gossip_SendPOI(296);
                        SendQuickMenu(4018);
                    }
                    break;

                case 13: //Warrior
                    {
                        Plr->Gossip_SendPOI(297);
                        SendQuickMenu(4019);
                    }
                    break;

                case 14: //Alchemy
                    {
                        Plr->Gossip_SendPOI(298);
                        SendQuickMenu(4020);
                    }
                    break;

                case 15: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(299);
                        SendQuickMenu(4021);
                    }
                    break;

                case 16: //Cooking
                    {
                        SendQuickMenu(4022);
                    }
                    break;

                case 17: //Enchanting
                    {
                        SendQuickMenu(4023);
                    }
                    break;

                case 18: //Engineering
                    {
                        Plr->Gossip_SendPOI(300);
                        SendQuickMenu(4024);
                    }
                    break;

                case 19: //First Aid
                    {
                        Plr->Gossip_SendPOI(301);
                        SendQuickMenu(4025);
                    }
                    break;

                case 20: //Fishing
                    {
                        Plr->Gossip_SendPOI(302);
                        SendQuickMenu(4026);
                    }
                    break;

                case 21: //Herbalism
                    {
                        Plr->Gossip_SendPOI(303);
                        SendQuickMenu(4027);
                    }
                    break;

                case 22: //Leatherworking
                    {
                        SendQuickMenu(4028);
                    }
                    break;

                case 23: //Mining
                    {
                        Plr->Gossip_SendPOI(304);
                        SendQuickMenu(4029);
                    }
                    break;

                case 24: //Skinning
                    {
                        Plr->Gossip_SendPOI(305);
                        SendQuickMenu(4030);
                    }
                    break;

                case 25: //Tailoring
                    {
                        SendQuickMenu(4031);
                    }
                    break;
            }
        }
};

class BrillGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2593, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BAT_H), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE_M), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS_T), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION_T), 6);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     // The bank
                    {
                        SendQuickMenu(4074);
                    }
                    break;

                case 2:     // The bat handler
                    {
                        SendQuickMenu(4075);
                    }
                    break;

                case 3:     // The inn
                    {
                        SendQuickMenu(4076);
                        Plr->Gossip_SendPOI(306);
                    }
                    break;

                case 4:     // The stable master
                    {
                        SendQuickMenu(5978);
                        Plr->Gossip_SendPOI(307);
                    }
                    break;

                case 5:     // A class trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4292, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MAGE), 7);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PALADIN), 8);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 9);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ROGUE), 10);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARLOCK), 11);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR), 12);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 6:     // A profession trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4300, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 13);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 14);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 24);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 7: //Mage
                    {
                        Plr->Gossip_SendPOI(308);
                        SendQuickMenu(4077);
                    }
                    break;

                case 8: //Paladin
                    {
                        SendQuickMenu(0); // Need to add correct text
                    }
                    break;

                case 9: //Priest
                    {
                        Plr->Gossip_SendPOI(309);
                        SendQuickMenu(4078);
                    }
                    break;

                case 10: //Rogue
                    {
                        Plr->Gossip_SendPOI(310);
                        SendQuickMenu(4079);
                    }
                    break;

                case 11: //Warlock
                    {
                        Plr->Gossip_SendPOI(311);
                        SendQuickMenu(4080);
                    }
                    break;

                case 12: //Warrior
                    {
                        Plr->Gossip_SendPOI(312);
                        SendQuickMenu(4081);
                    }
                    break;

                case 13: //Alchemy
                    {
                        Plr->Gossip_SendPOI(313);
                        SendQuickMenu(4082);
                    }
                    break;

                case 14: //Blacksmithing
                    {
                        SendQuickMenu(4083);
                    }
                    break;

                case 15: //Cooking
                    {
                        SendQuickMenu(4084);
                    }
                    break;

                case 16: //Enchanting
                    {
                        Plr->Gossip_SendPOI(314);
                        SendQuickMenu(4085);
                    }
                    break;

                case 17: //Engineering
                    {
                        SendQuickMenu(4086);
                    }
                    break;

                case 18: //First Aid
                    {
                        Plr->Gossip_SendPOI(315);
                        SendQuickMenu(4087);
                    }
                    break;

                case 19: //Fishing
                    {
                        Plr->Gossip_SendPOI(316);
                        SendQuickMenu(4088);
                    }
                    break;

                case 20: //Herbalism
                    {
                        Plr->Gossip_SendPOI(317);
                        SendQuickMenu(4089);
                    }
                    break;

                case 21: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(318);
                        SendQuickMenu(4090);
                    }
                    break;

                case 22: //Mining
                    {
                        SendQuickMenu(4091);
                    }
                    break;

                case 23: //Skinning
                    {
                        Plr->Gossip_SendPOI(319);
                        SendQuickMenu(4092);
                    }
                    break;

                case 24: //Tailoring
                    {
                        Plr->Gossip_SendPOI(320);
                        SendQuickMenu(4093);
                    }
                    break;
            }
        }
};

class IronforgeGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2760, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_BANK_OF_IRONFORGE), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DEEPRUN_TRAM), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GRYPHON_M), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD2_M), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 6);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MAILBOX), 7);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE2_M), 8);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_WEAPON3_M), 9);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BATTLE3_M), 10);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BARBER), 11);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS2_T), 12);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION2_T), 13);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_LEXICON_OF_POWER), 35);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     // Auction House
                    {
                        SendQuickMenu(3014);
                        Plr->Gossip_SendPOI(321);
                    }
                    break;

                case 2:     // Bank of Ironforge
                    {
                        SendQuickMenu(2761);
                        Plr->Gossip_SendPOI(322);
                    }
                    break;

                case 3:     // Deeprun Tram
                    {
                        SendQuickMenu(3814);
                        Plr->Gossip_SendPOI(323);
                    }
                    break;

                case 4:     // Gryphon Master
                    {
                        SendQuickMenu(2762);
                        Plr->Gossip_SendPOI(324);
                    }
                    break;

                case 5:     // Guild Master
                    {
                        SendQuickMenu(2764);
                        Plr->Gossip_SendPOI(325);
                    }
                    break;

                case 6:     // The Inn
                    {
                        SendQuickMenu(2768);
                        Plr->Gossip_SendPOI(326);
                    }
                    break;

                case 7:     // Mailbox
                    {
                        SendQuickMenu(2769);
                        Plr->Gossip_SendPOI(327);
                    }
                    break;

                case 8:     // Stable Master
                    {
                        SendQuickMenu(5986);
                        Plr->Gossip_SendPOI(328);
                    }
                    break;

                case 9:    // Weapon Trainer
                    {
                        SendQuickMenu(4518);
                        Plr->Gossip_SendPOI(329);
                    }
                    break;

                case 10:    // Battlemaster
                    {
                        SendQuickMenu(10216);
                        Plr->Gossip_SendPOI(330);
                    }
                    break;

                case 11:    // Barber
                    {
                        SendQuickMenu(13885);
                        Plr->Gossip_SendPOI(331);
                    }
                    break;

                case 12:    // A class trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2766, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HUNTER), 14);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MAGE), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PALADIN), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ROGUE), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARLOCK), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SHAMAN), 21);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 13:    // A profession trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2793, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 25);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING), 26);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 27);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 28);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 29);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_INSCRIPTION), 30);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 31);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 32);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 33);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 34);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 14: //Hunter
                    {
                        Plr->Gossip_SendPOI(332);
                        SendQuickMenu(2770);
                    }
                    break;

                case 15: //Mage
                    {
                        Plr->Gossip_SendPOI(333);
                        SendQuickMenu(2771);
                    }
                    break;

                case 16: //Paladin
                    {
                        Plr->Gossip_SendPOI(334);
                        SendQuickMenu(2773);
                    }
                    break;

                case 17: //Priest
                    {
                        Plr->Gossip_SendPOI(335);
                        SendQuickMenu(2772);
                    }
                    break;

                case 18: //Rogue
                    {
                        Plr->Gossip_SendPOI(336);
                        SendQuickMenu(2774);
                    }
                    break;

                case 19: //Warlock
                    {
                        Plr->Gossip_SendPOI(337);
                        SendQuickMenu(2775);
                    }
                    break;

                case 20: //Warrior
                    {
                        Plr->Gossip_SendPOI(338);
                        SendQuickMenu(2776);
                    }
                    break;

                case 21: //Shaman
                    {
                        Plr->Gossip_SendPOI(339);
                        SendQuickMenu(10842);
                    }
                    break;

                case 22: //Alchemy
                    {
                        Plr->Gossip_SendPOI(340);
                        SendQuickMenu(2794);
                    }
                    break;

                case 23: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(341);
                        SendQuickMenu(2795);
                    }
                    break;

                case 24: //Cooking
                    {
                        Plr->Gossip_SendPOI(342);
                        SendQuickMenu(2796);
                    }
                    break;

                case 25: //Enchanting
                    {
                        Plr->Gossip_SendPOI(343);
                        SendQuickMenu(2797);
                    }
                    break;

                case 26: //Engineering
                    {
                        Plr->Gossip_SendPOI(344);
                        SendQuickMenu(2798);
                    }
                    break;

                case 27: //First Aid
                    {
                        Plr->Gossip_SendPOI(345);
                        SendQuickMenu(2799);
                    }
                    break;

                case 28: //Fishing
                    {
                        Plr->Gossip_SendPOI(346);
                        SendQuickMenu(2800);
                    }
                    break;

                case 29: //Herbalism
                    {
                        Plr->Gossip_SendPOI(347);
                        SendQuickMenu(2801);
                    }
                    break;

                case 30: //Inscription
                    {
                        Plr->Gossip_SendPOI(348);
                        SendQuickMenu(13884);
                    }
                    break;

                case 31: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(349);
                        SendQuickMenu(2802);
                    }
                    break;

                case 32: //Mining
                    {
                        Plr->Gossip_SendPOI(350);
                        SendQuickMenu(2804);
                    }
                    break;

                case 33: //Skinning
                    {
                        Plr->Gossip_SendPOI(351);
                        SendQuickMenu(2805);
                    }
                    break;

                case 34: //Tailoring
                    {
                        Plr->Gossip_SendPOI(352);
                        SendQuickMenu(2807);
                    }
                    break;

                case 35: //Lexicon of Power
                    {
                        Plr->Gossip_SendPOI(353);
                        SendQuickMenu(14174);
                    }
                    break;
            }
        }
};

class KharanosGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4287, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK2), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GRYPHON_M), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD2_M), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE2_M), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS2_T), 6);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION2_T), 7);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     //Bank
                    {
                        SendQuickMenu(4288);
                    }
                    break;

                case 2:     //Gryphon Master
                    {
                        SendQuickMenu(4289);
                    }
                    break;

                case 3:     //Guild Master
                    {
                        SendQuickMenu(4290);
                    }
                    break;

                case 4:     //The Inn
                    {
                        SendQuickMenu(4291);
                        Plr->Gossip_SendPOI(354);
                    }
                    break;

                case 5:     //Stable Master
                    {
                        SendQuickMenu(5985);
                        Plr->Gossip_SendPOI(355);
                    }
                    break;

                case 6:     //Class Trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4292, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HUNTER), 8);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MAGE), 9);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PALADIN), 10);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 11);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ROGUE), 12);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARLOCK), 13);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR), 14);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 7:     // Profession Trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4300, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_INSCRIPTION), 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 25);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 26);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 27);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 8: //Hunter
                    {
                        Plr->Gossip_SendPOI(356);
                        SendQuickMenu(4293);
                    }
                    break;

                case 9: //Mage
                    {
                        Plr->Gossip_SendPOI(357);
                        SendQuickMenu(4294);
                    }
                    break;

                case 10: //Paladin
                    {
                        Plr->Gossip_SendPOI(358);
                        SendQuickMenu(4295);
                    }
                    break;

                case 11: //Priest
                    {
                        Plr->Gossip_SendPOI(359);
                        SendQuickMenu(4296);
                    }
                    break;

                case 12: //Rogue
                    {
                        Plr->Gossip_SendPOI(360);
                        SendQuickMenu(4297);
                    }
                    break;

                case 13: //Warlock
                    {
                        Plr->Gossip_SendPOI(361);
                        SendQuickMenu(4298);
                    }
                    break;

                case 14: //Warrior
                    {
                        Plr->Gossip_SendPOI(362);
                        SendQuickMenu(4299);
                    }
                    break;

                case 15: //Alchemy
                    {
                        SendQuickMenu(4301);
                    }
                    break;

                case 16: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(363);
                        SendQuickMenu(4302);
                    }
                    break;

                case 17: //Cooking
                    {
                        Plr->Gossip_SendPOI(364);
                        SendQuickMenu(4303);
                    }
                    break;

                case 18: //Enchanting
                    {
                        SendQuickMenu(4304);
                    }
                    break;

                case 19: //Engineering
                    {
                        SendQuickMenu(4305);
                    }
                    break;

                case 20: //First Aid
                    {
                        Plr->Gossip_SendPOI(365);
                        SendQuickMenu(4306);
                    }
                    break;

                case 21: //Fishing
                    {
                        Plr->Gossip_SendPOI(366);
                        SendQuickMenu(4307);
                    }
                    break;

                case 22: //Herbalism
                    {
                        SendQuickMenu(4308);
                    }
                    break;

                case 23: //Inscription
                    {
                        Plr->Gossip_SendPOI(367);
                        SendQuickMenu(13884);
                    }
                    break;

                case 24: //Leatherworking
                    {
                        SendQuickMenu(4310);
                    }
                    break;

                case 25: //Mining
                    {
                        Plr->Gossip_SendPOI(368);
                        SendQuickMenu(4311);
                    }
                    break;

                case 26: //Skinning
                    {
                        SendQuickMenu(4312);
                    }
                    break;

                case 27: //Tailoring
                    {
                        SendQuickMenu(4313);
                    }
                    break;
            }
        }
};

class FalconwingGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2593, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_BAT_HANDLER), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD2_M), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE2_M), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS2_T), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION2_T), 6);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     //Bat Handler
                    {
                        SendQuickMenu(2593);
                        Plr->Gossip_SendPOI(369);
                    }
                    break;

                case 2:     //Guild Master
                    {
                        SendQuickMenu(2593);
                    }
                    break;

                case 3:     //The Inn
                    {
                        SendQuickMenu(2593);
                        Plr->Gossip_SendPOI(370);
                    }
                    break;

                case 4:     //Stable Master
                    {
                        SendQuickMenu(2593);
                        Plr->Gossip_SendPOI(371);
                    }
                    break;

                case 5:     //Class Trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4292, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_DRUID), 7);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HUNTER), 8);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MAGE), 9);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PALADIN), 10);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 11);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ROGUE), 12);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARLOCK), 13);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 6:     // Profession Trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2593, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 14);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_JUWELCRAFTING), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 25);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 26);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 7: //Druid
                    {
                        SendQuickMenu(2593);
                    }
                    break;

                case 8: //Hunter
                    {
                        Plr->Gossip_SendPOI(372);
                        SendQuickMenu(2593);
                    }
                    break;

                case 9: //Mage
                    {
                        Plr->Gossip_SendPOI(373);
                        SendQuickMenu(2593);
                    }
                    break;

                case 10: //Paladin <-- Needs to change flag to other sign (don't know how to describe it)
                    {
                        Plr->Gossip_SendPOI(374);
                        SendQuickMenu(2593);
                    }
                    break;

                case 11: //Priest
                    {
                        Plr->Gossip_SendPOI(375);
                        SendQuickMenu(2593);
                    }
                    break;

                case 12: //Rogue
                    {
                        Plr->Gossip_SendPOI(376);
                        SendQuickMenu(2593);
                    }
                    break;

                case 13: //Warlock
                    {
                        Plr->Gossip_SendPOI(377);
                        SendQuickMenu(2593);
                    }
                    break;

                case 14: //Alchemy
                    {
                        Plr->Gossip_SendPOI(378);
                        SendQuickMenu(2593);
                    }
                    break;

                case 15: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(379);
                        SendQuickMenu(2593);
                    }
                    break;

                case 16: //Cooking
                    {
                        Plr->Gossip_SendPOI(380);
                        SendQuickMenu(2593);
                    }
                    break;

                case 17: //Enchanting
                    {
                        Plr->Gossip_SendPOI(381);
                        SendQuickMenu(2593);
                    }
                    break;

                case 18: //Engineering
                    {
                        SendQuickMenu(2593);
                    }
                    break;

                case 19: //First Aid
                    {
                        Plr->Gossip_SendPOI(382);
                        SendQuickMenu(2593);
                    }
                    break;

                case 20: //Fishing
                    {
                        SendQuickMenu(2593);
                    }
                    break;

                case 21: //Herbalism
                    {
                        Plr->Gossip_SendPOI(383);
                        SendQuickMenu(2593);
                    }
                    break;

                case 22: //Jewelcrafting
                    {
                        Plr->Gossip_SendPOI(384);
                        SendQuickMenu(2593);
                    }
                    break;

                case 23: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(385);
                        SendQuickMenu(2593);
                    }
                    break;

                case 24: //Mining
                    {
                        SendQuickMenu(2593);
                    }
                    break;

                case 25: //Skinning
                    {
                        Plr->Gossip_SendPOI(386);
                        SendQuickMenu(2593);
                    }
                    break;

                case 26: //Tailoring
                    {
                        Plr->Gossip_SendPOI(387);
                        SendQuickMenu(2593);
                    }
                    break;
            }
        }
};

class AzureWatchGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10066, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK2), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_HYPPOGRYPH_M), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD2_M), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN2), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_STABLE), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_CLASS2_T), 6);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION2_T), 7);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     //Bank
                    {
                        SendQuickMenu(10067);
                    }
                    break;

                case 2:     //Hippogryph Master
                    {
                        SendQuickMenu(10071);
                    }
                    break;

                case 3:     //Guild Master
                    {
                        SendQuickMenu(10073);
                    }
                    break;

                case 4:     //Inn
                    {
                        SendQuickMenu(10074);
                        Plr->Gossip_SendPOI(388);
                    }
                    break;

                case 5:     //Stable Master
                    {
                        SendQuickMenu(10075);
                        Plr->Gossip_SendPOI(389);
                    }
                    break;

                case 6:     //Class Trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10076, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_DRUID), 8);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HUNTER), 9);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MAGE), 10);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PALADIN), 11);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PRIEST), 12);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SHAMAN), 13);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_WARRIOR), 14);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 7:     //Profession Trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10087, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_INSCRIPTION), 23);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_JUWELCRAFTING), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 25);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 26);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 27);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 28);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 8: //Druid
                    {
                        SendQuickMenu(10077);
                    }
                    break;

                case 9: //Hunter
                    {
                        Plr->Gossip_SendPOI(390);
                        SendQuickMenu(10078);
                    }
                    break;

                case 10: //Mage
                    {
                        Plr->Gossip_SendPOI(391);
                        SendQuickMenu(10081);
                    }
                    break;

                case 11: //Paladin
                    {
                        Plr->Gossip_SendPOI(392);
                        SendQuickMenu(10083);
                    }
                    break;

                case 12: //Priest
                    {
                        Plr->Gossip_SendPOI(393);
                        SendQuickMenu(10084);
                    }
                    break;

                case 13: //Shaman
                    {
                        Plr->Gossip_SendPOI(394);
                        SendQuickMenu(10085);
                    }
                    break;

                case 14: //Warrior
                    {
                        Plr->Gossip_SendPOI(395);
                        SendQuickMenu(10086);
                    }
                    break;

                case 15: //Alchemy
                    {
                        Plr->Gossip_SendPOI(396);
                        SendQuickMenu(10088);
                    }
                    break;

                case 16: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(397);
                        SendQuickMenu(10089);
                    }
                    break;

                case 17: //Cooking
                    {
                        Plr->Gossip_SendPOI(398);
                        SendQuickMenu(10090);
                    }
                    break;

                case 18: //Enchanting
                    {
                        SendQuickMenu(10091);
                    }
                    break;

                case 19: //Engineering
                    {
                        Plr->Gossip_SendPOI(399);
                        SendQuickMenu(10092);
                    }
                    break;

                case 20: //First Aid
                    {
                        Plr->Gossip_SendPOI(400);
                        SendQuickMenu(10093);
                    }
                    break;

                case 21: //Fishing
                    {
                        Plr->Gossip_SendPOI(401);
                        SendQuickMenu(10094);
                    }
                    break;

                case 22: //Herbalism
                    {
                        Plr->Gossip_SendPOI(402);
                        SendQuickMenu(10095);
                    }
                    break;

                case 23: //Inscription
                    {
                        Plr->Gossip_SendPOI(403);
                        SendQuickMenu(13887);
                    }
                    break;

                case 24: //Jewelcrafting
                    {
                        SendQuickMenu(10100);
                    }
                    break;

                case 25: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(404);
                        SendQuickMenu(10096);
                    }
                    break;

                case 26: //Mining
                    {
                        Plr->Gossip_SendPOI(405);
                        SendQuickMenu(10097);
                    }
                    break;

                case 27: //Skinning
                    {
                        Plr->Gossip_SendPOI(406);
                        SendQuickMenu(10098);
                    }
                    break;

                case 28: //Tailoring
                    {
                        Plr->Gossip_SendPOI(407);
                        SendQuickMenu(10099);
                    }
                    break;
            }
        }
};

/*****************************************************************************************/
/* Shattrath Guards   original structure by AeThIs. Translated, updated and  by Pepsi1x1 */
/*****************************************************************************************/

class ShattrathGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10524, plr);

            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_WORLDS_END_TAV), 1);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BANK2), 2);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_INN2), 3);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_FLIGHT_M), 4);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MAILBOX), 5);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE2_M), 6);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THE_BATTLE3_M), 7);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION2_T), 8);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MANA_LOOM), 9);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_ALCHEMIE_LAB), 10);
            Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_GEM_MERCHANT), 11);

            Menu->SendTo(plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0:     // Return to start
                    GossipHello(pObject, Plr);
                    break;

                case 1:     // World's End Tavern
                    {
                        SendQuickMenu(10394);
                        Plr->Gossip_SendPOI(408);
                    }
                    break;

                case 2:     // Shattrath Banks
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10395, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_ALDOR_BANK), 12);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SCYERS_BANK), 13);

                        Menu->SendTo(Plr);
                    }
                    break;


                case 3:     // Inn's
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10398, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_ALDOR_INN), 14);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SCYERS_INN), 15);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 4:     // Gryphon Master
                    {
                        SendQuickMenu(10402);
                        Plr->Gossip_SendPOI(409);
                    }
                    break;

                case 5:     // Mailboxes
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10403, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_ALDOR_INN), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SCYERS_INN), 17);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_ALDOR_BANK), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SCYERS_BANK), 19);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 6:     // Stable Masters
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10404, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_ALDOR_STABLE), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SVYERS_STABLE), 21);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 7:     // Battlemasters
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10405, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_A_BATTLEMASTERS), 22);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_H_ARENA_BATTLEMASTERS), 23);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 8:     // Proffesion Trainers
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10391, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 25);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 26);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 27);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 28);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_JUWELCRAFTING), 29);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 30);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 31);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 9:     // Mana Loom
                    {
                        SendQuickMenu(10408);
                        Plr->Gossip_SendPOI(410);
                    }
                    break;

                case 10:    // Alchemy Lab
                    {
                        SendQuickMenu(10409);
                        Plr->Gossip_SendPOI(411);
                    }
                    break;

                case 11:    // Gem Merchants
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10410, Plr);

                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_ALDOR_GEM_MERCHANT), 32);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SCYER_GEM_MERCHANT), 33);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 12: //Aldor Bank
                    {
                        Plr->Gossip_SendPOI(412);
                        SendQuickMenu(10396);
                    }
                    break;

                case 13: //Scryers Bank
                    {
                        Plr->Gossip_SendPOI(413);
                        SendQuickMenu(10397);
                    }
                    break;

                case 14: //Aldor Inn
                    {
                        Plr->Gossip_SendPOI(414);
                        SendQuickMenu(10399);
                    }
                    break;

                case 15: //Scryers Inn
                    {
                        Plr->Gossip_SendPOI(415);
                        SendQuickMenu(10401);
                    }
                    break;

                case 16: //Aldor Inn
                    {
                        Plr->Gossip_SendPOI(416);
                        SendQuickMenu(10399);
                    }
                    break;

                case 17: //Scryers Bank
                    {
                        Plr->Gossip_SendPOI(417);
                        SendQuickMenu(10397);
                    }
                    break;

                case 18: //Aldor Bank
                    {
                        Plr->Gossip_SendPOI(418);
                        SendQuickMenu(10396);
                    }
                    break;

                case 19: //Scryers Inn
                    {
                        Plr->Gossip_SendPOI(419);
                        SendQuickMenu(10401);
                    }
                    break;

                case 20: //Aldor Stable Master
                    {
                        Plr->Gossip_SendPOI(420);
                        SendQuickMenu(10399);
                    }
                    break;

                case 21: //Scryers Stable Master
                    {
                        Plr->Gossip_SendPOI(421);
                        SendQuickMenu(10401);
                    }
                    break;

                case 22: //Alliance Battlemaster
                    {
                        Plr->Gossip_SendPOI(422);
                        SendQuickMenu(10406);
                    }
                    break;

                case 23: //Horde Battle Master and Arena Battle Master
                    {
                        Plr->Gossip_SendPOI(423);
                        SendQuickMenu(10407);
                    }
                    break;

                case 24: //Alchemy
                    {
                        Plr->Gossip_SendPOI(424);
                        SendQuickMenu(10413);
                    }
                    break;

                case 25: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(425);
                        SendQuickMenu(10400);
                    }
                    break;

                case 26: //Cooking
                    {
                        Plr->Gossip_SendPOI(426);
                        SendQuickMenu(10414);
                    }
                    break;

                case 27: //Enchanting
                    {
                        Plr->Gossip_SendPOI(427);
                        SendQuickMenu(10415);
                    }
                    break;

                case 28: //First Aid
                    {
                        Plr->Gossip_SendPOI(428);
                        SendQuickMenu(10416);
                    }
                    break;

                case 29: //Jewelcrafting
                    {
                        Plr->Gossip_SendPOI(429);
                        SendQuickMenu(10417);
                    }
                    break;

                case 30: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(430);
                        SendQuickMenu(10418);
                    }
                    break;

                case 31: //Skinning
                    {
                        Plr->Gossip_SendPOI(431);
                        SendQuickMenu(10419);
                    }
                    break;

                case 32: //Aldor gem merchant
                    {
                        Plr->Gossip_SendPOI(432);
                        SendQuickMenu(10411);
                    }
                    break;

                case 33: //Scryers gem merchant
                    {
                        Plr->Gossip_SendPOI(433);
                        SendQuickMenu(10412);
                    }
                    break;

            }
        }
};

class DalaranGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* Plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50000, Plr);

            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_ARENA),1);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BANK),2);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BANK2),3);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BARBER),4);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BATTLE2_M),5);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_CAPITAL_PORTS),6);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_FLIGHT_M),7);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD2_M),8);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_INN2),9);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_LOCKSMITH),77);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_MAILBOX),10);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_POI), 11);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE2_M),12);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_A_CLASS3_T),13);
            Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_VENDORS),14);

            Menu->SendTo(Plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
                case 0: 
                    GossipHello(pObject, Plr);
                    break;

                case 1:        // Arena
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 13976, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_EAST_SEW_ENTR), 15);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_WEST_SEW_ENTR), 16);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_WELL_ENTR), 17);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 2:        // Auction House
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14010, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_A_QUART), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_QUART), 19);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 3:        // Bank
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14007, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_NORTH_BANK), 20);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SOUTH_BANK), 21);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SEWERS), 22); // Sewers 1

                        Menu->SendTo(Plr);
                    }
                    break;

                case 4:        // Barber
                    {
                        Plr->Gossip_SendPOI(434);
                        SendQuickMenu(14003);
                    }
                    break;

                case 5:        // Battlemasters
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 13977, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_A_QUART), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_QUART), 19);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 6:        // Capital Portals
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 13977, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_A_QUART), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_QUART), 19);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 7:        // Flight Master
                    {
                        Plr->Gossip_SendPOI(435);
                        SendQuickMenu(10086);
                    }
                    break;

                case 8:        // Guild Master
                    {
                        Plr->Gossip_SendPOI(436);
                        SendQuickMenu(10095);
                    }
                    break;

                case 9:        // Inn
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14002, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_A_INN), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_INN), 25);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SEWERS), 26); // Sewers 2

                        Menu->SendTo(Plr);
                    }
                    break;

                case 10:    // Mailbox
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10090, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_INN2), 9);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BANK2), 3);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_KRASUS_LAND), 74);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 11:    // Points of Interest
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10056, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_A_QUART), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_QUART), 19);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_VIOLET_CITADEL), 27);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_VIOLET_HOLD), 28);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SEWERS), 22); // Sewers 1
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_TRADE_DISTRICT), 29);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_KRASUS_LAND), 74);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_ANTONIDAS_MEMORIAL), 30);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_RUNEWEAV_SQUARE), 31);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_EVENTIDE), 32);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_CEMETARY), 33);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_LEXICON_OF_POWER), 34);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 12:    // Stable Master
                    {
                        Plr->Gossip_SendPOI(437);
                        SendQuickMenu(10083);
                    }
                    break;

                case 13:    // Trainers
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10082, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_A_CLASS2_T), 35);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_CW_FLYING), 76);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PORTAL), 36);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION2_T), 37);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 14:    // Vendors
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10173, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_ARMOR), 38);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_CLOTHING), 39);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_EMBLEM_GEAR), 40);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_FLOWERS), 41);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_FRUIT), 42);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_GENERAL_GOODS), 43);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_JEWELRY), 44);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_PET_SUBS_EX_MOUNTS), 45);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_PIE_PASTRY_CAKES), 46);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_REAGENTS_MAG_GOODS), 47);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_TOYS), 48);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_TRADE_SUP), 43);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_TRINKETS_REL_OFF), 49);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_WEAPONS), 50);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_WINE_CHEESE), 51);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 15:    // Eastern Sewer Entrance
                    {
                        Plr->Gossip_SendPOI(438);
                        SendQuickMenu(13961);
                    }
                    break;

                case 16:    // Western Sewer Entrance
                    {
                        Plr->Gossip_SendPOI(439);
                        SendQuickMenu(13960);
                    }
                    break;

                case 17:    // Well Entrance
                    {
                        Plr->Gossip_SendPOI(440);
                        SendQuickMenu(13969);
                    }
                    break;

                case 18:    // The Alliance Quarter
                    {
                        Plr->Gossip_SendPOI(441);
                        SendQuickMenu(13973);
                    }
                    break;

                case 19:    // The Horde Quarter
                    {
                        Plr->Gossip_SendPOI(442);
                        SendQuickMenu(13972);
                    }
                    break;

                case 20:    // Northern Bank
                    {
                        Plr->Gossip_SendPOI(443);
                        SendQuickMenu(14005);
                    }
                    break;

                case 21:    // Southern Bank
                    {
                        Plr->Gossip_SendPOI(444);
                        SendQuickMenu(14006);
                    }
                    break;

                case 22:    // Sewers 1
                    {
                        Plr->Gossip_SendPOI(445);
                        SendQuickMenu(13974);
                    }
                    break;

                case 24:    // Alliance Inn
                    {
                        Plr->Gossip_SendPOI(446);
                        SendQuickMenu(13992);
                    }
                    break;

                case 25:    // Horde Inn
                    {
                        Plr->Gossip_SendPOI(447);
                        SendQuickMenu(13993);
                    }
                    break;

                case 26:    // Sewers 2
                    {
                        Plr->Gossip_SendPOI(448);
                        SendQuickMenu(13974);
                    }
                    break;

                case 27:    // The Violet Citadel
                    {
                        Plr->Gossip_SendPOI(449);
                        SendQuickMenu(13971);
                    }
                    break;

                case 28:    // The Violet Hold
                    {
                        Plr->Gossip_SendPOI(450);
                        SendQuickMenu(13970);
                    }
                    break;

                case 29:    // Trade District
                    {
                        Plr->Gossip_SendPOI(451);
                        SendQuickMenu(13980);
                    }
                    break;

                case 30:    // Antonidas Memorial
                    {
                        Plr->Gossip_SendPOI(452);
                        SendQuickMenu(13968);
                    }
                    break;

                case 31:    // Runeweaver Square
                    {
                        Plr->Gossip_SendPOI(453);
                        SendQuickMenu(13967);
                    }
                    break;

                case 32:    // The Eventide
                    {
                        Plr->Gossip_SendPOI(454);
                        SendQuickMenu(13966);
                    }
                    break;

                case 33:    // Cemetary
                    {
                        Plr->Gossip_SendPOI(455);
                        SendQuickMenu(13965);
                    }
                    break;

                case 34:    // Lexicon of Power
                    {
                        Plr->Gossip_SendPOI(456);
                        SendQuickMenu(14174);
                    }
                    break;

                case 35:    // Class Trainers
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14018, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_A_QUART), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_QUART), 19);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 36:    // Portal Trainer
                    {
                        Plr->Gossip_SendPOI(457);
                        SendQuickMenu(13999);
                    }
                    break;

                case 37:    // Profession Trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 13996, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 52);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 53);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 54);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 55);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING), 56);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 57);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 58);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 59);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_INSCRIPTION), 60);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_JUWELCRAFTING), 61);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 62);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 63);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 64);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 65);

                        Menu->SendTo(Plr);
                    }
                    break;

                case 38:    // Armor
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14117, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_CLOTH_ARMOR), 66);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_LEATHER_ARMOR), 67);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_MAIL_ARMOR), 68);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_PLATE_ARMOR), 69);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SHIELDS), 70);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 39:    // Clothing
                    {
                        Plr->Gossip_SendPOI(458);
                        SendQuickMenu(14112);
                    }
                    break;

                case 40:    // Emblem Gear
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14108, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_A_QUART), 18);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_QUART), 19);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 41:    // Flowers
                    {
                        Plr->Gossip_SendPOI(459);
                        SendQuickMenu(10159);
                    }
                    break;

                case 42:    // Fruit
                    {
                        Plr->Gossip_SendPOI(460);
                        SendQuickMenu(14106);
                    }
                    break;

                case 43:    // General Goods
                    {
                        Plr->Gossip_SendPOI(461);
                        SendQuickMenu(14105);
                    }
                    break;

                case 44:    // Jewelry
                    {
                        Plr->Gossip_SendPOI(462);
                        SendQuickMenu(13984);
                    }
                    break;

                case 45:    // Pet Supplies & Exotic Mounts
                    {
                        Plr->Gossip_SendPOI(463);
                        SendQuickMenu(14103);
                    }
                    break;

                case 46:    // Pie, Pastry & Cakes
                    {
                        Plr->Gossip_SendPOI(464);
                        SendQuickMenu(14102);
                    }
                    break;

                case 47:    // Reagents & Magical Goods
                    {
                        Plr->Gossip_SendPOI(465);
                        SendQuickMenu(14104);
                    }
                    break;

                case 48:    // Toys
                    {
                        Plr->Gossip_SendPOI(466);
                        SendQuickMenu(14100);
                    }
                    break;

                case 49:    // Trinkets. Relics & Off-hand items
                    {
                        Plr->Gossip_SendPOI(467);
                        SendQuickMenu(14110);
                    }
                    break;

                case 50:    // Weapons
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14113, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_MELEE_WEAPONS), 71);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_RANGE_THROW_WEAPONS), 72);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_STAVES_WANDS), 73);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 51:    // Wine & Cheese
                    {
                        Plr->Gossip_SendPOI(468);
                        SendQuickMenu(14095);
                    }
                    break;

                case 52:    // Alchemy
                    {
                        Plr->Gossip_SendPOI(469);
                        SendQuickMenu(13995);
                    }
                    break;

                case 53:    // Blacksmithing
                    {
                        Plr->Gossip_SendPOI(470);
                        SendQuickMenu(13994);
                    }
                    break;

                case 54:    // Cooking
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 13991, Plr);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_A_INN), 24);
                        Menu->AddItem(ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_INN), 25);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 55:    // Enchanting
                    {
                        Plr->Gossip_SendPOI(471);
                        SendQuickMenu(13990);
                    }
                    break;

                case 56:    // Engineering
                    {
                        Plr->Gossip_SendPOI(472);
                        SendQuickMenu(13989);
                    }
                    break;

                case 57:    // First Aid
                    {
                        Plr->Gossip_SendPOI(473);
                        SendQuickMenu(13988);
                    }
                    break;

                case 58:    // Fishing
                    {
                        Plr->Gossip_SendPOI(474);
                        SendQuickMenu(13987);
                    }
                    break;

                case 59:    // Herbalism
                    {
                        Plr->Gossip_SendPOI(475);
                        SendQuickMenu(13986);
                    }
                    break;

                case 60:    // Inscription
                    {
                        Plr->Gossip_SendPOI(476);
                        SendQuickMenu(13985);
                    }
                    break;

                case 61:    // Jewelcrafting
                    {
                        Plr->Gossip_SendPOI(477);
                        SendQuickMenu(13984);
                    }
                    break;

                case 62:    // Leatherworking
                    {
                        Plr->Gossip_SendPOI(478);
                        SendQuickMenu(13982);
                    }
                    break;

                case 63:    // Mining
                    {
                        Plr->Gossip_SendPOI(479);
                        SendQuickMenu(13983);
                    }
                    break;

                case 64:    // Skinning
                    {
                        Plr->Gossip_SendPOI(480);
                        SendQuickMenu(13982);
                    }
                    break;

                case 65:    // Tailoring
                    {
                        Plr->Gossip_SendPOI(481);
                        SendQuickMenu(13981);
                    }
                    break;

                case 66:    // Cloth Armor
                    {
                        Plr->Gossip_SendPOI(482);
                        SendQuickMenu(14112);
                    }
                    break;

                case 67:    // Leather Armor
                    {
                        Plr->Gossip_SendPOI(483);
                        SendQuickMenu(14111);
                    }
                    break;

                case 68:    // Mail Armor
                    {
                        Plr->Gossip_SendPOI(484);
                        SendQuickMenu(14111);
                    }
                    break;

                case 69:    // Plate Armor
                    {
                        Plr->Gossip_SendPOI(485);
                        SendQuickMenu(14109);
                    }
                    break;

                case 70:    // Shields
                    {
                        Plr->Gossip_SendPOI(486);
                        SendQuickMenu(14109);
                    }
                    break;

                case 71:    // Melee Weapons
                    {
                        Plr->Gossip_SendPOI(487);
                        SendQuickMenu(14098);
                    }
                    break;

                case 72:    // Ranged & Thrown Weapons
                    {
                        Plr->Gossip_SendPOI(488);
                        SendQuickMenu(14097);
                    }
                    break;

                case 73:    // Staves & Wands
                    {
                        Plr->Gossip_SendPOI(489);
                        SendQuickMenu(14096);
                    }
                    break;

                case 74:    // Krasu's Landing
                    {
                        Plr->Gossip_SendPOI(490);
                        SendQuickMenu(14009);
                    }
                    break;

                case 75:    // Trinkets, Relics & Off-hand Items
                    {
                        Plr->Gossip_SendPOI(491);
                        SendQuickMenu(14110);
                    }
                    break;

                case 76:    // Cold weather flying trainer
                    {
                        Plr->Gossip_SendPOI(492);
                        SendQuickMenu(60059);
                    }
                    break;

                case 77:    // Locksmith
                    {
                        Plr->Gossip_SendPOI(493);
                        SendQuickMenu(14004);
                    }
                    break;
            }
        }
};

void SetupGuardGossip(ScriptMgr* mgr)
{
    /* Guard List */
    mgr->register_gossip_script(1423, new GoldshireGuard);              // Stormwind Guard
    mgr->register_gossip_script(68, new StormwindGuard);                // Stormwind City Guard
    mgr->register_gossip_script(1976, new StormwindGuard);              // Stormwind City Patroller
    mgr->register_gossip_script(29712, new StormwindGuard);             // Stormwind Harbor Guard
    mgr->register_gossip_script(4262, new DarnassusGuard);              // Darnassus Sentinel
    mgr->register_gossip_script(5624, new UndercityGuard);              // Undercity Guardian
    mgr->register_gossip_script(36213, new UndercityGuardOverseer);     // Kor'kron Overseer
    mgr->register_gossip_script(3571, new TeldrassilGuard);             // Teldrassil Sentinel
    mgr->register_gossip_script(16222, new SilvermoonGuard);            // Silvermoon City Guardian
    mgr->register_gossip_script(16733, new ExodarGuard);                // Exodar Peacekeeper
    mgr->register_gossip_script(20674, new ExodarGuard);                // Shield of Velen
    mgr->register_gossip_script(3296, new OrgrimmarGuard);              // Orgrimmar Grunt
    mgr->register_gossip_script(3084, new ThunderbluffGuard);           // Bluffwatcher
    mgr->register_gossip_script(3222, new BloodhoofGuard);              // Brave Wildrunner
    mgr->register_gossip_script(3224, new BloodhoofGuard);              // Brave Cloudmane
    mgr->register_gossip_script(3220, new BloodhoofGuard);              // Brave Darksky
    mgr->register_gossip_script(3219, new BloodhoofGuard);              // Brave Leaping Deer
    mgr->register_gossip_script(3217, new BloodhoofGuard);              // Brave Dawneagle
    mgr->register_gossip_script(3215, new BloodhoofGuard);              // Brave Strongbash
    mgr->register_gossip_script(3218, new BloodhoofGuard);              // Brave Swiftwind
    mgr->register_gossip_script(3221, new BloodhoofGuard);              // Brave Rockhorn
    mgr->register_gossip_script(3223, new BloodhoofGuard);              // Brave Rainchaser
    mgr->register_gossip_script(3212, new BloodhoofGuard);              // Brave Ironhorn
    mgr->register_gossip_script(5953, new RazorHillGuard);              // Razor Hill Grunt
    mgr->register_gossip_script(5725, new BrillGuard);                  // Deathguard Lundmark
    mgr->register_gossip_script(1738, new BrillGuard);                  // Deathguard Terrence
    mgr->register_gossip_script(1652, new BrillGuard);                  // Deathguard Burgess
    mgr->register_gossip_script(1746, new BrillGuard);                  // Deathguard Cyrus
    mgr->register_gossip_script(1745, new BrillGuard);                  // Deathguard Morris
    mgr->register_gossip_script(1743, new BrillGuard);                  // Deathguard Lawrence
    mgr->register_gossip_script(1744, new BrillGuard);                  // Deathguard Mort
    mgr->register_gossip_script(1496, new BrillGuard);                  // Deathguard Dillinger
    mgr->register_gossip_script(1742, new BrillGuard);                  // Deathguard Bartholomew
    mgr->register_gossip_script(5595, new IronforgeGuard);              // Ironforge Guard
    mgr->register_gossip_script(727, new KharanosGuard);                // Ironforge Mountaineer
    mgr->register_gossip_script(16221, new FalconwingGuard);            // Silvermoon Guardian
    mgr->register_gossip_script(18038, new AzureWatchGuard);            // Azuremyst Peacekeeper
    mgr->register_gossip_script(19687, new ShattrathGuard);             // Shattrath City Guard
    mgr->register_gossip_script(18568, new ShattrathGuard);             // Shattrath City Guard Aruspice
    mgr->register_gossip_script(18549, new ShattrathGuard);             // Shattrath City Guard

    //Dalaran guards (updated to "new" gossip function)
    mgr->register_gossip_script(32675, new DalaranGuard);
    mgr->register_gossip_script(32676, new DalaranGuard);
    mgr->register_gossip_script(32677, new DalaranGuard);
    mgr->register_gossip_script(32678, new DalaranGuard);
    mgr->register_gossip_script(32679, new DalaranGuard);
    mgr->register_gossip_script(32680, new DalaranGuard);
    mgr->register_gossip_script(32681, new DalaranGuard);
    mgr->register_gossip_script(32683, new DalaranGuard);
    mgr->register_gossip_script(32684, new DalaranGuard);
    mgr->register_gossip_script(32685, new DalaranGuard);
    mgr->register_gossip_script(32686, new DalaranGuard);
    mgr->register_gossip_script(32687, new DalaranGuard);
    mgr->register_gossip_script(32688, new DalaranGuard);
    mgr->register_gossip_script(32689, new DalaranGuard);
    mgr->register_gossip_script(32690, new DalaranGuard);
    mgr->register_gossip_script(32691, new DalaranGuard);
    mgr->register_gossip_script(32692, new DalaranGuard);
    mgr->register_gossip_script(32693, new DalaranGuard);

}
