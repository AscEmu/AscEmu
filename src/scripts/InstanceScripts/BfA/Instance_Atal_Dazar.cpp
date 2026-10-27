/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Atal_Dazar.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>

enum AtalDazarEvents
{
    EVENT_ALUNZA_GILDED_CLAWS = 1,
    EVENT_ALUNZA_TAINTED_BLOOD,
    EVENT_ALUNZA_MOLTEN_GOLD,
    EVENT_ALUNZA_CORRUPTED_GOLD,
    EVENT_ALUNZA_SPIRIT_OF_GOLD,

    EVENT_VOLKAAL_TOXIC_LEAP,
    EVENT_VOLKAAL_NOXIOUS_STENCH,

    EVENT_REZAN_TERRIFYING_VISAGE,
    EVENT_REZAN_TAIL,
    EVENT_REZAN_SERRATED_TEETH,
    EVENT_REZAN_PURSUIT,
    EVENT_REZAN_PURSUIT_CATCH,
    EVENT_REZAN_PURSUIT_GIVE_UP,

    EVENT_YAZMA_WRACKING_PAIN,
    EVENT_YAZMA_SKEWER,
    EVENT_YAZMA_SOULREND,
    EVENT_YAZMA_ECHOES_OF_SHADRA
};

namespace
{
    enum class AtalDazarGateRule : uint8_t
    {
        Room,
        UpperTemple
    };

    struct AtalDazarGate
    {
        uint32_t entry;
        uint32_t encounterId;
        AtalDazarGateRule rule;
    };

    // room gates shut during the fight, the upper temple waits for the three lower bosses
    AtalDazarGate const AtalDazarGates[] =
    {
        { GO_VOLKAAL_GATE_1,   DATA_VOLKAAL, AtalDazarGateRule::Room },
        { GO_VOLKAAL_GATE_2,   DATA_VOLKAAL, AtalDazarGateRule::Room },
        { GO_VOLKAAL_GATE_3,   DATA_VOLKAAL, AtalDazarGateRule::Room },
        { GO_VOLKAAL_GATE_4,   DATA_VOLKAAL, AtalDazarGateRule::Room },
        { GO_YAZMA_GATE_1,     DATA_YAZMA,   AtalDazarGateRule::UpperTemple },
        { GO_YAZMA_GATE_2,     DATA_YAZMA,   AtalDazarGateRule::UpperTemple },
        { GO_WATERFALL_STAIRS, DATA_YAZMA,   AtalDazarGateRule::UpperTemple }
    };

    AtalDazarGate const* findAtalDazarGate(uint32_t entry)
    {
        for (AtalDazarGate const& gate : AtalDazarGates)
        {
            if (gate.entry == entry)
                return &gate;
        }

        return nullptr;
    }

    float const AtalDazarPi = 3.14159265f;

    // shrine ends the corrupted gold rolls in from
    LocationVector const CorruptedGoldSources[] =
    {
        { -1110.4965f, 2259.4306f, 741.8436f, 0.261799f },
        { -1110.5451f, 2325.2829f, 741.8436f, 0.785398f }
    };
    LocationVector const SpiritOfGoldRise(-1119.8f, 2292.46f, 745.114f);

    uint32_t const AlunzaBloodCap = 100;
    uint32_t const AlunzaBloodGain = 5;
    uint32_t const AlunzaBloodTickMs = 2000;
    uint32_t const AlunzaPoolsPerSpill = 5;
    float const CorruptedGoldRoll = 100.0f;

    uint32_t const TotemReanimateMs = 8000;

    uint32_t const RezanPursuitMs = 18000;
    float const RezanDevourRange = 5.0f;

    uint32_t const YazmaEchoTargets = 4;
    uint32_t const YazmaEchoLifeMs = 20000;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class AtalDazarInstanceScript : public InstanceScript
{
public:
    explicit AtalDazarInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(AtalDazarEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new AtalDazarInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_VOLKAAL:
                mTrackedGuids[BOSS_VOLKAAL] = pCreature->GetNewGUID();
                break;
            case BOSS_YAZMA:
            case BOSS_YAZMA_ALT:
                mTrackedGuids[BOSS_YAZMA] = pCreature->GetNewGUID();
                break;
            case NPC_REANIMATION_TOTEM:
                mTotemGuids.insert(pCreature->GetNewGUID());
                break;
            default:
                break;
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findAtalDazarGate(pGameObject->getEntry()) == nullptr)
            return;

        mGateGuids.push_back(pGameObject->GetNewGUID());
        alignGate(pGameObject);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (WoWGuid const& guid : mGateGuids)
        {
            if (GameObject* pGate = getGameObjectByGuid(guid))
                alignGate(pGate);
        }

        // a wiped Vol'kaal finds his totems standing again
        if (id == DATA_VOLKAAL && state == EncounterStates::Failed)
        {
            for (Creature* pTotem : getTotems())
            {
                if (!pTotem->isAlive())
                    pTotem->despawn(1000, 5000);
                else if (pTotem->GetScript() != nullptr)
                    pTotem->GetScript()->DoAction(ACTION_TOTEM_RESET);
            }
        }
    }

