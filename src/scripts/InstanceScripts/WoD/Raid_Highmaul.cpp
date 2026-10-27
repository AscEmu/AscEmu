/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Highmaul.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <vector>

enum HighmaulEvents
{
    // Kargath Bladefist
    EVENT_KARGATH_BLADE_DANCE = 1,
    EVENT_KARGATH_IMPALE,
    EVENT_KARGATH_BERSERKER_RUSH,
    EVENT_KARGATH_FIRST_BOMBERS,
    EVENT_KARGATH_CHAIN_HURL,
    EVENT_KARGATH_STANDS_RISE,
    EVENT_KARGATH_BERSERK,

    EVENT_IRON_BOMB,
    EVENT_MAULING_BREW,
    EVENT_VILE_BREATH,

    // The Butcher
    EVENT_BUTCHER_MEAT_HOOK,
    EVENT_BUTCHER_TENDERIZER,
    EVENT_BUTCHER_CLEAVE,
    EVENT_BUTCHER_THE_CLEAVER,
    EVENT_BUTCHER_BOUNDING_CLEAVE,
    EVENT_BUTCHER_BERSERK,

    // Tectus, shards and motes
    EVENT_MOUNTAIN_CRYSTALLINE_BARRAGE,
    EVENT_MOUNTAIN_FRACTURE,
    EVENT_MOUNTAIN_ACCRETION,
    EVENT_MOUNTAIN_EARTHEN_PILLAR,
    EVENT_MOUNTAIN_TECTONIC_UPHEAVAL,
    EVENT_TECTUS_EARTHWARPER,
    EVENT_TECTUS_BERSERKER,
    EVENT_TECTUS_BERSERK,

    EVENT_RAVING_ASSAULT,
    EVENT_EARTHEN_FLECHETTES,
    EVENT_GIFT_OF_EARTH,

    // Brackenspore
    EVENT_BRACKENSPORE_ROT,
    EVENT_BRACKENSPORE_MIND_FUNGUS,
    EVENT_BRACKENSPORE_LIVING_MUSHROOM,
    EVENT_BRACKENSPORE_SPORE_SHOOTERS,
    EVENT_BRACKENSPORE_NECROTIC_BREATH,
    EVENT_BRACKENSPORE_FLESH_EATER,
    EVENT_BRACKENSPORE_INFESTING_SPORES,
    EVENT_BRACKENSPORE_REJUVENATING_MUSHROOM,
    EVENT_BRACKENSPORE_BERSERK,

    EVENT_FLESH_EATER_FEAST,
    EVENT_FLESH_EATER_DECAY,
    EVENT_SPORE_SHOT,

    // Twin Ogron
    EVENT_OGRON_BERSERK,
    EVENT_PHEMOS_DOUBLE_SLASH,
    EVENT_PHEMOS_DOUBLE_SLASH_OFF_HAND,
    EVENT_PHEMOS_WHIRLWIND,
    EVENT_PHEMOS_ENFEEBLING_ROAR,
    EVENT_PHEMOS_QUAKE,
    EVENT_POL_SHIELD_BASH,
    EVENT_POL_SHIELD_CHARGE,
    EVENT_POL_INTERRUPTING_SHOUT,
    EVENT_POL_PULVERIZE,
    EVENT_POL_PULVERIZE_FIRST,
    EVENT_POL_PULVERIZE_SECOND,
    EVENT_POL_PULVERIZE_THIRD,
    EVENT_POL_PULVERIZE_DONE,

    // Ko'ragh
    EVENT_KORAGH_EXPEL_FIRE,
    EVENT_KORAGH_SUPPRESSION_FIELD,
    EVENT_KORAGH_EXPEL_ARCANE,
    EVENT_KORAGH_OVERFLOWING_ENERGY,
    EVENT_KORAGH_EXPEL_FROST,
    EVENT_KORAGH_EXPEL_SHADOW,
    EVENT_KORAGH_BREAKERS_STRENGTH,
    EVENT_KORAGH_RECHARGE_START,
    EVENT_KORAGH_RECHARGE_DONE,

    // Imperator Mar'gok
    EVENT_MARGOK_ARCANE_WRATH,
    EVENT_MARGOK_DESTRUCTIVE_RESONANCE,
    EVENT_MARGOK_ARCANE_ABERRATION,
    EVENT_MARGOK_MARK_OF_CHAOS,
    EVENT_MARGOK_FORCE_NOVA,
    EVENT_MARGOK_BERSERK,
    EVENT_MARGOK_CALL_GUARDS,
    EVENT_MARGOK_ANOMALY_WAVE,
    EVENT_MARGOK_RUNE_AWAKENED,

    EVENT_WARMAGE_FIXATE,
    EVENT_WARMAGE_NETHER_BLAST,
    EVENT_WARMAGE_SLOW,
    EVENT_REAVER_SHOCKWAVE,
    EVENT_REAVER_CRUSH_ARMOR,
    EVENT_REAVER_KICK
};

// scriptEvents phases: 0 always ticks
enum HighmaulEventPhases : uint32_t
{
    HIGHMAUL_PHASE_ROTATION = 1,
    HIGHMAUL_PHASE_BREAK    = 2
};

enum MargokRunes : uint8_t
{
    MARGOK_RUNE_CROWN = 0,
    MARGOK_RUNE_DISPLACEMENT,
    MARGOK_RUNE_FORTIFICATION,
    MARGOK_RUNE_REPLICATION,
    MARGOK_RUNE_COUNT
};

namespace
{
    float const HighmaulTwoPi = 6.28318531f;

    // Kargath: arena floor centre and stand slots
    LocationVector const KargathArenaCentre(3467.0f, 7579.2f, 55.3f);
    LocationVector const KargathStandSlots[] =
    {
        { 3465.95f, 7520.59f, 65.15f },
        { 3503.33f, 7548.39f, 65.44f },
        { 3521.71f, 7590.66f, 65.09f },
        { 3514.75f, 7623.96f, 65.44f },
        { 3483.57f, 7637.60f, 65.44f },
        { 3444.61f, 7621.95f, 65.42f },
        { 3418.62f, 7590.24f, 65.45f }
    };
    uint32_t const KargathStandSlotCount = 7;
    uint32_t const KargathBombersPerWave = 5;
    LocationVector const KargathBileslingerSpots[] =
    {
        { 3499.293f, 7642.336f, 67.58533f, 4.473204f },
        { 3512.144f, 7574.136f, 63.50087f, 2.765742f }
    };

    // Tectus: split counts
    uint32_t const TectusShardCount = 2;
    uint32_t const TectusMotesPerShard = 4;
    uint32_t const TectusUpheavalWindowMs = 10000;

    // Brackenspore: Flesh-Eaters climb out of the water here
    LocationVector const BrackensporeFleshEaterSpots[] =
    {
        { 4178.046f, 7791.621f, -0.3537667f, 3.052145f },
        { 4141.948f, 7720.839f, -1.169765f, 2.001688f }
    };
    uint32_t const BrackensporeSporeShooters = 2;

    // Ko'ragh: recharge spot
    LocationVector const KoraghRuneCentre(3903.39f, 8608.15f, 364.71f, 5.589f);

    // Mar'gok: intermission runes and reaver landing
    LocationVector const MargokFortificationRune(3885.65f, 8557.80f, 565.34f, 0.747137f);
    LocationVector const MargokReplicationRune(3890.47f, 8628.17f, 565.34f, 5.375480f);
    LocationVector const MargokReaverSpot(4026.755f, 8584.76f, 572.6546f, 3.138298f);
    uint32_t const MargokWarmagesPerIntermission = 2;
    uint32_t const MargokAnomaliesPerWave = 3;

