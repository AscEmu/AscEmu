/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Amirdrassil.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

enum AmirdrassilEvents
{
    // Fyrakk the Blazing
    EVENT_FYRAKK_BITE = 1,
    EVENT_FYRAKK_WILDFIRE,
    EVENT_FYRAKK_FIRESTORM,
    EVENT_FYRAKK_DREAM_REND,
    EVENT_FYRAKK_BLAZE,
    EVENT_FYRAKK_AFLAME,
    EVENT_FYRAKK_CORRUPT,
    EVENT_FYRAKK_ORBS,
    EVENT_FYRAKK_ASSAULT_OVER,
    EVENT_FYRAKK_GREATER_FIRESTORM,
    EVENT_FYRAKK_FLAMEFALL,
    EVENT_FYRAKK_DEVASTATION,
    EVENT_FYRAKK_ETERNAL_FIRESTORM,
    EVENT_FYRAKK_APOCALYPSE_ROAR,
    EVENT_FYRAKK_BREATH,
    EVENT_FYRAKK_INFERNAL_MAW,

    // Nymue
    EVENT_NYMUE_THREADED_BLAST,
    EVENT_NYMUE_VIRIDIAN_RAIN,
    EVENT_NYMUE_WEAVERS_BURDEN,
    EVENT_NYMUE_IMPENDING_LOOM,
    EVENT_NYMUE_CONTINUUM,
    EVENT_NYMUE_UNRAVEL,
    EVENT_NYMUE_BLOOM_FADES,

    // Igira the Cruel
    EVENT_IGIRA_BLISTERING_SPEAR,
    EVENT_IGIRA_TWISTING_BLADE,
    EVENT_IGIRA_SEARING_SPARKS,
    EVENT_IGIRA_FORGE,
    EVENT_IGIRA_HACKING_TORMENT,
    EVENT_IGIRA_SLICING_TORMENT,
    EVENT_IGIRA_FLAYING_TORMENT,

    // Council of Dreams
    EVENT_URCTOS_AGONIZING_CLAWS,
    EVENT_URCTOS_BARRELING_CHARGE,
    EVENT_AERWYNN_POISONOUS_JAVELIN,
    EVENT_AERWYNN_NOXIOUS_BLOSSOM,
    EVENT_PIP_POLYMORPH_BOMB,
    EVENT_PIP_EMERALD_WINDS,

    // Larodar
    EVENT_LARODAR_FIERY_FORCE,
    EVENT_LARODAR_SCORCHING_ROOTS,
    EVENT_LARODAR_FURIOUS_CHARGE,
    EVENT_LARODAR_FURIOUS_OUTBURST,
    EVENT_LARODAR_BLAZING_THORNS,
    EVENT_LARODAR_CONSUMING_FLAME,
    EVENT_LARODAR_AVATAR_OF_ASH,
    EVENT_LARODAR_FALLING_EMBERS,
    EVENT_LARODAR_FLASH_FIRE,
    EVENT_LARODAR_FIRE_WHIRL,
    EVENT_LARODAR_SMOLDERING_BACKDRAFT,
    EVENT_LARODAR_ASHEN_CALL,

    // Volcoross
    EVENT_VOLCOROSS_CATACLYSM_JAWS,
    EVENT_VOLCOROSS_SCORCHTAIL_CRASH,
    EVENT_VOLCOROSS_SERPENTS_FURY,
    EVENT_VOLCOROSS_VOLCANIC_DISGORGE,

    // Tindral Sageswift
    EVENT_TINDRAL_FALLING_STAR,
    EVENT_TINDRAL_MASS_ENTANGLEMENT,
    EVENT_TINDRAL_FIERY_GROWTH,
    EVENT_TINDRAL_BLAZING_MUSHROOM,
    EVENT_TINDRAL_FLAME_SURGE,
    EVENT_TINDRAL_SUNFLAME,
    EVENT_TINDRAL_FIRE_BEAM,
    EVENT_TINDRAL_FLARE_BOMB,
    EVENT_TINDRAL_SUPPRESSIVE_EMBER,
    EVENT_TINDRAL_FLAMING_GERMINATION,
    EVENT_TINDRAL_SEER_SUPERNOVA,
    EVENT_TINDRAL_SUPERNOVA,
    EVENT_TINDRAL_PURSUIT_OVER,

    // Gnarlroot
    EVENT_GNARLROOT_FLAMING_PESTILENCE,
    EVENT_GNARLROOT_SHADOWFLAME_CLEAVE,
    EVENT_GNARLROOT_TORTURED_SCREAM,
    EVENT_GNARLROOT_CONTROLLED_BURN,
    EVENT_GNARLROOT_DREADFIRE_BARRAGE,
    EVENT_GNARLROOT_UPROOTED,
    EVENT_GNARLROOT_RECOVERED,

    // Smolderon
    EVENT_SMOLDERON_LAVA_GEYSERS,
    EVENT_SMOLDERON_BRAND_OF_DAMNATION,
    EVENT_SMOLDERON_OVERHEATED,
    EVENT_SMOLDERON_FLAMES_SUBSIDE,

    // add spell tables use one event per row from here on
    EVENT_AMIRDRASSIL_ADD_SPELL_BASE = 500
};

enum AmirdrassilActions
{
    ACTION_DREAMER_LET_GO = 1
};

namespace
{
    float const AmirdrassilFullCircle = 6.28318531f;

    // berserk timers
    uint32_t const FyrakkBerserkMs = 600000;
    uint32_t const NymueBerserkMs = 540000;
    uint32_t const IgiraBerserkMs = 480000;
    uint32_t const CouncilBerserkMs = 540000;
    uint32_t const LarodarBerserkMs = 600000;
    uint32_t const VolcorossBerserkMs = 420000;
    uint32_t const TindralBerserkMs = 600000;
    uint32_t const GnarlrootBerserkMs = 480000;
    uint32_t const SmolderonBerserkMs = 540000;

    // time until a bar is full
    uint32_t const NymueBloomMs = 80000;
    uint32_t const IgiraTormentMs = 90000;
    uint32_t const CouncilUltimateMs = 60000;
    uint32_t const LarodarInfernoMs = 75000;
    uint32_t const VolcorossFloodMs = 70000;
    uint32_t const GnarlrootCultivationMs = 80000;
    uint32_t const SmolderonFlamesMs = 90000;
    uint32_t const ColossusCoreMs = 45000;

    // stage marks in health percent
    int32_t const FyrakkAssaultPct = 70;
    int32_t const FyrakkIncarnatePct = 35;
    int32_t const TindralFirstPursuitPct = 65;
    int32_t const TindralSecondPursuitPct = 35;

    uint32_t const LarodarInfernosToAsh = 3;
    uint32_t const RenewedTreantLifeMs = 12000;
    uint32_t const ScreamingSoulLifeMs = 60000;
    uint32_t const LivingFlameLifeMs = 30000;
    float const LivingFlameReach = 5.0f;

    uint32_t const DreamerRebirthMs = 12000;
    uint32_t const DreamerRebirthHealthPct = 25;
    uint32_t const DreamerFadedQuery = 1;

    uint32_t const CouncilOfDreams[3] = { BOSS_URCTOS, BOSS_AERWYNN, BOSS_PIP };

    enum FyrakkStage : uint32_t
    {
        FyrakkDreamRender = 1,
        FyrakkHeartAssault,
        FyrakkChildrenOfTheStars,
        FyrakkShadowflameIncarnate
    };

    enum NymueStage : uint32_t
    {
        NymueRapidIteration = 1,
        NymueCreationComplete
    };

    enum LarodarStage : uint32_t
    {
        LarodarCycleOfFlame = 1,
        LarodarUnreborn,
        LarodarAvatarOfAsh
    };

    enum TindralStage : uint32_t
    {
        TindralMoonkin = 1,
        TindralBurningPursuit,
        TindralTreeOfTheFlame,
        TindralPathOfFlame,
        TindralSeerOfTheFlame
    };

    enum GnarlrootStage : uint32_t
    {
        GnarlrootGardenOfDespair = 1,
        GnarlrootDoomCultivation,
        GnarlrootUprooted
    };

    enum SmolderonStage : uint32_t
    {
        SmolderonFirelordsFury = 1,
        SmolderonWorldInFlames
    };

    enum IgiraTorment : uint32_t
    {
        IgiraUnarmed = 0,
        IgiraHackingTorment,
        IgiraSlicingTorment,
        IgiraFlayingTorment
    };

    // energy as elapsed time against the time to a full bar
    class AmirdrassilEnergy
    {
    public:
        void start(uint32_t startPct, uint32_t msToFull)
        {
            mMsToFull = msToFull;
            mElapsedMs = msToFull / 100 * std::min<uint32_t>(startPct, 100);
        }

        void stop()
        {
            mMsToFull = 0;
            mElapsedMs = 0;
        }

        void empty() { mElapsedMs = 0; }

        // true while the bar is full
        bool advance(uint32_t diff)
        {
            if (mMsToFull == 0)
                return false;

            mElapsedMs = std::min(mElapsedMs + diff, mMsToFull);
            return mElapsedMs >= mMsToFull;
        }

    private:
        uint32_t mMsToFull = 0;
        uint32_t mElapsedMs = 0;
    };

    enum AmirdrassilCastAt : uint8_t
    {
        CAST_AT_VICTIM,
        CAST_AT_PLAYER,
        CAST_AT_AREA,
        CAST_AT_MASTER
    };

    struct AmirdrassilAddSpell
    {
        uint32_t spellId;
        AmirdrassilCastAt castAt;
        uint32_t firstMs;
        uint32_t repeatMs;
    };

    std::vector<AmirdrassilAddSpell> const BurningColossusSpells =
    {
        { SPELL_COLOSSUS_MOLTEN_GAUNTLET,       CAST_AT_VICTIM, 5000,  8000 },
        { SPELL_COLOSSUS_PULSING_CORE,          CAST_AT_AREA,   7000,  10000 }
    };

