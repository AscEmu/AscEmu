/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

#include "WanderingIsle.hpp"

#include "Chat/ChatDefines.hpp"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Management/QuestMgr.h"
#include "Map/Maps/MapScriptInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Movement/MovementManager.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellScript.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Utilities/LocationVector.hpp"

#include <set>

//////////////////////////////////////////////////////////////////////////////////////////
// Temple of Five Dawns - quests 29422 (Huo's offering), 29423, 29775, 29776.

enum TempleEntries
{
    NPC_MASTER_SHANG_XI_TEMPLE = 54786,
    NPC_HUO_QUESTGIVER = 54787,
    NPC_HUO_COMPANION = 54958,
    NPC_UPLIFT_DRAFT = 55685,
    NPC_AYSA_TEMPLE = 61126,
    NPC_JI_TEMPLE = 61127,
    NPC_FIRE_SPIRIT_AT_TEMPLE_CREDIT = 61128
};

enum TempleAreas
{
    AREA_TEMPLE_OF_FIVE_DAWNS = 5820
};

const uint32_t SOUND_MASTER_SHANG_XI_WELCOME_HUO = 27788;

const LocationVector jiTempleMovePoint1(966.1493f, 3607.0894f, 196.51373f);
const LocationVector jiTempleMovePoint2(958.9819f, 3594.94f, 196.6083f);
const LocationVector aysaTempleMovePoint1(966.3715f, 3602.764f, 196.47968f);
const LocationVector aysaTempleMovePoint2(943.1327f, 3572.154f, 193.6543f);
const LocationVector jiTempleSpawnPos(971.5566f, 3607.8015f, 195.71495f);
const LocationVector aysaTempleSpawnPos(968.2688f, 3602.207f, 196.6442f);
const LocationVector huoTemplePos(955.11584f, 3604.04f, 200.71805f);

//////////////////////////////////////////////////////////////////////////////////////////
// Huo's Offering (102522) - the dummy effect gives the credit for 29422

class HuoOfferingSpell : public SpellScript
{
public:
    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t effIndex) override
    {
        if (effIndex != 1)
            return SpellScriptCheckDummy::DUMMY_NOT_HANDLED;

        Unit* target = spell->getUnitTarget();
        Player* player = spell->getPlayerCaster();
        if (player != nullptr && target != nullptr && target->getEntry() == NPC_HUO_QUESTGIVER)
            giveIsleQuestCredit(player, QUEST_HUO_THE_SPIRIT_OF_FIRE, 0);

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Areatrigger 7835 - Huo arrives at the temple (quest 29423)

void onTempleOfFiveDawnsTrigger(Player* player)
{
    if (getIsleQuestStatus(player, QUEST_THE_PASSION_OF_SHEN_ZIN_SU) != ISLE_QUEST_INCOMPLETE)
        return;

    MapScriptInterface* mapInterface = player->getWorldMap()->getInterface();
    if (mapInterface == nullptr)
        return;

    // Huo has to walk with the player
    Creature* huo = mapInterface->findNearestCreature(player, NPC_HUO_COMPANION, 15.0f);
    Creature* master = mapInterface->findNearestCreature(player, NPC_MASTER_SHANG_XI_TEMPLE, 25.0f);
    if (huo == nullptr || master == nullptr)
        return;

    master->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Welcome, Huo. The people have missed your warmth.", player);
    master->PlaySoundToSet(SOUND_MASTER_SHANG_XI_WELCOME_HUO);

    if (Creature* aysa = master->summonCreature(NPC_AYSA_TEMPLE, aysaTempleSpawnPos, TIMED_OR_CORPSE_DESPAWN, 120000))
        aysa->getMovementManager()->movePoint(0, aysaTempleMovePoint1);

    if (Creature* ji = master->summonCreature(NPC_JI_TEMPLE, jiTempleSpawnPos, TIMED_OR_CORPSE_DESPAWN, 120000))
        ji->getMovementManager()->movePoint(0, jiTempleMovePoint1);

    giveIsleQuestCredit(player, QUEST_THE_PASSION_OF_SHEN_ZIN_SU, 0);

    huo->getMovementManager()->movePoint(0, huoTemplePos);
    huo->Despawn(60000, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Uplifting Draft (55685) - the wind carries the player to the roof of the temple

const LocationVector tornadoPos1(922.9829f, 3602.6472f, 198.44557f);
const LocationVector tornadoPos2(958.7378f, 3610.3413f, 209.64407f);
const LocationVector tornadoPos3(934.45074f, 3609.4578f, 240.62868f);
const LocationVector tornadoEndPos(920.4496f, 3604.7705f, 253.17314f);

class UpliftDraftAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UpliftDraftAI(c); }
    explicit UpliftDraftAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(1000);
    }

    enum Events
    {
        EVENT_MOVE_POS1 = 1,
        EVENT_MOVE_POS2,
        EVENT_MOVE_POS3,
        EVENT_MOVE_END_POS
    };

    void OnLoad() override
    {
        setFlyMode(true);
        scriptEvents.resetEvents();
        scriptEvents.addEvent(EVENT_MOVE_POS1, 1000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MOVE_POS1:
                movePoint(0, tornadoPos1);
                scriptEvents.addEvent(EVENT_MOVE_POS2, 1000);
                break;
            case EVENT_MOVE_POS2:
                movePoint(1, tornadoPos2);
                scriptEvents.addEvent(EVENT_MOVE_POS3, 5000);
                break;
            case EVENT_MOVE_POS3:
                movePoint(2, tornadoPos3);
                scriptEvents.addEvent(EVENT_MOVE_END_POS, 5000);
                break;
            case EVENT_MOVE_END_POS:
                movePoint(4, tornadoEndPos);
                break;
            default:
                break;
        }
    }
};

