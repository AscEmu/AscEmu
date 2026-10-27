/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Liberation_Of_Undermine.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <list>
#include <utility>
#include <vector>

enum LiberationOfUndermineEvents
{
    // The Geargrinder
    EVENT_GEARGRINDER_CALL_BIKERS = 1,
    EVENT_GEARGRINDER_SPEW_OIL,
    EVENT_GEARGRINDER_INCENDIARY_FIRE,
    EVENT_GEARGRINDER_BOMB_VOYAGE,
    EVENT_GEARGRINDER_TANK_BUSTER,
    EVENT_GEARGRINDER_BURNING_SHRAPNEL,
    EVENT_GEARGRINDER_PIT_STOP_OVER,

    // Flarendo and Torq
    EVENT_CAULDRON_SPITE,
    EVENT_CAULDRON_COLLATERAL,
    EVENT_FLARENDO_ERUPTION_STOMP,
    EVENT_FLARENDO_SCRAPBOMB,
    EVENT_FLARENDO_ROARCANNON,
    EVENT_TORQ_LIGHTNING_BASH,
    EVENT_TORQ_STATIC_CHARGE,
    EVENT_TORQ_THUNDERDRUM_SALVO,

    // Rik Reverb
    EVENT_RIK_AMPLIFICATION,
    EVENT_RIK_SONIC_BLAST,
    EVENT_RIK_ECHOING_CHANT,
    EVENT_RIK_SOUND_CANNON,
    EVENT_RIK_FAULTY_ZAP,
    EVENT_RIK_RESONANCE,
    EVENT_RIK_BLOWOUT,
    EVENT_RIK_BLARING_DROP,
    EVENT_RIK_HYPE_OVER,
    EVENT_AMPLIFIER_NOISE_POLLUTION,
    EVENT_AMPLIFIER_RESONANT_ECHOES,

    // Stix Bunkjunker
    EVENT_STIX_CLEANUP_CREW,
    EVENT_STIX_INCINERATOR,
    EVENT_STIX_DEMOLISH,
    EVENT_STIX_MELTDOWN,
    EVENT_STIX_OVERCHARGED_BOLT,
    EVENT_STIX_TRASH_COMPACTOR,

    // Sprocketmonger Lockenstock
    EVENT_LOCKENSTOCK_INVENTIONS,
    EVENT_LOCKENSTOCK_BLISTERIZER,
    EVENT_LOCKENSTOCK_GRAVI_GUNK,
    EVENT_LOCKENSTOCK_SONIC_BA_BOOM,
    EVENT_LOCKENSTOCK_WIRE_TRANSFER,
    EVENT_LOCKENSTOCK_PYRO_PARTY_PACK,
    EVENT_LOCKENSTOCK_FOOT_BLASTERS,
    EVENT_LOCKENSTOCK_BETA_OVER,

    // The One-Armed Bandit
    EVENT_BANDIT_BIG_HIT,
    EVENT_BANDIT_PAY_LINE,
    EVENT_BANDIT_FOUL_EXHAUST,
    EVENT_BANDIT_PRIZE,

    // Mug'Zee
    EVENT_MUGZEE_SWAP_HONCHO,
    EVENT_MUGZEE_MOXIE,
    EVENT_MUG_GOLD_KNUCKLES,
    EVENT_MUG_EARTHSHAKER_GAOL,
    EVENT_MUG_FROSTSHATTER_BOOTS,
    EVENT_MUG_FINGER_GUN,
    EVENT_ZEE_DOUBLE_WHAMMY,
    EVENT_ZEE_CRAWLER_MINES,
    EVENT_ZEE_GOBLIN_ROCKET,
    EVENT_ZEE_SPRAY_AND_PRAY,
    EVENT_ZEE_ROCKETEER_LANDS,
    EVENT_MUGZEE_STATIC_CHARGE,
    EVENT_MUGZEE_BULLETSTORM_OVER,

    // Chrome King Gallywix
    EVENT_GALLYWIX_CANISTERS,
    EVENT_GALLYWIX_GATLING_CANNON,
    EVENT_GALLYWIX_BOMBS,
    EVENT_GALLYWIX_VENTING_HEAT,
    EVENT_GALLYWIX_SUPPRESSION,
    EVENT_GALLYWIX_EGO_CHECK,
    EVENT_GALLYWIX_GIGA_BLAST,
    EVENT_GALLYWIX_CRONIES,
    EVENT_GALLYWIX_DOCKED_OVER,

    // adds with a fixed kit
    EVENT_UNDERMINE_ADD_FIRST_MOVE,
    EVENT_UNDERMINE_ADD_SECOND_MOVE
};

enum LiberationOfUndermineActions
{
    ACTION_GEARGRINDER_PLATING_STRIPPED = 1,
    ACTION_CAULDRON_PARTNER_FELL,
    ACTION_AMPLIFIER_RESONANCE
};

namespace
{
    float const UndermineTwoPi = 6.28318531f;
    uint32_t const UndermineEnergyFull = 100;

    uint32_t const GeargrinderBerserkMs = 480000;
    uint32_t const CauldronBerserkMs = 420000;
    uint32_t const RikBerserkMs = 480000;
    uint32_t const StixBerserkMs = 480000;
    uint32_t const LockenstockBerserkMs = 540000;
    uint32_t const BanditBerserkMs = 540000;
    uint32_t const MugZeeBerserkMs = 600000;
    uint32_t const GallywixBerserkMs = 600000;

    // milliseconds per energy point
    uint32_t const GeargrinderCarnageStepMs = 1500;
    uint32_t const CauldronClashStepMs = 1000;
    uint32_t const RikHypeStepMs = 1100;
    uint32_t const StixSortingStepMs = 1000;
    uint32_t const LockenstockBetaStepMs = 1100;
    uint32_t const BanditSpinStepMs = 900;
    uint32_t const GallywixCoilStepMs = 1000;
    uint32_t const AmplifierPowerStepMs = 500;

    // biker kills that break the plating
    uint32_t const GeargrinderPlatingStrength = 3;
    uint32_t const GeargrinderPitStopMs = 25000;
    uint32_t const RikHypeHustleMs = 30000;
    uint32_t const AmplifierResonancePower = 20;
    uint32_t const LockenstockBetaMs = 20000;
    uint32_t const InventionLifetimeMs = 30000;
    uint32_t const MugZeeHonchoMs = 45000;
    uint32_t const MugZeeBulletstormMs = 20000;
    uint32_t const GallywixDockedMs = 20000;

    int32_t const StixOverdrivePct = 25;
    int32_t const BanditRiggedPct = 30;
    int32_t const MugZeeBulletstormPct = 40;
    int32_t const GallywixDockedPct = 50;

    uint32_t const MugZeeHeadsStage = 1;
    uint32_t const MugZeeBulletstormStage = 2;
    uint32_t const MugZeeHonchoStage = 3;

    uint32_t const GallywixChromeStage = 1;
    uint32_t const GallywixManiacStage = 2;
    uint32_t const GallywixDockedStage = 3;
    uint32_t const GallywixArsenalStage = 4;

    // encounter members, optional ones only pass a pull on
    struct UndermineMember
    {
        uint32_t dataId;
        uint32_t entry;
        bool required;
    };

    UndermineMember const UndermineMembers[] =
    {
        { DATA_VEXIE_AND_THE_GEARGRINDERS,  BOSS_THE_GEARGRINDER,   true },
        { DATA_VEXIE_AND_THE_GEARGRINDERS,  NPC_VEXIE_FULLTHROTTLE, false },
        { DATA_CAULDRON_OF_CARNAGE,         BOSS_FLARENDO,          true },
        { DATA_CAULDRON_OF_CARNAGE,         BOSS_TORQ,              true },
        { DATA_MUGZEE,                      BOSS_MUGZEE,            true },
        { DATA_MUGZEE,                      NPC_MUG,                false },
        { DATA_MUGZEE,                      NPC_ZEE,                false }
    };

    uint32_t const GoblinInventions[3] = { NPC_BEAM_TURRET, NPC_ROCKET_LAUNCHER, NPC_MEGA_MAGNET };
    uint32_t const VoidInventions[2] = { NPC_VOID_TURRET, NPC_VOID_LAUNCHER };

    uint32_t const BanditPrizes[6] =
    {
        SPELL_BANDIT_REWARD_SHOCK_AND_FLAME,
        SPELL_BANDIT_REWARD_SHOCK_AND_BOMB,
        SPELL_BANDIT_REWARD_FLAME_AND_BOMB,
        SPELL_BANDIT_REWARD_FLAME_AND_COIN,
        SPELL_BANDIT_REWARD_COIN_AND_SHOCK,
        SPELL_BANDIT_REWARD_COIN_AND_BOMB
    };

    // rigged payouts in this order, the jackpot repeats
    uint32_t const BanditRiggedPrizes[4] =
    {
        SPELL_BANDIT_LINKED_MACHINES,
        SPELL_BANDIT_HOT_HOT_HEAT,
        SPELL_BANDIT_SCATTERED_PAYOUT,
        SPELL_BANDIT_EXPLOSIVE_JACKPOT
    };

    uint32_t const MugKitEvents[4] = { EVENT_MUG_GOLD_KNUCKLES, EVENT_MUG_EARTHSHAKER_GAOL, EVENT_MUG_FROSTSHATTER_BOOTS, EVENT_MUG_FINGER_GUN };
    uint32_t const ZeeKitEvents[4] = { EVENT_ZEE_DOUBLE_WHAMMY, EVENT_ZEE_CRAWLER_MINES, EVENT_ZEE_GOBLIN_ROCKET, EVENT_ZEE_SPRAY_AND_PRAY };