    std::vector<AmirdrassilAddSpell> const ScreamingSoulSpells =
    {
        { SPELL_SCREAMING_SOUL_SEARING_SCREAMS, CAST_AT_AREA,   3000,  7000 }
    };

    std::vector<AmirdrassilAddSpell> const CycleWardenSpells =
    {
        { SPELL_WARDEN_LIFE_WARD,               CAST_AT_MASTER, 6000,  20000 },
        { SPELL_WARDEN_LUMBERING_SLAM,          CAST_AT_VICTIM, 9000,  14000 },
        { SPELL_WARDEN_RADIAL_FLOURISH,         CAST_AT_AREA,   12000, 18000 }
    };

    std::vector<AmirdrassilAddSpell> const FieryTreantSpells =
    {
        { SPELL_TREANT_BLISTERING_SPLINTERS,    CAST_AT_PLAYER, 4000,  10000 },
        { SPELL_TREANT_FIERY_FLOURISH,          CAST_AT_AREA,   8000,  15000 }
    };

    std::vector<AmirdrassilAddSpell> const ScorchingRootsSpells =
    {
        { SPELL_ROOTS_SCORCHING_BRAMBLETHORN,   CAST_AT_VICTIM, 3000,  8000 }
    };

    std::vector<AmirdrassilAddSpell> const TaintedLasherSpells =
    {
        { SPELL_LASHER_SHADOW_SPINES,           CAST_AT_PLAYER, 2000,  6000 },
        { SPELL_LASHER_VICIOUS_THICKET,         CAST_AT_AREA,   9000,  15000 }
    };

    std::vector<AmirdrassilAddSpell> const NoAddSpells = {};

    LocationVector amirdrassilScatter(LocationVector const& center, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(AmirdrassilFullCircle);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(center.x + range * std::cos(angle), center.y + range * std::sin(angle), center.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class AmirdrassilInstanceScript : public InstanceScript
{
public:
    explicit AmirdrassilInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(AmirdrassilEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new AmirdrassilInstanceScript(pMapMgr); }

    // the council is only defeated with its last dreamer
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isEncounterCleared(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (id != DATA_COUNCIL_OF_DREAMS || state != EncounterStates::Failed)
            return;

        // fallen dreamers return for the next attempt
        for (Creature* pDreamer : getCouncil())
        {
            if (!pDreamer->isAlive())
                pDreamer->despawn(1000, 5000);
        }
    }

    std::vector<Creature*> getCouncil()
    {
        std::vector<Creature*> council;
        for (uint32_t entry : CouncilOfDreams)
        {
            for (Creature* pDreamer : getCreatureSetForEntry(entry))
                council.push_back(pDreamer);
        }

        return council;
    }

    // single boss encounters end with their boss
    bool isEncounterCleared(uint32_t dataId, Creature* pExcept)
    {
        if (dataId != DATA_COUNCIL_OF_DREAMS)
            return true;

        for (Creature* pDreamer : getCouncil())
        {
            if (pDreamer != pExcept && pDreamer->isAlive())
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

    // one dreamer pulled wakes the others
    void wakeCouncil(CreatureAIScript* pPuller)
    {
        if (mWakingCouncil)
            return;

        mWakingCouncil = true;
        for (Creature* pDreamer : getCouncil())
        {
            if (pDreamer != pPuller->getCreature() && pDreamer->isAlive() && !pDreamer->isInCombat())
                pPuller->setZoneWideCombat(pDreamer);
        }
        mWakingCouncil = false;
    }

private:
    bool mWakingCouncil = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event queue, berserk, held state and target helpers

class AmirdrassilFightAI : public CreatureAIScript
{
public:
    explicit AmirdrassilFightAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mPhase = 1;
        mHeld = false;
        onPull();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        clearFight();
        onReset();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);

        // a held creature only keeps its own clock running
        if (mHeld)
        {
            onFightTick(diff);
            return;
        }

        scriptEvents.updateEvents(diff, 0);
        tickBerserk(diff);
        onFightTick(diff);

        while (!mHeld && !_isCasting())
        {
            uint32_t const eventId = scriptEvents.getFinishedEvent();
            if (eventId == 0)
                break;

            onTimedEvent(eventId);
        }
    }

protected:
    virtual void onPull() {}
    virtual void onReset() {}
    virtual void onFightTick(uint32_t /*diff*/) {}
    virtual void onTimedEvent(uint32_t /*eventId*/) {}

    void clearFight()
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mPhase = 0;
        mHeld = false;
        mBerserkLeftMs = 0;
    }

    AmirdrassilInstanceScript* getRaid()
    {
        if (getCreature()->GetMapId() != MAP_AMIRDRASSIL)
            return nullptr;

        return static_cast<AmirdrassilInstanceScript*>(getInstanceScript());
    }

    Creature* findRaidCreature(uint32_t entry)
    {
        AmirdrassilInstanceScript* pRaid = getRaid();
        return pRaid != nullptr ? pRaid->findLiving(entry) : nullptr;
    }

    // distinct random players, the tank is skipped while others are around
    std::vector<Unit*> rollPlayers(size_t count, bool avoidTank)
    {
        TargetFilter const filter = avoidTank ? TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent) : TargetFilter_Player;

        std::vector<Unit*> candidates;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject != nullptr && isValidUnitTarget(pObject, filter, FilterArgs(filter)))
                candidates.push_back(static_cast<Unit*>(pObject));
        }

        if (candidates.empty() && avoidTank)
            return rollPlayers(count, false);

        std::vector<Unit*> chosen;
        while (!candidates.empty() && chosen.size() < count)
        {
            size_t const pick = Util::getRandomUInt(static_cast<uint32_t>(candidates.size() - 1));
            chosen.push_back(candidates[pick]);
            candidates[pick] = candidates.back();
            candidates.pop_back();
        }

        return chosen;
    }

    Unit* rollPlayer(bool avoidTank)
    {
        std::vector<Unit*> const chosen = rollPlayers(1, avoidTank);
        return chosen.empty() ? nullptr : chosen.front();
    }

    Unit* currentVictim() { return getCreature()->getAIInterface()->getCurrentTarget(); }

    void castAtPlayer(uint32_t spellId, bool avoidTank = false)
    {
        if (Unit* pTarget = rollPlayer(avoidTank))
            castSpell(pTarget, spellId);
    }

    // several targets at once, so these casts are triggered
    void castAtPlayers(uint32_t spellId, size_t count, bool avoidTank = false)
    {
        for (Unit* pTarget : rollPlayers(count, avoidTank))
            castSpell(pTarget, spellId, true);
    }

    void castUnderPlayer(uint32_t spellId, bool avoidTank = false)
    {
        if (Unit* pTarget = rollPlayer(avoidTank))
            getCreature()->castSpellLoc(pTarget->GetPosition(), spellId, false);
    }

    void castUnderPlayers(uint32_t spellId, size_t count)
    {
        for (Unit* pTarget : rollPlayers(count, false))
            getCreature()->castSpellLoc(pTarget->GetPosition(), spellId, true);
    }

