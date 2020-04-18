/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/WorldSession.h"
#include "Units/Creatures/Creature.h"
#include "Management/Gossip/GossipScript.h"
#include "Server/Script/ScriptMgr.h"
#include "Management/Gossip/GossipMenu.h"

class MasterHammersmith : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 7245);
        menu.addItem(GOSSIP_ICON_TRAINER, GI_T_HAMMERSMITH_LEARN, 1);
        menu.addItem(GOSSIP_ICON_TRAINER, GI_T_HAMMERSMITH_UNLEARN, 2);

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        uint32_t textid;
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
            else if (!plr->hasEnoughCoinage(250000) && plr->getLevel() <= 50 || !plr->hasEnoughCoinage(500000) && plr->getLevel() > 50 && plr->getLevel() <= 65
                || !plr->hasEnoughCoinage(1000000) && plr->getLevel() > 65)
                textid = 20008;
            else
            {
                int32_t unlearnGold = 0;
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
        GossipMenu::sendSimpleMenu(pObject->getGuid(), textid, plr);
    }

    void destroy() override { delete this; }

};

class MasterSwordsmith : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 7247);
        menu.addItem(GOSSIP_ICON_TRAINER, GI_T_SWORDSMITH_LEARN, 1);
        menu.addItem(GOSSIP_ICON_TRAINER, GI_T_SWORDSMITH_UNLEARN, 2);
        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        uint32_t textid;
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
            else if (!plr->hasEnoughCoinage(250000) && plr->getLevel() <= 50 || !plr->hasEnoughCoinage(500000) && plr->getLevel() > 50 && plr->getLevel() <= 65
                || !plr->hasEnoughCoinage(1000000) && plr->getLevel() > 65)
                textid = 20008;
            else
            {
                int32_t unlearnGold = 0;
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
        GossipMenu::sendSimpleMenu(pObject->getGuid(), textid, plr);
    }
};

class MasterAxesmith : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 7243);
        menu.addItem(GOSSIP_ICON_TRAINER, GI_T_AXESMITH_LEARN, 1);
        menu.addItem(GOSSIP_ICON_TRAINER, GI_T_AXESMITH_UNLEARN, 2);
        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        uint32_t textid;
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
            else if (!plr->hasEnoughCoinage(250000) && plr->getLevel() <= 50 || !plr->hasEnoughCoinage(500000) && plr->getLevel() > 50 && plr->getLevel() <= 65
                || !plr->hasEnoughCoinage(1000000) && plr->getLevel() > 65)
                textid = 20008;
            else
            {
                int32_t unlearnGold = 0;
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
        GossipMenu::sendSimpleMenu(pObject->getGuid(), textid, plr);
    }
};

void SetupTrainerScript(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(11191, new MasterHammersmith);    // Lilith the Lithe
    mgr->register_creature_gossip(11193, new MasterSwordsmith);     // Seril Scourgebane
    mgr->register_creature_gossip(11192, new MasterAxesmith);       // Kilram
}