    LocationVector undermineSpotNear(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(UndermineTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // how an add aims a move
    enum class UndermineAim : uint8_t
    {
        Self,
        Victim,
        Player,
        PlayerSpot,
        Area,
        Ally
    };

    struct UndermineAddMove
    {
        uint32_t spellId;
        UndermineAim aim;
        uint32_t firstMs;
        uint32_t repeatMs;
    };

    struct UndermineAddKit
    {
        UndermineAddMove first;
        UndermineAddMove second;
        bool anchored;          // rooted, no melee
        bool dropsSeatFlags;    // rider template flags do not apply on foot
        uint32_t partnerEntry;  // target of ally moves and of the death report
        int32_t deathAction;
    };

    UndermineAddMove const NoMove = { 0, UndermineAim::Self, 0, 0 };

    UndermineAddKit const BikerKit             = { { SPELL_BIKER_HOT_WHEELS, UndermineAim::Area, 4000, 12000 }, NoMove, false, true, BOSS_THE_GEARGRINDER, ACTION_GEARGRINDER_PLATING_STRIPPED };
    UndermineAddKit const PitMechanicKit       = { { SPELL_PIT_MECHANIC_REPAIR, UndermineAim::Ally, 3000, 6000 }, NoMove, false, false, BOSS_THE_GEARGRINDER, 0 };

    UndermineAddKit const ScrapmasterKit       = { { SPELL_SCRAPMASTER_SCRAP_ROCKETS, UndermineAim::Player, 5000, 12000 }, { SPELL_SCRAPMASTER_DUMPSTER_DIVE, UndermineAim::PlayerSpot, 15000, 25000 }, false, false, 0, 0 };
    UndermineAddKit const HyenaKit             = { { SPELL_HYENA_INFECTED_BITE, UndermineAim::Victim, 3000, 8000 }, NoMove, false, false, 0, 0 };
    UndermineAddKit const BombshellKit         = { { SPELL_BOMBSHELL_TERRITORIAL, UndermineAim::Self, 500, 0 }, { SPELL_BOMBSHELL_SHORT_FUSE, UndermineAim::Self, 20000, 0 }, false, false, 0, 0 };

    UndermineAddKit const BeamTurretKit        = { { SPELL_BEAM_TURRET_BLAZING_BEAM, UndermineAim::Self, 3000, 10000 }, NoMove, true, false, 0, 0 };
    UndermineAddKit const RocketLauncherKit    = { { SPELL_ROCKET_LAUNCHER_ROCKET_BARRAGE, UndermineAim::Self, 3000, 10000 }, NoMove, true, false, 0, 0 };
    UndermineAddKit const MegaMagnetKit        = { { SPELL_MEGA_MAGNET_MEGA_MAGNETIZE, UndermineAim::Self, 4000, 12000 }, NoMove, true, false, 0, 0 };
    UndermineAddKit const VoidTurretKit        = { { SPELL_VOID_TURRET_JUMBO_VOID_BEAM, UndermineAim::Self, 3000, 10000 }, NoMove, true, false, 0, 0 };
    UndermineAddKit const VoidLauncherKit      = { { SPELL_VOID_LAUNCHER_VOID_BARRAGE, UndermineAim::Self, 3000, 10000 }, NoMove, true, false, 0, 0 };

    UndermineAddKit const ReelAssistantKit     = { { SPELL_REEL_ASSISTANT_ELECTRIC_BLAST, UndermineAim::Victim, 4000, 8000 }, { SPELL_REEL_ASSISTANT_OVERLOAD, UndermineAim::Area, 10000, 15000 }, false, false, 0, 0 };

    UndermineAddKit const GoonKit              = { { SPELL_GOON_SHAKEDOWN, UndermineAim::PlayerSpot, 6000, 12000 }, NoMove, false, false, 0, 0 };
    UndermineAddKit const RocketeerKit         = { { SPELL_ROCKETEER_DISINTEGRATION_BEAM, UndermineAim::Player, 4000, 12000 }, { SPELL_ROCKETEER_ROCKET_JUMP, UndermineAim::PlayerSpot, 10000, 20000 }, false, false, 0, 0 };
    UndermineAddKit const ElectroShockerKit    = { { SPELL_SHOCKER_SURGING_ARC, UndermineAim::Victim, 3000, 8000 }, { SPELL_SHOCKER_FAULTY_WIRING, UndermineAim::Self, 10000, 20000 }, false, false, 0, 0 };

    UndermineAddKit const TechnicianKit        = { { SPELL_TECHNICIAN_JUICE_IT, UndermineAim::Victim, 5000, 15000 }, { SPELL_CRONY_PARTY_CRASHING_ROCKET, UndermineAim::PlayerSpot, 10000, 20000 }, false, false, 0, 0 };
    UndermineAddKit const SentryKit            = { { SPELL_SENTRY_SHOCK_BARRAGE, UndermineAim::Player, 3000, 10000 }, NoMove, true, false, 0, 0 };
    UndermineAddKit const WrenchmongerKit      = { { SPELL_WRENCHMONGER_WRENCH, UndermineAim::Victim, 4000, 10000 }, { SPELL_WRENCHMONGER_LUMBERING_RAGE, UndermineAim::Self, 20000, 30000 }, false, false, 0, 0 };
    UndermineAddKit const GigaTechnicianKit    = { { SPELL_GIGA_TECHNICIAN_STATIC_ZAP, UndermineAim::Victim, 3000, 8000 }, { SPELL_GIGA_TECHNICIAN_IONIZATION, UndermineAim::Player, 8000, 16000 }, false, false, 0, 0 };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class LiberationOfUndermineInstanceScript : public InstanceScript
{
public:
    explicit LiberationOfUndermineInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(LiberationOfUndermineEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new LiberationOfUndermineInstanceScript(pMapMgr); }

    static LiberationOfUndermineInstanceScript* of(CreatureAIScript* pScript)
    {
        if (pScript == nullptr || pScript->getCreature()->GetMapId() != MAP_LIBERATION_OF_UNDERMINE)
            return nullptr;

        return static_cast<LiberationOfUndermineInstanceScript*>(pScript->getInstanceScript());
    }

    // a fallen member must not end the encounter while a required partner stands
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isEncounterDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (state != EncounterStates::Failed)
            return;

        // fallen members are back for the next attempt
        for (Creature* pMember : getMembers(id, true))
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    std::vector<Creature*> getMembers(uint32_t dataId, bool requiredOnly)
    {
        std::vector<Creature*> members;
        for (UndermineMember const& member : UndermineMembers)
        {
            if (member.dataId != dataId || (requiredOnly && !member.required))
                continue;

            for (Creature* pCreature : getCreatureSetForEntry(member.entry))
                members.push_back(pCreature);
        }

        return members;
    }

    // single boss encounters are down with their boss
    bool isEncounterDown(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getMembers(dataId, true))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    bool isEncounterFighting(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getMembers(dataId, true))
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat())
                return true;
        }

        return false;
    }

    // pulling one member pulls the required ones
    void rallyEncounter(uint32_t dataId, CreatureAIScript* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getMembers(dataId, true))
        {
            if (pMember == pPuller->getCreature() || !pMember->isAlive() || pMember->isInCombat())
                continue;

            pPuller->setZoneWideCombat(pMember);
        }
        mRallying = false;
    }

    void notifyEncounter(uint32_t dataId, Creature* pExcept, int32_t action)
    {
        for (Creature* pMember : getMembers(dataId, true))
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->GetScript() != nullptr)
                pMember->GetScript()->DoAction(action);
        }
    }

private:
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy gauge, berserk and target helpers

class LiberationOfUndermineCombatAI : public CreatureAIScript
{
public:
    explicit LiberationOfUndermineCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mStage = 1;
        mBerserkLeftMs = 0;
        stopEnergy();
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mStage = 0;
        mBerserkLeftMs = 0;
        stopEnergy();

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
        tickBerserk(diff);
        tickEnergy(diff);
        onCombatTick(diff);

        // a full gauge goes first once no cast is running
        if (mEnergyFull && !_isCasting())
        {
            mEnergyFull = false;
            onEnergyFull();
        }

        for (uint8_t budget = 0; budget < 3 && !_isCasting(); ++budget)
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
    virtual void onEnergyFull() {}

    LiberationOfUndermineInstanceScript* getUndermine() { return LiberationOfUndermineInstanceScript::of(this); }

    // one point per stepMs up to 100
    void startEnergy(uint32_t stepMs)
    {
        mEnergy = 0;
        mEnergyClockMs = 0;
        mEnergyStepMs = stepMs;
        mEnergyFull = false;
    }

    void stopEnergy()
    {
        mEnergy = 0;
        mEnergyClockMs = 0;
        mEnergyStepMs = 0;
        mEnergyFull = false;
    }

    void addEnergy(uint32_t points)
    {
        if (mEnergyStepMs == 0 || mEnergyFull)
            return;

        mEnergy += points;
        if (mEnergy >= UndermineEnergyFull)
        {
            mEnergy = 0;
            mEnergyFull = true;
        }
    }

