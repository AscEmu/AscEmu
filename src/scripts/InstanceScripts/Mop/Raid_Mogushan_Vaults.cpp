/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Mogu'shan Vaults - six boss-state slots covering all seven named bosses (see header comment
// for sourcing/limitations), hand-ported into AscEmu's own CreatureAIScript / InstanceScript
// framework.

#include "Setup.h"
#include "Raid_Mogushan_Vaults.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum MogushanVaultsEvents
{
    EVENT_JASPER_CHAINS = 1,
    EVENT_JASPER_PETRIFICATION,
    EVENT_AMETHYST_POOL,
    EVENT_AMETHYST_PETRIFICATION,
    EVENT_JADE_SHARDS,
    EVENT_JADE_PETRIFICATION,
    EVENT_COBALT_MINE,
    EVENT_COBALT_PETRIFICATION,

    EVENT_FENG_SPIRIT_BOLT,
    EVENT_FENG_LIGHTNING_LASH,
    EVENT_FENG_FLAMING_SPEAR,
    EVENT_FENG_ARCANE_SHOCK,
    EVENT_FENG_SHADOWBURN,

    EVENT_GARAJAL_SHADOW_BOLT,
    EVENT_GARAJAL_VOODOO_DOLLS,
    EVENT_GARAJAL_SPIRITUAL_GRASP,
    EVENT_GARAJAL_SOUL_SEVER,

    EVENT_QIANG_MASSIVE_ATTACKS,
    EVENT_SUBETAI_RAIN_OF_ARROWS,
    EVENT_ZIAN_ROBBED_BLIND,
    EVENT_MENG_SHADOW_BLAST,

    EVENT_ELEGON_CELESTIAL_BREATH,
    EVENT_ELEGON_ARCING_ENERGY,
    EVENT_ELEGON_CLOSED_CIRCUIT,
    EVENT_ELEGON_STABILITY_FLUX,

    EVENT_EMPEROR_FOCUSED_ASSAULT,
    EVENT_EMPEROR_ENERGIZING_SMASH,
    EVENT_EMPEROR_DEVASTATING_ARC,
    EVENT_EMPEROR_STOMP,

    EVENT_MOGU_AMBUSHER_CLEAVE,
    EVENT_MOGU_AMBUSHER_ENRAGE
};

class MogushanVaultsInstanceScript : public InstanceScript
{
public:
    explicit MogushanVaultsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(MogushanVaultsEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new MogushanVaultsInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() == GO_GOLDEN_DOORS)
            mGoldenDoorGuids.push_back(pGameObject->getGuidLow());
    }

    void OpenGoldenDoors()
    {
        for (uint32_t guid : mGoldenDoorGuids)
        {
            if (GameObject* pDoor = GetGameObjectByGuid(guid))
                useDoorOrButton(pDoor);
        }
    }

private:
    std::vector<uint32_t> mGoldenDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Stone Guard - four elemental guardian statues sharing one boss-state slot.

class StoneGuardMemberAI : public CreatureAIScript
{
public:
    explicit StoneGuardMemberAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_STONE_GUARD, EncounterStates::InProgress);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_STONE_GUARD, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_STONE_GUARD, EncounterStates::Performed);
        static_cast<MogushanVaultsInstanceScript*>(getInstanceScript())->OpenGoldenDoors();
    }
};

class JasperGuardianAI : public StoneGuardMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new JasperGuardianAI(c); }
    explicit JasperGuardianAI(Creature* pCreature) : StoneGuardMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        StoneGuardMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_JASPER_CHAINS, 8000);
        scriptEvents.addEvent(EVENT_JASPER_PETRIFICATION, 16000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_JASPER_CHAINS:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_JASPER_CHAINS);
                scriptEvents.addEvent(EVENT_JASPER_CHAINS, 20000);
                break;
            case EVENT_JASPER_PETRIFICATION:
                castSpellOnVictim(SPELL_JASPER_PETRIFICATION);
                scriptEvents.addEvent(EVENT_JASPER_PETRIFICATION, 24000);
                break;
            default:
                break;
        }
    }
};

