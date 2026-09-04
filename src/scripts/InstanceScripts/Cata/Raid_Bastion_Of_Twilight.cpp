/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Bastion of Twilight - four bosses (verified against wowhead), hand-ported into AscEmu's own
// CreatureAIScript / InstanceScript framework.
//
// This raid is built almost entirely around elaborate mechanics: Halfus' randomized captive
// drake buffs, Theralion & Valiona's ground/dragonflight phase split, the Ascendant Council's
// four-room split fight with a merge phase, and Cho'gall's worship/corruption-conversion
// mechanic. None of that is ported - each boss (or, for the Council and the two dragons, each
// participant) instead runs a simplified but functional core rotation from its real kit.
// Doors/encounter gating between bosses are not tracked.

#include "Setup.h"
#include "Raid_Bastion_Of_Twilight.hpp"

#include "Objects/GameObject.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum BastionOfTwilightEvents
{
    EVENT_HALFUS_FURIOUS_ROAR = 1,
    EVENT_HALFUS_MALEVOLENT_STRIKES,
    EVENT_HALFUS_FIREBALL_BARRAGE,
    EVENT_HALFUS_BERSERK,

    EVENT_VALIONA_TWILIGHT_BLAST,
    EVENT_VALIONA_DEVOURING_FLAMES,
    EVENT_THERALION_ENGULFING_MAGIC,
    EVENT_THERALION_BLACKOUT,

    EVENT_FELUDIUS_GLACIATE,
    EVENT_FELUDIUS_HYDRO_LANCE,
    EVENT_IGNACIOUS_FLAME_TORRENT,
    EVENT_IGNACIOUS_RISING_FLAMES,
    EVENT_TERRASTRA_QUAKE,
    EVENT_TERRASTRA_SHATTER,
    EVENT_ARION_THUNDERSHOCK,
    EVENT_ARION_LIGHTNING_ROD,

    EVENT_CHOGALL_CORRUPTED_BLOOD,
    EVENT_CHOGALL_FURY_OF_CHOGALL,
    EVENT_CHOGALL_SUMMON_CORRUPTING_ADHERENT,

    EVENT_SHADOW_KNIGHT_DEVASTATE,
    EVENT_SHADOW_KNIGHT_DISMANTLE,
    EVENT_CROSSFIRE_SHOOT,
    EVENT_CROSSFIRE_RAPID_FIRE,
    EVENT_CROSSFIRE_WYVERN_STING,
    EVENT_CROSSFIRE_MULTI_SHOT,
    EVENT_SOUL_BLADE_DARK_POOL,
    EVENT_DARK_MENDER_HEAL,
    EVENT_DARK_MENDER_HUNGERING_SHADOWS,
    EVENT_PHASE_TWISTER_TWIST_PHASE,
    EVENT_BOUND_INFERNO_FLAMESTRIKE,
    EVENT_BOUND_ZEPHYR_LIGHTNING_SHOCK,
    EVENT_BOUND_ZEPHYR_RENDING_GALE,
    EVENT_BOUND_ZEPHYR_VAPORIZE,
    EVENT_BOUND_DELUGE_FROST_WHIRL,
    EVENT_BOUND_RUMBLER_ENTOMB,
    EVENT_BOUND_RUMBLER_SHOCKWAVE
};

