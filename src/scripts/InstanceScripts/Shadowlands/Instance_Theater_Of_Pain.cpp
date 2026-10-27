/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Theater_Of_Pain.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <map>
#include <vector>

enum TheaterOfPainEvents
{
    EVENT_KULTHAROK_NECROTIC_BOLT = 1,
    EVENT_KULTHAROK_NECROTIC_ERUPTION,
    EVENT_KULTHAROK_WELL_OF_DARKNESS,
    EVENT_KULTHAROK_DRAW_SOUL,
    EVENT_KULTHAROK_DEATH_SPIRAL,
    EVENT_KULTHAROK_FEAST_OF_THE_DAMNED,

    EVENT_GORECHOP_HATEFUL_STRIKE,
    EVENT_GORECHOP_TENDERIZING_SMASH,
    EVENT_GORECHOP_MEAT_HOOKS,
    EVENT_LEFTOVERS_LEAPING_THRASH,

    EVENT_XAV_BRUTAL_COMBO,
    EVENT_XAV_OPPRESSIVE_BANNER,
    EVENT_XAV_MIGHT_OF_MALDRAXXUS,
    EVENT_XAV_MIGHT_FOLLOW_UP,

    EVENT_CHALLENGER_FIRST_ABILITY,
    EVENT_CHALLENGER_SECOND_ABILITY,

    EVENT_MORDRETHA_REAPING_SCYTHE,
    EVENT_MORDRETHA_DARK_DEVASTATION,
    EVENT_MORDRETHA_MANIFEST_DEATH,
    EVENT_MORDRETHA_GRASPING_RIFT
};

namespace
{
    // members of the encounters fought as one
    struct TheaterGroupSlot
    {
        uint32_t dataId;
        uint32_t entry;
    };

    TheaterGroupSlot const TheaterGroupSlots[] =
    {
        { DATA_AFFRONT_OF_CHALLENGERS, BOSS_DESSIA_THE_DECAPITATOR },
        { DATA_AFFRONT_OF_CHALLENGERS, BOSS_PACERAN_THE_VIRULENT },
        { DATA_AFFRONT_OF_CHALLENGERS, BOSS_SATHEL_THE_ACCURSED }
    };

    float const TheaterTwoPi = 6.2831853f;

    // Gorechop
    uint32_t const LeftoversPerHookWave = 2;

    // Xav, avoidable strikes after Might of Maldraxxus
    uint32_t const XavMightFollowUps[] = { SPELL_XAV_DEAFENING_CRASH, SPELL_XAV_MASSIVE_CLEAVE, SPELL_XAV_CRUSHING_SLAM };
    uint32_t const XavMightFollowUpCount = 3;

    // Mordretha
    int32_t const MordrethaEchoesPct = 50;

    // how a challenger aims a spell
    enum class ChallengerAim : uint8_t
    {
        Victim,
        RandomPlayer,
        Area
    };

    // what each challenger casts, the first spell is the one released at full energy
    struct ChallengerKit
    {
        uint32_t energySpell;
        ChallengerAim energyAim;
        char const* energyText;
        uint32_t startEnergy;
        uint32_t firstSpell;
        ChallengerAim firstAim;
        uint32_t firstMs;
        uint32_t secondSpell;
        ChallengerAim secondAim;
        uint32_t secondMs;
    };

    ChallengerKit const DessiaKit =
    {
        SPELL_DESSIA_MORTAL_STRIKE, ChallengerAim::Victim, "Dessia the Decapitator readies a Mortal Strike!", 0,
        SPELL_DESSIA_MIGHTY_SMASH, ChallengerAim::Area, 14000,
        0, ChallengerAim::Area, 0
    };

    ChallengerKit const PaceranKit =
    {
        SPELL_PACERAN_NOXIOUS_SPORES, ChallengerAim::RandomPlayer, "Paceran the Virulent releases Noxious Spores!", 35,
        SPELL_PACERAN_DECAYING_BREATH, ChallengerAim::Victim, 16000,
        0, ChallengerAim::Area, 0
    };