class AmethystGuardianAI : public StoneGuardMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmethystGuardianAI(c); }
    explicit AmethystGuardianAI(Creature* pCreature) : StoneGuardMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        StoneGuardMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_AMETHYST_POOL, 10000);
        scriptEvents.addEvent(EVENT_AMETHYST_PETRIFICATION, 18000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_AMETHYST_POOL:
                castSpellAOE(SPELL_AMETHYST_POOL);
                scriptEvents.addEvent(EVENT_AMETHYST_POOL, 22000);
                break;
            case EVENT_AMETHYST_PETRIFICATION:
                castSpellOnVictim(SPELL_AMETHYST_PETRIFICATION);
                scriptEvents.addEvent(EVENT_AMETHYST_PETRIFICATION, 24000);
                break;
            default:
                break;
        }
    }
};

class JadeGuardianAI : public StoneGuardMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new JadeGuardianAI(c); }
    explicit JadeGuardianAI(Creature* pCreature) : StoneGuardMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        StoneGuardMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_JADE_SHARDS, 6000);
        scriptEvents.addEvent(EVENT_JADE_PETRIFICATION, 20000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_JADE_SHARDS:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_JADE_SHARDS);
                scriptEvents.addEvent(EVENT_JADE_SHARDS, 18000);
                break;
            case EVENT_JADE_PETRIFICATION:
                castSpellOnVictim(SPELL_JADE_PETRIFICATION);
                scriptEvents.addEvent(EVENT_JADE_PETRIFICATION, 24000);
                break;
            default:
                break;
        }
    }
};

class CobaltGuardianAI : public StoneGuardMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CobaltGuardianAI(c); }
    explicit CobaltGuardianAI(Creature* pCreature) : StoneGuardMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        StoneGuardMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_COBALT_MINE, 9000);
        scriptEvents.addEvent(EVENT_COBALT_PETRIFICATION, 17000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_COBALT_MINE:
                castSpellAOE(SPELL_COBALT_MINE);
                scriptEvents.addEvent(EVENT_COBALT_MINE, 21000);
                break;
            case EVENT_COBALT_PETRIFICATION:
                castSpellOnVictim(SPELL_COBALT_PETRIFICATION);
                scriptEvents.addEvent(EVENT_COBALT_PETRIFICATION, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Feng the Accursed

class FengTheAccursedAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FengTheAccursedAI(c); }
    explicit FengTheAccursedAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_FENG_THE_ACCURSED, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_FENG_SPIRIT_BOLT, 4000);
        scriptEvents.addEvent(EVENT_FENG_LIGHTNING_LASH, 10000);
        scriptEvents.addEvent(EVENT_FENG_FLAMING_SPEAR, 16000);
        scriptEvents.addEvent(EVENT_FENG_ARCANE_SHOCK, 22000);
        scriptEvents.addEvent(EVENT_FENG_SHADOWBURN, 28000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_FENG_THE_ACCURSED, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_FENG_THE_ACCURSED, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_FENG_SPIRIT_BOLT:
                castSpellOnVictim(SPELL_FENG_SPIRIT_BOLT);
                scriptEvents.addEvent(EVENT_FENG_SPIRIT_BOLT, 12000);
                break;
            case EVENT_FENG_LIGHTNING_LASH:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_FENG_LIGHTNING_LASH);
                scriptEvents.addEvent(EVENT_FENG_LIGHTNING_LASH, 20000);
                break;
            case EVENT_FENG_FLAMING_SPEAR:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_FENG_FLAMING_SPEAR);
                scriptEvents.addEvent(EVENT_FENG_FLAMING_SPEAR, 22000);
                break;
            case EVENT_FENG_ARCANE_SHOCK:
                castSpellAOE(SPELL_FENG_ARCANE_SHOCK);
                scriptEvents.addEvent(EVENT_FENG_ARCANE_SHOCK, 24000);
                break;
            case EVENT_FENG_SHADOWBURN:
                castSpellOnVictim(SPELL_FENG_SHADOWBURN);
                scriptEvents.addEvent(EVENT_FENG_SHADOWBURN, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gara'jal the Spiritbinder

class GarajalAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GarajalAI(c); }
    explicit GarajalAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_GARAJAL, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_GARAJAL_SHADOW_BOLT, 5000);
        scriptEvents.addEvent(EVENT_GARAJAL_VOODOO_DOLLS, 14000);
        scriptEvents.addEvent(EVENT_GARAJAL_SPIRITUAL_GRASP, 20000);
        scriptEvents.addEvent(EVENT_GARAJAL_SOUL_SEVER, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GARAJAL, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GARAJAL, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GARAJAL_SHADOW_BOLT:
                castSpellOnVictim(SPELL_GARAJAL_SHADOW_BOLT);
                scriptEvents.addEvent(EVENT_GARAJAL_SHADOW_BOLT, 12000);
                break;
            case EVENT_GARAJAL_VOODOO_DOLLS:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_GARAJAL_VOODOO_DOLLS);
                scriptEvents.addEvent(EVENT_GARAJAL_VOODOO_DOLLS, 24000);
                break;
            case EVENT_GARAJAL_SPIRITUAL_GRASP:
                castSpellOnVictim(SPELL_GARAJAL_SPIRITUAL_GRASP);
                scriptEvents.addEvent(EVENT_GARAJAL_SPIRITUAL_GRASP, 22000);
                break;
            case EVENT_GARAJAL_SOUL_SEVER:
                castSpellAOE(SPELL_GARAJAL_SOUL_SEVER);
                scriptEvents.addEvent(EVENT_GARAJAL_SOUL_SEVER, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Spirit Kings - four ghostly mogu rulers sharing one boss-state slot.

class SpiritKingMemberAI : public CreatureAIScript
{
public:
    explicit SpiritKingMemberAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SPIRIT_KINGS, EncounterStates::InProgress);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SPIRIT_KINGS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SPIRIT_KINGS, EncounterStates::Performed);
    }
};

