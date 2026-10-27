/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_The_Dawnbreaker.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

#include <cstdint>

enum TheDawnbreakerEvents
{
    EVENT_SHADOWCROWN_SHADOW_BOLT = 1,
    EVENT_SHADOWCROWN_BURNING_SHADOWS,
    EVENT_SHADOWCROWN_OBSIDIAN_BLAST,
    EVENT_SHADOWCROWN_COLLAPSING_DARKNESS,
    EVENT_SHADOWCROWN_FADE,

    EVENT_ANUBIKKAJ_TERRIFYING_SLAM,
    EVENT_ANUBIKKAJ_DARK_ORB,
    EVENT_ANUBIKKAJ_SHADOWY_DECAY,
    EVENT_ANUBIKKAJ_MIGHT_CHECK,

    EVENT_RASHANAN_ROLLING_ACID,
    EVENT_RASHANAN_EROSIVE_SPRAY,
    EVENT_RASHANAN_ARATHI_BOMBS,
    EVENT_RASHANAN_ACIDIC_ERUPTION,
    EVENT_RASHANAN_VENERATION_GROUNDS,
    EVENT_RASHANAN_SPINNERETS_STRANDS,
    EVENT_RASHANAN_TACKY_BURST
};

namespace
{
    uint32_t const AnubikkajLieutenants[] = { NPC_ASCENDANT_VISCOXRIA, NPC_IXKRETEN_THE_UNBREAKABLE, NPC_DEATHSCREAMER_IKENTAK };

    uint32_t const ShadowcrownFadeMs = 8000;
    int32_t const RashananTakeoffPct = 50;

