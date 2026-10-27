/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Freehold.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>

enum FreeholdEvents
{
    EVENT_KRAGG_PISTOL_SHOT = 1,
    EVENT_KRAGG_CHARRRRRGE,
    EVENT_KRAGG_AZERITE_POWDER_SHOT,
    EVENT_KRAGG_REVITALIZING_BREW,

    EVENT_RAOUL_BLACKOUT_BARREL,
    EVENT_RAOUL_BARREL_SMASH,
    EVENT_EUDORA_POWDER_SHOT,
    EVENT_EUDORA_GRAPESHOT_LEAP,
    EVENT_EUDORA_GRAPESHOT_VOLLEY,
    EVENT_JOLLY_CUTTING_SURGE,
    EVENT_JOLLY_WHIRLPOOL_OF_BLADES,

    EVENT_LUDWIG_SHELL_BOUNCE,
    EVENT_TROTHAK_RIPPER_PUNCH,
    EVENT_TROTHAK_SHARK_TORNADO,
    EVENT_TROTHAK_SHARK_TOSS,
    EVENT_TROTHAK_REARM,
    EVENT_TROTHAK_THROW_CHUM,

    EVENT_HARLAN_CANNON_BARRAGE,
    EVENT_HARLAN_SWIFTWIND_SABER,
    EVENT_HARLAN_GRENADIER
};

namespace
{
    float const FreeholdPi = 3.14159265f;

    // Sharkbait circles above the arena once Kragg is on foot
    LocationVector const SharkbaitPerch(-1768.29f, -1009.25f, 110.0f, 0.418879f);

    // ring floor Trothak strides onto
    LocationVector const TrothakRingSpot(-1812.53f, -473.63f, 35.35f, 4.72f);

    // deck the grenadiers jump down from
    LocationVector const GrenadierDeck(-1583.55f, -558.43f, 74.0f, 5.37f);

    uint32_t const SharkbaitBombardMs = 5000;
    uint32_t const EudoraGrapeshotVolleys = 5;
    uint32_t const SaberBaseCount = 3;
    float const SaberTravel = 100.0f;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class FreeholdInstanceScript : public InstanceScript
{
public:
    explicit FreeholdInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(FreeholdEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new FreeholdInstanceScript(pMapMgr); }

    // the captains fall together, the ring is won by Trothak alone
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress)
        {
            if (id == DATA_COUNCIL_O_CAPTAINS && !isCrewDown(nullptr))
                return false;

            if (id == DATA_RING_OF_BOOTY && !mTrothakBeaten)
                return false;
        }

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_SKYCAPN_KRAGG:
            case BOSS_CAPTAIN_RAOUL:
            case BOSS_CAPTAIN_EUDORA:
            case BOSS_CAPTAIN_JOLLY:
            case BOSS_LUDWIG_VON_TORTOLLAN:
            case BOSS_TROTHAK:
                mTrackedGuids[pCreature->getEntry()] = pCreature->GetNewGUID();
                break;
            default:
                break;
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (state != EncounterStates::Failed)
            return;

        // a wiped crew stands up again in full
        if (id == DATA_COUNCIL_O_CAPTAINS)
        {
            for (Creature* pCaptain : getCrew())
            {
                if (!pCaptain->isAlive())
                    pCaptain->despawn(1000, 5000);
            }
            return;
        }