static void summonUpliftDraft(Player* player)
{
    if (Creature* vehicle = player->summonCreature(NPC_UPLIFT_DRAFT, player->GetPosition(), TIMED_DESPAWN, 20000))
        player->callEnterVehicle(vehicle);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Master Shang Xi (54786) at the temple - welcomes Huo, sends Ji and Aysa out for the other spirits

class MasterShangXiTempleAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MasterShangXiTempleAI(c); }
    explicit MasterShangXiTempleAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(1000);
    }

    enum Events
    {
        EVENT_MASTER_SHANG_XI_TALK_1 = 1,
        EVENT_MASTER_SHANG_XI_TALK_2,
        EVENT_MASTER_SHANG_XI_TALK_3,
        EVENT_MASTER_SHANG_XI_TALK_4,
        EVENT_MASTER_SHANG_XI_TALK_5,
        EVENT_MASTER_SHANG_XI_TALK_6,
        EVENT_MASTER_SHANG_XI_TALK_7,
        EVENT_MASTER_SHANG_XI_TALK_8,
        EVENT_MASTER_SHANG_XI_TALK_9
    };

    void OnLoad() override
    {
        mStarted = false;
        scriptEvents.resetEvents();
    }

    void onQuestAccept(Player* player, QuestProperties const* qst) override
    {
        if (qst->id == QUEST_TEMPLE_OF_THE_FIVE_DAWNS)
            summonUpliftDraft(player);
    }

    // once per second (RegisterAIUpdateEvent)
    void AIUpdate() override
    {
        if (!mStarted)
            checkForArrivingDisciples();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mStarted)
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MASTER_SHANG_XI_TALK_1:
                for (Player* player : getPlayersInPhase(20.0f, QUEST_THE_PASSION_OF_SHEN_ZIN_SU, ISLE_QUEST_REWARDED))
                    getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "You have conquered every challenge I put before you, $n. You have found Huo and brought him safely to the temple.", player);
                scriptEvents.addEvent(EVENT_MASTER_SHANG_XI_TALK_2, 10000);
                break;
            case EVENT_MASTER_SHANG_XI_TALK_2:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "There is a much larger problem we face now, my students. Shen-zin Su is in pain. If we do not act the very land on which we stand could die, and all of us with it.");
                scriptEvents.addEvent(EVENT_MASTER_SHANG_XI_TALK_3, 10000);
                break;
            case EVENT_MASTER_SHANG_XI_TALK_3:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "We need to speak to Shen-zin Su and discover how to heal it. And to do that, we need the four elemental spirits returned. Huo was the first.");
                scriptEvents.addEvent(EVENT_MASTER_SHANG_XI_TALK_4, 10000);
                break;
            case EVENT_MASTER_SHANG_XI_TALK_4:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Ji, I'd like you to go to the Dai-Lo Farmstead in search of Wugou, the spirit of earth.");
                scriptEvents.addEvent(EVENT_MASTER_SHANG_XI_TALK_5, 10000);
                if (Creature* ji = findNearestCreatureInPhase(NPC_JI_TEMPLE, 20.0f))
                {
                    ji->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "On It!");
                    ji->getMovementManager()->movePoint(1, jiTempleMovePoint2);
                    ji->Despawn(2500, 0);
                }
                break;
            case EVENT_MASTER_SHANG_XI_TALK_5:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Aysa, I want you to go to the Singing Pools to find Shu, the spirit of water.");
                scriptEvents.addEvent(EVENT_MASTER_SHANG_XI_TALK_6, 10000);
                if (Creature* aysa = findNearestCreatureInPhase(NPC_AYSA_TEMPLE, 20.0f))
                {
                    aysa->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Yes master.");
                    aysa->getMovementManager()->movePoint(2, aysaTempleMovePoint2);
                    aysa->Despawn(2500, 0);
                }
                break;
            case EVENT_MASTER_SHANG_XI_TALK_6:
                for (Player* player : getPlayersInPhase(20.0f, QUEST_THE_PASSION_OF_SHEN_ZIN_SU, ISLE_QUEST_REWARDED))
                    getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "And $n, you shall be the hand that guides us all. Speak with me for a moment before you join Aysa at the Singing Pools to the east.", player);
                mStarted = false;
                break;
            case EVENT_MASTER_SHANG_XI_TALK_7:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "You've returned with the spirits of water and earth. You make an old master proud.");
                scriptEvents.addEvent(EVENT_MASTER_SHANG_XI_TALK_8, 10000);
                break;
            case EVENT_MASTER_SHANG_XI_TALK_8:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Wugou and Shu are welcome here. We will care for them well.");
                scriptEvents.addEvent(EVENT_MASTER_SHANG_XI_TALK_9, 10000);
                break;
            case EVENT_MASTER_SHANG_XI_TALK_9:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "The only remaining spirit is Dafeng, who hides somewhere across the Dawning Span to the west.");
                mStarted = false;
                break;
            default:
                break;
        }
    }

