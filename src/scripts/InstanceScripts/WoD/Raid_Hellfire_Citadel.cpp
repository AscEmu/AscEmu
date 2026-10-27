/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Hellfire_Citadel.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

#include <cmath>
#include <vector>

enum HellfireCitadelEvents
{
    EVENT_HFC_BERSERK = 1,

    EVENT_MARTAK_HOWLING_AXE,
    EVENT_MARTAK_SHOCKWAVE,
    EVENT_MARTAK_INFANTRY_WAVE,
    EVENT_MARTAK_DRAGOON_WAVE,

    EVENT_REAVER_BARRAGE,
    EVENT_REAVER_UNSTABLE_ORB,
    EVENT_REAVER_POUNDING,
    EVENT_REAVER_ARTILLERY,
    EVENT_REAVER_BLITZ,
    EVENT_REAVER_BOMBARDMENT_START,
    EVENT_REAVER_FIREBOMB,
    EVENT_REAVER_BOMBARDMENT_END,

    EVENT_KORMROK_SWAT,
    EVENT_KORMROK_POUND,
    EVENT_KORMROK_EXPLOSIVE_BURST,
    EVENT_KORMROK_FOUL_CRUSH,
    EVENT_KORMROK_RUNES,
    EVENT_KORMROK_HANDS,
    EVENT_KORMROK_OUTPOURING,
    EVENT_KORMROK_ENERGY_SWAP,

    EVENT_DIA_NIGHTMARE_VISAGE,
    EVENT_DIA_WAILING_HORROR,
    EVENT_DIA_REAP,
    EVENT_DIA_MARK_OF_THE_NECROMANCER,

    EVENT_GURTOGG_ACIDIC_WOUND,
    EVENT_GURTOGG_FEL_RAGE,
    EVENT_GURTOGG_BLOODBOIL,
    EVENT_GURTOGG_DEMOLISHING_LEAP,

    EVENT_JUBEITHOS_FELSTORM,
    EVENT_JUBEITHOS_FEL_BLADE,
    EVENT_JUBEITHOS_MIRROR_IMAGES,

    EVENT_KILROGG_SHRED_ARMOR,
    EVENT_KILROGG_HEART_SEEKER,
    EVENT_KILROGG_DEATH_THROES,
    EVENT_KILROGG_VISION_OF_DEATH,
    EVENT_KILROGG_BLOODTHIRSTERS,
    EVENT_KILROGG_HULKING_TERROR,

    EVENT_GOREFIEND_SHADOW_OF_DEATH,
    EVENT_GOREFIEND_TOUCH_OF_DOOM,
    EVENT_GOREFIEND_SHARED_FATE,
    EVENT_GOREFIEND_CRUSHING_DARKNESS,
    EVENT_GOREFIEND_FEAST_OF_SOULS,

    EVENT_ISKAR_EYE_OF_ANZU,
    EVENT_ISKAR_PHANTASMAL_WINDS,
    EVENT_ISKAR_PHANTASMAL_WOUNDS,
    EVENT_ISKAR_FEL_CHAKRAM,
    EVENT_ISKAR_FEL_BOMB,

    EVENT_CONSTRUCT_REVERBERATING_BLOW,
    EVENT_CONSTRUCT_FELBLAZE_CHARGE,
    EVENT_CONSTRUCT_VOLATILE_FEL_ORB,

    EVENT_SOCRETHAR_SHADOW_WORD_AGONY,
    EVENT_SOCRETHAR_APOCALYPSE,
    EVENT_SOCRETHAR_DOMINATOR,
    EVENT_SOCRETHAR_SHADOWCALLERS,
    EVENT_SOCRETHAR_HAUNTING_SOULS,

    EVENT_ZAKUUN_SOUL_CLEAVE,
    EVENT_ZAKUUN_RUMBLING_FISSURES,
    EVENT_ZAKUUN_BEFOULED,
    EVENT_ZAKUUN_CAVITATION,
    EVENT_ZAKUUN_SEED_OF_DESTRUCTION,
    EVENT_ZAKUUN_WAKE_OF_DESTRUCTION,
    EVENT_ZAKUUN_DROP_AXES,
    EVENT_ZAKUUN_REARM,

    EVENT_XHUL_FEL_STRIKE,
    EVENT_XHUL_VOID_STRIKE,
    EVENT_XHUL_FEL_SURGE,
    EVENT_XHUL_VOID_SURGE,
    EVENT_XHUL_AKKELION,
    EVENT_XHUL_IMPS,
    EVENT_XHUL_VOIDFIENDS,

    EVENT_VELHARI_ENFORCER,
    EVENT_VELHARI_EDICT,
    EVENT_VELHARI_INFERNAL_TEMPEST,
    EVENT_VELHARI_ANNIHILATING_STRIKE,
    EVENT_VELHARI_TAINTED_SHADOWS,
    EVENT_VELHARI_TOUCH_OF_HARM,
    EVENT_VELHARI_SEAL_OF_DECAY,
    EVENT_VELHARI_FONT_OF_CORRUPTION,
    EVENT_VELHARI_GAVEL,

    EVENT_MANNOROTH_FEL_IMPS,
    EVENT_MANNOROTH_DOOM_LORD,
    EVENT_MANNOROTH_DREAD_INFERNAL,
    EVENT_MANNOROTH_GLAIVE_COMBO,
    EVENT_MANNOROTH_MASSIVE_BLAST,
    EVENT_MANNOROTH_FEL_HELLSTORM,
    EVENT_MANNOROTH_SHADOWFORCE,
    EVENT_MANNOROTH_CURSE_OF_THE_LEGION,
    EVENT_MANNOROTH_CURSE_EXPIRES,
    EVENT_MANNOROTH_GAZE,
    EVENT_MANNOROTH_FELSEEKER,

    EVENT_ARCHIMONDE_DOOMFIRE,
    EVENT_ARCHIMONDE_ALLURE_OF_FLAMES,
    EVENT_ARCHIMONDE_DEATH_BRAND,
    EVENT_ARCHIMONDE_DESECRATE,
    EVENT_ARCHIMONDE_SHADOWFEL_BURST,
    EVENT_ARCHIMONDE_DEATHCALLERS,
    EVENT_ARCHIMONDE_SHACKLED_TORMENT,
    EVENT_ARCHIMONDE_WROUGHT_CHAOS,
    EVENT_ARCHIMONDE_OVERFIEND,
    EVENT_ARCHIMONDE_RAIN_OF_CHAOS,
    EVENT_ARCHIMONDE_DOOMBRINGERS,

    EVENT_FELCASTER_FELFIRE_VOLLEY,
    EVENT_BERSERKER_SLAM,
    EVENT_DRAGOON_SHOCK_PULSE,
    EVENT_TERROR_RENDING_HOWL,
    EVENT_TALONPRIEST_CORRUPTION,
    EVENT_TALONPRIEST_RIPOSTE,
    EVENT_WARDEN_FEL_CONDUIT,
    EVENT_WARDEN_FEL_BOMB,
    EVENT_DOMINATOR_GIFT,
    EVENT_AKKELION_FELBLAZE_FLURRY,
    EVENT_OMNUS_WITHERING_GAZE,
    EVENT_OMNUS_BLACK_HOLE,
    EVENT_ENFORCER_ONSLAUGHT,
    EVENT_HARBINGER_MENDING,
    EVENT_SOVEREIGN_WARD,
    EVENT_DOOM_LORD_MARK_OF_DOOM,
    EVENT_DOOM_LORD_VOLLEY,
    EVENT_FEL_IMP_FEL_BLAST,
    EVENT_DEATHCALLER_SHADOW_BLAST,
    EVENT_DEATHCALLER_DEMONIC_HAVOC
};

enum HellfireCitadelPhases : uint32_t
{
    HFC_PHASE_ONE   = 1,
    HFC_PHASE_TWO   = 2,
    HFC_PHASE_THREE = 3,
    HFC_PHASE_FOUR  = 4
};

namespace
{
    float const HfcFullCircle = 6.2831853f;
    float const HfcHalfCircle = 3.1415927f;

    struct HellfireCitadelDoor
    {
        uint32_t entry;
        uint32_t encounter;
        bool sealsRoom;     // closed while the encounter runs, else opened once it is done
    };

    HellfireCitadelDoor const HellfireCitadelDoors[] =
    {
        { GO_HFC_HELLFIRE_GATE,         DATA_HELLFIRE_ASSAULT,  false },
        { GO_HFC_REINFORCED_DOOR,       DATA_HELLFIRE_ASSAULT,  false },
        { GO_HFC_ZAKUUN_ENTRANCE,       DATA_FEL_LORD_ZAKUUN,   true },
        { GO_HFC_ZAKUUN_EXIT,           DATA_FEL_LORD_ZAKUUN,   false },
        { GO_HFC_SOCRETHAR_DOOR_1,      DATA_SOCRETHAR,         true },
        { GO_HFC_SOCRETHAR_DOOR_2,      DATA_SOCRETHAR,         true },
        { GO_HFC_SOCRETHAR_DOOR_3,      DATA_SOCRETHAR,         true },
        { GO_HFC_SOCRETHAR_DOOR_4,      DATA_SOCRETHAR,         true },
        { GO_HFC_ISKAR_DOOR_1,          DATA_SHADOW_LORD_ISKAR, true },
        { GO_HFC_ISKAR_DOOR_2,          DATA_SHADOW_LORD_ISKAR, true },
        { GO_HFC_ISKAR_DOOR_3,          DATA_SHADOW_LORD_ISKAR, true },
        { GO_HFC_ISKAR_DOOR_4,          DATA_SHADOW_LORD_ISKAR, true },
        { GO_HFC_VELHARI_EXIT_1,        DATA_TYRANT_VELHARI,    false },
        { GO_HFC_VELHARI_EXIT_2,        DATA_TYRANT_VELHARI,    false },
        { GO_HFC_VELHARI_ENTRANCE_1,    DATA_TYRANT_VELHARI,    true },
        { GO_HFC_VELHARI_ENTRANCE_2,    DATA_TYRANT_VELHARI,    true },
        { GO_HFC_DARK_PORTAL_FIRE_WALL, DATA_ARCHIMONDE,        true }
    };

    uint32_t const KormrokEnergyCycle[] = { SPELL_KORMROK_SHADOW_ENERGY, SPELL_KORMROK_EXPLOSIVE_ENERGY, SPELL_KORMROK_FOUL_ENERGY };