        // the ring starts over with Ludwig
        if (id == DATA_RING_OF_BOOTY)
        {
            mTrothakBeaten = false;

            Creature* pLudwig = getTracked(BOSS_LUDWIG_VON_TORTOLLAN);
            if (pLudwig != nullptr && !pLudwig->isAlive())
                pLudwig->despawn(1000, 5000);

            Creature* pTrothak = getTracked(BOSS_TROTHAK);
            if (mTrothakCalledIn && pTrothak != nullptr && pTrothak->isAlive())
                pTrothak->despawn(1000, 0);

            mTrothakCalledIn = false;
        }
    }

    Creature* getTracked(uint32_t entry)
    {
        auto const itr = mTrackedGuids.find(entry);
        return itr != mTrackedGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    std::vector<Creature*> getCrew()
    {
        std::vector<Creature*> crew;
        for (uint32_t entry : { BOSS_CAPTAIN_RAOUL, BOSS_CAPTAIN_EUDORA, BOSS_CAPTAIN_JOLLY })
        {
            if (Creature* pCaptain = getTracked(entry))
                crew.push_back(pCaptain);
        }

        return crew;
    }

    bool isCrewDown(Creature* pExcept)
    {
        for (Creature* pCaptain : getCrew())
        {
            if (pCaptain != pExcept && pCaptain->isAlive())
                return false;
        }

        return true;
    }

    void markTrothakBeaten() { mTrothakBeaten = true; }

    // Trothak joins once Ludwig is down, spawned when the map has none
    void callTrothak()
    {
        Creature* pTrothak = getTracked(BOSS_TROTHAK);
        if (pTrothak == nullptr || !pTrothak->isAlive())
        {
            pTrothak = spawnCreature(BOSS_TROTHAK, TrothakRingSpot.x, TrothakRingSpot.y, TrothakRingSpot.z, TrothakRingSpot.o);
            mTrothakCalledIn = pTrothak != nullptr;
        }

        if (pTrothak != nullptr && !pTrothak->isInCombat() && pTrothak->GetScript() != nullptr)
            pTrothak->GetScript()->setZoneWideCombat();
    }

private:
    std::map<uint32_t, WoWGuid> mTrackedGuids;
    bool mTrothakBeaten = false;
    bool mTrothakCalledIn = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class FreeholdCombatAI : public CreatureAIScript
{
public:
    explicit FreeholdCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    FreeholdInstanceScript* getFreehold()
    {
        if (getCreature()->GetMapId() != MAP_FREEHOLD)
            return nullptr;

        return static_cast<FreeholdInstanceScript*>(getInstanceScript());
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

    Unit* castAtPlayer(uint32_t spellId, bool skipTank = false, bool triggered = false)
    {
        Unit* pTarget = pickPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    LocationVector aroundSelf(float minRange, float maxRange)
    {
        Creature* pSelf = getCreature();
        float const angle = Util::getRandomFloat(0.0f, 2.0f * FreeholdPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(pSelf->GetPositionX() + range * std::cos(angle), pSelf->GetPositionY() + range * std::sin(angle), pSelf->GetPositionZ(), angle);
    }

    void warn(std::string const& text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and crew pull

class FreeholdBossAI : public FreeholdCombatAI
{
public:
    FreeholdBossAI(Creature* pCreature, uint32_t encounterId, bool endsEncounter = true) : FreeholdCombatAI(pCreature), mEncounterId(encounterId), mEndsEncounter(endsEncounter) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        // one captain drawn means all three join
        if (mEncounterId == DATA_COUNCIL_O_CAPTAINS)
        {
            if (FreeholdInstanceScript* pFreehold = getFreehold())
            {
                for (Creature* pCaptain : pFreehold->getCrew())
                {
                    if (pCaptain != getCreature() && pCaptain->isAlive() && !pCaptain->isInCombat())
                        setZoneWideCombat(pCaptain);
                }
            }
        }

        FreeholdCombatAI::OnCombatStart(pTarget);
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

        FreeholdCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr)
            return;

        pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        if (!mEndsEncounter)
            return;

        FreeholdInstanceScript* pFreehold = getFreehold();
        if (mEncounterId == DATA_COUNCIL_O_CAPTAINS && pFreehold != nullptr && !pFreehold->isCrewDown(getCreature()))
            return;

        pInstance->setBossState(mEncounterId, EncounterStates::Performed);
    }

protected:
    virtual void onDefeat() {}

    uint32_t mEncounterId;
    bool mEndsEncounter;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Skycap'n Kragg, takes to his feet when Sharkbait flies off

class FreeholdKraggAI : public FreeholdBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FreeholdKraggAI(c); }
    explicit FreeholdKraggAI(Creature* pCreature) : FreeholdBossAI(pCreature, DATA_SKYCAPN_KRAGG) {}

protected:
    void onEngage() override
    {
        mOnFoot = false;
        scriptEvents.addEvent(EVENT_KRAGG_PISTOL_SHOT, Util::getRandomUInt(3000, 5000));
        scriptEvents.addEvent(EVENT_KRAGG_CHARRRRRGE, 8000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mOnFoot && _getHealthPercent() <= 75)
            dismount();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KRAGG_PISTOL_SHOT:
                castAtPlayer(SPELL_KRAGG_PISTOL_SHOT);
                scriptEvents.addEvent(EVENT_KRAGG_PISTOL_SHOT, 8000);
                break;
            case EVENT_KRAGG_CHARRRRRGE:
                castAtPlayer(SPELL_KRAGG_CHARRRRRGE);
                scriptEvents.addEvent(EVENT_KRAGG_CHARRRRRGE, 8000);
                break;
            case EVENT_KRAGG_AZERITE_POWDER_SHOT:
                castAtPlayer(SPELL_KRAGG_AZERITE_POWDER_SHOT);
                scriptEvents.addEvent(EVENT_KRAGG_AZERITE_POWDER_SHOT, 8000);
                break;
            case EVENT_KRAGG_REVITALIZING_BREW:
                castSpellOnSelf(SPELL_KRAGG_REVITALIZING_BREW);
                scriptEvents.addEvent(EVENT_KRAGG_REVITALIZING_BREW, 15000);
                break;
            default:
                break;
        }
    }

private:
    // the mount keeps bombing from above while the captain fights on foot
    void dismount()
    {
        mOnFoot = true;
        scriptEvents.resetEvents();
        warn("Skycap'n Kragg leaps off Sharkbait!");

        summonCreature(NPC_SHARKBAIT, SharkbaitPerch, MANUAL_DESPAWN);

        scriptEvents.addEvent(EVENT_KRAGG_AZERITE_POWDER_SHOT, 2000);
        scriptEvents.addEvent(EVENT_KRAGG_REVITALIZING_BREW, 15000);
    }

    bool mOnFoot = false;
};

// out of reach in the air, rains bile on the party while Kragg fights
class FreeholdSharkbaitAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FreeholdSharkbaitAI(c); }
    explicit FreeholdSharkbaitAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setDisableGravity(true);
        setRooted(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mBombardClock > diff)
        {
            mBombardClock -= diff;
            return;
        }

        mBombardClock = SharkbaitBombardMs;
        if (!isCaptainFighting())
            return;

        std::vector<Unit*> players;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive())
                players.push_back(pPlayer);
        }

        if (!players.empty())
            castSpell(players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))], SPELL_SHARKBAIT_VILE_BOMBARDMENT);
    }

