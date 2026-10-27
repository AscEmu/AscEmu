/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Eye_Of_Azshara.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <set>
#include <utility>
#include <vector>

enum EyeOfAzsharaEvents
{
    EVENT_PARJESH_CRASHING_WAVE = 1,
    EVENT_PARJESH_REINFORCEMENTS,
    EVENT_PARJESH_THROW_SPEAR,
    EVENT_PARJESH_IMPALING_SPEAR,

    EVENT_HATECOIL_STATIC_NOVA,
    EVENT_HATECOIL_CURSE_OF_THE_WITCH,
    EVENT_HATECOIL_BECKON_STORM,
    EVENT_HATECOIL_FOCUSED_LIGHTNING,

    EVENT_DEEPBEARD_GROUND_SLAM,
    EVENT_DEEPBEARD_GASEOUS_BUBBLES,
    EVENT_DEEPBEARD_QUAKE,
    EVENT_DEEPBEARD_CALL_THE_SEAS,

    EVENT_SERPENTRIX_TOXIC_WOUND,
    EVENT_SERPENTRIX_POISON_SPIT,
    EVENT_SERPENTRIX_REACH_CHECK,
    EVENT_SERPENTRIX_RESURFACE,
    EVENT_HYDRA_SPAWN_ATTACK,

    EVENT_WRATH_MYSTIC_TORNADO,
    EVENT_WRATH_MASSIVE_DELUGE,
    EVENT_WRATH_ARCANE_BOMB,
    EVENT_WRATH_CRUSHING_DEPTHS,
    EVENT_WRATH_REACH_CHECK,

    EVENT_RITUALIST_LIGHTNING_BLAST,
    EVENT_RITUALIST_SIGNATURE,
    EVENT_RITUALIST_ARCANE_BOMB
};

namespace
{
    float const EyeOfAzsharaTwoPi = 6.28318531f;

    // Parjesh calls his guards to the rim of his platform
    LocationVector const ParjeshPlatformCentre(-3682.64f, 4417.02f, 32.44f);
    float const ParjeshReinforcementRange = 38.0f;
    // energy fills at four points per second
    uint32_t const ParjeshSpearEnergyMs = 25000;

    // the shield holds while more arcanists than this live
    uint32_t const HatecoilShieldArcanists = 2;
    uint32_t const HatecoilCurseTargets[] = { 1, 3, 5 };

    // energy starts at 30 and fills at five points per second
    uint32_t const DeepbeardQuakeFirstMs = 14000;
    uint32_t const DeepbeardQuakeCycleMs = 22000;

    LocationVector const SerpentrixSurfaceSpots[] =
    {
        { -3256.36f, 4370.39f,  0.37f },
        { -3294.20f, 4460.52f, -0.60f },
        { -3304.17f, 4405.53f,  0.08f },
        { -3193.61f, 4435.89f, -0.70f },
        { -3199.40f, 4384.95f,  0.16f },
        { -3246.71f, 4479.65f,  0.26f }
    };
    uint32_t const SerpentrixSurfaceSpotCount = 6;

    // the storm takes shape inside the bubble
    LocationVector const WrathOfAzsharaEye(-3485.11f, 4386.22f, -5.58f, 0.0f);
    uint32_t const ArcaneBombFuseMs = 15000;

