/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Brackenhide_Hollow.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <map>
#include <vector>

enum BrackenhideHollowEvents
{
    EVENT_GUTSHOT_GUT_SHOT = 1,
    EVENT_GUTSHOT_ENSNARING_TRAP,
    EVENT_GUTSHOT_MASTERS_CALL,
    EVENT_GUTSHOT_MEAT_TOSS,

    EVENT_HYENA_BOUNDING_LEAP,
    EVENT_HYENA_CRIPPLING_BITE,

    EVENT_TREEMOUTH_VINE_WHIP,
    EVENT_TREEMOUTH_DECAY_SPRAY,
    EVENT_TREEMOUTH_INFECTIOUS_SPIT,
    EVENT_TREEMOUTH_CONSUME,

    EVENT_SLIME_GUSHING_OOZE,

    EVENT_WRATHEYE_DECAYSTRIKE,
    EVENT_WRATHEYE_CHOKING_ROTCLOUD,
    EVENT_WRATHEYE_ROTBURST_TOTEM,

    EVENT_RIRA_BLADESTORM,
    EVENT_RIRA_CLEAVE,
    EVENT_RIRA_SAVAGE_CHARGE,

    EVENT_GASHTOOTH_GASH_FRENZY,
    EVENT_GASHTOOTH_MARKED_FOR_BUTCHERY,
    EVENT_GASHTOOTH_DECAYED_SENSES,

    EVENT_TRICKTOTEM_EARTH_BOLT,
    EVENT_TRICKTOTEM_HEALING_RAPIDS,
    EVENT_TRICKTOTEM_HEXTRICK_TOTEM
};

namespace
{
    // members of the encounters fought as one
    struct BrackenhideGroupSlot
    {
        uint32_t dataId;
        uint32_t entry;
    };

    BrackenhideGroupSlot const BrackenhideGroupSlots[] =
    {
        { DATA_HACKCLAWS_WAR_BAND, BOSS_RIRA_HACKCLAW },
        { DATA_HACKCLAWS_WAR_BAND, BOSS_GASHTOOTH },
        { DATA_HACKCLAWS_WAR_BAND, BOSS_TRICKTOTEM }
    };

    // energy bars, time from empty to full
    uint32_t const GutshotHyenaCallMs = 40000;
    uint32_t const TreemouthVinesMs = 36000;
    uint32_t const WratheyeStrengthMs = 45000;
    uint32_t const RiraComboMs = 42000;

    uint8_t const WarBandBloodfrenzyPct = 15;
    uint8_t const WarBandHealBelowPct = 70;