    Creature* getTracked(uint32_t entry)
    {
        auto const itr = mTrackedGuids.find(entry);
        return itr != mTrackedGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    std::vector<Creature*> getTotems()
    {
        std::vector<Creature*> totems;
        for (WoWGuid const& guid : mTotemGuids)
        {
            if (Creature* pTotem = getCreatureByGuid(guid))
                totems.push_back(pTotem);
        }

        return totems;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        Creature* pTarget = getTracked(entry);
        if (pTarget != nullptr && pTarget->isAlive() && pTarget->GetScript() != nullptr)
            pTarget->GetScript()->DoAction(action);
    }

private:
    bool isLowerTempleCleared() const
    {
        return getBossState(DATA_ALUNZA) == EncounterStates::Performed
            && getBossState(DATA_VOLKAAL) == EncounterStates::Performed
            && getBossState(DATA_REZAN) == EncounterStates::Performed;
    }

    void alignGate(GameObject* pGate)
    {
        AtalDazarGate const* gate = findAtalDazarGate(pGate->getEntry());
        if (gate == nullptr)
            return;

        bool const wantOpen = gate->rule == AtalDazarGateRule::Room ? getBossState(gate->encounterId) != EncounterStates::InProgress : isLowerTempleCleared();
        uint8_t const wantState = wantOpen ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pGate->getState() != wantState)
            pGate->setState(wantState);
    }

    std::map<uint32_t, WoWGuid> mTrackedGuids;
    std::set<WoWGuid> mTotemGuids;
    std::vector<WoWGuid> mGateGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class AtalDazarCombatAI : public CreatureAIScript
{
public:
    explicit AtalDazarCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    AtalDazarInstanceScript* getAtalDazar()
    {
        if (getCreature()->GetMapId() != MAP_ATAL_DAZAR)
            return nullptr;

        return static_cast<AtalDazarInstanceScript*>(getInstanceScript());
    }

    // random player, the tank only when nobody else is around
    Unit* pickPlayer(bool skipTank = false)
    {
        if (skipTank)
        {
            FilterArgs args(TargetFilter_Player);
            args.addFilter(TargetFilter_NotCurrent);
            if (Unit* pTarget = selectUnitTarget(args))
                return pTarget;
        }

        return selectUnitTarget(FilterArgs(TargetFilter_Player));
    }

    Unit* castAtPlayer(uint32_t spellId, bool skipTank = false)
    {
        Unit* pTarget = pickPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId);

        return pTarget;
    }

    std::vector<Unit*> livingPlayersInRange(float range)
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

    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    void warn(std::string const& text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void notify(uint32_t entry, int32_t action)
    {
        if (AtalDazarInstanceScript* pTemple = getAtalDazar())
            pTemple->sendAction(entry, action);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class AtalDazarBossAI : public AtalDazarCombatAI
{
public:
    AtalDazarBossAI(Creature* pCreature, uint32_t encounterId) : AtalDazarCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        AtalDazarCombatAI::OnCombatStart(pTarget);
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

        AtalDazarCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }
    }

protected:
    virtual void onDefeat() {}

    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Priestess Alun'za, blood fills her until she drains the party

class AtalDazarAlunzaAI : public AtalDazarBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AtalDazarAlunzaAI(c); }
    explicit AtalDazarAlunzaAI(Creature* pCreature) : AtalDazarBossAI(pCreature, DATA_ALUNZA) {}