    // point on a ring around centre, put on the ground and facing inwards
    LocationVector eyeRingPoint(Creature* pAnchor, LocationVector const& centre, float range)
    {
        float const angle = Util::getRandomFloat(EyeOfAzsharaTwoPi);
        float const x = centre.x + range * std::cos(angle);
        float const y = centre.y + range * std::sin(angle);
        float z = centre.z;
        pAnchor->updateGroundPositionZ(x, y, z);
        return LocationVector(x, y, z, std::atan2(centre.y - y, centre.x - x));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class EyeOfAzsharaInstanceScript : public InstanceScript
{
public:
    explicit EyeOfAzsharaInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(EyeOfAzsharaEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new EyeOfAzsharaInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_WRATH_OF_AZSHARA:
                mWrathGuid = pCreature->GetNewGUID();
                break;
            case NPC_HATECOIL_ARCANIST:
                mArcanistGuids.insert(pCreature->GetNewGUID());
                break;
            case NPC_MYSTIC_SSAVEH:
            case NPC_RITUALIST_LESHA:
            case NPC_CHANNELER_VARISZ:
            case NPC_BINDER_ASHIOI:
                mRitualistGuids.insert(pCreature->GetNewGUID());
                break;
            default:
                break;
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_DEEPBEARD_WATER_DOOR_1:
            case GO_DEEPBEARD_WATER_DOOR_2:
                mWaterDoorGuids.insert(pGameObject->GetNewGUID());
                alignDoor(pGameObject, !isFighting(DATA_KING_DEEPBEARD));
                break;
            case GO_AZSHARA_STORM_BUBBLE:
                mBubbleGuid = pGameObject->GetNewGUID();
                if (mStormAwake)
                    pGameObject->despawn(1000, 0);
                break;
            default:
                break;
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t /*state*/) override
    {
        if (id != DATA_KING_DEEPBEARD)
            return;

        for (WoWGuid const& guid : mWaterDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                alignDoor(pDoor, !isFighting(DATA_KING_DEEPBEARD));
        }
    }

    // the storm wakes once the four guardians are down
    void UpdateEvent() override
    {
        if (mStormAwake || !guardiansDefeated())
            return;

        mStormAwake = true;

        if (GameObject* pBubble = getGameObjectByGuid(mBubbleGuid))
            pBubble->despawn(1000, 0);

        if (getBossState(DATA_WRATH_OF_AZSHARA) == EncounterStates::Performed || getCreatureByGuid(mWrathGuid) != nullptr)
            return;

        if (Creature* pWrath = spawnCreature(BOSS_WRATH_OF_AZSHARA, WrathOfAzsharaEye.x, WrathOfAzsharaEye.y, WrathOfAzsharaEye.z, WrathOfAzsharaEye.o))
            mWrathGuid = pWrath->GetNewGUID();
    }

    bool isStormAwake() const { return mStormAwake; }
    uint32_t livingArcanists() { return countLiving(mArcanistGuids); }
    uint32_t livingRitualists() { return countLiving(mRitualistGuids); }

private:
    bool isFighting(uint32_t id) const { return getBossState(id) == EncounterStates::InProgress; }

    bool guardiansDefeated() const
    {
        for (uint32_t id = DATA_WARLORD_PARJESH; id < DATA_WRATH_OF_AZSHARA; ++id)
        {
            if (getBossState(id) != EncounterStates::Performed)
                return false;
        }

        return true;
    }

    uint32_t countLiving(std::set<WoWGuid> const& guids)
    {
        uint32_t living = 0;
        for (WoWGuid const& guid : guids)
        {
            Creature* pCreature = getCreatureByGuid(guid);
            if (pCreature != nullptr && pCreature->isAlive())
                ++living;
        }

        return living;
    }

    // changes the state only when it differs
    void alignDoor(GameObject* pDoor, bool open)
    {
        bool const isOpen = pDoor->getState() != GO_STATE_CLOSED;
        if (isOpen != open)
            pDoor->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    bool mStormAwake = false;
    WoWGuid mWrathGuid;
    WoWGuid mBubbleGuid;
    std::set<WoWGuid> mArcanistGuids;
    std::set<WoWGuid> mRitualistGuids;
    std::set<WoWGuid> mWaterDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick in combat, idle tick outside

class EyeOfAzsharaCombatAI : public CreatureAIScript
{
public:
    explicit EyeOfAzsharaCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        onDisengage();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (!_isInCombat())
        {
            onIdleTick(diff);
            return;
        }

        scriptEvents.updateEvents(diff, 0);
        onCombatTick();

        if (_isCasting())
            return;

        if (uint32_t eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onIdleTick(uint32_t /*diff*/) {}
    virtual void onCombatTick() {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    EyeOfAzsharaInstanceScript* getEyeOfAzshara()
    {
        if (getCreature()->GetMapId() != MAP_EYE_OF_AZSHARA)
            return nullptr;

        return static_cast<EyeOfAzsharaInstanceScript*>(getInstanceScript());
    }

    Unit* randomPlayer(bool avoidVictim = false)
    {
        if (avoidVictim)
        {
            FilterArgs args(TargetFilter_Player);
            args.addFilter(TargetFilter_NotCurrent);
            if (Unit* pTarget = selectUnitTarget(args))
                return pTarget;
        }

        return selectUnitTarget(FilterArgs(TargetFilter_Player));
    }

    void castOnRandomPlayer(uint32_t spellId, bool triggered = false)
    {
        if (Unit* pTarget = randomPlayer())
            castSpell(pTarget, spellId, triggered);
    }

    // living players in range in random order
    std::vector<Unit*> playersAround(float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = pObject != nullptr ? pObject->ToUnit() : nullptr;
            if (pPlayer != nullptr && pPlayer->isAlive() && getCreature()->getDistance(pPlayer) <= range)
                players.push_back(pPlayer);
        }

        for (size_t i = players.size(); i > 1; --i)
            std::swap(players[i - 1], players[Util::getRandomUInt(static_cast<uint32_t>(i - 1))]);

        return players;
    }

    bool anyPlayerInMelee()
    {
        for (Unit* pPlayer : playersAround(30.0f))
        {
            if (getCreature()->isWithinMeleeRange(pPlayer))
                return true;
        }

        return false;
    }

    bool victimInMelee()
    {
        Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
        return pVictim == nullptr || getCreature()->isWithinMeleeRange(pVictim);
    }

    // out of reach and out of combat until released
    void seal(bool sealed)
    {
        setCanEnterCombat(!sealed);
        if (sealed)
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        else
            removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    void dropArcaneBomb()
    {
        if (Unit* pTarget = randomPlayer())
            summonCreature(NPC_AZSHARA_ARCANE_BOMB, pTarget->GetPosition(), TIMED_DESPAWN, ArcaneBombFuseMs + 3000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class EyeOfAzsharaBossAI : public EyeOfAzsharaCombatAI
{
public:
    EyeOfAzsharaBossAI(Creature* pCreature, uint32_t encounterId) : EyeOfAzsharaCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        EyeOfAzsharaCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        // a dying boss leaves combat as well
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mEncounterId, EncounterStates::Failed);
            }
        }

        EyeOfAzsharaCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }
    }

protected:
    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Warlord Parjesh

class EyeOfAzsharaParjeshAI : public EyeOfAzsharaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EyeOfAzsharaParjeshAI(c); }
    explicit EyeOfAzsharaParjeshAI(Creature* pCreature) : EyeOfAzsharaBossAI(pCreature, DATA_WARLORD_PARJESH) {}

protected:
    void onEngage() override
    {
        mEnraged = false;
        mShellbreakerNext = Util::getRandomUInt(1) == 0;

        scriptEvents.addEvent(EVENT_PARJESH_CRASHING_WAVE, 3000);
        scriptEvents.addEvent(EVENT_PARJESH_REINFORCEMENTS, Util::getRandomUInt(3500, 4500));
        scriptEvents.addEvent(EVENT_PARJESH_THROW_SPEAR, 11000);
        scriptEvents.addEvent(EVENT_PARJESH_IMPALING_SPEAR, ParjeshSpearEnergyMs);
    }

    void onCombatTick() override
    {
        if (!mEnraged && _getHealthPercent() <= 30)
        {
            mEnraged = true;
            castSpellOnSelf(SPELL_PARJESH_ENRAGE, true);
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Warlord Parjesh becomes enraged!");
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_PARJESH_CRASHING_WAVE:
                castSpellOnVictim(SPELL_PARJESH_CRASHING_WAVE);
                scriptEvents.addEvent(EVENT_PARJESH_CRASHING_WAVE, 3000);
                break;
            case EVENT_PARJESH_REINFORCEMENTS:
                callReinforcements();
                scriptEvents.addEvent(EVENT_PARJESH_REINFORCEMENTS, Util::getRandomUInt(28000, 35000));
                break;
            case EVENT_PARJESH_THROW_SPEAR:
                castOnRandomPlayer(SPELL_PARJESH_THROW_SPEAR);
                scriptEvents.addEvent(EVENT_PARJESH_THROW_SPEAR, 16000);
                break;
            case EVENT_PARJESH_IMPALING_SPEAR:
                if (Unit* pTarget = randomPlayer(true))
                {
                    sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Warlord Parjesh hurls an Impaling Spear!");
                    castSpell(pTarget, SPELL_PARJESH_IMPALING_SPEAR, true);
                    castSpell(pTarget, SPELL_PARJESH_IMPALING_KNOCKBACK, true);
                }
                scriptEvents.addEvent(EVENT_PARJESH_IMPALING_SPEAR, ParjeshSpearEnergyMs);
                break;
            default:
                break;
        }
    }

private:
    // one guard in turn on normal, both from heroic on
    void callReinforcements()
    {
        if (isHeroic())
        {
            callAdd(NPC_HATECOIL_SHELLBREAKER, eyeRingPoint(getCreature(), ParjeshPlatformCentre, ParjeshReinforcementRange));
            callAdd(NPC_HATECOIL_CRESTRIDER, eyeRingPoint(getCreature(), ParjeshPlatformCentre, ParjeshReinforcementRange));
            return;
        }

        callAdd(mShellbreakerNext ? NPC_HATECOIL_SHELLBREAKER : NPC_HATECOIL_CRESTRIDER, eyeRingPoint(getCreature(), ParjeshPlatformCentre, ParjeshReinforcementRange));
        mShellbreakerNext = !mShellbreakerNext;
    }

    bool mEnraged = false;
    bool mShellbreakerNext = true;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lady Hatecoil, shielded by her arcanists

class EyeOfAzsharaHatecoilAI : public EyeOfAzsharaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EyeOfAzsharaHatecoilAI(c); }
    explicit EyeOfAzsharaHatecoilAI(Creature* pCreature) : EyeOfAzsharaBossAI(pCreature, DATA_LADY_HATECOIL) {}

    void OnLoad() override
    {
        mShielded = true;
        mShieldCheckMs = 5000;
        seal(true);
        _applyAura(SPELL_HATECOIL_ARCANE_SHIELDING);
    }

protected:
    void onIdleTick(uint32_t diff) override
    {
        if (!mShielded)
            return;

        if (mShieldCheckMs > diff)
        {
            mShieldCheckMs -= diff;
            return;
        }

        mShieldCheckMs = 2000;

        EyeOfAzsharaInstanceScript* pEye = getEyeOfAzshara();
        if (pEye != nullptr && pEye->livingArcanists() > HatecoilShieldArcanists)
            return;

        mShielded = false;
        _removeAura(SPELL_HATECOIL_ARCANE_SHIELDING);
        seal(false);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Lady Hatecoil's arcane shield collapses!");
    }

    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_HATECOIL_STATIC_NOVA, 11000);
        scriptEvents.addEvent(EVENT_HATECOIL_CURSE_OF_THE_WITCH, 17000);
        scriptEvents.addEvent(EVENT_HATECOIL_BECKON_STORM, 19000);
        scriptEvents.addEvent(EVENT_HATECOIL_FOCUSED_LIGHTNING, 25000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HATECOIL_STATIC_NOVA:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Lady Hatecoil begins to cast Static Nova!");
                castSpellAOE(SPELL_HATECOIL_STATIC_NOVA);
                scriptEvents.addEvent(EVENT_HATECOIL_STATIC_NOVA, 35000);
                break;
            case EVENT_HATECOIL_CURSE_OF_THE_WITCH:
                curseWitches();
                scriptEvents.addEvent(EVENT_HATECOIL_CURSE_OF_THE_WITCH, 24000);
                break;
            case EVENT_HATECOIL_BECKON_STORM:
                castSpellAOE(SPELL_HATECOIL_BECKON_STORM);
                scriptEvents.addEvent(EVENT_HATECOIL_BECKON_STORM, 47000);
                break;
            case EVENT_HATECOIL_FOCUSED_LIGHTNING:
                castSpellAOE(SPELL_HATECOIL_FOCUSED_LIGHTNING);
                scriptEvents.addEvent(EVENT_HATECOIL_FOCUSED_LIGHTNING, 36000);
                break;
            default:
                break;
        }
    }

private:
    // one, three or five players are cursed
    void curseWitches()
    {
        uint32_t const wanted = HatecoilCurseTargets[Util::getRandomUInt(2)];
        uint32_t cursed = 0;
        for (Unit* pPlayer : playersAround(60.0f))
        {
            if (cursed >= wanted)
                break;

            castSpell(pPlayer, SPELL_HATECOIL_CURSE_OF_THE_WITCH, true);
            ++cursed;
        }
    }