    LocationVector brackenhideRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(6.2831853f);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // random living player around a creature without a threat list
    Unit* brackenhidePlayerAround(Creature* pCreature, float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : pCreature->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pCreature->getDistance(pPlayer) <= range)
                players.push_back(pPlayer);
        }

        if (players.empty())
            return nullptr;

        return players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class BrackenhideHollowInstanceScript : public InstanceScript
{
public:
    explicit BrackenhideHollowInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(BrackenhideHollowEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new BrackenhideHollowInstanceScript(pMapMgr); }

    // a fallen war-band member must not end the encounter while the others stand
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (BrackenhideGroupSlot const& slot : BrackenhideGroupSlots)
        {
            if (slot.entry == pCreature->getEntry())
            {
                mMemberGuids[slot.entry] = pCreature->GetNewGUID();
                return;
            }
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (state != EncounterStates::Failed)
            return;

        // a wiped group fights again in full
        for (Creature* pMember : getGroup(id))
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    Creature* getMember(uint32_t entry)
    {
        auto const itr = mMemberGuids.find(entry);
        return itr != mMemberGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    std::vector<Creature*> getGroup(uint32_t dataId)
    {
        std::vector<Creature*> group;
        for (BrackenhideGroupSlot const& slot : BrackenhideGroupSlots)
        {
            if (slot.dataId != dataId)
                continue;

            if (Creature* pMember = getMember(slot.entry))
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

    bool isGroupFighting(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat())
                return true;
        }

        return false;
    }

    // pulling one member pulls the rest
    void rallyGroup(uint32_t dataId, Creature* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember == pPuller || !pMember->isAlive() || pMember->isInCombat() || pMember->GetScript() == nullptr)
                continue;

            pMember->GetScript()->setZoneWideCombat();
        }
        mRallying = false;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        Creature* pMember = getMember(entry);
        if (pMember != nullptr && pMember->isAlive() && pMember->GetScript() != nullptr)
            pMember->GetScript()->DoAction(action);
    }

private:
    std::map<uint32_t, WoWGuid> mMemberGuids;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy stand-in and target helpers

class BrackenhideHollowCombatAI : public CreatureAIScript
{
public:
    explicit BrackenhideHollowCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mEnergyMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mEnergyMs = 0;

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

    BrackenhideHollowInstanceScript* getHollow()
    {
        if (getCreature()->GetMapId() != MAP_BRACKENHIDE_HOLLOW)
            return nullptr;

        return static_cast<BrackenhideHollowInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    Unit* nearestPlayer() { return selectUnitTarget(FilterArgs(TargetFilter(TargetFilter_Player | TargetFilter_Closest))); }

    Unit* castAtPlayer(uint32_t spellId, bool triggered = false)
    {
        Unit* pTarget = anyPlayer();
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    // normal difficulty has its own spell versions
    uint32_t byDifficulty(uint32_t heroicSpellId, uint32_t normalSpellId) { return isHeroic() ? heroicSpellId : normalSpellId; }

    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return brackenhideRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

    void notify(uint32_t entry, int32_t action)
    {
        if (BrackenhideHollowInstanceScript* pHollow = getHollow())
            pHollow->sendAction(entry, action);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // energy stand-in: true once the bar is full and nothing is being cast
    bool energyFull(uint32_t diff, uint32_t fullAfterMs)
    {
        if (mEnergyMs < fullAfterMs)
            mEnergyMs += diff;

        if (mEnergyMs < fullAfterMs || _isCasting())
            return false;

        mEnergyMs = 0;
        return true;
    }

    uint32_t mEnergyMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class BrackenhideHollowBossAI : public BrackenhideHollowCombatAI
{
public:
    BrackenhideHollowBossAI(Creature* pCreature, uint32_t dataId) : BrackenhideHollowCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (BrackenhideHollowInstanceScript* pHollow = getHollow())
        {
            pHollow->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pHollow->setBossState(mDataId, EncounterStates::InProgress);
            pHollow->rallyGroup(mDataId, getCreature());
        }

        BrackenhideHollowCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (BrackenhideHollowInstanceScript* pHollow = getHollow())
            {
                pHollow->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last partner has left the fight
                if (!pHollow->isGroupFighting(mDataId, getCreature()))
                    pHollow->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        BrackenhideHollowCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (BrackenhideHollowInstanceScript* pHollow = getHollow())
        {
            pHollow->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            if (pHollow->isGroupDown(mDataId, getCreature()))
                pHollow->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gutshot

class BrackenhideHollowGutshotAI : public BrackenhideHollowBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BrackenhideHollowGutshotAI(c); }
    explicit BrackenhideHollowGutshotAI(Creature* pCreature) : BrackenhideHollowBossAI(pCreature, DATA_GUTSHOT) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GUTSHOT_GUT_SHOT, 8000);
        scriptEvents.addEvent(EVENT_GUTSHOT_ENSNARING_TRAP, 12000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_GUTSHOT_MEAT_TOSS, 18000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!energyFull(diff, GutshotHyenaCallMs))
            return;

        warn("Gutshot calls her hyenas!");
        for (uint8_t i = 0; i < 2; ++i)
            callAdd(NPC_ROTFANG_HYENA, aroundSelf(4.0f, 8.0f));
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GUTSHOT_GUT_SHOT:
                castSpellOnVictim(SPELL_GUTSHOT_GUT_SHOT);
                scriptEvents.addEvent(EVENT_GUTSHOT_GUT_SHOT, 16000);
                break;
            case EVENT_GUTSHOT_ENSNARING_TRAP:
                castAtPlayer(SPELL_GUTSHOT_ENSNARING_TRAP);
                scriptEvents.addEvent(EVENT_GUTSHOT_MASTERS_CALL, 6000);
                scriptEvents.addEvent(EVENT_GUTSHOT_ENSNARING_TRAP, 24000);
                break;
            case EVENT_GUTSHOT_MASTERS_CALL:
                // frees her hyenas from the traps
                if (!livingHyenas().empty())
                    castSpellAOE(SPELL_GUTSHOT_MASTERS_CALL);
                break;
            case EVENT_GUTSHOT_MEAT_TOSS:
                tossMeat();
                scriptEvents.addEvent(EVENT_GUTSHOT_MEAT_TOSS, 28000);
                break;
            default:
                break;
        }
    }

private:
    std::vector<Creature*> livingHyenas()
    {
        std::list<Creature*> hyenas;
        GetCreatureListWithEntryInGrid(hyenas, NPC_ROTFANG_HYENA, 60.0f);

        std::vector<Creature*> living;
        for (Creature* pHyena : hyenas)
        {
            if (pHyena->isAlive())
                living.push_back(pHyena);
        }

        return living;
    }

    // every hyena turns on the player holding the meat
    void tossMeat()
    {
        Unit* pTarget = anyPlayer();
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, SPELL_GUTSHOT_MEAT_TOSS);
        for (Creature* pHyena : livingHyenas())
        {
            addThreat(pTarget, 100000.0f, pHyena);
            castSpell(pHyena, SPELL_GUTSHOT_FEEDING_FRENZY, true);
        }
    }
};

class BrackenhideHollowRotfangHyenaAI : public BrackenhideHollowCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BrackenhideHollowRotfangHyenaAI(c); }
    explicit BrackenhideHollowRotfangHyenaAI(Creature* pCreature) : BrackenhideHollowCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_HYENA_CRIPPLING_BITE, 4000);
        scriptEvents.addEvent(EVENT_HYENA_BOUNDING_LEAP, 6000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HYENA_BOUNDING_LEAP:
                castAtPlayer(byDifficulty(SPELL_HYENA_BOUNDING_LEAP, SPELL_HYENA_BOUNDING_LEAP_NORMAL));
                scriptEvents.addEvent(EVENT_HYENA_BOUNDING_LEAP, 14000);
                break;
            case EVENT_HYENA_CRIPPLING_BITE:
                castSpellOnVictim(SPELL_HYENA_CRIPPLING_BITE);
                scriptEvents.addEvent(EVENT_HYENA_CRIPPLING_BITE, 10000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Treemouth

class BrackenhideHollowTreemouthAI : public BrackenhideHollowBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BrackenhideHollowTreemouthAI(c); }
    explicit BrackenhideHollowTreemouthAI(Creature* pCreature) : BrackenhideHollowBossAI(pCreature, DATA_TREEMOUTH) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_TREEMOUTH_INFECTIOUS_SPIT, 5000);
        scriptEvents.addEvent(EVENT_TREEMOUTH_VINE_WHIP, 7000);
        scriptEvents.addEvent(EVENT_TREEMOUTH_DECAY_SPRAY, 12000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!energyFull(diff, TreemouthVinesMs))
            return;

        warn("Treemouth drags everyone in with Grasping Vines!");
        castSpellAOE(SPELL_TREEMOUTH_GRASPING_VINES);
        scriptEvents.addEvent(EVENT_TREEMOUTH_CONSUME, 2500);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TREEMOUTH_VINE_WHIP:
                castSpellOnVictim(SPELL_TREEMOUTH_VINE_WHIP);
                scriptEvents.addEvent(EVENT_TREEMOUTH_VINE_WHIP, 16000);
                break;
            case EVENT_TREEMOUTH_DECAY_SPRAY:
                castSpellAOE(SPELL_TREEMOUTH_DECAY_SPRAY);
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_DECAYING_SLIME, aroundSelf(6.0f, 12.0f));
                scriptEvents.addEvent(EVENT_TREEMOUTH_DECAY_SPRAY, 26000);
                break;
            case EVENT_TREEMOUTH_INFECTIOUS_SPIT:
                castAtPlayer(SPELL_TREEMOUTH_INFECTIOUS_SPIT);
                scriptEvents.addEvent(EVENT_TREEMOUTH_INFECTIOUS_SPIT, 12000);
                break;
            case EVENT_TREEMOUTH_CONSUME:
                if (Unit* pTarget = nearestPlayer())
                    castSpell(pTarget, SPELL_TREEMOUTH_CONSUME);
                break;
            default:
                break;
        }
    }
};

