/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

#include "WanderingIsle.hpp"

#include "Management/ItemInterface.h"
#include "Movement/MovementManager.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Spell/Definitions/AuraEffects.hpp"
#include "Utilities/LocationVector.hpp"
#include "Utilities/Random.hpp"

#include <cmath>

//////////////////////////////////////////////////////////////////////////////////////////
// Shang Xi Academy - quests 29404..29409 and 29524.

// Jaomin Ro's spar faction template is friendly to players, challengers need him attackable
enum AcademyFactions
{
    FACTION_JAOMIN_SPAR = 2104,
    FACTION_NEUTRAL_CREATURE = 7
};

enum AcademyEntries
{
    NPC_ASPIRING_TRAINEE = 53565,
    NPC_ASPIRING_TRAINEE_2 = 65469,
    NPC_TUSHUI_TRAINEE = 54587,
    NPC_TUSHUI_TRAINEE_2 = 65471,
    NPC_HUOJIN_TRAINEE = 54586,
    NPC_HUOJIN_TRAINEE_2 = 65470,
    NPC_MASTER_SHANG_XI = 53566,
    NPC_INSTRUCTOR_QUN = 57748,
    NPC_INSTRUCTOR_ZHI = 61411,
    NPC_QUIET_LAM = 57752,
    NPC_IRONFIST_ZHOU = 57753,
    NPC_TRAINING_TARGET = 57873,
    NPC_TRAINING_TARGET_DESTRUCTIBLE = 53714,
    NPC_THE_MASTERS_FLAME = 59591,
    NPC_JAOMIN_RO = 54611,
    NPC_JAOMIN_HAWK = 57750
};

enum AcademyDisplays
{
    JAOMIN_HAWK_DISPLAY = 39796
};

enum AcademySpells
{
    SPELL_JAB = 108967,
    SPELL_CREATE_MASTERS_FLAME = 114611,
    SPELL_JAOMIN_JUMP = 108938,
    SPELL_JAOMIN_JUMP_DAMAGE = 108937,
    SPELL_JAOMIN_FALCON = 108955,
    SPELL_JAOMIN_FALCON_STUN = 108971
};

enum AcademyActions
{
    ACTION_TRAINEE_JAB_LOOP = 1,        // trainees start sparring
    ACTION_TRAINEE_TALK = 2,            // trainees stop sparring and comment
    ACTION_TRAINEE_EMOTE_BASE = 30,     // + move index 1..5 shown by Instructor Qun
    ACTION_TUSHUI_EMOTE_BASE = 0        // + move index 1..3 shown by Instructor Zhi
};

// The Lesson of the Iron Bough (one quest per class): the client start outfit already contains the class weapons
// of the weapon racks. They are taken away on accept so the player loots them from the racks and equips them,
// the equip spells of the weapons give the credits (54139 main hand, 57849 off hand).
struct AcademyClassWeapon
{
    uint32_t quest;
    uint32_t mainHandItem;
    uint32_t offHandItem;   // 0 = single weapon
};

const AcademyClassWeapon academyClassWeapons[7] =
{
    { 30038, 73210, 0 },        // warrior: Trainee's Sword
    { 30033, 76390, 76392 },    // mage: Trainee's Spellblade, Trainee's Hand Fan
    { 30034, 73211, 0 },        // hunter: Trainee's Crossbow
    { 30035, 73207, 76393 },    // priest: Trainee's Mace, Trainee's Book of Prayers
    { 30036, 73208, 73212 },    // rogue: two Trainee's Daggers
    { 30037, 76391, 73213 },    // shaman: Trainee's Axe, Trainee's Shield
    { 30027, 73209, 0 }         // monk: Trainee's Staff
};

const uint32_t TRAINEE_RESPAWN_TIME = 120000;
const uint32_t traineeSounds[3] = { 33643, 33645, 33646 };
const uint32_t traineeEmotes[4] = { 507, 509, 511, 543 };
const uint32_t qunEmotes[5] = { 507, 509, 511, 543, 508 };
const uint32_t zhiEmotes[3] = { 507, 508, 509 };

