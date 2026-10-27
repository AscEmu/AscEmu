/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Cinderbrew_Meadery.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

enum CinderbrewMeaderyEvents
{
    EVENT_ALDRYR_KEG_SMASH = 1,
    EVENT_ALDRYR_THROW_CINDERBREW,
    EVENT_ALDRYR_BLAZING_BELCH,

    EVENT_IPA_BOTTOMS_UPPERCUT,
    EVENT_IPA_BURNING_FERMENTATION,
    EVENT_IPA_FERMENTATION_SPREAD,
    EVENT_IPA_SPOUTING_STOUT,

    EVENT_GOLDIE_SPREAD_THE_LOVE,
    EVENT_GOLDIE_CASH_CANNON,
    EVENT_GOLDIE_CANNON_IGNITE,
    EVENT_GOLDIE_BURNING_RICOCHET,
    EVENT_GOLDIE_RICOCHET_IGNITE,

    EVENT_BENK_SNACK_TIME,
    EVENT_BENK_HONEY_MARINADE,
    EVENT_BENK_FLUTTERING_WING,
    EVENT_BENK_CINDERBEE_STING
};

namespace
{
    float const TwoPi = 6.28318531f;
    float const FightReach = 60.0f;

    // stools at Aldryr's counter
    LocationVector const PatronStools[4] =
    {
        { 2662.5f, -4948.4f, 99.7f, 4.71f },
        { 2655.7f, -4947.0f, 99.7f, 4.71f },
        { 2645.8f, -4947.0f, 99.7f, 4.71f },
        { 2637.6f, -4951.1f, 99.7f, 4.71f }
    };

    int32_t const HappyHourMarks[2] = { 66, 33 };
    uint32_t const HappyHourMaxMs = 30000;
    uint32_t const PatronCheckMs = 1000;

    float const BrewDropArriveRange = 3.0f;
    uint32_t const BrewDropStepMs = 1000;

    uint32_t const GoldieEnergyPerSecond = 2;
    float const CannonIgniteRange = 8.0f;
    float const RicochetIgniteRange = 6.0f;
    float const BombBlastRange = 8.0f;

    uint32_t const TamedBeeLingerMs = 15000;

    LocationVector pointAround(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(TwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    std::vector<Unit*> playersNear(Creature* pCreature, LocationVector const& spot, float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : pCreature->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pPlayer->getDistance(spot) <= range)
                players.push_back(pPlayer);
        }

        return players;
    }

    // energy kept by the script, the bar is full at 100
    struct EnergyBar
    {
        uint32_t value = 0;
        uint32_t clockMs = 0;

        void reset()
        {
            value = 0;
            clockMs = 0;
        }

        bool fill(uint32_t diff, uint32_t perSecond)
        {
            clockMs += diff;
            while (clockMs >= 1000)
            {
                clockMs -= 1000;
                value = value + perSecond >= 100 ? 100 : value + perSecond;
            }

            return value >= 100;
        }
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class CinderbrewMeaderyInstanceScript : public InstanceScript
{
public:
    explicit CinderbrewMeaderyInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(CinderbrewMeaderyEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new CinderbrewMeaderyInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock and target helpers

class CinderbrewCombatAI : public CreatureAIScript
{
public:
    explicit CinderbrewCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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
    virtual void onReset() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    Unit* victim() { return getCreature()->getAIInterface()->getCurrentTarget(); }

    // distinct random players in reach, the tank only when nobody else is left
    std::vector<Unit*> pickPlayers(size_t count, bool skipTank = false)
    {
        Unit* pTank = skipTank ? victim() : nullptr;
        std::vector<Unit*> pool;
        for (Unit* pPlayer : playersNear(getCreature(), getCreature()->GetPosition(), FightReach))
        {
            if (pPlayer != pTank)
                pool.push_back(pPlayer);
        }

        if (pool.empty() && pTank != nullptr && pTank->isAlive())
            pool.push_back(pTank);

        for (size_t i = 0; i < pool.size() && i < count; ++i)
        {
            size_t const pick = i + Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1 - i));
            std::swap(pool[i], pool[pick]);
        }

        if (pool.size() > count)
            pool.resize(count);

        return pool;
    }

    Unit* randomPlayer(bool skipTank = false)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipTank);
        return picked.empty() ? nullptr : picked.front();
    }

