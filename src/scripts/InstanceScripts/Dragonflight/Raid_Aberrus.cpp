/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Aberrus.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

enum AberrusEvents
{
    // shared by the simple adds
    EVENT_ABERRUS_ADD_SKILL_0 = 1,
    EVENT_ABERRUS_ADD_SKILL_1,
    EVENT_ABERRUS_ADD_SKILL_2,

    // Rashok
    EVENT_RASHOK_SEARING_SLAM,
    EVENT_RASHOK_DOOM_FLAMES,
    EVENT_RASHOK_SHADOWLAVA_BLAST,
    EVENT_RASHOK_CHARGED_SMASH,
    EVENT_RASHOK_UNLEASH_SHADOWFLAME,
    EVENT_RASHOK_WRATH_OF_DJARUUN,
    EVENT_RASHOK_ENTER_CONDUIT,
    EVENT_RASHOK_SCORCHING_HEATWAVE,
    EVENT_RASHOK_SHADOWFLAME_FISSURES,
    EVENT_RASHOK_LEAVE_CONDUIT,

    // Warlord Kagni and Ignara
    EVENT_KAGNI_HEAVY_CUDGEL,
    EVENT_KAGNI_DEVASTATING_LEAP,
    EVENT_KAGNI_WARBAND_WAVE,
    EVENT_KAGNI_CATASTROPHIC_SLAM,
    EVENT_KAGNI_FLAMING_CUDGEL,
    EVENT_IGNARA_PHOENIX_RUSH,
    EVENT_IGNARA_VIGOROUS_GALE,
    EVENT_IGNARA_DESPERATE_IMMOLATION,

    // Magmorax
    EVENT_MAGMORAX_MOLTEN_SPITTLE,
    EVENT_MAGMORAX_IGNITING_ROAR,
    EVENT_MAGMORAX_LAVA_EJECTION,
    EVENT_MAGMORAX_OVERPOWERING_STOMP,
    EVENT_MAGMORAX_BLAZING_BREATH,
    EVENT_MAGMORAX_INCINERATING_MAWS,
    EVENT_MAGMORAX_ERUPTION,

    // Echo of Neltharion
    EVENT_NELTHARION_ECHOING_FISSURE,
    EVENT_NELTHARION_CALAMITOUS_STRIKE,
    EVENT_NELTHARION_RUSHING_DARKNESS,
    EVENT_NELTHARION_TWISTED_EARTH,
    EVENT_NELTHARION_VOLCANIC_HEART,
    EVENT_NELTHARION_CORRUPTION,
    EVENT_NELTHARION_UMBRAL_ANNIHILATION,
    EVENT_NELTHARION_SUNDER_SHADOW,
    EVENT_NELTHARION_VOICE_CHECK,
    EVENT_NELTHARION_SUNDER_REALITY,
    EVENT_NELTHARION_EBON_DESTRUCTION,

    // Scalecommander Sarkareth
    EVENT_SARKARETH_OPPRESSING_HOWL,
    EVENT_SARKARETH_GLITTERING_SURGE,
    EVENT_SARKARETH_BURNING_CLAWS,
    EVENT_SARKARETH_SCORCHING_BOMB,
    EVENT_SARKARETH_MASS_DISINTEGRATE,
    EVENT_SARKARETH_SEARING_BREATH,
    EVENT_SARKARETH_VOID_CLAWS,
    EVENT_SARKARETH_VOID_BOMB,
    EVENT_SARKARETH_ABYSSAL_BREATH,
    EVENT_SARKARETH_EBON_MIGHT,
    EVENT_SARKARETH_DESOLATE_BLOSSOM,
    EVENT_SARKARETH_INFINITE_DURESS,
    EVENT_SARKARETH_VOID_SLASH,
    EVENT_SARKARETH_HURTLING_BARRAGE,
    EVENT_SARKARETH_EMBRACE_OF_NOTHINGNESS,
    EVENT_SARKARETH_SCOURING_ETERNITY,

    // The Amalgamation Chamber
    EVENT_CHAMBER_INSTABILITY_CHECK,
    EVENT_BLAZE_FLAME_SLASH,
    EVENT_BLAZE_BLAZING_HEAT,
    EVENT_BLAZE_FIERY_METEOR,
    EVENT_BLAZE_SWIRLING_FLAME,
    EVENT_SHADOW_SHADOW_SPIKE,
    EVENT_SHADOW_CORRUPTING_SHADOW,
    EVENT_SHADOW_UMBRAL_DETONATION,
    EVENT_SHADOW_COALESCING_VOID,
    EVENT_AMALGAMATION_WITHERING_VULNERABILITY,
    EVENT_AMALGAMATION_SHADOWFLAME_BURST,
    EVENT_AMALGAMATION_SHADOW_AND_FLAME,
    EVENT_AMALGAMATION_GLOOM_CONFLAGRATION,
    EVENT_AMALGAMATION_BLISTERING_TWILIGHT,
    EVENT_AMALGAMATION_CONVERGENT_ERUPTION,

    // Kazzara
    EVENT_KAZZARA_TERROR_CLAWS,
    EVENT_KAZZARA_DREAD_RIFTS,
    EVENT_KAZZARA_RAYS_OF_ANGUISH,
    EVENT_KAZZARA_WINGS_OF_EXTINCTION,

    // Zskarn
    EVENT_ZSKARN_SEARING_CLAWS,
    EVENT_ZSKARN_BLAST_WAVE,
    EVENT_ZSKARN_UNSTABLE_EMBERS,
    EVENT_ZSKARN_SHRAPNEL_BOMB,
    EVENT_ZSKARN_ANIMATE_GOLEMS,

    // The Forgotten Experiments
    EVENT_NELDRIS_MASSIVE_SLAM,
    EVENT_NELDRIS_BELLOWING_ROAR,
    EVENT_NELDRIS_RENDING_CHARGE,
    EVENT_THADRION_UNSTABLE_ESSENCE,
    EVENT_THADRION_VOLATILE_SPEW,
    EVENT_THADRION_VIOLENT_ERUPTION,
    EVENT_RIONTHUS_DISINTEGRATE,
    EVENT_RIONTHUS_TEMPORAL_ANOMALY,
    EVENT_RIONTHUS_DEEP_BREATH
};

namespace
{
    float const AberrusFullCircle = 6.28318531f;
    uint32_t const AberrusEnergyCap = 100;
    uint32_t const AberrusLastBerserkMs = 10000;

    // encounter berserk, counted from the pull
    uint32_t const RashokBerserkMs = 480000;
    uint32_t const ZaqaliBerserkMs = 540000;
    uint32_t const NeltharionBerserkMs = 600000;
    uint32_t const SarkarethBerserkMs = 600000;
    uint32_t const AmalgamationBerserkMs = 480000;
    uint32_t const KazzaraBerserkMs = 420000;
    uint32_t const ZskarnBerserkMs = 480000;
    uint32_t const ExperimentsBerserkMs = 540000;

    // milliseconds per energy point
    uint32_t const RashokEnergyStepMs = 1000;
    uint32_t const MagmoraxEnergyStepMs = 3600;
    uint32_t const KazzaraEnergyStepMs = 500;
    uint32_t const ZskarnEnergyStepMs = 800;

    uint32_t const RashokConduitMs = 18000;
    int32_t const KagniFinalAssaultPct = 25;
    int32_t const NeltharionSurrenderPct = 60;
    int32_t const NeltharionBarrierPct = 35;
    int32_t const SarkarethSecondStagePct = 65;
    int32_t const SarkarethThirdStagePct = 35;
    int32_t const ChamberMergePct = 50;
    float const ChamberInstabilityRange = 15.0f;
    int32_t const KazzaraCarnageMarks[3] = { 80, 60, 40 };

    // members of the encounters fought as one
    struct AberrusMember
    {
        uint32_t dataId;
        uint32_t entry;
    };

    AberrusMember const AberrusMembers[] =
    {
        { DATA_AMALGAMATION,    BOSS_CHAMBER_ETERNAL_BLAZE },
        { DATA_AMALGAMATION,    BOSS_CHAMBER_ESSENCE_OF_SHADOW },
        { DATA_AMALGAMATION,    BOSS_CHAMBER_AMALGAMATION },
        { DATA_EXPERIMENTS,     BOSS_EXPERIMENT_NELDRIS },
        { DATA_EXPERIMENTS,     BOSS_EXPERIMENT_THADRION },
        { DATA_EXPERIMENTS,     BOSS_EXPERIMENT_RIONTHUS }
    };

    // vat order of the experiments
    uint32_t const ExperimentOrder[3] = { BOSS_EXPERIMENT_NELDRIS, BOSS_EXPERIMENT_THADRION, BOSS_EXPERIMENT_RIONTHUS };

    // room doors shut while the fight runs, passages open after the kill
    struct AberrusDoor
    {
        uint32_t entry;
        uint32_t dataId;
        bool passage;
    };

    AberrusDoor const AberrusDoors[] =
    {
        { GO_KAZZARA_ARENA_DOOR, DATA_KAZZARA, false }
    };

