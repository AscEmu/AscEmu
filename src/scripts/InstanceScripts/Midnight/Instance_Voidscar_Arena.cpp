/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Voidscar_Arena.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

enum VoidscarArenaEvents
{
    EVENT_TAZRAH_VOID_BLAST = 1,
    EVENT_TAZRAH_ETHEREAL_SHADES,
    EVENT_TAZRAH_DARK_BLOOM,
    EVENT_TAZRAH_VOID_FISSURES,
    EVENT_TAZRAH_UMBRAL_RUPTURE,
    EVENT_SHADE_NETHER_DASH,

    EVENT_ATROXUS_HULKING_CLAW,
    EVENT_ATROXUS_NOXIOUS_BREATH,
    EVENT_ATROXUS_POISON_SPLASH,
    EVENT_ATROXUS_MONSTROUS_ROAR,
    EVENT_ATROXUS_CALL_CREEPER,
    EVENT_CREEPER_SICKENING_BITE,

    EVENT_CHARONUS_UNSTABLE_SINGULARITY,
    EVENT_CHARONUS_GRAVITIC_ORBS,
    EVENT_CHARONUS_DARK_WAVES,
    EVENT_CHARONUS_COSMIC_CRASH
};

namespace
{
    float const VoidscarTwoPi = 6.28318531f;
    float const VoidscarFixateThreat = 1000000.0f;

    uint32_t const ShadesPerWave = 2;
    size_t const VoidFissureTargets = 2;

    uint32_t const OrbsPerWave = 2;
    uint32_t const OrbLifetimeMs = 23000;
    uint32_t const OrbCheckMs = 500;
    float const OrbContactReach = 3.0f;
    float const SingularityCollapseReach = 10.0f;

    LocationVector voidscarRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(VoidscarTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class VoidscarArenaInstanceScript : public InstanceScript
{
public:
    explicit VoidscarArenaInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(VoidscarArenaEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new VoidscarArenaInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class VoidscarArenaCombatAI : public CreatureAIScript
{
public:
    explicit VoidscarArenaCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
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
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    // distinct random players, the tank only when nobody else is left
    std::vector<Unit*> pickPlayers(size_t count, bool skipTank = false)
    {
        TargetFilter const filter = skipTank ? TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent) : TargetFilter_Player;
        FilterArgs const args(filter);

        std::vector<Unit*> pool;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject != nullptr && isValidUnitTarget(pObject, filter, args))
                pool.push_back(static_cast<Unit*>(pObject));
        }

        if (pool.empty() && skipTank)
            return pickPlayers(count, false);

        for (size_t i = 0; i < pool.size() && i < count; ++i)
        {
            size_t const pick = i + Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1 - i));
            std::swap(pool[i], pool[pick]);
        }

        if (pool.size() > count)
            pool.resize(count);

        return pool;
    }

    Unit* anyPlayer(bool skipTank = false)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipTank);
        return picked.empty() ? nullptr : picked.front();
    }

    void castAt(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    // ground targeted spell at the feet of a unit
    void castAtGround(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            getCreature()->castSpellLoc(pTarget->GetPosition(), spellId, triggered);
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t durationMs = 0)
    {
        Creature* pAdd = summonCreature(entry, spot, despawnType, durationMs);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange)
    {
        return voidscarRingPoint(getCreature()->GetPosition(), minRange, maxRange);
    }

    // the add chases one player until it dies
    void fixateRandomPlayer()
    {
        if (Unit* pTarget = anyPlayer())
            addThreat(pTarget, VoidscarFixateThreat);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class VoidscarArenaBossAI : public VoidscarArenaCombatAI
{
public:
    VoidscarArenaBossAI(Creature* pCreature, uint32_t dataId) : VoidscarArenaCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        VoidscarArenaCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        VoidscarArenaCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::Performed);
        }
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Taz'Rah

class VoidscarArenaTazrahAI : public VoidscarArenaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidscarArenaTazrahAI(c); }
    explicit VoidscarArenaTazrahAI(Creature* pCreature) : VoidscarArenaBossAI(pCreature, DATA_TAZRAH) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_TAZRAH_VOID_BLAST, 5000);
        scriptEvents.addEvent(EVENT_TAZRAH_ETHEREAL_SHADES, 8000);
        scriptEvents.addEvent(EVENT_TAZRAH_DARK_BLOOM, 16000);
        scriptEvents.addEvent(EVENT_TAZRAH_UMBRAL_RUPTURE, 22000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TAZRAH_VOID_BLAST:
                castSpellOnVictim(SPELL_TAZRAH_VOID_BLAST);
                scriptEvents.addEvent(EVENT_TAZRAH_VOID_BLAST, 10000);
                break;
            case EVENT_TAZRAH_ETHEREAL_SHADES:
                for (uint32_t i = 0; i < ShadesPerWave; ++i)
                    callAdd(NPC_ETHEREAL_SHADE, aroundSelf(6.0f, 10.0f));
                scriptEvents.addEvent(EVENT_TAZRAH_ETHEREAL_SHADES, 35000);
                break;
            case EVENT_TAZRAH_DARK_BLOOM:
                warn("Taz'Rah's Dark Bloom tears the arena open!");
                castSpellAOE(SPELL_TAZRAH_DARK_BLOOM);
                scriptEvents.addEvent(EVENT_TAZRAH_VOID_FISSURES, 500);
                scriptEvents.addEvent(EVENT_TAZRAH_DARK_BLOOM, 30000);
                break;
            case EVENT_TAZRAH_VOID_FISSURES:
                // the bloom makes fissures erupt below some players
                for (Unit* pTarget : pickPlayers(VoidFissureTargets))
                    castSpell(pTarget, SPELL_TAZRAH_VOID_FISSURE, true);
                break;
            case EVENT_TAZRAH_UMBRAL_RUPTURE:
                castSpellAOE(SPELL_TAZRAH_UMBRAL_RUPTURE);
                scriptEvents.addEvent(EVENT_TAZRAH_UMBRAL_RUPTURE, 26000);
                break;
            default:
                break;
        }
    }
};

