/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Court_Of_Stars.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

#include <list>
#include <map>
#include <vector>

enum CourtOfStarsEvents
{
    EVENT_COS_GERDO_RESONANT_SLASH = 1,
    EVENT_COS_GERDO_ARCANE_LOCKDOWN,
    EVENT_COS_GERDO_SIGNAL_BEACON,
    EVENT_COS_GERDO_DRINK_FLASK,

    EVENT_COS_DUSKWATCH_HINDER,

    EVENT_COS_TALIXAE_BURNING_INTENSITY,
    EVENT_COS_TALIXAE_WITHERING_SOUL,
    EVENT_COS_TALIXAE_INFERNAL_ERUPTION,

    EVENT_COS_COVENANT_SIGNATURE,

    EVENT_COS_MELANDRUS_PIERCING_GALE,
    EVENT_COS_MELANDRUS_SLICING_MAELSTROM,
    EVENT_COS_MELANDRUS_BLADE_SURGE
};

namespace
{
    // spot of the flask when no flask creature is in reach
    LocationVector const GerdoFlaskSpot = { 1308.29f, 3537.42f, 20.76f, 0.0f };

    struct FelCovenantGuard
    {
        uint32_t entry;
        uint32_t bondSpell;
        uint32_t signatureSpell;
        uint32_t firstCast;
        uint32_t recast;
        bool onRandomPlayer;
    };

    FelCovenantGuard const FelCovenantGuards[] =
    {
        { NPC_JAZSHARIU,            SPELL_JAZSHARIU_BOND_OF_STRENGTH, SPELL_JAZSHARIU_SHOCKWAVE,         7000, 11000, false },
        { NPC_BAALGAR_THE_WATCHFUL, SPELL_BAALGAR_BOND_OF_CRUELTY,    SPELL_BAALGAR_DISINTEGRATION_BEAM, 3000, 13000, true },
        { NPC_IMACUTYA,             SPELL_IMACUTYA_BOND_OF_CUNNING,   SPELL_IMACUTYA_WHIRLING_BLADES,    3000, 17000, false }
    };

    FelCovenantGuard const* findCovenantGuard(uint32_t entry)
    {
        for (FelCovenantGuard const& guard : FelCovenantGuards)
        {
            if (guard.entry == entry)
                return &guard;
        }

        return nullptr;
    }
}

