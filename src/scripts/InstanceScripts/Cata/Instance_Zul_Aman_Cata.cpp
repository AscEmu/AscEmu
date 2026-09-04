/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Zul'Aman (Cataclysm heroic revamp) - six bosses (verified against wowhead), hand-ported
// into AscEmu's own CreatureAIScript / InstanceScript framework.
//
// Akilzon and Janalai run a simplified core rotation (their eagle-grab/fire-bomb-gauntlet
// mechanics are dropped). Nalorakk, Halazzi, Hexlord Malacrass and Daakara get encounter
// tracking only, matching the reference implementation, which leaves them unscripted too.
// Progression gating (per wowhead: the four animal-god avatars in any order, then Hex Lord
// Malacrass, then Daakara) is tracked via the wind door (Akil'zon), the Lynx Temple exit
// (Halazzi), the Hexlord entrance doors (all four avatars), and the Massive Gate (Hexlord
// Malacrass).

#include "Setup.h"
#include "Instance_Zul_Aman_Cata.hpp"

#include "Objects/GameObject.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum ZulAmanCataEvents
{
    EVENT_AKILZON_STATIC_DISRUPTION = 1,
    EVENT_AKILZON_CALL_LIGHTNING,
    EVENT_AKILZON_GUST_OF_WIND,
    EVENT_AKILZON_ELECTRICAL_STORM,

    EVENT_JANALAI_FLAME_BREATH,
    EVENT_JANALAI_FIRE_BOMB,

    EVENT_AMANISHI_GUARDIAN_REND,

    EVENT_AMANISHI_SCOUT_AMBUSH,
    EVENT_AMANISHI_SCOUT_SHOOT,
    EVENT_ELDER_LYNX_CLAW,
    EVENT_ELDER_LYNX_BITE,

    EVENT_NALORAKK_BRUTAL_STRIKE,
    EVENT_NALORAKK_SURGE,
    EVENT_NALORAKK_REND_FLESH,
    EVENT_NALORAKK_LACERATING_SLASH,
    EVENT_NALORAKK_DEAFENING_ROAR,
    EVENT_NALORAKK_FORM_SWAP,

    EVENT_HALAZZI_ENRAGE,
    EVENT_HALAZZI_WATER_TOTEM,
    EVENT_HALAZZI_LIGHTNING_TOTEM,
    EVENT_HALAZZI_EARTH_SHOCK,
    EVENT_HALAZZI_FLAME_SHOCK,
    EVENT_HALAZZI_SHRED_ARMOR,

    EVENT_MALACRASS_SPIRIT_BOLTS,
    EVENT_MALACRASS_SIPHON_SOUL,

    EVENT_DAAKARA_WHIRLWIND,
    EVENT_DAAKARA_GRIEVOUS_THROW
};

class ZulAmanCataInstanceScript : public InstanceScript
{
public:
    explicit ZulAmanCataInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(ZulAmanCataEncounterCount);
        mWindDoorGuid = 0;
        mLynxTempleExitGuid = 0;
        mHexlordEntranceGuid = 0;
        mHexlordWoodenDoorGuid = 0;
        mMassiveGateGuid = 0;
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ZulAmanCataInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_ZULAMAN_WIND_DOOR:
                mWindDoorGuid = pGameObject->getGuidLow();
                break;
            case GO_LYNX_TEMPLE_EXIT:
                mLynxTempleExitGuid = pGameObject->getGuidLow();
                break;
            case GO_HEXLORD_ENTRANCE:
                mHexlordEntranceGuid = pGameObject->getGuidLow();
                break;
            case GO_HEXLORD_WOODEN_DOOR:
                mHexlordWoodenDoorGuid = pGameObject->getGuidLow();
                break;
            case GO_MASSIVE_GATE:
                mMassiveGateGuid = pGameObject->getGuidLow();
                break;
            default:
                break;
        }
    }

    void OpenWindDoor()
    {
        if (GameObject* pDoor = GetGameObjectByGuid(mWindDoorGuid))
            useDoorOrButton(pDoor);
    }

    void OpenLynxTempleExit()
    {
        if (GameObject* pDoor = GetGameObjectByGuid(mLynxTempleExitGuid))
            useDoorOrButton(pDoor);
    }

    // Opens once all four animal-god avatars are down, granting access to Hex Lord Malacrass.
    void TryOpenHexlordDoors()
    {
        if (getBossState(DATA_ALKILZON) != EncounterStates::Performed)
            return;
        if (getBossState(DATA_NALORAKK) != EncounterStates::Performed)
            return;
        if (getBossState(DATA_JANALAI) != EncounterStates::Performed)
            return;
        if (getBossState(DATA_HALAZZI) != EncounterStates::Performed)
            return;

        if (GameObject* pDoor = GetGameObjectByGuid(mHexlordEntranceGuid))
            useDoorOrButton(pDoor);
        if (GameObject* pDoor = GetGameObjectByGuid(mHexlordWoodenDoorGuid))
            useDoorOrButton(pDoor);
    }

    // Opens the way to Daakara's summit after Hex Lord Malacrass.
    void OpenMassiveGate()
    {
        if (GameObject* pDoor = GetGameObjectByGuid(mMassiveGateGuid))
            useDoorOrButton(pDoor);
    }

