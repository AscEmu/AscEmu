/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_March_On_Quel_Danas.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

enum MarchOnQuelDanasEvents
{
    // Belo'ren, Child of Al'ar
    EVENT_BELOREN_VOIDLIGHT_CONVERGENCE = 1,
    EVENT_BELOREN_GUARDIANS_EDICT,
    EVENT_BELOREN_ETERNAL_BURNS,
    EVENT_BELOREN_INFUSED_QUILLS,
    EVENT_BELOREN_EMBERS,
    EVENT_BELOREN_RADIANT_ECHOES,
    EVENT_BELOREN_DEATH_DROP,
    EVENT_BELOREN_INCUBATION,
    EVENT_BELOREN_REBORN,

    // Midnight Falls
    EVENT_LURA_HEAVENS_LANCE,
    EVENT_LURA_HEAVENS_GLAIVES,
    EVENT_LURA_DEATHS_DIRGE,
    EVENT_LURA_DISINTEGRATION,
    EVENT_LURA_DARK_QUASAR,
    EVENT_LURA_SHATTERED_SKY,
    EVENT_LURA_STARSPLINTER,
    EVENT_LURA_ECLIPSE_QUASAR,
    EVENT_LURA_ECLIPSE_END,
    EVENT_LURA_GALVANIZE,
    EVENT_LURA_CORE_HARVEST,
    EVENT_LURA_DARK_MELTDOWN,
    EVENT_LURA_BLACK_TIDE,
    EVENT_LURA_DARK_CONSTELLATION,
    EVENT_LURA_DEATHS_REQUIEM,
    EVENT_LURA_LIGHT_SIPHON,
    EVENT_MIDNIGHT_CRYSTAL_FRACTURE,
    EVENT_DUSK_CRYSTAL_CHECK
};

namespace
{
    float const MarchQuelDanasTwoPi = 6.28318531f;

    // kit spells of the adds use their own event range
    uint32_t const MarchQuelDanasKitEventBase = 1000;

    uint32_t const BelorenBerserkMs = 540000;
    uint32_t const LuraBerserkMs = 600000;

    // the ashen egg holds a smaller Immortal Flame after every rebirth
    uint32_t const BelorenShellPct = 30;
    uint32_t const BelorenShellPctStep = 10;
    uint32_t const BelorenShellPctFloor = 10;
    uint32_t const BelorenRebirthMs = 30000;

    int32_t const LuraEclipsePct = 70;
    int32_t const LuraMidnightPct = 40;
    uint32_t const LuraEclipseMs = 20000;

    uint32_t const MidnightCrystalFractureMs = 25000;
    uint32_t const DuskCrystalStartPct = 30;
    uint32_t const DawnCrystalRadianceMs = 12000;
    uint32_t const DawnCrystalTearsMs = 20000;
    float const DawnCrystalSearchRange = 150.0f;

    enum BelorenStage : uint8_t
    {
        BELOREN_STAGE_NONE = 0,
        BELOREN_STAGE_PHOENIX,
        BELOREN_STAGE_ASHEN_SHELL
    };

    enum LuraStage : uint8_t
    {
        LURA_STAGE_NONE = 0,
        LURA_STAGE_FINAL_TOLLS,
        LURA_STAGE_TOTAL_ECLIPSE,
        LURA_STAGE_DARK_REACTOR,
        LURA_STAGE_MIDNIGHT_FALLS
    };

    enum MarchQuelDanasAim : uint8_t
    {
        AIM_VICTIM,
        AIM_PLAYER,
        AIM_SELF,
        AIM_AROUND
    };

    struct MarchQuelDanasKitSpell
    {
        uint32_t spellId;
        MarchQuelDanasAim aim;
        uint32_t firstMs;
        uint32_t repeatMs;
    };

