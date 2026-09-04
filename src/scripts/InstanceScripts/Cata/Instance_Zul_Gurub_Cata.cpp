/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Zul'Gurub (Cataclysm revamp) - five main bosses (verified against wowhead), hand-ported
// into AscEmu's own CreatureAIScript / InstanceScript framework, plus the four optional
// "animal god" rare-spawn bosses (encounter tracking only, matching the reference
// implementation, which leaves those four unscripted too).
//
// The five main bosses each drop a multi-phase or transform gimmick (Venoxis' totem/venom
// phase, Mandokir's Ohgan revival loop, Kilnara's panther-camouflage phase, Zanzil's
// resurrection-elixir gauntlet, Jindo's spirit-world phase) in favor of a continuous core
// rotation built from each fight's real abilities.

#include "Setup.h"
#include "Instance_Zul_Gurub_Cata.hpp"

#include "Objects/GameObject.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum ZulGurubCataEvents
{
    EVENT_VENOXIS_WHISPERS_OF_HETHISS = 1,
    EVENT_VENOXIS_TOXIC_LINK,
    EVENT_VENOXIS_POOL_OF_ACRID_TEARS,

    EVENT_MANDOKIR_DECAPITATE,
    EVENT_MANDOKIR_BLOODLETTING,
    EVENT_MANDOKIR_SUMMON_OHGAN,
    EVENT_MANDOKIR_DEVASTATING_SLAM,

    EVENT_KILNARA_SHADOW_BOLT,
    EVENT_KILNARA_TEARS_OF_BLOOD,
    EVENT_KILNARA_LASH_OF_ANGUISH,
    EVENT_KILNARA_WAVE_OF_AGONY,

    EVENT_ZANZIL_ZANZILI_FIRE,
    EVENT_ZANZIL_TERRIBLE_TONIC,

    EVENT_JINDO_DEADZONE,
    EVENT_JINDO_SHADOWS_OF_HAKKAR,
    EVENT_JINDO_SHADOW_SPIKE,

    EVENT_QUIN_RITUAL,
    EVENT_CAULDRON_MIXER_A_HEX,
    EVENT_CAULDRON_MIXER_A_TOXIN,
    EVENT_CAULDRON_MIXER_A_SPEED_BUFF,
    EVENT_CAULDRON_MIXER_A_HEX_BOLT,
    EVENT_CAULDRON_MIXER_B_TOXIN,
    EVENT_CAULDRON_MIXER_B_SPEED_BUFF,
    EVENT_CAULDRON_MIXER_B_HEX_BOLT,
    EVENT_RAZZASHI_ADDER_VENOM_SPIT,
    EVENT_RAZZASHI_ADDER_POISON,
    EVENT_WITCH_DOCTOR_HEX,
    EVENT_WITCH_DOCTOR_CURSE,
    EVENT_WITCH_DOCTOR_HEALING_WARD,
    EVENT_HETHISS_PULSE,
    EVENT_BLOOD_DRINKER_FRENZY,
    EVENT_BLOOD_DRINKER_LEECH,
    EVENT_SHADOW_HUNTER_TOXIN,
    EVENT_SHADOW_HUNTER_SHADOW_BOLT,
    EVENT_BETHEKK_PRIEST_SMITE,
    EVENT_BETHEKK_PRIEST_HEAL,
    EVENT_BETHEKK_PRIEST_SHIELD,
    EVENT_PRIDE_OF_BETHEKK_CLAW,
    EVENT_VENOMANCER_TOXIN,
    EVENT_VENOMANCER_WHISPERS,
    EVENT_TIKI_TORCH_FLARE,
    EVENT_SOUL_EATER_DRAIN,
    EVENT_SOUL_EATER_SHIELD,
    EVENT_JUGGERNAUT_SMASH,
    EVENT_HIEROPHANT_ARCANE_BOLT,
    EVENT_HIEROPHANT_ARCANE_STORM,
    EVENT_HIEROPHANT_MANA_BURN,
    EVENT_ARCHON_LIGHTNING_BOLT,
    EVENT_ARCHON_CHAIN_LIGHTNING,
    EVENT_ARCHON_THUNDERSTORM,
    EVENT_ARCHON_STATIC_SHOCK,

    EVENT_GRILEK_AVATAR,
    EVENT_GRILEK_PURSUIT,
    EVENT_GRILEK_RUPTURE_LINE,

    EVENT_HAZZARAH_WRATH,
    EVENT_HAZZARAH_EARTH_SHOCK,
    EVENT_HAZZARAH_SLEEP,

    EVENT_RENATAKI_DEADLY_POISON,
    EVENT_RENATAKI_VANISH_AMBUSH,
    EVENT_RENATAKI_THOUSAND_BLADES,

    EVENT_WUSHOOLAY_LIGHTNING_CLOUD,
    EVENT_WUSHOOLAY_FORKED_LIGHTNING,
    EVENT_WUSHOOLAY_LIGHTNING_RUSH
};

class ZulGurubCataInstanceScript : public InstanceScript
{
public:
    explicit ZulGurubCataInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(ZulGurubCataEncounterCount);
        mVenoxisDoorGuid = 0;
        mMandokirDoorGuid = 0;
        mKilnaraDoorGuid = 0;
        mZanzilDoorGuid = 0;
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ZulGurubCataInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_VENOXIS_COIL:
                mVenoxisDoorGuid = pGameObject->getGuidLow();
                break;
            case GO_ARENA_DOOR_1:
                mMandokirDoorGuid = pGameObject->getGuidLow();
                break;
            case GO_FORCEFIELD:
                mKilnaraDoorGuid = pGameObject->getGuidLow();
                break;
            case GO_ZANZIL_DOOR:
                mZanzilDoorGuid = pGameObject->getGuidLow();
                break;
            default:
                break;
        }
    }

    void OpenDoorForBoss(uint32_t dataId)
    {
        uint32_t doorGuid = 0;
        switch (dataId)
        {
            case DATA_HIGH_PRIEST_VENOXIS: doorGuid = mVenoxisDoorGuid; break;
            case DATA_BLOODLORD_MANDOKIR: doorGuid = mMandokirDoorGuid; break;
            case DATA_HIGH_PRIESTESS_KILNARA: doorGuid = mKilnaraDoorGuid; break;
            case DATA_ZANZIL: doorGuid = mZanzilDoorGuid; break;
            default: break;
        }

        if (doorGuid == 0)
            return;

        if (GameObject* pDoor = GetGameObjectByGuid(doorGuid))
            useDoorOrButton(pDoor);
    }