static void playRandomTraineeEmote(Creature* creature)
{
    creature->emote(static_cast<EmoteType>(traineeEmotes[Util::getRandomUInt(3)]));
    creature->PlaySoundToSet(traineeSounds[Util::getRandomUInt(2)]);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Aspiring Trainee (53565, 65469)
// Three spawn variants: trainees hitting a training target, one talking
// pair which starts sparring after the chat, and idle trainees which only mimic Instructor Qun.

class AspiringTraineeAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AspiringTraineeAI(c); }
    explicit AspiringTraineeAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(1000);
    }

    enum Events
    {
        EVENT_TRAIN = 1,
        EVENT_CONVERSATION,
        EVENT_CONVERSATION_END,
        EVENT_JAB
    };

    void OnLoad() override
    {
        scriptEvents.resetEvents();
        mConversationLine = 0;
        mJabLoop = false;

        if (findNearestCreatureInPhase(NPC_TRAINING_TARGET, 5.0f) != nullptr)
        {
            scriptEvents.addEvent(EVENT_TRAIN, Util::getRandomUInt(1000, 5000));
            return;
        }

        // the eastern trainee of a pair standing together is the speaker
        if (Creature* partner = findNearestCreatureInPhase(getCreature()->getEntry(), 3.0f))
        {
            if (partner->GetPositionX() < getCreature()->GetPositionX())
                scriptEvents.addEvent(EVENT_CONVERSATION, 20000);
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_TRAIN:
            {
                playRandomTraineeEmote(getCreature());
                if (Creature* target = findNearestCreatureInPhase(NPC_TRAINING_TARGET, 5.0f))
                    castSpell(target, SPELL_JAB, true);
                scriptEvents.addEvent(EVENT_TRAIN, Util::getRandomUInt(6000, 7000));
                break;
            }
            case EVENT_CONVERSATION:
            {
                // chat lines 1..4, delays 16/6/11/6 seconds
                const uint32_t delays[4] = { 16000, 6000, 11000, 6000 };
                sendDBChatMessage(TEXT_ASPIRING_TRAINEE_CHAT_FIRST + mConversationLine);
                const uint32_t delay = delays[mConversationLine];
                ++mConversationLine;
                scriptEvents.addEvent(mConversationLine < 4 ? EVENT_CONVERSATION : EVENT_CONVERSATION_END, delay);
                break;
            }
            case EVENT_CONVERSATION_END:
            {
                DoAction(ACTION_TRAINEE_JAB_LOOP);
                std::list<Creature*> partners;
                getCreaturesInPhase(getCreature()->getEntry(), 3.0f, partners);
                for (Creature* partner : partners)
                {
                    if (partner->GetScript() != nullptr)
                        partner->GetScript()->DoAction(ACTION_TRAINEE_JAB_LOOP);
                }
                break;
            }
            case EVENT_JAB:
            {
                playRandomTraineeEmote(getCreature());
                if (Creature* partner = findNearestCreatureInPhase(getCreature()->getEntry(), 4.0f))
                    castSpell(partner, SPELL_JAB, true);
                scriptEvents.addEvent(EVENT_JAB, 5000);
                break;
            }
            default:
                break;
        }
    }

    void DoAction(int32_t action) override
    {
        switch (action)
        {
            case ACTION_TRAINEE_JAB_LOOP:
            {
                if (mJabLoop)
                    return;

                mJabLoop = true;
                getCreature()->setEmoteState(EMOTE_STATE_MONKOFFENSE_READYUNARMED);
                scriptEvents.addEvent(EVENT_JAB, 1000);
                break;
            }
            case ACTION_TRAINEE_TALK:
            {
                mJabLoop = false;
                scriptEvents.removeEvent(EVENT_JAB);
                getCreature()->setEmoteState(0);

                if (getCreature()->getEntry() == NPC_ASPIRING_TRAINEE)
                    sendDBChatMessage(TEXT_ASPIRING_TRAINEE_TARGET_FIRST + Util::getRandomUInt(7));
                else
                    sendDBChatMessage(TEXT_ASPIRING_TRAINEE_2_TARGET_FIRST + Util::getRandomUInt(3));
                break;
            }
            default:
            {
                if (action > ACTION_TRAINEE_EMOTE_BASE && action <= ACTION_TRAINEE_EMOTE_BASE + 5)
                {
                    getCreature()->emote(static_cast<EmoteType>(qunEmotes[action - ACTION_TRAINEE_EMOTE_BASE - 1]));
                    getCreature()->PlaySoundToSet(traineeSounds[Util::getRandomUInt(2)]);
                }
                break;
            }
        }
    }