    bool mShielded = true;
    uint32_t mShieldCheckMs = 5000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// King Deepbeard

class EyeOfAzsharaDeepbeardAI : public EyeOfAzsharaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EyeOfAzsharaDeepbeardAI(c); }
    explicit EyeOfAzsharaDeepbeardAI(Creature* pCreature) : EyeOfAzsharaBossAI(pCreature, DATA_KING_DEEPBEARD) {}

protected:
    void onEngage() override
    {
        mFrenzied = false;
        scriptEvents.addEvent(EVENT_DEEPBEARD_GROUND_SLAM, 7000);
        scriptEvents.addEvent(EVENT_DEEPBEARD_GASEOUS_BUBBLES, 11000);
        scriptEvents.addEvent(EVENT_DEEPBEARD_QUAKE, DeepbeardQuakeFirstMs);
        scriptEvents.addEvent(EVENT_DEEPBEARD_CALL_THE_SEAS, 20000);
    }

    void onCombatTick() override
    {
        if (!mFrenzied && _getHealthPercent() <= 30)
        {
            mFrenzied = true;
            castSpellOnSelf(SPELL_DEEPBEARD_FRENZY, true);
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "King Deepbeard goes into a frenzy!");
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DEEPBEARD_GROUND_SLAM:
                castSpellOnVictim(SPELL_DEEPBEARD_GROUND_SLAM);
                castSpellOnSelf(SPELL_DEEPBEARD_GROUND_SLAM_MISSILE, true);
                scriptEvents.addEvent(EVENT_DEEPBEARD_GROUND_SLAM, 18000);
                break;
            case EVENT_DEEPBEARD_GASEOUS_BUBBLES:
                castSpellAOE(SPELL_DEEPBEARD_GASEOUS_BUBBLES);
                scriptEvents.addEvent(EVENT_DEEPBEARD_GASEOUS_BUBBLES, 33000);
                break;
            case EVENT_DEEPBEARD_QUAKE:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "King Deepbeard begins to cast Quake!");
                castSpellAOE(SPELL_DEEPBEARD_QUAKE);
                scriptEvents.addEvent(EVENT_DEEPBEARD_QUAKE, DeepbeardQuakeCycleMs);
                break;
            case EVENT_DEEPBEARD_CALL_THE_SEAS:
                castSpellAOE(SPELL_DEEPBEARD_CALL_THE_SEAS);
                scriptEvents.addEvent(EVENT_DEEPBEARD_CALL_THE_SEAS, 30000);
                break;
            default:
                break;
        }
    }

