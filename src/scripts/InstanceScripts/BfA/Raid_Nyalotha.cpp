/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Nyalotha.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

enum NyalothaEvents
{
    // shared adds
    EVENT_NYALOTHA_ADD_SPELL = 1,

    // Maut
    EVENT_MAUT_SHADOW_CLAWS,
    EVENT_MAUT_DEVOUR_MAGIC,
    EVENT_MAUT_DEVOUR_MARKS,
    EVENT_MAUT_CONSUMING_SHADOWS,
    EVENT_MAUT_STYGIAN_ANNIHILATION,
    EVENT_MAUT_STYGIAN_BURST,
    EVENT_MAUT_DARK_MANIFESTATION,
    EVENT_MAUT_BLACK_WINGS,
    EVENT_MAUT_DRAIN_ESSENCE,
    EVENT_MAUT_OBSIDIAN_SKIN,
    EVENT_MAUT_SKIN_SHATTERS,

    // Dark Inquisitor Xanesh
    EVENT_XANESH_ABYSSAL_STRIKE,
    EVENT_XANESH_VOID_RITUAL,
    EVENT_XANESH_SOUL_FLAY,
    EVENT_XANESH_FLAYED_SOULS,
    EVENT_XANESH_TORMENT,
    EVENT_XANESH_OBELISKS,

    // Wrathion
    EVENT_WRATHION_TAIL_SWIPE,
    EVENT_WRATHION_SEARING_BREATH,
    EVENT_WRATHION_BREATH_LANDS,
    EVENT_WRATHION_INCINERATION,
    EVENT_WRATHION_GALE_BLAST,
    EVENT_WRATHION_GALE_MISSILES,
    EVENT_WRATHION_BURNING_CATACLYSM,
    EVENT_WRATHION_CATACLYSM_ENDS,
    EVENT_WRATHION_SCALES,
    EVENT_WRATHION_SMOKE_CLEARS,
    EVENT_WRATHION_RETURNS,

    // Ra-den
    EVENT_RADEN_NULLIFYING_STRIKE,
    EVENT_RADEN_ESSENCES,
    EVENT_RADEN_CRACKLING_STALKER,
    EVENT_RADEN_VOID_HUNTER,
    EVENT_RADEN_GORGE_ENERGY,
    EVENT_RADEN_VOID_ERUPTION,
    EVENT_RADEN_DECAYING_STRIKE,
    EVENT_RADEN_CHARGED_BONDS,
    EVENT_RADEN_RUIN,

    // The Hivemind
    EVENT_HIVEMIND_CONTROL,
    EVENT_HIVEMIND_RECONSTITUTION,
    EVENT_TEKRIS_AQIR_DRONES,
    EVENT_TEKRIS_AQIR_DARTERS,
    EVENT_TEKRIS_ECHOING_VOID,
    EVENT_TEKRIS_NULLIFICATION_BLAST,
    EVENT_TEKRIS_ACCELERATED_EVOLUTION,
    EVENT_TEKRIS_EVOLUTION_DONE,
    EVENT_KAZIR_ACIDIC_AQIR,
    EVENT_KAZIR_MIND_NUMBING_NOVA,
    EVENT_AQIR_DRONE_FIXATE,

    // Prophet Skitra
    EVENT_SKITRA_SHADOW_SHOCK,
    EVENT_SKITRA_SHRED_PSYCHE,
    EVENT_SKITRA_PSYCHE_TEARS,
    EVENT_SKITRA_DARK_RITUAL,
    EVENT_SKITRA_PROJECTIONS_END,
    EVENT_SHREDDED_PSYCHE_OUTBURST,
    EVENT_SHREDDED_PSYCHE_REVERBERATIONS,

    // Shad'har
    EVENT_SHADHAR_BREATH,
    EVENT_SHADHAR_CRUSH,
    EVENT_SHADHAR_DISSOLVE,
    EVENT_SHADHAR_UMBRAL_ERUPTION,
    EVENT_SHADHAR_DEBILITATING_SPIT,
    EVENT_SHADHAR_LIVING_MIASMA,
    EVENT_SHADHAR_MANTLE,
    EVENT_LIVING_MIASMA_FIXATE,

    // Vexiona
    EVENT_VEXIONA_VOID_ASCENDANT,
    EVENT_VEXIONA_ENCROACHING_SHADOWS,
    EVENT_VEXIONA_DESPAIR,
    EVENT_VEXIONA_TWILIGHT_BREATH,
    EVENT_VEXIONA_DARK_GATEWAY,
    EVENT_VEXIONA_HEART_OF_DARKNESS,
    EVENT_VEXIONA_DESOLATION,

    // Carapace of N'Zoth
    EVENT_CARAPACE_MANDIBLE_SLAM,
    EVENT_CARAPACE_BLACK_SCAR,
    EVENT_CARAPACE_MENTAL_DECAY,
    EVENT_CARAPACE_DECAY_SPREADS,
    EVENT_CARAPACE_MADNESS_BOMB,
    EVENT_CARAPACE_BOMB_MARKS,
    EVENT_CARAPACE_ANTIGENS,
    EVENT_CARAPACE_ADAPTIVE_MEMBRANE,
    EVENT_CARAPACE_GROWTH_TENTACLE,
    EVENT_CARAPACE_GAZE_OF_MADNESS,
    EVENT_CARAPACE_ETERNAL_DARKNESS,
    EVENT_CARAPACE_INSANITY_BOMB,
    EVENT_CARAPACE_OCCIPITAL_BLAST,
    EVENT_CARAPACE_BLAST_LANDS,

    // Drest'agath
    EVENT_DRESTAGATH_VOLATILE_SEED,
    EVENT_DRESTAGATH_ENTROPIC_CRASH,
    EVENT_DRESTAGATH_VOID_GLARE,
    EVENT_DRESTAGATH_LIMB_WAVE,
    EVENT_DRESTAGATH_THROES_OF_AGONY,

    // Il'gynoth
    EVENT_ILGYNOTH_EYE_OF_NZOTH,
    EVENT_ILGYNOTH_CORRUPTORS_GAZE,
    EVENT_ILGYNOTH_CURSED_BLOOD,
    EVENT_ILGYNOTH_SUBMERGE,
    EVENT_ILGYNOTH_CLOTTED_CORRUPTION,
    EVENT_BLOOD_OF_NYALOTHA_FIXATE,

    // N'Zoth
    EVENT_NZOTH_DOMINANT_MIND,
    EVENT_NZOTH_EGO_RECOVERS,
    EVENT_NZOTH_ETERNAL_TORMENT,
    EVENT_NZOTH_CATACLYSMIC_FLAMES,
    EVENT_NZOTH_TENTACLE_WAVE,
    EVENT_NZOTH_STUPEFYING_GLARE,
    EVENT_NZOTH_MINDGRASP,
    EVENT_NZOTH_THOUGHT_HARVESTER,
    EVENT_PSYCHUS_MINDWRACK,
    EVENT_PSYCHUS_CREEPING_ANGUISH,
    EVENT_PSYCHUS_MANIFEST_MADNESS,
    EVENT_BASHER_TUMULTUOUS_BURST,
    EVENT_BASHER_VOID_LASH
};

enum NyalothaActions
{
    ACTION_RADEN_VITA_ARRIVED = 1,
    ACTION_RADEN_VOID_ARRIVED,
    ACTION_DRESTAGATH_AGONY,
    ACTION_DRESTAGATH_GLARE,
    ACTION_ORGAN_EXPOSE,
    ACTION_ORGAN_SEAL
};

namespace
{
    float const NyalothaTwoPi = 6.28318531f;
    float const NyalothaPi = 3.14159265f;
    uint32_t const NyalothaRaidCap = 30;

    // energy and mana bars kept as fill times
    uint32_t const MautManaFillMs = 90000;
    uint32_t const ShadharBreathFillMs = 29000;
    uint32_t const VexionaHeartFillMs = 100000;
    uint32_t const CarapaceSlamFillMs = 17000;
    uint32_t const DrestagathAgonyFillMs = 100000;
    uint32_t const PsychusMadnessFillMs = 101000;

    uint32_t const MautSkinHits = 100;
    uint32_t const MautSkinMaxMs = 30000;
    uint8_t const WrathionShardCount = 4;
    uint8_t const WrathionAssassinCount = 3;
    uint8_t const SkitraProjectionCount = 5;
    uint8_t const IlgynothOrganCount = 3;
    uint32_t const HivemindSwapMs = 70000;
    float const HivemindVeilRange = 20.0f;

    uint32_t const HivemindMembers[] = { BOSS_KAZIR, BOSS_TEKRIS };

    //////////////////////////////////////////////////////////////////////////////////////////
    // Positions

    LocationVector const WrathionRoomSouthWest(-1792.64f, -73.56f, -1478.16f, 0.0f);
    LocationVector const WrathionRoomNorthEast(-1718.55f, -12.73f, -1478.16f, 0.0f);

    LocationVector const RadenVitaSpot(-266.726f, 180.448f, -211.817f, 3.255f);
    LocationVector const RadenVoidSpot(-342.462f, 247.273f, -211.817f, 4.952f);

    LocationVector const SkitraProjectionCentre(-1487.0f, -580.0f, -518.4f, 4.71f);

    LocationVector const CarapaceTentacleSpots[4] =
    {
        { -11603.432f, 9261.467f, 123.591f, 1.540f },
        { -11565.716f, 9269.769f, 124.516f, 1.869f },
        { -11617.975f, 9359.151f, 125.251f, 5.165f },
        { -11568.685f, 9365.344f, 124.740f, 4.462f }
    };

    LocationVector const CarapaceGazeSpot(-11536.851f, 9343.937f, 126.788f, 4.0266f);

    LocationVector const DrestagathEyeSpot(-530.989f, 213.483f, -484.662f, 4.833f);

    LocationVector const IlgynothOrganSpots[3] =
    {
        { -284.047f, 112.586f, -524.631f, 3.925f },
        { -294.877f, -1.4498f, -524.986f, 2.198f },
        { -408.385f, 7.823f, -524.518f, 0.711f }
    };

    LocationVector const NzothPsychusSpot(-11194.0f, 9323.0f, 125.865f, 3.0f);
    LocationVector const NzothBasherSpot(-11099.0f, 9278.0f, 125.866f, 2.2f);
    LocationVector const NzothHarvesterSpot(-11157.0f, 9321.0f, 125.866f, 3.0f);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Drest'agath limb waves

    struct NyalothaLimbPlacement
    {
        uint8_t wave;
        uint32_t entry;
        LocationVector spot;
    };

    uint32_t const DrestagathWaveTimes[] = { 17000, 110000, 200000, 290000, 380000 };
    size_t const DrestagathWaveCount = sizeof(DrestagathWaveTimes) / sizeof(DrestagathWaveTimes[0]);

    NyalothaLimbPlacement const DrestagathLimbs[] =
    {
        { 0, NPC_DRESTAGATH_EYE,      { -537.979f, 249.764f, -484.515f, 4.476f } },
        { 0, NPC_DRESTAGATH_TENTACLE, { -539.835f, 230.644f, -484.739f, 5.117f } },
        { 0, NPC_DRESTAGATH_TENTACLE, { -488.273f, 242.526f, -484.631f, 5.026f } },
        { 0, NPC_DRESTAGATH_TENTACLE, { -475.170f, 183.860f, -484.911f, 1.679f } },
        { 0, NPC_DRESTAGATH_MAW,      { -551.022f, 183.074f, -484.938f, 1.690f } },

        { 1, NPC_DRESTAGATH_MAW,      { -517.787f, 193.074f, -484.684f, 1.568f } },
        { 1, NPC_DRESTAGATH_MAW,      { -494.643f, 206.261f, -484.450f, 1.745f } },
        { 1, NPC_DRESTAGATH_MAW,      { -527.579f, 265.148f, -484.951f, 4.722f } },
        { 1, NPC_DRESTAGATH_EYE,      { -503.748f, 181.565f, -484.576f, 1.681f } },
        { 1, NPC_DRESTAGATH_TENTACLE, { -528.391f, 173.251f, -484.553f, 1.426f } },

        { 2, NPC_DRESTAGATH_EYE,      { -540.627f, 230.830f, -484.735f, 4.825f } },
        { 2, NPC_DRESTAGATH_EYE,      { -550.973f, 188.269f, -484.962f, 0.532f } },
        { 2, NPC_DRESTAGATH_EYE,      { -506.552f, 181.430f, -484.615f, 1.868f } },
        { 2, NPC_DRESTAGATH_TENTACLE, { -479.870f, 227.161f, -484.547f, 3.375f } },
        { 2, NPC_DRESTAGATH_MAW,      { -546.596f, 171.310f, -484.794f, 0.199f } },

        { 3, NPC_DRESTAGATH_MAW,      { -558.856f, 239.233f, -484.629f, 5.773f } },
        { 3, NPC_DRESTAGATH_MAW,      { -500.945f, 212.461f, -484.682f, 2.643f } },
        { 3, NPC_DRESTAGATH_EYE,      { -503.177f, 185.026f, -484.567f, 1.650f } },
        { 3, NPC_DRESTAGATH_EYE,      { -544.403f, 225.410f, -484.636f, 5.732f } },
        { 3, NPC_DRESTAGATH_TENTACLE, { -561.334f, 181.176f, -484.936f, 0.561f } },
        { 3, NPC_DRESTAGATH_TENTACLE, { -518.446f, 275.674f, -484.946f, 4.590f } },

        { 4, NPC_DRESTAGATH_MAW,      { -473.772f, 179.005f, -484.926f, 1.344f } },
        { 4, NPC_DRESTAGATH_MAW,      { -563.086f, 202.996f, -484.894f, 0.023f } },
        { 4, NPC_DRESTAGATH_EYE,      { -568.914f, 188.183f, -484.989f, 0.420f } },
        { 4, NPC_DRESTAGATH_EYE,      { -492.998f, 233.653f, -484.702f, 3.734f } },
        { 4, NPC_DRESTAGATH_TENTACLE, { -493.332f, 221.747f, -484.632f, 3.760f } },
        { 4, NPC_DRESTAGATH_TENTACLE, { -515.428f, 196.642f, -484.751f, 1.094f } }
    };

