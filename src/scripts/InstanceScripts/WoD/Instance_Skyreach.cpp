/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Skyreach.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <vector>

enum SkyreachEvents
{
    EVENT_RANJIT_SPINNING_BLADE = 1,
    EVENT_RANJIT_FAN_OF_BLADES,
    EVENT_RANJIT_WINDWALL,
    EVENT_RANJIT_PIERCING_RUSH,
    EVENT_RANJIT_FOUR_WINDS,

    EVENT_ARAKNATH_SMASH,
    EVENT_ARAKNATH_BURST,

    EVENT_RUKHRAN_PIERCE_ARMOR,
    EVENT_RUKHRAN_SCREECH_CHECK,
    EVENT_RUKHRAN_SOLAR_FLARE,
    EVENT_RUKHRAN_QUILLS,

    EVENT_SKYREACH_FLARE_CHASE,

    EVENT_VIRYX_SOLAR_BURST,
    EVENT_VIRYX_CAST_DOWN,
    EVENT_VIRYX_LENS_FLARE,
    EVENT_VIRYX_CALL_CONSTRUCT,

    EVENT_SKYREACH_CONSTRUCT_SHIELDING
};

class SkyreachInstanceScript : public InstanceScript
{
public:
    explicit SkyreachInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(SkyreachEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new SkyreachInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_SKYREACH_RANJIT_ENTRANCE:
            case GO_SKYREACH_RANJIT_EXIT:
            case GO_SKYREACH_ARAKNATH_ENTRANCE_1:
            case GO_SKYREACH_ARAKNATH_ENTRANCE_2:
            case GO_SKYREACH_ARAKNATH_EXIT_1:
            case GO_SKYREACH_ARAKNATH_EXIT_2:
            case GO_SKYREACH_RUKHRAN_ENTRANCE:
            case GO_SKYREACH_RUKHRAN_EXIT:
            case GO_SKYREACH_VIRYX_ENTRANCE:
                mDoorGuids.push_back(pGameObject->GetNewGUID());
                applyDoorState(pGameObject);
                break;
            default:
                break;
        }
    }

    void OnEncounterStateChange(uint32_t /*bossId*/, uint32_t /*state*/) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                applyDoorState(pDoor);
        }
    }