// bursts into a pool of decay when slain
class BrackenhideHollowDecayingSlimeAI : public BrackenhideHollowCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BrackenhideHollowDecayingSlimeAI(c); }
    explicit BrackenhideHollowDecayingSlimeAI(Creature* pCreature) : BrackenhideHollowCombatAI(pCreature) {}

    void OnDied(Unit* /*pKiller*/) override { castSpellOnSelf(SPELL_SLIME_BURST, true); }

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_SLIME_GUSHING_OOZE, 4000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SLIME_GUSHING_OOZE)
            return;

        castAtPlayer(SPELL_SLIME_GUSHING_OOZE);
        scriptEvents.addEvent(EVENT_SLIME_GUSHING_OOZE, 9000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Decatriarch Wratheye

class BrackenhideHollowWratheyeAI : public BrackenhideHollowBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BrackenhideHollowWratheyeAI(c); }
    explicit BrackenhideHollowWratheyeAI(Creature* pCreature) : BrackenhideHollowBossAI(pCreature, DATA_DECATRIARCH_WRATHEYE) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_WRATHEYE_DECAYSTRIKE, 6000);
        scriptEvents.addEvent(EVENT_WRATHEYE_CHOKING_ROTCLOUD, 10000);
        scriptEvents.addEvent(EVENT_WRATHEYE_ROTBURST_TOTEM, 15000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!energyFull(diff, WratheyeStrengthMs))
            return;

        warn("Decatriarch Wratheye absorbs the Withering Rot!");
        castSpellOnSelf(byDifficulty(SPELL_WRATHEYE_DECAYING_STRENGTH, SPELL_WRATHEYE_DECAYING_STRENGTH_NORMAL));
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_WRATHEYE_DECAYSTRIKE:
                castSpellOnVictim(SPELL_WRATHEYE_DECAYSTRIKE);
                scriptEvents.addEvent(EVENT_WRATHEYE_DECAYSTRIKE, 15000);
                break;
            case EVENT_WRATHEYE_CHOKING_ROTCLOUD:
                castSpellAOE(byDifficulty(SPELL_WRATHEYE_CHOKING_ROTCLOUD, SPELL_WRATHEYE_CHOKING_ROTCLOUD_NORMAL));
                scriptEvents.addEvent(EVENT_WRATHEYE_CHOKING_ROTCLOUD, 20000);
                break;
            case EVENT_WRATHEYE_ROTBURST_TOTEM:
                placeTotem();
                scriptEvents.addEvent(EVENT_WRATHEYE_ROTBURST_TOTEM, 30000);
                break;
            default:
                break;
        }
    }