    // random point on a ring around origin, facing away from it
    LocationVector nyalothaRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(NyalothaTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // random point on the floor of Wrathion's arena
    LocationVector wrathionRoomPoint()
    {
        return LocationVector(Util::getRandomFloat(WrathionRoomSouthWest.x, WrathionRoomNorthEast.x),
            Util::getRandomFloat(WrathionRoomSouthWest.y, WrathionRoomNorthEast.y), WrathionRoomSouthWest.z, 0.0f);
    }

    // true at or below one percent health
    bool isAtFloor(Creature* pCreature)
    {
        return static_cast<uint64_t>(pCreature->getHealth()) * 100 <= pCreature->getMaxHealth();
    }

    enum class NyalothaAim : uint8_t
    {
        Victim,
        RandomPlayer,
        Area,
        Self
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class NyalothaInstanceScript : public InstanceScript
{
public:
    explicit NyalothaInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(NyalothaEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new NyalothaInstanceScript(pMapMgr); }

    // a credited member falling before its partner must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (id == DATA_THE_HIVEMIND && state != EncounterStates::InProgress)
            mHivemindController = 0;

        if (state != EncounterStates::Failed)
            return;

        // a wiped pair fights again in full
        for (Creature* pMember : getGroup(id))
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    std::vector<Creature*> getGroup(uint32_t dataId)
    {
        std::vector<Creature*> group;
        if (dataId != DATA_THE_HIVEMIND)
            return group;

        for (uint32_t entry : HivemindMembers)
        {
            for (Creature* pMember : getCreatureSetForEntry(entry))
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

    // pulling one member pulls the rest
    void rallyGroup(uint32_t dataId, Creature* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember == pPuller || !pMember->isAlive() || pMember->isInCombat())
                continue;

            if (CreatureAIScript* pScript = pMember->GetScript())
                pScript->setZoneWideCombat();
        }
        mRallying = false;
    }

    void setHivemindController(uint32_t entry) { mHivemindController = entry; }
    uint32_t getHivemindController() const { return mHivemindController; }

private:
    uint32_t mHivemindController = 0;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick, berserk and target helpers

class NyalothaCombatAI : public CreatureAIScript
{
public:
    explicit NyalothaCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        if (mImmobile)
            setRooted(true);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mStage = 1;
        mBerserkLeftMs = 0;
        if (mImmobile)
            setRooted(true);

        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mStage = 0;
        mBerserkLeftMs = 0;
        setDormant(false);
        onDisengage();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive() || !_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        tickBerserk(diff);
        onCombatTick(diff);

        if (!isAlive() || _isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    NyalothaInstanceScript* getNyalotha()
    {
        if (getCreature()->GetMapId() != MAP_NYALOTHA)
            return nullptr;

        return static_cast<NyalothaInstanceScript*>(getInstanceScript());
    }

    Unit* currentVictim() { return getCreature()->getAIInterface()->getCurrentTarget(); }

    // shuffled living players, the tank only when nobody else is left
    std::vector<Unit*> pickPlayers(uint32_t count, bool skipVictim = false)
    {
        Unit* pVictim = currentVictim();

        std::vector<Unit*> candidates;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr || !pObject->isPlayer())
                continue;

            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (!pPlayer->isAlive() || getRangeToObject(pPlayer) > 100.0f || (skipVictim && pPlayer == pVictim))
                continue;

            candidates.push_back(pPlayer);
        }

        if (candidates.empty() && skipVictim && pVictim != nullptr && pVictim->isPlayer() && pVictim->isAlive())
            candidates.push_back(pVictim);

        Util::randomShuffleVector(candidates);
        if (candidates.size() > count)
            candidates.resize(count);

        return candidates;
    }

    Unit* randomPlayer(bool skipVictim = false)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipVictim);
        return picked.empty() ? nullptr : picked.front();
    }

    Unit* nearestPlayer(float maxRange)
    {
        Unit* pNearest = nullptr;
        float nearest = maxRange;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr || !pObject->isPlayer())
                continue;

            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (!pPlayer->isAlive())
                continue;

            float const range = getRangeToObject(pPlayer);
            if (range <= nearest)
            {
                nearest = range;
                pNearest = pPlayer;
            }
        }

        return pNearest;
    }

    Unit* castAtRandomPlayer(uint32_t spellId, bool skipVictim = false, bool triggered = true)
    {
        Unit* pTarget = randomPlayer(skipVictim);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    void castOnPlayers(uint32_t spellId, uint32_t count, bool skipVictim = false)
    {
        for (Unit* pTarget : pickPlayers(count, skipVictim))
            castSpell(pTarget, spellId, true);
    }

    void castOnAllPlayers(uint32_t spellId) { castOnPlayers(spellId, NyalothaRaidCap); }

    void aimSpell(uint32_t spellId, NyalothaAim aim, bool triggered)
    {
        switch (aim)
        {
            case NyalothaAim::Victim:
                castSpellOnVictim(spellId, triggered);
                break;
            case NyalothaAim::RandomPlayer:
                castAtRandomPlayer(spellId, false, triggered);
                break;
            case NyalothaAim::Area:
                castSpellAOE(spellId, triggered);
                break;
            case NyalothaAim::Self:
                castSpellOnSelf(spellId, triggered);
                break;
        }
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // summoned add that waits until it is attacked
    Creature* placeAdd(uint32_t entry, LocationVector const& spot)
    {
        return summonCreature(entry, spot, CORPSE_DESPAWN);
    }

    // summoned helper outside the fight, lifetime 0 keeps it until despawned
    Creature* placeHelper(uint32_t entry, LocationVector const& spot, uint32_t lifetimeMs)
    {
        return summonCreature(entry, spot, lifetimeMs != 0 ? TIMED_DESPAWN : MANUAL_DESPAWN, lifetimeMs);
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

    void tell(Creature* pTarget, int32_t action)
    {
        if (pTarget != nullptr && pTarget->isAlive() && pTarget->GetScript() != nullptr)
            pTarget->GetScript()->DoAction(action);
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return nyalothaRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void showFrame(bool engaged)
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(engaged ? EncounterFrameEngage : EncounterFrameDisengaged, getCreature());
    }

    void armBerserk(uint32_t spellId, uint32_t delayMs)
    {
        mBerserkSpell = spellId;
        mBerserkLeftMs = delayMs;
    }

    // a new stage drops every pending event
    void enterStage(uint32_t stage)
    {
        mStage = stage;
        scriptEvents.resetEvents();
    }

    // out of reach: no melee, not selectable, rooted
    void setDormant(bool dormant)
    {
        if (mDormant == dormant)
            return;

        mDormant = dormant;
        setRooted(dormant || mImmobile);
        _setMeleeDisabled(dormant);

        if (dormant)
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        else
            removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    bool isDormant() const { return mDormant; }

    // drops all threat and sticks to one player
    void fixateOn(Unit* pTarget)
    {
        if (pTarget == nullptr)
            return;

        _clearHateList();
        addThreat(pTarget, 1000000.0f);
        attackStart(pTarget);
    }

    // true once when the countdown runs out
    static bool countDown(uint32_t& leftMs, uint32_t diff)
    {
        if (leftMs == 0)
            return false;

        if (leftMs > diff)
        {
            leftMs -= diff;
            return false;
        }

        leftMs = 0;
        return true;
    }

    uint32_t mStage = 0;
    bool mImmobile = false;

private:
    void tickBerserk(uint32_t diff)
    {
        if (countDown(mBerserkLeftMs, diff))
            castSpellOnSelf(mBerserkSpell, true);
    }

    uint32_t mBerserkSpell = 0;
    uint32_t mBerserkLeftMs = 0;
    bool mDormant = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class NyalothaBossAI : public NyalothaCombatAI
{
public:
    NyalothaBossAI(Creature* pCreature, uint32_t dataId) : NyalothaCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (NyalothaInstanceScript* pNyalotha = getNyalotha())
            pNyalotha->rallyGroup(mDataId, getCreature());

        NyalothaCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        // only a living boss means a wipe
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        NyalothaCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* pKiller) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            NyalothaInstanceScript* pNyalotha = getNyalotha();
            if (pNyalotha == nullptr || pNyalotha->isGroupDown(mDataId, getCreature()))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
        onDefeat(pKiller);
    }

    // area damage does not reach a boss out of reach
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && isDormant())
            *damage = 0;
    }

protected:
    virtual void onDefeat(Unit* /*pKiller*/) {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared add and helper scripts

// add with one spell, a zero repeat casts it once
class NyalothaSpellAddAI : public NyalothaCombatAI
{
public:
    NyalothaSpellAddAI(Creature* pCreature, uint32_t spellId, NyalothaAim aim, uint32_t firstMs, uint32_t minRepeatMs, uint32_t maxRepeatMs, bool immobile)
        : NyalothaCombatAI(pCreature), mSpellId(spellId), mAim(aim), mFirstMs(firstMs), mMinRepeatMs(minRepeatMs), mMaxRepeatMs(maxRepeatMs)
    {
        mImmobile = immobile;
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_NYALOTHA_ADD_SPELL, mFirstMs);
        onAddEngage();
    }

    virtual void onAddEngage() {}

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_NYALOTHA_ADD_SPELL)
            return;

        aimSpell(mSpellId, mAim, false);

        if (mMinRepeatMs != 0)
            scriptEvents.addEvent(EVENT_NYALOTHA_ADD_SPELL, mMaxRepeatMs > mMinRepeatMs ? Util::getRandomUInt(mMinRepeatMs, mMaxRepeatMs) : mMinRepeatMs);
    }

private:
    uint32_t mSpellId;
    NyalothaAim mAim;
    uint32_t mFirstMs;
    uint32_t mMinRepeatMs;
    uint32_t mMaxRepeatMs;
};

// unreachable helper that releases up to two spells once after a delay
class NyalothaHelperAI : public CreatureAIScript
{
public:
    NyalothaHelperAI(Creature* pCreature, uint32_t selfSpellId, uint32_t areaSpellId, uint32_t delayMs)
        : CreatureAIScript(pCreature), mSelfSpellId(selfSpellId), mAreaSpellId(areaSpellId), mLeftMs(delayMs) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        setRooted(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mLeftMs == 0)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mLeftMs > diff)
        {
            mLeftMs -= diff;
            return;
        }

        mLeftMs = 0;
        if (mSelfSpellId != 0)
            castSpellOnSelf(mSelfSpellId, true);
        if (mAreaSpellId != 0)
            castSpellAOE(mAreaSpellId, true);
    }

private:
    uint32_t mSelfSpellId;
    uint32_t mAreaSpellId;
    uint32_t mLeftMs;
};

// rooted target that never strikes back
class NyalothaAnchoredAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaAnchoredAI(c); }
    explicit NyalothaAnchoredAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Maut

class NyalothaMautAI : public NyalothaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaMautAI(c); }
    explicit NyalothaMautAI(Creature* pCreature) : NyalothaBossAI(pCreature, DATA_MAUT) {}

    // every blow on the skin burns a share of the stored mana
    void DamageTaken(Unit* pAttacker, uint32_t* /*damage*/) override
    {
        if (mSkinHitsLeft == 0 || pAttacker == nullptr || pAttacker == getCreature())
            return;

        if (--mSkinHitsLeft == 0)
        {
            scriptEvents.removeEvent(EVENT_MAUT_SKIN_SHATTERS);
            scriptEvents.addEvent(EVENT_MAUT_SKIN_SHATTERS, 100);
        }
    }

protected:
    void onEngage() override
    {
        armBerserk(SPELL_MAUT_ENRAGED, 600000);
        beginDevouring();
    }

    void onDisengage() override { dropSkin(); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MAUT_SHADOW_CLAWS:
                castSpellOnVictim(SPELL_MAUT_SHADOW_CLAWS);
                scriptEvents.addEvent(EVENT_MAUT_SHADOW_CLAWS, 20000);
                break;
            case EVENT_MAUT_DEVOUR_MAGIC:
                castSpellAOE(SPELL_MAUT_DEVOUR_MAGIC);
                scriptEvents.addEvent(EVENT_MAUT_DEVOUR_MARKS, 100);
                scriptEvents.addEvent(EVENT_MAUT_DEVOUR_MAGIC, 25000);
                break;
            case EVENT_MAUT_DEVOUR_MARKS:
                castOnPlayers(SPELL_MAUT_DEVOUR_MAGIC_MARK, 2);
                break;
            case EVENT_MAUT_CONSUMING_SHADOWS:
                castOnPlayers(SPELL_MAUT_CONSUMING_SHADOWS, 3);
                scriptEvents.addEvent(EVENT_MAUT_CONSUMING_SHADOWS, 15000);
                break;
            case EVENT_MAUT_STYGIAN_ANNIHILATION:
                warn("Maut begins to cast Stygian Annihilation!");
                castSpellAOE(SPELL_MAUT_STYGIAN_ANNIHILATION);
                scriptEvents.addEvent(EVENT_MAUT_STYGIAN_BURST, 100);
                scriptEvents.addEvent(EVENT_MAUT_STYGIAN_ANNIHILATION, 30000);
                break;
            case EVENT_MAUT_STYGIAN_BURST:
                castOnAllPlayers(SPELL_MAUT_STYGIAN_ANNIHILATION_HIT);
                break;
            case EVENT_MAUT_DARK_MANIFESTATION:
                manifestDarkness();
                scriptEvents.addEvent(EVENT_MAUT_DARK_MANIFESTATION, 40000);
                break;
            case EVENT_MAUT_BLACK_WINGS:
                if (Unit* pTarget = nearestPlayer(40.0f))
                    castSpell(pTarget, SPELL_MAUT_BLACK_WINGS);
                scriptEvents.addEvent(EVENT_MAUT_BLACK_WINGS, 23000);
                break;
            case EVENT_MAUT_DRAIN_ESSENCE:
                castOnPlayers(SPELL_MAUT_DRAIN_ESSENCE, 2);
                break;
            case EVENT_MAUT_OBSIDIAN_SKIN:
                hardenSkin();
                break;
            case EVENT_MAUT_SKIN_SHATTERS:
                dropSkin();
                beginDevouring();
                break;
            default:
                break;
        }
    }