private:
    bool mFrenzied = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Serpentrix, stationary, dives at two thirds and one third

class EyeOfAzsharaSerpentrixAI : public EyeOfAzsharaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EyeOfAzsharaSerpentrixAI(c); }
    explicit EyeOfAzsharaSerpentrixAI(Creature* pCreature) : EyeOfAzsharaBossAI(pCreature, DATA_SERPENTRIX) {}

    void OnLoad() override { setRooted(true); }

protected:
    void onEngage() override
    {
        mNextSubmergePct = 66;
        mSurfaceSpot = SerpentrixSurfaceSpotCount;

        scriptEvents.addEvent(EVENT_SERPENTRIX_TOXIC_WOUND, 6000);
        scriptEvents.addEvent(EVENT_SERPENTRIX_POISON_SPIT, 11000);
        scriptEvents.addEvent(EVENT_SERPENTRIX_REACH_CHECK, 2000);
    }

    // rooted, so the way home is a teleport
    void onDisengage() override
    {
        if (isAlive())
            MoveTeleport(getCreature()->GetSpawnPosition());
    }

    void onCombatTick() override
    {
        if (mNextSubmergePct == 0 || _getHealthPercent() > mNextSubmergePct)
            return;

        mNextSubmergePct = mNextSubmergePct > 33 ? 33 : 0;

        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Serpentrix submerges!");
        castSpellOnSelf(SPELL_SERPENTRIX_SUBMERGE, true);
        scriptEvents.delayAllEvents(2000);
        scriptEvents.addEvent(EVENT_SERPENTRIX_RESURFACE, 1500);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SERPENTRIX_TOXIC_WOUND:
                castSpellAOE(SPELL_SERPENTRIX_TOXIC_WOUND);
                scriptEvents.addEvent(EVENT_SERPENTRIX_TOXIC_WOUND, 25000);
                break;
            case EVENT_SERPENTRIX_POISON_SPIT:
                castOnRandomPlayer(SPELL_SERPENTRIX_POISON_SPIT);
                scriptEvents.addEvent(EVENT_SERPENTRIX_POISON_SPIT, 9000);
                break;
            case EVENT_SERPENTRIX_REACH_CHECK:
                // nobody to bite, the whole cove pays
                if (!anyPlayerInMelee() && !hasAura(SPELL_SERPENTRIX_RAMPAGE))
                    castSpellOnSelf(SPELL_SERPENTRIX_RAMPAGE);
                scriptEvents.addEvent(EVENT_SERPENTRIX_REACH_CHECK, 3000);
                break;
            case EVENT_SERPENTRIX_RESURFACE:
                resurface();
                break;
            default:
                break;
        }
    }