    void castUnderVictim(uint32_t spellId)
    {
        if (Unit* pVictim = currentVictim())
            getCreature()->castSpellLoc(pVictim->GetPosition(), spellId, false);
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t durationMs = 0)
    {
        Creature* pAdd = summonCreature(entry, spot, despawnType, durationMs);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector nearSelf(float minRange, float maxRange) { return amirdrassilScatter(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(std::string const& text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // a new stage drops everything still pending
    void beginPhase(uint32_t phase)
    {
        mPhase = phase;
        scriptEvents.resetEvents();
    }

    void armBerserk(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

    void setHeld(bool held) { mHeld = held; }

    uint32_t mPhase = 0;

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
        castSpellOnSelf(SPELL_AMIRDRASSIL_BERSERK, true);
    }

    bool mHeld = false;
    uint32_t mBerserkLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state, council pull and berserk

class AmirdrassilBossAI : public AmirdrassilFightAI
{
public:
    AmirdrassilBossAI(Creature* pCreature, uint32_t dataId, uint32_t berserkMs) : AmirdrassilFightAI(pCreature), mDataId(dataId), mBerserkMs(berserkMs) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (mDataId == DATA_COUNCIL_OF_DREAMS)
        {
            if (AmirdrassilInstanceScript* pRaid = getRaid())
                pRaid->wakeCouncil(this);
        }

        armBerserk(mBerserkMs);
        AmirdrassilFightAI::OnCombatStart(pTarget);
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

        AmirdrassilFightAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            AmirdrassilInstanceScript* pRaid = getRaid();
            if (pRaid == nullptr || pRaid->isEncounterCleared(mDataId, getCreature()))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        clearFight();
        onReset();
    }

protected:
    uint32_t mDataId;
    uint32_t mBerserkMs;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Adds driven by a spell table

class AmirdrassilAddAI : public AmirdrassilFightAI
{
public:
    AmirdrassilAddAI(Creature* pCreature, std::vector<AmirdrassilAddSpell> const& spells, uint32_t masterEntry = 0)
        : AmirdrassilFightAI(pCreature), mSpells(spells), mMasterEntry(masterEntry) {}

protected:
    void onPull() override
    {
        for (size_t i = 0; i < mSpells.size(); ++i)
            scriptEvents.addEvent(EVENT_AMIRDRASSIL_ADD_SPELL_BASE + static_cast<uint32_t>(i), mSpells[i].firstMs);
    }

    void onTimedEvent(uint32_t eventId) override
    {
        if (eventId < EVENT_AMIRDRASSIL_ADD_SPELL_BASE || eventId - EVENT_AMIRDRASSIL_ADD_SPELL_BASE >= mSpells.size())
            return;

        AmirdrassilAddSpell const& row = mSpells[eventId - EVENT_AMIRDRASSIL_ADD_SPELL_BASE];
        switch (row.castAt)
        {
            case CAST_AT_VICTIM:
                castSpellOnVictim(row.spellId);
                break;
            case CAST_AT_PLAYER:
                castAtPlayer(row.spellId);
                break;
            case CAST_AT_MASTER:
                if (Creature* pMaster = findRaidCreature(mMasterEntry))
                    castSpell(pMaster, row.spellId);
                break;
            default:
                castSpellAOE(row.spellId);
                break;
        }

        scriptEvents.addEvent(eventId, row.repeatMs);
    }

private:
    std::vector<AmirdrassilAddSpell> const& mSpells;
    uint32_t mMasterEntry;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gnarlroot

class AmirdrassilGnarlrootAI : public AmirdrassilBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilGnarlrootAI(c); }
    explicit AmirdrassilGnarlrootAI(Creature* pCreature) : AmirdrassilBossAI(pCreature, DATA_GNARLROOT, GnarlrootBerserkMs) {}

protected:
    void onPull() override
    {
        mCultivation.start(0, GnarlrootCultivationMs);
        tillTheGarden();
    }

    void onReset() override
    {
        mCultivation.stop();
        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onFightTick(uint32_t diff) override
    {
        if (mPhase != GnarlrootGardenOfDespair || !mCultivation.advance(diff) || _isCasting())
            return;

        // a full bar draws energy from the ground
        beginPhase(GnarlrootDoomCultivation);
        setRooted(true);
        _setMeleeDisabled(true);
        warn("Gnarlroot begins Doom Cultivation! Burn the Doom Roots!");
        castAtPlayers(SPELL_GNARLROOT_SEARING_BRAMBLE, 3);
        castSpellOnSelf(SPELL_GNARLROOT_DOOM_CULTIVATION);
        scriptEvents.addEvent(EVENT_GNARLROOT_UPROOTED, 20000);
    }

    void onTimedEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GNARLROOT_FLAMING_PESTILENCE:
                castSpellAOE(SPELL_GNARLROOT_FLAMING_PESTILENCE);
                for (uint8_t i = 0; i < 3; ++i)
                    callAdd(NPC_GNARLROOT_TAINTED_LASHER, nearSelf(10.0f, 25.0f));
                scriptEvents.addEvent(EVENT_GNARLROOT_FLAMING_PESTILENCE, 40000);
                break;
            case EVENT_GNARLROOT_SHADOWFLAME_CLEAVE:
                castSpellOnVictim(SPELL_GNARLROOT_SHADOWFLAME_CLEAVE);
                scriptEvents.addEvent(EVENT_GNARLROOT_SHADOWFLAME_CLEAVE, 15000);
                break;
            case EVENT_GNARLROOT_TORTURED_SCREAM:
                castSpellAOE(SPELL_GNARLROOT_TORTURED_SCREAM);
                scriptEvents.addEvent(EVENT_GNARLROOT_TORTURED_SCREAM, 25000);
                break;
            case EVENT_GNARLROOT_CONTROLLED_BURN:
                castAtPlayers(SPELL_GNARLROOT_CONTROLLED_BURN, 2);
                scriptEvents.addEvent(EVENT_GNARLROOT_CONTROLLED_BURN, 35000);
                break;
            case EVENT_GNARLROOT_DREADFIRE_BARRAGE:
                castSpellOnVictim(SPELL_GNARLROOT_DREADFIRE_BARRAGE);
                scriptEvents.addEvent(EVENT_GNARLROOT_DREADFIRE_BARRAGE, 30000);
                break;
            case EVENT_GNARLROOT_UPROOTED:
                beginPhase(GnarlrootUprooted);
                warn("Gnarlroot is Uprooted and writhes in agony!");
                castSpellOnSelf(SPELL_GNARLROOT_UPROOTED_AGONY, true);
                scriptEvents.addEvent(EVENT_GNARLROOT_RECOVERED, 20000);
                break;
            case EVENT_GNARLROOT_RECOVERED:
                castSpellOnSelf(SPELL_GNARLROOT_RISING_MANIA, true);
                setRooted(false);
                _setMeleeDisabled(false);
                tillTheGarden();
                break;
            default:
                break;
        }
    }

private:
    void tillTheGarden()
    {
        beginPhase(GnarlrootGardenOfDespair);
        mCultivation.empty();
        scriptEvents.addEvent(EVENT_GNARLROOT_FLAMING_PESTILENCE, 6000);
        scriptEvents.addEvent(EVENT_GNARLROOT_SHADOWFLAME_CLEAVE, 8000);
        scriptEvents.addEvent(EVENT_GNARLROOT_TORTURED_SCREAM, 12000);
        scriptEvents.addEvent(EVENT_GNARLROOT_CONTROLLED_BURN, 16000);
        scriptEvents.addEvent(EVENT_GNARLROOT_DREADFIRE_BARRAGE, 20000);
    }

    AmirdrassilEnergy mCultivation;
};

class AmirdrassilTaintedLasherAI : public AmirdrassilAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilTaintedLasherAI(c); }
    explicit AmirdrassilTaintedLasherAI(Creature* pCreature) : AmirdrassilAddAI(pCreature, TaintedLasherSpells) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Igira the Cruel

class AmirdrassilIgiraAI : public AmirdrassilBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilIgiraAI(c); }
    explicit AmirdrassilIgiraAI(Creature* pCreature) : AmirdrassilBossAI(pCreature, DATA_IGIRA, IgiraBerserkMs) {}

protected:
    void onPull() override
    {
        mWeapon = IgiraUnarmed;
        mTorment.start(0, IgiraTormentMs);
        castSpellOnSelf(SPELL_IGIRA_VICIOUS_SWING, true);
        scriptEvents.addEvent(EVENT_IGIRA_BLISTERING_SPEAR, 15000);
        scriptEvents.addEvent(EVENT_IGIRA_TWISTING_BLADE, 22000);
    }

    void onReset() override
    {
        mWeapon = IgiraUnarmed;
        mTorment.stop();
    }

    void onFightTick(uint32_t diff) override
    {
        if (!mTorment.advance(diff) || _isCasting())
            return;

        mTorment.empty();
        warn("Igira marks her chamber for torment!");
        castAtPlayers(SPELL_IGIRA_GATHERING_TORMENT, 3);
        castSpellAOE(SPELL_IGIRA_MARKED_FOR_TORMENT);
        scriptEvents.addEvent(EVENT_IGIRA_SEARING_SPARKS, 6000);
        scriptEvents.addEvent(EVENT_IGIRA_FORGE, 12000);
    }

    void onTimedEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_IGIRA_BLISTERING_SPEAR:
                castAtPlayers(SPELL_IGIRA_BLISTERING_SPEAR, 3, true);
                scriptEvents.addEvent(EVENT_IGIRA_BLISTERING_SPEAR, 40000);
                break;
            case EVENT_IGIRA_TWISTING_BLADE:
                castAtPlayer(SPELL_IGIRA_TWISTING_BLADE, true);
                scriptEvents.addEvent(EVENT_IGIRA_TWISTING_BLADE, 30000);
                break;
            case EVENT_IGIRA_SEARING_SPARKS:
                castSpellAOE(SPELL_IGIRA_SEARING_SPARKS);
                break;
            case EVENT_IGIRA_FORGE:
                forgeWeapon();
                break;
            case EVENT_IGIRA_HACKING_TORMENT:
                castSpellAOE(SPELL_IGIRA_UMBRAL_DESTRUCTION);
                scriptEvents.addEvent(EVENT_IGIRA_HACKING_TORMENT, 25000);
                break;
            case EVENT_IGIRA_SLICING_TORMENT:
                if (isHeroic())
                    castSpellAOE(SPELL_IGIRA_DEVASTATION, true);
                castAtPlayer(SPELL_IGIRA_SMASHING_VISCERA);
                scriptEvents.addEvent(EVENT_IGIRA_SLICING_TORMENT, 20000);
                break;
            case EVENT_IGIRA_FLAYING_TORMENT:
                castAtPlayers(SPELL_IGIRA_HEART_STOPPER, 3);
                scriptEvents.addEvent(EVENT_IGIRA_FLAYING_TORMENT, 30000);
                break;
            default:
                break;
        }
    }

private:
    // each torment forges a different weapon than the one in hand
    void forgeWeapon()
    {
        scriptEvents.removeEvent(EVENT_IGIRA_HACKING_TORMENT);
        scriptEvents.removeEvent(EVENT_IGIRA_SLICING_TORMENT);
        scriptEvents.removeEvent(EVENT_IGIRA_FLAYING_TORMENT);

        std::vector<uint32_t> forges;
        for (uint32_t weapon = IgiraHackingTorment; weapon <= IgiraFlayingTorment; ++weapon)
        {
            if (weapon != mWeapon)
                forges.push_back(weapon);
        }

        mWeapon = forges[Util::getRandomUInt(static_cast<uint32_t>(forges.size() - 1))];
        castSpellOnSelf(SPELL_IGIRA_HARVEST_OF_SCREAMS, true);

        switch (mWeapon)
        {
            case IgiraHackingTorment:
                warn("Igira forges an axe of Hacking Torment!");
                scriptEvents.addEvent(EVENT_IGIRA_HACKING_TORMENT, 8000);
                break;
            case IgiraSlicingTorment:
                warn("Igira forges a sword of Slicing Torment!");
                scriptEvents.addEvent(EVENT_IGIRA_SLICING_TORMENT, 8000);
                break;
            default:
                warn("Igira forges a knife of Flaying Torment!");
                scriptEvents.addEvent(EVENT_IGIRA_FLAYING_TORMENT, 8000);
                break;
        }
    }

    AmirdrassilEnergy mTorment;
    uint32_t mWeapon = IgiraUnarmed;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Volcoross

class AmirdrassilVolcorossAI : public AmirdrassilBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilVolcorossAI(c); }
    explicit AmirdrassilVolcorossAI(Creature* pCreature) : AmirdrassilBossAI(pCreature, DATA_VOLCOROSS, VolcorossBerserkMs) {}