private:
    // the mana bar fills again from empty
    void beginDevouring()
    {
        enterStage(1);
        scriptEvents.addEvent(EVENT_MAUT_SHADOW_CLAWS, 5000);
        scriptEvents.addEvent(EVENT_MAUT_DEVOUR_MAGIC, 10000);
        scriptEvents.addEvent(EVENT_MAUT_CONSUMING_SHADOWS, 13000);
        scriptEvents.addEvent(EVENT_MAUT_STYGIAN_ANNIHILATION, 30000);
        scriptEvents.addEvent(EVENT_MAUT_DARK_MANIFESTATION, 40000);
        scriptEvents.addEvent(EVENT_MAUT_OBSIDIAN_SKIN, MautManaFillMs);

        if (isHeroic())
        {
            scriptEvents.addEvent(EVENT_MAUT_BLACK_WINGS, 7000);
            scriptEvents.addEvent(EVENT_MAUT_DRAIN_ESSENCE, 14000);
        }
    }

    void hardenSkin()
    {
        enterStage(2);
        warn("Maut is encased in Obsidian Skin!");
        setRooted(true);
        castSpellOnSelf(SPELL_MAUT_OBSIDIAN_SKIN, true);
        castSpellOnSelf(SPELL_MAUT_OBSIDIAN_SKIN_VISUAL, true);
        castSpellOnSelf(SPELL_MAUT_OBSIDIAN_SKIN_MANA, true);
        mSkinHitsLeft = MautSkinHits;
        scriptEvents.addEvent(EVENT_MAUT_SKIN_SHATTERS, MautSkinMaxMs);
    }

    void dropSkin()
    {
        mSkinHitsLeft = 0;
        setRooted(false);
        _removeAura(SPELL_MAUT_OBSIDIAN_SKIN);
        _removeAura(SPELL_MAUT_OBSIDIAN_SKIN_VISUAL);
        _removeAura(SPELL_MAUT_OBSIDIAN_SKIN_MANA);
    }

    void manifestDarkness()
    {
        Unit* pTarget = randomPlayer(true);
        LocationVector const spot = pTarget != nullptr ? nyalothaRingPoint(pTarget->GetPosition(), 2.0f, 6.0f) : aroundSelf(5.0f, 15.0f);
        if (Creature* pManifestation = callAdd(NPC_MAUT_DARK_MANIFESTATION, spot))
            pManifestation->castSpell(pManifestation, SPELL_MAUT_DARK_AEGIS, true);
    }

    uint32_t mSkinHitsLeft = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Dark Inquisitor Xanesh

class NyalothaXaneshAI : public NyalothaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaXaneshAI(c); }
    explicit NyalothaXaneshAI(Creature* pCreature) : NyalothaBossAI(pCreature, DATA_DARK_INQUISITOR_XANESH) {}

protected:
    void onEngage() override
    {
        armBerserk(SPELL_XANESH_BERSERK, 900000);
        scriptEvents.addEvent(EVENT_XANESH_ABYSSAL_STRIKE, 10000);
        scriptEvents.addEvent(EVENT_XANESH_VOID_RITUAL, 18000);
        scriptEvents.addEvent(EVENT_XANESH_SOUL_FLAY, 25000);
        scriptEvents.addEvent(EVENT_XANESH_TORMENT, 50000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_XANESH_OBELISKS, 5000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_XANESH_ABYSSAL_STRIKE:
                castSpellOnVictim(SPELL_XANESH_ABYSSAL_STRIKE);
                scriptEvents.addEvent(EVENT_XANESH_ABYSSAL_STRIKE, 45000);
                break;
            case EVENT_XANESH_VOID_RITUAL:
                warn("Dark Inquisitor Xanesh begins a Void Ritual!");
                castSpellAOE(SPELL_XANESH_VOID_RITUAL);
                scriptEvents.addEvent(EVENT_XANESH_VOID_RITUAL, 80000);
                break;
            case EVENT_XANESH_SOUL_FLAY:
                castSpellAOE(SPELL_XANESH_SOUL_FLAY);
                scriptEvents.addEvent(EVENT_XANESH_FLAYED_SOULS, 100);
                scriptEvents.addEvent(EVENT_XANESH_SOUL_FLAY, 35000);
                break;
            case EVENT_XANESH_FLAYED_SOULS:
                // a flayed soul rises beside every marked player
                for (Unit* pTarget : pickPlayers(3))
                {
                    castSpell(pTarget, SPELL_XANESH_SOUL_FLAY_MARK, true);
                    callAdd(NPC_XANESH_FLAYED_SOUL, nyalothaRingPoint(pTarget->GetPosition(), 2.0f, 5.0f));
                }
                break;
            case EVENT_XANESH_TORMENT:
                warn("Dark Inquisitor Xanesh unleashes Torment!");
                castSpellAOE(SPELL_XANESH_TORMENT);
                scriptEvents.addEvent(EVENT_XANESH_TORMENT, 30000);
                break;
            case EVENT_XANESH_OBELISKS:
                raiseObelisks();
                scriptEvents.addEvent(EVENT_XANESH_OBELISKS, 65000);
                break;
            default:
                break;
        }
    }

private:
    // a fresh ring of obelisks around Azshara replaces the old one
    void raiseObelisks()
    {
        summons.despawnEntry(NPC_XANESH_RITUAL_OBELISK);

        Creature* pAzshara = findNearestCreature(NPC_XANESH_QUEEN_AZSHARA, 100.0f);
        LocationVector const centre = pAzshara != nullptr ? pAzshara->GetPosition() : getCreature()->GetPosition();
        for (uint8_t i = 0; i < 8; ++i)
            placeHelper(NPC_XANESH_RITUAL_OBELISK, nyalothaRingPoint(centre, 8.0f, 25.0f), 0);
    }
};

class NyalothaFlayedSoulAI : public NyalothaSpellAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaFlayedSoulAI(c); }
    explicit NyalothaFlayedSoulAI(Creature* pCreature) : NyalothaSpellAddAI(pCreature, SPELL_XANESH_SOUL_FLAY_CHANNEL, NyalothaAim::RandomPlayer, 1000, 0, 0, false) {}
};

class NyalothaRitualObeliskAI : public NyalothaHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaRitualObeliskAI(c); }
    explicit NyalothaRitualObeliskAI(Creature* pCreature) : NyalothaHelperAI(pCreature, SPELL_XANESH_OBELISK_AREA_DENIAL, 0, 500) {}
};

class NyalothaLargeRitualObeliskAI : public NyalothaHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaLargeRitualObeliskAI(c); }
    explicit NyalothaLargeRitualObeliskAI(Creature* pCreature) : NyalothaHelperAI(pCreature, SPELL_XANESH_LARGE_OBELISK_AREA_DENIAL, 0, 500) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Wrathion

class NyalothaWrathionAI : public NyalothaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaWrathionAI(c); }
    explicit NyalothaWrathionAI(Creature* pCreature) : NyalothaBossAI(pCreature, DATA_WRATHION) {}

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || pSummon->getEntry() != NPC_WRATHION_CRACKLING_SHARD)
            return;

        getCreature()->castSpellLoc(pSummon->GetPosition(), SPELL_WRATHION_PYROCLASTIC_FLOW, true);

        // the smoke lifts once every shard is broken
        if (mStage == 2 && mShardsLeft > 0 && --mShardsLeft == 0)
            scriptEvents.addEvent(EVENT_WRATHION_SMOKE_CLEARS, 100);
    }

protected:
    void onEngage() override
    {
        mCataclysms = 0;
        mSmokePhases = 0;
        mShardsLeft = 0;
        beginScorching();
    }

    void onDisengage() override { _removeAura(SPELL_WRATHION_SMOKE_AND_MIRRORS); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_WRATHION_TAIL_SWIPE:
                if (isAnyoneBehind())
                {
                    castSpellAOE(SPELL_WRATHION_TAIL_SWIPE);
                    scriptEvents.addEvent(EVENT_WRATHION_TAIL_SWIPE, 3000);
                }
                else
                {
                    scriptEvents.addEvent(EVENT_WRATHION_TAIL_SWIPE, 1000);
                }
                break;
            case EVENT_WRATHION_SEARING_BREATH:
                castSpellOnVictim(SPELL_WRATHION_SEARING_BREATH);
                scriptEvents.addEvent(EVENT_WRATHION_BREATH_LANDS, 100);
                scriptEvents.addEvent(EVENT_WRATHION_SEARING_BREATH, 8000);
                break;
            case EVENT_WRATHION_BREATH_LANDS:
                if (Unit* pVictim = currentVictim())
                {
                    castSpell(pVictim, SPELL_WRATHION_SEARING_BREATH_HIT, true);
                    castSpell(pVictim, SPELL_WRATHION_SEARING_ARMOR, true);
                }
                break;
            case EVENT_WRATHION_INCINERATION:
                castOnPlayers(SPELL_WRATHION_INCINERATION, 3);
                scriptEvents.addEvent(EVENT_WRATHION_INCINERATION, 18000);
                break;
            case EVENT_WRATHION_GALE_BLAST:
                castSpellAOE(SPELL_WRATHION_GALE_BLAST);
                scriptEvents.addEvent(EVENT_WRATHION_GALE_MISSILES, 1000);
                scriptEvents.addEvent(EVENT_WRATHION_GALE_BLAST, 40000);
                break;
            case EVENT_WRATHION_GALE_MISSILES:
                castOnPlayers(SPELL_WRATHION_GALE_BLAST_MISSILE, 5);
                break;
            case EVENT_WRATHION_BURNING_CATACLYSM:
                warn("Wrathion begins to cast Burning Cataclysm!");
                scriptEvents.resetEvents();
                setDormant(true);
                castSpellAOE(SPELL_WRATHION_BURNING_CATACLYSM);
                scriptEvents.addEvent(EVENT_WRATHION_CATACLYSM_ENDS, 100);
                break;
            case EVENT_WRATHION_CATACLYSM_ENDS:
                // every second cataclysm drains the bar
                if (++mCataclysms % 2 == 0)
                    beginSmokeAndMirrors();
                else
                    beginScorching();
                break;
            case EVENT_WRATHION_SCALES:
                getCreature()->castSpellLoc(wrathionRoomPoint(), SPELL_WRATHION_SCALES_OF_WRATHION, true);
                scriptEvents.addEvent(EVENT_WRATHION_SCALES, 10000);
                break;
            case EVENT_WRATHION_SMOKE_CLEARS:
                scriptEvents.resetEvents();
                warn("Wrathion emerges from the smoke!");
                _removeAura(SPELL_WRATHION_SMOKE_AND_MIRRORS);
                castSpellAOE(SPELL_WRATHION_SMOKE_AND_MIRRORS_END);
                scriptEvents.addEvent(EVENT_WRATHION_RETURNS, 5000);
                break;
            case EVENT_WRATHION_RETURNS:
                beginScorching();
                break;
            default:
                break;
        }
    }

private:
    void beginScorching()
    {
        enterStage(1);
        setDormant(false);
        scriptEvents.addEvent(EVENT_WRATHION_TAIL_SWIPE, 1000);
        scriptEvents.addEvent(EVENT_WRATHION_SEARING_BREATH, 8000);
        scriptEvents.addEvent(EVENT_WRATHION_INCINERATION, 28000);
        scriptEvents.addEvent(EVENT_WRATHION_GALE_BLAST, 45000);
        scriptEvents.addEvent(EVENT_WRATHION_BURNING_CATACLYSM, 60000);
    }

    void beginSmokeAndMirrors()
    {
        enterStage(2);
        ++mSmokePhases;
        warn("Wrathion vanishes into Smoke and Mirrors! Destroy the Crackling Shards!");
        castSpellOnSelf(SPELL_WRATHION_SMOKE_AND_MIRRORS, true);

        if (mSmokePhases >= 3)
            castSpellOnSelf(SPELL_WRATHION_BERSERK, true);

        mShardsLeft = 0;
        for (uint8_t i = 0; i < WrathionShardCount; ++i)
        {
            if (placeAdd(NPC_WRATHION_CRACKLING_SHARD, wrathionRoomPoint()) != nullptr)
                ++mShardsLeft;
        }

        if (isHeroic())
        {
            for (uint8_t i = 0; i < WrathionAssassinCount; ++i)
                callAdd(NPC_WRATHION_ASHWALKER_ASSASSIN, wrathionRoomPoint());
        }

        scriptEvents.addEvent(EVENT_WRATHION_SCALES, 3000);

        if (mShardsLeft == 0)
            scriptEvents.addEvent(EVENT_WRATHION_SMOKE_CLEARS, 100);
    }

    bool isAnyoneBehind()
    {
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr || !pObject->isPlayer())
                continue;

            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer->isAlive() && getRangeToObject(pPlayer) <= 15.0f && !getCreature()->isInFront(pPlayer))
                return true;
        }

        return false;
    }

    uint32_t mCataclysms = 0;
    uint32_t mSmokePhases = 0;
    uint32_t mShardsLeft = 0;
};

class NyalothaAshwalkerAssassinAI : public NyalothaSpellAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaAshwalkerAssassinAI(c); }
    explicit NyalothaAshwalkerAssassinAI(Creature* pCreature) : NyalothaSpellAddAI(pCreature, SPELL_WRATHION_SMOKE_SHROUD, NyalothaAim::Self, 2000, 0, 0, false) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ra-den the Despoiled

class NyalothaRadenAI : public NyalothaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaRadenAI(c); }
    explicit NyalothaRadenAI(Creature* pCreature) : NyalothaBossAI(pCreature, DATA_RADEN) {}

    void DoAction(int32_t action) override
    {
        if (!_isInCombat())
            return;

        if (action == ACTION_RADEN_VITA_ARRIVED)
        {
            castSpellOnSelf(SPELL_RADEN_VITA_EMPOWERED, true);
            castSpellAOE(SPELL_RADEN_UNLEASHED_VITA, true);
            castAtRandomPlayer(SPELL_RADEN_UNSTABLE_VITA);
            scriptEvents.addEvent(EVENT_RADEN_CRACKLING_STALKER, 5000);
        }
        else if (action == ACTION_RADEN_VOID_ARRIVED)
        {
            castSpellOnSelf(SPELL_RADEN_VOID_EMPOWERED, true);
            castAtRandomPlayer(SPELL_RADEN_UNSTABLE_VOID);
            scriptEvents.addEvent(EVENT_RADEN_VOID_HUNTER, 6000);
        }
    }

    // a slain essence leaves its power unclaimed
    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr)
            return;

        if (pSummon->getEntry() == NPC_RADEN_ESSENCE_OF_VITA)
            claimLoss(mUnclaimedVita, SPELL_RADEN_UNCLAIMED_VITA);
        else if (pSummon->getEntry() == NPC_RADEN_ESSENCE_OF_VOID)
            claimLoss(mUnclaimedVoid, SPELL_RADEN_UNCLAIMED_VOID);
    }

