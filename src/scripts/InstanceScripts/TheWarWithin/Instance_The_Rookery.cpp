/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_The_Rookery.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <vector>

enum TheRookeryEvents
{
    EVENT_KYRIOSS_WILD_LIGHTNING = 1,
    EVENT_KYRIOSS_CRASHING_THUNDER,
    EVENT_KYRIOSS_LIGHTNING_DASH,
    EVENT_KYRIOSS_TORRENT_BEGIN,
    EVENT_KYRIOSS_TORRENT_PULSE,
    EVENT_KYRIOSS_TORRENT_END,

    EVENT_MONSTROSITY_OBLIVION_WAVE,
    EVENT_MONSTROSITY_UNLEASH_CORRUPTION,
    EVENT_MONSTROSITY_NULL_UPHEAVAL,
    EVENT_MONSTROSITY_REACH_CHECK,
    EVENT_MONSTROSITY_SHELL_CHECK,
    EVENT_MONSTROSITY_SHELL_REFORM,

    EVENT_FRAGMENT_AWAKEN,
    EVENT_AWAKENED_CORRUPTION_OVERLOAD,

    EVENT_GORREN_CHAOTIC_CORRUPTION,
    EVENT_GORREN_CRUSH_REALITY,
    EVENT_GORREN_DARK_GRAVITY
};

namespace
{
    float const RookeryTwoPi = 6.28318531f;

    // middle of Kyrioss' platform between the torrent walks
    LocationVector const KyriossPlatformCenter(2799.8f, -1614.1f, 330.4f);
    uint32_t const KyriossEnergyPerSecond = 2;
    uint32_t const KyriossTorrentPulses = 4;

    float const MonstrosityMeleeReach = 8.0f;
    uint32_t const FragmentReshapeMs = 15000;
    float const VokmarSearchRange = 150.0f;

