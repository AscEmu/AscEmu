/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Sanctum_Of_Domination.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <map>
#include <string>
#include <utility>
#include <vector>

enum SanctumOfDominationEvents
{
    // single spell adds
    EVENT_SOD_KIT_FIRST = 1,
    EVENT_SOD_KIT_SECOND,

    // Kel'Thuzad
    EVENT_KT_ICE_SHARD,
    EVENT_KT_SOUL_FRACTURE,
    EVENT_KT_OBLIVIONS_ECHO,
    EVENT_KT_FROST_BLAST,
    EVENT_KT_GLACIAL_WRATH,
    EVENT_KT_DARK_EVOCATION,
    EVENT_KT_HOWLING_BLIZZARD,
    EVENT_KT_CLOSE_PHYLACTERY,
    EVENT_KT_FINAL_MIASMA,
    EVENT_KT_ONSLAUGHT,
    EVENT_KT_FINAL_MARCH,
    EVENT_SPIKE_DECAY,
    EVENT_DEVOTED_HAUNT,

    // The Tarragrue
    EVENT_TARRAGRUE_OVERPOWER,
    EVENT_TARRAGRUE_CHAINS,
    EVENT_TARRAGRUE_SMASH,
    EVENT_TARRAGRUE_GRASP,
    EVENT_TARRAGRUE_HOWL,
    EVENT_TARRAGRUE_TORMENTS,
    EVENT_TARRAGRUE_FURY,
    EVENT_TARRAGRUE_MIST_PULSE,

    // The Nine
    EVENT_KYRA_UNENDING_STRIKE,
    EVENT_KYRA_FORMLESS_MASS,
    EVENT_SIGNE_SOULFUL_BLAST,
    EVENT_SIGNE_SONG,
    EVENT_SKYJA_FRAGMENTS,
    EVENT_SKYJA_PIERCE_SOUL,
    EVENT_SKYJA_WORD_OF_RECALL,
    EVENT_SKYJA_RECALLED_POWER,

    // Painsmith Raznal
    EVENT_RAZNAL_INSTRUMENT,
    EVENT_RAZNAL_SPIKED_BALLS,
    EVENT_RAZNAL_CHAINS,
    EVENT_RAZNAL_FLAMECLASP,
    EVENT_RAZNAL_BLACK_FLAMES,
    EVENT_RAZNAL_FORGE_DONE,

    // Fatescribe Roh-Kalo
    EVENT_ROH_KALO_PROBE,
    EVENT_ROH_KALO_CALL_OF_ETERNITY,
    EVENT_ROH_KALO_INVOKE_DESTINY,
    EVENT_ROH_KALO_CONJUNCTION,
    EVENT_ROH_KALO_TWIST_FATE,
    EVENT_ROH_KALO_EXTEMPORANEOUS,
    EVENT_ROH_KALO_ACCRETION,
    EVENT_ROH_KALO_MONSTROSITY,
    EVENT_ROH_KALO_REALIGN_END,
    EVENT_SHADE_PURSUIT,

    // Remnant of Ner'zhul
    EVENT_NERZHUL_ORBS,
    EVENT_NERZHUL_MALEVOLENCE,
    EVENT_NERZHUL_GRASP,
    EVENT_ORB_TORMENT,
    EVENT_ORB_PROCESSION,

    // The Eye of the Jailer
    EVENT_EYE_PIERCING_LENS,
    EVENT_EYE_DEATHLINK,
    EVENT_EYE_FRACTURE_SOUL,
    EVENT_EYE_LETHARGY,
    EVENT_EYE_GLARE,
    EVENT_EYE_BOLT,
    EVENT_EYE_DEATH_GAZE,
    EVENT_EYE_DESOLATION_BEAM,
    EVENT_EYE_WATCH_DEATHSEEKERS,
    EVENT_EYE_EXTERMINATION,
    EVENT_DEATHSEEKER_LENS,
    EVENT_DEATHSEEKER_CORRUPTION,
    EVENT_DEATHSEEKER_MISERY,
    EVENT_DEATHSEEKER_PROXIMITY,

    // Soulrender Dormazain
    EVENT_DORMAZAIN_TORMENT,
    EVENT_DORMAZAIN_RUINBLADE,
    EVENT_DORMAZAIN_BRAND,
    EVENT_DORMAZAIN_SHACKLES,
    EVENT_DORMAZAIN_HELLSCREAM,
    EVENT_DORMAZAIN_AGONIZERS,
    EVENT_DORMAZAIN_OVERLORD,
    EVENT_AGONIZER_NOVA,
    EVENT_OVERLORD_INFUSE,
    EVENT_OVERLORD_REACH,

    // Sylvanas Windrunner
    EVENT_SYLVANAS_WINDRUNNER,
    EVENT_SYLVANAS_WITHERING_FIRE,
    EVENT_SYLVANAS_DESECRATING_SHOT,
    EVENT_SYLVANAS_SHADOW_DAGGER,
    EVENT_SYLVANAS_DOMINATION_CHAINS,
    EVENT_SYLVANAS_VEIL,
    EVENT_SYLVANAS_WAILING_ARROW,
    EVENT_SYLVANAS_HEARTSEEKER,
    EVENT_SYLVANAS_RIVE,
    EVENT_SYLVANAS_BANSHEE_WAIL,
    EVENT_SYLVANAS_MONUMENT_FALLS,
    EVENT_SYLVANAS_RUIN,
    EVENT_SYLVANAS_HAUNTING_WAVE,
    EVENT_SYLVANAS_FORCES,
    EVENT_SYLVANAS_WATCH_COMMANDERS,
    EVENT_SYLVANAS_BANE_ARROWS,
    EVENT_SYLVANAS_BANSHEES_HEARTSEEKER,
    EVENT_SYLVANAS_BANSHEES_FURY,
    EVENT_SYLVANAS_BANSHEE_SCREAM,
    EVENT_SYLVANAS_RAZE,
    EVENT_SUMMONER_ORBS,
    EVENT_SUMMONER_LETHARGY,
    EVENT_DECREPIT_ORB_REACH,

    // Guardian of the First Ones
    EVENT_GUARDIAN_ENERGY,
    EVENT_GUARDIAN_PURGE,
    EVENT_GUARDIAN_ELIMINATION,
    EVENT_GUARDIAN_OBLITERATE,
    EVENT_GUARDIAN_NEUTRALIZATION,
    EVENT_GUARDIAN_SENTRY,
    EVENT_GUARDIAN_DISINTEGRATION
};

namespace
{
    float const SanctumTwoPi = 6.28318531f;

    // raid berserk timers
    uint32_t const KelThuzadBerserkMs = 600000;
    uint32_t const TarragrueBerserkMs = 420000;
    uint32_t const NineBerserkMs = 540000;
    uint32_t const RaznalBerserkMs = 480000;
    uint32_t const RohKaloBerserkMs = 540000;
    uint32_t const NerzhulBerserkMs = 420000;
    uint32_t const JailersEyeBerserkMs = 480000;
    uint32_t const DormazainBerserkMs = 540000;
    uint32_t const SylvanasBerserkMs = 600000;
    uint32_t const GuardianBerserkMs = 480000;

    // Kel'Thuzad: the phylactery opens twice, the final stand follows the second
    int32_t const KelThuzadPhylacteryMarks[2] = { 66, 33 };
    uint32_t const KelThuzadPhylacteryMs = 45000;
    uint32_t const GlacialSpikeDecayPct = 5;
    float const DevotedReach = 3.0f;

    // The Tarragrue
    int32_t const TarragrueFrenzyPct = 10;
    uint32_t const TarragrueEnergyStepMs = 800;
    uint32_t const TarragrueMistPulses = 4;
    uint32_t const TarragrueMistPulseMs = 5000;

    // The Nine
    int32_t const NineDescentPct = 10;
    uint32_t const KyraEnergyStepMs = 600;
    uint32_t const SigneEnergyStepMs = 660;
    uint32_t const SkyjaEnergyStepMs = 700;

    // Painsmith Raznal
    int32_t const RaznalForgeMarks[2] = { 70, 40 };
    uint32_t const RaznalForgeMs = 30000;
    uint32_t const SpikedBallLifeMs = 12000;
    float const SpikedBallReach = 4.0f;
    float const SpikedBallOvershoot = 25.0f;

    // Fatescribe Roh-Kalo
    int32_t const RohKaloRealignMarks[2] = { 70, 40 };
    uint32_t const RohKaloRealignMs = 35000;
    uint32_t const RohKaloProbeVolley = 3;
    float const ShadeReach = 3.0f;

    // Remnant of Ner'zhul
    int32_t const NerzhulShatterMarks[3] = { 80, 60, 30 };
    uint32_t const NerzhulEnergyStepMs = 300;
    float const SufferingReach = 20.0f;

    // The Eye of the Jailer
    int32_t const JailersEyeRetreatMarks[2] = { 66, 33 };
    float const DeathseekerCloseRange = 8.0f;

    // Soulrender Dormazain
    uint32_t const DormazainEnergyStepMs = 500;
    uint32_t const DormazainTormentEnergy = 10;
    uint32_t const AgonizerEnergyStepMs = 500;
    float const GarroshReach = 8.0f;

    // Sylvanas Windrunner
    uint32_t const SylvanasCycleOfHatred = 1;
    uint32_t const SylvanasMonument = 2;
    uint32_t const SylvanasBansheeQueen = 3;
    uint32_t const SylvanasFreedomOfChoice = 4;
    int32_t const SylvanasMonumentPct = 80;
    uint32_t const SylvanasRiveCount = 5;
    uint32_t const SylvanasMonumentMs = 30000;
    uint32_t const SylvanasLeaveMs = 15000;
    float const DecrepitOrbReach = 3.0f;
    uint32_t const DecrepitOrbLifeMs = 20000;

    // Guardian of the First Ones
    uint32_t const GuardianEnergyMax = 100;
    uint32_t const GuardianLinkGain = 4;
    uint32_t const CoreChargeMax = 100;
    uint32_t const CoreDrainPerLink = 2;
    float const CoreLinkRange = 12.0f;
    float const CoreRingRange = 18.0f;

    uint32_t const NineMembers[3] = { BOSS_NINE_KYRA, BOSS_NINE_SIGNE, BOSS_NINE_SKYJA };

    uint32_t const ValkyrPowers[6] =
    {
        SPELL_VALKYR_AGATHAS_ETERNAL_BLADE,
        SPELL_VALKYR_DASCHLAS_MIGHTY_IMPACT,
        SPELL_VALKYR_ANNHYLDES_BRIGHT_AEGIS,
        SPELL_VALKYR_ARADNES_FALLING_STRIKE,
        SPELL_VALKYR_BRYNJAS_MOURNFUL_DIRGE,
        SPELL_VALKYR_ARTHURAS_CRUSHING_GAZE
    };

    // Raznal's weapon per forge and the armor it breaks
    uint32_t const RaznalInstruments[3] = { SPELL_RAZNAL_CRUCIFORM_AXE, SPELL_RAZNAL_REVERBERATING_HAMMER, SPELL_RAZNAL_DUALBLADE_SCYTHE };
    uint32_t const RaznalArmorBreak[3] = { SPELL_RAZNAL_AXE_BLACKENED_ARMOR, SPELL_RAZNAL_BLACKENED_ARMOR, SPELL_RAZNAL_BLACKENED_ARMOR };
    char const* const RaznalInstrumentNames[3] = { "Cruciform Axe", "Reverberating Hammer", "Dualblade Scythe" };

    uint32_t const TarragrueRemnants[3] = { SPELL_TARRAGRUE_UPPER_REACHES_MIGHT, SPELL_TARRAGRUE_MORTREGARS_ECHOES, SPELL_TARRAGRUE_SOULFORGE_HEAT };

    // adds with one or two timed spells
    enum SanctumKitTarget : uint8_t
    {
        KitOnVictim,
        KitOnSelf,
        KitOnPlayer,
        KitAround
    };

    struct SanctumKitSpell
    {
        uint32_t spellId;
        uint32_t firstMs;
        uint32_t repeatMs;
        SanctumKitTarget target;
    };

    struct SanctumAddKit
    {
        SanctumKitSpell spells[2];
        bool rooted;
    };

    SanctumAddKit const AbominationKit = { { { SPELL_KT_ABOMINATION_DEMOLISH, 6000, 14000, KitOnVictim }, { 0, 0, 0, KitOnVictim } }, false };
    SanctumAddKit const SoulReaverKit = { { { SPELL_KT_REAVER_BANSHEES_CRY, 4000, 12000, KitAround }, { 0, 0, 0, KitOnVictim } }, false };
    SanctumAddKit const SoulShardKit = { { { SPELL_KT_SHARD_PIERCING_WAIL, 2000, 6000, KitAround }, { 0, 0, 0, KitOnVictim } }, true };
    SanctumAddKit const FormlessMassKit = { { { SPELL_FORMLESS_MASS_SIPHON_VITALITY, 3000, 5000, KitOnPlayer }, { 0, 0, 0, KitOnVictim } }, false };
    SanctumAddKit const MonstrosityKit = { { { SPELL_MONSTROSITY_BOON, 1000, 30000, KitOnSelf }, { SPELL_MONSTROSITY_DESPAIR, 6000, 15000, KitOnPlayer } }, false };
    SanctumAddKit const VanguardKit = { { { SPELL_VANGUARD_UNSTOPPABLE_FORCE, 5000, 15000, KitOnVictim }, { SPELL_VANGUARD_ACCURSED_MIGHT, 8000, 25000, KitOnSelf } }, false };
    SanctumAddKit const HopebreakerKit = { { { SPELL_HOPEBREAKER_ENFLAME, 4000, 20000, KitOnSelf }, { SPELL_HOPEBREAKER_DESTABILIZE, 8000, 18000, KitOnPlayer } }, false };
    SanctumAddKit const SouljudgeKit = { { { SPELL_SOULJUDGE_LASHING_STRIKE, 5000, 9000, KitOnVictim }, { SPELL_SOULJUDGE_CRUSHING_DREAD, 10000, 22000, KitOnPlayer } }, false };
    SanctumAddKit const GoliathKit = { { { SPELL_GOLIATH_FURY, 8000, 30000, KitOnSelf }, { 0, 0, 0, KitOnVictim } }, false };
    SanctumAddKit const SentryKit = { { { SPELL_SENTRY_SUPPRESSION_FIELD, 1000, 60000, KitOnSelf }, { SPELL_SENTRY_FIRE, 2000, 4000, KitOnPlayer } }, true };