protected:
    void onEngage() override
    {
        mUnclaimedVita = 0;
        mUnclaimedVoid = 0;
        scriptEvents.addEvent(EVENT_RADEN_ESSENCES, 5000);
        scriptEvents.addEvent(EVENT_RADEN_NULLIFYING_STRIKE, 21000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == 1 && _getHealthPercent() <= 40)
            beginDespoiled();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RADEN_NULLIFYING_STRIKE:
                castSpellOnVictim(SPELL_RADEN_NULLIFYING_STRIKE);
                scriptEvents.addEvent(EVENT_RADEN_NULLIFYING_STRIKE, 20000);
                break;
            case EVENT_RADEN_ESSENCES:
                warn("Ra-den draws Vita and Void into the chamber!");
                placeAdd(NPC_RADEN_ESSENCE_OF_VITA, RadenVitaSpot);
                placeAdd(NPC_RADEN_ESSENCE_OF_VOID, RadenVoidSpot);
                scriptEvents.addEvent(EVENT_RADEN_ESSENCES, 60000);
                break;
            case EVENT_RADEN_CRACKLING_STALKER:
                callAdd(NPC_RADEN_CRACKLING_STALKER, RadenVitaSpot);
                break;
            case EVENT_RADEN_VOID_HUNTER:
                callAdd(NPC_RADEN_VOID_HUNTER, RadenVoidSpot);
                break;
            case EVENT_RADEN_GORGE_ENERGY:
                castSpellAOE(SPELL_RADEN_GORGE_ENERGY);
                scriptEvents.addEvent(EVENT_RADEN_GORGE_ENERGY, 30000);
                break;
            case EVENT_RADEN_VOID_ERUPTION:
                for (Unit* pTarget : pickPlayers(3))
                    getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_RADEN_VOID_ERUPTION, true);
                scriptEvents.addEvent(EVENT_RADEN_VOID_ERUPTION, 20000);
                break;
            case EVENT_RADEN_DECAYING_STRIKE:
                if (Unit* pVictim = currentVictim())
                {
                    castSpell(pVictim, SPELL_RADEN_DECAYING_STRIKE);
                    castSpell(pVictim, SPELL_RADEN_DECAYING_WOUND, true);
                }
                scriptEvents.addEvent(EVENT_RADEN_DECAYING_STRIKE, 21000);
                break;
            case EVENT_RADEN_CHARGED_BONDS:
                castOnPlayers(SPELL_RADEN_CHARGED_BONDS, isHeroic() ? 3 : 2);
                scriptEvents.addEvent(EVENT_RADEN_CHARGED_BONDS, 10000);
                break;
            case EVENT_RADEN_RUIN:
                warn("Ra-den unleashes Ruin!");
                castSpellAOE(SPELL_RADEN_RUIN);
                break;
            default:
                break;
        }
    }

private:
    void beginDespoiled()
    {
        enterStage(2);
        warn("Ra-den is despoiled and gorges on the remaining energy!");
        scriptEvents.addEvent(EVENT_RADEN_GORGE_ENERGY, 1000);
        scriptEvents.addEvent(EVENT_RADEN_CHARGED_BONDS, 10000);
        scriptEvents.addEvent(EVENT_RADEN_VOID_ERUPTION, 20000);
        scriptEvents.addEvent(EVENT_RADEN_DECAYING_STRIKE, 21000);
        scriptEvents.addEvent(EVENT_RADEN_RUIN, 25000);
    }

    // two unclaimed essences of a kind drive him into a rage
    void claimLoss(uint32_t& count, uint32_t spellId)
    {
        castSpellOnSelf(spellId, true);
        if (++count < 2)
            return;

        count = 0;
        castSpellOnSelf(SPELL_RADEN_OVERWHELMING_RAGE, true);
    }

    uint32_t mUnclaimedVita = 0;
    uint32_t mUnclaimedVoid = 0;
};

// walks to Ra-den and empowers him on arrival
class NyalothaRadenEssenceAI : public CreatureAIScript
{
public:
    NyalothaRadenEssenceAI(Creature* pCreature, int32_t arrivalAction) : CreatureAIScript(pCreature), mArrivalAction(arrivalAction) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mArrived || !isAlive() || !getCreature()->isSummon())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mStepLeftMs > diff)
        {
            mStepLeftMs -= diff;
            return;
        }

        mStepLeftMs = 1000;

        Creature* pRaden = findNearestCreature(BOSS_RADEN, 200.0f);
        if (pRaden == nullptr || !pRaden->isAlive())
            return;

        if (getRangeToObject(pRaden) <= 10.0f)
        {
            mArrived = true;
            if (pRaden->GetScript() != nullptr)
                pRaden->GetScript()->DoAction(mArrivalAction);

            despawn(100, 0);
            return;
        }

        moveTo(pRaden->GetPositionX(), pRaden->GetPositionY(), pRaden->GetPositionZ(), false);
    }

private:
    int32_t mArrivalAction;
    uint32_t mStepLeftMs = 1000;
    bool mArrived = false;
};

class NyalothaEssenceOfVitaAI : public NyalothaRadenEssenceAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaEssenceOfVitaAI(c); }
    explicit NyalothaEssenceOfVitaAI(Creature* pCreature) : NyalothaRadenEssenceAI(pCreature, ACTION_RADEN_VITA_ARRIVED) {}
};

class NyalothaEssenceOfVoidAI : public NyalothaRadenEssenceAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaEssenceOfVoidAI(c); }
    explicit NyalothaEssenceOfVoidAI(Creature* pCreature) : NyalothaRadenEssenceAI(pCreature, ACTION_RADEN_VOID_ARRIVED) {}
};

class NyalothaVoidHunterAI : public NyalothaSpellAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaVoidHunterAI(c); }
    explicit NyalothaVoidHunterAI(Creature* pCreature) : NyalothaSpellAddAI(pCreature, SPELL_RADEN_VOID_COLLAPSE, NyalothaAim::Area, 3000, 15000, 15000, false) {}
};

class NyalothaCracklingStalkerAI : public NyalothaSpellAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaCracklingStalkerAI(c); }
    explicit NyalothaCracklingStalkerAI(Creature* pCreature) : NyalothaSpellAddAI(pCreature, SPELL_RADEN_CHAIN_LIGHTNING, NyalothaAim::RandomPlayer, 3000, 15000, 15000, false) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Hivemind

class NyalothaHivemindAI : public NyalothaBossAI
{
public:
    NyalothaHivemindAI(Creature* pCreature, uint32_t partnerEntry, uint32_t controlSpellId, uint32_t firstControlMs)
        : NyalothaBossAI(pCreature, DATA_THE_HIVEMIND), mPartnerEntry(partnerEntry), mControlSpellId(controlSpellId), mFirstControlMs(firstControlMs) {}

    // neither falls while the other still stands
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || *damage == 0)
            return;

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health || !partnerHoldsOn())
            return;

        *damage = health > 1 ? health - 1 : 0;
        if (!mReconstituting)
        {
            mReconstituting = true;
            scriptEvents.addEvent(EVENT_HIVEMIND_RECONSTITUTION, 100);
        }
    }

protected:
    virtual void onHivemindEngage() {}
    virtual void onHivemindEvent(uint32_t /*eventId*/) {}

    void onEngage() override
    {
        mVeilCheckLeftMs = 2000;
        mReconstituting = false;
        scriptEvents.addEvent(EVENT_HIVEMIND_CONTROL, mFirstControlMs);
        onHivemindEngage();
    }

    void onDisengage() override { _removeAura(SPELL_HIVEMIND_SHADOW_VEIL); }

    void onCombatTick(uint32_t diff) override
    {
        if (mReconstituting && !isAtFloor(getCreature()))
            mReconstituting = false;

        if (!countDown(mVeilCheckLeftMs, diff))
            return;

        mVeilCheckLeftMs = 2000;

        // shrouded while the pair stays close
        Creature* pPartner = partner();
        bool const close = pPartner != nullptr && getRangeToObject(pPartner) <= HivemindVeilRange;
        if (close && !hasAura(SPELL_HIVEMIND_SHADOW_VEIL))
            castSpellOnSelf(SPELL_HIVEMIND_SHADOW_VEIL, true);
        else if (!close && hasAura(SPELL_HIVEMIND_SHADOW_VEIL))
            _removeAura(SPELL_HIVEMIND_SHADOW_VEIL);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HIVEMIND_CONTROL:
                takeControl();
                scriptEvents.addEvent(EVENT_HIVEMIND_CONTROL, HivemindSwapMs * 2);
                break;
            case EVENT_HIVEMIND_RECONSTITUTION:
                castSpellOnSelf(SPELL_HIVEMIND_DARK_RECONSTITUTION);
                break;
            default:
                onHivemindEvent(eventId);
                break;
        }
    }

    // the swarm takes on the will of the mind in control
    Creature* callSwarm(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = callAdd(entry, spot);
        if (pAdd == nullptr)
            return nullptr;

        NyalothaInstanceScript* pNyalotha = getNyalotha();
        uint32_t const controller = pNyalotha != nullptr ? pNyalotha->getHivemindController() : 0;
        if (controller == BOSS_TEKRIS)
            pAdd->castSpell(pAdd, SPELL_HIVEMIND_VOID_INFUSION, true);
        else if (controller == BOSS_KAZIR)
            pAdd->castSpell(pAdd, SPELL_HIVEMIND_REGENERATION, true);

        return pAdd;
    }

private:
    void takeControl()
    {
        castSpellAOE(mControlSpellId);
        if (NyalothaInstanceScript* pNyalotha = getNyalotha())
            pNyalotha->setHivemindController(getCreature()->getEntry());
    }

    Creature* partner()
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr)
            return nullptr;

        for (Creature* pMember : pInstance->getCreatureSetForEntry(mPartnerEntry))
        {
            if (pMember->isAlive())
                return pMember;
        }

        return nullptr;
    }

    bool partnerHoldsOn()
    {
        Creature* pPartner = partner();
        return pPartner != nullptr && !isAtFloor(pPartner);
    }

    uint32_t mPartnerEntry;
    uint32_t mControlSpellId;
    uint32_t mFirstControlMs;
    uint32_t mVeilCheckLeftMs = 0;
    bool mReconstituting = false;
};

class NyalothaTekrisAI : public NyalothaHivemindAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaTekrisAI(c); }
    explicit NyalothaTekrisAI(Creature* pCreature) : NyalothaHivemindAI(pCreature, BOSS_KAZIR, SPELL_TEKRIS_HIVEMIND_CONTROL, 3000) {}

protected:
    void onHivemindEngage() override
    {
        mEvolvingDroneGuid = 0;
        scriptEvents.addEvent(EVENT_TEKRIS_AQIR_DRONES, 10000);
        scriptEvents.addEvent(EVENT_TEKRIS_AQIR_DARTERS, 20000);
        scriptEvents.addEvent(EVENT_TEKRIS_ECHOING_VOID, 26000);
        scriptEvents.addEvent(EVENT_TEKRIS_NULLIFICATION_BLAST, 30000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_TEKRIS_ACCELERATED_EVOLUTION, 15000);
    }

    void onHivemindEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TEKRIS_AQIR_DRONES:
                for (uint8_t i = 0; i < 15; ++i)
                    callSwarm(NPC_HIVEMIND_AQIR_DRONE, aroundSelf(5.0f, 30.0f));
                scriptEvents.addEvent(EVENT_TEKRIS_AQIR_DRONES, HivemindSwapMs);
                break;
            case EVENT_TEKRIS_AQIR_DARTERS:
                for (uint8_t i = 0; i < 3; ++i)
                    callSwarm(NPC_HIVEMIND_AQIR_DARTER, aroundSelf(5.0f, 30.0f));
                scriptEvents.addEvent(EVENT_TEKRIS_AQIR_DARTERS, HivemindSwapMs);
                break;
            case EVENT_TEKRIS_ECHOING_VOID:
                warn("Tek'ris begins to cast Echoing Void!");
                castSpellAOE(SPELL_TEKRIS_ECHOING_VOID);
                scriptEvents.addEvent(EVENT_TEKRIS_ECHOING_VOID, 40000);
                break;
            case EVENT_TEKRIS_NULLIFICATION_BLAST:
                castSpellOnVictim(SPELL_TEKRIS_NULLIFICATION_BLAST);
                scriptEvents.addEvent(EVENT_TEKRIS_NULLIFICATION_BLAST, 30000);
                break;
            case EVENT_TEKRIS_ACCELERATED_EVOLUTION:
                if (Creature* pDrone = findNearestCreature(NPC_HIVEMIND_AQIR_DRONE, 100.0f))
                {
                    if (pDrone->isAlive())
                    {
                        castSpell(pDrone, SPELL_TEKRIS_ACCELERATED_EVOLUTION);
                        mEvolvingDroneGuid = pDrone->getGuid();
                        scriptEvents.addEvent(EVENT_TEKRIS_EVOLUTION_DONE, 100);
                    }
                }
                scriptEvents.addEvent(EVENT_TEKRIS_ACCELERATED_EVOLUTION, 65000);
                break;
            case EVENT_TEKRIS_EVOLUTION_DONE:
                evolveDrone();
                break;
            default:
                break;
        }
    }

private:
    // the evolved drone grows into a ravager
    void evolveDrone()
    {
        Creature* pDrone = getCreature()->getWorldMapCreature(mEvolvingDroneGuid);
        mEvolvingDroneGuid = 0;
        if (pDrone == nullptr || !pDrone->isAlive())
            return;

        LocationVector const spot = pDrone->GetPosition();
        pDrone->Despawn(0, 0);
        callSwarm(NPC_HIVEMIND_AQIR_RAVAGER, spot);
    }

    uint64_t mEvolvingDroneGuid = 0;
};

class NyalothaKazirAI : public NyalothaHivemindAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaKazirAI(c); }
    explicit NyalothaKazirAI(Creature* pCreature) : NyalothaHivemindAI(pCreature, BOSS_TEKRIS, SPELL_KAZIR_HIVEMIND_CONTROL, 3000 + HivemindSwapMs) {}