class CourtOfStarsInstanceScript : public InstanceScript
{
public:
    explicit CourtOfStarsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(CourtOfStarsEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new CourtOfStarsInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_PATROL_CAPTAIN_GERDO:
            case BOSS_TALIXAE_FLAMEWREATH:
            case BOSS_ADVISOR_MELANDRUS:
            case NPC_JAZSHARIU:
            case NPC_BAALGAR_THE_WATCHFUL:
            case NPC_IMACUTYA:
                mKnownCreatures[pCreature->getEntry()] = pCreature->GetNewGUID();
                break;
            default:
                break;
        }
    }

    void OnCreatureDeath(Creature* pVictim, Unit* /*pKiller*/) override
    {
        if (findCovenantGuard(pVictim->getEntry()) == nullptr || isCovenantStanding(pVictim))
            return;

        Creature* pTalixae = getKnown(BOSS_TALIXAE_FLAMEWREATH);
        if (pTalixae != nullptr && pTalixae->isAlive() && pTalixae->GetScript() != nullptr)
            pTalixae->GetScript()->DoAction(ACTION_COS_BONDS_BROKEN);
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_COS_WAYPOINT_DOOR:
            case GO_COS_MANOR_DOOR:
            case GO_COS_SKYWARD_TERRACE:
                mDoorGuids.push_back(pGameObject->GetNewGUID());
                alignDoor(pGameObject);
                break;
            default:
                break;
        }
    }

    void OnEncounterStateChange(uint32_t /*id*/, uint32_t /*state*/) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                alignDoor(pDoor);
        }
    }

    Creature* getKnown(uint32_t entry)
    {
        auto const itr = mKnownCreatures.find(entry);
        return itr != mKnownCreatures.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    // true while any Fel Covenant guard other than pExcept lives
    bool isCovenantStanding(Creature* pExcept)
    {
        for (FelCovenantGuard const& guard : FelCovenantGuards)
        {
            Creature* pGuard = getKnown(guard.entry);
            if (pGuard != nullptr && pGuard != pExcept && pGuard->isAlive())
                return true;
        }

        return false;
    }

    std::vector<Creature*> getStandingCovenant()
    {
        std::vector<Creature*> standing;
        for (FelCovenantGuard const& guard : FelCovenantGuards)
        {
            Creature* pGuard = getKnown(guard.entry);
            if (pGuard != nullptr && pGuard->isAlive())
                standing.push_back(pGuard);
        }

        return standing;
    }

private:
    bool shouldBeOpen(uint32_t entry) const
    {
        switch (entry)
        {
            case GO_COS_WAYPOINT_DOOR:
                return getBossState(DATA_PATROL_CAPTAIN_GERDO) == EncounterStates::Performed;
            case GO_COS_MANOR_DOOR:
                return getBossState(DATA_TALIXAE_FLAMEWREATH) == EncounterStates::Performed;
            case GO_COS_SKYWARD_TERRACE:
                // the spy hunt is not scripted, the terrace follows Talixae and seals Melandrus in
                return getBossState(DATA_TALIXAE_FLAMEWREATH) == EncounterStates::Performed
                    && getBossState(DATA_ADVISOR_MELANDRUS) != EncounterStates::InProgress;
            default:
                return true;
        }
    }

    void alignDoor(GameObject* pDoor)
    {
        pDoor->setState(shouldBeOpen(pDoor->getEntry()) ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    std::map<uint32_t, WoWGuid> mKnownCreatures;
    std::vector<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared fighter base: event clock while engaged

class CourtOfStarsFighterAI : public CreatureAIScript
{
public:
    explicit CourtOfStarsFighterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        onPull();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        onFightTick(diff);

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onPull() {}
    virtual void onFightTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    CourtOfStarsInstanceScript* getCourt()
    {
        if (getCreature()->GetMapId() != MAP_COURT_OF_STARS)
            return nullptr;

        return static_cast<CourtOfStarsInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    void castAtAnyPlayer(uint32_t spellId)
    {
        if (Unit* pTarget = anyPlayer())
            castSpell(pTarget, spellId);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class CourtOfStarsBossAI : public CourtOfStarsFighterAI
{
public:
    CourtOfStarsBossAI(Creature* pCreature, uint32_t encounterId) : CourtOfStarsFighterAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        CourtOfStarsFighterAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (!isAlive())
            return;

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Failed);
        }

        onReset();
        CourtOfStarsFighterAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }

        onDefeated();
    }

protected:
    virtual void onReset() {}
    virtual void onDefeated() {}

    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Patrol Captain Gerdo

class CourtOfStarsGerdoAI : public CourtOfStarsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CourtOfStarsGerdoAI(c); }
    explicit CourtOfStarsGerdoAI(Creature* pCreature) : CourtOfStarsBossAI(pCreature, DATA_PATROL_CAPTAIN_GERDO) {}

protected:
    void onPull() override
    {
        mFlaskReached = false;
        scriptEvents.addEvent(EVENT_COS_GERDO_RESONANT_SLASH, 7000);
        scriptEvents.addEvent(EVENT_COS_GERDO_ARCANE_LOCKDOWN, 15000);
        scriptEvents.addEvent(EVENT_COS_GERDO_SIGNAL_BEACON, 23000);
    }

    void onReset() override { setReactState(REACT_AGGRESSIVE); }

    void onFightTick(uint32_t /*diff*/) override
    {
        // a quarter left, he runs for the flask
        if (mFlaskReached || _getHealthPercent() > 25)
            return;

        mFlaskReached = true;
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Patrol Captain Gerdo runs for the Flask of the Solemn Night!");
        setReactState(REACT_PASSIVE);

        LocationVector spot = GerdoFlaskSpot;
        if (Creature* pFlask = findNearestCreature(NPC_FLASK_OF_THE_SOLEMN_NIGHT, 80.0f))
            spot = pFlask->GetPosition();

        moveTo(spot.x, spot.y, spot.z);
        scriptEvents.addEvent(EVENT_COS_GERDO_DRINK_FLASK, 4000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_COS_GERDO_RESONANT_SLASH:
                castSpellAOE(SPELL_GERDO_RESONANT_SLASH);
                scriptEvents.addEvent(EVENT_COS_GERDO_RESONANT_SLASH, 12000);
                break;
            case EVENT_COS_GERDO_ARCANE_LOCKDOWN:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Patrol Captain Gerdo begins to cast Arcane Lockdown!");
                castSpellAOE(SPELL_GERDO_ARCANE_LOCKDOWN);
                scriptEvents.addEvent(EVENT_COS_GERDO_ARCANE_LOCKDOWN, 28000);
                break;
            case EVENT_COS_GERDO_SIGNAL_BEACON:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Patrol Captain Gerdo signals the Arcane Beacons!");
                castSpellAOE(SPELL_GERDO_SIGNAL_BEACON);
                callNightwatch();
                break;
            case EVENT_COS_GERDO_DRINK_FLASK:
                castSpellOnSelf(SPELL_GERDO_FLASK_OF_SOLEMN_NIGHT, true);
                setReactState(REACT_AGGRESSIVE);
                if (Unit* pTarget = anyPlayer())
                    attackStart(pTarget);
                break;
            default:
                break;
        }
    }

private:
    // every beacon the players left active sends one Vigilant Duskwatch
    void callNightwatch()
    {
        std::list<Creature*> beacons;
        GetCreatureListWithEntryInGrid(beacons, NPC_ARCANE_BEACON, 250.0f);

        for (Creature* pBeacon : beacons)
        {
            if (!pBeacon->isAlive() || pBeacon->hasAurasWithId(SPELL_ARCANE_BEACON_DISABLED))
                continue;

            LocationVector spot = pBeacon->GetPosition();
            spot.x += 2.0f;
            if (Creature* pGuard = summonCreature(NPC_VIGILANT_DUSKWATCH, spot, CORPSE_DESPAWN))
                setZoneWideCombat(pGuard);
        }
    }

    bool mFlaskReached = false;
};

class CourtOfStarsVigilantDuskwatchAI : public CourtOfStarsFighterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CourtOfStarsVigilantDuskwatchAI(c); }
    explicit CourtOfStarsVigilantDuskwatchAI(Creature* pCreature) : CourtOfStarsFighterAI(pCreature) {}