private:
    uint32_t mVenoxisDoorGuid;
    uint32_t mMandokirDoorGuid;
    uint32_t mKilnaraDoorGuid;
    uint32_t mZanzilDoorGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// High Priest Venoxis

class HighPriestVenoxisAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighPriestVenoxisAI(c); }
    explicit HighPriestVenoxisAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    // Reference dialogue data for entry 52155 also has SAY_TRANSFROM ("Let the coils of death
    // unfurl!", sound 24319), EMOTE_BLOODVENOM, and EMOTE_VENOM_WITHDRAWAL tied to a
    // venom-buildup/withdrawal phase mechanic this simplified kit doesn't model, so they have
    // no cast point to attach to here.
    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24326, "You disssssturb the plans of Gurubashi, little one. It'sss too late for you. Too late for all of you!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_HIGH_PRIEST_VENOXIS, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_VENOXIS_WHISPERS_OF_HETHISS, 6000);
        scriptEvents.addEvent(EVENT_VENOXIS_TOXIC_LINK, 14400);
        scriptEvents.addEvent(EVENT_VENOXIS_POOL_OF_ACRID_TEARS, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HIGH_PRIEST_VENOXIS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24322, "The mortal coil unwindsss...");
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24318, "My death means...nothing...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HIGH_PRIEST_VENOXIS, EncounterStates::Performed);
        static_cast<ZulGurubCataInstanceScript*>(getInstanceScript())->OpenDoorForBoss(DATA_HIGH_PRIEST_VENOXIS);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_VENOXIS_WHISPERS_OF_HETHISS:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 24321, "Hisss word, FILLS me, MY BLOOD IS VENOM, AND YOU WILL BATHE IN THE GLORY OF THE SNAKE GOD!!!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_VENOXIS_WHISPERS_OF_HETHISS);
                scriptEvents.addEvent(EVENT_VENOXIS_WHISPERS_OF_HETHISS, 20000);
                break;
            case EVENT_VENOXIS_TOXIC_LINK:
                castSpellAOE(SPELL_VENOXIS_TOXIC_LINK);
                scriptEvents.addEvent(EVENT_VENOXIS_TOXIC_LINK, 24000);
                break;
            case EVENT_VENOXIS_POOL_OF_ACRID_TEARS:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 24320, "Yesss...ssssuccumb to the venom...");
                castSpellOnVictim(SPELL_VENOXIS_POOL_OF_ACRID_TEARS);
                scriptEvents.addEvent(EVENT_VENOXIS_POOL_OF_ACRID_TEARS, 18000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Bloodlord Mandokir

class BloodlordMandokirAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BloodlordMandokirAI(c); }
    explicit BloodlordMandokirAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    // Reference dialogue data for entry 52151 also has EMOTE_FRENZY/SAY_FRENZY ("%s goes into a
    // frenzy!"/"Off with your head!", sound 24291) tied to a frenzy mechanic this simplified
    // kit doesn't model, so it has no cast point to attach to here.
    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24289, "Make peace, worms. I be deliverin' you to Hakkar myself!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_BLOODLORD_MANDOKIR, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_MANDOKIR_DECAPITATE, 10000);
        scriptEvents.addEvent(EVENT_MANDOKIR_BLOODLETTING, 15000);
        scriptEvents.addEvent(EVENT_MANDOKIR_SUMMON_OHGAN, 20000);
        scriptEvents.addEvent(EVENT_MANDOKIR_DEVASTATING_SLAM, 25000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_BLOODLORD_MANDOKIR, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24293, "Ha ha ha! Is that all you got?");
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24290, "My blood feeds Hakkar! My soul... feeds... Jin'do!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_BLOODLORD_MANDOKIR, EncounterStates::Performed);
        static_cast<ZulGurubCataInstanceScript*>(getInstanceScript())->OpenDoorForBoss(DATA_BLOODLORD_MANDOKIR);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MANDOKIR_DECAPITATE:
                castSpellOnVictim(SPELL_MANDOKIR_DECAPITATE);
                scriptEvents.addEvent(EVENT_MANDOKIR_DECAPITATE, 18000);
                break;
            case EVENT_MANDOKIR_BLOODLETTING:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_MANDOKIR_BLOODLETTING);
                scriptEvents.addEvent(EVENT_MANDOKIR_BLOODLETTING, 20000);
                break;
            case EVENT_MANDOKIR_SUMMON_OHGAN:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 24298, "Go an' get dem, Ohgan! We WON'T be fooled again!");
                castSpellOnSelf(SPELL_MANDOKIR_SUMMON_OHGAN);
                scriptEvents.addEvent(EVENT_MANDOKIR_SUMMON_OHGAN, 45000);
                break;
            case EVENT_MANDOKIR_DEVASTATING_SLAM:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Bloodlord Mandokir begins to cast Devastating Slam!");
                castSpellAOE(SPELL_MANDOKIR_DEVASTATING_SLAM);
                scriptEvents.addEvent(EVENT_MANDOKIR_DEVASTATING_SLAM, 25000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// High Priestess Kilnara

class HighPriestessKilnaraAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighPriestessKilnaraAI(c); }
    explicit HighPriestessKilnaraAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    // Reference dialogue data for entry 52059 also has SAY_TRANSFROM_1/SAY_TRANSFROM_2 ("What
    // have you done? I can't... control... RRAAAARRRGHHH!"/"Haaa ha ha haa! Now, heathens...
    // face the TRUE might of Bethekk!", sounds 24286/24287) tied to a possession-transform
    // phase this simplified kit doesn't model, so they have no cast point to attach to here.
    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24269, "No. NO! Get out! Leave me here with the memory of my sister!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_HIGH_PRIESTESS_KILNARA, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_KILNARA_SHADOW_BOLT, 1500);
        scriptEvents.addEvent(EVENT_KILNARA_TEARS_OF_BLOOD, 8800);
        scriptEvents.addEvent(EVENT_KILNARA_LASH_OF_ANGUISH, 17000);
        scriptEvents.addEvent(EVENT_KILNARA_WAVE_OF_AGONY, 29500);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HIGH_PRIESTESS_KILNARA, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24277, "I told you to leave!");
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24274, "This body is useless!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HIGH_PRIESTESS_KILNARA, EncounterStates::Performed);
        static_cast<ZulGurubCataInstanceScript*>(getInstanceScript())->OpenDoorForBoss(DATA_HIGH_PRIESTESS_KILNARA);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_KILNARA_SHADOW_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_KILNARA_SHADOW_BOLT);
                scriptEvents.addEvent(EVENT_KILNARA_SHADOW_BOLT, 6000);
                break;
            case EVENT_KILNARA_TEARS_OF_BLOOD:
                castSpellOnVictim(SPELL_KILNARA_TEARS_OF_BLOOD);
                scriptEvents.addEvent(EVENT_KILNARA_TEARS_OF_BLOOD, 18000);
                break;
            case EVENT_KILNARA_LASH_OF_ANGUISH:
                castSpellOnVictim(SPELL_KILNARA_LASH_OF_ANGUISH);
                scriptEvents.addEvent(EVENT_KILNARA_LASH_OF_ANGUISH, 20000);
                break;
            case EVENT_KILNARA_WAVE_OF_AGONY:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 24284, "Feel me agony!");
                castSpellAOE(SPELL_KILNARA_WAVE_OF_AGONY);
                scriptEvents.addEvent(EVENT_KILNARA_WAVE_OF_AGONY, 29500);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Zanzil

class ZanzilAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ZanzilAI(c); }
    explicit ZanzilAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    // Reference dialogue data for entry 52053 also has EMOTE_ZANZIL_ZOMBIES/SAY_ZANZIL_ZOMBIES
    // ("Rise up! Zanzil's elixir gives you life!", sound 24352) and EMOTE_ZANZIL_BERSEKER/
    // SAY_ZANZIL_BERSEKER ("Go, little one! Fight them! KILL THEM!", sound 24350) tied to his
    // corpse-resurrection adds mechanic, which this simplified kit doesn't model, so they have
    // no cast point to attach to here.
    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24337, "What? You've come to laugh at Zanzil, too? Not again! I'll make you pay!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ZANZIL, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ZANZIL_ZANZILI_FIRE, 13500);
        scriptEvents.addEvent(EVENT_ZANZIL_TERRIBLE_TONIC, 10500);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ZANZIL, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24343, "How does that taste?");
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24338, "You'll all suffer for this! Zul'Gurub is NOTHING without Zan... Zan...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ZANZIL, EncounterStates::Performed);
        static_cast<ZulGurubCataInstanceScript*>(getInstanceScript())->OpenDoorForBoss(DATA_ZANZIL);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ZANZIL_ZANZILI_FIRE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ZANZIL_ZANZILI_FIRE);
                scriptEvents.addEvent(EVENT_ZANZIL_ZANZILI_FIRE, 14000);
                break;
            case EVENT_ZANZIL_TERRIBLE_TONIC:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 24341, "Breathe deep, friends! Breathe it all in!");
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 24341, "Zanzil fills the area with a toxic gas!");
                castSpellOnSelf(SPELL_ZANZIL_TERRIBLE_TONIC);
                scriptEvents.addEvent(EVENT_ZANZIL_TERRIBLE_TONIC, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Jindo the Godbreaker

class JindoTheGodbreakerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new JindoTheGodbreakerAI(c); }
    explicit JindoTheGodbreakerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    // Reference dialogue data for entry 52148 also has a separate SAY_AGGRO ("Welcome to de
    // great show, friends. Just wait 'til ya see what I got in store for ya.", sound 24254) -
    // this simplified kit only has one engage hook, already used below for SAY_INTRO, so
    // AGGRO has no second cast point to attach to. No SAY_DEATH line exists for this boss in
    // the reference at all.
    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24268, "Meddlesome insects! Now you will feel my wrath!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_JINDO_THE_GODBREAKER, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_JINDO_DEADZONE, 13000);
        scriptEvents.addEvent(EVENT_JINDO_SHADOWS_OF_HAKKAR, 21000);
        scriptEvents.addEvent(EVENT_JINDO_SHADOW_SPIKE, 2500);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_JINDO_THE_GODBREAKER, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_JINDO_THE_GODBREAKER, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_JINDO_DEADZONE:
                castSpellAOE(SPELL_JINDO_DEADZONE);
                scriptEvents.addEvent(EVENT_JINDO_DEADZONE, 22000);
                break;
            case EVENT_JINDO_SHADOWS_OF_HAKKAR:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Jin'do charges his weapon with the Shadows of Hakkar!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_JINDO_SHADOWS_OF_HAKKAR);
                scriptEvents.addEvent(EVENT_JINDO_SHADOWS_OF_HAKKAR, 24000);
                break;
            case EVENT_JINDO_SHADOW_SPIKE:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 24267, "Now, have a taste of Jin'do's true powah!");
                castSpellOnVictim(SPELL_JINDO_SHADOW_SPIKE);
                scriptEvents.addEvent(EVENT_JINDO_SHADOW_SPIKE, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Animal gods - encounter tracking only (unscripted upstream too).

class ZulGurubUnscriptedBossAI : public CreatureAIScript
{
public:
    ZulGurubUnscriptedBossAI(Creature* pCreature, uint32_t dataId) : CreatureAIScript(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
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
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(mDataId, EncounterStates::Performed);
    }

private:
    uint32_t mDataId;
};

// Gri'lek - per wowhead: a melee mob that periodically fixates a random player (Pursuit),
// roots and DoTs them (Entangling Roots) while he chases, and drops random ground-spike
// hazards (Rupture Line). The taunt-immunity during Pursuit isn't replicated - our
// CreatureAIScript has no taunt-lock API - only the root/DoT and ground-hazard parts are.
class GrilekAI : public ZulGurubUnscriptedBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GrilekAI(c); }
    explicit GrilekAI(Creature* pCreature) : ZulGurubUnscriptedBossAI(pCreature, DATA_GRILEK) {}

    void OnCombatStart(Unit* pTarget) override
    {
        ZulGurubUnscriptedBossAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_GRILEK_AVATAR, 20000);
        scriptEvents.addEvent(EVENT_GRILEK_PURSUIT, 15000);
        scriptEvents.addEvent(EVENT_GRILEK_RUPTURE_LINE, 8000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GRILEK_AVATAR:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Gri'lek grows to massive size!");
                castSpellOnSelf(SPELL_GRILEK_AVATAR);
                scriptEvents.addEvent(EVENT_GRILEK_AVATAR, 30000);
                break;
            case EVENT_GRILEK_PURSUIT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                {
                    sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Gri'lek fixates on his prey!");
                    castSpell(pTarget, SPELL_GRILEK_PURSUIT);
                    castSpell(pTarget, SPELL_GRILEK_ENTANGLING_ROOTS);
                }
                scriptEvents.addEvent(EVENT_GRILEK_PURSUIT, 20000);
                break;
            case EVENT_GRILEK_RUPTURE_LINE:
                castSpellAOE(SPELL_GRILEK_RUPTURE_LINE);
                scriptEvents.addEvent(EVENT_GRILEK_RUPTURE_LINE, 12000);
                break;
            default:
                break;
        }
    }
};