    std::vector<uint32_t> const HighCouncilMembers = { BOSS_DIA_DARKWHISPER, BOSS_GURTOGG_BLOODBOIL, BOSS_BLADEMASTER_JUBEITHOS };

    // point on a circle around the creature, angle relative to its facing, facing the center
    LocationVector hfcPointAround(Creature* creature, float distance, float angle)
    {
        LocationVector point = creature->GetPosition();
        float const direction = creature->GetOrientation() + angle;
        point.x += distance * std::cos(direction);
        point.y += distance * std::sin(direction);
        point.o = LocationVector::normalizeOrientation(direction + HfcHalfCircle);
        return point;
    }

    // spreads count summons evenly around the summoner, durationMs 0 keeps them until despawnAll
    void hfcSummonRing(CreatureAIScript* ai, uint32_t entry, uint32_t count, float distance, uint32_t durationMs = 0)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            LocationVector const point = hfcPointAround(ai->getCreature(), distance, HfcFullCircle * static_cast<float>(i) / static_cast<float>(count));
            if (durationMs != 0)
                ai->summonCreature(entry, point, TIMED_DESPAWN, durationMs);
            else
                ai->summonCreature(entry, point, MANUAL_DESPAWN);
        }
    }

    void hfcCastOnRandomPlayer(CreatureAIScript* ai, uint32_t spellId, float minRange = 0.0f)
    {
        if (Unit* target = ai->selectUnitTarget(FilterArgs(TargetFilter_Player, minRange, 100.0f)))
            ai->castSpell(target, spellId);
    }
}

class HellfireCitadelInstanceScript : public InstanceScript
{
public:
    explicit HellfireCitadelInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(HellfireCitadelEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new HellfireCitadelInstanceScript(pMapMgr); }

    // a credited member falling before its partners must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && id == DATA_HELLFIRE_HIGH_COUNCIL && getBossState(id) == EncounterStates::InProgress)
        {
            for (Creature* pMember : getCreatureSetForEntries(HighCouncilMembers))
            {
                if (pMember->isAlive())
                    return false;
            }
        }

        return InstanceScript::setBossState(id, state);
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        for (HellfireCitadelDoor const& door : HellfireCitadelDoors)
        {
            if (door.entry != pGameObject->getEntry())
                continue;

            if (door.sealsRoom)
            {
                mRoomDoorGuids[door.encounter].push_back(pGameObject->GetNewGUID());
                setDoorOpen(pGameObject, getBossState(door.encounter) != EncounterStates::InProgress);
            }
            else
            {
                mPassageDoorGuids[door.encounter].push_back(pGameObject->GetNewGUID());
                setDoorOpen(pGameObject, getBossState(door.encounter) == EncounterStates::Performed);
            }
            break;
        }
    }

    void OnEncounterStateChange(uint32_t encounter, uint32_t state) override
    {
        if (encounter >= HellfireCitadelEncounterCount)
            return;

        for (WoWGuid const& guid : mRoomDoorGuids[encounter])
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                setDoorOpen(pDoor, state != EncounterStates::InProgress);
        }

        if (state != EncounterStates::Performed)
            return;

        for (WoWGuid const& guid : mPassageDoorGuids[encounter])
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                setDoorOpen(pDoor, true);
        }
    }

    // the first council member to engage starts the shared encounter
    bool claimHighCouncil()
    {
        if (mHighCouncilActive)
            return false;

        mHighCouncilActive = true;
        return true;
    }

    // true for the first caller after an engage, keeps wipe handling to one member
    bool releaseHighCouncil()
    {
        if (!mHighCouncilActive)
            return false;

        mHighCouncilActive = false;
        return true;
    }

private:
    // the encounter decides, the spawn state of a door is not reliable
    void setDoorOpen(GameObject* pDoor, bool open)
    {
        uint8_t const state = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != state)
            pDoor->setState(state);
    }

    std::vector<WoWGuid> mRoomDoorGuids[HellfireCitadelEncounterCount];
    std::vector<WoWGuid> mPassageDoorGuids[HellfireCitadelEncounterCount];
    bool mHighCouncilActive = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hellfire Assault - Siegemaster Mar'tak

class HellfireCitadelMartakAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelMartakAI(c); }
    explicit HellfireCitadelMartakAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_HELLFIRE_ASSAULT, EncounterStates::InProgress);

        castSpellOnSelf(SPELL_MARTAK_INSPIRING_PRESENCE, true);
        scriptEvents.addEvent(EVENT_MARTAK_HOWLING_AXE, 8000);
        scriptEvents.addEvent(EVENT_MARTAK_SHOCKWAVE, 15000);
        scriptEvents.addEvent(EVENT_MARTAK_INFANTRY_WAVE, 5000);
        scriptEvents.addEvent(EVENT_MARTAK_DRAGOON_WAVE, 35000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HELLFIRE_ASSAULT, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
        _removeAllAuras();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HELLFIRE_ASSAULT, EncounterStates::Performed);
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MARTAK_HOWLING_AXE:
                hfcCastOnRandomPlayer(this, SPELL_MARTAK_HOWLING_AXE);
                scriptEvents.addEvent(EVENT_MARTAK_HOWLING_AXE, 22000);
                break;
            case EVENT_MARTAK_SHOCKWAVE:
                castSpellOnVictim(SPELL_MARTAK_SHOCKWAVE);
                scriptEvents.addEvent(EVENT_MARTAK_SHOCKWAVE, 25000);
                break;
            case EVENT_MARTAK_INFANTRY_WAVE:
                hfcSummonRing(this, NPC_GOREBOUND_FELCASTER, 2, 25.0f);
                hfcSummonRing(this, NPC_HULKING_BERSERKER, 1, 20.0f);
                scriptEvents.addEvent(EVENT_MARTAK_INFANTRY_WAVE, 40000);
                break;
            case EVENT_MARTAK_DRAGOON_WAVE:
                hfcSummonRing(this, NPC_IRON_DRAGOON, 2, 22.0f);
                scriptEvents.addEvent(EVENT_MARTAK_DRAGOON_WAVE, 50000);
                break;
            default:
                break;
        }
    }
};

class HellfireCitadelFelcasterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelFelcasterAI(c); }
    explicit HellfireCitadelFelcasterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_FELCASTER_FELFIRE_VOLLEY, 4000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_FELCASTER_FELFIRE_VOLLEY)
        {
            castSpellAOE(SPELL_FELCASTER_FELFIRE_VOLLEY);
            scriptEvents.addEvent(EVENT_FELCASTER_FELFIRE_VOLLEY, 12000);
        }
    }
};

class HellfireCitadelBerserkerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelBerserkerAI(c); }
    explicit HellfireCitadelBerserkerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_BERSERKER_SLAM, 6000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_BERSERKER_SLAM)
        {
            castSpellOnVictim(SPELL_BERSERKER_SLAM);
            scriptEvents.addEvent(EVENT_BERSERKER_SLAM, 14000);
        }
    }
};

class HellfireCitadelDragoonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelDragoonAI(c); }
    explicit HellfireCitadelDragoonAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_DRAGOON_SHOCK_PULSE, 7000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_DRAGOON_SHOCK_PULSE)
        {
            hfcCastOnRandomPlayer(this, SPELL_DRAGOON_CONDUCTED_SHOCK_PULSE);
            scriptEvents.addEvent(EVENT_DRAGOON_SHOCK_PULSE, 16000);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Iron Reaver

class HellfireCitadelIronReaverAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelIronReaverAI(c); }
    explicit HellfireCitadelIronReaverAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_IRON_REAVER, EncounterStates::InProgress);

        mPhase = HFC_PHASE_ONE;
        scriptEvents.addEvent(EVENT_HFC_BERSERK, 600000);
        scriptEvents.addEvent(EVENT_REAVER_UNSTABLE_ORB, 9000, HFC_PHASE_ONE);
        scriptEvents.addEvent(EVENT_REAVER_BARRAGE, 12000, HFC_PHASE_ONE);
        scriptEvents.addEvent(EVENT_REAVER_ARTILLERY, 16000, HFC_PHASE_ONE);
        scriptEvents.addEvent(EVENT_REAVER_POUNDING, 25000, HFC_PHASE_ONE);
        scriptEvents.addEvent(EVENT_REAVER_BLITZ, 35000, HFC_PHASE_ONE);
        scriptEvents.addEvent(EVENT_REAVER_BOMBARDMENT_START, 60000, HFC_PHASE_ONE);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_IRON_REAVER, EncounterStates::Failed);
        scriptEvents.resetEvents();
        _removeAllAuras();
        leaveBombardment();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_IRON_REAVER, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mPhase);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_REAVER_UNSTABLE_ORB:
                hfcCastOnRandomPlayer(this, SPELL_REAVER_UNSTABLE_ORB, 10.0f);
                scriptEvents.addEvent(EVENT_REAVER_UNSTABLE_ORB, 12000, HFC_PHASE_ONE);
                break;
            case EVENT_REAVER_BARRAGE:
                castSpellAOE(SPELL_REAVER_BARRAGE);
                scriptEvents.addEvent(EVENT_REAVER_BARRAGE, 18000, HFC_PHASE_ONE);
                break;
            case EVENT_REAVER_ARTILLERY:
                hfcCastOnRandomPlayer(this, SPELL_REAVER_ARTILLERY);
                scriptEvents.addEvent(EVENT_REAVER_ARTILLERY, 20000, HFC_PHASE_ONE);
                break;
            case EVENT_REAVER_POUNDING:
                castSpellOnSelf(SPELL_REAVER_POUNDING);
                scriptEvents.addEvent(EVENT_REAVER_POUNDING, 30000, HFC_PHASE_ONE);
                break;
            case EVENT_REAVER_BLITZ:
                hfcCastOnRandomPlayer(this, SPELL_REAVER_BLITZ, 8.0f);
                scriptEvents.addEvent(EVENT_REAVER_BLITZ, 35000, HFC_PHASE_ONE);
                break;
            case EVENT_REAVER_BOMBARDMENT_START:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Iron Reaver begins a firebomb bombardment!");
                mPhase = HFC_PHASE_TWO;
                setRooted(true);
                _setMeleeDisabled(true);
                scriptEvents.addEvent(EVENT_REAVER_FIREBOMB, 3000, HFC_PHASE_TWO);
                scriptEvents.addEvent(EVENT_REAVER_BOMBARDMENT_END, 30000, HFC_PHASE_TWO);
                break;
            case EVENT_REAVER_FIREBOMB:
                castSpellAOE(SPELL_REAVER_FIREBOMB);
                scriptEvents.addEvent(EVENT_REAVER_FIREBOMB, 8000, HFC_PHASE_TWO);
                break;
            case EVENT_REAVER_BOMBARDMENT_END:
                scriptEvents.removeEvent(EVENT_REAVER_FIREBOMB);
                leaveBombardment();
                castSpellAOE(SPELL_REAVER_FALLING_SLAM);
                scriptEvents.addEvent(EVENT_REAVER_BOMBARDMENT_START, 60000, HFC_PHASE_ONE);
                break;
            default:
                break;
        }
    }

