/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Crucible_Of_Storms.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

enum CrucibleOfStormsEvents
{
    // The Restless Cabal
    EVENT_CRUCIBLE_ZAXASJ_APHOTIC_BLAST = 1,
    EVENT_CRUCIBLE_ZAXASJ_CEREBRAL_ASSAULT,
    EVENT_CRUCIBLE_ZAXASJ_DARK_HERALD,
    EVENT_CRUCIBLE_FATHUUL_SHEAR_MIND,
    EVENT_CRUCIBLE_FATHUUL_CRUSHING_DOUBT,
    EVENT_CRUCIBLE_FATHUUL_ABOMINATIONS,
    EVENT_CRUCIBLE_VISAGE_ECHO,
    EVENT_CRUCIBLE_ABOMINATION_WITNESS,

    // Uu'nat
    EVENT_CRUCIBLE_UUNAT_TOUCH,
    EVENT_CRUCIBLE_UUNAT_TEAR,
    EVENT_CRUCIBLE_UUNAT_EYES,
    EVENT_CRUCIBLE_UUNAT_VOID_CRASH,
    EVENT_CRUCIBLE_UUNAT_GIFT,
    EVENT_CRUCIBLE_UUNAT_MINDBENDER,
    EVENT_CRUCIBLE_UUNAT_TERROR,
    EVENT_CRUCIBLE_UUNAT_TORMENT,
    EVENT_CRUCIBLE_UUNAT_DEMISE,
    EVENT_CRUCIBLE_MINDBENDER_ESSENCE,
    EVENT_CRUCIBLE_TERROR_FEAR
};

namespace
{
    float const CrucibleTwoPi = 6.28318531f;

    uint32_t const CrucibleBerserkMs = 600000;
    uint32_t const VisageLifetimeMs = 180000;

    // Uu'nat's stages by health
    int32_t const UunatSecondStagePct = 70;
    int32_t const UunatThirdStagePct = 45;

    // each Cabal member empowers a relic at these health marks
    int32_t const CabalRelicMarks[3] = { 75, 50, 25 };

    struct CrucibleRelic
    {
        uint32_t spellId;
        bool onCaster;
        char const* text;
    };

    CrucibleRelic const CabalRelics[3] =
    {
        { SPELL_RELIC_STORM_OF_ANNIHILATION, true,  "The Tempest Caller unleashes a Storm of Annihilation!" },
        { SPELL_RELIC_CUSTODY_OF_THE_DEEP,   true,  "The Trident of Deep Ocean wraps the Cabal in Custody of the Deep!" },
        { SPELL_RELIC_EMBRACE_OF_THE_VOID,   false, "The Void Stone spreads the Embrace of the Void!" }
    };

    uint32_t const CabalMembers[2] = { BOSS_ZAXASJ_THE_SPEAKER, BOSS_FATHUUL_THE_FEARED };