// Hazza'rah - per wowhead: alternates Wrath/Earth Shock nature bolts on his current target,
// and at 66%/33% health puts most of the raid to sleep while summoning a Nightmare Illusion
// (real add entry NPC_NIGHTMARE_ILLUSION) that walks toward and kills whichever player it
// reaches. Simplified to two random players per trigger rather than "all but one".
class HazzarahAI : public ZulGurubUnscriptedBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HazzarahAI(c); }
    explicit HazzarahAI(Creature* pCreature) : ZulGurubUnscriptedBossAI(pCreature, DATA_HAZZARAH) {}

    void OnCombatStart(Unit* pTarget) override
    {
        ZulGurubUnscriptedBossAI::OnCombatStart(pTarget);
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24247, "Today, you'll wish you never stirred from your bed!");
        mSleepPhase = 0;
        scriptEvents.addEvent(EVENT_HAZZARAH_WRATH, 4000);
        scriptEvents.addEvent(EVENT_HAZZARAH_EARTH_SHOCK, 9000);
    }

    void OnDied(Unit* pKiller) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24248, "My spirit dreams no longer...");
        ZulGurubUnscriptedBossAI::OnDied(pKiller);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (mSleepPhase < 2 && _getHealthPercent() <= (mSleepPhase == 0 ? 66 : 33))
        {
            ++mSleepPhase;
            sendChatMessage(CHAT_MSG_MONSTER_YELL, mSleepPhase == 1 ? 24249u : 24251u,
                mSleepPhase == 1 ? "Let us see what horrors stir within your nightmares!" : "Slumber... another dream awaits you...");
            for (int i = 0; i < 2; ++i)
            {
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                {
                    castSpell(pTarget, SPELL_HAZZARAH_SLEEP);
                    summonCreature(NPC_NIGHTMARE_ILLUSION, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), pTarget->GetOrientation(), MANUAL_DESPAWN, 30000);
                }
            }
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HAZZARAH_WRATH:
                castSpellOnVictim(SPELL_HAZZARAH_WRATH);
                scriptEvents.addEvent(EVENT_HAZZARAH_WRATH, 7000);
                break;
            case EVENT_HAZZARAH_EARTH_SHOCK:
                castSpellOnVictim(SPELL_HAZZARAH_EARTH_SHOCK);
                scriptEvents.addEvent(EVENT_HAZZARAH_EARTH_SHOCK, 12000);
                break;
            default:
                break;
        }
    }

private:
    int mSleepPhase = 0;
};

// Renataki - per wowhead: a Deadly Poison DoT on the tank, a Vanish -> Ambush burst on a
// random player, and a Thousand Blades whirlwind. Ambush's "90% of max health" nova-style
// burst is approximated with a flat-damage stand-in spell rather than a percentage effect.
class RenatakiAI : public ZulGurubUnscriptedBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RenatakiAI(c); }
    explicit RenatakiAI(Creature* pCreature) : ZulGurubUnscriptedBossAI(pCreature, DATA_RENATAKI) {}

    void OnCombatStart(Unit* pTarget) override
    {
        ZulGurubUnscriptedBossAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_RENATAKI_DEADLY_POISON, 3000);
        scriptEvents.addEvent(EVENT_RENATAKI_VANISH_AMBUSH, 25000);
        scriptEvents.addEvent(EVENT_RENATAKI_THOUSAND_BLADES, 18000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_RENATAKI_DEADLY_POISON:
                castSpellOnVictim(SPELL_RENATAKI_DEADLY_POISON);
                scriptEvents.addEvent(EVENT_RENATAKI_DEADLY_POISON, 3000);
                break;
            case EVENT_RENATAKI_VANISH_AMBUSH:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Renataki vanishes into the shadows!");
                castSpellOnSelf(SPELL_RENATAKI_VANISH);
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_RENATAKI_AMBUSH);
                scriptEvents.addEvent(EVENT_RENATAKI_VANISH_AMBUSH, 30000);
                break;
            case EVENT_RENATAKI_THOUSAND_BLADES:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Renataki spins wildly, hurling blades in all directions!");
                castSpellAOE(SPELL_RENATAKI_THOUSAND_BLADES);
                scriptEvents.addEvent(EVENT_RENATAKI_THOUSAND_BLADES, 26000);
                break;
            default:
                break;
        }
    }
};