protected:
    void onHivemindEngage() override
    {
        scriptEvents.addEvent(EVENT_KAZIR_MIND_NUMBING_NOVA, Util::getRandomUInt(8000, 12000));
        scriptEvents.addEvent(EVENT_KAZIR_ACIDIC_AQIR, 45000);
    }

    void onHivemindEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KAZIR_MIND_NUMBING_NOVA:
                castSpellAOE(SPELL_KAZIR_MIND_NUMBING_NOVA);
                scriptEvents.addEvent(EVENT_KAZIR_MIND_NUMBING_NOVA, Util::getRandomUInt(8000, 12000));
                break;
            case EVENT_KAZIR_ACIDIC_AQIR:
                warn("Ka'zir sends out a swarm of Acidic Aqir!");
                for (uint8_t i = 0; i < 10; ++i)
                {
                    // they run inwards across the room
                    LocationVector spot = aroundSelf(10.0f, 40.0f);
                    spot.o += NyalothaPi;
                    placeHelper(NPC_HIVEMIND_ACIDIC_AQIR, spot, 10000);
                }
                scriptEvents.addEvent(EVENT_KAZIR_ACIDIC_AQIR, 60000);
                break;
            default:
                break;
        }
    }
};

// runs straight ahead, corroding whoever it passes
class NyalothaAcidicAqirAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaAcidicAqirAI(c); }
    explicit NyalothaAcidicAqirAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive() || !getCreature()->isSummon())
            return;

        if (!mRunning)
        {
            mRunning = true;
            castSpellOnSelf(SPELL_ACIDIC_AQIR_VISUAL, true);

            Creature* pSelf = getCreature();
            float const facing = pSelf->GetOrientation();
            moveTo(pSelf->GetPositionX() + 60.0f * std::cos(facing), pSelf->GetPositionY() + 60.0f * std::sin(facing), pSelf->GetPositionZ(), true);
        }

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mCheckLeftMs > diff)
        {
            mCheckLeftMs -= diff;
            return;
        }

        mCheckLeftMs = 500;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr || !pObject->isPlayer())
                continue;

            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer->isAlive() && getRangeToObject(pPlayer) <= 5.0f && !pPlayer->hasAurasWithId(SPELL_ACIDIC_AQIR_CORROSION))
                castSpell(pPlayer, SPELL_ACIDIC_AQIR_CORROSION, true);
        }
    }

private:
    uint32_t mCheckLeftMs = 500;
    bool mRunning = false;
};

class NyalothaAqirDroneAI : public NyalothaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaAqirDroneAI(c); }
    explicit NyalothaAqirDroneAI(Creature* pCreature) : NyalothaCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_AQIR_DRONE_MINDLESS, true);
        scriptEvents.addEvent(EVENT_AQIR_DRONE_FIXATE, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_AQIR_DRONE_FIXATE)
            return;

        if (Unit* pTarget = randomPlayer())
        {
            castSpell(pTarget, SPELL_AQIR_DRONE_FIXATE, true);
            fixateOn(pTarget);
        }
    }
};

class NyalothaAqirDarterAI : public NyalothaSpellAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaAqirDarterAI(c); }
    explicit NyalothaAqirDarterAI(Creature* pCreature) : NyalothaSpellAddAI(pCreature, SPELL_AQIR_DARTER_PSIONIC_RESONANCE, NyalothaAim::Area, 3000, 15000, 15000, false) {}
};

class NyalothaAqirRavagerAI : public NyalothaSpellAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaAqirRavagerAI(c); }
    explicit NyalothaAqirRavagerAI(Creature* pCreature) : NyalothaSpellAddAI(pCreature, SPELL_AQIR_RAVAGER_RAVAGE, NyalothaAim::Victim, 3000, 18000, 18000, false) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Prophet Skitra

class NyalothaSkitraAI : public NyalothaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaSkitraAI(c); }
    explicit NyalothaSkitraAI(Creature* pCreature) : NyalothaBossAI(pCreature, DATA_PROPHET_SKITRA) {}

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || pSummon->getEntry() != NPC_SKITRA_PROJECTION)
            return;

        getCreature()->castSpellLoc(pSummon->GetPosition(), SPELL_SKITRA_MINDQUAKE, true);

        // the prophet returns once every projection is gone
        if (mStage == 2 && mProjectionsLeft > 0 && --mProjectionsLeft == 0)
            scriptEvents.addEvent(EVENT_SKITRA_PROJECTIONS_END, 100);
    }

protected:
    void onEngage() override
    {
        mProjectionsDone = false;
        mProjectionsLeft = 0;
        mPsycheTargetGuid = 0;
        armBerserk(SPELL_SKITRA_RAGE_OF_THE_CORRUPTOR, 480000);
        scheduleProphecy();
    }

    void onDisengage() override { clearMinds(); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mProjectionsDone && mStage == 1 && _getHealthPercent() <= 80)
            beginProjections();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SKITRA_SHADOW_SHOCK:
                castSpellOnVictim(SPELL_SKITRA_SHADOW_SHOCK);
                scriptEvents.addEvent(EVENT_SKITRA_SHADOW_SHOCK, 2000);
                break;
            case EVENT_SKITRA_SHRED_PSYCHE:
                if (Unit* pTarget = castAtRandomPlayer(SPELL_SKITRA_SHRED_PSYCHE, true))
                {
                    mPsycheTargetGuid = pTarget->getGuid();
                    scriptEvents.addEvent(EVENT_SKITRA_PSYCHE_TEARS, 5000);
                }
                scriptEvents.addEvent(EVENT_SKITRA_SHRED_PSYCHE, 30000);
                break;
            case EVENT_SKITRA_PSYCHE_TEARS:
            {
                // the shredded psyche tears free where its owner stands
                Unit* pTarget = getCreature()->getWorldMapUnit(mPsycheTargetGuid);
                mPsycheTargetGuid = 0;
                callAdd(NPC_SKITRA_SHREDDED_PSYCHE, pTarget != nullptr ? pTarget->GetPosition() : aroundSelf(5.0f, 15.0f));
                break;
            }
            case EVENT_SKITRA_DARK_RITUAL:
                castOnAllPlayers(SPELL_SKITRA_DARK_RITUAL);
                scriptEvents.addEvent(EVENT_SKITRA_DARK_RITUAL, 5000);
                break;
            case EVENT_SKITRA_PROJECTIONS_END:
                warn("Prophet Skitra reappears!");
                setDormant(false);
                clearMinds();
                enterStage(3);
                scheduleProphecy();
                break;
            default:
                break;
        }
    }

private:
    void scheduleProphecy()
    {
        scriptEvents.addEvent(EVENT_SKITRA_SHADOW_SHOCK, 3000);
        scriptEvents.addEvent(EVENT_SKITRA_SHRED_PSYCHE, 15000);
    }

    void beginProjections()
    {
        mProjectionsDone = true;
        enterStage(2);
        warn("Prophet Skitra splits into Illusionary Projections!");
        castSpellOnSelf(SPELL_SKITRA_ILLUSIONARY_PROJECTION, true);
        setDormant(true);

        // half of the raid sees one set of images, half the other
        std::vector<Unit*> const players = pickPlayers(NyalothaRaidCap);
        for (size_t i = 0; i < players.size(); ++i)
            castSpell(players[i], i % 2 == 0 ? SPELL_SKITRA_CLOUDED_MIND : SPELL_SKITRA_TWISTED_MIND, true);

        mProjectionsLeft = 0;
        for (uint8_t i = 0; i < SkitraProjectionCount; ++i)
        {
            if (placeAdd(NPC_SKITRA_PROJECTION, nyalothaRingPoint(SkitraProjectionCentre, 10.0f, 35.0f)) != nullptr)
                ++mProjectionsLeft;
        }

        scriptEvents.addEvent(EVENT_SKITRA_DARK_RITUAL, 5000);

        if (mProjectionsLeft == 0)
            scriptEvents.addEvent(EVENT_SKITRA_PROJECTIONS_END, 100);
    }

    void clearMinds()
    {
        _removeAuraOnPlayers(SPELL_SKITRA_CLOUDED_MIND);
        _removeAuraOnPlayers(SPELL_SKITRA_TWISTED_MIND);
    }

    uint64_t mPsycheTargetGuid = 0;
    uint32_t mProjectionsLeft = 0;
    bool mProjectionsDone = false;
};

class NyalothaShreddedPsycheAI : public NyalothaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaShreddedPsycheAI(c); }
    explicit NyalothaShreddedPsycheAI(Creature* pCreature) : NyalothaCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_SHREDDED_PSYCHE_OUTBURST, 500); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SHREDDED_PSYCHE_OUTBURST:
                castSpellAOE(SPELL_SHREDDED_PSYCHE_PSYCHIC_OUTBURST);
                scriptEvents.addEvent(EVENT_SHREDDED_PSYCHE_REVERBERATIONS, 100);
                break;
            case EVENT_SHREDDED_PSYCHE_REVERBERATIONS:
                castSpellAOE(SPELL_SHREDDED_PSYCHE_REVERBERATIONS);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shad'har the Insatiable

class NyalothaShadharAI : public NyalothaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaShadharAI(c); }
    explicit NyalothaShadharAI(Creature* pCreature) : NyalothaBossAI(pCreature, DATA_SHADHAR) {}

protected:
    void onEngage() override { scheduleFeeding(); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == 1 && _getHealthPercent() <= 66)
        {
            enterStage(2);
            warn("Shad'har devours an Entropic Carcass!");
            castSpellAOE(SPELL_SHADHAR_EAT_ENTROPIC_CARCASS);
            scheduleFeeding();
        }
        else if (mStage == 2 && _getHealthPercent() <= 33)
        {
            enterStage(3);
            warn("Shad'har devours a Noxious Carcass and goes into a frenzy!");
            castSpellAOE(SPELL_SHADHAR_EAT_NOXIOUS_CARCASS);
            castSpellOnSelf(SPELL_SHADHAR_FRENZY, true);
            scheduleFeeding();
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SHADHAR_BREATH:
                castSpellOnVictim(breathForStage());
                scriptEvents.addEvent(EVENT_SHADHAR_BREATH, ShadharBreathFillMs);
                break;
            case EVENT_SHADHAR_CRUSH:
                castSpellOnVictim(SPELL_SHADHAR_CRUSH);
                scriptEvents.addEvent(EVENT_SHADHAR_DISSOLVE, 100);
                scriptEvents.addEvent(EVENT_SHADHAR_CRUSH, Util::getRandomUInt(30000, 35000));
                break;
            case EVENT_SHADHAR_DISSOLVE:
                castSpellOnVictim(SPELL_SHADHAR_DISSOLVE);
                break;
            case EVENT_SHADHAR_UMBRAL_ERUPTION:
                for (uint8_t i = 0; i < 15; ++i)
                    getCreature()->castSpellLoc(aroundSelf(3.0f, 20.0f), SPELL_SHADHAR_UMBRAL_ERUPTION, true);
                scriptEvents.addEvent(EVENT_SHADHAR_UMBRAL_ERUPTION, 20000);
                break;
            case EVENT_SHADHAR_DEBILITATING_SPIT:
                castAtRandomPlayer(SPELL_SHADHAR_DEBILITATING_SPIT);
                scriptEvents.addEvent(EVENT_SHADHAR_DEBILITATING_SPIT, 35000);
                break;
            case EVENT_SHADHAR_LIVING_MIASMA:
                callAdd(NPC_SHADHAR_LIVING_MIASMA, aroundSelf(10.0f, 30.0f));
                scriptEvents.addEvent(EVENT_SHADHAR_LIVING_MIASMA, 30000);
                break;
            case EVENT_SHADHAR_MANTLE:
                if (mStage == 1)
                {
                    castOnAllPlayers(SPELL_SHADHAR_UMBRAL_MANTLE);
                    scriptEvents.addEvent(EVENT_SHADHAR_MANTLE, 20000);
                }
                else if (mStage == 2)
                {
                    castOnAllPlayers(SPELL_SHADHAR_ENTROPIC_MANTLE);
                    scriptEvents.addEvent(EVENT_SHADHAR_MANTLE, 20000);
                }
                else
                {
                    castOnAllPlayers(SPELL_SHADHAR_NOXIOUS_MANTLE);
                }
                break;
            default:
                break;
        }
    }

private:
    // each carcass starts the hunger over
    void scheduleFeeding()
    {
        scriptEvents.addEvent(EVENT_SHADHAR_CRUSH, 5000);
        scriptEvents.addEvent(EVENT_SHADHAR_UMBRAL_ERUPTION, 9500);
        scriptEvents.addEvent(EVENT_SHADHAR_DEBILITATING_SPIT, 10000);
        scriptEvents.addEvent(EVENT_SHADHAR_LIVING_MIASMA, 15000);
        scriptEvents.addEvent(EVENT_SHADHAR_MANTLE, mStage == 3 ? 4000 : 20000);
        scriptEvents.addEvent(EVENT_SHADHAR_BREATH, ShadharBreathFillMs);
    }

    uint32_t breathForStage() const
    {
        if (mStage == 2)
            return SPELL_SHADHAR_ENTROPIC_BREATH;
        if (mStage == 3)
            return SPELL_SHADHAR_BUBBLING_BREATH;

        return SPELL_SHADHAR_UMBRAL_BREATH;
    }
};

// fixates one player and bursts on reaching them
class NyalothaLivingMiasmaAI : public NyalothaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaLivingMiasmaAI(c); }
    explicit NyalothaLivingMiasmaAI(Creature* pCreature) : NyalothaCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        mFixateGuid = 0;
        scriptEvents.addEvent(EVENT_LIVING_MIASMA_FIXATE, 500);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mFixateGuid == 0 || !countDown(mCheckLeftMs, diff))
            return;

        mCheckLeftMs = 500;

        Unit* pTarget = getCreature()->getWorldMapUnit(mFixateGuid);
        if (pTarget == nullptr || !pTarget->isAlive())
        {
            mFixateGuid = 0;
            return;
        }

        if (getRangeToObject(pTarget) <= 5.0f)
        {
            castSpell(pTarget, SPELL_LIVING_MIASMA_SLURRY_OUTBURST, true);
            pTarget->removeAllAurasById(SPELL_LIVING_MIASMA_FIXATE);
            mFixateGuid = 0;
        }
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_LIVING_MIASMA_FIXATE)
            return;

        if (Unit* pTarget = randomPlayer())
        {
            castSpell(pTarget, SPELL_LIVING_MIASMA_FIXATE, true);
            fixateOn(pTarget);
            mFixateGuid = pTarget->getGuid();
            mCheckLeftMs = 500;
        }
    }