    AberrusDoor const* findAberrusDoor(uint32_t entry)
    {
        for (AberrusDoor const& door : AberrusDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    // random spot between two radii around a point
    LocationVector aberrusSpotAround(LocationVector const& origin, float innerRadius, float outerRadius)
    {
        float const angle = Util::getRandomFloat(AberrusFullCircle);
        float const radius = Util::getRandomFloat(innerRadius, outerRadius);
        return LocationVector(origin.x + radius * std::cos(angle), origin.y + radius * std::sin(angle), origin.z, angle);
    }

    // how a simple add uses a skill
    enum AberrusAddAim : uint8_t
    {
        ADD_AIM_VICTIM,
        ADD_AIM_PLAYER,
        ADD_AIM_SELF,
        ADD_AIM_AREA
    };

    struct AberrusAddSkill
    {
        uint32_t spellId;
        uint32_t firstMs;
        uint32_t repeatMs;
        AberrusAddAim aim;
        bool heroicOnly;
    };

    AberrusAddSkill const HuntsmanSkills[] =
    {
        { SPELL_HUNTSMAN_BLAZING_SPEAR, 4000, 10000, ADD_AIM_PLAYER, false }
    };

    AberrusAddSkill const MysticSkills[] =
    {
        { SPELL_MYSTIC_MOLTEN_BARRIER, 3000, 30000, ADD_AIM_SELF, false },
        { SPELL_MYSTIC_LAVA_BOLT, 5000, 4500, ADD_AIM_PLAYER, false },
        { SPELL_MYSTIC_MAGMA_FLOW, 12000, 20000, ADD_AIM_AREA, false }
    };

    AberrusAddSkill const GuardSkills[] =
    {
        { SPELL_GUARD_SCORCHING_ROAR, 8000, 15000, ADD_AIM_AREA, false },
        { SPELL_GUARD_VOLCANIC_SHIELD, 12000, 25000, ADD_AIM_SELF, false }
    };

    AberrusAddSkill const HatchlingSkills[] =
    {
        { SPELL_HATCHLING_BLAZING_FOCUS, 1000, 12000, ADD_AIM_PLAYER, false }
    };

    AberrusAddSkill const VoiceSkills[] =
    {
        { SPELL_VOICE_EMPOWER_SHADOWS, 5000, 15000, ADD_AIM_AREA, false }
    };

    AberrusAddSkill const AberrationSkills[] =
    {
        { SPELL_ABERRATION_UNSTABLE_MUTATION, 8000, 10000, ADD_AIM_SELF, true }
    };

    AberrusAddSkill const RecollectionSkills[] =
    {
        { SPELL_RECOLLECTION_EMPTY_STRIKE, 4000, 8000, ADD_AIM_VICTIM, false },
        { SPELL_RECOLLECTION_COSMIC_VOLLEY, 10000, 14000, ADD_AIM_AREA, false }
    };

    AberrusAddSkill const GlimmerSkills[] =
    {
        { SPELL_GLIMMER_BLASTING_SCREAM, 6000, 12000, ADD_AIM_AREA, false }
    };

    AberrusAddSkill const GolemSkills[] =
    {
        { SPELL_GOLEM_ACTIVATE_DRAGONFIRE_TRAP, 6000, 20000, ADD_AIM_SELF, false }
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class AberrusInstanceScript : public InstanceScript
{
public:
    explicit AberrusInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(AberrusEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new AberrusInstanceScript(pMapMgr); }

    bool setBossState(uint32_t id, EncounterStates state) override
    {
        EncounterStates const current = getBossState(id);

        // a standing member or a running merge keeps the encounter open
        if (state == EncounterStates::Performed && current == EncounterStates::InProgress)
        {
            if ((id == DATA_AMALGAMATION && mMerging) || !isEncounterDown(id, nullptr))
                return false;
        }

        if (state == EncounterStates::InProgress && current != EncounterStates::InProgress && id < AberrusEncounterCount)
            mPullMs[id] = Util::getMSTime();

        return InstanceScript::setBossState(id, state);
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findAberrusDoor(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        refreshDoor(pGameObject);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                refreshDoor(pDoor);
        }

        if (state != EncounterStates::Failed)
            return;

        if (id == DATA_AMALGAMATION)
        {
            mMerged = false;
            mMerging = false;
        }

        // fallen members return, a formed Amalgamation goes away
        for (Creature* pMember : getMembers(id))
        {
            if (pMember->getEntry() == BOSS_CHAMBER_AMALGAMATION)
                pMember->despawn(1000, 0);
            else if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    std::vector<Creature*> getMembers(uint32_t dataId)
    {
        std::vector<Creature*> members;
        for (AberrusMember const& member : AberrusMembers)
        {
            if (member.dataId != dataId)
                continue;

            for (Creature* pCreature : getCreatureSetForEntry(member.entry))
                members.push_back(pCreature);
        }

        return members;
    }

    // encounters without members are down with their boss
    bool isEncounterDown(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getMembers(dataId))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    bool isEncounterFighting(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getMembers(dataId))
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat())
                return true;
        }

        return false;
    }

    uint32_t encounterElapsedMs(uint32_t dataId) const
    {
        if (dataId >= AberrusEncounterCount || getBossState(dataId) != EncounterStates::InProgress)
            return 0;

        return Util::getMSTime() - mPullMs[dataId];
    }

    bool anyAlive(uint32_t entry)
    {
        for (Creature* pCreature : getCreatureSetForEntry(entry))
        {
            if (pCreature->isAlive())
                return true;
        }

        return false;
    }

    void sendActionTo(uint32_t entry, int32_t action)
    {
        for (Creature* pCreature : getCreatureSetForEntry(entry))
        {
            if (pCreature->isAlive() && pCreature->GetScript() != nullptr)
                pCreature->GetScript()->DoAction(action);
        }
    }

    // both elementals answer the pull together
    void rallyElementals(CreatureAIScript* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getMembers(DATA_AMALGAMATION))
        {
            if (pMember == pPuller->getCreature() || pMember->getEntry() == BOSS_CHAMBER_AMALGAMATION)
                continue;

            if (pMember->isAlive() && !pMember->isInCombat())
                pPuller->setZoneWideCombat(pMember);
        }
        mRallying = false;
    }

    // only the first elemental below the mark starts the merge
    bool beginMerge()
    {
        if (mMerged)
            return false;

        mMerged = true;
        mMerging = true;
        return true;
    }

    void endMerge() { mMerging = false; }
    bool isMerged() const { return mMerged; }

    // the next experiment in line leaves its vat
    void releaseExperimentAfter(uint32_t entry)
    {
        bool found = false;
        for (uint32_t next : ExperimentOrder)
        {
            if (!found)
            {
                found = next == entry;
                continue;
            }

            if (anyAlive(next))
            {
                sendActionTo(next, ACTION_EXPERIMENT_BREAK_FREE);
                return;
            }
        }
    }

private:
    void refreshDoor(GameObject* pDoor)
    {
        AberrusDoor const* pRule = findAberrusDoor(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        EncounterStates const state = getBossState(pRule->dataId);
        bool const open = pRule->passage ? state == EncounterStates::Performed : state != EncounterStates::InProgress;
        uint8_t const wanted = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    std::vector<WoWGuid> mDoorGuids;
    uint32_t mPullMs[AberrusEncounterCount] = {};
    bool mRallying = false;
    bool mMerged = false;
    bool mMerging = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, stages, energy, berserk and target helpers

class AberrusCombatAI : public CreatureAIScript
{
public:
    explicit AberrusCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        // templates shared with other maps stay passive there
        if (!onAberrus())
            return;

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
        onDisengage();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mStage == 0 || !_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        tickBerserk(diff);
        tickEnergy(diff);
        onCombatTick(diff);

        // due events run one after another unless a cast holds them
        while (mStage != 0 && !_isCasting())
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

    bool onAberrus() { return getCreature()->GetMapId() == MAP_ABERRUS; }

    AberrusInstanceScript* getAberrus()
    {
        if (!onAberrus())
            return nullptr;

        return static_cast<AberrusInstanceScript*>(getInstanceScript());
    }

    // up to count different players, the current target only when nobody else is left
    std::vector<Unit*> randomPlayers(size_t count, bool spareTank)
    {
        TargetFilter const filter = spareTank ? TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent) : TargetFilter_Player;
        FilterArgs const args(filter);

        std::vector<Unit*> candidates;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject != nullptr && isValidUnitTarget(pObject, filter, args))
                candidates.push_back(static_cast<Unit*>(pObject));
        }

        if (candidates.empty() && spareTank)
            return randomPlayers(count, false);

        std::vector<Unit*> picked;
        while (!candidates.empty() && picked.size() < count)
        {
            size_t const index = Util::getRandomUInt(static_cast<uint32_t>(candidates.size() - 1));
            picked.push_back(candidates[index]);
            candidates[index] = candidates.back();
            candidates.pop_back();
        }

        return picked;
    }

    Unit* castAtRandom(uint32_t spellId, bool spareTank, bool triggered = false)
    {
        std::vector<Unit*> const picked = randomPlayers(1, spareTank);
        if (picked.empty())
            return nullptr;

        castSpell(picked.front(), spellId, triggered);
        return picked.front();
    }

    // several targets at once need triggered casts
    void castAtRandomPlayers(uint32_t spellId, size_t count, bool spareTank)
    {
        for (Unit* pTarget : randomPlayers(count, spareTank))
            castSpell(pTarget, spellId, true);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float innerRadius, float outerRadius)
    {
        return aberrusSpotAround(getCreature()->GetPosition(), innerRadius, outerRadius);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // a new stage drops every pending event
    void switchStage(uint32_t stage)
    {
        mStage = stage;
        scriptEvents.resetEvents();
    }

    void armBerserk(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

    // own energy counter filling up to 100
    void startEnergy(uint32_t msPerPoint)
    {
        mEnergy = 0;
        mEnergyClockMs = 0;
        mEnergyStepMs = msPerPoint;
    }

    void stopEnergy()
    {
        mEnergy = 0;
        mEnergyClockMs = 0;
        mEnergyStepMs = 0;
    }

    bool isEnergyFull() const { return mEnergy >= AberrusEnergyCap; }

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
        castSpellOnSelf(SPELL_ABERRUS_BERSERK, true);
    }

    void tickEnergy(uint32_t diff)
    {
        if (mEnergyStepMs == 0 || mEnergy >= AberrusEnergyCap)
            return;

        mEnergyClockMs += diff;
        while (mEnergyClockMs >= mEnergyStepMs && mEnergy < AberrusEnergyCap)
        {
            mEnergyClockMs -= mEnergyStepMs;
            ++mEnergy;
        }
    }

    uint32_t mBerserkLeftMs = 0;
    uint32_t mEnergy = 0;
    uint32_t mEnergyClockMs = 0;
    uint32_t mEnergyStepMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state, berserk and group handling

class AberrusBossAI : public AberrusCombatAI
{
public:
    AberrusBossAI(Creature* pCreature, uint32_t dataId, uint32_t berserkMs) : AberrusCombatAI(pCreature), mDataId(dataId), mBerserkMs(berserkMs) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (onAberrus())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
                pInstance->setBossState(mDataId, EncounterStates::InProgress);
            }

            AberrusInstanceScript* pAberrus = getAberrus();
            if (pAberrus != nullptr && mDataId == DATA_AMALGAMATION)
                pAberrus->rallyElementals(this);
        }

        AberrusCombatAI::OnCombatStart(pTarget);

        if (mBerserkMs != 0 && onAberrus())
            armBerserk(berserkLeft());
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive() && onAberrus())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last member has left the fight
                AberrusInstanceScript* pAberrus = getAberrus();
                if (pAberrus == nullptr || !pAberrus->isEncounterFighting(mDataId, getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        AberrusCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (onAberrus())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                AberrusInstanceScript* pAberrus = getAberrus();
                if (pAberrus == nullptr || pAberrus->isEncounterDown(mDataId, getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Performed);
            }

            onDefeat();
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;

private:
    // members joining late share the berserk of the pull
    uint32_t berserkLeft()
    {
        AberrusInstanceScript* pAberrus = getAberrus();
        uint32_t const elapsed = pAberrus != nullptr ? pAberrus->encounterElapsedMs(mDataId) : 0;
        if (elapsed + AberrusLastBerserkMs >= mBerserkMs)
            return AberrusLastBerserkMs;

        return mBerserkMs - elapsed;
    }

    uint32_t mBerserkMs;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Adds with a few timed skills

class AberrusAddAI : public AberrusCombatAI
{
public:
    static CreatureAIScript* CreateHuntsman(Creature* c) { return build(c, HuntsmanSkills); }
    static CreatureAIScript* CreateMystic(Creature* c) { return build(c, MysticSkills); }
    static CreatureAIScript* CreateObsidianGuard(Creature* c) { return build(c, GuardSkills); }
    static CreatureAIScript* CreateHatchling(Creature* c) { return build(c, HatchlingSkills); }
    static CreatureAIScript* CreateVoice(Creature* c) { return build(c, VoiceSkills); }
    static CreatureAIScript* CreateAberration(Creature* c) { return build(c, AberrationSkills); }
    static CreatureAIScript* CreateRecollection(Creature* c) { return build(c, RecollectionSkills); }
    static CreatureAIScript* CreateGlimmer(Creature* c) { return build(c, GlimmerSkills); }
    static CreatureAIScript* CreateGolem(Creature* c) { return build(c, GolemSkills); }

    AberrusAddAI(Creature* pCreature, AberrusAddSkill const* skills, size_t skillCount)
        : AberrusCombatAI(pCreature), mSkills(skills), mSkillCount(skillCount > 3 ? 3 : skillCount) {}

protected:
    void onEngage() override
    {
        for (size_t i = 0; i < mSkillCount; ++i)
        {
            if (mSkills[i].heroicOnly && !isHeroic())
                continue;

            scriptEvents.addEvent(EVENT_ABERRUS_ADD_SKILL_0 + static_cast<uint32_t>(i), mSkills[i].firstMs);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId < EVENT_ABERRUS_ADD_SKILL_0 || eventId >= EVENT_ABERRUS_ADD_SKILL_0 + mSkillCount)
            return;

        AberrusAddSkill const& skill = mSkills[eventId - EVENT_ABERRUS_ADD_SKILL_0];
        switch (skill.aim)
        {
            case ADD_AIM_VICTIM:
                castSpellOnVictim(skill.spellId);
                break;
            case ADD_AIM_PLAYER:
                castAtRandom(skill.spellId, false);
                break;
            case ADD_AIM_SELF:
                castSpellOnSelf(skill.spellId);
                break;
            default:
                castSpellAOE(skill.spellId);
                break;
        }

        scriptEvents.addEvent(eventId, skill.repeatMs);
    }

private:
    template <size_t N>
    static CreatureAIScript* build(Creature* c, AberrusAddSkill const (&skills)[N]) { return new AberrusAddAI(c, skills, N); }

    AberrusAddSkill const* mSkills;
    size_t mSkillCount;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rashok, the Elder

class AberrusRashokAI : public AberrusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AberrusRashokAI(c); }
    explicit AberrusRashokAI(Creature* pCreature) : AberrusBossAI(pCreature, DATA_RASHOK, RashokBerserkMs) {}

protected:
    void onEngage() override
    {
        mFurious = false;
        mInConduit = false;
        fightOutside(8000);
    }

    void onDisengage() override
    {
        if (mInConduit)
            releaseFromConduit();

        mFurious = false;
        _removeAura(SPELL_RASHOK_ANCIENT_FURY);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mInConduit || mFurious || !isEnergyFull() || _isCasting())
            return;

        // full energy: fury, then the wrath carries him into the conduit
        mFurious = true;
        warn("Rashok seethes with Ancient Fury!");
        castSpellOnSelf(SPELL_RASHOK_ANCIENT_FURY, true);
        scriptEvents.addEvent(EVENT_RASHOK_WRATH_OF_DJARUUN, 4000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RASHOK_SHADOWLAVA_BLAST:
                castSpellOnVictim(SPELL_RASHOK_SHADOWLAVA_BLAST);
                scriptEvents.addEvent(EVENT_RASHOK_SHADOWLAVA_BLAST, 22000);
                break;
            case EVENT_RASHOK_SEARING_SLAM:
                castAtRandom(SPELL_RASHOK_SEARING_SLAM, true);
                scriptEvents.addEvent(EVENT_RASHOK_SEARING_SLAM, 26000);
                break;
            case EVENT_RASHOK_DOOM_FLAMES:
                castAtRandom(SPELL_RASHOK_DOOM_FLAMES, true);
                scriptEvents.addEvent(EVENT_RASHOK_DOOM_FLAMES, 31000);
                break;
            case EVENT_RASHOK_CHARGED_SMASH:
                castSpellOnVictim(SPELL_RASHOK_CHARGED_SMASH);
                scriptEvents.addEvent(EVENT_RASHOK_CHARGED_SMASH, 35000);
                break;
            case EVENT_RASHOK_UNLEASH_SHADOWFLAME:
                castSpellAOE(SPELL_RASHOK_UNLEASH_SHADOWFLAME);
                scriptEvents.addEvent(EVENT_RASHOK_UNLEASH_SHADOWFLAME, 45000);
                break;
            case EVENT_RASHOK_WRATH_OF_DJARUUN:
                castSpellOnVictim(SPELL_RASHOK_WRATH_OF_DJARUUN);
                scriptEvents.addEvent(EVENT_RASHOK_ENTER_CONDUIT, 7000);
                break;
            case EVENT_RASHOK_ENTER_CONDUIT:
                enterConduit();
                break;
            case EVENT_RASHOK_SCORCHING_HEATWAVE:
                castSpellAOE(SPELL_RASHOK_SCORCHING_HEATWAVE, true);
                scriptEvents.addEvent(EVENT_RASHOK_SCORCHING_HEATWAVE, 5000);
                break;
            case EVENT_RASHOK_SHADOWFLAME_FISSURES:
                castSpellAOE(SPELL_RASHOK_SHADOWFLAME_FISSURES, true);
                break;
            case EVENT_RASHOK_LEAVE_CONDUIT:
                releaseFromConduit();
                fightOutside(5000);
                break;
            default:
                break;
        }
    }

private:
    void fightOutside(uint32_t openerMs)
    {
        switchStage(1);
        startEnergy(RashokEnergyStepMs);
        scriptEvents.addEvent(EVENT_RASHOK_SHADOWLAVA_BLAST, openerMs);
        scriptEvents.addEvent(EVENT_RASHOK_SEARING_SLAM, openerMs + 4000);
        scriptEvents.addEvent(EVENT_RASHOK_DOOM_FLAMES, openerMs + 12000);
        scriptEvents.addEvent(EVENT_RASHOK_CHARGED_SMASH, openerMs + 22000);
        scriptEvents.addEvent(EVENT_RASHOK_UNLEASH_SHADOWFLAME, openerMs + 32000);
    }

    // incapacitated, energy drains while the raid burns
    void enterConduit()
    {
        switchStage(2);
        mInConduit = true;
        mFurious = false;
        stopEnergy();
        _removeAura(SPELL_RASHOK_ANCIENT_FURY);
        setRooted(true);
        _setMeleeDisabled(true);

        warn("Rashok is drawn into the Elder's Conduit!");
        scriptEvents.addEvent(EVENT_RASHOK_SCORCHING_HEATWAVE, 2000);
        scriptEvents.addEvent(EVENT_RASHOK_SHADOWFLAME_FISSURES, 5000);
        scriptEvents.addEvent(EVENT_RASHOK_LEAVE_CONDUIT, RashokConduitMs);
    }

    void releaseFromConduit()
    {
        mInConduit = false;
        setRooted(false);
        _setMeleeDisabled(false);
    }

    bool mFurious = false;
    bool mInConduit = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Assault of the Zaqali

class AberrusKagniAI : public AberrusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AberrusKagniAI(c); }
    explicit AberrusKagniAI(Creature* pCreature) : AberrusBossAI(pCreature, DATA_ZAQALI_ASSAULT, ZaqaliBerserkMs) {}

protected:
    void onEngage() override
    {
        mMysticWave = true;
        scriptEvents.addEvent(EVENT_KAGNI_WARBAND_WAVE, 5000);
        scriptEvents.addEvent(EVENT_KAGNI_HEAVY_CUDGEL, 10000);
        scriptEvents.addEvent(EVENT_KAGNI_DEVASTATING_LEAP, 30000);

        // Ignara supports the warband from above
        Creature* pIgnara = summonCreature(NPC_ZAQALI_IGNARA, aroundSelf(6.0f, 10.0f), MANUAL_DESPAWN);
        if (pIgnara != nullptr && pIgnara->GetScript() != nullptr)
            pIgnara->GetScript()->DoAction(ACTION_IGNARA_TAKE_WING);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage != 1 || _getHealthPercent() > KagniFinalAssaultPct)
            return;

        // the warband stops, Kagni storms the Elder's Door himself
        switchStage(2);
        warn("Warlord Kagni storms the Elder's Door with Catastrophic Slams!");
        scriptEvents.addEvent(EVENT_KAGNI_CATASTROPHIC_SLAM, 6000);
        scriptEvents.addEvent(EVENT_KAGNI_FLAMING_CUDGEL, 12000);

        if (AberrusInstanceScript* pAberrus = getAberrus())
            pAberrus->sendActionTo(NPC_ZAQALI_IGNARA, ACTION_IGNARA_DESPERATE_ASSAULT);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KAGNI_WARBAND_WAVE:
                sendWarband();
                scriptEvents.addEvent(EVENT_KAGNI_WARBAND_WAVE, 40000);
                break;
            case EVENT_KAGNI_HEAVY_CUDGEL:
                castSpellOnVictim(SPELL_KAGNI_HEAVY_CUDGEL);
                scriptEvents.addEvent(EVENT_KAGNI_HEAVY_CUDGEL, 20000);
                break;
            case EVENT_KAGNI_DEVASTATING_LEAP:
                castAtRandom(SPELL_KAGNI_DEVASTATING_LEAP, true);
                scriptEvents.addEvent(EVENT_KAGNI_DEVASTATING_LEAP, 40000);
                break;
            case EVENT_KAGNI_CATASTROPHIC_SLAM:
                castSpellAOE(SPELL_KAGNI_CATASTROPHIC_SLAM);
                scriptEvents.addEvent(EVENT_KAGNI_CATASTROPHIC_SLAM, 25000);
                break;
            case EVENT_KAGNI_FLAMING_CUDGEL:
                castSpellOnVictim(SPELL_KAGNI_FLAMING_CUDGEL);
                scriptEvents.addEvent(EVENT_KAGNI_FLAMING_CUDGEL, 16000);
                break;
            default:
                break;
        }
    }