private:
    uint8_t mConversationLine = 0;
    bool mJabLoop = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Training Target (53714) - quest 29406 The Lesson of the Sandy Fist
// Stands still, never fights back and can be destroyed by the player.

class DestructibleTrainingTargetAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DestructibleTrainingTargetAI(c); }
    explicit DestructibleTrainingTargetAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        getCreature()->setControlled(true, UNIT_STATE_STUNNED);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Instructor Qun (57748) - shows a move every 5 seconds, the trainees around him repeat it

class InstructorQunAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new InstructorQunAI(c); }
    explicit InstructorQunAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(5000);
    }

    void AIUpdate() override
    {
        const uint32_t move = Util::getRandomUInt(1, 5);
        getCreature()->emote(static_cast<EmoteType>(qunEmotes[move - 1]));
        getCreature()->PlaySoundToSet(traineeSounds[Util::getRandomUInt(2)]);

        std::list<Creature*> trainees;
        getCreaturesInPhase(NPC_ASPIRING_TRAINEE, 15.0f, trainees);
        getCreaturesInPhase(NPC_ASPIRING_TRAINEE_2, 15.0f, trainees);
        for (Creature* trainee : trainees)
        {
            if (trainee->GetScript() != nullptr)
                trainee->GetScript()->DoAction(ACTION_TRAINEE_EMOTE_BASE + move);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Instructor Zhi (61411) - same for the Tushui trainees, who follow one second later

class InstructorZhiAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new InstructorZhiAI(c); }
    explicit InstructorZhiAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(1000);
        scriptEvents.addEvent(EVENT_SHOW_MOVE, 5000);
    }

    enum Events
    {
        EVENT_SHOW_MOVE = 1,
        EVENT_TRAINEES_REPEAT
    };

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SHOW_MOVE:
                mMove = Util::getRandomUInt(1, 3);
                getCreature()->emote(static_cast<EmoteType>(zhiEmotes[mMove - 1]));
                scriptEvents.addEvent(EVENT_TRAINEES_REPEAT, 1000);
                scriptEvents.addEvent(EVENT_SHOW_MOVE, 5000);
                break;
            case EVENT_TRAINEES_REPEAT:
            {
                std::list<Creature*> trainees;
                getCreaturesInPhase(NPC_TUSHUI_TRAINEE, 10.0f, trainees);
                getCreaturesInPhase(NPC_TUSHUI_TRAINEE_2, 10.0f, trainees);
                for (Creature* trainee : trainees)
                {
                    if (trainee->GetScript() != nullptr)
                        trainee->GetScript()->DoAction(ACTION_TUSHUI_EMOTE_BASE + mMove);
                }
                break;
            }
            default:
                break;
        }
    }

private:
    uint32_t mMove = 1;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Quiet Lam (57752) and Ironfist Zhou (57753) - sparring in front of the academy

class SparringMonkAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SparringMonkAI(c); }
    explicit SparringMonkAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(5000);
    }

    void AIUpdate() override
    {
        playRandomTraineeEmote(getCreature());
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Tushui (54587, 65471) and Huojin (54586, 65470) trainees - quest 29524 The Lesson of Stifled Pride
// They cannot die: at 1% health they yield, turn friendly, give credit and despawn.

class YieldingTraineeAI : public WanderingIsleCreatureAI
{
public:
    explicit YieldingTraineeAI(Creature* pCreature, uint32_t firstYieldText) : WanderingIsleCreatureAI(pCreature), mFirstYieldText(firstYieldText)
    {
        RegisterAIUpdateEvent(1000);
    }

    void OnLoad() override
    {
        // respawns reuse the object, restore what the yield changed
        mYielded = false;
        setReactState(REACT_DEFENSIVE);
        getCreature()->setFaction(getCreature()->GetCreatureProperties()->Faction);
        getCreature()->setEmoteState(0);
        scriptEvents.resetEvents();
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        if (!mYielded)
            scriptEvents.resetEvents();
    }

    void DamageTaken(Unit* attacker, uint32_t* damage) override
    {
        if (mYielded)
        {
            *damage = 0;
            return;
        }

        // the trainee cannot die, a lethal hit leaves 1 health and makes him yield
        const uint32_t health = getCreature()->getHealth();
        if (*damage >= health)
            *damage = health - 1;

        const uint32_t remainingHealth = health - *damage;
        if (remainingHealth <= 1 || remainingHealth * 100 <= getCreature()->getMaxHealth())
            yield(attacker);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        onIdleUpdate();
    }

protected:
    virtual void onIdleUpdate() {}

    bool mYielded = false;

private:
    void yield(Unit* attacker)
    {
        mYielded = true;
        scriptEvents.resetEvents();

        _wipeHateList();
        getCreature()->getAIInterface()->attackStop();
        getCreature()->setFaction(35);
        getCreature()->setEmoteState(0);
        sendDBChatMessage(mFirstYieldText + Util::getRandomUInt(2));

        Player* player = attacker != nullptr ? attacker->getPlayerOwnerOrSelf() : nullptr;
        if (player != nullptr)
            giveIsleQuestCredit(player, QUEST_THE_LESSON_OF_STIFLED_PRIDE, 0);

        despawn(3000, TRAINEE_RESPAWN_TIME);
    }

    uint32_t mFirstYieldText;
};

class TushuiTraineeAI : public YieldingTraineeAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TushuiTraineeAI(c); }
    explicit TushuiTraineeAI(Creature* pCreature)
        : YieldingTraineeAI(pCreature, pCreature->getEntry() == NPC_TUSHUI_TRAINEE ? TEXT_TUSHUI_TRAINEE_YIELD_FIRST : TEXT_TUSHUI_TRAINEE_2_YIELD_FIRST) {}

    enum Events
    {
        EVENT_LEAD_MOVE = 1,
        EVENT_LEAD_REPEAT
    };

    void OnLoad() override
    {
        YieldingTraineeAI::OnLoad();

        // the trainee at this position leads the group training without an instructor
        mLeader = getCreature()->isInRange(1329.16f, 3308.09f, getCreature()->GetPositionZ(), 1.0f);
        if (mLeader)
            scriptEvents.addEvent(EVENT_LEAD_MOVE, 5000);
    }

    void DoAction(int32_t action) override
    {
        if (action > ACTION_TUSHUI_EMOTE_BASE && action <= ACTION_TUSHUI_EMOTE_BASE + 3)
            getCreature()->emote(static_cast<EmoteType>(zhiEmotes[action - ACTION_TUSHUI_EMOTE_BASE - 1]));
    }

protected:
    void onIdleUpdate() override
    {
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_LEAD_MOVE:
                mMove = Util::getRandomUInt(1, 3);
                getCreature()->emote(static_cast<EmoteType>(zhiEmotes[mMove - 1]));
                scriptEvents.addEvent(EVENT_LEAD_REPEAT, 1000);
                scriptEvents.addEvent(EVENT_LEAD_MOVE, 5000);
                break;
            case EVENT_LEAD_REPEAT:
            {
                std::list<Creature*> trainees;
                getCreaturesInPhase(NPC_TUSHUI_TRAINEE, 10.0f, trainees);
                for (Creature* trainee : trainees)
                {
                    if (trainee->GetScript() != nullptr)
                        trainee->GetScript()->DoAction(ACTION_TUSHUI_EMOTE_BASE + mMove);
                }
                break;
            }
            default:
                break;
        }
    }

private:
    bool mLeader = false;
    uint32_t mMove = 1;
};