private:
    uint64_t mFixateGuid = 0;
    uint32_t mCheckLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Vexiona

class NyalothaVexionaAI : public NyalothaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaVexionaAI(c); }
    explicit NyalothaVexionaAI(Creature* pCreature) : NyalothaBossAI(pCreature, DATA_VEXIONA) {}

protected:
    void onEngage() override
    {
        armBerserk(SPELL_VEXIONA_BERSERK, 720000);
        scriptEvents.addEvent(EVENT_VEXIONA_VOID_ASCENDANT, 1000);
        scriptEvents.addEvent(EVENT_VEXIONA_ENCROACHING_SHADOWS, 13000);
        scriptEvents.addEvent(EVENT_VEXIONA_DESPAIR, 15000);
        scriptEvents.addEvent(EVENT_VEXIONA_TWILIGHT_BREATH, 18000);
        scriptEvents.addEvent(EVENT_VEXIONA_DARK_GATEWAY, 32000);
        scriptEvents.addEvent(EVENT_VEXIONA_HEART_OF_DARKNESS, VexionaHeartFillMs);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage != 1 || _getHealthPercent() > 40)
            return;

        // the cult is spent, she fights on her own
        mStage = 2;
        warn("Vexiona unleashes her full power!");
        castSpellOnSelf(SPELL_VEXIONA_TERRIFYING_PRESENCE, true);
        scriptEvents.removeEvent(EVENT_VEXIONA_ENCROACHING_SHADOWS);
        scriptEvents.removeEvent(EVENT_VEXIONA_DARK_GATEWAY);
        scriptEvents.addEvent(EVENT_VEXIONA_DESOLATION, 25000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VEXIONA_VOID_ASCENDANT:
                callAdd(NPC_VEXIONA_VOID_ASCENDANT, aroundSelf(3.0f, 6.0f));
                break;
            case EVENT_VEXIONA_ENCROACHING_SHADOWS:
                castOnPlayers(SPELL_VEXIONA_ENCROACHING_SHADOWS, 6);
                scriptEvents.addEvent(EVENT_VEXIONA_ENCROACHING_SHADOWS, 35000);
                break;
            case EVENT_VEXIONA_DESPAIR:
                if (Unit* pVictim = currentVictim())
                    castSpell(pVictim, SPELL_VEXIONA_DESPAIR, true);
                scriptEvents.addEvent(EVENT_VEXIONA_DESPAIR, 30000);
                break;
            case EVENT_VEXIONA_TWILIGHT_BREATH:
                castSpellOnVictim(SPELL_VEXIONA_TWILIGHT_BREATH);
                scriptEvents.addEvent(EVENT_VEXIONA_TWILIGHT_BREATH, 18000);
                break;
            case EVENT_VEXIONA_DARK_GATEWAY:
                openGateway();
                scriptEvents.addEvent(EVENT_VEXIONA_DARK_GATEWAY, 32000);
                break;
            case EVENT_VEXIONA_HEART_OF_DARKNESS:
                warn("Vexiona begins to cast Heart of Darkness! Spread out!");
                castSpellAOE(SPELL_VEXIONA_HEART_OF_DARKNESS);
                scriptEvents.addEvent(EVENT_VEXIONA_HEART_OF_DARKNESS, VexionaHeartFillMs);
                break;
            case EVENT_VEXIONA_DESOLATION:
                castAtRandomPlayer(SPELL_VEXIONA_DESOLATION, false, false);
                scriptEvents.addEvent(EVENT_VEXIONA_DESOLATION, 25000);
                break;
            default:
                break;
        }
    }

private:
    // a gateway brings either a band of cultists or a ritualist
    void openGateway()
    {
        LocationVector const gate = aroundSelf(10.0f, 30.0f);
        placeHelper(NPC_VEXIONA_DARK_GATEWAY, gate, 5000);

        if (Util::getRandomUInt(1) == 0)
        {
            for (uint8_t i = 0; i < 3; ++i)
                callAdd(NPC_VEXIONA_FANATICAL_CULTIST, nyalothaRingPoint(gate, 1.0f, 5.0f));
        }
        else
        {
            callAdd(NPC_VEXIONA_SPELLBOUND_RITUALIST, nyalothaRingPoint(gate, 1.0f, 5.0f));
        }

        if (isHeroic())
            callAdd(NPC_VEXIONA_SINISTER_SOULCARVER, nyalothaRingPoint(gate, 1.0f, 5.0f));
    }
};

class NyalothaVoidAscendantAI : public NyalothaSpellAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaVoidAscendantAI(c); }
    explicit NyalothaVoidAscendantAI(Creature* pCreature) : NyalothaSpellAddAI(pCreature, SPELL_VOID_ASCENDANT_ANNIHILATION, NyalothaAim::Victim, 3000, 20000, 20000, false) {}
};

class NyalothaSpellboundRitualistAI : public NyalothaSpellAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaSpellboundRitualistAI(c); }
    explicit NyalothaSpellboundRitualistAI(Creature* pCreature) : NyalothaSpellAddAI(pCreature, SPELL_SPELLBOUND_RITUALIST_VOID_BOLT, NyalothaAim::RandomPlayer, 3000, 3000, 3500, false) {}
};

class NyalothaDarkGatewayAI : public NyalothaHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaDarkGatewayAI(c); }
    explicit NyalothaDarkGatewayAI(Creature* pCreature) : NyalothaHelperAI(pCreature, SPELL_DARK_GATEWAY_VISUAL, SPELL_DARK_GATEWAY, 500) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Carapace of N'Zoth

class NyalothaCarapaceAI : public NyalothaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaCarapaceAI(c); }
    explicit NyalothaCarapaceAI(Creature* pCreature) : NyalothaBossAI(pCreature, DATA_CARAPACE_OF_NZOTH) {}

protected:
    void onEngage() override
    {
        mBlastTargetGuid = 0;
        armBerserk(SPELL_CARAPACE_BERSERK, 780000);
        scriptEvents.addEvent(EVENT_CARAPACE_MANDIBLE_SLAM, 3000);
        scriptEvents.addEvent(EVENT_CARAPACE_MENTAL_DECAY, 9000);
        scriptEvents.addEvent(EVENT_CARAPACE_MADNESS_BOMB, 12000);
        scriptEvents.addEvent(EVENT_CARAPACE_ANTIGENS, 16000);
        scriptEvents.addEvent(EVENT_CARAPACE_ADAPTIVE_MEMBRANE, 18000);
        scriptEvents.addEvent(EVENT_CARAPACE_GROWTH_TENTACLE, 30000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_CARAPACE_GAZE_OF_MADNESS, 40000);
    }

    void onDisengage() override { _removeAura(SPELL_CARAPACE_SYNTHESIS); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == 1 && _getHealthPercent() <= 50)
            beginUnleashedMadness();
        else if (mStage == 2 && _getHealthPercent() <= 40 && !isGrowthAlive())
            beginConvergence();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CARAPACE_MANDIBLE_SLAM:
                castSpellOnVictim(SPELL_CARAPACE_MANDIBLE_SLAM);
                scriptEvents.addEvent(EVENT_CARAPACE_BLACK_SCAR, 100);
                scriptEvents.addEvent(EVENT_CARAPACE_MANDIBLE_SLAM, CarapaceSlamFillMs);
                break;
            case EVENT_CARAPACE_BLACK_SCAR:
                if (Unit* pVictim = currentVictim())
                    castSpell(pVictim, SPELL_CARAPACE_BLACK_SCAR, true);
                break;
            case EVENT_CARAPACE_MENTAL_DECAY:
                castSpellAOE(SPELL_CARAPACE_MENTAL_DECAY);
                scriptEvents.addEvent(EVENT_CARAPACE_DECAY_SPREADS, 100);
                scriptEvents.addEvent(EVENT_CARAPACE_MENTAL_DECAY, 30000);
                break;
            case EVENT_CARAPACE_DECAY_SPREADS:
                castOnPlayers(SPELL_CARAPACE_MENTAL_DECAY_HIT, 3);
                break;
            case EVENT_CARAPACE_MADNESS_BOMB:
                warn("Fury of N'Zoth marks players with Madness Bomb!");
                castSpellAOE(SPELL_CARAPACE_MADNESS_BOMB);
                scriptEvents.addEvent(EVENT_CARAPACE_BOMB_MARKS, 100);
                scriptEvents.addEvent(EVENT_CARAPACE_MADNESS_BOMB, 25000);
                break;
            case EVENT_CARAPACE_BOMB_MARKS:
                castOnPlayers(SPELL_CARAPACE_MADNESS_BOMB_MARK, 3);
                break;
            case EVENT_CARAPACE_ANTIGENS:
                for (uint8_t i = 0; i < 15; ++i)
                    callAdd(NPC_CARAPACE_NIGHTMARE_ANTIGEN, aroundSelf(10.0f, 30.0f));
                break;
            case EVENT_CARAPACE_ADAPTIVE_MEMBRANE:
                castSpellAOE(SPELL_CARAPACE_ADAPTIVE_MEMBRANE);
                scriptEvents.addEvent(EVENT_CARAPACE_ADAPTIVE_MEMBRANE, 35000);
                break;
            case EVENT_CARAPACE_GROWTH_TENTACLE:
                warn("A Growth-Covered Tentacle bursts from the flesh!");
                placeHelper(NPC_CARAPACE_THRASHING_TENTACLE, CarapaceTentacleSpots[Util::getRandomUInt(3)], 20000);
                scriptEvents.addEvent(EVENT_CARAPACE_GROWTH_TENTACLE, 45000);
                break;
            case EVENT_CARAPACE_GAZE_OF_MADNESS:
                callAdd(NPC_CARAPACE_GAZE_OF_MADNESS, CarapaceGazeSpot);
                scriptEvents.addEvent(EVENT_CARAPACE_GAZE_OF_MADNESS, 22500);
                break;
            case EVENT_CARAPACE_ETERNAL_DARKNESS:
                warn("Fury of N'Zoth begins to cast Eternal Darkness!");
                castSpellAOE(SPELL_CARAPACE_ETERNAL_DARKNESS);
                scriptEvents.addEvent(EVENT_CARAPACE_ETERNAL_DARKNESS, 55000);
                break;
            case EVENT_CARAPACE_INSANITY_BOMB:
                castOnAllPlayers(SPELL_CARAPACE_INSANITY_BOMB);
                scriptEvents.addEvent(EVENT_CARAPACE_INSANITY_BOMB, 25000);
                break;
            case EVENT_CARAPACE_OCCIPITAL_BLAST:
                if (Unit* pTarget = randomPlayer())
                {
                    warn("Fury of N'Zoth begins to cast Occipital Blast!");
                    castSpell(pTarget, SPELL_CARAPACE_OCCIPITAL_BLAST);
                    mBlastTargetGuid = pTarget->getGuid();
                    scriptEvents.addEvent(EVENT_CARAPACE_BLAST_LANDS, 100);
                }
                scriptEvents.addEvent(EVENT_CARAPACE_OCCIPITAL_BLAST, 60000);
                break;
            case EVENT_CARAPACE_BLAST_LANDS:
                if (Unit* pTarget = getCreature()->getWorldMapUnit(mBlastTargetGuid))
                    castSpell(pTarget, SPELL_CARAPACE_OCCIPITAL_BLAST_HIT, true);
                mBlastTargetGuid = 0;
                break;
            default:
                break;
        }
    }

private:
    // the inside of the carapace, fought in place
    void beginUnleashedMadness()
    {
        enterStage(2);
        warn("The Carapace of N'Zoth splits open!");
        if (isGrowthAlive())
            castSpellOnSelf(SPELL_CARAPACE_SYNTHESIS, true);

        scriptEvents.addEvent(EVENT_CARAPACE_MENTAL_DECAY, 9000);
        scriptEvents.addEvent(EVENT_CARAPACE_MADNESS_BOMB, 12000);
        scriptEvents.addEvent(EVENT_CARAPACE_MANDIBLE_SLAM, CarapaceSlamFillMs);
        scriptEvents.addEvent(EVENT_CARAPACE_ADAPTIVE_MEMBRANE, 18000);
        scriptEvents.addEvent(EVENT_CARAPACE_ETERNAL_DARKNESS, 20000);
    }

    void beginConvergence()
    {
        enterStage(3);
        warn("Fury of N'Zoth enrages as the synthesis fails!");
        _removeAura(SPELL_CARAPACE_SYNTHESIS);

        scriptEvents.addEvent(EVENT_CARAPACE_MANDIBLE_SLAM, 3000);
        scriptEvents.addEvent(EVENT_CARAPACE_MENTAL_DECAY, 9000);
        scriptEvents.addEvent(EVENT_CARAPACE_INSANITY_BOMB, 12000);
        scriptEvents.addEvent(EVENT_CARAPACE_ANTIGENS, 16000);
        scriptEvents.addEvent(EVENT_CARAPACE_ADAPTIVE_MEMBRANE, 18000);
        scriptEvents.addEvent(EVENT_CARAPACE_OCCIPITAL_BLAST, 22000);
    }

    bool isGrowthAlive()
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr)
            return false;

        for (Creature* pGrowth : pInstance->getCreatureSetForEntry(NPC_CARAPACE_SYNTHESIS_GROWTH))
        {
            if (pGrowth->isAlive())
                return true;
        }

        return false;
    }

    uint64_t mBlastTargetGuid = 0;
};

class NyalothaThrashingTentacleAI : public NyalothaHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaThrashingTentacleAI(c); }
    explicit NyalothaThrashingTentacleAI(Creature* pCreature) : NyalothaHelperAI(pCreature, 0, SPELL_THRASHING_TENTACLE_SHADOW, 1000) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Drest'agath

class NyalothaDrestagathAI : public NyalothaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaDrestagathAI(c); }
    explicit NyalothaDrestagathAI(Creature* pCreature) : NyalothaBossAI(pCreature, DATA_DRESTAGATH)
    {
        mImmobile = true;
    }

    // every severed limb weakens the body
    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || !isLimb(pSummon->getEntry()))
            return;

        castSpellOnSelf(SPELL_DRESTAGATH_DISMEMBERMENT, true);
    }