    uint32_t dawnbreakerPercentOf(uint32_t value, uint32_t pct)
    {
        return static_cast<uint32_t>(static_cast<uint64_t>(value) * pct / 100);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class TheDawnbreakerInstanceScript : public InstanceScript
{
public:
    explicit TheDawnbreakerInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TheDawnbreakerEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheDawnbreakerInstanceScript(pMapMgr); }

    // any nerubian lieutenant of Anub'ikkaj still standing
    bool isLieutenantAlive()
    {
        for (uint32_t entry : AnubikkajLieutenants)
        {
            for (Creature* pLieutenant : getCreatureSetForEntry(entry))
            {
                if (pLieutenant->isAlive())
                    return true;
            }
        }

        return false;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock and target helpers

class TheDawnbreakerCombatAI : public CreatureAIScript
{
public:
    explicit TheDawnbreakerCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

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

    TheDawnbreakerInstanceScript* getDawnbreaker()
    {
        if (getCreature()->GetMapId() != MAP_THE_DAWNBREAKER)
            return nullptr;

        return static_cast<TheDawnbreakerInstanceScript*>(getInstanceScript());
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

    void castAt(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class TheDawnbreakerBossAI : public TheDawnbreakerCombatAI
{
public:
    TheDawnbreakerBossAI(Creature* pCreature, uint32_t dataId) : TheDawnbreakerCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (TheDawnbreakerInstanceScript* pDawnbreaker = getDawnbreaker())
        {
            pDawnbreaker->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pDawnbreaker->setBossState(mDataId, EncounterStates::InProgress);
        }

        TheDawnbreakerCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (TheDawnbreakerInstanceScript* pDawnbreaker = getDawnbreaker())
            {
                pDawnbreaker->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pDawnbreaker->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        TheDawnbreakerCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (TheDawnbreakerInstanceScript* pDawnbreaker = getDawnbreaker())
        {
            pDawnbreaker->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pDawnbreaker->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Speaker Shadowcrown, Darkness Comes at half health and at the brink of defeat

class TheDawnbreakerSpeakerShadowcrownAI : public TheDawnbreakerBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheDawnbreakerSpeakerShadowcrownAI(c); }
    explicit TheDawnbreakerSpeakerShadowcrownAI(Creature* pCreature) : TheDawnbreakerBossAI(pCreature, DATA_SPEAKER_SHADOWCROWN) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || !_isInCombat())
            return;

        Creature* pSelf = getCreature();
        uint32_t const health = pSelf->getHealth();
        uint32_t const lastPercent = dawnbreakerPercentOf(pSelf->getMaxHealth(), 1);
        uint32_t const headroom = health > lastPercent ? health - lastPercent : 0;

        // he holds on at the last percent until the final darkness has passed
        if (!mFadeAllowed && *damage >= headroom)
        {
            *damage = headroom;
            if (!mFinalCalled)
            {
                mFinalCalled = true;
                mDarknessPending = true;
            }
            return;
        }

        uint32_t const left = health > *damage ? health - *damage : 0;
        if (!mHalfCalled && left <= dawnbreakerPercentOf(pSelf->getMaxHealth(), 50))
        {
            mHalfCalled = true;
            mDarknessPending = true;
        }
    }

protected:
    void onEngage() override
    {
        mHalfCalled = false;
        mFinalCalled = false;
        mDarknessPending = false;
        mFadeAllowed = false;

        scriptEvents.addEvent(EVENT_SHADOWCROWN_SHADOW_BOLT, 2000);
        scriptEvents.addEvent(EVENT_SHADOWCROWN_BURNING_SHADOWS, 6000);
        scriptEvents.addEvent(EVENT_SHADOWCROWN_OBSIDIAN_BLAST, 9000);
        scriptEvents.addEvent(EVENT_SHADOWCROWN_COLLAPSING_DARKNESS, 14000);
    }

    void onReset() override
    {
        mDarknessPending = false;
        mFadeAllowed = false;
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mDarknessPending || _isCasting())
            return;

        mDarknessPending = false;
        warn("Speaker Shadowcrown begins to cast Darkness Comes!");
        castSpellAOE(SPELL_SHADOWCROWN_DARKNESS_COMES);

        if (mFinalCalled)
        {
            scriptEvents.resetEvents();
            scriptEvents.addEvent(EVENT_SHADOWCROWN_FADE, ShadowcrownFadeMs);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SHADOWCROWN_SHADOW_BOLT:
                castAt(anyPlayer(), SPELL_SHADOWCROWN_SHADOW_BOLT);
                scriptEvents.addEvent(EVENT_SHADOWCROWN_SHADOW_BOLT, 5000);
                break;
            case EVENT_SHADOWCROWN_BURNING_SHADOWS:
                castSpellAOE(SPELL_SHADOWCROWN_BURNING_SHADOWS);
                scriptEvents.addEvent(EVENT_SHADOWCROWN_BURNING_SHADOWS, 18000);
                break;
            case EVENT_SHADOWCROWN_OBSIDIAN_BLAST:
                castSpellOnVictim(SPELL_SHADOWCROWN_OBSIDIAN_BLAST);
                scriptEvents.addEvent(EVENT_SHADOWCROWN_OBSIDIAN_BLAST, 16000);
                break;
            case EVENT_SHADOWCROWN_COLLAPSING_DARKNESS:
                warn("Speaker Shadowcrown calls down Collapsing Darkness!");
                castSpellAOE(SPELL_SHADOWCROWN_COLLAPSING_DARKNESS);
                scriptEvents.addEvent(EVENT_SHADOWCROWN_COLLAPSING_DARKNESS, 24000);
                break;
            case EVENT_SHADOWCROWN_FADE:
                // spent after the last darkness
                mFadeAllowed = true;
                getCreature()->dealDamage(getCreature(), getCreature()->getHealth(), 0);
                break;
            default:
                break;
        }
    }

private:
    bool mHalfCalled = false;
    bool mFinalCalled = false;
    bool mDarknessPending = false;
    bool mFadeAllowed = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Anub'ikkaj, empowered while his lieutenants live

class TheDawnbreakerAnubikkajAI : public TheDawnbreakerBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheDawnbreakerAnubikkajAI(c); }
    explicit TheDawnbreakerAnubikkajAI(Creature* pCreature) : TheDawnbreakerBossAI(pCreature, DATA_ANUBIKKAJ) {}

protected:
    void onEngage() override
    {
        updateMight();

        scriptEvents.addEvent(EVENT_ANUBIKKAJ_TERRIFYING_SLAM, 8000);
        scriptEvents.addEvent(EVENT_ANUBIKKAJ_DARK_ORB, 12000);
        scriptEvents.addEvent(EVENT_ANUBIKKAJ_SHADOWY_DECAY, 25000);
        scriptEvents.addEvent(EVENT_ANUBIKKAJ_MIGHT_CHECK, 3000);
    }

    void onReset() override { _removeAura(SPELL_ANUBIKKAJ_EMPOWERED_MIGHT); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ANUBIKKAJ_TERRIFYING_SLAM:
                castSpellOnVictim(SPELL_ANUBIKKAJ_TERRIFYING_SLAM);
                scriptEvents.addEvent(EVENT_ANUBIKKAJ_TERRIFYING_SLAM, 15000);
                break;
            case EVENT_ANUBIKKAJ_DARK_ORB:
                warn("Anub'ikkaj hurls a Dark Orb!");
                castSpellAOE(SPELL_ANUBIKKAJ_DARK_ORB);
                scriptEvents.addEvent(EVENT_ANUBIKKAJ_DARK_ORB, 20000);
                break;
            case EVENT_ANUBIKKAJ_SHADOWY_DECAY:
                warn("Anub'ikkaj spreads Shadowy Decay!");
                castSpellOnSelf(SPELL_ANUBIKKAJ_SHADOWY_DECAY);
                scriptEvents.addEvent(EVENT_ANUBIKKAJ_SHADOWY_DECAY, 30000);
                break;
            case EVENT_ANUBIKKAJ_MIGHT_CHECK:
                updateMight();
                scriptEvents.addEvent(EVENT_ANUBIKKAJ_MIGHT_CHECK, 3000);
                break;
            default:
                break;
        }
    }

private:
    void updateMight()
    {
        TheDawnbreakerInstanceScript* pDawnbreaker = getDawnbreaker();
        bool const empowered = pDawnbreaker != nullptr && pDawnbreaker->isLieutenantAlive();
        bool const hasMight = hasAura(SPELL_ANUBIKKAJ_EMPOWERED_MIGHT);

        if (empowered && !hasMight)
            castSpellOnSelf(SPELL_ANUBIKKAJ_EMPOWERED_MIGHT, true);
        else if (!empowered && hasMight)
            _removeAura(SPELL_ANUBIKKAJ_EMPOWERED_MIGHT);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rasha'nan of the Dawnbreaker, the flight across Hallowfall is fought in place

class TheDawnbreakerRashananAI : public TheDawnbreakerBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheDawnbreakerRashananAI(c); }
    explicit TheDawnbreakerRashananAI(Creature* pCreature) : TheDawnbreakerBossAI(pCreature, DATA_DAWNBREAKER_RASHANAN) {}

protected:
    void onEngage() override
    {
        mTookFlight = false;

        scriptEvents.addEvent(EVENT_RASHANAN_ROLLING_ACID, 8000);
        scriptEvents.addEvent(EVENT_RASHANAN_EROSIVE_SPRAY, 14000);
        scriptEvents.addEvent(EVENT_RASHANAN_ARATHI_BOMBS, 20000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mTookFlight || _getHealthPercent() > RashananTakeoffPct || _isCasting())
            return;

        // intermission
        mTookFlight = true;
        scriptEvents.resetEvents();
        warn("Rasha'nan takes flight across Hallowfall!");
        scriptEvents.addEvent(EVENT_RASHANAN_ACIDIC_ERUPTION, 1500);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RASHANAN_ROLLING_ACID:
                castAt(anyPlayer(), SPELL_RASHANAN_ROLLING_ACID);
                scriptEvents.addEvent(EVENT_RASHANAN_ROLLING_ACID, 18000);
                break;
            case EVENT_RASHANAN_EROSIVE_SPRAY:
                castSpellAOE(SPELL_RASHANAN_EROSIVE_SPRAY);
                scriptEvents.addEvent(EVENT_RASHANAN_EROSIVE_SPRAY, 24000);
                break;
            case EVENT_RASHANAN_ARATHI_BOMBS:
                warn("Nightfall Bombers hurl Arathi Bombs aboard the Dawnbreaker!");
                castAt(anyPlayer(), SPELL_RASHANAN_ARATHI_BOMBS);
                scriptEvents.addEvent(EVENT_RASHANAN_ARATHI_BOMBS, 30000);
                break;
            case EVENT_RASHANAN_ACIDIC_ERUPTION:
                warn("Rasha'nan begins to cast Acidic Eruption! Interrupt her!");
                castSpellAOE(SPELL_RASHANAN_ACIDIC_ERUPTION);
                scriptEvents.addEvent(EVENT_RASHANAN_VENERATION_GROUNDS, 6000);
                break;
            case EVENT_RASHANAN_VENERATION_GROUNDS:
                // stage two
                scriptEvents.addEvent(EVENT_RASHANAN_SPINNERETS_STRANDS, 5000);
                scriptEvents.addEvent(EVENT_RASHANAN_ROLLING_ACID, 10000);
                scriptEvents.addEvent(EVENT_RASHANAN_EROSIVE_SPRAY, 15000);
                scriptEvents.addEvent(EVENT_RASHANAN_TACKY_BURST, 20000);
                break;
            case EVENT_RASHANAN_SPINNERETS_STRANDS:
                castSpellOnSelf(SPELL_RASHANAN_SPINNERETS_STRANDS);
                scriptEvents.addEvent(EVENT_RASHANAN_SPINNERETS_STRANDS, 24000);
                break;
            case EVENT_RASHANAN_TACKY_BURST:
                castSpellAOE(SPELL_RASHANAN_TACKY_BURST);
                scriptEvents.addEvent(EVENT_RASHANAN_TACKY_BURST, 22000);
                break;
            default:
                break;
        }
    }

private:
    bool mTookFlight = false;
};

void SetupTheDawnbreaker(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_DAWNBREAKER, &TheDawnbreakerInstanceScript::Create);

    mgr->register_creature_script(BOSS_SPEAKER_SHADOWCROWN, &TheDawnbreakerSpeakerShadowcrownAI::Create);
    mgr->register_creature_script(BOSS_ANUBIKKAJ, &TheDawnbreakerAnubikkajAI::Create);

    mgr->register_creature_script(BOSS_DAWNBREAKER_RASHANAN, &TheDawnbreakerRashananAI::Create);
}