    // living players within range of a creature
    std::vector<Unit*> rookeryPlayersNear(Creature* pCreature, float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : pCreature->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pCreature->getDistance(pPlayer) <= range)
                players.push_back(pPlayer);
        }

        return players;
    }

    LocationVector rookeryRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(RookeryTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class TheRookeryInstanceScript : public InstanceScript
{
public:
    explicit TheRookeryInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TheRookeryEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheRookeryInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy and target helpers

class TheRookeryCombatAI : public CreatureAIScript
{
public:
    explicit TheRookeryCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();

        if (isAlive())
            onReset();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        onCombatTick(diff);

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onReset() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    InstanceScript* rookeryScript()
    {
        if (getCreature()->GetMapId() != MAP_THE_ROOKERY)
            return nullptr;

        return getInstanceScript();
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // anyone but the tank, the tank when alone
    Unit* offTankPlayer()
    {
        FilterArgs args(TargetFilter_Player);
        args.addFilter(TargetFilter_NotCurrent);
        if (Unit* pTarget = selectUnitTarget(args))
            return pTarget;

        return anyPlayer();
    }

    bool anyPlayerInReach(float range) { return !rookeryPlayersNear(getCreature(), range).empty(); }

    void castAt(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return rookeryRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // true once the bar is full
    bool fillEnergy(uint32_t diff, uint32_t perSecond)
    {
        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy += perSecond;
        }

        if (mEnergy > 100)
            mEnergy = 100;

        return mEnergy >= 100;
    }

    void emptyEnergy()
    {
        mEnergy = 0;
        mEnergyClock = 0;
    }

private:
    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class TheRookeryBossAI : public TheRookeryCombatAI
{
public:
    TheRookeryBossAI(Creature* pCreature, uint32_t dataId) : TheRookeryCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = rookeryScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        TheRookeryCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = rookeryScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        TheRookeryCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = rookeryScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kyrioss, flies to the platform center at full energy

class TheRookeryKyriossAI : public TheRookeryBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheRookeryKyriossAI(c); }
    explicit TheRookeryKyriossAI(Creature* pCreature) : TheRookeryBossAI(pCreature, DATA_KYRIOSS) {}

protected:
    void onEngage() override
    {
        mInTorrent = false;
        mPulsesLeft = 0;
        scheduleGroundPhase();
    }

    void onReset() override
    {
        mInTorrent = false;
        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mInTorrent || !fillEnergy(diff, KyriossEnergyPerSecond) || _isCasting())
            return;

        mInTorrent = true;
        scriptEvents.resetEvents();
        warn("Kyrioss surges to the center of the chamber!");
        moveJump(KyriossPlatformCenter, 25.0f, 8.0f);
        scriptEvents.addEvent(EVENT_KYRIOSS_TORRENT_BEGIN, 2500);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KYRIOSS_WILD_LIGHTNING:
                castAt(anyPlayer(), SPELL_KYRIOSS_WILD_LIGHTNING);
                if (isHeroic())
                    castAt(offTankPlayer(), SPELL_KYRIOSS_WILD_LIGHTNING, true);
                scriptEvents.addEvent(EVENT_KYRIOSS_WILD_LIGHTNING, 9000);
                break;
            case EVENT_KYRIOSS_CRASHING_THUNDER:
                castSpellAOE(SPELL_KYRIOSS_CRASHING_THUNDER);
                scriptEvents.addEvent(EVENT_KYRIOSS_CRASHING_THUNDER, 20000);
                break;
            case EVENT_KYRIOSS_LIGHTNING_DASH:
                castAt(offTankPlayer(), SPELL_KYRIOSS_LIGHTNING_DASH);
                scriptEvents.addEvent(EVENT_KYRIOSS_LIGHTNING_DASH, 24000);
                break;
            case EVENT_KYRIOSS_TORRENT_BEGIN:
                setRooted(true);
                _setMeleeDisabled(true);
                warn("Kyrioss unleashes a Lightning Torrent!");
                mPulsesLeft = KyriossTorrentPulses;
                scriptEvents.addEvent(EVENT_KYRIOSS_TORRENT_PULSE, 500);
                break;
            case EVENT_KYRIOSS_TORRENT_PULSE:
                castAt(anyPlayer(), SPELL_KYRIOSS_LIGHTNING_TORRENT, true);
                if (mPulsesLeft > 0 && --mPulsesLeft > 0)
                    scriptEvents.addEvent(EVENT_KYRIOSS_TORRENT_PULSE, 2500);
                else
                    scriptEvents.addEvent(EVENT_KYRIOSS_TORRENT_END, 2000);
                break;
            case EVENT_KYRIOSS_TORRENT_END:
                // back into the fight
                setRooted(false);
                _setMeleeDisabled(false);
                mInTorrent = false;
                emptyEnergy();
                scheduleGroundPhase();
                break;
            default:
                break;
        }
    }

private:
    void scheduleGroundPhase()
    {
        scriptEvents.addEvent(EVENT_KYRIOSS_WILD_LIGHTNING, 5000);
        scriptEvents.addEvent(EVENT_KYRIOSS_CRASHING_THUNDER, 12000);
        scriptEvents.addEvent(EVENT_KYRIOSS_LIGHTNING_DASH, 18000);
    }

    bool mInTorrent = false;
    uint32_t mPulsesLeft = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Voidstone Monstrosity, Stormrider Vokmar strikes whenever the Void Shell breaks

class TheRookeryVoidstoneMonstrosityAI : public TheRookeryBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheRookeryVoidstoneMonstrosityAI(c); }
    explicit TheRookeryVoidstoneMonstrosityAI(Creature* pCreature) : TheRookeryBossAI(pCreature, DATA_VOIDSTONE_MONSTROSITY) {}

protected:
    void onEngage() override
    {
        raiseShell();

        scriptEvents.addEvent(EVENT_MONSTROSITY_UNLEASH_CORRUPTION, 6000);
        scriptEvents.addEvent(EVENT_MONSTROSITY_OBLIVION_WAVE, 10000);
        scriptEvents.addEvent(EVENT_MONSTROSITY_NULL_UPHEAVAL, 15000);
        scriptEvents.addEvent(EVENT_MONSTROSITY_REACH_CHECK, 3000);
        scriptEvents.addEvent(EVENT_MONSTROSITY_SHELL_CHECK, 1000);
    }

    void onReset() override
    {
        mShellUp = false;
        _removeAura(SPELL_MONSTROSITY_VOID_SHELL);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MONSTROSITY_UNLEASH_CORRUPTION:
                castSpellAOE(SPELL_MONSTROSITY_UNLEASH_CORRUPTION);
                scriptEvents.addEvent(EVENT_MONSTROSITY_UNLEASH_CORRUPTION, 20000);
                break;
            case EVENT_MONSTROSITY_OBLIVION_WAVE:
                warn("The Voidstone Monstrosity sends out an Oblivion Wave!");
                castSpellOnVictim(SPELL_MONSTROSITY_OBLIVION_WAVE);
                scriptEvents.addEvent(EVENT_MONSTROSITY_OBLIVION_WAVE, 18000);
                break;
            case EVENT_MONSTROSITY_NULL_UPHEAVAL:
            {
                castSpellAOE(SPELL_MONSTROSITY_NULL_UPHEAVAL);
                uint32_t const count = isHeroic() ? 3 : 2;
                for (uint32_t i = 0; i < count; ++i)
                    callAdd(NPC_VOIDSTONE_FRAGMENT, aroundSelf(8.0f, 16.0f));
                scriptEvents.addEvent(EVENT_MONSTROSITY_NULL_UPHEAVAL, 32000);
                break;
            }
            case EVENT_MONSTROSITY_REACH_CHECK:
                // nobody in melee: everyone is irradiated
                if (!anyPlayerInReach(MonstrosityMeleeReach))
                    castSpellAOE(SPELL_MONSTROSITY_ENTROPY);
                scriptEvents.addEvent(EVENT_MONSTROSITY_REACH_CHECK, 3000);
                break;
            case EVENT_MONSTROSITY_SHELL_CHECK:
                if (mShellUp && !hasAura(SPELL_MONSTROSITY_VOID_SHELL))
                    shellShattered();
                scriptEvents.addEvent(EVENT_MONSTROSITY_SHELL_CHECK, 1000);
                break;
            case EVENT_MONSTROSITY_SHELL_REFORM:
                raiseShell();
                break;
            default:
                break;
        }
    }