    ChallengerKit const SathelKit =
    {
        SPELL_SATHEL_SEARING_DEATH, ChallengerAim::RandomPlayer, "Sathel the Accursed calls down Searing Death!", 70,
        SPELL_SATHEL_NECROMANTIC_BOLT, ChallengerAim::Victim, 3500,
        SPELL_SATHEL_WITHERING_TOUCH, ChallengerAim::Area, 18000
    };

    uint32_t const ChallengerEnergyFull = 100;
    uint32_t const ChallengerEnergyPerSecond = 5;

    LocationVector theaterPointAround(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(TheaterTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class TheaterOfPainInstanceScript : public InstanceScript
{
public:
    explicit TheaterOfPainInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TheaterOfPainEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheaterOfPainInstanceScript(pMapMgr); }

    // a fallen challenger must not end the encounter while the others stand
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (TheaterGroupSlot const& slot : TheaterGroupSlots)
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
        for (TheaterGroupSlot const& slot : TheaterGroupSlots)
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

    // tells every other living member of the group
    void broadcastAction(uint32_t dataId, Creature* pSender, int32_t action)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pSender && pMember->isAlive() && pMember->GetScript() != nullptr)
                pMember->GetScript()->DoAction(action);
        }
    }

private:
    std::map<uint32_t, WoWGuid> mMemberGuids;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class TheaterOfPainCombatAI : public CreatureAIScript
{
public:
    explicit TheaterOfPainCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    TheaterOfPainInstanceScript* getTheater()
    {
        if (getCreature()->GetMapId() != MAP_THEATER_OF_PAIN)
            return nullptr;

        return static_cast<TheaterOfPainInstanceScript*>(getInstanceScript());
    }

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    void castAt(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class TheaterOfPainBossAI : public TheaterOfPainCombatAI
{
public:
    TheaterOfPainBossAI(Creature* pCreature, uint32_t dataId) : TheaterOfPainCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (TheaterOfPainInstanceScript* pTheater = getTheater())
        {
            pTheater->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pTheater->setBossState(mDataId, EncounterStates::InProgress);
            pTheater->rallyGroup(mDataId, getCreature());
        }

        TheaterOfPainCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (TheaterOfPainInstanceScript* pTheater = getTheater())
            {
                pTheater->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last member has left the fight
                if (!pTheater->isGroupFighting(mDataId, getCreature()))
                    pTheater->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        TheaterOfPainCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        // the instance holds the credit back while a member stands
        if (TheaterOfPainInstanceScript* pTheater = getTheater())
        {
            pTheater->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pTheater->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kul'tharok

class TheaterOfPainKultharokAI : public TheaterOfPainBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheaterOfPainKultharokAI(c); }
    explicit TheaterOfPainKultharokAI(Creature* pCreature) : TheaterOfPainBossAI(pCreature, DATA_KULTHAROK) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_KULTHAROK_NECROTIC_BOLT, 3000);
        scriptEvents.addEvent(EVENT_KULTHAROK_WELL_OF_DARKNESS, 9000);
        scriptEvents.addEvent(EVENT_KULTHAROK_NECROTIC_ERUPTION, 14000);
        scriptEvents.addEvent(EVENT_KULTHAROK_DEATH_SPIRAL, 18000);
        scriptEvents.addEvent(EVENT_KULTHAROK_DRAW_SOUL, 24000);
        scriptEvents.addEvent(EVENT_KULTHAROK_FEAST_OF_THE_DAMNED, 40000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KULTHAROK_NECROTIC_BOLT:
                castSpellOnVictim(SPELL_KULTHAROK_NECROTIC_BOLT);
                scriptEvents.addEvent(EVENT_KULTHAROK_NECROTIC_BOLT, 5000);
                break;
            case EVENT_KULTHAROK_WELL_OF_DARKNESS:
                castAt(randomPlayer(), SPELL_KULTHAROK_WELL_OF_DARKNESS);
                scriptEvents.addEvent(EVENT_KULTHAROK_WELL_OF_DARKNESS, 24000);
                break;
            case EVENT_KULTHAROK_NECROTIC_ERUPTION:
                castSpellOnVictim(SPELL_KULTHAROK_NECROTIC_ERUPTION);
                scriptEvents.addEvent(EVENT_KULTHAROK_NECROTIC_ERUPTION, 20000);
                break;
            case EVENT_KULTHAROK_DEATH_SPIRAL:
                castSpellOnSelf(SPELL_KULTHAROK_DEATH_SPIRAL);
                scriptEvents.addEvent(EVENT_KULTHAROK_DEATH_SPIRAL, 26000);
                break;
            case EVENT_KULTHAROK_DRAW_SOUL:
                warn("Kul'tharok begins to Draw Soul!");
                castSpellAOE(SPELL_KULTHAROK_DRAW_SOUL);
                scriptEvents.addEvent(EVENT_KULTHAROK_DRAW_SOUL, 30000);
                break;
            case EVENT_KULTHAROK_FEAST_OF_THE_DAMNED:
                warn("Kul'tharok begins the Feast of the Damned!");
                castSpellOnSelf(SPELL_KULTHAROK_FEAST_OF_THE_DAMNED);
                scriptEvents.addEvent(EVENT_KULTHAROK_FEAST_OF_THE_DAMNED, 45000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gorechop

class TheaterOfPainGorechopAI : public TheaterOfPainBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheaterOfPainGorechopAI(c); }
    explicit TheaterOfPainGorechopAI(Creature* pCreature) : TheaterOfPainBossAI(pCreature, DATA_GORECHOP) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GORECHOP_HATEFUL_STRIKE, 6000);
        scriptEvents.addEvent(EVENT_GORECHOP_MEAT_HOOKS, 10000);
        scriptEvents.addEvent(EVENT_GORECHOP_TENDERIZING_SMASH, 16000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GORECHOP_HATEFUL_STRIKE:
                castSpellOnVictim(SPELL_GORECHOP_HATEFUL_STRIKE);
                scriptEvents.addEvent(EVENT_GORECHOP_HATEFUL_STRIKE, 10000);
                break;
            case EVENT_GORECHOP_TENDERIZING_SMASH:
                castSpellAOE(SPELL_GORECHOP_TENDERIZING_SMASH);
                scriptEvents.addEvent(EVENT_GORECHOP_TENDERIZING_SMASH, 18000);
                break;
            case EVENT_GORECHOP_MEAT_HOOKS:
                warn("Gorechop calls forth his Meat Hooks!");
                castSpellOnSelf(SPELL_GORECHOP_MEAT_HOOKS);

                // some hooks carry leftovers into the arena
                for (uint32_t i = 0; i < LeftoversPerHookWave; ++i)
                    callAdd(NPC_OOZING_LEFTOVERS, theaterPointAround(getCreature()->GetPosition(), 10.0f, 18.0f));

                scriptEvents.addEvent(EVENT_GORECHOP_MEAT_HOOKS, 20000);
                break;
            default:
                break;
        }
    }
};

class TheaterOfPainOozingLeftoversAI : public TheaterOfPainCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheaterOfPainOozingLeftoversAI(c); }
    explicit TheaterOfPainOozingLeftoversAI(Creature* pCreature) : TheaterOfPainCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_LEFTOVERS_OOZING, true);
        scriptEvents.addEvent(EVENT_LEFTOVERS_LEAPING_THRASH, 4000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_LEFTOVERS_LEAPING_THRASH)
            return;

        castAt(randomPlayer(), SPELL_LEFTOVERS_LEAPING_THRASH);
        scriptEvents.addEvent(EVENT_LEFTOVERS_LEAPING_THRASH, 12000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Xav the Unfallen

class TheaterOfPainXavAI : public TheaterOfPainBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheaterOfPainXavAI(c); }
    explicit TheaterOfPainXavAI(Creature* pCreature) : TheaterOfPainBossAI(pCreature, DATA_XAV_THE_UNFALLEN) {}

