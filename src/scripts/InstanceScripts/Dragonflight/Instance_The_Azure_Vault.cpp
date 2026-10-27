/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_The_Azure_Vault.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <set>
#include <vector>

enum TheAzureVaultEvents
{
    EVENT_LEYMOR_LEY_LINE_SPROUTS = 1,
    EVENT_LEYMOR_INFUSED_STRIKE,
    EVENT_LEYMOR_ERUPTING_FISSURE,
    EVENT_LEYMOR_FISSURE_IMPACT,
    EVENT_LEYMOR_EXPLOSIVE_BRAND,
    EVENT_LEYMOR_BRAND_DETONATION,
    EVENT_LEYMOR_CONSUMING_STOMP,
    EVENT_LEYMOR_STOMP_RELEASE,

    EVENT_TELASH_FROST_BOMB,
    EVENT_TELASH_FROST_BOMB_BURST,
    EVENT_TELASH_ICY_DEVASTATOR,
    EVENT_TELASH_LEAP_TO_CENTRE,
    EVENT_TELASH_ABSOLUTE_ZERO,
    EVENT_TELASH_RETURN,

    EVENT_UMBRELSKUL_DRAGON_STRIKE,
    EVENT_UMBRELSKUL_ARCANE_ERUPTION,
    EVENT_UMBRELSKUL_CRYSTALLINE_ROAR,
    EVENT_UMBRELSKUL_DETONATION,

    EVENT_AZUREBLADE_ARCANE_CLEAVE,
    EVENT_AZUREBLADE_ANCIENT_ORB,
    EVENT_AZUREBLADE_DRACONIC_IMAGE,

    EVENT_IMAGE_ILLUSIONARY_BOLT
};

namespace
{
    float const AzureVaultTwoPi = 6.28318531f;

    // Leymor's grove, sprouts grow on the centre and a ring of six patches
    LocationVector const LeymorGroveCentre(-5129.4f, 1253.3f, 555.6f);
    float const LeymorSproutRing = 26.5f;
    uint32_t const LeymorSproutPatches = 6;
    uint32_t const LeymorSproutsPerPatch = 2;
    float const LeymorFissureReach = 23.0f;
    float const LeymorBrandReach = 8.0f;
    uint32_t const LeymorBrandFuseMs = 5000;
    uint32_t const LeymorRotationMs = 48000;

    // Telash's rune hall
    LocationVector const TelashHallCentre(-5336.8f, 1066.6f, 339.6f);
    float const TelashRuneRing = 42.5f;
    float const TelashLandingRing = 26.5f;
    float const TelashRuneReach = 5.0f;
    uint32_t const TelashRuneCount = 3;
    uint32_t const TelashOpeningEnergy = 60;
    uint32_t const TelashEnergyPerSecond = 2;
    uint32_t const TelashFrostBombFuseMs = 6000;

    // Umbrelskul sheds crystals at these health marks
    int32_t const UmbrelskulBrittleMarks[3] = { 75, 50, 25 };
    uint32_t const UmbrelskulCrystalsPerMark = 3;
    uint32_t const UmbrelskulCrystalFuseMs = 20000;
    uint32_t const UmbrelskulVortexLifeMs = 20000;

    int32_t const AzurebladeEnergyMarkPct = 50;
    uint32_t const AzurebladeIllusionCount = 4;