private:
    // every player triggers the dialogue once - the trigger condition (29423 rewarded) is permanent
    void checkForArrivingDisciples()
    {
        if (getCreature()->getAreaId() != AREA_TEMPLE_OF_FIVE_DAWNS)
            return;

        for (Player* player : getPlayersInPhase(20.0f, QUEST_THE_PASSION_OF_SHEN_ZIN_SU, ISLE_QUEST_REWARDED))
        {
            if (mGreetedPlayers.count(player->getGuid()) != 0)
                continue;

            mGreetedPlayers.insert(player->getGuid());
            mStarted = true;

            if (player->hasQuestFinished(QUEST_THE_DAWNING_VALLEY))
                scriptEvents.addEvent(EVENT_MASTER_SHANG_XI_TALK_7, 1000);
            else
                scriptEvents.addEvent(EVENT_MASTER_SHANG_XI_TALK_1, 1000);
            return;
        }
    }

    bool mStarted = false;
    std::set<uint64_t> mGreetedPlayers;
};

class MasterShangXiTempleGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        uint32_t textId = sMySQLStore.getGossipTextIdForNpc(pObject->getEntry());
        if (sMySQLStore.getNpcGossipText(textId) == nullptr)
            textId = DefaultGossipTextId;

        GossipMenu menu(pObject->getGuid(), textId, plr->getSession()->language);
        sQuestMgr.FillQuestMenu(static_cast<Creature*>(pObject), plr, menu);

        if (getIsleQuestStatus(plr, QUEST_TEMPLE_OF_THE_FIVE_DAWNS) != ISLE_QUEST_NONE)
            menu.addItem(GOSSIP_ICON_CHAT, 0, 1, "I would like to go back on the top of the temple");

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* /*pObject*/, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        summonUpliftDraft(plr);
    }
};

void SetupWanderingIsleTempleOfFiveDawns(ScriptMgr* mgr)
{
    mgr->register_spell_script(102522, new HuoOfferingSpell);
    mgr->register_creature_script(NPC_UPLIFT_DRAFT, &UpliftDraftAI::Create);
    mgr->register_creature_script(NPC_MASTER_SHANG_XI_TEMPLE, &MasterShangXiTempleAI::Create);
    mgr->register_creature_gossip(NPC_MASTER_SHANG_XI_TEMPLE, new MasterShangXiTempleGossip);
}