protected:
    void onEngage() override
    {
        mFollowUp = 0;
        scriptEvents.addEvent(EVENT_XAV_BRUTAL_COMBO, 8000);
        scriptEvents.addEvent(EVENT_XAV_OPPRESSIVE_BANNER, 14000);
        scriptEvents.addEvent(EVENT_XAV_MIGHT_OF_MALDRAXXUS, 25000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_XAV_BRUTAL_COMBO:
                castSpellOnSelf(SPELL_XAV_BRUTAL_COMBO);
                scriptEvents.addEvent(EVENT_XAV_BRUTAL_COMBO, 16000);
                break;
            case EVENT_XAV_OPPRESSIVE_BANNER:
                castSpellAOE(SPELL_XAV_OPPRESSIVE_BANNER);
                scriptEvents.addEvent(EVENT_XAV_OPPRESSIVE_BANNER, 25000);
                break;
            case EVENT_XAV_MIGHT_OF_MALDRAXXUS:
                warn("Xav the Unfallen invokes the Might of Maldraxxus!");
                castAt(randomPlayer(), SPELL_XAV_MIGHT_OF_MALDRAXXUS);
                mFollowUp = 0;
                scriptEvents.addEvent(EVENT_XAV_MIGHT_FOLLOW_UP, 3000);
                scriptEvents.addEvent(EVENT_XAV_MIGHT_OF_MALDRAXXUS, 40000);
                break;
            case EVENT_XAV_MIGHT_FOLLOW_UP:
                // the strikes come one after another
                if (mFollowUp < XavMightFollowUpCount)
                {
                    uint32_t const spellId = XavMightFollowUps[mFollowUp++];
                    if (spellId == SPELL_XAV_DEAFENING_CRASH)
                        castSpellAOE(spellId);
                    else
                        castSpellOnVictim(spellId);

                    if (mFollowUp < XavMightFollowUpCount)
                        scriptEvents.addEvent(EVENT_XAV_MIGHT_FOLLOW_UP, 2500);
                }
                break;
            default:
                break;
        }
    }

