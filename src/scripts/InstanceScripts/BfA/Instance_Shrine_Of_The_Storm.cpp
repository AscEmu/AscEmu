/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Shrine_Of_The_Storm.hpp"

#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

enum ShrineOfTheStormEvents
{
    EVENT_TIDE_SEA_BLAST = 1,
    EVENT_TIDE_CHOKING_BRINE,
    EVENT_TIDE_SURGING_RUSH,
    EVENT_TIDE_UNDERTOW,
    EVENT_AQUSIRR_GRASP,

    EVENT_IRONHULL_HINDERING_CLEAVE,
    EVENT_IRONHULL_IRONSIDES,
    EVENT_FAYE_SLICING_BLAST,
    EVENT_FAYE_TEMPEST,

    EVENT_STORMSONG_VOID_BOLT,
    EVENT_STORMSONG_WAKEN_THE_VOID,
    EVENT_STORMSONG_MIND_REND,

    EVENT_VOLZITH_YAWNING_GATE,
    EVENT_VOLZITH_TENTACLE,
    EVENT_VOLZITH_WHISPERS,
    EVENT_VOLZITH_CALL_THE_ABYSS,

    EVENT_TENTACLE_SLAM,
    EVENT_FORGOTTEN_DENIZEN_ESSENCE
};

namespace
{
    float const ShrineTwoPi = 6.2831853f;

    uint32_t const TidesageCouncilMembers[] = { BOSS_GALECALLER_FAYE, BOSS_BROTHER_IRONHULL };

    // Aqualings rise around the platform when Aqu'sirr bursts apart
    LocationVector const AqualingRises[] =
    {
        { 3917.31f, -1234.08f, 128.130f, 0.0f },
        { 3924.02f, -1259.52f, 128.141f, 1.5f },
        { 3950.80f, -1238.47f, 127.831f, 3.1f }
    };
    uint32_t const AqusirrAqualingHealthCutPct = 11;

    // the two tides of the sunken city Vol'zith pulls the party into
    LocationVector const ForgottenDenizenRises[] =
    {
        { 3658.58f, -1715.67f, 85.89f, 3.6f },
        { 3603.31f, -1742.71f, 85.82f, 0.6f }
    };
    LocationVector const SunkenDenizenRise(3619.73f, -1709.18f, 86.14f, 4.7f);

    LocationVector const AbyssRifts[] =
    {
        { 3675.83f, -1688.60f, 83.4349f },
        { 3656.23f, -1675.91f, 83.8259f },
        { 3636.15f, -1673.52f, 83.3167f },
        { 3621.70f, -1673.33f, 84.4263f },
        { 3611.91f, -1678.02f, 83.5110f },
        { 3593.11f, -1687.05f, 84.3794f },
        { 3575.16f, -1698.99f, 83.5286f },
        { 3570.94f, -1714.89f, 83.6414f },
        { 3570.48f, -1730.42f, 83.3317f },
        { 3576.01f, -1751.30f, 83.4642f },
        { 3581.68f, -1761.81f, 83.5611f },
        { 3594.58f, -1772.18f, 83.6898f },
        { 3610.53f, -1783.90f, 83.6852f },
        { 3624.24f, -1788.61f, 82.8457f },
        { 3635.22f, -1786.49f, 84.6600f }
    };
    uint32_t const AbyssRiftCount = 15;
    uint32_t const AbyssManifestationsPerCall = 4;
    uint32_t const AbyssEruptionEnergy = 10;

    uint32_t const AwokenVoidsPerWaken = 3;

    // energy bar in tenths, the boss acts on a full bar
    class ShrineEnergyBar
    {
    public:
        void reset(uint32_t energy)
        {
            mTenths = std::min<uint32_t>(energy * 10, 1000);
            mClockMs = 0;
        }

        void add(uint32_t energy) { mTenths = std::min<uint32_t>(mTenths + energy * 10, 1000); }