private:
    bool isCaptainFighting()
    {
        if (getCreature()->GetMapId() != MAP_FREEHOLD)
            return false;

        FreeholdInstanceScript* pFreehold = static_cast<FreeholdInstanceScript*>(getInstanceScript());
        Creature* pKragg = pFreehold != nullptr ? pFreehold->getTracked(BOSS_SKYCAPN_KRAGG) : nullptr;
        return pKragg != nullptr && pKragg->isAlive() && pKragg->isInCombat();
    }

    uint32_t mBombardClock = SharkbaitBombardMs;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Council o' Captains

class FreeholdCaptainAI : public FreeholdBossAI
{
public:
    explicit FreeholdCaptainAI(Creature* pCreature) : FreeholdBossAI(pCreature, DATA_COUNCIL_O_CAPTAINS) {}

protected:
    void onEngage() override
    {
        // three hostile crews fight under one flag
        castSpellOnSelf(SPELL_CAPTAINS_UNDER_ONE_BANNER, true);
        onCaptainEngage();
    }

    virtual void onCaptainEngage() {}
};

class FreeholdRaoulAI : public FreeholdCaptainAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FreeholdRaoulAI(c); }
    explicit FreeholdRaoulAI(Creature* pCreature) : FreeholdCaptainAI(pCreature) {}