private:
    // a new spot each time, hydra spawns rise under players
    void resurface()
    {
        uint32_t spot = Util::getRandomUInt(SerpentrixSurfaceSpotCount - 1);
        if (spot == mSurfaceSpot)
            spot = (spot + 1) % SerpentrixSurfaceSpotCount;

        mSurfaceSpot = spot;
        MoveTeleport(SerpentrixSurfaceSpots[spot]);

        std::vector<Unit*> players = playersAround(100.0f);
        if (players.empty())
            return;

        callAdd(NPC_BLAZING_HYDRA_SPAWN, players[0]->GetPosition());
        if (isHeroic())
            callAdd(NPC_ARCANE_HYDRA_SPAWN, players[players.size() > 1 ? 1 : 0]->GetPosition());
    }

    int32_t mNextSubmergePct = 66;
    uint32_t mSurfaceSpot = SerpentrixSurfaceSpotCount;
};

class EyeOfAzsharaHydraSpawnAI : public EyeOfAzsharaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EyeOfAzsharaHydraSpawnAI(c); }
    explicit EyeOfAzsharaHydraSpawnAI(Creature* pCreature) : EyeOfAzsharaCombatAI(pCreature) {}

    void OnLoad() override { setRooted(true); }

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_HYDRA_SPAWN_ATTACK, 2000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_HYDRA_SPAWN_ATTACK)
            return;

        if (getCreature()->getEntry() == NPC_BLAZING_HYDRA_SPAWN)
            castSpellAOE(SPELL_HYDRA_BLAZING_NOVA);
        else
            castOnRandomPlayer(SPELL_HYDRA_ARCANE_BLAST);

        scriptEvents.addEvent(EVENT_HYDRA_SPAWN_ATTACK, 2500);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Wrath of Azshara, bound until her four ritualists fall

