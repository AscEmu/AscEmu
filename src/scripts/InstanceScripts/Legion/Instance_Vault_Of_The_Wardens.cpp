/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Vault_Of_The_Wardens.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <sstream>
#include <string>
#include <vector>

enum VaultOfTheWardensEvents
{
    EVENT_TIRATHON_DARKSTRIKES = 1,
    EVENT_TIRATHON_SWOOP,
    EVENT_TIRATHON_FURIOUS_BLAST,
    EVENT_TIRATHON_FEL_MORTAR,
    EVENT_TIRATHON_METAMORPHOSIS,

    EVENT_ASHGOLM_VOLCANO,
    EVENT_ASHGOLM_LAVA_WREATH,
    EVENT_ASHGOLM_FISSURE,
    EVENT_ASHGOLM_EMBERS,

    EVENT_EMBER_SEAR,

    EVENT_GLAZER_PULSE,
    EVENT_GLAZER_LINGERING_GAZE,
    EVENT_GLAZER_TELEPORT,
    EVENT_GLAZER_ENERGIZE,
    EVENT_GLAZER_FOCUS,

    EVENT_CORDANA_KNOCKDOWN_KICK,
    EVENT_CORDANA_DEEPENING_SHADOWS,
    EVENT_CORDANA_MOONGLAIVE,
    EVENT_CORDANA_AVATAR,

    EVENT_TORMENTORUM_SAP_SOUL,
    EVENT_TORMENTORUM_FLEE_TO_CELLS,
    EVENT_TORMENTORUM_OPEN_PRISONS,
    EVENT_TORMENTORUM_FREE_PRISONERS
};

namespace
{
    enum class WardenDoorKind : uint8_t
    {
        Room,
        Passage,
        SpiritGate
    };

    struct WardenDoorRule
    {
        uint32_t entry;
        uint32_t encounterId;
        WardenDoorKind kind;
    };

    WardenDoorRule const WardenDoorRules[] =
    {
        { GO_TIRATHON_DOOR,         DATA_TIRATHON_SALTHERIL,        WardenDoorKind::Room },
        { GO_TIRATHON_EXIT_1,       DATA_TIRATHON_SALTHERIL,        WardenDoorKind::Passage },
        { GO_TIRATHON_EXIT_2,       DATA_TIRATHON_SALTHERIL,        WardenDoorKind::Passage },
        { GO_TIRATHON_FIREWALL_1,   DATA_TIRATHON_SALTHERIL,        WardenDoorKind::Passage },
        { GO_TIRATHON_FIREWALL_2,   DATA_TIRATHON_SALTHERIL,        WardenDoorKind::Passage },
        { GO_TORMENTORUM_EXIT,      DATA_INQUISITOR_TORMENTORUM,    WardenDoorKind::Passage },
        { GO_CORDANA_DOOR,          DATA_CORDANA_FELSONG,           WardenDoorKind::SpiritGate }
    };

    WardenDoorRule const* findWardenDoor(uint32_t entry)
    {
        for (WardenDoorRule const& rule : WardenDoorRules)
        {
            if (rule.entry == entry)
                return &rule;
        }

        return nullptr;
    }

    bool isTormentCell(uint32_t entry) { return entry >= GO_TORMENT_CELL_FIRST && entry <= GO_TORMENT_CELL_LAST; }

    float const WardenTwoPi = 6.28318531f;

    // spirit standing guard in front of Cordana's door
    LocationVector const CordanaDoorSpiritSpot(4234.50f, -297.59f, -281.39f);
    float const CordanaDoorSpiritRange = 15.0f;

    LocationVector const CordanaAvatarSpots[] =
    {
        { 4027.47f, -273.93f, -281.11f },
        { 4074.04f, -274.42f, -281.11f },
        { 4074.04f, -320.78f, -281.11f },
        { 4027.47f, -320.78f, -281.11f }
    };

    // cell blocks of the inquisitor: orcs, mogu, void, corruption
    LocationVector const TormentorumCellBlocks[] =
    {
        { 4423.45f, -423.79f, 119.87f, 2.33f },
        { 4478.04f, -423.82f, 119.87f, 0.75f },
        { 4478.52f, -478.88f, 119.87f, 5.44f },
        { 4423.03f, -478.81f, 119.87f, 3.90f }
    };
    uint32_t const TormentorumCellBlockCount = 4;
    uint32_t const TormentorumCellOpenings = 2;