class HuojinTraineeAI : public YieldingTraineeAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HuojinTraineeAI(c); }
    explicit HuojinTraineeAI(Creature* pCreature)
        : YieldingTraineeAI(pCreature, pCreature->getEntry() == NPC_HUOJIN_TRAINEE ? TEXT_HUOJIN_TRAINEE_YIELD_FIRST : TEXT_HUOJIN_TRAINEE_2_YIELD_FIRST) {}

    enum Events
    {
        EVENT_TRAIN = 1
    };

    void OnLoad() override
    {
        YieldingTraineeAI::OnLoad();
        scriptEvents.addEvent(EVENT_TRAIN, Util::getRandomUInt(2000, 4000));
    }

protected:
    void onIdleUpdate() override
    {
        if (scriptEvents.getFinishedEvent() != EVENT_TRAIN)
            return;

        if (!_isInCombat() && !mYielded)
            getCreature()->emote(static_cast<EmoteType>(traineeEmotes[Util::getRandomUInt(3)]));

        scriptEvents.addEvent(EVENT_TRAIN, Util::getRandomUInt(2000, 4000));
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Master Shang Xi (53566) - rewarding 29406 leaves the class phase of the first quests,
// accepting 29408 summons the Master's Flame in front of him

class MasterShangXiAcademyAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MasterShangXiAcademyAI(c); }
    explicit MasterShangXiAcademyAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(2000);
    }

    void onQuestRewarded(Player* player, QuestProperties const* qst) override
    {
        if (qst->id != QUEST_DISCIPLE_OF_THE_SANDY_FIST)
            return;

        player->removeAllAurasByAuraEffect(SPELL_AURA_PHASE);
        player->setPhase(PHASE_SET, PHASE_DEFAULT_PLAYER);
    }

    // quest 29408 The Lesson of the Burning Scroll: the master holds out his flame, the player snatches it
    void onQuestAccept(Player* player, QuestProperties const* qst) override
    {
        if (qst->id == QUEST_THE_LESSON_OF_THE_BURNING_SCROLL)
        {
            summonFlame();
            return;
        }

        for (const auto& classWeapon : academyClassWeapons)
        {
            if (classWeapon.quest != qst->id)
                continue;

            removeClassWeapon(player, classWeapon.mainHandItem);
            removeClassWeapon(player, classWeapon.offHandItem);
            return;
        }
    }

    // players who still have to snatch the flame (relog, despawned flame) get a new one
    void AIUpdate() override
    {
        if (findNearestCreatureInPhase(NPC_THE_MASTERS_FLAME, 5.0f) != nullptr)
            return;

        for (Player* player : getPlayersInPhase(30.0f, QUEST_THE_LESSON_OF_THE_BURNING_SCROLL, ISLE_QUEST_INCOMPLETE))
        {
            QuestLogEntry const* questLog = player->getQuestLogByQuestId(QUEST_THE_LESSON_OF_THE_BURNING_SCROLL);
            if (questLog != nullptr && questLog->getMobCountByIndex(0) == 0)
            {
                summonFlame();
                return;
            }
        }
    }

private:
    static void removeClassWeapon(Player* player, uint32_t itemId)
    {
        if (itemId == 0)
            return;

        if (const uint32_t count = player->getItemInterface()->GetItemCount(itemId, true))
            player->getItemInterface()->RemoveItemAmt(itemId, count);
    }

    void summonFlame()
    {
        if (findNearestCreatureInPhase(NPC_THE_MASTERS_FLAME, 5.0f) != nullptr)
            return;

        LocationVector flamePos = getCreature()->GetPosition();
        flamePos.x += 2.0f * std::cos(flamePos.o);
        flamePos.y += 2.0f * std::sin(flamePos.o);
        flamePos.z += 1.0f;
        summonCreature(NPC_THE_MASTERS_FLAME, flamePos, TIMED_OR_CORPSE_DESPAWN, 60000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Master's Flame (59591) - clicking it gives the flame item and the credit of quest 29408

class MastersFlameAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MastersFlameAI(c); }
    explicit MastersFlameAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        getCreature()->setControlled(true, UNIT_STATE_STUNNED);
    }

    void OnSpellClick(Unit* clicker, bool /*spellClickHandled*/) override
    {
        Player* player = clicker != nullptr ? clicker->getPlayerOwnerOrSelf() : nullptr;
        if (player == nullptr || !player->hasQuestInQuestLog(QUEST_THE_LESSON_OF_THE_BURNING_SCROLL) || mSnatched)
            return;

        mSnatched = true;
        player->castSpell(player, SPELL_CREATE_MASTERS_FLAME, true);
        giveIsleQuestCredit(player, QUEST_THE_LESSON_OF_THE_BURNING_SCROLL, 0);
        despawn(500, 0);
    }