private:
    bool isDoorOpen(uint32_t entry) const
    {
        switch (entry)
        {
            case GO_SKYREACH_RANJIT_ENTRANCE:
                return getBossState(DATA_RANJIT) != InProgress;
            case GO_SKYREACH_RANJIT_EXIT:
                return getBossState(DATA_RANJIT) == Performed;
            case GO_SKYREACH_ARAKNATH_ENTRANCE_1:
            case GO_SKYREACH_ARAKNATH_ENTRANCE_2:
                return getBossState(DATA_ARAKNATH) != InProgress;
            case GO_SKYREACH_ARAKNATH_EXIT_1:
            case GO_SKYREACH_ARAKNATH_EXIT_2:
                return getBossState(DATA_ARAKNATH) == Performed;
            case GO_SKYREACH_RUKHRAN_ENTRANCE:
                return getBossState(DATA_RUKHRAN) != InProgress;
            case GO_SKYREACH_RUKHRAN_EXIT:
                return getBossState(DATA_RUKHRAN) == Performed;
            case GO_SKYREACH_VIRYX_ENTRANCE:
                return getBossState(DATA_VIRYX) != InProgress;
            default:
                return true;
        }
    }

    void applyDoorState(GameObject* pDoor)
    {
        pDoor->setState(isDoorOpen(pDoor->getEntry()) ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    std::vector<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ranjit

class SkyreachRanjitAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SkyreachRanjitAI(c); }
    explicit SkyreachRanjitAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_RANJIT, EncounterStates::InProgress);

        mWindwallCount = 0;
        scriptEvents.addEvent(EVENT_RANJIT_PIERCING_RUSH, 3000);
        scriptEvents.addEvent(EVENT_RANJIT_SPINNING_BLADE, 4000);
        scriptEvents.addEvent(EVENT_RANJIT_FAN_OF_BLADES, 6000);
        scriptEvents.addEvent(EVENT_RANJIT_WINDWALL, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RANJIT, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RANJIT, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_RANJIT_SPINNING_BLADE:
                castSpellAOE(SPELL_RANJIT_SPINNING_BLADE);
                scriptEvents.addEvent(EVENT_RANJIT_SPINNING_BLADE, 26000);
                break;
            case EVENT_RANJIT_FAN_OF_BLADES:
                castSpellAOE(SPELL_RANJIT_FAN_OF_BLADES);
                scriptEvents.addEvent(EVENT_RANJIT_FAN_OF_BLADES, 16000);
                break;
            case EVENT_RANJIT_WINDWALL:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_RANJIT_WINDWALL);

                // every third windwall is followed by the four winds
                if (++mWindwallCount >= 3)
                {
                    mWindwallCount = 0;
                    scriptEvents.addEvent(EVENT_RANJIT_FOUR_WINDS, 2500);
                }
                scriptEvents.addEvent(EVENT_RANJIT_WINDWALL, Util::getRandomUInt(19000, 25000));
                break;
            case EVENT_RANJIT_PIERCING_RUSH:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent))))
                    castSpell(pTarget, SPELL_RANJIT_PIERCING_RUSH);
                scriptEvents.addEvent(EVENT_RANJIT_PIERCING_RUSH, Util::getRandomUInt(13000, 16000));
                break;
            case EVENT_RANJIT_FOUR_WINDS:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Ranjit calls upon the Four Winds!");
                castSpellAOE(SPELL_RANJIT_FOUR_WINDS);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mWindwallCount = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Araknath

class SkyreachAraknathAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SkyreachAraknathAI(c); }
    explicit SkyreachAraknathAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ARAKNATH, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ARAKNATH_SMASH, 6000);
        scriptEvents.addEvent(EVENT_ARAKNATH_BURST, 22000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ARAKNATH, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ARAKNATH, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ARAKNATH_SMASH:
                castSpellOnVictim(SPELL_ARAKNATH_SMASH);
                scriptEvents.addEvent(EVENT_ARAKNATH_SMASH, Util::getRandomUInt(6000, 7500));
                break;
            case EVENT_ARAKNATH_BURST:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Araknath begins to Burst!");
                castSpellAOE(SPELL_ARAKNATH_BURST);
                scriptEvents.addEvent(EVENT_ARAKNATH_BURST, 23000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rukhran

class SkyreachRukhranAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SkyreachRukhranAI(c); }
    explicit SkyreachRukhranAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_RUKHRAN, EncounterStates::InProgress);

        setRooted(true);
        scriptEvents.addEvent(EVENT_RUKHRAN_SCREECH_CHECK, 2000);
        scriptEvents.addEvent(EVENT_RUKHRAN_PIERCE_ARMOR, 6000);
        scriptEvents.addEvent(EVENT_RUKHRAN_SOLAR_FLARE, 10000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_RUKHRAN_QUILLS, 25000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        setRooted(false);
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RUKHRAN, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RUKHRAN, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
    }

    // a slain flare collapses into a pile of ash that can be rekindled later
    void OnSummonDies(Creature* summon, Unit* /*killer*/) override
    {
        if (summon->getEntry() == NPC_SKYREACH_SOLAR_FLARE && isAlive() && _isInCombat())
            summonCreature(NPC_SKYREACH_PILE_OF_ASH, summon->GetPosition());
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_RUKHRAN_SCREECH_CHECK:
                if (selectUnitTarget(FilterArgs(TargetFilter(TargetFilter_Player | TargetFilter_InMeleeRange))) == nullptr)
                    castSpellAOE(SPELL_RUKHRAN_SCREECH);
                scriptEvents.addEvent(EVENT_RUKHRAN_SCREECH_CHECK, 2000);
                break;
            case EVENT_RUKHRAN_PIERCE_ARMOR:
                castSpellOnVictim(SPELL_RUKHRAN_PIERCE_ARMOR);
                scriptEvents.addEvent(EVENT_RUKHRAN_PIERCE_ARMOR, Util::getRandomUInt(10000, 13000));
                break;
            case EVENT_RUKHRAN_SOLAR_FLARE:
                spawnSolarFlare();
                scriptEvents.addEvent(EVENT_RUKHRAN_SOLAR_FLARE, Util::getRandomUInt(14000, 16000));
                break;
            case EVENT_RUKHRAN_QUILLS:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Rukhran begins to cast Quills! Take cover!");
                castSpellAOE(SPELL_RUKHRAN_QUILLS);
                scriptEvents.addEvent(EVENT_RUKHRAN_QUILLS, 25000);
                break;
            default:
                break;
        }
    }

