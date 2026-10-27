/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Eco_Dome_Aldani.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <map>
#include <utility>
#include <vector>

enum EcoDomeAldaniEvents
{
    EVENT_AZHICCAR_THRASH = 1,
    EVENT_AZHICCAR_TOXIC_REGURGITATION,
    EVENT_AZHICCAR_REGURGITATION_SPLASH,
    EVENT_AZHICCAR_INVADING_SHRIEK,
    EVENT_AZHICCAR_FEAST,
    EVENT_AZHICCAR_MITE_HOP,

    EVENT_AWAZJ_RIFT_CLAWS,
    EVENT_AWAZJ_BINDING_JAVELIN,
    EVENT_AWAZJ_JAVELIN_LANDS,
    EVENT_AWAZJ_WARP_STRIKE,
    EVENT_AWAZJ_BLITZ_WARP_STRIKE,

    EVENT_TAAHBAT_ARCANE_OVERLOAD,
    EVENT_TAAHBAT_BLITZ_FADES,
    EVENT_TAAHBAT_REMOUNT,

    EVENT_SOUL_SCRIBE_WHISPERS_OF_FATE,
    EVENT_SOUL_SCRIBE_WEAVE_RELEASE,
    EVENT_SOUL_SCRIBE_CEREMONIAL_DAGGER,
    EVENT_SOUL_SCRIBE_DAGGER_STRIKE,
    EVENT_SOUL_SCRIBE_DREAD,
    EVENT_SOUL_SCRIBE_DREAD_DETONATES
};

namespace
{
    float const TwoPi = 6.28318531f;
    float const FightReach = 60.0f;

    uint32_t const PairEntries[] = { BOSS_TAAHBAT, BOSS_AWAZJ };

    // Azhiccar
    uint32_t const AzhiccarEnergyPerSecond = 2;
    float const FeastMiteRange = 15.0f;
    float const FeastPlayerRange = 10.0f;
    uint32_t const FeastHealPct = 3;

    // Taah'bat and A'wazj
    uint32_t const TaahbatEnergyPerSecond = 3;
    uint32_t const RiderFollowMs = 1000;
    float const RiderSeatRange = 3.0f;
    float const WarpStrikeReach = 8.0f;
    uint32_t const BlitzMaxMs = 30000;
    uint32_t const DestabilizedMs = 10000;
    uint32_t const JavelinPulseMs = 2000;

    // Soul-Scribe
    uint32_t const SoulScribeEnergyPerSecond = 2;
    float const DaggerReach = 5.0f;
    float const DreadReach = 8.0f;

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

    // a summoned object and the player it belongs to
    struct BoundPair
    {
        uint64_t objectGuid;
        uint64_t playerGuid;
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class EcoDomeAldaniInstanceScript : public InstanceScript
{
public:
    explicit EcoDomeAldaniInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(EcoDomeAldaniEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new EcoDomeAldaniInstanceScript(pMapMgr); }

    // the rider or the mount falling alone must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (uint32_t entry : PairEntries)
        {
            if (entry == pCreature->getEntry())
            {
                mMemberGuids[entry] = pCreature->GetNewGUID();
                return;
            }
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (state != EncounterStates::Failed)
            return;

        // a wiped pair fights again in full
        for (Creature* pMember : getGroup(id))
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    Creature* getMember(uint32_t entry)
    {
        auto const itr = mMemberGuids.find(entry);
        return itr != mMemberGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    std::vector<Creature*> getGroup(uint32_t dataId)
    {
        std::vector<Creature*> group;
        if (dataId != DATA_TAAHBAT_AND_AWAZJ)
            return group;

        for (uint32_t entry : PairEntries)
        {
            if (Creature* pMember = getMember(entry))
                group.push_back(pMember);
        }

        return group;
    }

    bool isGroupDown(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    bool isGroupFighting(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat())
                return true;
        }

        return false;
    }

    // pulling one pulls the other
    void rallyGroup(uint32_t dataId, Creature* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember == pPuller || !pMember->isAlive() || pMember->isInCombat() || pMember->GetScript() == nullptr)
                continue;

            pMember->GetScript()->setZoneWideCombat();
        }
        mRallying = false;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        Creature* pMember = getMember(entry);
        if (pMember != nullptr && pMember->isAlive() && pMember->GetScript() != nullptr)
            pMember->GetScript()->DoAction(action);
    }

private:
    std::map<uint32_t, WoWGuid> mMemberGuids;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock and target helpers

class EcoDomeCombatAI : public CreatureAIScript
{
public:
    explicit EcoDomeCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    EcoDomeAldaniInstanceScript* getEcoDome()
    {
        if (getCreature()->GetMapId() != MAP_ECO_DOME_ALDANI)
            return nullptr;

        return static_cast<EcoDomeAldaniInstanceScript*>(getInstanceScript());
    }

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