private:
    void leaveBombardment()
    {
        mPhase = HFC_PHASE_ONE;
        setRooted(false);
        _setMeleeDisabled(false);
    }

    uint32_t mPhase = HFC_PHASE_ONE;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kormrok

class HellfireCitadelKormrokAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelKormrokAI(c); }
    explicit HellfireCitadelKormrokAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_KORMROK, EncounterStates::InProgress);

        mEnergyAura = 0;
        mEnergyIndex = 0;
        scriptEvents.addEvent(EVENT_HFC_BERSERK, 540000);
        scriptEvents.addEvent(EVENT_KORMROK_ENERGY_SWAP, 10000);
        scriptEvents.addEvent(EVENT_KORMROK_RUNES, 15000);
        scriptEvents.addEvent(EVENT_KORMROK_SWAT, 20000);
        scriptEvents.addEvent(EVENT_KORMROK_HANDS, 25000);
        scriptEvents.addEvent(EVENT_KORMROK_EXPLOSIVE_BURST, 28000);
        scriptEvents.addEvent(EVENT_KORMROK_FOUL_CRUSH, 32000);
        scriptEvents.addEvent(EVENT_KORMROK_OUTPOURING, 35000);
        scriptEvents.addEvent(EVENT_KORMROK_POUND, 40000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_KORMROK, EncounterStates::Failed);
        scriptEvents.resetEvents();
        _removeAllAuras();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_KORMROK, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_KORMROK_ENERGY_SWAP:
                if (mEnergyAura != 0)
                    _removeAura(mEnergyAura);
                mEnergyAura = KormrokEnergyCycle[mEnergyIndex % 3];
                ++mEnergyIndex;
                castSpellOnSelf(mEnergyAura, true);
                scriptEvents.addEvent(EVENT_KORMROK_ENERGY_SWAP, 60000);
                break;
            case EVENT_KORMROK_RUNES:
                castSpellAOE(mEnergyAura == SPELL_KORMROK_EXPLOSIVE_ENERGY ? SPELL_KORMROK_EMPOWERED_EXPLOSIVE_RUNES : SPELL_KORMROK_EXPLOSIVE_RUNES);
                scriptEvents.addEvent(EVENT_KORMROK_RUNES, 30000);
                break;
            case EVENT_KORMROK_HANDS:
                castSpellAOE(mEnergyAura == SPELL_KORMROK_SHADOW_ENERGY ? SPELL_KORMROK_DRAGGING_HANDS : SPELL_KORMROK_GRASPING_HANDS);
                scriptEvents.addEvent(EVENT_KORMROK_HANDS, 30000);
                break;
            case EVENT_KORMROK_OUTPOURING:
                castSpellAOE(mEnergyAura == SPELL_KORMROK_FOUL_ENERGY ? SPELL_KORMROK_EMPOWERED_FEL_OUTPOURING : SPELL_KORMROK_FEL_OUTPOURING);
                scriptEvents.addEvent(EVENT_KORMROK_OUTPOURING, 30000);
                break;
            case EVENT_KORMROK_SWAT:
                castSpellOnVictim(SPELL_KORMROK_SWAT);
                scriptEvents.addEvent(EVENT_KORMROK_SWAT, 33000);
                break;
            case EVENT_KORMROK_EXPLOSIVE_BURST:
                hfcCastOnRandomPlayer(this, SPELL_KORMROK_EXPLOSIVE_BURST);
                scriptEvents.addEvent(EVENT_KORMROK_EXPLOSIVE_BURST, 30000);
                break;
            case EVENT_KORMROK_FOUL_CRUSH:
                castSpellOnVictim(SPELL_KORMROK_FOUL_CRUSH);
                scriptEvents.addEvent(EVENT_KORMROK_FOUL_CRUSH, 40000);
                break;
            case EVENT_KORMROK_POUND:
                castSpellOnSelf(SPELL_KORMROK_POUND);
                scriptEvents.addEvent(EVENT_KORMROK_POUND, 45000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mEnergyAura = 0;
    uint32_t mEnergyIndex = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hellfire High Council - one encounter slot for Dia, Gurtogg and Jubei'thos

class HellfireCitadelCouncilMemberAI : public CreatureAIScript
{
public:
    explicit HellfireCitadelCouncilMemberAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        scriptEvents.resetEvents();
        summons.despawnAll();
        _removeAllAuras();

        if (!councilInstance()->releaseHighCouncil())
            return;

        getInstanceScript()->setBossState(DATA_HELLFIRE_HIGH_COUNCIL, EncounterStates::Failed);

        // fallen members return for the next attempt
        for (Creature* member : getInstanceScript()->getCreatureSetForEntries(HighCouncilMembers))
        {
            if (member->isAlive())
                continue;

            if (CreatureAIScript* memberAI = member->GetScript())
                memberAI->despawn(1000, 5000);
        }
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        summons.despawnAll();

        for (Creature* member : getInstanceScript()->getCreatureSetForEntries(HighCouncilMembers))
        {
            if (member != getCreature() && member->isAlive())
                return;
        }

        councilInstance()->releaseHighCouncil();
        getInstanceScript()->setBossState(DATA_HELLFIRE_HIGH_COUNCIL, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

protected:
    void engageHighCouncil()
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        if (!councilInstance()->claimHighCouncil())
            return;

        getInstanceScript()->setBossState(DATA_HELLFIRE_HIGH_COUNCIL, EncounterStates::InProgress);
        for (Creature* member : getInstanceScript()->getCreatureSetForEntries(HighCouncilMembers))
        {
            if (member != getCreature() && member->isAlive())
                setZoneWideCombat(member);
        }
    }

    HellfireCitadelInstanceScript* councilInstance() { return static_cast<HellfireCitadelInstanceScript*>(getInstanceScript()); }
};

class HellfireCitadelDiaDarkwhisperAI : public HellfireCitadelCouncilMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelDiaDarkwhisperAI(c); }
    explicit HellfireCitadelDiaDarkwhisperAI(Creature* pCreature) : HellfireCitadelCouncilMemberAI(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        engageHighCouncil();
        scriptEvents.addEvent(EVENT_HFC_BERSERK, 600000);
        scriptEvents.addEvent(EVENT_DIA_MARK_OF_THE_NECROMANCER, 10000);
        scriptEvents.addEvent(EVENT_DIA_REAP, 20000);
        scriptEvents.addEvent(EVENT_DIA_NIGHTMARE_VISAGE, 25000);
        scriptEvents.addEvent(EVENT_DIA_WAILING_HORROR, 40000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_DIA_MARK_OF_THE_NECROMANCER:
                hfcCastOnRandomPlayer(this, SPELL_DIA_MARK_OF_THE_NECROMANCER);
                scriptEvents.addEvent(EVENT_DIA_MARK_OF_THE_NECROMANCER, 25000);
                break;
            case EVENT_DIA_REAP:
                hfcCastOnRandomPlayer(this, SPELL_DIA_REAP);
                scriptEvents.addEvent(EVENT_DIA_REAP, 35000);
                break;
            case EVENT_DIA_NIGHTMARE_VISAGE:
                castSpellOnVictim(SPELL_DIA_NIGHTMARE_VISAGE);
                scriptEvents.addEvent(EVENT_DIA_NIGHTMARE_VISAGE, 30000);
                break;
            case EVENT_DIA_WAILING_HORROR:
                castSpellAOE(SPELL_DIA_WAILING_HORROR);
                scriptEvents.addEvent(EVENT_DIA_WAILING_HORROR, 45000);
                break;
            default:
                break;
        }
    }
};

class HellfireCitadelGurtoggAI : public HellfireCitadelCouncilMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelGurtoggAI(c); }
    explicit HellfireCitadelGurtoggAI(Creature* pCreature) : HellfireCitadelCouncilMemberAI(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        engageHighCouncil();
        mLeapsStarted = false;
        scriptEvents.addEvent(EVENT_HFC_BERSERK, 600000);
        scriptEvents.addEvent(EVENT_GURTOGG_ACIDIC_WOUND, 6000);
        scriptEvents.addEvent(EVENT_GURTOGG_BLOODBOIL, 15000);
        scriptEvents.addEvent(EVENT_GURTOGG_FEL_RAGE, 30000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        // keeps leaping between players below 30%
        if (!mLeapsStarted && _getHealthPercent() <= 30)
        {
            mLeapsStarted = true;
            scriptEvents.addEvent(EVENT_GURTOGG_DEMOLISHING_LEAP, 1000);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_GURTOGG_ACIDIC_WOUND:
                castSpellOnVictim(SPELL_GURTOGG_ACIDIC_WOUND);
                scriptEvents.addEvent(EVENT_GURTOGG_ACIDIC_WOUND, 12000);
                break;
            case EVENT_GURTOGG_BLOODBOIL:
                castSpellAOE(SPELL_GURTOGG_BLOODBOIL);
                scriptEvents.addEvent(EVENT_GURTOGG_BLOODBOIL, 22000);
                break;
            case EVENT_GURTOGG_FEL_RAGE:
                hfcCastOnRandomPlayer(this, SPELL_GURTOGG_FEL_RAGE);
                scriptEvents.addEvent(EVENT_GURTOGG_FEL_RAGE, 40000);
                break;
            case EVENT_GURTOGG_DEMOLISHING_LEAP:
                hfcCastOnRandomPlayer(this, SPELL_GURTOGG_DEMOLISHING_LEAP, 8.0f);
                scriptEvents.addEvent(EVENT_GURTOGG_DEMOLISHING_LEAP, 20000);
                break;
            default:
                break;
        }
    }

private:
    bool mLeapsStarted = false;
};