        bool fill(uint32_t diff, uint32_t tenthsPerSecond)
        {
            mClockMs += diff;
            while (mClockMs >= 1000)
            {
                mClockMs -= 1000;
                mTenths = std::min<uint32_t>(mTenths + tenthsPerSecond, 1000);
            }

            return mTenths >= 1000;
        }

        void drain() { mTenths = 0; }

    private:
        uint32_t mTenths = 0;
        uint32_t mClockMs = 0;
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class ShrineOfTheStormInstanceScript : public InstanceScript
{
public:
    explicit ShrineOfTheStormInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(ShrineOfTheStormEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ShrineOfTheStormInstanceScript(pMapMgr); }

    // the council ends only with its last member
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isCouncilDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (state != EncounterStates::Failed)
            return;

        // a wiped council fights again in full
        for (Creature* pMember : getCouncil(id))
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    std::vector<Creature*> getCouncil(uint32_t id)
    {
        std::vector<Creature*> members;
        if (id != DATA_TIDESAGE_COUNCIL)
            return members;

        for (uint32_t entry : TidesageCouncilMembers)
        {
            for (Creature* pMember : getCreatureSetForEntry(entry))
                members.push_back(pMember);
        }

        return members;
    }

    bool isCouncilDown(uint32_t id, Creature* pExcept)
    {
        for (Creature* pMember : getCouncil(id))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    Creature* findLiving(uint32_t entry)
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
// Shared combat base: event clock, target helpers and the water rotation

class ShrineOfTheStormCombatAI : public CreatureAIScript
{
public:
    explicit ShrineOfTheStormCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    ShrineOfTheStormInstanceScript* getShrine()
    {
        if (getCreature()->GetMapId() != MAP_SHRINE_OF_THE_STORM)
            return nullptr;

        return static_cast<ShrineOfTheStormInstanceScript*>(getInstanceScript());
    }

    Unit* currentVictim() { return getCreature()->getAIInterface()->getCurrentTarget(); }

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

    std::vector<Unit*> livingPlayers(float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && getCreature()->getDistance(pPlayer) <= range)
                players.push_back(pPlayer);
        }

        return players;
    }

    Unit* nearestPlayer(float range)
    {
        Unit* pNearest = nullptr;
        float nearestRange = range;
        for (Unit* pPlayer : livingPlayers(range))
        {
            float const distance = getCreature()->getDistance(pPlayer);
            if (distance <= nearestRange)
            {
                nearestRange = distance;
                pNearest = pPlayer;
            }
        }

        return pNearest;
    }

    void castAt(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector ringSpot(float minRange, float maxRange)
    {
        Creature* pSelf = getCreature();
        float const angle = Util::getRandomFloat(ShrineTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(pSelf->GetPositionX() + range * std::cos(angle), pSelf->GetPositionY() + range * std::sin(angle), pSelf->GetPositionZ(), angle);
    }

    void warn(std::string const& text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // Aqu'sirr and the Aqualings share one water rotation
    void startTideRotation()
    {
        scriptEvents.addEvent(EVENT_TIDE_SEA_BLAST, 1000);
        scriptEvents.addEvent(EVENT_TIDE_CHOKING_BRINE, 8900);
        scriptEvents.addEvent(EVENT_TIDE_SURGING_RUSH, 17000);
        scriptEvents.addEvent(EVENT_TIDE_UNDERTOW, 36800);
    }

    bool runTideEvent(uint32_t eventId)
    {
        switch (eventId)
        {
            case EVENT_TIDE_SEA_BLAST:
            {
                // only when nobody stands in melee reach
                Unit* pVictim = currentVictim();
                if (pVictim != nullptr && getRangeToObject(pVictim) > 6.0f)
                {
                    castAt(anyPlayer(), SPELL_AQUSIRR_SEA_BLAST);
                    scriptEvents.addEvent(EVENT_TIDE_SEA_BLAST, 2400);
                }
                else
                {
                    scriptEvents.addEvent(EVENT_TIDE_SEA_BLAST, 1000);
                }
                return true;
            }
            case EVENT_TIDE_CHOKING_BRINE:
                castAt(anyPlayer(), SPELL_AQUSIRR_CHOKING_BRINE);
                scriptEvents.addEvent(EVENT_TIDE_CHOKING_BRINE, Util::getRandomUInt(34000, 39000));
                return true;
            case EVENT_TIDE_SURGING_RUSH:
                if (Unit* pTarget = anyPlayer())
                {
                    castSpell(pTarget, SPELL_AQUSIRR_SURGING_RUSH, true);
                    moveCharge(pTarget->GetPosition(), 30.0f);
                }
                scriptEvents.removeEvent(EVENT_TIDE_SEA_BLAST);
                scriptEvents.addEvent(EVENT_TIDE_SEA_BLAST, 10000);
                scriptEvents.addEvent(EVENT_TIDE_SURGING_RUSH, Util::getRandomUInt(31500, 39300));
                return true;
            case EVENT_TIDE_UNDERTOW:
                castAt(anyPlayer(), SPELL_AQUSIRR_UNDERTOW);
                scriptEvents.addEvent(EVENT_TIDE_UNDERTOW, Util::getRandomUInt(32400, 34000));
                return true;
            default:
                return false;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and council pull

class ShrineOfTheStormBossAI : public ShrineOfTheStormCombatAI
{
public:
    ShrineOfTheStormBossAI(Creature* pCreature, uint32_t encounterId) : ShrineOfTheStormCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        if (ShrineOfTheStormInstanceScript* pShrine = getShrine())
        {
            for (Creature* pMember : pShrine->getCouncil(mEncounterId))
            {
                if (pMember != getCreature() && pMember->isAlive() && !pMember->isInCombat())
                    setZoneWideCombat(pMember);
            }
        }

        ShrineOfTheStormCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mEncounterId, EncounterStates::Failed);
            }
        }

        ShrineOfTheStormCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            ShrineOfTheStormInstanceScript* pShrine = getShrine();
            if (pShrine == nullptr || pShrine->isCouncilDown(mEncounterId, getCreature()))
                pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
        onDefeat();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Aqu'sirr, bursts into three Aqualings at half health

class ShrineOfTheStormAqusirrAI : public ShrineOfTheStormBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShrineOfTheStormAqusirrAI(c); }
    explicit ShrineOfTheStormAqusirrAI(Creature* pCreature) : ShrineOfTheStormBossAI(pCreature, DATA_AQUSIRR) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (mBurst && damage != nullptr)
            *damage = 0;
    }

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (!mBurst || pSummon->getEntry() != NPC_AQUALING)
            return;

        // every Aqualing slain tears a piece off Aqu'sirr
        uint32_t const cut = getCreature()->getMaxHealth() / 100 * AqusirrAqualingHealthCutPct;
        uint32_t const health = getCreature()->getHealth();
        getCreature()->setHealth(health > cut ? health - cut : 1);

        if (mAqualingsLeft > 0 && --mAqualingsLeft == 0)
            reform(true);
    }

protected:
    void onEngage() override
    {
        mSplitDone = false;
        mBurst = false;
        mAqualingsLeft = 0;
        startRotation();
    }

    void onReset() override
    {
        reform(false);
        _removeAuraOnPlayers(SPELL_AQUSIRR_GRASP_HOLD);
    }

    void onDefeat() override { _removeAuraOnPlayers(SPELL_AQUSIRR_GRASP_HOLD); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mSplitDone && _getHealthPercent() <= 50)
            burstApart();
    }