class ZianAI : public SpiritKingMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ZianAI(c); }
    explicit ZianAI(Creature* pCreature) : SpiritKingMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        SpiritKingMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_ZIAN_ROBBED_BLIND, 6000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_ZIAN_ROBBED_BLIND)
        {
            castSpellOnVictim(SPELL_ZIAN_ROBBED_BLIND);
            scriptEvents.addEvent(EVENT_ZIAN_ROBBED_BLIND, 14000);
        }
    }
};

class MengAI : public SpiritKingMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MengAI(c); }
    explicit MengAI(Creature* pCreature) : SpiritKingMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        SpiritKingMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_MENG_SHADOW_BLAST, 8000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_MENG_SHADOW_BLAST)
        {
            castSpellOnVictim(SPELL_MENG_SHADOW_BLAST);
            scriptEvents.addEvent(EVENT_MENG_SHADOW_BLAST, 16000);
        }
    }
};

class QiangAI : public SpiritKingMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new QiangAI(c); }
    explicit QiangAI(Creature* pCreature) : SpiritKingMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        SpiritKingMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_QIANG_MASSIVE_ATTACKS, 12000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_QIANG_MASSIVE_ATTACKS)
        {
            castSpellAOE(SPELL_QIANG_MASSIVE_ATTACKS);
            scriptEvents.addEvent(EVENT_QIANG_MASSIVE_ATTACKS, 22000);
        }
    }
};