    std::vector<Unit*> playersAround(Creature* pCreature, float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : pCreature->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pCreature->getDistance(pPlayer) <= range)
                players.push_back(pPlayer);
        }

        return players;
    }

    LocationVector ringPoint(LocationVector const& centre, float radius, float angle)
    {
        return LocationVector(centre.x + radius * std::cos(angle), centre.y + radius * std::sin(angle), centre.z, angle);
    }

    LocationVector pointNear(LocationVector const& origin, float minRange, float maxRange)
    {
        return ringPoint(origin, Util::getRandomFloat(minRange, maxRange), Util::getRandomFloat(AzureVaultTwoPi));
    }

    // living creatures of an entry around a creature
    std::vector<Creature*> livingAround(CreatureAIScript* pScript, uint32_t entry, float range)
    {
        std::list<Creature*> found;
        pScript->GetCreatureListWithEntryInGrid(found, entry, range);

        std::vector<Creature*> living;
        for (Creature* pCreature : found)
        {
            if (pCreature != nullptr && pCreature->isAlive())
                living.push_back(pCreature);
        }

        return living;
    }

    // room doors shut during the fight, passages open after the kill
    struct TheAzureVaultDoor
    {
        uint32_t entry;
        uint32_t dataId;
        bool passage;
    };

    TheAzureVaultDoor const TheAzureVaultDoors[] =
    {
        { GO_LEYMOR_ENTRANCE_DOOR, DATA_LEYMOR, false },
        { GO_LEYMOR_EXIT_DOOR,     DATA_LEYMOR, true }
    };

    TheAzureVaultDoor const* findTheAzureVaultDoor(uint32_t entry)
    {
        for (TheAzureVaultDoor const& door : TheAzureVaultDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class TheAzureVaultInstanceScript : public InstanceScript
{
public:
    explicit TheAzureVaultInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TheAzureVaultEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheAzureVaultInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_LEYMOR:
                mLeymorGuid = pCreature->GetNewGUID();
                holdLeymor();
                break;
            case NPC_ARCANE_TENDER:
                mTenderGuids.insert(pCreature->GetNewGUID());
                holdLeymor();
                break;
            default:
                break;
        }
    }

    void OnCreatureDeath(Creature* pVictim, Unit* /*pKiller*/) override
    {
        if (pVictim == nullptr || pVictim->getEntry() != NPC_ARCANE_TENDER || mLeymorFreed || anyTenderStanding(pVictim))
            return;

        // the last tender falls, the stasis breaks
        mLeymorFreed = true;
        Creature* pLeymor = getCreatureByGuid(mLeymorGuid);
        if (pLeymor != nullptr && pLeymor->isAlive() && pLeymor->GetScript() != nullptr)
            pLeymor->GetScript()->DoAction(ACTION_LEYMOR_AWAKEN);
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findTheAzureVaultDoor(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        applyDoorState(pGameObject);
    }

    void OnEncounterStateChange(uint32_t /*id*/, uint32_t /*state*/) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                applyDoorState(pDoor);
        }
    }