    // random point on a ring around origin
    LocationVector sanctumRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(SanctumTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // living players in range of a creature
    std::vector<Unit*> sanctumPlayersAround(Creature* pCreature, float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : pCreature->getInRangePlayersSet())
        {
            if (pObject == nullptr)
                continue;

            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer->isAlive() && pCreature->getDistance(pPlayer) <= range)
                players.push_back(pPlayer);
        }

        return players;
    }

    uint32_t sanctumPercentOf(uint32_t value, uint32_t pct)
    {
        return static_cast<uint32_t>(static_cast<uint64_t>(value) * pct / 100);
    }

    // fills a counter by one point per step, true once it is full
    bool sanctumFill(uint32_t& value, uint32_t& clock, uint32_t diff, uint32_t stepMs)
    {
        clock += diff;
        while (clock >= stepMs && value < 100)
        {
            clock -= stepMs;
            ++value;
        }

        if (value >= 100)
            clock = 0;

        return value >= 100;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class SanctumOfDominationInstanceScript : public InstanceScript
{
public:
    explicit SanctumOfDominationInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(SanctumOfDominationEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new SanctumOfDominationInstanceScript(pMapMgr); }

    // a fallen val'kyr must not end The Nine while a sister still stands
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isEncounterDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_NINE_KYRA:
            case BOSS_NINE_SIGNE:
            case BOSS_NINE_SKYJA:
            case NPC_DORMAZAIN_SHACKLED_GARROSH:
                mTrackedGuids[pCreature->getEntry()] = pCreature->GetNewGUID();
                break;
            case NPC_SYLVANAS_THRONE:
                mTrackedGuids[pCreature->getEntry()] = pCreature->GetNewGUID();
                mThroneHidden = false;
                applyThroneState(pCreature);
                break;
            default:
                break;
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (id == DATA_SYLVANAS)
        {
            if (Creature* pThrone = getTracked(NPC_SYLVANAS_THRONE))
                applyThroneState(pThrone);
            return;
        }

        if (id != DATA_NINE || state != EncounterStates::Failed)
            return;

        // a wiped Nine fights again in full
        for (Creature* pMember : getNine())
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    Creature* getTracked(uint32_t entry)
    {
        auto const itr = mTrackedGuids.find(entry);
        return itr != mTrackedGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    std::vector<Creature*> getNine()
    {
        std::vector<Creature*> nine;
        for (uint32_t entry : NineMembers)
        {
            if (Creature* pMember = getTracked(entry))
                nine.push_back(pMember);
        }

        return nine;
    }

    // single boss encounters are down with their boss
    bool isEncounterDown(uint32_t dataId, Creature* pExcept)
    {
        if (dataId != DATA_NINE)
            return true;

        for (Creature* pMember : getNine())
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    bool isEncounterFighting(uint32_t dataId, Creature* pExcept)
    {
        if (dataId != DATA_NINE)
            return false;

        for (Creature* pMember : getNine())
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat())
                return true;
        }

        return false;
    }

    // pulling one val'kyr pulls her sisters
    void rallyNine(Creature* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getNine())
        {
            if (pMember == pPuller || !pMember->isAlive() || pMember->isInCombat() || pMember->GetScript() == nullptr)
                continue;

            pMember->GetScript()->setZoneWideCombat();
        }
        mRallying = false;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        Creature* pTarget = getTracked(entry);
        if (pTarget != nullptr && pTarget->isAlive() && pTarget->GetScript() != nullptr)
            pTarget->GetScript()->DoAction(action);
    }

private:
    // the throne teleporter is gone while Sylvanas fights
    void applyThroneState(Creature* pThrone)
    {
        bool const hide = getBossState(DATA_SYLVANAS) == EncounterStates::InProgress;
        if (hide == mThroneHidden)
            return;

        mThroneHidden = hide;
        pThrone->setVisible(!hide);
    }

    std::map<uint32_t, WoWGuid> mTrackedGuids;
    bool mRallying = false;
    bool mThroneHidden = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, berserk, untouchable stages and target helpers

class SanctumOfDominationCombatAI : public CreatureAIScript
{
public:
    explicit SanctumOfDominationCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mStage = 1;
        mBerserkLeftMs = 0;
        mUntouchable = false;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mStage = 0;
        mBerserkLeftMs = 0;
        mUntouchable = false;
        onDisengage();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mUntouchable)
        {
            *damage = 0;
            return;
        }

        onDamage(*damage);
    }

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
    virtual void onDamage(uint32_t& /*damage*/) {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    SanctumOfDominationInstanceScript* getSanctum()
    {
        if (getCreature()->GetMapId() != MAP_SANCTUM_OF_DOMINATION)
            return nullptr;

        return static_cast<SanctumOfDominationInstanceScript*>(getInstanceScript());
    }

    Unit* getVictim() { return getCreature()->getAIInterface()->getCurrentTarget(); }

    Unit* findUnit(WoWGuid const& guid)
    {
        if (guid.isEmpty() || getCreature()->getWorldMap() == nullptr)
            return nullptr;

        return getCreature()->getWorldMap()->getUnit(guid);
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
        return sanctumRingPoint(getCreature()->GetPosition(), minRange, maxRange);
    }

    std::vector<Creature*> livingOfEntry(uint32_t entry)
    {
        std::vector<Creature*> living;
        if (InstanceScript* pInstance = getInstanceScript())
        {
            for (Creature* pCreature : pInstance->getCreatureSetForEntry(entry))
            {
                if (pCreature != nullptr && pCreature->isAlive())
                    living.push_back(pCreature);
            }
        }

        return living;
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void armBerserk(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

    // a new stage drops every pending event
    void enterStage(uint32_t stage)
    {
        mStage = stage;
        scriptEvents.resetEvents();
    }

    // out of reach: takes no damage and holds its ground
    void setUntouchable(bool untouchable)
    {
        mUntouchable = untouchable;
        setRooted(untouchable);
        _setMeleeDisabled(untouchable);
    }

    bool isUntouchable() const { return mUntouchable; }

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
        castSpellOnSelf(SPELL_SOD_BERSERK, true);
    }

    uint32_t mBerserkLeftMs = 0;
    bool mUntouchable = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class SanctumOfDominationBossAI : public SanctumOfDominationCombatAI
{
public:
    SanctumOfDominationBossAI(Creature* pCreature, uint32_t dataId) : SanctumOfDominationCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (mDataId == DATA_NINE)
        {
            if (SanctumOfDominationInstanceScript* pSanctum = getSanctum())
                pSanctum->rallyNine(getCreature());
        }

        SanctumOfDominationCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last sister has left the fight
                SanctumOfDominationInstanceScript* pSanctum = getSanctum();
                if (pSanctum == nullptr || !pSanctum->isEncounterFighting(mDataId, getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        SanctumOfDominationCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            SanctumOfDominationInstanceScript* pSanctum = getSanctum();
            if (pSanctum == nullptr || pSanctum->isEncounterDown(mDataId, getCreature()))
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
// Adds with one or two timed spells

class SanctumOfDominationKitAI : public SanctumOfDominationCombatAI
{
public:
    static CreatureAIScript* CreateAbomination(Creature* c) { return new SanctumOfDominationKitAI(c, AbominationKit); }
    static CreatureAIScript* CreateSoulReaver(Creature* c) { return new SanctumOfDominationKitAI(c, SoulReaverKit); }
    static CreatureAIScript* CreateSoulShard(Creature* c) { return new SanctumOfDominationKitAI(c, SoulShardKit); }
    static CreatureAIScript* CreateFormlessMass(Creature* c) { return new SanctumOfDominationKitAI(c, FormlessMassKit); }
    static CreatureAIScript* CreateMonstrosity(Creature* c) { return new SanctumOfDominationKitAI(c, MonstrosityKit); }
    static CreatureAIScript* CreateVanguard(Creature* c) { return new SanctumOfDominationKitAI(c, VanguardKit); }
    static CreatureAIScript* CreateHopebreaker(Creature* c) { return new SanctumOfDominationKitAI(c, HopebreakerKit); }
    static CreatureAIScript* CreateSouljudge(Creature* c) { return new SanctumOfDominationKitAI(c, SouljudgeKit); }
    static CreatureAIScript* CreateGoliath(Creature* c) { return new SanctumOfDominationKitAI(c, GoliathKit); }
    static CreatureAIScript* CreateSentry(Creature* c) { return new SanctumOfDominationKitAI(c, SentryKit); }

    SanctumOfDominationKitAI(Creature* pCreature, SanctumAddKit const& kit) : SanctumOfDominationCombatAI(pCreature), mKit(kit) {}

protected:
    void onEngage() override
    {
        if (mKit.rooted)
        {
            setRooted(true);
            _setMeleeDisabled(true);
        }

        for (uint32_t i = 0; i < 2; ++i)
        {
            if (mKit.spells[i].spellId != 0)
                scriptEvents.addEvent(EVENT_SOD_KIT_FIRST + i, mKit.spells[i].firstMs);
        }
    }

    void onDisengage() override
    {
        if (!mKit.rooted)
            return;

        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SOD_KIT_FIRST && eventId != EVENT_SOD_KIT_SECOND)
            return;

        SanctumKitSpell const& entry = mKit.spells[eventId - EVENT_SOD_KIT_FIRST];
        switch (entry.target)
        {
            case KitOnVictim:
                castSpellOnVictim(entry.spellId);
                break;
            case KitOnSelf:
                castSpellOnSelf(entry.spellId);
                break;
            case KitOnPlayer:
                castAtPlayer(entry.spellId);
                break;
            default:
                castSpellAOE(entry.spellId);
                break;
        }

        scriptEvents.addEvent(eventId, entry.repeatMs);
    }

private:
    SanctumAddKit const& mKit;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kel'Thuzad

class SanctumOfDominationKelThuzadAI : public SanctumOfDominationBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationKelThuzadAI(c); }
    explicit SanctumOfDominationKelThuzadAI(Creature* pCreature) : SanctumOfDominationBossAI(pCreature, DATA_KEL_THUZAD) {}

protected:
    void onEngage() override
    {
        mPhylacteryOpenings = 0;
        armBerserk(KelThuzadBerserkMs);
        startChainsAndIce();
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage != 1 || mPhylacteryOpenings >= 2 || _getHealthPercent() > KelThuzadPhylacteryMarks[mPhylacteryOpenings])
            return;

        openPhylactery();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KT_ICE_SHARD:
                castSpellOnVictim(SPELL_KT_ICE_SHARD);
                scriptEvents.addEvent(EVENT_KT_ICE_SHARD, 12000);
                break;
            case EVENT_KT_SOUL_FRACTURE:
                fractureSoul();
                scriptEvents.addEvent(EVENT_KT_SOUL_FRACTURE, 30000);
                break;
            case EVENT_KT_OBLIVIONS_ECHO:
                castAtPlayer(SPELL_KT_OBLIVIONS_ECHO, true);
                raiseDevoted(2);
                scriptEvents.addEvent(EVENT_KT_OBLIVIONS_ECHO, 40000);
                break;
            case EVENT_KT_FROST_BLAST:
                frostBlast();
                scriptEvents.addEvent(EVENT_KT_FROST_BLAST, mStage == 3 ? 25000 : 35000);
                break;
            case EVENT_KT_GLACIAL_WRATH:
                warn("Kel'Thuzad calls down his Glacial Wrath!");
                for (Unit* pTarget : pickPlayers(2, true))
                    callAdd(NPC_KT_GLACIAL_SPIKE, sanctumRingPoint(pTarget->GetPosition(), 2.0f, 4.0f));
                castSpellAOE(SPELL_KT_GLACIAL_WRATH);
                scriptEvents.addEvent(EVENT_KT_GLACIAL_WRATH, 45000);
                break;
            case EVENT_KT_DARK_EVOCATION:
                castSpellAOE(SPELL_KT_DARK_EVOCATION);
                scriptEvents.addEvent(EVENT_KT_DARK_EVOCATION, 50000);
                break;
            case EVENT_KT_HOWLING_BLIZZARD:
                warn("Kel'Thuzad unleashes a Howling Blizzard!");
                castSpellAOE(SPELL_KT_HOWLING_BLIZZARD);
                scriptEvents.addEvent(EVENT_KT_HOWLING_BLIZZARD, 60000);
                break;
            case EVENT_KT_CLOSE_PHYLACTERY:
                closePhylactery();
                break;
            case EVENT_KT_FINAL_MIASMA:
                castSpellAOE(SPELL_KT_FINAL_MIASMA);
                scriptEvents.addEvent(EVENT_KT_FINAL_MIASMA, 25000);
                break;
            case EVENT_KT_ONSLAUGHT:
                warn("Kel'Thuzad calls an Onslaught of the Damned!");
                callAdd(NPC_KT_UNSTOPPABLE_ABOMINATION, aroundSelf(10.0f, 18.0f));
                callAdd(NPC_KT_SOUL_REAVER, aroundSelf(10.0f, 18.0f));
                raiseDevoted(2);
                castSpellAOE(SPELL_KT_ONSLAUGHT_OF_THE_DAMNED);
                scriptEvents.addEvent(EVENT_KT_ONSLAUGHT, 60000);
                break;
            case EVENT_KT_FINAL_MARCH:
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_KT_SOUL_REAVER, aroundSelf(10.0f, 18.0f));
                castSpellAOE(SPELL_KT_FINAL_MARCH);
                scriptEvents.addEvent(EVENT_KT_FINAL_MARCH, 45000);
                break;
            default:
                break;
        }
    }

private:
    void startChainsAndIce()
    {
        enterStage(1);
        scriptEvents.addEvent(EVENT_KT_ICE_SHARD, 6000);
        scriptEvents.addEvent(EVENT_KT_SOUL_FRACTURE, 10000);
        scriptEvents.addEvent(EVENT_KT_OBLIVIONS_ECHO, 16000);
        scriptEvents.addEvent(EVENT_KT_FROST_BLAST, 22000);
        scriptEvents.addEvent(EVENT_KT_GLACIAL_WRATH, 30000);
        scriptEvents.addEvent(EVENT_KT_DARK_EVOCATION, 38000);
        scriptEvents.addEvent(EVENT_KT_HOWLING_BLIZZARD, 50000);
    }

    // the phylactery restores him while his minions march
    void openPhylactery()
    {
        ++mPhylacteryOpenings;
        enterStage(2);
        setUntouchable(true);

        warn("Kel'Thuzad's phylactery opens! His minions march forth!");
        castSpellOnSelf(SPELL_KT_UNDYING, true);
        for (uint8_t i = 0; i < 2; ++i)
        {
            callAdd(NPC_KT_UNSTOPPABLE_ABOMINATION, aroundSelf(10.0f, 18.0f));
            callAdd(NPC_KT_SOUL_REAVER, aroundSelf(10.0f, 18.0f));
        }
        castSpellAOE(SPELL_KT_MARCH_OF_THE_FORSAKEN, true);

        scriptEvents.addEvent(EVENT_KT_CLOSE_PHYLACTERY, KelThuzadPhylacteryMs);
    }

    void closePhylactery()
    {
        setUntouchable(false);

        // minions still standing are empowered
        for (uint32_t entry : { NPC_KT_UNSTOPPABLE_ABOMINATION, NPC_KT_SOUL_REAVER })
        {
            for (Creature* pMinion : livingOfEntry(entry))
                castSpell(pMinion, SPELL_KT_NECROTIC_EMPOWERMENT, true);
        }

        castSpellOnSelf(SPELL_KT_NECROTIC_SURGE, true);

        if (mPhylacteryOpenings < 2)
        {
            warn("Kel'Thuzad returns with renewed power!");
            startChainsAndIce();
            return;
        }

        startFinalStand();
    }

    void startFinalStand()
    {
        enterStage(3);
        warn("The phylactery shatters! Kel'Thuzad makes his final stand!");
        castSpellOnSelf(SPELL_KT_DEEP_FREEZE, true);

        scriptEvents.addEvent(EVENT_KT_ICE_SHARD, 6000);
        scriptEvents.addEvent(EVENT_KT_FROST_BLAST, 10000);
        scriptEvents.addEvent(EVENT_KT_FINAL_MIASMA, 15000);
        scriptEvents.addEvent(EVENT_KT_ONSLAUGHT, 20000);
        scriptEvents.addEvent(EVENT_KT_FINAL_MARCH, 45000);
    }

    // the tank's soul splinters into a wailing shard
    void fractureSoul()
    {
        Unit* pTank = getVictim();
        if (pTank == nullptr)
            return;

        callAdd(NPC_KT_SOUL_SHARD, sanctumRingPoint(pTank->GetPosition(), 2.0f, 4.0f));
        castSpell(pTank, SPELL_KT_SOUL_FRACTURE);
    }

    void frostBlast()
    {
        Unit* pTarget = pickPlayer();
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, SPELL_KT_FROZEN_BINDS, true);
        castSpell(pTarget, SPELL_KT_FROST_BLAST);
    }

    void raiseDevoted(uint8_t count)
    {
        for (uint8_t i = 0; i < count; ++i)
            callAdd(NPC_KT_FROSTBOUND_DEVOTED, aroundSelf(12.0f, 20.0f));
    }

    uint32_t mPhylacteryOpenings = 0;
};