    void onEvent(uint32_t eventId) override
    {
        if (runTideEvent(eventId))
            return;

        if (eventId != EVENT_AQUSIRR_GRASP)
            return;

        if (Unit* pTarget = anyPlayer())
        {
            castSpell(pTarget, SPELL_AQUSIRR_GRASP_FROM_THE_DEPTHS);
            if (Creature* pTentacle = summonCreature(NPC_GRASPING_TENTACLES, pTarget->GetPosition(), CORPSE_DESPAWN))
                pTentacle->castSpell(pTarget, SPELL_AQUSIRR_GRASP_HOLD, true);
        }
        scriptEvents.addEvent(EVENT_AQUSIRR_GRASP, 38000);
    }

private:
    void startRotation()
    {
        startTideRotation();
        if (isHeroic())
            scriptEvents.addEvent(EVENT_AQUSIRR_GRASP, 14300);
    }

    void burstApart()
    {
        mSplitDone = true;
        mBurst = true;
        mAqualingsLeft = 0;

        scriptEvents.resetEvents();
        attackStop();
        setReactState(REACT_PASSIVE);
        setRooted(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        warn("Aqu'sirr bursts apart into Aqualings!");
        for (LocationVector const& rise : AqualingRises)
        {
            if (callAdd(NPC_AQUALING, rise) != nullptr)
                ++mAqualingsLeft;
        }

        if (mAqualingsLeft == 0)
            reform(true);
    }

    void reform(bool resume)
    {
        if (!mBurst)
            return;

        mBurst = false;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(false);
        setReactState(REACT_AGGRESSIVE);

        if (resume)
            startRotation();
    }

    bool mSplitDone = false;
    bool mBurst = false;
    uint32_t mAqualingsLeft = 0;
};

class ShrineOfTheStormAqualingAI : public ShrineOfTheStormCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShrineOfTheStormAqualingAI(c); }
    explicit ShrineOfTheStormAqualingAI(Creature* pCreature) : ShrineOfTheStormCombatAI(pCreature) {}