private:
    bool anyTenderStanding(Creature* pExcept)
    {
        for (WoWGuid const& guid : mTenderGuids)
        {
            Creature* pTender = getCreatureByGuid(guid);
            if (pTender != nullptr && pTender != pExcept && pTender->isAlive())
                return true;
        }

        return false;
    }

    // Leymor sleeps while one of his tenders still channels
    void holdLeymor()
    {
        if (mLeymorFreed || getBossState(DATA_LEYMOR) == EncounterStates::Performed || !anyTenderStanding(nullptr))
            return;

        Creature* pLeymor = getCreatureByGuid(mLeymorGuid);
        if (pLeymor != nullptr && pLeymor->isAlive() && pLeymor->GetScript() != nullptr)
            pLeymor->GetScript()->DoAction(ACTION_LEYMOR_STASIS);
    }

    void applyDoorState(GameObject* pDoor)
    {
        TheAzureVaultDoor const* pRule = findTheAzureVaultDoor(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        EncounterStates const state = getBossState(pRule->dataId);
        bool const open = pRule->passage ? state == EncounterStates::Performed : state != EncounterStates::InProgress;
        uint8_t const wanted = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    WoWGuid mLeymorGuid;
    std::set<WoWGuid> mTenderGuids;
    std::vector<WoWGuid> mDoorGuids;
    bool mLeymorFreed = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class TheAzureVaultCombatAI : public CreatureAIScript
{
public:
    explicit TheAzureVaultCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

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

    // summoned without pulling it into the fight
    Creature* placeMarker(uint32_t entry, LocationVector const& position, uint32_t lifeMs = 0)
    {
        return summonCreature(entry, position, lifeMs != 0 ? TIMED_DESPAWN : CORPSE_DESPAWN, lifeMs);
    }

    // the client bar follows the scripted energy where the creature has one
    void showEnergy(uint32_t value)
    {
        if (getCreature()->getMaxPower(POWER_TYPE_ENERGY) > 0)
            getCreature()->setPower(POWER_TYPE_ENERGY, value);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class TheAzureVaultBossAI : public TheAzureVaultCombatAI
{
public:
    TheAzureVaultBossAI(Creature* pCreature, uint32_t dataId) : TheAzureVaultCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        TheAzureVaultCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        TheAzureVaultCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Leymor

class TheAzureVaultLeymorAI : public TheAzureVaultBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheAzureVaultLeymorAI(c); }
    explicit TheAzureVaultLeymorAI(Creature* pCreature) : TheAzureVaultBossAI(pCreature, DATA_LEYMOR) {}

    void DoAction(int32_t action) override
    {
        if (!isAlive())
            return;

        if (action == ACTION_LEYMOR_STASIS && !mInStasis && !_isInCombat())
        {
            mInStasis = true;
            castSpellOnSelf(SPELL_LEYMOR_STASIS, true);
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
            setCanEnterCombat(false);
            return;
        }

        if (action == ACTION_LEYMOR_AWAKEN && mInStasis)
        {
            mInStasis = false;
            _removeAura(SPELL_LEYMOR_STASIS);
            removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
            setCanEnterCombat(true);
            warn("Leymor breaks free of its stasis!");
        }
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_LEYMOR_LEY_LINE_SPROUTS, 3000);
        scriptEvents.addEvent(EVENT_LEYMOR_INFUSED_STRIKE, 10000);
        scriptEvents.addEvent(EVENT_LEYMOR_ERUPTING_FISSURE, 20000);
        scriptEvents.addEvent(EVENT_LEYMOR_EXPLOSIVE_BRAND, 31000);
        scriptEvents.addEvent(EVENT_LEYMOR_CONSUMING_STOMP, 45000);
    }

    void onReset() override { _removeAura(SPELL_LEYMOR_ARCANE_POWER); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LEYMOR_LEY_LINE_SPROUTS:
                castSpellOnSelf(SPELL_LEYMOR_LEY_LINE_SPROUTS);
                growSprouts();
                scriptEvents.addEvent(EVENT_LEYMOR_LEY_LINE_SPROUTS, LeymorRotationMs);
                break;
            case EVENT_LEYMOR_INFUSED_STRIKE:
                castSpellOnVictim(SPELL_LEYMOR_INFUSED_STRIKE);
                scriptEvents.addEvent(EVENT_LEYMOR_INFUSED_STRIKE, LeymorRotationMs);
                break;
            case EVENT_LEYMOR_ERUPTING_FISSURE:
                castSpellOnVictim(SPELL_LEYMOR_ERUPTING_FISSURE);
                scriptEvents.addEvent(EVENT_LEYMOR_FISSURE_IMPACT, 500);
                scriptEvents.addEvent(EVENT_LEYMOR_ERUPTING_FISSURE, LeymorRotationMs);
                break;
            case EVENT_LEYMOR_FISSURE_IMPACT:
                crushSproutsAhead();
                break;
            case EVENT_LEYMOR_EXPLOSIVE_BRAND:
                warn("Leymor brands the intruders with Explosive Brand!");
                castSpellAOE(SPELL_LEYMOR_EXPLOSIVE_BRAND);
                scriptEvents.addEvent(EVENT_LEYMOR_BRAND_DETONATION, LeymorBrandFuseMs);
                scriptEvents.addEvent(EVENT_LEYMOR_EXPLOSIVE_BRAND, LeymorRotationMs);
                break;
            case EVENT_LEYMOR_BRAND_DETONATION:
                detonateBrands();
                break;
            case EVENT_LEYMOR_CONSUMING_STOMP:
            {
                // every surviving sprout feeds the stomp
                std::vector<Creature*> const sprouts = livingAround(this, NPC_LEY_LINE_SPROUT, 80.0f);
                for (size_t i = 0; i < sprouts.size(); ++i)
                    castSpellOnSelf(SPELL_LEYMOR_ARCANE_POWER, true);

                warn("Leymor begins a Consuming Stomp!");
                castSpellOnSelf(SPELL_LEYMOR_CONSUMING_STOMP);
                scriptEvents.addEvent(EVENT_LEYMOR_STOMP_RELEASE, 500);
                scriptEvents.addEvent(EVENT_LEYMOR_CONSUMING_STOMP, LeymorRotationMs);
                break;
            }
            case EVENT_LEYMOR_STOMP_RELEASE:
                castSpellAOE(SPELL_LEYMOR_CONSUMING_STOMP_BURST, true);
                for (Creature* pSprout : livingAround(this, NPC_LEY_LINE_SPROUT, 80.0f))
                    destroySprout(pSprout);
                _removeAura(SPELL_LEYMOR_ARCANE_POWER);
                break;
            default:
                break;
        }
    }

private:
    void growSprouts()
    {
        for (uint32_t patch = 0; patch <= LeymorSproutPatches; ++patch)
        {
            // patch 0 is the grove centre, the others sit on the ring
            LocationVector const origin = patch == 0 ? LeymorGroveCentre : ringPoint(LeymorGroveCentre, LeymorSproutRing, (patch - 1) * AzureVaultTwoPi / LeymorSproutPatches);
            for (uint32_t i = 0; i < LeymorSproutsPerPatch; ++i)
                placeMarker(NPC_LEY_LINE_SPROUT, pointNear(origin, 1.5f, 5.0f));
        }
    }

    void destroySprout(Creature* pSprout)
    {
        if (pSprout->isAlive())
            getCreature()->dealDamage(pSprout, pSprout->getHealth(), 0);
    }

    // the fissure rips through the sprouts in front of Leymor
    void crushSproutsAhead()
    {
        for (Creature* pSprout : livingAround(this, NPC_LEY_LINE_SPROUT, LeymorFissureReach))
        {
            if (getCreature()->isInFront(pSprout))
                destroySprout(pSprout);
        }
    }

    void detonateBrands()
    {
        std::vector<Creature*> const sprouts = livingAround(this, NPC_LEY_LINE_SPROUT, 80.0f);
        for (Unit* pPlayer : playersAround(getCreature(), 80.0f))
        {
            castSpell(pPlayer, SPELL_LEYMOR_EXPLOSIVE_BRAND_BURST, true);
            for (Creature* pSprout : sprouts)
            {
                if (pPlayer->getDistance(pSprout) <= LeymorBrandReach)
                    destroySprout(pSprout);
            }
        }

        _removeAuraOnPlayers(SPELL_LEYMOR_EXPLOSIVE_BRAND);
    }

    bool mInStasis = false;
};

// rooted sprout, saps everyone standing in it
class TheAzureVaultLeyLineSproutAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheAzureVaultLeyLineSproutAI(c); }
    explicit TheAzureVaultLeyLineSproutAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mSapClock > diff)
        {
            mSapClock -= diff;
            return;
        }

        mSapClock = 2000;
        for (Unit* pPlayer : playersAround(getCreature(), 4.0f))
            castSpell(pPlayer, SPELL_SPROUT_STINGING_SAP, true);
    }

