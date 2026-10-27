/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Nexus_Point_Xenas.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <set>
#include <utility>
#include <vector>

enum NexusPointXenasEvents
{
    EVENT_KASRETH_ARCANE_ZAP = 1,
    EVENT_KASRETH_REFLUX_CHARGE,
    EVENT_KASRETH_FLUX_COLLAPSE,
    EVENT_KASRETH_FULL_ENERGY,

    EVENT_NYSARRA_UMBRAL_LASH,
    EVENT_NYSARRA_ECLIPSING_STEP,
    EVENT_NYSARRA_LIGHTSCAR_FLARE,
    EVENT_NYSARRA_NULL_VANGUARD,
    EVENT_NULLIFIER_NULLIFY,
    EVENT_NULLIFIER_DUSK_FRIGHTS,

    EVENT_LOTHRAXION_SEARING_REND,
    EVENT_LOTHRAXION_BRILLIANT_DISPERSION,
    EVENT_LOTHRAXION_FULL_POWER,
    EVENT_IMAGE_MIRRORED_REND,
    EVENT_IMAGE_FLICKER
};

namespace
{
    float const NexusTwoPi = 6.28318531f;

    // room doors shut during the fight, passages open after the kill
    struct NexusDoor
    {
        uint32_t entry;
        uint32_t dataId;
        bool passage;
    };

    NexusDoor const NexusDoors[] =
    {
        { GO_NEXUS_POINT_BOSS_ONE_DOOR, DATA_KASRETH, false },
        { GO_NEXUS_POINT_BOSS_TWO_DOOR, DATA_NYSARRA, false }
    };

