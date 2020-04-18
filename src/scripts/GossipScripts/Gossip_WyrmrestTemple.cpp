/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/TaxiMgr.h"
#include "Objects/ObjectMgr.h"

enum CreatureEntry
{
    CN_TORASTRASZA = 26949,
    CN_AFRASASTRASZ = 27575,
    CN_TARIOLSTRASZ = 26443
};

enum eGossipText
{
    GT_TORASTRASZA = 12714,
    GT_AFRASASTRASZ = 12887,
    GT_TARIOLSTRASZ = 12713
};

enum eGossipItems
{
    GI_WYMREST_TOP_TO_BOTTOM = 413,     // "Yes, please, I would like to return to the ground level of the temple."
    GI_WYMREST_TOP_TO_MIDDLE = 414,     // "I would like to go Lord Afrasastrasz in the middle of the temple."
    GI_WYMREST_MIDDLE_TO_TOP = 415,     // "My lord, I need to get to the top of the temple."
    GI_WYMREST_MIDDLE_TO_BOTTOM = 416,  // "Can I get a ride back to ground level, Lord Afrasastrasz?"
    GI_WYMREST_BOTTOM_TO_TOP = 417,     // "Steward, please allow me to ride one of the drakes to the queen's chamber at the top of the temple."
    GI_WYMREST_BOTTOM_TO_MIDDLE = 418   // "Can you spare a drake to take me to Lord Afrasastrasz in the middle of the temple?"
};

class WyrmrestTemple_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        switch (pObject->getEntry())
        {
            case CN_TORASTRASZA:
            {
                GossipMenu menu(pObject->getGuid(), GT_TORASTRASZA, 0);
                menu.addItem(GOSSIP_ICON_CHAT, GI_WYMREST_TOP_TO_BOTTOM, 1);
                menu.addItem(GOSSIP_ICON_CHAT, GI_WYMREST_TOP_TO_MIDDLE, 2);
                menu.sendGossipPacket(plr);
            }
            break;
            case CN_AFRASASTRASZ:
            {
                GossipMenu menu(pObject->getGuid(), GT_AFRASASTRASZ, 0);
                menu.addItem(GOSSIP_ICON_CHAT, GI_WYMREST_MIDDLE_TO_TOP, 3);
                menu.addItem(GOSSIP_ICON_CHAT, GI_WYMREST_MIDDLE_TO_BOTTOM, 4);
                menu.sendGossipPacket(plr);
            }
            break;
            case CN_TARIOLSTRASZ:
            {
                GossipMenu menu(pObject->getGuid(), GT_TARIOLSTRASZ, 0);
                menu.addItem(GOSSIP_ICON_CHAT, GI_WYMREST_BOTTOM_TO_TOP, 5);
                menu.addItem(GOSSIP_ICON_CHAT, GI_WYMREST_BOTTOM_TO_MIDDLE, 6);
                menu.sendGossipPacket(plr);
            }
            break;
        }

    };

    void onSelectOption(Object* /*pObject*/, Player* plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
                plr->TaxiStart(sTaxiMgr.GetTaxiPath(879), 6371, 0);     // Drake
                break;
            case 2:
                plr->TaxiStart(sTaxiMgr.GetTaxiPath(880), 6371, 0);     // Drake
                break;
            case 3:
                plr->TaxiStart(sTaxiMgr.GetTaxiPath(881), 6371, 0);     // Drake
                break;
            case 4:
                plr->TaxiStart(sTaxiMgr.GetTaxiPath(882), 6371, 0);     // Drake
                break;
            case 5:
                plr->TaxiStart(sTaxiMgr.GetTaxiPath(878), 6371, 0);     // Drake
                break;
            case 6:
                plr->TaxiStart(sTaxiMgr.GetTaxiPath(883), 6371, 0);     // Drake
                break;
            default:
                break;
        }
        GossipMenu::senGossipComplete(plr);
    };
};

void SetupWyrmrestTempleGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(CN_TORASTRASZA, new WyrmrestTemple_Gossip());
    mgr->register_creature_gossip(CN_AFRASASTRASZ, new WyrmrestTemple_Gossip());
    mgr->register_creature_gossip(CN_TARIOLSTRASZ, new WyrmrestTemple_Gossip());
}