private:
    uint32_t mSapClock = 2000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Telash Greywing, Absolute Zero at full Ice Power

class TheAzureVaultTelashAI : public TheAzureVaultBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheAzureVaultTelashAI(c); }
    explicit TheAzureVaultTelashAI(Creature* pCreature) : TheAzureVaultBossAI(pCreature, DATA_TELASH_GREYWING) {}

    void OnLoad() override { showEnergy(TelashOpeningEnergy); }

protected:
    void onEngage() override
    {
        mEnergy = TelashOpeningEnergy;
        mEnergyClock = 0;
        mFrozen = false;
        showEnergy(mEnergy);

        placeVaultRunes();
        queueRotation(1, 1, 3500, 14500);
    }

    void onReset() override
    {
        restoreFromCentre();
        _removeAuraOnPlayers(SPELL_TELASH_FROST_BOMB_MARK);
        mEnergy = TelashOpeningEnergy;
        showEnergy(mEnergy);
    }

    void onDefeat() override
    {
        _removeAuraOnPlayers(SPELL_TELASH_VAULT_RUNE);
        _removeAuraOnPlayers(SPELL_TELASH_FROST_BOMB_MARK);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mFrozen)
            return;

        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy = mEnergy + TelashEnergyPerSecond > 100 ? 100 : mEnergy + TelashEnergyPerSecond;
            showEnergy(mEnergy);
        }

        if (mEnergy >= 100 && !_isCasting())
            beginAbsoluteZero();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TELASH_FROST_BOMB:
                castSpellAOE(SPELL_TELASH_FROST_BOMB);
                for (Unit* pPlayer : playersAround(getCreature(), 100.0f))
                    castSpell(pPlayer, SPELL_TELASH_FROST_BOMB_MARK, true);
                scriptEvents.addEvent(EVENT_TELASH_FROST_BOMB_BURST, TelashFrostBombFuseMs);
                if (mBombsLeft > 0 && --mBombsLeft > 0)
                    scriptEvents.addEvent(EVENT_TELASH_FROST_BOMB, 17000);
                break;
            case EVENT_TELASH_FROST_BOMB_BURST:
                for (Unit* pPlayer : playersAround(getCreature(), 100.0f))
                    castSpell(pPlayer, SPELL_TELASH_FROST_BOMB_BURST, true);
                _removeAuraOnPlayers(SPELL_TELASH_FROST_BOMB_MARK);
                break;
            case EVENT_TELASH_ICY_DEVASTATOR:
                castAt(anyPlayer(), SPELL_TELASH_ICY_DEVASTATOR);
                if (mDevastatorsLeft > 0 && --mDevastatorsLeft > 0)
                    scriptEvents.addEvent(EVENT_TELASH_ICY_DEVASTATOR, 23000);
                break;
            case EVENT_TELASH_LEAP_TO_CENTRE:
                moveJump(TelashHallCentre, 25.0f, 10.0f);
                scriptEvents.addEvent(EVENT_TELASH_ABSOLUTE_ZERO, 2500);
                break;
            case EVENT_TELASH_ABSOLUTE_ZERO:
                setRooted(true);
                shieldRuneStanders();
                castSpellAOE(SPELL_TELASH_ABSOLUTE_ZERO);
                scriptEvents.addEvent(EVENT_TELASH_RETURN, 500);
                break;
            case EVENT_TELASH_RETURN:
                returnToFight();
                break;
            default:
                break;
        }
    }