protected:
    void onCaptainEngage() override
    {
        scriptEvents.addEvent(EVENT_RAOUL_BLACKOUT_BARREL, Util::getRandomUInt(3000, 5000));
        scriptEvents.addEvent(EVENT_RAOUL_BARREL_SMASH, 8000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RAOUL_BLACKOUT_BARREL:
                castAtPlayer(SPELL_RAOUL_BLACKOUT_BARREL, true);
                scriptEvents.addEvent(EVENT_RAOUL_BLACKOUT_BARREL, 8000);
                break;
            case EVENT_RAOUL_BARREL_SMASH:
                castSpellAOE(SPELL_RAOUL_BARREL_SMASH);
                scriptEvents.addEvent(EVENT_RAOUL_BARREL_SMASH, 8000);
                break;
            default:
                break;
        }
    }
};

class FreeholdEudoraAI : public FreeholdCaptainAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FreeholdEudoraAI(c); }
    explicit FreeholdEudoraAI(Creature* pCreature) : FreeholdCaptainAI(pCreature) {}

protected:
    void onCaptainEngage() override
    {
        mVolleysLeft = 0;
        scriptEvents.addEvent(EVENT_EUDORA_POWDER_SHOT, Util::getRandomUInt(3000, 5000));
        scriptEvents.addEvent(EVENT_EUDORA_GRAPESHOT_LEAP, 8000);
    }

    void onReset() override
    {
        mVolleysLeft = 0;
        setReactState(REACT_AGGRESSIVE);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_EUDORA_POWDER_SHOT:
                castAtPlayer(SPELL_EUDORA_POWDER_SHOT);
                scriptEvents.addEvent(EVENT_EUDORA_POWDER_SHOT, 8000);
                break;
            case EVENT_EUDORA_GRAPESHOT_LEAP:
                leapAside();
                break;
            case EVENT_EUDORA_GRAPESHOT_VOLLEY:
                fireVolley();
                break;
            default:
                break;
        }
    }

private:
    // jumps clear of the melee and sprays the deck
    void leapAside()
    {
        warn("Captain Eudora takes aim with her Grapeshot!");
        scriptEvents.removeEvent(EVENT_EUDORA_POWDER_SHOT);
        setReactState(REACT_PASSIVE);
        attackStop();
        moveJump(aroundSelf(18.0f, 20.0f), 10.0f, 10.0f);

        mVolleysLeft = EudoraGrapeshotVolleys;
        scriptEvents.addEvent(EVENT_EUDORA_GRAPESHOT_VOLLEY, 1500);
    }

    void fireVolley()
    {
        castAtPlayer(SPELL_EUDORA_GRAPESHOT, false, true);
        if (mVolleysLeft > 0 && --mVolleysLeft > 0)
        {
            scriptEvents.addEvent(EVENT_EUDORA_GRAPESHOT_VOLLEY, 1500);
            return;
        }

        setReactState(REACT_AGGRESSIVE);
        scriptEvents.addEvent(EVENT_EUDORA_POWDER_SHOT, Util::getRandomUInt(3000, 5000));
        scriptEvents.addEvent(EVENT_EUDORA_GRAPESHOT_LEAP, 15000);
    }

    uint32_t mVolleysLeft = 0;
};

class FreeholdJollyAI : public FreeholdCaptainAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FreeholdJollyAI(c); }
    explicit FreeholdJollyAI(Creature* pCreature) : FreeholdCaptainAI(pCreature) {}

protected:
    void onCaptainEngage() override
    {
        scriptEvents.addEvent(EVENT_JOLLY_CUTTING_SURGE, Util::getRandomUInt(3000, 5000));
        scriptEvents.addEvent(EVENT_JOLLY_WHIRLPOOL_OF_BLADES, 8000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_JOLLY_CUTTING_SURGE:
                castAtPlayer(SPELL_JOLLY_CUTTING_SURGE);
                scriptEvents.addEvent(EVENT_JOLLY_CUTTING_SURGE, 15000);
                break;
            case EVENT_JOLLY_WHIRLPOOL_OF_BLADES:
                castAtPlayer(SPELL_JOLLY_WHIRLPOOL_OF_BLADES);
                scriptEvents.addEvent(EVENT_JOLLY_WHIRLPOOL_OF_BLADES, 15000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ring of Booty

// opening bout, his fall calls the main event in
class FreeholdLudwigAI : public FreeholdBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FreeholdLudwigAI(c); }
    explicit FreeholdLudwigAI(Creature* pCreature) : FreeholdBossAI(pCreature, DATA_RING_OF_BOOTY, false) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_LUDWIG_SHELL_BOUNCE, 5000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_LUDWIG_SHELL_BOUNCE)
            return;

        castAtPlayer(SPELL_LUDWIG_SHELL_BOUNCE);
        scriptEvents.addEvent(EVENT_LUDWIG_SHELL_BOUNCE, 8000);
    }

    void onDefeat() override
    {
        if (FreeholdInstanceScript* pFreehold = getFreehold())
            pFreehold->callTrothak();
    }
};