protected:
    // fights from his lava pool
    void onPull() override
    {
        setRooted(true);
        mFlood.start(0, VolcorossFloodMs);
        scriptEvents.addEvent(EVENT_VOLCOROSS_CATACLYSM_JAWS, 8000);
        scriptEvents.addEvent(EVENT_VOLCOROSS_SCORCHTAIL_CRASH, 14000);
        scriptEvents.addEvent(EVENT_VOLCOROSS_SERPENTS_FURY, 20000);
        scriptEvents.addEvent(EVENT_VOLCOROSS_VOLCANIC_DISGORGE, 25000);
    }

    void onReset() override
    {
        mFlood.stop();
        setRooted(false);
    }

    void onFightTick(uint32_t diff) override
    {
        if (!mFlood.advance(diff) || _isCasting())
            return;

        mFlood.empty();
        warn("Volcoross unleashes a Flood of the Firelands!");
        castSpellAOE(SPELL_VOLCOROSS_FLOOD_OF_THE_FIRELANDS);
    }

    void onTimedEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VOLCOROSS_CATACLYSM_JAWS:
                castSpellOnVictim(SPELL_VOLCOROSS_CATACLYSM_JAWS);
                scriptEvents.addEvent(EVENT_VOLCOROSS_CATACLYSM_JAWS, 18000);
                break;
            case EVENT_VOLCOROSS_SCORCHTAIL_CRASH:
                warn("Volcoross raises his tail for a Scorchtail Crash!");
                castUnderPlayer(SPELL_VOLCOROSS_SCORCHTAIL_CRASH);
                scriptEvents.addEvent(EVENT_VOLCOROSS_SCORCHTAIL_CRASH, 25000);
                break;
            case EVENT_VOLCOROSS_SERPENTS_FURY:
                castAtPlayers(SPELL_VOLCOROSS_COILING_FLAMES, 2);
                castSpellAOE(SPELL_VOLCOROSS_SERPENTS_FURY);
                scriptEvents.addEvent(EVENT_VOLCOROSS_SERPENTS_FURY, 35000);
                break;
            case EVENT_VOLCOROSS_VOLCANIC_DISGORGE:
                castUnderPlayer(SPELL_VOLCOROSS_VOLCANIC_DISGORGE);
                scriptEvents.addEvent(EVENT_VOLCOROSS_VOLCANIC_DISGORGE, 30000);
                break;
            default:
                break;
        }
    }

private:
    AmirdrassilEnergy mFlood;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Council of Dreams

// a fallen dreamer rises again unless the whole council falls together
class AmirdrassilDreamerAI : public AmirdrassilBossAI
{
public:
    AmirdrassilDreamerAI(Creature* pCreature, char const* name, uint32_t startEnergyPct)
        : AmirdrassilBossAI(pCreature, DATA_COUNCIL_OF_DREAMS, CouncilBerserkMs), mName(name), mStartEnergyPct(startEnergyPct) {}

    uint32_t GetCreatureData(uint32_t type) const override { return type == DreamerFadedQuery && mFaded ? 1 : 0; }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_DREAMER_LET_GO || !mFaded || mLetGo || !getCreature()->isAlive())
            return;

        mLetGo = true;
        getCreature()->dealDamage(getCreature(), getCreature()->getHealth(), 0);
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mLetGo)
            return;

        if (mFaded)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health || !hasStandingPartner())
            return;

        *damage = health > 1 ? health - 1 : 0;
        fade();
    }

    void OnDied(Unit* pKiller) override
    {
        AmirdrassilBossAI::OnDied(pKiller);

        // the last dreamer standing takes the fading ones along
        AmirdrassilInstanceScript* pRaid = getRaid();
        if (pRaid == nullptr)
            return;

        for (Creature* pDreamer : pRaid->getCouncil())
        {
            if (pDreamer != getCreature() && pDreamer->isAlive() && pDreamer->GetScript() != nullptr)
                pDreamer->GetScript()->DoAction(ACTION_DREAMER_LET_GO);
        }
    }

protected:
    void onPull() override
    {
        mFaded = false;
        mLetGo = false;
        mUltimate.start(mStartEnergyPct, CouncilUltimateMs);
        openFight();
    }

    void onReset() override
    {
        mUltimate.stop();
        if (mFaded)
            wake();

        mFaded = false;
        mLetGo = false;
    }

    void onFightTick(uint32_t diff) override
    {
        if (mFaded)
        {
            if (mLetGo)
                return;

            if (mRebirthLeftMs > diff)
            {
                mRebirthLeftMs -= diff;
                return;
            }

            rebirth();
            return;
        }

        if (!mUltimate.advance(diff) || _isCasting())
            return;

        mUltimate.empty();
        unleashUltimate();
    }

    virtual void openFight() = 0;
    virtual void unleashUltimate() = 0;

private:
    bool hasStandingPartner()
    {
        AmirdrassilInstanceScript* pRaid = getRaid();
        if (pRaid == nullptr)
            return false;

        for (Creature* pDreamer : pRaid->getCouncil())
        {
            if (pDreamer == getCreature() || !pDreamer->isAlive() || pDreamer->GetScript() == nullptr)
                continue;

            if (pDreamer->GetScript()->GetCreatureData(DreamerFadedQuery) == 0)
                return true;
        }

        return false;
    }

    void fade()
    {
        mFaded = true;
        mRebirthLeftMs = DreamerRebirthMs;
        getCreature()->interruptSpell();
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        _setMeleeDisabled(true);
        setHeld(true);
        warn(std::string(mName) + " fades into the Dream!");
    }

    void wake()
    {
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        _setMeleeDisabled(false);
        setHeld(false);
    }

    void rebirth()
    {
        mFaded = false;
        wake();
        getCreature()->setHealth(std::max<uint32_t>(1, getCreature()->getMaxHealth() / 100 * DreamerRebirthHealthPct));
        castSpellOnSelf(SPELL_COUNCIL_REBIRTH, true);
        warn(std::string(mName) + " is reborn from the Dream!");
    }

    char const* mName;
    uint32_t mStartEnergyPct;
    AmirdrassilEnergy mUltimate;
    bool mFaded = false;
    bool mLetGo = false;
    uint32_t mRebirthLeftMs = 0;
};

class AmirdrassilUrctosAI : public AmirdrassilDreamerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilUrctosAI(c); }
    explicit AmirdrassilUrctosAI(Creature* pCreature) : AmirdrassilDreamerAI(pCreature, "Urctos", 0) {}

protected:
    void openFight() override
    {
        scriptEvents.addEvent(EVENT_URCTOS_AGONIZING_CLAWS, 6000);
        scriptEvents.addEvent(EVENT_URCTOS_BARRELING_CHARGE, 18000);
    }

    void unleashUltimate() override
    {
        warn("Urctos flies into a Blind Rage!");
        castSpellOnSelf(SPELL_URCTOS_BLIND_RAGE);
    }

    void onTimedEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_URCTOS_AGONIZING_CLAWS:
                castSpellOnVictim(SPELL_URCTOS_AGONIZING_CLAWS);
                scriptEvents.addEvent(EVENT_URCTOS_AGONIZING_CLAWS, 10000);
                break;
            case EVENT_URCTOS_BARRELING_CHARGE:
                warn("Urctos prepares a Barreling Charge!");
                castAtPlayer(SPELL_URCTOS_BARRELING_CHARGE, true);
                scriptEvents.addEvent(EVENT_URCTOS_BARRELING_CHARGE, 30000);
                break;
            default:
                break;
        }
    }
};

class AmirdrassilAerwynnAI : public AmirdrassilDreamerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilAerwynnAI(c); }
    explicit AmirdrassilAerwynnAI(Creature* pCreature) : AmirdrassilDreamerAI(pCreature, "Aerwynn", 33) {}

protected:
    void openFight() override
    {
        scriptEvents.addEvent(EVENT_AERWYNN_POISONOUS_JAVELIN, 10000);
        scriptEvents.addEvent(EVENT_AERWYNN_NOXIOUS_BLOSSOM, 14000);
    }

    void unleashUltimate() override
    {
        warn("Aerwynn calls forth a Constricting Thicket!");
        castSpellAOE(SPELL_AERWYNN_CONSTRICTING_THICKET);
    }

    void onTimedEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AERWYNN_POISONOUS_JAVELIN:
                castAtPlayer(SPELL_AERWYNN_POISONOUS_JAVELIN, true);
                scriptEvents.addEvent(EVENT_AERWYNN_POISONOUS_JAVELIN, 25000);
                break;
            case EVENT_AERWYNN_NOXIOUS_BLOSSOM:
                castUnderPlayer(SPELL_AERWYNN_NOXIOUS_BLOSSOM);
                scriptEvents.addEvent(EVENT_AERWYNN_NOXIOUS_BLOSSOM, 20000);
                break;
            default:
                break;
        }
    }
};

class AmirdrassilPipAI : public AmirdrassilDreamerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilPipAI(c); }
    explicit AmirdrassilPipAI(Creature* pCreature) : AmirdrassilDreamerAI(pCreature, "Pip", 66) {}

protected:
    void openFight() override
    {
        scriptEvents.addEvent(EVENT_PIP_POLYMORPH_BOMB, 12000);
        scriptEvents.addEvent(EVENT_PIP_EMERALD_WINDS, 25000);
    }

    void unleashUltimate() override
    {
        warn("Pip begins the Song of the Dragon!");
        castSpellAOE(SPELL_PIP_SONG_OF_THE_DRAGON);
    }

    void onTimedEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_PIP_POLYMORPH_BOMB:
                castAtPlayers(SPELL_PIP_POLYMORPH_BOMB, 2, true);
                scriptEvents.addEvent(EVENT_PIP_POLYMORPH_BOMB, 30000);
                break;
            case EVENT_PIP_EMERALD_WINDS:
                castSpellAOE(SPELL_PIP_EMERALD_WINDS);
                scriptEvents.addEvent(EVENT_PIP_EMERALD_WINDS, 40000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Larodar, Keeper of the Flame

class AmirdrassilLarodarAI : public AmirdrassilBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilLarodarAI(c); }
    explicit AmirdrassilLarodarAI(Creature* pCreature) : AmirdrassilBossAI(pCreature, DATA_LARODAR, LarodarBerserkMs) {}

    // slain treants and roots leave charred remains behind
    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || !_isInCombat())
            return;

        if (pSummon->getEntry() == NPC_LARODAR_FIERY_TREANT)
            leaveRemains(NPC_LARODAR_CHARRED_TREANT, pSummon->GetPosition());
        else if (pSummon->getEntry() == NPC_LARODAR_SCORCHING_ROOTS)
            leaveRemains(NPC_LARODAR_CHARRED_BRAMBLES, pSummon->GetPosition());
    }