private:
    void spawnSolarFlare()
    {
        std::list<Creature*> piles;
        GetCreatureListWithEntryInGrid(piles, NPC_SKYREACH_PILE_OF_ASH, 80.0f);

        std::vector<Creature*> dormant;
        for (Creature* pPile : piles)
        {
            if (pPile->isAlive())
                dormant.push_back(pPile);
        }

        // rekindle an old pile first, otherwise call a fresh flare next to the perch
        if (!dormant.empty())
        {
            Creature* pPile = Util::selectRandomContainerElement(dormant);
            LocationVector const spot = pPile->GetPosition();
            pPile->Despawn(100, 0);
            summonCreature(NPC_SKYREACH_SOLAR_FLARE, spot);
            return;
        }

        float const angle = Util::getRandomFloat(0.0f, 6.28f);
        summonCreature(NPC_SKYREACH_SOLAR_FLARE, getCreature()->GetPositionX() + 10.0f * std::cos(angle), getCreature()->GetPositionY() + 10.0f * std::sin(angle), getCreature()->GetPositionZ(), angle);
    }
};

// Solar Flare, fixates a player and explodes on contact
class SkyreachSolarFlareAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SkyreachSolarFlareAI(c); }
    explicit SkyreachSolarFlareAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* /*summoner*/) override
    {
        if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
        {
            castSpell(pTarget, SPELL_SOLAR_FLARE_FIXATE, true);
            addThreat(pTarget, 10000.0f);
            attackStart(pTarget);
        }
        scriptEvents.addEvent(EVENT_SKYREACH_FLARE_CHASE, 1000);
    }

    void DamageTaken(Unit* /*attacker*/, uint32_t* damage) override
    {
        if (!mBurst && *damage >= getCreature()->getHealth())
        {
            mBurst = true;
            castSpellAOE(SPELL_SOLAR_FLARE_SUNBURST, true);
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (scriptEvents.getFinishedEvent() != EVENT_SKYREACH_FLARE_CHASE)
            return;

        Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
        if (pVictim != nullptr && getRangeToObject(pVictim) <= 4.0f)
        {
            castSpellAOE(SPELL_SOLAR_FLARE_SUNSTRIKE, true);
            despawn(500, 0);
            return;
        }

        scriptEvents.addEvent(EVENT_SKYREACH_FLARE_CHASE, 500);
    }

private:
    bool mBurst = false;
};

