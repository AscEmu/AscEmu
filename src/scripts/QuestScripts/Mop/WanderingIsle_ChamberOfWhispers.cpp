/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

#include "WanderingIsle.hpp"

#include "Chat/ChatDefines.hpp"
#include "Movement/MovementManager.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/CreatureDefines.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/LocationVector.hpp"
#include "Utilities/Random.hpp"

#include <list>

//////////////////////////////////////////////////////////////////////////////////////////
// Chamber of Whispers and the Wind Temple - quests 29785 Dafeng, the Spirit of Air and
// 29786 Battle for the Skies.

enum ChamberEntries
{
    NPC_AYSA_CHAMBER_OF_WHISPERS = 55744,
    NPC_AYSA_WIND_TEMPLE = 55595,
    NPC_FIREWORK_LAUNCHER = 64507,
    NPC_ZHAO_REN = 55786,
    NPC_AIR_CHAMBER_CROSSED_CREDIT = 55666,
    GO_WIND_CLOUD = 209685
};

enum ChamberSpells
{
    SPELL_FIREWORK_HIT = 104855,
    SPELL_ZHAO_REN_STUNNED_VISUAL = 125992,
    SPELL_ZHAO_REN_STUN = 125990,
    SPELL_ZHAO_REN_LIGHTNING = 126006,
    SPELL_BATTLE_FOR_THE_SKIES_COMPLETE = 60922
};

//////////////////////////////////////////////////////////////////////////////////////////
// Aysa (55744) crossing the Chamber of Whispers

const LocationVector aysaChamberMovePos1(647.493f, 4224.63f, 202.90865f, 2.426f);
const LocationVector aysaChamberMovePos2(598.57294f, 4266.661f, 206.54927f);
const LocationVector aysaChamberMovePos3(580.1649f, 4283.193f, 210.18248f);
const LocationVector aysaChamberMoveEnd(543.9549f, 4317.2744f, 212.22935f);
const LocationVector secondWindCloudPos(566.759f, 4299.13f, 212.301f);    // the far cloud

class AysaChamberOfWhispersAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AysaChamberOfWhispersAI(c); }
    explicit AysaChamberOfWhispersAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(1000);
    }

    enum Events
    {
        EVENT_INTRO = 1,
        EVENT_DEACTIVATE_1,
        EVENT_MOVE_POS_MID,
        EVENT_MOVE_POS_3,
        EVENT_DEACTIVATE_2,
        EVENT_MOVE_POS_END,
        EVENT_OUTRO
    };

    void OnLoad() override
    {
        sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Wait!");
        scriptEvents.resetEvents();
        scriptEvents.addEvent(EVENT_INTRO, 1000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_INTRO:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "We need to wait for the winds to settle, then make a break for the cover of the far hallway.");
                movePoint(0, aysaChamberMovePos1);
                scriptEvents.addEvent(EVENT_DEACTIVATE_1, 2000);
                break;
            case EVENT_DEACTIVATE_1:
                if (GameObject* cloud = findNearestGameObject(GO_WIND_CLOUD, 30.0f))
                    useDoorOrButton(cloud, 60000);
                scriptEvents.addEvent(EVENT_MOVE_POS_MID, 5000);
                break;
            case EVENT_MOVE_POS_MID:
                movePoint(1, aysaChamberMovePos2);
                scriptEvents.addEvent(EVENT_MOVE_POS_3, 10000);
                break;
            case EVENT_MOVE_POS_3:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Wait for another opening. I'll meet you on the far side.");
                scriptEvents.addEvent(EVENT_DEACTIVATE_2, 2000);
                break;
            case EVENT_DEACTIVATE_2:
            {
                movePoint(2, aysaChamberMovePos3);

                std::list<GameObject*> clouds;
                GetGameObjectListWithEntryInGrid(clouds, GO_WIND_CLOUD, 60.0f);
                for (GameObject* cloud : clouds)
                {
                    if (cloud->getDistance2d(secondWindCloudPos.x, secondWindCloudPos.y) < 1.0f)
                        useDoorOrButton(cloud, 60000);
                }
                scriptEvents.addEvent(EVENT_MOVE_POS_END, 1000);
                break;
            }
            case EVENT_MOVE_POS_END:
                movePoint(3, aysaChamberMoveEnd);
                scriptEvents.addEvent(EVENT_OUTRO, 10000);
                break;
            case EVENT_OUTRO:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Dafeng! What's wrong? Why are you hiding back here?");
                for (Player* player : getPlayersInPhase(45.0f))
                    giveIsleQuestCredit(player, QUEST_THE_CHAMBER_OF_WHISPERS, 0);
                despawn(500, 0);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Aysa (55595) in the Wind Temple - accepting 29786 moves the player into the fight phase

class AysaWindTempleAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AysaWindTempleAI(c); }
    explicit AysaWindTempleAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void onQuestAccept(Player* player, QuestProperties const* qst) override
    {
        if (qst->id != QUEST_BATTLE_FOR_THE_SKIES)
            return;

        player->setPhase(PHASE_SET, PHASE_WIND_TEMPLE);
        getCreature()->setPhase(PHASE_SET, PHASE_WIND_TEMPLE);
        movePoint(0, aysaChamberMovePos1);
        despawn(10000, 120000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Firework Launcher (64507) - clicking fires at Zhao-Ren when he is close

class FireworkLauncherAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FireworkLauncherAI(c); }
    explicit FireworkLauncherAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(1000);
    }

    void OnLoad() override
    {
        mCooldown = 0;
        getCreature()->addNpcFlags(UNIT_NPC_FLAG_SPELLCLICK);
    }

    void OnSpellClick(Unit* clicker, bool /*spellClickHandled*/) override
    {
        if (mCooldown != 0)
            return;

        Creature* zhao = findNearestCreatureInPhase(NPC_ZHAO_REN, 50.0f);
        if (zhao == nullptr || zhao->getDistance2d(clicker) > 25.0f)
            return;

        zhao->castSpell(zhao, SPELL_FIREWORK_HIT, false);

        mCooldown = 5000;
        getCreature()->removeNpcFlags(UNIT_NPC_FLAG_SPELLCLICK);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mCooldown == 0)
            return;

        if (mCooldown <= time_passed)
        {
            mCooldown = 0;
            getCreature()->addNpcFlags(UNIT_NPC_FLAG_SPELLCLICK);
        }
        else
        {
            mCooldown -= static_cast<uint32_t>(time_passed);
        }
    }

private:
    uint32_t mCooldown = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Zhao-Ren (55786) - circles the Wind Temple, five firework hits bring him down for a while

const LocationVector zhaoRenPos[7] =
{
    { 719.36f, 4164.60f, 216.06f },
    { 736.90f, 4183.85f, 221.41f },
    { 704.77f, 4190.16f, 218.24f },
    { 684.53f, 4173.24f, 216.98f },
    { 689.62f, 4153.16f, 217.63f },
    { 717.04f, 4141.16f, 219.83f },
    { 745.91f, 4154.35f, 223.48f }
};
const LocationVector zhaoRenStunPos(723.163025f, 4163.799805f, 202.082993f);