private:
    void raiseShell()
    {
        castSpellOnSelf(SPELL_MONSTROSITY_VOID_SHELL, true);
        mShellUp = true;
    }

    void shellShattered()
    {
        mShellUp = false;
        warn("The Void Shell shatters! Stormrider Vokmar strikes with Storm's Vengeance!");

        Creature* pVokmar = findNearestCreature(NPC_STORMRIDER_VOKMAR, VokmarSearchRange);
        if (pVokmar != nullptr && pVokmar->isAlive())
            pVokmar->castSpell(getCreature(), SPELL_VOKMAR_STORMS_VENGEANCE, true);

        scriptEvents.addEvent(EVENT_MONSTROSITY_SHELL_REFORM, 20000);
    }

    bool mShellUp = false;
};

// reshapes into a Voidstone Awakened unless destroyed in time
class TheRookeryVoidstoneFragmentAI : public TheRookeryCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheRookeryVoidstoneFragmentAI(c); }
    explicit TheRookeryVoidstoneFragmentAI(Creature* pCreature) : TheRookeryCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_FRAGMENT_RESHAPE, true);
        scriptEvents.addEvent(EVENT_FRAGMENT_AWAKEN, FragmentReshapeMs);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_FRAGMENT_AWAKEN)
            return;

        // the Monstrosity owns the new add so a wipe clears it
        Creature* pMonstrosity = findNearestCreature(BOSS_VOIDSTONE_MONSTROSITY, 150.0f);
        if (pMonstrosity != nullptr && pMonstrosity->isAlive() && pMonstrosity->GetScript() != nullptr)
        {
            Creature* pAwakened = pMonstrosity->GetScript()->summonCreature(NPC_VOIDSTONE_AWAKENED, getCreature()->GetPosition(), CORPSE_DESPAWN);
            if (pAwakened != nullptr)
                pMonstrosity->GetScript()->setZoneWideCombat(pAwakened);
        }

        despawn(100, 0);
    }
};

class TheRookeryVoidstoneAwakenedAI : public TheRookeryCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheRookeryVoidstoneAwakenedAI(c); }
    explicit TheRookeryVoidstoneAwakenedAI(Creature* pCreature) : TheRookeryCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_AWAKENED_CORRUPTION_OVERLOAD, 5000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_AWAKENED_CORRUPTION_OVERLOAD)
            return;

        castSpellAOE(SPELL_AWAKENED_CORRUPTION_OVERLOAD);
        scriptEvents.addEvent(EVENT_AWAKENED_CORRUPTION_OVERLOAD, 8000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Stormguard Gorren

class TheRookeryStormguardGorrenAI : public TheRookeryBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheRookeryStormguardGorrenAI(c); }
    explicit TheRookeryStormguardGorrenAI(Creature* pCreature) : TheRookeryBossAI(pCreature, DATA_STORMGUARD_GORREN) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GORREN_CHAOTIC_CORRUPTION, 7000);
        scriptEvents.addEvent(EVENT_GORREN_CRUSH_REALITY, 13000);
        scriptEvents.addEvent(EVENT_GORREN_DARK_GRAVITY, 28000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GORREN_CHAOTIC_CORRUPTION:
                castAt(offTankPlayer(), SPELL_GORREN_CHAOTIC_CORRUPTION);
                scriptEvents.addEvent(EVENT_GORREN_CHAOTIC_CORRUPTION, 22000);
                break;
            case EVENT_GORREN_CRUSH_REALITY:
                castAt(anyPlayer(), SPELL_GORREN_CRUSH_REALITY);
                scriptEvents.addEvent(EVENT_GORREN_CRUSH_REALITY, 20000);
                break;
            case EVENT_GORREN_DARK_GRAVITY:
                warn("Stormguard Gorren pulls everything in with Dark Gravity!");
                castSpellAOE(SPELL_GORREN_DARK_GRAVITY);
                scriptEvents.addEvent(EVENT_GORREN_DARK_GRAVITY, 34000);
                break;
            default:
                break;
        }
    }
};

void SetupTheRookery(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_ROOKERY, &TheRookeryInstanceScript::Create);

    mgr->register_creature_script(BOSS_KYRIOSS, &TheRookeryKyriossAI::Create);

    mgr->register_creature_script(BOSS_VOIDSTONE_MONSTROSITY, &TheRookeryVoidstoneMonstrosityAI::Create);
    mgr->register_creature_script(NPC_VOIDSTONE_FRAGMENT, &TheRookeryVoidstoneFragmentAI::Create);
    mgr->register_creature_script(NPC_VOIDSTONE_AWAKENED, &TheRookeryVoidstoneAwakenedAI::Create);

    mgr->register_creature_script(BOSS_STORMGUARD_GORREN, &TheRookeryStormguardGorrenAI::Create);
    mgr->register_creature_script(BOSS_STORMGUARD_GORREN_ALT, &TheRookeryStormguardGorrenAI::Create);
}