    void OnLoad() override { castSpellOnSelf(SPELL_AQUALING_DIMINISH, true); }

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());

        ShrineOfTheStormCombatAI::OnCombatStart(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }

protected:
    void onEngage() override { startTideRotation(); }

    void onEvent(uint32_t eventId) override { runTideEvent(eventId); }
};

// holds one player until cut down
class ShrineOfTheStormGraspingTentaclesAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShrineOfTheStormGraspingTentaclesAI(c); }
    explicit ShrineOfTheStormGraspingTentaclesAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (Unit* pPlayer = static_cast<Unit*>(pObject))
                pPlayer->removeAllAurasById(SPELL_AQUSIRR_GRASP_HOLD, getCreature()->getGuid());
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Tidesage Council, each member places a ward on a full energy bar

class ShrineOfTheStormCouncilAI : public ShrineOfTheStormBossAI
{
public:
    ShrineOfTheStormCouncilAI(Creature* pCreature, uint32_t energyOnPull) : ShrineOfTheStormBossAI(pCreature, DATA_TIDESAGE_COUNCIL), mEnergyOnPull(energyOnPull) {}

protected:
    void onEngage() override
    {
        mEnergy.reset(mEnergyOnPull);
        startRotation();
    }

    void onCombatTick(uint32_t diff) override
    {
        // a full bar takes twenty seconds
        if (!mEnergy.fill(diff, 50) || _isCasting())
            return;

        mEnergy.drain();
        raiseWard();
    }

    virtual void startRotation() = 0;
    virtual void raiseWard() = 0;

    Creature* partner(uint32_t entry)
    {
        ShrineOfTheStormInstanceScript* pShrine = getShrine();
        return pShrine != nullptr ? pShrine->findLiving(entry) : nullptr;
    }

private:
    ShrineEnergyBar mEnergy;
    uint32_t mEnergyOnPull;
};

class ShrineOfTheStormIronhullAI : public ShrineOfTheStormCouncilAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShrineOfTheStormIronhullAI(c); }
    explicit ShrineOfTheStormIronhullAI(Creature* pCreature) : ShrineOfTheStormCouncilAI(pCreature, 0) {}