    struct TormentorumPrisoner
    {
        uint32_t entry;
        LocationVector spot;
    };

    TormentorumPrisoner const TormentorumPrisoners[4][3] =
    {
        {
            { NPC_SHADOWMOON_WARLOCK,       { 4417.47f, -417.87f, 120.39f, 5.50f } },
            { NPC_FELGUARD_ANNIHILATOR,     { 4405.09f, -418.98f, 120.15f, 4.65f } },
            { NPC_SHADOWMOON_TECHNICIAN,    { 4418.89f, -405.60f, 120.23f, 0.08f } }
        },
        {
            { NPC_ENORMOUS_STONE_QUILEN,    { 4483.31f, -418.44f, 119.96f, 3.97f } },
            { NPC_MOGUSHAN_SECRET_KEEPER,   { 4481.22f, -405.33f, 119.96f, 3.13f } },
            { NPC_MOGUSHAN_SECRET_KEEPER,   { 4496.58f, -419.27f, 120.39f, 4.91f } }
        },
        {
            { NPC_VOID_TOUCHED_JUGGERNAUT,  { 4484.36f, -484.62f, 120.31f, 2.36f } },
            { NPC_DERANGED_MINDFLAYER,      { 4496.40f, -482.99f, 120.15f, 1.47f } },
            { NPC_FACELESS_VOIDCASTER,      { 4483.17f, -496.78f, 120.15f, 3.25f } }
        },
        {
            { NPC_LINGERING_CORRUPTION,     { 4417.06f, -484.70f, 120.31f, 0.79f } },
            { NPC_LINGERING_CORRUPTION,     { 4418.96f, -496.95f, 120.25f, 0.02f } },
            { NPC_LINGERING_CORRUPTION,     { 4405.20f, -483.58f, 120.25f, 1.68f } }
        }
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class VaultOfTheWardensInstanceScript : public InstanceScript
{
public:
    explicit VaultOfTheWardensInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(VaultOfTheWardensEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new VaultOfTheWardensInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        if (pCreature->getEntry() != NPC_SPIRIT_OF_VENGEANCE || !isDoorSpirit(pCreature))
            return;

        mDoorSpiritGuid = pCreature->GetNewGUID();
        alignDoors();
    }

    void OnCreatureDeath(Creature* pVictim, Unit* /*pKiller*/) override
    {
        if (pVictim->getEntry() != NPC_SPIRIT_OF_VENGEANCE || !isDoorSpirit(pVictim))
            return;

        mDoorSpiritSlain = true;
        saveToDB();
        alignDoors();
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (isTormentCell(pGameObject->getEntry()))
        {
            mCellGuids.push_back(pGameObject->GetNewGUID());
            return;
        }

        if (findWardenDoor(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        alignDoor(pGameObject);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        alignDoors();

        // a failed inquisitor locks his prisoners away again
        if (id == DATA_INQUISITOR_TORMENTORUM && (state == EncounterStates::Failed || state == EncounterStates::NotStarted))
        {
            for (WoWGuid const& guid : mCellGuids)
            {
                if (GameObject* pCell = getGameObjectByGuid(guid))
                    pCell->setState(GO_STATE_CLOSED);
            }
        }
    }

    void writeSaveDataExtended(std::ostringstream& data) override { data << (mDoorSpiritSlain ? 1 : 0) << ' '; }

    void readSaveDataExtended(std::istringstream& data) override
    {
        uint32_t slain = 0;
        if (data >> slain)
            mDoorSpiritSlain = slain != 0;
    }

private:
    static bool isDoorSpirit(Creature* pCreature)
    {
        return pCreature->GetSpawnPosition().distanceSquare(CordanaDoorSpiritSpot) <= CordanaDoorSpiritRange * CordanaDoorSpiritRange;
    }

    // without a door spirit spawn the gate follows Glazer
    bool isSpiritGateOpen()
    {
        if (mDoorSpiritSlain || getBossState(DATA_CORDANA_FELSONG) == EncounterStates::Performed)
            return true;

        Creature* pSpirit = getCreatureByGuid(mDoorSpiritGuid);
        if (pSpirit != nullptr && pSpirit->isAlive())
            return false;

        return getBossState(DATA_GLAZER) == EncounterStates::Performed;
    }

    void alignDoors()
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                alignDoor(pDoor);
        }
    }

    void alignDoor(GameObject* pDoor)
    {
        WardenDoorRule const* rule = findWardenDoor(pDoor->getEntry());
        if (rule == nullptr)
            return;

        EncounterStates const state = getBossState(rule->encounterId);
        bool wantOpen = false;
        switch (rule->kind)
        {
            case WardenDoorKind::Room:
                wantOpen = state != EncounterStates::InProgress;
                break;
            case WardenDoorKind::Passage:
                wantOpen = state == EncounterStates::Performed;
                break;
            case WardenDoorKind::SpiritGate:
                wantOpen = isSpiritGateOpen();
                break;
        }

        bool const isOpen = pDoor->getState() != GO_STATE_CLOSED;
        if (wantOpen != isOpen)
            pDoor->setState(wantOpen ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    WoWGuid mDoorSpiritGuid;
    std::vector<WoWGuid> mDoorGuids;
    std::vector<WoWGuid> mCellGuids;
    bool mDoorSpiritSlain = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class VaultOfTheWardensCombatAI : public CreatureAIScript
{
public:
    explicit VaultOfTheWardensCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    Unit* anyPlayer() { return getBestPlayerTarget(TargetFilter_None); }

    void castAt(Unit* pTarget, uint32_t spellId)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId);
    }

    Creature* raiseAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector nearSelf(float radius)
    {
        float const angle = Util::getRandomFloat(WardenTwoPi);
        Creature* pSelf = getCreature();
        return LocationVector(pSelf->GetPositionX() + radius * std::cos(angle), pSelf->GetPositionY() + radius * std::sin(angle), pSelf->GetPositionZ(), angle);
    }

    void warn(std::string const& text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class VaultOfTheWardensBossAI : public VaultOfTheWardensCombatAI
{
public:
    VaultOfTheWardensBossAI(Creature* pCreature, uint32_t encounterId) : VaultOfTheWardensCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        VaultOfTheWardensCombatAI::OnCombatStart(pTarget);
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

        VaultOfTheWardensCombatAI::OnCombatStop(pTarget);
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
// Tirathon Saltheril

class VaultOfTheWardensTirathonAI : public VaultOfTheWardensBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheWardensTirathonAI(c); }
    explicit VaultOfTheWardensTirathonAI(Creature* pCreature) : VaultOfTheWardensBossAI(pCreature, DATA_TIRATHON_SALTHERIL) {}

protected:
    void onEngage() override
    {
        mMetamorphosed = false;
        scriptEvents.addEvent(EVENT_TIRATHON_FEL_MORTAR, 10000);
        scriptEvents.addEvent(EVENT_TIRATHON_DARKSTRIKES, 16000);
        scriptEvents.addEvent(EVENT_TIRATHON_SWOOP, 22000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mMetamorphosed || _getHealthPercent() > 50)
            return;

        mMetamorphosed = true;
        scriptEvents.addEvent(EVENT_TIRATHON_METAMORPHOSIS, 100);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TIRATHON_DARKSTRIKES:
                warn("Tirathon Saltheril empowers his warglaives with Darkstrikes!");
                castSpellOnSelf(SPELL_TIRATHON_DARKSTRIKES);
                scriptEvents.addEvent(EVENT_TIRATHON_DARKSTRIKES, 60000);
                break;
            case EVENT_TIRATHON_SWOOP:
                castAt(anyPlayer(), SPELL_TIRATHON_SWOOP);
                scriptEvents.addEvent(EVENT_TIRATHON_FURIOUS_BLAST, 2000);
                scriptEvents.addEvent(EVENT_TIRATHON_SWOOP, 30000);
                break;
            case EVENT_TIRATHON_FURIOUS_BLAST:
                castSpellAOE(SPELL_TIRATHON_FURIOUS_BLAST);
                break;
            case EVENT_TIRATHON_FEL_MORTAR:
                castAt(anyPlayer(), SPELL_TIRATHON_FEL_MORTAR);
                scriptEvents.addEvent(EVENT_TIRATHON_FEL_MORTAR, 16000);
                break;
            case EVENT_TIRATHON_METAMORPHOSIS:
                warn("Tirathon Saltheril unleashes his inner demon!");
                // one of the two demon forms, vengeance keeps him on the ground
                if (Util::getRandomUInt(1) == 0)
                {
                    castSpellOnSelf(SPELL_TIRATHON_METAMORPHOSIS_HAVOC);
                }
                else
                {
                    scriptEvents.removeEvent(EVENT_TIRATHON_SWOOP);
                    scriptEvents.removeEvent(EVENT_TIRATHON_FURIOUS_BLAST);
                    castSpellOnSelf(SPELL_TIRATHON_METAMORPHOSIS_VENGEANCE);
                }
                break;
            default:
                break;
        }
    }

private:
    bool mMetamorphosed = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ash'Golm

class VaultOfTheWardensAshgolmAI : public VaultOfTheWardensBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheWardensAshgolmAI(c); }
    explicit VaultOfTheWardensAshgolmAI(Creature* pCreature) : VaultOfTheWardensBossAI(pCreature, DATA_ASHGOLM) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_ASHGOLM_HEAT_UP, true);
        scriptEvents.addEvent(EVENT_ASHGOLM_VOLCANO, 10000);
        scriptEvents.addEvent(EVENT_ASHGOLM_EMBERS, 15000);
        scriptEvents.addEvent(EVENT_ASHGOLM_LAVA_WREATH, 25000);
        scriptEvents.addEvent(EVENT_ASHGOLM_FISSURE, 40000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ASHGOLM_VOLCANO:
                castAt(anyPlayer(), SPELL_ASHGOLM_VOLCANO);
                scriptEvents.addEvent(EVENT_ASHGOLM_VOLCANO, 20000);
                break;
            case EVENT_ASHGOLM_LAVA_WREATH:
                warn("Ash'Golm surrounds himself with a Lava Wreath!");
                castSpellAOE(SPELL_ASHGOLM_LAVA_WREATH);
                scriptEvents.addEvent(EVENT_ASHGOLM_LAVA_WREATH, 55000);
                break;
            case EVENT_ASHGOLM_FISSURE:
                castSpellOnVictim(SPELL_ASHGOLM_FISSURE);
                scriptEvents.addEvent(EVENT_ASHGOLM_FISSURE, 46000);
                break;
            case EVENT_ASHGOLM_EMBERS:
                stokeEmber();
                scriptEvents.addEvent(EVENT_ASHGOLM_EMBERS, 15000);
                break;
            default:
                break;
        }
    }

private:
    // embers crawl out of a volcano, or out of the boss when none is up
    void stokeEmber()
    {
        if (GameObject* pVolcano = findNearestGameObject(GO_ASHGOLM_VOLCANO, 60.0f))
            raiseAdd(NPC_EMBER, pVolcano->GetPosition());
        else
            raiseAdd(NPC_EMBER, nearSelf(10.0f));
    }
};