    Unit* unitByGuid(uint64_t guid)
    {
        Unit* pUnit = guid != 0 ? getCreature()->getWorldMapUnit(guid) : nullptr;
        return pUnit != nullptr && pUnit->isAlive() ? pUnit : nullptr;
    }

    void healPct(uint32_t pct)
    {
        Creature* pSelf = getCreature();
        uint64_t const healed = static_cast<uint64_t>(pSelf->getHealth()) + static_cast<uint64_t>(pSelf->getMaxHealth()) * pct / 100;
        pSelf->setHealth(healed >= pSelf->getMaxHealth() ? pSelf->getMaxHealth() : static_cast<uint32_t>(healed));
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class EcoDomeBossAI : public EcoDomeCombatAI
{
public:
    EcoDomeBossAI(Creature* pCreature, uint32_t dataId) : EcoDomeCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (EcoDomeAldaniInstanceScript* pEcoDome = getEcoDome())
            pEcoDome->rallyGroup(mDataId, getCreature());

        EcoDomeCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last partner has left the fight
                EcoDomeAldaniInstanceScript* pEcoDome = getEcoDome();
                if (pEcoDome == nullptr || !pEcoDome->isGroupFighting(mDataId, getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        EcoDomeCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            EcoDomeAldaniInstanceScript* pEcoDome = getEcoDome();
            if (pEcoDome == nullptr || pEcoDome->isGroupDown(mDataId, getCreature()))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Azhiccar, Devour at full energy

class EcoDomeAzhiccarAI : public EcoDomeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EcoDomeAzhiccarAI(c); }
    explicit EcoDomeAzhiccarAI(Creature* pCreature) : EcoDomeBossAI(pCreature, DATA_AZHICCAR) {}

protected:
    void onEngage() override
    {
        mEnergy.reset();
        scriptEvents.addEvent(EVENT_AZHICCAR_THRASH, 5000);
        scriptEvents.addEvent(EVENT_AZHICCAR_TOXIC_REGURGITATION, 7000);
        scriptEvents.addEvent(EVENT_AZHICCAR_INVADING_SHRIEK, 12000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mEnergy.fill(diff, AzhiccarEnergyPerSecond) || _isCasting())
            return;

        mEnergy.reset();
        warn("Azhiccar tries to Devour everything around it!");
        castSpellOnSelf(SPELL_AZHICCAR_DEVOUR);
        scriptEvents.addEvent(EVENT_AZHICCAR_FEAST, 500);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AZHICCAR_THRASH:
                castSpellAOE(SPELL_AZHICCAR_THRASH);
                scriptEvents.addEvent(EVENT_AZHICCAR_THRASH, 10000);
                break;
            case EVENT_AZHICCAR_TOXIC_REGURGITATION:
                castSpellAOE(SPELL_AZHICCAR_TOXIC_REGURGITATION);
                scriptEvents.addEvent(EVENT_AZHICCAR_REGURGITATION_SPLASH, 300);
                scriptEvents.addEvent(EVENT_AZHICCAR_TOXIC_REGURGITATION, 15000);
                break;
            case EVENT_AZHICCAR_REGURGITATION_SPLASH:
                for (Unit* pTarget : pickPlayers(2))
                    castSpell(pTarget, SPELL_AZHICCAR_TOXIC_REGURGITATION_DOT, true);
                break;
            case EVENT_AZHICCAR_INVADING_SHRIEK:
            {
                warn("Azhiccar lets out an Invading Shriek! Frenzied Mites swarm in!");
                castSpellOnSelf(SPELL_AZHICCAR_INVADING_SHRIEK);
                uint8_t const mites = isHeroic() ? 5 : 4;
                for (uint8_t i = 0; i < mites; ++i)
                    callAdd(NPC_AZHICCAR_FRENZIED_MITE, pointAround(getCreature()->GetPosition(), 10.0f, 16.0f));
                scriptEvents.addEvent(EVENT_AZHICCAR_INVADING_SHRIEK, 30000);
                break;
            }
            case EVENT_AZHICCAR_FEAST:
                feast();
                break;
            default:
                break;
        }
    }

private:
    // every mite and player drawn in feeds him
    void feast()
    {
        uint32_t meals = 0;

        for (Creature* pMite : livingSummons(NPC_AZHICCAR_FRENZIED_MITE))
        {
            if (getCreature()->getDistance(pMite) > FeastMiteRange)
                continue;

            pMite->despawn(200, 0);
            ++meals;
        }

        for (Unit* pPlayer : playersNear(getCreature(), getCreature()->GetPosition(), FeastPlayerRange))
        {
            castSpell(pPlayer, SPELL_AZHICCAR_FEAST, true);
            ++meals;
        }

        if (meals > 0)
            healPct(FeastHealPct * meals);
    }

    EnergyBar mEnergy;
};

// flies wildly from one player to the next
class EcoDomeFrenziedMiteAI : public EcoDomeCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EcoDomeFrenziedMiteAI(c); }
    explicit EcoDomeFrenziedMiteAI(Creature* pCreature) : EcoDomeCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_AZHICCAR_MITE_UNCONTROLLED, true);
        scriptEvents.addEvent(EVENT_AZHICCAR_MITE_HOP, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_AZHICCAR_MITE_HOP)
            return;