private:
    // wallclimbers storm the door, the escort alternates
    void sendWarband()
    {
        for (uint8_t i = 0; i < 2; ++i)
            callAdd(NPC_ZAQALI_WALLCLIMBER, aroundSelf(18.0f, 28.0f));

        callAdd(NPC_ZAQALI_FLAMEBOUND_HUNTSMAN, aroundSelf(18.0f, 28.0f));
        callAdd(mMysticWave ? NPC_ZAQALI_MAGMA_MYSTIC : NPC_ZAQALI_OBSIDIAN_GUARD, aroundSelf(18.0f, 28.0f));
        mMysticWave = !mMysticWave;
    }

    bool mMysticWave = true;
};

// Kagni's companion, out of reach and active while called
class AberrusIgnaraAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AberrusIgnaraAI(c); }
    explicit AberrusIgnaraAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_IGNARA_TAKE_WING && !mActive)
        {
            mActive = true;
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            setRooted(true);
            _setMeleeDisabled(true);
            scriptEvents.resetEvents();
            scriptEvents.addEvent(EVENT_IGNARA_PHOENIX_RUSH, 15000);
            scriptEvents.addEvent(EVENT_IGNARA_VIGOROUS_GALE, 25000);
            return;
        }

        if (action == ACTION_IGNARA_DESPERATE_ASSAULT && mActive && !mDesperate)
        {
            mDesperate = true;
            scriptEvents.removeEvent(EVENT_IGNARA_PHOENIX_RUSH);
            scriptEvents.addEvent(EVENT_IGNARA_DESPERATE_IMMOLATION, 3000);
        }
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void OnDespawn() override { summons.despawnAll(); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mActive || !isAlive())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_IGNARA_PHOENIX_RUSH:
                phoenixRush();
                scriptEvents.addEvent(EVENT_IGNARA_PHOENIX_RUSH, 35000);
                break;
            case EVENT_IGNARA_VIGOROUS_GALE:
                castSpellAOE(SPELL_IGNARA_VIGOROUS_GALE);
                scriptEvents.addEvent(EVENT_IGNARA_VIGOROUS_GALE, 30000);
                break;
            case EVENT_IGNARA_DESPERATE_IMMOLATION:
                castSpellAOE(SPELL_IGNARA_DESPERATE_IMMOLATION);
                scriptEvents.addEvent(EVENT_IGNARA_DESPERATE_IMMOLATION, 30000);
                break;
            default:
                break;
        }
    }