class VaultOfTheWardensEmberAI : public VaultOfTheWardensCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheWardensEmberAI(c); }
    explicit VaultOfTheWardensEmberAI(Creature* pCreature) : VaultOfTheWardensCombatAI(pCreature) {}

    void OnLoad() override { castSpellOnSelf(SPELL_EMBER_SMOLDERING, true); }

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_EMBER_SEAR, 2000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_EMBER_SEAR)
            return;

        castAt(anyPlayer(), SPELL_EMBER_SEAR);
        scriptEvents.addEvent(EVENT_EMBER_SEAR, 4000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Glazer

class VaultOfTheWardensGlazerAI : public VaultOfTheWardensBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheWardensGlazerAI(c); }
    explicit VaultOfTheWardensGlazerAI(Creature* pCreature) : VaultOfTheWardensBossAI(pCreature, DATA_GLAZER) {}

protected:
    void onEngage() override
    {
        mFocusing = false;
        scriptEvents.addEvent(EVENT_GLAZER_PULSE, 6000);
        scriptEvents.addEvent(EVENT_GLAZER_LINGERING_GAZE, 12000);
        scriptEvents.addEvent(EVENT_GLAZER_TELEPORT, 30000);
    }

    void onReset() override { endFocus(false); }

    // the beam ends together with its channel
    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mFocusing && !_isCasting())
            endFocus(true);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GLAZER_PULSE:
                castSpellAOE(SPELL_GLAZER_PULSE);
                scriptEvents.addEvent(EVENT_GLAZER_PULSE, 12000);
                break;
            case EVENT_GLAZER_LINGERING_GAZE:
                castAt(anyPlayer(), SPELL_GLAZER_LINGERING_GAZE);
                scriptEvents.addEvent(EVENT_GLAZER_LINGERING_GAZE, 18000);
                break;
            case EVENT_GLAZER_TELEPORT:
                attackStop();
                setReactState(REACT_PASSIVE);
                castSpellOnSelf(SPELL_GLAZER_TELEPORT);
                scriptEvents.addEvent(EVENT_GLAZER_ENERGIZE, 500);
                break;
            case EVENT_GLAZER_ENERGIZE:
                warn("Glazer focuses arcane energy through his eye!");
                castSpellOnSelf(SPELL_GLAZER_ENERGIZE);
                scriptEvents.addEvent(EVENT_GLAZER_FOCUS, 500);
                break;
            case EVENT_GLAZER_FOCUS:
                setRooted(true);
                castAt(anyPlayer(), SPELL_GLAZER_FOCUSED);
                mFocusing = true;
                break;
            default:
                break;
        }
    }