// melts away by itself and erupts when it breaks
class SanctumOfDominationGlacialSpikeAI : public SanctumOfDominationCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationGlacialSpikeAI(c); }
    explicit SanctumOfDominationGlacialSpikeAI(Creature* pCreature) : SanctumOfDominationCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        mErupted = false;
        setRooted(true);
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_SPIKE_DECAY, 2000);
    }

    void onDamage(uint32_t& damage) override
    {
        uint32_t const health = getCreature()->getHealth();
        if (damage < health)
            return;

        damage = health > 1 ? health - 1 : 0;
        erupt();
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SPIKE_DECAY || mErupted)
            return;

        Creature* pSelf = getCreature();
        uint32_t const loss = sanctumPercentOf(pSelf->getMaxHealth(), GlacialSpikeDecayPct);
        if (pSelf->getHealth() <= loss + 1)
        {
            erupt();
            return;
        }

        pSelf->setHealth(pSelf->getHealth() - loss);
        scriptEvents.addEvent(EVENT_SPIKE_DECAY, 2000);
    }

private:
    void erupt()
    {
        if (mErupted)
            return;

        mErupted = true;
        castSpellAOE(SPELL_KT_SPIKE_FROZEN_DESTRUCTION, true);
        despawn(1000, 0);
    }

    bool mErupted = false;
};

// walks at a random player and bursts on contact
class SanctumOfDominationFrostboundDevotedAI : public SanctumOfDominationCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationFrostboundDevotedAI(c); }
    explicit SanctumOfDominationFrostboundDevotedAI(Creature* pCreature) : SanctumOfDominationCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        mBurst = false;
        _setMeleeDisabled(true);
        huntNewPrey();
        scriptEvents.addEvent(EVENT_DEVOTED_HAUNT, 1000);
    }

    void onDisengage() override
    {
        _setMeleeDisabled(false);
        getCreature()->getThreatManager().clearFixate();
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_DEVOTED_HAUNT || mBurst)
            return;

        Unit* pPrey = findUnit(mPreyGuid);
        if (pPrey == nullptr || !pPrey->isAlive())
        {
            huntNewPrey();
        }
        else if (getCreature()->getDistance(pPrey) <= DevotedReach)
        {
            mBurst = true;
            castSpell(pPrey, SPELL_KT_DEVOTED_RELENTLESS_HAUNT, true);
            despawn(500, 0);
            return;
        }

        scriptEvents.addEvent(EVENT_DEVOTED_HAUNT, 1000);
    }

private:
    void huntNewPrey()
    {
        Unit* pPrey = pickPlayer();
        if (pPrey == nullptr)
            return;

        mPreyGuid = pPrey->GetNewGUID();
        getCreature()->getThreatManager().fixateTarget(pPrey);
    }

    WoWGuid mPreyGuid;
    bool mBurst = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Tarragrue

class SanctumOfDominationTarragrueAI : public SanctumOfDominationBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationTarragrueAI(c); }
    explicit SanctumOfDominationTarragrueAI(Creature* pCreature) : SanctumOfDominationBossAI(pCreature, DATA_TARRAGRUE) {}

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        mMistPulsesLeft = 0;
        mFrenzied = false;
        armBerserk(TarragrueBerserkMs);

        scriptEvents.addEvent(EVENT_TARRAGRUE_OVERPOWER, 8000);
        scriptEvents.addEvent(EVENT_TARRAGRUE_CHAINS, 12000);
        scriptEvents.addEvent(EVENT_TARRAGRUE_GRASP, 18000);
        scriptEvents.addEvent(EVENT_TARRAGRUE_HOWL, 24000);
        scriptEvents.addEvent(EVENT_TARRAGRUE_TORMENTS, 35000);
        scriptEvents.addEvent(EVENT_TARRAGRUE_FURY, 40000);
    }

    void onDisengage() override { setUntouchable(false); }

    void onCombatTick(uint32_t diff) override
    {
        // the Jailer's last push at low health
        if (!mFrenzied && _getHealthPercent() <= TarragrueFrenzyPct)
        {
            mFrenzied = true;
            warn("The Jailer turns his gaze upon The Tarragrue, sending it into a wild frenzy!");
            castSpellOnSelf(SPELL_TARRAGRUE_JAILERS_GAZE, true);
        }

        if (mMistPulsesLeft > 0 || !sanctumFill(mEnergy, mEnergyClock, diff, TarragrueEnergyStepMs) || _isCasting())
            return;

        dissolveIntoMist();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TARRAGRUE_OVERPOWER:
                if (Unit* pTank = getVictim())
                {
                    castSpell(pTank, SPELL_TARRAGRUE_CRUSHED_ARMOR, true);
                    castSpell(pTank, SPELL_TARRAGRUE_OVERPOWER);
                }
                scriptEvents.addEvent(EVENT_TARRAGRUE_OVERPOWER, 22000);
                break;
            case EVENT_TARRAGRUE_CHAINS:
                if (Unit* pTarget = castAtPlayer(SPELL_TARRAGRUE_CHAINS_OF_ETERNITY, true))
                {
                    mChainedGuid = pTarget->GetNewGUID();
                    scriptEvents.addEvent(EVENT_TARRAGRUE_SMASH, 5000);
                }
                scriptEvents.addEvent(EVENT_TARRAGRUE_CHAINS, 30000);
                break;
            case EVENT_TARRAGRUE_SMASH:
                // the chained player is crushed
                if (Unit* pChained = findUnit(mChainedGuid))
                {
                    if (pChained->isAlive())
                        castSpell(pChained, SPELL_TARRAGRUE_ANNIHILATING_SMASH);
                }
                mChainedGuid = WoWGuid();
                break;
            case EVENT_TARRAGRUE_GRASP:
                castAtPlayers(SPELL_TARRAGRUE_GRASP_OF_DEATH, isHeroic() ? 3 : 2, true);
                scriptEvents.addEvent(EVENT_TARRAGRUE_GRASP, 30000);
                break;
            case EVENT_TARRAGRUE_HOWL:
                castSpellAOE(SPELL_TARRAGRUE_PREDATORS_HOWL);
                scriptEvents.addEvent(EVENT_TARRAGRUE_HOWL, 28000);
                break;
            case EVENT_TARRAGRUE_TORMENTS:
                warn("The Jailer calls forth a Remnant of Forgotten Torments!");
                castSpellAOE(SPELL_TARRAGRUE_FORGOTTEN_TORMENTS, true);
                castAtPlayers(TarragrueRemnants[Util::getRandomUInt(2)], 3);
                scriptEvents.addEvent(EVENT_TARRAGRUE_TORMENTS, 45000);
                break;
            case EVENT_TARRAGRUE_FURY:
                castSpellOnSelf(SPELL_TARRAGRUE_FURY_OF_THE_AGES);
                scriptEvents.addEvent(EVENT_TARRAGRUE_FURY, 35000);
                break;
            case EVENT_TARRAGRUE_MIST_PULSE:
                castSpellAOE(SPELL_TARRAGRUE_HUNGERING_MIST, true);
                if (mMistPulsesLeft > 0 && --mMistPulsesLeft > 0)
                {
                    scriptEvents.addEvent(EVENT_TARRAGRUE_MIST_PULSE, TarragrueMistPulseMs);
                    break;
                }
                setUntouchable(false);
                break;
            default:
                break;
        }
    }

private:
    // at full energy it stalks from within the mist
    void dissolveIntoMist()
    {
        mEnergy = 0;
        mMistPulsesLeft = TarragrueMistPulses;
        setUntouchable(true);
        warn("The Tarragrue dissolves into the Hungering Mist!");

        scriptEvents.delayAllEvents(TarragrueMistPulses * TarragrueMistPulseMs);
        scriptEvents.addEvent(EVENT_TARRAGRUE_MIST_PULSE, 2000);
    }

    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    uint32_t mMistPulsesLeft = 0;
    bool mFrenzied = false;
    WoWGuid mChainedGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Nine

// Kyra and Signe hold the ground, Skyja lands once one of them is close to falling
class SanctumOfDominationNineGroundAI : public SanctumOfDominationBossAI
{
public:
    explicit SanctumOfDominationNineGroundAI(Creature* pCreature) : SanctumOfDominationBossAI(pCreature, DATA_NINE) {}

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        mCalledSkyja = false;
        armBerserk(NineBerserkMs);
        onGroundEngage();
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mCalledSkyja && _getHealthPercent() <= NineDescentPct)
            callSkyja();

        if (!sanctumFill(mEnergy, mEnergyClock, diff, energyStepMs()) || _isCasting())
            return;

        mEnergy = 0;
        onFullEnergy();
    }

    void onDefeat() override { callSkyja(); }

    virtual void onGroundEngage() {}
    virtual uint32_t energyStepMs() const = 0;
    virtual void onFullEnergy() = 0;

private:
    void callSkyja()
    {
        mCalledSkyja = true;
        if (SanctumOfDominationInstanceScript* pSanctum = getSanctum())
            pSanctum->sendAction(BOSS_NINE_SKYJA, ACTION_SKYJA_DESCEND);
    }

    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    bool mCalledSkyja = false;
};

class SanctumOfDominationKyraAI : public SanctumOfDominationNineGroundAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationKyraAI(c); }
    explicit SanctumOfDominationKyraAI(Creature* pCreature) : SanctumOfDominationNineGroundAI(pCreature) {}

protected:
    void onGroundEngage() override
    {
        scriptEvents.addEvent(EVENT_KYRA_UNENDING_STRIKE, 6000);
        scriptEvents.addEvent(EVENT_KYRA_FORMLESS_MASS, 20000);
    }

    uint32_t energyStepMs() const override { return KyraEnergyStepMs; }

    void onFullEnergy() override
    {
        warn("Kyra unfurls her Wings of Rage!");
        castSpellAOE(SPELL_KYRA_WINGS_OF_RAGE);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KYRA_UNENDING_STRIKE:
                castSpellOnVictim(SPELL_KYRA_UNENDING_STRIKE);
                scriptEvents.addEvent(EVENT_KYRA_UNENDING_STRIKE, 10000);
                break;
            case EVENT_KYRA_FORMLESS_MASS:
                callAdd(NPC_NINE_FORMLESS_MASS, aroundSelf(5.0f, 10.0f));
                scriptEvents.addEvent(EVENT_KYRA_FORMLESS_MASS, 45000);
                break;
            default:
                break;
        }
    }
};