private:
    // the rush leaves two hatchlings behind
    void phoenixRush()
    {
        Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player));
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, SPELL_IGNARA_PHOENIX_RUSH);
        for (uint8_t i = 0; i < 2; ++i)
        {
            Creature* pHatchling = summonCreature(NPC_ZAQALI_RENEWED_HATCHLING, aberrusSpotAround(pTarget->GetPosition(), 3.0f, 6.0f), TIMED_OR_DEAD_DESPAWN, 60000);
            if (pHatchling != nullptr)
                setZoneWideCombat(pHatchling);
        }
    }

    bool mActive = false;
    bool mDesperate = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Magmorax

class AberrusMagmoraxAI : public AberrusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AberrusMagmoraxAI(c); }

    // full molten energy is his enrage
    explicit AberrusMagmoraxAI(Creature* pCreature) : AberrusBossAI(pCreature, DATA_MAGMORAX, 0) {}

protected:
    void onEngage() override
    {
        startEnergy(MagmoraxEnergyStepMs);
        scriptEvents.addEvent(EVENT_MAGMORAX_INCINERATING_MAWS, 8000);
        scriptEvents.addEvent(EVENT_MAGMORAX_MOLTEN_SPITTLE, 12000);
        scriptEvents.addEvent(EVENT_MAGMORAX_BLAZING_BREATH, 18000);
        scriptEvents.addEvent(EVENT_MAGMORAX_OVERPOWERING_STOMP, 26000);
        scriptEvents.addEvent(EVENT_MAGMORAX_IGNITING_ROAR, 32000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage != 1 || !isEnergyFull() || _isCasting())
            return;

        switchStage(2);
        stopEnergy();
        warn("Magmorax brims with molten energy!");
        scriptEvents.addEvent(EVENT_MAGMORAX_ERUPTION, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MAGMORAX_INCINERATING_MAWS:
                castSpellOnVictim(SPELL_MAGMORAX_INCINERATING_MAWS);
                scriptEvents.addEvent(EVENT_MAGMORAX_INCINERATING_MAWS, 20000);
                break;
            case EVENT_MAGMORAX_MOLTEN_SPITTLE:
                castAtRandomPlayers(SPELL_MAGMORAX_MOLTEN_SPITTLE, isHeroic() ? 3 : 2, true);
                scriptEvents.addEvent(EVENT_MAGMORAX_MOLTEN_SPITTLE, 25000);
                break;
            case EVENT_MAGMORAX_BLAZING_BREATH:
                castSpellOnVictim(SPELL_MAGMORAX_BLAZING_BREATH);
                scriptEvents.addEvent(EVENT_MAGMORAX_BLAZING_BREATH, 24000);
                break;
            case EVENT_MAGMORAX_OVERPOWERING_STOMP:
                castSpellAOE(SPELL_MAGMORAX_OVERPOWERING_STOMP);
                scriptEvents.addEvent(EVENT_MAGMORAX_OVERPOWERING_STOMP, 30000);
                break;
            case EVENT_MAGMORAX_IGNITING_ROAR:
                castSpellAOE(SPELL_MAGMORAX_IGNITING_ROAR);
                scriptEvents.addEvent(EVENT_MAGMORAX_LAVA_EJECTION, 3000);
                scriptEvents.addEvent(EVENT_MAGMORAX_IGNITING_ROAR, 33000);
                break;
            case EVENT_MAGMORAX_LAVA_EJECTION:
                castAtRandom(SPELL_MAGMORAX_LAVA_EJECTION, true);
                break;
            case EVENT_MAGMORAX_ERUPTION:
                warn("Magmorax unleashes a Catastrophic Eruption!");
                castSpellAOE(SPELL_MAGMORAX_CATASTROPHIC_ERUPTION, true);
                scriptEvents.addEvent(EVENT_MAGMORAX_ERUPTION, 8000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Echo of Neltharion

class AberrusNeltharionAI : public AberrusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AberrusNeltharionAI(c); }
    explicit AberrusNeltharionAI(Creature* pCreature) : AberrusBossAI(pCreature, DATA_NELTHARION_ECHO, NeltharionBerserkMs) {}

protected:
    void onEngage() override
    {
        mVoicesCalled = false;
        mBarrierRaised = false;
        scriptEvents.addEvent(EVENT_NELTHARION_ECHOING_FISSURE, 5000);
        scriptEvents.addEvent(EVENT_NELTHARION_CALAMITOUS_STRIKE, 12000);
        scriptEvents.addEvent(EVENT_NELTHARION_RUSHING_DARKNESS, 18000);
        scriptEvents.addEvent(EVENT_NELTHARION_TWISTED_EARTH, 24000);
        scriptEvents.addEvent(EVENT_NELTHARION_VOLCANIC_HEART, 28000);
    }

    void onDisengage() override { _removeAura(SPELL_NELTHARION_SHADOW_BARRIER); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        int32_t const health = _getHealthPercent();
        if (mStage == 1 && health <= NeltharionSurrenderPct)
        {
            surrenderToCorruption();
            return;
        }

        // the Voices shield him while they live
        if (mStage == 2 && !mBarrierRaised && health <= NeltharionBarrierPct && voicesRemain())
        {
            mBarrierRaised = true;
            warn("Neltharion hides behind a Shadow Barrier!");
            castSpellOnSelf(SPELL_NELTHARION_SHADOW_BARRIER, true);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NELTHARION_ECHOING_FISSURE:
                castSpellAOE(SPELL_NELTHARION_ECHOING_FISSURE);
                scriptEvents.addEvent(EVENT_NELTHARION_ECHOING_FISSURE, 35000);
                break;
            case EVENT_NELTHARION_CALAMITOUS_STRIKE:
                castSpellOnVictim(SPELL_NELTHARION_CALAMITOUS_STRIKE);
                scriptEvents.addEvent(EVENT_NELTHARION_CALAMITOUS_STRIKE, 25000);
                break;
            case EVENT_NELTHARION_RUSHING_DARKNESS:
                castAtRandom(SPELL_NELTHARION_RUSHING_DARKNESS, true);
                scriptEvents.addEvent(EVENT_NELTHARION_RUSHING_DARKNESS, 30000);
                break;
            case EVENT_NELTHARION_TWISTED_EARTH:
                castSpellAOE(SPELL_NELTHARION_TWISTED_EARTH);
                scriptEvents.addEvent(EVENT_NELTHARION_TWISTED_EARTH, 32000);
                break;
            case EVENT_NELTHARION_VOLCANIC_HEART:
                castAtRandomPlayers(SPELL_NELTHARION_VOLCANIC_HEART, 2, true);
                scriptEvents.addEvent(EVENT_NELTHARION_VOLCANIC_HEART, 30000);
                break;
            case EVENT_NELTHARION_CORRUPTION:
                castAtRandomPlayers(SPELL_NELTHARION_CORRUPTION, isHeroic() ? 3 : 2, true);
                scriptEvents.addEvent(EVENT_NELTHARION_CORRUPTION, 35000);
                break;
            case EVENT_NELTHARION_UMBRAL_ANNIHILATION:
                castSpellAOE(SPELL_NELTHARION_UMBRAL_ANNIHILATION);
                scriptEvents.addEvent(EVENT_NELTHARION_UMBRAL_ANNIHILATION, 30000);
                break;
            case EVENT_NELTHARION_SUNDER_SHADOW:
                castSpellOnVictim(SPELL_NELTHARION_SUNDER_SHADOW);
                scriptEvents.addEvent(EVENT_NELTHARION_SUNDER_SHADOW, 25000);
                break;
            case EVENT_NELTHARION_VOICE_CHECK:
                if (mVoicesCalled && !voicesRemain())
                {
                    ripRealityOpen();
                    break;
                }
                scriptEvents.addEvent(EVENT_NELTHARION_VOICE_CHECK, 2000);
                break;
            case EVENT_NELTHARION_SUNDER_REALITY:
                castSpellAOE(SPELL_NELTHARION_SUNDER_REALITY);
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_NELTHARION_TWISTED_ABERRATION, aroundSelf(12.0f, 25.0f));
                scriptEvents.addEvent(EVENT_NELTHARION_SUNDER_REALITY, 35000);
                break;
            case EVENT_NELTHARION_EBON_DESTRUCTION:
                warn("Neltharion gathers Ebon Destruction!");
                castSpellAOE(SPELL_NELTHARION_EBON_DESTRUCTION);
                scriptEvents.addEvent(EVENT_NELTHARION_EBON_DESTRUCTION, 45000);
                break;
            default:
                break;
        }
    }

private:
    bool voicesRemain()
    {
        AberrusInstanceScript* pAberrus = getAberrus();
        return pAberrus != nullptr && pAberrus->anyAlive(NPC_NELTHARION_VOICE_FROM_BEYOND);
    }

    // Voices From Beyond fill the chamber
    void surrenderToCorruption()
    {
        switchStage(2);
        warn("Neltharion surrenders to corruption!");
        castSpellOnSelf(SPELL_NELTHARION_SURRENDER, true);
        castSpellAOE(SPELL_NELTHARION_RAZE_THE_EARTH);

        for (uint8_t i = 0; i < 4; ++i)
            callAdd(NPC_NELTHARION_VOICE_FROM_BEYOND, aroundSelf(15.0f, 30.0f));
        mVoicesCalled = true;

        scriptEvents.addEvent(EVENT_NELTHARION_VOICE_CHECK, 2000);
        scriptEvents.addEvent(EVENT_NELTHARION_SUNDER_SHADOW, 10000);
        scriptEvents.addEvent(EVENT_NELTHARION_CORRUPTION, 12000);
        scriptEvents.addEvent(EVENT_NELTHARION_RUSHING_DARKNESS, 16000);
        scriptEvents.addEvent(EVENT_NELTHARION_UMBRAL_ANNIHILATION, 20000);
        scriptEvents.addEvent(EVENT_NELTHARION_VOLCANIC_HEART, 24000);
        scriptEvents.addEvent(EVENT_NELTHARION_TWISTED_EARTH, 28000);
    }

    // all Voices slain: portals and aberrations
    void ripRealityOpen()
    {
        switchStage(3);
        _removeAura(SPELL_NELTHARION_SHADOW_BARRIER);
        warn("Neltharion rips open portals to the Void!");

        scriptEvents.addEvent(EVENT_NELTHARION_SUNDER_REALITY, 5000);
        scriptEvents.addEvent(EVENT_NELTHARION_CALAMITOUS_STRIKE, 9000);
        scriptEvents.addEvent(EVENT_NELTHARION_RUSHING_DARKNESS, 12000);
        scriptEvents.addEvent(EVENT_NELTHARION_EBON_DESTRUCTION, 30000);
    }

    bool mVoicesCalled = false;
    bool mBarrierRaised = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Scalecommander Sarkareth

class AberrusSarkarethAI : public AberrusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AberrusSarkarethAI(c); }
    explicit AberrusSarkarethAI(Creature* pCreature) : AberrusBossAI(pCreature, DATA_SARKARETH, SarkarethBerserkMs) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_SARKARETH_OPPRESSING_HOWL, 6000);
        scriptEvents.addEvent(EVENT_SARKARETH_BURNING_CLAWS, 10000);
        scriptEvents.addEvent(EVENT_SARKARETH_GLITTERING_SURGE, 16000);
        scriptEvents.addEvent(EVENT_SARKARETH_SCORCHING_BOMB, 20000);
        scriptEvents.addEvent(EVENT_SARKARETH_MASS_DISINTEGRATE, 26000);
        scriptEvents.addEvent(EVENT_SARKARETH_SEARING_BREATH, 30000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        int32_t const health = _getHealthPercent();
        if (mStage == 1 && health <= SarkarethSecondStagePct)
            touchTheForbidden();
        else if (mStage == 2 && health <= SarkarethThirdStagePct)
            embraceTheVoid();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SARKARETH_OPPRESSING_HOWL:
                castSpellAOE(SPELL_SARKARETH_OPPRESSING_HOWL);
                scriptEvents.addEvent(EVENT_SARKARETH_OPPRESSING_HOWL, 32000);
                break;
            case EVENT_SARKARETH_BURNING_CLAWS:
                castSpellOnVictim(SPELL_SARKARETH_BURNING_CLAWS);
                scriptEvents.addEvent(EVENT_SARKARETH_BURNING_CLAWS, 20000);
                break;
            case EVENT_SARKARETH_GLITTERING_SURGE:
                warn("Sarkareth unleashes a Glittering Surge!");
                castSpellAOE(SPELL_SARKARETH_GLITTERING_SURGE);
                scriptEvents.addEvent(EVENT_SARKARETH_GLITTERING_SURGE, 32000);
                break;
            case EVENT_SARKARETH_SCORCHING_BOMB:
                castAtRandomPlayers(SPELL_SARKARETH_SCORCHING_BOMB, 2, true);
                scriptEvents.addEvent(EVENT_SARKARETH_SCORCHING_BOMB, 28000);
                break;
            case EVENT_SARKARETH_MASS_DISINTEGRATE:
                castAtRandomPlayers(SPELL_SARKARETH_MASS_DISINTEGRATE, 3, true);
                scriptEvents.addEvent(EVENT_SARKARETH_MASS_DISINTEGRATE, 34000);
                break;
            case EVENT_SARKARETH_SEARING_BREATH:
                castAtRandom(SPELL_SARKARETH_SEARING_BREATH, false);
                scriptEvents.addEvent(EVENT_SARKARETH_SEARING_BREATH, 30000);
                break;
            case EVENT_SARKARETH_VOID_CLAWS:
                castSpellOnVictim(SPELL_SARKARETH_VOID_CLAWS);
                scriptEvents.addEvent(EVENT_SARKARETH_VOID_CLAWS, 20000);
                break;
            case EVENT_SARKARETH_VOID_BOMB:
                castAtRandomPlayers(SPELL_SARKARETH_VOID_BOMB, 2, true);
                scriptEvents.addEvent(EVENT_SARKARETH_VOID_BOMB, 30000);
                break;
            case EVENT_SARKARETH_ABYSSAL_BREATH:
                castAtRandom(SPELL_SARKARETH_ABYSSAL_BREATH, false);
                leaveImages(true);
                scriptEvents.addEvent(EVENT_SARKARETH_EBON_MIGHT, 8000);
                scriptEvents.addEvent(EVENT_SARKARETH_ABYSSAL_BREATH, 45000);
                break;
            case EVENT_SARKARETH_EBON_MIGHT:
                castSpellAOE(SPELL_SARKARETH_EBON_MIGHT);
                break;
            case EVENT_SARKARETH_DESOLATE_BLOSSOM:
                castSpellAOE(SPELL_SARKARETH_DESOLATE_BLOSSOM);
                scriptEvents.addEvent(EVENT_SARKARETH_DESOLATE_BLOSSOM, 35000);
                break;
            case EVENT_SARKARETH_INFINITE_DURESS:
                castAtRandomPlayers(SPELL_SARKARETH_INFINITE_DURESS, 2, true);
                scriptEvents.addEvent(EVENT_SARKARETH_INFINITE_DURESS, 40000);
                break;
            case EVENT_SARKARETH_VOID_SLASH:
                castSpellOnVictim(SPELL_SARKARETH_VOID_SLASH);
                scriptEvents.addEvent(EVENT_SARKARETH_VOID_SLASH, 20000);
                break;
            case EVENT_SARKARETH_HURTLING_BARRAGE:
                castAtRandomPlayers(SPELL_SARKARETH_HURTLING_BARRAGE, 2, true);
                scriptEvents.addEvent(EVENT_SARKARETH_HURTLING_BARRAGE, 30000);
                break;
            case EVENT_SARKARETH_EMBRACE_OF_NOTHINGNESS:
                castAtRandom(SPELL_SARKARETH_EMBRACE_OF_NOTHINGNESS, true);
                scriptEvents.addEvent(EVENT_SARKARETH_EMBRACE_OF_NOTHINGNESS, 40000);
                break;
            case EVENT_SARKARETH_SCOURING_ETERNITY:
                warn("Sarkareth begins to cast Scouring Eternity!");
                castSpellAOE(SPELL_SARKARETH_SCOURING_ETERNITY);
                scriptEvents.addEvent(EVENT_SARKARETH_SCOURING_ETERNITY, 60000);
                break;
            default:
                break;
        }
    }