private:
    void endFocus(bool scheduleNext)
    {
        mFocusing = false;
        setRooted(false);
        setReactState(REACT_AGGRESSIVE);

        if (scheduleNext)
            scriptEvents.addEvent(EVENT_GLAZER_TELEPORT, 30000);
    }

    bool mFocusing = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Cordana Felsong

class VaultOfTheWardensCordanaAI : public VaultOfTheWardensBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheWardensCordanaAI(c); }
    explicit VaultOfTheWardensCordanaAI(Creature* pCreature) : VaultOfTheWardensBossAI(pCreature, DATA_CORDANA_FELSONG) {}

    // her shield holds while any avatar stands
    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || pSummon->getEntry() != NPC_AVATAR_OF_VENGEANCE || mAvatarsAlive == 0)
            return;

        if (--mAvatarsAlive == 0)
            _removeAura(SPELL_CORDANA_VENGEANCE);
    }

protected:
    void onEngage() override
    {
        mVengeanceCalled = false;
        mAvatarsAlive = 0;
        scriptEvents.addEvent(EVENT_CORDANA_KNOCKDOWN_KICK, 8300);
        scriptEvents.addEvent(EVENT_CORDANA_DEEPENING_SHADOWS, 11000);
        scriptEvents.addEvent(EVENT_CORDANA_MOONGLAIVE, 11000);
    }

    void onReset() override
    {
        mAvatarsAlive = 0;
        _removeAura(SPELL_CORDANA_VENGEANCE);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        int32_t const threshold = isHeroic() ? 25 : 50;
        if (mVengeanceCalled || _getHealthPercent() > threshold)
            return;

        mVengeanceCalled = true;
        scriptEvents.addEvent(EVENT_CORDANA_AVATAR, 100);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CORDANA_KNOCKDOWN_KICK:
                castSpellOnVictim(SPELL_CORDANA_KNOCKDOWN_KICK);
                scriptEvents.addEvent(EVENT_CORDANA_KNOCKDOWN_KICK, 16000);
                break;
            case EVENT_CORDANA_DEEPENING_SHADOWS:
                if (isHeroic())
                    castAt(anyPlayer(), SPELL_CORDANA_DEEPENING_SHADOWS_HEROIC);
                else
                    castSpellOnVictim(SPELL_CORDANA_DEEPENING_SHADOWS);
                scriptEvents.addEvent(EVENT_CORDANA_DEEPENING_SHADOWS, 30500);
                break;
            case EVENT_CORDANA_MOONGLAIVE:
                castSpellAOE(SPELL_CORDANA_DETONATING_MOONGLAIVE);
                scriptEvents.addEvent(EVENT_CORDANA_MOONGLAIVE, 9000);
                break;
            case EVENT_CORDANA_AVATAR:
                callAvatar();
                scriptEvents.addEvent(EVENT_CORDANA_AVATAR, 35000);
                break;
            default:
                break;
        }
    }