    Unit* castAtRandomPlayer(uint32_t spellId, bool skipTank = false, bool triggered = false)
    {
        Unit* pTarget = randomPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    std::vector<Creature*> livingSummons(uint32_t entry)
    {
        std::vector<Creature*> found;
        for (uint64_t const guid : summons)
        {
            Creature* pSummon = getCreature()->getWorldMapCreature(guid);
            if (pSummon != nullptr && pSummon->getEntry() == entry && pSummon->isAlive())
                found.push_back(pSummon);
        }

        return found;
    }

    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class CinderbrewBossAI : public CinderbrewCombatAI
{
public:
    CinderbrewBossAI(Creature* pCreature, uint32_t dataId) : CinderbrewCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        CinderbrewCombatAI::OnCombatStart(pTarget);
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

        CinderbrewCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
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
// Brew Master Aldryr, Happy Hour at 66% and 33%

class CinderbrewAldryrAI : public CinderbrewBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CinderbrewAldryrAI(c); }
    explicit CinderbrewAldryrAI(Creature* pCreature) : CinderbrewBossAI(pCreature, DATA_BREWMASTER_ALDRYR) {}

protected:
    void onEngage() override
    {
        mHappyHoursCalled = 0;
        mHappyHour = false;
        mHappyHourMs = 0;
        mPatronCheckMs = 0;

        scriptEvents.addEvent(EVENT_ALDRYR_KEG_SMASH, 7000);
        scriptEvents.addEvent(EVENT_ALDRYR_THROW_CINDERBREW, 11000);
        scriptEvents.addEvent(EVENT_ALDRYR_BLAZING_BELCH, 18000);
    }

    void onReset() override
    {
        mHappyHour = false;
        _removeAura(SPELL_ALDRYR_HAPPY_HOUR);
        _removeAura(SPELL_ALDRYR_DISREGARD);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mHappyHour)
        {
            mHappyHourMs += diff;
            mPatronCheckMs += diff;
            if (mHappyHourMs >= HappyHourMaxMs)
            {
                closeHappyHour();
                return;
            }

            // every patron appeased ends it early
            if (mPatronCheckMs >= PatronCheckMs)
            {
                mPatronCheckMs = 0;
                if (livingSummons(NPC_ALDRYR_THIRSTY_PATRON).empty())
                    closeHappyHour();
            }
            return;
        }

        if (mHappyHoursCalled < 2 && _getHealthPercent() <= HappyHourMarks[mHappyHoursCalled])
            openHappyHour();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ALDRYR_KEG_SMASH:
                castSpellOnVictim(SPELL_ALDRYR_KEG_SMASH);
                scriptEvents.addEvent(EVENT_ALDRYR_KEG_SMASH, 17000);
                break;
            case EVENT_ALDRYR_THROW_CINDERBREW:
                castAtRandomPlayer(SPELL_ALDRYR_THROW_CINDERBREW, true);
                scriptEvents.addEvent(EVENT_ALDRYR_THROW_CINDERBREW, 15000);
                break;
            case EVENT_ALDRYR_BLAZING_BELCH:
                if (castAtRandomPlayer(SPELL_ALDRYR_BLAZING_BELCH) != nullptr)
                    warn("Brew Master Aldryr winds up a Blazing Belch!");
                scriptEvents.addEvent(EVENT_ALDRYR_BLAZING_BELCH, 23000);
                break;
            default:
                break;
        }
    }