protected:
    void onPull() override { scriptEvents.addEvent(EVENT_COS_DUSKWATCH_HINDER, 5000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_COS_DUSKWATCH_HINDER)
            return;

        castSpellOnVictim(SPELL_VIGILANT_DUSKWATCH_HINDER);
        scriptEvents.addEvent(EVENT_COS_DUSKWATCH_HINDER, 10000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Talixae Flamewreath and the Fel Covenant

class CourtOfStarsTalixaeAI : public CourtOfStarsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CourtOfStarsTalixaeAI(c); }
    explicit CourtOfStarsTalixaeAI(Creature* pCreature) : CourtOfStarsBossAI(pCreature, DATA_TALIXAE_FLAMEWREATH) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_COS_BONDS_BROKEN)
            return;

        _removeAura(SPELL_TALIXAE_BOND_OF_FLAME);
        if (_isInCombat())
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "The Bond of Flame around Talixae Flamewreath fades!");
    }

protected:
    void onPull() override
    {
        // the surviving guards join and keep her bond burning
        if (CourtOfStarsInstanceScript* pCourt = getCourt())
        {
            std::vector<Creature*> const standing = pCourt->getStandingCovenant();
            for (Creature* pGuard : standing)
            {
                if (!pGuard->isInCombat())
                    setZoneWideCombat(pGuard);
            }

            if (!standing.empty())
                castSpellOnSelf(SPELL_TALIXAE_BOND_OF_FLAME, true);
        }

        scriptEvents.addEvent(EVENT_COS_TALIXAE_BURNING_INTENSITY, 6000);
        scriptEvents.addEvent(EVENT_COS_TALIXAE_WITHERING_SOUL, 12000);
        scriptEvents.addEvent(EVENT_COS_TALIXAE_INFERNAL_ERUPTION, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_COS_TALIXAE_BURNING_INTENSITY:
                castSpellOnSelf(SPELL_TALIXAE_BURNING_INTENSITY);
                break;
            case EVENT_COS_TALIXAE_WITHERING_SOUL:
                castSpellAOE(SPELL_TALIXAE_WITHERING_SOUL);
                scriptEvents.addEvent(EVENT_COS_TALIXAE_WITHERING_SOUL, 15000);
                break;
            case EVENT_COS_TALIXAE_INFERNAL_ERUPTION:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Talixae Flamewreath begins to cast Infernal Eruption!");
                castSpellAOE(SPELL_TALIXAE_INFERNAL_ERUPTION);
                scriptEvents.addEvent(EVENT_COS_TALIXAE_INFERNAL_ERUPTION, 32000);
                break;
            default:
                break;
        }
    }
};

class CourtOfStarsFelCovenantAI : public CourtOfStarsFighterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CourtOfStarsFelCovenantAI(c); }
    explicit CourtOfStarsFelCovenantAI(Creature* pCreature) : CourtOfStarsFighterAI(pCreature), mGuard(findCovenantGuard(pCreature->getEntry())) {}

    void OnLoad() override { applyBond(); }

    void OnCombatStop(Unit* pTarget) override
    {
        CourtOfStarsFighterAI::OnCombatStop(pTarget);
        if (isAlive())
            applyBond();
    }

protected:
    void onPull() override
    {
        applyBond();
        if (mGuard != nullptr)
            scriptEvents.addEvent(EVENT_COS_COVENANT_SIGNATURE, mGuard->firstCast);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_COS_COVENANT_SIGNATURE || mGuard == nullptr)
            return;

        if (mGuard->onRandomPlayer)
            castAtAnyPlayer(mGuard->signatureSpell);
        else
            castSpellAOE(mGuard->signatureSpell);

        scriptEvents.addEvent(EVENT_COS_COVENANT_SIGNATURE, mGuard->recast);
    }