    void armBerserk(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

    // a new stage drops every pending event
    void enterStage(uint32_t stage)
    {
        mStage = stage;
        scriptEvents.resetEvents();
    }

    // living players in random order, the tank only when nobody else is left
    std::vector<Unit*> shuffledPlayers(bool spareTank)
    {
        TargetFilter const filter = spareTank ? TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent) : TargetFilter_Player;
        FilterArgs const args(filter);

        std::vector<Unit*> players;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject != nullptr && isValidUnitTarget(pObject, filter, args))
                players.push_back(static_cast<Unit*>(pObject));
        }

        if (players.empty() && spareTank)
            return shuffledPlayers(false);

        for (size_t left = players.size(); left > 1; --left)
            std::swap(players[left - 1], players[Util::getRandomUInt(static_cast<uint32_t>(left - 1))]);

        return players;
    }

    Unit* anyPlayer(bool spareTank = false)
    {
        std::vector<Unit*> const players = shuffledPlayers(spareTank);
        return players.empty() ? nullptr : players.front();
    }

    void castAtPlayers(uint32_t spellId, size_t count, bool spareTank = false, bool triggered = false)
    {
        std::vector<Unit*> const players = shuffledPlayers(spareTank);
        for (size_t i = 0; i < players.size() && i < count; ++i)
            castSpell(players[i], spellId, triggered);
    }

    // ground targeted spell under random players
    void castAtPlayerSpots(uint32_t spellId, size_t count, bool spareTank = false, bool triggered = true)
    {
        std::vector<Unit*> const players = shuffledPlayers(spareTank);
        for (size_t i = 0; i < players.size() && i < count; ++i)
            getCreature()->castSpellLoc(players[i]->GetPosition(), spellId, triggered);
    }

    // summoned add that joins the fight at once
    Creature* bringAdd(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t durationMs = 0)
    {
        Creature* pAdd = summonCreature(entry, spot, despawnType, durationMs);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector spotAroundSelf(float minRange, float maxRange) { return undermineSpotNear(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

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
        castSpellOnSelf(SPELL_UNDERMINE_BERSERK, true);
    }

    void tickEnergy(uint32_t diff)
    {
        if (mEnergyStepMs == 0 || mEnergyFull)
            return;

        mEnergyClockMs += diff;
        while (mEnergyClockMs >= mEnergyStepMs && !mEnergyFull)
        {
            mEnergyClockMs -= mEnergyStepMs;
            if (++mEnergy >= UndermineEnergyFull)
            {
                mEnergy = 0;
                mEnergyClockMs = 0;
                mEnergyFull = true;
            }
        }
    }

    uint32_t mBerserkLeftMs = 0;
    uint32_t mEnergy = 0;
    uint32_t mEnergyClockMs = 0;
    uint32_t mEnergyStepMs = 0;
    bool mEnergyFull = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class LiberationOfUndermineBossAI : public LiberationOfUndermineCombatAI
{
public:
    LiberationOfUndermineBossAI(Creature* pCreature, uint32_t dataId) : LiberationOfUndermineCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (LiberationOfUndermineInstanceScript* pUndermine = getUndermine())
            pUndermine->rallyEncounter(mDataId, this);

        LiberationOfUndermineCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once no partner fights on
                LiberationOfUndermineInstanceScript* pUndermine = getUndermine();
                if (pUndermine == nullptr || !pUndermine->isEncounterFighting(mDataId, getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        LiberationOfUndermineCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            LiberationOfUndermineInstanceScript* pUndermine = getUndermine();
            if (pUndermine == nullptr || pUndermine->isEncounterDown(mDataId, getCreature()))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
        stopEnergy();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Riders and heads of an encounter, a pull is passed on to the boss

class LiberationOfUndermineCompanionAI : public CreatureAIScript
{
public:
    static CreatureAIScript* CreateVexie(Creature* c) { return new LiberationOfUndermineCompanionAI(c, DATA_VEXIE_AND_THE_GEARGRINDERS); }
    static CreatureAIScript* CreateHead(Creature* c) { return new LiberationOfUndermineCompanionAI(c, DATA_MUGZEE); }

    LiberationOfUndermineCompanionAI(Creature* pCreature, uint32_t dataId) : CreatureAIScript(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        if (LiberationOfUndermineInstanceScript* pUndermine = LiberationOfUndermineInstanceScript::of(this))
            pUndermine->rallyEncounter(mDataId, this);
    }

private:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Adds with a fixed kit of up to two moves

class LiberationOfUndermineAddAI : public LiberationOfUndermineCombatAI
{
public:
    static CreatureAIScript* CreateBiker(Creature* c) { return new LiberationOfUndermineAddAI(c, BikerKit); }
    static CreatureAIScript* CreatePitMechanic(Creature* c) { return new LiberationOfUndermineAddAI(c, PitMechanicKit); }
    static CreatureAIScript* CreateScrapmaster(Creature* c) { return new LiberationOfUndermineAddAI(c, ScrapmasterKit); }
    static CreatureAIScript* CreateHyena(Creature* c) { return new LiberationOfUndermineAddAI(c, HyenaKit); }
    static CreatureAIScript* CreateBombshell(Creature* c) { return new LiberationOfUndermineAddAI(c, BombshellKit); }
    static CreatureAIScript* CreateBeamTurret(Creature* c) { return new LiberationOfUndermineAddAI(c, BeamTurretKit); }
    static CreatureAIScript* CreateRocketLauncher(Creature* c) { return new LiberationOfUndermineAddAI(c, RocketLauncherKit); }
    static CreatureAIScript* CreateMegaMagnet(Creature* c) { return new LiberationOfUndermineAddAI(c, MegaMagnetKit); }
    static CreatureAIScript* CreateVoidTurret(Creature* c) { return new LiberationOfUndermineAddAI(c, VoidTurretKit); }
    static CreatureAIScript* CreateVoidLauncher(Creature* c) { return new LiberationOfUndermineAddAI(c, VoidLauncherKit); }
    static CreatureAIScript* CreateReelAssistant(Creature* c) { return new LiberationOfUndermineAddAI(c, ReelAssistantKit); }
    static CreatureAIScript* CreateGoon(Creature* c) { return new LiberationOfUndermineAddAI(c, GoonKit); }
    static CreatureAIScript* CreateRocketeer(Creature* c) { return new LiberationOfUndermineAddAI(c, RocketeerKit); }
    static CreatureAIScript* CreateElectroShocker(Creature* c) { return new LiberationOfUndermineAddAI(c, ElectroShockerKit); }
    static CreatureAIScript* CreateTechnician(Creature* c) { return new LiberationOfUndermineAddAI(c, TechnicianKit); }
    static CreatureAIScript* CreateSentry(Creature* c) { return new LiberationOfUndermineAddAI(c, SentryKit); }
    static CreatureAIScript* CreateWrenchmonger(Creature* c) { return new LiberationOfUndermineAddAI(c, WrenchmongerKit); }
    static CreatureAIScript* CreateGigaTechnician(Creature* c) { return new LiberationOfUndermineAddAI(c, GigaTechnicianKit); }

    LiberationOfUndermineAddAI(Creature* pCreature, UndermineAddKit const& kit) : LiberationOfUndermineCombatAI(pCreature), mKit(kit) {}

    void OnLoad() override
    {
        if (mKit.dropsSeatFlags)
            removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IGNORE_PLAYER_COMBAT);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        scriptEvents.resetEvents();
        if (mKit.deathAction == 0 || mKit.partnerEntry == 0)
            return;

        Creature* pPartner = findNearestCreature(mKit.partnerEntry, 200.0f);
        if (pPartner != nullptr && pPartner->isAlive() && pPartner->GetScript() != nullptr)
            pPartner->GetScript()->DoAction(mKit.deathAction);
    }

protected:
    void onEngage() override
    {
        if (mKit.anchored)
        {
            setRooted(true);
            _setMeleeDisabled(true);
        }

        if (mKit.first.spellId != 0)
            scriptEvents.addEvent(EVENT_UNDERMINE_ADD_FIRST_MOVE, mKit.first.firstMs);
        if (mKit.second.spellId != 0)
            scriptEvents.addEvent(EVENT_UNDERMINE_ADD_SECOND_MOVE, mKit.second.firstMs);
    }

    void onReset() override
    {
        if (!mKit.anchored)
            return;

        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onEvent(uint32_t eventId) override
    {
        UndermineAddMove const* pMove = nullptr;
        if (eventId == EVENT_UNDERMINE_ADD_FIRST_MOVE)
            pMove = &mKit.first;
        else if (eventId == EVENT_UNDERMINE_ADD_SECOND_MOVE)
            pMove = &mKit.second;

        if (pMove == nullptr || pMove->spellId == 0)
            return;

        perform(*pMove);
        if (pMove->repeatMs != 0)
            scriptEvents.addEvent(eventId, pMove->repeatMs);
    }

private:
    void perform(UndermineAddMove const& move)
    {
        switch (move.aim)
        {
            case UndermineAim::Self:
                castSpellOnSelf(move.spellId);
                break;
            case UndermineAim::Victim:
                castSpellOnVictim(move.spellId);
                break;
            case UndermineAim::Player:
                castAtPlayers(move.spellId, 1);
                break;
            case UndermineAim::PlayerSpot:
                castAtPlayerSpots(move.spellId, 1, false, false);
                break;
            case UndermineAim::Area:
                castSpellAOE(move.spellId);
                break;
            case UndermineAim::Ally:
            {
                Creature* pAlly = findNearestCreature(mKit.partnerEntry, 150.0f);
                if (pAlly != nullptr && pAlly->isAlive())
                    castSpell(pAlly, move.spellId);
                break;
            }
        }
    }

    UndermineAddKit const& mKit;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Vexie and the Geargrinders

// plating holds until three bikers are down, then the Pit Stop opens it up
class LiberationOfUndermineGeargrinderAI : public LiberationOfUndermineBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LiberationOfUndermineGeargrinderAI(c); }
    explicit LiberationOfUndermineGeargrinderAI(Creature* pCreature) : LiberationOfUndermineBossAI(pCreature, DATA_VEXIE_AND_THE_GEARGRINDERS) {}

    // the race intro is not played, the tank can be attacked from the start
    void OnLoad() override { removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IGNORE_PLAYER_COMBAT); }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_GEARGRINDER_PLATING_STRIPPED || mStage != 1 || !_isInCombat() || mPlatingLeft == 0)
            return;

        if (--mPlatingLeft == 0)
            startPitStop();
    }

protected:
    void onEngage() override
    {
        armBerserk(GeargrinderBerserkMs);
        startFuryRoad(false);
    }

    void onEnergyFull() override
    {
        warn("The Geargrinder unleashes Unrelenting CAR-nage!");
        castSpellOnSelf(SPELL_GEARGRINDER_UNRELENTING_CARNAGE);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GEARGRINDER_CALL_BIKERS:
                callBikers();
                scriptEvents.addEvent(EVENT_GEARGRINDER_CALL_BIKERS, 30000);
                break;
            case EVENT_GEARGRINDER_SPEW_OIL:
                castSpellAOE(SPELL_GEARGRINDER_SPEW_OIL);
                scriptEvents.addEvent(EVENT_GEARGRINDER_SPEW_OIL, 20000);
                break;
            case EVENT_GEARGRINDER_INCENDIARY_FIRE:
                castSpellAOE(SPELL_GEARGRINDER_INCENDIARY_FIRE);
                scriptEvents.addEvent(EVENT_GEARGRINDER_INCENDIARY_FIRE, 25000);
                break;
            case EVENT_GEARGRINDER_BOMB_VOYAGE:
                castAtPlayers(SPELL_GEARGRINDER_BOMB_VOYAGE, 1, true);
                scriptEvents.addEvent(EVENT_GEARGRINDER_BOMB_VOYAGE, 30000);
                break;
            case EVENT_GEARGRINDER_TANK_BUSTER:
                castSpellOnVictim(SPELL_GEARGRINDER_TANK_BUSTER);
                scriptEvents.addEvent(EVENT_GEARGRINDER_TANK_BUSTER, 18000);
                break;
            case EVENT_GEARGRINDER_BURNING_SHRAPNEL:
                castAtPlayerSpots(SPELL_GEARGRINDER_BURNING_SHRAPNEL, isHeroic() ? 3 : 2);
                scriptEvents.addEvent(EVENT_GEARGRINDER_BURNING_SHRAPNEL, 8000);
                break;
            case EVENT_GEARGRINDER_PIT_STOP_OVER:
                startFuryRoad(true);
                break;
            default:
                break;
        }
    }

private:
    void startFuryRoad(bool afterPitStop)
    {
        enterStage(1);
        mPlatingLeft = GeargrinderPlatingStrength;

        if (afterPitStop)
        {
            _removeAura(SPELL_GEARGRINDER_MECHANICAL_BREAKDOWN);
            _removeAura(SPELL_GEARGRINDER_BACKFIRE);
            warn("Vexie tunes up the Geargrinder!");
            castSpellOnSelf(SPELL_GEARGRINDER_TUNE_UP, true);
        }

        castSpellOnSelf(SPELL_GEARGRINDER_PROTECTIVE_PLATING, true);
        startEnergy(GeargrinderCarnageStepMs);

        scriptEvents.addEvent(EVENT_GEARGRINDER_CALL_BIKERS, 8000);
        scriptEvents.addEvent(EVENT_GEARGRINDER_TANK_BUSTER, 10000);
        scriptEvents.addEvent(EVENT_GEARGRINDER_SPEW_OIL, 14000);
        scriptEvents.addEvent(EVENT_GEARGRINDER_INCENDIARY_FIRE, 19000);
        scriptEvents.addEvent(EVENT_GEARGRINDER_BOMB_VOYAGE, 24000);
    }

    void startPitStop()
    {
        enterStage(2);
        stopEnergy();
        _removeAura(SPELL_GEARGRINDER_PROTECTIVE_PLATING);

        warn("The Geargrinder breaks down! Vexie calls for a Pit Stop!");
        castSpellOnSelf(SPELL_GEARGRINDER_MECHANICAL_BREAKDOWN, true);
        castSpellOnSelf(SPELL_GEARGRINDER_BACKFIRE, true);

        uint8_t const mechanics = isHeroic() ? 3 : 2;
        for (uint8_t i = 0; i < mechanics; ++i)
            bringAdd(NPC_PIT_MECHANIC, spotAroundSelf(6.0f, 12.0f));

        scriptEvents.addEvent(EVENT_GEARGRINDER_BURNING_SHRAPNEL, 4000);
        scriptEvents.addEvent(EVENT_GEARGRINDER_PIT_STOP_OVER, GeargrinderPitStopMs);
    }

    void callBikers()
    {
        castSpellOnSelf(SPELL_GEARGRINDER_CALL_BIKERS, true);

        uint8_t const bikers = isHeroic() ? 3 : 2;
        for (uint8_t i = 0; i < bikers; ++i)
            bringAdd(NPC_GEARGRINDER_BIKER, spotAroundSelf(15.0f, 25.0f));
    }

    uint32_t mPlatingLeft = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Cauldron of Carnage

// both fill their gauge together and meet in a Colossal Clash
class LiberationOfUndermineBrawlerAI : public LiberationOfUndermineBossAI
{
public:
    LiberationOfUndermineBrawlerAI(Creature* pCreature, uint32_t spiteSpellId) : LiberationOfUndermineBossAI(pCreature, DATA_CAULDRON_OF_CARNAGE), mSpiteSpellId(spiteSpellId) {}

    void DoAction(int32_t action) override
    {
        // the last fighter standing takes the crowd
        if (action == ACTION_CAULDRON_PARTNER_FELL && _isInCombat())
            castSpellOnSelf(SPELL_CAULDRON_KING_OF_CARNAGE, true);
    }

protected:
    void onEngage() override
    {
        armBerserk(CauldronBerserkMs);
        startEnergy(CauldronClashStepMs);
        scriptEvents.addEvent(EVENT_CAULDRON_SPITE, 20000);
        onBrawl();
    }

    void onDefeat() override
    {
        if (LiberationOfUndermineInstanceScript* pUndermine = getUndermine())
            pUndermine->notifyEncounter(mDataId, getCreature(), ACTION_CAULDRON_PARTNER_FELL);
    }

    void onEnergyFull() override
    {
        castSpellOnSelf(SPELL_CAULDRON_COLOSSAL_CLASH);
        scriptEvents.addEvent(EVENT_CAULDRON_COLLATERAL, 4000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CAULDRON_SPITE:
                castSpellOnSelf(mSpiteSpellId, true);
                scriptEvents.addEvent(EVENT_CAULDRON_SPITE, 20000);
                break;
            case EVENT_CAULDRON_COLLATERAL:
                onCollateral();
                break;
            default:
                onBrawlEvent(eventId);
                break;
        }
    }

    virtual void onBrawl() {}
    virtual void onCollateral() {}
    virtual void onBrawlEvent(uint32_t /*eventId*/) {}

private:
    uint32_t mSpiteSpellId;
};

class LiberationOfUndermineFlarendoAI : public LiberationOfUndermineBrawlerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LiberationOfUndermineFlarendoAI(c); }
    explicit LiberationOfUndermineFlarendoAI(Creature* pCreature) : LiberationOfUndermineBrawlerAI(pCreature, SPELL_FLARENDO_BLISTERING_SPITE) {}

protected:
    void onBrawl() override
    {
        scriptEvents.addEvent(EVENT_FLARENDO_ERUPTION_STOMP, 10000);
        scriptEvents.addEvent(EVENT_FLARENDO_SCRAPBOMB, 15000);
        scriptEvents.addEvent(EVENT_FLARENDO_ROARCANNON, 25000);
    }

    void onEnergyFull() override
    {
        warn("Flarendo and Torq charge into a Colossal Clash!");
        LiberationOfUndermineBrawlerAI::onEnergyFull();
    }

    void onCollateral() override { castSpellAOE(SPELL_FLARENDO_FIERY_WAVE); }

    void onBrawlEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_FLARENDO_ERUPTION_STOMP:
                castSpellOnVictim(SPELL_FLARENDO_ERUPTION_STOMP);
                scriptEvents.addEvent(EVENT_FLARENDO_ERUPTION_STOMP, 25000);
                break;
            case EVENT_FLARENDO_SCRAPBOMB:
                castAtPlayerSpots(SPELL_FLARENDO_SCRAPBOMB, isHeroic() ? 3 : 2);
                scriptEvents.addEvent(EVENT_FLARENDO_SCRAPBOMB, 35000);
                break;
            case EVENT_FLARENDO_ROARCANNON:
                castSpellAOE(SPELL_FLARENDO_BLASTBURN_ROARCANNON);
                scriptEvents.addEvent(EVENT_FLARENDO_ROARCANNON, 40000);
                break;
            default:
                break;
        }
    }
};