private:
    uint32_t mWindDoorGuid;
    uint32_t mLynxTempleExitGuid;
    uint32_t mHexlordEntranceGuid;
    uint32_t mHexlordWoodenDoorGuid;
    uint32_t mMassiveGateGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Akil'zon

class AkilzonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AkilzonAI(c); }
    explicit AkilzonAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 12013, "I be da predator! You da prey...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ALKILZON, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_AKILZON_STATIC_DISRUPTION, 6000);
        scriptEvents.addEvent(EVENT_AKILZON_CALL_LIGHTNING, 7000);
        scriptEvents.addEvent(EVENT_AKILZON_GUST_OF_WIND, 9500);
        scriptEvents.addEvent(EVENT_AKILZON_ELECTRICAL_STORM, 49000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ALKILZON, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    // Reference dialogue data for entry 23574 also has SAY_SUMMON_EAGLE ("All you be doin' is
    // wastin' me time!", sound 12015) and SAY_SUMMON_BIRDS ("Ya got nothin'!", sound 12014) -
    // both tied to his eagle-grab/summon-birds gauntlet mechanic, which this simplified kit
    // doesn't model, so they have no cast point to attach to here.
    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 12017, "Feed, me bruddahs!");
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 12019, "You can't... kill... me spirit!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ALKILZON, EncounterStates::Performed);
        static_cast<ZulAmanCataInstanceScript*>(getInstanceScript())->OpenWindDoor();
        static_cast<ZulAmanCataInstanceScript*>(getInstanceScript())->TryOpenHexlordDoors();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_AKILZON_STATIC_DISRUPTION:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_AKILZON_STATIC_DISRUPTION);
                scriptEvents.addEvent(EVENT_AKILZON_STATIC_DISRUPTION, 20000);
                break;
            case EVENT_AKILZON_CALL_LIGHTNING:
                castSpellAOE(SPELL_AKILZON_CALL_LIGHTNING);
                scriptEvents.addEvent(EVENT_AKILZON_CALL_LIGHTNING, 20000);
                break;
            case EVENT_AKILZON_GUST_OF_WIND:
                castSpellAOE(SPELL_AKILZON_GUST_OF_WIND);
                scriptEvents.addEvent(EVENT_AKILZON_GUST_OF_WIND, 25000);
                break;
            case EVENT_AKILZON_ELECTRICAL_STORM:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Your death gonna be quick, strangers. You shoulda never come to this place....");
                castSpellAOE(SPELL_AKILZON_ELECTRICAL_STORM);
                scriptEvents.addEvent(EVENT_AKILZON_ELECTRICAL_STORM, 49000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Jan'alai

class JanalaiAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new JanalaiAI(c); }
    explicit JanalaiAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 12031, "Spirits of da wind be your doom!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_JANALAI, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_JANALAI_FLAME_BREATH, 7000);
        scriptEvents.addEvent(EVENT_JANALAI_FIRE_BOMB, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_JANALAI, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    // Reference dialogue data for entry 23578 also has SAY_SUMMON_HATCHER ("Where ma hatcha?
    // Get to work on dem eggs!", sound 12033) and SAY_HATCH_ALL_EGGS ("You done run outta
    // time!", sound 12034) tied to his egg-hatching gauntlet mechanic, which this simplified
    // kit doesn't model, so they have no cast point to attach to here.
    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 12036, "I burn ya now!");
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 12038, "Zul'jin... got a surprise for you....");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_JANALAI, EncounterStates::Performed);
        static_cast<ZulAmanCataInstanceScript*>(getInstanceScript())->TryOpenHexlordDoors();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_JANALAI_FLAME_BREATH:
                castSpellOnVictim(SPELL_JANALAI_FLAME_BREATH);
                scriptEvents.addEvent(EVENT_JANALAI_FLAME_BREATH, 12000);
                break;
            case EVENT_JANALAI_FIRE_BOMB:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 12032, "I show you strength... in numbers.");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_JANALAI_FIRE_BOMB);
                scriptEvents.addEvent(EVENT_JANALAI_FIRE_BOMB, 22000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nalorakk / Halazzi / Hexlord Malacrass / Daakara - encounter tracking only.

class ZulAmanUnscriptedBossAI : public CreatureAIScript
{
public:
    ZulAmanUnscriptedBossAI(Creature* pCreature, uint32_t dataId, const char* aggroText, uint32_t aggroSound, const char* deathText, uint32_t deathSound)
        : CreatureAIScript(pCreature), mDataId(dataId), mAggroText(aggroText), mAggroSound(aggroSound), mDeathText(deathText), mDeathSound(deathSound) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, mAggroSound, mAggroText.c_str());
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(mDataId, EncounterStates::InProgress);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(mDataId, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, mDeathSound, mDeathText.c_str());
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(mDataId, EncounterStates::Performed);
        OnBossDefeated();
    }