class FreeholdTrothakAI : public FreeholdBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FreeholdTrothakAI(c); }
    explicit FreeholdTrothakAI(Creature* pCreature) : FreeholdBossAI(pCreature, DATA_RING_OF_BOOTY) {}

protected:
    void onEngage() override
    {
        mSharkGuid = WoWGuid();
        mHammerNext = Util::getRandomUInt(1) == 0;
        scriptEvents.addEvent(EVENT_TROTHAK_RIPPER_PUNCH, 3000);
        scriptEvents.addEvent(EVENT_TROTHAK_SHARK_TOSS, 8000);
        scriptEvents.addEvent(EVENT_TROTHAK_SHARK_TORNADO, 10000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_TROTHAK_THROW_CHUM, 8000);
    }

    void onReset() override { mSharkGuid = WoWGuid(); }

    void onDefeat() override
    {
        if (FreeholdInstanceScript* pFreehold = getFreehold())
            pFreehold->markTrothakBeaten();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TROTHAK_RIPPER_PUNCH:
                castSpellOnVictim(SPELL_TROTHAK_RIPPER_PUNCH);
                scriptEvents.addEvent(EVENT_TROTHAK_RIPPER_PUNCH, 15000);
                break;
            case EVENT_TROTHAK_SHARK_TORNADO:
                castSpellOnSelf(SPELL_TROTHAK_SHARK_TORNADO);
                scriptEvents.addEvent(EVENT_TROTHAK_SHARK_TORNADO, 15000);
                break;
            case EVENT_TROTHAK_SHARK_TOSS:
                tossShark();
                break;
            case EVENT_TROTHAK_REARM:
                rearm();
                scriptEvents.addEvent(EVENT_TROTHAK_SHARK_TOSS, 10000);
                break;
            case EVENT_TROTHAK_THROW_CHUM:
                castSpellOnSelf(SPELL_TROTHAK_THROW_CHUM, true);
                scriptEvents.addEvent(EVENT_TROTHAK_THROW_CHUM, 10000);
                break;
            default:
                break;
        }
    }

private:
    // one of his sharks lands next to a player and thrashes there
    void tossShark()
    {
        Unit* pTarget = pickPlayer();
        if (pTarget == nullptr)
        {
            scriptEvents.addEvent(EVENT_TROTHAK_SHARK_TOSS, 2000);
            return;
        }

        castSpell(pTarget, SPELL_TROTHAK_SHARK_TOSS, true);
        uint32_t const sharkEntry = mHammerNext ? NPC_HAMMER_SHARK : NPC_SAWTOOTH_SHARK;
        mHammerNext = !mHammerNext;

        Creature* pShark = summonCreature(sharkEntry, pTarget->GetPosition(), MANUAL_DESPAWN);
        mSharkGuid = pShark != nullptr ? pShark->GetNewGUID() : WoWGuid();

        scriptEvents.addEvent(EVENT_TROTHAK_REARM, 15000);
    }

    // charges back to the shark and takes it up again
    void rearm()
    {
        Creature* pShark = mSharkGuid.isEmpty() || getCreature()->getWorldMap() == nullptr ? nullptr : getCreature()->getWorldMap()->getCreature(mSharkGuid);
        mSharkGuid = WoWGuid();
        if (pShark == nullptr || !pShark->isAlive())
            return;

        warn("Trothak charges to rearm himself with his shark!");
        moveCharge(pShark->GetPosition(), 30.0f);
        pShark->despawn(1500, 0);
    }

    WoWGuid mSharkGuid;
    bool mHammerNext = true;
};