    NexusDoor const* findNexusDoor(uint32_t entry)
    {
        for (NexusDoor const& door : NexusDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    uint32_t const NullifiersPerCall = 2;
    uint32_t const LightscarImageLifetimeMs = 20000;

    size_t const DispersionTargets = 2;
    uint32_t const LothraxionImageCap = 6;
    float const MirroredRendReach = 6.0f;

    LocationVector nexusRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(NexusTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class NexusPointXenasInstanceScript : public InstanceScript
{
public:
    explicit NexusPointXenasInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(NexusPointXenasEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new NexusPointXenasInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findNexusDoor(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.insert(pGameObject->GetNewGUID());
        applyDoorState(pGameObject);
    }

    void OnEncounterStateChange(uint32_t /*id*/, uint32_t /*state*/) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                applyDoorState(pDoor);
        }
    }

private:
    // the encounter decides, the spawn state of a door is not reliable
    void applyDoorState(GameObject* pDoor)
    {
        NexusDoor const* pRule = findNexusDoor(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        EncounterStates const state = getBossState(pRule->dataId);
        bool const open = pRule->passage ? state == EncounterStates::Performed : state != EncounterStates::InProgress;
        uint8_t const wanted = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    std::set<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class NexusPointXenasCombatAI : public CreatureAIScript
{
public:
    explicit NexusPointXenasCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
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

    Unit* nearestPlayerWithin(float range)
    {
        FilterArgs const args(TargetFilter_Player);
        Unit* pNearest = nullptr;
        float nearestRange = range;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr || !isValidUnitTarget(pObject, TargetFilter_Player, args))
                continue;

            float const distance = getCreature()->getDistance(pObject);
            if (distance <= nearestRange)
            {
                nearestRange = distance;
                pNearest = static_cast<Unit*>(pObject);
            }
        }

        return pNearest;
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

    uint32_t countLivingSummons(uint32_t entry)
    {
        uint32_t count = 0;
        for (uint64_t const guid : summons)
        {
            Creature* pSummon = getCreature()->getWorldMapCreature(guid);
            if (pSummon != nullptr && pSummon->getEntry() == entry && pSummon->isAlive())
                ++count;
        }

        return count;
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class NexusPointXenasBossAI : public NexusPointXenasCombatAI
{
public:
    NexusPointXenasBossAI(Creature* pCreature, uint32_t dataId) : NexusPointXenasCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        NexusPointXenasCombatAI::OnCombatStart(pTarget);
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

        NexusPointXenasCombatAI::OnCombatStop(pTarget);
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
// Chief Corewright Kasreth

class NexusPointXenasKasrethAI : public NexusPointXenasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NexusPointXenasKasrethAI(c); }
    explicit NexusPointXenasKasrethAI(Creature* pCreature) : NexusPointXenasBossAI(pCreature, DATA_KASRETH) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_KASRETH_ARCANE_ZAP, 3000);
        scriptEvents.addEvent(EVENT_KASRETH_REFLUX_CHARGE, 9000);
        scriptEvents.addEvent(EVENT_KASRETH_FLUX_COLLAPSE, 16000);
        // the engine fills his energy in 45 seconds
        scriptEvents.addEvent(EVENT_KASRETH_FULL_ENERGY, 45000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KASRETH_ARCANE_ZAP:
                castAt(anyPlayer(), SPELL_KASRETH_ARCANE_ZAP);
                scriptEvents.addEvent(EVENT_KASRETH_ARCANE_ZAP, 8000);
                break;
            case EVENT_KASRETH_REFLUX_CHARGE:
                castAt(anyPlayer(true), SPELL_KASRETH_REFLUX_CHARGE);
                scriptEvents.addEvent(EVENT_KASRETH_REFLUX_CHARGE, 20000);
                break;
            case EVENT_KASRETH_FLUX_COLLAPSE:
                castAtGround(anyPlayer(), SPELL_KASRETH_FLUX_COLLAPSE);
                scriptEvents.addEvent(EVENT_KASRETH_FLUX_COLLAPSE, 28000);
                break;
            case EVENT_KASRETH_FULL_ENERGY:
                warn("Chief Corewright Kasreth picks up a corespark! Corespark Detonation incoming!");
                castAtGround(getCreature(), SPELL_KASRETH_CORESPARK_DETONATION);
                scriptEvents.addEvent(EVENT_KASRETH_FULL_ENERGY, 45000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Corewarden Nysarra

class NexusPointXenasNysarraAI : public NexusPointXenasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NexusPointXenasNysarraAI(c); }
    explicit NexusPointXenasNysarraAI(Creature* pCreature) : NexusPointXenasBossAI(pCreature, DATA_NYSARRA) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_NYSARRA_LIGHTSCARRED, true);

        scriptEvents.addEvent(EVENT_NYSARRA_UMBRAL_LASH, 5000);
        scriptEvents.addEvent(EVENT_NYSARRA_ECLIPSING_STEP, 13000);
        scriptEvents.addEvent(EVENT_NYSARRA_LIGHTSCAR_FLARE, 18000);
        scriptEvents.addEvent(EVENT_NYSARRA_NULL_VANGUARD, 25000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NYSARRA_UMBRAL_LASH:
                castSpellOnVictim(SPELL_NYSARRA_UMBRAL_LASH);
                scriptEvents.addEvent(EVENT_NYSARRA_UMBRAL_LASH, 11000);
                break;
            case EVENT_NYSARRA_ECLIPSING_STEP:
                castAt(anyPlayer(true), SPELL_NYSARRA_ECLIPSING_STEP);
                scriptEvents.addEvent(EVENT_NYSARRA_ECLIPSING_STEP, 19000);
                break;
            case EVENT_NYSARRA_LIGHTSCAR_FLARE:
                // a fractured image of Lothraxion strikes next to a player
                if (Unit* pTarget = anyPlayer())
                    callAdd(NPC_NYSARRA_FRACTURED_IMAGE, nexusRingPoint(pTarget->GetPosition(), 2.0f, 4.0f), TIMED_DESPAWN, LightscarImageLifetimeMs);
                scriptEvents.addEvent(EVENT_NYSARRA_LIGHTSCAR_FLARE, 32000);
                break;
            case EVENT_NYSARRA_NULL_VANGUARD:
                warn("Corewarden Nysarra calls her Null Vanguard!");
                castSpellAOE(SPELL_NYSARRA_NULL_VANGUARD);
                for (uint32_t i = 0; i < NullifiersPerCall; ++i)
                    callAdd(NPC_GRAND_NULLIFIER, nexusRingPoint(getCreature()->GetPosition(), 8.0f, 14.0f));
                scriptEvents.addEvent(EVENT_NYSARRA_NULL_VANGUARD, 50000);
                break;
            default:
                break;
        }
    }
};

// erupts in a Lightscar Flare and fades again
class NexusPointXenasLightscarImageAI : public NexusPointXenasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NexusPointXenasLightscarImageAI(c); }
    explicit NexusPointXenasLightscarImageAI(Creature* pCreature) : NexusPointXenasCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_IMAGE_LIGHTSCAR_FLARE);
    }
};