class BastionOfTwilightInstanceScript : public InstanceScript
{
public:
    explicit BastionOfTwilightInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(BastionOfTwilightEncounterCount);
        mHalfusExitGuid = 0;
        mDragonSiblingsExitGuid = 0;
        mAscendantCouncilExitGuid = 0;
        mChogallEntranceGuid = 0;
        mTwilightsHammerThroneGuid = 0;
        mTwilightDragonsDead = 0;
        mAscendantCouncilMembersDead = 0;
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new BastionOfTwilightInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_HALFUS_EXIT:
                mHalfusExitGuid = pGameObject->getGuidLow();
                break;
            case GO_DRAGON_SIBLINGS_EXIT:
                mDragonSiblingsExitGuid = pGameObject->getGuidLow();
                break;
            case GO_ASCENDANT_COUNCIL_EXIT:
                mAscendantCouncilExitGuid = pGameObject->getGuidLow();
                break;
            case GO_CHOGALL_ENTRANCE:
                mChogallEntranceGuid = pGameObject->getGuidLow();
                break;
            case GO_TWILIGHTS_HAMMER_THRONE:
                mTwilightsHammerThroneGuid = pGameObject->getGuidLow();
                break;
            default:
                break;
        }
    }

    void OpenHalfusExit()
    {
        if (GameObject* pDoor = GetGameObjectByGuid(mHalfusExitGuid))
            useDoorOrButton(pDoor);
        TryOpenChogallEntrance();
    }

    // Theralion and Valiona are two independent creatures sharing one boss slot; the slot
    // only completes (and the exit door only opens) once both are dead.
    void OnTwilightDragonDied()
    {
        if (++mTwilightDragonsDead < 2)
            return;

        setBossState(DATA_THERALION_AND_VALIONA, EncounterStates::Performed);
        if (GameObject* pDoor = GetGameObjectByGuid(mDragonSiblingsExitGuid))
            useDoorOrButton(pDoor);
        TryOpenChogallEntrance();
    }

    // Feludius/Ignacious/Terrastra/Arion are four independent creatures sharing one boss
    // slot; the slot only completes (and the exit door only opens) once all four are dead.
    void OnAscendantCouncilMemberDied()
    {
        if (++mAscendantCouncilMembersDead < 4)
            return;

        setBossState(DATA_ASCENDANT_COUNCIL, EncounterStates::Performed);
        if (GameObject* pDoor = GetGameObjectByGuid(mAscendantCouncilExitGuid))
            useDoorOrButton(pDoor);
        TryOpenChogallEntrance();
    }

    // Cho'gall's chamber only opens once all three side encounters are down, since the real
    // instance lets Halfus, Theralion & Valiona and the Ascendant Council be done in any order.
    void TryOpenChogallEntrance()
    {
        if (getBossState(DATA_HALFUS_WYRMBREAKER) != EncounterStates::Performed)
            return;
        if (getBossState(DATA_THERALION_AND_VALIONA) != EncounterStates::Performed)
            return;
        if (getBossState(DATA_ASCENDANT_COUNCIL) != EncounterStates::Performed)
            return;

        if (GameObject* pDoor = GetGameObjectByGuid(mChogallEntranceGuid))
            useDoorOrButton(pDoor);
        if (GameObject* pDoor = GetGameObjectByGuid(mTwilightsHammerThroneGuid))
            useDoorOrButton(pDoor);
    }

private:
    uint32_t mHalfusExitGuid;
    uint32_t mDragonSiblingsExitGuid;
    uint32_t mAscendantCouncilExitGuid;
    uint32_t mChogallEntranceGuid;
    uint32_t mTwilightsHammerThroneGuid;
    uint32_t mTwilightDragonsDead;
    uint32_t mAscendantCouncilMembersDead;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Halfus Wyrmbreaker

class HalfusWyrmbreakerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HalfusWyrmbreakerAI(c); }
    explicit HalfusWyrmbreakerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Cho'gall will have your heads! ALL OF THEM!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_HALFUS_WYRMBREAKER, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_HALFUS_FURIOUS_ROAR, 15000);
        scriptEvents.addEvent(EVENT_HALFUS_MALEVOLENT_STRIKES, 10000);
        scriptEvents.addEvent(EVENT_HALFUS_FIREBALL_BARRAGE, 20000);
        scriptEvents.addEvent(EVENT_HALFUS_BERSERK, 600000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HALFUS_WYRMBREAKER, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The burden of the damned falls upon you!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HALFUS_WYRMBREAKER, EncounterStates::Performed);
        static_cast<BastionOfTwilightInstanceScript*>(getInstanceScript())->OpenHalfusExit();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HALFUS_FURIOUS_ROAR:
                castSpellAOE(SPELL_HALFUS_FURIOUS_ROAR);
                scriptEvents.addEvent(EVENT_HALFUS_FURIOUS_ROAR, 25000);
                break;
            case EVENT_HALFUS_MALEVOLENT_STRIKES:
                castSpellOnVictim(SPELL_HALFUS_MALEVOLENT_STRIKES);
                scriptEvents.addEvent(EVENT_HALFUS_MALEVOLENT_STRIKES, 16000);
                break;
            case EVENT_HALFUS_FIREBALL_BARRAGE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_HALFUS_FIREBALL_BARRAGE);
                scriptEvents.addEvent(EVENT_HALFUS_FIREBALL_BARRAGE, 22000);
                break;
            case EVENT_HALFUS_BERSERK:
                castSpellOnSelf(SPELL_HALFUS_BERSERK, true);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Theralion & Valiona