protected:
    void startRotation() override
    {
        scriptEvents.addEvent(EVENT_IRONHULL_HINDERING_CLEAVE, 8500);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_IRONHULL_IRONSIDES, 6100);
    }

    void raiseWard() override
    {
        warn("Brother Ironhull raises a Reinforcing Ward!");
        castSpellOnSelf(SPELL_IRONHULL_REINFORCING_WARD);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_IRONHULL_HINDERING_CLEAVE:
                castSpellOnVictim(SPELL_IRONHULL_HINDERING_CLEAVE);
                scriptEvents.addEvent(EVENT_IRONHULL_HINDERING_CLEAVE, 17000);
                break;
            case EVENT_IRONHULL_IRONSIDES:
                castSpellOnSelf(SPELL_IRONHULL_BLESSING_OF_IRONSIDES);
                scriptEvents.addEvent(EVENT_IRONHULL_IRONSIDES, 33000);
                break;
            default:
                break;
        }
    }
};

class ShrineOfTheStormFayeAI : public ShrineOfTheStormCouncilAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShrineOfTheStormFayeAI(c); }
    explicit ShrineOfTheStormFayeAI(Creature* pCreature) : ShrineOfTheStormCouncilAI(pCreature, 50) {}

protected:
    void startRotation() override
    {
        scriptEvents.addEvent(EVENT_FAYE_SLICING_BLAST, 1000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_FAYE_TEMPEST, 26700);
    }

    // the ward lands under her brother while he stands
    void raiseWard() override
    {
        warn("Galecaller Faye creates a Swiftness Ward!");
        Creature* pBrother = partner(BOSS_BROTHER_IRONHULL);
        castSpell(pBrother != nullptr ? pBrother : getCreature(), SPELL_FAYE_SWIFTNESS_WARD);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_FAYE_SLICING_BLAST:
                castSpellOnVictim(SPELL_FAYE_SLICING_BLAST);
                scriptEvents.addEvent(EVENT_FAYE_SLICING_BLAST, 2500);
                break;
            case EVENT_FAYE_TEMPEST:
                castSpellOnSelf(SPELL_FAYE_BLESSING_OF_THE_TEMPEST);
                scriptEvents.addEvent(EVENT_FAYE_TEMPEST, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lord Stormsong, an Ancient Mindbender on every full bar

class ShrineOfTheStormStormsongAI : public ShrineOfTheStormBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShrineOfTheStormStormsongAI(c); }
    explicit ShrineOfTheStormStormsongAI(Creature* pCreature) : ShrineOfTheStormBossAI(pCreature, DATA_LORD_STORMSONG) {}

protected:
    void onEngage() override
    {
        mEnergy.reset(55);
        scriptEvents.addEvent(EVENT_STORMSONG_VOID_BOLT, 8500);
        scriptEvents.addEvent(EVENT_STORMSONG_WAKEN_THE_VOID, 13500);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_STORMSONG_MIND_REND, 16000);
    }

    void onReset() override { _removeAuraOnPlayers(SPELL_STORMSONG_ANCIENT_MINDBENDER); }

    void onDefeat() override { _removeAuraOnPlayers(SPELL_STORMSONG_ANCIENT_MINDBENDER); }

    void onCombatTick(uint32_t diff) override
    {
        // two and a half energy per second
        if (!mEnergy.fill(diff, 25) || _isCasting())
            return;

        Unit* pTarget = offTankPlayer();
        if (pTarget == nullptr)
            return;

        mEnergy.drain();
        warn("An Ancient Mindbender attaches itself to a victim!");
        castSpell(pTarget, SPELL_STORMSONG_ANCIENT_MINDBENDER);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_STORMSONG_VOID_BOLT:
                castSpellOnVictim(SPELL_STORMSONG_VOID_BOLT);
                scriptEvents.addEvent(EVENT_STORMSONG_VOID_BOLT, 8500);
                break;
            case EVENT_STORMSONG_WAKEN_THE_VOID:
                castSpellOnSelf(SPELL_STORMSONG_WAKEN_THE_VOID);
                for (uint32_t i = 0; i < AwokenVoidsPerWaken; ++i)
                    summonCreature(NPC_AWOKEN_VOID, ringSpot(6.0f, 12.0f), TIMED_DESPAWN, 40000);
                scriptEvents.addEvent(EVENT_STORMSONG_WAKEN_THE_VOID, 43500);
                break;
            case EVENT_STORMSONG_MIND_REND:
                castSpellAOE(SPELL_STORMSONG_MIND_REND);
                scriptEvents.addEvent(EVENT_STORMSONG_MIND_REND, 10500);
                break;
            default:
                break;
        }
    }