private:
    // the totem rises next to a player
    void placeTotem()
    {
        Unit* pTarget = anyPlayer();
        LocationVector const spot = pTarget != nullptr ? brackenhideRingPoint(pTarget->GetPosition(), 2.0f, 4.0f) : aroundSelf(5.0f, 10.0f);
        summonCreature(NPC_ROTBURST_TOTEM, spot, CORPSE_DESPAWN);
    }
};

// stationary totem, pulses its spell while it stands
class BrackenhideHollowTotemAI : public CreatureAIScript
{
public:
    static CreatureAIScript* CreateRotburst(Creature* c) { return new BrackenhideHollowTotemAI(c, SPELL_TOTEM_ROTTING_BURST, SPELL_TOTEM_ROTTING_BURST_NORMAL, 2000, 4000, false); }
    static CreatureAIScript* CreateHextrick(Creature* c) { return new BrackenhideHollowTotemAI(c, SPELL_HEXTRICK_TOTEM_HEXTRICK, SPELL_HEXTRICK_TOTEM_HEXTRICK, 1000, 0, true); }

    BrackenhideHollowTotemAI(Creature* pCreature, uint32_t heroicSpellId, uint32_t normalSpellId, uint32_t firstMs, uint32_t repeatMs, bool atPlayer)
        : CreatureAIScript(pCreature), mHeroicSpellId(heroicSpellId), mNormalSpellId(normalSpellId), mClockMs(firstMs), mRepeatMs(repeatMs), mAtPlayer(atPlayer) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    // the hex holds only as long as the totem
    void OnDied(Unit* /*pKiller*/) override
    {
        if (mAtPlayer)
            _removeAuraOnPlayers(mHeroicSpellId);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive() || mClockMs == 0)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mClockMs > diff)
        {
            mClockMs -= diff;
            return;
        }

        // a repeat of 0 pulses once
        mClockMs = mRepeatMs;
        uint32_t const spellId = isHeroic() ? mHeroicSpellId : mNormalSpellId;
        if (!mAtPlayer)
        {
            castSpellAOE(spellId, true);
            return;
        }

        if (Unit* pTarget = brackenhidePlayerAround(getCreature(), 40.0f))
            castSpell(pTarget, spellId, true);
    }