class ZhaoRenAI : public WanderingIsleCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ZhaoRenAI(c); }
    explicit ZhaoRenAI(Creature* pCreature) : WanderingIsleCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(1000);
    }

    enum Events
    {
        EVENT_MOVE_POSITION = 1,
        EVENT_STUNNED,
        EVENT_LIGHTNING
    };

    void OnLoad() override
    {
        reset();
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        if (mEncounterStarted)
            reset();
    }

    void OnDied(Unit* /*_killer*/) override
    {
        for (Player* player : getPlayersInPhase(60.0f))
        {
            giveIsleQuestCredit(player, QUEST_BATTLE_FOR_THE_SKIES, 0);
            player->castSpell(player, SPELL_BATTLE_FOR_THE_SKIES_COMPLETE, false);
            player->setPhase(PHASE_SET, PHASE_DEFAULT_PLAYER);
        }
    }

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type != POINT_MOTION_TYPE || id == 0)
            return;

        if (id == 100)
        {
            castSpellOnSelf(SPELL_ZHAO_REN_STUNNED_VISUAL, true);
            scriptEvents.removeEvent(EVENT_LIGHTNING);
            scriptEvents.addEvent(EVENT_LIGHTNING, 17000);
            scriptEvents.addEvent(EVENT_STUNNED, 12000);
        }
        else
        {
            scriptEvents.addEvent(EVENT_MOVE_POSITION, 1000);
        }
    }

    void OnHitBySpell(uint32_t spellId, Unit* /*caster*/) override
    {
        if (spellId != SPELL_FIREWORK_HIT)
            return;

        if (++mFireworkHits >= 5)
        {
            getMovementManager()->clear();
            movePoint(100, zhaoRenStunPos);
            mFireworkHits = 0;
        }
    }

    // once per second (RegisterAIUpdateEvent): the fight runs while disciples with the quest are near
    void AIUpdate() override
    {
        const bool participants = !getPlayersInPhase(60.0f, QUEST_BATTLE_FOR_THE_SKIES, ISLE_QUEST_INCOMPLETE).empty();
        if (participants && !mEncounterStarted)
        {
            mEncounterStarted = true;
            scriptEvents.addEvent(EVENT_MOVE_POSITION, 1000);
            scriptEvents.addEvent(EVENT_LIGHTNING, 5000);
        }
        else if (!participants && mEncounterStarted)
        {
            reset();
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mEncounterStarted)
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MOVE_POSITION:
                if (getCreature()->hasAurasWithId(SPELL_ZHAO_REN_STUNNED_VISUAL))
                    scriptEvents.addEvent(EVENT_MOVE_POSITION, 2000);
                rotatePosition();
                break;
            case EVENT_STUNNED:
                _removeAura(SPELL_ZHAO_REN_STUNNED_VISUAL);
                castSpellOnSelf(SPELL_ZHAO_REN_STUN, false);
                getCreature()->setMoveDisableGravity(true);
                scriptEvents.addEvent(EVENT_MOVE_POSITION, 4000);
                break;
            case EVENT_LIGHTNING:
            {
                const auto players = getPlayersInPhase(45.0f);
                if (!players.empty())
                    castSpell(players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))], SPELL_ZHAO_REN_LIGHTNING, false);
                scriptEvents.addEvent(EVENT_LIGHTNING, 5000);
                break;
            }
            default:
                break;
        }
    }

private:
    void reset()
    {
        scriptEvents.resetEvents();
        setReactState(REACT_PASSIVE);
        getCreature()->setMoveDisableGravity(true);
        setFlyMode(true);
        mEncounterStarted = false;
        mFireworkHits = 0;
        mCurrentPos = 0;
        getCreature()->setFullHealth();
        getMovementManager()->clear();
        movePoint(0, zhaoRenPos[0]);
    }

    void rotatePosition()
    {
        if (++mCurrentPos > 6)
            mCurrentPos = 1;

        movePoint(mCurrentPos, zhaoRenPos[mCurrentPos]);
    }

    bool mEncounterStarted = false;
    uint8_t mCurrentPos = 0;
    uint8_t mFireworkHits = 0;
};

void SetupWanderingIsleChamberOfWhispers(ScriptMgr* mgr)
{
    mgr->register_creature_script(NPC_AYSA_CHAMBER_OF_WHISPERS, &AysaChamberOfWhispersAI::Create);
    mgr->register_creature_script(NPC_AYSA_WIND_TEMPLE, &AysaWindTempleAI::Create);
    mgr->register_creature_script(NPC_FIREWORK_LAUNCHER, &FireworkLauncherAI::Create);
    mgr->register_creature_script(NPC_ZHAO_REN, &ZhaoRenAI::Create);
}