    uint32_t const MargokPowers[MARGOK_RUNE_COUNT] = { 0, SPELL_MARGOK_POWER_OF_DISPLACEMENT, SPELL_MARGOK_POWER_OF_FORTIFICATION, SPELL_MARGOK_POWER_OF_REPLICATION };
    uint32_t const MargokMarkOfChaos[MARGOK_RUNE_COUNT] = { SPELL_MARGOK_MARK_OF_CHAOS, SPELL_MARGOK_MARK_OF_CHAOS_DISPLACE, SPELL_MARGOK_MARK_OF_CHAOS_FORTIFY, SPELL_MARGOK_MARK_OF_CHAOS_REPLICATE };
    uint32_t const MargokForceNova[MARGOK_RUNE_COUNT] = { SPELL_MARGOK_FORCE_NOVA, SPELL_MARGOK_FORCE_NOVA_DISPLACE, SPELL_MARGOK_FORCE_NOVA_FORTIFY, SPELL_MARGOK_FORCE_NOVA_REPLICATE };
    uint32_t const MargokArcaneWrath[MARGOK_RUNE_COUNT] = { SPELL_MARGOK_ARCANE_WRATH, SPELL_MARGOK_ARCANE_WRATH_DISPLACE, SPELL_MARGOK_ARCANE_WRATH_FORTIFY, SPELL_MARGOK_ARCANE_WRATH_REPLICATE };
    uint32_t const MargokAberrations[MARGOK_RUNE_COUNT] = { NPC_ARCANE_ABERRATION, NPC_DISPLACING_ARCANE_ABERRATION, NPC_FORTIFIED_ARCANE_ABERRATION, NPC_REPLICATING_ARCANE_ABERRATION };
    uint32_t const MargokRotationEvents[] = { EVENT_MARGOK_ARCANE_WRATH, EVENT_MARGOK_DESTRUCTIVE_RESONANCE, EVENT_MARGOK_ARCANE_ABERRATION, EVENT_MARGOK_MARK_OF_CHAOS, EVENT_MARGOK_FORCE_NOVA };

    // encounter frame and boss state together
    void highmaulEncounter(CreatureAIScript* ai, uint32_t dataId, EncounterFrameType frame, EncounterStates state)
    {
        if (InstanceScript* instance = ai->getInstanceScript())
        {
            instance->sendUnitEncounter(frame, ai->getCreature());
            instance->setBossState(dataId, state);
        }
    }

    // random player, optionally not the current victim
    Unit* highmaulPickPlayer(CreatureAIScript* ai, bool skipVictim)
    {
        if (skipVictim)
        {
            if (Unit* target = ai->getBestPlayerTarget(TargetFilter_NotCurrent))
                return target;
        }

        return ai->getBestPlayerTarget(TargetFilter_None);
    }

    // random point on a ring around origin
    LocationVector highmaulScatter(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(HighmaulTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class HighmaulInstanceScript : public InstanceScript
{
public:
    explicit HighmaulInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(HighmaulEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new HighmaulInstanceScript(pMapMgr); }

    // a credited member falling before its partners must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && id == DATA_TWIN_OGRON && getBossState(id) == EncounterStates::InProgress)
        {
            for (Creature* pMember : getCreatureSetForEntries({ BOSS_PHEMOS, BOSS_POL }))
            {
                if (pMember->isAlive())
                    return false;
            }
        }

        return InstanceScript::setBossState(id, state);
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        bool open = false;
        if (!doorShouldBeOpen(pGameObject->getEntry(), open))
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        alignDoor(pGameObject, open);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            GameObject* door = getGameObjectByGuid(guid);
            bool open = false;
            if (door != nullptr && doorShouldBeOpen(door->getEntry(), open))
                alignDoor(door, open);
        }

        if (id == DATA_TWIN_OGRON && state == EncounterStates::Failed)
            reviveOgrons();
    }

private:
    bool isDone(uint32_t id) const { return getBossState(id) == EncounterStates::Performed; }
    bool isFighting(uint32_t id) const { return getBossState(id) == EncounterStates::InProgress; }

    // false for objects that are no encounter door
    bool doorShouldBeOpen(uint32_t entry, bool& open) const
    {
        switch (entry)
        {
            case GO_ARENA_EXIT_GATE:
            case GO_FUNGAL_GIANT_DOOR:
                open = isDone(DATA_KARGATH_BLADEFIST);
                return true;
            case GO_WIND_DOOR:
                open = isDone(DATA_KARGATH_BLADEFIST) && !isFighting(DATA_BRACKENSPORE);
                return true;
            case GO_EARTHEN_PILLAR_DOOR:
                open = !isFighting(DATA_THE_BUTCHER);
                return true;
            case GO_EARTHWALL_1:
            case GO_EARTHWALL_2:
            case GO_EARTHWALL_3:
            case GO_EARTHWALL_4:
                open = !isFighting(DATA_TECTUS);
                return true;
            case GO_PORTAL_SIDE_DOOR:
                open = isDone(DATA_TECTUS);
                return true;
            case GO_TWIN_OGRON_ENTRANCE:
                open = !isFighting(DATA_TWIN_OGRON);
                return true;
            case GO_TWIN_OGRON_EXIT:
                open = isDone(DATA_TWIN_OGRON);
                return true;
            case GO_FEL_BREAKER_ENTRANCE:
                open = !isFighting(DATA_KORAGH);
                return true;
            case GO_FEL_BREAKER_EXIT:
                open = isDone(DATA_KORAGH);
                return true;
            case GO_THRONE_ROOM_DOOR:
                open = !isFighting(DATA_IMPERATOR_MARGOK);
                return true;
            default:
                return false;
        }
    }

    // changes the state only when it differs
    void alignDoor(GameObject* door, bool open)
    {
        bool const isOpen = door->getState() != GO_STATE_CLOSED;
        if (isOpen != open)
            door->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    // a wipe brings a fallen brother back
    void reviveOgrons()
    {
        uint32_t const brothers[] = { BOSS_PHEMOS, BOSS_POL };
        for (uint32_t entry : brothers)
        {
            for (Creature* ogron : getCreatureSetForEntry(entry))
            {
                if (ogron != nullptr && !ogron->isAlive())
                    ogron->Despawn(1000, 5000);
            }
        }
    }

    std::vector<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kargath Bladefist

class HighmaulKargathAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulKargathAI(c); }
    explicit HighmaulKargathAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        highmaulEncounter(this, DATA_KARGATH_BLADEFIST, EncounterFrameEngage, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_KARGATH_BLADE_DANCE, 3000);
        scriptEvents.addEvent(EVENT_KARGATH_IMPALE, 35000);
        scriptEvents.addEvent(EVENT_KARGATH_BERSERKER_RUSH, 48000);
        scriptEvents.addEvent(EVENT_KARGATH_FIRST_BOMBERS, 60000);
        scriptEvents.addEvent(EVENT_KARGATH_CHAIN_HURL, 91000);
        scriptEvents.addEvent(EVENT_KARGATH_BERSERK, 600000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        highmaulEncounter(this, DATA_KARGATH_BLADEFIST, EncounterFrameDisengaged, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        highmaulEncounter(this, DATA_KARGATH_BLADEFIST, EncounterFrameDisengaged, EncounterStates::Performed);
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* summon) override { summons.summon(summon); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_KARGATH_BLADE_DANCE:
                castSpellOnSelf(SPELL_KARGATH_BLADE_DANCE, true);
                scriptEvents.addEvent(EVENT_KARGATH_BLADE_DANCE, 20000);
                break;
            case EVENT_KARGATH_IMPALE:
                castSpellOnVictim(SPELL_KARGATH_IMPALE);
                scriptEvents.addEvent(EVENT_KARGATH_IMPALE, 43500);
                break;
            case EVENT_KARGATH_BERSERKER_RUSH:
                if (Unit* target = highmaulPickPlayer(this, true))
                {
                    sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Kargath Bladefist goes into a Berserker Rush!");
                    castSpell(target, SPELL_KARGATH_BERSERKER_RUSH);
                }
                scriptEvents.addEvent(EVENT_KARGATH_BERSERKER_RUSH, 45000);
                break;
            case EVENT_KARGATH_FIRST_BOMBERS:
                manStands(false);
                break;
            case EVENT_KARGATH_CHAIN_HURL:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Kargath Bladefist hurls his foes into the stands!");
                castSpellAOE(SPELL_KARGATH_CHAIN_HURL, true);
                scriptEvents.delayEvent(EVENT_KARGATH_IMPALE, 18000);
                scriptEvents.delayEvent(EVENT_KARGATH_BERSERKER_RUSH, 18000);
                scriptEvents.addEvent(EVENT_KARGATH_STANDS_RISE, 8000);
                scriptEvents.addEvent(EVENT_KARGATH_CHAIN_HURL, 106000);
                break;
            case EVENT_KARGATH_STANDS_RISE:
                manStands(true);
                break;
            case EVENT_KARGATH_BERSERK:
                castSpellOnSelf(SPELL_HIGHMAUL_BERSERK, true);
                break;
            default:
                break;
        }
    }

private:
    // bombers into free stand slots, bileslingers replaced on request
    void manStands(bool withBileslingers)
    {
        std::list<Creature*> bombers;
        GetCreatureListWithEntryInGrid(bombers, NPC_IRON_BOMBER, 200.0f);

        uint32_t const firstSlot = Util::getRandomUInt(KargathStandSlotCount - 1);
        uint32_t placed = 0;

        for (uint32_t i = 0; i < KargathStandSlotCount && placed < KargathBombersPerWave; ++i)
        {
            LocationVector const& slot = KargathStandSlots[(firstSlot + i) % KargathStandSlotCount];

            bool taken = false;
            for (Creature* bomber : bombers)
            {
                if (bomber->isAlive() && bomber->GetPosition().getExactDist(slot) < 3.0f)
                {
                    taken = true;
                    break;
                }
            }

            if (taken)
                continue;

            float const facing = slot.getAbsoluteAngle(KargathArenaCentre.x, KargathArenaCentre.y);
            if (Creature* bomber = summonCreature(NPC_IRON_BOMBER, slot.x, slot.y, slot.z, facing))
            {
                setZoneWideCombat(bomber);
                ++placed;
            }
        }

        if (!withBileslingers)
            return;

        summons.despawnEntry(NPC_DRUNKEN_BILESLINGER);
        for (LocationVector const& spot : KargathBileslingerSpots)
        {
            if (Creature* bileslinger = summonCreature(NPC_DRUNKEN_BILESLINGER, spot))
                setZoneWideCombat(bileslinger);
        }
    }
};

class HighmaulIronBomberAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulIronBomberAI(c); }
    explicit HighmaulIronBomberAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override { setRooted(true); }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_IRON_BOMB, Util::getRandomUInt(3000, 6000));
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_IRON_BOMB)
        {
            castSpellOnSelf(SPELL_IRON_BOMBER_IRON_BOMB);
            scriptEvents.addEvent(EVENT_IRON_BOMB, Util::getRandomUInt(5000, 9000));
        }
    }
};

class HighmaulBileslingerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulBileslingerAI(c); }
    explicit HighmaulBileslingerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override { setRooted(true); }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_VILE_BREATH, 3000);
        scriptEvents.addEvent(EVENT_MAULING_BREW, Util::getRandomUInt(3000, 5000));
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_VILE_BREATH:
                castSpellOnSelf(SPELL_BILESLINGER_VILE_BREATH);
                scriptEvents.addEvent(EVENT_VILE_BREATH, 10000);
                break;
            case EVENT_MAULING_BREW:
                if (Unit* target = highmaulPickPlayer(this, false))
                    castSpell(target, SPELL_BILESLINGER_MAULING_BREW, true);
                scriptEvents.addEvent(EVENT_MAULING_BREW, Util::getRandomUInt(15000, 25000));
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Butcher

class HighmaulButcherAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulButcherAI(c); }
    explicit HighmaulButcherAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        highmaulEncounter(this, DATA_THE_BUTCHER, EncounterFrameEngage, EncounterStates::InProgress);

        mFrenzied = false;
        mEnergy = 0;
        mEnergyClock = 0;

        scriptEvents.addEvent(EVENT_BUTCHER_MEAT_HOOK, 5000);
        scriptEvents.addEvent(EVENT_BUTCHER_TENDERIZER, 6000);
        scriptEvents.addEvent(EVENT_BUTCHER_CLEAVE, 10000);
        scriptEvents.addEvent(EVENT_BUTCHER_THE_CLEAVER, 12000);
        scriptEvents.addEvent(EVENT_BUTCHER_BERSERK, 300000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        highmaulEncounter(this, DATA_THE_BUTCHER, EncounterFrameDisengaged, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        highmaulEncounter(this, DATA_THE_BUTCHER, EncounterFrameDisengaged, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        buildEnergy(diff);

        if (!mFrenzied && _isInCombat() && _getHealthPercent() <= 30)
        {
            mFrenzied = true;
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "The Butcher goes into a frenzy!");
            castSpellOnSelf(SPELL_BUTCHER_FRENZY, true);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BUTCHER_MEAT_HOOK:
                if (Unit* victim = getCreature()->getAIInterface()->getCurrentTarget())
                {
                    if (getRangeToObject(victim) > 5.0f)
                        castSpell(victim, SPELL_BUTCHER_MEAT_HOOK, true);
                }
                scriptEvents.addEvent(EVENT_BUTCHER_MEAT_HOOK, 5000);
                break;
            case EVENT_BUTCHER_TENDERIZER:
                castSpellOnVictim(SPELL_BUTCHER_TENDERIZER, true);
                scriptEvents.addEvent(EVENT_BUTCHER_TENDERIZER, 16000);
                break;
            case EVENT_BUTCHER_CLEAVE:
                castSpellOnVictim(SPELL_BUTCHER_CLEAVE);
                scriptEvents.addEvent(EVENT_BUTCHER_CLEAVE, 5000);
                break;
            case EVENT_BUTCHER_THE_CLEAVER:
                castSpellOnVictim(SPELL_BUTCHER_THE_CLEAVER, true);
                scriptEvents.addEvent(EVENT_BUTCHER_THE_CLEAVER, 8000);
                break;
            case EVENT_BUTCHER_BOUNDING_CLEAVE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "The Butcher prepares a Bounding Cleave!");
                castSpellOnSelf(SPELL_BUTCHER_BOUNDING_CLEAVE, true);
                castSpellOnSelf(SPELL_BUTCHER_BOUNDING_CLEAVE_WIND);
                scriptEvents.delayAllEvents(12000);
                break;
            case EVENT_BUTCHER_BERSERK:
                castSpellOnSelf(SPELL_HIGHMAUL_BERSERK, true);
                break;
            default:
                break;
        }
    }

private:
    // 5 energy per 3s, 10 once frenzied; full bar means Bounding Cleave
    void buildEnergy(uint32_t diff)
    {
        if (!_isInCombat())
            return;

        mEnergyClock += diff;
        while (mEnergyClock >= 3000)
        {
            mEnergyClock -= 3000;
            mEnergy += mFrenzied ? 10 : 5;
        }

        if (mEnergy >= 100)
        {
            mEnergy = 0;
            scriptEvents.addEvent(EVENT_BUTCHER_BOUNDING_CLEAVE, 100);
        }
    }

    bool mFrenzied = false;
    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Tectus - shared kit of Tectus, his shards and motes

class HighmaulMountainAI : public CreatureAIScript
{
public:
    explicit HighmaulMountainAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

protected:
    void beginMountainKit()
    {
        mEnergy = 0;
        mEnergyClock = 0;
        mUpheavalLeft = 0;

        scriptEvents.addEvent(EVENT_MOUNTAIN_CRYSTALLINE_BARRAGE, 5000);
        scriptEvents.addEvent(EVENT_MOUNTAIN_ACCRETION, 5000);
        scriptEvents.addEvent(EVENT_MOUNTAIN_FRACTURE, 8000);
    }

    uint32_t mountainAura()
    {
        switch (getCreature()->getEntry())
        {
            case NPC_SHARD_OF_TECTUS:
                return SPELL_TECTUS_SHARD_OF_THE_MOUNTAIN;
            case NPC_MOTE_OF_TECTUS:
                return SPELL_TECTUS_MOTE_OF_THE_MOUNTAIN;
            default:
                return SPELL_TECTUS_LIVING_MOUNTAIN;
        }
    }

    // lethal blows only break a piece while this is open
    bool isUpheavalOpen() const { return mUpheavalLeft > 0; }

    // runs the shared rotation, returns events left for the caller
    uint32_t updateMountain(uint32_t diff)
    {
        if (mUpheavalLeft > 0)
        {
            if (mUpheavalLeft > diff)
            {
                mUpheavalLeft -= diff;
            }
            else
            {
                mUpheavalLeft = 0;
                castSpellOnSelf(mountainAura(), true);
            }
        }

        gatherEnergy(diff);
        scriptEvents.updateEvents(diff, 0);

        if (_isCasting())
            return 0;

        uint32_t const eventId = scriptEvents.getFinishedEvent();
        switch (eventId)
        {
            case EVENT_MOUNTAIN_CRYSTALLINE_BARRAGE:
                if (Unit* target = highmaulPickPlayer(this, true))
                    castSpell(target, SPELL_TECTUS_CRYSTALLINE_BARRAGE, true);
                scriptEvents.addEvent(EVENT_MOUNTAIN_CRYSTALLINE_BARRAGE, 30000);
                return 0;
            case EVENT_MOUNTAIN_FRACTURE:
                if (Unit* target = highmaulPickPlayer(this, false))
                    castSpell(target, SPELL_TECTUS_FRACTURE, true);
                scriptEvents.addEvent(EVENT_MOUNTAIN_FRACTURE, 6000);
                return 0;
            case EVENT_MOUNTAIN_ACCRETION:
                castSpellOnSelf(SPELL_TECTUS_ACCRETION, true);
                scriptEvents.addEvent(EVENT_MOUNTAIN_ACCRETION, 5000);
                return 0;
            case EVENT_MOUNTAIN_EARTHEN_PILLAR:
                if (Unit* target = highmaulPickPlayer(this, true))
                    summonCreature(NPC_EARTHEN_PILLAR_STALKER, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TIMED_DESPAWN, 65000);
                return 0;
            case EVENT_MOUNTAIN_TECTONIC_UPHEAVAL:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "The ground heaves with a Tectonic Upheaval!");
                _removeAura(mountainAura());
                castSpellOnSelf(SPELL_TECTUS_TECTONIC_UPHEAVAL);
                mUpheavalLeft = TectusUpheavalWindowMs;
                return 0;
            default:
                return eventId;
        }
    }