private:
    uint32_t mFollowUp = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// An Affront of Challengers

class TheaterOfPainChallengerAI : public TheaterOfPainBossAI
{
public:
    static CreatureAIScript* CreateDessia(Creature* c) { return new TheaterOfPainChallengerAI(c, DessiaKit); }
    static CreatureAIScript* CreatePaceran(Creature* c) { return new TheaterOfPainChallengerAI(c, PaceranKit); }
    static CreatureAIScript* CreateSathel(Creature* c) { return new TheaterOfPainChallengerAI(c, SathelKit); }

    TheaterOfPainChallengerAI(Creature* pCreature, ChallengerKit const& kit) : TheaterOfPainBossAI(pCreature, DATA_AFFRONT_OF_CHALLENGERS), mKit(kit) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_THEATER_CHALLENGER_FALLEN || !isAlive() || !_isInCombat())
            return;

        // the fallen hand their will to the survivors
        castSpellOnSelf(SPELL_CHALLENGER_FINAL_WILL, true);
    }

protected:
    void onEngage() override
    {
        mEnergy = mKit.startEnergy;
        mEnergyClock = 0;

        scriptEvents.addEvent(EVENT_CHALLENGER_FIRST_ABILITY, mKit.firstMs / 2);
        if (mKit.secondSpell != 0)
            scriptEvents.addEvent(EVENT_CHALLENGER_SECOND_ABILITY, mKit.secondMs / 2);
    }

    void onReset() override { _removeAura(SPELL_CHALLENGER_FINAL_WILL); }

    void onDefeat() override
    {
        if (TheaterOfPainInstanceScript* pTheater = getTheater())
            pTheater->broadcastAction(DATA_AFFRONT_OF_CHALLENGERS, getCreature(), ACTION_THEATER_CHALLENGER_FALLEN);
    }

    void onCombatTick(uint32_t diff) override
    {
        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy += ChallengerEnergyPerSecond;
        }

        if (mEnergy < ChallengerEnergyFull || _isCasting())
            return;

        mEnergy = 0;
        warn(mKit.energyText);
        aimSpell(mKit.energySpell, mKit.energyAim);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CHALLENGER_FIRST_ABILITY:
                aimSpell(mKit.firstSpell, mKit.firstAim);
                scriptEvents.addEvent(EVENT_CHALLENGER_FIRST_ABILITY, mKit.firstMs);
                break;
            case EVENT_CHALLENGER_SECOND_ABILITY:
                aimSpell(mKit.secondSpell, mKit.secondAim);
                scriptEvents.addEvent(EVENT_CHALLENGER_SECOND_ABILITY, mKit.secondMs);
                break;
            default:
                break;
        }
    }

