/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

#include "WanderingIsle.hpp"

#include "Chat/ChatDefines.hpp"
#include "Movement/MovementManager.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/LocationVector.hpp"
#include "Utilities/MathConstants.hpp"

#include <string>

//////////////////////////////////////////////////////////////////////////////////////////
// Elders' Path and the Wood of Staves - quests 29787 Worthy of Passing, 29790 Passing Wisdom
// and 29791 The Suffering of Shen-zin Su.

enum WoodEntries
{
    NPC_MASTER_SHANG_XI_WORTHY_QUESTGIVER = 55586,
    NPC_MASTER_SHANG_XI_WORTHY_ESCORT = 56159,
    NPC_GUARDIAN_OF_THE_ELDERS = 56274,
    NPC_MASTER_SHANG_XI_WOOD_OF_STAVES = 55672,
    NPC_WALKING_STICK = 57874,
    NPC_PLANTING_STAVE_CREDIT = 56688,
    NPC_HOT_AIR_BALLOON = 55918,
    NPC_HOT_AIR_BALLOON_VEHICLE = 55649,
    NPC_AYSA_BALLOON = 56661,
    NPC_JI_BALLOON = 56663,
    NPC_SHEN_ZIN_SU = 57769,
    NPC_BOARD_HOT_AIR_BALLOON_CREDIT = 56378,
    NPC_SPEAK_WITH_SHEN_ZIN_SU_CREDIT = 55939,
    GO_ELDERS_GATE = 209922
};