    void OnLoad() override
    {
        // vehicle template: the core keeps it passive unless the AI is switched on
        getCreature()->setAItoUse(true);
        setCanEnterCombat(true);
        castSpellOnSelf(SPELL_ALUNZA_RITUAL, true);
    }

protected:
    void onEngage() override
    {
        _removeAura(SPELL_ALUNZA_RITUAL);
        mBlood = 0;
        mBloodClock = 0;

        spillTaintedBlood(2000);
        scriptEvents.addEvent(EVENT_ALUNZA_GILDED_CLAWS, 12600);
        scriptEvents.addEvent(EVENT_ALUNZA_MOLTEN_GOLD, 16600);

        if (isHeroic())
        {
            scriptEvents.addEvent(EVENT_ALUNZA_CORRUPTED_GOLD, 3000);
            scriptEvents.addEvent(EVENT_ALUNZA_SPIRIT_OF_GOLD, 9000);
        }
    }

    void onReset() override
    {
        mBlood = 0;
        castSpellOnSelf(SPELL_ALUNZA_RITUAL, true);
    }

    void onCombatTick(uint32_t diff) override
    {
        mBloodClock += diff;
        while (mBloodClock >= AlunzaBloodTickMs)
        {
            mBloodClock -= AlunzaBloodTickMs;
            mBlood = std::min(AlunzaBloodCap, mBlood + AlunzaBloodGain);
        }

        if (mBlood >= AlunzaBloodCap && !_isCasting())
            beginTransfusion();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ALUNZA_GILDED_CLAWS:
                castSpellOnSelf(SPELL_ALUNZA_GILDED_CLAWS);
                scriptEvents.addEvent(EVENT_ALUNZA_GILDED_CLAWS, 34000);
                break;
            case EVENT_ALUNZA_TAINTED_BLOOD:
                castSpellAOE(SPELL_ALUNZA_TAINTED_BLOOD);
                if (mPoolsLeft > 0 && --mPoolsLeft > 0)
                    scriptEvents.addEvent(EVENT_ALUNZA_TAINTED_BLOOD, Util::getRandomUInt(2000, 4000));
                break;
            case EVENT_ALUNZA_MOLTEN_GOLD:
                castAtPlayer(SPELL_ALUNZA_MOLTEN_GOLD);
                scriptEvents.addEvent(EVENT_ALUNZA_MOLTEN_GOLD, 24000);
                break;
            case EVENT_ALUNZA_CORRUPTED_GOLD:
                for (LocationVector const& source : CorruptedGoldSources)
                    summonCreature(NPC_CORRUPTED_GOLD, source, TIMED_DESPAWN, 40000);
                scriptEvents.addEvent(EVENT_ALUNZA_CORRUPTED_GOLD, 3000);
                break;
            case EVENT_ALUNZA_SPIRIT_OF_GOLD:
                callAdd(NPC_SPIRIT_OF_GOLD, SpiritOfGoldRise);
                scriptEvents.addEvent(EVENT_ALUNZA_SPIRIT_OF_GOLD, 34000);
                break;
            default:
                break;
        }
    }

private:
    void spillTaintedBlood(uint32_t delayMs)
    {
        mPoolsLeft = AlunzaPoolsPerSpill;
        scriptEvents.addEvent(EVENT_ALUNZA_TAINTED_BLOOD, delayMs);
    }

    // a full blood meter drains the party, fresh pools follow
    void beginTransfusion()
    {
        mBlood = 0;
        warn("Priestess Alun'za begins a Transfusion!");
        castSpellOnSelf(SPELL_ALUNZA_TRANSFUSION);
        spillTaintedBlood(11000);
    }

    uint32_t mBlood = 0;
    uint32_t mBloodClock = 0;
    uint32_t mPoolsLeft = 0;
};

// rolls across the shrine and burns the first player it touches
class AtalDazarCorruptedGoldAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AtalDazarCorruptedGoldAI(c); }
    explicit AtalDazarCorruptedGoldAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_CORRUPTED_GOLD_AURA, true);

        Creature* pSelf = getCreature();
        float const angle = pSelf->GetOrientation() + Util::getRandomFloat(-AtalDazarPi / 3.0f, AtalDazarPi / 3.0f);
        moveTo(pSelf->GetPositionX() + CorruptedGoldRoll * std::cos(angle), pSelf->GetPositionY() + CorruptedGoldRoll * std::sin(angle), pSelf->GetPositionZ(), false);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mSpent)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mCheckClock > diff)
        {
            mCheckClock -= diff;
            return;
        }

        mCheckClock = 500;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer == nullptr || !pPlayer->isAlive() || getCreature()->getDistance(pPlayer) > 2.5f)
                continue;

            mSpent = true;
            castSpell(pPlayer, SPELL_CORRUPTED_GOLD_TOUCH, true);
            despawn(500, 0);
            return;
        }
    }