private:
    void touchTheForbidden()
    {
        switchStage(2);
        warn("Sarkareth draws upon the power of the Void!");
        scriptEvents.addEvent(EVENT_SARKARETH_VOID_CLAWS, 8000);
        scriptEvents.addEvent(EVENT_SARKARETH_VOID_BOMB, 12000);
        scriptEvents.addEvent(EVENT_SARKARETH_ABYSSAL_BREATH, 20000);
        scriptEvents.addEvent(EVENT_SARKARETH_DESOLATE_BLOSSOM, 28000);
        scriptEvents.addEvent(EVENT_SARKARETH_INFINITE_DURESS, 34000);
    }

    void embraceTheVoid()
    {
        switchStage(3);
        warn("Sarkareth fully embraces the Void!");
        castSpellOnSelf(SPELL_SARKARETH_COSMIC_ASCENSION, true);
        leaveImages(false);

        scriptEvents.addEvent(EVENT_SARKARETH_VOID_SLASH, 8000);
        scriptEvents.addEvent(EVENT_SARKARETH_VOID_BOMB, 14000);
        scriptEvents.addEvent(EVENT_SARKARETH_HURTLING_BARRAGE, 18000);
        scriptEvents.addEvent(EVENT_SARKARETH_EMBRACE_OF_NOTHINGNESS, 26000);
        scriptEvents.addEvent(EVENT_SARKARETH_INFINITE_DURESS, 32000);
        scriptEvents.addEvent(EVENT_SARKARETH_SCOURING_ETERNITY, 45000);
    }

    // images left behind by the breath or the ascension
    void leaveImages(bool withRecollection)
    {
        for (uint8_t i = 0; i < 2; ++i)
            callAdd(NPC_SARKARETH_NULL_GLIMMER, aroundSelf(6.0f, 14.0f));

        if (withRecollection)
            callAdd(NPC_SARKARETH_EMPTY_RECOLLECTION, aroundSelf(6.0f, 14.0f));
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Amalgamation Chamber

// Eternal Blaze and Essence of Shadow, merged once one of them is down to half
class AberrusChamberElementalAI : public AberrusBossAI
{
public:
    explicit AberrusChamberElementalAI(Creature* pCreature) : AberrusBossAI(pCreature, DATA_AMALGAMATION, AmalgamationBerserkMs) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        // nobody falls alone while the merge is still ahead
        AberrusInstanceScript* pAberrus = getAberrus();
        if (damage == nullptr || pAberrus == nullptr || pAberrus->isMerged())
            return;

        uint32_t const health = getCreature()->getHealth();
        if (*damage >= health)
            *damage = health > 1 ? health - 1 : 0;
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_CHAMBER_INSTABILITY_CHECK, 2000);
        onElementalEngage();
    }

    void onDisengage() override { _removeAura(SPELL_CHAMBER_ELEMENTAL_INSTABILITY); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        AberrusInstanceScript* pAberrus = getAberrus();
        if (pAberrus == nullptr || pAberrus->isMerged() || _getHealthPercent() > ChamberMergePct)
            return;

        formAmalgamation(pAberrus);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_CHAMBER_INSTABILITY_CHECK)
        {
            onElementalEvent(eventId);
            return;
        }

        // close to the partner both grow unstable
        bool const close = partnerInRange();
        if (close && !hasAura(SPELL_CHAMBER_ELEMENTAL_INSTABILITY))
            castSpellOnSelf(SPELL_CHAMBER_ELEMENTAL_INSTABILITY, true);
        else if (!close && hasAura(SPELL_CHAMBER_ELEMENTAL_INSTABILITY))
            _removeAura(SPELL_CHAMBER_ELEMENTAL_INSTABILITY);

        scriptEvents.addEvent(EVENT_CHAMBER_INSTABILITY_CHECK, 2000);
    }

    virtual void onElementalEngage() {}
    virtual void onElementalEvent(uint32_t /*eventId*/) {}