protected:
    void onPull() override
    {
        mInfernos = 0;
        mInferno.start(0, LarodarInfernoMs);
        castSpellOnSelf(SPELL_LARODAR_COMBUSTING_PRESENCE, true);
        scriptEvents.addEvent(EVENT_LARODAR_FIERY_FORCE, 8000);
        scriptEvents.addEvent(EVENT_LARODAR_FURIOUS_CHARGE, 14000);
        scriptEvents.addEvent(EVENT_LARODAR_SCORCHING_ROOTS, 20000);
        scriptEvents.addEvent(EVENT_LARODAR_BLAZING_THORNS, 25000);
    }

    void onReset() override
    {
        mInferno.stop();
        setRooted(false);
    }

    void onFightTick(uint32_t diff) override
    {
        if (mPhase != LarodarCycleOfFlame || !mInferno.advance(diff) || _isCasting())
            return;

        mInferno.empty();
        warn("Larodar envelops the arena in a Raging Inferno!");
        castSpellAOE(SPELL_LARODAR_RAGING_INFERNO);

        // the third inferno consumes him
        if (++mInfernos >= LarodarInfernosToAsh)
        {
            mInferno.stop();
            scriptEvents.addEvent(EVENT_LARODAR_CONSUMING_FLAME, 5000);
        }
    }

    void onTimedEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LARODAR_FIERY_FORCE:
                castSpellAOE(SPELL_LARODAR_FIERY_FORCE_OF_NATURE);
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_LARODAR_FIERY_TREANT, nearSelf(10.0f, 20.0f));
                scriptEvents.addEvent(EVENT_LARODAR_FIERY_FORCE, 45000);
                break;
            case EVENT_LARODAR_SCORCHING_ROOTS:
                castSpellAOE(SPELL_LARODAR_SCORCHING_ROOTS);
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_LARODAR_SCORCHING_ROOTS, nearSelf(8.0f, 18.0f));
                scriptEvents.addEvent(EVENT_LARODAR_SCORCHING_ROOTS, 40000);
                break;
            case EVENT_LARODAR_FURIOUS_CHARGE:
                castAtPlayer(SPELL_LARODAR_FURIOUS_CHARGE, true);
                if (isHeroic())
                    scriptEvents.addEvent(EVENT_LARODAR_FURIOUS_OUTBURST, 2000);
                scriptEvents.addEvent(EVENT_LARODAR_FURIOUS_CHARGE, 30000);
                break;
            case EVENT_LARODAR_FURIOUS_OUTBURST:
                castSpellAOE(SPELL_LARODAR_FURIOUS_OUTBURST, true);
                break;
            case EVENT_LARODAR_BLAZING_THORNS:
                castSpellAOE(SPELL_LARODAR_BLAZING_THORNS);
                scriptEvents.addEvent(EVENT_LARODAR_BLAZING_THORNS, 35000);
                break;
            case EVENT_LARODAR_CONSUMING_FLAME:
                beginPhase(LarodarUnreborn);
                setRooted(true);
                warn("Larodar is engulfed by a Consuming Flame!");
                castSpellOnSelf(SPELL_LARODAR_CONSUMING_FLAME);
                scriptEvents.addEvent(EVENT_LARODAR_AVATAR_OF_ASH, 15000);
                break;
            case EVENT_LARODAR_AVATAR_OF_ASH:
                beginPhase(LarodarAvatarOfAsh);
                setRooted(false);
                castSpellOnSelf(SPELL_LARODAR_SEARING_ASH, true);
                scriptEvents.addEvent(EVENT_LARODAR_FALLING_EMBERS, 8000);
                scriptEvents.addEvent(EVENT_LARODAR_SMOLDERING_BACKDRAFT, 12000);
                scriptEvents.addEvent(EVENT_LARODAR_FLASH_FIRE, 15000);
                scriptEvents.addEvent(EVENT_LARODAR_ASHEN_CALL, 18000);
                scriptEvents.addEvent(EVENT_LARODAR_FIRE_WHIRL, 22000);
                break;
            case EVENT_LARODAR_FALLING_EMBERS:
                castSpellAOE(SPELL_LARODAR_FALLING_EMBERS);
                scriptEvents.addEvent(EVENT_LARODAR_FALLING_EMBERS, 25000);
                break;
            case EVENT_LARODAR_FLASH_FIRE:
                castAtPlayers(SPELL_LARODAR_FLASH_FIRE, 2);
                scriptEvents.addEvent(EVENT_LARODAR_FLASH_FIRE, 30000);
                break;
            case EVENT_LARODAR_FIRE_WHIRL:
                castSpellAOE(SPELL_LARODAR_FIRE_WHIRL);
                scriptEvents.addEvent(EVENT_LARODAR_FIRE_WHIRL, 35000);
                break;
            case EVENT_LARODAR_SMOLDERING_BACKDRAFT:
                castSpellOnVictim(SPELL_LARODAR_SMOLDERING_BACKDRAFT);
                scriptEvents.addEvent(EVENT_LARODAR_SMOLDERING_BACKDRAFT, 20000);
                break;
            case EVENT_LARODAR_ASHEN_CALL:
                warn("Larodar calls forth Ashen Treants!");
                castSpellAOE(SPELL_LARODAR_ASHEN_CALL);
                for (uint8_t i = 0; i < 3; ++i)
                    callAdd(NPC_LARODAR_ASHEN_TREANT, nearSelf(10.0f, 20.0f));
                scriptEvents.addEvent(EVENT_LARODAR_ASHEN_CALL, 40000);
                break;
            default:
                break;
        }
    }

private:
    // remains are inert and start badly burnt
    void leaveRemains(uint32_t entry, LocationVector const& spot)
    {
        if (Creature* pRemains = summonCreature(entry, spot, CORPSE_DESPAWN))
            pRemains->setHealth(std::max<uint32_t>(1, pRemains->getMaxHealth() / 10));
    }

    AmirdrassilEnergy mInferno;
    uint32_t mInfernos = 0;
};

class AmirdrassilFieryTreantAI : public AmirdrassilAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilFieryTreantAI(c); }
    explicit AmirdrassilFieryTreantAI(Creature* pCreature) : AmirdrassilAddAI(pCreature, FieryTreantSpells) {}
};

// fixates one player
class AmirdrassilScorchingRootsAI : public AmirdrassilAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilScorchingRootsAI(c); }
    explicit AmirdrassilScorchingRootsAI(Creature* pCreature) : AmirdrassilAddAI(pCreature, ScorchingRootsSpells) {}

protected:
    void onPull() override
    {
        AmirdrassilAddAI::onPull();
        if (Unit* pPrey = rollPlayer(false))
            addThreat(pPrey, 1000000.0f);
    }
};

// erupts once when felled
class AmirdrassilAshenTreantAI : public AmirdrassilAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilAshenTreantAI(c); }
    explicit AmirdrassilAshenTreantAI(Creature* pCreature) : AmirdrassilAddAI(pCreature, NoAddSpells) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mErupted || *damage < getCreature()->getHealth())
            return;

        mErupted = true;
        castSpellAOE(SPELL_TREANT_EXPLOSIVE_BARK, true);
        castSpellAOE(SPELL_ASHEN_TREANT_BURNING_GROUND, true);
    }

private:
    bool mErupted = false;
};

// charred remains wait for healing, at full health they are renewed
class AmirdrassilCharredRemainsAI : public CreatureAIScript
{
public:
    static CreatureAIScript* CreateTreant(Creature* c) { return new AmirdrassilCharredRemainsAI(c, NPC_LARODAR_RENEWED_TREANT, TIMED_DESPAWN, RenewedTreantLifeMs); }
    static CreatureAIScript* CreateBrambles(Creature* c) { return new AmirdrassilCharredRemainsAI(c, NPC_LARODAR_RENEWED_BRAMBLE_BARRIER, CORPSE_DESPAWN, 0); }

    AmirdrassilCharredRemainsAI(Creature* pCreature, uint32_t renewedEntry, CreatureSummonDespawnType renewedDespawn, uint32_t renewedLifeMs)
        : CreatureAIScript(pCreature), mRenewedEntry(renewedEntry), mRenewedDespawn(renewedDespawn), mRenewedLifeMs(renewedLifeMs) {}

    void AIUpdate(unsigned long /*time_passed*/) override
    {
        Creature* pSelf = getCreature();
        if (mRenewed || !pSelf->isAlive())
            return;

        if (!mSettled)
        {
            mSettled = true;
            pSelf->m_canRegenerateHP = false;
            setRooted(true);
            _setMeleeDisabled(true);
            return;
        }

        if (pSelf->getHealth() < pSelf->getMaxHealth())
            return;

        mRenewed = true;

        // Larodar owns the renewed form so it leaves with his fight
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance != nullptr && pSelf->GetMapId() == MAP_AMIRDRASSIL)
        {
            if (Creature* pLarodar = static_cast<AmirdrassilInstanceScript*>(pInstance)->findLiving(BOSS_LARODAR))
                pLarodar->summonCreature(mRenewedEntry, pSelf->GetPosition(), mRenewedDespawn, mRenewedLifeMs);
        }

        despawn(500, 0);
    }