private:
    // pillar at 25 energy, upheaval at 100
    void gatherEnergy(uint32_t diff)
    {
        if (!_isInCombat())
            return;

        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;

            uint32_t const before = mEnergy;
            mEnergy += energyPerSecond();

            if (before < 25 && mEnergy >= 25)
                scriptEvents.addEvent(EVENT_MOUNTAIN_EARTHEN_PILLAR, 100);

            if (mEnergy >= 100)
            {
                mEnergy = 0;
                scriptEvents.addEvent(EVENT_MOUNTAIN_TECTONIC_UPHEAVAL, 100);
            }
        }
    }

    // the lower the health the faster the energy
    uint32_t energyPerSecond()
    {
        int32_t const health = _getHealthPercent();
        if (health >= 50)
            return 1;
        if (health >= 20)
            return 2;
        if (health >= 10)
            return 3;
        if (health >= 5)
            return 4;
        return 5;
    }

    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    uint32_t mUpheavalLeft = 0;
};

class HighmaulTectusAI : public HighmaulMountainAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulTectusAI(c); }
    explicit HighmaulTectusAI(Creature* pCreature) : HighmaulMountainAI(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        highmaulEncounter(this, DATA_TECTUS, EncounterFrameEngage, EncounterStates::InProgress);

        mShattered = false;
        mShatterPending = false;
        mCollapsePending = false;
        mMotesBroken = 0;
        mMoteSpots.clear();

        castSpellOnSelf(SPELL_TECTUS_LIVING_MOUNTAIN, true);
        beginMountainKit();

        scriptEvents.addEvent(EVENT_TECTUS_EARTHWARPER, 8000);
        scriptEvents.addEvent(EVENT_TECTUS_BERSERKER, 18000);
        scriptEvents.addEvent(EVENT_TECTUS_BERSERK, 600000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        highmaulEncounter(this, DATA_TECTUS, EncounterFrameDisengaged, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();

        if (mShattered)
        {
            removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
            setRooted(false);
            setReactState(REACT_AGGRESSIVE);
        }

        mShattered = false;
        mShatterPending = false;
        mCollapsePending = false;
        mMoteSpots.clear();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        highmaulEncounter(this, DATA_TECTUS, EncounterFrameDisengaged, EncounterStates::Performed);
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* summon) override { summons.summon(summon); }

    void DamageTaken(Unit* pAttacker, uint32_t* damage) override
    {
        // own collapse blow passes
        if (damage == nullptr || pAttacker == getCreature())
            return;

        if (mShattered)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        if (isUpheavalOpen())
        {
            mShattered = true;
            mShatterPending = true;
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mShattered)
        {
            updateShattered();
            return;
        }

        switch (updateMountain(static_cast<uint32_t>(time_passed)))
        {
            case EVENT_TECTUS_EARTHWARPER:
                callNightTwisted(NPC_NIGHT_TWISTED_EARTHWARPER);
                scriptEvents.addEvent(EVENT_TECTUS_EARTHWARPER, 40000);
                break;
            case EVENT_TECTUS_BERSERKER:
                callNightTwisted(NPC_NIGHT_TWISTED_BERSERKER);
                scriptEvents.addEvent(EVENT_TECTUS_BERSERKER, 40000);
                break;
            case EVENT_TECTUS_BERSERK:
                castSpellOnSelf(SPELL_HIGHMAUL_BERSERK, true);
                break;
            default:
                break;
        }
    }

    // a shard broke, motes follow at its spot
    void onShardBroken(LocationVector const& where) { mMoteSpots.push_back(where); }

    void onMoteBroken()
    {
        if (++mMotesBroken >= TectusShardCount * TectusMotesPerShard)
            mCollapsePending = true;
    }

private:
    // the body waits untouchable until every mote is gone
    void updateShattered()
    {
        if (mShatterPending)
        {
            mShatterPending = false;
            scriptEvents.resetEvents();
            attackStop();
            setReactState(REACT_PASSIVE);
            setRooted(true);
            addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Tectus shatters!");

            if (InstanceScript* instance = getInstanceScript())
                instance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            splitInto(NPC_SHARD_OF_TECTUS, TectusShardCount, getCreature()->GetPosition());
        }

        for (LocationVector const& spot : mMoteSpots)
            splitInto(NPC_MOTE_OF_TECTUS, TectusMotesPerShard, spot);
        mMoteSpots.clear();

        if (mCollapsePending)
        {
            mCollapsePending = false;
            removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
            getCreature()->dealDamage(getCreature(), getCreature()->getHealth(), 0);
        }
    }

    void splitInto(uint32_t entry, uint32_t count, LocationVector const& origin)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            if (Creature* piece = summonCreature(entry, highmaulScatter(origin, 7.0f, 7.0f)))
                setZoneWideCombat(piece);
        }
    }

    // Night-Twisted adds rise next to a raid member
    void callNightTwisted(uint32_t entry)
    {
        Unit* target = highmaulPickPlayer(this, true);
        if (target == nullptr)
            return;

        if (Creature* add = summonCreature(entry, highmaulScatter(target->GetPosition(), 5.0f, 5.0f)))
        {
            setZoneWideCombat(add);
            if (CreatureAIScript* addAI = add->GetScript())
                addAI->attackStart(target);
        }
    }

    bool mShattered = false;
    bool mShatterPending = false;
    bool mCollapsePending = false;
    uint32_t mMotesBroken = 0;
    std::vector<LocationVector> mMoteSpots;
};

class HighmaulTectusFragmentAI : public HighmaulMountainAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulTectusFragmentAI(c); }
    explicit HighmaulTectusFragmentAI(Creature* pCreature) : HighmaulMountainAI(pCreature) {}

    void OnLoad() override
    {
        castSpellOnSelf(mountainAura(), true);
        castSpellOnSelf(isShard() ? SPELL_TECTUS_SHARD_DAMAGE_DOWN : SPELL_TECTUS_MOTE_DAMAGE_DOWN, true);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        if (InstanceScript* instance = getInstanceScript())
            instance->sendUnitEncounter(EncounterFrameEngage, getCreature());

        beginMountainKit();
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* instance = getInstanceScript())
            instance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        HighmaulTectusAI* tectus = dynamic_cast<HighmaulTectusAI*>(getNearestCreatureAI(BOSS_TECTUS));
        if (tectus == nullptr)
            return;

        if (isShard())
            tectus->onShardBroken(getCreature()->GetPosition());
        else
            tectus->onMoteBroken();
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        // shards hold at 1 hp outside their upheaval, motes break freely
        if (damage == nullptr || !isShard() || isUpheavalOpen())
            return;

        uint32_t const health = getCreature()->getHealth();
        if (*damage >= health)
            *damage = health > 1 ? health - 1 : 0;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        updateMountain(static_cast<uint32_t>(time_passed));
    }

private:
    bool isShard() { return getCreature()->getEntry() == NPC_SHARD_OF_TECTUS; }
};

class HighmaulEarthenPillarAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulEarthenPillarAI(c); }
    explicit HighmaulEarthenPillarAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        castSpellOnSelf(SPELL_TECTUS_EARTHEN_PILLAR_TIMER, true);
        mEruptIn = 5000;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mEruptIn == 0)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mEruptIn > diff)
        {
            mEruptIn -= diff;
            return;
        }

        mEruptIn = 0;
        castSpellOnSelf(SPELL_TECTUS_EARTHEN_PILLAR, true);
    }

private:
    uint32_t mEruptIn = 0;
};

class HighmaulNightTwistedBerserkerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulNightTwistedBerserkerAI(c); }
    explicit HighmaulNightTwistedBerserkerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_RAVING_ASSAULT, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_RAVING_ASSAULT)
        {
            if (Unit* target = highmaulPickPlayer(this, false))
                castSpell(target, SPELL_BERSERKER_RAVING_ASSAULT);
            scriptEvents.addEvent(EVENT_RAVING_ASSAULT, 20000);
        }
    }
};

class HighmaulNightTwistedEarthwarperAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulNightTwistedEarthwarperAI(c); }
    explicit HighmaulNightTwistedEarthwarperAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_EARTHEN_FLECHETTES, 5000);
        scriptEvents.addEvent(EVENT_GIFT_OF_EARTH, 12000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_EARTHEN_FLECHETTES:
                castSpellOnSelf(SPELL_EARTHWARPER_EARTHEN_FLECHETTES);
                scriptEvents.addEvent(EVENT_EARTHEN_FLECHETTES, 15000);
                break;
            case EVENT_GIFT_OF_EARTH:
                castSpellOnSelf(SPELL_EARTHWARPER_GIFT_OF_EARTH);
                scriptEvents.addEvent(EVENT_GIFT_OF_EARTH, 25000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Brackenspore

class HighmaulBrackensporeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulBrackensporeAI(c); }
    explicit HighmaulBrackensporeAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        highmaulEncounter(this, DATA_BRACKENSPORE, EncounterFrameEngage, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_BRACKENSPORE_ROT, 10000);
        scriptEvents.addEvent(EVENT_BRACKENSPORE_MIND_FUNGUS, 10000);
        scriptEvents.addEvent(EVENT_BRACKENSPORE_LIVING_MUSHROOM, 17000);
        scriptEvents.addEvent(EVENT_BRACKENSPORE_SPORE_SHOOTERS, 20000);
        scriptEvents.addEvent(EVENT_BRACKENSPORE_NECROTIC_BREATH, 30000);
        scriptEvents.addEvent(EVENT_BRACKENSPORE_FLESH_EATER, 32000);
        scriptEvents.addEvent(EVENT_BRACKENSPORE_INFESTING_SPORES, 60000);
        scriptEvents.addEvent(EVENT_BRACKENSPORE_REJUVENATING_MUSHROOM, 80000);
        scriptEvents.addEvent(EVENT_BRACKENSPORE_BERSERK, 600000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        highmaulEncounter(this, DATA_BRACKENSPORE, EncounterFrameDisengaged, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        highmaulEncounter(this, DATA_BRACKENSPORE, EncounterFrameDisengaged, EncounterStates::Performed);
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* summon) override { summons.summon(summon); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BRACKENSPORE_ROT:
                castSpellOnVictim(SPELL_BRACKENSPORE_ROT, true);
                scriptEvents.addEvent(EVENT_BRACKENSPORE_ROT, 10000);
                break;
            case EVENT_BRACKENSPORE_MIND_FUNGUS:
                sproutNearRaid(NPC_MIND_FUNGUS, 4.0f, 12.0f, false);
                scriptEvents.addEvent(EVENT_BRACKENSPORE_MIND_FUNGUS, 30000);
                break;
            case EVENT_BRACKENSPORE_LIVING_MUSHROOM:
                sproutNearRaid(NPC_LIVING_MUSHROOM, 4.0f, 12.0f, false);
                scriptEvents.addEvent(EVENT_BRACKENSPORE_LIVING_MUSHROOM, 55000);
                break;
            case EVENT_BRACKENSPORE_SPORE_SHOOTERS:
                for (uint32_t i = 0; i < BrackensporeSporeShooters; ++i)
                    sproutNearRaid(NPC_SPORE_SHOOTER, 8.0f, 16.0f, true);
                scriptEvents.addEvent(EVENT_BRACKENSPORE_SPORE_SHOOTERS, 57000);
                break;
            case EVENT_BRACKENSPORE_NECROTIC_BREATH:
                castSpellOnSelf(SPELL_BRACKENSPORE_NECROTIC_BREATH);
                scriptEvents.addEvent(EVENT_BRACKENSPORE_NECROTIC_BREATH, 32000);
                break;
            case EVENT_BRACKENSPORE_FLESH_EATER:
            {
                LocationVector const& shore = BrackensporeFleshEaterSpots[Util::getRandomUInt(1)];
                if (Creature* fleshEater = summonCreature(NPC_FUNGAL_FLESH_EATER, shore))
                    setZoneWideCombat(fleshEater);
                scriptEvents.addEvent(EVENT_BRACKENSPORE_FLESH_EATER, 120000);
                break;
            }
            case EVENT_BRACKENSPORE_INFESTING_SPORES:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Brackenspore releases Infesting Spores!");
                castSpellOnSelf(SPELL_BRACKENSPORE_INFESTING_SPORES);
                scriptEvents.addEvent(EVENT_BRACKENSPORE_INFESTING_SPORES, 72000);
                break;
            case EVENT_BRACKENSPORE_REJUVENATING_MUSHROOM:
                sproutNearRaid(NPC_REJUVENATING_MUSHROOM, 0.0f, 2.0f, false);
                scriptEvents.addEvent(EVENT_BRACKENSPORE_REJUVENATING_MUSHROOM, 130000);
                break;
            case EVENT_BRACKENSPORE_BERSERK:
                castSpellOnSelf(SPELL_HIGHMAUL_BERSERK, true);
                break;
            default:
                break;
        }
    }

private:
    // fungus grows around a random raid member
    void sproutNearRaid(uint32_t entry, float minRange, float maxRange, bool hostile)
    {
        Unit* target = highmaulPickPlayer(this, false);
        if (target == nullptr)
            return;

        Creature* growth = summonCreature(entry, highmaulScatter(target->GetPosition(), minRange, maxRange));
        if (growth != nullptr && hostile)
            setZoneWideCombat(growth);
    }
};

class HighmaulFungalFleshEaterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulFungalFleshEaterAI(c); }
    explicit HighmaulFungalFleshEaterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_FLESH_EATER_FEAST, 1000);
        scriptEvents.addEvent(EVENT_FLESH_EATER_DECAY, 5000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_FLESH_EATER_FEAST:
                castSpellOnSelf(SPELL_FLESH_EATER_FLESH_EATER, true);
                scriptEvents.addEvent(EVENT_FLESH_EATER_FEAST, 10000);
                break;
            case EVENT_FLESH_EATER_DECAY:
                castSpellOnSelf(SPELL_FLESH_EATER_DECAY);
                scriptEvents.addEvent(EVENT_FLESH_EATER_DECAY, 10000);
                break;
            default:
                break;
        }
    }
};

class HighmaulSporeShooterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulSporeShooterAI(c); }
    explicit HighmaulSporeShooterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override { setRooted(true); }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SPORE_SHOT, Util::getRandomUInt(100, 1500));
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_SPORE_SHOT)
        {
            castSpellOnSelf(SPELL_SPORE_SHOOTER_SPORE_SHOT);
            scriptEvents.addEvent(EVENT_SPORE_SHOT, 4500);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Twin Ogron - Phemos and Pol share one encounter

class HighmaulOgronBrotherAI : public CreatureAIScript
{
public:
    HighmaulOgronBrotherAI(Creature* pCreature, uint32_t brotherEntry) : CreatureAIScript(pCreature), mBrotherEntry(brotherEntry) {}

    void OnCombatStart(Unit* pTarget) override
    {
        highmaulEncounter(this, DATA_TWIN_OGRON, EncounterFrameEngage, EncounterStates::InProgress);

        mEnergy = 0;
        mEnergyClock = 0;
        scriptEvents.addEvent(EVENT_OGRON_BERSERK, 480000);

        pullBrother(pTarget);
        onBrotherEngaged();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        highmaulEncounter(this, DATA_TWIN_OGRON, EncounterFrameDisengaged, EncounterStates::Failed);
        scriptEvents.resetEvents();
        setRooted(false);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        InstanceScript* instance = getInstanceScript();
        if (instance == nullptr)
            return;

        instance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        // the last brother standing completes the encounter
        if (!isBrotherAlive())
            instance->setBossState(DATA_TWIN_OGRON, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        chargeEnergy(diff);

        if (_isCasting())
            return;

        uint32_t const eventId = scriptEvents.getFinishedEvent();
        if (eventId == EVENT_OGRON_BERSERK)
        {
            castSpellOnSelf(SPELL_HIGHMAUL_BERSERK, true);
            return;
        }

        if (eventId != 0)
            onBrotherEvent(eventId);
    }

protected:
    virtual void onBrotherEngaged() = 0;
    virtual uint32_t nextEnergyGain() = 0;
    virtual void onEnergyReached(uint32_t threshold) = 0;
    virtual void onBrotherEvent(uint32_t eventId) = 0;

private:
    // abilities fire at 33, 66 and 100 energy
    void chargeEnergy(uint32_t diff)
    {
        if (!_isInCombat())
            return;

        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;

            uint32_t const before = mEnergy;
            mEnergy += nextEnergyGain();

            if (before < 33 && mEnergy >= 33)
                onEnergyReached(33);
            if (before < 66 && mEnergy >= 66)
                onEnergyReached(66);
            if (mEnergy >= 100)
            {
                mEnergy = 0;
                onEnergyReached(100);
            }
        }
    }

    bool isBrotherAlive()
    {
        if (InstanceScript* instance = getInstanceScript())
        {
            for (Creature* brother : instance->getCreatureSetForEntry(mBrotherEntry))
            {
                if (brother != nullptr && brother->isAlive())
                    return true;
            }
        }
        return false;
    }

    void pullBrother(Unit* target)
    {
        InstanceScript* instance = getInstanceScript();
        if (target == nullptr || instance == nullptr)
            return;

        for (Creature* brother : instance->getCreatureSetForEntry(mBrotherEntry))
        {
            if (brother == nullptr || !brother->isAlive())
                continue;

            CreatureAIScript* brotherAI = brother->GetScript();
            if (brotherAI != nullptr && !brotherAI->_isInCombat())
                brotherAI->attackStart(target);
        }
    }

    uint32_t mBrotherEntry;
    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
};

class HighmaulPhemosAI : public HighmaulOgronBrotherAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulPhemosAI(c); }
    explicit HighmaulPhemosAI(Creature* pCreature) : HighmaulOgronBrotherAI(pCreature, BOSS_POL) {}