    LocationVector marchQuelDanasPointAround(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(MarchQuelDanasTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    uint32_t percentOfHealth(Creature* pCreature, uint32_t pct)
    {
        return static_cast<uint32_t>(static_cast<uint64_t>(pCreature->getMaxHealth()) * pct / 100);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class MarchQuelDanasInstanceScript : public InstanceScript
{
public:
    explicit MarchQuelDanasInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(MarchOnQuelDanasEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new MarchQuelDanasInstanceScript(pMapMgr); }

    Creature* livingCreature(uint32_t entry)
    {
        for (Creature* pCreature : getCreatureSetForEntry(entry))
        {
            if (pCreature->isAlive())
                return pCreature;
        }

        return nullptr;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, berserk and target helpers

class MarchQuelDanasCombatAI : public CreatureAIScript
{
public:
    explicit MarchQuelDanasCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mBerserkLeftMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mBerserkLeftMs = 0;
        onDisengage();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        tickBerserk(diff);
        onCombatTick(diff);

        // due events wait while a cast runs
        while (!_isCasting())
        {
            uint32_t const eventId = scriptEvents.getFinishedEvent();
            if (eventId == 0)
                break;

            onEvent(eventId);
        }
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    MarchQuelDanasInstanceScript* getMarch()
    {
        if (getCreature()->GetMapId() != MAP_MARCH_ON_QUEL_DANAS)
            return nullptr;

        return static_cast<MarchQuelDanasInstanceScript*>(getInstanceScript());
    }

    // distinct random players, the tank only fills up
    std::vector<Unit*> pickPlayers(size_t count, bool skipTank = false)
    {
        Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
        FilterArgs const args(TargetFilter_Player);

        std::vector<Unit*> pool;
        std::vector<Unit*> reserve;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr || !isValidUnitTarget(pObject, TargetFilter_Player, args))
                continue;

            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (skipTank && pPlayer == pVictim)
                reserve.push_back(pPlayer);
            else
                pool.push_back(pPlayer);
        }

        std::vector<Unit*> picked;
        while (picked.size() < count && !pool.empty())
        {
            size_t const index = Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1));
            picked.push_back(pool[index]);
            pool[index] = pool.back();
            pool.pop_back();
        }

        for (Unit* pPlayer : reserve)
        {
            if (picked.size() < count)
                picked.push_back(pPlayer);
        }

        return picked;
    }

    Unit* castAtPlayer(uint32_t spellId, bool skipTank = false, bool triggered = false)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipTank);
        if (picked.empty())
            return nullptr;

        castSpell(picked.front(), spellId, triggered);
        return picked.front();
    }

    void castAtPlayers(uint32_t spellId, size_t count, bool skipTank = false)
    {
        for (Unit* pTarget : pickPlayers(count, skipTank))
            castSpell(pTarget, spellId, true);
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
        return marchQuelDanasPointAround(getCreature()->GetPosition(), minRange, maxRange);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void armBerserk(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

    // holds its spot and keeps out of melee
    void anchor(bool anchored)
    {
        setRooted(anchored);
        _setMeleeDisabled(anchored);
    }

private:
    void tickBerserk(uint32_t diff)
    {
        if (mBerserkLeftMs == 0)
            return;

        if (mBerserkLeftMs > diff)
        {
            mBerserkLeftMs -= diff;
            return;
        }

        mBerserkLeftMs = 0;
        castSpellOnSelf(SPELL_MARCH_QUEL_DANAS_BERSERK, true);
    }

    uint32_t mBerserkLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class MarchQuelDanasBossAI : public MarchQuelDanasCombatAI
{
public:
    MarchQuelDanasBossAI(Creature* pCreature, uint32_t dataId) : MarchQuelDanasCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (MarchQuelDanasInstanceScript* pMarch = getMarch())
        {
            pMarch->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pMarch->setBossState(mDataId, EncounterStates::InProgress);
        }

        MarchQuelDanasCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (getCreature()->isAlive())
        {
            if (MarchQuelDanasInstanceScript* pMarch = getMarch())
            {
                pMarch->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pMarch->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        MarchQuelDanasCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();

        if (MarchQuelDanasInstanceScript* pMarch = getMarch())
        {
            pMarch->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pMarch->setBossState(mDataId, EncounterStates::Performed);
        }
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Adds driven by a list of timed spells, bursting with a last spell instead of dying

class MarchQuelDanasBursterAI : public MarchQuelDanasCombatAI
{
public:
    MarchQuelDanasBursterAI(Creature* pCreature, std::vector<MarchQuelDanasKitSpell> kit, uint32_t burstSpellId)
        : MarchQuelDanasCombatAI(pCreature), mKit(std::move(kit)), mBurstSpellId(burstSpellId) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mBurst)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        mBurst = true;
        scriptEvents.resetEvents();
        getCreature()->interruptSpell();
        castSpellAOE(mBurstSpellId, true);
        despawn(500, 0);
    }

protected:
    void onEngage() override
    {
        for (size_t i = 0; i < mKit.size(); ++i)
            scriptEvents.addEvent(MarchQuelDanasKitEventBase + static_cast<uint32_t>(i), mKit[i].firstMs);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId < MarchQuelDanasKitEventBase || eventId >= MarchQuelDanasKitEventBase + mKit.size())
            return;

        MarchQuelDanasKitSpell const& entry = mKit[eventId - MarchQuelDanasKitEventBase];
        switch (entry.aim)
        {
            case AIM_VICTIM:
                castSpellOnVictim(entry.spellId);
                break;
            case AIM_PLAYER:
                castAtPlayer(entry.spellId);
                break;
            case AIM_SELF:
                castSpellOnSelf(entry.spellId);
                break;
            default:
                castSpellAOE(entry.spellId);
                break;
        }

        scriptEvents.addEvent(eventId, entry.repeatMs);
    }

private:
    std::vector<MarchQuelDanasKitSpell> mKit;
    uint32_t mBurstSpellId;
    bool mBurst = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Belo'ren, Child of Al'ar

// falls into an ashen egg and is reborn unless the Immortal Flame is broken in time
class MarchQuelDanasBelorenAI : public MarchQuelDanasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MarchQuelDanasBelorenAI(c); }
    explicit MarchQuelDanasBelorenAI(Creature* pCreature) : MarchQuelDanasBossAI(pCreature, DATA_BELOREN) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mStage != BELOREN_STAGE_PHOENIX)
            return;

        if (*damage < getCreature()->getHealth())
            return;

        *damage = 0;
        enterAshenShell();
    }

protected:
    void onEngage() override
    {
        mRebirths = 0;
        armBerserk(BelorenBerserkMs);
        castSpellOnSelf(SPELL_BELOREN_IMMORTAL_FLAME, true);
        ascend();
    }

