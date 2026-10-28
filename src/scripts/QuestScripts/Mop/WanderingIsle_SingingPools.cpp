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
#include "Movement/MovementDefines.h"
#include "Movement/MovementManager.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Utilities/LocationVector.hpp"
#include "Utilities/MathConstants.hpp"
#include "Utilities/Random.hpp"

#include <cmath>

//////////////////////////////////////////////////////////////////////////////////////////
// Singing Pools and Dai-Lo Farmstead - quests 29679 A New Friend and 29774 Not In the Face!

enum PoolEntries
{
    NPC_SHU_POOL_OF_REFLECTION = 65493,
    NPC_WATER_SPOUT_BUNNY = 60488,
    NPC_AYSA_SINGING_POOLS = 54975,
    NPC_SHU_DAI_LO = 55556,
    NPC_SHU_COMPANION = 55558,
    NPC_WUGOU = 55539,
    NPC_WATER_SPIRIT_SPEAK_CREDIT = 55548,
    NPC_WATER_SPLASH_CREDIT = 55547
};

enum PoolSpells
{
    SPELL_WATER_SPOUT_VISUAL = 116695,
    SPELL_WATER_SPOUT_HIT = 116696,
    SPELL_WATER_SPOUT_BURST = 117057,
    SPELL_AYSA_CONGRATULATION_TRIGGER = 128588,
    SPELL_WUGOU_WAKE_UP = 118027
};

enum PoolAreas
{
    AREA_POOL_OF_REFLECTION = 5862
};

const uint32_t POOL_RESPAWN_TIME = 120000;

//////////////////////////////////////////////////////////////////////////////////////////
// Shu (65493) at the Pool of Reflection - jumps around and summons water spouts

const LocationVector shuJumpPos[4] =
{
    { 1102.05f, 2882.11f, 94.32f, 0.11f },
    { 1120.01f, 2883.20f, 96.44f, 4.17f },
    { 1128.09f, 2859.44f, 97.64f, 2.51f },
    { 1111.52f, 2849.84f, 94.84f, 1.94f }
};

class ShuPoolOfReflectionAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShuPoolOfReflectionAI(c); }
    explicit ShuPoolOfReflectionAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(1000);
    }

    enum Events
    {
        EVENT_ROTATE_POSITION = 1,
        EVENT_SUMMON_WATER_SPOUT,
        EVENT_WATER_SPOUT_BURST,
        EVENT_WATER_SPOUT_DESPAWN
    };

    void OnLoad() override
    {
        reset();
    }

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type == POINT_MOTION_TYPE && id == 0 && mRunning)
            scriptEvents.addEvent(EVENT_ROTATE_POSITION, 5000);
    }

    // once per second (RegisterAIUpdateEvent): wait for disciples with the quest
    void AIUpdate() override
    {
        if (mRunning || getCreature()->getAreaId() != AREA_POOL_OF_REFLECTION)
            return;

        if (!getPlayersInPhase(20.0f, QUEST_THE_SOURCE_OF_OUR_LIVELIHOOD, ISLE_QUEST_INCOMPLETE).empty())
        {
            mRunning = true;
            scriptEvents.addEvent(EVENT_ROTATE_POSITION, 5000);
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mRunning)
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ROTATE_POSITION:
            {
                // the fourth position is never used
                const uint32_t newPlace = Util::getRandomUInt(2);
                moveJump(shuJumpPos[newPlace], 10.0f, 10.0f, 1);
                _applyAura(SPELL_WATER_SPOUT_VISUAL);
                scriptEvents.addEvent(EVENT_SUMMON_WATER_SPOUT, 2000);
                break;
            }
            case EVENT_SUMMON_WATER_SPOUT:
            {
                const float angle = getCreature()->GetOrientation() + Util::getRandomFloat(-AscEmu::Math::PiF, AscEmu::Math::PiF);
                const float x = getCreature()->GetPositionX() + 5.0f * std::cos(angle);
                const float y = getCreature()->GetPositionY() + 5.0f * std::sin(angle);

                if (Creature* waterSpout = summonCreature(NPC_WATER_SPOUT_BUNNY, x, y, 92.189629f, 0.0f))
                {
                    mWaterSpoutGuid = waterSpout->getGuid();
                    waterSpout->castSpell(waterSpout, SPELL_WATER_SPOUT_VISUAL, true);
                }
                scriptEvents.addEvent(EVENT_WATER_SPOUT_BURST, 7000);
                break;
            }
            case EVENT_WATER_SPOUT_BURST:
            {
                if (Creature* waterSpout = getCreature()->getWorldMapCreature(mWaterSpoutGuid))
                {
                    for (Object* object : waterSpout->getInRangePlayersSet())
                    {
                        Player* player = static_cast<Player*>(object);
                        if (player != nullptr && waterSpout->isInSamePhase(player) && waterSpout->getDistance2d(player) <= 1.0f)
                            player->castSpell(player, SPELL_WATER_SPOUT_HIT, true);
                    }
                    waterSpout->castSpell(waterSpout, SPELL_WATER_SPOUT_BURST, true);
                }
                scriptEvents.addEvent(EVENT_WATER_SPOUT_DESPAWN, 3000);
                break;
            }
            case EVENT_WATER_SPOUT_DESPAWN:
            {
                if (Creature* waterSpout = getCreature()->getWorldMapCreature(mWaterSpoutGuid))
                    waterSpout->Despawn(100, 0);
                mWaterSpoutGuid = 0;

                if (!getPlayersInPhase(20.0f, QUEST_THE_SOURCE_OF_OUR_LIVELIHOOD, ISLE_QUEST_INCOMPLETE).empty())
                {
                    scriptEvents.addEvent(EVENT_ROTATE_POSITION, 5000);
                    break;
                }

                // nobody left to train: Aysa congratulates those who finished
                if (Creature* aysa = findNearestCreatureInPhase(NPC_AYSA_SINGING_POOLS, 60.0f))
                {
                    for (Object* object : aysa->getInRangePlayersSet())
                    {
                        Player* player = static_cast<Player*>(object);
                        if (player == nullptr || !aysa->isInSamePhase(player) || aysa->getDistance2d(player) > 60.0f)
                            continue;

                        if (getIsleQuestStatus(player, QUEST_THE_SOURCE_OF_OUR_LIVELIHOOD) == ISLE_QUEST_COMPLETE && !player->hasAurasWithId(SPELL_AYSA_CONGRATULATION_TRIGGER))
                        {
                            player->castSpell(player, SPELL_AYSA_CONGRATULATION_TRIGGER, false);
                            aysa->SendScriptTextChatMessage(TEXT_AYSA_POOL_FUN);
                        }
                    }
                }
                reset();
                break;
            }
            default:
                break;
        }
    }

private:
    void reset()
    {
        mRunning = false;
        mWaterSpoutGuid = 0;
        scriptEvents.resetEvents();
    }

    bool mRunning = false;
    uint64_t mWaterSpoutGuid = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shu (55556) at the Dai-Lo Farmstead - wakes Wugou up, afterwards both follow the player

const LocationVector shuPos1(650.30f, 3127.16f, 89.62f);
const LocationVector shuPos2(625.25f, 3127.88f, 87.95f);
const LocationVector shuPos3(624.44f, 3142.94f, 87.75f);

class ShuDaiLoAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShuDaiLoAI(c); }
    explicit ShuDaiLoAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(500);
    }

    enum Events
    {
        EVENT_MOVE_POS1 = 1,
        EVENT_MOVE_POS2,
        EVENT_MOVE_POS3,
        EVENT_CAST,
        EVENT_GIVE_CREDIT
    };

    void OnLoad() override
    {
        scriptEvents.resetEvents();
    }

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type == POINT_MOTION_TYPE && id == 0)
            scriptEvents.addEvent(EVENT_MOVE_POS1, 1000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MOVE_POS1:
                movePoint(1, shuPos1);
                scriptEvents.addEvent(EVENT_MOVE_POS2, 500);
                break;
            case EVENT_MOVE_POS2:
                movePoint(2, shuPos2);
                scriptEvents.addEvent(EVENT_MOVE_POS3, 4000);
                break;
            case EVENT_MOVE_POS3:
                movePoint(3, shuPos3);
                scriptEvents.addEvent(EVENT_CAST, 3000);
                break;
            case EVENT_CAST:
                if (Creature* wugou = findNearestCreatureInPhase(NPC_WUGOU, 15.0f))
                {
                    wugou->castSpell(wugou, SPELL_WUGOU_WAKE_UP, false);
                    getCreature()->setFacingToObject(wugou);
                    scriptEvents.addEvent(EVENT_GIVE_CREDIT, 5000);
                }
                break;
            case EVENT_GIVE_CREDIT:
                if (Creature* wugou = findNearestCreatureInPhase(NPC_WUGOU, 15.0f))
                {
                    for (Object* object : wugou->getInRangePlayersSet())
                    {
                        Player* player = static_cast<Player*>(object);
                        if (player == nullptr || !wugou->isInSamePhase(player) || wugou->getDistance2d(player) > 15.0f)
                            continue;

                        giveIsleQuestCredit(player, QUEST_NOT_IN_THE_FACE, 1);

                        // both spirits follow the player as personal companions until Mandori Village (areatriggers 7858/7116)
                        if (Creature* wugouCopy = player->summonCreature(NPC_WUGOU, wugou->GetSpawnPosition(), MANUAL_DESPAWN))
                            wugouCopy->getMovementManager()->moveFollow(player, PET_FOLLOW_DIST, ChaseAngle(PET_FOLLOW_ANGLE));

                        if (Creature* shuCopy = player->summonCreature(NPC_SHU_COMPANION, getCreature()->GetPosition(), MANUAL_DESPAWN))
                            shuCopy->getMovementManager()->moveFollow(player, PET_FOLLOW_DIST, ChaseAngle(-PET_FOLLOW_ANGLE));
                    }
                    wugou->Despawn(500, POOL_RESPAWN_TIME);
                    despawn(500, POOL_RESPAWN_TIME);
                }
                break;
            default:
                break;
        }
    }
};

class ShuDaiLoGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        uint32_t textId = sMySQLStore.getGossipTextIdForNpc(pObject->getEntry());
        if (sMySQLStore.getNpcGossipText(textId) == nullptr)
            textId = DefaultGossipTextId;

        GossipMenu menu(pObject->getGuid(), textId, plr->getSession()->language);
        sQuestMgr.FillQuestMenu(static_cast<Creature*>(pObject), plr, menu);

        const auto status = getIsleQuestStatus(plr, QUEST_NOT_IN_THE_FACE);
        if (status == ISLE_QUEST_INCOMPLETE || status == ISLE_QUEST_REWARDED)
            menu.addItem(GOSSIP_ICON_CHAT, 0, 1, "Can you please help us to wake up Wugou ?");

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Creature* shu = pObject->isCreature() ? static_cast<Creature*>(pObject) : nullptr;
        if (shu == nullptr)
            return;

        giveIsleQuestCredit(plr, QUEST_NOT_IN_THE_FACE, 0);
        shu->getMovementManager()->movePoint(0, shuPos1);
    }
};

void SetupWanderingIsleSingingPools(ScriptMgr* mgr)
{
    mgr->register_creature_script(NPC_SHU_POOL_OF_REFLECTION, &ShuPoolOfReflectionAI::Create);
    mgr->register_creature_script(NPC_SHU_DAI_LO, &ShuDaiLoAI::Create);
    mgr->register_creature_gossip(NPC_SHU_DAI_LO, new ShuDaiLoGossip);
}