        if (Unit* pPrey = randomPlayer())
        {
            _clearHateList();
            addThreat(pPrey, 1000.0f);
            attackStart(pPrey);
        }
        scriptEvents.addEvent(EVENT_AZHICCAR_MITE_HOP, 6000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Taah'bat and A'wazj

// A'wazj carries Taah'bat into the fight and throws the javelins
class EcoDomeAwazjAI : public EcoDomeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EcoDomeAwazjAI(c); }
    explicit EcoDomeAwazjAI(Creature* pCreature) : EcoDomeBossAI(pCreature, DATA_TAAHBAT_AND_AWAZJ) {}

    void DoAction(int32_t action) override
    {
        if (!isAlive() || !_isInCombat())
            return;

        switch (action)
        {
            case ACTION_TAAHBAT_BLITZ_BEGIN:
                mBlitzing = true;
                scriptEvents.addEvent(EVENT_AWAZJ_BLITZ_WARP_STRIKE, 3000);
                break;
            case ACTION_TAAHBAT_BLITZ_END:
                mBlitzing = false;
                scriptEvents.removeEvent(EVENT_AWAZJ_BLITZ_WARP_STRIKE);
                break;
            case ACTION_TAAHBAT_DESTABILIZED:
                mBlitzing = false;
                scriptEvents.removeEvent(EVENT_AWAZJ_BLITZ_WARP_STRIKE);
                castSpellOnSelf(SPELL_TAAHBAT_DESTABILIZED, true);
                break;
            default:
                break;
        }
    }

protected:
    void onEngage() override
    {
        mBlitzing = false;
        mJavelins.clear();
        mJavelinPulseMs = 0;

        castSpellOnSelf(SPELL_TAAHBAT_BEASTMASTERS_BOND, true);
        bringRider();

        scriptEvents.addEvent(EVENT_AWAZJ_RIFT_CLAWS, 5000);
        scriptEvents.addEvent(EVENT_AWAZJ_BINDING_JAVELIN, 10000);
        scriptEvents.addEvent(EVENT_AWAZJ_WARP_STRIKE, 15000);
    }

    void onReset() override { releaseJavelinVictims(); }

    void onDefeat() override
    {
        releaseJavelinVictims();
        if (EcoDomeAldaniInstanceScript* pEcoDome = getEcoDome())
            pEcoDome->sendAction(BOSS_TAAHBAT, ACTION_AWAZJ_FELL);
    }