    void onDisengage() override
    {
        mStage = BELOREN_STAGE_NONE;
        anchor(false);
        _removeAura(SPELL_BELOREN_REBIRTH);
        _removeAura(SPELL_BELOREN_IMMORTAL_FLAME);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BELOREN_VOIDLIGHT_CONVERGENCE:
                warn("Belo'ren infuses the raid with Light and Void!");
                castSpellAOE(SPELL_BELOREN_VOIDLIGHT_CONVERGENCE);
                scriptEvents.addEvent(EVENT_BELOREN_VOIDLIGHT_CONVERGENCE, 40000);
                break;
            case EVENT_BELOREN_GUARDIANS_EDICT:
                castSpellOnVictim(SPELL_BELOREN_GUARDIANS_EDICT);
                scriptEvents.addEvent(EVENT_BELOREN_GUARDIANS_EDICT, 20000);
                break;
            case EVENT_BELOREN_ETERNAL_BURNS:
                castAtPlayers(SPELL_BELOREN_ETERNAL_BURNS, 2, true);
                scriptEvents.addEvent(EVENT_BELOREN_ETERNAL_BURNS, 25000);
                break;
            case EVENT_BELOREN_INFUSED_QUILLS:
                castSpellAOE(SPELL_BELOREN_INFUSED_QUILLS);
                scriptEvents.addEvent(EVENT_BELOREN_INFUSED_QUILLS, 30000);
                break;
            case EVENT_BELOREN_EMBERS:
                releaseEmbers();
                scriptEvents.addEvent(EVENT_BELOREN_EMBERS, 50000);
                break;
            case EVENT_BELOREN_RADIANT_ECHOES:
                castSpellAOE(SPELL_BELOREN_RADIANT_ECHOES);
                scriptEvents.addEvent(EVENT_BELOREN_RADIANT_ECHOES, 35000);
                break;
            case EVENT_BELOREN_DEATH_DROP:
                castAtPlayer(SPELL_BELOREN_DEATH_DROP, true);
                scriptEvents.addEvent(EVENT_BELOREN_DEATH_DROP, 45000);
                break;
            case EVENT_BELOREN_INCUBATION:
                // stunned inside the egg, the flames still pulse
                castSpellAOE(SPELL_BELOREN_INCUBATION_OF_FLAMES, true);
                scriptEvents.addEvent(EVENT_BELOREN_INCUBATION, 8000);
                break;
            case EVENT_BELOREN_REBORN:
                reborn();
                break;
            default:
                break;
        }
    }