private:
    uint32_t mHeroicSpellId;
    uint32_t mNormalSpellId;
    uint32_t mClockMs;
    uint32_t mRepeatMs;
    bool mAtPlayer;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hackclaw's War-Band

class BrackenhideHollowRiraHackclawAI : public BrackenhideHollowBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BrackenhideHollowRiraHackclawAI(c); }
    explicit BrackenhideHollowRiraHackclawAI(Creature* pCreature) : BrackenhideHollowBossAI(pCreature, DATA_HACKCLAWS_WAR_BAND) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_RIRA_BLADESTORM, 15000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_RIRA_CLEAVE, 6000);
    }

    // full energy: totem, senses and charge in one sequence
    void onCombatTick(uint32_t diff) override
    {
        if (!energyFull(diff, RiraComboMs))
            return;

        warn("Rira Hackclaw calls the war-band to a combined attack!");
        notify(BOSS_TRICKTOTEM, ACTION_WARBAND_HEXTRICK_TOTEM);
        notify(BOSS_GASHTOOTH, ACTION_WARBAND_DECAYED_SENSES);
        scriptEvents.addEvent(EVENT_RIRA_SAVAGE_CHARGE, 4000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RIRA_BLADESTORM:
                castSpellOnSelf(SPELL_RIRA_BLADESTORM);
                scriptEvents.addEvent(EVENT_RIRA_BLADESTORM, 30000);
                break;
            case EVENT_RIRA_CLEAVE:
                castSpellOnVictim(SPELL_RIRA_CLEAVE);
                scriptEvents.addEvent(EVENT_RIRA_CLEAVE, 10000);
                break;
            case EVENT_RIRA_SAVAGE_CHARGE:
                castAtPlayer(SPELL_RIRA_SAVAGE_CHARGE);
                break;
            default:
                break;
        }
    }
};

class BrackenhideHollowGashtoothAI : public BrackenhideHollowBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BrackenhideHollowGashtoothAI(c); }
    explicit BrackenhideHollowGashtoothAI(Creature* pCreature) : BrackenhideHollowBossAI(pCreature, DATA_HACKCLAWS_WAR_BAND) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_WARBAND_DECAYED_SENSES && _isInCombat())
            scriptEvents.addEvent(EVENT_GASHTOOTH_DECAYED_SENSES, 2000);
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GASHTOOTH_GASH_FRENZY, 7000);
        scriptEvents.addEvent(EVENT_GASHTOOTH_MARKED_FOR_BUTCHERY, 16000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GASHTOOTH_GASH_FRENZY:
                castSpellOnVictim(byDifficulty(SPELL_GASHTOOTH_GASH_FRENZY, SPELL_GASHTOOTH_GASH_FRENZY_NORMAL));
                scriptEvents.addEvent(EVENT_GASHTOOTH_GASH_FRENZY, 18000);
                break;
            case EVENT_GASHTOOTH_MARKED_FOR_BUTCHERY:
                castAtPlayer(SPELL_GASHTOOTH_MARKED_FOR_BUTCHERY);
                scriptEvents.addEvent(EVENT_GASHTOOTH_MARKED_FOR_BUTCHERY, 30000);
                break;
            case EVENT_GASHTOOTH_DECAYED_SENSES:
                castAtPlayer(byDifficulty(SPELL_GASHTOOTH_DECAYED_SENSES, SPELL_GASHTOOTH_DECAYED_SENSES_NORMAL));
                break;
            default:
                break;
        }
    }
};

class BrackenhideHollowTricktotemAI : public BrackenhideHollowBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BrackenhideHollowTricktotemAI(c); }
    explicit BrackenhideHollowTricktotemAI(Creature* pCreature) : BrackenhideHollowBossAI(pCreature, DATA_HACKCLAWS_WAR_BAND) {}

    void DoAction(int32_t action) override
    {
        // the hex totem is part of the heroic sequence only
        if (action == ACTION_WARBAND_HEXTRICK_TOTEM && _isInCombat() && isHeroic())
            scriptEvents.addEvent(EVENT_TRICKTOTEM_HEXTRICK_TOTEM, 500);
    }