private:
    bool partnerInRange()
    {
        AberrusInstanceScript* pAberrus = getAberrus();
        if (pAberrus == nullptr)
            return false;

        for (Creature* pMember : pAberrus->getMembers(DATA_AMALGAMATION))
        {
            if (pMember == getCreature() || pMember->getEntry() == BOSS_CHAMBER_AMALGAMATION || !pMember->isAlive())
                continue;

            if (getCreature()->getDistance(pMember) <= ChamberInstabilityRange)
                return true;
        }

        return false;
    }

    // both elementals pour their remaining health into the Amalgamation
    void formAmalgamation(AberrusInstanceScript* pAberrus)
    {
        if (!pAberrus->beginMerge())
            return;

        std::vector<Creature*> elementals;
        uint64_t health = 0;
        uint64_t maxHealth = 0;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        for (Creature* pMember : pAberrus->getMembers(DATA_AMALGAMATION))
        {
            if (pMember->getEntry() == BOSS_CHAMBER_AMALGAMATION || !pMember->isAlive())
                continue;

            elementals.push_back(pMember);
            health += pMember->getHealth();
            maxHealth += pMember->getMaxHealth();
            x += pMember->GetPositionX();
            y += pMember->GetPositionY();
            z += pMember->GetPositionZ();
        }

        float const count = static_cast<float>(elementals.size());
        LocationVector const spot(x / count, y / count, z / count, getCreature()->GetOrientation());

        Creature* pAmalgamation = nullptr;
        for (Creature* pPlaced : pAberrus->getCreatureSetForEntry(BOSS_CHAMBER_AMALGAMATION))
        {
            if (pPlaced->isAlive())
                pAmalgamation = pPlaced;
        }

        if (pAmalgamation == nullptr)
            pAmalgamation = spawnCreature(BOSS_CHAMBER_AMALGAMATION, spot);

        // without the Amalgamation both elementals fight to the end
        if (pAmalgamation == nullptr)
        {
            pAberrus->endMerge();
            return;
        }

        uint64_t const share = maxHealth != 0 ? static_cast<uint64_t>(pAmalgamation->getMaxHealth()) * health / maxHealth : pAmalgamation->getMaxHealth();
        pAmalgamation->setHealth(share > 0 ? static_cast<uint32_t>(share) : 1);

        warn("The Eternal Blaze and the Essence of Shadow combine into the Shadowflame Amalgamation!");
        setZoneWideCombat(pAmalgamation);

        for (Creature* pElemental : elementals)
            pElemental->dealDamage(pElemental, pElemental->getHealth(), 0);

        pAberrus->endMerge();
    }
};

class AberrusEternalBlazeAI : public AberrusChamberElementalAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AberrusEternalBlazeAI(c); }
    explicit AberrusEternalBlazeAI(Creature* pCreature) : AberrusChamberElementalAI(pCreature) {}

protected:
    void onElementalEngage() override
    {
        scriptEvents.addEvent(EVENT_BLAZE_BLAZING_HEAT, 5000);
        scriptEvents.addEvent(EVENT_BLAZE_FLAME_SLASH, 8000);
        scriptEvents.addEvent(EVENT_BLAZE_FIERY_METEOR, 18000);
        scriptEvents.addEvent(EVENT_BLAZE_SWIRLING_FLAME, 26000);
    }

    void onElementalEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BLAZE_BLAZING_HEAT:
                castAtRandomPlayers(SPELL_BLAZE_BLAZING_HEAT, 2, true);
                scriptEvents.addEvent(EVENT_BLAZE_BLAZING_HEAT, 20000);
                break;
            case EVENT_BLAZE_FLAME_SLASH:
                castSpellOnVictim(SPELL_BLAZE_FLAME_SLASH);
                scriptEvents.addEvent(EVENT_BLAZE_FLAME_SLASH, 16000);
                break;
            case EVENT_BLAZE_FIERY_METEOR:
                if (castAtRandom(SPELL_BLAZE_FIERY_METEOR, true) != nullptr)
                    warn("The Eternal Blaze calls down a Fiery Meteor!");
                scriptEvents.addEvent(EVENT_BLAZE_FIERY_METEOR, 35000);
                break;
            case EVENT_BLAZE_SWIRLING_FLAME:
                castSpellAOE(SPELL_BLAZE_SWIRLING_FLAME);
                scriptEvents.addEvent(EVENT_BLAZE_SWIRLING_FLAME, 30000);
                break;
            default:
                break;
        }
    }
};