protected:
    void onEngage() override
    {
        mNextWave = 0;
        armBerserk(SPELL_DRESTAGATH_BERSERK, 600000);
        castSpellOnSelf(SPELL_DRESTAGATH_ABERRANT_REGENERATION, true);
        callAdd(NPC_DRESTAGATH_EYE, DrestagathEyeSpot);

        scriptEvents.addEvent(EVENT_DRESTAGATH_VOLATILE_SEED, 3000);
        scriptEvents.addEvent(EVENT_DRESTAGATH_ENTROPIC_CRASH, 15000);
        scriptEvents.addEvent(EVENT_DRESTAGATH_LIMB_WAVE, DrestagathWaveTimes[0]);
        scriptEvents.addEvent(EVENT_DRESTAGATH_VOID_GLARE, 45000);
        scriptEvents.addEvent(EVENT_DRESTAGATH_THROES_OF_AGONY, DrestagathAgonyFillMs);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DRESTAGATH_VOLATILE_SEED:
                castSpellOnVictim(SPELL_DRESTAGATH_VOLATILE_SEED);
                scriptEvents.addEvent(EVENT_DRESTAGATH_VOLATILE_SEED, 18000);
                break;
            case EVENT_DRESTAGATH_ENTROPIC_CRASH:
                warn("Drest'agath begins to cast Entropic Crash!");
                castSpellAOE(SPELL_DRESTAGATH_ENTROPIC_CRASH);
                scriptEvents.addEvent(EVENT_DRESTAGATH_ENTROPIC_CRASH, 30000);
                break;
            case EVENT_DRESTAGATH_VOID_GLARE:
                castSpellAOE(SPELL_DRESTAGATH_VOID_GLARE);
                castAtRandomPlayer(SPELL_DRESTAGATH_VOID_GLARE_BEAM);
                tellLimbs(NPC_DRESTAGATH_EYE, ACTION_DRESTAGATH_GLARE);
                scriptEvents.addEvent(EVENT_DRESTAGATH_VOID_GLARE, 55000);
                break;
            case EVENT_DRESTAGATH_LIMB_WAVE:
                raiseWave(mNextWave);
                if (++mNextWave < DrestagathWaveCount)
                    scriptEvents.addEvent(EVENT_DRESTAGATH_LIMB_WAVE, DrestagathWaveTimes[mNextWave] - DrestagathWaveTimes[mNextWave - 1]);
                break;
            case EVENT_DRESTAGATH_THROES_OF_AGONY:
                warn("Drest'agath writhes in Throes of Agony!");
                castSpellOnSelf(SPELL_DRESTAGATH_THROES_OF_AGONY, true);
                tellLimbs(NPC_DRESTAGATH_EYE, ACTION_DRESTAGATH_AGONY);
                tellLimbs(NPC_DRESTAGATH_MAW, ACTION_DRESTAGATH_AGONY);
                scriptEvents.addEvent(EVENT_DRESTAGATH_THROES_OF_AGONY, DrestagathAgonyFillMs);
                break;
            default:
                break;
        }
    }

private:
    static bool isLimb(uint32_t entry)
    {
        return entry == NPC_DRESTAGATH_EYE || entry == NPC_DRESTAGATH_TENTACLE || entry == NPC_DRESTAGATH_MAW;
    }

    void raiseWave(size_t wave)
    {
        for (NyalothaLimbPlacement const& limb : DrestagathLimbs)
        {
            if (limb.wave == wave)
                callAdd(limb.entry, limb.spot);
        }
    }

    void tellLimbs(uint32_t entry, int32_t action)
    {
        for (Creature* pLimb : livingSummons(entry))
            tell(pLimb, action);
    }

    size_t mNextWave = 0;
};

// rooted limb with a main spell, a heroic aura and a reaction to the boss
class NyalothaDrestagathLimbAI : public NyalothaSpellAddAI
{
public:
    NyalothaDrestagathLimbAI(Creature* pCreature, uint32_t spellId, NyalothaAim aim, uint32_t minRepeatMs, uint32_t maxRepeatMs, uint32_t heroicAuraId, uint32_t agonySpellId, NyalothaAim agonyAim)
        : NyalothaSpellAddAI(pCreature, spellId, aim, 3000, minRepeatMs, maxRepeatMs, true), mHeroicAuraId(heroicAuraId), mAgonySpellId(agonySpellId), mAgonyAim(agonyAim) {}

    void DoAction(int32_t action) override
    {
        if (!_isInCombat())
            return;

        if (action == ACTION_DRESTAGATH_AGONY && mAgonySpellId != 0)
            aimSpell(mAgonySpellId, mAgonyAim, true);
        else if (action == ACTION_DRESTAGATH_GLARE)
            castAtRandomPlayer(SPELL_DRESTAGATH_VOID_GLARE_BEAM);
    }

protected:
    void onAddEngage() override
    {
        if (mHeroicAuraId != 0 && isHeroic())
            castSpellOnSelf(mHeroicAuraId, true);
    }

private:
    uint32_t mHeroicAuraId;
    uint32_t mAgonySpellId;
    NyalothaAim mAgonyAim;
};

class NyalothaEyeOfDrestagathAI : public NyalothaDrestagathLimbAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaEyeOfDrestagathAI(c); }
    explicit NyalothaEyeOfDrestagathAI(Creature* pCreature)
        : NyalothaDrestagathLimbAI(pCreature, SPELL_DRESTAGATH_EYE_MIND_FLAY, NyalothaAim::RandomPlayer, 6000, 6000, 0, SPELL_DRESTAGATH_EYE_ERRANT_BLAST, NyalothaAim::RandomPlayer) {}
};

class NyalothaTentacleOfDrestagathAI : public NyalothaDrestagathLimbAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaTentacleOfDrestagathAI(c); }
    explicit NyalothaTentacleOfDrestagathAI(Creature* pCreature)
        : NyalothaDrestagathLimbAI(pCreature, SPELL_DRESTAGATH_TENTACLE_CRUSHING_SLAM, NyalothaAim::Victim, 15000, 15000, SPELL_DRESTAGATH_TENTACLE_VOID_MIASMA, 0, NyalothaAim::Area) {}
};

class NyalothaMawOfDrestagathAI : public NyalothaDrestagathLimbAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaMawOfDrestagathAI(c); }
    explicit NyalothaMawOfDrestagathAI(Creature* pCreature)
        : NyalothaDrestagathLimbAI(pCreature, SPELL_DRESTAGATH_MAW_ACID_SPLASH, NyalothaAim::RandomPlayer, 3500, 4000, SPELL_DRESTAGATH_MAW_MUTTERINGS, SPELL_DRESTAGATH_MAW_SPINE_ERUPTION, NyalothaAim::Area) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Il'gynoth, Corruption Reborn

class NyalothaIlgynothAI : public NyalothaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaIlgynothAI(c); }
    explicit NyalothaIlgynothAI(Creature* pCreature) : NyalothaBossAI(pCreature, DATA_ILGYNOTH)
    {
        mImmobile = true;
    }

    // held at the brink while organs remain, submerges instead of dying
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || *damage == 0)
            return;

        if (mStage == 2)
        {
            *damage = 0;
            return;
        }

        if (mOrgansSlain >= IlgynothOrganCount)
            return;

        uint32_t const health = getCreature()->getHealth();
        uint32_t const brink = static_cast<uint32_t>(static_cast<uint64_t>(getCreature()->getMaxHealth()) * 2 / 100);
        uint32_t const allowed = health > brink ? health - brink : 0;
        if (*damage < allowed)
            return;

        *damage = allowed;
        if (!mSubmergePending)
        {
            mSubmergePending = true;
            scriptEvents.addEvent(EVENT_ILGYNOTH_SUBMERGE, 100);
        }
    }

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr)
            return;

        if (pSummon->getEntry() == NPC_ILGYNOTH_BLOOD_OF_NYALOTHA)
        {
            getCreature()->castSpellLoc(pSummon->GetPosition(), SPELL_BLOOD_OF_NYALOTHA_HEMORRHAGE, true);
            return;
        }

        if (pSummon->getEntry() != NPC_ILGYNOTH_ORGAN_OF_CORRUPTION || mStage != 2)
            return;

        // one organ per dive, the others close up again
        ++mOrgansSlain;
        warn("An Organ of Corruption has been destroyed!");
        for (Creature* pOrgan : livingSummons(NPC_ILGYNOTH_ORGAN_OF_CORRUPTION))
            tell(pOrgan, ACTION_ORGAN_SEAL);

        resumeGaze(3000);
    }

protected:
    void onEngage() override
    {
        mOrgansSlain = 0;
        mOrgansPlaced = false;
        armBerserk(SPELL_ILGYNOTH_BERSERK, 600000);
        resumeGaze(18000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ILGYNOTH_EYE_OF_NZOTH:
                castSpellOnVictim(SPELL_ILGYNOTH_EYE_OF_NZOTH);
                scriptEvents.addEvent(EVENT_ILGYNOTH_EYE_OF_NZOTH, 25000);
                break;
            case EVENT_ILGYNOTH_CORRUPTORS_GAZE:
                castAtRandomPlayer(SPELL_ILGYNOTH_CORRUPTORS_GAZE, true);
                scriptEvents.addEvent(EVENT_ILGYNOTH_CORRUPTORS_GAZE, 35000);
                break;
            case EVENT_ILGYNOTH_CURSED_BLOOD:
                castOnPlayers(SPELL_ILGYNOTH_CURSED_BLOOD, 3);
                scriptEvents.addEvent(EVENT_ILGYNOTH_CURSED_BLOOD, 18000);
                break;
            case EVENT_ILGYNOTH_SUBMERGE:
                submerge();
                break;
            case EVENT_ILGYNOTH_CLOTTED_CORRUPTION:
                for (uint8_t i = 0; i < 3; ++i)
                    callAdd(NPC_ILGYNOTH_CLOTTED_CORRUPTION, aroundSelf(10.0f, 50.0f));
                break;
            default:
                break;
        }
    }

private:
    void resumeGaze(uint32_t cursedBloodMs)
    {
        enterStage(1);
        mSubmergePending = false;
        setDormant(false);
        scriptEvents.addEvent(EVENT_ILGYNOTH_EYE_OF_NZOTH, 5000);
        scriptEvents.addEvent(EVENT_ILGYNOTH_CORRUPTORS_GAZE, 12000);
        scriptEvents.addEvent(EVENT_ILGYNOTH_CURSED_BLOOD, cursedBloodMs);
    }

    // heals while hidden, the organs open and pump blood
    void submerge()
    {
        enterStage(2);
        warn("Il'gynoth submerges! Destroy an Organ of Corruption!");
        setDormant(true);
        getCreature()->setHealth(getCreature()->getMaxHealth());

        if (!mOrgansPlaced)
        {
            mOrgansPlaced = true;
            for (LocationVector const& spot : IlgynothOrganSpots)
                placeAdd(NPC_ILGYNOTH_ORGAN_OF_CORRUPTION, spot);
        }

        std::vector<Creature*> const organs = livingSummons(NPC_ILGYNOTH_ORGAN_OF_CORRUPTION);
        if (organs.empty())
        {
            mOrgansSlain = IlgynothOrganCount;
            resumeGaze(3000);
            return;
        }

        for (Creature* pOrgan : organs)
        {
            tell(pOrgan, ACTION_ORGAN_EXPOSE);
            callAdd(NPC_ILGYNOTH_BLOOD_OF_NYALOTHA, aroundSelf(10.0f, 40.0f));
        }

        scriptEvents.addEvent(EVENT_ILGYNOTH_CLOTTED_CORRUPTION, 20000);
    }

    uint32_t mOrgansSlain = 0;
    bool mOrgansPlaced = false;
    bool mSubmergePending = false;
};

// sealed until Il'gynoth submerges
class NyalothaOrganOfCorruptionAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaOrganOfCorruptionAI(c); }
    explicit NyalothaOrganOfCorruptionAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        applyExposure();
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_ORGAN_EXPOSE)
            mExposed = true;
        else if (action == ACTION_ORGAN_SEAL)
            mExposed = false;
        else
            return;

        getCreature()->setHealth(getCreature()->getMaxHealth());
        applyExposure();
    }

private:
    void applyExposure()
    {
        if (mExposed)
            removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        else
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    bool mExposed = false;
};

class NyalothaBloodOfNyalothaAI : public NyalothaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaBloodOfNyalothaAI(c); }
    explicit NyalothaBloodOfNyalothaAI(Creature* pCreature) : NyalothaCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_BLOOD_OF_NYALOTHA_AURA, true);
        scriptEvents.addEvent(EVENT_BLOOD_OF_NYALOTHA_FIXATE, 500);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId == EVENT_BLOOD_OF_NYALOTHA_FIXATE)
            fixateOn(randomPlayer());
    }
};

class NyalothaClottedCorruptionAI : public NyalothaSpellAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaClottedCorruptionAI(c); }
    explicit NyalothaClottedCorruptionAI(Creature* pCreature) : NyalothaSpellAddAI(pCreature, SPELL_CLOTTED_CORRUPTION_ABSORBING_CHARGE, NyalothaAim::RandomPlayer, 3000, 0, 0, false) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// N'Zoth the Corruptor

class NyalothaNzothAI : public NyalothaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaNzothAI(c); }
    explicit NyalothaNzothAI(Creature* pCreature) : NyalothaBossAI(pCreature, DATA_NZOTH)
    {
        mImmobile = true;
    }

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr)
            return;

        switch (pSummon->getEntry())
        {
            case NPC_NZOTH_PSYCHUS:
                if (mStage == 1)
                {
                    warn("Psychus falls and N'Zoth's ego is shattered!");
                    castSpellOnSelf(SPELL_NZOTH_SHATTERED_EGO, true);
                    scriptEvents.addEvent(EVENT_NZOTH_EGO_RECOVERS, 30000);
                }
                break;
            case NPC_NZOTH_SPIKE_TENTACLE:
                getCreature()->castSpellLoc(pSummon->GetPosition(), SPELL_SPIKE_TENTACLE_CORRUPTED_VISCERA, true);
                countTentacle();
                break;
            case NPC_NZOTH_BASHER_TENTACLE:
            case NPC_NZOTH_CORRUPTOR_TENTACLE:
                countTentacle();
                break;
            default:
                break;
        }
    }

