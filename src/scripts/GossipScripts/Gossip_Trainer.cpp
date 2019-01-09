/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/WorldSession.h"
#include "Units/Creatures/Creature.h"
#include "Management/Gossip/Gossip.h"
#include "Server/Script/ScriptMgr.h"

class MasterHammersmith : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), 7245);
        menu.AddItem(GOSSIP_ICON_TRAINER, plr->GetSession()->LocalizedGossipOption(GI_T_HAMMERSMITH_LEARN), 1);
        menu.AddItem(GOSSIP_ICON_TRAINER, plr->GetSession()->LocalizedGossipOption(GI_T_HAMMERSMITH_UNLEARN), 2);

        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        uint32 textid;
        if (1 == Id)
        {
            if (!plr->_HasSkillLine(164) || plr->_GetSkillLineCurrent(164, false) < 300)
                textid = 20001;
            else if (!plr->HasSpell(9787))
                textid = 20002;
            else if (plr->HasSpell(17040))
                textid = 20003;
            else if (plr->HasSpell(17041) || plr->HasSpell(17039) || plr->HasSpell(9788))
                textid = 20004;
            else
            {
                if (!plr->hasEnoughCoinage(600))
                    textid = 20005;
                else
                {
                    //pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Make good use of this knowledge." );
                    textid = 20006;
                    dynamic_cast<Creature*>(pObject)->castSpell(plr, 39099, true);
                    plr->modCoinage(-600);
                }
            }
        }
        else
        {
            if (!plr->HasSpell(17040))
                textid = 20007;
            else if ((!plr->hasEnoughCoinage(250000) && plr->getLevel() <= 50) || (!plr->hasEnoughCoinage(500000) && plr->getLevel() > 50 && plr->getLevel() <= 65)
                || (!plr->hasEnoughCoinage(1000000) && plr->getLevel() > 65))
                textid = 20008;
            else
            {
                int32 unlearnGold = 0;
                if (plr->getLevel() <= 50)
                    unlearnGold = 250000;
                if (plr->getLevel() > 50 && plr->getLevel() <= 65)
                    unlearnGold = 500000;
                if (plr->getLevel() > 65)
                    unlearnGold = 1000000;

                plr->modCoinage(-unlearnGold);
                plr->removeSpell(17040, false, false, 0);
                textid = 20009;
            }
        }
        Arcemu::Gossip::Menu::SendSimpleMenu(pObject->getGuid(), textid, plr);
    }

    void Destroy() override { delete this; }

};

class MasterSwordsmith : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), 7247);
        menu.AddItem(GOSSIP_ICON_TRAINER, plr->GetSession()->LocalizedGossipOption(GI_T_SWORDSMITH_LEARN), 1);
        menu.AddItem(GOSSIP_ICON_TRAINER, plr->GetSession()->LocalizedGossipOption(GI_T_SWORDSMITH_UNLEARN), 2);
        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        uint32 textid;
        if (1 == Id)
        {
            if (!plr->_HasSkillLine(164) || plr->_GetSkillLineCurrent(164, false) < 300)
                textid = 20001;
            else if (!plr->HasSpell(9787))
                textid = 20002;
            else if (plr->HasSpell(17039))
                textid = 20003;
            else if (plr->HasSpell(17041) || plr->HasSpell(17040) || plr->HasSpell(9788))
                textid = 20004;
            else
            {
                if (!plr->hasEnoughCoinage(600))
                    textid = 20005;
                else
                {
                    //pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Make good use of this knowledge." );
                    textid = 20006;
                    dynamic_cast<Creature*>(pObject)->castSpell(plr, 39097, true);
                    plr->modCoinage(-600);
                }
            }
        }
        else
        {
            if (!plr->HasSpell(17039))
                textid = 20007;
            else if ((!plr->hasEnoughCoinage(250000) && plr->getLevel() <= 50) || (!plr->hasEnoughCoinage(500000) && plr->getLevel() > 50 && plr->getLevel() <= 65)
                || (!plr->hasEnoughCoinage(1000000) && plr->getLevel() > 65))
                textid = 20008;
            else
            {
                int32 unlearnGold = 0;
                if (plr->getLevel() <= 50)
                    unlearnGold = 250000;
                if (plr->getLevel() > 50 && plr->getLevel() <= 65)
                    unlearnGold = 500000;
                if (plr->getLevel() > 65)
                    unlearnGold = 1000000;

                plr->modCoinage(-unlearnGold);
                plr->removeSpell(17039, false, false, 0);
                textid = 20009;
            }
        }
        Arcemu::Gossip::Menu::SendSimpleMenu(pObject->getGuid(), textid, plr);
    }
};

class MasterAxesmith : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), 7243);
        menu.AddItem(GOSSIP_ICON_TRAINER, plr->GetSession()->LocalizedGossipOption(GI_T_AXESMITH_LEARN), 1);
        menu.AddItem(GOSSIP_ICON_TRAINER, plr->GetSession()->LocalizedGossipOption(GI_T_AXESMITH_UNLEARN), 2);
        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        uint32 textid;
        if (1 == Id)
        {
            if (!plr->_HasSkillLine(164) || plr->_GetSkillLineCurrent(164, false) < 300)
                textid = 20001;
            else if (!plr->HasSpell(9787))
                textid = 20002;
            else if (plr->HasSpell(17041))
                textid = 20003;
            else if (plr->HasSpell(17039) || plr->HasSpell(17040) || plr->HasSpell(9788))
                textid = 20004;
            else
            {
                if (!plr->hasEnoughCoinage(600))
                    textid = 20005;
                else
                {
                    //pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Make good use of this knowledge." );
                    textid = 20006;
                    dynamic_cast<Creature*>(pObject)->castSpell(plr, 39098, true);
                    plr->modCoinage(-600);
                }
            }
        }
        else
        {
            if (!plr->HasSpell(17041))
                textid = 20007;
            else if ((!plr->hasEnoughCoinage(250000) && plr->getLevel() <= 50) || (!plr->hasEnoughCoinage(500000) && plr->getLevel() > 50 && plr->getLevel() <= 65)
                || (!plr->hasEnoughCoinage(1000000) && plr->getLevel() > 65))
                textid = 20008;
            else
            {
                int32 unlearnGold = 0;
                if (plr->getLevel() <= 50)
                    unlearnGold = 250000;
                if (plr->getLevel() > 50 && plr->getLevel() <= 65)
                    unlearnGold = 500000;
                if (plr->getLevel() > 65)
                    unlearnGold = 1000000;

                plr->modCoinage(-unlearnGold);
                plr->removeSpell(17041, false, false, 0);
                textid = 20009;
            }
        }
        Arcemu::Gossip::Menu::SendSimpleMenu(pObject->getGuid(), textid, plr);
    }
};

void SetupTrainerScript(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(11191, new MasterHammersmith);    // Lilith the Lithe
    mgr->register_creature_gossip(11193, new MasterSwordsmith);     // Seril Scourgebane
    mgr->register_creature_gossip(11192, new MasterAxesmith);       // Kilram
}