class SanctumOfDominationSigneAI : public SanctumOfDominationNineGroundAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationSigneAI(c); }
    explicit SanctumOfDominationSigneAI(Creature* pCreature) : SanctumOfDominationNineGroundAI(pCreature) {}

protected:
    void onGroundEngage() override
    {
        scriptEvents.addEvent(EVENT_SIGNE_SOULFUL_BLAST, 4000);
        scriptEvents.addEvent(EVENT_SIGNE_SONG, 15000);
    }

    uint32_t energyStepMs() const override { return SigneEnergyStepMs; }

    void onFullEnergy() override
    {
        warn("Signe sings a Reverberating Refrain!");
        castSpellAOE(SPELL_SIGNE_REVERBERATING_REFRAIN);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SIGNE_SOULFUL_BLAST:
                castAtPlayer(SPELL_SIGNE_SOULFUL_BLAST);
                scriptEvents.addEvent(EVENT_SIGNE_SOULFUL_BLAST, 8000);
                break;
            case EVENT_SIGNE_SONG:
                castAtPlayer(SPELL_SIGNE_SONG_OF_DISSOLUTION);
                scriptEvents.addEvent(EVENT_SIGNE_SONG, 22000);
                break;
            default:
                break;
        }
    }
};

// commands the fallen val'kyr from above, then lands
class SanctumOfDominationSkyjaAI : public SanctumOfDominationBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationSkyjaAI(c); }
    explicit SanctumOfDominationSkyjaAI(Creature* pCreature) : SanctumOfDominationBossAI(pCreature, DATA_NINE) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_SKYJA_DESCEND && _isInCombat() && !mLanded)
            land();
    }

protected:
    void onEngage() override
    {
        mLanded = false;
        mEnergy = 0;
        mEnergyClock = 0;
        mRecallWings = true;
        armBerserk(NineBerserkMs);

        // out of reach in the skies
        setUntouchable(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        scriptEvents.addEvent(EVENT_SKYJA_FRAGMENTS, 25000);
    }

    void onDisengage() override
    {
        setUntouchable(false);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void onCombatTick(uint32_t diff) override
    {
        // nobody left on the ground to hold the line
        if (!mLanded && !sistersStanding())
            land();

        if (!sanctumFill(mEnergy, mEnergyClock, diff, SkyjaEnergyStepMs) || _isCasting())
            return;

        mEnergy = 0;
        callTheValkyr();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SKYJA_FRAGMENTS:
                castSpellAOE(mLanded ? SPELL_SKYJA_FRAGMENTS_OF_DESTINY : SPELL_SKYJA_FRAGMENTS_FROM_ABOVE, true);
                castAtPlayers(SPELL_SKYJA_SHARD_OF_DESTINY, 3);
                scriptEvents.addEvent(EVENT_SKYJA_FRAGMENTS, mLanded ? 35000 : 40000);
                break;
            case EVENT_SKYJA_PIERCE_SOUL:
                castSpellOnVictim(SPELL_SKYJA_PIERCE_SOUL);
                scriptEvents.addEvent(EVENT_SKYJA_PIERCE_SOUL, 10000);
                break;
            case EVENT_SKYJA_WORD_OF_RECALL:
                warn("Skyja speaks a Word of Recall!");
                castSpellAOE(SPELL_SKYJA_WORD_OF_RECALL);
                scriptEvents.addEvent(EVENT_SKYJA_RECALLED_POWER, 3000);
                scriptEvents.addEvent(EVENT_SKYJA_WORD_OF_RECALL, 60000);
                break;
            case EVENT_SKYJA_RECALLED_POWER:
                // her sisters' powers answer in turn
                castSpellAOE(mRecallWings ? SPELL_SKYJA_RECALLED_WINGS : SPELL_SKYJA_RECALLED_REFRAIN, true);
                mRecallWings = !mRecallWings;
                break;
            default:
                break;
        }
    }

private:
    void land()
    {
        mLanded = true;
        enterStage(2);
        setUntouchable(false);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        warn("Skyja descends to take matters into her own hands!");

        scriptEvents.addEvent(EVENT_SKYJA_PIERCE_SOUL, 8000);
        scriptEvents.addEvent(EVENT_SKYJA_FRAGMENTS, 15000);
        scriptEvents.addEvent(EVENT_SKYJA_WORD_OF_RECALL, 30000);
    }

    bool sistersStanding()
    {
        SanctumOfDominationInstanceScript* pSanctum = getSanctum();
        if (pSanctum == nullptr)
            return false;

        for (uint32_t entry : { BOSS_NINE_KYRA, BOSS_NINE_SIGNE })
        {
            Creature* pSister = pSanctum->getTracked(entry);
            if (pSister != nullptr && pSister->isAlive())
                return true;
        }

        return false;
    }

    // two of the fallen val'kyr lend their power
    void callTheValkyr()
    {
        warn("Skyja sounds the Call of the Val'kyr!");
        castSpellAOE(SPELL_SKYJA_CALL_OF_THE_VALKYR, true);

        uint32_t const first = Util::getRandomUInt(5);
        uint32_t const second = (first + 1 + Util::getRandomUInt(4)) % 6;
        for (uint32_t power : { ValkyrPowers[first], ValkyrPowers[second] })
        {
            if (power == SPELL_VALKYR_ANNHYLDES_BRIGHT_AEGIS)
                castSpellOnSelf(power, true);
            else if (power == SPELL_VALKYR_BRYNJAS_MOURNFUL_DIRGE)
                castAtPlayers(power, 2);
            else
                castAtPlayer(power, false, true);
        }
    }

    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    bool mLanded = false;
    bool mRecallWings = true;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Painsmith Raznal

class SanctumOfDominationRaznalAI : public SanctumOfDominationBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationRaznalAI(c); }
    explicit SanctumOfDominationRaznalAI(Creature* pCreature) : SanctumOfDominationBossAI(pCreature, DATA_RAZNAL) {}

protected:
    void onEngage() override
    {
        mForges = 0;
        armBerserk(RaznalBerserkMs);
        startChamber();
    }

    void onDisengage() override { setUntouchable(false); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage != 1 || mForges >= 2 || _getHealthPercent() > RaznalForgeMarks[mForges])
            return;

        startForge();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RAZNAL_INSTRUMENT:
                if (Unit* pTank = getVictim())
                {
                    castSpell(pTank, RaznalArmorBreak[mForges], true);
                    castSpell(pTank, RaznalInstruments[mForges]);
                }
                scriptEvents.addEvent(EVENT_RAZNAL_INSTRUMENT, 20000);
                break;
            case EVENT_RAZNAL_SPIKED_BALLS:
                rollSpikedBalls(isHeroic() ? 4 : 3);
                scriptEvents.addEvent(EVENT_RAZNAL_SPIKED_BALLS, mStage == 2 ? 15000 : 45000);
                break;
            case EVENT_RAZNAL_CHAINS:
                castAtPlayers(SPELL_RAZNAL_SHADOWSTEEL_CHAINS, 2);
                scriptEvents.addEvent(EVENT_RAZNAL_CHAINS, 30000);
                break;
            case EVENT_RAZNAL_FLAMECLASP:
                castAtPlayer(SPELL_RAZNAL_FLAMECLASP_TRAP, true, true);
                scriptEvents.addEvent(EVENT_RAZNAL_FLAMECLASP, 32000);
                break;
            case EVENT_RAZNAL_BLACK_FLAMES:
                castAtPlayer(SPELL_RAZNAL_BLACK_FLAMES);
                scriptEvents.addEvent(EVENT_RAZNAL_BLACK_FLAMES, 6000);
                break;
            case EVENT_RAZNAL_FORGE_DONE:
                setUntouchable(false);
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, std::string("Painsmith Raznal returns wielding the ") + RaznalInstrumentNames[mForges] + "!");
                startChamber();
                break;
            default:
                break;
        }
    }

private:
    void startChamber()
    {
        enterStage(1);
        scriptEvents.addEvent(EVENT_RAZNAL_INSTRUMENT, 10000);
        scriptEvents.addEvent(EVENT_RAZNAL_CHAINS, 18000);
        scriptEvents.addEvent(EVENT_RAZNAL_FLAMECLASP, 25000);
        scriptEvents.addEvent(EVENT_RAZNAL_SPIKED_BALLS, 30000);
    }

    // a new Instrument of Pain is forged on the anvil
    void startForge()
    {
        ++mForges;
        enterStage(2);
        setUntouchable(true);
        warn("Painsmith Raznal leaps to his anvil to forge a new Instrument of Pain!");
        castSpellAOE(SPELL_RAZNAL_SHADOWSTEEL_EMBERS, true);

        scriptEvents.addEvent(EVENT_RAZNAL_BLACK_FLAMES, 3000);
        scriptEvents.addEvent(EVENT_RAZNAL_SPIKED_BALLS, 8000);
        scriptEvents.addEvent(EVENT_RAZNAL_FORGE_DONE, RaznalForgeMs);
    }

    void rollSpikedBalls(uint32_t count)
    {
        warn("Spiked Balls roll across the chamber!");
        castSpellOnSelf(SPELL_RAZNAL_SPIKED_BALLS, true);
        for (uint32_t i = 0; i < count; ++i)
            summonCreature(NPC_RAZNAL_SPIKED_BALL, aroundSelf(20.0f, 30.0f), TIMED_DESPAWN, SpikedBallLifeMs);
    }

    uint32_t mForges = 0;
};

// rolls through a random player and on across the chamber
class SanctumOfDominationSpikedBallAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationSpikedBallAI(c); }
    explicit SanctumOfDominationSpikedBallAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        if (!mRolling)
        {
            mRolling = true;
            roll();
        }

        mCrushClock += static_cast<uint32_t>(time_passed);
        if (mCrushClock < 1000)
            return;

        mCrushClock = 0;
        for (Unit* pPlayer : sanctumPlayersAround(getCreature(), SpikedBallReach))
            castSpell(pPlayer, SPELL_RAZNAL_SPIKED, true);
    }

private:
    void roll()
    {
        std::vector<Unit*> const players = sanctumPlayersAround(getCreature(), 80.0f);
        if (players.empty())
            return;

        Unit* pTarget = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
        LocationVector const from = getCreature()->GetPosition();
        LocationVector const through = pTarget->GetPosition();

        float const dx = through.x - from.x;
        float const dy = through.y - from.y;
        float const length = std::sqrt(dx * dx + dy * dy);
        if (length < 0.1f)
            return;

        float const scale = (length + SpikedBallOvershoot) / length;
        movePoint(1, from.x + dx * scale, from.y + dy * scale, through.z, false);
    }

    bool mRolling = false;
    uint32_t mCrushClock = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Fatescribe Roh-Kalo

class SanctumOfDominationRohKaloAI : public SanctumOfDominationBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationRohKaloAI(c); }
    explicit SanctumOfDominationRohKaloAI(Creature* pCreature) : SanctumOfDominationBossAI(pCreature, DATA_ROH_KALO) {}

protected:
    void onEngage() override
    {
        mRealigns = 0;
        mProbes = 0;
        armBerserk(RohKaloBerserkMs);
        startScrying(1);
    }

    void onDisengage() override { setUntouchable(false); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == 2 || mRealigns >= 2 || _getHealthPercent() > RohKaloRealignMarks[mRealigns])
            return;

        startRealign();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ROH_KALO_PROBE:
                // a short volley of probes, then a pause
                castSpellOnVictim(SPELL_ROH_KALO_DIVINERS_PROBE);
                scriptEvents.addEvent(EVENT_ROH_KALO_PROBE, (++mProbes % RohKaloProbeVolley) != 0 ? 2500 : 20000);
                break;
            case EVENT_ROH_KALO_CALL_OF_ETERNITY:
                castAtPlayers(SPELL_ROH_KALO_CALL_OF_ETERNITY, 3);
                scriptEvents.addEvent(EVENT_ROH_KALO_CALL_OF_ETERNITY, 30000);
                break;
            case EVENT_ROH_KALO_INVOKE_DESTINY:
                invokeDestiny();
                scriptEvents.addEvent(EVENT_ROH_KALO_INVOKE_DESTINY, 37000);
                break;
            case EVENT_ROH_KALO_CONJUNCTION:
                warn("Fatescribe Roh-Kalo weaves a Fated Conjunction!");
                castSpellAOE(SPELL_ROH_KALO_FATED_CONJUNCTION);
                scriptEvents.addEvent(EVENT_ROH_KALO_CONJUNCTION, 35000);
                break;
            case EVENT_ROH_KALO_TWIST_FATE:
                castAtPlayer(SPELL_ROH_KALO_TWIST_FATE);
                scriptEvents.addEvent(EVENT_ROH_KALO_TWIST_FATE, 40000);
                break;
            case EVENT_ROH_KALO_EXTEMPORANEOUS:
                warn("Fatescribe Roh-Kalo shapes an Extemporaneous Fate!");
                castSpellOnSelf(SPELL_ROH_KALO_EXTEMPORANEOUS_FATE);
                scriptEvents.addEvent(EVENT_ROH_KALO_EXTEMPORANEOUS, 45000);
                break;
            case EVENT_ROH_KALO_ACCRETION:
                castAtPlayers(SPELL_ROH_KALO_UNSTABLE_ACCRETION, 2);
                scriptEvents.addEvent(EVENT_ROH_KALO_ACCRETION, 8000);
                break;
            case EVENT_ROH_KALO_MONSTROSITY:
                callAdd(NPC_ROH_KALO_FATESPAWN_MONSTROSITY, aroundSelf(10.0f, 15.0f));
                break;
            case EVENT_ROH_KALO_REALIGN_END:
                setUntouchable(false);
                warn("The runes are realigned! Fatescribe Roh-Kalo resumes his assault!");
                startScrying(mRealigns >= 2 ? 3 : 1);
                break;
            default:
                break;
        }
    }