protected:
    virtual void OnBossDefeated() {}

private:
    uint32_t mDataId;
    std::string mAggroText;
    uint32_t mAggroSound;
    std::string mDeathText;
    uint32_t mDeathSound;
};

// Nalorakk alternates troll form (Brutal Strike on the tank, Surge charging the farthest
// enemy) and bear form (Rend Flesh/Lacerating Slash bleeds, a silencing Deafening Roar) per
// wowhead - this timed alternation is simple enough to keep, unlike the other three bosses'
// mechanics below.
class NalorakkAI : public ZulAmanUnscriptedBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NalorakkAI(c); }
    explicit NalorakkAI(Creature* pCreature) : ZulAmanUnscriptedBossAI(pCreature, DATA_NALORAKK,
        "You be dead soon enough!", 12070, "I... be waitin' on da udda side....", 12077) {}

    // Reference dialogue data for entry 23576 also has four SAY_WAVE_1..4 pull-chant lines
    // ("Get da move on, guards!...", sounds 12066-12069) said before the guard-wave gauntlet
    // that leads into this fight, which this simplified kit doesn't model, so they have no
    // cast point to attach to here.

    void OnCombatStart(Unit* pTarget) override
    {
        ZulAmanUnscriptedBossAI::OnCombatStart(pTarget);
        mBearForm = false;
        scriptEvents.addEvent(EVENT_NALORAKK_BRUTAL_STRIKE, 6000);
        scriptEvents.addEvent(EVENT_NALORAKK_SURGE, 12000);
        scriptEvents.addEvent(EVENT_NALORAKK_FORM_SWAP, 40000);
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 12075, "Mua-ha-ha! Now whatchoo got to say?");
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_NALORAKK_BRUTAL_STRIKE:
                if (!mBearForm)
                    castSpellOnVictim(SPELL_NALORAKK_BRUTAL_STRIKE);
                else
                    castSpellOnVictim(SPELL_NALORAKK_REND_FLESH);
                scriptEvents.addEvent(EVENT_NALORAKK_BRUTAL_STRIKE, 10000);
                break;
            case EVENT_NALORAKK_SURGE:
                if (!mBearForm)
                {
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 12071, "I bring da pain!");
                    if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                        castSpell(pTarget, SPELL_NALORAKK_SURGE);
                }
                else
                {
                    castSpellOnVictim(SPELL_NALORAKK_LACERATING_SLASH);
                }
                scriptEvents.addEvent(EVENT_NALORAKK_SURGE, 18000);
                break;
            case EVENT_NALORAKK_FORM_SWAP:
                mBearForm = !mBearForm;
                if (mBearForm)
                {
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 12072, "You call on da beast, you gonna get more dan you bargain for!");
                    castSpellOnSelf(SPELL_NALORAKK_BEAR_FORM);
                    scriptEvents.addEvent(EVENT_NALORAKK_DEAFENING_ROAR, 8000);
                }
                else
                {
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 12073, "Make way for da Nalorakk!");
                }
                scriptEvents.addEvent(EVENT_NALORAKK_FORM_SWAP, 40000);
                break;
            case EVENT_NALORAKK_DEAFENING_ROAR:
                if (mBearForm)
                    castSpellAOE(SPELL_NALORAKK_DEAFENING_ROAR);
                break;
            default:
                break;
        }
    }

    void OnBossDefeated() override
    {
        static_cast<ZulAmanCataInstanceScript*>(getInstanceScript())->TryOpenHexlordDoors();
    }