protected:
    void onBrotherEngaged() override
    {
        scriptEvents.addEvent(EVENT_PHEMOS_DOUBLE_SLASH, 26000);
    }

    uint32_t nextEnergyGain() override { return 1; }

    void onEnergyReached(uint32_t threshold) override
    {
        if (threshold == 33)
            scriptEvents.addEvent(EVENT_PHEMOS_WHIRLWIND, 100);
        else if (threshold == 66)
            scriptEvents.addEvent(EVENT_PHEMOS_ENFEEBLING_ROAR, 100);
        else
            scriptEvents.addEvent(EVENT_PHEMOS_QUAKE, 100);
    }

    void onBrotherEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_PHEMOS_DOUBLE_SLASH:
                castSpellOnVictim(SPELL_PHEMOS_DOUBLE_SLASH);
                scriptEvents.addEvent(EVENT_PHEMOS_DOUBLE_SLASH_OFF_HAND, 1000);
                scriptEvents.addEvent(EVENT_PHEMOS_DOUBLE_SLASH, 27000);
                break;
            case EVENT_PHEMOS_DOUBLE_SLASH_OFF_HAND:
                castSpellOnVictim(SPELL_PHEMOS_DOUBLE_SLASH_OFF_HAND, true);
                break;
            case EVENT_PHEMOS_WHIRLWIND:
                castSpellOnSelf(SPELL_PHEMOS_WHIRLWIND);
                break;
            case EVENT_PHEMOS_ENFEEBLING_ROAR:
                castSpellOnSelf(SPELL_PHEMOS_ENFEEBLING_ROAR);
                break;
            case EVENT_PHEMOS_QUAKE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Phemos shakes the ground with a Quake!");
                castSpellOnSelf(SPELL_PHEMOS_QUAKE);
                break;
            default:
                break;
        }
    }
};

class HighmaulPolAI : public HighmaulOgronBrotherAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulPolAI(c); }
    explicit HighmaulPolAI(Creature* pCreature) : HighmaulOgronBrotherAI(pCreature, BOSS_PHEMOS) {}

protected:
    void onBrotherEngaged() override
    {
        mBigTick = false;
        scriptEvents.addEvent(EVENT_POL_SHIELD_BASH, 22000);
    }

    // Pol charges faster: 1 and 2 energy in turn
    uint32_t nextEnergyGain() override
    {
        mBigTick = !mBigTick;
        return mBigTick ? 2 : 1;
    }

    void onEnergyReached(uint32_t threshold) override
    {
        if (threshold == 33)
            scriptEvents.addEvent(EVENT_POL_SHIELD_CHARGE, 100);
        else if (threshold == 66)
            scriptEvents.addEvent(EVENT_POL_INTERRUPTING_SHOUT, 100);
        else
            scriptEvents.addEvent(EVENT_POL_PULVERIZE, 100);
    }

    void onBrotherEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_POL_SHIELD_BASH:
                castSpellOnVictim(SPELL_POL_SHIELD_BASH);
                scriptEvents.addEvent(EVENT_POL_SHIELD_BASH, 22000);
                break;
            case EVENT_POL_SHIELD_CHARGE:
                if (Unit* target = highmaulPickPlayer(this, true))
                    castSpell(target, SPELL_POL_SHIELD_CHARGE);
                break;
            case EVENT_POL_INTERRUPTING_SHOUT:
                castSpellOnSelf(SPELL_POL_INTERRUPTING_SHOUT);
                break;
            case EVENT_POL_PULVERIZE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Pol begins to Pulverize!");
                setRooted(true);
                castSpellOnSelf(SPELL_POL_PULVERIZE, true);
                scriptEvents.addEvent(EVENT_POL_PULVERIZE_FIRST, 2000);
                scriptEvents.addEvent(EVENT_POL_PULVERIZE_SECOND, 5000);
                scriptEvents.addEvent(EVENT_POL_PULVERIZE_THIRD, 9000);
                scriptEvents.addEvent(EVENT_POL_PULVERIZE_DONE, 15000);
                break;
            case EVENT_POL_PULVERIZE_FIRST:
                castSpellOnSelf(SPELL_POL_PULVERIZE_FIRST);
                break;
            case EVENT_POL_PULVERIZE_SECOND:
                castSpellOnSelf(SPELL_POL_PULVERIZE_SECOND);
                break;
            case EVENT_POL_PULVERIZE_THIRD:
                castSpellOnSelf(SPELL_POL_PULVERIZE_THIRD);
                break;
            case EVENT_POL_PULVERIZE_DONE:
                setRooted(false);
                break;
            default:
                break;
        }
    }

private:
    bool mBigTick = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ko'ragh

class HighmaulKoraghAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulKoraghAI(c); }
    explicit HighmaulKoraghAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        highmaulEncounter(this, DATA_KORAGH, EncounterFrameEngage, EncounterStates::InProgress);

        mRecharging = false;
        raiseBarrier();

        scriptEvents.addEvent(EVENT_KORAGH_EXPEL_FIRE, 6000, HIGHMAUL_PHASE_ROTATION);
        scriptEvents.addEvent(EVENT_KORAGH_BREAKERS_STRENGTH, 10000, HIGHMAUL_PHASE_ROTATION);
        scriptEvents.addEvent(EVENT_KORAGH_SUPPRESSION_FIELD, 15000, HIGHMAUL_PHASE_ROTATION);
        scriptEvents.addEvent(EVENT_KORAGH_EXPEL_ARCANE, 30000, HIGHMAUL_PHASE_ROTATION);
        scriptEvents.addEvent(EVENT_KORAGH_OVERFLOWING_ENERGY, 36000, HIGHMAUL_PHASE_ROTATION);
        scriptEvents.addEvent(EVENT_KORAGH_EXPEL_FROST, 40000, HIGHMAUL_PHASE_ROTATION);
        scriptEvents.addEvent(EVENT_KORAGH_EXPEL_SHADOW, 55000, HIGHMAUL_PHASE_ROTATION);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        highmaulEncounter(this, DATA_KORAGH, EncounterFrameDisengaged, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();

        mRecharging = false;
        setRooted(false);
        setReactState(REACT_AGGRESSIVE);
        _removeAura(SPELL_KORAGH_VULNERABILITY);
        _removeAura(SPELL_KORAGH_VOLATILE_ANOMALIES);
        _removeAura(SPELL_KORAGH_BREAKERS_STRENGTH);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        highmaulEncounter(this, DATA_KORAGH, EncounterFrameDisengaged, EncounterStates::Performed);
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* summon) override { summons.summon(summon); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mRecharging ? HIGHMAUL_PHASE_BREAK : HIGHMAUL_PHASE_ROTATION);

        if (!mRecharging && _isInCombat() && barrierBroken())
            beginRecharge();

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_KORAGH_EXPEL_FIRE:
                castSpellOnSelf(SPELL_KORAGH_EXPEL_MAGIC_FIRE);
                scriptEvents.addEvent(EVENT_KORAGH_EXPEL_FIRE, 60000, HIGHMAUL_PHASE_ROTATION);
                break;
            case EVENT_KORAGH_SUPPRESSION_FIELD:
                if (Unit* target = highmaulPickPlayer(this, false))
                    castSpell(target, SPELL_KORAGH_SUPPRESSION_FIELD);
                scriptEvents.addEvent(EVENT_KORAGH_SUPPRESSION_FIELD, 17000, HIGHMAUL_PHASE_ROTATION);
                break;
            case EVENT_KORAGH_EXPEL_ARCANE:
                castSpellOnVictim(SPELL_KORAGH_EXPEL_MAGIC_ARCANE);
                scriptEvents.addEvent(EVENT_KORAGH_EXPEL_ARCANE, 30000, HIGHMAUL_PHASE_ROTATION);
                break;
            case EVENT_KORAGH_OVERFLOWING_ENERGY:
                castSpellOnSelf(SPELL_KORAGH_OVERFLOWING_ENERGY, true);
                scriptEvents.addEvent(EVENT_KORAGH_OVERFLOWING_ENERGY, 30000, HIGHMAUL_PHASE_ROTATION);
                break;
            case EVENT_KORAGH_EXPEL_FROST:
                if (Unit* target = highmaulPickPlayer(this, true))
                    castSpell(target, SPELL_KORAGH_EXPEL_MAGIC_FROST);
                scriptEvents.addEvent(EVENT_KORAGH_EXPEL_FROST, 60000, HIGHMAUL_PHASE_ROTATION);
                break;
            case EVENT_KORAGH_EXPEL_SHADOW:
                castSpellOnSelf(SPELL_KORAGH_EXPEL_MAGIC_SHADOW);
                scriptEvents.addEvent(EVENT_KORAGH_EXPEL_SHADOW, 60000, HIGHMAUL_PHASE_ROTATION);
                break;
            case EVENT_KORAGH_BREAKERS_STRENGTH:
                castSpellOnSelf(SPELL_KORAGH_BREAKERS_STRENGTH, true);
                scriptEvents.addEvent(EVENT_KORAGH_BREAKERS_STRENGTH, 10000, HIGHMAUL_PHASE_ROTATION);
                break;
            case EVENT_KORAGH_RECHARGE_START:
                setRooted(true);
                castSpellOnSelf(SPELL_KORAGH_RECHARGE_KNOCKBACK, true);
                castSpellOnSelf(SPELL_KORAGH_VULNERABILITY, true);
                castSpellOnSelf(SPELL_KORAGH_VOLATILE_ANOMALIES, true);
                scriptEvents.addEvent(EVENT_KORAGH_RECHARGE_DONE, 20000, HIGHMAUL_PHASE_BREAK);
                break;
            case EVENT_KORAGH_RECHARGE_DONE:
                finishRecharge();
                break;
            default:
                break;
        }
    }

private:
    void raiseBarrier()
    {
        castSpellOnSelf(SPELL_KORAGH_NULLIFICATION_BARRIER, true);
        mBarrierSeen = false;
    }

    // true once a barrier that was up has been absorbed away
    bool barrierBroken()
    {
        bool const barrierUp = hasAura(SPELL_KORAGH_NULLIFICATION_BARRIER);
        if (!mBarrierSeen)
        {
            mBarrierSeen = barrierUp;
            return false;
        }
        return !barrierUp;
    }

    void beginRecharge()
    {
        mRecharging = true;
        mBarrierSeen = false;

        _removeAura(SPELL_KORAGH_BREAKERS_STRENGTH);
        sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Ko'ragh's barrier is broken, he starts to recharge!");

        attackStop();
        setReactState(REACT_PASSIVE);
        moveTo(KoraghRuneCentre.x, KoraghRuneCentre.y, KoraghRuneCentre.z);
        scriptEvents.addEvent(EVENT_KORAGH_RECHARGE_START, 4000, HIGHMAUL_PHASE_BREAK);
    }

    void finishRecharge()
    {
        _removeAura(SPELL_KORAGH_VULNERABILITY);
        _removeAura(SPELL_KORAGH_VOLATILE_ANOMALIES);
        setRooted(false);
        setReactState(REACT_AGGRESSIVE);

        mRecharging = false;
        raiseBarrier();
        scriptEvents.addEvent(EVENT_KORAGH_BREAKERS_STRENGTH, 10000, HIGHMAUL_PHASE_ROTATION);
    }

    bool mRecharging = false;
    bool mBarrierSeen = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Imperator Mar'gok

class HighmaulMargokAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulMargokAI(c); }
    explicit HighmaulMargokAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        highmaulEncounter(this, DATA_IMPERATOR_MARGOK, EncounterFrameEngage, EncounterStates::InProgress);

        mRune = MARGOK_RUNE_CROWN;
        mNextRune = MARGOK_RUNE_CROWN;
        mIntermission = false;

        scriptEvents.addEvent(EVENT_MARGOK_ARCANE_WRATH, 6000, HIGHMAUL_PHASE_ROTATION);
        scriptEvents.addEvent(EVENT_MARGOK_DESTRUCTIVE_RESONANCE, 15000, HIGHMAUL_PHASE_ROTATION);
        scriptEvents.addEvent(EVENT_MARGOK_ARCANE_ABERRATION, 25000, HIGHMAUL_PHASE_ROTATION);
        scriptEvents.addEvent(EVENT_MARGOK_MARK_OF_CHAOS, 34000, HIGHMAUL_PHASE_ROTATION);
        scriptEvents.addEvent(EVENT_MARGOK_FORCE_NOVA, 45000, HIGHMAUL_PHASE_ROTATION);
        scriptEvents.addEvent(EVENT_MARGOK_BERSERK, 900000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        highmaulEncounter(this, DATA_IMPERATOR_MARGOK, EncounterFrameDisengaged, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();

        mIntermission = false;
        setRooted(false);
        setReactState(REACT_AGGRESSIVE);
        _removeAura(SPELL_MARGOK_ARCANE_PROTECTION);
        for (uint32_t power : MargokPowers)
        {
            if (power != 0)
                _removeAura(power);
        }
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        highmaulEncounter(this, DATA_IMPERATOR_MARGOK, EncounterFrameDisengaged, EncounterStates::Performed);
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* summon) override { summons.summon(summon); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mIntermission ? HIGHMAUL_PHASE_BREAK : HIGHMAUL_PHASE_ROTATION);
        checkRunes();

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MARGOK_ARCANE_WRATH:
                if (Unit* target = highmaulPickPlayer(this, true))
                    castSpell(target, MargokArcaneWrath[mRune], true);
                scriptEvents.addEvent(EVENT_MARGOK_ARCANE_WRATH, 50000, HIGHMAUL_PHASE_ROTATION);
                break;
            case EVENT_MARGOK_DESTRUCTIVE_RESONANCE:
                if (Unit* target = highmaulPickPlayer(this, true))
                    castSpell(target, SPELL_MARGOK_DESTRUCTIVE_RESONANCE, true);
                scriptEvents.addEvent(EVENT_MARGOK_DESTRUCTIVE_RESONANCE, 15000, HIGHMAUL_PHASE_ROTATION);
                break;
            case EVENT_MARGOK_ARCANE_ABERRATION:
                if (Creature* aberration = summonCreature(MargokAberrations[mRune], highmaulScatter(getCreature()->GetPosition(), 6.0f, 10.0f)))
                    setZoneWideCombat(aberration);
                scriptEvents.addEvent(EVENT_MARGOK_ARCANE_ABERRATION, 45000, HIGHMAUL_PHASE_ROTATION);
                break;
            case EVENT_MARGOK_MARK_OF_CHAOS:
                castSpellOnVictim(MargokMarkOfChaos[mRune]);
                scriptEvents.addEvent(EVENT_MARGOK_MARK_OF_CHAOS, 51000, HIGHMAUL_PHASE_ROTATION);
                break;
            case EVENT_MARGOK_FORCE_NOVA:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Imperator Mar'gok unleashes a Force Nova!");
                castSpellOnSelf(MargokForceNova[mRune]);
                scriptEvents.addEvent(EVENT_MARGOK_FORCE_NOVA, 45000, HIGHMAUL_PHASE_ROTATION);
                break;
            case EVENT_MARGOK_BERSERK:
                castSpellOnSelf(SPELL_HIGHMAUL_BERSERK, true);
                break;
            case EVENT_MARGOK_CALL_GUARDS:
                callRuneGuards();
                break;
            case EVENT_MARGOK_ANOMALY_WAVE:
                for (uint32_t i = 0; i < MargokAnomaliesPerWave; ++i)
                {
                    if (Creature* anomaly = summonCreature(NPC_VOLATILE_ANOMALY, highmaulScatter(activeRune(), 0.0f, 3.0f)))
                        setZoneWideCombat(anomaly);
                }
                scriptEvents.addEvent(EVENT_MARGOK_ANOMALY_WAVE, 15000, HIGHMAUL_PHASE_BREAK);
                break;
            case EVENT_MARGOK_RUNE_AWAKENED:
                leaveIntermission();
                break;
            default:
                break;
        }
    }