private:
    void startScrying(uint32_t stage)
    {
        enterStage(stage);
        scriptEvents.addEvent(EVENT_ROH_KALO_PROBE, 5000);
        scriptEvents.addEvent(EVENT_ROH_KALO_CALL_OF_ETERNITY, 12000);
        scriptEvents.addEvent(EVENT_ROH_KALO_INVOKE_DESTINY, 20000);
        scriptEvents.addEvent(EVENT_ROH_KALO_CONJUNCTION, 28000);
        scriptEvents.addEvent(EVENT_ROH_KALO_TWIST_FATE, 35000);

        if (stage == 3)
            scriptEvents.addEvent(EVENT_ROH_KALO_EXTEMPORANEOUS, 15000);
    }

    // the Loom of Fates draws his attention
    void startRealign()
    {
        ++mRealigns;
        enterStage(2);
        setUntouchable(true);
        warn("Fatescribe Roh-Kalo begins to Realign Fate!");
        castSpellAOE(SPELL_ROH_KALO_REALIGN_FATE, true);

        scriptEvents.addEvent(EVENT_ROH_KALO_ACCRETION, 5000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_ROH_KALO_MONSTROSITY, 3000);
        scriptEvents.addEvent(EVENT_ROH_KALO_REALIGN_END, RohKaloRealignMs);
    }

    // a Shade of Destiny closes in on the marked player
    void invokeDestiny()
    {
        Unit* pTarget = pickPlayer(true);
        if (pTarget == nullptr)
            return;

        Creature* pShade = summonCreature(NPC_ROH_KALO_SHADE_OF_DESTINY, sanctumRingPoint(pTarget->GetPosition(), 8.0f, 12.0f), CORPSE_DESPAWN);
        if (pShade != nullptr)
        {
            if (pShade->GetScript() != nullptr)
                pShade->GetScript()->SetCreatureData64(SOD_CREATURE_DATA_PREY, pTarget->getGuid());
            setZoneWideCombat(pShade);
        }

        castSpell(pTarget, SPELL_ROH_KALO_INVOKE_DESTINY);
    }

    uint32_t mRealigns = 0;
    uint32_t mProbes = 0;
};

// bound to one player, shatters into anomalies when destroyed
class SanctumOfDominationShadeOfDestinyAI : public SanctumOfDominationCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationShadeOfDestinyAI(c); }
    explicit SanctumOfDominationShadeOfDestinyAI(Creature* pCreature) : SanctumOfDominationCombatAI(pCreature) {}

    void SetCreatureData64(uint32_t type, uint64_t data) override
    {
        if (type == SOD_CREATURE_DATA_PREY)
            mPreyGuid = WoWGuid(data);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        scriptEvents.resetEvents();
        if (mReachedPrey)
            return;

        // the anomalies belong to the Fatescribe's fight
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr)
            return;

        for (Creature* pRohKalo : pInstance->getCreatureSetForEntry(BOSS_ROH_KALO))
        {
            if (!pRohKalo->isAlive())
                continue;

            for (uint8_t i = 0; i < 3; ++i)
            {
                Creature* pAnomaly = pRohKalo->summonCreature(NPC_ROH_KALO_FATESPAWN_ANOMALY, sanctumRingPoint(getCreature()->GetPosition(), 1.0f, 4.0f), CORPSE_DESPAWN);
                if (pAnomaly != nullptr)
                    setZoneWideCombat(pAnomaly);
            }
            break;
        }
    }

protected:
    void onEngage() override
    {
        mReachedPrey = false;
        _setMeleeDisabled(true);

        Unit* pPrey = findUnit(mPreyGuid);
        if (pPrey == nullptr || !pPrey->isAlive())
            pPrey = pickPlayer();

        bindPrey(pPrey);
        scriptEvents.addEvent(EVENT_SHADE_PURSUIT, 1000);
    }

    void onDisengage() override
    {
        _setMeleeDisabled(false);
        getCreature()->getThreatManager().clearFixate();
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SHADE_PURSUIT || mReachedPrey)
            return;

        Unit* pPrey = findUnit(mPreyGuid);
        if (pPrey == nullptr || !pPrey->isAlive())
        {
            bindPrey(pickPlayer());
        }
        else if (getCreature()->getDistance(pPrey) <= ShadeReach)
        {
            // the burden overwhelms its bearer
            mReachedPrey = true;
            castSpell(pPrey, SPELL_SHADE_OVERWHELMING_BURDEN, true);
            despawn(500, 0);
            return;
        }

        scriptEvents.addEvent(EVENT_SHADE_PURSUIT, 1000);
    }

private:
    void bindPrey(Unit* pPrey)
    {
        if (pPrey == nullptr)
            return;

        mPreyGuid = pPrey->GetNewGUID();
        castSpell(pPrey, SPELL_SHADE_BURDEN_OF_DESTINY, true);
        getCreature()->getThreatManager().fixateTarget(pPrey);
    }

    WoWGuid mPreyGuid;
    bool mReachedPrey = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Remnant of Ner'zhul

class SanctumOfDominationNerzhulAI : public SanctumOfDominationBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationNerzhulAI(c); }
    explicit SanctumOfDominationNerzhulAI(Creature* pCreature) : SanctumOfDominationBossAI(pCreature, DATA_NERZHUL_REMNANT) {}

protected:
    void onEngage() override
    {
        mShatters = 0;
        mEnergy = 0;
        mEnergyClock = 0;
        armBerserk(NerzhulBerserkMs);
        castSpellOnSelf(SPELL_NERZHUL_AURA_OF_SPITE, true);

        scriptEvents.addEvent(EVENT_NERZHUL_ORBS, 8000);
        scriptEvents.addEvent(EVENT_NERZHUL_MALEVOLENCE, 12000);
        scriptEvents.addEvent(EVENT_NERZHUL_GRASP, 20000);
    }

    void onDisengage() override { _removeAura(SPELL_NERZHUL_AURA_OF_SPITE); }

    void onCombatTick(uint32_t diff) override
    {
        // the armor breaks apart piece by piece
        if (mShatters < 3 && _getHealthPercent() <= NerzhulShatterMarks[mShatters])
        {
            ++mShatters;
            warn("A piece of the Remnant of Ner'zhul's armor shatters!");
            castSpellAOE(SPELL_NERZHUL_SHATTER, true);
        }

        if (!sanctumFill(mEnergy, mEnergyClock, diff, NerzhulEnergyStepMs) || _isCasting())
            return;

        mEnergy = 0;
        suffering();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NERZHUL_ORBS:
                warn("The Remnant of Ner'zhul manifests Orbs of Torment!");
                for (uint32_t i = 0; i < (isHeroic() ? 3u : 2u); ++i)
                    callAdd(NPC_NERZHUL_ORB_OF_TORMENT, aroundSelf(10.0f, 20.0f));
                scriptEvents.addEvent(EVENT_NERZHUL_ORBS, 35000);
                break;
            case EVENT_NERZHUL_MALEVOLENCE:
                castAtPlayers(SPELL_NERZHUL_MALEVOLENCE, isHeroic() ? 3 : 2);
                scriptEvents.addEvent(EVENT_NERZHUL_MALEVOLENCE, 30000);
                break;
            case EVENT_NERZHUL_GRASP:
                castSpellAOE(SPELL_NERZHUL_GRASP_OF_MALICE);
                scriptEvents.addEvent(EVENT_NERZHUL_GRASP, 25000);
                break;
            default:
                break;
        }
    }

private:
    // the orbs caught near the tank lose their protection
    void suffering()
    {
        Unit* pTank = getVictim();
        if (pTank == nullptr)
            return;

        for (Creature* pOrb : livingOfEntry(NPC_NERZHUL_ORB_OF_TORMENT))
        {
            if (pOrb->getDistance(pTank) <= SufferingReach && pOrb->GetScript() != nullptr)
                pOrb->GetScript()->DoAction(ACTION_ORB_STRUCK);
        }

        castSpell(pTank, SPELL_NERZHUL_SUFFERING);
    }

    uint32_t mShatters = 0;
    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
};

class SanctumOfDominationOrbOfTormentAI : public SanctumOfDominationCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationOrbOfTormentAI(c); }
    explicit SanctumOfDominationOrbOfTormentAI(Creature* pCreature) : SanctumOfDominationCombatAI(pCreature) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_ORB_STRUCK)
            _removeAura(SPELL_ORB_ETERNAL_TORMENT);
    }

protected:
    void onEngage() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_ORB_ETERNAL_TORMENT, true);

        scriptEvents.addEvent(EVENT_ORB_TORMENT, 2000);
        scriptEvents.addEvent(EVENT_ORB_PROCESSION, 12000);
    }

    void onDisengage() override
    {
        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ORB_TORMENT:
                castAtPlayer(SPELL_ORB_TORMENT, false, true);
                scriptEvents.addEvent(EVENT_ORB_TORMENT, 8000);
                break;
            case EVENT_ORB_PROCESSION:
                castAtPlayer(SPELL_ORB_SORROWFUL_PROCESSION, false, true);
                scriptEvents.addEvent(EVENT_ORB_PROCESSION, 25000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Eye of the Jailer

class SanctumOfDominationJailersEyeAI : public SanctumOfDominationBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationJailersEyeAI(c); }
    explicit SanctumOfDominationJailersEyeAI(Creature* pCreature) : SanctumOfDominationBossAI(pCreature, DATA_JAILERS_EYE) {}

protected:
    void onEngage() override
    {
        mRetreats = 0;
        armBerserk(JailersEyeBerserkMs);
        startGaze(1);
    }

    void onDisengage() override { setUntouchable(false); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage != 1 || mRetreats >= 2 || _getHealthPercent() > JailersEyeRetreatMarks[mRetreats])
            return;

        retreat();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_EYE_PIERCING_LENS:
                castSpellOnVictim(SPELL_EYE_PIERCING_LENS);
                scriptEvents.addEvent(EVENT_EYE_PIERCING_LENS, 12000);
                break;
            case EVENT_EYE_DEATHLINK:
                castSpellOnVictim(SPELL_EYE_DEATHLINK);
                scriptEvents.addEvent(EVENT_EYE_DEATHLINK, 20000);
                break;
            case EVENT_EYE_FRACTURE_SOUL:
                castAtPlayer(SPELL_EYE_FRACTURE_SOUL, true);
                scriptEvents.addEvent(EVENT_EYE_FRACTURE_SOUL, 25000);
                break;
            case EVENT_EYE_LETHARGY:
                castAtPlayers(SPELL_EYE_HOPELESS_LETHARGY, 2, true);
                scriptEvents.addEvent(EVENT_EYE_LETHARGY, 22000);
                break;
            case EVENT_EYE_GLARE:
                warn("The Eye of the Jailer focuses an Annihilating Glare!");
                castSpellAOE(SPELL_EYE_ANNIHILATING_GLARE);
                scriptEvents.addEvent(EVENT_EYE_GLARE, 45000);
                break;
            case EVENT_EYE_BOLT:
                castAtPlayer(SPELL_EYE_EYE_BOLT);
                scriptEvents.addEvent(EVENT_EYE_BOLT, 4000);
                break;
            case EVENT_EYE_DEATH_GAZE:
                warn("The Eye of the Jailer casts its Titanic Death Gaze!");
                castSpellAOE(SPELL_EYE_TITANIC_DEATH_GAZE);
                scriptEvents.addEvent(EVENT_EYE_DEATH_GAZE, 30000);
                break;
            case EVENT_EYE_DESOLATION_BEAM:
                castAtPlayer(SPELL_EYE_DESOLATION_BEAM, true);
                scriptEvents.addEvent(EVENT_EYE_DESOLATION_BEAM, mStage == 3 ? 15000 : 20000);
                break;
            case EVENT_EYE_WATCH_DEATHSEEKERS:
                if (livingOfEntry(NPC_EYE_DEATHSEEKER_EYE).empty())
                {
                    returnToPlatform();
                    break;
                }
                scriptEvents.addEvent(EVENT_EYE_WATCH_DEATHSEEKERS, 2000);
                break;
            case EVENT_EYE_EXTERMINATION:
                castSpellAOE(SPELL_EYE_IMMEDIATE_EXTERMINATION, true);
                scriptEvents.addEvent(EVENT_EYE_EXTERMINATION, 20000);
                break;
            default:
                break;
        }
    }

private:
    void startGaze(uint32_t stage)
    {
        enterStage(stage);
        scriptEvents.addEvent(EVENT_EYE_PIERCING_LENS, 6000);
        scriptEvents.addEvent(EVENT_EYE_DEATHLINK, 12000);
        scriptEvents.addEvent(EVENT_EYE_FRACTURE_SOUL, 18000);
        scriptEvents.addEvent(EVENT_EYE_LETHARGY, 24000);
        scriptEvents.addEvent(EVENT_EYE_GLARE, 40000);

        if (stage == 3)
        {
            scriptEvents.addEvent(EVENT_EYE_DESOLATION_BEAM, 10000);
            scriptEvents.addEvent(EVENT_EYE_EXTERMINATION, 20000);
        }
    }

    // two Deathseeker Eyes take the platform while it fights from afar
    void retreat()
    {
        ++mRetreats;
        enterStage(2);
        setUntouchable(true);
        warn("The Eye of the Jailer retreats and unleashes its Deathseeker Eyes!");

        for (uint8_t i = 0; i < 2; ++i)
            callAdd(NPC_EYE_DEATHSEEKER_EYE, aroundSelf(10.0f, 18.0f));

        scriptEvents.addEvent(EVENT_EYE_BOLT, 2000);
        scriptEvents.addEvent(EVENT_EYE_DESOLATION_BEAM, 8000);
        scriptEvents.addEvent(EVENT_EYE_DEATH_GAZE, 12000);
        scriptEvents.addEvent(EVENT_EYE_FRACTURE_SOUL, 15000);
        scriptEvents.addEvent(EVENT_EYE_WATCH_DEATHSEEKERS, 3000);
    }

    void returnToPlatform()
    {
        setUntouchable(false);
        if (mRetreats < 2)
        {
            warn("The Eye of the Jailer returns to the platform!");
            startGaze(1);
            return;
        }

        warn("The Eye of the Jailer becomes enraged and begins Immediate Extermination!");
        castSpellAOE(SPELL_EYE_IMMEDIATE_EXTERMINATION, true);
        startGaze(3);
    }

    uint32_t mRetreats = 0;
};