class TwilightDragonAI : public CreatureAIScript
{
public:
    explicit TwilightDragonAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_THERALION_AND_VALIONA, EncounterStates::InProgress);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_THERALION_AND_VALIONA, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }
};

class ValionaAI : public TwilightDragonAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ValionaAI(c); }
    explicit ValionaAI(Creature* pCreature) : TwilightDragonAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Enter Twilight!");
        TwilightDragonAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_VALIONA_TWILIGHT_BLAST, 10000);
        scriptEvents.addEvent(EVENT_VALIONA_DEVOURING_FLAMES, 16000);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "At least... Theralion dies with me...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        static_cast<BastionOfTwilightInstanceScript*>(getInstanceScript())->OnTwilightDragonDied();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_VALIONA_TWILIGHT_BLAST:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Valiona takes a Deep Breath!");
                castSpellAOE(SPELL_VALIONA_TWILIGHT_BLAST);
                scriptEvents.addEvent(EVENT_VALIONA_TWILIGHT_BLAST, 20000);
                break;
            case EVENT_VALIONA_DEVOURING_FLAMES:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_VALIONA_DEVOURING_FLAMES);
                scriptEvents.addEvent(EVENT_VALIONA_DEVOURING_FLAMES, 22000);
                break;
            default:
                break;
        }
    }
};

class TheralionAI : public TwilightDragonAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheralionAI(c); }
    explicit TheralionAI(Creature* pCreature) : TwilightDragonAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "WRITHE IN AGONY!");
        TwilightDragonAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_THERALION_ENGULFING_MAGIC, 13000);
        scriptEvents.addEvent(EVENT_THERALION_BLACKOUT, 19000);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "I knew I should have stayed out of this...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        static_cast<BastionOfTwilightInstanceScript*>(getInstanceScript())->OnTwilightDragonDied();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_THERALION_ENGULFING_MAGIC:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Theralion begins to cast Engulfing Magic!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_THERALION_ENGULFING_MAGIC);
                scriptEvents.addEvent(EVENT_THERALION_ENGULFING_MAGIC, 21000);
                break;
            case EVENT_THERALION_BLACKOUT:
                castSpellAOE(SPELL_THERALION_BLACKOUT);
                scriptEvents.addEvent(EVENT_THERALION_BLACKOUT, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ascendant Council - Feludius / Ignacious / Terrastra / Arion, fought together.

class CouncilMemberAI : public CreatureAIScript
{
public:
    explicit CouncilMemberAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ASCENDANT_COUNCIL, EncounterStates::InProgress);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ASCENDANT_COUNCIL, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }
};

class FeludiusAI : public CouncilMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FeludiusAI(c); }
    explicit FeludiusAI(Creature* pCreature) : CouncilMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "You dare invade our lord's sanctum?");
        CouncilMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_FELUDIUS_GLACIATE, 15000);
        scriptEvents.addEvent(EVENT_FELUDIUS_HYDRO_LANCE, 9000);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "But now, witness true power...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        static_cast<BastionOfTwilightInstanceScript*>(getInstanceScript())->OnAscendantCouncilMemberDied();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_FELUDIUS_GLACIATE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Feludius begins to cast Glaciate!");
                castSpellAOE(SPELL_FELUDIUS_GLACIATE);
                scriptEvents.addEvent(EVENT_FELUDIUS_GLACIATE, 22000);
                break;
            case EVENT_FELUDIUS_HYDRO_LANCE:
                castSpellOnVictim(SPELL_FELUDIUS_HYDRO_LANCE);
                scriptEvents.addEvent(EVENT_FELUDIUS_HYDRO_LANCE, 18000);
                break;
            default:
                break;
        }
    }
};

class IgnaciousAI : public CouncilMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IgnaciousAI(c); }
    explicit IgnaciousAI(Creature* pCreature) : CouncilMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "You will die for your insolence!");
        CouncilMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_IGNACIOUS_FLAME_TORRENT, 12000);
        scriptEvents.addEvent(EVENT_IGNACIOUS_RISING_FLAMES, 20000);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "...the fury of the elements!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        static_cast<BastionOfTwilightInstanceScript*>(getInstanceScript())->OnAscendantCouncilMemberDied();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_IGNACIOUS_FLAME_TORRENT:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Ignacious begins to cast Rising Flames!");
                castSpellAOE(SPELL_IGNACIOUS_FLAME_TORRENT);
                scriptEvents.addEvent(EVENT_IGNACIOUS_FLAME_TORRENT, 20000);
                break;
            case EVENT_IGNACIOUS_RISING_FLAMES:
                castSpellOnSelf(SPELL_IGNACIOUS_RISING_FLAMES);
                scriptEvents.addEvent(EVENT_IGNACIOUS_RISING_FLAMES, 24000);
                break;
            default:
                break;
        }
    }
};