private:
    void ascend()
    {
        mStage = BELOREN_STAGE_PHOENIX;
        scriptEvents.addEvent(EVENT_BELOREN_GUARDIANS_EDICT, 8000);
        scriptEvents.addEvent(EVENT_BELOREN_VOIDLIGHT_CONVERGENCE, 12000);
        scriptEvents.addEvent(EVENT_BELOREN_ETERNAL_BURNS, 18000);
        scriptEvents.addEvent(EVENT_BELOREN_INFUSED_QUILLS, 24000);
        scriptEvents.addEvent(EVENT_BELOREN_EMBERS, 30000);
        scriptEvents.addEvent(EVENT_BELOREN_RADIANT_ECHOES, 36000);
        scriptEvents.addEvent(EVENT_BELOREN_DEATH_DROP, 45000);
    }

    void enterAshenShell()
    {
        mStage = BELOREN_STAGE_ASHEN_SHELL;
        scriptEvents.resetEvents();
        getCreature()->interruptSpell();
        warn("Belo'ren retreats into an ashen egg! Break his Immortal Flame before he is reborn!");

        anchor(true);
        uint32_t const step = mRebirths * BelorenShellPctStep;
        uint32_t const shellPct = BelorenShellPct > step + BelorenShellPctFloor ? BelorenShellPct - step : BelorenShellPctFloor;
        getCreature()->setHealth(std::max<uint32_t>(1, percentOfHealth(getCreature(), shellPct)));
        castSpellOnSelf(SPELL_BELOREN_REBIRTH, true);

        scriptEvents.addEvent(EVENT_BELOREN_INCUBATION, 4000);
        scriptEvents.addEvent(EVENT_BELOREN_REBORN, BelorenRebirthMs);
    }

    void reborn()
    {
        ++mRebirths;
        scriptEvents.resetEvents();
        _removeAura(SPELL_BELOREN_REBIRTH);
        anchor(false);

        getCreature()->setHealth(getCreature()->getMaxHealth());
        castSpellOnSelf(SPELL_BELOREN_ASHEN_BENEDICTION, true);
        warn("Belo'ren rises from the ashes, reborn anew!");
        ascend();
    }

    void releaseEmbers()
    {
        castSpellAOE(SPELL_BELOREN_EMBERS_OF_BELOREN);
        callAdd(NPC_BELOREN_LIGHT_EMBER, aroundSelf(10.0f, 18.0f));
        callAdd(NPC_BELOREN_VOID_EMBER, aroundSelf(10.0f, 18.0f));
    }

    uint8_t mStage = BELOREN_STAGE_NONE;
    uint32_t mRebirths = 0;
};

class MarchQuelDanasLightEmberAI : public MarchQuelDanasBursterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MarchQuelDanasLightEmberAI(c); }
    explicit MarchQuelDanasLightEmberAI(Creature* pCreature)
        : MarchQuelDanasBursterAI(pCreature, {
            { SPELL_LIGHT_EMBER_LIGHT_BLAST, AIM_VICTIM, 3000, 6000 },
            { SPELL_LIGHT_EMBER_LIGHT_DIVE, AIM_PLAYER, 8000, 14000 } }, SPELL_LIGHT_EMBER_LIGHT_ERUPTION) {}
};

class MarchQuelDanasVoidEmberAI : public MarchQuelDanasBursterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MarchQuelDanasVoidEmberAI(c); }
    explicit MarchQuelDanasVoidEmberAI(Creature* pCreature)
        : MarchQuelDanasBursterAI(pCreature, {
            { SPELL_VOID_EMBER_VOID_BLAST, AIM_VICTIM, 3000, 6000 },
            { SPELL_VOID_EMBER_VOID_DIVE, AIM_PLAYER, 8000, 14000 } }, SPELL_VOID_EMBER_VOID_ERUPTION) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Midnight Falls

class MarchQuelDanasLuraAI : public MarchQuelDanasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MarchQuelDanasLuraAI(c); }
    explicit MarchQuelDanasLuraAI(Creature* pCreature) : MarchQuelDanasBossAI(pCreature, DATA_MIDNIGHT_FALLS) {}

    // beyond reach during the eclipse
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && mStage == LURA_STAGE_TOTAL_ECLIPSE)
            *damage = 0;
    }