class LiberationOfUndermineTorqAI : public LiberationOfUndermineBrawlerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LiberationOfUndermineTorqAI(c); }
    explicit LiberationOfUndermineTorqAI(Creature* pCreature) : LiberationOfUndermineBrawlerAI(pCreature, SPELL_TORQ_GALVANIZED_SPITE) {}

protected:
    void onBrawl() override
    {
        scriptEvents.addEvent(EVENT_TORQ_STATIC_CHARGE, 8000);
        scriptEvents.addEvent(EVENT_TORQ_LIGHTNING_BASH, 12000);
        scriptEvents.addEvent(EVENT_TORQ_THUNDERDRUM_SALVO, 22000);
    }

    void onCollateral() override { castAtPlayerSpots(SPELL_TORQ_ZAPBOLT, 3); }

    void onBrawlEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TORQ_STATIC_CHARGE:
                castAtPlayers(SPELL_TORQ_STATIC_CHARGE, isHeroic() ? 3 : 2, true, true);
                scriptEvents.addEvent(EVENT_TORQ_STATIC_CHARGE, 30000);
                break;
            case EVENT_TORQ_LIGHTNING_BASH:
                castSpellOnVictim(SPELL_TORQ_LIGHTNING_BASH);
                scriptEvents.addEvent(EVENT_TORQ_LIGHTNING_BASH, 25000);
                break;
            case EVENT_TORQ_THUNDERDRUM_SALVO:
                castSpellAOE(SPELL_TORQ_THUNDERDRUM_SALVO);
                scriptEvents.addEvent(EVENT_TORQ_THUNDERDRUM_SALVO, 40000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rik Reverb

class LiberationOfUndermineRikReverbAI : public LiberationOfUndermineBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LiberationOfUndermineRikReverbAI(c); }
    explicit LiberationOfUndermineRikReverbAI(Creature* pCreature) : LiberationOfUndermineBossAI(pCreature, DATA_RIK_REVERB) {}