    void onCombatTick(uint32_t diff) override
    {
        mJavelinPulseMs += diff;
        if (mJavelinPulseMs < JavelinPulseMs)
            return;
        mJavelinPulseMs = 0;

        // a standing javelin keeps hurting its player
        for (auto itr = mJavelins.begin(); itr != mJavelins.end();)
        {
            Creature* pJavelin = getCreature()->getWorldMapCreature(itr->objectGuid);
            Unit* pPlayer = unitByGuid(itr->playerGuid);
            if (pJavelin == nullptr || !pJavelin->isAlive() || pPlayer == nullptr)
            {
                if (pPlayer != nullptr)
                    pPlayer->removeAllAurasById(SPELL_AWAZJ_BINDING_JAVELIN_SNARE);

                itr = mJavelins.erase(itr);
                continue;
            }

            castSpell(pPlayer, SPELL_AWAZJ_BINDING_JAVELIN_PULSE, true);
            ++itr;
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AWAZJ_RIFT_CLAWS:
                castSpellOnVictim(SPELL_AWAZJ_RIFT_CLAWS);
                scriptEvents.addEvent(EVENT_AWAZJ_RIFT_CLAWS, 12000);
                break;
            case EVENT_AWAZJ_BINDING_JAVELIN:
                castSpellAOE(SPELL_AWAZJ_BINDING_JAVELIN);
                scriptEvents.addEvent(EVENT_AWAZJ_JAVELIN_LANDS, 300);
                scriptEvents.addEvent(EVENT_AWAZJ_BINDING_JAVELIN, 22000);
                break;
            case EVENT_AWAZJ_JAVELIN_LANDS:
                for (Unit* pTarget : pickPlayers(1, true))
                {
                    castSpell(pTarget, SPELL_AWAZJ_BINDING_JAVELIN_SNARE, true);
                    if (Creature* pJavelin = summonCreature(NPC_AWAZJ_BINDING_JAVELIN, pTarget->GetPosition(), CORPSE_DESPAWN))
                        mJavelins.push_back({ pJavelin->getGuid(), pTarget->getGuid() });
                }
                break;
            case EVENT_AWAZJ_WARP_STRIKE:
                if (!mBlitzing)
                    castAtRandomPlayer(SPELL_AWAZJ_WARP_STRIKE);
                scriptEvents.addEvent(EVENT_AWAZJ_WARP_STRIKE, 20000);
                break;
            case EVENT_AWAZJ_BLITZ_WARP_STRIKE:
                if (mBlitzing)
                {
                    blitzWarpStrike();
                    scriptEvents.addEvent(EVENT_AWAZJ_BLITZ_WARP_STRIKE, 4000);
                }
                break;
            default:
                break;
        }
    }

private:
    // the rider has no spawn of its own, it joins with the mount
    void bringRider()
    {
        EcoDomeAldaniInstanceScript* pEcoDome = getEcoDome();
        if (pEcoDome == nullptr)
            return;

        Creature* pRider = pEcoDome->getMember(BOSS_TAAHBAT);
        if (pRider == nullptr || !pRider->isAlive())
        {
            LocationVector spot = getCreature()->GetPosition();
            spot.x += 1.5f;
            pRider = spawnCreature(BOSS_TAAHBAT, spot);
        }

        if (pRider != nullptr && pRider->isAlive() && !pRider->isInCombat())
            setZoneWideCombat(pRider);
    }

    // a strike landing next to Taah'bat tears away one Incorporeal layer
    void blitzWarpStrike()
    {
        Unit* pTarget = randomPlayer();
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, SPELL_AWAZJ_BLITZ_WARP_STRIKE);

        EcoDomeAldaniInstanceScript* pEcoDome = getEcoDome();
        Creature* pRider = pEcoDome != nullptr ? pEcoDome->getMember(BOSS_TAAHBAT) : nullptr;
        if (pRider != nullptr && pRider->isAlive() && pRider->getDistance(pTarget) <= WarpStrikeReach)
            pEcoDome->sendAction(BOSS_TAAHBAT, ACTION_TAAHBAT_WARP_STRUCK);
    }