private:
    uint32_t mCheckClock = 500;
    bool mSpent = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Vol'kaal, out of reach while his totems stand

class AtalDazarVolkaalAI : public AtalDazarBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AtalDazarVolkaalAI(c); }
    explicit AtalDazarVolkaalAI(Creature* pCreature) : AtalDazarBossAI(pCreature, DATA_VOLKAAL) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && !mDecaying && _isInCombat())
            *damage = 0;
    }

    void DoAction(int32_t action) override
    {
        if (!_isInCombat() || mDecaying)
            return;

        if (action == ACTION_VOLKAAL_TOTEM_FALLEN)
        {
            if (++mTotemsDown >= mTotemsLinked)
                beginDecay();
        }
        else if (action == ACTION_VOLKAAL_TOTEM_RISEN && mTotemsDown > 0)
        {
            --mTotemsDown;
        }
    }

protected:
    void onEngage() override
    {
        mDecaying = false;
        mTotemsDown = 0;
        mTotemsLinked = 0;
        mLeapCount = 0;

        if (AtalDazarInstanceScript* pTemple = getAtalDazar())
        {
            for (Creature* pTotem : pTemple->getTotems())
            {
                if (!pTotem->isAlive())
                    continue;

                ++mTotemsLinked;
                pTemple->sendUnitEncounter(EncounterFrameEngage, pTotem);
                if (!pTotem->isInCombat())
                    setZoneWideCombat(pTotem);
            }
        }

        scriptEvents.addEvent(EVENT_VOLKAAL_TOXIC_LEAP, 2200);
        scriptEvents.addEvent(EVENT_VOLKAAL_NOXIOUS_STENCH, 5800);

        // without totems in the room there is nothing to break
        if (mTotemsLinked == 0)
        {
            beginDecay();
            return;
        }

        castSpellOnSelf(SPELL_VOLKAAL_BAD_VOODOO, true);
    }

    void onReset() override
    {
        mDecaying = false;
        _removeAura(SPELL_VOLKAAL_RAPID_DECAY);
        _removeAura(SPELL_VOLKAAL_BAD_VOODOO);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VOLKAAL_TOXIC_LEAP:
                castAtPlayer(SPELL_VOLKAAL_TOXIC_LEAP);
                ++mLeapCount;
                // every third leap comes after a longer pause
                scriptEvents.addEvent(EVENT_VOLKAAL_TOXIC_LEAP, mLeapCount % 3 == 1 ? 8500 : 6000);
                break;
            case EVENT_VOLKAAL_NOXIOUS_STENCH:
                castSpellOnVictim(SPELL_VOLKAAL_NOXIOUS_STENCH);
                scriptEvents.addEvent(EVENT_VOLKAAL_NOXIOUS_STENCH, mDecaying ? 20600 : 19400);
                break;
            default:
                break;
        }
    }

private:
    void beginDecay()
    {
        mDecaying = true;
        warn("Vol'kaal's totems are broken, Rapid Decay spreads from his body!");
        _removeAura(SPELL_VOLKAAL_BAD_VOODOO);
        castSpellOnSelf(SPELL_VOLKAAL_RAPID_DECAY, true);
        scriptEvents.removeEvent(EVENT_VOLKAAL_TOXIC_LEAP);

        if (AtalDazarInstanceScript* pTemple = getAtalDazar())
        {
            for (Creature* pTotem : pTemple->getTotems())
            {
                if (pTotem->isAlive() && pTotem->GetScript() != nullptr)
                    pTotem->GetScript()->DoAction(ACTION_TOTEM_SHATTER);
            }
        }
    }

    bool mDecaying = false;
    uint32_t mTotemsDown = 0;
    uint32_t mTotemsLinked = 0;
    uint32_t mLeapCount = 0;
};