// Wushoolay - per wowhead: Lightning Cloud on a random player, a frontal Forked Lightning
// cone, and a Lightning Rush-to-Lightning Rod combo. The rush-to-location movement isn't
// replicated - the AoE knockback is simply cast where Wushoolay currently stands.
class WushoolayAI : public ZulGurubUnscriptedBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WushoolayAI(c); }
    explicit WushoolayAI(Creature* pCreature) : ZulGurubUnscriptedBossAI(pCreature, DATA_WUSHOOLAY) {}

    void OnCombatStart(Unit* pTarget) override
    {
        ZulGurubUnscriptedBossAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_WUSHOOLAY_LIGHTNING_CLOUD, 7000);
        scriptEvents.addEvent(EVENT_WUSHOOLAY_FORKED_LIGHTNING, 13000);
        scriptEvents.addEvent(EVENT_WUSHOOLAY_LIGHTNING_RUSH, 22000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_WUSHOOLAY_LIGHTNING_CLOUD:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_WUSHOOLAY_LIGHTNING_CLOUD);
                scriptEvents.addEvent(EVENT_WUSHOOLAY_LIGHTNING_CLOUD, 20000);
                break;
            case EVENT_WUSHOOLAY_FORKED_LIGHTNING:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Wushoolay unleashes Forked Lightning!");
                castSpellAOE(SPELL_WUSHOOLAY_FORKED_LIGHTNING);
                scriptEvents.addEvent(EVENT_WUSHOOLAY_FORKED_LIGHTNING, 18000);
                break;
            case EVENT_WUSHOOLAY_LIGHTNING_RUSH:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Wushoolay transforms into living lightning!");
                castSpellAOE(SPELL_WUSHOOLAY_LIGHTNING_ROD);
                scriptEvents.addEvent(EVENT_WUSHOOLAY_LIGHTNING_RUSH, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash

class GurubashiCauldronMixerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GurubashiCauldronMixerAI(c); }
    explicit GurubashiCauldronMixerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        castSpellOnSelf(SPELL_CAULDRON_MIXER_A_BREW);
        scriptEvents.addEvent(EVENT_CAULDRON_MIXER_A_HEX, 5000);
        scriptEvents.addEvent(EVENT_CAULDRON_MIXER_A_TOXIN, 5000);
        scriptEvents.addEvent(EVENT_CAULDRON_MIXER_A_SPEED_BUFF, 10000);
        scriptEvents.addEvent(EVENT_CAULDRON_MIXER_A_HEX_BOLT, 21000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CAULDRON_MIXER_A_HEX:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_CAULDRON_MIXER_A_HEX);
                scriptEvents.addEvent(EVENT_CAULDRON_MIXER_A_HEX, 5000);
                break;
            case EVENT_CAULDRON_MIXER_A_TOXIN:
                castSpellOnVictim(SPELL_CAULDRON_MIXER_A_TOXIN);
                scriptEvents.addEvent(EVENT_CAULDRON_MIXER_A_TOXIN, 2500);
                break;
            case EVENT_CAULDRON_MIXER_A_SPEED_BUFF:
                castSpellOnSelf(SPELL_CAULDRON_MIXER_A_SPEED_BUFF);
                scriptEvents.addEvent(EVENT_CAULDRON_MIXER_A_SPEED_BUFF, 20000);
                break;
            case EVENT_CAULDRON_MIXER_A_HEX_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_CAULDRON_MIXER_A_HEX_BOLT);
                scriptEvents.addEvent(EVENT_CAULDRON_MIXER_A_HEX_BOLT, 20000);
                break;
            default:
                break;
        }
    }
};

class GurubashiCauldronMixerVariantAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GurubashiCauldronMixerVariantAI(c); }
    explicit GurubashiCauldronMixerVariantAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        castSpellOnSelf(SPELL_CAULDRON_MIXER_B_BREW);
        scriptEvents.addEvent(EVENT_CAULDRON_MIXER_B_TOXIN, 5000);
        scriptEvents.addEvent(EVENT_CAULDRON_MIXER_B_SPEED_BUFF, 10000);
        scriptEvents.addEvent(EVENT_CAULDRON_MIXER_B_HEX_BOLT, 21000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CAULDRON_MIXER_B_TOXIN:
                castSpellOnVictim(SPELL_CAULDRON_MIXER_B_TOXIN);
                scriptEvents.addEvent(EVENT_CAULDRON_MIXER_B_TOXIN, 2500);
                break;
            case EVENT_CAULDRON_MIXER_B_SPEED_BUFF:
                castSpellOnSelf(SPELL_CAULDRON_MIXER_A_SPEED_BUFF);
                scriptEvents.addEvent(EVENT_CAULDRON_MIXER_B_SPEED_BUFF, 20000);
                break;
            case EVENT_CAULDRON_MIXER_B_HEX_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_CAULDRON_MIXER_A_HEX_BOLT);
                scriptEvents.addEvent(EVENT_CAULDRON_MIXER_B_HEX_BOLT, 20000);
                break;
            default:
                break;
        }
    }
};

class RazzashiAdderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RazzashiAdderAI(c); }
    explicit RazzashiAdderAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_RAZZASHI_ADDER_VENOM_SPIT, 100);
        scriptEvents.addEvent(EVENT_RAZZASHI_ADDER_POISON, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_RAZZASHI_ADDER_VENOM_SPIT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_RAZZASHI_ADDER_VENOM_SPIT);
                scriptEvents.addEvent(EVENT_RAZZASHI_ADDER_VENOM_SPIT, 11000);
                break;
            case EVENT_RAZZASHI_ADDER_POISON:
                castSpellOnVictim(SPELL_RAZZASHI_ADDER_POISON);
                scriptEvents.addEvent(EVENT_RAZZASHI_ADDER_POISON, 15000);
                break;
            default:
                break;
        }
    }
};

class HakkariWitchDoctorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HakkariWitchDoctorAI(c); }
    explicit HakkariWitchDoctorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        castSpellOnSelf(SPELL_WITCH_DOCTOR_MOJO);
        scriptEvents.addEvent(EVENT_WITCH_DOCTOR_HEX, 400);
        scriptEvents.addEvent(EVENT_WITCH_DOCTOR_CURSE, 18000);
        scriptEvents.addEvent(EVENT_WITCH_DOCTOR_HEALING_WARD, 28000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_WITCH_DOCTOR_HEX:
                castSpellOnVictim(SPELL_WITCH_DOCTOR_HEX);
                scriptEvents.addEvent(EVENT_WITCH_DOCTOR_HEX, 2500);
                break;
            case EVENT_WITCH_DOCTOR_CURSE:
                castSpellOnSelf(SPELL_WITCH_DOCTOR_CURSE);
                scriptEvents.addEvent(EVENT_WITCH_DOCTOR_CURSE, 30000);
                break;
            case EVENT_WITCH_DOCTOR_HEALING_WARD:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_WITCH_DOCTOR_HEALING_WARD);
                scriptEvents.addEvent(EVENT_WITCH_DOCTOR_HEALING_WARD, 30000);
                break;
            default:
                break;
        }
    }
};

// Reference has no repeating timer for this NPC's sole ability - approximated with a
// 20s recast interval after the initial on-aggro cast.
class WitchDoctorQuinAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WitchDoctorQuinAI(c); }
    explicit WitchDoctorQuinAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Sacrifice yo' bodies ta Hethiss mah bruddahs, da voodoo will protect you!");
        castSpellOnSelf(SPELL_QUIN_RITUAL);
        scriptEvents.addEvent(EVENT_QUIN_RITUAL, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_QUIN_RITUAL)
        {
            castSpellOnSelf(SPELL_QUIN_RITUAL);
            scriptEvents.addEvent(EVENT_QUIN_RITUAL, 20000);
        }
    }
};

class ChosenOfHethissAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ChosenOfHethissAI(c); }
    explicit ChosenOfHethissAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        castSpellOnSelf(SPELL_HETHISS_CURSE);
        scriptEvents.addEvent(EVENT_HETHISS_PULSE, 200);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_HETHISS_PULSE)
        {
            castSpellOnSelf(SPELL_HETHISS_PULSE);
            scriptEvents.addEvent(EVENT_HETHISS_PULSE, 6300);
        }
    }
};

class GurubashiBloodDrinkerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GurubashiBloodDrinkerAI(c); }
    explicit GurubashiBloodDrinkerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_BLOOD_DRINKER_FRENZY, 7000);
        scriptEvents.addEvent(EVENT_BLOOD_DRINKER_LEECH, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BLOOD_DRINKER_FRENZY:
                castSpellOnVictim(SPELL_BLOOD_DRINKER_FRENZY);
                scriptEvents.addEvent(EVENT_BLOOD_DRINKER_FRENZY, 17000);
                break;
            case EVENT_BLOOD_DRINKER_LEECH:
                castSpellOnSelf(SPELL_BLOOD_DRINKER_LEECH);
                scriptEvents.addEvent(EVENT_BLOOD_DRINKER_LEECH, 31000);
                break;
            default:
                break;
        }
    }
};

class GurubashiShadowHunterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GurubashiShadowHunterAI(c); }
    explicit GurubashiShadowHunterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SHADOW_HUNTER_TOXIN, 400);
        scriptEvents.addEvent(EVENT_SHADOW_HUNTER_SHADOW_BOLT, 7000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SHADOW_HUNTER_TOXIN:
                castSpellOnVictim(SPELL_SHADOW_HUNTER_TOXIN);
                scriptEvents.addEvent(EVENT_SHADOW_HUNTER_TOXIN, 2500);
                break;
            case EVENT_SHADOW_HUNTER_SHADOW_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SHADOW_HUNTER_SHADOW_BOLT);
                scriptEvents.addEvent(EVENT_SHADOW_HUNTER_SHADOW_BOLT, 8000);
                break;
            default:
                break;
        }
    }
};

class LesserPriestOfBethekkAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LesserPriestOfBethekkAI(c); }
    explicit LesserPriestOfBethekkAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_BETHEKK_PRIEST_SMITE, 100);
        scriptEvents.addEvent(EVENT_BETHEKK_PRIEST_HEAL, 10000);
        scriptEvents.addEvent(EVENT_BETHEKK_PRIEST_SHIELD, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BETHEKK_PRIEST_SMITE:
                castSpellOnVictim(SPELL_BETHEKK_PRIEST_SMITE);
                scriptEvents.addEvent(EVENT_BETHEKK_PRIEST_SMITE, 2500);
                break;
            case EVENT_BETHEKK_PRIEST_HEAL:
                castSpellOnSelf(SPELL_BETHEKK_PRIEST_HEAL);
                scriptEvents.addEvent(EVENT_BETHEKK_PRIEST_HEAL, 32000);
                break;
            case EVENT_BETHEKK_PRIEST_SHIELD:
                castSpellOnSelf(SPELL_BETHEKK_PRIEST_SHIELD);
                scriptEvents.addEvent(EVENT_BETHEKK_PRIEST_SHIELD, 30000);
                break;
            default:
                break;
        }
    }
};

class PrideOfBethekkAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PrideOfBethekkAI(c); }
    explicit PrideOfBethekkAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_PRIDE_OF_BETHEKK_CLAW, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_PRIDE_OF_BETHEKK_CLAW)
        {
            if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                castSpell(pTarget, SPELL_PRIDE_OF_BETHEKK_CLAW);
            scriptEvents.addEvent(EVENT_PRIDE_OF_BETHEKK_CLAW, 8000);
        }
    }
};

// Reference drives the venom-phase transform sequence (stealth/shapeshift flags, action
// lists) through AI scripting data events we have no direct equivalent for; only the two clear
// periodic ability casts below are ported.
class VenomancerMauriAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomancerMauriAI(c); }
    explicit VenomancerMauriAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_VENOMANCER_TOXIN, 4000);
        scriptEvents.addEvent(EVENT_VENOMANCER_WHISPERS, 7000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Mauri's death removes the protections from a nearby cauldron!");
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_VENOMANCER_TOXIN:
                castSpellOnVictim(SPELL_VENOMANCER_TOXIN);
                scriptEvents.addEvent(EVENT_VENOMANCER_TOXIN, 7000);
                break;
            case EVENT_VENOMANCER_WHISPERS:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_VENOXIS_WHISPERS_OF_HETHISS);
                scriptEvents.addEvent(EVENT_VENOMANCER_WHISPERS, 9000);
                break;
            default:
                break;
        }
    }
};

class VenomancerTKuluAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomancerTKuluAI(c); }
    explicit VenomancerTKuluAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "I'm gonna turn ya' blood ta venom!");
        scriptEvents.addEvent(EVENT_VENOMANCER_TOXIN, 4000);
        scriptEvents.addEvent(EVENT_VENOXIS_TOXIC_LINK, 7000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "T'kulu's death removes the protections from a nearby cauldron!");
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_VENOMANCER_TOXIN:
                castSpellOnVictim(SPELL_VENOMANCER_TOXIN);
                scriptEvents.addEvent(EVENT_VENOMANCER_TOXIN, 6000);
                break;
            case EVENT_VENOXIS_TOXIC_LINK:
                castSpellOnSelf(SPELL_VENOXIS_TOXIC_LINK);
                scriptEvents.addEvent(EVENT_VENOXIS_TOXIC_LINK, 14000);
                break;
            default:
                break;
        }
    }
};

class TikiTorchAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TikiTorchAI(c); }
    explicit TikiTorchAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_TIKI_TORCH_FLARE, 7000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_TIKI_TORCH_FLARE)
        {
            castSpellOnSelf(SPELL_TIKI_TORCH_FLARE);
            scriptEvents.addEvent(EVENT_TIKI_TORCH_FLARE, 7000);
        }
    }
};

class GurubashiSoulEaterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GurubashiSoulEaterAI(c); }
    explicit GurubashiSoulEaterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SOUL_EATER_DRAIN, 6000);
        scriptEvents.addEvent(EVENT_SOUL_EATER_SHIELD, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SOUL_EATER_DRAIN:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SOUL_EATER_DRAIN);
                scriptEvents.addEvent(EVENT_SOUL_EATER_DRAIN, 15000);
                break;
            case EVENT_SOUL_EATER_SHIELD:
                castSpellOnSelf(SPELL_SOUL_EATER_SHIELD);
                scriptEvents.addEvent(EVENT_SOUL_EATER_SHIELD, 20000);
                break;
            default:
                break;
        }
    }
};

class GurubashiWarmongerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GurubashiWarmongerAI(c); }
    explicit GurubashiWarmongerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { mEnrageCast = false; }

    void AIUpdate(unsigned long /*time_passed*/) override
    {
        if (!mEnrageCast && _getHealthPercent() <= 30)
        {
            mEnrageCast = true;
            castSpellOnSelf(SPELL_WARMONGER_ENRAGE);
        }
    }

private:
    bool mEnrageCast = false;
};

class ZandalariJuggernautAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ZandalariJuggernautAI(c); }
    explicit ZandalariJuggernautAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Ancestors be beside me, let 'dem hear yo' voice!");
        castSpellOnSelf(SPELL_JUGGERNAUT_WAR_STOMP);
        castSpellOnSelf(SPELL_JUGGERNAUT_BATTLE_CRY);
        castSpellOnSelf(SPELL_JUGGERNAUT_CHARGE);
        scriptEvents.addEvent(EVENT_JUGGERNAUT_SMASH, 500);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_JUGGERNAUT_SMASH)
        {
            castSpellOnSelf(SPELL_JUGGERNAUT_SMASH);
            scriptEvents.addEvent(EVENT_JUGGERNAUT_SMASH, 6000);
        }
    }
};

class ZandalariHierophantAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ZandalariHierophantAI(c); }
    explicit ZandalariHierophantAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Ancients of Zandalar, grant dis one da power ta' mock death itself!");
        castSpellOnSelf(SPELL_HIEROPHANT_SHIELD);
        mHealCast = false;
        scriptEvents.addEvent(EVENT_HIEROPHANT_ARCANE_BOLT, 3000);
        scriptEvents.addEvent(EVENT_HIEROPHANT_ARCANE_STORM, 12000);
        scriptEvents.addEvent(EVENT_HIEROPHANT_MANA_BURN, 500);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mHealCast && _getHealthPercent() <= 50)
        {
            mHealCast = true;
            castSpellOnSelf(SPELL_HIEROPHANT_HEAL);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HIEROPHANT_ARCANE_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_HIEROPHANT_ARCANE_BOLT);
                scriptEvents.addEvent(EVENT_HIEROPHANT_ARCANE_BOLT, 5000);
                break;
            case EVENT_HIEROPHANT_ARCANE_STORM:
                castSpellAOE(SPELL_HIEROPHANT_ARCANE_STORM);
                scriptEvents.addEvent(EVENT_HIEROPHANT_ARCANE_STORM, 22000);
                break;
            case EVENT_HIEROPHANT_MANA_BURN:
                castSpellAOE(SPELL_HIEROPHANT_MANA_BURN);
                scriptEvents.addEvent(EVENT_HIEROPHANT_MANA_BURN, 30000);
                break;
            default:
                break;
        }
    }