class AberrusEssenceOfShadowAI : public AberrusChamberElementalAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AberrusEssenceOfShadowAI(c); }
    explicit AberrusEssenceOfShadowAI(Creature* pCreature) : AberrusChamberElementalAI(pCreature) {}

protected:
    void onElementalEngage() override
    {
        scriptEvents.addEvent(EVENT_SHADOW_CORRUPTING_SHADOW, 4000);
        scriptEvents.addEvent(EVENT_SHADOW_SHADOW_SPIKE, 7000);
        scriptEvents.addEvent(EVENT_SHADOW_UMBRAL_DETONATION, 14000);
        scriptEvents.addEvent(EVENT_SHADOW_COALESCING_VOID, 24000);
    }

    void onElementalEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SHADOW_CORRUPTING_SHADOW:
                castAtRandomPlayers(SPELL_SHADOW_CORRUPTING_SHADOW, 2, true);
                scriptEvents.addEvent(EVENT_SHADOW_CORRUPTING_SHADOW, 20000);
                break;
            case EVENT_SHADOW_SHADOW_SPIKE:
                castSpellOnVictim(SPELL_SHADOW_SHADOW_SPIKE);
                scriptEvents.addEvent(EVENT_SHADOW_SHADOW_SPIKE, 16000);
                break;
            case EVENT_SHADOW_UMBRAL_DETONATION:
                castAtRandomPlayers(SPELL_SHADOW_UMBRAL_DETONATION, 3, true);
                scriptEvents.addEvent(EVENT_SHADOW_UMBRAL_DETONATION, 28000);
                break;
            case EVENT_SHADOW_COALESCING_VOID:
                castSpellAOE(SPELL_SHADOW_COALESCING_VOID);
                scriptEvents.addEvent(EVENT_SHADOW_COALESCING_VOID, 35000);
                break;
            default:
                break;
        }
    }
};

class AberrusShadowflameAmalgamationAI : public AberrusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AberrusShadowflameAmalgamationAI(c); }
    explicit AberrusShadowflameAmalgamationAI(Creature* pCreature) : AberrusBossAI(pCreature, DATA_AMALGAMATION, AmalgamationBerserkMs) {}

protected:
    void onEngage() override
    {
        // grows until the Amalgamation is defeated
        castSpellOnSelf(SPELL_AMALGAMATION_CONTAMINATION, true);

        scriptEvents.addEvent(EVENT_AMALGAMATION_WITHERING_VULNERABILITY, 6000);
        scriptEvents.addEvent(EVENT_AMALGAMATION_SHADOW_AND_FLAME, 10000);
        scriptEvents.addEvent(EVENT_AMALGAMATION_GLOOM_CONFLAGRATION, 18000);
        scriptEvents.addEvent(EVENT_AMALGAMATION_BLISTERING_TWILIGHT, 26000);
        scriptEvents.addEvent(EVENT_AMALGAMATION_CONVERGENT_ERUPTION, 34000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AMALGAMATION_WITHERING_VULNERABILITY:
                castSpellOnVictim(SPELL_AMALGAMATION_WITHERING_VULNERABILITY);
                scriptEvents.addEvent(EVENT_AMALGAMATION_SHADOWFLAME_BURST, 3000);
                scriptEvents.addEvent(EVENT_AMALGAMATION_WITHERING_VULNERABILITY, 24000);
                break;
            case EVENT_AMALGAMATION_SHADOWFLAME_BURST:
                castSpellAOE(SPELL_AMALGAMATION_SHADOWFLAME_BURST);
                break;
            case EVENT_AMALGAMATION_SHADOW_AND_FLAME:
                castAtRandomPlayers(SPELL_AMALGAMATION_SHADOW_AND_FLAME, 3, true);
                scriptEvents.addEvent(EVENT_AMALGAMATION_SHADOW_AND_FLAME, 22000);
                break;
            case EVENT_AMALGAMATION_GLOOM_CONFLAGRATION:
                warn("The Shadowflame Amalgamation begins a Gloom Conflagration!");
                castSpellAOE(SPELL_AMALGAMATION_GLOOM_CONFLAGRATION);
                scriptEvents.addEvent(EVENT_AMALGAMATION_GLOOM_CONFLAGRATION, 35000);
                break;
            case EVENT_AMALGAMATION_BLISTERING_TWILIGHT:
                castAtRandomPlayers(SPELL_AMALGAMATION_BLISTERING_TWILIGHT, 2, true);
                scriptEvents.addEvent(EVENT_AMALGAMATION_BLISTERING_TWILIGHT, 30000);
                break;
            case EVENT_AMALGAMATION_CONVERGENT_ERUPTION:
                castSpellAOE(SPELL_AMALGAMATION_CONVERGENT_ERUPTION);
                scriptEvents.addEvent(EVENT_AMALGAMATION_CONVERGENT_ERUPTION, 40000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kazzara, the Hellforged

class AberrusKazzaraAI : public AberrusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AberrusKazzaraAI(c); }
    explicit AberrusKazzaraAI(Creature* pCreature) : AberrusBossAI(pCreature, DATA_KAZZARA, KazzaraBerserkMs) {}

protected:
    void onEngage() override
    {
        mPlatesLost = 0;
        startEnergy(KazzaraEnergyStepMs);
        scriptEvents.addEvent(EVENT_KAZZARA_TERROR_CLAWS, 9000);
        scriptEvents.addEvent(EVENT_KAZZARA_DREAD_RIFTS, 14000);
        scriptEvents.addEvent(EVENT_KAZZARA_RAYS_OF_ANGUISH, 22000);
        scriptEvents.addEvent(EVENT_KAZZARA_WINGS_OF_EXTINCTION, 30000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (_isCasting())
            return;

        // armor plates break off at 80%, 60% and 40%
        if (mPlatesLost < 3 && _getHealthPercent() <= KazzaraCarnageMarks[mPlatesLost])
        {
            ++mPlatesLost;
            warn("Plates of Kazzara's armor shatter in Hellsteel Carnage!");
            castSpellOnSelf(SPELL_KAZZARA_HELLSTEEL_CARNAGE, true);
            return;
        }

        if (isEnergyFull())
        {
            warn("Kazzara unleashes a Hellbeam!");
            castSpellAOE(SPELL_KAZZARA_HELLBEAM);
            startEnergy(KazzaraEnergyStepMs);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KAZZARA_TERROR_CLAWS:
                castSpellOnVictim(SPELL_KAZZARA_TERROR_CLAWS);
                scriptEvents.addEvent(EVENT_KAZZARA_TERROR_CLAWS, 15000);
                break;
            case EVENT_KAZZARA_DREAD_RIFTS:
                castAtRandomPlayers(SPELL_KAZZARA_DREAD_RIFTS, 3, true);
                scriptEvents.addEvent(EVENT_KAZZARA_DREAD_RIFTS, 35000);
                break;
            case EVENT_KAZZARA_RAYS_OF_ANGUISH:
                castAtRandom(SPELL_KAZZARA_RAYS_OF_ANGUISH, true);
                scriptEvents.addEvent(EVENT_KAZZARA_RAYS_OF_ANGUISH, 30000);
                break;
            case EVENT_KAZZARA_WINGS_OF_EXTINCTION:
                castSpellAOE(SPELL_KAZZARA_WINGS_OF_EXTINCTION);
                scriptEvents.addEvent(EVENT_KAZZARA_WINGS_OF_EXTINCTION, 38000);
                break;
            default:
                break;
        }
    }

private:
    uint8_t mPlatesLost = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Vigilant Steward, Zskarn

class AberrusZskarnAI : public AberrusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AberrusZskarnAI(c); }
    explicit AberrusZskarnAI(Creature* pCreature) : AberrusBossAI(pCreature, DATA_ZSKARN, ZskarnBerserkMs) {}

protected:
    void onEngage() override
    {
        castSpellAOE(SPELL_ZSKARN_DRAGONFIRE_TRAPS, true);
        startEnergy(ZskarnEnergyStepMs);
        scriptEvents.addEvent(EVENT_ZSKARN_SEARING_CLAWS, 6000);
        scriptEvents.addEvent(EVENT_ZSKARN_ANIMATE_GOLEMS, 10000);
        scriptEvents.addEvent(EVENT_ZSKARN_BLAST_WAVE, 15000);
        scriptEvents.addEvent(EVENT_ZSKARN_UNSTABLE_EMBERS, 20000);
        scriptEvents.addEvent(EVENT_ZSKARN_SHRAPNEL_BOMB, 28000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!isEnergyFull() || _isCasting())
            return;

        // the dragon statues answer at full energy
        warn("Zskarn unleashes Tactical Destruction!");
        castSpellAOE(SPELL_ZSKARN_TACTICAL_DESTRUCTION);
        startEnergy(ZskarnEnergyStepMs);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ZSKARN_SEARING_CLAWS:
                castSpellOnVictim(SPELL_ZSKARN_SEARING_CLAWS);
                scriptEvents.addEvent(EVENT_ZSKARN_SEARING_CLAWS, 12000);
                break;
            case EVENT_ZSKARN_ANIMATE_GOLEMS:
                castSpellOnSelf(SPELL_ZSKARN_ANIMATE_GOLEMS);
                for (uint8_t i = 0; i < (isHeroic() ? 2 : 1); ++i)
                    callAdd(NPC_ZSKARN_DRAGONFIRE_GOLEM, aroundSelf(10.0f, 20.0f));
                scriptEvents.addEvent(EVENT_ZSKARN_ANIMATE_GOLEMS, 60000);
                break;
            case EVENT_ZSKARN_BLAST_WAVE:
                castSpellAOE(SPELL_ZSKARN_BLAST_WAVE);
                scriptEvents.addEvent(EVENT_ZSKARN_BLAST_WAVE, 30000);
                break;
            case EVENT_ZSKARN_UNSTABLE_EMBERS:
                castAtRandomPlayers(SPELL_ZSKARN_UNSTABLE_EMBERS, 3, true);
                scriptEvents.addEvent(EVENT_ZSKARN_UNSTABLE_EMBERS, 30000);
                break;
            case EVENT_ZSKARN_SHRAPNEL_BOMB:
                castSpellAOE(SPELL_ZSKARN_SHRAPNEL_BOMB);
                scriptEvents.addEvent(EVENT_ZSKARN_SHRAPNEL_BOMB, 40000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Forgotten Experiments

// later experiments wait in their vats until the one before them falls
class AberrusExperimentAI : public AberrusBossAI
{
public:
    AberrusExperimentAI(Creature* pCreature, bool startsInVat, char const* breakFreeText)
        : AberrusBossAI(pCreature, DATA_EXPERIMENTS, ExperimentsBerserkMs), mStartsInVat(startsInVat), mBreakFreeText(breakFreeText) {}

    void OnLoad() override
    {
        if (mStartsInVat && onAberrus())
            sealInVat();
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_EXPERIMENT_BREAK_FREE || !mInVat || !isAlive())
            return;

        mInVat = false;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setCanEnterCombat(true);
        if (mBreakFreeText != nullptr)
            warn(mBreakFreeText);

        setZoneWideCombat();
    }

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_EXPERIMENT_INFUSED_STRIKES, true);
        onExperimentEngage();
    }

    void onDisengage() override
    {
        if (mStartsInVat && isAlive())
            sealInVat();
    }

    void onDefeat() override
    {
        if (AberrusInstanceScript* pAberrus = getAberrus())
            pAberrus->releaseExperimentAfter(getCreature()->getEntry());
    }

    virtual void onExperimentEngage() {}

private:
    void sealInVat()
    {
        mInVat = true;
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setCanEnterCombat(false);
    }

    bool mStartsInVat;
    char const* mBreakFreeText;
    bool mInVat = false;
};

class AberrusNeldrisAI : public AberrusExperimentAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AberrusNeldrisAI(c); }
    explicit AberrusNeldrisAI(Creature* pCreature) : AberrusExperimentAI(pCreature, false, nullptr) {}