class NexusPointXenasGrandNullifierAI : public NexusPointXenasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NexusPointXenasGrandNullifierAI(c); }
    explicit NexusPointXenasGrandNullifierAI(Creature* pCreature) : NexusPointXenasCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_NULLIFIER_NULLIFY, 4000);
        scriptEvents.addEvent(EVENT_NULLIFIER_DUSK_FRIGHTS, 8000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NULLIFIER_NULLIFY:
                castAt(anyPlayer(), SPELL_NULLIFIER_NULLIFY);
                scriptEvents.addEvent(EVENT_NULLIFIER_NULLIFY, 16000);
                break;
            case EVENT_NULLIFIER_DUSK_FRIGHTS:
                castAt(anyPlayer(), SPELL_NULLIFIER_DUSK_FRIGHTS);
                scriptEvents.addEvent(EVENT_NULLIFIER_DUSK_FRIGHTS, 11000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lothraxion

class NexusPointXenasLothraxionAI : public NexusPointXenasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NexusPointXenasLothraxionAI(c); }
    explicit NexusPointXenasLothraxionAI(Creature* pCreature) : NexusPointXenasBossAI(pCreature, DATA_LOTHRAXION) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_LOTHRAXION_SEARING_REND, 7000);
        scriptEvents.addEvent(EVENT_LOTHRAXION_BRILLIANT_DISPERSION, 12000);
        // full power after 50 seconds
        scriptEvents.addEvent(EVENT_LOTHRAXION_FULL_POWER, 50000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LOTHRAXION_SEARING_REND:
                castSpellOnVictim(SPELL_LOTHRAXION_SEARING_REND);
                scriptEvents.addEvent(EVENT_LOTHRAXION_SEARING_REND, 14000);
                break;
            case EVENT_LOTHRAXION_BRILLIANT_DISPERSION:
            {
                // an image lingers where each beam lands
                castSpellAOE(SPELL_LOTHRAXION_BRILLIANT_DISPERSION);
                for (Unit* pTarget : pickPlayers(DispersionTargets))
                    leaveImage(pTarget->GetPosition());
                scriptEvents.addEvent(EVENT_LOTHRAXION_BRILLIANT_DISPERSION, 26000);
                break;
            }
            case EVENT_LOTHRAXION_FULL_POWER:
                warn("Lothraxion flickers into hiding amongst his Fractured Images!");
                castSpellAOE(SPELL_LOTHRAXION_DIVINE_GUILE);
                leaveImage(nexusRingPoint(getCreature()->GetPosition(), 3.0f, 6.0f));
                scriptEvents.addEvent(EVENT_LOTHRAXION_FULL_POWER, 50000);
                break;
            default:
                break;
        }
    }

private:
    void leaveImage(LocationVector const& spot)
    {
        if (countLivingSummons(NPC_FRACTURED_IMAGE) >= LothraxionImageCap)
            return;

        callAdd(NPC_FRACTURED_IMAGE, spot);
    }
};

// stays where it appeared, slices anyone who comes close
class NexusPointXenasFracturedImageAI : public NexusPointXenasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NexusPointXenasFracturedImageAI(c); }
    explicit NexusPointXenasFracturedImageAI(Creature* pCreature) : NexusPointXenasCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_IMAGE_MIRRORED_REND, 2000);
        scriptEvents.addEvent(EVENT_IMAGE_FLICKER, 15000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_IMAGE_MIRRORED_REND:
                castAt(nearestPlayerWithin(MirroredRendReach), SPELL_IMAGE_MIRRORED_REND);
                scriptEvents.addEvent(EVENT_IMAGE_MIRRORED_REND, 4000);
                break;
            case EVENT_IMAGE_FLICKER:
                castSpellAOE(SPELL_IMAGE_FLICKER, true);
                MoveTeleport(nexusRingPoint(getCreature()->GetPosition(), 4.0f, 8.0f));
                scriptEvents.addEvent(EVENT_IMAGE_FLICKER, 15000);
                break;
            default:
                break;
        }
    }
};

void SetupNexusPointXenas(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_NEXUS_POINT_XENAS, &NexusPointXenasInstanceScript::Create);

    mgr->register_creature_script(BOSS_KASRETH, &NexusPointXenasKasrethAI::Create);

    mgr->register_creature_script(BOSS_NYSARRA, &NexusPointXenasNysarraAI::Create);
    mgr->register_creature_script(BOSS_NYSARRA_ALT, &NexusPointXenasNysarraAI::Create);
    mgr->register_creature_script(NPC_NYSARRA_FRACTURED_IMAGE, &NexusPointXenasLightscarImageAI::Create);
    mgr->register_creature_script(NPC_GRAND_NULLIFIER, &NexusPointXenasGrandNullifierAI::Create);
    mgr->register_creature_script(NPC_GRAND_NULLIFIER_ALT, &NexusPointXenasGrandNullifierAI::Create);

    mgr->register_creature_script(BOSS_LOTHRAXION, &NexusPointXenasLothraxionAI::Create);
    mgr->register_creature_script(NPC_FRACTURED_IMAGE, &NexusPointXenasFracturedImageAI::Create);
    mgr->register_creature_script(NPC_FRACTURED_IMAGE_ALT_1, &NexusPointXenasFracturedImageAI::Create);
}