// heroic: a broken totem anchors itself and rises again unless all fall together
class AtalDazarReanimationTotemAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AtalDazarReanimationTotemAI(c); }
    explicit AtalDazarReanimationTotemAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        if (AtalDazarInstanceScript* pTemple = getAtalDazar())
        {
            Creature* pVolkaal = pTemple->getTracked(BOSS_VOLKAAL);
            if (pVolkaal != nullptr && pVolkaal->isAlive() && !pVolkaal->isInCombat())
                setZoneWideCombat(pVolkaal);
        }
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        standUp();
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mShattered || !isHeroic())
            return;

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        if (!mFallen)
            fall();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        // normal difficulty: every kill counts once
        if (!mFallen && !mShattered)
            notifyVolkaal(ACTION_VOLKAAL_TOTEM_FALLEN);
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_TOTEM_SHATTER)
            mShatterPending = true;
        else if (action == ACTION_TOTEM_RESET)
            standUp();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        // the killing blow is dealt outside of a damage callback
        if (mShatterPending)
        {
            mShatterPending = false;
            mShattered = true;
            getCreature()->dealDamage(getCreature(), getCreature()->getHealth(), 0);
            return;
        }

        if (!mFallen || mRiseClock == 0)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mRiseClock > diff)
        {
            mRiseClock -= diff;
            return;
        }

        mRiseClock = 0;
        rise();
    }

private:
    AtalDazarInstanceScript* getAtalDazar()
    {
        if (getCreature()->GetMapId() != MAP_ATAL_DAZAR)
            return nullptr;

        return static_cast<AtalDazarInstanceScript*>(getInstanceScript());
    }

    void notifyVolkaal(int32_t action)
    {
        if (AtalDazarInstanceScript* pTemple = getAtalDazar())
            pTemple->sendAction(BOSS_VOLKAAL, action);
    }

    void fall()
    {
        mFallen = true;
        mRiseClock = TotemReanimateMs;
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_TOTEM_SOUL_ANCHOR, true);
        notifyVolkaal(ACTION_VOLKAAL_TOTEM_FALLEN);
    }

    void rise()
    {
        mFallen = false;
        castSpellOnSelf(SPELL_TOTEM_REANIMATE, true);
        getCreature()->setHealth(getCreature()->getMaxHealth());
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        notifyVolkaal(ACTION_VOLKAAL_TOTEM_RISEN);
    }

    void standUp()
    {
        mFallen = false;
        mShattered = false;
        mShatterPending = false;
        mRiseClock = 0;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        if (isAlive())
            getCreature()->setHealth(getCreature()->getMaxHealth());
    }

    bool mFallen = false;
    bool mShattered = false;
    bool mShatterPending = false;
    uint32_t mRiseClock = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rezan, hunts a single prey from time to time

class AtalDazarRezanAI : public AtalDazarBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AtalDazarRezanAI(c); }
    explicit AtalDazarRezanAI(Creature* pCreature) : AtalDazarBossAI(pCreature, DATA_REZAN) {}

    void OnLoad() override
    {
        // vehicle template: the core keeps it passive unless the AI is switched on
        getCreature()->setAItoUse(true);
        setCanEnterCombat(true);
    }

protected:
    void onEngage() override
    {
        mPreyGuid = WoWGuid();
        scriptEvents.addEvent(EVENT_REZAN_TAIL, 6100);
        scriptEvents.addEvent(EVENT_REZAN_SERRATED_TEETH, 8500);
        scriptEvents.addEvent(EVENT_REZAN_TERRIFYING_VISAGE, 12200);
        scriptEvents.addEvent(EVENT_REZAN_PURSUIT, 21900);
    }

    void onReset() override { endPursuit(); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_REZAN_TERRIFYING_VISAGE:
                warn("Rezan lets out a Terrifying Visage!");
                castSpellAOE(SPELL_REZAN_TERRIFYING_VISAGE);
                scriptEvents.addEvent(EVENT_REZAN_TERRIFYING_VISAGE, 35500);
                break;
            case EVENT_REZAN_TAIL:
                castSpellAOE(SPELL_REZAN_TAIL);
                scriptEvents.addEvent(EVENT_REZAN_TAIL, Util::getRandomUInt(15800, 18200));
                break;
            case EVENT_REZAN_SERRATED_TEETH:
                castSpellOnVictim(SPELL_REZAN_SERRATED_TEETH);
                scriptEvents.addEvent(EVENT_REZAN_SERRATED_TEETH, 34100);
                break;
            case EVENT_REZAN_PURSUIT:
                beginPursuit();
                scriptEvents.addEvent(EVENT_REZAN_PURSUIT, 35200);
                break;
            case EVENT_REZAN_PURSUIT_CATCH:
                tryToCatchPrey();
                break;
            case EVENT_REZAN_PURSUIT_GIVE_UP:
                endPursuit();
                break;
            default:
                break;
        }
    }