protected:
    void onEngage() override
    {
        mStage = LURA_STAGE_FINAL_TOLLS;
        armBerserk(LuraBerserkMs);
        castSpellOnSelf(SPELL_LURA_THE_DARKWELL, true);

        scriptEvents.addEvent(EVENT_LURA_HEAVENS_LANCE, 10000);
        scriptEvents.addEvent(EVENT_LURA_HEAVENS_GLAIVES, 15000);
        scriptEvents.addEvent(EVENT_LURA_DEATHS_DIRGE, 22000);
        scriptEvents.addEvent(EVENT_LURA_DISINTEGRATION, 30000);
        scriptEvents.addEvent(EVENT_LURA_DARK_QUASAR, 36000);
        scriptEvents.addEvent(EVENT_LURA_SHATTERED_SKY, 45000);
    }

    void onDisengage() override
    {
        mStage = LURA_STAGE_NONE;
        anchor(false);
        _removeAura(SPELL_LURA_THE_DARK_ARCHANGEL);
        _removeAura(SPELL_LURA_THE_DARKWELL);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == LURA_STAGE_FINAL_TOLLS && _getHealthPercent() <= LuraEclipsePct)
            beginTotalEclipse();
        else if (mStage == LURA_STAGE_DARK_REACTOR && _getHealthPercent() <= LuraMidnightPct)
            beginMidnightFalls();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LURA_HEAVENS_LANCE:
                castSpellOnVictim(SPELL_LURA_HEAVENS_LANCE);
                scriptEvents.addEvent(EVENT_LURA_HEAVENS_LANCE, 25000);
                break;
            case EVENT_LURA_HEAVENS_GLAIVES:
                castAtPlayer(SPELL_LURA_HEAVENS_GLAIVES, true);
                scriptEvents.addEvent(EVENT_LURA_HEAVENS_GLAIVES, 20000);
                break;
            case EVENT_LURA_DEATHS_DIRGE:
                warn("The toll of Death's Dirge echoes off the sacred walls!");
                castSpellAOE(SPELL_LURA_DEATHS_DIRGE);
                scriptEvents.addEvent(EVENT_LURA_DEATHS_DIRGE, 30000);
                break;
            case EVENT_LURA_DISINTEGRATION:
                disintegrate();
                scriptEvents.addEvent(EVENT_LURA_DISINTEGRATION, 40000);
                break;
            case EVENT_LURA_DARK_QUASAR:
                castSpellAOE(SPELL_LURA_DARK_QUASAR);
                scriptEvents.addEvent(EVENT_LURA_DARK_QUASAR, 35000);
                break;
            case EVENT_LURA_SHATTERED_SKY:
                castSpellAOE(SPELL_LURA_SHATTERED_SKY);
                scriptEvents.addEvent(EVENT_LURA_SHATTERED_SKY, 45000);
                break;
            case EVENT_LURA_STARSPLINTER:
                castAtPlayer(SPELL_LURA_STARSPLINTER, false, true);
                scriptEvents.addEvent(EVENT_LURA_STARSPLINTER, 8000);
                break;
            case EVENT_LURA_ECLIPSE_QUASAR:
                castSpellAOE(SPELL_LURA_ECLIPSE_DARK_QUASAR);
                break;
            case EVENT_LURA_ECLIPSE_END:
                enterDarkReactor();
                break;
            case EVENT_LURA_GALVANIZE:
                castAtPlayer(SPELL_LURA_GALVANIZE, true);
                scriptEvents.addEvent(EVENT_LURA_GALVANIZE, 25000);
                break;
            case EVENT_LURA_CORE_HARVEST:
                castSpellAOE(SPELL_LURA_CORE_HARVEST);
                scriptEvents.addEvent(EVENT_LURA_CORE_HARVEST, 40000);
                break;
            case EVENT_LURA_DARK_MELTDOWN:
                warn("The Darkwell reactor approaches a Dark Meltdown!");
                castSpellAOE(SPELL_LURA_DARK_MELTDOWN);
                scriptEvents.addEvent(EVENT_LURA_DARK_MELTDOWN, 50000);
                break;
            case EVENT_LURA_BLACK_TIDE:
                castSpellAOE(SPELL_LURA_BLACK_TIDE);
                scriptEvents.addEvent(EVENT_LURA_BLACK_TIDE, 30000);
                break;
            case EVENT_LURA_DARK_CONSTELLATION:
                castAtPlayer(SPELL_LURA_DARK_CONSTELLATION, true);
                scriptEvents.addEvent(EVENT_LURA_DARK_CONSTELLATION, 22000);
                break;
            case EVENT_LURA_DEATHS_REQUIEM:
                warn("L'ura begins Death's Requiem!");
                castSpellAOE(SPELL_LURA_DEATHS_REQUIEM);
                scriptEvents.addEvent(EVENT_LURA_DEATHS_REQUIEM, 35000);
                break;
            case EVENT_LURA_LIGHT_SIPHON:
                castAtPlayer(SPELL_LURA_LIGHT_SIPHON, true);
                scriptEvents.addEvent(EVENT_LURA_LIGHT_SIPHON, 40000);
                break;
            default:
                break;
        }
    }

