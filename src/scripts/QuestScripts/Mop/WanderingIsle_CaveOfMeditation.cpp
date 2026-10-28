/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

#include "WanderingIsle.hpp"

#include "Chat/ChatDefines.hpp"
#include "Movement/MovementManager.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/LocationVector.hpp"
#include "Utilities/Random.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Fu's Pond, Cave of Meditation and Li Fei's cave - quests 29410..29414, 29421, 29422.

enum CaveEntries
{
    NPC_AYSA_CLOUDSINGER = 54567,
    NPC_AYSA_MEDITATION = 59642,
    NPC_LI_FEI_SPIRIT = 54856,
    NPC_LI_FEI_QUESTGIVER = 54135,
    NPC_LI_FEI_FIGHT = 54734,
    NPC_AMBERLEAF_TROUBLEMAKER = 61801,
    NPC_HUO = 54787
};

enum CaveSpells
{
    SPELL_SUMMON_LI_FEI = 102445,
    SPELL_SEE_QUEST_INVIS_7 = 102396,
    SPELL_MEDITATION_AREA_AURA = 116421,
    SPELL_FEET_OF_FURY = 108958,
    SPELL_FLYING_SHADOW_KICK = 108944
};

enum CaveAreas
{
    AREA_CAVE_OF_MEDITATION = 5848
};

const uint32_t CAVE_RESPAWN_TIME = 120000;

//////////////////////////////////////////////////////////////////////////////////////////
// Aysa Cloudsinger (54567) at Fu's Pond - accepting 29414 sends her ahead into the cave

const LocationVector aysaSpawnPos(1206.31f, 3507.45f, 85.99f);
const LocationVector aysaJumpPos1(1196.72f, 3492.85f, 90.9836f);
const LocationVector aysaJumpPos2(1192.29f, 3478.69f, 108.788f);
const LocationVector aysaJumpPos3(1197.99f, 3460.63f, 103.04f);
const LocationVector aysaMovePos4(1176.1909f, 3444.8743f, 103.35291f);
const LocationVector aysaMovePos5(1149.9497f, 3437.1702f, 104.967064f);

class AysaCloudsingerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AysaCloudsingerAI(c); }
    explicit AysaCloudsingerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(1000);
    }

    enum Events
    {
        EVENT_AYSA_JUMP_POS_1 = 1,
        EVENT_AYSA_JUMP_POS_2,
        EVENT_AYSA_JUMP_POS_3,
        EVENT_AYSA_MOVE_POS_4,
        EVENT_AYSA_MOVE_POS_5,
        EVENT_AYSA_DESPAWN
    };

    void OnLoad() override
    {
        scriptEvents.resetEvents();
    }

    void onQuestAccept(Player* /*player*/, QuestProperties const* qst) override
    {
        if (qst->id != QUEST_THE_WAY_OF_THE_TUSHUI)
            return;

        sendChatMessage(CHAT_MSG_MONSTER_SAY, 27397, "Meet me up in the cave if you would. friend.");
        getCreature()->emote(EMOTE_ONESHOT_TALK);
        movePoint(0, aysaSpawnPos);
    }

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if ((type == POINT_MOTION_TYPE || type == EFFECT_MOTION_TYPE) && id == 0)
            scriptEvents.addEvent(EVENT_AYSA_JUMP_POS_1, 5000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_AYSA_JUMP_POS_1:
                moveJump(aysaJumpPos1, 15.0f, 15.0f, 1);
                scriptEvents.addEvent(EVENT_AYSA_JUMP_POS_2, 2000);
                break;
            case EVENT_AYSA_JUMP_POS_2:
                moveJump(aysaJumpPos2, 15.0f, 25.0f, 2);
                scriptEvents.addEvent(EVENT_AYSA_JUMP_POS_3, 2000);
                break;
            case EVENT_AYSA_JUMP_POS_3:
                moveJump(aysaJumpPos3, 15.0f, 15.0f, 3);
                scriptEvents.addEvent(EVENT_AYSA_MOVE_POS_4, 2000);
                break;
            case EVENT_AYSA_MOVE_POS_4:
                movePoint(4, aysaMovePos4);
                scriptEvents.addEvent(EVENT_AYSA_MOVE_POS_5, 2000);
                break;
            case EVENT_AYSA_MOVE_POS_5:
                movePoint(5, aysaMovePos5);
                scriptEvents.addEvent(EVENT_AYSA_DESPAWN, 4000);
                break;
            case EVENT_AYSA_DESPAWN:
                despawn(1000, CAVE_RESPAWN_TIME);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Aysa Cloudsinger (59642) meditating in the cave - quest 29414 The Way of the Tushui
// The client shows an alternate power bar filling over 90 seconds; AscEmu has no alternate power,
// the timer alone drives the event.

const LocationVector troublemakerSpawnPos[2] = { { 1184.7f, 3448.3f, 102.5f, 0.0f }, { 1186.7f, 3439.8f, 102.5f, 0.0f } };
const LocationVector troublemakerMovePos[2] = { { 1145.13f, 3432.88f, 105.268f, 0.0f }, { 1143.36f, 3437.39f, 104.973f, 0.0f } };

class AysaMeditationAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AysaMeditationAI(c); }
    explicit AysaMeditationAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(1000);
    }

    enum Events
    {
        EVENT_POWER = 1,
        EVENT_ADDS,
        EVENT_FINISHED
    };

    void OnLoad() override
    {
        reset();
    }

    // once per second (RegisterAIUpdateEvent): wait for disciples with the quest
    void AIUpdate() override
    {
        if (mStarted || getCreature()->getAreaId() != AREA_CAVE_OF_MEDITATION)
            return;

        const auto participants = getPlayersInPhase(20.0f, QUEST_THE_WAY_OF_THE_TUSHUI, ISLE_QUEST_INCOMPLETE);
        if (participants.empty())
            return;

        for (Player* player : participants)
            mParticipants.push_back(player->getGuid());

        mStarted = true;
        scriptEvents.addEvent(EVENT_POWER, 1000);
        scriptEvents.addEvent(EVENT_ADDS, 1000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mStarted)
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_POWER:
                ++mPower;
                if (mPower == 1)
                {
                    sendDBChatMessage(TEXT_AYSA_MEDITATION_START);
                    castSpellOnSelf(SPELL_SUMMON_LI_FEI);
                }

                if (mPower >= 90)
                    scriptEvents.addEvent(EVENT_FINISHED, 100);
                else
                    scriptEvents.addEvent(EVENT_POWER, 1000);
                break;
            case EVENT_ADDS:
                for (uint8_t i = 0; i < 2; ++i)
                {
                    if (Creature* troublemaker = summonCreature(NPC_AMBERLEAF_TROUBLEMAKER, troublemakerSpawnPos[i], CORPSE_TIMED_DESPAWN, 30000))
                        troublemaker->getMovementManager()->movePoint(0, troublemakerMovePos[i]);
                }
                scriptEvents.addEvent(EVENT_ADDS, 60000);
                break;
            case EVENT_FINISHED:
                sendDBChatMessage(TEXT_AYSA_MEDITATION_END);
                for (const uint64_t guid : mParticipants)
                {
                    Player* player = getCreature()->getWorldMapPlayer(guid);
                    if (player == nullptr)
                        continue;

                    player->castSpell(player, SPELL_SEE_QUEST_INVIS_7, true);
                    giveIsleQuestCredit(player, QUEST_THE_WAY_OF_THE_TUSHUI, 0);
                    player->removeAllAurasById(SPELL_MEDITATION_AREA_AURA);
                }
                reset();
                break;
            default:
                break;
        }
    }

private:
    void reset()
    {
        mStarted = false;
        mPower = 0;
        mParticipants.clear();
        scriptEvents.resetEvents();
    }

    bool mStarted = false;
    uint32_t mPower = 0;
    std::vector<uint64_t> mParticipants;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Master Li Fei (54856) - the spirit summoned by Aysa's meditation, speaks to the disciples

const LocationVector liFeiSpawnPos(1126.9323f, 3428.6692f, 105.89006f);
const LocationVector liFeiMovePos[6] =
{
    { 1131.3281f, 3437.5156f, 105.45826f },
    { 1131.8889f, 3428.2065f, 105.51409f },
    { 1130.5278f, 3425.5027f, 105.88636f },
    { 1130.5278f, 3425.5027f, 105.88636f },
    { 1129.7743f, 3433.302f, 105.531296f },
    { 1130.5573f, 3436.087f, 105.483864f }
};

class LiFeiSpiritAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LiFeiSpiritAI(c); }
    explicit LiFeiSpiritAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(1000);
    }

    enum Events
    {
        EVENT_LI_FEI_MOVE_POS_1 = 1,
        EVENT_LI_FEI_MOVE_POS_2,
        EVENT_LI_FEI_MOVE_POS_3,
        EVENT_LI_FEI_MOVE_POS_4,
        EVENT_LI_FEI_MOVE_POS_5,
        EVENT_LI_FEI_MOVE_POS_6,
        EVENT_LI_FEI_DESPAWN
    };

    void OnLoad() override
    {
        scriptEvents.resetEvents();

        if (getCreature()->getAreaId() == AREA_CAVE_OF_MEDITATION)
        {
            getCreature()->setMoveWalk(true);
            movePoint(0, liFeiSpawnPos);
        }
    }

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type == POINT_MOTION_TYPE && id == 0)
        {
            scriptEvents.addEvent(EVENT_LI_FEI_MOVE_POS_1, 10000);
            movePoint(1, liFeiMovePos[0]);
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_LI_FEI_MOVE_POS_1:
                movePoint(1, liFeiMovePos[0]);
                scriptEvents.addEvent(EVENT_LI_FEI_MOVE_POS_2, 10000);
                break;
            case EVENT_LI_FEI_MOVE_POS_2:
                speak("Master Li Fei's voice echoes, \"The way of the Tushui... enlightenment through patience and meditation... the principled life.\"");
                movePoint(2, liFeiMovePos[1]);
                scriptEvents.addEvent(EVENT_LI_FEI_MOVE_POS_3, 10000);
                break;
            case EVENT_LI_FEI_MOVE_POS_3:
                speak("Master Li Fei's voice echoes, \"It is good to see you again, Aysa. You've come with respect, and so I shall give you the answers you seek.\"");
                movePoint(3, liFeiMovePos[2]);
                scriptEvents.addEvent(EVENT_LI_FEI_MOVE_POS_4, 10000);
                break;
            case EVENT_LI_FEI_MOVE_POS_4:
                speak("Master Li Fei's voice echoes, \"Huo, the spirit of fire, is known for his hunger. He wants for tinder to eat. He needs the caress of the wind to rouse him.\"");
                movePoint(4, liFeiMovePos[3]);
                scriptEvents.addEvent(EVENT_LI_FEI_MOVE_POS_5, 10000);
                break;
            case EVENT_LI_FEI_MOVE_POS_5:
                speak("Master Li Fei's voice echoes, \"If you find these things and bring them to his cave, on the far side of Wu-Song Village, you will face a challenge within.\"");
                movePoint(5, liFeiMovePos[4]);
                scriptEvents.addEvent(EVENT_LI_FEI_MOVE_POS_6, 10000);
                break;
            case EVENT_LI_FEI_MOVE_POS_6:
                speak("Master Li Fei's voice echoes, \"Overcome that challenge, and you shall be graced by Huo's presence. Rekindle his flame, and if your spirit is pure, he shall follow you.\"");
                movePoint(6, liFeiMovePos[5]);
                scriptEvents.addEvent(EVENT_LI_FEI_DESPAWN, 10000);
                break;
            case EVENT_LI_FEI_DESPAWN:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Master Li Fei's voice echoes, \"Go, children. We shall meet again very soon.\"");
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Master Li Fei fades away.");
                despawn(1000, 0);
                break;
            default:
                break;
        }
    }

private:
    void speak(std::string const& text)
    {
        getCreature()->emote(EMOTE_ONESHOT_TALK);
        sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, text);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Master Li Fei (54135) in his cave - quests 29421 Only the Worthy Shall Pass / 29422 Huo
// The single phase value of AscEmu leaves phase 632 again when 29422 is rewarded at Huo.

class LiFeiQuestgiverAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LiFeiQuestgiverAI(c); }
    explicit LiFeiQuestgiverAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void onQuestAccept(Player* player, QuestProperties const* qst) override
    {
        if (qst->id == QUEST_ONLY_THE_WORTHY_SHALL_PASS)
            player->setPhase(PHASE_SET, PHASE_LI_FEI_FIGHT);
        else if (qst->id == QUEST_HUO_THE_SPIRIT_OF_FIRE)
            player->setPhase(PHASE_SET, PHASE_LI_FEI_DONE);
    }
};

class LiFeiFightAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LiFeiFightAI(c); }
    explicit LiFeiFightAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature) {}

    enum Events
    {
        EVENT_FEET_OF_FURY = 1,
        EVENT_FLYING_SHADOW_KICK
    };

    void OnLoad() override
    {
        mDefeated = false;
        scriptEvents.resetEvents();
    }

    void OnCombatStart(Unit* /*_target*/) override
    {
        scriptEvents.addEvent(EVENT_FEET_OF_FURY, Util::getRandomUInt(4000, 4500));
        scriptEvents.addEvent(EVENT_FLYING_SHADOW_KICK, Util::getRandomUInt(5000, 6000));
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        scriptEvents.resetEvents();
    }

    void DamageTaken(Unit* attacker, uint32_t* damage) override
    {
        if (mDefeated)
        {
            *damage = 0;
            return;
        }

        const uint32_t health = getCreature()->getHealth();
        if (*damage >= health)
            *damage = health - 1;

        if ((health - *damage) * 100 > getCreature()->getMaxHealth() * 20)
            return;

        // at 20% health the disciple has proven himself
        mDefeated = true;
        scriptEvents.resetEvents();
        _wipeHateList();
        getCreature()->getAIInterface()->attackStop();

        if (Player* player = attacker != nullptr ? attacker->getPlayerOwnerOrSelf() : nullptr)
        {
            giveIsleQuestCredit(player, QUEST_ONLY_THE_WORTHY_SHALL_PASS, 0);
            player->setPhase(PHASE_SET, PHASE_LI_FEI_DONE);
        }

        despawn(1000, CAVE_RESPAWN_TIME);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat() || mDefeated)
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        Unit* victim = getCreature()->getAIInterface()->getCurrentTarget();
        if (victim == nullptr)
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_FEET_OF_FURY:
                castSpell(victim, SPELL_FEET_OF_FURY);
                scriptEvents.addEvent(EVENT_FEET_OF_FURY, Util::getRandomUInt(10000, 10500));
                break;
            case EVENT_FLYING_SHADOW_KICK:
                castSpell(victim, SPELL_FLYING_SHADOW_KICK);
                scriptEvents.addEvent(EVENT_FLYING_SHADOW_KICK, Util::getRandomUInt(11000, 12000));
                break;
            default:
                break;
        }
    }

private:
    bool mDefeated = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Huo (54787) - rewarding 29422 ends the cave phases, accepting 29423 blesses the player

class HuoAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HuoAI(c); }
    explicit HuoAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void onQuestRewarded(Player* player, QuestProperties const* qst) override
    {
        if (qst->id == QUEST_HUO_THE_SPIRIT_OF_FIRE && player->GetPhase() == PHASE_LI_FEI_DONE)
            player->setPhase(PHASE_SET, PHASE_DEFAULT_PLAYER);
    }

    void onQuestAccept(Player* player, QuestProperties const* qst) override
    {
        if (qst->id != QUEST_THE_PASSION_OF_SHEN_ZIN_SU)
            return;

        player->castSpell(player, 102630, true);    // Blessing of Huo (summons the companion)
        player->castSpell(player, 128700, true);
    }
};

void SetupWanderingIsleCaveOfMeditation(ScriptMgr* mgr)
{
    mgr->register_creature_script(NPC_AYSA_CLOUDSINGER, &AysaCloudsingerAI::Create);
    mgr->register_creature_script(NPC_AYSA_MEDITATION, &AysaMeditationAI::Create);
    mgr->register_creature_script(NPC_LI_FEI_SPIRIT, &LiFeiSpiritAI::Create);
    mgr->register_creature_script(NPC_LI_FEI_QUESTGIVER, &LiFeiQuestgiverAI::Create);
    mgr->register_creature_script(NPC_LI_FEI_FIGHT, &LiFeiFightAI::Create);
    mgr->register_creature_script(NPC_HUO, &HuoAI::Create);
}