private:
    bool mBearForm = false;
};

// Halazzi - per wowhead: an Enrage attack-speed buff, a Water Totem that heals everyone
// nearby (including the raid), and, in his later stages, Lightning/Earth/Flame shocks plus a
// Shred Armor bleed. The split into Halazzi the Worshipper + Spirit of the Lynx (two separate
// bodies that recombine) isn't ported - our simplified framework has no clean way to
// spawn a temporary second "half" of the same boss - so Halazzi instead runs the full ability
// pool continuously from a single body.
class HalazziAI : public ZulAmanUnscriptedBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HalazziAI(c); }
    explicit HalazziAI(Creature* pCreature) : ZulAmanUnscriptedBossAI(pCreature, DATA_HALAZZI,
        "Get on ya knees and bow.... to da fang and claw!", 12020, "Chaga... choka'jinn.", 12028) {}

    // Reference dialogue data for entry 23577 also has SAY_SPLIT ("I fight wit' untamed
    // spirit....", sound 12021) and SAY_COMBINE ("Spirit, come back to me!", sound 12022) tied
    // to the Halazzi the Worshipper/Spirit of the Lynx split mechanic, which the header
    // comment already notes isn't ported, so they have no cast point to attach to here.
    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 12026, "You can't fight da power!");
    }

    void OnCombatStart(Unit* pTarget) override
    {
        ZulAmanUnscriptedBossAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_HALAZZI_ENRAGE, 15000);
        scriptEvents.addEvent(EVENT_HALAZZI_WATER_TOTEM, 20000);
        scriptEvents.addEvent(EVENT_HALAZZI_LIGHTNING_TOTEM, 12000);
        scriptEvents.addEvent(EVENT_HALAZZI_EARTH_SHOCK, 6000);
        scriptEvents.addEvent(EVENT_HALAZZI_FLAME_SHOCK, 9000);
        scriptEvents.addEvent(EVENT_HALAZZI_SHRED_ARMOR, 4000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HALAZZI_ENRAGE:
                castSpellOnSelf(SPELL_HALAZZI_ENRAGE);
                scriptEvents.addEvent(EVENT_HALAZZI_ENRAGE, 30000);
                break;
            case EVENT_HALAZZI_WATER_TOTEM:
                castSpellOnSelf(SPELL_HALAZZI_WATER_TOTEM);
                scriptEvents.addEvent(EVENT_HALAZZI_WATER_TOTEM, 45000);
                break;
            case EVENT_HALAZZI_LIGHTNING_TOTEM:
                castSpellOnSelf(SPELL_HALAZZI_LIGHTNING_TOTEM);
                scriptEvents.addEvent(EVENT_HALAZZI_LIGHTNING_TOTEM, 35000);
                break;
            case EVENT_HALAZZI_EARTH_SHOCK:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_HALAZZI_EARTH_SHOCK);
                scriptEvents.addEvent(EVENT_HALAZZI_EARTH_SHOCK, 14000);
                break;
            case EVENT_HALAZZI_FLAME_SHOCK:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_HALAZZI_FLAME_SHOCK);
                scriptEvents.addEvent(EVENT_HALAZZI_FLAME_SHOCK, 16000);
                break;
            case EVENT_HALAZZI_SHRED_ARMOR:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 12023, "Me gonna carve ya now!");
                castSpellOnVictim(SPELL_HALAZZI_SHRED_ARMOR);
                scriptEvents.addEvent(EVENT_HALAZZI_SHRED_ARMOR, 8000);
                break;
            default:
                break;
        }
    }

    void OnBossDefeated() override
    {
        static_cast<ZulAmanCataInstanceScript*>(getInstanceScript())->OpenLynxTempleExit();
        static_cast<ZulAmanCataInstanceScript*>(getInstanceScript())->TryOpenHexlordDoors();
    }
};