private:
    void aimSpell(uint32_t spellId, ChallengerAim aim)
    {
        switch (aim)
        {
            case ChallengerAim::Victim:
                castSpellOnVictim(spellId);
                break;
            case ChallengerAim::RandomPlayer:
                castAt(randomPlayer(), spellId);
                break;
            case ChallengerAim::Area:
                castSpellAOE(spellId);
                break;
        }
    }

    ChallengerKit const& mKit;
    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mordretha, the Endless Empress

class TheaterOfPainMordrethaAI : public TheaterOfPainBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheaterOfPainMordrethaAI(c); }
    explicit TheaterOfPainMordrethaAI(Creature* pCreature) : TheaterOfPainBossAI(pCreature, DATA_MORDRETHA) {}

protected:
    void onEngage() override
    {
        mEchoesDone = false;
        scriptEvents.addEvent(EVENT_MORDRETHA_REAPING_SCYTHE, 6000);
        scriptEvents.addEvent(EVENT_MORDRETHA_GRASPING_RIFT, 11000);
        scriptEvents.addEvent(EVENT_MORDRETHA_DARK_DEVASTATION, 15000);
        scriptEvents.addEvent(EVENT_MORDRETHA_MANIFEST_DEATH, 20000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mEchoesDone || _isCasting() || _getHealthPercent() > MordrethaEchoesPct)
            return;

        mEchoesDone = true;
        warn("Mordretha calls upon the Echoes of Carnage!");
        castSpellOnSelf(SPELL_MORDRETHA_ECHOES_OF_CARNAGE);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MORDRETHA_REAPING_SCYTHE:
                castSpellOnVictim(SPELL_MORDRETHA_REAPING_SCYTHE);
                scriptEvents.addEvent(EVENT_MORDRETHA_REAPING_SCYTHE, 12000);
                break;
            case EVENT_MORDRETHA_GRASPING_RIFT:
                castAt(randomPlayer(), SPELL_MORDRETHA_GRASPING_RIFT);
                scriptEvents.addEvent(EVENT_MORDRETHA_GRASPING_RIFT, 26000);
                break;
            case EVENT_MORDRETHA_DARK_DEVASTATION:
                castAt(randomPlayer(), SPELL_MORDRETHA_DARK_DEVASTATION);
                scriptEvents.addEvent(EVENT_MORDRETHA_DARK_DEVASTATION, 22000);
                break;
            case EVENT_MORDRETHA_MANIFEST_DEATH:
                castSpellAOE(SPELL_MORDRETHA_MANIFEST_DEATH);
                scriptEvents.addEvent(EVENT_MORDRETHA_MANIFEST_DEATH, 30000);
                break;
            default:
                break;
        }
    }

private:
    bool mEchoesDone = false;
};

void SetupTheaterOfPain(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THEATER_OF_PAIN, &TheaterOfPainInstanceScript::Create);

    mgr->register_creature_script(BOSS_KULTHAROK, &TheaterOfPainKultharokAI::Create);

    mgr->register_creature_script(BOSS_GORECHOP, &TheaterOfPainGorechopAI::Create);
    mgr->register_creature_script(NPC_OOZING_LEFTOVERS, &TheaterOfPainOozingLeftoversAI::Create);

    mgr->register_creature_script(BOSS_XAV_THE_UNFALLEN, &TheaterOfPainXavAI::Create);

    mgr->register_creature_script(BOSS_DESSIA_THE_DECAPITATOR, &TheaterOfPainChallengerAI::CreateDessia);
    mgr->register_creature_script(BOSS_PACERAN_THE_VIRULENT, &TheaterOfPainChallengerAI::CreatePaceran);
    mgr->register_creature_script(BOSS_SATHEL_THE_ACCURSED, &TheaterOfPainChallengerAI::CreateSathel);

    mgr->register_creature_script(BOSS_MORDRETHA, &TheaterOfPainMordrethaAI::Create);
}