private:
    // thirsty patrons take the stools and yell until appeased
    void openHappyHour()
    {
        ++mHappyHoursCalled;
        mHappyHour = true;
        mHappyHourMs = 0;
        mPatronCheckMs = 0;

        warn("Brew Master Aldryr declares Happy Hour! Appease the Thirsty Patrons!");
        castSpellOnSelf(SPELL_ALDRYR_HAPPY_HOUR, true);

        uint8_t const patrons = isHeroic() ? 4 : 3;
        for (uint8_t i = 0; i < patrons; ++i)
            summonCreature(NPC_ALDRYR_THIRSTY_PATRON, PatronStools[i], CORPSE_DESPAWN);
    }

    void closeHappyHour()
    {
        mHappyHour = false;
        _removeAura(SPELL_ALDRYR_HAPPY_HOUR);
        _removeAura(SPELL_ALDRYR_DISREGARD);
        summons.despawnEntry(NPC_ALDRYR_THIRSTY_PATRON);
    }

    uint8_t mHappyHoursCalled = 0;
    bool mHappyHour = false;
    uint32_t mHappyHourMs = 0;
    uint32_t mPatronCheckMs = 0;
};

// seated at the counter, yells for a drink
class CinderbrewThirstyPatronAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CinderbrewThirstyPatronAI(c); }
    explicit CinderbrewThirstyPatronAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_ALDRYR_PATRON_ROWDY_YELL, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// I'pa, refilled by every Brew Drop that reaches him

class CinderbrewIpaAI : public CinderbrewBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CinderbrewIpaAI(c); }
    explicit CinderbrewIpaAI(Creature* pCreature) : CinderbrewBossAI(pCreature, DATA_IPA) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_IPA_BREW_ARRIVED && isAlive() && _isInCombat())
            castSpellOnSelf(SPELL_IPA_FILL_ER_UP, true);
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_IPA_BOTTOMS_UPPERCUT, 6000);
        scriptEvents.addEvent(EVENT_IPA_BURNING_FERMENTATION, 12000);
        scriptEvents.addEvent(EVENT_IPA_SPOUTING_STOUT, 18000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_IPA_BOTTOMS_UPPERCUT:
                castSpellOnVictim(SPELL_IPA_BOTTOMS_UPPERCUT);
                scriptEvents.addEvent(EVENT_IPA_BOTTOMS_UPPERCUT, 14000);
                break;
            case EVENT_IPA_BURNING_FERMENTATION:
                castSpellAOE(SPELL_IPA_BURNING_FERMENTATION);
                scriptEvents.addEvent(EVENT_IPA_FERMENTATION_SPREAD, 300);
                scriptEvents.addEvent(EVENT_IPA_BURNING_FERMENTATION, 20000);
                break;
            case EVENT_IPA_FERMENTATION_SPREAD:
                for (Unit* pTarget : pickPlayers(2))
                    castSpell(pTarget, SPELL_IPA_BURNING_FERMENTATION_DOT, true);
                break;
            case EVENT_IPA_SPOUTING_STOUT:
                spoutStout();
                scriptEvents.addEvent(EVENT_IPA_SPOUTING_STOUT, 32000);
                break;
            default:
                break;
        }
    }

private:
    void spoutStout()
    {
        warn("I'pa erupts with Spouting Stout! Stop the Brew Drops!");
        castSpellOnSelf(SPELL_IPA_SPOUTING_STOUT);

        uint8_t const drops = isHeroic() ? 4 : 3;
        for (uint8_t i = 0; i < drops; ++i)
            summonCreature(NPC_IPA_BREW_DROP, pointAround(getCreature()->GetPosition(), 12.0f, 18.0f), CORPSE_DESPAWN);
    }
};

// rolls back to I'pa unless it is popped on the way
class CinderbrewBrewDropAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CinderbrewBrewDropAI(c); }
    explicit CinderbrewBrewDropAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_IPA_BREW_DROP_FROTHY, true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive() || mArrived)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mStepMs > diff)
        {
            mStepMs -= diff;
            return;
        }
        mStepMs = BrewDropStepMs;

        Creature* pIpa = findNearestCreature(BOSS_IPA, 100.0f);
        if (pIpa == nullptr || !pIpa->isAlive())
            return;

        if (getCreature()->getDistance(pIpa) <= BrewDropArriveRange)
        {
            mArrived = true;
            if (pIpa->GetScript() != nullptr)
                pIpa->GetScript()->DoAction(ACTION_IPA_BREW_ARRIVED);

            despawn(500, 0);
            return;
        }

        moveTo(pIpa->GetPositionX(), pIpa->GetPositionY(), pIpa->GetPositionZ(), false);
    }