protected:
    void onEngage() override
    {
        armBerserk(RikBerserkMs);
        startPartyStarter();
    }

    void onReset() override { _removeAura(SPELL_RIK_SOUND_CLOUD); }

    // full energy drops the beat
    void onEnergyFull() override
    {
        enterStage(2);
        stopEnergy();

        warn("Rik Reverb drops the beat! The amplifiers blow out!");
        castSpellOnSelf(SPELL_RIK_SOUND_CLOUD, true);
        castSpellOnSelf(SPELL_RIK_HYPE_FEVER, true);

        scriptEvents.addEvent(EVENT_RIK_BLOWOUT, 2000);
        scriptEvents.addEvent(EVENT_RIK_BLARING_DROP, 5000);
        scriptEvents.addEvent(EVENT_RIK_HYPE_OVER, RikHypeHustleMs);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RIK_AMPLIFICATION:
                placeAmplifiers();
                scriptEvents.addEvent(EVENT_RIK_AMPLIFICATION, 40000);
                break;
            case EVENT_RIK_SONIC_BLAST:
                castSpellOnSelf(SPELL_RIK_SONIC_BLAST);
                scriptEvents.addEvent(EVENT_RIK_SONIC_BLAST, 20000);
                break;
            case EVENT_RIK_FAULTY_ZAP:
                castSpellAOE(SPELL_RIK_FAULTY_ZAP);
                scriptEvents.addEvent(EVENT_RIK_FAULTY_ZAP, 25000);
                break;
            case EVENT_RIK_ECHOING_CHANT:
                castSpellAOE(SPELL_RIK_ECHOING_CHANT);
                scriptEvents.addEvent(EVENT_RIK_ECHOING_CHANT, 30000);
                break;
            case EVENT_RIK_RESONANCE:
                resonate();
                scriptEvents.addEvent(EVENT_RIK_RESONANCE, 20000);
                break;
            case EVENT_RIK_SOUND_CANNON:
                warn("Rik Reverb aims his Sound Cannon!");
                castAtPlayers(SPELL_RIK_SOUND_CANNON, 1, true);
                scriptEvents.addEvent(EVENT_RIK_SOUND_CANNON, 35000);
                break;
            case EVENT_RIK_BLOWOUT:
                blowOutAmplifiers();
                scriptEvents.addEvent(EVENT_RIK_BLOWOUT, 10000);
                break;
            case EVENT_RIK_BLARING_DROP:
                castAtPlayers(SPELL_RIK_BLARING_DROP, isHeroic() ? 4 : 3, false, true);
                scriptEvents.addEvent(EVENT_RIK_BLARING_DROP, 8000);
                break;
            case EVENT_RIK_HYPE_OVER:
                // the yard is cleared for the next set
                summons.despawnAll();
                startPartyStarter();
                break;
            default:
                break;
        }
    }

private:
    void startPartyStarter()
    {
        enterStage(1);
        _removeAura(SPELL_RIK_SOUND_CLOUD);
        startEnergy(RikHypeStepMs);

        scriptEvents.addEvent(EVENT_RIK_AMPLIFICATION, 4000);
        scriptEvents.addEvent(EVENT_RIK_SONIC_BLAST, 10000);
        scriptEvents.addEvent(EVENT_RIK_FAULTY_ZAP, 14000);
        scriptEvents.addEvent(EVENT_RIK_ECHOING_CHANT, 18000);
        scriptEvents.addEvent(EVENT_RIK_RESONANCE, 22000);
        scriptEvents.addEvent(EVENT_RIK_SOUND_CANNON, 26000);
    }

    void placeAmplifiers()
    {
        castSpellAOE(SPELL_RIK_AMPLIFICATION, true);
        for (uint8_t i = 0; i < 3; ++i)
            bringAdd(NPC_AMPLIFIER, spotAroundSelf(15.0f, 30.0f));
    }

    // feeds extra power into the closest amplifier
    void resonate()
    {
        Creature* pAmplifier = findNearestCreature(NPC_AMPLIFIER, 60.0f);
        if (pAmplifier == nullptr || !pAmplifier->isAlive())
            return;

        castSpell(pAmplifier, SPELL_RIK_RESONANCE, true);
        if (pAmplifier->GetScript() != nullptr)
            pAmplifier->GetScript()->DoAction(ACTION_AMPLIFIER_RESONANCE);
    }

    void blowOutAmplifiers()
    {
        std::list<Creature*> amplifiers;
        GetCreatureListWithEntryInGrid(amplifiers, NPC_AMPLIFIER, 100.0f);
        for (Creature* pAmplifier : amplifiers)
        {
            if (pAmplifier != nullptr && pAmplifier->isAlive())
                getCreature()->castSpellLoc(pAmplifier->GetPosition(), SPELL_RIK_BLOWOUT, true);
        }
    }
};

// placed by Rik, cannot be destroyed and goes haywire at full power
class LiberationOfUndermineAmplifierAI : public LiberationOfUndermineCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LiberationOfUndermineAmplifierAI(c); }
    explicit LiberationOfUndermineAmplifierAI(Creature* pCreature) : LiberationOfUndermineCombatAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr)
            *damage = 0;
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_AMPLIFIER_RESONANCE && _isInCombat())
            addEnergy(AmplifierResonancePower);
    }

protected:
    void onEngage() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        startEnergy(AmplifierPowerStepMs);
        scriptEvents.addEvent(EVENT_AMPLIFIER_NOISE_POLLUTION, 6000);
        scriptEvents.addEvent(EVENT_AMPLIFIER_RESONANT_ECHOES, 9000);
    }

    void onReset() override
    {
        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onEnergyFull() override
    {
        stopEnergy();
        warn("An Amplifier goes Haywire!");
        castSpellOnSelf(SPELL_AMPLIFIER_HAYWIRE, true);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AMPLIFIER_NOISE_POLLUTION:
                castSpellAOE(SPELL_AMPLIFIER_NOISE_POLLUTION);
                scriptEvents.addEvent(EVENT_AMPLIFIER_NOISE_POLLUTION, 8000);
                break;
            case EVENT_AMPLIFIER_RESONANT_ECHOES:
                castAtPlayers(SPELL_AMPLIFIER_RESONANT_ECHOES, 1);
                scriptEvents.addEvent(EVENT_AMPLIFIER_RESONANT_ECHOES, 15000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Stix Bunkjunker

class LiberationOfUndermineStixAI : public LiberationOfUndermineBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LiberationOfUndermineStixAI(c); }
    explicit LiberationOfUndermineStixAI(Creature* pCreature) : LiberationOfUndermineBossAI(pCreature, DATA_STIX_BUNKJUNKER) {}

protected:
    void onEngage() override
    {
        armBerserk(StixBerserkMs);
        mOverdrive = false;
        startEnergy(StixSortingStepMs);

        scriptEvents.addEvent(EVENT_STIX_DEMOLISH, 8000);
        scriptEvents.addEvent(EVENT_STIX_INCINERATOR, 12000);
        scriptEvents.addEvent(EVENT_STIX_CLEANUP_CREW, 20000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mOverdrive || _getHealthPercent() > StixOverdrivePct)
            return;

        mOverdrive = true;
        warn("Stix Bunkjunker kicks into Overdrive!");
        castSpellOnSelf(SPELL_STIX_OVERDRIVE, true);
        castSpellOnSelf(SPELL_STIX_MAXIMUM_OUTPUT, true);
        scriptEvents.addEvent(EVENT_STIX_OVERCHARGED_BOLT, 3000);
        scriptEvents.addEvent(EVENT_STIX_TRASH_COMPACTOR, 10000);
    }

    void onEnergyFull() override
    {
        warn("Stix Bunkjunker begins Electromagnetic Sorting!");
        castSpellOnSelf(SPELL_STIX_ELECTROMAGNETIC_SORTING);

        uint8_t const bombshells = isHeroic() ? 3 : 2;
        for (uint8_t i = 0; i < bombshells; ++i)
            bringAdd(NPC_TERRITORIAL_BOMBSHELL, spotAroundSelf(10.0f, 25.0f));
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_STIX_DEMOLISH:
                castSpellOnVictim(SPELL_STIX_DEMOLISH);
                scriptEvents.addEvent(EVENT_STIX_MELTDOWN, 4000);
                scriptEvents.addEvent(EVENT_STIX_DEMOLISH, 20000);
                break;
            case EVENT_STIX_MELTDOWN:
                castSpellOnVictim(SPELL_STIX_MELTDOWN);
                break;
            case EVENT_STIX_INCINERATOR:
                castSpellOnSelf(SPELL_STIX_INCINERATOR);
                scriptEvents.addEvent(EVENT_STIX_INCINERATOR, 25000);
                break;
            case EVENT_STIX_CLEANUP_CREW:
                warn("Stix Bunkjunker calls in his Cleanup Crew!");
                bringAdd(NPC_SCRAPMASTER, spotAroundSelf(15.0f, 25.0f));
                for (uint8_t i = 0; i < 2; ++i)
                    bringAdd(NPC_JUNKYARD_HYENA, spotAroundSelf(15.0f, 25.0f));
                scriptEvents.addEvent(EVENT_STIX_CLEANUP_CREW, 45000);
                break;
            case EVENT_STIX_OVERCHARGED_BOLT:
                castSpellAOE(SPELL_STIX_OVERCHARGED_BOLT);
                scriptEvents.addEvent(EVENT_STIX_OVERCHARGED_BOLT, 6000);
                break;
            case EVENT_STIX_TRASH_COMPACTOR:
                castSpellAOE(SPELL_STIX_TRASH_COMPACTOR);
                scriptEvents.addEvent(EVENT_STIX_TRASH_COMPACTOR, 25000);
                break;
            default:
                break;
        }
    }

private:
    bool mOverdrive = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sprocketmonger Lockenstock

class LiberationOfUndermineLockenstockAI : public LiberationOfUndermineBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LiberationOfUndermineLockenstockAI(c); }
    explicit LiberationOfUndermineLockenstockAI(Creature* pCreature) : LiberationOfUndermineBossAI(pCreature, DATA_SPROCKETMONGER_LOCKENSTOCK) {}