private:
    bool mHealCast = false;
};

class ZandalariArchonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ZandalariArchonAI(c); }
    explicit ZandalariArchonAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Ancients of Zandalar, mark 'dem for de black road.");
        castSpellOnSelf(SPELL_ARCHON_RITUAL);
        mHealCast = false;
        scriptEvents.addEvent(EVENT_ARCHON_LIGHTNING_BOLT, 3000);
        scriptEvents.addEvent(EVENT_ARCHON_CHAIN_LIGHTNING, 4000);
        scriptEvents.addEvent(EVENT_ARCHON_THUNDERSTORM, 15000);
        scriptEvents.addEvent(EVENT_ARCHON_STATIC_SHOCK, 600);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mHealCast && _getHealthPercent() <= 50)
        {
            mHealCast = true;
            castSpellOnSelf(SPELL_ARCHON_HEAL);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ARCHON_LIGHTNING_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ARCHON_LIGHTNING_BOLT);
                scriptEvents.addEvent(EVENT_ARCHON_LIGHTNING_BOLT, 5000);
                break;
            case EVENT_ARCHON_CHAIN_LIGHTNING:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ARCHON_CHAIN_LIGHTNING);
                scriptEvents.addEvent(EVENT_ARCHON_CHAIN_LIGHTNING, 5000);
                break;
            case EVENT_ARCHON_THUNDERSTORM:
                castSpellOnSelf(SPELL_ARCHON_THUNDERSTORM);
                scriptEvents.addEvent(EVENT_ARCHON_THUNDERSTORM, 16000);
                break;
            case EVENT_ARCHON_STATIC_SHOCK:
                castSpellOnSelf(SPELL_ARCHON_STATIC_SHOCK);
                scriptEvents.addEvent(EVENT_ARCHON_STATIC_SHOCK, 30000);
                break;
            default:
                break;
        }
    }

private:
    bool mHealCast = false;
};

void SetupZulGurubCata(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ZUL_GURUB_CATACLYSM, &ZulGurubCataInstanceScript::Create);

    mgr->register_creature_script(BOSS_HIGH_PRIEST_VENOXIS, &HighPriestVenoxisAI::Create);
    mgr->register_creature_script(BOSS_BLOODLORD_MANDOKIR, &BloodlordMandokirAI::Create);
    mgr->register_creature_script(BOSS_HIGH_PRIESTESS_KILNARA, &HighPriestessKilnaraAI::Create);
    mgr->register_creature_script(BOSS_ZANZIL, &ZanzilAI::Create);
    mgr->register_creature_script(BOSS_JINDO_THE_GODBREAKER, &JindoTheGodbreakerAI::Create);

    mgr->register_creature_script(BOSS_HAZZARAH, &HazzarahAI::Create);
    mgr->register_creature_script(BOSS_RENATAKI, &RenatakiAI::Create);
    mgr->register_creature_script(BOSS_WUSHOOLAY, &WushoolayAI::Create);
    mgr->register_creature_script(BOSS_GRILEK, &GrilekAI::Create);

    mgr->register_creature_script(NPC_GURUBASHI_CAULDRON_MIXER_A, &GurubashiCauldronMixerAI::Create);
    mgr->register_creature_script(NPC_GURUBASHI_CAULDRON_MIXER_C, &GurubashiCauldronMixerAI::Create);
    mgr->register_creature_script(NPC_GURUBASHI_CAULDRON_MIXER_B, &GurubashiCauldronMixerVariantAI::Create);
    mgr->register_creature_script(NPC_RAZZASHI_ADDER, &RazzashiAdderAI::Create);
    mgr->register_creature_script(NPC_HAKKARI_WITCH_DOCTOR, &HakkariWitchDoctorAI::Create);
    mgr->register_creature_script(NPC_WITCH_DOCTOR_QUIN, &WitchDoctorQuinAI::Create);
    mgr->register_creature_script(NPC_CHOSEN_OF_HETHISS, &ChosenOfHethissAI::Create);
    mgr->register_creature_script(NPC_GURUBASHI_BLOOD_DRINKER, &GurubashiBloodDrinkerAI::Create);
    mgr->register_creature_script(NPC_GURUBASHI_SHADOW_HUNTER, &GurubashiShadowHunterAI::Create);
    mgr->register_creature_script(NPC_LESSER_PRIEST_OF_BETHEKK, &LesserPriestOfBethekkAI::Create);
    mgr->register_creature_script(NPC_PRIDE_OF_BETHEKK, &PrideOfBethekkAI::Create);
    mgr->register_creature_script(NPC_VENOMANCER_MAURI, &VenomancerMauriAI::Create);
    mgr->register_creature_script(NPC_VENOMANCER_TKULU, &VenomancerTKuluAI::Create);
    mgr->register_creature_script(NPC_TIKI_TORCH, &TikiTorchAI::Create);
    mgr->register_creature_script(NPC_GURUBASHI_SOUL_EATER, &GurubashiSoulEaterAI::Create);
    mgr->register_creature_script(NPC_GURUBASHI_WARMONGER, &GurubashiWarmongerAI::Create);
    mgr->register_creature_script(NPC_ZANDALARI_JUGGERNAUT, &ZandalariJuggernautAI::Create);
    mgr->register_creature_script(NPC_ZANDALARI_HIEROPHANT, &ZandalariHierophantAI::Create);
    mgr->register_creature_script(NPC_ZANDALARI_ARCHON, &ZandalariArchonAI::Create);
}