    void releaseJavelinVictims()
    {
        mJavelins.clear();
        _removeAuraOnPlayers(SPELL_AWAZJ_BINDING_JAVELIN_SNARE);
    }

    bool mBlitzing = false;
    std::vector<BoundPair> mJavelins;
    uint32_t mJavelinPulseMs = 0;
};

// rides A'wazj until the Arcane Blitz at full energy
class EcoDomeTaahbatAI : public EcoDomeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EcoDomeTaahbatAI(c); }
    explicit EcoDomeTaahbatAI(Creature* pCreature) : EcoDomeBossAI(pCreature, DATA_TAAHBAT_AND_AWAZJ) {}

    void DoAction(int32_t action) override
    {
        if (!isAlive() || !_isInCombat())
            return;

        if (action == ACTION_TAAHBAT_WARP_STRUCK)
        {
            if (mBlitzing && mIncorporealLeft > 0 && --mIncorporealLeft == 0)
                destabilize();
        }
        else if (action == ACTION_AWAZJ_FELL)
        {
            mAwazjFell = true;
            if (mMounted)
                dismount();

            if (!mBlitzing)
                fightOnFoot();
        }
    }

protected:
    void onEngage() override
    {
        mAwazjFell = false;
        mBlitzing = false;
        mIncorporealLeft = 0;
        mFollowMs = 0;
        mEnergy.reset();

        castSpellOnSelf(SPELL_TAAHBAT_BEASTMASTERS_BOND, true);
        mount();
    }

    void onReset() override
    {
        mBlitzing = false;
        dismount();
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mMounted)
            return;

        mFollowMs += diff;
        if (mFollowMs >= RiderFollowMs)
        {
            mFollowMs = 0;
            keepSeat();
        }

        if (mAwazjFell || !mEnergy.fill(diff, TaahbatEnergyPerSecond) || _isCasting())
            return;

        beginBlitz();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TAAHBAT_ARCANE_OVERLOAD:
                castSpellAOE(SPELL_TAAHBAT_ARCANE_OVERLOAD);
                scriptEvents.addEvent(EVENT_TAAHBAT_ARCANE_OVERLOAD, mBlitzing ? 5000 : 12000);
                break;
            case EVENT_TAAHBAT_BLITZ_FADES:
                endBlitz();
                notifyMount(ACTION_TAAHBAT_BLITZ_END);
                scriptEvents.addEvent(EVENT_TAAHBAT_REMOUNT, 2000);
                break;
            case EVENT_TAAHBAT_REMOUNT:
                if (mAwazjFell)
                    fightOnFoot();
                else
                    mount();
                break;
            default:
                break;
        }
    }