private:
    uint32_t mStepMs = 0;
    bool mArrived = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Goldie Baronbottom, Let It Hail! at full energy

class CinderbrewGoldieAI : public CinderbrewBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CinderbrewGoldieAI(c); }
    explicit CinderbrewGoldieAI(Creature* pCreature) : CinderbrewBossAI(pCreature, DATA_GOLDIE_BARONBOTTOM) {}

protected:
    void onEngage() override
    {
        mEnergy.reset();
        scriptEvents.addEvent(EVENT_GOLDIE_SPREAD_THE_LOVE, 6000);
        scriptEvents.addEvent(EVENT_GOLDIE_BURNING_RICOCHET, 9000);
        scriptEvents.addEvent(EVENT_GOLDIE_CASH_CANNON, 12000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mEnergy.fill(diff, GoldieEnergyPerSecond) || _isCasting())
            return;

        mEnergy.reset();
        warn("Goldie Baronbottom shouts: Let It Hail!");
        castSpellOnSelf(SPELL_GOLDIE_LET_IT_HAIL);
        igniteBombs(getCreature()->GetPosition(), 500.0f);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GOLDIE_SPREAD_THE_LOVE:
            {
                warn("Goldie Baronbottom spreads the love with Cinderbrew Bombs!");
                uint8_t const bombs = isHeroic() ? 5 : 4;
                for (uint8_t i = 0; i < bombs; ++i)
                    summonCreature(NPC_GOLDIE_CINDERBREW_BOMB, pointAround(getCreature()->GetPosition(), 6.0f, 18.0f), CORPSE_DESPAWN);
                scriptEvents.addEvent(EVENT_GOLDIE_SPREAD_THE_LOVE, 25000);
                break;
            }
            case EVENT_GOLDIE_CASH_CANNON:
                if (Unit* pTarget = victim())
                {
                    mCannonSpot = pTarget->GetPosition();
                    castSpell(pTarget, SPELL_GOLDIE_CASH_CANNON);
                    scriptEvents.addEvent(EVENT_GOLDIE_CANNON_IGNITE, 200);
                }
                scriptEvents.addEvent(EVENT_GOLDIE_CASH_CANNON, 16000);
                break;
            case EVENT_GOLDIE_CANNON_IGNITE:
                igniteBombs(mCannonSpot, CannonIgniteRange);
                break;
            case EVENT_GOLDIE_BURNING_RICOCHET:
                if (Unit* pTarget = castAtRandomPlayer(SPELL_GOLDIE_BURNING_RICOCHET))
                {
                    mRicochetSpot = pTarget->GetPosition();
                    scriptEvents.addEvent(EVENT_GOLDIE_RICOCHET_IGNITE, 200);
                }
                scriptEvents.addEvent(EVENT_GOLDIE_BURNING_RICOCHET, 18000);
                break;
            case EVENT_GOLDIE_RICOCHET_IGNITE:
                igniteBombs(mRicochetSpot, RicochetIgniteRange);
                break;
            default:
                break;
        }
    }

private:
    void igniteBombs(LocationVector const& spot, float range)
    {
        for (Creature* pBomb : livingSummons(NPC_GOLDIE_CINDERBREW_BOMB))
        {
            if (pBomb->getDistance(spot) <= range && pBomb->GetScript() != nullptr)
                pBomb->GetScript()->DoAction(ACTION_GOLDIE_BOMB_IGNITE);
        }
    }

    EnergyBar mEnergy;
    LocationVector mCannonSpot;
    LocationVector mRicochetSpot;
};

class CinderbrewGoldieBombAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CinderbrewGoldieBombAI(c); }
    explicit CinderbrewGoldieBombAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_GOLDIE_BOMB_IGNITE || mSpent || !isAlive())
            return;

        mSpent = true;
        for (Unit* pPlayer : playersNear(getCreature(), getCreature()->GetPosition(), BombBlastRange))
            castSpell(pPlayer, SPELL_GOLDIE_BOMB_CINDER_BOOM, true);

        despawn(300, 0);
    }