class TerrastraAI : public CouncilMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TerrastraAI(c); }
    explicit TerrastraAI(Creature* pCreature) : CouncilMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "We will handle them!");
        CouncilMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_TERRASTRA_QUAKE, 14000);
        scriptEvents.addEvent(EVENT_TERRASTRA_SHATTER, 22000);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "...to have made it this far.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        static_cast<BastionOfTwilightInstanceScript*>(getInstanceScript())->OnAscendantCouncilMemberDied();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_TERRASTRA_QUAKE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Terrastra begins to cast Quake!");
                castSpellAOE(SPELL_TERRASTRA_QUAKE);
                scriptEvents.addEvent(EVENT_TERRASTRA_QUAKE, 22000);
                break;
            case EVENT_TERRASTRA_SHATTER:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_TERRASTRA_SHATTER);
                scriptEvents.addEvent(EVENT_TERRASTRA_SHATTER, 26000);
                break;
            default:
                break;
        }
    }
};

class ArionAI : public CouncilMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ArionAI(c); }
    explicit ArionAI(Creature* pCreature) : CouncilMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Enough of this foolishness!");
        CouncilMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_ARION_THUNDERSHOCK, 11000);
        scriptEvents.addEvent(EVENT_ARION_LIGHTNING_ROD, 18000);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "An impressive display...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        static_cast<BastionOfTwilightInstanceScript*>(getInstanceScript())->OnAscendantCouncilMemberDied();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ARION_THUNDERSHOCK:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Arion begins to cast Thundershock!");
                castSpellAOE(SPELL_ARION_THUNDERSHOCK);
                scriptEvents.addEvent(EVENT_ARION_THUNDERSHOCK, 20000);
                break;
            case EVENT_ARION_LIGHTNING_ROD:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ARION_LIGHTNING_ROD);
                scriptEvents.addEvent(EVENT_ARION_LIGHTNING_ROD, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Cho'gall

class ChogallAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ChogallAI(c); }
    explicit ChogallAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Valiona, Theralion, put them in their place.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_CHOGALL, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_CHOGALL_CORRUPTED_BLOOD, 10000);
        scriptEvents.addEvent(EVENT_CHOGALL_FURY_OF_CHOGALL, 16000);
        scriptEvents.addEvent(EVENT_CHOGALL_SUMMON_CORRUPTING_ADHERENT, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_CHOGALL, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "It is finished.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_CHOGALL, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CHOGALL_CORRUPTED_BLOOD:
                castSpellOnSelf(SPELL_CHOGALL_CORRUPTED_BLOOD);
                scriptEvents.addEvent(EVENT_CHOGALL_CORRUPTED_BLOOD, 30000);
                break;
            case EVENT_CHOGALL_FURY_OF_CHOGALL:
                castSpellOnVictim(SPELL_CHOGALL_FURY_OF_CHOGALL);
                scriptEvents.addEvent(EVENT_CHOGALL_FURY_OF_CHOGALL, 20000);
                break;
            case EVENT_CHOGALL_SUMMON_CORRUPTING_ADHERENT:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Cho'gall begins to summon Corrupted Adherents to aid him!");
                castSpellOnSelf(SPELL_CHOGALL_SUMMON_CORRUPTING_ADHERENT);
                scriptEvents.addEvent(EVENT_CHOGALL_SUMMON_CORRUPTING_ADHERENT, 35000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash

class TwilightShadowKnightAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightShadowKnightAI(c); }
    explicit TwilightShadowKnightAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SHADOW_KNIGHT_DEVASTATE, 6000);
        scriptEvents.addEvent(EVENT_SHADOW_KNIGHT_DISMANTLE, 15000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SHADOW_KNIGHT_DEVASTATE:
                castSpellOnVictim(SPELL_SHADOW_KNIGHT_DEVASTATE);
                scriptEvents.addEvent(EVENT_SHADOW_KNIGHT_DEVASTATE, 6000);
                break;
            case EVENT_SHADOW_KNIGHT_DISMANTLE:
                castSpellOnVictim(SPELL_SHADOW_KNIGHT_DISMANTLE);
                scriptEvents.addEvent(EVENT_SHADOW_KNIGHT_DISMANTLE, 30000);
                break;
            default:
                break;
        }
    }
};

class TwilightCrossfireAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightCrossfireAI(c); }
    explicit TwilightCrossfireAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_CROSSFIRE_SHOOT, 800);
        scriptEvents.addEvent(EVENT_CROSSFIRE_RAPID_FIRE, 18000);
        scriptEvents.addEvent(EVENT_CROSSFIRE_WYVERN_STING, 22000);
        scriptEvents.addEvent(EVENT_CROSSFIRE_MULTI_SHOT, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CROSSFIRE_SHOOT:
                castSpellOnVictim(SPELL_CROSSFIRE_SHOOT);
                scriptEvents.addEvent(EVENT_CROSSFIRE_SHOOT, 2000);
                break;
            case EVENT_CROSSFIRE_RAPID_FIRE:
                castSpellOnSelf(SPELL_CROSSFIRE_RAPID_FIRE);
                scriptEvents.addEvent(EVENT_CROSSFIRE_RAPID_FIRE, 36000);
                break;
            case EVENT_CROSSFIRE_WYVERN_STING:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_CROSSFIRE_WYVERN_STING);
                scriptEvents.addEvent(EVENT_CROSSFIRE_WYVERN_STING, 11000);
                break;
            case EVENT_CROSSFIRE_MULTI_SHOT:
                castSpellOnVictim(SPELL_CROSSFIRE_MULTI_SHOT);
                scriptEvents.addEvent(EVENT_CROSSFIRE_MULTI_SHOT, 13000);
                break;
            default:
                break;
        }
    }
};

class TwilightSoulBladeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightSoulBladeAI(c); }
    explicit TwilightSoulBladeAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SOUL_BLADE_DARK_POOL, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_SOUL_BLADE_DARK_POOL)
        {
            if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                castSpell(pTarget, SPELL_SOUL_BLADE_DARK_POOL);
            scriptEvents.addEvent(EVENT_SOUL_BLADE_DARK_POOL, 8000);
        }
    }
};

// Reference casts its heal on the lowest-health nearby ally whenever one drops below 90% hp;
// we have no "lowest-health ally" target filter, so this is approximated as a periodic
// self-cast.
class TwilightDarkMenderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightDarkMenderAI(c); }
    explicit TwilightDarkMenderAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_DARK_MENDER_HEAL, 15000);
        scriptEvents.addEvent(EVENT_DARK_MENDER_HUNGERING_SHADOWS, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DARK_MENDER_HEAL:
                castSpellOnSelf(SPELL_DARK_MENDER_HEAL);
                scriptEvents.addEvent(EVENT_DARK_MENDER_HEAL, 60000);
                break;
            case EVENT_DARK_MENDER_HUNGERING_SHADOWS:
                castSpellOnSelf(SPELL_DARK_MENDER_HUNGERING_SHADOWS);
                scriptEvents.addEvent(EVENT_DARK_MENDER_HUNGERING_SHADOWS, 12000);
                break;
            default:
                break;
        }
    }
};

class TwilightPhaseTwisterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightPhaseTwisterAI(c); }
    explicit TwilightPhaseTwisterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_PHASE_TWISTER_TWIST_PHASE, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_PHASE_TWISTER_TWIST_PHASE)
        {
            castSpellOnSelf(SPELL_PHASE_TWISTER_TWIST_PHASE);
            scriptEvents.addEvent(EVENT_PHASE_TWISTER_TWIST_PHASE, 8000);
        }
    }
};

class BoundInfernoAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BoundInfernoAI(c); }
    explicit BoundInfernoAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_BOUND_INFERNO_FLAMESTRIKE, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_BOUND_INFERNO_FLAMESTRIKE)
        {
            if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                castSpell(pTarget, SPELL_BOUND_INFERNO_FLAMESTRIKE);
            scriptEvents.addEvent(EVENT_BOUND_INFERNO_FLAMESTRIKE, 14000);
        }
    }
};

class BoundZephyrAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BoundZephyrAI(c); }
    explicit BoundZephyrAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_BOUND_ZEPHYR_LIGHTNING_SHOCK, 4000);
        scriptEvents.addEvent(EVENT_BOUND_ZEPHYR_RENDING_GALE, 5500);
        scriptEvents.addEvent(EVENT_BOUND_ZEPHYR_VAPORIZE, 15000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BOUND_ZEPHYR_LIGHTNING_SHOCK:
                castSpellOnVictim(SPELL_BOUND_ZEPHYR_LIGHTNING_SHOCK);
                scriptEvents.addEvent(EVENT_BOUND_ZEPHYR_LIGHTNING_SHOCK, 6000);
                break;
            case EVENT_BOUND_ZEPHYR_RENDING_GALE:
                castSpellOnVictim(SPELL_BOUND_ZEPHYR_RENDING_GALE);
                scriptEvents.addEvent(EVENT_BOUND_ZEPHYR_RENDING_GALE, 6000);
                break;
            case EVENT_BOUND_ZEPHYR_VAPORIZE:
                castSpellOnVictim(SPELL_BOUND_ZEPHYR_VAPORIZE);
                scriptEvents.addEvent(EVENT_BOUND_ZEPHYR_VAPORIZE, 15000);
                break;
            default:
                break;
        }
    }
};

class BoundDelugeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BoundDelugeAI(c); }
    explicit BoundDelugeAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_BOUND_DELUGE_FROST_WHIRL, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_BOUND_DELUGE_FROST_WHIRL)
        {
            castSpellOnSelf(SPELL_BOUND_DELUGE_FROST_WHIRL);
            scriptEvents.addEvent(EVENT_BOUND_DELUGE_FROST_WHIRL, 16000);
        }
    }
};

class BoundRumblerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BoundRumblerAI(c); }
    explicit BoundRumblerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_BOUND_RUMBLER_ENTOMB, 5500);
        scriptEvents.addEvent(EVENT_BOUND_RUMBLER_SHOCKWAVE, 7000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BOUND_RUMBLER_ENTOMB:
                castSpellOnVictim(SPELL_BOUND_RUMBLER_ENTOMB);
                scriptEvents.addEvent(EVENT_BOUND_RUMBLER_ENTOMB, 6000);
                break;
            case EVENT_BOUND_RUMBLER_SHOCKWAVE:
                castSpellOnSelf(SPELL_BOUND_RUMBLER_SHOCKWAVE);
                scriptEvents.addEvent(EVENT_BOUND_RUMBLER_SHOCKWAVE, 15000);
                break;
            default:
                break;
        }
    }
};

void SetupBastionOfTwilight(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BASTION_OF_TWILIGHT, &BastionOfTwilightInstanceScript::Create);

    mgr->register_creature_script(BOSS_HALFUS_WYRMBREAKER, &HalfusWyrmbreakerAI::Create);
    mgr->register_creature_script(BOSS_VALIONA, &ValionaAI::Create);
    mgr->register_creature_script(BOSS_THERALION, &TheralionAI::Create);
    mgr->register_creature_script(BOSS_FELUDIUS, &FeludiusAI::Create);
    mgr->register_creature_script(BOSS_IGNACIOUS, &IgnaciousAI::Create);
    mgr->register_creature_script(BOSS_TERRASTRA, &TerrastraAI::Create);
    mgr->register_creature_script(BOSS_ARION, &ArionAI::Create);
    mgr->register_creature_script(BOSS_CHOGALL, &ChogallAI::Create);

    mgr->register_creature_script(NPC_TWILIGHT_SHADOW_KNIGHT, &TwilightShadowKnightAI::Create);
    mgr->register_creature_script(NPC_TWILIGHT_CROSSFIRE, &TwilightCrossfireAI::Create);
    mgr->register_creature_script(NPC_TWILIGHT_SOUL_BLADE, &TwilightSoulBladeAI::Create);
    mgr->register_creature_script(NPC_TWILIGHT_DARK_MENDER, &TwilightDarkMenderAI::Create);
    mgr->register_creature_script(NPC_TWILIGHT_PHASE_TWISTER, &TwilightPhaseTwisterAI::Create);
    mgr->register_creature_script(NPC_BOUND_INFERNO, &BoundInfernoAI::Create);
    mgr->register_creature_script(NPC_BOUND_ZEPHYR, &BoundZephyrAI::Create);
    mgr->register_creature_script(NPC_BOUND_DELUGE, &BoundDelugeAI::Create);
    mgr->register_creature_script(NPC_BOUND_RUMBLER, &BoundRumblerAI::Create);
}