private:
    ShrineEnergyBar mEnergy;
};

// drifts after one player and bursts on contact
class ShrineOfTheStormAwokenVoidAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShrineOfTheStormAwokenVoidAI(c); }
    explicit ShrineOfTheStormAwokenVoidAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mBurst || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mClockMs > diff)
        {
            mClockMs -= diff;
            return;
        }

        mClockMs = 500;
        Unit* pPrey = nullptr;
        if (!mPreyGuid.isEmpty() && getCreature()->getWorldMap() != nullptr)
            pPrey = getCreature()->getWorldMap()->getUnit(mPreyGuid);

        if (pPrey == nullptr || !pPrey->isAlive())
        {
            pPrey = choosePrey();
            if (pPrey == nullptr)
                return;
        }

        if (getRangeToObject(pPrey) <= 3.0f)
        {
            mBurst = true;
            castSpell(pPrey, SPELL_AWOKEN_VOID_EXPLOSIVE_VOID, true);
            despawn(500, 0);
            return;
        }

        moveTo(pPrey->GetPositionX(), pPrey->GetPositionY(), pPrey->GetPositionZ(), false);
    }

private:
    Unit* choosePrey()
    {
        std::vector<Unit*> players;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && getRangeToObject(pPlayer) <= 80.0f)
                players.push_back(pPlayer);
        }

        if (players.empty())
            return nullptr;

        Unit* pPrey = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
        mPreyGuid = pPrey->GetNewGUID();
        castSpell(pPrey, SPELL_AWOKEN_VOID_FIXATE, true);
        return pPrey;
    }

    WoWGuid mPreyGuid;
    uint32_t mClockMs = 1000;
    bool mBurst = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Vol'zith the Whisperer, drags the party into the sunken city on a full bar

class ShrineOfTheStormVolzithAI : public ShrineOfTheStormBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShrineOfTheStormVolzithAI(c); }
    explicit ShrineOfTheStormVolzithAI(Creature* pCreature) : ShrineOfTheStormBossAI(pCreature, DATA_VOLZITH) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (mGrasping && damage != nullptr)
            *damage = 0;
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_VOLZITH_ABYSS_ENERGY && _isInCombat() && !mGrasping)
            mEnergy.add(AbyssEruptionEnergy);
    }

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (!mGrasping)
            return;

        if (pSummon->getEntry() != NPC_FORGOTTEN_DENIZEN && pSummon->getEntry() != NPC_SUNKEN_DENIZEN)
            return;

        if (mDenizensLeft > 0 && --mDenizensLeft == 0)
            releaseGrasp(true);
    }