private:
    // out of reach while seated on the mount
    void mount()
    {
        mMounted = true;
        mEnergy.reset();
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        _setMeleeDisabled(true);
        keepSeat();
    }

    void dismount()
    {
        mMounted = false;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        _setMeleeDisabled(false);
    }

    void keepSeat()
    {
        EcoDomeAldaniInstanceScript* pEcoDome = getEcoDome();
        Creature* pMount = pEcoDome != nullptr ? pEcoDome->getMember(BOSS_AWAZJ) : nullptr;
        if (pMount != nullptr && pMount->isAlive() && getCreature()->getDistance(pMount) > RiderSeatRange)
            moveTo(pMount->GetPositionX(), pMount->GetPositionY(), pMount->GetPositionZ(), true);
    }

    void beginBlitz()
    {
        dismount();
        mBlitzing = true;
        mIncorporealLeft = isHeroic() ? 4 : 3;

        warn("Taah'bat leaps from A'wazj and begins an Arcane Blitz! Bait the Warp Strikes into him!");
        castSpellOnSelf(SPELL_TAAHBAT_ARCANE_BLITZ, true);
        castSpellOnSelf(SPELL_TAAHBAT_INCORPOREAL, true);
        notifyMount(ACTION_TAAHBAT_BLITZ_BEGIN);

        scriptEvents.addEvent(EVENT_TAAHBAT_ARCANE_OVERLOAD, 2000);
        scriptEvents.addEvent(EVENT_TAAHBAT_BLITZ_FADES, BlitzMaxMs);
    }

    void endBlitz()
    {
        mBlitzing = false;
        _removeAura(SPELL_TAAHBAT_INCORPOREAL);
        _removeAura(SPELL_TAAHBAT_ARCANE_BLITZ);
        scriptEvents.removeEvent(EVENT_TAAHBAT_ARCANE_OVERLOAD);
        scriptEvents.removeEvent(EVENT_TAAHBAT_BLITZ_FADES);
    }

    // every layer broken, rider and mount reel
    void destabilize()
    {
        endBlitz();
        warn("Taah'bat and A'wazj are Destabilized!");
        castSpellOnSelf(SPELL_TAAHBAT_DESTABILIZED, true);
        notifyMount(ACTION_TAAHBAT_DESTABILIZED);
        scriptEvents.addEvent(EVENT_TAAHBAT_REMOUNT, DestabilizedMs);
    }

    void fightOnFoot()
    {
        scriptEvents.removeEvent(EVENT_TAAHBAT_ARCANE_OVERLOAD);
        scriptEvents.addEvent(EVENT_TAAHBAT_ARCANE_OVERLOAD, 5000);
    }

    void notifyMount(int32_t action)
    {
        if (EcoDomeAldaniInstanceScript* pEcoDome = getEcoDome())
            pEcoDome->sendAction(BOSS_AWAZJ, action);
    }

    EnergyBar mEnergy;
    bool mMounted = false;
    bool mBlitzing = false;
    bool mAwazjFell = false;
    uint8_t mIncorporealLeft = 0;
    uint32_t mFollowMs = 0;
};

class EcoDomeBindingJavelinAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EcoDomeBindingJavelinAI(c); }
    explicit EcoDomeBindingJavelinAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Soul-Scribe, strikes the party through their Fates

class EcoDomeSoulScribeAI : public EcoDomeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EcoDomeSoulScribeAI(c); }
    explicit EcoDomeSoulScribeAI(Creature* pCreature) : EcoDomeBossAI(pCreature, DATA_SOUL_SCRIBE) {}

protected:
    void onEngage() override
    {
        mEnergy.reset();
        mFates.clear();
        mDaggerTarget = 0;

        scriptEvents.addEvent(EVENT_SOUL_SCRIBE_WHISPERS_OF_FATE, 2000);
        scriptEvents.addEvent(EVENT_SOUL_SCRIBE_CEREMONIAL_DAGGER, 10000);
        scriptEvents.addEvent(EVENT_SOUL_SCRIBE_DREAD, 15000);
    }

    void onReset() override { mFates.clear(); }

    void onCombatTick(uint32_t diff) override
    {
        if (!mEnergy.fill(diff, SoulScribeEnergyPerSecond) || _isCasting())
            return;

        mEnergy.reset();
        warn("Soul-Scribe begins the Eternal Weave!");
        castSpellOnSelf(SPELL_SOUL_SCRIBE_ETERNAL_WEAVE);
        scriptEvents.addEvent(EVENT_SOUL_SCRIBE_WEAVE_RELEASE, 300);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SOUL_SCRIBE_WHISPERS_OF_FATE:
                drawFates();
                break;
            case EVENT_SOUL_SCRIBE_WEAVE_RELEASE:
                for (Unit* pPlayer : playersNear(getCreature(), getCreature()->GetPosition(), FightReach))
                    castSpell(pPlayer, SPELL_SOUL_SCRIBE_ETERNAL_WEAVE_HIT, true);
                // the weave spends the fates, new ones are drawn
                scriptEvents.addEvent(EVENT_SOUL_SCRIBE_WHISPERS_OF_FATE, 3000);
                break;
            case EVENT_SOUL_SCRIBE_CEREMONIAL_DAGGER:
                if (Unit* pTarget = castAtRandomPlayer(SPELL_SOUL_SCRIBE_CEREMONIAL_DAGGER))
                {
                    mDaggerTarget = pTarget->getGuid();
                    mDaggerSpot = pTarget->GetPosition();
                    scriptEvents.addEvent(EVENT_SOUL_SCRIBE_DAGGER_STRIKE, 300);
                }
                scriptEvents.addEvent(EVENT_SOUL_SCRIBE_CEREMONIAL_DAGGER, 18000);
                break;
            case EVENT_SOUL_SCRIBE_DAGGER_STRIKE:
                if (Unit* pTarget = unitByGuid(mDaggerTarget))
                    castSpell(pTarget, SPELL_SOUL_SCRIBE_DAGGER_STRIKE, true);
                woundFatesNear(mDaggerSpot, DaggerReach);
                break;
            case EVENT_SOUL_SCRIBE_DREAD:
                if (Unit* pTarget = randomPlayer())
                {
                    warn("Soul-Scribe conjures Dread of the Unknown! Keep it away from your Fate!");
                    mDreadSpot = pTarget->GetPosition();
                    castSpellAOE(SPELL_SOUL_SCRIBE_DREAD_OF_THE_UNKNOWN);
                    scriptEvents.addEvent(EVENT_SOUL_SCRIBE_DREAD_DETONATES, 300);
                }
                scriptEvents.addEvent(EVENT_SOUL_SCRIBE_DREAD, 22000);
                break;
            case EVENT_SOUL_SCRIBE_DREAD_DETONATES:
                for (Unit* pPlayer : playersNear(getCreature(), mDreadSpot, DreadReach))
                    castSpell(pPlayer, SPELL_SOUL_SCRIBE_DREAD_DETONATION, true);
                woundFatesNear(mDreadSpot, DreadReach);
                break;
            default:
                break;
        }
    }