private:
    uint32_t mRenewedEntry;
    CreatureSummonDespawnType mRenewedDespawn;
    uint32_t mRenewedLifeMs;
    bool mSettled = false;
    bool mRenewed = false;
};

// gives its life to the Seed of Life
class AmirdrassilRenewedTreantAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilRenewedTreantAI(c); }
    explicit AmirdrassilRenewedTreantAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void AIUpdate(unsigned long /*time_passed*/) override
    {
        if (mGifted || !getCreature()->isAlive())
            return;

        mGifted = true;
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellAOE(SPELL_TREANT_NATURES_GIFT);
    }

private:
    bool mGifted = false;
};

// shelters players from Raging Inferno, bursts when destroyed
class AmirdrassilBrambleBarrierAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilBrambleBarrierAI(c); }
    explicit AmirdrassilBrambleBarrierAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void AIUpdate(unsigned long /*time_passed*/) override
    {
        if (mRaised || !getCreature()->isAlive())
            return;

        mRaised = true;
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_BRAMBLE_NATURES_BULWARK, true);
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mBurst || *damage < getCreature()->getHealth())
            return;

        mBurst = true;
        castSpellAOE(SPELL_TREANT_EXPLOSIVE_BARK, true);
    }

private:
    bool mRaised = false;
    bool mBurst = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nymue, Weaver of the Cycle

class AmirdrassilNymueAI : public AmirdrassilBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilNymueAI(c); }
    explicit AmirdrassilNymueAI(Creature* pCreature) : AmirdrassilBossAI(pCreature, DATA_NYMUE, NymueBerserkMs) {}

    // the cycle resumes once her wardens are gone
    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || pSummon->getEntry() != NPC_NYMUE_CYCLE_WARDEN || mWardensLeft == 0)
            return;

        if (--mWardensLeft == 0 && _isInCombat() && mPhase == NymueCreationComplete)
            weaveTheCycle();
    }

protected:
    void onPull() override
    {
        mWardensLeft = 0;
        mBloom.start(0, NymueBloomMs);
        castSpellOnSelf(SPELL_NYMUE_VERDANT_MATRIX, true);
        weaveTheCycle();
    }

    void onReset() override
    {
        mWardensLeft = 0;
        mBloom.stop();
    }

    void onFightTick(uint32_t diff) override
    {
        if (mPhase != NymueRapidIteration || !mBloom.advance(diff) || _isCasting())
            return;

        beginPhase(NymueCreationComplete);
        warn("Nymue reaches Full Bloom! Cycle Wardens awaken!");
        castSpellAOE(SPELL_NYMUE_FULL_BLOOM);

        mWardensLeft = 0;
        for (uint8_t i = 0; i < 2; ++i)
        {
            if (callAdd(NPC_NYMUE_CYCLE_WARDEN, nearSelf(12.0f, 20.0f)) != nullptr)
                ++mWardensLeft;
        }

        scriptEvents.addEvent(EVENT_NYMUE_UNRAVEL, 6000);
        scriptEvents.addEvent(EVENT_NYMUE_BLOOM_FADES, 60000);
    }

    void onTimedEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NYMUE_THREADED_BLAST:
                castSpellOnVictim(SPELL_NYMUE_THREADED_BLAST);
                scriptEvents.addEvent(EVENT_NYMUE_THREADED_BLAST, 20000);
                break;
            case EVENT_NYMUE_VIRIDIAN_RAIN:
                castSpellAOE(SPELL_NYMUE_VIRIDIAN_RAIN);
                scriptEvents.addEvent(EVENT_NYMUE_VIRIDIAN_RAIN, 25000);
                break;
            case EVENT_NYMUE_WEAVERS_BURDEN:
                castSpellOnVictim(SPELL_NYMUE_WEAVERS_BURDEN);
                scriptEvents.addEvent(EVENT_NYMUE_WEAVERS_BURDEN, 30000);
                break;
            case EVENT_NYMUE_IMPENDING_LOOM:
                warn("Nymue weaves an Impending Loom!");
                castSpellAOE(SPELL_NYMUE_IMPENDING_LOOM);
                scriptEvents.addEvent(EVENT_NYMUE_IMPENDING_LOOM, 35000);
                break;
            case EVENT_NYMUE_CONTINUUM:
                castSpellOnSelf(SPELL_NYMUE_VERDANT_MATRIX, true);
                castSpellAOE(SPELL_NYMUE_CONTINUUM);
                scriptEvents.addEvent(EVENT_NYMUE_CONTINUUM, 60000);
                break;
            case EVENT_NYMUE_UNRAVEL:
                castSpellAOE(SPELL_NYMUE_UNRAVEL);
                scriptEvents.addEvent(EVENT_NYMUE_UNRAVEL, 10000);
                break;
            case EVENT_NYMUE_BLOOM_FADES:
                weaveTheCycle();
                break;
            default:
                break;
        }
    }

private:
    void weaveTheCycle()
    {
        beginPhase(NymueRapidIteration);
        mBloom.empty();
        mWardensLeft = 0;
        scriptEvents.addEvent(EVENT_NYMUE_THREADED_BLAST, 10000);
        scriptEvents.addEvent(EVENT_NYMUE_VIRIDIAN_RAIN, 15000);
        scriptEvents.addEvent(EVENT_NYMUE_WEAVERS_BURDEN, 20000);
        scriptEvents.addEvent(EVENT_NYMUE_IMPENDING_LOOM, 25000);
        scriptEvents.addEvent(EVENT_NYMUE_CONTINUUM, 45000);
    }

    AmirdrassilEnergy mBloom;
    uint32_t mWardensLeft = 0;
};

// shields Nymue with Life Ward
class AmirdrassilCycleWardenAI : public AmirdrassilAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilCycleWardenAI(c); }
    explicit AmirdrassilCycleWardenAI(Creature* pCreature) : AmirdrassilAddAI(pCreature, CycleWardenSpells, BOSS_NYMUE) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Smolderon

class AmirdrassilSmolderonAI : public AmirdrassilBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilSmolderonAI(c); }
    explicit AmirdrassilSmolderonAI(Creature* pCreature) : AmirdrassilBossAI(pCreature, DATA_SMOLDERON, SmolderonBerserkMs) {}

protected:
    void onPull() override
    {
        mWorldInFlames.start(0, SmolderonFlamesMs);
        castSpellOnSelf(SPELL_SMOLDERON_BLISTERING_HEAT, true);
        firelordsFury();
    }

    void onReset() override
    {
        mWorldInFlames.stop();
        setRooted(false);
    }

    void onFightTick(uint32_t diff) override
    {
        if (mPhase != SmolderonFirelordsFury || !mWorldInFlames.advance(diff) || _isCasting())
            return;

        beginPhase(SmolderonWorldInFlames);
        setRooted(true);
        warn("Smolderon slams Emberscar into the ground! The world is in flames!");
        castSpellOnSelf(SPELL_SMOLDERON_HEATING_UP, true);
        castSpellOnSelf(SPELL_SMOLDERON_BLAZING_SOUL, true);

        // essence torn out of the marked drifts back to him
        castSpellAOE(SPELL_SMOLDERON_DEVOUR_ESSENCE, true);
        for (Unit* pMarked : rollPlayers(4, false))
            summonCreature(NPC_SMOLDERON_LIVING_FLAME, pMarked->GetPosition(), TIMED_DESPAWN, LivingFlameLifeMs);

        castSpellAOE(SPELL_SMOLDERON_WORLD_IN_FLAMES);
        scriptEvents.addEvent(EVENT_SMOLDERON_FLAMES_SUBSIDE, 25000);
    }

    void onTimedEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SMOLDERON_LAVA_GEYSERS:
                castUnderPlayers(SPELL_SMOLDERON_LAVA_GEYSERS, 3);
                scriptEvents.addEvent(EVENT_SMOLDERON_LAVA_GEYSERS, 20000);
                break;
            case EVENT_SMOLDERON_BRAND_OF_DAMNATION:
                warn("Smolderon raises Emberscar! Share the Brand of Damnation!");
                if (isHeroic())
                {
                    if (Unit* pVictim = currentVictim())
                        castSpell(pVictim, SPELL_SMOLDERON_SEARING_AFTERMATH, true);
                }
                castSpellOnVictim(SPELL_SMOLDERON_BRAND_OF_DAMNATION);
                scriptEvents.addEvent(EVENT_SMOLDERON_BRAND_OF_DAMNATION, 30000);
                break;
            case EVENT_SMOLDERON_OVERHEATED:
                castAtPlayers(SPELL_SMOLDERON_OVERHEATED, 2);
                scriptEvents.addEvent(EVENT_SMOLDERON_OVERHEATED, 25000);
                break;
            case EVENT_SMOLDERON_FLAMES_SUBSIDE:
                setRooted(false);
                firelordsFury();
                break;
            default:
                break;
        }
    }

private:
    void firelordsFury()
    {
        beginPhase(SmolderonFirelordsFury);
        mWorldInFlames.empty();
        scriptEvents.addEvent(EVENT_SMOLDERON_LAVA_GEYSERS, 8000);
        scriptEvents.addEvent(EVENT_SMOLDERON_BRAND_OF_DAMNATION, 12000);
        scriptEvents.addEvent(EVENT_SMOLDERON_OVERHEATED, 18000);
    }

    AmirdrassilEnergy mWorldInFlames;
};

// drifts to Smolderon and feeds him unless destroyed on the way
class AmirdrassilLivingFlameAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilLivingFlameAI(c); }
    explicit AmirdrassilLivingFlameAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (mSpent || !getCreature()->isAlive())
            return;

        mSteerMs += static_cast<uint32_t>(time_passed);
        if (mSteerMs < 1000)
            return;

        mSteerMs = 0;

        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr || getCreature()->GetMapId() != MAP_AMIRDRASSIL)
            return;

        Creature* pFirelord = static_cast<AmirdrassilInstanceScript*>(pInstance)->findLiving(BOSS_SMOLDERON);
        if (pFirelord == nullptr)
            return;

        if (getCreature()->getDistance(pFirelord) <= LivingFlameReach)
        {
            mSpent = true;
            castSpell(pFirelord, SPELL_LIVING_FLAME_IGNITED_ESSENCE, true);
            despawn(500, 0);
            return;
        }

        if (!_isInCombat())
            moveTo(pFirelord->GetPositionX(), pFirelord->GetPositionY(), pFirelord->GetPositionZ(), false);
    }