// a stranded shark flails at whoever comes close
class FreeholdTossedSharkAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FreeholdTossedSharkAI(c); }
    explicit FreeholdTossedSharkAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        setRooted(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mFlailClock > diff)
        {
            mFlailClock -= diff;
            return;
        }

        mFlailClock = 6000;

        Unit* pNearest = nullptr;
        float nearestRange = 30.0f;
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

        if (pNearest != nullptr)
            castSpell(pNearest, SPELL_SHARK_FLAILING_SHARK);
    }

private:
    uint32_t mFlailClock = 1500;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Harlan Sweete, loads his dice twice

class FreeholdHarlanAI : public FreeholdBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FreeholdHarlanAI(c); }
    explicit FreeholdHarlanAI(Creature* pCreature) : FreeholdBossAI(pCreature, DATA_HARLAN_SWEETE) {}

protected:
    void onEngage() override
    {
        mAllHands = false;
        mManOWar = false;
        scriptEvents.addEvent(EVENT_HARLAN_CANNON_BARRAGE, 8000);
        scriptEvents.addEvent(EVENT_HARLAN_SWIFTWIND_SABER, 12000);
        scriptEvents.addEvent(EVENT_HARLAN_GRENADIER, 15000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mAllHands && _getHealthPercent() <= 60)
        {
            mAllHands = true;
            warn("Harlan Sweete rolls the Loaded Dice: All Hands!");
            castSpellOnSelf(SPELL_HARLAN_LOADED_DICE_HANDS, true);
        }

        if (!mManOWar && _getHealthPercent() <= 30)
        {
            mManOWar = true;
            warn("Harlan Sweete rolls the Loaded Dice: Man-O-War!");
            castSpellOnSelf(SPELL_HARLAN_LOADED_DICE_WAR, true);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HARLAN_CANNON_BARRAGE:
                fireCannons();
                scriptEvents.addEvent(EVENT_HARLAN_CANNON_BARRAGE, mManOWar ? 8000 : 12000);
                break;
            case EVENT_HARLAN_SWIFTWIND_SABER:
                throwSabers();
                scriptEvents.addEvent(EVENT_HARLAN_SWIFTWIND_SABER, mManOWar ? 8000 : 12000);
                break;
            case EVENT_HARLAN_GRENADIER:
                summonCreature(NPC_IRONTIDE_GRENADIER, GrenadierDeck, CORPSE_DESPAWN);
                scriptEvents.addEvent(EVENT_HARLAN_GRENADIER, mManOWar ? 15000 : 20000);
                break;
            default:
                break;
        }
    }

private:
    // a second target joins the barrage once all hands are on deck
    void fireCannons()
    {
        Unit* pFirst = castAtPlayer(SPELL_HARLAN_CANNON_BARRAGE, true);
        if (!mAllHands || pFirst == nullptr)
            return;

        Unit* pTank = getCreature()->getAIInterface()->getCurrentTarget();
        std::vector<Unit*> others;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pPlayer != pFirst && pPlayer != pTank)
                others.push_back(pPlayer);
        }

        if (!others.empty())
            castSpell(others[Util::getRandomUInt(static_cast<uint32_t>(others.size() - 1))], SPELL_HARLAN_CANNON_BARRAGE, true);
    }

    // a fan of blades flies out in front of him
    void throwSabers()
    {
        castSpellOnSelf(SPELL_HARLAN_SWIFTWIND_SABER);

        Creature* pSelf = getCreature();
        uint32_t const count = mAllHands ? SaberBaseCount * 2 : SaberBaseCount;
        for (uint32_t i = 0; i < count; ++i)
        {
            float const facing = pSelf->GetOrientation() - (FreeholdPi / 6.0f) * static_cast<float>(i);
            summonCreature(NPC_SWIFTWIND_SABER, LocationVector(pSelf->GetPositionX(), pSelf->GetPositionY(), pSelf->GetPositionZ(), facing), TIMED_DESPAWN, 6000);
        }
    }

    bool mAllHands = false;
    bool mManOWar = false;
};