protected:
    void onExperimentEngage() override
    {
        scriptEvents.addEvent(EVENT_NELDRIS_MASSIVE_SLAM, 8000);
        scriptEvents.addEvent(EVENT_NELDRIS_BELLOWING_ROAR, 14000);
        scriptEvents.addEvent(EVENT_NELDRIS_RENDING_CHARGE, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NELDRIS_MASSIVE_SLAM:
                castSpellOnVictim(SPELL_NELDRIS_MASSIVE_SLAM);
                scriptEvents.addEvent(EVENT_NELDRIS_MASSIVE_SLAM, 20000);
                break;
            case EVENT_NELDRIS_BELLOWING_ROAR:
                castSpellAOE(SPELL_NELDRIS_BELLOWING_ROAR);
                scriptEvents.addEvent(EVENT_NELDRIS_BELLOWING_ROAR, 25000);
                break;
            case EVENT_NELDRIS_RENDING_CHARGE:
                warn("Neldris marks her prey for a Rending Charge!");
                castAtRandomPlayers(SPELL_NELDRIS_RENDING_CHARGE, 3, true);
                scriptEvents.addEvent(EVENT_NELDRIS_RENDING_CHARGE, 30000);
                break;
            default:
                break;
        }
    }
};

class AberrusThadrionAI : public AberrusExperimentAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AberrusThadrionAI(c); }
    explicit AberrusThadrionAI(Creature* pCreature) : AberrusExperimentAI(pCreature, true, "Thadrion bursts from his containment vat!") {}

protected:
    void onExperimentEngage() override
    {
        scriptEvents.addEvent(EVENT_THADRION_UNSTABLE_ESSENCE, 6000);
        scriptEvents.addEvent(EVENT_THADRION_VOLATILE_SPEW, 12000);
        scriptEvents.addEvent(EVENT_THADRION_VIOLENT_ERUPTION, 25000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_THADRION_UNSTABLE_ESSENCE:
                castAtRandom(SPELL_THADRION_UNSTABLE_ESSENCE, true);
                scriptEvents.addEvent(EVENT_THADRION_UNSTABLE_ESSENCE, 25000);
                break;
            case EVENT_THADRION_VOLATILE_SPEW:
                castSpellAOE(SPELL_THADRION_VOLATILE_SPEW);
                scriptEvents.addEvent(EVENT_THADRION_VOLATILE_SPEW, 30000);
                break;
            case EVENT_THADRION_VIOLENT_ERUPTION:
                castSpellAOE(SPELL_THADRION_VIOLENT_ERUPTION);
                scriptEvents.addEvent(EVENT_THADRION_VIOLENT_ERUPTION, 40000);
                break;
            default:
                break;
        }
    }
};

class AberrusRionthusAI : public AberrusExperimentAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AberrusRionthusAI(c); }
    explicit AberrusRionthusAI(Creature* pCreature) : AberrusExperimentAI(pCreature, true, "Rionthus escapes his containment vat!") {}

protected:
    void onExperimentEngage() override
    {
        scriptEvents.addEvent(EVENT_RIONTHUS_DISINTEGRATE, 8000);
        scriptEvents.addEvent(EVENT_RIONTHUS_TEMPORAL_ANOMALY, 14000);
        scriptEvents.addEvent(EVENT_RIONTHUS_DEEP_BREATH, 22000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RIONTHUS_DISINTEGRATE:
                castAtRandom(SPELL_RIONTHUS_DISINTEGRATE, true);
                scriptEvents.addEvent(EVENT_RIONTHUS_DISINTEGRATE, 20000);
                break;
            case EVENT_RIONTHUS_TEMPORAL_ANOMALY:
                castSpellAOE(SPELL_RIONTHUS_TEMPORAL_ANOMALY);
                scriptEvents.addEvent(EVENT_RIONTHUS_TEMPORAL_ANOMALY, 30000);
                break;
            case EVENT_RIONTHUS_DEEP_BREATH:
                if (castAtRandom(SPELL_RIONTHUS_DEEP_BREATH, false) != nullptr)
                    warn("Rionthus takes a Deep Breath!");
                scriptEvents.addEvent(EVENT_RIONTHUS_DEEP_BREATH, 40000);
                break;
            default:
                break;
        }
    }
};

void SetupAberrus(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ABERRUS, &AberrusInstanceScript::Create);

    // Rashok, the Elder
    mgr->register_creature_script(BOSS_RASHOK, &AberrusRashokAI::Create);

    // Assault of the Zaqali
    mgr->register_creature_script(BOSS_KAGNI, &AberrusKagniAI::Create);
    mgr->register_creature_script(NPC_ZAQALI_IGNARA, &AberrusIgnaraAI::Create);
    mgr->register_creature_script(NPC_ZAQALI_FLAMEBOUND_HUNTSMAN, &AberrusAddAI::CreateHuntsman);
    mgr->register_creature_script(NPC_ZAQALI_MAGMA_MYSTIC, &AberrusAddAI::CreateMystic);
    mgr->register_creature_script(NPC_ZAQALI_OBSIDIAN_GUARD, &AberrusAddAI::CreateObsidianGuard);
    mgr->register_creature_script(NPC_ZAQALI_RENEWED_HATCHLING, &AberrusAddAI::CreateHatchling);

    // Magmorax
    mgr->register_creature_script(BOSS_MAGMORAX, &AberrusMagmoraxAI::Create);

    // Echo of Neltharion
    mgr->register_creature_script(BOSS_NELTHARION_ECHO, &AberrusNeltharionAI::Create);
    mgr->register_creature_script(NPC_NELTHARION_VOICE_FROM_BEYOND, &AberrusAddAI::CreateVoice);
    mgr->register_creature_script(NPC_NELTHARION_TWISTED_ABERRATION, &AberrusAddAI::CreateAberration);

    // Scalecommander Sarkareth
    mgr->register_creature_script(BOSS_SARKARETH, &AberrusSarkarethAI::Create);
    mgr->register_creature_script(NPC_SARKARETH_EMPTY_RECOLLECTION, &AberrusAddAI::CreateRecollection);
    mgr->register_creature_script(NPC_SARKARETH_NULL_GLIMMER, &AberrusAddAI::CreateGlimmer);

    // The Amalgamation Chamber
    mgr->register_creature_script(BOSS_CHAMBER_ETERNAL_BLAZE, &AberrusEternalBlazeAI::Create);
    mgr->register_creature_script(BOSS_CHAMBER_ESSENCE_OF_SHADOW, &AberrusEssenceOfShadowAI::Create);
    mgr->register_creature_script(BOSS_CHAMBER_AMALGAMATION, &AberrusShadowflameAmalgamationAI::Create);

    // Kazzara, the Hellforged
    mgr->register_creature_script(BOSS_KAZZARA, &AberrusKazzaraAI::Create);

    // The Vigilant Steward, Zskarn
    mgr->register_creature_script(BOSS_ZSKARN, &AberrusZskarnAI::Create);
    mgr->register_creature_script(BOSS_ZSKARN_ALT, &AberrusZskarnAI::Create);
    mgr->register_creature_script(NPC_ZSKARN_DRAGONFIRE_GOLEM, &AberrusAddAI::CreateGolem);

    // The Forgotten Experiments
    mgr->register_creature_script(BOSS_EXPERIMENT_NELDRIS, &AberrusNeldrisAI::Create);
    mgr->register_creature_script(BOSS_EXPERIMENT_THADRION, &AberrusThadrionAI::Create);
    mgr->register_creature_script(BOSS_EXPERIMENT_RIONTHUS, &AberrusRionthusAI::Create);
}
