/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
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

#include "Setup.h"
#include "Storage/WorldStrings.h"
#include "Server/WorldSession.h"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Units/Creatures/Creature.h"
#include "Management/Gossip/Gossip.h"
#include "Server/Script/ScriptMgr.h"

//#define USE_THE_STATUS    // Decoment this is for the status

class ExarchNasuun_Gossip : public Arcemu::Gossip::Script
{
    public:
    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 12227);
#ifdef USE_THE_STATUS
        menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_SHATT_EXARCH_NASUUN_1), 1);   // this is the status
        menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_SHATT_EXARCH_NASUUN_2), 2);
#else
        menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_SHATT_EXARCH_NASUUN_2), 3);
#endif
        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        switch (Id)
        {
            case 0:
                OnHello(pObject, plr);
                break;
            case 1:
                Arcemu::Gossip::Menu::SendQuickMenu(pObject->GetGUID(), 12303, plr, 0, GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_SHATT_EXARCH_NASUUN_3));
                break;
            case 2:
                Arcemu::Gossip::Menu::SendQuickMenu(pObject->GetGUID(), 12305, plr, 0, GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_SHATT_EXARCH_NASUUN_3));
                break;
            case 3:
                Arcemu::Gossip::Menu::SendQuickMenu(pObject->GetGUID(), 12623, plr, 0, GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_SHATT_EXARCH_NASUUN_3));
                break;
            default:
                break;
        }
    }

};

class ZephyrGossipScript : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* Plr) override
        {
            Arcemu::Gossip::Menu::SendQuickMenu(pObject->GetGUID(), 1, Plr, 1, GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SHATT_ZEPH_COT));
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*EnteredCode*/, uint32 /*gossipId*/) override
        {
            if (plr->GetStanding(989) >= 21000)
                //plr->SafeTeleport( 1, 0, -8170.441406f, -4751.321777f, 33.457771f, 5.136f);
                static_cast<Creature*>(pObject)->CastSpell(plr, sSpellCustomizations.GetSpellInfo(37778), true);
            else
                plr->BroadcastMessage(plr->GetSession()->LocalizedWorldSrv(ServerString::SHATT_ZEPH_KOT)); // Dunno what the correct text is ^^
            Arcemu::Gossip::Menu::Complete(plr);
        }

        void Destroy() override { delete this; }
};

void SetupShattrathGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(25967, new ZephyrGossipScript);        // Zephyr
    mgr->register_creature_gossip(24932, new ExarchNasuun_Gossip); // Exarch Nasuun
}