class HellfireCitadelJubeithosAI : public HellfireCitadelCouncilMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelJubeithosAI(c); }
    explicit HellfireCitadelJubeithosAI(Creature* pCreature) : HellfireCitadelCouncilMemberAI(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        engageHighCouncil();
        scriptEvents.addEvent(EVENT_HFC_BERSERK, 600000);
        scriptEvents.addEvent(EVENT_JUBEITHOS_FEL_BLADE, 12000);
        scriptEvents.addEvent(EVENT_JUBEITHOS_FELSTORM, 30000);
        scriptEvents.addEvent(EVENT_JUBEITHOS_MIRROR_IMAGES, 50000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_JUBEITHOS_FEL_BLADE:
                hfcCastOnRandomPlayer(this, SPELL_JUBEITHOS_FEL_BLADE);
                scriptEvents.addEvent(EVENT_JUBEITHOS_FEL_BLADE, 18000);
                break;
            case EVENT_JUBEITHOS_FELSTORM:
                castSpellOnSelf(SPELL_JUBEITHOS_FELSTORM);
                scriptEvents.addEvent(EVENT_JUBEITHOS_FELSTORM, 45000);
                break;
            case EVENT_JUBEITHOS_MIRROR_IMAGES:
                castSpellOnSelf(SPELL_JUBEITHOS_MIRROR_IMAGES, true);
                hfcSummonRing(this, NPC_JUBEITHOS_MIRROR_IMAGE, 3, 8.0f, 30000);
                scriptEvents.addEvent(EVENT_JUBEITHOS_MIRROR_IMAGES, 60000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kilrogg Deadeye

class HellfireCitadelKilroggAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelKilroggAI(c); }
    explicit HellfireCitadelKilroggAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_KILROGG_DEADEYE, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_HFC_BERSERK, 600000);
        scriptEvents.addEvent(EVENT_KILROGG_SHRED_ARMOR, 8000);
        scriptEvents.addEvent(EVENT_KILROGG_BLOODTHIRSTERS, 10000);
        scriptEvents.addEvent(EVENT_KILROGG_HEART_SEEKER, 20000);
        scriptEvents.addEvent(EVENT_KILROGG_DEATH_THROES, 50000);
        scriptEvents.addEvent(EVENT_KILROGG_VISION_OF_DEATH, 60000);
        scriptEvents.addEvent(EVENT_KILROGG_HULKING_TERROR, 70000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_KILROGG_DEADEYE, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
        _removeAllAuras();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_KILROGG_DEADEYE, EncounterStates::Performed);
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_KILROGG_SHRED_ARMOR:
                castSpellOnVictim(SPELL_KILROGG_SHRED_ARMOR);
                scriptEvents.addEvent(EVENT_KILROGG_SHRED_ARMOR, 15000);
                break;
            case EVENT_KILROGG_HEART_SEEKER:
                hfcCastOnRandomPlayer(this, SPELL_KILROGG_HEART_SEEKER);
                scriptEvents.addEvent(EVENT_KILROGG_HEART_SEEKER, 25000);
                break;
            case EVENT_KILROGG_DEATH_THROES:
                castSpellOnSelf(SPELL_KILROGG_DEATH_THROES);
                scriptEvents.addEvent(EVENT_KILROGG_DEATH_THROES, 40000);
                break;
            case EVENT_KILROGG_VISION_OF_DEATH:
                hfcCastOnRandomPlayer(this, SPELL_KILROGG_VISION_OF_DEATH);
                scriptEvents.addEvent(EVENT_KILROGG_VISION_OF_DEATH, 75000);
                break;
            case EVENT_KILROGG_BLOODTHIRSTERS:
                hfcSummonRing(this, NPC_SALIVATING_BLOODTHIRSTER, 3, 30.0f);
                scriptEvents.addEvent(EVENT_KILROGG_BLOODTHIRSTERS, 45000);
                break;
            case EVENT_KILROGG_HULKING_TERROR:
                hfcSummonRing(this, NPC_HULKING_TERROR, 1, 30.0f);
                scriptEvents.addEvent(EVENT_KILROGG_HULKING_TERROR, 75000);
                break;
            default:
                break;
        }
    }
};

class HellfireCitadelHulkingTerrorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelHulkingTerrorAI(c); }
    explicit HellfireCitadelHulkingTerrorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_TERROR_RENDING_HOWL, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_TERROR_RENDING_HOWL)
        {
            castSpellAOE(SPELL_HULKING_TERROR_RENDING_HOWL);
            scriptEvents.addEvent(EVENT_TERROR_RENDING_HOWL, 18000);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gorefiend

class HellfireCitadelGorefiendAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelGorefiendAI(c); }
    explicit HellfireCitadelGorefiendAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_GOREFIEND, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_HFC_BERSERK, 540000);
        scriptEvents.addEvent(EVENT_GOREFIEND_SHADOW_OF_DEATH, 4000);
        scriptEvents.addEvent(EVENT_GOREFIEND_TOUCH_OF_DOOM, 10000);
        scriptEvents.addEvent(EVENT_GOREFIEND_CRUSHING_DARKNESS, 12000);
        scriptEvents.addEvent(EVENT_GOREFIEND_SHARED_FATE, 20000);
        scriptEvents.addEvent(EVENT_GOREFIEND_FEAST_OF_SOULS, 120000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GOREFIEND, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
        _removeAllAuras();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GOREFIEND, EncounterStates::Performed);
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_GOREFIEND_SHADOW_OF_DEATH:
                hfcCastOnRandomPlayer(this, SPELL_GOREFIEND_SHADOW_OF_DEATH);
                scriptEvents.addEvent(EVENT_GOREFIEND_SHADOW_OF_DEATH, 30000);
                break;
            case EVENT_GOREFIEND_TOUCH_OF_DOOM:
                hfcCastOnRandomPlayer(this, SPELL_GOREFIEND_TOUCH_OF_DOOM);
                scriptEvents.addEvent(EVENT_GOREFIEND_TOUCH_OF_DOOM, 25000);
                break;
            case EVENT_GOREFIEND_CRUSHING_DARKNESS:
                hfcCastOnRandomPlayer(this, SPELL_GOREFIEND_CRUSHING_DARKNESS);
                scriptEvents.addEvent(EVENT_GOREFIEND_CRUSHING_DARKNESS, 16000);
                break;
            case EVENT_GOREFIEND_SHARED_FATE:
                hfcCastOnRandomPlayer(this, SPELL_GOREFIEND_SHARED_FATE);
                scriptEvents.addEvent(EVENT_GOREFIEND_SHARED_FATE, 30000);
                break;
            case EVENT_GOREFIEND_FEAST_OF_SOULS:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Gorefiend begins to feast on the souls of the fallen!");
                castSpellOnSelf(SPELL_GOREFIEND_FEAST_OF_SOULS);
                hfcSummonRing(this, NPC_CORRUPTED_SOUL, 4, 25.0f);
                scriptEvents.addEvent(EVENT_GOREFIEND_FEAST_OF_SOULS, 120000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shadow-Lord Iskar

class HellfireCitadelIskarAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelIskarAI(c); }
    explicit HellfireCitadelIskarAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SHADOW_LORD_ISKAR, EncounterStates::InProgress);

        mServantWaves = 0;
        scriptEvents.addEvent(EVENT_HFC_BERSERK, 600000);
        scriptEvents.addEvent(EVENT_ISKAR_EYE_OF_ANZU, 2000);
        scriptEvents.addEvent(EVENT_ISKAR_PHANTASMAL_WOUNDS, 10000);
        scriptEvents.addEvent(EVENT_ISKAR_FEL_BOMB, 15000);
        scriptEvents.addEvent(EVENT_ISKAR_PHANTASMAL_WINDS, 20000);
        scriptEvents.addEvent(EVENT_ISKAR_FEL_CHAKRAM, 25000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SHADOW_LORD_ISKAR, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
        _removeAllAuras();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SHADOW_LORD_ISKAR, EncounterStates::Performed);
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        // servant waves at 66% and 33%
        if (mServantWaves < 2 && _getHealthPercent() <= (mServantWaves == 0 ? 66 : 33))
        {
            ++mServantWaves;
            castSpellAOE(SPELL_ISKAR_FOCUSED_BLAST);
            hfcSummonRing(this, NPC_CORRUPTED_TALONPRIEST, 1, 20.0f);
            hfcSummonRing(this, NPC_SHADOWFEL_WARDEN, 1, 25.0f);
            hfcSummonRing(this, NPC_ISKAR_FEL_RAVEN, 2, 15.0f);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_ISKAR_EYE_OF_ANZU:
                hfcCastOnRandomPlayer(this, SPELL_ISKAR_EYE_OF_ANZU);
                break;
            case EVENT_ISKAR_PHANTASMAL_WOUNDS:
                hfcCastOnRandomPlayer(this, SPELL_ISKAR_PHANTASMAL_WOUNDS);
                scriptEvents.addEvent(EVENT_ISKAR_PHANTASMAL_WOUNDS, 30000);
                break;
            case EVENT_ISKAR_FEL_BOMB:
                hfcCastOnRandomPlayer(this, SPELL_ISKAR_FEL_BOMB);
                scriptEvents.addEvent(EVENT_ISKAR_FEL_BOMB, 20000);
                break;
            case EVENT_ISKAR_PHANTASMAL_WINDS:
                hfcCastOnRandomPlayer(this, SPELL_ISKAR_PHANTASMAL_WINDS);
                scriptEvents.addEvent(EVENT_ISKAR_PHANTASMAL_WINDS, 40000);
                break;
            case EVENT_ISKAR_FEL_CHAKRAM:
                castSpellOnVictim(SPELL_ISKAR_FEL_CHAKRAM);
                scriptEvents.addEvent(EVENT_ISKAR_FEL_CHAKRAM, 30000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mServantWaves = 0;
};

class HellfireCitadelTalonpriestAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelTalonpriestAI(c); }
    explicit HellfireCitadelTalonpriestAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_TALONPRIEST_RIPOSTE, 5000);
        scriptEvents.addEvent(EVENT_TALONPRIEST_CORRUPTION, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_TALONPRIEST_RIPOSTE:
                castSpellOnVictim(SPELL_TALONPRIEST_SHADOW_RIPOSTE);
                scriptEvents.addEvent(EVENT_TALONPRIEST_RIPOSTE, 14000);
                break;
            case EVENT_TALONPRIEST_CORRUPTION:
                hfcCastOnRandomPlayer(this, SPELL_TALONPRIEST_PHANTASMAL_CORRUPTION);
                scriptEvents.addEvent(EVENT_TALONPRIEST_CORRUPTION, 20000);
                break;
            default:
                break;
        }
    }
};

class HellfireCitadelShadowfelWardenAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelShadowfelWardenAI(c); }
    explicit HellfireCitadelShadowfelWardenAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_WARDEN_FEL_CONDUIT, 4000);
        scriptEvents.addEvent(EVENT_WARDEN_FEL_BOMB, 12000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_WARDEN_FEL_CONDUIT:
                hfcCastOnRandomPlayer(this, SPELL_WARDEN_FEL_CONDUIT);
                scriptEvents.addEvent(EVENT_WARDEN_FEL_CONDUIT, 8000);
                break;
            case EVENT_WARDEN_FEL_BOMB:
                hfcCastOnRandomPlayer(this, SPELL_WARDEN_PHANTASMAL_FEL_BOMB);
                scriptEvents.addEvent(EVENT_WARDEN_FEL_BOMB, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Socrethar the Eternal - the construct goes first, its death releases Socrethar

class HellfireCitadelSoulboundConstructAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelSoulboundConstructAI(c); }
    explicit HellfireCitadelSoulboundConstructAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SOCRETHAR, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_HFC_BERSERK, 600000);
        scriptEvents.addEvent(EVENT_CONSTRUCT_VOLATILE_FEL_ORB, 10000);
        scriptEvents.addEvent(EVENT_CONSTRUCT_REVERBERATING_BLOW, 15000);
        scriptEvents.addEvent(EVENT_CONSTRUCT_FELBLAZE_CHARGE, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SOCRETHAR, EncounterStates::Failed);
        scriptEvents.resetEvents();
        _removeAllAuras();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        // spawned instead of summoned, summons of a dying creature are removed
        if (Creature* socrethar = spawnCreature(BOSS_SOCRETHAR, getCreature()->GetPosition()))
        {
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Socrethar's soul tears free of the construct!");
            setZoneWideCombat(socrethar);
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_CONSTRUCT_VOLATILE_FEL_ORB:
                hfcCastOnRandomPlayer(this, SPELL_CONSTRUCT_VOLATILE_FEL_ORB);
                scriptEvents.addEvent(EVENT_CONSTRUCT_VOLATILE_FEL_ORB, 30000);
                break;
            case EVENT_CONSTRUCT_REVERBERATING_BLOW:
                castSpellOnVictim(SPELL_CONSTRUCT_REVERBERATING_BLOW);
                scriptEvents.addEvent(EVENT_CONSTRUCT_REVERBERATING_BLOW, 17000);
                break;
            case EVENT_CONSTRUCT_FELBLAZE_CHARGE:
                hfcCastOnRandomPlayer(this, SPELL_CONSTRUCT_FELBLAZE_CHARGE, 8.0f);
                scriptEvents.addEvent(EVENT_CONSTRUCT_FELBLAZE_CHARGE, 30000);
                break;
            default:
                break;
        }
    }
};

class HellfireCitadelSocretharAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelSocretharAI(c); }
    explicit HellfireCitadelSocretharAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SOCRETHAR, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_HFC_BERSERK, 420000);
        scriptEvents.addEvent(EVENT_SOCRETHAR_SHADOW_WORD_AGONY, 5000);
        scriptEvents.addEvent(EVENT_SOCRETHAR_DOMINATOR, 10000);
        scriptEvents.addEvent(EVENT_SOCRETHAR_SHADOWCALLERS, 20000);
        scriptEvents.addEvent(EVENT_SOCRETHAR_HAUNTING_SOULS, 30000);
        scriptEvents.addEvent(EVENT_SOCRETHAR_APOCALYPSE, 45000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SOCRETHAR, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();

        // the construct returns for the next attempt, the soul leaves
        for (Creature* construct : getInstanceScript()->getCreatureSetForEntry(BOSS_SOULBOUND_CONSTRUCT))
        {
            if (construct->isAlive())
                continue;

            if (CreatureAIScript* constructAI = construct->GetScript())
                constructAI->despawn(1000, 5000);
        }
        despawn(1000, 0);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SOCRETHAR, EncounterStates::Performed);
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_SOCRETHAR_SHADOW_WORD_AGONY:
                hfcCastOnRandomPlayer(this, SPELL_SOCRETHAR_SHADOW_WORD_AGONY);
                scriptEvents.addEvent(EVENT_SOCRETHAR_SHADOW_WORD_AGONY, 12000);
                break;
            case EVENT_SOCRETHAR_DOMINATOR:
                hfcSummonRing(this, NPC_SARGEREI_DOMINATOR, 1, 20.0f);
                scriptEvents.addEvent(EVENT_SOCRETHAR_DOMINATOR, 60000);
                break;
            case EVENT_SOCRETHAR_SHADOWCALLERS:
                hfcSummonRing(this, NPC_SARGEREI_SHADOWCALLER, 2, 20.0f);
                scriptEvents.addEvent(EVENT_SOCRETHAR_SHADOWCALLERS, 45000);
                break;
            case EVENT_SOCRETHAR_HAUNTING_SOULS:
                hfcSummonRing(this, NPC_HAUNTING_SOUL, 3, 25.0f);
                scriptEvents.addEvent(EVENT_SOCRETHAR_HAUNTING_SOULS, 30000);
                break;
            case EVENT_SOCRETHAR_APOCALYPSE:
                castSpellOnSelf(SPELL_SOCRETHAR_APOCALYPSE);
                scriptEvents.addEvent(EVENT_SOCRETHAR_APOCALYPSE, 60000);
                break;
            default:
                break;
        }
    }
};

class HellfireCitadelSargereiDominatorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelSargereiDominatorAI(c); }
    explicit HellfireCitadelSargereiDominatorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_DOMINATOR_GIFT, 6000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_DOMINATOR_GIFT)
        {
            hfcCastOnRandomPlayer(this, SPELL_DOMINATOR_GIFT_OF_THE_MANARI);
            scriptEvents.addEvent(EVENT_DOMINATOR_GIFT, 25000);
        }
    }
};

class HellfireCitadelHauntingSoulAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelHauntingSoulAI(c); }
    explicit HellfireCitadelHauntingSoulAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        hfcCastOnRandomPlayer(this, SPELL_HAUNTING_SOUL_GHASTLY_FIXATION);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Fel Lord Zakuun

class HellfireCitadelZakuunAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelZakuunAI(c); }
    explicit HellfireCitadelZakuunAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_FEL_LORD_ZAKUUN, EncounterStates::InProgress);

        mPhase = HFC_PHASE_ONE;
        mEnraged = false;
        castSpellOnSelf(SPELL_ZAKUUN_HEAVILY_ARMED, true);
        scriptEvents.addEvent(EVENT_HFC_BERSERK, 600000);
        scriptEvents.addEvent(EVENT_ZAKUUN_SOUL_CLEAVE, 8000, HFC_PHASE_ONE);
        scriptEvents.addEvent(EVENT_ZAKUUN_RUMBLING_FISSURES, 15000, HFC_PHASE_ONE);
        scriptEvents.addEvent(EVENT_ZAKUUN_BEFOULED, 20000, HFC_PHASE_ONE);
        scriptEvents.addEvent(EVENT_ZAKUUN_DROP_AXES, 70000, HFC_PHASE_ONE);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_FEL_LORD_ZAKUUN, EncounterStates::Failed);
        scriptEvents.resetEvents();
        _removeAllAuras();
        mPhase = HFC_PHASE_ONE;
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_FEL_LORD_ZAKUUN, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mPhase);

        if (!mEnraged && _getHealthPercent() <= 30)
        {
            mEnraged = true;
            castSpellOnSelf(SPELL_ZAKUUN_ENRAGE, true);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_ZAKUUN_SOUL_CLEAVE:
                castSpellOnVictim(SPELL_ZAKUUN_SOUL_CLEAVE);
                scriptEvents.addEvent(EVENT_ZAKUUN_SOUL_CLEAVE, 18000, HFC_PHASE_ONE);
                break;
            case EVENT_ZAKUUN_RUMBLING_FISSURES:
                castSpellAOE(SPELL_ZAKUUN_RUMBLING_FISSURES);
                scriptEvents.addEvent(EVENT_ZAKUUN_RUMBLING_FISSURES, 40000, HFC_PHASE_ONE);
                break;
            case EVENT_ZAKUUN_BEFOULED:
                hfcCastOnRandomPlayer(this, SPELL_ZAKUUN_BEFOULED);
                scriptEvents.addEvent(EVENT_ZAKUUN_BEFOULED, 38000, HFC_PHASE_ONE);
                break;
            case EVENT_ZAKUUN_DROP_AXES:
                _removeAura(SPELL_ZAKUUN_HEAVILY_ARMED);
                hfcCastOnRandomPlayer(this, SPELL_ZAKUUN_THROW_AXE);
                castSpellOnSelf(SPELL_ZAKUUN_DISARMED, true);
                mPhase = HFC_PHASE_TWO;
                scriptEvents.addEvent(EVENT_ZAKUUN_SEED_OF_DESTRUCTION, 5000, HFC_PHASE_TWO);
                scriptEvents.addEvent(EVENT_ZAKUUN_CAVITATION, 10000, HFC_PHASE_TWO);
                scriptEvents.addEvent(EVENT_ZAKUUN_WAKE_OF_DESTRUCTION, 15000, HFC_PHASE_TWO);
                scriptEvents.addEvent(EVENT_ZAKUUN_REARM, 40000, HFC_PHASE_TWO);
                break;
            case EVENT_ZAKUUN_SEED_OF_DESTRUCTION:
                hfcCastOnRandomPlayer(this, SPELL_ZAKUUN_SEED_OF_DESTRUCTION);
                scriptEvents.addEvent(EVENT_ZAKUUN_SEED_OF_DESTRUCTION, 20000, HFC_PHASE_TWO);
                break;
            case EVENT_ZAKUUN_CAVITATION:
                castSpellAOE(SPELL_ZAKUUN_CAVITATION);
                scriptEvents.addEvent(EVENT_ZAKUUN_CAVITATION, 30000, HFC_PHASE_TWO);
                break;
            case EVENT_ZAKUUN_WAKE_OF_DESTRUCTION:
                hfcCastOnRandomPlayer(this, SPELL_ZAKUUN_WAKE_OF_DESTRUCTION);
                scriptEvents.addEvent(EVENT_ZAKUUN_WAKE_OF_DESTRUCTION, 20000, HFC_PHASE_TWO);
                break;
            case EVENT_ZAKUUN_REARM:
                _removeAura(SPELL_ZAKUUN_DISARMED);
                castSpellOnSelf(SPELL_ZAKUUN_HEAVILY_ARMED, true);
                mPhase = HFC_PHASE_ONE;
                scriptEvents.addEvent(EVENT_ZAKUUN_DROP_AXES, 70000, HFC_PHASE_ONE);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mPhase = HFC_PHASE_ONE;
    bool mEnraged = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Xhul'horac

class HellfireCitadelXhulhoracAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelXhulhoracAI(c); }
    explicit HellfireCitadelXhulhoracAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_XHULHORAC, EncounterStates::InProgress);

        mPhase = HFC_PHASE_ONE;
        mChaosReleased = false;
        scriptEvents.addEvent(EVENT_HFC_BERSERK, 600000);
        scriptEvents.addEvent(EVENT_XHUL_FEL_STRIKE, 8000);
        scriptEvents.addEvent(EVENT_XHUL_VOID_STRIKE, 16000);
        scriptEvents.addEvent(EVENT_XHUL_AKKELION, 10000);
        scriptEvents.addEvent(EVENT_XHUL_FEL_SURGE, 15000);
        scriptEvents.addEvent(EVENT_XHUL_VOID_SURGE, 25000);
        scriptEvents.addEvent(EVENT_XHUL_IMPS, 20000);
        scriptEvents.addEvent(EVENT_XHUL_VOIDFIENDS, 30000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_XHULHORAC, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
        _removeAllAuras();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_XHULHORAC, EncounterStates::Performed);
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        // fel until 70%, void until 40%, both below
        if (mPhase == HFC_PHASE_ONE && _getHealthPercent() <= 70)
        {
            mPhase = HFC_PHASE_TWO;
            hfcSummonRing(this, NPC_OMNUS, 1, 20.0f);
        }
        else if (mPhase == HFC_PHASE_TWO && _getHealthPercent() <= 40)
        {
            mPhase = HFC_PHASE_THREE;
        }

        if (!mChaosReleased && _getHealthPercent() <= 10)
        {
            mChaosReleased = true;
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Xhul'horac unleashes overwhelming chaos!");
            castSpellOnSelf(SPELL_XHUL_OVERWHELMING_CHAOS, true);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_XHUL_FEL_STRIKE:
                if (mPhase != HFC_PHASE_TWO)
                    castSpellOnVictim(SPELL_XHUL_FEL_STRIKE);
                scriptEvents.addEvent(EVENT_XHUL_FEL_STRIKE, 16000);
                break;
            case EVENT_XHUL_VOID_STRIKE:
                if (mPhase != HFC_PHASE_ONE)
                    castSpellOnVictim(SPELL_XHUL_VOID_STRIKE);
                scriptEvents.addEvent(EVENT_XHUL_VOID_STRIKE, 16000);
                break;
            case EVENT_XHUL_FEL_SURGE:
                if (mPhase != HFC_PHASE_TWO)
                    hfcCastOnRandomPlayer(this, SPELL_XHUL_FEL_SURGE);
                scriptEvents.addEvent(EVENT_XHUL_FEL_SURGE, 30000);
                break;
            case EVENT_XHUL_VOID_SURGE:
                if (mPhase != HFC_PHASE_ONE)
                    hfcCastOnRandomPlayer(this, SPELL_XHUL_VOID_SURGE);
                scriptEvents.addEvent(EVENT_XHUL_VOID_SURGE, 30000);
                break;
            case EVENT_XHUL_AKKELION:
                hfcSummonRing(this, NPC_VANGUARD_AKKELION, 1, 20.0f);
                break;
            case EVENT_XHUL_IMPS:
                if (mPhase != HFC_PHASE_TWO)
                    hfcSummonRing(this, NPC_WILD_PYROMANIAC, 3, 30.0f);
                scriptEvents.addEvent(EVENT_XHUL_IMPS, 35000);
                break;
            case EVENT_XHUL_VOIDFIENDS:
                if (mPhase != HFC_PHASE_ONE)
                    hfcSummonRing(this, NPC_UNSTABLE_VOIDFIEND, 2, 30.0f);
                scriptEvents.addEvent(EVENT_XHUL_VOIDFIENDS, 35000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mPhase = HFC_PHASE_ONE;
    bool mChaosReleased = false;
};

class HellfireCitadelAkkelionAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelAkkelionAI(c); }
    explicit HellfireCitadelAkkelionAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_AKKELION_FELBLAZE_FLURRY, 6000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_AKKELION_FELBLAZE_FLURRY)
        {
            castSpellOnSelf(SPELL_AKKELION_FELBLAZE_FLURRY);
            scriptEvents.addEvent(EVENT_AKKELION_FELBLAZE_FLURRY, 15000);
        }
    }
};

class HellfireCitadelOmnusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelOmnusAI(c); }
    explicit HellfireCitadelOmnusAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_OMNUS_WITHERING_GAZE, 6000);
        scriptEvents.addEvent(EVENT_OMNUS_BLACK_HOLE, 15000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_OMNUS_WITHERING_GAZE:
                castSpellOnVictim(SPELL_OMNUS_WITHERING_GAZE);
                scriptEvents.addEvent(EVENT_OMNUS_WITHERING_GAZE, 15000);
                break;
            case EVENT_OMNUS_BLACK_HOLE:
                hfcCastOnRandomPlayer(this, SPELL_OMNUS_BLACK_HOLE);
                scriptEvents.addEvent(EVENT_OMNUS_BLACK_HOLE, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Tyrant Velhari

class HellfireCitadelVelhariAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelVelhariAI(c); }
    explicit HellfireCitadelVelhariAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_TYRANT_VELHARI, EncounterStates::InProgress);

        mPhase = HFC_PHASE_ONE;
        castSpellOnSelf(SPELL_VELHARI_AURA_OF_OPPRESSION, true);
        scriptEvents.addEvent(EVENT_HFC_BERSERK, 600000);
        scriptEvents.addEvent(EVENT_VELHARI_ENFORCER, 3000);
        scriptEvents.addEvent(EVENT_VELHARI_TAINTED_SHADOWS, 5000);
        scriptEvents.addEvent(EVENT_VELHARI_ANNIHILATING_STRIKE, 10000);
        scriptEvents.addEvent(EVENT_VELHARI_TOUCH_OF_HARM, 15000);
        scriptEvents.addEvent(EVENT_VELHARI_INFERNAL_TEMPEST, 40000);
        scriptEvents.addEvent(EVENT_VELHARI_EDICT, 55000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_TYRANT_VELHARI, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
        _removeAllAuras();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_TYRANT_VELHARI, EncounterStates::Performed);
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        // each phase swaps the aura and calls the next ancient
        if (mPhase == HFC_PHASE_ONE && _getHealthPercent() <= 70)
        {
            mPhase = HFC_PHASE_TWO;
            _removeAura(SPELL_VELHARI_AURA_OF_OPPRESSION);
            castSpellOnSelf(SPELL_VELHARI_AURA_OF_CONTEMPT, true);
            hfcSummonRing(this, NPC_ANCIENT_HARBINGER, 1, 15.0f);
            scriptEvents.addEvent(EVENT_VELHARI_SEAL_OF_DECAY, 6000);
            scriptEvents.addEvent(EVENT_VELHARI_FONT_OF_CORRUPTION, 12000);
        }
        else if (mPhase == HFC_PHASE_TWO && _getHealthPercent() <= 40)
        {
            mPhase = HFC_PHASE_THREE;
            _removeAura(SPELL_VELHARI_AURA_OF_CONTEMPT);
            castSpellOnSelf(SPELL_VELHARI_AURA_OF_MALICE, true);
            hfcSummonRing(this, NPC_ANCIENT_SOVEREIGN, 1, 15.0f);
            scriptEvents.addEvent(EVENT_VELHARI_GAVEL, 15000);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_VELHARI_ENFORCER:
                hfcSummonRing(this, NPC_ANCIENT_ENFORCER, 1, 15.0f);
                break;
            case EVENT_VELHARI_TAINTED_SHADOWS:
                castSpellOnVictim(SPELL_VELHARI_TAINTED_SHADOWS);
                scriptEvents.addEvent(EVENT_VELHARI_TAINTED_SHADOWS, 8000);
                break;
            case EVENT_VELHARI_ANNIHILATING_STRIKE:
                hfcCastOnRandomPlayer(this, SPELL_VELHARI_ANNIHILATING_STRIKE);
                scriptEvents.addEvent(EVENT_VELHARI_ANNIHILATING_STRIKE, 22000);
                break;
            case EVENT_VELHARI_TOUCH_OF_HARM:
                hfcCastOnRandomPlayer(this, SPELL_VELHARI_TOUCH_OF_HARM);
                scriptEvents.addEvent(EVENT_VELHARI_TOUCH_OF_HARM, 30000);
                break;
            case EVENT_VELHARI_INFERNAL_TEMPEST:
                castSpellOnSelf(SPELL_VELHARI_INFERNAL_TEMPEST);
                scriptEvents.addEvent(EVENT_VELHARI_INFERNAL_TEMPEST, 45000);
                break;
            case EVENT_VELHARI_EDICT:
                hfcCastOnRandomPlayer(this, SPELL_VELHARI_EDICT_OF_CONDEMNATION);
                scriptEvents.addEvent(EVENT_VELHARI_EDICT, 60000);
                break;
            case EVENT_VELHARI_SEAL_OF_DECAY:
                castSpellOnVictim(SPELL_VELHARI_SEAL_OF_DECAY);
                scriptEvents.addEvent(EVENT_VELHARI_SEAL_OF_DECAY, 12000);
                break;
            case EVENT_VELHARI_FONT_OF_CORRUPTION:
                hfcCastOnRandomPlayer(this, SPELL_VELHARI_FONT_OF_CORRUPTION);
                scriptEvents.addEvent(EVENT_VELHARI_FONT_OF_CORRUPTION, 35000);
                break;
            case EVENT_VELHARI_GAVEL:
                hfcCastOnRandomPlayer(this, SPELL_VELHARI_GAVEL_OF_THE_TYRANT);
                scriptEvents.addEvent(EVENT_VELHARI_GAVEL, 40000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mPhase = HFC_PHASE_ONE;
};

class HellfireCitadelAncientEnforcerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelAncientEnforcerAI(c); }
    explicit HellfireCitadelAncientEnforcerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_ENFORCER_ONSLAUGHT, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_ENFORCER_ONSLAUGHT)
        {
            castSpellOnVictim(SPELL_ENFORCER_ONSLAUGHT);
            scriptEvents.addEvent(EVENT_ENFORCER_ONSLAUGHT, 18000);
        }
    }
};

class HellfireCitadelAncientHarbingerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelAncientHarbingerAI(c); }
    explicit HellfireCitadelAncientHarbingerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_HARBINGER_MENDING, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_HARBINGER_MENDING)
        {
            Creature* velhari = getNearestCreature(BOSS_TYRANT_VELHARI);
            if (velhari != nullptr && velhari->isAlive())
                castSpell(velhari, SPELL_HARBINGER_MENDING);
            scriptEvents.addEvent(EVENT_HARBINGER_MENDING, 20000);
        }
    }
};

class HellfireCitadelAncientSovereignAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelAncientSovereignAI(c); }
    explicit HellfireCitadelAncientSovereignAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SOVEREIGN_WARD, 5000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_SOVEREIGN_WARD)
        {
            Creature* velhari = getNearestCreature(BOSS_TYRANT_VELHARI);
            if (velhari != nullptr && velhari->isAlive())
                castSpell(velhari, SPELL_SOVEREIGN_WARD);
            scriptEvents.addEvent(EVENT_SOVEREIGN_WARD, 30000);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mannoroth - bound until the three Fel Iron Summoners fall

class HellfireCitadelMannorothAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelMannorothAI(c); }
    explicit HellfireCitadelMannorothAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_MANNOROTH, EncounterStates::InProgress);

        mPhase = HFC_PHASE_ONE;
        mSummonersLeft = 3;
        setRooted(true);
        _setMeleeDisabled(true);
        hfcSummonRing(this, NPC_FEL_IRON_SUMMONER, mSummonersLeft, 25.0f);
        scriptEvents.addEvent(EVENT_HFC_BERSERK, 720000);
        scriptEvents.addEvent(EVENT_MANNOROTH_FEL_IMPS, 10000, HFC_PHASE_ONE);
        scriptEvents.addEvent(EVENT_MANNOROTH_DOOM_LORD, 30000, HFC_PHASE_ONE);
        scriptEvents.addEvent(EVENT_MANNOROTH_DREAD_INFERNAL, 40000, HFC_PHASE_ONE);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MANNOROTH, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
        _removeAllAuras();
        setRooted(false);
        _setMeleeDisabled(false);
        mPhase = HFC_PHASE_ONE;
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MANNOROTH, EncounterStates::Performed);
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    void OnSummonDies(Creature* summon, Unit* /*pKiller*/) override
    {
        if (mPhase != HFC_PHASE_ONE || summon->getEntry() != NPC_FEL_IRON_SUMMONER)
            return;

        if (mSummonersLeft > 0)
            --mSummonersLeft;

        if (mSummonersLeft == 0)
            breakFree();
    }

    // immune while bound
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (mPhase == HFC_PHASE_ONE)
            *damage = 0;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mPhase);

        // empowered abilities from 65% and 30%
        if (mPhase == HFC_PHASE_TWO && _getHealthPercent() <= 65)
        {
            mPhase = HFC_PHASE_THREE;
            scriptEvents.addEvent(EVENT_MANNOROTH_GAZE, 15000);
        }
        else if (mPhase == HFC_PHASE_THREE && _getHealthPercent() <= 30)
        {
            mPhase = HFC_PHASE_FOUR;
            scriptEvents.addEvent(EVENT_MANNOROTH_FELSEEKER, 20000);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_MANNOROTH_FEL_IMPS:
                hfcSummonRing(this, NPC_MANNOROTH_FEL_IMP, 3, 30.0f);
                scriptEvents.addEvent(EVENT_MANNOROTH_FEL_IMPS, 25000, HFC_PHASE_ONE);
                break;
            case EVENT_MANNOROTH_DOOM_LORD:
                hfcSummonRing(this, NPC_MANNOROTH_DOOM_LORD, 1, 25.0f);
                scriptEvents.addEvent(EVENT_MANNOROTH_DOOM_LORD, 45000, HFC_PHASE_ONE);
                break;
            case EVENT_MANNOROTH_DREAD_INFERNAL:
                hfcSummonRing(this, NPC_MANNOROTH_DREAD_INFERNAL, 1, 25.0f);
                scriptEvents.addEvent(EVENT_MANNOROTH_DREAD_INFERNAL, 50000, HFC_PHASE_ONE);
                break;
            case EVENT_MANNOROTH_GLAIVE_COMBO:
                castSpellOnVictim(SPELL_MANNOROTH_GLAIVE_COMBO);
                scriptEvents.addEvent(EVENT_MANNOROTH_GLAIVE_COMBO, 30000);
                break;
            case EVENT_MANNOROTH_MASSIVE_BLAST:
                castSpellOnVictim(SPELL_MANNOROTH_MASSIVE_BLAST);
                scriptEvents.addEvent(EVENT_MANNOROTH_MASSIVE_BLAST, 60000);
                break;
            case EVENT_MANNOROTH_FEL_HELLSTORM:
                castSpellAOE(mPhase >= HFC_PHASE_THREE ? SPELL_MANNOROTH_EMPOWERED_FEL_HELLSTORM : SPELL_MANNOROTH_FEL_HELLSTORM);
                scriptEvents.addEvent(EVENT_MANNOROTH_FEL_HELLSTORM, 35000);
                break;
            case EVENT_MANNOROTH_SHADOWFORCE:
                castSpellOnSelf(mPhase >= HFC_PHASE_FOUR ? SPELL_MANNOROTH_EMPOWERED_SHADOWFORCE : SPELL_MANNOROTH_SHADOWFORCE);
                scriptEvents.addEvent(EVENT_MANNOROTH_SHADOWFORCE, 50000);
                break;
            case EVENT_MANNOROTH_CURSE_OF_THE_LEGION:
                hfcCastOnRandomPlayer(this, SPELL_MANNOROTH_CURSE_OF_THE_LEGION);
                scriptEvents.addEvent(EVENT_MANNOROTH_CURSE_EXPIRES, 20000);
                scriptEvents.addEvent(EVENT_MANNOROTH_CURSE_OF_THE_LEGION, 65000);
                break;
            case EVENT_MANNOROTH_CURSE_EXPIRES:
                hfcSummonRing(this, NPC_MANNOROTH_DOOM_LORD, 1, 20.0f);
                break;
            case EVENT_MANNOROTH_GAZE:
                hfcCastOnRandomPlayer(this, mPhase >= HFC_PHASE_FOUR ? SPELL_MANNOROTH_EMPOWERED_GAZE : SPELL_MANNOROTH_GAZE);
                scriptEvents.addEvent(EVENT_MANNOROTH_GAZE, 47000);
                break;
            case EVENT_MANNOROTH_FELSEEKER:
                castSpellAOE(SPELL_MANNOROTH_FELSEEKER);
                scriptEvents.addEvent(EVENT_MANNOROTH_FELSEEKER, 40000);
                break;
            default:
                break;
        }
    }

private:
    void breakFree()
    {
        mPhase = HFC_PHASE_TWO;
        setRooted(false);
        _setMeleeDisabled(false);
        sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Mannoroth breaks free of his bindings!");
        scriptEvents.addEvent(EVENT_MANNOROTH_GLAIVE_COMBO, 15000);
        scriptEvents.addEvent(EVENT_MANNOROTH_FEL_HELLSTORM, 20000);
        scriptEvents.addEvent(EVENT_MANNOROTH_CURSE_OF_THE_LEGION, 25000);
        scriptEvents.addEvent(EVENT_MANNOROTH_MASSIVE_BLAST, 30000);
        scriptEvents.addEvent(EVENT_MANNOROTH_SHADOWFORCE, 40000);
    }

    uint32_t mPhase = HFC_PHASE_ONE;
    uint32_t mSummonersLeft = 3;
};

class HellfireCitadelDoomLordAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelDoomLordAI(c); }
    explicit HellfireCitadelDoomLordAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_DOOM_LORD_MARK_OF_DOOM, 5000);
        scriptEvents.addEvent(EVENT_DOOM_LORD_VOLLEY, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DOOM_LORD_MARK_OF_DOOM:
                hfcCastOnRandomPlayer(this, SPELL_DOOM_LORD_MARK_OF_DOOM);
                scriptEvents.addEvent(EVENT_DOOM_LORD_MARK_OF_DOOM, 25000);
                break;
            case EVENT_DOOM_LORD_VOLLEY:
                castSpellAOE(SPELL_DOOM_LORD_SHADOW_BOLT_VOLLEY);
                scriptEvents.addEvent(EVENT_DOOM_LORD_VOLLEY, 12000);
                break;
            default:
                break;
        }
    }
};

class HellfireCitadelFelImpAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelFelImpAI(c); }
    explicit HellfireCitadelFelImpAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_FEL_IMP_FEL_BLAST, 3000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_FEL_IMP_FEL_BLAST)
        {
            castSpellOnVictim(SPELL_FEL_IMP_FEL_BLAST);
            scriptEvents.addEvent(EVENT_FEL_IMP_FEL_BLAST, 6000);
        }
    }
};

class HellfireCitadelDreadInfernalAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelDreadInfernalAI(c); }
    explicit HellfireCitadelDreadInfernalAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        castSpellOnSelf(SPELL_DREAD_INFERNAL_FEL_HELLFIRE, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Archimonde - three phases, the Twisting Nether part is left out

class HellfireCitadelArchimondeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelArchimondeAI(c); }
    explicit HellfireCitadelArchimondeAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ARCHIMONDE, EncounterStates::InProgress);

        mPhase = HFC_PHASE_ONE;
        scriptEvents.addEvent(EVENT_HFC_BERSERK, 900000);
        scriptEvents.addEvent(EVENT_ARCHIMONDE_DESECRATE, 7000);
        scriptEvents.addEvent(EVENT_ARCHIMONDE_DEATH_BRAND, 35000);
        scriptEvents.addEvent(EVENT_ARCHIMONDE_DOOMFIRE, 10000, HFC_PHASE_ONE);
        scriptEvents.addEvent(EVENT_ARCHIMONDE_DEATHCALLERS, 20000, HFC_PHASE_ONE);
        scriptEvents.addEvent(EVENT_ARCHIMONDE_ALLURE_OF_FLAMES, 30000, HFC_PHASE_ONE);
        scriptEvents.addEvent(EVENT_ARCHIMONDE_SHADOWFEL_BURST, 40000, HFC_PHASE_ONE);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ARCHIMONDE, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
        _removeAllAuras();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ARCHIMONDE, EncounterStates::Performed);
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mPhase);

        if (mPhase == HFC_PHASE_ONE && _getHealthPercent() <= 70)
        {
            mPhase = HFC_PHASE_TWO;
            scriptEvents.addEvent(EVENT_ARCHIMONDE_SHACKLED_TORMENT, 10000, HFC_PHASE_TWO);
            scriptEvents.addEvent(EVENT_ARCHIMONDE_OVERFIEND, 15000, HFC_PHASE_TWO);
            scriptEvents.addEvent(EVENT_ARCHIMONDE_WROUGHT_CHAOS, 25000);
        }
        else if (mPhase == HFC_PHASE_TWO && _getHealthPercent() <= 40)
        {
            mPhase = HFC_PHASE_THREE;
            scriptEvents.addEvent(EVENT_ARCHIMONDE_RAIN_OF_CHAOS, 10000, HFC_PHASE_THREE);
            scriptEvents.addEvent(EVENT_ARCHIMONDE_DOOMBRINGERS, 20000, HFC_PHASE_THREE);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HFC_BERSERK:
                castSpellOnSelf(SPELL_HFC_BERSERK, true);
                break;
            case EVENT_ARCHIMONDE_DESECRATE:
                castSpellAOE(SPELL_ARCHIMONDE_DESECRATE);
                scriptEvents.addEvent(EVENT_ARCHIMONDE_DESECRATE, 27000);
                break;
            case EVENT_ARCHIMONDE_DEATH_BRAND:
                castSpellOnVictim(SPELL_ARCHIMONDE_DEATH_BRAND);
                scriptEvents.addEvent(EVENT_ARCHIMONDE_DEATH_BRAND, 42000);
                break;
            case EVENT_ARCHIMONDE_DOOMFIRE:
                hfcCastOnRandomPlayer(this, SPELL_ARCHIMONDE_DOOMFIRE);
                scriptEvents.addEvent(EVENT_ARCHIMONDE_DOOMFIRE, 40000, HFC_PHASE_ONE);
                break;
            case EVENT_ARCHIMONDE_DEATHCALLERS:
                hfcSummonRing(this, NPC_HELLFIRE_DEATHCALLER, 2, 30.0f);
                scriptEvents.addEvent(EVENT_ARCHIMONDE_DEATHCALLERS, 70000, HFC_PHASE_ONE);
                break;
            case EVENT_ARCHIMONDE_ALLURE_OF_FLAMES:
                castSpellAOE(SPELL_ARCHIMONDE_ALLURE_OF_FLAMES);
                scriptEvents.addEvent(EVENT_ARCHIMONDE_ALLURE_OF_FLAMES, 47000, HFC_PHASE_ONE);
                break;
            case EVENT_ARCHIMONDE_SHADOWFEL_BURST:
                hfcCastOnRandomPlayer(this, SPELL_ARCHIMONDE_SHADOWFEL_BURST);
                scriptEvents.addEvent(EVENT_ARCHIMONDE_SHADOWFEL_BURST, 55000, HFC_PHASE_ONE);
                break;
            case EVENT_ARCHIMONDE_SHACKLED_TORMENT:
                hfcCastOnRandomPlayer(this, SPELL_ARCHIMONDE_SHACKLED_TORMENT);
                scriptEvents.addEvent(EVENT_ARCHIMONDE_SHACKLED_TORMENT, 30000, HFC_PHASE_TWO);
                break;
            case EVENT_ARCHIMONDE_OVERFIEND:
                hfcSummonRing(this, NPC_FELBORNE_OVERFIEND, 1, 30.0f);
                scriptEvents.addEvent(EVENT_ARCHIMONDE_OVERFIEND, 45000, HFC_PHASE_TWO);
                break;
            case EVENT_ARCHIMONDE_WROUGHT_CHAOS:
                hfcCastOnRandomPlayer(this, SPELL_ARCHIMONDE_WROUGHT_CHAOS);
                scriptEvents.addEvent(EVENT_ARCHIMONDE_WROUGHT_CHAOS, 45000);
                break;
            case EVENT_ARCHIMONDE_RAIN_OF_CHAOS:
                castSpellAOE(SPELL_ARCHIMONDE_RAIN_OF_CHAOS);
                scriptEvents.addEvent(EVENT_ARCHIMONDE_RAIN_OF_CHAOS, 60000, HFC_PHASE_THREE);
                break;
            case EVENT_ARCHIMONDE_DOOMBRINGERS:
                hfcSummonRing(this, NPC_INFERNAL_DOOMBRINGER, 2, 30.0f);
                scriptEvents.addEvent(EVENT_ARCHIMONDE_DOOMBRINGERS, 60000, HFC_PHASE_THREE);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mPhase = HFC_PHASE_ONE;
};

class HellfireCitadelDeathcallerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireCitadelDeathcallerAI(c); }
    explicit HellfireCitadelDeathcallerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_DEATHCALLER_SHADOW_BLAST, 3000);
        scriptEvents.addEvent(EVENT_DEATHCALLER_DEMONIC_HAVOC, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DEATHCALLER_SHADOW_BLAST:
                castSpellOnVictim(SPELL_DEATHCALLER_SHADOW_BLAST);
                scriptEvents.addEvent(EVENT_DEATHCALLER_SHADOW_BLAST, 6000);
                break;
            case EVENT_DEATHCALLER_DEMONIC_HAVOC:
                hfcCastOnRandomPlayer(this, SPELL_DEATHCALLER_DEMONIC_HAVOC);
                scriptEvents.addEvent(EVENT_DEATHCALLER_DEMONIC_HAVOC, 25000);
                break;
            default:
                break;
        }
    }
};

void SetupHellfireCitadel(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HELLFIRE_CITADEL, &HellfireCitadelInstanceScript::Create);

    mgr->register_creature_script(BOSS_SIEGEMASTER_MARTAK, &HellfireCitadelMartakAI::Create);
    mgr->register_creature_script(BOSS_IRON_REAVER, &HellfireCitadelIronReaverAI::Create);
    mgr->register_creature_script(BOSS_KORMROK, &HellfireCitadelKormrokAI::Create);
    mgr->register_creature_script(BOSS_DIA_DARKWHISPER, &HellfireCitadelDiaDarkwhisperAI::Create);
    mgr->register_creature_script(BOSS_GURTOGG_BLOODBOIL, &HellfireCitadelGurtoggAI::Create);
    mgr->register_creature_script(BOSS_BLADEMASTER_JUBEITHOS, &HellfireCitadelJubeithosAI::Create);
    mgr->register_creature_script(BOSS_KILROGG_DEADEYE, &HellfireCitadelKilroggAI::Create);
    mgr->register_creature_script(BOSS_GOREFIEND, &HellfireCitadelGorefiendAI::Create);
    mgr->register_creature_script(BOSS_SHADOW_LORD_ISKAR, &HellfireCitadelIskarAI::Create);
    mgr->register_creature_script(BOSS_SOULBOUND_CONSTRUCT, &HellfireCitadelSoulboundConstructAI::Create);
    mgr->register_creature_script(BOSS_SOCRETHAR, &HellfireCitadelSocretharAI::Create);
    mgr->register_creature_script(BOSS_FEL_LORD_ZAKUUN, &HellfireCitadelZakuunAI::Create);
    mgr->register_creature_script(BOSS_XHULHORAC, &HellfireCitadelXhulhoracAI::Create);
    mgr->register_creature_script(BOSS_TYRANT_VELHARI, &HellfireCitadelVelhariAI::Create);
    mgr->register_creature_script(BOSS_MANNOROTH, &HellfireCitadelMannorothAI::Create);
    mgr->register_creature_script(BOSS_ARCHIMONDE, &HellfireCitadelArchimondeAI::Create);

    mgr->register_creature_script(NPC_GOREBOUND_FELCASTER, &HellfireCitadelFelcasterAI::Create);
    mgr->register_creature_script(NPC_HULKING_BERSERKER, &HellfireCitadelBerserkerAI::Create);
    mgr->register_creature_script(NPC_IRON_DRAGOON, &HellfireCitadelDragoonAI::Create);
    mgr->register_creature_script(NPC_HULKING_TERROR, &HellfireCitadelHulkingTerrorAI::Create);
    mgr->register_creature_script(NPC_CORRUPTED_TALONPRIEST, &HellfireCitadelTalonpriestAI::Create);
    mgr->register_creature_script(NPC_SHADOWFEL_WARDEN, &HellfireCitadelShadowfelWardenAI::Create);
    mgr->register_creature_script(NPC_SARGEREI_DOMINATOR, &HellfireCitadelSargereiDominatorAI::Create);
    mgr->register_creature_script(NPC_HAUNTING_SOUL, &HellfireCitadelHauntingSoulAI::Create);
    mgr->register_creature_script(NPC_VANGUARD_AKKELION, &HellfireCitadelAkkelionAI::Create);
    mgr->register_creature_script(NPC_OMNUS, &HellfireCitadelOmnusAI::Create);
    mgr->register_creature_script(NPC_ANCIENT_ENFORCER, &HellfireCitadelAncientEnforcerAI::Create);
    mgr->register_creature_script(NPC_ANCIENT_HARBINGER, &HellfireCitadelAncientHarbingerAI::Create);
    mgr->register_creature_script(NPC_ANCIENT_SOVEREIGN, &HellfireCitadelAncientSovereignAI::Create);
    mgr->register_creature_script(NPC_MANNOROTH_DOOM_LORD, &HellfireCitadelDoomLordAI::Create);
    mgr->register_creature_script(NPC_MANNOROTH_FEL_IMP, &HellfireCitadelFelImpAI::Create);
    mgr->register_creature_script(NPC_MANNOROTH_DREAD_INFERNAL, &HellfireCitadelDreadInfernalAI::Create);
    mgr->register_creature_script(NPC_HELLFIRE_DEATHCALLER, &HellfireCitadelDeathcallerAI::Create);
}