protected:
    void onEngage() override
    {
        mBloodfrenzy = false;
        scriptEvents.addEvent(EVENT_TRICKTOTEM_EARTH_BOLT, 2000);
        scriptEvents.addEvent(EVENT_TRICKTOTEM_HEALING_RAPIDS, 12000);
    }

    void onReset() override { mBloodfrenzy = false; }

    // once any member of the band is close to death
    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mBloodfrenzy || _isCasting())
            return;

        BrackenhideHollowInstanceScript* pHollow = getHollow();
        if (pHollow == nullptr)
            return;

        for (Creature* pMember : pHollow->getGroup(DATA_HACKCLAWS_WAR_BAND))
        {
            if (!pMember->isAlive() || pMember->getHealthPct() > WarBandBloodfrenzyPct)
                continue;

            mBloodfrenzy = true;
            warn("Tricktotem whips the war-band into a Bloodfrenzy!");
            castSpellAOE(SPELL_TRICKTOTEM_BLOODFRENZY);
            return;
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TRICKTOTEM_EARTH_BOLT:
                castAtPlayer(SPELL_TRICKTOTEM_EARTH_BOLT);
                scriptEvents.addEvent(EVENT_TRICKTOTEM_EARTH_BOLT, 6000);
                break;
            case EVENT_TRICKTOTEM_HEALING_RAPIDS:
                if (Creature* pWounded = mostWoundedMember())
                    castSpell(pWounded, SPELL_TRICKTOTEM_GREATER_HEALING_RAPIDS);
                scriptEvents.addEvent(EVENT_TRICKTOTEM_HEALING_RAPIDS, 18000);
                break;
            case EVENT_TRICKTOTEM_HEXTRICK_TOTEM:
                if (Unit* pTarget = anyPlayer())
                    summonCreature(NPC_HEXTRICK_TOTEM, brackenhideRingPoint(pTarget->GetPosition(), 2.0f, 4.0f), CORPSE_DESPAWN);
                break;
            default:
                break;
        }
    }

private:
    Creature* mostWoundedMember()
    {
        BrackenhideHollowInstanceScript* pHollow = getHollow();
        if (pHollow == nullptr)
            return nullptr;

        Creature* pWounded = nullptr;
        for (Creature* pMember : pHollow->getGroup(DATA_HACKCLAWS_WAR_BAND))
        {
            if (!pMember->isAlive() || pMember->getHealthPct() >= WarBandHealBelowPct)
                continue;

            if (pWounded == nullptr || pMember->getHealthPct() < pWounded->getHealthPct())
                pWounded = pMember;
        }

        return pWounded;
    }

    bool mBloodfrenzy = false;
};

void SetupBrackenhideHollow(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BRACKENHIDE_HOLLOW, &BrackenhideHollowInstanceScript::Create);

    mgr->register_creature_script(BOSS_GUTSHOT, &BrackenhideHollowGutshotAI::Create);
    mgr->register_creature_script(NPC_ROTFANG_HYENA, &BrackenhideHollowRotfangHyenaAI::Create);

    mgr->register_creature_script(BOSS_TREEMOUTH, &BrackenhideHollowTreemouthAI::Create);
    mgr->register_creature_script(NPC_DECAYING_SLIME, &BrackenhideHollowDecayingSlimeAI::Create);

    mgr->register_creature_script(BOSS_DECATRIARCH_WRATHEYE, &BrackenhideHollowWratheyeAI::Create);
    mgr->register_creature_script(NPC_ROTBURST_TOTEM, &BrackenhideHollowTotemAI::CreateRotburst);

    mgr->register_creature_script(BOSS_RIRA_HACKCLAW, &BrackenhideHollowRiraHackclawAI::Create);
    mgr->register_creature_script(BOSS_GASHTOOTH, &BrackenhideHollowGashtoothAI::Create);
    mgr->register_creature_script(BOSS_TRICKTOTEM, &BrackenhideHollowTricktotemAI::Create);
    mgr->register_creature_script(NPC_HEXTRICK_TOTEM, &BrackenhideHollowTotemAI::CreateHextrick);
}