private:
    Unit* getPrey()
    {
        if (mPreyGuid.isEmpty() || getCreature()->getWorldMap() == nullptr)
            return nullptr;

        return getCreature()->getWorldMap()->getUnit(mPreyGuid);
    }

    // everything else waits while he chases one player
    void beginPursuit()
    {
        Unit* pPrey = pickPlayer();
        if (pPrey == nullptr)
            return;

        mPreyGuid = pPrey->GetNewGUID();
        scriptEvents.delayAllEvents(RezanPursuitMs);

        warn("Rezan begins his Pursuit!");
        castSpell(pPrey, SPELL_REZAN_PURSUIT, true);
        setReactState(REACT_PASSIVE);
        moveChase(pPrey);

        scriptEvents.addEvent(EVENT_REZAN_PURSUIT_CATCH, 500);
        scriptEvents.addEvent(EVENT_REZAN_PURSUIT_GIVE_UP, RezanPursuitMs);
    }

    void tryToCatchPrey()
    {
        Unit* pPrey = getPrey();
        if (pPrey == nullptr || !pPrey->isAlive())
        {
            endPursuit();
            return;
        }

        if (getCreature()->getDistance(pPrey) > RezanDevourRange)
        {
            scriptEvents.addEvent(EVENT_REZAN_PURSUIT_CATCH, 500);
            return;
        }

        castSpell(pPrey, SPELL_REZAN_DEVOUR, true);
        endPursuit();
    }

    void endPursuit()
    {
        if (mPreyGuid.isEmpty())
            return;

        mPreyGuid = WoWGuid();
        scriptEvents.removeEvent(EVENT_REZAN_PURSUIT_CATCH);
        scriptEvents.removeEvent(EVENT_REZAN_PURSUIT_GIVE_UP);
        setReactState(REACT_AGGRESSIVE);
    }

    WoWGuid mPreyGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Yazma, rends souls that crawl back to feed her

class AtalDazarYazmaAI : public AtalDazarBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AtalDazarYazmaAI(c); }
    explicit AtalDazarYazmaAI(Creature* pCreature) : AtalDazarBossAI(pCreature, DATA_YAZMA) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_YAZMA_WRACKING_PAIN, 3500);
        scriptEvents.addEvent(EVENT_YAZMA_SKEWER, 5000);
        scriptEvents.addEvent(EVENT_YAZMA_SOULREND, 10000);
        scriptEvents.addEvent(EVENT_YAZMA_ECHOES_OF_SHADRA, 16900);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_YAZMA_WRACKING_PAIN:
                castAtPlayer(SPELL_YAZMA_WRACKING_PAIN);
                scriptEvents.addEvent(EVENT_YAZMA_WRACKING_PAIN, Util::getRandomUInt(17000, 23000));
                break;
            case EVENT_YAZMA_SKEWER:
                castSpellOnVictim(SPELL_YAZMA_SKEWER);
                scriptEvents.addEvent(EVENT_YAZMA_SKEWER, 32800);
                break;
            case EVENT_YAZMA_SOULREND:
                rendSouls();
                scriptEvents.addEvent(EVENT_YAZMA_SOULREND, 41400);
                break;
            case EVENT_YAZMA_ECHOES_OF_SHADRA:
                callEchoes();
                scriptEvents.addEvent(EVENT_YAZMA_ECHOES_OF_SHADRA, 12000);
                break;
            default:
                break;
        }
    }

private:
    // normal rends one player, heroic everyone but the tank
    void rendSouls()
    {
        warn("Yazma tears out the souls of her foes!");

        std::vector<Unit*> victims;
        if (isHeroic())
        {
            Unit* pTank = getCreature()->getAIInterface()->getCurrentTarget();
            for (Unit* pPlayer : livingPlayersInRange(100.0f))
            {
                if (pPlayer != pTank)
                    victims.push_back(pPlayer);
            }
        }
        else if (Unit* pTarget = pickPlayer(true))
        {
            victims.push_back(pTarget);
        }

        for (Unit* pVictim : victims)
        {
            castSpell(pVictim, SPELL_YAZMA_SOULREND, true);
            summonCreature(NPC_SOULSPAWN, pVictim->GetPosition(), CORPSE_DESPAWN);
        }
    }

    void callEchoes()
    {
        std::vector<Unit*> players = livingPlayersInRange(100.0f);
        for (uint32_t i = 0; i < YazmaEchoTargets && !players.empty(); ++i)
        {
            uint32_t const pick = Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1));
            Unit* pPlayer = players[pick];
            players.erase(players.begin() + pick);

            castSpell(pPlayer, SPELL_YAZMA_ECHOES_OF_SHADRA, true);
            summonCreature(NPC_ECHO_OF_SHADRA, pPlayer->GetPosition(), TIMED_DESPAWN, YazmaEchoLifeMs);
        }
    }
};