// flies straight ahead and cuts everyone in its path once
class FreeholdSwiftwindSaberAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FreeholdSwiftwindSaberAI(c); }
    explicit FreeholdSwiftwindSaberAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        uint32_t const diff = static_cast<uint32_t>(time_passed);

        if (mLaunchClock != 0)
        {
            if (mLaunchClock > diff)
            {
                mLaunchClock -= diff;
                return;
            }

            mLaunchClock = 0;
            Creature* pSelf = getCreature();
            float const facing = pSelf->GetOrientation();
            moveTo(pSelf->GetPositionX() + SaberTravel * std::cos(facing), pSelf->GetPositionY() + SaberTravel * std::sin(facing), pSelf->GetPositionZ());
        }

        if (mCutClock > diff)
        {
            mCutClock -= diff;
            return;
        }

        mCutClock = 500;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer == nullptr || !pPlayer->isAlive() || getCreature()->getDistance(pPlayer) > 3.0f)
                continue;

            if (mCutGuids.insert(pPlayer->GetNewGUID()).second)
                castSpell(pPlayer, SPELL_SABER_SWIFTWIND_SABER, true);
        }
    }

private:
    uint32_t mLaunchClock = 2000;
    uint32_t mCutClock = 500;
    std::set<WoWGuid> mCutGuids;
};

// marks a player, runs at him and blows up on arrival
class FreeholdGrenadierAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FreeholdGrenadierAI(c); }
    explicit FreeholdGrenadierAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mBlown || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mStepClock > diff)
        {
            mStepClock -= diff;
            return;
        }

        mStepClock = 500;

        Unit* pMark = getMark();
        if (pMark == nullptr || !pMark->isAlive())
        {
            chooseMark();
            return;
        }

        if (getCreature()->getDistance(pMark) > 3.0f)
            return;

        mBlown = true;
        castSpellAOE(SPELL_GRENADIER_BLACK_POWDER, true);
        despawn(1000, 0);
    }

private:
    Unit* getMark()
    {
        if (mMarkGuid.isEmpty() || getCreature()->getWorldMap() == nullptr)
            return nullptr;

        return getCreature()->getWorldMap()->getUnit(mMarkGuid);
    }

    void chooseMark()
    {
        std::vector<Unit*> players;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive())
                players.push_back(pPlayer);
        }

        mMarkGuid = WoWGuid();
        if (players.empty())
            return;

        Unit* pMark = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
        mMarkGuid = pMark->GetNewGUID();
        castSpell(pMark, SPELL_GRENADIER_BOMB_MARK, true);
        moveChase(pMark);
    }

    WoWGuid mMarkGuid;
    uint32_t mStepClock = 800;
    bool mBlown = false;
};

void SetupFreehold(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_FREEHOLD, &FreeholdInstanceScript::Create);

    mgr->register_creature_script(BOSS_SKYCAPN_KRAGG, &FreeholdKraggAI::Create);
    mgr->register_creature_script(NPC_SHARKBAIT, &FreeholdSharkbaitAI::Create);

    mgr->register_creature_script(BOSS_CAPTAIN_RAOUL, &FreeholdRaoulAI::Create);
    mgr->register_creature_script(BOSS_CAPTAIN_EUDORA, &FreeholdEudoraAI::Create);
    mgr->register_creature_script(BOSS_CAPTAIN_JOLLY, &FreeholdJollyAI::Create);

    mgr->register_creature_script(BOSS_LUDWIG_VON_TORTOLLAN, &FreeholdLudwigAI::Create);
    mgr->register_creature_script(BOSS_TROTHAK, &FreeholdTrothakAI::Create);
    mgr->register_creature_script(NPC_HAMMER_SHARK, &FreeholdTossedSharkAI::Create);
    mgr->register_creature_script(NPC_SAWTOOTH_SHARK, &FreeholdTossedSharkAI::Create);

    mgr->register_creature_script(BOSS_HARLAN_SWEETE, &FreeholdHarlanAI::Create);
    mgr->register_creature_script(NPC_SWIFTWIND_SABER, &FreeholdSwiftwindSaberAI::Create);
    mgr->register_creature_script(NPC_IRONTIDE_GRENADIER, &FreeholdGrenadierAI::Create);
}