// the Deathseeker Eyes must fall together
class SanctumOfDominationDeathseekerEyeAI : public SanctumOfDominationCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationDeathseekerEyeAI(c); }
    explicit SanctumOfDominationDeathseekerEyeAI(Creature* pCreature) : SanctumOfDominationCombatAI(pCreature) {}

    uint32_t GetCreatureData(uint32_t type) const override { return type == SOD_CREATURE_DATA_EYE_HELD && mHeld ? 1 : 0; }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_DEATHSEEKER_RELEASE && mHeld)
            mReleasePending = true;
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        scriptEvents.resetEvents();

        // a held twin follows
        for (Creature* pTwin : livingOfEntry(getCreature()->getEntry()))
        {
            if (pTwin != getCreature() && pTwin->GetScript() != nullptr)
                pTwin->GetScript()->DoAction(ACTION_DEATHSEEKER_RELEASE);
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        // the killing blow is dealt outside of a damage callback
        if (mReleasePending && isAlive())
        {
            mReleasePending = false;
            mHeld = false;
            setUntouchable(false);
            getCreature()->dealDamage(getCreature(), getCreature()->getHealth(), 0);
            return;
        }

        SanctumOfDominationCombatAI::AIUpdate(time_passed);
    }

protected:
    void onEngage() override
    {
        mHeld = false;
        mReleasePending = false;

        scriptEvents.addEvent(EVENT_DEATHSEEKER_LENS, 4000);
        scriptEvents.addEvent(EVENT_DEATHSEEKER_CORRUPTION, 8000);
        scriptEvents.addEvent(EVENT_DEATHSEEKER_MISERY, 12000);
        scriptEvents.addEvent(EVENT_DEATHSEEKER_PROXIMITY, 3000);
    }

    void onDisengage() override
    {
        setUntouchable(false);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void onDamage(uint32_t& damage) override
    {
        uint32_t const health = getCreature()->getHealth();
        if (damage < health)
            return;

        if (mHeld || twinStanding())
        {
            damage = health > 1 ? health - 1 : 0;
            if (!mHeld)
                hold();
        }
    }

    void onEvent(uint32_t eventId) override
    {
        if (mHeld)
            return;

        switch (eventId)
        {
            case EVENT_DEATHSEEKER_LENS:
                castSpellOnVictim(SPELL_DEATHSEEKER_PIERCING_LENS);
                scriptEvents.addEvent(EVENT_DEATHSEEKER_LENS, 10000);
                break;
            case EVENT_DEATHSEEKER_CORRUPTION:
                castAtPlayer(SPELL_DEATHSEEKER_SLOTHFUL_CORRUPTION);
                scriptEvents.addEvent(EVENT_DEATHSEEKER_CORRUPTION, 20000);
                break;
            case EVENT_DEATHSEEKER_MISERY:
                // the primary target and one more
                if (Unit* pTank = getVictim())
                    castSpell(pTank, SPELL_DEATHSEEKER_SPREADING_MISERY, true);
                castAtPlayer(SPELL_DEATHSEEKER_SPREADING_MISERY, true, true);
                scriptEvents.addEvent(EVENT_DEATHSEEKER_MISERY, 22000);
                break;
            case EVENT_DEATHSEEKER_PROXIMITY:
                if (twinClose())
                    castSpellAOE(SPELL_DEATHSEEKER_SUFFERING, true);
                scriptEvents.addEvent(EVENT_DEATHSEEKER_PROXIMITY, 3000);
                break;
            default:
                break;
        }
    }

private:
    // falls alone: lingers and pulses Suffering until its twin follows
    void hold()
    {
        mHeld = true;
        scriptEvents.resetEvents();
        setUntouchable(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_DEATHSEEKER_DYING_SUFFERING, true);
    }

    bool twinStanding()
    {
        for (Creature* pTwin : livingOfEntry(getCreature()->getEntry()))
        {
            if (pTwin == getCreature())
                continue;

            CreatureAIScript* pScript = pTwin->GetScript();
            if (pScript == nullptr || pScript->GetCreatureData(SOD_CREATURE_DATA_EYE_HELD) == 0)
                return true;
        }

        return false;
    }

    bool twinClose()
    {
        for (Creature* pTwin : livingOfEntry(getCreature()->getEntry()))
        {
            if (pTwin != getCreature() && getCreature()->getDistance(pTwin) <= DeathseekerCloseRange)
                return true;
        }

        return false;
    }

    bool mHeld = false;
    bool mReleasePending = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Soulrender Dormazain

class SanctumOfDominationDormazainAI : public SanctumOfDominationBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationDormazainAI(c); }
    explicit SanctumOfDominationDormazainAI(Creature* pCreature) : SanctumOfDominationBossAI(pCreature, DATA_DORMAZAIN) {}

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        armBerserk(DormazainBerserkMs);

        scriptEvents.addEvent(EVENT_DORMAZAIN_TORMENT, 8000);
        scriptEvents.addEvent(EVENT_DORMAZAIN_RUINBLADE, 12000);
        scriptEvents.addEvent(EVENT_DORMAZAIN_BRAND, 20000);
        scriptEvents.addEvent(EVENT_DORMAZAIN_AGONIZERS, 25000);
        scriptEvents.addEvent(EVENT_DORMAZAIN_SHACKLES, 45000);
        scriptEvents.addEvent(EVENT_DORMAZAIN_OVERLORD, 50000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!sanctumFill(mEnergy, mEnergyClock, diff, DormazainEnergyStepMs) || _isCasting())
            return;

        mEnergy = 0;
        warn("Soulrender Dormazain unleashes Tormented Eruptions!");
        castSpellAOE(SPELL_DORMAZAIN_TORMENTED_ERUPTIONS);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DORMAZAIN_TORMENT:
                // tormenting Garrosh feeds his power
                castSpellAOE(SPELL_DORMAZAIN_TORMENT);
                mEnergy = mEnergy + DormazainTormentEnergy < 100 ? mEnergy + DormazainTormentEnergy : 99;
                scriptEvents.addEvent(EVENT_DORMAZAIN_TORMENT, 20000);
                break;
            case EVENT_DORMAZAIN_RUINBLADE:
                castSpellOnVictim(SPELL_DORMAZAIN_RUINBLADE);
                scriptEvents.addEvent(EVENT_DORMAZAIN_RUINBLADE, 32000);
                break;
            case EVENT_DORMAZAIN_BRAND:
                castAtPlayers(SPELL_DORMAZAIN_TORMENTED, 2, true);
                castAtPlayer(SPELL_DORMAZAIN_BRAND_OF_TORMENT, true);
                scriptEvents.addEvent(EVENT_DORMAZAIN_BRAND, 35000);
                break;
            case EVENT_DORMAZAIN_SHACKLES:
                warn("Soulrender Dormazain binds Garrosh with Warmonger Shackles!");
                castSpellAOE(SPELL_DORMAZAIN_WARMONGER_SHACKLES);
                scriptEvents.addEvent(EVENT_DORMAZAIN_HELLSCREAM, 12000);
                scriptEvents.addEvent(EVENT_DORMAZAIN_SHACKLES, 60000);
                break;
            case EVENT_DORMAZAIN_HELLSCREAM:
                garroshScreams();
                break;
            case EVENT_DORMAZAIN_AGONIZERS:
                for (uint32_t i = 0; i < (isHeroic() ? 3u : 2u); ++i)
                    callAdd(NPC_DORMAZAIN_MAWSWORN_AGONIZER, aroundSelf(20.0f, 30.0f));
                scriptEvents.addEvent(EVENT_DORMAZAIN_AGONIZERS, 50000);
                break;
            case EVENT_DORMAZAIN_OVERLORD:
                callAdd(NPC_DORMAZAIN_MAWSWORN_OVERLORD, aroundSelf(20.0f, 30.0f));
                scriptEvents.addEvent(EVENT_DORMAZAIN_OVERLORD, 75000);
                break;
            default:
                break;
        }
    }

private:
    // the shackled warlord roars across the room
    void garroshScreams()
    {
        SanctumOfDominationInstanceScript* pSanctum = getSanctum();
        Creature* pGarrosh = pSanctum != nullptr ? pSanctum->getTracked(NPC_DORMAZAIN_SHACKLED_GARROSH) : nullptr;
        if (pGarrosh == nullptr || !pGarrosh->isAlive())
            return;

        warn("Garrosh Hellscream lets out a furious Hellscream!");
        pGarrosh->castSpell(pGarrosh, SPELL_GARROSH_HELLSCREAM, false);
    }

    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
};

// defiant at full energy or once it stands next to Garrosh
class SanctumOfDominationMawswornAgonizerAI : public SanctumOfDominationCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationMawswornAgonizerAI(c); }
    explicit SanctumOfDominationMawswornAgonizerAI(Creature* pCreature) : SanctumOfDominationCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        mDefiant = false;
        scriptEvents.addEvent(EVENT_AGONIZER_NOVA, 6000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mDefiant)
            return;

        bool const full = sanctumFill(mEnergy, mEnergyClock, diff, AgonizerEnergyStepMs);
        if (!full && !nearGarrosh())
            return;

        mDefiant = true;
        castSpellOnSelf(SPELL_MAWSWORN_DEFIANCE, true);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_AGONIZER_NOVA)
            return;

        castSpellAOE(SPELL_AGONIZER_AGONIZING_NOVA);
        scriptEvents.addEvent(EVENT_AGONIZER_NOVA, 12000);
    }

private:
    bool nearGarrosh()
    {
        SanctumOfDominationInstanceScript* pSanctum = getSanctum();
        Creature* pGarrosh = pSanctum != nullptr ? pSanctum->getTracked(NPC_DORMAZAIN_SHACKLED_GARROSH) : nullptr;
        return pGarrosh != nullptr && getCreature()->getDistance(pGarrosh) <= GarroshReach;
    }

    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    bool mDefiant = false;
};

// empowers nearby Mawsworn, all of them once it reaches Garrosh
class SanctumOfDominationMawswornOverlordAI : public SanctumOfDominationCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationMawswornOverlordAI(c); }
    explicit SanctumOfDominationMawswornOverlordAI(Creature* pCreature) : SanctumOfDominationCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        mDefiant = false;
        scriptEvents.addEvent(EVENT_OVERLORD_INFUSE, 5000);
        scriptEvents.addEvent(EVENT_OVERLORD_REACH, 2000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_OVERLORD_INFUSE:
                castSpellOnSelf(SPELL_OVERLORD_INFUSE_DEFIANCE);
                scriptEvents.addEvent(EVENT_OVERLORD_INFUSE, 15000);
                break;
            case EVENT_OVERLORD_REACH:
                if (reachedGarrosh())
                {
                    mDefiant = true;
                    warn("A Mawsworn Overlord reaches Garrosh and empowers the Mawsworn!");
                    castSpellOnSelf(SPELL_OVERLORD_DEFIANT, true);
                    break;
                }
                scriptEvents.addEvent(EVENT_OVERLORD_REACH, 2000);
                break;
            default:
                break;
        }
    }

private:
    bool reachedGarrosh()
    {
        if (mDefiant)
            return false;

        SanctumOfDominationInstanceScript* pSanctum = getSanctum();
        Creature* pGarrosh = pSanctum != nullptr ? pSanctum->getTracked(NPC_DORMAZAIN_SHACKLED_GARROSH) : nullptr;
        return pGarrosh != nullptr && getCreature()->getDistance(pGarrosh) <= GarroshReach;
    }

    bool mDefiant = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sylvanas Windrunner

class SanctumOfDominationSylvanasAI : public SanctumOfDominationBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationSylvanasAI(c); }
    explicit SanctumOfDominationSylvanasAI(Creature* pCreature) : SanctumOfDominationBossAI(pCreature, DATA_SYLVANAS) {}

    void OnLoad() override
    {
        // the template carries a vehicle id, she fights on her own
        getCreature()->setAItoUse(true);
        setCanEnterCombat(true);
        castSpellOnSelf(SPELL_SYLVANAS_OFFHAND, true);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        // a defeated Sylvanas leaves without failing the encounter
        if (mYielded)
        {
            SanctumOfDominationCombatAI::OnCombatStop(pTarget);
            return;
        }

        SanctumOfDominationBossAI::OnCombatStop(pTarget);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mYieldPending && !mYielded && isAlive())
        {
            yield();
            return;
        }

        SanctumOfDominationBossAI::AIUpdate(time_passed);
    }