protected:
    void onEngage() override
    {
        armBerserk(LockenstockBerserkMs);
        mVoidTech = false;
        startAssembly();
    }

    void onReset() override { _removeAura(SPELL_LOCKENSTOCK_BLEEDING_EDGE); }

    // full energy starts the Beta Launch behind a shield
    void onEnergyFull() override
    {
        enterStage(2);
        stopEnergy();

        warn("Sprocketmonger Lockenstock initiates the Beta Launch!");
        castSpellAOE(SPELL_LOCKENSTOCK_BETA_LAUNCH);
        castSpellOnSelf(SPELL_LOCKENSTOCK_BLEEDING_EDGE, true);
        scriptEvents.addEvent(EVENT_LOCKENSTOCK_BETA_OVER, LockenstockBetaMs);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LOCKENSTOCK_INVENTIONS:
                activateInventions();
                scriptEvents.addEvent(EVENT_LOCKENSTOCK_INVENTIONS, 35000);
                break;
            case EVENT_LOCKENSTOCK_BLISTERIZER:
                castSpellOnVictim(SPELL_LOCKENSTOCK_BLISTERIZER_MK_II);
                scriptEvents.addEvent(EVENT_LOCKENSTOCK_BLISTERIZER, 15000);
                break;
            case EVENT_LOCKENSTOCK_GRAVI_GUNK:
                castSpellOnVictim(SPELL_LOCKENSTOCK_GRAVI_GUNK);
                scriptEvents.addEvent(EVENT_LOCKENSTOCK_GRAVI_GUNK, 20000);
                break;
            case EVENT_LOCKENSTOCK_SONIC_BA_BOOM:
                castSpellAOE(SPELL_LOCKENSTOCK_SONIC_BA_BOOM);
                scriptEvents.addEvent(EVENT_LOCKENSTOCK_SONIC_BA_BOOM, 30000);
                break;
            case EVENT_LOCKENSTOCK_WIRE_TRANSFER:
                castAtPlayers(SPELL_LOCKENSTOCK_WIRE_TRANSFER, 1, true);
                scriptEvents.addEvent(EVENT_LOCKENSTOCK_WIRE_TRANSFER, 30000);
                break;
            case EVENT_LOCKENSTOCK_PYRO_PARTY_PACK:
                castSpellOnVictim(SPELL_LOCKENSTOCK_PYRO_PARTY_PACK);
                scriptEvents.addEvent(EVENT_LOCKENSTOCK_PYRO_PARTY_PACK, 35000);
                break;
            case EVENT_LOCKENSTOCK_FOOT_BLASTERS:
                castSpellAOE(SPELL_LOCKENSTOCK_FOOT_BLASTERS);
                scriptEvents.addEvent(EVENT_LOCKENSTOCK_FOOT_BLASTERS, 40000);
                break;
            case EVENT_LOCKENSTOCK_BETA_OVER:
                _removeAura(SPELL_LOCKENSTOCK_BLEEDING_EDGE);
                castSpellAOE(SPELL_LOCKENSTOCK_VOIDSPLOSION, true);
                castSpellOnSelf(SPELL_LOCKENSTOCK_UPGRADED_BLOODTECH, true);
                mVoidTech = true;
                startAssembly();
                break;
            default:
                break;
        }
    }

private:
    void startAssembly()
    {
        enterStage(1);
        startEnergy(LockenstockBetaStepMs);

        scriptEvents.addEvent(EVENT_LOCKENSTOCK_BLISTERIZER, 6000);
        scriptEvents.addEvent(EVENT_LOCKENSTOCK_INVENTIONS, 8000);
        scriptEvents.addEvent(EVENT_LOCKENSTOCK_GRAVI_GUNK, 12000);
        scriptEvents.addEvent(EVENT_LOCKENSTOCK_SONIC_BA_BOOM, 16000);
        scriptEvents.addEvent(EVENT_LOCKENSTOCK_WIRE_TRANSFER, 20000);
        scriptEvents.addEvent(EVENT_LOCKENSTOCK_PYRO_PARTY_PACK, 26000);
        scriptEvents.addEvent(EVENT_LOCKENSTOCK_FOOT_BLASTERS, 32000);
    }

    // after a Beta Launch one invention of each batch runs on void tech
    void activateInventions()
    {
        castSpellOnSelf(SPELL_LOCKENSTOCK_ACTIVATE_INVENTIONS, true);

        uint8_t const inventions = isHeroic() ? 3 : 2;
        for (uint8_t i = 0; i < inventions; ++i)
        {
            uint32_t const entry = (mVoidTech && i == 0) ? VoidInventions[Util::getRandomUInt(1)] : GoblinInventions[Util::getRandomUInt(2)];
            bringAdd(entry, spotAroundSelf(15.0f, 30.0f), TIMED_DESPAWN, InventionLifetimeMs);
        }
    }

    bool mVoidTech = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The One-Armed Bandit

class LiberationOfUndermineBanditAI : public LiberationOfUndermineBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LiberationOfUndermineBanditAI(c); }
    explicit LiberationOfUndermineBanditAI(Creature* pCreature) : LiberationOfUndermineBossAI(pCreature, DATA_ONE_ARMED_BANDIT) {}

protected:
    void onEngage() override
    {
        armBerserk(BanditBerserkMs);
        mRigged = false;
        mRiggedPrize = 0;
        startEnergy(BanditSpinStepMs);

        scriptEvents.addEvent(EVENT_BANDIT_BIG_HIT, 8000);
        scriptEvents.addEvent(EVENT_BANDIT_PAY_LINE, 12000);
        scriptEvents.addEvent(EVENT_BANDIT_FOUL_EXHAUST, 20000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mRigged || _getHealthPercent() > BanditRiggedPct)
            return;

        mRigged = true;
        mStage = 2;
        warn("The One-Armed Bandit declares: This game is rigged!");
    }

    void onEnergyFull() override
    {
        if (mRigged)
        {
            warn("The One-Armed Bandit cheats to win!");
            castSpellOnSelf(SPELL_BANDIT_CHEAT_TO_WIN);
            scriptEvents.addEvent(EVENT_BANDIT_PRIZE, 4000);
            return;
        }

        warn("The One-Armed Bandit spins to win!");
        castSpellOnSelf(SPELL_BANDIT_SPIN_TO_WIN);

        uint8_t const assistants = isHeroic() ? 3 : 2;
        for (uint8_t i = 0; i < assistants; ++i)
            bringAdd(NPC_REEL_ASSISTANT, spotAroundSelf(10.0f, 20.0f));

        scriptEvents.addEvent(EVENT_BANDIT_PRIZE, 12000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BANDIT_BIG_HIT:
                castSpellOnVictim(SPELL_BANDIT_THE_BIG_HIT);
                scriptEvents.addEvent(EVENT_BANDIT_BIG_HIT, 20000);
                break;
            case EVENT_BANDIT_PAY_LINE:
                castSpellAOE(SPELL_BANDIT_PAY_LINE);
                scriptEvents.addEvent(EVENT_BANDIT_PAY_LINE, 30000);
                break;
            case EVENT_BANDIT_FOUL_EXHAUST:
                castSpellAOE(SPELL_BANDIT_FOUL_EXHAUST);
                scriptEvents.addEvent(EVENT_BANDIT_FOUL_EXHAUST, 30000);
                break;
            case EVENT_BANDIT_PRIZE:
                payOut();
                break;
            default:
                break;
        }
    }

private:
    // the reels are not tracked, a random prize pays out
    void payOut()
    {
        if (!mRigged)
        {
            castSpellOnSelf(BanditPrizes[Util::getRandomUInt(5)], true);
            return;
        }

        castSpellOnSelf(BanditRiggedPrizes[mRiggedPrize], true);
        if (mRiggedPrize < 3)
            ++mRiggedPrize;
    }

    bool mRigged = false;
    uint8_t mRiggedPrize = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mug'Zee, Heads of Security

// the heads take turns as Head Honcho until both work together at 40%
class LiberationOfUndermineMugZeeAI : public LiberationOfUndermineBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LiberationOfUndermineMugZeeAI(c); }
    explicit LiberationOfUndermineMugZeeAI(Creature* pCreature) : LiberationOfUndermineBossAI(pCreature, DATA_MUGZEE) {}