private:
    // one Fate echo next to every player
    void drawFates()
    {
        castSpellAOE(SPELL_SOUL_SCRIBE_WHISPERS_OF_FATE);
        summons.despawnEntry(NPC_SOUL_SCRIBE_WHISPER_OF_FATE);
        mFates.clear();

        for (Unit* pPlayer : pickPlayers(5))
        {
            if (Creature* pFate = summonCreature(NPC_SOUL_SCRIBE_WHISPER_OF_FATE, pointAround(pPlayer->GetPosition(), 3.0f, 5.0f), CORPSE_DESPAWN))
                mFates.push_back({ pFate->getGuid(), pPlayer->getGuid() });
        }
    }

    // a struck Fate wounds the player bound to it
    void woundFatesNear(LocationVector const& spot, float range)
    {
        for (BoundPair const& fate : mFates)
        {
            Creature* pFate = getCreature()->getWorldMapCreature(fate.objectGuid);
            if (pFate == nullptr || !pFate->isAlive() || pFate->getDistance(spot) > range)
                continue;

            if (Unit* pOwner = unitByGuid(fate.playerGuid))
                castSpell(pOwner, SPELL_SOUL_SCRIBE_WOUNDED_FATE, true);
        }
    }

    EnergyBar mEnergy;
    std::vector<BoundPair> mFates;
    uint64_t mDaggerTarget = 0;
    LocationVector mDaggerSpot;
    LocationVector mDreadSpot;
};

class EcoDomeWhisperOfFateAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EcoDomeWhisperOfFateAI(c); }
    explicit EcoDomeWhisperOfFateAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
    }
};

void SetupEcoDomeAldani(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ECO_DOME_ALDANI, &EcoDomeAldaniInstanceScript::Create);

    mgr->register_creature_script(BOSS_AZHICCAR, &EcoDomeAzhiccarAI::Create);
    mgr->register_creature_script(NPC_AZHICCAR_FRENZIED_MITE, &EcoDomeFrenziedMiteAI::Create);

    mgr->register_creature_script(BOSS_AWAZJ, &EcoDomeAwazjAI::Create);
    mgr->register_creature_script(BOSS_TAAHBAT, &EcoDomeTaahbatAI::Create);
    mgr->register_creature_script(NPC_AWAZJ_BINDING_JAVELIN, &EcoDomeBindingJavelinAI::Create);

    mgr->register_creature_script(BOSS_SOUL_SCRIBE, &EcoDomeSoulScribeAI::Create);
    mgr->register_creature_script(NPC_SOUL_SCRIBE_WHISPER_OF_FATE, &EcoDomeWhisperOfFateAI::Create);
}