static std::string replacePlayerName(std::string text, std::string const& token, Player* player)
{
    const auto pos = text.find(token);
    if (pos != std::string::npos)
        text.replace(pos, token.length(), player->getName());
    return text;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Master Shang Xi (55586) at the Elders' Path - accepting 29787 sends his escort copy ahead

class MasterShangXiWorthyQuestgiverAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MasterShangXiWorthyQuestgiverAI(c); }
    explicit MasterShangXiWorthyQuestgiverAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void onQuestAccept(Player* /*player*/, QuestProperties const* qst) override
    {
        if (qst->id != QUEST_WORTHY_OF_PASSING)
            return;

        if (Creature* escort = summonCreature(NPC_MASTER_SHANG_XI_WORTHY_ESCORT, getCreature()->GetPosition(), MANUAL_DESPAWN))
            escort->getMovementManager()->movePoint(0, escort->GetPosition());
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Master Shang Xi (56159) walking the Elders' Path with the disciple

const LocationVector worthyPos[11] =
{
    { 776.52606f, 4178.652f, 206.94368f },
    { 792.8524f, 4185.137f, 208.41711f },
    { 827.27637f, 4205.244f, 199.61674f },
    { 839.7535f, 4215.7915f, 197.77382f },
    { 845.00696f, 4267.5884f, 196.7384f },
    { 843.81946f, 4301.1743f, 210.9836f },
    { 843.26215f, 4339.1035f, 223.98082f },
    { 828.6528f, 4353.315f, 223.98082f },
    { 830.25867f, 4368.5503f, 223.94623f },
    { 845.59398f, 4372.6298f, 224.06399f },
    { 844.69406f, 4401.2700f, 237.26740f }
};
const LocationVector worthyEndPos(874.10767f, 4459.6313f, 241.18892f);

class MasterShangXiWorthyOfPassingAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MasterShangXiWorthyOfPassingAI(c); }
    explicit MasterShangXiWorthyOfPassingAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(1000);
    }

    enum Events
    {
        EVENT_WORTHY_TALK1 = 1,
        EVENT_WORTHY_MOVE_POS1,
        EVENT_WORTHY_MOVE_POS2,
        EVENT_WORTHY_MOVE_POS3,
        EVENT_WORTHY_MOVE_POS4,
        EVENT_WORTHY_MOVE_POS5,
        EVENT_WORTHY_TALK2,
        EVENT_WORTHY_MOVE_POS6,
        EVENT_WORTHY_TALK3,
        EVENT_WORTHY_MOVE_POS7,
        EVENT_WORTHY_TALK4,
        EVENT_WORTHY_MOVE_POS8,
        EVENT_WORTHY_MOVE_POS9,
        EVENT_WORTHY_MOVE_POS10,
        EVENT_WORTHY_MOVE_POS11
    };

    void OnLoad() override
    {
        scriptEvents.resetEvents();
    }

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (id == 0)
            scriptEvents.addEvent(EVENT_WORTHY_TALK1, 5000);
        else if (id == 10)
            scriptEvents.addEvent(EVENT_WORTHY_MOVE_POS9, 1000);    // the Guardian of the Elders has fallen
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_WORTHY_TALK1:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Come child. We have one final journey to take together before your training is complete.");
                scriptEvents.addEvent(EVENT_WORTHY_MOVE_POS1, 5000);
                break;
            case EVENT_WORTHY_MOVE_POS1:
                movePoint(1, worthyPos[0]);
                scriptEvents.addEvent(EVENT_WORTHY_MOVE_POS2, 5000);
                break;
            case EVENT_WORTHY_MOVE_POS2:
                movePoint(2, worthyPos[1]);
                scriptEvents.addEvent(EVENT_WORTHY_MOVE_POS3, 5000);
                break;
            case EVENT_WORTHY_MOVE_POS3:
                movePoint(3, worthyPos[2]);
                scriptEvents.addEvent(EVENT_WORTHY_MOVE_POS4, 5000);
                break;
            case EVENT_WORTHY_MOVE_POS4:
                movePoint(4, worthyPos[3]);
                scriptEvents.addEvent(EVENT_WORTHY_MOVE_POS5, 5000);
                break;
            case EVENT_WORTHY_MOVE_POS5:
                movePoint(5, worthyPos[4]);
                scriptEvents.addEvent(EVENT_WORTHY_TALK2, 5000);
                break;
            case EVENT_WORTHY_TALK2:
                getCreature()->setMoveWalk(true);
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Beyond the Elders' Path lies the Wood of Staves, a sacred place that only the worthy may enter.");
                scriptEvents.addEvent(EVENT_WORTHY_MOVE_POS6, 5000);
                break;
            case EVENT_WORTHY_MOVE_POS6:
                movePoint(6, worthyPos[5]);
                scriptEvents.addEvent(EVENT_WORTHY_TALK3, 15000);
                break;
            case EVENT_WORTHY_TALK3:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Of the many ways to prove your worth, I require the simplest of you now. I must know that you will fight for our people. I must know that you can keep them safe.");
                scriptEvents.addEvent(EVENT_WORTHY_MOVE_POS7, 10000);
                break;
            case EVENT_WORTHY_MOVE_POS7:
                movePoint(7, worthyPos[6]);
                scriptEvents.addEvent(EVENT_WORTHY_TALK4, 20000);
                break;
            case EVENT_WORTHY_TALK4:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Defeat the Guardian of the Elders, and we may pass.");
                movePoint(8, worthyPos[7]);
                scriptEvents.addEvent(EVENT_WORTHY_MOVE_POS8, 5000);
                break;
            case EVENT_WORTHY_MOVE_POS8:
                movePoint(9, worthyPos[8]);
                break;
            case EVENT_WORTHY_MOVE_POS9:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "You've become strong indeed, child. This is good. You will need that strength soon.");
                movePoint(11, worthyPos[9]);
                scriptEvents.addEvent(EVENT_WORTHY_MOVE_POS10, 3000);
                break;
            case EVENT_WORTHY_MOVE_POS10:
                movePoint(12, worthyPos[10]);
                scriptEvents.addEvent(EVENT_WORTHY_MOVE_POS11, 5000);
                break;
            case EVENT_WORTHY_MOVE_POS11:
                movePoint(13, worthyEndPos);
                despawn(10000, 0);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Guardian of the Elders (56274) - its death opens the gate to the Wood of Staves

class GuardianOfTheEldersAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GuardianOfTheEldersAI(c); }
    explicit GuardianOfTheEldersAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature) {}

    void OnDied(Unit* /*_killer*/) override
    {
        GameObject* gate = findNearestGameObject(GO_ELDERS_GATE, 40.0f);
        if (gate == nullptr)
            return;

        useDoorOrButton(gate, 60000);

        if (Creature* masterShangXi = findNearestCreatureInPhase(NPC_MASTER_SHANG_XI_WORTHY_ESCORT, 40.0f))
        {
            masterShangXi->setMoveWalk(false);
            masterShangXi->getMovementManager()->movePoint(10, masterShangXi->GetPosition());
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Master Shang Xi (55672) in the Wood of Staves - quest 29790 Passing Wisdom

const LocationVector woodsPos[6] =
{
    { 871.0573f, 4460.548f, 241.33667f },
    { 868.00696f, 4464.8384f, 241.60161f },
    { 869.67883f, 4467.752f, 241.66815f },
    { 872.2448f, 4467.272f, 241.60721f },
    { 872.7934f, 4465.126f, 241.33447f },
    { 874.205f, 4464.75f, 241.35117f }
};

class MasterShangXiWoodOfStavesAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MasterShangXiWoodOfStavesAI(c); }
    explicit MasterShangXiWoodOfStavesAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(1000);
    }

    enum Events
    {
        EVENT_WOODS_TALK1 = 1,
        EVENT_WOODS_TALK2,
        EVENT_WOODS_MOVE_POS1,
        EVENT_WOODS_SET_FACING1,
        EVENT_WOODS_TALK3,
        EVENT_WOODS_MOVE_POS2,
        EVENT_WOODS_MOVE_POS3,
        EVENT_WOODS_MOVE_POS4,
        EVENT_WOODS_MOVE_POS5,
        EVENT_WOODS_TALK4,
        EVENT_WOODS_MOVE_POS6,
        EVENT_WOODS_TALK5,
        EVENT_WOODS_TALK6,
        EVENT_WOODS_KNEEL,
        EVENT_WOODS_CREDIT,
        EVENT_WOODS_RESET
    };

    void OnLoad() override
    {
        scriptEvents.resetEvents();
    }

    void onQuestAccept(Player* /*player*/, QuestProperties const* qst) override
    {
        if (qst->id != QUEST_PASSING_WISDOM)
            return;

        getCreature()->setMoveWalk(true);
        movePoint(0, getCreature()->GetPosition());
    }

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type == POINT_MOTION_TYPE && id == 0)
            scriptEvents.addEvent(EVENT_WOODS_TALK1, 1000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_WOODS_TALK1:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "For 3,000 years, we have passed the knowledge of our people down. Elder to youth. Master to student.");
                scriptEvents.addEvent(EVENT_WOODS_TALK2, 10000);
                break;
            case EVENT_WOODS_TALK2:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Every elder reaches the day where he must pass on and plant his stave with the staves of his ancestors. Today is the day when my staff joins these woods.");
                scriptEvents.addEvent(EVENT_WOODS_MOVE_POS1, 6000);
                break;
            case EVENT_WOODS_MOVE_POS1:
                movePoint(1, woodsPos[0]);
                scriptEvents.addEvent(EVENT_WOODS_SET_FACING1, 2000);
                break;
            case EVENT_WOODS_SET_FACING1:
                getCreature()->setFacing(-AscEmu::Math::PiF * 1.75f);
                scriptEvents.addEvent(EVENT_WOODS_TALK3, 8000);
                break;
            case EVENT_WOODS_TALK3:
                for (Player* player : getPlayersInPhase(15.0f))
                    getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, replacePlayerName("$p, our people have lived the wholes of their lives on this great turtle, Shen-zin Su, but not in hundreds of years has anyone spoken to him.", "$p", player), player);
                scriptEvents.addEvent(EVENT_WOODS_MOVE_POS2, 4000);
                break;
            case EVENT_WOODS_MOVE_POS2:
                movePoint(2, woodsPos[1]);
                scriptEvents.addEvent(EVENT_WOODS_MOVE_POS3, 1000);
                break;
            case EVENT_WOODS_MOVE_POS3:
                movePoint(3, woodsPos[2]);
                scriptEvents.addEvent(EVENT_WOODS_MOVE_POS4, 1000);
                break;
            case EVENT_WOODS_MOVE_POS4:
                movePoint(4, woodsPos[3]);
                scriptEvents.addEvent(EVENT_WOODS_MOVE_POS5, 1000);
                break;
            case EVENT_WOODS_MOVE_POS5:
                movePoint(5, woodsPos[4]);
                scriptEvents.addEvent(EVENT_WOODS_TALK4, 1000);
                break;
            case EVENT_WOODS_TALK4:
                if (Creature* staff = summonCreature(NPC_WALKING_STICK, 873.09375f, 4462.259765625f, 241.41162109375f, 3.804818391799926757f, TIMED_OR_CORPSE_DESPAWN, 70000))
                    getCreature()->setFacingToObject(staff);
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Now Shen-zin Su is ill, and we are all in danger. With the help of the elements, you will break the silence. You will speak to him.");
                scriptEvents.addEvent(EVENT_WOODS_MOVE_POS6, 5000);
                break;
            case EVENT_WOODS_MOVE_POS6:
                movePoint(6, woodsPos[5]);
                scriptEvents.addEvent(EVENT_WOODS_TALK5, 2000);
                break;
            case EVENT_WOODS_TALK5:
                getCreature()->setFacing(AscEmu::Math::PiF * 1.25f);
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Aysa and Ji have retrieved the spirits and brought them here. You are to go with them, speak to the great Shen-zin Su, and do what must be done to save our people.");
                scriptEvents.addEvent(EVENT_WOODS_TALK6, 5000);
                break;
            case EVENT_WOODS_TALK6:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "You've come far, my young student. I see within you a great hero. I leave the fate of this land to you.");
                scriptEvents.addEvent(EVENT_WOODS_KNEEL, 5000);
                break;
            case EVENT_WOODS_KNEEL:
                getCreature()->emote(EMOTE_ONESHOT_KNEEL);
                scriptEvents.addEvent(EVENT_WOODS_CREDIT, 2000);
                break;
            case EVENT_WOODS_CREDIT:
                for (Player* player : getPlayersInPhase(15.0f))
                    giveIsleQuestCredit(player, QUEST_PASSING_WISDOM, 0);
                scriptEvents.addEvent(EVENT_WOODS_RESET, 60000);
                break;
            case EVENT_WOODS_RESET:
                movePoint(7, getCreature()->GetSpawnPosition());
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shang Xi's Hot Air Balloon (55918) - clicking summons the vehicle with Ji and Aysa on board

class HotAirBalloonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HotAirBalloonAI(c); }
    explicit HotAirBalloonAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSpellClick(Unit* clicker, bool /*spellClickHandled*/) override
    {
        Player* player = clicker != nullptr && clicker->isPlayer() ? static_cast<Player*>(clicker) : nullptr;
        if (player == nullptr)
            return;

        const LocationVector home = getCreature()->GetSpawnPosition();
        Creature* vehicle = summonCreature(NPC_HOT_AIR_BALLOON_VEHICLE, home, MANUAL_DESPAWN);
        if (vehicle == nullptr)
            return;

        player->callEnterVehicle(vehicle, 0);

        if (Creature* ji = summonCreature(NPC_JI_BALLOON, home, MANUAL_DESPAWN))
        {
            ji->callEnterVehicle(vehicle, 1);
            ji->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, replacePlayerName("$n, where's Master Shang?", "$n", player), player);
            ji->PlaySoundToSet(27297);
        }

        if (Creature* aysa = summonCreature(NPC_AYSA_BALLOON, home, MANUAL_DESPAWN))
            aysa->callEnterVehicle(vehicle, 2);

        vehicle->getMovementManager()->movePoint(0, home);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The balloon ride (55649) - flies along the shell while Aysa, Ji and Shen-zin Su talk

const LocationVector balloonPos[25] =
{
    { 915.123f, 4564.1523f, 231.37447f },
    { 922.3584f, 4567.7495f, 234.48523f },
    { 954.0022f, 4578.619f, 230.6169f },
    { 1005.63586f, 4599.781f, 219.48938f },
    { 1035.7178f, 4621.3955f, 205.60277f },
    { 1056.7085f, 4664.024f, 186.15259f },
    { 1079.8335f, 4795.34f, 157.87029f },
    { 1091.337f, 4865.452f, 144.9579f },
    { 1090.1652f, 4928.4194f, 138.01312f },
    { 1062.7473f, 5065.7837f, 137.5515f },
    { 992.0422f, 5163.364f, 137.56874f },
    { 885.60614f, 5206.965f, 135.15846f },
    { 779.9042f, 5208.6396f, 135.60544f },
    { 736.7593f, 5192.4f, 137.05084f },
    { 649.841f, 5145.8667f, 141.09795f },
    { 623.2541f, 5131.7183f, 142.2216f },
    { 560.43494f, 5053.6377f, 132.38977f },
    { 485.70917f, 4949.409f, 125.14896f },
    { 429.59842f, 4820.912f, 110.39427f },
    { 305.63647f, 4435.072f, 79.58694f },
    { 156.1306f, 4266.2583f, 116.03974f },
    { 112.64921f, 4032.78f, 125.91718f },
    { 203.35933f, 3835.7214f, 136.13402f },
    { 395.93735f, 3764.5327f, 160.51057f },
    { 744.496f, 3664.6455f, 193.9989f }
};

class HotAirBalloonVehicleAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HotAirBalloonVehicleAI(c); }
    explicit HotAirBalloonVehicleAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(1000);
    }

    // one step per waypoint with the speech timings of the ride
    struct RideStep
    {
        uint32_t speaker;       // 0 = nobody talks at this waypoint
        uint8_t chatType;
        uint32_t sound;
        const char* text;
        uint32_t nextDelay;
    };

    void OnLoad() override
    {
        mStep = 0;
        scriptEvents.resetEvents();
    }

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type == POINT_MOTION_TYPE && id == 0)
            scriptEvents.addEvent(EVENT_RIDE_STEP, 1000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_RIDE_STEP:
            {
                static const RideStep rideSteps[25] =
                {
                    { 0, 0, 0, "", 1000 },
                    { 0, 0, 0, "", 2000 },
                    { NPC_AYSA_BALLOON, CHAT_MSG_MONSTER_SAY, 27431, "...Ji, were in the Wood of Staves. You know where Master Shang is now.", 8000 },
                    { NPC_JI_BALLOON, CHAT_MSG_MONSTER_SAY, 27298, "Bah, let a pandaren hope, would you? I'm going to miss the old man.", 12000 },
                    { NPC_AYSA_BALLOON, CHAT_MSG_MONSTER_SAY, 27432, "Ji, be respectful when we speak to Shen-zin Su.", 4000 },
                    { NPC_JI_BALLOON, CHAT_MSG_MONSTER_SAY, 27299, "When am I not respectful? You hurt me, Aysa.", 5000 },
                    { NPC_AYSA_BALLOON, CHAT_MSG_MONSTER_SAY, 27433, "I might if you embarrass us.", 8000 },
                    { NPC_AYSA_BALLOON, CHAT_MSG_MONSTER_YELL, 27434, "Shen-zin Su, we are the descendants of Liu Lang. We've sensed your pain, and we want to help.", 12000 },
                    { NPC_AYSA_BALLOON, CHAT_MSG_MONSTER_YELL, 27435, "What ails you Shen-zin Su? What can we do?", 7000 },
                    { NPC_SHEN_ZIN_SU, CHAT_MSG_MONSTER_SAY, 27435, "I am in pain, but it warms my heart that Liu Lang's grandchildren have not forgotten me.", 15000 },
                    { NPC_SHEN_ZIN_SU, CHAT_MSG_MONSTER_SAY, 27823, "There is a thorn in my side.  I cannot remove it.", 14000 },
                    { NPC_SHEN_ZIN_SU, CHAT_MSG_MONSTER_SAY, 27824, "The pain is unbearable, and I can no longer swim straight.", 15000 },
                    { NPC_SHEN_ZIN_SU, CHAT_MSG_MONSTER_SAY, 27825, "Please grandchildren, can you remove this thorn?  I cannot do so on my own.", 15000 },
                    { NPC_AYSA_BALLOON, CHAT_MSG_MONSTER_YELL, 27436, "Of course, Shen-zin Su!  But your shell is large, and I do not know where this thorn could be.", 7000 },
                    { NPC_SHEN_ZIN_SU, CHAT_MSG_MONSTER_SAY, 27826, "It is in the forest where your feet do not walk.  Continue along the mountains and you will find it.", 18000 },
                    { NPC_AYSA_BALLOON, CHAT_MSG_MONSTER_YELL, 27437, "We will find it, and we will remove it.  You have our word!", 8000 },
                    { NPC_SHEN_ZIN_SU, CHAT_MSG_MONSTER_SAY, 27827, "Thank you, grandchildren.", 5000 },
                    { NPC_JI_BALLOON, CHAT_MSG_MONSTER_SAY, 27300, "A thorn?  And I left my tweezers at home.", 7000 },
                    { NPC_JI_BALLOON, CHAT_MSG_MONSTER_SAY, 27301, "How could such a thing cause pain to something so large?", 7000 },
                    { NPC_AYSA_BALLOON, CHAT_MSG_MONSTER_SAY, 27438, "We'll know soon enough.", 14000 },
                    { NPC_JI_BALLOON, CHAT_MSG_MONSTER_SAY, 27302, "Are you seeing what I'm seeing?!  Is that a boat?!", 6000 },
                    { NPC_AYSA_BALLOON, CHAT_MSG_MONSTER_SAY, 27439, "It is a boat... a whole airship!  That's a bigger thorn than I was expecting.", 8000 },
                    { NPC_JI_BALLOON, CHAT_MSG_MONSTER_SAY, 27303, "And those aren't pandaren down there.  They've got no fur.", 7000 },
                    { NPC_AYSA_BALLOON, CHAT_MSG_MONSTER_SAY, 27440, "Someone has crashed into our island.  Removing this thorn may be more complicated than we thought.", 8000 },
                    { NPC_AYSA_BALLOON, CHAT_MSG_MONSTER_SAY, 27441, "We should let Elder Shaopai know and then plan our next move.", 20000 }
                };

                if (mStep >= 25)
                    break;

                const RideStep& step = rideSteps[mStep];

                // the balloon speeds up after Shen-zin Su has spoken
                if (mStep == 17)
                    getCreature()->setSpeedRate(TYPE_RUN, 4.0f, true);

                movePoint(mStep + 1, balloonPos[mStep]);

                if (step.speaker != 0)
                {
                    const float range = step.speaker == NPC_SHEN_ZIN_SU ? 90.0f : 15.0f;
                    if (Creature* speaker = findNearestCreatureInPhase(step.speaker, range))
                    {
                        speaker->sendChatMessage(step.chatType, LANG_UNIVERSAL, step.text);
                        speaker->PlaySoundToSet(step.sound);
                    }
                }

                ++mStep;
                scriptEvents.addEvent(mStep < 25 ? EVENT_RIDE_STEP : EVENT_RIDE_END, step.nextDelay);
                break;
            }
            case EVENT_RIDE_END:
            {
                for (Player* player : getPlayersInPhase(15.0f))
                {
                    giveIsleQuestCredit(player, QUEST_THE_SUFFERING_OF_SHEN_ZIN_SU, 0);
                    giveIsleQuestCredit(player, QUEST_THE_SUFFERING_OF_SHEN_ZIN_SU, 1);
                }

                if (Creature* aysa = findNearestCreatureInPhase(NPC_AYSA_BALLOON, 15.0f))
                    aysa->Despawn(500, 0);
                if (Creature* ji = findNearestCreatureInPhase(NPC_JI_BALLOON, 15.0f))
                    ji->Despawn(500, 0);
                despawn(1000, 0);
                break;
            }
            default:
                break;
        }
    }

private:
    enum Events
    {
        EVENT_RIDE_STEP = 1,
        EVENT_RIDE_END
    };

    uint8_t mStep = 0;
};

void SetupWanderingIsleWoodOfStaves(ScriptMgr* mgr)
{
    mgr->register_creature_script(NPC_MASTER_SHANG_XI_WORTHY_QUESTGIVER, &MasterShangXiWorthyQuestgiverAI::Create);
    mgr->register_creature_script(NPC_MASTER_SHANG_XI_WORTHY_ESCORT, &MasterShangXiWorthyOfPassingAI::Create);
    mgr->register_creature_script(NPC_GUARDIAN_OF_THE_ELDERS, &GuardianOfTheEldersAI::Create);
    mgr->register_creature_script(NPC_MASTER_SHANG_XI_WOOD_OF_STAVES, &MasterShangXiWoodOfStavesAI::Create);
    mgr->register_creature_script(NPC_HOT_AIR_BALLOON, &HotAirBalloonAI::Create);
    mgr->register_creature_script(NPC_HOT_AIR_BALLOON_VEHICLE, &HotAirBalloonVehicleAI::Create);
}