class EyeOfAzsharaWrathAI : public EyeOfAzsharaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EyeOfAzsharaWrathAI(c); }
    explicit EyeOfAzsharaWrathAI(Creature* pCreature) : EyeOfAzsharaBossAI(pCreature, DATA_WRATH_OF_AZSHARA) {}

    void OnLoad() override
    {
        setRooted(true);
        mBound = true;
        mBindCheckMs = 2000;
        seal(true);
    }

protected:
    void onIdleTick(uint32_t diff) override
    {
        if (!mBound)
            return;

        if (mBindCheckMs > diff)
        {
            mBindCheckMs -= diff;
            return;
        }

        mBindCheckMs = 2000;

        EyeOfAzsharaInstanceScript* pEye = getEyeOfAzshara();
        if (pEye != nullptr && (!pEye->isStormAwake() || pEye->livingRitualists() > 0))
            return;

        mBound = false;
        seal(false);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "The ritual is broken! Wrath of Azshara is vulnerable!");
    }

    void onEngage() override
    {
        mCryOfWrath = false;
        castSpellOnSelf(SPELL_WRATH_TIDAL_WAVE_SLOW, true);
        castSpellOnSelf(SPELL_WRATH_LIGHTNING_SLOW, true);

        scriptEvents.addEvent(EVENT_WRATH_MYSTIC_TORNADO, 9000);
        scriptEvents.addEvent(EVENT_WRATH_MASSIVE_DELUGE, 12000);
        scriptEvents.addEvent(EVENT_WRATH_ARCANE_BOMB, 23000);
        scriptEvents.addEvent(EVENT_WRATH_REACH_CHECK, 2000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_WRATH_CRUSHING_DEPTHS, 20000);
    }

    void onCombatTick() override
    {
        if (mCryOfWrath || _getHealthPercent() > 10)
            return;

        // the storm turns violent below a tenth
        mCryOfWrath = true;
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Wrath of Azshara lets out a Cry of Wrath!");
        _removeAura(SPELL_WRATH_TIDAL_WAVE_SLOW);
        _removeAura(SPELL_WRATH_LIGHTNING_SLOW);
        castSpellAOE(SPELL_WRATH_CRY_OF_WRATH, true);
        castSpellOnSelf(SPELL_WRATH_TIDAL_WAVE_FAST, true);
        castSpellOnSelf(SPELL_WRATH_LIGHTNING_FAST, true);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_WRATH_MYSTIC_TORNADO:
                castSpellOnSelf(SPELL_WRATH_MYSTIC_TORNADO, true);
                scriptEvents.addEvent(EVENT_WRATH_MYSTIC_TORNADO, 25000);
                break;
            case EVENT_WRATH_MASSIVE_DELUGE:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Wrath of Azshara begins to cast Massive Deluge!");
                castSpellAOE(SPELL_WRATH_MASSIVE_DELUGE);
                scriptEvents.addEvent(EVENT_WRATH_MASSIVE_DELUGE, 48000);
                break;
            case EVENT_WRATH_ARCANE_BOMB:
                castSpellAOE(SPELL_WRATH_ARCANE_BOMB);
                dropArcaneBomb();
                scriptEvents.addEvent(EVENT_WRATH_ARCANE_BOMB, 30000);
                break;
            case EVENT_WRATH_CRUSHING_DEPTHS:
                castOnRandomPlayer(SPELL_WRATH_CRUSHING_DEPTHS);
                scriptEvents.addEvent(EVENT_WRATH_CRUSHING_DEPTHS, 40000);
                break;
            case EVENT_WRATH_REACH_CHECK:
                if (!victimInMelee())
                    castSpellAOE(SPELL_WRATH_RAGING_STORMS);
                scriptEvents.addEvent(EVENT_WRATH_REACH_CHECK, 2000);
                break;
            default:
                break;
        }
    }