private:
    void queueRotation(uint32_t bombs, uint32_t devastators, uint32_t firstBombMs, uint32_t firstDevastatorMs)
    {
        mBombsLeft = bombs;
        mDevastatorsLeft = devastators;
        scriptEvents.addEvent(EVENT_TELASH_FROST_BOMB, firstBombMs);
        scriptEvents.addEvent(EVENT_TELASH_ICY_DEVASTATOR, firstDevastatorMs);
    }

    void placeVaultRunes()
    {
        for (uint32_t i = 0; i < TelashRuneCount; ++i)
            placeMarker(NPC_VAULT_RUNE, ringPoint(TelashHallCentre, TelashRuneRing, i * AzureVaultTwoPi / TelashRuneCount));
    }

    void beginAbsoluteZero()
    {
        mFrozen = true;
        mEnergy = 0;
        showEnergy(0);

        scriptEvents.removeEvent(EVENT_TELASH_FROST_BOMB);
        scriptEvents.removeEvent(EVENT_TELASH_ICY_DEVASTATOR);

        warn("Telash Greywing gathers Absolute Zero! Stand on a Vault Rune!");
        setReactState(REACT_PASSIVE);
        attackStop();
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_TELASH_GLACIAL_SHIELD, true);
        castSpellOnSelf(SPELL_TELASH_ABSOLUTE_ZERO_CAST);
        scriptEvents.addEvent(EVENT_TELASH_LEAP_TO_CENTRE, 500);
    }

    // a rune halves the blast for whoever stands on it
    void shieldRuneStanders()
    {
        std::vector<Unit*> const players = playersAround(getCreature(), 100.0f);
        for (Creature* pRune : livingAround(this, NPC_VAULT_RUNE, 80.0f))
        {
            for (Unit* pPlayer : players)
            {
                if (pPlayer->getDistance(pRune) <= TelashRuneReach)
                    castSpell(pPlayer, SPELL_TELASH_VAULT_RUNE, true);
            }
        }
    }

    void returnToFight()
    {
        restoreFromCentre();

        // lands on the ring on the side of a foe
        Unit* pFoe = anyPlayer();
        if (pFoe != nullptr)
        {
            float const angle = std::atan2(pFoe->GetPositionY() - TelashHallCentre.y, pFoe->GetPositionX() - TelashHallCentre.x);
            moveJump(ringPoint(TelashHallCentre, TelashLandingRing, angle), 25.0f, 10.0f);
        }

        queueRotation(3, 2, 4300, 15200);
    }

    void restoreFromCentre()
    {
        mFrozen = false;
        _removeAuraOnPlayers(SPELL_TELASH_VAULT_RUNE);
        _removeAura(SPELL_TELASH_GLACIAL_SHIELD);
        setRooted(false);
        _setMeleeDisabled(false);
        setReactState(REACT_AGGRESSIVE);
    }

    uint32_t mEnergy = TelashOpeningEnergy;
    uint32_t mEnergyClock = 0;
    uint32_t mBombsLeft = 0;
    uint32_t mDevastatorsLeft = 0;
    bool mFrozen = false;
};