protected:
    void onEngage() override
    {
        mYieldPending = false;
        mRivesLeft = 0;
        mGoliathCalled = false;
        armBerserk(SylvanasBerserkMs);
        castSpellOnSelf(SPELL_SYLVANAS_DAGGER_FORM, true);
        startCycleOfHatred();
    }

    void onDisengage() override
    {
        setUntouchable(false);
        _removeAura(SPELL_SYLVANAS_BANSHEE_FORM);
        _removeAura(SPELL_SYLVANAS_BANSHEE_SHROUD);
    }

    void onDamage(uint32_t& damage) override
    {
        if (mYielded)
        {
            damage = 0;
            return;
        }

        // she is brought to her knees, not slain
        uint32_t const health = getCreature()->getHealth();
        uint32_t const yieldHealth = getCreature()->getMaxHealth() / 100 > 0 ? getCreature()->getMaxHealth() / 100 : 1;
        if (health > damage + yieldHealth)
            return;

        damage = health > yieldHealth ? health - yieldHealth : 0;
        mYieldPending = true;
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == SylvanasCycleOfHatred && _getHealthPercent() <= SylvanasMonumentPct)
            startMonument();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SYLVANAS_WINDRUNNER:
                // a burst of shots while she darts across the platform
                castSpellOnSelf(SPELL_SYLVANAS_WINDRUNNER, true);
                scriptEvents.addEvent(EVENT_SYLVANAS_WITHERING_FIRE, 1500);
                scriptEvents.addEvent(EVENT_SYLVANAS_DESECRATING_SHOT, 4000);
                scriptEvents.addEvent(EVENT_SYLVANAS_SHADOW_DAGGER, 6500);
                scriptEvents.addEvent(EVENT_SYLVANAS_WINDRUNNER, 45000);
                break;
            case EVENT_SYLVANAS_WITHERING_FIRE:
                castAtPlayers(SPELL_SYLVANAS_WITHERING_FIRE, 3);
                if (mStage != SylvanasCycleOfHatred)
                    scriptEvents.addEvent(EVENT_SYLVANAS_WITHERING_FIRE, 15000);
                break;
            case EVENT_SYLVANAS_DESECRATING_SHOT:
                castAtPlayer(SPELL_SYLVANAS_DESECRATING_SHOT, true);
                break;
            case EVENT_SYLVANAS_SHADOW_DAGGER:
                castAtPlayers(SPELL_SYLVANAS_SHADOW_DAGGER, 2, true);
                if (mStage != SylvanasCycleOfHatred)
                    scriptEvents.addEvent(EVENT_SYLVANAS_SHADOW_DAGGER, 25000);
                break;
            case EVENT_SYLVANAS_DOMINATION_CHAINS:
                warn("Sylvanas fires Domination Chains!");
                castAtPlayers(SPELL_SYLVANAS_DOMINATION_CHAINS, isHeroic() ? 5 : 4);
                scriptEvents.addEvent(EVENT_SYLVANAS_DOMINATION_CHAINS, 50000);
                break;
            case EVENT_SYLVANAS_VEIL:
                warn("Sylvanas casts a Veil of Darkness!");
                castAtPlayer(SPELL_SYLVANAS_VEIL_OF_DARKNESS, true);
                scriptEvents.addEvent(EVENT_SYLVANAS_VEIL, 45000);
                break;
            case EVENT_SYLVANAS_WAILING_ARROW:
                castSpellOnVictim(SPELL_SYLVANAS_WAILING_ARROW);
                scriptEvents.addEvent(EVENT_SYLVANAS_WAILING_ARROW, 35000);
                break;
            case EVENT_SYLVANAS_HEARTSEEKER:
                if (Unit* pTank = getVictim())
                {
                    castSpell(pTank, SPELL_SYLVANAS_BANSHEES_MARK, true);
                    castSpell(pTank, SPELL_SYLVANAS_RANGERS_HEARTSEEKER);
                }
                scriptEvents.addEvent(EVENT_SYLVANAS_HEARTSEEKER, 20000);
                break;
            case EVENT_SYLVANAS_RIVE:
                castAtPlayer(SPELL_SYLVANAS_RIVE, false, true);
                if (mRivesLeft > 0 && --mRivesLeft > 0)
                    scriptEvents.addEvent(EVENT_SYLVANAS_RIVE, 4000);
                break;
            case EVENT_SYLVANAS_BANSHEE_WAIL:
                castSpellAOE(SPELL_SYLVANAS_BANSHEE_WAIL);
                if (mStage == SylvanasBansheeQueen)
                    scriptEvents.addEvent(EVENT_SYLVANAS_BANSHEE_WAIL, 50000);
                break;
            case EVENT_SYLVANAS_MONUMENT_FALLS:
                startBansheeQueen();
                break;
            case EVENT_SYLVANAS_RUIN:
                warn("Sylvanas channels Ruin! Interrupt her!");
                castSpellAOE(SPELL_SYLVANAS_RUIN);
                scriptEvents.addEvent(EVENT_SYLVANAS_RUIN, 50000);
                break;
            case EVENT_SYLVANAS_HAUNTING_WAVE:
                castAtPlayer(SPELL_SYLVANAS_HAUNTING_WAVE);
                scriptEvents.addEvent(EVENT_SYLVANAS_HAUNTING_WAVE, 25000);
                break;
            case EVENT_SYLVANAS_FORCES:
                callAdd(NPC_SYLVANAS_MAWSWORN_VANGUARD, aroundSelf(20.0f, 30.0f));
                callAdd(NPC_SYLVANAS_MAWSWORN_HOPEBREAKER, aroundSelf(20.0f, 30.0f));
                scriptEvents.addEvent(EVENT_SYLVANAS_FORCES, 35000);
                break;
            case EVENT_SYLVANAS_WATCH_COMMANDERS:
                watchCommanders();
                break;
            case EVENT_SYLVANAS_BANE_ARROWS:
                castAtPlayers(SPELL_SYLVANAS_BANE_ARROWS, 3);
                scriptEvents.addEvent(EVENT_SYLVANAS_BANE_ARROWS, 25000);
                break;
            case EVENT_SYLVANAS_BANSHEES_HEARTSEEKER:
                castSpellOnVictim(SPELL_SYLVANAS_BANSHEES_HEARTSEEKER);
                scriptEvents.addEvent(EVENT_SYLVANAS_BANSHEES_HEARTSEEKER, 22000);
                break;
            case EVENT_SYLVANAS_BANSHEES_FURY:
                warn("Sylvanas unleashes Banshee's Fury!");
                castSpellAOE(SPELL_SYLVANAS_BANSHEES_FURY);
                scriptEvents.addEvent(EVENT_SYLVANAS_BANSHEES_FURY, 55000);
                break;
            case EVENT_SYLVANAS_BANSHEE_SCREAM:
                castAtPlayers(SPELL_SYLVANAS_BANSHEE_SCREAM, 3);
                scriptEvents.addEvent(EVENT_SYLVANAS_BANSHEE_SCREAM, 32000);
                break;
            case EVENT_SYLVANAS_RAZE:
                warn("Sylvanas begins to Raze the arena!");
                castSpellAOE(SPELL_SYLVANAS_RAZE);
                scriptEvents.addEvent(EVENT_SYLVANAS_RAZE, 75000);
                break;
            default:
                break;
        }
    }

private:
    void startCycleOfHatred()
    {
        enterStage(SylvanasCycleOfHatred);
        scriptEvents.addEvent(EVENT_SYLVANAS_HEARTSEEKER, 8000);
        scriptEvents.addEvent(EVENT_SYLVANAS_WINDRUNNER, 12000);
        scriptEvents.addEvent(EVENT_SYLVANAS_WAILING_ARROW, 22000);
        scriptEvents.addEvent(EVENT_SYLVANAS_VEIL, 30000);
        scriptEvents.addEvent(EVENT_SYLVANAS_DOMINATION_CHAINS, 40000);
    }

    // everyone is chained while she tears the tower apart
    void startMonument()
    {
        enterStage(SylvanasMonument);
        setUntouchable(true);
        warn("Sylvanas chains everyone with Domination Chains and tears the tower apart!");
        castSpellOnSelf(SPELL_SYLVANAS_BANSHEE_SHROUD, true);
        _castOnInrangePlayers(SPELL_SYLVANAS_DOMINATION_CHAINS, true);

        mRivesLeft = SylvanasRiveCount;
        scriptEvents.addEvent(EVENT_SYLVANAS_RIVE, 4000);
        scriptEvents.addEvent(EVENT_SYLVANAS_BANSHEE_WAIL, SylvanasMonumentMs - 4000);
        scriptEvents.addEvent(EVENT_SYLVANAS_MONUMENT_FALLS, SylvanasMonumentMs);
    }

    // she harries from above until the Mawforged commanders fall
    void startBansheeQueen()
    {
        enterStage(SylvanasBansheeQueen);
        _removeAura(SPELL_SYLVANAS_BANSHEE_SHROUD);
        castSpellOnSelf(SPELL_SYLVANAS_BANSHEE_FORM, true);
        castSpellOnSelf(SPELL_SYLVANAS_BOW_FORM, true);
        warn("Sylvanas takes to the skies in Banshee Form! Defeat the Mawforged commanders!");

        mGoliathCalled = false;
        callAdd(NPC_SYLVANAS_MAWFORGED_SOULJUDGE, aroundSelf(15.0f, 25.0f));
        callAdd(NPC_SYLVANAS_MAWFORGED_SUMMONER, aroundSelf(15.0f, 25.0f));
        for (uint8_t i = 0; i < 2; ++i)
            callAdd(NPC_SYLVANAS_MAWSWORN_VANGUARD, aroundSelf(20.0f, 30.0f));
        callAdd(NPC_SYLVANAS_MAWSWORN_HOPEBREAKER, aroundSelf(20.0f, 30.0f));

        scriptEvents.addEvent(EVENT_SYLVANAS_WITHERING_FIRE, 6000);
        scriptEvents.addEvent(EVENT_SYLVANAS_HAUNTING_WAVE, 10000);
        scriptEvents.addEvent(EVENT_SYLVANAS_SHADOW_DAGGER, 15000);
        scriptEvents.addEvent(EVENT_SYLVANAS_VEIL, 20000);
        scriptEvents.addEvent(EVENT_SYLVANAS_RUIN, 30000);
        scriptEvents.addEvent(EVENT_SYLVANAS_FORCES, 35000);
        scriptEvents.addEvent(EVENT_SYLVANAS_BANSHEE_WAIL, 40000);
        scriptEvents.addEvent(EVENT_SYLVANAS_WATCH_COMMANDERS, 3000);
    }

    void watchCommanders()
    {
        bool const firstAlive = !livingOfEntry(NPC_SYLVANAS_MAWFORGED_SOULJUDGE).empty() || !livingOfEntry(NPC_SYLVANAS_MAWFORGED_SUMMONER).empty();
        if (!firstAlive && !mGoliathCalled)
        {
            mGoliathCalled = true;
            warn("A Mawforged Goliath blocks the way!");
            callAdd(NPC_SYLVANAS_MAWFORGED_GOLIATH, aroundSelf(15.0f, 25.0f));
        }
        else if (!firstAlive && livingOfEntry(NPC_SYLVANAS_MAWFORGED_GOLIATH).empty())
        {
            startFreedomOfChoice();
            return;
        }

        scriptEvents.addEvent(EVENT_SYLVANAS_WATCH_COMMANDERS, 2000);
    }

    // the final showdown on the ground
    void startFreedomOfChoice()
    {
        enterStage(SylvanasFreedomOfChoice);
        summons.despawnAll();
        setUntouchable(false);
        _removeAura(SPELL_SYLVANAS_BANSHEE_FORM);
        castSpellOnSelf(SPELL_SYLVANAS_DAGGER_FORM, true);
        warn("Sylvanas lands for the final showdown!");

        scriptEvents.addEvent(EVENT_SYLVANAS_WITHERING_FIRE, 6000);
        scriptEvents.addEvent(EVENT_SYLVANAS_BANSHEES_HEARTSEEKER, 8000);
        scriptEvents.addEvent(EVENT_SYLVANAS_BANE_ARROWS, 12000);
        scriptEvents.addEvent(EVENT_SYLVANAS_SHADOW_DAGGER, 16000);
        scriptEvents.addEvent(EVENT_SYLVANAS_BANSHEE_SCREAM, 20000);
        scriptEvents.addEvent(EVENT_SYLVANAS_WAILING_ARROW, 26000);
        scriptEvents.addEvent(EVENT_SYLVANAS_VEIL, 32000);
        scriptEvents.addEvent(EVENT_SYLVANAS_BANSHEES_FURY, 45000);
        scriptEvents.addEvent(EVENT_SYLVANAS_RAZE, 70000);
    }

    void yield()
    {
        mYieldPending = false;
        mYielded = true;
        scriptEvents.resetEvents();
        summons.despawnAll();
        warn("Sylvanas Windrunner falls to her knees, defeated!");

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(DATA_SYLVANAS, EncounterStates::Performed);
        }

        setReactState(REACT_PASSIVE);
        attackStop();
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setCanEnterCombat(false);

        if (_isInCombat())
            getCreature()->getAIInterface()->enterEvadeMode();

        despawn(SylvanasLeaveMs, 0);
    }

    bool mYieldPending = false;
    bool mYielded = false;
    bool mGoliathCalled = false;
    uint32_t mRivesLeft = 0;
};

class SanctumOfDominationMawforgedSummonerAI : public SanctumOfDominationCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationMawforgedSummonerAI(c); }
    explicit SanctumOfDominationMawforgedSummonerAI(Creature* pCreature) : SanctumOfDominationCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_SUMMONER_ORBS, 6000);
        scriptEvents.addEvent(EVENT_SUMMONER_LETHARGY, 10000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SUMMONER_ORBS:
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_SYLVANAS_DECREPIT_ORB, aroundSelf(3.0f, 6.0f), TIMED_DESPAWN, DecrepitOrbLifeMs);
                castSpellAOE(SPELL_SUMMONER_DECREPIT_ORBS);
                scriptEvents.addEvent(EVENT_SUMMONER_ORBS, 25000);
                break;
            case EVENT_SUMMONER_LETHARGY:
                castAtPlayer(SPELL_SUMMONER_CURSE_OF_LETHARGY);
                scriptEvents.addEvent(EVENT_SUMMONER_LETHARGY, 16000);
                break;
            default:
                break;
        }
    }
};

// drifts at a player and detonates on contact
class SanctumOfDominationDecrepitOrbAI : public SanctumOfDominationCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationDecrepitOrbAI(c); }
    explicit SanctumOfDominationDecrepitOrbAI(Creature* pCreature) : SanctumOfDominationCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        mDetonated = false;
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_DECREPIT_ORB, true);
        chaseNewPrey();
        scriptEvents.addEvent(EVENT_DECREPIT_ORB_REACH, 1000);
    }

    void onDisengage() override
    {
        _setMeleeDisabled(false);
        getCreature()->getThreatManager().clearFixate();
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_DECREPIT_ORB_REACH || mDetonated)
            return;

        Unit* pPrey = findUnit(mPreyGuid);
        if (pPrey == nullptr || !pPrey->isAlive())
        {
            chaseNewPrey();
        }
        else if (getCreature()->getDistance(pPrey) <= DecrepitOrbReach)
        {
            mDetonated = true;
            castSpellAOE(SPELL_DECREPIT_ORB_DETONATION, true);
            despawn(300, 0);
            return;
        }

        scriptEvents.addEvent(EVENT_DECREPIT_ORB_REACH, 1000);
    }