    // random point on a ring around origin
    LocationVector crucibleRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(CrucibleTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class CrucibleOfStormsInstanceScript : public InstanceScript
{
public:
    explicit CrucibleOfStormsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(CrucibleOfStormsEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new CrucibleOfStormsInstanceScript(pMapMgr); }

    // one fallen Cabal member must not end the encounter while the other stands
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isEncounterDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (id != DATA_RESTLESS_CABAL || state != EncounterStates::Failed)
            return;

        // a wiped Cabal fights again in full
        for (Creature* pMember : getCabal())
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    std::vector<Creature*> getCabal()
    {
        std::vector<Creature*> cabal;
        for (uint32_t entry : CabalMembers)
        {
            for (Creature* pMember : getCreatureSetForEntry(entry))
                cabal.push_back(pMember);
        }

        return cabal;
    }

    // single boss encounters are down with their boss
    bool isEncounterDown(uint32_t dataId, Creature* pExcept)
    {
        if (dataId != DATA_RESTLESS_CABAL)
            return true;

        for (Creature* pMember : getCabal())
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    // pulling one member pulls the other
    void rallyCabal(CreatureAIScript* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getCabal())
        {
            if (pMember == pPuller->getCreature() || !pMember->isAlive() || pMember->isInCombat())
                continue;

            pPuller->setZoneWideCombat(pMember);
        }
        mRallying = false;
    }

private:
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick, berserk and target helpers

class CrucibleCombatAI : public CreatureAIScript
{
public:
    explicit CrucibleCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mStage = 1;
        mBerserkLeftMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mStage = 0;
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

        // every due event runs unless a cast holds the queue
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

    CrucibleOfStormsInstanceScript* getCrucible()
    {
        if (getCreature()->GetMapId() != MAP_CRUCIBLE_OF_STORMS)
            return nullptr;

        return static_cast<CrucibleOfStormsInstanceScript*>(getInstanceScript());
    }

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

    Unit* pickPlayer(bool skipTank = false)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipTank);
        return picked.empty() ? nullptr : picked.front();
    }

    Unit* castAtPlayer(uint32_t spellId, bool skipTank = false, bool triggered = false)
    {
        Unit* pTarget = pickPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    void castAtPlayers(uint32_t spellId, size_t count, bool skipTank = false, bool triggered = true)
    {
        for (Unit* pTarget : pickPlayers(count, skipTank))
            castSpell(pTarget, spellId, triggered);
    }

    // ground targeted spell under a random player
    void castUnderPlayer(uint32_t spellId, bool skipTank, bool triggered)
    {
        if (Unit* pTarget = pickPlayer(skipTank))
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
        return crucibleRingPoint(getCreature()->GetPosition(), minRange, maxRange);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void armBerserk(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

    // a new stage drops every pending event
    void enterStage(uint32_t stage)
    {
        mStage = stage;
        scriptEvents.resetEvents();
    }

    uint32_t mStage = 0;

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
        castSpellOnSelf(SPELL_CRUCIBLE_BERSERK, true);
    }

    uint32_t mBerserkLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class CrucibleBossAI : public CrucibleCombatAI
{
public:
    CrucibleBossAI(Creature* pCreature, uint32_t dataId) : CrucibleCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (CrucibleOfStormsInstanceScript* pCrucible = getCrucible())
        {
            if (mDataId == DATA_RESTLESS_CABAL)
                pCrucible->rallyCabal(this);
        }

        CrucibleCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (getCreature()->isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        CrucibleCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            CrucibleOfStormsInstanceScript* pCrucible = getCrucible();
            if (pCrucible == nullptr || pCrucible->isEncounterDown(mDataId, getCreature()))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Adds with one timed spell

class CrucibleAddAI : public CrucibleCombatAI
{
public:
    CrucibleAddAI(Creature* pCreature, uint32_t eventId, uint32_t spellId, uint32_t firstMs, uint32_t repeatMs)
        : CrucibleCombatAI(pCreature), mEventId(eventId), mSpellId(spellId), mFirstMs(firstMs), mRepeatMs(repeatMs) {}

protected:
    void onEngage() override { scriptEvents.addEvent(mEventId, mFirstMs); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != mEventId)
            return;

        castSpellAOE(mSpellId);
        scriptEvents.addEvent(mEventId, mRepeatMs);
    }

private:
    uint32_t mEventId;
    uint32_t mSpellId;
    uint32_t mFirstMs;
    uint32_t mRepeatMs;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Restless Cabal

// a member empowers the next relic at 75%, 50% and 25%
class CrucibleCabalMemberAI : public CrucibleBossAI
{
public:
    CrucibleCabalMemberAI(Creature* pCreature, uint8_t relicOffset) : CrucibleBossAI(pCreature, DATA_RESTLESS_CABAL), mRelicOffset(relicOffset) {}

protected:
    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mRelicsUsed >= 3 || _getHealthPercent() > CabalRelicMarks[mRelicsUsed])
            return;

        CrucibleRelic const& relic = CabalRelics[(mRelicsUsed + mRelicOffset) % 3];
        ++mRelicsUsed;

        warn(relic.text);
        if (relic.onCaster)
            castSpellOnSelf(relic.spellId, true);
        else
            castSpellAOE(relic.spellId, true);

        onRelicEmpowered();
    }

    void resetRelics() { mRelicsUsed = 0; }

    virtual void onRelicEmpowered() {}

private:
    uint8_t mRelicOffset;
    uint8_t mRelicsUsed = 0;
};

class CrucibleZaxasjAI : public CrucibleCabalMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CrucibleZaxasjAI(c); }
    explicit CrucibleZaxasjAI(Creature* pCreature) : CrucibleCabalMemberAI(pCreature, 0) {}

protected:
    // assails his foes from range
    void onEngage() override
    {
        resetRelics();
        armBerserk(CrucibleBerserkMs);
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_CRUCIBLE_ZAXASJ_APHOTIC_BLAST, 5000);
        scriptEvents.addEvent(EVENT_CRUCIBLE_ZAXASJ_CEREBRAL_ASSAULT, 15000);
        scriptEvents.addEvent(EVENT_CRUCIBLE_ZAXASJ_DARK_HERALD, 22000);
    }

    void onDisengage() override { _setMeleeDisabled(false); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CRUCIBLE_ZAXASJ_APHOTIC_BLAST:
                castSpellOnVictim(SPELL_ZAXASJ_APHOTIC_BLAST);
                scriptEvents.addEvent(EVENT_CRUCIBLE_ZAXASJ_APHOTIC_BLAST, 8000);
                break;
            case EVENT_CRUCIBLE_ZAXASJ_CEREBRAL_ASSAULT:
                if (castAtPlayer(SPELL_ZAXASJ_CEREBRAL_ASSAULT, true) != nullptr)
                    warn("Zaxasj the Speaker prepares a Cerebral Assault!");
                scriptEvents.addEvent(EVENT_CRUCIBLE_ZAXASJ_CEREBRAL_ASSAULT, 31000);
                break;
            case EVENT_CRUCIBLE_ZAXASJ_DARK_HERALD:
                castAtPlayer(SPELL_ZAXASJ_DARK_HERALD, true);
                scriptEvents.addEvent(EVENT_CRUCIBLE_ZAXASJ_DARK_HERALD, 32000);
                break;
            default:
                break;
        }
    }