private:
    void callAvatar()
    {
        LocationVector const& spot = CordanaAvatarSpots[Util::getRandomUInt(3)];
        Creature* pAvatar = raiseAdd(NPC_AVATAR_OF_VENGEANCE, spot);
        if (pAvatar == nullptr)
            return;

        warn("Cordana Felsong calls forth an Avatar of Vengeance!");
        pAvatar->setControlled(true, UNIT_STATE_ROOTED);
        ++mAvatarsAlive;
        castSpellOnSelf(SPELL_CORDANA_VENGEANCE, true);
    }

    bool mVengeanceCalled = false;
    uint32_t mAvatarsAlive = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Inquisitor Tormentorum: frees a cell block at 70% and 40% health

class VaultOfTheWardensTormentorumAI : public VaultOfTheWardensBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheWardensTormentorumAI(c); }
    explicit VaultOfTheWardensTormentorumAI(Creature* pCreature) : VaultOfTheWardensBossAI(pCreature, DATA_INQUISITOR_TORMENTORUM) {}

    void OnLoad() override { castSpellOnSelf(SPELL_TORMENTORUM_TORMENT, true); }

protected:
    void onEngage() override
    {
        _removeAura(SPELL_TORMENTORUM_TORMENT);
        chooseCellBlocks();
        mOpenings = 0;
        mNextThreshold = 70;
        mFleeing = false;
        scriptEvents.addEvent(EVENT_TORMENTORUM_SAP_SOUL, 15000);
    }

    void onReset() override
    {
        mFleeing = false;
        setReactState(REACT_AGGRESSIVE);
        castSpellOnSelf(SPELL_TORMENTORUM_TORMENT, true);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mFleeing || mOpenings >= TormentorumCellOpenings || _getHealthPercent() > mNextThreshold)
            return;

        mFleeing = true;
        mNextThreshold -= 30;
        attackStop();
        setReactState(REACT_PASSIVE);
        scriptEvents.addEvent(EVENT_TORMENTORUM_FLEE_TO_CELLS, 500);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TORMENTORUM_SAP_SOUL:
                castSpellAOE(SPELL_TORMENTORUM_SAP_SOUL);
                scriptEvents.addEvent(EVENT_TORMENTORUM_SAP_SOUL, 21000);
                break;
            case EVENT_TORMENTORUM_FLEE_TO_CELLS:
                warn("Inquisitor Tormentorum retreats to the prison cells!");
                MoveTeleport(TormentorumCellBlocks[mBlocks[mOpenings]]);
                scriptEvents.addEvent(EVENT_TORMENTORUM_OPEN_PRISONS, 2500);
                break;
            case EVENT_TORMENTORUM_OPEN_PRISONS:
                castSpellAOE(SPELL_TORMENTORUM_OPEN_PRISONS);
                scriptEvents.addEvent(EVENT_TORMENTORUM_FREE_PRISONERS, 5000);
                break;
            case EVENT_TORMENTORUM_FREE_PRISONERS:
                freePrisoners(mBlocks[mOpenings]);
                ++mOpenings;
                mFleeing = false;
                setReactState(REACT_AGGRESSIVE);
                break;
            default:
                break;
        }
    }