private:
    bool mBound = true;
    bool mCryOfWrath = false;
    uint32_t mBindCheckMs = 2000;
};

// Mystic Ssa'veh, Ritualist Lesha, Channeler Varisz and Binder Ashioi
class EyeOfAzsharaRitualistAI : public EyeOfAzsharaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EyeOfAzsharaRitualistAI(c); }
    explicit EyeOfAzsharaRitualistAI(Creature* pCreature) : EyeOfAzsharaCombatAI(pCreature) {}

    void OnLoad() override
    {
        mAttuned = true;
        mAttuneCheckMs = 2000;
        seal(true);
    }

protected:
    // the ritual opens to attack when the storm wakes
    void onIdleTick(uint32_t diff) override
    {
        if (!mAttuned)
            return;

        if (mAttuneCheckMs > diff)
        {
            mAttuneCheckMs -= diff;
            return;
        }

        mAttuneCheckMs = 2000;

        EyeOfAzsharaInstanceScript* pEye = getEyeOfAzshara();
        if (pEye != nullptr && !pEye->isStormAwake())
            return;

        mAttuned = false;
        seal(false);
        castSpellOnSelf(SPELL_RITUALIST_STORM_CONDUIT, true);
    }

    // auras are stripped on reset
    void onDisengage() override
    {
        if (!mAttuned && isAlive())
            castSpellOnSelf(SPELL_RITUALIST_STORM_CONDUIT, true);
    }

    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_RITUALIST_LIGHTNING_BLAST, 6000);
        scriptEvents.addEvent(EVENT_RITUALIST_SIGNATURE, 9000);
        if (getCreature()->getEntry() != NPC_MYSTIC_SSAVEH)
            scriptEvents.addEvent(EVENT_RITUALIST_ARCANE_BOMB, 18000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RITUALIST_LIGHTNING_BLAST:
                castSpellOnVictim(SPELL_RITUALIST_LIGHTNING_BLAST);
                scriptEvents.addEvent(EVENT_RITUALIST_LIGHTNING_BLAST, 6000);
                break;
            case EVENT_RITUALIST_SIGNATURE:
                castSignature();
                scriptEvents.addEvent(EVENT_RITUALIST_SIGNATURE, 18000);
                break;
            case EVENT_RITUALIST_ARCANE_BOMB:
                castSpellAOE(SPELL_WRATH_ARCANE_BOMB);
                dropArcaneBomb();
                scriptEvents.addEvent(EVENT_RITUALIST_ARCANE_BOMB, 18000);
                break;
            default:
                break;
        }
    }