private:
    uint32_t mSteerMs = 0;
    bool mSpent = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Tindral Sageswift, Seer of the Flame

class AmirdrassilTindralAI : public AmirdrassilBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilTindralAI(c); }
    explicit AmirdrassilTindralAI(Creature* pCreature) : AmirdrassilBossAI(pCreature, DATA_TINDRAL, TindralBerserkMs) {}

protected:
    void onPull() override
    {
        castSpellOnSelf(SPELL_TINDRAL_SEARING_WRATH, true);
        scheduleStage(TindralMoonkin);
    }

    void onReset() override { setRooted(false); }

    void onFightTick(uint32_t /*diff*/) override
    {
        int32_t const health = _getHealthPercent();
        if (mPhase == TindralMoonkin && health <= TindralFirstPursuitPct)
            startPursuit(TindralBurningPursuit);
        else if (mPhase == TindralTreeOfTheFlame && health <= TindralSecondPursuitPct)
            startPursuit(TindralPathOfFlame);
    }

    void onTimedEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TINDRAL_FALLING_STAR:
                castAtPlayers(SPELL_TINDRAL_FALLING_STAR, 2);
                scriptEvents.addEvent(EVENT_TINDRAL_FALLING_STAR, 25000);
                break;
            case EVENT_TINDRAL_MASS_ENTANGLEMENT:
                castAtPlayers(SPELL_TINDRAL_MASS_ENTANGLEMENT, 3, true);
                scriptEvents.addEvent(EVENT_TINDRAL_MASS_ENTANGLEMENT, 35000);
                break;
            case EVENT_TINDRAL_FIERY_GROWTH:
                castAtPlayers(SPELL_TINDRAL_FIERY_GROWTH, 3);
                scriptEvents.addEvent(EVENT_TINDRAL_FIERY_GROWTH, 30000);
                break;
            case EVENT_TINDRAL_BLAZING_MUSHROOM:
                castUnderVictim(SPELL_TINDRAL_BLAZING_MUSHROOM);
                scriptEvents.addEvent(EVENT_TINDRAL_BLAZING_MUSHROOM, 30000);
                break;
            case EVENT_TINDRAL_FLAME_SURGE:
                castSpellAOE(SPELL_TINDRAL_FLAME_SURGE);
                scriptEvents.addEvent(EVENT_TINDRAL_FLAME_SURGE, 40000);
                break;
            case EVENT_TINDRAL_SUNFLAME:
                castSpellAOE(SPELL_TINDRAL_SUNFLAME);
                scriptEvents.addEvent(EVENT_TINDRAL_SUNFLAME, 10000);
                break;
            case EVENT_TINDRAL_FIRE_BEAM:
                castAtPlayer(SPELL_TINDRAL_FIRE_BEAM, true);
                scriptEvents.addEvent(EVENT_TINDRAL_FIRE_BEAM, 30000);
                break;
            case EVENT_TINDRAL_FLARE_BOMB:
                castUnderPlayer(SPELL_TINDRAL_FLARE_BOMB);
                scriptEvents.addEvent(EVENT_TINDRAL_FLARE_BOMB, 45000);
                break;
            case EVENT_TINDRAL_SUPPRESSIVE_EMBER:
                castAtPlayers(SPELL_TINDRAL_SUPPRESSIVE_EMBER, 3);
                scriptEvents.addEvent(EVENT_TINDRAL_SUPPRESSIVE_EMBER, 25000);
                break;
            case EVENT_TINDRAL_FLAMING_GERMINATION:
                castSpellAOE(SPELL_TINDRAL_FLAMING_GERMINATION);
                scriptEvents.addEvent(EVENT_TINDRAL_FLAMING_GERMINATION, 35000);
                break;
            case EVENT_TINDRAL_SEER_SUPERNOVA:
                warn("Tindral Sageswift channels a Supernova!");
                castSpellOnSelf(SPELL_TINDRAL_SEER_SUPERNOVA);
                scriptEvents.addEvent(EVENT_TINDRAL_SEER_SUPERNOVA, 60000);
                break;
            case EVENT_TINDRAL_SUPERNOVA:
                warn("Tindral Sageswift begins a Supernova! Break his shield!");
                castSpellOnSelf(SPELL_TINDRAL_SUPERNOVA);
                break;
            case EVENT_TINDRAL_PURSUIT_OVER:
            {
                uint32_t const nextStage = mPhase == TindralBurningPursuit ? TindralTreeOfTheFlame : TindralSeerOfTheFlame;
                setRooted(false);
                beginPhase(nextStage);
                scheduleStage(nextStage);
                break;
            }
            default:
                break;
        }
    }

private:
    // the flight itself is not scripted, he holds his ground and casts Supernova
    void startPursuit(uint32_t stage)
    {
        beginPhase(stage);
        getCreature()->interruptSpell();
        setRooted(true);
        if (stage == TindralPathOfFlame)
            castSpellOnSelf(SPELL_TINDRAL_OWL_OF_THE_FLAME, true);

        scriptEvents.addEvent(EVENT_TINDRAL_SUPERNOVA, 3000);
        scriptEvents.addEvent(EVENT_TINDRAL_PURSUIT_OVER, 25000);
    }

    void scheduleStage(uint32_t stage)
    {
        scriptEvents.addEvent(EVENT_TINDRAL_FALLING_STAR, 8000);
        scriptEvents.addEvent(EVENT_TINDRAL_MASS_ENTANGLEMENT, 12000);
        scriptEvents.addEvent(EVENT_TINDRAL_FIERY_GROWTH, 18000);
        scriptEvents.addEvent(EVENT_TINDRAL_BLAZING_MUSHROOM, 22000);
        scriptEvents.addEvent(EVENT_TINDRAL_FLAME_SURGE, 35000);
        scriptEvents.addEvent(EVENT_TINDRAL_FLARE_BOMB, 40000);

        // Searing Moonkin Form
        if (stage != TindralTreeOfTheFlame)
        {
            scriptEvents.addEvent(EVENT_TINDRAL_SUNFLAME, 5000);
            scriptEvents.addEvent(EVENT_TINDRAL_FIRE_BEAM, 26000);
        }

        // Burning Treant Form
        if (stage != TindralMoonkin)
        {
            scriptEvents.addEvent(EVENT_TINDRAL_SUPPRESSIVE_EMBER, 10000);
            scriptEvents.addEvent(EVENT_TINDRAL_FLAMING_GERMINATION, 20000);
        }

        if (stage == TindralSeerOfTheFlame)
            scriptEvents.addEvent(EVENT_TINDRAL_SEER_SUPERNOVA, 30000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Fyrakk the Blazing

class AmirdrassilFyrakkAI : public AmirdrassilBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilFyrakkAI(c); }
    explicit AmirdrassilFyrakkAI(Creature* pCreature) : AmirdrassilBossAI(pCreature, DATA_FYRAKK, FyrakkBerserkMs) {}

protected:
    void onPull() override
    {
        castSpellOnSelf(SPELL_FYRAKK_BURNING_PRESENCE, true);
        castSpellOnSelf(SPELL_FYRAKK_PREDATOR, true);
        scriptEvents.addEvent(EVENT_FYRAKK_WILDFIRE, 6000);
        scriptEvents.addEvent(EVENT_FYRAKK_BITE, 8000);
        scriptEvents.addEvent(EVENT_FYRAKK_AFLAME, 12000);
        scriptEvents.addEvent(EVENT_FYRAKK_BLAZE, 14000);
        scriptEvents.addEvent(EVENT_FYRAKK_FIRESTORM, 20000);
        scriptEvents.addEvent(EVENT_FYRAKK_DREAM_REND, 30000);
    }

    void onReset() override { setRooted(false); }

    void onFightTick(uint32_t /*diff*/) override
    {
        int32_t const health = _getHealthPercent();
        if (mPhase == FyrakkDreamRender && health <= FyrakkAssaultPct)
            assaultTheHeart();
        else if (mPhase == FyrakkChildrenOfTheStars && health <= FyrakkIncarnatePct)
            becomeIncarnate();
    }

    void onTimedEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_FYRAKK_BITE:
                castSpellOnVictim(SPELL_FYRAKK_FYRALATHS_BITE);
                scriptEvents.addEvent(EVENT_FYRAKK_BITE, 12000);
                break;
            case EVENT_FYRAKK_WILDFIRE:
                castSpellAOE(SPELL_FYRAKK_WILDFIRE);
                scriptEvents.addEvent(EVENT_FYRAKK_WILDFIRE, 25000);
                break;
            case EVENT_FYRAKK_FIRESTORM:
                castUnderPlayer(SPELL_FYRAKK_FIRESTORM);
                scriptEvents.addEvent(EVENT_FYRAKK_FIRESTORM, 40000);
                break;
            case EVENT_FYRAKK_DREAM_REND:
                warn("Fyrakk tears open the Dream with Dream Rend!");
                castSpellAOE(SPELL_FYRAKK_DREAM_REND);
                scriptEvents.addEvent(EVENT_FYRAKK_DREAM_REND, 50000);
                break;
            case EVENT_FYRAKK_BLAZE:
                castAtPlayer(SPELL_FYRAKK_BLAZE, true);
                scriptEvents.addEvent(EVENT_FYRAKK_BLAZE, mPhase == FyrakkHeartAssault ? 12000 : 30000);
                break;
            case EVENT_FYRAKK_AFLAME:
                castAtPlayers(SPELL_FYRAKK_AFLAME, 2);
                scriptEvents.addEvent(EVENT_FYRAKK_AFLAME, 20000);
                break;
            case EVENT_FYRAKK_CORRUPT:
                castSpellOnSelf(SPELL_FYRAKK_CORRUPT);
                break;
            case EVENT_FYRAKK_ORBS:
                castSpellAOE(SPELL_FYRAKK_SHADOWFLAME_ORBS);
                scriptEvents.addEvent(EVENT_FYRAKK_ORBS, 10000);
                break;
            case EVENT_FYRAKK_ASSAULT_OVER:
                childrenOfTheStars();
                break;
            case EVENT_FYRAKK_GREATER_FIRESTORM:
                warn("Fyrakk calls down Burning Colossi with Greater Firestorm!");
                castSpellAOE(SPELL_FYRAKK_GREATER_FIRESTORM);
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_FYRAKK_BURNING_COLOSSUS, nearSelf(15.0f, 25.0f));
                scriptEvents.addEvent(EVENT_FYRAKK_GREATER_FIRESTORM, 60000);
                break;
            case EVENT_FYRAKK_FLAMEFALL:
                castSpellAOE(SPELL_FYRAKK_FLAMEFALL);
                scriptEvents.addEvent(EVENT_FYRAKK_FLAMEFALL, 35000);
                break;
            case EVENT_FYRAKK_DEVASTATION:
                castAtPlayer(SPELL_FYRAKK_SHADOWFLAME_DEVASTATION, true);
                for (uint8_t i = 0; i < 3; ++i)
                    callAdd(NPC_FYRAKK_SCREAMING_SOUL, nearSelf(8.0f, 20.0f), TIMED_DESPAWN, ScreamingSoulLifeMs);
                scriptEvents.addEvent(EVENT_FYRAKK_DEVASTATION, 45000);
                break;
            case EVENT_FYRAKK_ETERNAL_FIRESTORM:
                castSpellAOE(SPELL_FYRAKK_ETERNAL_FIRESTORM);
                scriptEvents.addEvent(EVENT_FYRAKK_ETERNAL_FIRESTORM, 50000);
                break;
            case EVENT_FYRAKK_APOCALYPSE_ROAR:
                warn("Fyrakk lets out an Apocalypse Roar!");
                castSpellAOE(SPELL_FYRAKK_APOCALYPSE_ROAR);
                scriptEvents.addEvent(EVENT_FYRAKK_APOCALYPSE_ROAR, 40000);
                break;
            case EVENT_FYRAKK_BREATH:
                castAtPlayer(SPELL_FYRAKK_SHADOWFLAME_BREATH);
                scriptEvents.addEvent(EVENT_FYRAKK_BREATH, 30000);
                break;
            case EVENT_FYRAKK_INFERNAL_MAW:
                castSpellOnVictim(SPELL_FYRAKK_INFERNAL_MAW);
                scriptEvents.addEvent(EVENT_FYRAKK_INFERNAL_MAW, 10000);
                break;
            default:
                break;
        }
    }