private:
    bool mSnatched = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Jaomin Ro (54611) - quest 29409 The Disciple's Challenge
// He fights with jumps and his falcon and yields at 10% health.

class JaominRoAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new JaominRoAI(c); }
    explicit JaominRoAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(1000);
    }

    enum Events
    {
        EVENT_JAOMIN_JUMP = 1,
        EVENT_JAOMIN_JUMP_DAMAGE,
        EVENT_FALCON,
        EVENT_JAOMIN_JUMP_LANDED,
        EVENT_FALCON_VEHICLE_EXIT,
        EVENT_FALCON_STUN,
        EVENT_FALCON_STOP,
        EVENT_END_OF_COMBAT
    };

    void OnLoad() override
    {
        mYielded = false;
        scriptEvents.resetEvents();
    }

    void OnCombatStart(Unit* /*_target*/) override
    {
        scriptEvents.addEvent(EVENT_JAOMIN_JUMP, 2000);
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        mYielded = false;
        scriptEvents.resetEvents();
    }

    void DamageTaken(Unit* /*attacker*/, uint32_t* damage) override
    {
        if (mYielded)
        {
            if (*damage >= getCreature()->getHealth())
                *damage = 0;
            return;
        }

        if (_getHealthPercent() >= 10)
            return;

        mYielded = true;
        for (Player* player : getPlayersInPhase(10.0f))
            giveIsleQuestCredit(player, QUEST_THE_DISCIPLES_CHALLENGE, 0);

        getCreature()->emote(EMOTE_ONESHOT_SALUTE);
        sendDBChatMessage(TEXT_JAOMIN_RO_WELL_FOUGHT);

        if (*damage >= getCreature()->getHealth())
            *damage = 0;

        scriptEvents.resetEvents();
        scriptEvents.addEvent(EVENT_END_OF_COMBAT, 1000);
    }

    // once per second (RegisterAIUpdateEvent)
    void AIUpdate() override
    {
        if (!_isInCombat())
            greetChallengers();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        Unit* victim = getCreature()->getAIInterface()->getCurrentTarget();

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_JAOMIN_JUMP:
                // the jump spell of the client data (effect 42) would place him behind the target, the leap is scripted below
                scriptEvents.addEvent(EVENT_JAOMIN_JUMP_DAMAGE, 2500);
                break;
            case EVENT_JAOMIN_JUMP_DAMAGE:
                if (victim != nullptr)
                {
                    moveJump(victim->GetPosition(), 20.0f, 20.0f);
                    castSpell(victim, SPELL_JAOMIN_JUMP_DAMAGE);
                    for (Player* player : getPlayersInPhase(5.0f))
                        player->handleKnockback(getCreature(), 10.0f, 8.0f);
                }
                scriptEvents.addEvent(EVENT_JAOMIN_JUMP_LANDED, 1500);
                scriptEvents.addEvent(EVENT_FALCON, 10000);
                break;
            case EVENT_JAOMIN_JUMP_LANDED:
                faceVictim(victim);
                break;
            case EVENT_FALCON:
            {
                // he takes the shape of a hawk and dives 20 yards straight ahead
                getCreature()->setDisplayId(JAOMIN_HAWK_DISPLAY);
                const float orientation = getCreature()->GetOrientation();
                const LocationVector chargePos(getCreature()->GetPositionX() + 20.0f * std::cos(orientation),
                    getCreature()->GetPositionY() + 20.0f * std::sin(orientation), getCreature()->GetPositionZ());
                getCreature()->getMovementManager()->moveCharge(chargePos);
                scriptEvents.addEvent(EVENT_FALCON_VEHICLE_EXIT, 2000);
            } break;
            case EVENT_FALCON_VEHICLE_EXIT:
                getCreature()->resetDisplayId();
                faceVictim(victim);
                scriptEvents.addEvent(EVENT_FALCON_STUN, 1000);
                break;
            case EVENT_FALCON_STUN:
                if (victim != nullptr)
                    castSpell(victim, SPELL_JAOMIN_FALCON_STUN);
                scriptEvents.addEvent(EVENT_FALCON_STOP, 2000);
                break;
            case EVENT_FALCON_STOP:
                getCreature()->removeAllAurasByAuraEffect(SPELL_AURA_PERIODIC_TRIGGER_SPELL);
                scriptEvents.addEvent(EVENT_JAOMIN_JUMP, 10000);
                break;
            case EVENT_END_OF_COMBAT:
                _wipeHateList();
                getCreature()->getAIInterface()->enterEvadeMode();
                break;
            default:
                break;
        }
    }