protected:
    void onEngage() override
    {
        armBerserk(MugZeeBerserkMs);
        enterStage(MugZeeHeadsStage);
        takeCommand(true);
        scriptEvents.addEvent(EVENT_MUGZEE_SWAP_HONCHO, MugZeeHonchoMs);
        scriptEvents.addEvent(EVENT_MUGZEE_MOXIE, 15000);
    }

    void onReset() override { dropHonchoAuras(); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == MugZeeHeadsStage && _getHealthPercent() <= MugZeeBulletstormPct)
            startBulletstorm();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MUGZEE_SWAP_HONCHO:
                takeCommand(!mMugInCommand);
                scriptEvents.addEvent(EVENT_MUGZEE_SWAP_HONCHO, MugZeeHonchoMs);
                break;
            case EVENT_MUGZEE_MOXIE:
                castSpellOnSelf(SPELL_MUGZEE_MOXIE, true);
                scriptEvents.addEvent(EVENT_MUGZEE_MOXIE, 15000);
                break;
            case EVENT_MUG_GOLD_KNUCKLES:
                castSpellOnVictim(SPELL_MUG_MOLTEN_GOLD_KNUCKLES);
                scriptEvents.addEvent(EVENT_MUG_GOLD_KNUCKLES, 20000);
                break;
            case EVENT_MUG_EARTHSHAKER_GAOL:
                earthshakerGaol();
                scriptEvents.addEvent(EVENT_MUG_EARTHSHAKER_GAOL, 30000);
                break;
            case EVENT_MUG_FROSTSHATTER_BOOTS:
                castAtPlayers(SPELL_MUG_FROSTSHATTER_BOOTS, 2, true, true);
                scriptEvents.addEvent(EVENT_MUG_FROSTSHATTER_BOOTS, 30000);
                break;
            case EVENT_MUG_FINGER_GUN:
                castAtPlayers(SPELL_MUG_STORMFURY_FINGER_GUN, 1);
                scriptEvents.addEvent(EVENT_MUG_FINGER_GUN, 25000);
                break;
            case EVENT_ZEE_DOUBLE_WHAMMY:
                castAtPlayers(SPELL_ZEE_DOUBLE_WHAMMY_SHOT, 1, true);
                scriptEvents.addEvent(EVENT_ZEE_DOUBLE_WHAMMY, 25000);
                break;
            case EVENT_ZEE_CRAWLER_MINES:
                for (uint8_t i = 0; i < 3; ++i)
                    getCreature()->castSpellLoc(spotAroundSelf(8.0f, 20.0f), SPELL_ZEE_UNSTABLE_CRAWLER_MINES, true);
                scriptEvents.addEvent(EVENT_ZEE_CRAWLER_MINES, 30000);
                break;
            case EVENT_ZEE_GOBLIN_ROCKET:
                goblinGuidedRocket();
                scriptEvents.addEvent(EVENT_ZEE_GOBLIN_ROCKET, 35000);
                break;
            case EVENT_ZEE_ROCKETEER_LANDS:
                bringAdd(NPC_VOLUNTEER_ROCKETEER, mRocketSpot);
                break;
            case EVENT_ZEE_SPRAY_AND_PRAY:
                castAtPlayers(SPELL_ZEE_SPRAY_AND_PRAY, 1, true);
                scriptEvents.addEvent(EVENT_ZEE_SPRAY_AND_PRAY, 30000);
                break;
            case EVENT_MUGZEE_STATIC_CHARGE:
                castAtPlayerSpots(SPELL_MUGZEE_STATIC_CHARGE, 3);
                scriptEvents.addEvent(EVENT_MUGZEE_STATIC_CHARGE, 5000);
                break;
            case EVENT_MUGZEE_BULLETSTORM_OVER:
                startHeadHoncho();
                break;
            default:
                break;
        }
    }

private:
    void takeCommand(bool mug)
    {
        mMugInCommand = mug;
        for (uint32_t eventId : MugKitEvents)
            scriptEvents.removeEvent(eventId);
        for (uint32_t eventId : ZeeKitEvents)
            scriptEvents.removeEvent(eventId);

        // Moxie builds again for the new head
        dropHonchoAuras();
        castSpellOnSelf(SPELL_MUGZEE_HEAD_HONCHO, true);

        if (mug)
        {
            warn("Mug takes charge as the Head Honcho!");
            castSpellOnSelf(SPELL_MUGZEE_ELEMENTAL_CARNAGE, true);
            scheduleMugKit();
            return;
        }

        warn("Zee takes charge as the Head Honcho!");
        castSpellOnSelf(SPELL_MUGZEE_UNCONTROLLED_DESTRUCTION, true);
        scheduleZeeKit();
        for (uint8_t i = 0; i < 2; ++i)
            bringAdd(NPC_MK_II_ELECTRO_SHOCKER, spotAroundSelf(10.0f, 20.0f));
    }

    void startBulletstorm()
    {
        enterStage(MugZeeBulletstormStage);
        dropHonchoAuras();

        warn("Mug and Zee join forces and unleash a Bulletstorm!");
        castSpellOnSelf(SPELL_MUGZEE_BULLETSTORM);
        scriptEvents.addEvent(EVENT_MUGZEE_STATIC_CHARGE, 2000);
        scriptEvents.addEvent(EVENT_MUGZEE_BULLETSTORM_OVER, MugZeeBulletstormMs);
    }

    void startHeadHoncho()
    {
        enterStage(MugZeeHonchoStage);

        warn("Mug'Zee becomes the Head Honcho!");
        castSpellOnSelf(SPELL_MUGZEE_BLOODLUST, true);
        scheduleMugKit();
        scheduleZeeKit();
        scriptEvents.addEvent(EVENT_MUGZEE_MOXIE, 15000);
    }

    void scheduleMugKit()
    {
        scriptEvents.addEvent(EVENT_MUG_GOLD_KNUCKLES, 6000);
        scriptEvents.addEvent(EVENT_MUG_EARTHSHAKER_GAOL, 10000);
        scriptEvents.addEvent(EVENT_MUG_FROSTSHATTER_BOOTS, 16000);
        scriptEvents.addEvent(EVENT_MUG_FINGER_GUN, 22000);
    }

    void scheduleZeeKit()
    {
        scriptEvents.addEvent(EVENT_ZEE_DOUBLE_WHAMMY, 8000);
        scriptEvents.addEvent(EVENT_ZEE_CRAWLER_MINES, 12000);
        scriptEvents.addEvent(EVENT_ZEE_GOBLIN_ROCKET, 18000);
        scriptEvents.addEvent(EVENT_ZEE_SPRAY_AND_PRAY, 24000);
    }

    void dropHonchoAuras()
    {
        _removeAura(SPELL_MUGZEE_MOXIE);
        _removeAura(SPELL_MUGZEE_ELEMENTAL_CARNAGE);
        _removeAura(SPELL_MUGZEE_UNCONTROLLED_DESTRUCTION);
    }

    // goons rush in to guard the walls
    void earthshakerGaol()
    {
        Unit* pTarget = anyPlayer(true);
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, SPELL_MUG_EARTHSHAKER_GAOL);
        for (uint8_t i = 0; i < 2; ++i)
            bringAdd(NPC_GALLAGIO_GOON, undermineSpotNear(pTarget->GetPosition(), 3.0f, 6.0f));
    }

    // a rocketeer rides the rocket in and survives the impact
    void goblinGuidedRocket()
    {
        Unit* pTarget = anyPlayer(true);
        if (pTarget == nullptr)
            return;

        mRocketSpot = pTarget->GetPosition();
        getCreature()->castSpellLoc(mRocketSpot, SPELL_ZEE_GOBLIN_GUIDED_ROCKET, false);
        scriptEvents.addEvent(EVENT_ZEE_ROCKETEER_LANDS, 3000);
    }

    bool mMugInCommand = true;
    LocationVector mRocketSpot;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Chrome King Gallywix

class LiberationOfUndermineGallywixAI : public LiberationOfUndermineBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LiberationOfUndermineGallywixAI(c); }
    explicit LiberationOfUndermineGallywixAI(Creature* pCreature) : LiberationOfUndermineBossAI(pCreature, DATA_CHROME_KING_GALLYWIX) {}

protected:
    void onEngage() override
    {
        armBerserk(GallywixBerserkMs);
        mDocked = false;
        enterStage(GallywixChromeStage);
        startEnergy(GallywixCoilStepMs);

        scriptEvents.addEvent(EVENT_GALLYWIX_CANISTERS, 8000);
        scriptEvents.addEvent(EVENT_GALLYWIX_GATLING_CANNON, 12000);
        scriptEvents.addEvent(EVENT_GALLYWIX_BOMBS, 18000);
        scriptEvents.addEvent(EVENT_GALLYWIX_VENTING_HEAT, 24000);
        scriptEvents.addEvent(EVENT_GALLYWIX_SUPPRESSION, 30000);
    }

    void onReset() override
    {
        _removeAura(SPELL_GALLYWIX_ARMAGEDDON_CLASS_PLATING);
        _removeAura(SPELL_GALLYWIX_TOTAL_DESTRUCTION);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mDocked && mStage <= GallywixManiacStage && _getHealthPercent() <= GallywixDockedPct)
            startDocked();
    }

    void onEnergyFull() override
    {
        warn("Chrome King Gallywix overloads the Giga Coils!");
        castSpellOnSelf(SPELL_GALLYWIX_GIGA_COILS);
        scriptEvents.addEvent(EVENT_GALLYWIX_GIGA_BLAST, 4000);

        // the first full charge brings in the cronies
        if (mStage == GallywixChromeStage)
        {
            mStage = GallywixManiacStage;
            warn("Gallywix calls in his Darkfuse cronies!");
            scriptEvents.addEvent(EVENT_GALLYWIX_CRONIES, 6000);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GALLYWIX_CANISTERS:
                castSpellOnVictim(currentCanisters());
                scriptEvents.addEvent(EVENT_GALLYWIX_CANISTERS, 25000);
                break;
            case EVENT_GALLYWIX_GATLING_CANNON:
                castAtPlayers(SPELL_GALLYWIX_GATLING_CANNON, 1);
                scriptEvents.addEvent(EVENT_GALLYWIX_GATLING_CANNON, 30000);
                break;
            case EVENT_GALLYWIX_BOMBS:
                if (mStage == GallywixArsenalStage)
                    castSpellOnVictim(SPELL_GALLYWIX_BIGGER_BADDER_BOMB_BLAST);
                else
                    castAtPlayerSpots(SPELL_GALLYWIX_BIG_BAD_BUNCHA_BOMBS, isHeroic() ? 3 : 2);
                scriptEvents.addEvent(EVENT_GALLYWIX_BOMBS, 40000);
                break;
            case EVENT_GALLYWIX_VENTING_HEAT:
                castSpellAOE(SPELL_GALLYWIX_VENTING_HEAT);
                scriptEvents.addEvent(EVENT_GALLYWIX_VENTING_HEAT, 35000);
                break;
            case EVENT_GALLYWIX_SUPPRESSION:
                castSpellOnSelf(SPELL_GALLYWIX_SUPPRESSION);
                scriptEvents.addEvent(EVENT_GALLYWIX_SUPPRESSION, 45000);
                break;
            case EVENT_GALLYWIX_EGO_CHECK:
                castSpellOnVictim(SPELL_GALLYWIX_EGO_CHECK);
                scriptEvents.addEvent(EVENT_GALLYWIX_EGO_CHECK, 25000);
                break;
            case EVENT_GALLYWIX_GIGA_BLAST:
                castAtPlayerSpots(SPELL_GALLYWIX_GIGA_BLAST, 1, false, false);
                break;
            case EVENT_GALLYWIX_CRONIES:
                callCronies();
                scriptEvents.addEvent(EVENT_GALLYWIX_CRONIES, 45000);
                break;
            case EVENT_GALLYWIX_DOCKED_OVER:
                startArsenal();
                break;
            default:
                break;
        }
    }