    void onRelicEmpowered() override
    {
        warn("Zaxasj the Speaker calls forth a Visage from Beyond!");
        callAdd(NPC_CABAL_VISAGE_FROM_BEYOND, aroundSelf(5.0f, 10.0f), TIMED_DESPAWN, VisageLifetimeMs);
    }
};

class CrucibleFathuulAI : public CrucibleCabalMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CrucibleFathuulAI(c); }
    explicit CrucibleFathuulAI(Creature* pCreature) : CrucibleCabalMemberAI(pCreature, 1) {}

protected:
    void onEngage() override
    {
        resetRelics();
        armBerserk(CrucibleBerserkMs);
        scriptEvents.addEvent(EVENT_CRUCIBLE_FATHUUL_SHEAR_MIND, 6000);
        scriptEvents.addEvent(EVENT_CRUCIBLE_FATHUUL_CRUSHING_DOUBT, 12000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CRUCIBLE_FATHUUL_SHEAR_MIND:
                castSpellOnVictim(SPELL_FATHUUL_SHEAR_MIND);
                scriptEvents.addEvent(EVENT_CRUCIBLE_FATHUUL_SHEAR_MIND, 8500);
                break;
            case EVENT_CRUCIBLE_FATHUUL_CRUSHING_DOUBT:
                castAtPlayers(SPELL_FATHUUL_CRUSHING_DOUBT, 2, true);
                scriptEvents.addEvent(EVENT_CRUCIBLE_FATHUUL_CRUSHING_DOUBT, 31000);
                break;
            case EVENT_CRUCIBLE_FATHUUL_ABOMINATIONS:
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_CABAL_ELDRITCH_ABOMINATION, aroundSelf(8.0f, 15.0f));
                break;
            default:
                break;
        }
    }

    void onRelicEmpowered() override
    {
        warn("Fa'thuul the Feared calls an Eldritch Revelation!");
        castSpellOnSelf(SPELL_FATHUUL_ELDRITCH_REVELATION, true);
        scriptEvents.addEvent(EVENT_CRUCIBLE_FATHUUL_ABOMINATIONS, 3000);
    }
};

// cannot be killed, at 1 health it stops its echo and coalesces again
class CrucibleVisageFromBeyondAI : public CrucibleCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CrucibleVisageFromBeyondAI(c); }
    explicit CrucibleVisageFromBeyondAI(Creature* pCreature) : CrucibleCombatAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        if (mCoalescing)
            return;

        mCoalescing = true;
        getCreature()->interruptSpell();
        castSpellOnSelf(SPELL_VISAGE_COALESCE_VOID, true);
    }

protected:
    void onEngage() override
    {
        mCoalescing = false;
        scriptEvents.addEvent(EVENT_CRUCIBLE_VISAGE_ECHO, 3000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mCoalescing && _getHealthPercent() >= 100)
        {
            mCoalescing = false;
            _removeAura(SPELL_VISAGE_COALESCE_VOID);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_CRUCIBLE_VISAGE_ECHO)
            return;

        // a coalescing Visage waits until it is whole again
        if (mCoalescing)
        {
            scriptEvents.addEvent(EVENT_CRUCIBLE_VISAGE_ECHO, 3000);
            return;
        }

        warn("A Visage from Beyond begins a Terrifying Echo!");
        castSpellAOE(SPELL_VISAGE_TERRIFYING_ECHO);
        scriptEvents.addEvent(EVENT_CRUCIBLE_VISAGE_ECHO, 25000);
    }

private:
    bool mCoalescing = false;
};