private:
    // scripted movement leaves him where it ends, turn back to the challenger and chase him again
    void faceVictim(Unit* victim)
    {
        if (victim == nullptr)
            return;

        getCreature()->setInFront(victim);
        getCreature()->getMovementManager()->moveChase(victim);
    }

    // he stands up and calls out when a challenger with the quest approaches
    void greetChallengers()
    {
        const bool challengerNearby = !getPlayersInPhase(15.0f, QUEST_THE_DISCIPLES_CHALLENGE, ISLE_QUEST_INCOMPLETE).empty();

        // attackable while a challenger is around, back to the friendly spar faction afterwards
        if (challengerNearby && getCreature()->getFactionTemplate() != FACTION_NEUTRAL_CREATURE)
            getCreature()->setFaction(FACTION_NEUTRAL_CREATURE);
        else if (!challengerNearby && getCreature()->getFactionTemplate() != FACTION_JAOMIN_SPAR)
            getCreature()->setFaction(FACTION_JAOMIN_SPAR);

        if (!challengerNearby || getCreature()->getStandState() == STANDSTATE_STAND)
            return;

        sendDBChatMessage(TEXT_JAOMIN_RO_CHALLENGER);
        getCreature()->setStandState(STANDSTATE_STAND);
    }

    bool mYielded = false;
};

void SetupWanderingIsleAcademy(ScriptMgr* mgr)
{
    mgr->register_creature_script(NPC_ASPIRING_TRAINEE, &AspiringTraineeAI::Create);
    mgr->register_creature_script(NPC_ASPIRING_TRAINEE_2, &AspiringTraineeAI::Create);
    mgr->register_creature_script(NPC_TRAINING_TARGET_DESTRUCTIBLE, &DestructibleTrainingTargetAI::Create);
    mgr->register_creature_script(NPC_INSTRUCTOR_QUN, &InstructorQunAI::Create);
    mgr->register_creature_script(NPC_INSTRUCTOR_ZHI, &InstructorZhiAI::Create);
    mgr->register_creature_script(NPC_QUIET_LAM, &SparringMonkAI::Create);
    mgr->register_creature_script(NPC_IRONFIST_ZHOU, &SparringMonkAI::Create);
    mgr->register_creature_script(NPC_TUSHUI_TRAINEE, &TushuiTraineeAI::Create);
    mgr->register_creature_script(NPC_TUSHUI_TRAINEE_2, &TushuiTraineeAI::Create);
    mgr->register_creature_script(NPC_HUOJIN_TRAINEE, &HuojinTraineeAI::Create);
    mgr->register_creature_script(NPC_HUOJIN_TRAINEE_2, &HuojinTraineeAI::Create);
    mgr->register_creature_script(NPC_MASTER_SHANG_XI, &MasterShangXiAcademyAI::Create);
    mgr->register_creature_script(NPC_THE_MASTERS_FLAME, &MastersFlameAI::Create);
    mgr->register_creature_script(NPC_JAOMIN_RO, &JaominRoAI::Create);
}