private:
    void chaseNewPrey()
    {
        Unit* pPrey = pickPlayer();
        if (pPrey == nullptr)
            return;

        mPreyGuid = pPrey->GetNewGUID();
        getCreature()->getThreatManager().fixateTarget(pPrey);
    }

    WoWGuid mPreyGuid;
    bool mDetonated = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Guardian of the First Ones

class SanctumOfDominationGuardianAI : public SanctumOfDominationBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationGuardianAI(c); }
    explicit SanctumOfDominationGuardianAI(Creature* pCreature) : SanctumOfDominationBossAI(pCreature, DATA_FIRST_ONES_GUARDIAN) {}

protected:
    void onEngage() override
    {
        mEnergy = GuardianEnergyMax;
        mLinked = false;
        mPurging = false;
        armBerserk(GuardianBerserkMs);
        placeEnergyCores();

        scriptEvents.addEvent(EVENT_GUARDIAN_ENERGY, 1000);
        scriptEvents.addEvent(EVENT_GUARDIAN_ELIMINATION, 8000);
        scriptEvents.addEvent(EVENT_GUARDIAN_NEUTRALIZATION, 14000);
        scriptEvents.addEvent(EVENT_GUARDIAN_SENTRY, 20000);
        scriptEvents.addEvent(EVENT_GUARDIAN_DISINTEGRATION, 30000);
    }

    void onDisengage() override
    {
        _removeAura(SPELL_GUARDIAN_ENERGIZING_LINK);
        _removeAura(SPELL_GUARDIAN_PURGING_PROTOCOL);

        // placed cores recover for the next attempt
        for (Creature* pCore : livingOfEntry(NPC_GUARDIAN_ENERGY_CORE))
        {
            if (pCore->GetScript() != nullptr)
                pCore->GetScript()->DoAction(ACTION_CORE_RESET);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GUARDIAN_ENERGY:
                updateEnergy();
                scriptEvents.addEvent(EVENT_GUARDIAN_ENERGY, 1000);
                break;
            case EVENT_GUARDIAN_PURGE:
                if (!mPurging)
                    break;
                castSpellAOE(SPELL_GUARDIAN_PURGING_PROTOCOL, true);
                scriptEvents.addEvent(EVENT_GUARDIAN_PURGE, 4000);
                break;
            case EVENT_GUARDIAN_ELIMINATION:
                castSpellOnSelf(SPELL_GUARDIAN_ELIMINATION_PATTERN, true);
                castSpellOnVictim(SPELL_GUARDIAN_SUNDER);
                scriptEvents.addEvent(EVENT_GUARDIAN_OBLITERATE, 4000);
                scriptEvents.addEvent(EVENT_GUARDIAN_ELIMINATION, 25000);
                break;
            case EVENT_GUARDIAN_OBLITERATE:
                castSpellOnVictim(SPELL_GUARDIAN_OBLITERATE);
                break;
            case EVENT_GUARDIAN_NEUTRALIZATION:
                castAtPlayer(SPELL_GUARDIAN_THREAT_NEUTRALIZATION, true);
                scriptEvents.addEvent(EVENT_GUARDIAN_NEUTRALIZATION, 28000);
                break;
            case EVENT_GUARDIAN_SENTRY:
                warn("The Guardian forms a Sentry!");
                callAdd(NPC_GUARDIAN_SENTRY, aroundSelf(10.0f, 20.0f));
                scriptEvents.addEvent(EVENT_GUARDIAN_SENTRY, 40000);
                break;
            case EVENT_GUARDIAN_DISINTEGRATION:
                warn("The Guardian of the First Ones begins Disintegration!");
                castAtPlayer(SPELL_GUARDIAN_DISINTEGRATION, true);
                scriptEvents.addEvent(EVENT_GUARDIAN_DISINTEGRATION, 35000);
                break;
            default:
                break;
        }
    }

private:
    // three cores around the room when none is placed
    void placeEnergyCores()
    {
        if (!livingOfEntry(NPC_GUARDIAN_ENERGY_CORE).empty())
            return;

        LocationVector const center = getCreature()->GetPosition();
        float const start = Util::getRandomFloat(SanctumTwoPi);
        for (uint8_t i = 0; i < 3; ++i)
        {
            float const angle = start + i * SanctumTwoPi / 3.0f;
            LocationVector const spot(center.x + CoreRingRange * std::cos(angle), center.y + CoreRingRange * std::sin(angle), center.z, angle);
            summonCreature(NPC_GUARDIAN_ENERGY_CORE, spot, MANUAL_DESPAWN);
        }
    }

    Creature* coreInReach()
    {
        Creature* pBest = nullptr;
        float bestRange = CoreLinkRange;
        for (Creature* pCore : livingOfEntry(NPC_GUARDIAN_ENERGY_CORE))
        {
            CreatureAIScript* pScript = pCore->GetScript();
            if (pScript != nullptr && pScript->GetCreatureData(SOD_CREATURE_DATA_CORE_SPENT) != 0)
                continue;

            float const range = getCreature()->getDistance(pCore);
            if (range <= bestRange)
            {
                bestRange = range;
                pBest = pCore;
            }
        }

        return pBest;
    }

    // energy drains away, a nearby core refills it
    void updateEnergy()
    {
        Creature* pCore = coreInReach();
        if (pCore != nullptr)
        {
            if (!mLinked)
            {
                mLinked = true;
                castSpellOnSelf(SPELL_GUARDIAN_ENERGIZING_LINK, true);
            }

            if (mPurging)
            {
                mPurging = false;
                _removeAura(SPELL_GUARDIAN_PURGING_PROTOCOL);
            }

            mEnergy = mEnergy + GuardianLinkGain < GuardianEnergyMax ? mEnergy + GuardianLinkGain : GuardianEnergyMax;
            if (pCore->GetScript() != nullptr)
                pCore->GetScript()->DoAction(ACTION_CORE_DRAIN);
            return;
        }

        if (mLinked)
        {
            mLinked = false;
            _removeAura(SPELL_GUARDIAN_ENERGIZING_LINK);
        }

        if (mEnergy > 0)
            --mEnergy;

        if (mEnergy == 0 && !mPurging)
        {
            mPurging = true;
            warn("The Guardian's energy is depleted! Purging Protocol activated!");
            scriptEvents.addEvent(EVENT_GUARDIAN_PURGE, 500);
        }
    }

    uint32_t mEnergy = 0;
    bool mLinked = false;
    bool mPurging = false;
};

// feeds the Guardian until it melts down
class SanctumOfDominationEnergyCoreAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanctumOfDominationEnergyCoreAI(c); }
    explicit SanctumOfDominationEnergyCoreAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        mCharge = CoreChargeMax;
        mSpent = false;
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_CORE_RADIANT_ENERGY, true);
    }

    uint32_t GetCreatureData(uint32_t type) const override { return type == SOD_CREATURE_DATA_CORE_SPENT && mSpent ? 1 : 0; }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_CORE_RESET)
        {
            if (!mSpent)
                return;

            mSpent = false;
            mCharge = CoreChargeMax;
            castSpellOnSelf(SPELL_CORE_RADIANT_ENERGY, true);
            return;
        }

        if (action != ACTION_CORE_DRAIN || mSpent)
            return;

        mCharge = mCharge > CoreDrainPerLink ? mCharge - CoreDrainPerLink : 0;
        if (mCharge > 0)
            return;

        // a drained core melts down
        mSpent = true;
        _removeAura(SPELL_CORE_RADIANT_ENERGY);
        castSpellAOE(SPELL_CORE_MELTDOWN, true);
    }

private:
    uint32_t mCharge = CoreChargeMax;
    bool mSpent = false;
};

void SetupSanctumOfDomination(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SANCTUM_OF_DOMINATION, &SanctumOfDominationInstanceScript::Create);

    // Kel'Thuzad
    mgr->register_creature_script(BOSS_SOD_KEL_THUZAD, &SanctumOfDominationKelThuzadAI::Create);
    mgr->register_creature_script(NPC_KT_SOUL_SHARD, &SanctumOfDominationKitAI::CreateSoulShard);
    mgr->register_creature_script(NPC_KT_GLACIAL_SPIKE, &SanctumOfDominationGlacialSpikeAI::Create);
    mgr->register_creature_script(NPC_KT_FROSTBOUND_DEVOTED, &SanctumOfDominationFrostboundDevotedAI::Create);
    mgr->register_creature_script(NPC_KT_FROSTBOUND_DEVOTED_ALT, &SanctumOfDominationFrostboundDevotedAI::Create);
    mgr->register_creature_script(NPC_KT_FROSTBOUND_DEVOTED_ALT2, &SanctumOfDominationFrostboundDevotedAI::Create);
    mgr->register_creature_script(NPC_KT_UNSTOPPABLE_ABOMINATION, &SanctumOfDominationKitAI::CreateAbomination);
    mgr->register_creature_script(NPC_KT_UNSTOPPABLE_ABOMINATION_ALT, &SanctumOfDominationKitAI::CreateAbomination);
    mgr->register_creature_script(NPC_KT_SOUL_REAVER, &SanctumOfDominationKitAI::CreateSoulReaver);
    mgr->register_creature_script(NPC_KT_SOUL_REAVER_ALT, &SanctumOfDominationKitAI::CreateSoulReaver);

    // The Tarragrue
    mgr->register_creature_script(BOSS_TARRAGRUE, &SanctumOfDominationTarragrueAI::Create);

    // The Nine
    mgr->register_creature_script(BOSS_NINE_KYRA, &SanctumOfDominationKyraAI::Create);
    mgr->register_creature_script(BOSS_NINE_SIGNE, &SanctumOfDominationSigneAI::Create);
    mgr->register_creature_script(BOSS_NINE_SKYJA, &SanctumOfDominationSkyjaAI::Create);
    mgr->register_creature_script(NPC_NINE_FORMLESS_MASS, &SanctumOfDominationKitAI::CreateFormlessMass);

    // Painsmith Raznal
    mgr->register_creature_script(BOSS_RAZNAL, &SanctumOfDominationRaznalAI::Create);
    mgr->register_creature_script(NPC_RAZNAL_SPIKED_BALL, &SanctumOfDominationSpikedBallAI::Create);
    mgr->register_creature_script(NPC_RAZNAL_SPIKED_BALL_ALT, &SanctumOfDominationSpikedBallAI::Create);

    // Fatescribe Roh-Kalo
    mgr->register_creature_script(BOSS_ROH_KALO, &SanctumOfDominationRohKaloAI::Create);
    mgr->register_creature_script(NPC_ROH_KALO_SHADE_OF_DESTINY, &SanctumOfDominationShadeOfDestinyAI::Create);
    mgr->register_creature_script(NPC_ROH_KALO_FATESPAWN_MONSTROSITY, &SanctumOfDominationKitAI::CreateMonstrosity);

    // Remnant of Ner'zhul
    mgr->register_creature_script(BOSS_NERZHUL_REMNANT, &SanctumOfDominationNerzhulAI::Create);
    mgr->register_creature_script(NPC_NERZHUL_ORB_OF_TORMENT, &SanctumOfDominationOrbOfTormentAI::Create);

    // The Eye of the Jailer
    mgr->register_creature_script(BOSS_JAILERS_EYE, &SanctumOfDominationJailersEyeAI::Create);
    mgr->register_creature_script(NPC_EYE_DEATHSEEKER_EYE, &SanctumOfDominationDeathseekerEyeAI::Create);
    mgr->register_creature_script(NPC_EYE_DEATHSEEKER_EYE_ALT, &SanctumOfDominationDeathseekerEyeAI::Create);
    mgr->register_creature_script(NPC_EYE_DEATHSEEKER_EYE_ALT2, &SanctumOfDominationDeathseekerEyeAI::Create);

    // Soulrender Dormazain
    mgr->register_creature_script(BOSS_DORMAZAIN, &SanctumOfDominationDormazainAI::Create);
    mgr->register_creature_script(NPC_DORMAZAIN_MAWSWORN_AGONIZER, &SanctumOfDominationMawswornAgonizerAI::Create);
    mgr->register_creature_script(NPC_DORMAZAIN_MAWSWORN_OVERLORD, &SanctumOfDominationMawswornOverlordAI::Create);

    // Sylvanas Windrunner
    mgr->register_creature_script(BOSS_SOD_SYLVANAS, &SanctumOfDominationSylvanasAI::Create);
    mgr->register_creature_script(NPC_SYLVANAS_MAWSWORN_VANGUARD, &SanctumOfDominationKitAI::CreateVanguard);
    mgr->register_creature_script(NPC_SYLVANAS_MAWSWORN_HOPEBREAKER, &SanctumOfDominationKitAI::CreateHopebreaker);
    mgr->register_creature_script(NPC_SYLVANAS_MAWFORGED_SOULJUDGE, &SanctumOfDominationKitAI::CreateSouljudge);
    mgr->register_creature_script(NPC_SYLVANAS_MAWFORGED_SUMMONER, &SanctumOfDominationMawforgedSummonerAI::Create);
    mgr->register_creature_script(NPC_SYLVANAS_MAWFORGED_GOLIATH, &SanctumOfDominationKitAI::CreateGoliath);
    mgr->register_creature_script(NPC_SYLVANAS_DECREPIT_ORB, &SanctumOfDominationDecrepitOrbAI::Create);

    // Guardian of the First Ones
    mgr->register_creature_script(BOSS_FIRST_ONES_GUARDIAN, &SanctumOfDominationGuardianAI::Create);
    mgr->register_creature_script(NPC_GUARDIAN_ENERGY_CORE, &SanctumOfDominationEnergyCoreAI::Create);
    mgr->register_creature_script(NPC_GUARDIAN_SENTRY, &SanctumOfDominationKitAI::CreateSentry);
}