private:
    void castSignature()
    {
        switch (getCreature()->getEntry())
        {
            case NPC_BINDER_ASHIOI:
                castSpellAOE(SPELL_ASHIOI_MAGIC_BINDING);
                break;
            case NPC_MYSTIC_SSAVEH:
                castSpellOnSelf(SPELL_SSAVEH_STORM);
                break;
            case NPC_CHANNELER_VARISZ:
                castOnRandomPlayer(SPELL_VARISZ_POLYMORPH_FISH);
                break;
            case NPC_RITUALIST_LESHA:
                castSpellAOE(SPELL_LESHA_AQUA_SPOUT);
                break;
            default:
                break;
        }
    }

    bool mAttuned = true;
    uint32_t mAttuneCheckMs = 2000;
};

// detonates where it was dropped
class EyeOfAzsharaArcaneBombAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EyeOfAzsharaArcaneBombAI(c); }
    explicit EyeOfAzsharaArcaneBombAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        mFuseMs = ArcaneBombFuseMs;
        mDetonated = false;
        setRooted(true);
        setCanEnterCombat(false);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_ARCANE_BOMB_VISUAL, true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mDetonated)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mFuseMs > diff)
        {
            mFuseMs -= diff;
            return;
        }

        mDetonated = true;
        castSpellAOE(SPELL_ARCANE_BOMB_DETONATION, true);
        despawn(2000, 0);
    }

private:
    uint32_t mFuseMs = ArcaneBombFuseMs;
    bool mDetonated = false;
};

void SetupEyeOfAzshara(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_EYE_OF_AZSHARA, &EyeOfAzsharaInstanceScript::Create);

    mgr->register_creature_script(BOSS_WARLORD_PARJESH, &EyeOfAzsharaParjeshAI::Create);
    mgr->register_creature_script(BOSS_LADY_HATECOIL, &EyeOfAzsharaHatecoilAI::Create);
    mgr->register_creature_script(BOSS_KING_DEEPBEARD, &EyeOfAzsharaDeepbeardAI::Create);

    mgr->register_creature_script(BOSS_SERPENTRIX, &EyeOfAzsharaSerpentrixAI::Create);
    mgr->register_creature_script(NPC_BLAZING_HYDRA_SPAWN, &EyeOfAzsharaHydraSpawnAI::Create);
    mgr->register_creature_script(NPC_ARCANE_HYDRA_SPAWN, &EyeOfAzsharaHydraSpawnAI::Create);

    mgr->register_creature_script(BOSS_WRATH_OF_AZSHARA, &EyeOfAzsharaWrathAI::Create);
    mgr->register_creature_script(NPC_MYSTIC_SSAVEH, &EyeOfAzsharaRitualistAI::Create);
    mgr->register_creature_script(NPC_RITUALIST_LESHA, &EyeOfAzsharaRitualistAI::Create);
    mgr->register_creature_script(NPC_CHANNELER_VARISZ, &EyeOfAzsharaRitualistAI::Create);
    mgr->register_creature_script(NPC_BINDER_ASHIOI, &EyeOfAzsharaRitualistAI::Create);
    mgr->register_creature_script(NPC_AZSHARA_ARCANE_BOMB, &EyeOfAzsharaArcaneBombAI::Create);
}