// shared walker: follows a goal and acts once it gets close
class AtalDazarSeekerAI : public CreatureAIScript
{
public:
    explicit AtalDazarSeekerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mDone || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mStepClock > diff)
        {
            mStepClock -= diff;
            return;
        }

        mStepClock = 1000;
        Unit* pGoal = findGoal();
        if (pGoal == nullptr)
            return;

        if (getCreature()->getDistance(pGoal) > reachRange())
        {
            moveTo(pGoal->GetPositionX(), pGoal->GetPositionY(), pGoal->GetPositionZ(), false);
            return;
        }

        mDone = true;
        onReached();
        despawn(500, 0);
    }

protected:
    virtual Unit* findGoal() = 0;
    virtual float reachRange() const { return 2.5f; }
    virtual void onReached() = 0;

    AtalDazarInstanceScript* getAtalDazar()
    {
        if (getCreature()->GetMapId() != MAP_ATAL_DAZAR)
            return nullptr;

        return static_cast<AtalDazarInstanceScript*>(getInstanceScript());
    }

private:
    uint32_t mStepClock = 1000;
    bool mDone = false;
};

// crawls back to Yazma and feeds her
class AtalDazarSoulspawnAI : public AtalDazarSeekerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AtalDazarSoulspawnAI(c); }
    explicit AtalDazarSoulspawnAI(Creature* pCreature) : AtalDazarSeekerAI(pCreature) {}

protected:
    Unit* findGoal() override
    {
        AtalDazarInstanceScript* pTemple = getAtalDazar();
        Creature* pYazma = pTemple != nullptr ? pTemple->getTracked(BOSS_YAZMA) : nullptr;
        return pYazma != nullptr && pYazma->isAlive() ? pYazma : nullptr;
    }

    float reachRange() const override { return 3.0f; }

    void onReached() override { castSpellOnSelf(SPELL_SOULSPAWN_SOULFEAST, true); }
};

// creeps towards the nearest player and bursts on contact
class AtalDazarEchoOfShadraAI : public AtalDazarSeekerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AtalDazarEchoOfShadraAI(c); }
    explicit AtalDazarEchoOfShadraAI(Creature* pCreature) : AtalDazarSeekerAI(pCreature) {}

protected:
    Unit* findGoal() override
    {
        Unit* pNearest = nullptr;
        float nearestRange = 40.0f;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer == nullptr || !pPlayer->isAlive())
                continue;

            float const range = getCreature()->getDistance(pPlayer);
            if (range < nearestRange)
            {
                nearestRange = range;
                pNearest = pPlayer;
            }
        }

        return pNearest;
    }

    void onReached() override { castSpellAOE(SPELL_ECHO_OF_SHADRA_BURST, true); }
};

void SetupAtalDazar(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ATAL_DAZAR, &AtalDazarInstanceScript::Create);

    mgr->register_creature_script(BOSS_ALUNZA, &AtalDazarAlunzaAI::Create);
    mgr->register_creature_script(BOSS_ALUNZA_ALT, &AtalDazarAlunzaAI::Create);
    mgr->register_creature_script(NPC_CORRUPTED_GOLD, &AtalDazarCorruptedGoldAI::Create);

    mgr->register_creature_script(BOSS_VOLKAAL, &AtalDazarVolkaalAI::Create);
    mgr->register_creature_script(NPC_REANIMATION_TOTEM, &AtalDazarReanimationTotemAI::Create);

    mgr->register_creature_script(BOSS_REZAN, &AtalDazarRezanAI::Create);

    mgr->register_creature_script(BOSS_YAZMA, &AtalDazarYazmaAI::Create);
    mgr->register_creature_script(BOSS_YAZMA_ALT, &AtalDazarYazmaAI::Create);
    mgr->register_creature_script(NPC_SOULSPAWN, &AtalDazarSoulspawnAI::Create);
    mgr->register_creature_script(NPC_ECHO_OF_SHADRA, &AtalDazarEchoOfShadraAI::Create);
}