// consumed once Witness the End completes
class CrucibleEldritchAbominationAI : public CrucibleCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CrucibleEldritchAbominationAI(c); }
    explicit CrucibleEldritchAbominationAI(Creature* pCreature) : CrucibleCombatAI(pCreature) {}

    void OnCastSpell(uint32_t spellId) override
    {
        if (spellId == SPELL_ABOMINATION_WITNESS_THE_END)
            despawn(500, 0);
    }

protected:
    void onEngage() override
    {
        setRooted(true);
        scriptEvents.addEvent(EVENT_CRUCIBLE_ABOMINATION_WITNESS, 2000);
    }

    void onDisengage() override { setRooted(false); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_CRUCIBLE_ABOMINATION_WITNESS)
            return;

        castSpellAOE(SPELL_ABOMINATION_WITNESS_THE_END);
        scriptEvents.addEvent(EVENT_CRUCIBLE_ABOMINATION_WITNESS, 12000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Uu'nat, Harbinger of the Void

class CrucibleUunatAI : public CrucibleBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CrucibleUunatAI(c); }
    explicit CrucibleUunatAI(Creature* pCreature) : CrucibleBossAI(pCreature, DATA_UUNAT) {}

protected:
    void onEngage() override
    {
        armBerserk(CrucibleBerserkMs);
        startStage(1);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        int32_t const health = _getHealthPercent();
        if (mStage == 1 && health <= UunatSecondStagePct)
        {
            warn("Uu'nat calls upon N'Zoth's dutiful servants!");
            startStage(2);
        }
        else if (mStage == 2 && health <= UunatThirdStagePct)
        {
            warn("Uu'nat turns N'Zoth's unwavering gaze upon you!");
            startStage(3);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CRUCIBLE_UUNAT_TOUCH:
                castSpellOnVictim(SPELL_UUNAT_TOUCH_OF_THE_END);
                scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_TOUCH, 25000);
                break;
            case EVENT_CRUCIBLE_UUNAT_TEAR:
                castUnderPlayer(SPELL_UUNAT_OBLIVION_TEAR, false, false);
                scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_TEAR, 17000);
                break;
            case EVENT_CRUCIBLE_UUNAT_EYES:
                warn("The Eyes of N'Zoth open!");
                castUnderPlayer(SPELL_UUNAT_MADDENING_EYES, true, false);
                castAtPlayers(SPELL_UUNAT_PIERCING_GAZE, 2);
                scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_EYES, 32000);
                break;
            case EVENT_CRUCIBLE_UUNAT_VOID_CRASH:
                castUnderPlayer(SPELL_UUNAT_VOID_CRASH, false, true);
                scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_VOID_CRASH, 31000);
                break;
            case EVENT_CRUCIBLE_UUNAT_GIFT:
                castGiftOfNzoth();
                scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_GIFT, 45000);
                break;
            case EVENT_CRUCIBLE_UUNAT_MINDBENDER:
                callAdd(NPC_UUNAT_PRIMORDIAL_MINDBENDER, aroundSelf(15.0f, 30.0f));
                scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_MINDBENDER, 40000);
                break;
            case EVENT_CRUCIBLE_UUNAT_TERROR:
                warn("An Unknowable Terror is drawn from the void!");
                callAdd(NPC_UUNAT_UNKNOWABLE_TERROR, aroundSelf(10.0f, 25.0f));
                scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_TERROR, 50000);
                break;
            case EVENT_CRUCIBLE_UUNAT_TORMENT:
                castAtPlayer(SPELL_UUNAT_INSATIABLE_TORMENT, true, true);
                scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_TORMENT, 45000);
                break;
            case EVENT_CRUCIBLE_UUNAT_DEMISE:
                warn("The Eyes of N'Zoth sweep the field!");
                castAtPlayers(SPELL_UUNAT_PIERCING_GAZE, 3);
                scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_DEMISE, 32000);
                break;
            default:
                break;
        }
    }