private:
    // a dark crystal to destroy and a dimming one to restore
    void disintegrate()
    {
        castAtPlayer(SPELL_LURA_DISINTEGRATION, true);
        callAdd(NPC_LURA_MIDNIGHT_CRYSTAL, aroundSelf(15.0f, 25.0f));
        callAdd(NPC_LURA_DUSK_CRYSTAL, aroundSelf(15.0f, 25.0f));
    }

    void beginTotalEclipse()
    {
        mStage = LURA_STAGE_TOTAL_ECLIPSE;
        scriptEvents.resetEvents();
        getCreature()->interruptSpell();
        anchor(true);
        warn("L'ura shrouds the Sunwell in a Total Eclipse!");
        castSpellAOE(SPELL_LURA_TOTAL_ECLIPSE);

        scriptEvents.addEvent(EVENT_LURA_STARSPLINTER, 6000);
        scriptEvents.addEvent(EVENT_LURA_ECLIPSE_QUASAR, 10000);
        scriptEvents.addEvent(EVENT_LURA_ECLIPSE_END, LuraEclipseMs);
    }

    void enterDarkReactor()
    {
        mStage = LURA_STAGE_DARK_REACTOR;
        scriptEvents.resetEvents();
        anchor(false);
        castSpellOnSelf(SPELL_LURA_INTO_THE_DARKWELL, true);

        scriptEvents.addEvent(EVENT_LURA_HEAVENS_LANCE, 8000);
        scriptEvents.addEvent(EVENT_LURA_GALVANIZE, 12000);
        scriptEvents.addEvent(EVENT_LURA_CORE_HARVEST, 20000);
        scriptEvents.addEvent(EVENT_LURA_DARK_MELTDOWN, 32000);
        scriptEvents.addEvent(EVENT_LURA_SHATTERED_SKY, 40000);
    }

    void beginMidnightFalls()
    {
        mStage = LURA_STAGE_MIDNIGHT_FALLS;
        scriptEvents.resetEvents();
        warn("Time is up. Midnight falls!");
        castSpellOnSelf(SPELL_LURA_THE_DARK_ARCHANGEL, true);
        castSpellAOE(SPELL_LURA_MIDNIGHT, true);

        scriptEvents.addEvent(EVENT_LURA_HEAVENS_LANCE, 8000);
        scriptEvents.addEvent(EVENT_LURA_DARK_CONSTELLATION, 12000);
        scriptEvents.addEvent(EVENT_LURA_BLACK_TIDE, 18000);
        scriptEvents.addEvent(EVENT_LURA_DEATHS_REQUIEM, 26000);
        scriptEvents.addEvent(EVENT_LURA_LIGHT_SIPHON, 34000);
        scriptEvents.addEvent(EVENT_LURA_SHATTERED_SKY, 45000);
    }

    uint8_t mStage = LURA_STAGE_NONE;
};

// fractures the cosmos unless destroyed in time
class MarchQuelDanasMidnightCrystalAI : public MarchQuelDanasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MarchQuelDanasMidnightCrystalAI(c); }
    explicit MarchQuelDanasMidnightCrystalAI(Creature* pCreature) : MarchQuelDanasCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        anchor(true);
        scriptEvents.addEvent(EVENT_MIDNIGHT_CRYSTAL_FRACTURE, MidnightCrystalFractureMs);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_MIDNIGHT_CRYSTAL_FRACTURE)
            return;

        warn("A Midnight Crystal undergoes a Cosmic Fracture!");
        castSpellAOE(SPELL_MIDNIGHT_CRYSTAL_COSMIC_FRACTURE, true);
        despawn(1000, 0);
    }
};