class SubetaiAI : public SpiritKingMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SubetaiAI(c); }
    explicit SubetaiAI(Creature* pCreature) : SpiritKingMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        SpiritKingMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_SUBETAI_RAIN_OF_ARROWS, 5000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_SUBETAI_RAIN_OF_ARROWS)
        {
            castSpellAOE(SPELL_SUBETAI_RAIN_OF_ARROWS);
            scriptEvents.addEvent(EVENT_SUBETAI_RAIN_OF_ARROWS, 18000);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Elegon

class ElegonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ElegonAI(c); }
    explicit ElegonAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "You dare disturb my slumber?");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ELEGON, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ELEGON_CELESTIAL_BREATH, 8000);
        scriptEvents.addEvent(EVENT_ELEGON_ARCING_ENERGY, 5000);
        scriptEvents.addEvent(EVENT_ELEGON_CLOSED_CIRCUIT, 16000);
        scriptEvents.addEvent(EVENT_ELEGON_STABILITY_FLUX, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ELEGON, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ELEGON, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ELEGON_CELESTIAL_BREATH:
                castSpellAOE(SPELL_ELEGON_CELESTIAL_BREATH);
                scriptEvents.addEvent(EVENT_ELEGON_CELESTIAL_BREATH, 22000);
                break;
            case EVENT_ELEGON_ARCING_ENERGY:
                castSpellOnVictim(SPELL_ELEGON_ARCING_ENERGY);
                scriptEvents.addEvent(EVENT_ELEGON_ARCING_ENERGY, 14000);
                break;
            case EVENT_ELEGON_CLOSED_CIRCUIT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ELEGON_CLOSED_CIRCUIT);
                scriptEvents.addEvent(EVENT_ELEGON_CLOSED_CIRCUIT, 20000);
                break;
            case EVENT_ELEGON_STABILITY_FLUX:
                castSpellOnSelf(SPELL_ELEGON_STABILITY_FLUX);
                scriptEvents.addEvent(EVENT_ELEGON_STABILITY_FLUX, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Will of the Emperor - fought as a pair of animated mogu constructs; Qin-xi is the
// registered representative running the shared kit (see header comment).

class WillOfTheEmperorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WillOfTheEmperorAI(c); }
    explicit WillOfTheEmperorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_WILL_OF_THE_EMPEROR, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_EMPEROR_FOCUSED_ASSAULT, 8000);
        scriptEvents.addEvent(EVENT_EMPEROR_ENERGIZING_SMASH, 14000);
        scriptEvents.addEvent(EVENT_EMPEROR_DEVASTATING_ARC, 20000);
        scriptEvents.addEvent(EVENT_EMPEROR_STOMP, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_WILL_OF_THE_EMPEROR, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_WILL_OF_THE_EMPEROR, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_EMPEROR_FOCUSED_ASSAULT:
                castSpellOnVictim(SPELL_EMPEROR_FOCUSED_ASSAULT);
                scriptEvents.addEvent(EVENT_EMPEROR_FOCUSED_ASSAULT, 18000);
                break;
            case EVENT_EMPEROR_ENERGIZING_SMASH:
                castSpellOnVictim(SPELL_EMPEROR_ENERGIZING_SMASH);
                scriptEvents.addEvent(EVENT_EMPEROR_ENERGIZING_SMASH, 20000);
                break;
            case EVENT_EMPEROR_DEVASTATING_ARC:
                castSpellAOE(SPELL_EMPEROR_DEVASTATING_ARC);
                scriptEvents.addEvent(EVENT_EMPEROR_DEVASTATING_ARC, 24000);
                break;
            case EVENT_EMPEROR_STOMP:
                castSpellAOE(SPELL_EMPEROR_STOMP);
                scriptEvents.addEvent(EVENT_EMPEROR_STOMP, 22000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Mogu Ambusher

class MoguAmbusherAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MoguAmbusherAI(c); }
    explicit MoguAmbusherAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_MOGU_AMBUSHER_CLEAVE, 6000);
        scriptEvents.addEvent(EVENT_MOGU_AMBUSHER_ENRAGE, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MOGU_AMBUSHER_CLEAVE:
                castSpellOnVictim(SPELL_MOGU_AMBUSHER_CLEAVE);
                scriptEvents.addEvent(EVENT_MOGU_AMBUSHER_CLEAVE, 12000);
                break;
            case EVENT_MOGU_AMBUSHER_ENRAGE:
                castSpellOnSelf(SPELL_MOGU_AMBUSHER_ENRAGE);
                scriptEvents.addEvent(EVENT_MOGU_AMBUSHER_ENRAGE, 26000);
                break;
            default:
                break;
        }
    }
};

void SetupMogushanVaults(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MOGUSHAN_VAULTS, &MogushanVaultsInstanceScript::Create);

    mgr->register_creature_script(BOSS_JASPER_GUARDIAN, &JasperGuardianAI::Create);
    mgr->register_creature_script(BOSS_AMETHYST_GUARDIAN, &AmethystGuardianAI::Create);
    mgr->register_creature_script(BOSS_JADE_GUARDIAN, &JadeGuardianAI::Create);
    mgr->register_creature_script(BOSS_COBALT_GUARDIAN, &CobaltGuardianAI::Create);

    mgr->register_creature_script(BOSS_FENG_THE_ACCURSED, &FengTheAccursedAI::Create);
    mgr->register_creature_script(BOSS_GARAJAL, &GarajalAI::Create);

    mgr->register_creature_script(BOSS_ZIAN, &ZianAI::Create);
    mgr->register_creature_script(BOSS_MENG, &MengAI::Create);
    mgr->register_creature_script(BOSS_QIANG, &QiangAI::Create);
    mgr->register_creature_script(BOSS_SUBETAI, &SubetaiAI::Create);

    mgr->register_creature_script(BOSS_ELEGON, &ElegonAI::Create);

    mgr->register_creature_script(BOSS_QIN_XI, &WillOfTheEmperorAI::Create);

    mgr->register_creature_script(NPC_MOGU_AMBUSHER, &MoguAmbusherAI::Create);
}