class VoidscarArenaEtherealShadeAI : public VoidscarArenaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidscarArenaEtherealShadeAI(c); }
    explicit VoidscarArenaEtherealShadeAI(Creature* pCreature) : VoidscarArenaCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_SHADE_NETHER_DASH, 3000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SHADE_NETHER_DASH)
            return;

        castAt(anyPlayer(), SPELL_SHADE_NETHER_DASH);
        scriptEvents.addEvent(EVENT_SHADE_NETHER_DASH, 12000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Atroxus

class VoidscarArenaAtroxusAI : public VoidscarArenaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidscarArenaAtroxusAI(c); }
    explicit VoidscarArenaAtroxusAI(Creature* pCreature) : VoidscarArenaBossAI(pCreature, DATA_ATROXUS) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_ATROXUS_HULKING_CLAW, 4000);
        scriptEvents.addEvent(EVENT_ATROXUS_NOXIOUS_BREATH, 9000);
        scriptEvents.addEvent(EVENT_ATROXUS_POISON_SPLASH, 12000);
        scriptEvents.addEvent(EVENT_ATROXUS_MONSTROUS_ROAR, 24000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ATROXUS_HULKING_CLAW:
                castSpellOnVictim(SPELL_ATROXUS_HULKING_CLAW);
                scriptEvents.addEvent(EVENT_ATROXUS_HULKING_CLAW, 9000);
                break;
            case EVENT_ATROXUS_NOXIOUS_BREATH:
                castSpellOnVictim(SPELL_ATROXUS_NOXIOUS_BREATH);
                scriptEvents.addEvent(EVENT_ATROXUS_NOXIOUS_BREATH, 16000);
                break;
            case EVENT_ATROXUS_POISON_SPLASH:
                castAtGround(anyPlayer(), SPELL_ATROXUS_POISON_SPLASH);
                scriptEvents.addEvent(EVENT_ATROXUS_POISON_SPLASH, 18000);
                break;
            case EVENT_ATROXUS_MONSTROUS_ROAR:
                warn("Atroxus lets out a Monstrous Roar!");
                castSpellAOE(SPELL_ATROXUS_MONSTROUS_ROAR);
                scriptEvents.addEvent(EVENT_ATROXUS_CALL_CREEPER, 500);
                scriptEvents.addEvent(EVENT_ATROXUS_MONSTROUS_ROAR, 40000);
                break;
            case EVENT_ATROXUS_CALL_CREEPER:
                callAdd(NPC_TOXIC_CREEPER, aroundSelf(5.0f, 9.0f));
                break;
            default:
                break;
        }
    }
};