// dims away, turns into a Dawn Crystal once restored to full health
class MarchQuelDanasDuskCrystalAI : public MarchQuelDanasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MarchQuelDanasDuskCrystalAI(c); }
    explicit MarchQuelDanasDuskCrystalAI(Creature* pCreature) : MarchQuelDanasCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        anchor(true);
        if (!mDimmed)
        {
            mDimmed = true;
            getCreature()->setHealth(std::max<uint32_t>(1, percentOfHealth(getCreature(), DuskCrystalStartPct)));
        }

        castSpellOnSelf(SPELL_DUSK_CRYSTAL_DIMMING, true);
        scriptEvents.addEvent(EVENT_DUSK_CRYSTAL_CHECK, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_DUSK_CRYSTAL_CHECK)
            return;

        if (getCreature()->getHealth() < getCreature()->getMaxHealth())
        {
            scriptEvents.addEvent(EVENT_DUSK_CRYSTAL_CHECK, 1000);
            return;
        }

        restore();
    }

private:
    // L'ura owns the Dawn Crystal so it leaves with her fight
    void restore()
    {
        MarchQuelDanasInstanceScript* pMarch = getMarch();
        Creature* pLura = pMarch != nullptr ? pMarch->livingCreature(BOSS_LURA) : findNearestCreature(BOSS_LURA, DawnCrystalSearchRange);
        if (pLura != nullptr && pLura->isAlive())
            pLura->summonCreature(NPC_LURA_DAWN_CRYSTAL, getCreature()->GetPosition(), CORPSE_DESPAWN);

        despawn(200, 0);
    }

    bool mDimmed = false;
};

// purified fragment, shines Radiance while L'ura fights
class MarchQuelDanasDawnCrystalAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MarchQuelDanasDawnCrystalAI(c); }
    explicit MarchQuelDanasDawnCrystalAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        if (!mGlimmering)
        {
            mGlimmering = true;
            castSpellOnSelf(SPELL_DAWN_CRYSTAL_GLIMMERING, true);
        }

        // L'ura is looked up once a second
        mStepMs += static_cast<uint32_t>(time_passed);
        if (mStepMs < 1000)
            return;

        uint32_t const diff = mStepMs;
        mStepMs = 0;
        mRadianceMs += diff;
        mTearsMs += diff;

        Creature* pLura = findNearestCreature(BOSS_LURA, DawnCrystalSearchRange);
        if (pLura == nullptr || !pLura->isAlive() || !pLura->isInCombat())
        {
            mRadianceMs = 0;
            mTearsMs = 0;
            return;
        }

        if (mRadianceMs >= DawnCrystalRadianceMs)
        {
            mRadianceMs = 0;
            castSpellAOE(SPELL_DAWN_CRYSTAL_RADIANCE, true);
        }

        if (isHeroic() && mTearsMs >= DawnCrystalTearsMs)
        {
            mTearsMs = 0;
            castSpellAOE(SPELL_DAWN_CRYSTAL_TEARS_OF_LURA, true);
        }
    }

private:
    uint32_t mStepMs = 0;
    uint32_t mRadianceMs = 0;
    uint32_t mTearsMs = 0;
    bool mGlimmering = false;
};

void SetupMarchOnQuelDanas(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MARCH_ON_QUEL_DANAS, &MarchQuelDanasInstanceScript::Create);

    // Belo'ren, Child of Al'ar
    mgr->register_creature_script(BOSS_BELOREN, &MarchQuelDanasBelorenAI::Create);
    mgr->register_creature_script(NPC_BELOREN_LIGHT_EMBER, &MarchQuelDanasLightEmberAI::Create);
    mgr->register_creature_script(NPC_BELOREN_LIGHT_EMBER_ALT, &MarchQuelDanasLightEmberAI::Create);
    mgr->register_creature_script(NPC_BELOREN_VOID_EMBER, &MarchQuelDanasVoidEmberAI::Create);
    mgr->register_creature_script(NPC_BELOREN_VOID_EMBER_ALT, &MarchQuelDanasVoidEmberAI::Create);

    // Midnight Falls
    mgr->register_creature_script(BOSS_LURA, &MarchQuelDanasLuraAI::Create);
    mgr->register_creature_script(NPC_LURA_MIDNIGHT_CRYSTAL, &MarchQuelDanasMidnightCrystalAI::Create);
    mgr->register_creature_script(NPC_LURA_DUSK_CRYSTAL, &MarchQuelDanasDuskCrystalAI::Create);
    mgr->register_creature_script(NPC_LURA_DAWN_CRYSTAL, &MarchQuelDanasDawnCrystalAI::Create);
}