private:
    uint32_t currentCanisters() const
    {
        if (mStage == GallywixArsenalStage)
            return SPELL_GALLYWIX_TICK_TOCK_CANISTERS;

        return mStage == GallywixManiacStage ? SPELL_GALLYWIX_FUSED_CANISTERS : SPELL_GALLYWIX_SCATTERBLAST_CANISTERS;
    }

    // drops into the arena and blasts away behind heavy plating
    void startDocked()
    {
        mDocked = true;
        enterStage(GallywixDockedStage);
        stopEnergy();

        warn("Chrome King Gallywix drops into the arena and begins TOTAL DESTRUCTION!!!");
        castAtPlayerSpots(SPELL_GALLYWIX_CRATERING, 1);
        castSpellOnSelf(SPELL_GALLYWIX_ARMAGEDDON_CLASS_PLATING, true);
        castSpellOnSelf(SPELL_GALLYWIX_TOTAL_DESTRUCTION, true);
        scriptEvents.addEvent(EVENT_GALLYWIX_DOCKED_OVER, GallywixDockedMs);
    }

    void startArsenal()
    {
        _removeAura(SPELL_GALLYWIX_TOTAL_DESTRUCTION);
        _removeAura(SPELL_GALLYWIX_ARMAGEDDON_CLASS_PLATING);
        enterStage(GallywixArsenalStage);
        startEnergy(GallywixCoilStepMs);

        warn("Chrome King Gallywix shows off his upgraded arsenal!");
        scriptEvents.addEvent(EVENT_GALLYWIX_CANISTERS, 6000);
        scriptEvents.addEvent(EVENT_GALLYWIX_EGO_CHECK, 10000);
        scriptEvents.addEvent(EVENT_GALLYWIX_CRONIES, 12000);
        scriptEvents.addEvent(EVENT_GALLYWIX_BOMBS, 16000);
        scriptEvents.addEvent(EVENT_GALLYWIX_VENTING_HEAT, 22000);
        scriptEvents.addEvent(EVENT_GALLYWIX_SUPPRESSION, 28000);
    }

    void callCronies()
    {
        if (mStage == GallywixArsenalStage)
        {
            bringAdd(NPC_GIGA_JUICED_TECHNICIAN, spotAroundSelf(15.0f, 30.0f));
            bringAdd(NPC_DARKFUSE_WRENCHMONGER, spotAroundSelf(15.0f, 30.0f));
            return;
        }

        bringAdd(NPC_DARKFUSE_TECHNICIAN, spotAroundSelf(15.0f, 30.0f));
        bringAdd(NPC_SHARPSHOT_SENTRY, spotAroundSelf(15.0f, 30.0f));
        bringAdd(NPC_DARKFUSE_WRENCHMONGER, spotAroundSelf(15.0f, 30.0f));
    }

    bool mDocked = false;
};

void SetupLiberationOfUndermine(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_LIBERATION_OF_UNDERMINE, &LiberationOfUndermineInstanceScript::Create);

    // Vexie and the Geargrinders
    mgr->register_creature_script(BOSS_THE_GEARGRINDER, &LiberationOfUndermineGeargrinderAI::Create);
    mgr->register_creature_script(NPC_VEXIE_FULLTHROTTLE, &LiberationOfUndermineCompanionAI::CreateVexie);
    mgr->register_creature_script(NPC_GEARGRINDER_BIKER, &LiberationOfUndermineAddAI::CreateBiker);
    mgr->register_creature_script(NPC_GEARGRINDER_BIKER_ALT, &LiberationOfUndermineAddAI::CreateBiker);
    mgr->register_creature_script(NPC_GEARGRINDER_BIKER_ALT2, &LiberationOfUndermineAddAI::CreateBiker);
    mgr->register_creature_script(NPC_PIT_MECHANIC, &LiberationOfUndermineAddAI::CreatePitMechanic);

    // Cauldron of Carnage
    mgr->register_creature_script(BOSS_FLARENDO, &LiberationOfUndermineFlarendoAI::Create);
    mgr->register_creature_script(BOSS_TORQ, &LiberationOfUndermineTorqAI::Create);

    // Rik Reverb
    mgr->register_creature_script(BOSS_RIK_REVERB, &LiberationOfUndermineRikReverbAI::Create);
    mgr->register_creature_script(NPC_AMPLIFIER, &LiberationOfUndermineAmplifierAI::Create);
    mgr->register_creature_script(NPC_AMPLIFIER_ALT, &LiberationOfUndermineAmplifierAI::Create);
    mgr->register_creature_script(NPC_AMPLIFIER_ALT2, &LiberationOfUndermineAmplifierAI::Create);

    // Stix Bunkjunker
    mgr->register_creature_script(BOSS_STIX_BUNKJUNKER, &LiberationOfUndermineStixAI::Create);
    mgr->register_creature_script(NPC_SCRAPMASTER, &LiberationOfUndermineAddAI::CreateScrapmaster);
    mgr->register_creature_script(NPC_JUNKYARD_HYENA, &LiberationOfUndermineAddAI::CreateHyena);
    mgr->register_creature_script(NPC_TERRITORIAL_BOMBSHELL, &LiberationOfUndermineAddAI::CreateBombshell);

    // Sprocketmonger Lockenstock
    mgr->register_creature_script(BOSS_SPROCKETMONGER_LOCKENSTOCK, &LiberationOfUndermineLockenstockAI::Create);
    mgr->register_creature_script(NPC_BEAM_TURRET, &LiberationOfUndermineAddAI::CreateBeamTurret);
    mgr->register_creature_script(NPC_ROCKET_LAUNCHER, &LiberationOfUndermineAddAI::CreateRocketLauncher);
    mgr->register_creature_script(NPC_MEGA_MAGNET, &LiberationOfUndermineAddAI::CreateMegaMagnet);
    mgr->register_creature_script(NPC_VOID_TURRET, &LiberationOfUndermineAddAI::CreateVoidTurret);
    mgr->register_creature_script(NPC_VOID_LAUNCHER, &LiberationOfUndermineAddAI::CreateVoidLauncher);

    // The One-Armed Bandit
    mgr->register_creature_script(BOSS_ONE_ARMED_BANDIT, &LiberationOfUndermineBanditAI::Create);
    mgr->register_creature_script(NPC_REEL_ASSISTANT, &LiberationOfUndermineAddAI::CreateReelAssistant);
    mgr->register_creature_script(NPC_REEL_ASSISTANT_ALT, &LiberationOfUndermineAddAI::CreateReelAssistant);
    mgr->register_creature_script(NPC_REEL_ASSISTANT_ALT2, &LiberationOfUndermineAddAI::CreateReelAssistant);

    // Mug'Zee, Heads of Security
    mgr->register_creature_script(BOSS_MUGZEE, &LiberationOfUndermineMugZeeAI::Create);
    mgr->register_creature_script(NPC_MUG, &LiberationOfUndermineCompanionAI::CreateHead);
    mgr->register_creature_script(NPC_ZEE, &LiberationOfUndermineCompanionAI::CreateHead);
    mgr->register_creature_script(NPC_GALLAGIO_GOON, &LiberationOfUndermineAddAI::CreateGoon);
    mgr->register_creature_script(NPC_VOLUNTEER_ROCKETEER, &LiberationOfUndermineAddAI::CreateRocketeer);
    mgr->register_creature_script(NPC_MK_II_ELECTRO_SHOCKER, &LiberationOfUndermineAddAI::CreateElectroShocker);

    // Chrome King Gallywix
    mgr->register_creature_script(BOSS_CHROME_KING_GALLYWIX, &LiberationOfUndermineGallywixAI::Create);
    mgr->register_creature_script(NPC_DARKFUSE_TECHNICIAN, &LiberationOfUndermineAddAI::CreateTechnician);
    mgr->register_creature_script(NPC_SHARPSHOT_SENTRY, &LiberationOfUndermineAddAI::CreateSentry);
    mgr->register_creature_script(NPC_DARKFUSE_WRENCHMONGER, &LiberationOfUndermineAddAI::CreateWrenchmonger);
    mgr->register_creature_script(NPC_GIGA_JUICED_TECHNICIAN, &LiberationOfUndermineAddAI::CreateGigaTechnician);
}