private:
    // two of the four blocks per attempt
    void chooseCellBlocks()
    {
        mBlocks[0] = Util::getRandomUInt(TormentorumCellBlockCount - 1);
        mBlocks[1] = (mBlocks[0] + 1 + Util::getRandomUInt(TormentorumCellBlockCount - 2)) % TormentorumCellBlockCount;
    }

    void freePrisoners(uint32_t block)
    {
        for (uint32_t entry = GO_TORMENT_CELL_FIRST; entry <= GO_TORMENT_CELL_LAST; ++entry)
        {
            std::list<GameObject*> cells;
            GetGameObjectListWithEntryInGrid(cells, entry, 30.0f);
            for (GameObject* pCell : cells)
            {
                if (pCell->getState() == GO_STATE_CLOSED)
                    pCell->setState(GO_STATE_OPEN);
            }
        }

        for (TormentorumPrisoner const& prisoner : TormentorumPrisoners[block])
            raiseAdd(prisoner.entry, prisoner.spot);
    }

    uint32_t mBlocks[TormentorumCellOpenings] = { 0, 1 };
    uint32_t mOpenings = 0;
    int32_t mNextThreshold = 70;
    bool mFleeing = false;
};

void SetupVaultOfTheWardens(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_VAULT_OF_THE_WARDENS, &VaultOfTheWardensInstanceScript::Create);

    mgr->register_creature_script(BOSS_TIRATHON_SALTHERIL, &VaultOfTheWardensTirathonAI::Create);

    mgr->register_creature_script(BOSS_ASHGOLM, &VaultOfTheWardensAshgolmAI::Create);
    mgr->register_creature_script(NPC_EMBER, &VaultOfTheWardensEmberAI::Create);

    mgr->register_creature_script(BOSS_GLAZER, &VaultOfTheWardensGlazerAI::Create);

    mgr->register_creature_script(BOSS_CORDANA_FELSONG, &VaultOfTheWardensCordanaAI::Create);

    mgr->register_creature_script(BOSS_INQUISITOR_TORMENTORUM, &VaultOfTheWardensTormentorumAI::Create);
}