// marker on the hall floor, the rune itself does nothing
class TheAzureVaultVaultRuneAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheAzureVaultVaultRuneAI(c); }
    explicit TheAzureVaultVaultRuneAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        setCanEnterCombat(false);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Umbrelskul

class TheAzureVaultUmbrelskulAI : public TheAzureVaultBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheAzureVaultUmbrelskulAI(c); }
    explicit TheAzureVaultUmbrelskulAI(Creature* pCreature) : TheAzureVaultBossAI(pCreature, DATA_UMBRELSKUL) {}

protected:
    void onEngage() override
    {
        mMarksPassed = 0;
        scriptEvents.addEvent(EVENT_UMBRELSKUL_DRAGON_STRIKE, 6000);
        scriptEvents.addEvent(EVENT_UMBRELSKUL_ARCANE_ERUPTION, 11000);
        scriptEvents.addEvent(EVENT_UMBRELSKUL_CRYSTALLINE_ROAR, 22000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mMarksPassed < 3 && _getHealthPercent() <= UmbrelskulBrittleMarks[mMarksPassed] && !_isCasting())
        {
            ++mMarksPassed;
            shedCrystals();
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_UMBRELSKUL_DRAGON_STRIKE:
                castSpellOnVictim(SPELL_UMBRELSKUL_DRAGON_STRIKE);
                scriptEvents.addEvent(EVENT_UMBRELSKUL_DRAGON_STRIKE, 13000);
                break;
            case EVENT_UMBRELSKUL_ARCANE_ERUPTION:
                castSpellAOE(SPELL_UMBRELSKUL_ARCANE_ERUPTION);
                releaseVortexes();
                scriptEvents.addEvent(EVENT_UMBRELSKUL_ARCANE_ERUPTION, 26000);
                break;
            case EVENT_UMBRELSKUL_CRYSTALLINE_ROAR:
                castSpellOnVictim(SPELL_UMBRELSKUL_CRYSTALLINE_ROAR);
                scriptEvents.addEvent(EVENT_UMBRELSKUL_CRYSTALLINE_ROAR, 28000);
                break;
            case EVENT_UMBRELSKUL_DETONATION:
                detonateCrystals();
                break;
            default:
                break;
        }
    }

private:
    void shedCrystals()
    {
        warn("Umbrelskul grows Brittle and sheds Detonating Crystals!");
        castSpellOnSelf(SPELL_UMBRELSKUL_BRITTLE, true);

        for (uint32_t i = 0; i < UmbrelskulCrystalsPerMark; ++i)
            placeMarker(NPC_DETONATING_CRYSTAL, pointNear(getCreature()->GetPosition(), 8.0f, 15.0f));

        scriptEvents.addEvent(EVENT_UMBRELSKUL_DETONATION, UmbrelskulCrystalFuseMs);
    }

    // crystals left standing go off together
    void detonateCrystals()
    {
        std::vector<Creature*> const crystals = livingAround(this, NPC_DETONATING_CRYSTAL, 80.0f);
        if (crystals.empty())
            return;

        warn("The remaining Detonating Crystals explode in Unleashed Destruction!");
        castSpellAOE(SPELL_UMBRELSKUL_UNLEASHED_DESTRUCTION, true);
        for (Creature* pCrystal : crystals)
            pCrystal->despawn(500, 0);
    }

    void releaseVortexes()
    {
        for (uint8_t i = 0; i < 2; ++i)
        {
            if (Unit* pTarget = anyPlayer())
                placeMarker(NPC_CRACKLING_VORTEX, pointNear(pTarget->GetPosition(), 4.0f, 8.0f), UmbrelskulVortexLifeMs);
        }
    }

    uint32_t mMarksPassed = 0;
};