private:
    bool mSpent = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Benk Buzzbee, Snack Time draws in the Cinderbees

class CinderbrewBenkAI : public CinderbrewBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CinderbrewBenkAI(c); }
    explicit CinderbrewBenkAI(Creature* pCreature) : CinderbrewBossAI(pCreature, DATA_BENK_BUZZBEE) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_BENK_SNACK_TIME, 5000);
        scriptEvents.addEvent(EVENT_BENK_HONEY_MARINADE, 9000);
        scriptEvents.addEvent(EVENT_BENK_FLUTTERING_WING, 15000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BENK_SNACK_TIME:
            {
                warn("Benk Buzzbee rolls out the barrels for Snack Time! Ravenous Cinderbees swarm in!");
                castSpellAOE(SPELL_BENK_SNACK_TIME, true);
                uint8_t const bees = isHeroic() ? 3 : 2;
                for (uint8_t i = 0; i < bees; ++i)
                    callAdd(NPC_BENK_RAVENOUS_CINDERBEE, pointAround(getCreature()->GetPosition(), 10.0f, 16.0f));
                scriptEvents.addEvent(EVENT_BENK_SNACK_TIME, 30000);
                break;
            }
            case EVENT_BENK_HONEY_MARINADE:
                castAtRandomPlayer(SPELL_BENK_HONEY_MARINADE);
                scriptEvents.addEvent(EVENT_BENK_HONEY_MARINADE, 17000);
                break;
            case EVENT_BENK_FLUTTERING_WING:
                castSpellAOE(SPELL_BENK_FLUTTERING_WING);
                scriptEvents.addEvent(EVENT_BENK_FLUTTERING_WING, 24000);
                break;
            default:
                break;
        }
    }
};

// at 1 health it is tamed instead of slain
class CinderbrewRavenousCinderbeeAI : public CinderbrewCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CinderbrewRavenousCinderbeeAI(c); }
    explicit CinderbrewRavenousCinderbeeAI(Creature* pCreature) : CinderbrewCombatAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        uint32_t const health = getCreature()->getHealth();
        if (mTamed)
        {
            *damage = 0;
            return;
        }

        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        becomeTamed();
    }

protected:
    void onEngage() override
    {
        if (!mTamed)
            scriptEvents.addEvent(EVENT_BENK_CINDERBEE_STING, 3000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_BENK_CINDERBEE_STING || mTamed)
            return;

        castAtRandomPlayer(SPELL_BENK_CINDERBEE_SHREDDING_STING);
        scriptEvents.addEvent(EVENT_BENK_CINDERBEE_STING, Util::getRandomUInt(6000, 8000));
    }

private:
    void becomeTamed()
    {
        mTamed = true;
        scriptEvents.resetEvents();
        getCreature()->interruptSpell();

        castSpellOnSelf(SPELL_BENK_CINDERBEE_TAMED, true);
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        _clearHateList();
        despawn(TamedBeeLingerMs, 0);
    }

    bool mTamed = false;
};

void SetupCinderbrewMeadery(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_CINDERBREW_MEADERY, &CinderbrewMeaderyInstanceScript::Create);

    mgr->register_creature_script(BOSS_BREWMASTER_ALDRYR, &CinderbrewAldryrAI::Create);
    mgr->register_creature_script(NPC_ALDRYR_THIRSTY_PATRON, &CinderbrewThirstyPatronAI::Create);

    mgr->register_creature_script(BOSS_IPA, &CinderbrewIpaAI::Create);
    mgr->register_creature_script(NPC_IPA_BREW_DROP, &CinderbrewBrewDropAI::Create);

    mgr->register_creature_script(BOSS_GOLDIE_BARONBOTTOM, &CinderbrewGoldieAI::Create);
    mgr->register_creature_script(NPC_GOLDIE_CINDERBREW_BOMB, &CinderbrewGoldieBombAI::Create);

    mgr->register_creature_script(BOSS_BENK_BUZZBEE, &CinderbrewBenkAI::Create);
    mgr->register_creature_script(NPC_BENK_RAVENOUS_CINDERBEE, &CinderbrewRavenousCinderbeeAI::Create);
}