protected:
    void onEngage() override
    {
        mTentaclesSlain = 0;
        armBerserk(SPELL_NZOTH_BERSERK, 780000);
        scriptEvents.addEvent(EVENT_NZOTH_DOMINANT_MIND, 3000);
    }

    void onDisengage() override { _removeAura(SPELL_NZOTH_SHATTERED_EGO); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NZOTH_DOMINANT_MIND:
                warn("N'Zoth withdraws into the depths and Psychus emerges!");
                setDormant(true);
                if (callAdd(NPC_NZOTH_PSYCHUS, NzothPsychusSpot) == nullptr)
                    beginWrithingOnslaught();
                break;
            case EVENT_NZOTH_EGO_RECOVERS:
                beginWrithingOnslaught();
                break;
            case EVENT_NZOTH_ETERNAL_TORMENT:
                castSpellOnSelf(SPELL_NZOTH_ETERNAL_TORMENT, true);
                scriptEvents.addEvent(EVENT_NZOTH_ETERNAL_TORMENT, 35000);
                break;
            case EVENT_NZOTH_CATACLYSMIC_FLAMES:
                castOnPlayers(SPELL_NZOTH_CATACLYSMIC_FLAMES, 10);
                scriptEvents.addEvent(EVENT_NZOTH_CATACLYSMIC_FLAMES, 20000);
                break;
            case EVENT_NZOTH_TENTACLE_WAVE:
                if (raiseTentacles() == 0)
                    beginConvergence();
                break;
            case EVENT_NZOTH_STUPEFYING_GLARE:
                castAtRandomPlayer(SPELL_NZOTH_STUPEFYING_GLARE, false, false);
                scriptEvents.addEvent(EVENT_NZOTH_STUPEFYING_GLARE, 30000);
                break;
            case EVENT_NZOTH_MINDGRASP:
                warn("N'Zoth begins to cast Mindgrasp!");
                castSpellAOE(SPELL_NZOTH_MINDGRASP);
                scriptEvents.addEvent(EVENT_NZOTH_MINDGRASP, 30000);
                break;
            case EVENT_NZOTH_THOUGHT_HARVESTER:
                callAdd(NPC_NZOTH_THOUGHT_HARVESTER, nyalothaRingPoint(NzothHarvesterSpot, 0.0f, 30.0f));
                break;
            default:
                break;
        }
    }

private:
    void beginWrithingOnslaught()
    {
        enterStage(2);
        _removeAura(SPELL_NZOTH_SHATTERED_EGO);
        warn("N'Zoth's tentacles burst forth!");
        scriptEvents.addEvent(EVENT_NZOTH_TENTACLE_WAVE, 100);
        scriptEvents.addEvent(EVENT_NZOTH_ETERNAL_TORMENT, 3000);
        scriptEvents.addEvent(EVENT_NZOTH_CATACLYSMIC_FLAMES, 13000);
    }

    void beginConvergence()
    {
        enterStage(3);
        warn("N'Zoth rises to face you!");
        setDormant(false);
        scriptEvents.addEvent(EVENT_NZOTH_STUPEFYING_GLARE, 3000);
        scriptEvents.addEvent(EVENT_NZOTH_MINDGRASP, 8000);
        scriptEvents.addEvent(EVENT_NZOTH_THOUGHT_HARVESTER, 8000);
    }

    // a second wave after four kills, N'Zoth himself after eight
    void countTentacle()
    {
        if (mStage != 2)
            return;

        ++mTentaclesSlain;
        if (mTentaclesSlain == 4)
        {
            warn("More tentacles writhe through the Mindgate!");
            scriptEvents.addEvent(EVENT_NZOTH_TENTACLE_WAVE, 100);
        }
        else if (mTentaclesSlain == 8)
        {
            beginConvergence();
        }
    }

    uint32_t raiseTentacles()
    {
        uint32_t raised = 0;
        if (callAdd(NPC_NZOTH_BASHER_TENTACLE, NzothBasherSpot) != nullptr)
            ++raised;
        if (callAdd(NPC_NZOTH_CORRUPTOR_TENTACLE, nyalothaRingPoint(NzothBasherSpot, 3.0f, 10.0f)) != nullptr)
            ++raised;

        for (uint8_t i = 0; i < 3; ++i)
        {
            if (callAdd(NPC_NZOTH_SPIKE_TENTACLE, nyalothaRingPoint(NzothBasherSpot, 5.0f, 15.0f)) != nullptr)
                ++raised;
        }

        return raised;
    }

    uint32_t mTentaclesSlain = 0;
};

class NyalothaPsychusAI : public NyalothaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaPsychusAI(c); }
    explicit NyalothaPsychusAI(Creature* pCreature) : NyalothaCombatAI(pCreature) {}

    void OnDied(Unit* /*pKiller*/) override { showFrame(false); }

protected:
    void onEngage() override
    {
        showFrame(true);
        scriptEvents.addEvent(EVENT_PSYCHUS_MINDWRACK, 3000);
        scriptEvents.addEvent(EVENT_PSYCHUS_CREEPING_ANGUISH, 8000);
        scriptEvents.addEvent(EVENT_PSYCHUS_MANIFEST_MADNESS, PsychusMadnessFillMs);
    }

    void onDisengage() override
    {
        if (isAlive())
            showFrame(false);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_PSYCHUS_MINDWRACK:
                castSpellOnVictim(SPELL_PSYCHUS_MINDWRACK);
                scriptEvents.addEvent(EVENT_PSYCHUS_MINDWRACK, 10000);
                break;
            case EVENT_PSYCHUS_CREEPING_ANGUISH:
                castSpellAOE(SPELL_PSYCHUS_CREEPING_ANGUISH);
                scriptEvents.addEvent(EVENT_PSYCHUS_CREEPING_ANGUISH, 20000);
                break;
            case EVENT_PSYCHUS_MANIFEST_MADNESS:
                warn("Psychus begins to cast Manifest Madness!");
                castSpellAOE(SPELL_PSYCHUS_MANIFEST_MADNESS);
                scriptEvents.addEvent(EVENT_PSYCHUS_MANIFEST_MADNESS, PsychusMadnessFillMs);
                break;
            default:
                break;
        }
    }
};

class NyalothaBasherTentacleAI : public NyalothaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaBasherTentacleAI(c); }
    explicit NyalothaBasherTentacleAI(Creature* pCreature) : NyalothaCombatAI(pCreature)
    {
        mImmobile = true;
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_BASHER_TUMULTUOUS_BURST, 1000);
        scriptEvents.addEvent(EVENT_BASHER_VOID_LASH, 3000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BASHER_TUMULTUOUS_BURST:
            {
                // lashes out when nobody holds it in melee
                Unit* pVictim = currentVictim();
                if (pVictim == nullptr || getRangeToObject(pVictim) > 8.0f)
                    castSpellAOE(SPELL_BASHER_TENTACLE_TUMULTUOUS_BURST);
                scriptEvents.addEvent(EVENT_BASHER_TUMULTUOUS_BURST, 10000);
                break;
            }
            case EVENT_BASHER_VOID_LASH:
                castSpellOnVictim(SPELL_BASHER_TENTACLE_VOID_LASH);
                scriptEvents.addEvent(EVENT_BASHER_VOID_LASH, 15000);
                break;
            default:
                break;
        }
    }
};

class NyalothaCorruptorTentacleAI : public NyalothaSpellAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaCorruptorTentacleAI(c); }
    explicit NyalothaCorruptorTentacleAI(Creature* pCreature) : NyalothaSpellAddAI(pCreature, SPELL_CORRUPTOR_TENTACLE_CORRUPTED_MIND, NyalothaAim::RandomPlayer, 3000, 15000, 15000, true) {}
};

class NyalothaSpikeTentacleAI : public NyalothaSpellAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaSpikeTentacleAI(c); }
    explicit NyalothaSpikeTentacleAI(Creature* pCreature) : NyalothaSpellAddAI(pCreature, SPELL_SPIKE_TENTACLE_PAIN_SPIKE, NyalothaAim::RandomPlayer, 3000, 15000, 15000, true) {}
};

class NyalothaThoughtHarvesterAI : public NyalothaSpellAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NyalothaThoughtHarvesterAI(c); }
    explicit NyalothaThoughtHarvesterAI(Creature* pCreature) : NyalothaSpellAddAI(pCreature, SPELL_THOUGHT_HARVESTER_HARVEST, NyalothaAim::Area, 3000, 60000, 60000, false) {}
};

void SetupNyalotha(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_NYALOTHA, &NyalothaInstanceScript::Create);

    // Maut
    mgr->register_creature_script(BOSS_MAUT, &NyalothaMautAI::Create);

    // Dark Inquisitor Xanesh
    mgr->register_creature_script(BOSS_XANESH, &NyalothaXaneshAI::Create);
    mgr->register_creature_script(NPC_XANESH_FLAYED_SOUL, &NyalothaFlayedSoulAI::Create);
    mgr->register_creature_script(NPC_XANESH_RITUAL_OBELISK, &NyalothaRitualObeliskAI::Create);
    mgr->register_creature_script(NPC_XANESH_LARGE_RITUAL_OBELISK, &NyalothaLargeRitualObeliskAI::Create);

    // Wrathion
    mgr->register_creature_script(BOSS_WRATHION, &NyalothaWrathionAI::Create);
    mgr->register_creature_script(NPC_WRATHION_CRACKLING_SHARD, &NyalothaAnchoredAI::Create);
    mgr->register_creature_script(NPC_WRATHION_ASHWALKER_ASSASSIN, &NyalothaAshwalkerAssassinAI::Create);

    // Ra-den the Despoiled
    mgr->register_creature_script(BOSS_RADEN, &NyalothaRadenAI::Create);
    mgr->register_creature_script(NPC_RADEN_ESSENCE_OF_VITA, &NyalothaEssenceOfVitaAI::Create);
    mgr->register_creature_script(NPC_RADEN_ESSENCE_OF_VOID, &NyalothaEssenceOfVoidAI::Create);
    mgr->register_creature_script(NPC_RADEN_VOID_HUNTER, &NyalothaVoidHunterAI::Create);
    mgr->register_creature_script(NPC_RADEN_CRACKLING_STALKER, &NyalothaCracklingStalkerAI::Create);

    // The Hivemind
    mgr->register_creature_script(BOSS_TEKRIS, &NyalothaTekrisAI::Create);
    mgr->register_creature_script(BOSS_KAZIR, &NyalothaKazirAI::Create);
    mgr->register_creature_script(NPC_HIVEMIND_AQIR_DRONE, &NyalothaAqirDroneAI::Create);
    mgr->register_creature_script(NPC_HIVEMIND_AQIR_DARTER, &NyalothaAqirDarterAI::Create);
    mgr->register_creature_script(NPC_HIVEMIND_AQIR_RAVAGER, &NyalothaAqirRavagerAI::Create);
    mgr->register_creature_script(NPC_HIVEMIND_ACIDIC_AQIR, &NyalothaAcidicAqirAI::Create);

    // Prophet Skitra
    mgr->register_creature_script(BOSS_PROPHET_SKITRA, &NyalothaSkitraAI::Create);
    mgr->register_creature_script(NPC_SKITRA_PROJECTION, &NyalothaAnchoredAI::Create);
    mgr->register_creature_script(NPC_SKITRA_SHREDDED_PSYCHE, &NyalothaShreddedPsycheAI::Create);

    // Shad'har the Insatiable
    mgr->register_creature_script(BOSS_SHADHAR, &NyalothaShadharAI::Create);
    mgr->register_creature_script(NPC_SHADHAR_LIVING_MIASMA, &NyalothaLivingMiasmaAI::Create);

    // Vexiona
    mgr->register_creature_script(BOSS_VEXIONA, &NyalothaVexionaAI::Create);
    mgr->register_creature_script(NPC_VEXIONA_VOID_ASCENDANT, &NyalothaVoidAscendantAI::Create);
    mgr->register_creature_script(NPC_VEXIONA_SPELLBOUND_RITUALIST, &NyalothaSpellboundRitualistAI::Create);
    mgr->register_creature_script(NPC_VEXIONA_DARK_GATEWAY, &NyalothaDarkGatewayAI::Create);

    // Carapace of N'Zoth
    mgr->register_creature_script(BOSS_FURY_OF_NZOTH, &NyalothaCarapaceAI::Create);
    mgr->register_creature_script(NPC_CARAPACE_THRASHING_TENTACLE, &NyalothaThrashingTentacleAI::Create);

    // Drest'agath
    mgr->register_creature_script(BOSS_DRESTAGATH, &NyalothaDrestagathAI::Create);
    mgr->register_creature_script(NPC_DRESTAGATH_EYE, &NyalothaEyeOfDrestagathAI::Create);
    mgr->register_creature_script(NPC_DRESTAGATH_TENTACLE, &NyalothaTentacleOfDrestagathAI::Create);
    mgr->register_creature_script(NPC_DRESTAGATH_MAW, &NyalothaMawOfDrestagathAI::Create);

    // N'Zoth the Corruptor
    mgr->register_creature_script(BOSS_NZOTH, &NyalothaNzothAI::Create);
    mgr->register_creature_script(NPC_NZOTH_PSYCHUS, &NyalothaPsychusAI::Create);
    mgr->register_creature_script(NPC_NZOTH_BASHER_TENTACLE, &NyalothaBasherTentacleAI::Create);
    mgr->register_creature_script(NPC_NZOTH_CORRUPTOR_TENTACLE, &NyalothaCorruptorTentacleAI::Create);
    mgr->register_creature_script(NPC_NZOTH_SPIKE_TENTACLE, &NyalothaSpikeTentacleAI::Create);
    mgr->register_creature_script(NPC_NZOTH_THOUGHT_HARVESTER, &NyalothaThoughtHarvesterAI::Create);

    // Il'gynoth, Corruption Reborn
    mgr->register_creature_script(BOSS_ILGYNOTH, &NyalothaIlgynothAI::Create);
    mgr->register_creature_script(NPC_ILGYNOTH_ORGAN_OF_CORRUPTION, &NyalothaOrganOfCorruptionAI::Create);
    mgr->register_creature_script(NPC_ILGYNOTH_BLOOD_OF_NYALOTHA, &NyalothaBloodOfNyalothaAI::Create);
    mgr->register_creature_script(NPC_ILGYNOTH_CLOTTED_CORRUPTION, &NyalothaClottedCorruptionAI::Create);
}