// Pile of Ash, dormant remains of a slain flare
class SkyreachPileOfAshAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SkyreachPileOfAshAI(c); }
    explicit SkyreachPileOfAshAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* /*summoner*/) override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        castSpellOnSelf(SPELL_PILE_OF_ASH_DORMANT, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// High Sage Viryx

class SkyreachViryxAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SkyreachViryxAI(c); }
    explicit SkyreachViryxAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_VIRYX, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_VIRYX_SOLAR_BURST, 8000);
        scriptEvents.addEvent(EVENT_VIRYX_CAST_DOWN, 16000);
        scriptEvents.addEvent(EVENT_VIRYX_LENS_FLARE, 26000);
        scriptEvents.addEvent(EVENT_VIRYX_CALL_CONSTRUCT, 32000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_VIRYX, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_VIRYX, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        if (summon->getEntry() == NPC_SKYREACH_SHIELD_CONSTRUCT)
            setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_VIRYX_SOLAR_BURST:
                castSpellOnVictim(SPELL_VIRYX_SOLAR_BURST);
                scriptEvents.addEvent(EVENT_VIRYX_SOLAR_BURST, 12000);
                break;
            case EVENT_VIRYX_CAST_DOWN:
            {
                Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent)));
                if (pTarget == nullptr)
                    pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player));

                if (pTarget != nullptr)
                {
                    sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "High Sage Viryx calls down a Solar Zealot!");
                    summonCreature(NPC_SKYREACH_SOLAR_ZEALOT, pTarget->GetPositionX() + 2.0f, pTarget->GetPositionY(), pTarget->GetPositionZ(), pTarget->GetOrientation());
                }
                scriptEvents.addEvent(EVENT_VIRYX_CAST_DOWN, 36000);
            } break;
            case EVENT_VIRYX_LENS_FLARE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                {
                    sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "High Sage Viryx focuses a Lens Flare!");
                    castSpell(pTarget, SPELL_VIRYX_LENS_FLARE);
                }
                scriptEvents.addEvent(EVENT_VIRYX_LENS_FLARE, 36000);
                break;
            case EVENT_VIRYX_CALL_CONSTRUCT:
                summonCreature(NPC_SKYREACH_SHIELD_CONSTRUCT, getCreature()->GetPositionX() + 5.0f, getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
                scriptEvents.addEvent(EVENT_VIRYX_CALL_CONSTRUCT, 60000);
                break;
            default:
                break;
        }
    }
};

// Solar Zealot, grabs the nearest player
class SkyreachSolarZealotAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SkyreachSolarZealotAI(c); }
    explicit SkyreachSolarZealotAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* /*summoner*/) override
    {
        // force the hostile monster faction
        getCreature()->setFaction(16);

        if (Player* pPlayer = getNearestPlayer())
        {
            addThreat(pPlayer, 1000.0f);
            attackStart(pPlayer);
        }
    }
};

// Skyreach Shield Construct, shields its allies
class SkyreachShieldConstructAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SkyreachShieldConstructAI(c); }
    explicit SkyreachShieldConstructAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SKYREACH_CONSTRUCT_SHIELDING, 4000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_SKYREACH_CONSTRUCT_SHIELDING)
        {
            castSpellAOE(SPELL_SHIELD_CONSTRUCT_SHIELDING);
            scriptEvents.addEvent(EVENT_SKYREACH_CONSTRUCT_SHIELDING, 15000);
        }
    }
};

void SetupSkyreach(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SKYREACH, &SkyreachInstanceScript::Create);

    mgr->register_creature_script(BOSS_RANJIT, &SkyreachRanjitAI::Create);
    mgr->register_creature_script(BOSS_ARAKNATH, &SkyreachAraknathAI::Create);
    mgr->register_creature_script(BOSS_RUKHRAN, &SkyreachRukhranAI::Create);
    mgr->register_creature_script(BOSS_HIGH_SAGE_VIRYX, &SkyreachViryxAI::Create);

    mgr->register_creature_script(NPC_SKYREACH_SOLAR_FLARE, &SkyreachSolarFlareAI::Create);
    mgr->register_creature_script(NPC_SKYREACH_PILE_OF_ASH, &SkyreachPileOfAshAI::Create);
    mgr->register_creature_script(NPC_SKYREACH_SOLAR_ZEALOT, &SkyreachSolarZealotAI::Create);
    mgr->register_creature_script(NPC_SKYREACH_SHIELD_CONSTRUCT, &SkyreachShieldConstructAI::Create);
}
