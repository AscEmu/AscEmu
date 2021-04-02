/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

void GuardsOnSalute(Player* pPlayer, Unit* pUnit)
{
    if (pPlayer == NULL || pUnit == NULL)
        return;

    // Check if we are friendly with our Guards (they will salute only when You are)
    if (((pUnit->getEntry() == 68 || pUnit->getEntry() == 1976) && pPlayer->GetStandingRank(72) >= STANDING_FRIENDLY) || (pUnit->getEntry() == 3296 && pPlayer->GetStandingRank(76) >= STANDING_FRIENDLY))
    {
        uint32_t EmoteChance = Util::getRandomUInt(100);
        if (EmoteChance < 33) // 1/3 chance to get Salute from Guard
            pUnit->emote(EMOTE_ONESHOT_SALUTE);
    }
}

void GaurdsOnKiss(Player* pPlayer, Unit* pUnit)
{
    if (pPlayer == NULL || pUnit == NULL)
        return;

    // Check if we are friendly with our Guards (they will bow only when You are)
    if (((pUnit->getEntry() == 68 || pUnit->getEntry() == 1976) && pPlayer->GetStandingRank(72) >= STANDING_FRIENDLY) || (pUnit->getEntry() == 3296 && pPlayer->GetStandingRank(76) >= STANDING_FRIENDLY))
    {
        uint32_t EmoteChance = Util::getRandomUInt(100);
        if (EmoteChance < 33) // 1/3 chance to get Bow from Guard
            pUnit->emote(EMOTE_ONESHOT_BOW);
    }
}

void GuardsOnWave(Player* pPlayer, Unit* pUnit)
{
    if (pPlayer == NULL || pUnit == NULL)
        return;

    // Check if we are friendly with our Guards (they will wave only when You are)
    if (((pUnit->getEntry() == 68 || pUnit->getEntry() == 1976) && pPlayer->GetStandingRank(72) >= STANDING_FRIENDLY) || (pUnit->getEntry() == 3296 && pPlayer->GetStandingRank(76) >= STANDING_FRIENDLY))
    {
        uint32_t EmoteChance = Util::getRandomUInt(100);
        if (EmoteChance < 33) // 1/3 chance to get Bow from Guard
            pUnit->emote(EMOTE_ONESHOT_WAVE);
    }
}

void OnEmote(Player* pPlayer, uint32_t Emote, Unit* pUnit)
{
    if (!pUnit || !pUnit->isAlive() || pUnit->GetAIInterface()->getNextTarget())
        return;

    // Switch For Emote Name (You do EmoteName to Script Name link).
    switch (Emote)
    {
        case EMOTE_ONESHOT_SALUTE: // <- Its EMOTE name.
            GuardsOnSalute(pPlayer, pUnit); // <- Its Link.
            break;

        case EMOTE_ONESHOT_KISS:
            GaurdsOnKiss(pPlayer, pUnit);
            break;

        case EMOTE_ONESHOT_WAVE:
            GuardsOnWave(pPlayer, pUnit);
            break;
    }
}

class JeanPierrePoulain : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 14500);
        if (plr->HasFinishedQuest(13668) || plr->hasQuestInQuestLog(13668) || plr->HasFinishedQuest(13667) || plr->hasQuestInQuestLog(13667))
        {
            menu.sendGossipPacket(plr);
        }
        else
        {
            menu.addItem(GOSSIP_ICON_CHAT, 446, 1);     // I'll take the flight.
            menu.sendGossipPacket(plr);
        }
    }

    void onSelectOption(Object* /*pObject*/, Player* Plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Plr->castSpell(Plr, 64795, true);
        GossipMenu::senGossipComplete(Plr);
    }
};

class Wormhole : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        if (plr->_GetSkillLineCurrent(202, false) >= 415)
        {
            GossipMenu menu(pObject->getGuid(), 14785);
            menu.addItem(GOSSIP_ICON_CHAT, 447, 1);     // Borean Tundra
            menu.addItem(GOSSIP_ICON_CHAT, 448, 2);     // Howling Fjord
            menu.addItem(GOSSIP_ICON_CHAT, 449, 3);     // Sholazar Basin
            menu.addItem(GOSSIP_ICON_CHAT, 450, 4);     // Icecrown
            menu.addItem(GOSSIP_ICON_CHAT, 451, 5);     // Storm Peaks

            if (Util::getRandomUInt(100) > 50)
                menu.addItem(GOSSIP_ICON_CHAT, 452, 6);     // Underground...

            menu.sendGossipPacket(plr);
        }
    }

    void onSelectOption(Object* /*pObject*/, Player* Plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
                Plr->castSpell(Plr, 67834, true);
                break;
            case 2:
                Plr->castSpell(Plr, 67838, true);
                break;
            case 3:
                Plr->castSpell(Plr, 67835, true);
                break;
            case 4:
                Plr->castSpell(Plr, 67836, true);
                break;
            case 5:
                Plr->castSpell(Plr, 67837, true);
                break;
            case 6:
                Plr->castSpell(Plr, 68081, true);
                break;
        }
        GossipMenu::senGossipComplete(Plr);
    }
};

void SetupRandomScripts(ScriptMgr* mgr)
{
    // Register Hook Event here
    mgr->register_hook(SERVER_HOOK_EVENT_ON_EMOTE, (void*)&OnEmote);

    GossipScript* jeanPierrePoulain = new JeanPierrePoulain();
    mgr->register_creature_gossip(34244, jeanPierrePoulain);

    GossipScript* wormhole = new Wormhole();
    mgr->register_creature_gossip(35646, wormhole);
}