private:
    void applyBond()
    {
        if (mGuard != nullptr && !hasAura(mGuard->bondSpell))
            castSpellOnSelf(mGuard->bondSpell, true);
    }

    FelCovenantGuard const* mGuard;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Advisor Melandrus

class CourtOfStarsMelandrusAI : public CourtOfStarsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CourtOfStarsMelandrusAI(c); }
    explicit CourtOfStarsMelandrusAI(Creature* pCreature) : CourtOfStarsBossAI(pCreature, DATA_ADVISOR_MELANDRUS) {}

protected:
    void onPull() override
    {
        scriptEvents.addEvent(EVENT_COS_MELANDRUS_PIERCING_GALE, 6000);
        scriptEvents.addEvent(EVENT_COS_MELANDRUS_SLICING_MAELSTROM, 11000);
        scriptEvents.addEvent(EVENT_COS_MELANDRUS_BLADE_SURGE, 19000);
    }

    void onReset() override { dismissImages(); }
    void onDefeated() override { dismissImages(); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_COS_MELANDRUS_PIERCING_GALE:
                castSpellOnVictim(SPELL_MELANDRUS_PIERCING_GALE);
                commandImages(ACTION_COS_IMAGE_GALE);
                scriptEvents.addEvent(EVENT_COS_MELANDRUS_PIERCING_GALE, 20000);
                break;
            case EVENT_COS_MELANDRUS_SLICING_MAELSTROM:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Advisor Melandrus begins to cast Slicing Maelstrom!");
                castSpellAOE(SPELL_MELANDRUS_SLICING_MAELSTROM);
                commandImages(ACTION_COS_IMAGE_MAELSTROM);
                scriptEvents.addEvent(EVENT_COS_MELANDRUS_SLICING_MAELSTROM, 18000);
                break;
            case EVENT_COS_MELANDRUS_BLADE_SURGE:
                castAtAnyPlayer(SPELL_MELANDRUS_BLADE_SURGE);
                scriptEvents.addEvent(EVENT_COS_MELANDRUS_BLADE_SURGE, 19000);
                break;
            default:
                break;
        }
    }

private:
    // images left behind by Blade Surge repeat his attacks
    void commandImages(int32_t action)
    {
        std::list<Creature*> images;
        GetCreatureListWithEntryInGrid(images, NPC_IMAGE_OF_ADVISOR_MELANDRUS, 100.0f);

        for (Creature* pImage : images)
        {
            if (pImage->isAlive() && pImage->GetScript() != nullptr)
                pImage->GetScript()->DoAction(action);
        }
    }

    void dismissImages()
    {
        std::list<Creature*> images;
        GetCreatureListWithEntryInGrid(images, NPC_IMAGE_OF_ADVISOR_MELANDRUS, 100.0f);

        for (Creature* pImage : images)
            pImage->despawn(500, 0);
    }
};

class CourtOfStarsMelandrusImageAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CourtOfStarsMelandrusImageAI(c); }
    explicit CourtOfStarsMelandrusImageAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_MELANDRUS_IMAGE_WIND_IMAGE, true);
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_COS_IMAGE_GALE)
            castSpellAOE(SPELL_MELANDRUS_PIERCING_GALE, true);
        else if (action == ACTION_COS_IMAGE_MAELSTROM)
            castSpellAOE(SPELL_MELANDRUS_IMAGE_MAELSTROM, true);
    }
};

void SetupCourtOfStars(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_COURT_OF_STARS, &CourtOfStarsInstanceScript::Create);

    mgr->register_creature_script(BOSS_PATROL_CAPTAIN_GERDO, &CourtOfStarsGerdoAI::Create);
    mgr->register_creature_script(NPC_VIGILANT_DUSKWATCH, &CourtOfStarsVigilantDuskwatchAI::Create);

    mgr->register_creature_script(BOSS_TALIXAE_FLAMEWREATH, &CourtOfStarsTalixaeAI::Create);
    mgr->register_creature_script(NPC_JAZSHARIU, &CourtOfStarsFelCovenantAI::Create);
    mgr->register_creature_script(NPC_BAALGAR_THE_WATCHFUL, &CourtOfStarsFelCovenantAI::Create);
    mgr->register_creature_script(NPC_IMACUTYA, &CourtOfStarsFelCovenantAI::Create);

    mgr->register_creature_script(BOSS_ADVISOR_MELANDRUS, &CourtOfStarsMelandrusAI::Create);
    mgr->register_creature_script(NPC_IMAGE_OF_ADVISOR_MELANDRUS, &CourtOfStarsMelandrusImageAI::Create);
}