// fractures as long as it stands
class TheAzureVaultDetonatingCrystalAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheAzureVaultDetonatingCrystalAI(c); }
    explicit TheAzureVaultDetonatingCrystalAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        uint64_t const self = getCreature()->getGuid();
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject != nullptr)
                static_cast<Unit*>(pObject)->removeAllAurasById(SPELL_CRYSTAL_FRACTURE, self);
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mFractureClock > diff)
        {
            mFractureClock -= diff;
            return;
        }

        mFractureClock = 8000;
        castSpellAOE(SPELL_CRYSTAL_FRACTURE, true);
    }

private:
    uint32_t mFractureClock = 1000;
};

// drifts after a player and crackles whoever it touches
class TheAzureVaultCracklingVortexAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheAzureVaultCracklingVortexAI(c); }
    explicit TheAzureVaultCracklingVortexAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        setCanEnterCombat(false);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        mDriftClock = mDriftClock > diff ? mDriftClock - diff : 0;
        mTouchClock = mTouchClock > diff ? mTouchClock - diff : 0;

        if (mDriftClock == 0)
        {
            mDriftClock = 4000;
            std::vector<Unit*> const players = playersAround(getCreature(), 60.0f);
            if (!players.empty())
            {
                Unit* pPrey = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
                moveTo(pPrey->GetPositionX(), pPrey->GetPositionY(), pPrey->GetPositionZ(), false);
            }
        }

        if (mTouchClock == 0)
        {
            mTouchClock = 1000;
            for (Unit* pPlayer : playersAround(getCreature(), 3.0f))
            {
                if (!pPlayer->hasAurasWithId(SPELL_VORTEX_CRACKLING_VORTEX))
                    castSpell(pPlayer, SPELL_VORTEX_CRACKLING_VORTEX, true);
            }
        }
    }

private:
    uint32_t mDriftClock = 0;
    uint32_t mTouchClock = 1000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Azureblade

class TheAzureVaultAzurebladeAI : public TheAzureVaultBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheAzureVaultAzurebladeAI(c); }
    explicit TheAzureVaultAzurebladeAI(Creature* pCreature) : TheAzureVaultBossAI(pCreature, DATA_AZUREBLADE) {}

protected:
    void onEngage() override
    {
        mEnergyUsed = false;
        mEnergyActive = false;
        mIllusionClock = 0;
        queueRotation();
    }

    void onReset() override { leaveOverwhelmingEnergy(false); }

    void onCombatTick(uint32_t diff) override
    {
        if (!mEnergyUsed && _getHealthPercent() <= AzurebladeEnergyMarkPct && !_isCasting())
        {
            enterOverwhelmingEnergy();
            return;
        }

        if (!mEnergyActive)
            return;

        // the energy holds until every illusion is broken
        mIllusionClock += diff;
        if (mIllusionClock < 1000)
            return;

        mIllusionClock = 0;
        if (livingAround(this, NPC_DRACONIC_ILLUSION, 80.0f).empty())
            leaveOverwhelmingEnergy(true);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AZUREBLADE_ARCANE_CLEAVE:
                castSpellOnVictim(SPELL_AZUREBLADE_ARCANE_CLEAVE);
                scriptEvents.addEvent(EVENT_AZUREBLADE_ARCANE_CLEAVE, 13000);
                break;
            case EVENT_AZUREBLADE_ANCIENT_ORB:
                castAt(anyPlayer(), SPELL_AZUREBLADE_ANCIENT_ORB);
                scriptEvents.addEvent(EVENT_AZUREBLADE_ANCIENT_ORB, 19000);
                break;
            case EVENT_AZUREBLADE_DRACONIC_IMAGE:
            {
                warn("Azureblade conjures a Draconic Image!");
                uint32_t const images = isHeroic() ? 2 : 1;
                for (uint32_t i = 0; i < images; ++i)
                    callAdd(NPC_DRACONIC_IMAGE, pointNear(getCreature()->GetPosition(), 4.0f, 8.0f));
                scriptEvents.addEvent(EVENT_AZUREBLADE_DRACONIC_IMAGE, 25000);
                break;
            }
            default:
                break;
        }
    }

