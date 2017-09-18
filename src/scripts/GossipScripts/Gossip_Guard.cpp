/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Management/Gossip/Gossip.h"
#include "Setup.h"
#include "Management/Gossip/GossipMenu.hpp"
#include "Objects/ObjectMgr.h"


class StormwindGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 114;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class DarnassusGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 122;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class UndercityGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 142;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class UndercityGuardOverseer : public GossipScript
{
public:

    uint32_t definedGossipMenu = 163;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class ThunderbluffGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 152;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class GoldshireGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 132;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class TeldrassilGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 172;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class SilvermoonGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 180;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class ExodarGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 191;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class OrgrimmarGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 724;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class BloodhoofGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 751;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class RazorHillGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 989;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class BrillGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 1003;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class IronforgeGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 1012;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class KharanosGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 1035;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class FalconwingGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 1047;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class AzureWatchGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 1058;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

class ShattrathGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 1068;
    void GossipHello(Object* pObject, Player* plr)
    {
        objmgr.createGuardGossipMenuForPlayer(pObject->GetGUID(), definedGossipMenu, plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        if (IntId > 0)
        {
            objmgr.createGuardGossipOptionAndSubMenu(pObject->GetGUID(), Plr, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(pObject, Plr);
        }
    }
};

/**
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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
        Menu->AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_LOOKING_SOMTH_ELSE), 0); \
        Menu->SendTo(Plr);
#endif


class DalaranGuard : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* Plr)
    {
        GossipMenu* Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50000, Plr);

        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_ARENA), 1);
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BANK), 2);
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BANK2), 3);
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BARBER), 4);
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BATTLE2_M), 5);
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_CAPITAL_PORTS), 6);
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_FLIGHT_M), 7);
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_GUILD2_M), 8);
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_INN2), 9);
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_LOCKSMITH), 77);
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_MAILBOX), 10);
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_POI), 11);
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_STABLE2_M), 12);
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_A_CLASS3_T), 13);
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_VENDORS), 14);

        Menu->SendTo(Plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        GossipMenu* Menu;
        switch (IntId)
        {
            case 0:
                GossipHello(pObject, Plr);
                break;

            case 1:        // Arena
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 13976, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_EAST_SEW_ENTR), 15);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_WEST_SEW_ENTR), 16);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_WELL_ENTR), 17);

                Menu->SendTo(Plr);
            }
            break;

            case 2:        // Auction House
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14010, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_A_QUART), 18);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_QUART), 19);
                Menu->SendTo(Plr);
            }
            break;

            case 3:        // Bank
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14007, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_NORTH_BANK), 20);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SOUTH_BANK), 21);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SEWERS), 22); // Sewers 1

                Menu->SendTo(Plr);
            }
            break;

            case 4:        // Barber
            {
                Plr->Gossip_SendSQLPOI(434);
                SendQuickMenu(14003);
            }
            break;

            case 5:        // Battlemasters
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 13977, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_A_QUART), 18);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_QUART), 19);

                Menu->SendTo(Plr);
            }
            break;

            case 6:        // Capital Portals
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 13977, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_A_QUART), 18);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_QUART), 19);

                Menu->SendTo(Plr);
            }
            break;

            case 7:        // Flight Master
            {
                Plr->Gossip_SendSQLPOI(435);
                SendQuickMenu(10086);
            }
            break;

            case 8:        // Guild Master
            {
                Plr->Gossip_SendSQLPOI(436);
                SendQuickMenu(10095);
            }
            break;

            case 9:        // Inn
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14002, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_A_INN), 24);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_INN), 25);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SEWERS), 26); // Sewers 2

                Menu->SendTo(Plr);
            }
            break;

            case 10:    // Mailbox
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10090, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_INN2), 9);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_BANK2), 3);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_KRASUS_LAND), 74);

                Menu->SendTo(Plr);
            }
            break;

            case 11:    // Points of Interest
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10056, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_A_QUART), 18);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_QUART), 19);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_VIOLET_CITADEL), 27);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_VIOLET_HOLD), 28);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SEWERS), 22); // Sewers 1
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_TRADE_DISTRICT), 29);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_KRASUS_LAND), 74);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_ANTONIDAS_MEMORIAL), 30);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_RUNEWEAV_SQUARE), 31);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_EVENTIDE), 32);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_CEMETARY), 33);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_LEXICON_OF_POWER), 34);

                Menu->SendTo(Plr);
            }
            break;

            case 12:    // Stable Master
            {
                Plr->Gossip_SendSQLPOI(437);
                SendQuickMenu(10083);
            }
            break;

            case 13:    // Trainers
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10082, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_A_CLASS2_T), 35);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_CW_FLYING), 76);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_PORTAL), 36);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_A_PROFESSION2_T), 37);

                Menu->SendTo(Plr);
            }
            break;

            case 14:    // Vendors
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10173, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_ARMOR), 38);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_CLOTHING), 39);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_EMBLEM_GEAR), 40);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_FLOWERS), 41);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_FRUIT), 42);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_GENERAL_GOODS), 43);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_JEWELRY), 44);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_PET_SUBS_EX_MOUNTS), 45);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_PIE_PASTRY_CAKES), 46);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_REAGENTS_MAG_GOODS), 47);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_TOYS), 48);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_TRADE_SUP), 43);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_TRINKETS_REL_OFF), 49);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_WEAPONS), 50);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_WINE_CHEESE), 51);

                Menu->SendTo(Plr);
            }
            break;

            case 15:    // Eastern Sewer Entrance
            {
                Plr->Gossip_SendSQLPOI(438);
                SendQuickMenu(13961);
            }
            break;

            case 16:    // Western Sewer Entrance
            {
                Plr->Gossip_SendSQLPOI(439);
                SendQuickMenu(13960);
            }
            break;

            case 17:    // Well Entrance
            {
                Plr->Gossip_SendSQLPOI(440);
                SendQuickMenu(13969);
            }
            break;

            case 18:    // The Alliance Quarter
            {
                Plr->Gossip_SendSQLPOI(441);
                SendQuickMenu(13973);
            }
            break;

            case 19:    // The Horde Quarter
            {
                Plr->Gossip_SendSQLPOI(442);
                SendQuickMenu(13972);
            }
            break;

            case 20:    // Northern Bank
            {
                Plr->Gossip_SendSQLPOI(443);
                SendQuickMenu(14005);
            }
            break;

            case 21:    // Southern Bank
            {
                Plr->Gossip_SendSQLPOI(444);
                SendQuickMenu(14006);
            }
            break;

            case 22:    // Sewers 1
            {
                Plr->Gossip_SendSQLPOI(445);
                SendQuickMenu(13974);
            }
            break;

            case 24:    // Alliance Inn
            {
                Plr->Gossip_SendSQLPOI(446);
                SendQuickMenu(13992);
            }
            break;

            case 25:    // Horde Inn
            {
                Plr->Gossip_SendSQLPOI(447);
                SendQuickMenu(13993);
            }
            break;

            case 26:    // Sewers 2
            {
                Plr->Gossip_SendSQLPOI(448);
                SendQuickMenu(13974);
            }
            break;

            case 27:    // The Violet Citadel
            {
                Plr->Gossip_SendSQLPOI(449);
                SendQuickMenu(13971);
            }
            break;

            case 28:    // The Violet Hold
            {
                Plr->Gossip_SendSQLPOI(450);
                SendQuickMenu(13970);
            }
            break;

            case 29:    // Trade District
            {
                Plr->Gossip_SendSQLPOI(451);
                SendQuickMenu(13980);
            }
            break;

            case 30:    // Antonidas Memorial
            {
                Plr->Gossip_SendSQLPOI(452);
                SendQuickMenu(13968);
            }
            break;

            case 31:    // Runeweaver Square
            {
                Plr->Gossip_SendSQLPOI(453);
                SendQuickMenu(13967);
            }
            break;

            case 32:    // The Eventide
            {
                Plr->Gossip_SendSQLPOI(454);
                SendQuickMenu(13966);
            }
            break;

            case 33:    // Cemetary
            {
                Plr->Gossip_SendSQLPOI(455);
                SendQuickMenu(13965);
            }
            break;

            case 34:    // Lexicon of Power
            {
                Plr->Gossip_SendSQLPOI(456);
                SendQuickMenu(14174);
            }
            break;

            case 35:    // Class Trainers
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14018, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_A_QUART), 18);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_QUART), 19);
                Menu->SendTo(Plr);
            }
            break;

            case 36:    // Portal Trainer
            {
                Plr->Gossip_SendSQLPOI(457);
                SendQuickMenu(13999);
            }
            break;

            case 37:    // Profession Trainer
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 13996, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ALCHEMY), 52);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_BSMITHING), 53);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_COOKING), 54);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENCHANTING), 55);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_ENGINEERING), 56);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FIRST_AID), 57);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_FISHING), 58);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_HERBALISM), 59);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_INSCRIPTION), 60);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_JUWELCRAFTING), 61);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_LEATHER_W), 62);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_MINING), 63);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_SKINNING), 64);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(MENU_ITEM_TAILORING), 65);

                Menu->SendTo(Plr);
            }
            break;

            case 38:    // Armor
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14117, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_CLOTH_ARMOR), 66);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_LEATHER_ARMOR), 67);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_MAIL_ARMOR), 68);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_PLATE_ARMOR), 69);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SHIELDS), 70);
                Menu->SendTo(Plr);
            }
            break;

            case 39:    // Clothing
            {
                Plr->Gossip_SendSQLPOI(458);
                SendQuickMenu(14112);
            }
            break;

            case 40:    // Emblem Gear
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14108, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_A_QUART), 18);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_QUART), 19);
                Menu->SendTo(Plr);
            }
            break;

            case 41:    // Flowers
            {
                Plr->Gossip_SendSQLPOI(459);
                SendQuickMenu(10159);
            }
            break;

            case 42:    // Fruit
            {
                Plr->Gossip_SendSQLPOI(460);
                SendQuickMenu(14106);
            }
            break;

            case 43:    // General Goods
            {
                Plr->Gossip_SendSQLPOI(461);
                SendQuickMenu(14105);
            }
            break;

            case 44:    // Jewelry
            {
                Plr->Gossip_SendSQLPOI(462);
                SendQuickMenu(13984);
            }
            break;

            case 45:    // Pet Supplies & Exotic Mounts
            {
                Plr->Gossip_SendSQLPOI(463);
                SendQuickMenu(14103);
            }
            break;

            case 46:    // Pie, Pastry & Cakes
            {
                Plr->Gossip_SendSQLPOI(464);
                SendQuickMenu(14102);
            }
            break;

            case 47:    // Reagents & Magical Goods
            {
                Plr->Gossip_SendSQLPOI(465);
                SendQuickMenu(14104);
            }
            break;

            case 48:    // Toys
            {
                Plr->Gossip_SendSQLPOI(466);
                SendQuickMenu(14100);
            }
            break;

            case 49:    // Trinkets. Relics & Off-hand items
            {
                Plr->Gossip_SendSQLPOI(467);
                SendQuickMenu(14110);
            }
            break;

            case 50:    // Weapons
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14113, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_MELEE_WEAPONS), 71);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_RANGE_THROW_WEAPONS), 72);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_STAVES_WANDS), 73);
                Menu->SendTo(Plr);
            }
            break;

            case 51:    // Wine & Cheese
            {
                Plr->Gossip_SendSQLPOI(468);
                SendQuickMenu(14095);
            }
            break;

            case 52:    // Alchemy
            {
                Plr->Gossip_SendSQLPOI(469);
                SendQuickMenu(13995);
            }
            break;

            case 53:    // Blacksmithing
            {
                Plr->Gossip_SendSQLPOI(470);
                SendQuickMenu(13994);
            }
            break;

            case 54:    // Cooking
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 13991, Plr);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_A_INN), 24);
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_THE_H_INN), 25);
                Menu->SendTo(Plr);
            }
            break;

            case 55:    // Enchanting
            {
                Plr->Gossip_SendSQLPOI(471);
                SendQuickMenu(13990);
            }
            break;

            case 56:    // Engineering
            {
                Plr->Gossip_SendSQLPOI(472);
                SendQuickMenu(13989);
            }
            break;

            case 57:    // First Aid
            {
                Plr->Gossip_SendSQLPOI(473);
                SendQuickMenu(13988);
            }
            break;

            case 58:    // Fishing
            {
                Plr->Gossip_SendSQLPOI(474);
                SendQuickMenu(13987);
            }
            break;

            case 59:    // Herbalism
            {
                Plr->Gossip_SendSQLPOI(475);
                SendQuickMenu(13986);
            }
            break;

            case 60:    // Inscription
            {
                Plr->Gossip_SendSQLPOI(476);
                SendQuickMenu(13985);
            }
            break;

            case 61:    // Jewelcrafting
            {
                Plr->Gossip_SendSQLPOI(477);
                SendQuickMenu(13984);
            }
            break;

            case 62:    // Leatherworking
            {
                Plr->Gossip_SendSQLPOI(478);
                SendQuickMenu(13982);
            }
            break;

            case 63:    // Mining
            {
                Plr->Gossip_SendSQLPOI(479);
                SendQuickMenu(13983);
            }
            break;

            case 64:    // Skinning
            {
                Plr->Gossip_SendSQLPOI(480);
                SendQuickMenu(13982);
            }
            break;

            case 65:    // Tailoring
            {
                Plr->Gossip_SendSQLPOI(481);
                SendQuickMenu(13981);
            }
            break;

            case 66:    // Cloth Armor
            {
                Plr->Gossip_SendSQLPOI(482);
                SendQuickMenu(14112);
            }
            break;

            case 67:    // Leather Armor
            {
                Plr->Gossip_SendSQLPOI(483);
                SendQuickMenu(14111);
            }
            break;

            case 68:    // Mail Armor
            {
                Plr->Gossip_SendSQLPOI(484);
                SendQuickMenu(14111);
            }
            break;

            case 69:    // Plate Armor
            {
                Plr->Gossip_SendSQLPOI(485);
                SendQuickMenu(14109);
            }
            break;

            case 70:    // Shields
            {
                Plr->Gossip_SendSQLPOI(486);
                SendQuickMenu(14109);
            }
            break;

            case 71:    // Melee Weapons
            {
                Plr->Gossip_SendSQLPOI(487);
                SendQuickMenu(14098);
            }
            break;

            case 72:    // Ranged & Thrown Weapons
            {
                Plr->Gossip_SendSQLPOI(488);
                SendQuickMenu(14097);
            }
            break;

            case 73:    // Staves & Wands
            {
                Plr->Gossip_SendSQLPOI(489);
                SendQuickMenu(14096);
            }
            break;

            case 74:    // Krasu's Landing
            {
                Plr->Gossip_SendSQLPOI(490);
                SendQuickMenu(14009);
            }
            break;

            case 75:    // Trinkets, Relics & Off-hand Items
            {
                Plr->Gossip_SendSQLPOI(491);
                SendQuickMenu(14110);
            }
            break;

            case 76:    // Cold weather flying trainer
            {
                Plr->Gossip_SendSQLPOI(492);
                SendQuickMenu(60059);
            }
            break;

            case 77:    // Locksmith
            {
                Plr->Gossip_SendSQLPOI(493);
                SendQuickMenu(14004);
            }
            break;
        }
    }
};

void SetupGuardGossip(ScriptMgr* mgr)
{
    // Guard List
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