class VoidscarArenaToxicCreeperAI : public VoidscarArenaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidscarArenaToxicCreeperAI(c); }
    explicit VoidscarArenaToxicCreeperAI(Creature* pCreature) : VoidscarArenaCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        fixateRandomPlayer();
        scriptEvents.addEvent(EVENT_CREEPER_SICKENING_BITE, 2000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_CREEPER_SICKENING_BITE)
            return;

        castSpellOnVictim(SPELL_CREEPER_SICKENING_BITE);
        scriptEvents.addEvent(EVENT_CREEPER_SICKENING_BITE, 6000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Charonus

class VoidscarArenaCharonusAI : public VoidscarArenaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidscarArenaCharonusAI(c); }
    explicit VoidscarArenaCharonusAI(Creature* pCreature) : VoidscarArenaBossAI(pCreature, DATA_CHARONUS) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_CHARONUS_UNSTABLE_SINGULARITY, 8000);
        scriptEvents.addEvent(EVENT_CHARONUS_DARK_WAVES, 12000);
        scriptEvents.addEvent(EVENT_CHARONUS_GRAVITIC_ORBS, 15000);
        scriptEvents.addEvent(EVENT_CHARONUS_COSMIC_CRASH, 26000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CHARONUS_UNSTABLE_SINGULARITY:
                if (Unit* pTarget = anyPlayer())
                {
                    castAtGround(pTarget, SPELL_CHARONUS_UNSTABLE_SINGULARITY);
                    collapseOrbsNear(pTarget->GetPosition());
                }
                scriptEvents.addEvent(EVENT_CHARONUS_UNSTABLE_SINGULARITY, 20000);
                break;
            case EVENT_CHARONUS_DARK_WAVES:
                castSpellOnVictim(SPELL_CHARONUS_DARK_WAVES);
                scriptEvents.addEvent(EVENT_CHARONUS_DARK_WAVES, 22000);
                break;
            case EVENT_CHARONUS_GRAVITIC_ORBS:
                warn("Charonus creates Gravitic Orbs!");
                for (uint32_t i = 0; i < OrbsPerWave; ++i)
                    callAdd(NPC_GRAVITIC_ORB, aroundSelf(5.0f, 8.0f), TIMED_DESPAWN, OrbLifetimeMs);
                scriptEvents.addEvent(EVENT_CHARONUS_GRAVITIC_ORBS, 30000);
                break;
            case EVENT_CHARONUS_COSMIC_CRASH:
                castSpellAOE(SPELL_CHARONUS_COSMIC_CRASH);
                scriptEvents.addEvent(EVENT_CHARONUS_COSMIC_CRASH, 34000);
                break;
            default:
                break;
        }
    }

private:
    // orbs caught by the singularity collapse
    void collapseOrbsNear(LocationVector const& spot)
    {
        float const reachSq = SingularityCollapseReach * SingularityCollapseReach;
        std::vector<Creature*> caught;
        for (uint64_t const guid : summons)
        {
            Creature* pOrb = getCreature()->getWorldMapCreature(guid);
            if (pOrb != nullptr && pOrb->getEntry() == NPC_GRAVITIC_ORB && pOrb->isAlive() && pOrb->getDistanceSq(spot) <= reachSq)
                caught.push_back(pOrb);
        }

        for (Creature* pOrb : caught)
        {
            summons.despawn(pOrb);
            pOrb->despawn(500, 0);
        }
    }
};

// seeks players and bursts on the first one it touches
class VoidscarArenaGraviticOrbAI : public VoidscarArenaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidscarArenaGraviticOrbAI(c); }
    explicit VoidscarArenaGraviticOrbAI(Creature* pCreature) : VoidscarArenaCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        mCheckClock = 0;
        mBurst = false;
        fixateRandomPlayer();
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mBurst)
            return;

        mCheckClock += diff;
        if (mCheckClock < OrbCheckMs)
            return;

        mCheckClock = 0;
        Unit* pTouched = playerInContact();
        if (pTouched == nullptr)
            return;

        mBurst = true;
        castSpell(pTouched, SPELL_ORB_CONDENSED_MASS, true);
        despawn(1000, 0);
    }

private:
    Unit* playerInContact()
    {
        FilterArgs const args(TargetFilter_Player);
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject != nullptr && isValidUnitTarget(pObject, TargetFilter_Player, args) && getCreature()->getDistance(pObject) <= OrbContactReach)
                return static_cast<Unit*>(pObject);
        }

        return nullptr;
    }

    uint32_t mCheckClock = 0;
    bool mBurst = false;
};

void SetupVoidscarArena(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_VOIDSCAR_ARENA, &VoidscarArenaInstanceScript::Create);

    mgr->register_creature_script(BOSS_TAZRAH, &VoidscarArenaTazrahAI::Create);
    mgr->register_creature_script(NPC_ETHEREAL_SHADE, &VoidscarArenaEtherealShadeAI::Create);

    mgr->register_creature_script(BOSS_ATROXUS, &VoidscarArenaAtroxusAI::Create);
    mgr->register_creature_script(BOSS_ATROXUS_ALT, &VoidscarArenaAtroxusAI::Create);
    mgr->register_creature_script(NPC_TOXIC_CREEPER, &VoidscarArenaToxicCreeperAI::Create);

    mgr->register_creature_script(BOSS_CHARONUS, &VoidscarArenaCharonusAI::Create);
    mgr->register_creature_script(BOSS_CHARONUS_ALT, &VoidscarArenaCharonusAI::Create);
    mgr->register_creature_script(NPC_GRAVITIC_ORB, &VoidscarArenaGraviticOrbAI::Create);
}