// Hex Lord Malacrass - per wowhead's strategy guide: channels Spirit Bolts (raid-wide Shadow
// damage) roughly every 30s, then Siphon Souls a random player and applies a stacking Drain
// Power raid debuff. His pool of 4-random-of-8 unique minions (Thurg, Alyson Antille, Slither,
// Lord Raadan, Fenstalker, Gazakroth, Koragg, Darkheart) isn't ported - none of the 8 have
// recoverable entry ids or kits in the reference either, so only Malacrass' own core rotation
// is implemented.
class HexlordMalacrassAI : public ZulAmanUnscriptedBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HexlordMalacrassAI(c); }
    explicit HexlordMalacrassAI(Creature* pCreature) : ZulAmanUnscriptedBossAI(pCreature, DATA_HEXLORD_MALACRASS,
        "Da shadow gonna fall on you....", 12041, "Dis not... da end for me!", 12051) {}

    // Reference dialogue data for entry 24239 also has three SAY_PET_DEATH lines ("It not gonna
    // make no difference.", "Dat no bodda me.", "You gonna die worse dan him..", sounds
    // 12048-12050) tied to his 4-random-of-8 minion pool, which the header comment already
    // notes isn't ported, so they have no cast point to attach to here.
    void OnCombatStart(Unit* pTarget) override
    {
        ZulAmanUnscriptedBossAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_MALACRASS_SPIRIT_BOLTS, 30000);
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 12043, "Dis a nightmare ya don' wake up from!");
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MALACRASS_SPIRIT_BOLTS:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 12046, "Darkness comin' for you...");
                castSpellAOE(SPELL_MALACRASS_SPIRIT_BOLTS);
                scriptEvents.addEvent(EVENT_MALACRASS_SIPHON_SOUL, 10000);
                break;
            case EVENT_MALACRASS_SIPHON_SOUL:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 12045, "Your will belong ta me now!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_MALACRASS_SIPHON_SOUL);
                castSpellAOE(SPELL_MALACRASS_DRAIN_POWER);
                scriptEvents.addEvent(EVENT_MALACRASS_SPIRIT_BOLTS, 30000);
                break;
            default:
                break;
        }
    }

    void OnBossDefeated() override
    {
        static_cast<ZulAmanCataInstanceScript*>(getInstanceScript())->OpenMassiveGate();
    }
};

// Daakara - per wowhead: begins in troll form with Whirlwind and Grievous Throw, then at
// 80%/40% health transforms into two of four random animal avatar shapes (Eagle/Bear/
// Dragonhawk/Lynx), each with its own unique kit and adds. That shapeshift system isn't
// ported - none of the four shape-specific kits have recoverable ids for this simplified
// framework to drive reliably - so Daakara instead keeps running his troll-form rotation for
// the whole fight.
class DaakaraAI : public ZulAmanUnscriptedBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DaakaraAI(c); }
    explicit DaakaraAI(Creature* pCreature) : ZulAmanUnscriptedBossAI(pCreature, DATA_DAAKARA,
        "De Zandalari give us strength. Nobody push around de Amani no more!", 24353,
        "Mebbe me fall... but da Amani empire... never going to die.", 24222) {}

    // Reference dialogue data for entry 23863 also has: SAY_INTRO ("Everybody try to keep de
    // Amani Empire down...", sound 24231, a longer lore line that could replace/precede
    // SAY_AGGRO on first pull), SAY_FIRE_BREATH ("Dere be no hidin' from da eagle!", sound
    // 24225), and four SAY_TRANSFORMS_*/two SAY_ABSORBS_* lines tied to the animal-avatar
    // shapeshift system the header comment already notes isn't ported - none have a cast
    // point to attach to in this simplified troll-form-only kit.
    void OnCombatStart(Unit* pTarget) override
    {
        ZulAmanUnscriptedBossAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_DAAKARA_WHIRLWIND, 14000);
        scriptEvents.addEvent(EVENT_DAAKARA_GRIEVOUS_THROW, 8000);
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24226, "Got me some new tricks... like me brudda bear....");
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DAAKARA_WHIRLWIND:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Daakara spins into a wild Whirlwind!");
                castSpellAOE(SPELL_DAAKARA_WHIRLWIND);
                scriptEvents.addEvent(EVENT_DAAKARA_WHIRLWIND, 22000);
                break;
            case EVENT_DAAKARA_GRIEVOUS_THROW:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_DAAKARA_GRIEVOUS_THROW);
                scriptEvents.addEvent(EVENT_DAAKARA_GRIEVOUS_THROW, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash: Amanishi Guardian

class AmanishiGuardianAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmanishiGuardianAI(c); }
    explicit AmanishiGuardianAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    // Reference dialogue data for entry 23597 also has a "%s becomes enraged!" emote line (no
    // sound) with no health-percent trigger modeled for this trash type to attach it to.
    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 12104, "More intruders! Sound da alarm!");
        scriptEvents.addEvent(EVENT_AMANISHI_GUARDIAN_REND, 5000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_AMANISHI_GUARDIAN_REND)
        {
            castSpellOnVictim(SPELL_AMANISHI_GUARDIAN_REND);
            scriptEvents.addEvent(EVENT_AMANISHI_GUARDIAN_REND, 14000);
        }
    }
};

class AmanishiScoutAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmanishiScoutAI(c); }
    explicit AmanishiScoutAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 12104, "Invaders! Sound the alarm!");
        castSpellOnSelf(SPELL_AMANISHI_SCOUT_CURSE);
        scriptEvents.addEvent(EVENT_AMANISHI_SCOUT_AMBUSH, 2000);
        scriptEvents.addEvent(EVENT_AMANISHI_SCOUT_SHOOT, 6000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_AMANISHI_SCOUT_AMBUSH:
                castSpellOnVictim(SPELL_AMANISHI_SCOUT_AMBUSH);
                scriptEvents.addEvent(EVENT_AMANISHI_SCOUT_AMBUSH, 4000);
                break;
            case EVENT_AMANISHI_SCOUT_SHOOT:
                castSpellOnVictim(SPELL_AMANISHI_SCOUT_SHOOT);
                scriptEvents.addEvent(EVENT_AMANISHI_SCOUT_SHOOT, 20000);
                break;
            default:
                break;
        }
    }
};

class AmaniElderLynxAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmaniElderLynxAI(c); }
    explicit AmaniElderLynxAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mEnrageCast = false;
        scriptEvents.addEvent(EVENT_ELDER_LYNX_CLAW, 12000);
        scriptEvents.addEvent(EVENT_ELDER_LYNX_BITE, 1500);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mEnrageCast && _getHealthPercent() <= 30)
        {
            mEnrageCast = true;
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Amani Elder Lynx becomes enraged!");
            castSpellOnSelf(SPELL_ELDER_LYNX_ENRAGE);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ELDER_LYNX_CLAW:
                castSpellOnVictim(SPELL_ELDER_LYNX_CLAW);
                scriptEvents.addEvent(EVENT_ELDER_LYNX_CLAW, 20000);
                break;
            case EVENT_ELDER_LYNX_BITE:
                castSpellOnVictim(SPELL_ELDER_LYNX_BITE);
                scriptEvents.addEvent(EVENT_ELDER_LYNX_BITE, 9000);
                break;
            default:
                break;
        }
    }

private:
    bool mEnrageCast = false;
};

void SetupZulAmanCata(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ZUL_AMAN_CATACLYSM, &ZulAmanCataInstanceScript::Create);

    mgr->register_creature_script(BOSS_AKILZON, &AkilzonAI::Create);
    mgr->register_creature_script(BOSS_NALORAKK, &NalorakkAI::Create);
    mgr->register_creature_script(BOSS_JANALAI, &JanalaiAI::Create);
    mgr->register_creature_script(BOSS_HALAZZI, &HalazziAI::Create);
    mgr->register_creature_script(BOSS_HEXLORD_MALACRASS, &HexlordMalacrassAI::Create);
    mgr->register_creature_script(BOSS_DAAKARA, &DaakaraAI::Create);

    mgr->register_creature_script(NPC_AMANISHI_SCOUT, &AmanishiScoutAI::Create);
    mgr->register_creature_script(NPC_AMANI_ELDER_LYNX, &AmaniElderLynxAI::Create);

    mgr->register_creature_script(NPC_AMANISHI_GUARDIAN, &AmanishiGuardianAI::Create);
}