private:
    void startStage(uint32_t stage)
    {
        enterStage(stage);

        warn("An Undying Guardian rises to shield Uu'nat!");
        callAdd(NPC_UUNAT_UNDYING_GUARDIAN, aroundSelf(8.0f, 15.0f));

        scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_TOUCH, 10000);
        scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_TEAR, 13000);
        scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_GIFT, 40000);

        switch (stage)
        {
            case 1:
                scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_EYES, 20000);
                if (isHeroic())
                    scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_VOID_CRASH, 27000);
                break;
            case 2:
                scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_MINDBENDER, 5000);
                scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_TERROR, 25000);
                break;
            default:
                scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_TORMENT, 8000);
                if (isHeroic())
                    scriptEvents.addEvent(EVENT_CRUCIBLE_UUNAT_DEMISE, 20000);
                break;
        }
    }

    // each stage has its own Gift of N'Zoth
    void castGiftOfNzoth()
    {
        warn("Uu'nat calls upon the Gift of N'Zoth!");
        if (mStage == 1)
            castSpellAOE(SPELL_UUNAT_GIFT_OBSCURITY);
        else if (mStage == 2)
            castSpellAOE(SPELL_UUNAT_GIFT_HYSTERIA);
        else
            castSpellAOE(SPELL_UUNAT_GIFT_LUNACY);
    }
};

// shrouded by whispers, regenerates once from 1 health and falls the second time
class CrucibleUndyingGuardianAI : public CrucibleCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CrucibleUndyingGuardianAI(c); }
    explicit CrucibleUndyingGuardianAI(Creature* pCreature) : CrucibleCombatAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mRegenerated)
            return;

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        mRegenerated = true;
        mRegenerating = true;
        castSpellOnSelf(SPELL_GUARDIAN_UNNATURAL_REGENERATION, true);
    }

protected:
    void onEngage() override
    {
        mRegenerated = false;
        mRegenerating = false;
        castSpellOnSelf(SPELL_GUARDIAN_SHROUD_OF_WHISPERS, true);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mRegenerating && _getHealthPercent() >= 100)
        {
            mRegenerating = false;
            _removeAura(SPELL_GUARDIAN_UNNATURAL_REGENERATION);
        }
    }

private:
    bool mRegenerated = false;
    bool mRegenerating = false;
};

// its remains spew Swarming Voidspawns
class CruciblePrimordialMindbenderAI : public CrucibleAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CruciblePrimordialMindbenderAI(c); }
    explicit CruciblePrimordialMindbenderAI(Creature* pCreature)
        : CrucibleAddAI(pCreature, EVENT_CRUCIBLE_MINDBENDER_ESSENCE, SPELL_MINDBENDER_CONSUME_ESSENCE, 3000, 20000) {}

    void OnDied(Unit* /*pKiller*/) override
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr)
            return;

        // Uu'nat owns the swarm so it goes away with his fight
        for (Creature* pUunat : pInstance->getCreatureSetForEntry(BOSS_UUNAT))
        {
            if (!pUunat->isAlive())
                continue;

            for (uint8_t i = 0; i < 3; ++i)
            {
                Creature* pSpawn = pUunat->summonCreature(NPC_UUNAT_SWARMING_VOIDSPAWN, crucibleRingPoint(getCreature()->GetPosition(), 1.0f, 4.0f), CORPSE_DESPAWN);
                if (pSpawn != nullptr)
                    setZoneWideCombat(pSpawn);
            }
            break;
        }
    }
};

class CrucibleUnknowableTerrorAI : public CrucibleAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CrucibleUnknowableTerrorAI(c); }
    explicit CrucibleUnknowableTerrorAI(Creature* pCreature)
        : CrucibleAddAI(pCreature, EVENT_CRUCIBLE_TERROR_FEAR, SPELL_TERROR_UNKNOWABLE_TERROR, 3000, 20000) {}
};

void SetupCrucibleOfStorms(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_CRUCIBLE_OF_STORMS, &CrucibleOfStormsInstanceScript::Create);

    // The Restless Cabal
    mgr->register_creature_script(BOSS_ZAXASJ_THE_SPEAKER, &CrucibleZaxasjAI::Create);
    mgr->register_creature_script(BOSS_FATHUUL_THE_FEARED, &CrucibleFathuulAI::Create);
    mgr->register_creature_script(NPC_CABAL_VISAGE_FROM_BEYOND, &CrucibleVisageFromBeyondAI::Create);
    mgr->register_creature_script(NPC_CABAL_ELDRITCH_ABOMINATION, &CrucibleEldritchAbominationAI::Create);

    // Uu'nat, Harbinger of the Void
    mgr->register_creature_script(BOSS_UUNAT, &CrucibleUunatAI::Create);
    mgr->register_creature_script(NPC_UUNAT_UNDYING_GUARDIAN, &CrucibleUndyingGuardianAI::Create);
    mgr->register_creature_script(NPC_UUNAT_PRIMORDIAL_MINDBENDER, &CruciblePrimordialMindbenderAI::Create);
    mgr->register_creature_script(NPC_UUNAT_UNKNOWABLE_TERROR, &CrucibleUnknowableTerrorAI::Create);
}