private:
    // 85% displacement, 55% and 25% start an intermission
    void checkRunes()
    {
        if (mIntermission || !_isInCombat())
            return;

        int32_t const health = _getHealthPercent();
        if (mRune == MARGOK_RUNE_CROWN && health <= 85)
            takeDisplacement();
        else if (mRune == MARGOK_RUNE_DISPLACEMENT && health <= 55)
            enterIntermission(MARGOK_RUNE_FORTIFICATION);
        else if (mRune == MARGOK_RUNE_FORTIFICATION && health <= 25)
            enterIntermission(MARGOK_RUNE_REPLICATION);
    }

    void takeDisplacement()
    {
        mRune = MARGOK_RUNE_DISPLACEMENT;
        sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Imperator Mar'gok draws power from the Rune of Displacement!");
        castSpellOnSelf(MargokPowers[mRune], true);

        for (uint32_t eventId : MargokRotationEvents)
            scriptEvents.delayEvent(eventId, 11000);
    }

    void enterIntermission(uint8_t nextRune)
    {
        mIntermission = true;
        mNextRune = nextRune;

        attackStop();
        setReactState(REACT_PASSIVE);
        setRooted(true);
        castSpellOnSelf(SPELL_MARGOK_ARCANE_PROTECTION, true);
        sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Imperator Mar'gok awakens a runestone!");

        scriptEvents.addEvent(EVENT_MARGOK_CALL_GUARDS, 10000, HIGHMAUL_PHASE_BREAK);
        scriptEvents.addEvent(EVENT_MARGOK_ANOMALY_WAVE, 12000, HIGHMAUL_PHASE_BREAK);
        scriptEvents.addEvent(EVENT_MARGOK_RUNE_AWAKENED, 75000, HIGHMAUL_PHASE_BREAK);
    }

    void leaveIntermission()
    {
        mIntermission = false;
        scriptEvents.removeEvent(EVENT_MARGOK_ANOMALY_WAVE);

        _removeAura(SPELL_MARGOK_ARCANE_PROTECTION);
        if (MargokPowers[mRune] != 0)
            _removeAura(MargokPowers[mRune]);

        mRune = mNextRune;
        castSpellOnSelf(MargokPowers[mRune], true);

        setRooted(false);
        setReactState(REACT_AGGRESSIVE);
    }

    LocationVector const& activeRune() const
    {
        return mNextRune == MARGOK_RUNE_REPLICATION ? MargokReplicationRune : MargokFortificationRune;
    }

    // warmages at the rune, the reaver joins the second intermission
    void callRuneGuards()
    {
        for (uint32_t i = 0; i < MargokWarmagesPerIntermission; ++i)
        {
            if (Creature* warmage = summonCreature(NPC_GORIAN_WARMAGE, highmaulScatter(activeRune(), 4.0f, 8.0f)))
                setZoneWideCombat(warmage);
        }

        if (mNextRune == MARGOK_RUNE_REPLICATION)
        {
            if (Creature* reaver = summonCreature(NPC_GORIAN_REAVER, MargokReaverSpot))
                setZoneWideCombat(reaver);
        }
    }

    uint8_t mRune = MARGOK_RUNE_CROWN;
    uint8_t mNextRune = MARGOK_RUNE_CROWN;
    bool mIntermission = false;
};

class HighmaulGorianWarmageAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulGorianWarmageAI(c); }
    explicit HighmaulGorianWarmageAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_WARMAGE_FIXATE, 3000);
        scriptEvents.addEvent(EVENT_WARMAGE_NETHER_BLAST, 4000);
        scriptEvents.addEvent(EVENT_WARMAGE_SLOW, 5000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_WARMAGE_FIXATE:
                if (Unit* target = highmaulPickPlayer(this, false))
                {
                    castSpell(target, SPELL_WARMAGE_FIXATE, true);
                    attackStart(target);
                }
                scriptEvents.addEvent(EVENT_WARMAGE_FIXATE, 15000);
                break;
            case EVENT_WARMAGE_NETHER_BLAST:
                castSpellOnVictim(SPELL_WARMAGE_NETHER_BLAST);
                scriptEvents.addEvent(EVENT_WARMAGE_NETHER_BLAST, 2500);
                break;
            case EVENT_WARMAGE_SLOW:
                castSpellOnSelf(SPELL_WARMAGE_SLOW, true);
                scriptEvents.addEvent(EVENT_WARMAGE_SLOW, 17000);
                break;
            default:
                break;
        }
    }
};

class HighmaulGorianReaverAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighmaulGorianReaverAI(c); }
    explicit HighmaulGorianReaverAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_REAVER_SHOCKWAVE, 12000);
        scriptEvents.addEvent(EVENT_REAVER_CRUSH_ARMOR, 22000);
        scriptEvents.addEvent(EVENT_REAVER_KICK, 41000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_REAVER_SHOCKWAVE:
                castSpellOnSelf(SPELL_REAVER_DEVASTATING_SHOCKWAVE, true);
                scriptEvents.addEvent(EVENT_REAVER_SHOCKWAVE, 12000);
                break;
            case EVENT_REAVER_CRUSH_ARMOR:
                castSpellOnVictim(SPELL_REAVER_CRUSH_ARMOR, true);
                scriptEvents.addEvent(EVENT_REAVER_CRUSH_ARMOR, 11000);
                break;
            case EVENT_REAVER_KICK:
                castSpellOnVictim(SPELL_REAVER_KICK_TO_THE_FACE, true);
                scriptEvents.addEvent(EVENT_REAVER_KICK, 25000);
                break;
            default:
                break;
        }
    }
};

void SetupHighmaul(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HIGHMAUL, &HighmaulInstanceScript::Create);

    mgr->register_creature_script(BOSS_KARGATH_BLADEFIST, &HighmaulKargathAI::Create);
    mgr->register_creature_script(NPC_IRON_BOMBER, &HighmaulIronBomberAI::Create);
    mgr->register_creature_script(NPC_DRUNKEN_BILESLINGER, &HighmaulBileslingerAI::Create);

    mgr->register_creature_script(BOSS_THE_BUTCHER, &HighmaulButcherAI::Create);

    mgr->register_creature_script(BOSS_TECTUS, &HighmaulTectusAI::Create);
    mgr->register_creature_script(NPC_SHARD_OF_TECTUS, &HighmaulTectusFragmentAI::Create);
    mgr->register_creature_script(NPC_MOTE_OF_TECTUS, &HighmaulTectusFragmentAI::Create);
    mgr->register_creature_script(NPC_EARTHEN_PILLAR_STALKER, &HighmaulEarthenPillarAI::Create);
    mgr->register_creature_script(NPC_NIGHT_TWISTED_BERSERKER, &HighmaulNightTwistedBerserkerAI::Create);
    mgr->register_creature_script(NPC_NIGHT_TWISTED_EARTHWARPER, &HighmaulNightTwistedEarthwarperAI::Create);

    mgr->register_creature_script(BOSS_BRACKENSPORE, &HighmaulBrackensporeAI::Create);
    mgr->register_creature_script(NPC_FUNGAL_FLESH_EATER, &HighmaulFungalFleshEaterAI::Create);
    mgr->register_creature_script(NPC_SPORE_SHOOTER, &HighmaulSporeShooterAI::Create);

    mgr->register_creature_script(BOSS_PHEMOS, &HighmaulPhemosAI::Create);
    mgr->register_creature_script(BOSS_POL, &HighmaulPolAI::Create);

    mgr->register_creature_script(BOSS_KORAGH, &HighmaulKoraghAI::Create);

    mgr->register_creature_script(BOSS_IMPERATOR_MARGOK, &HighmaulMargokAI::Create);
    mgr->register_creature_script(NPC_GORIAN_WARMAGE, &HighmaulGorianWarmageAI::Create);
    mgr->register_creature_script(NPC_GORIAN_REAVER, &HighmaulGorianReaverAI::Create);
}