protected:
    void onEngage() override
    {
        mGrasping = false;
        mDenizensLeft = 0;
        mEnergy.reset(60);
        startRotation();
    }

    void onReset() override { releaseGrasp(false); }

    void onCombatTick(uint32_t diff) override
    {
        if (mGrasping)
            return;

        // two energy per second
        if (mEnergy.fill(diff, 20) && !_isCasting())
            graspTheParty();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VOLZITH_YAWNING_GATE:
                castSpellOnSelf(SPELL_VOLZITH_YAWNING_GATE);
                scriptEvents.addEvent(EVENT_VOLZITH_YAWNING_GATE, 15000);
                break;
            case EVENT_VOLZITH_TENTACLE:
                if (Unit* pTarget = anyPlayer())
                    callAdd(NPC_VOLZITH_TENTACLE, pTarget->GetPosition());
                scriptEvents.addEvent(EVENT_VOLZITH_TENTACLE, 12000);
                break;
            case EVENT_VOLZITH_WHISPERS:
                castAt(anyPlayer(), SPELL_VOLZITH_WHISPERS_OF_POWER);
                scriptEvents.addEvent(EVENT_VOLZITH_WHISPERS, 30000);
                break;
            case EVENT_VOLZITH_CALL_THE_ABYSS:
                castSpellOnSelf(SPELL_VOLZITH_CALL_THE_ABYSS);
                for (uint32_t i = 0; i < AbyssManifestationsPerCall; ++i)
                    summonCreature(NPC_MANIFESTATION_OF_THE_DEEP, AbyssRifts[Util::getRandomUInt(AbyssRiftCount - 1)], CORPSE_DESPAWN);
                scriptEvents.addEvent(EVENT_VOLZITH_CALL_THE_ABYSS, 45000);
                break;
            default:
                break;
        }
    }

private:
    void startRotation()
    {
        scriptEvents.addEvent(EVENT_VOLZITH_TENTACLE, 12000);
        scriptEvents.addEvent(EVENT_VOLZITH_YAWNING_GATE, 15000);
        scriptEvents.addEvent(EVENT_VOLZITH_WHISPERS, 30000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_VOLZITH_CALL_THE_ABYSS, 45000);
    }

    // shielded until the denizens of the sunken city fall
    void graspTheParty()
    {
        mEnergy.drain();
        mGrasping = true;
        mDenizensLeft = 0;

        scriptEvents.resetEvents();
        attackStop();
        setReactState(REACT_PASSIVE);
        setRooted(true);

        warn("Vol'zith the Whisperer drags you beneath the tides!");
        castSpellOnSelf(SPELL_VOLZITH_GRASP_OF_THE_SUNKEN_CITY, true);

        for (LocationVector const& rise : ForgottenDenizenRises)
        {
            if (callAdd(NPC_FORGOTTEN_DENIZEN, rise) != nullptr)
                ++mDenizensLeft;
        }

        if (callAdd(NPC_SUNKEN_DENIZEN, SunkenDenizenRise) != nullptr)
            ++mDenizensLeft;

        if (mDenizensLeft == 0)
            releaseGrasp(true);
    }

    void releaseGrasp(bool resume)
    {
        if (!mGrasping)
            return;

        mGrasping = false;
        _removeAura(SPELL_VOLZITH_GRASP_OF_THE_SUNKEN_CITY);
        setRooted(false);
        setReactState(REACT_AGGRESSIVE);

        if (resume)
            startRotation();
    }

    ShrineEnergyBar mEnergy;
    bool mGrasping = false;
    uint32_t mDenizensLeft = 0;
};

// rooted tentacle that slams the closest player
class ShrineOfTheStormVolzithTentacleAI : public ShrineOfTheStormCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShrineOfTheStormVolzithTentacleAI(c); }
    explicit ShrineOfTheStormVolzithTentacleAI(Creature* pCreature) : ShrineOfTheStormCombatAI(pCreature) {}

    void OnLoad() override { setRooted(true); }

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_TENTACLE_SLAM, 2000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_TENTACLE_SLAM)
            return;

        castAt(nearestPlayer(100.0f), SPELL_TENTACLE_SLAM);
        scriptEvents.addEvent(EVENT_TENTACLE_SLAM, 8000);
    }
};

