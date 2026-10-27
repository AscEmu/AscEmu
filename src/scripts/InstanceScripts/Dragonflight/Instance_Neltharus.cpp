/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Neltharus.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>

enum NeltharusEvents
{
    EVENT_MAGMATUSK_MAGMA_LOB = 1,
    EVENT_MAGMATUSK_LAVA_SPRAY,
    EVENT_MAGMATUSK_BLAZING_CHARGE,
    EVENT_MAGMATUSK_LAVA_WAVE,

    EVENT_SARGHA_MOLTEN_GOLD,
    EVENT_SARGHA_BURNING_EMBER,
    EVENT_SARGHA_THE_DRAGONS_KILN,
    EVENT_SARGHA_FLAME_ERUPTION,

    EVENT_RAGING_EMBER_BURNING_PURSUIT,

    EVENT_GOREK_HEATED_SWINGS,
    EVENT_GOREK_MIGHT_OF_THE_FORGE,
    EVENT_GOREK_BLAZING_HAMMER,
    EVENT_GOREK_BLAZING_AEGIS,
    EVENT_GOREK_FORGESTORM,

    EVENT_CHARGATH_DRAGON_STRIKE,
    EVENT_CHARGATH_MAGMA_WAVE,
    EVENT_CHARGATH_GROUNDING_SPEAR
};

namespace
{
    // energy bars, time from empty to full
    uint32_t const MagmatuskMagmaMs = 30000;
    uint32_t const SarghaShieldMs = 50000;
    uint32_t const ChargathFocusMs = 45000;

    uint32_t const SarghaFlameEruptions = 5;
    uint8_t const SarghaEmbersPerCast = 2;