private:
    void queueRotation()
    {
        scriptEvents.addEvent(EVENT_AZUREBLADE_DRACONIC_IMAGE, 4000);
        scriptEvents.addEvent(EVENT_AZUREBLADE_ARCANE_CLEAVE, 7000);
        scriptEvents.addEvent(EVENT_AZUREBLADE_ANCIENT_ORB, 11000);
    }

    void enterOverwhelmingEnergy()
    {
        mEnergyUsed = true;
        mEnergyActive = true;
        mIllusionClock = 0;
        scriptEvents.resetEvents();

        warn("Azureblade channels Overwhelming Energy! Destroy the Draconic Illusions!");
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_AZUREBLADE_OVERWHELMING_ENERGY);

        for (uint32_t i = 0; i < AzurebladeIllusionCount; ++i)
            placeMarker(NPC_DRACONIC_ILLUSION, ringPoint(getCreature()->GetPosition(), 12.0f, i * AzureVaultTwoPi / AzurebladeIllusionCount));
    }

    void leaveOverwhelmingEnergy(bool resume)
    {
        mEnergyActive = false;
        _removeAura(SPELL_AZUREBLADE_OVERWHELMING_ENERGY);
        _removeAura(SPELL_ILLUSION_SEAL_EMPOWERMENT);
        setRooted(false);
        _setMeleeDisabled(false);

        if (!resume)
            return;

        warn("Azureblade's Overwhelming Energy fades!");
        queueRotation();
    }

    bool mEnergyUsed = false;
    bool mEnergyActive = false;
    uint32_t mIllusionClock = 0;
};

class TheAzureVaultDraconicImageAI : public TheAzureVaultCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheAzureVaultDraconicImageAI(c); }
    explicit TheAzureVaultDraconicImageAI(Creature* pCreature) : TheAzureVaultCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_IMAGE_ILLUSIONARY_BOLT, 2000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_IMAGE_ILLUSIONARY_BOLT)
            return;

        castAt(anyPlayer(), SPELL_IMAGE_ILLUSIONARY_BOLT);
        scriptEvents.addEvent(EVENT_IMAGE_ILLUSIONARY_BOLT, 4500);
    }
};

// keeps Azureblade sealed while it stands
class TheAzureVaultDraconicIllusionAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheAzureVaultDraconicIllusionAI(c); }
    explicit TheAzureVaultDraconicIllusionAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mSealClock > diff)
        {
            mSealClock -= diff;
            return;
        }

        mSealClock = 5000;
        Creature* pBlade = findNearestCreature(BOSS_AZUREBLADE, 60.0f);
        if (pBlade != nullptr && pBlade->isAlive() && pBlade->isInCombat())
            castSpell(pBlade, SPELL_ILLUSION_SEAL_EMPOWERMENT, true);
    }

private:
    uint32_t mSealClock = 1000;
};

void SetupTheAzureVault(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_AZURE_VAULT, &TheAzureVaultInstanceScript::Create);

    mgr->register_creature_script(BOSS_LEYMOR, &TheAzureVaultLeymorAI::Create);
    mgr->register_creature_script(NPC_LEY_LINE_SPROUT, &TheAzureVaultLeyLineSproutAI::Create);

    mgr->register_creature_script(BOSS_TELASH_GREYWING, &TheAzureVaultTelashAI::Create);
    mgr->register_creature_script(NPC_VAULT_RUNE, &TheAzureVaultVaultRuneAI::Create);

    mgr->register_creature_script(BOSS_UMBRELSKUL, &TheAzureVaultUmbrelskulAI::Create);
    mgr->register_creature_script(NPC_DETONATING_CRYSTAL, &TheAzureVaultDetonatingCrystalAI::Create);
    mgr->register_creature_script(NPC_CRACKLING_VORTEX, &TheAzureVaultCracklingVortexAI::Create);

    mgr->register_creature_script(BOSS_AZUREBLADE, &TheAzureVaultAzurebladeAI::Create);
    mgr->register_creature_script(NPC_DRACONIC_IMAGE, &TheAzureVaultDraconicImageAI::Create);
    mgr->register_creature_script(NPC_DRACONIC_ILLUSION, &TheAzureVaultDraconicIllusionAI::Create);
}