class ShrineOfTheStormForgottenDenizenAI : public ShrineOfTheStormCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShrineOfTheStormForgottenDenizenAI(c); }
    explicit ShrineOfTheStormForgottenDenizenAI(Creature* pCreature) : ShrineOfTheStormCombatAI(pCreature) {}

    void OnLoad() override { setRooted(true); }

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_FORGOTTEN_DENIZEN_ESSENCE, 3000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_FORGOTTEN_DENIZEN_ESSENCE)
            return;

        castSpellOnVictim(SPELL_FORGOTTEN_DENIZEN_CONSUME_ESSENCE);
        scriptEvents.addEvent(EVENT_FORGOTTEN_DENIZEN_ESSENCE, 11000);
    }
};

class ShrineOfTheStormSunkenDenizenAI : public ShrineOfTheStormCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShrineOfTheStormSunkenDenizenAI(c); }
    explicit ShrineOfTheStormSunkenDenizenAI(Creature* pCreature) : ShrineOfTheStormCombatAI(pCreature) {}

    void OnLoad() override { castSpellOnSelf(SPELL_SUNKEN_DENIZEN_LEVIATHANS_WAKE, true); }
};

// crawls to Vol'zith and feeds him energy unless slain on the way
class ShrineOfTheStormManifestationAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShrineOfTheStormManifestationAI(c); }
    explicit ShrineOfTheStormManifestationAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override { setReactState(REACT_PASSIVE); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mErupted || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mClockMs > diff)
        {
            mClockMs -= diff;
            return;
        }

        mClockMs = 1000;
        Creature* pVolzith = findNearestCreature(BOSS_VOLZITH, 150.0f);
        if (pVolzith == nullptr || !pVolzith->isAlive())
            return;

        if (getRangeToObject(pVolzith) > 4.0f)
        {
            moveTo(pVolzith->GetPositionX(), pVolzith->GetPositionY(), pVolzith->GetPositionZ(), false);
            return;
        }

        mErupted = true;
        castSpellAOE(SPELL_MANIFESTATION_ABYSSAL_ERUPTION, true);
        if (pVolzith->GetScript() != nullptr)
            pVolzith->GetScript()->DoAction(ACTION_VOLZITH_ABYSS_ENERGY);

        despawn(500, 0);
    }

private:
    uint32_t mClockMs = 1000;
    bool mErupted = false;
};

void SetupShrineOfTheStorm(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SHRINE_OF_THE_STORM, &ShrineOfTheStormInstanceScript::Create);

    mgr->register_creature_script(BOSS_AQUSIRR, &ShrineOfTheStormAqusirrAI::Create);
    mgr->register_creature_script(NPC_AQUALING, &ShrineOfTheStormAqualingAI::Create);
    mgr->register_creature_script(NPC_GRASPING_TENTACLES, &ShrineOfTheStormGraspingTentaclesAI::Create);

    mgr->register_creature_script(BOSS_BROTHER_IRONHULL, &ShrineOfTheStormIronhullAI::Create);
    mgr->register_creature_script(BOSS_GALECALLER_FAYE, &ShrineOfTheStormFayeAI::Create);

    mgr->register_creature_script(BOSS_LORD_STORMSONG, &ShrineOfTheStormStormsongAI::Create);
    mgr->register_creature_script(NPC_AWOKEN_VOID, &ShrineOfTheStormAwokenVoidAI::Create);

    mgr->register_creature_script(BOSS_VOLZITH, &ShrineOfTheStormVolzithAI::Create);
    mgr->register_creature_script(NPC_VOLZITH_TENTACLE, &ShrineOfTheStormVolzithTentacleAI::Create);
    mgr->register_creature_script(NPC_FORGOTTEN_DENIZEN, &ShrineOfTheStormForgottenDenizenAI::Create);
    mgr->register_creature_script(NPC_SUNKEN_DENIZEN, &ShrineOfTheStormSunkenDenizenAI::Create);
    mgr->register_creature_script(NPC_MANIFESTATION_OF_THE_DEEP, &ShrineOfTheStormManifestationAI::Create);
}