    LocationVector neltharusRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(6.2831853f);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class NeltharusInstanceScript : public InstanceScript
{
public:
    explicit NeltharusInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(NeltharusEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new NeltharusInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy stand-in and target helpers

class NeltharusCombatAI : public CreatureAIScript
{
public:
    explicit NeltharusCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mEnergyMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mEnergyMs = 0;

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

    NeltharusInstanceScript* getNeltharus()
    {
        if (getCreature()->GetMapId() != MAP_NELTHARUS)
            return nullptr;

        return static_cast<NeltharusInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    Unit* castAtPlayer(uint32_t spellId, bool triggered = false)
    {
        Unit* pTarget = anyPlayer();
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    // normal difficulty has its own spell versions
    uint32_t byDifficulty(uint32_t heroicSpellId, uint32_t normalSpellId) { return isHeroic() ? heroicSpellId : normalSpellId; }

    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return neltharusRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // energy stand-in: true once the bar is full and nothing is being cast
    bool energyFull(uint32_t diff, uint32_t fullAfterMs)
    {
        if (mEnergyMs < fullAfterMs)
            mEnergyMs += diff;

        if (mEnergyMs < fullAfterMs || _isCasting())
            return false;

        mEnergyMs = 0;
        return true;
    }

    uint32_t mEnergyMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class NeltharusBossAI : public NeltharusCombatAI
{
public:
    NeltharusBossAI(Creature* pCreature, uint32_t dataId) : NeltharusCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (NeltharusInstanceScript* pNeltharus = getNeltharus())
        {
            pNeltharus->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pNeltharus->setBossState(mDataId, EncounterStates::InProgress);
        }

        NeltharusCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (NeltharusInstanceScript* pNeltharus = getNeltharus())
            {
                pNeltharus->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pNeltharus->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        NeltharusCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (NeltharusInstanceScript* pNeltharus = getNeltharus())
        {
            pNeltharus->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pNeltharus->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Magmatusk

class NeltharusMagmatuskAI : public NeltharusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NeltharusMagmatuskAI(c); }
    explicit NeltharusMagmatuskAI(Creature* pCreature) : NeltharusBossAI(pCreature, DATA_MAGMATUSK) {}

protected:
    void onEngage() override
    {
        mTentacles = 0;
        mWavesLeft = 0;

        scriptEvents.addEvent(EVENT_MAGMATUSK_MAGMA_LOB, 6000);
        scriptEvents.addEvent(EVENT_MAGMATUSK_LAVA_SPRAY, 12000);
        scriptEvents.addEvent(EVENT_MAGMATUSK_BLAZING_CHARGE, 18000);
    }

    void onReset() override
    {
        mTentacles = 0;
        _removeAura(SPELL_MAGMATUSK_MAGMA_TENTACLE);
    }

    // full magma grows another tentacle
    void onCombatTick(uint32_t diff) override
    {
        if (!energyFull(diff, MagmatuskMagmaMs))
            return;

        warn("Magmatusk's Volatile Mutation grows another Magma Tentacle!");
        castSpellOnSelf(SPELL_MAGMATUSK_VOLATILE_MUTATION);
        castSpellOnSelf(SPELL_MAGMATUSK_MAGMA_TENTACLE, true);
        ++mTentacles;
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MAGMATUSK_MAGMA_LOB:
                castAtPlayer(SPELL_MAGMATUSK_MAGMA_LOB);
                scriptEvents.addEvent(EVENT_MAGMATUSK_MAGMA_LOB, 12000);
                break;
            case EVENT_MAGMATUSK_LAVA_SPRAY:
                castAtPlayer(SPELL_MAGMATUSK_LAVA_SPRAY);
                scriptEvents.addEvent(EVENT_MAGMATUSK_LAVA_SPRAY, 20000);
                break;
            case EVENT_MAGMATUSK_BLAZING_CHARGE:
                castAtPlayer(SPELL_MAGMATUSK_BLAZING_CHARGE);
                // one lava wave per tentacle
                mWavesLeft = mTentacles;
                if (mWavesLeft > 0)
                    scriptEvents.addEvent(EVENT_MAGMATUSK_LAVA_WAVE, 2000);
                scriptEvents.addEvent(EVENT_MAGMATUSK_BLAZING_CHARGE, 30000);
                break;
            case EVENT_MAGMATUSK_LAVA_WAVE:
                castAtPlayer(SPELL_MAGMATUSK_LAVA_WAVE, true);
                if (mWavesLeft > 0 && --mWavesLeft > 0)
                    scriptEvents.addEvent(EVENT_MAGMATUSK_LAVA_WAVE, 1000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mTentacles = 0;
    uint32_t mWavesLeft = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Warlord Sargha

class NeltharusWarlordSarghaAI : public NeltharusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NeltharusWarlordSarghaAI(c); }
    explicit NeltharusWarlordSarghaAI(Creature* pCreature) : NeltharusBossAI(pCreature, DATA_WARLORD_SARGHA) {}

protected:
    void onEngage() override
    {
        mEruptionsLeft = 0;

        scriptEvents.addEvent(EVENT_SARGHA_MOLTEN_GOLD, 8000);
        scriptEvents.addEvent(EVENT_SARGHA_BURNING_EMBER, 14000);
        scriptEvents.addEvent(EVENT_SARGHA_THE_DRAGONS_KILN, 20000);
    }

    void onReset() override
    {
        mEruptionsLeft = 0;
        _removeAura(SPELL_SARGHA_MAGMA_SHIELD);
    }

    // full molten energy: the shield goes up and flames erupt beneath it
    void onCombatTick(uint32_t diff) override
    {
        if (mEruptionsLeft > 0 || !energyFull(diff, SarghaShieldMs))
            return;

        warn("Warlord Sargha channels a Magma Shield!");
        castSpellOnSelf(SPELL_SARGHA_MAGMA_SHIELD);
        mEruptionsLeft = SarghaFlameEruptions;
        scriptEvents.addEvent(EVENT_SARGHA_FLAME_ERUPTION, 3000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SARGHA_MOLTEN_GOLD:
                castAtPlayer(SPELL_SARGHA_MOLTEN_GOLD);
                scriptEvents.addEvent(EVENT_SARGHA_MOLTEN_GOLD, 18000);
                break;
            case EVENT_SARGHA_BURNING_EMBER:
                for (uint8_t i = 0; i < SarghaEmbersPerCast; ++i)
                    callAdd(NPC_RAGING_EMBER, aroundSelf(8.0f, 14.0f));
                scriptEvents.addEvent(EVENT_SARGHA_BURNING_EMBER, 30000);
                break;
            case EVENT_SARGHA_THE_DRAGONS_KILN:
                castAtPlayer(SPELL_SARGHA_THE_DRAGONS_KILN);
                scriptEvents.addEvent(EVENT_SARGHA_THE_DRAGONS_KILN, 28000);
                break;
            case EVENT_SARGHA_FLAME_ERUPTION:
                castSpellAOE(byDifficulty(SPELL_SARGHA_FLAME_ERUPTION, SPELL_SARGHA_FLAME_ERUPTION_NORMAL), true);
                if (mEruptionsLeft > 0 && --mEruptionsLeft > 0)
                    scriptEvents.addEvent(EVENT_SARGHA_FLAME_ERUPTION, 3000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mEruptionsLeft = 0;
};

// heroic: hunts down a single player
class NeltharusRagingEmberAI : public NeltharusCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NeltharusRagingEmberAI(c); }
    explicit NeltharusRagingEmberAI(Creature* pCreature) : NeltharusCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        if (isHeroic())
            scriptEvents.addEvent(EVENT_RAGING_EMBER_BURNING_PURSUIT, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_RAGING_EMBER_BURNING_PURSUIT)
            return;

        if (Unit* pTarget = castAtPlayer(SPELL_RAGING_EMBER_BURNING_PURSUIT))
            addThreat(pTarget, 100000.0f);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Forgemaster Gorek

class NeltharusForgemasterGorekAI : public NeltharusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NeltharusForgemasterGorekAI(c); }
    explicit NeltharusForgemasterGorekAI(Creature* pCreature) : NeltharusBossAI(pCreature, DATA_FORGEMASTER_GOREK) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GOREK_HEATED_SWINGS, 6000);
        scriptEvents.addEvent(EVENT_GOREK_MIGHT_OF_THE_FORGE, 10000);
        scriptEvents.addEvent(EVENT_GOREK_FORGESTORM, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GOREK_HEATED_SWINGS:
                castSpellOnVictim(SPELL_GOREK_HEATED_SWINGS);
                scriptEvents.addEvent(EVENT_GOREK_HEATED_SWINGS, 14000);
                break;
            case EVENT_GOREK_MIGHT_OF_THE_FORGE:
                // hammer blows, then the forged aegis is hurled
                warn("Forgemaster Gorek draws on the Might of the Forge!");
                castSpellOnSelf(SPELL_GOREK_MIGHT_OF_THE_FORGE);
                scriptEvents.addEvent(EVENT_GOREK_BLAZING_HAMMER, 1500);
                scriptEvents.addEvent(EVENT_GOREK_BLAZING_AEGIS, 4500);
                scriptEvents.addEvent(EVENT_GOREK_MIGHT_OF_THE_FORGE, 35000);
                break;
            case EVENT_GOREK_BLAZING_HAMMER:
                castSpellOnSelf(SPELL_GOREK_BLAZING_HAMMER);
                break;
            case EVENT_GOREK_BLAZING_AEGIS:
                castAtPlayer(SPELL_GOREK_BLAZING_AEGIS);
                break;
            case EVENT_GOREK_FORGESTORM:
                castSpellOnSelf(SPELL_GOREK_FORGESTORM);
                scriptEvents.addEvent(EVENT_GOREK_FORGESTORM, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Chargath, Bane of Scales

class NeltharusChargathAI : public NeltharusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NeltharusChargathAI(c); }
    explicit NeltharusChargathAI(Creature* pCreature) : NeltharusBossAI(pCreature, DATA_CHARGATH) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_CHARGATH_DRAGON_STRIKE, 6000);
        scriptEvents.addEvent(EVENT_CHARGATH_MAGMA_WAVE, 10000);
        scriptEvents.addEvent(EVENT_CHARGATH_GROUNDING_SPEAR, 14000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!energyFull(diff, ChargathFocusMs))
            return;

        warn("Chargath unleashes his Fiery Focus!");
        castSpellOnVictim(SPELL_CHARGATH_FIERY_FOCUS);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CHARGATH_DRAGON_STRIKE:
                castSpellOnVictim(SPELL_CHARGATH_DRAGON_STRIKE);
                scriptEvents.addEvent(EVENT_CHARGATH_DRAGON_STRIKE, 14000);
                break;
            case EVENT_CHARGATH_MAGMA_WAVE:
                castSpellAOE(SPELL_CHARGATH_MAGMA_WAVE);
                scriptEvents.addEvent(EVENT_CHARGATH_MAGMA_WAVE, 20000);
                break;
            case EVENT_CHARGATH_GROUNDING_SPEAR:
                castAtPlayer(SPELL_CHARGATH_GROUNDING_SPEAR);
                scriptEvents.addEvent(EVENT_CHARGATH_GROUNDING_SPEAR, 26000);
                break;
            default:
                break;
        }
    }
};

void SetupNeltharus(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_NELTHARUS, &NeltharusInstanceScript::Create);

    mgr->register_creature_script(BOSS_MAGMATUSK, &NeltharusMagmatuskAI::Create);

    mgr->register_creature_script(BOSS_WARLORD_SARGHA, &NeltharusWarlordSarghaAI::Create);
    mgr->register_creature_script(NPC_RAGING_EMBER, &NeltharusRagingEmberAI::Create);

    mgr->register_creature_script(BOSS_FORGEMASTER_GOREK, &NeltharusForgemasterGorekAI::Create);

    mgr->register_creature_script(BOSS_CHARGATH, &NeltharusChargathAI::Create);
}