private:
    // the flight to the Heart is not scripted, he corrupts it from where he stands
    void assaultTheHeart()
    {
        beginPhase(FyrakkHeartAssault);
        getCreature()->interruptSpell();
        setRooted(true);
        warn("Fyrakk turns his corrupting flames on the Heart of Amirdrassil!");
        castSpellOnSelf(SPELL_FYRAKK_INCARNATE, true);
        castSpellOnSelf(SPELL_FYRAKK_BURNING_SCALES, true);
        castSpellAOE(SPELL_FYRAKK_SHADOWFLAME_ORBS, true);
        scriptEvents.addEvent(EVENT_FYRAKK_CORRUPT, 2000);
        scriptEvents.addEvent(EVENT_FYRAKK_BLAZE, 6000);
        scriptEvents.addEvent(EVENT_FYRAKK_ORBS, 10000);
        scriptEvents.addEvent(EVENT_FYRAKK_ASSAULT_OVER, 35000);
    }

    void childrenOfTheStars()
    {
        beginPhase(FyrakkChildrenOfTheStars);
        setRooted(false);
        warn("A power stirs within the Heart of Amirdrassil!");
        scriptEvents.addEvent(EVENT_FYRAKK_BITE, 8000);
        scriptEvents.addEvent(EVENT_FYRAKK_AFLAME, 12000);
        scriptEvents.addEvent(EVENT_FYRAKK_GREATER_FIRESTORM, 15000);
        scriptEvents.addEvent(EVENT_FYRAKK_BLAZE, 18000);
        scriptEvents.addEvent(EVENT_FYRAKK_FLAMEFALL, 25000);
        scriptEvents.addEvent(EVENT_FYRAKK_DEVASTATION, 35000);
    }

    void becomeIncarnate()
    {
        beginPhase(FyrakkShadowflameIncarnate);
        getCreature()->interruptSpell();
        warn("Fyrakk unleashes the full power of an Incarnate!");
        castSpellOnSelf(SPELL_FYRAKK_BURNING_SCALES, true);
        castSpellAOE(SPELL_FYRAKK_ETERNAL_FIRESTORM, true);
        scriptEvents.addEvent(EVENT_FYRAKK_INFERNAL_MAW, 6000);
        scriptEvents.addEvent(EVENT_FYRAKK_BREATH, 15000);
        scriptEvents.addEvent(EVENT_FYRAKK_BLAZE, 20000);
        scriptEvents.addEvent(EVENT_FYRAKK_APOCALYPSE_ROAR, 30000);
        scriptEvents.addEvent(EVENT_FYRAKK_ETERNAL_FIRESTORM, 50000);
    }
};

// explodes its core on a full bar
class AmirdrassilBurningColossusAI : public AmirdrassilAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilBurningColossusAI(c); }
    explicit AmirdrassilBurningColossusAI(Creature* pCreature) : AmirdrassilAddAI(pCreature, BurningColossusSpells) {}

protected:
    void onPull() override
    {
        mCore.start(0, ColossusCoreMs);
        AmirdrassilAddAI::onPull();
    }

    void onReset() override { mCore.stop(); }

    void onFightTick(uint32_t diff) override
    {
        if (!mCore.advance(diff) || _isCasting())
            return;

        mCore.empty();
        warn("A Burning Colossus unleashes an Exploding Core!");
        castSpellAOE(SPELL_COLOSSUS_EXPLODING_CORE);
    }

private:
    AmirdrassilEnergy mCore;
};

class AmirdrassilScreamingSoulAI : public AmirdrassilAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmirdrassilScreamingSoulAI(c); }
    explicit AmirdrassilScreamingSoulAI(Creature* pCreature) : AmirdrassilAddAI(pCreature, ScreamingSoulSpells) {}
};

void SetupAmirdrassil(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_AMIRDRASSIL, &AmirdrassilInstanceScript::Create);

    // Gnarlroot
    mgr->register_creature_script(BOSS_GNARLROOT, &AmirdrassilGnarlrootAI::Create);
    mgr->register_creature_script(NPC_GNARLROOT_TAINTED_LASHER, &AmirdrassilTaintedLasherAI::Create);

    // Igira the Cruel
    mgr->register_creature_script(BOSS_IGIRA, &AmirdrassilIgiraAI::Create);

    // Volcoross
    mgr->register_creature_script(BOSS_VOLCOROSS, &AmirdrassilVolcorossAI::Create);

    // Council of Dreams
    mgr->register_creature_script(BOSS_URCTOS, &AmirdrassilUrctosAI::Create);
    mgr->register_creature_script(BOSS_AERWYNN, &AmirdrassilAerwynnAI::Create);
    mgr->register_creature_script(BOSS_PIP, &AmirdrassilPipAI::Create);

    // Larodar, Keeper of the Flame
    mgr->register_creature_script(BOSS_LARODAR, &AmirdrassilLarodarAI::Create);
    mgr->register_creature_script(NPC_LARODAR_FIERY_TREANT, &AmirdrassilFieryTreantAI::Create);
    mgr->register_creature_script(NPC_LARODAR_SCORCHING_ROOTS, &AmirdrassilScorchingRootsAI::Create);
    mgr->register_creature_script(NPC_LARODAR_ASHEN_TREANT, &AmirdrassilAshenTreantAI::Create);
    mgr->register_creature_script(NPC_LARODAR_CHARRED_TREANT, &AmirdrassilCharredRemainsAI::CreateTreant);
    mgr->register_creature_script(NPC_LARODAR_CHARRED_BRAMBLES, &AmirdrassilCharredRemainsAI::CreateBrambles);
    mgr->register_creature_script(NPC_LARODAR_RENEWED_TREANT, &AmirdrassilRenewedTreantAI::Create);
    mgr->register_creature_script(NPC_LARODAR_RENEWED_BRAMBLE_BARRIER, &AmirdrassilBrambleBarrierAI::Create);

    // Nymue, Weaver of the Cycle
    mgr->register_creature_script(BOSS_NYMUE, &AmirdrassilNymueAI::Create);
    mgr->register_creature_script(NPC_NYMUE_CYCLE_WARDEN, &AmirdrassilCycleWardenAI::Create);

    // Smolderon
    mgr->register_creature_script(BOSS_SMOLDERON, &AmirdrassilSmolderonAI::Create);
    mgr->register_creature_script(NPC_SMOLDERON_LIVING_FLAME, &AmirdrassilLivingFlameAI::Create);

    // Tindral Sageswift, Seer of the Flame
    mgr->register_creature_script(BOSS_TINDRAL, &AmirdrassilTindralAI::Create);

    // Fyrakk the Blazing
    mgr->register_creature_script(BOSS_FYRAKK, &AmirdrassilFyrakkAI::Create);
    mgr->register_creature_script(NPC_FYRAKK_BURNING_COLOSSUS, &AmirdrassilBurningColossusAI::Create);
    mgr->register_creature_script(NPC_FYRAKK_SCREAMING_SOUL, &AmirdrassilScreamingSoulAI::Create);
}
