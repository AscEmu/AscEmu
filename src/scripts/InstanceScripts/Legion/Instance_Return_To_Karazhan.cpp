/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Return_To_Karazhan.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <set>
#include <sstream>
#include <vector>

enum ReturnToKarazhanEvents
{
    EVENT_MAIDEN_HOLY_BOLT = 1,
    EVENT_MAIDEN_SACRED_GROUND,
    EVENT_MAIDEN_HOLY_SHOCK,
    EVENT_MAIDEN_MASS_REPENTANCE,
    EVENT_MAIDEN_HOLY_BULWARK,
    EVENT_MAIDEN_HOLY_WRATH,

    EVENT_GALINDRE_FLASHY_BOLT,
    EVENT_GALINDRE_WONDROUS_RADIANCE,
    EVENT_GALINDRE_MAGIC_MAGNIFICENT,
    EVENT_GALINDRE_SHIELD_FADES,
    EVENT_ELFYRA_DREARY_BOLT,
    EVENT_ELFYRA_DEFY_GRAVITY,
    EVENT_ELFYRA_ASSISTANTS,

    EVENT_TOE_KNEE_LEG_SWEEP,
    EVENT_TOE_KNEE_FLAME_GALE,
    EVENT_TOE_KNEE_FLAME_GALE_CHARGE,
    EVENT_MRRGRIA_THUNDER_RITUAL,
    EVENT_MRRGRIA_WASH_AWAY,
    EVENT_RUFFIAN_SHANK,
    EVENT_TIDESPEAKER_BUBBLE_BLAST,

    EVENT_COGGLESTON_DENT_ARMOR,
    EVENT_COGGLESTON_KARA_KAZHAM,
    EVENT_COGGLESTON_DINNER_BELL,
    EVENT_LUMINORE_BURNING_BLAZE,
    EVENT_LUMINORE_HEAT_WAVE,
    EVENT_BABBLET_FIXATE,
    EVENT_CAULDRONS_SOUP_SPRAY,
    EVENT_CAULDRONS_LEFTOVERS,

    EVENT_DEVOURER_ARCANE_BOMB,
    EVENT_DEVOURER_ENERGY_VOID,
    EVENT_DEVOURER_ENERGY_DISCHARGE,
    EVENT_DEVOURER_COALESCE_POWER,

    EVENT_MIDNIGHT_INTANGIBLE_PRESENCE,
    EVENT_MIDNIGHT_MIGHTY_STOMP,
    EVENT_MIDNIGHT_MEZAIR,
    EVENT_ATTUMEN_MORTAL_STRIKE,
    EVENT_ATTUMEN_SHARED_SUFFERING,

    EVENT_MOROES_VANISH,
    EVENT_MOROES_GARROTE,
    EVENT_MOROES_COAT_CHECK,
    EVENT_GUEST_ABILITY_1,
    EVENT_GUEST_ABILITY_2,

    EVENT_CURATOR_VOLATILE_ENERGY,
    EVENT_CURATOR_POWER_DISCHARGE,
    EVENT_CURATOR_EVOCATION,
    EVENT_CURATOR_OVERLOAD,
    EVENT_VOLATILE_STATIC_CHARGE,

    EVENT_SHADE_BASIC_SPELL,
    EVENT_IMAGE_SHIMMER,
    EVENT_IMAGE_ARCANE_BOLT,

    EVENT_VIZADUUM_BURNING_BLAST,
    EVENT_VIZADUUM_FEL_BEAM,
    EVENT_VIZADUUM_DISINTEGRATE,
    EVENT_VIZADUUM_CHAOTIC_SHADOWS,
    EVENT_VIZADUUM_BOMBARDMENT,
    EVENT_VIZADUUM_PHASE_SHIFT,
    EVENT_VIZADUUM_STABILIZE_RIFT,
    EVENT_VIZADUUM_FELGUARDS,
    EVENT_SPITTER_SHADOW_PHLEGM,

    EVENT_NIGHTBANE_CHARRED_EARTH,
    EVENT_NIGHTBANE_CINDER_BREATH,
    EVENT_NIGHTBANE_TAIL_SWIPE,
    EVENT_NIGHTBANE_REVERBERATING_SHADOWS,
    EVENT_NIGHTBANE_BURNING_BONES,
    EVENT_NIGHTBANE_IGNITE_SOUL,
    EVENT_NIGHTBANE_CONCENTRATED_POWER,
    EVENT_NIGHTBANE_INFERNAL_POWER,
    EVENT_NIGHTBANE_BELLOWING_ROAR,
    EVENT_NIGHTBANE_RAIN_OF_BONES
};

namespace
{
    float const KarazhanTwoPi = 6.28318531f;

    // hostile faction for the opera cast
    uint32_t const OperaCastFaction = 16;
    uint32_t const OperaRestageDelayMs = 15000;

    struct OperaCastSlot
    {
        uint32_t entry;
        LocationVector position;
    };

    std::vector<OperaCastSlot> const WikketCast =
    {
        { BOSS_GALINDRE, { -10891.5f, -1761.89f, 90.47f, 4.56f } },
        { BOSS_ELFYRA,   { -10896.4f, -1758.50f, 90.48f, 4.58f } }
    };

    std::vector<OperaCastSlot> const WestfallCast =
    {
        { BOSS_TOE_KNEE,             { -10905.0f, -1755.24f, 90.55f, 0.00f } },
        { NPC_GANG_RUFFIAN,          { -10908.7f, -1760.70f, 90.56f, 0.35f } },
        { NPC_GANG_RUFFIAN,          { -10909.4f, -1756.20f, 90.56f, 6.18f } },
        { NPC_GANG_RUFFIAN,          { -10907.4f, -1752.71f, 90.55f, 5.88f } },
        { BOSS_MRRGRIA,              { -10879.0f, -1756.53f, 90.55f, 2.92f } },
        { NPC_SHORELINE_TIDESPEAKER, { -10874.1f, -1756.75f, 90.56f, 2.71f } },
        { NPC_SHORELINE_TIDESPEAKER, { -10872.5f, -1759.25f, 90.56f, 2.68f } },
        { NPC_SHORELINE_TIDESPEAKER, { -10874.5f, -1753.44f, 90.56f, 3.55f } }
    };

    std::vector<OperaCastSlot> const BeautifulBeastCast =
    {
        { BOSS_COGGLESTON,    { -10883.4f, -1752.55f, 90.55f, 3.74f } },
        { BOSS_LUMINORE,      { -10888.2f, -1751.59f, 90.55f, 3.62f } },
        { BOSS_BABBLET,       { -10892.2f, -1749.48f, 90.55f, 3.74f } },
        { BOSS_MRS_CAULDRONS, { -10884.7f, -1757.01f, 90.56f, 3.74f } }
    };

    std::vector<uint32_t> const MoroesGuestEntries =
    {
        NPC_BARONESS_DOROTHEA, NPC_LADY_CATRIONA, NPC_BARON_RAFE, NPC_LADY_KEIRA, NPC_LORD_ROBIN, NPC_LORD_CRISPIN
    };

    enum GuestTargeting : uint8_t
    {
        GUEST_TARGET_VICTIM,
        GUEST_TARGET_RANDOM,
        GUEST_TARGET_SELF,
        GUEST_TARGET_AREA
    };

    struct DinnerGuestAbility
    {
        uint32_t entry;
        uint32_t spellId;
        uint32_t firstMs;
        uint32_t repeatMs;
        GuestTargeting targeting;
    };

    DinnerGuestAbility const DinnerGuestAbilities[] =
    {
        { NPC_BARONESS_DOROTHEA, SPELL_DOROTHEA_ARCANE_BLAST,   1000,  4000,  GUEST_TARGET_RANDOM },
        { NPC_BARONESS_DOROTHEA, SPELL_DOROTHEA_MANA_DRAIN,     9000,  15000, GUEST_TARGET_RANDOM },
        { NPC_LADY_CATRIONA,     SPELL_CATRIONA_SMITE,          1000,  4000,  GUEST_TARGET_VICTIM },
        { NPC_LADY_CATRIONA,     SPELL_CATRIONA_HEALING_STREAM, 20000, 30000, GUEST_TARGET_AREA },
        { NPC_BARON_RAFE,        SPELL_RAFE_IRON_WHIRLWIND,     5000,  10000, GUEST_TARGET_SELF },
        { NPC_LADY_KEIRA,        SPELL_KEIRA_EMPOWERED_ARMS,    10000, 12000, GUEST_TARGET_AREA },
        { NPC_LORD_ROBIN,        SPELL_ROBIN_WHIRLING_EDGE,     1000,  18000, GUEST_TARGET_RANDOM },
        { NPC_LORD_CRISPIN,      SPELL_CRISPIN_WILL_BREAKER,    11000, 18000, GUEST_TARGET_AREA }
    };

    // Guardian's Image spots around the Shade of Medivh
    LocationVector const ShadeImageSpots[] =
    {
        { -4581.10f, -2526.68f, 2876.63f },
        { -4618.89f, -2526.83f, 2876.63f },
        { -4599.80f, -2494.75f, 2876.63f }
    };

    enum ShadeSpecial : uint8_t
    {
        SHADE_SPECIAL_NONE,
        SHADE_SPECIAL_FROST,
        SHADE_SPECIAL_FIRE,
        SHADE_SPECIAL_ARCANE
    };

    // health share in permille, never zero
    uint32_t healthForShare(Creature* pCreature, uint64_t share)
    {
        uint32_t const health = static_cast<uint32_t>(uint64_t(pCreature->getMaxHealth()) * share / 1000);
        return health > 0 ? health : 1;
    }

    uint64_t healthShare(Creature* pCreature)
    {
        uint32_t const maxHealth = pCreature->getMaxHealth();
        return maxHealth > 0 ? uint64_t(pCreature->getHealth()) * 1000 / maxHealth : 0;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class ReturnToKarazhanInstanceScript : public InstanceScript
{
public:
    explicit ReturnToKarazhanInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(ReturnToKarazhanEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ReturnToKarazhanInstanceScript(pMapMgr); }

    // a credited member falling before its partners must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isEncounterDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnPlayerEnter(Player* /*pPlayer*/) override
    {
        if (!mShowOnStage && mRestageDelay == 0)
            stageOperaShow();
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        bool open = false;
        if (!doorShouldBeOpen(pGameObject->getEntry(), open))
            return;

        mDoorGuids.insert(pGameObject->GetNewGUID());
        alignDoor(pGameObject, open);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        refreshDoors();

        if (state != EncounterStates::Failed)
            return;

        if (id == DATA_OPERA_HALL)
        {
            // the show starts over with a fresh cast
            clearOperaShow();
            mRestageDelay = OperaRestageDelayMs;
        }
        else if (id == DATA_MOROES)
        {
            for (Creature* pGuest : getCreatureSetForEntries(MoroesGuestEntries))
            {
                if (pGuest != nullptr && !pGuest->isAlive())
                    pGuest->despawn(1000, 5000);
            }
        }
    }

    void UpdateEvent() override
    {
        if (mRestageDelay == 0)
            return;

        uint32_t const step = getUpdateFrequency();
        if (mRestageDelay > step)
        {
            mRestageDelay -= step;
            return;
        }

        mRestageDelay = 0;
        stageOperaShow();
    }

    void writeSaveDataExtended(std::ostringstream& data) override
    {
        data << mOperaShow;
    }

    void readSaveDataExtended(std::istringstream& data) override
    {
        data >> mOperaShow;
    }

    // members that have to fall for the encounter
    std::vector<Creature*> getMembers(uint32_t encounterId)
    {
        return collect(requiredEntries(encounterId));
    }

    // members and the adds that join a pull
    std::vector<Creature*> getPullGroup(uint32_t encounterId)
    {
        std::vector<uint32_t> entries = requiredEntries(encounterId);
        if (encounterId == DATA_OPERA_HALL && mOperaShow == OPERA_SHOW_WESTFALL)
        {
            entries.push_back(NPC_GANG_RUFFIAN);
            entries.push_back(NPC_SHORELINE_TIDESPEAKER);
        }
        else if (encounterId == DATA_MOROES)
        {
            entries.insert(entries.end(), MoroesGuestEntries.begin(), MoroesGuestEntries.end());
        }

        return collect(entries);
    }

    bool isEncounterDown(uint32_t encounterId, Creature* pExcept)
    {
        for (Creature* pMember : getMembers(encounterId))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    // living creature first, otherwise any corpse
    Creature* findCreature(uint32_t entry)
    {
        Creature* pCorpse = nullptr;
        for (Creature* pCreature : getCreatureSetForEntry(entry))
        {
            if (pCreature == nullptr)
                continue;

            if (pCreature->isAlive())
                return pCreature;

            pCorpse = pCreature;
        }

        return pCorpse;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        for (Creature* pCreature : getCreatureSetForEntry(entry))
        {
            if (pCreature != nullptr && pCreature->isAlive() && pCreature->GetScript() != nullptr)
                pCreature->GetScript()->DoAction(action);
        }
    }

    void sendActionToMembers(uint32_t encounterId, int32_t action)
    {
        for (Creature* pMember : getMembers(encounterId))
        {
            if (pMember->isAlive() && pMember->GetScript() != nullptr)
                pMember->GetScript()->DoAction(action);
        }
    }

private:
    std::vector<uint32_t> requiredEntries(uint32_t encounterId) const
    {
        switch (encounterId)
        {
            case DATA_OPERA_HALL:
            {
                switch (mOperaShow)
                {
                    case OPERA_SHOW_WIKKET:
                        return { BOSS_GALINDRE, BOSS_ELFYRA };
                    case OPERA_SHOW_WESTFALL:
                        return { BOSS_TOE_KNEE, BOSS_MRRGRIA };
                    case OPERA_SHOW_BEAUTIFUL_BEAST:
                        return { BOSS_COGGLESTON, BOSS_LUMINORE, BOSS_BABBLET, BOSS_MRS_CAULDRONS };
                    default:
                        return {};
                }
            }
            case DATA_ATTUMEN:
                return { BOSS_MIDNIGHT, BOSS_ATTUMEN };
            case DATA_MOROES:
                return { BOSS_MOROES };
            default:
                return {};
        }
    }

    std::vector<Creature*> collect(std::vector<uint32_t> const& entries)
    {
        std::vector<Creature*> found;
        if (entries.empty())
            return found;

        for (Creature* pCreature : getCreatureSetForEntries(entries))
        {
            if (pCreature != nullptr)
                found.push_back(pCreature);
        }

        return found;
    }

    std::vector<OperaCastSlot> const& getShowCast(uint32_t show) const
    {
        switch (show)
        {
            case OPERA_SHOW_WIKKET:
                return WikketCast;
            case OPERA_SHOW_WESTFALL:
                return WestfallCast;
            default:
                return BeautifulBeastCast;
        }
    }

    // one of three shows, kept for the lifetime of the instance
    void stageOperaShow()
    {
        if (getBossState(DATA_OPERA_HALL) == EncounterStates::Performed)
            return;

        if (mOperaShow < OPERA_SHOW_WIKKET || mOperaShow > OPERA_SHOW_BEAUTIFUL_BEAST)
            mOperaShow = Util::getRandomUInt(OPERA_SHOW_WIKKET, OPERA_SHOW_BEAUTIFUL_BEAST);

        for (OperaCastSlot const& slot : getShowCast(mOperaShow))
        {
            LocationVector const& pos = slot.position;
            if (Creature* pActor = spawnCreature(slot.entry, pos.x, pos.y, pos.z, pos.o, OperaCastFaction))
                mOperaCast.push_back(pActor->GetNewGUID());
        }

        mShowOnStage = true;
        refreshDoors();
    }

    void clearOperaShow()
    {
        for (WoWGuid const& guid : mOperaCast)
        {
            if (Creature* pActor = getCreatureByGuid(guid))
                pActor->despawn(1000, 0);
        }

        mOperaCast.clear();
        mShowOnStage = false;
        refreshDoors();
    }

    bool isDone(uint32_t id) const { return getBossState(id) == EncounterStates::Performed; }
    bool isFighting(uint32_t id) const { return getBossState(id) == EncounterStates::InProgress; }

    // false for objects that are no encounter door
    bool doorShouldBeOpen(uint32_t entry, bool& open) const
    {
        switch (entry)
        {
            case GO_OPERA_STAGE_CURTAIN:
                open = mShowOnStage || isDone(DATA_OPERA_HALL);
                return true;
            case GO_OPERA_STAGE_DOOR_LEFT:
            case GO_OPERA_BALLROOM_DOOR:
                open = isDone(DATA_OPERA_HALL);
                return true;
            case GO_OPERA_STAGE_DOOR_RIGHT:
                open = !isFighting(DATA_OPERA_HALL);
                return true;
            case GO_MOROES_DOOR_1:
            case GO_MOROES_DOOR_2:
                open = !isFighting(DATA_MOROES);
                return true;
            case GO_CURATOR_WALL:
                open = isDone(DATA_CURATOR);
                return true;
            case GO_MEDIVH_DOOR:
                open = !isFighting(DATA_SHADE_OF_MEDIVH);
                return true;
            case GO_MEDIVH_BOOKCASE:
                open = isDone(DATA_SHADE_OF_MEDIVH);
                return true;
            case GO_VIZADUUM_TOWER_DOOR:
                open = !isFighting(DATA_VIZADUUM);
                return true;
            default:
                return false;
        }
    }

    void refreshDoors()
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            GameObject* pDoor = getGameObjectByGuid(guid);
            bool open = false;
            if (pDoor != nullptr && doorShouldBeOpen(pDoor->getEntry(), open))
                alignDoor(pDoor, open);
        }
    }

    // changes the state only when it differs
    void alignDoor(GameObject* pDoor, bool open)
    {
        bool const isOpen = pDoor->getState() != GO_STATE_CLOSED;
        if (isOpen != open)
            pDoor->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    uint32_t mOperaShow = OPERA_SHOW_NONE;
    bool mShowOnStage = false;
    uint32_t mRestageDelay = 0;
    std::vector<WoWGuid> mOperaCast;
    std::set<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick while in combat

class ReturnToKarazhanCombatAI : public CreatureAIScript
{
public:
    explicit ReturnToKarazhanCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
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

        if (_isCasting() || !getCreature()->isAlive())
            return;

        if (uint32_t eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onReset() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    ReturnToKarazhanInstanceScript* getKarazhan()
    {
        if (getCreature()->GetMapId() != MAP_RETURN_TO_KARAZHAN)
            return nullptr;

        return static_cast<ReturnToKarazhanInstanceScript*>(getInstanceScript());
    }

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // anyone but the tank if possible
    Unit* randomNonTank()
    {
        if (Unit* pTarget = getBestPlayerTarget(TargetFilter_NotCurrent))
            return pTarget;

        return randomPlayer();
    }

    void castOnRandomPlayer(uint32_t spellId, bool triggered = false)
    {
        if (Unit* pTarget = randomPlayer())
            castSpell(pTarget, spellId, triggered);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector nearSelf(float offsetX, float offsetY)
    {
        Creature* pSelf = getCreature();
        return LocationVector(pSelf->GetPositionX() + offsetX, pSelf->GetPositionY() + offsetY, pSelf->GetPositionZ(), pSelf->GetOrientation());
    }

    // random point on a ring around a unit
    LocationVector aroundUnit(Unit* pUnit, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(KarazhanTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(pUnit->GetPositionX() + range * std::cos(angle), pUnit->GetPositionY() + range * std::sin(angle), pUnit->GetPositionZ(), angle);
    }

    void notify(uint32_t entry, int32_t action)
    {
        if (ReturnToKarazhanInstanceScript* pKarazhan = getKarazhan())
            pKarazhan->sendAction(entry, action);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class ReturnToKarazhanBossAI : public ReturnToKarazhanCombatAI
{
public:
    ReturnToKarazhanBossAI(Creature* pCreature, uint32_t encounterId) : ReturnToKarazhanCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        // the rest of the group joins, units waiting off stage stay out
        if (ReturnToKarazhanInstanceScript* pKarazhan = getKarazhan())
        {
            for (Creature* pMember : pKarazhan->getPullGroup(mEncounterId))
            {
                if (pMember != getCreature() && pMember->isAlive() && !pMember->isInCombat() && !pMember->hasUnitFlags(UNIT_FLAG_NOT_SELECTABLE))
                    setZoneWideCombat(pMember);
            }
        }

        ReturnToKarazhanCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (!getCreature()->isAlive())
            return;

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Failed);
        }

        ReturnToKarazhanCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            ReturnToKarazhanInstanceScript* pKarazhan = getKarazhan();
            if (pKarazhan == nullptr || pKarazhan->isEncounterDown(mEncounterId, getCreature()))
                pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }

        onDefeat();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Maiden of Virtue

class ReturnToKarazhanMaidenOfVirtueAI : public ReturnToKarazhanBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanMaidenOfVirtueAI(c); }
    explicit ReturnToKarazhanMaidenOfVirtueAI(Creature* pCreature) : ReturnToKarazhanBossAI(pCreature, DATA_MAIDEN_OF_VIRTUE) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_MAIDEN_HOLY_BOLT, 9000);
        scriptEvents.addEvent(EVENT_MAIDEN_SACRED_GROUND, 11000);
        scriptEvents.addEvent(EVENT_MAIDEN_HOLY_SHOCK, 16000);
        scriptEvents.addEvent(EVENT_MAIDEN_MASS_REPENTANCE, 50000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MAIDEN_HOLY_BOLT:
                castOnRandomPlayer(SPELL_MAIDEN_HOLY_BOLT);
                scriptEvents.addEvent(EVENT_MAIDEN_HOLY_BOLT, 14000);
                break;
            case EVENT_MAIDEN_SACRED_GROUND:
                castOnRandomPlayer(SPELL_MAIDEN_SACRED_GROUND);
                scriptEvents.addEvent(EVENT_MAIDEN_SACRED_GROUND, 23000);
                break;
            case EVENT_MAIDEN_HOLY_SHOCK:
                castSpellOnVictim(SPELL_MAIDEN_HOLY_SHOCK);
                scriptEvents.addEvent(EVENT_MAIDEN_HOLY_SHOCK, 13000);
                break;
            case EVENT_MAIDEN_MASS_REPENTANCE:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Maiden of Virtue begins to cast Mass Repentance!");
                scriptEvents.delayAllEvents(6000);
                castSpellAOE(SPELL_MAIDEN_MASS_REPENTANCE);
                // runs once the repentance cast is through
                scriptEvents.addEvent(EVENT_MAIDEN_HOLY_BULWARK, 100);
                scriptEvents.addEvent(EVENT_MAIDEN_MASS_REPENTANCE, 50000);
                break;
            case EVENT_MAIDEN_HOLY_BULWARK:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Maiden of Virtue shields herself with Holy Bulwark! Break it before Holy Wrath!");
                castSpellOnSelf(SPELL_MAIDEN_HOLY_BULWARK);
                scriptEvents.addEvent(EVENT_MAIDEN_HOLY_WRATH, 1000);
                break;
            case EVENT_MAIDEN_HOLY_WRATH:
                castSpellAOE(SPELL_MAIDEN_HOLY_WRATH);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Opera Hall: Wikket - Galindre and Elfyra share one pool of health

class ReturnToKarazhanWikketAI : public ReturnToKarazhanBossAI
{
public:
    ReturnToKarazhanWikketAI(Creature* pCreature, uint32_t partnerEntry) : ReturnToKarazhanBossAI(pCreature, DATA_OPERA_HALL), mPartnerEntry(partnerEntry) {}

protected:
    void onCombatTick(uint32_t /*diff*/) override { shareHealth(); }

private:
    // the higher share drops to the lower one, a fallen partner takes the other along
    void shareHealth()
    {
        Creature* pPartner = getPartner();
        if (pPartner == nullptr)
            return;

        Creature* pSelf = getCreature();
        if (!pPartner->isAlive())
        {
            pSelf->dealDamage(pSelf, pSelf->getHealth(), 0);
            return;
        }

        uint64_t const partnerShare = healthShare(pPartner);
        if (partnerShare < healthShare(pSelf))
            pSelf->setHealth(healthForShare(pSelf, partnerShare));
    }

    Creature* getPartner()
    {
        ReturnToKarazhanInstanceScript* pKarazhan = getKarazhan();
        if (pKarazhan == nullptr)
            return nullptr;

        if (Creature* pKnown = pKarazhan->getCreatureByGuid(mPartnerGuid))
            return pKnown;

        Creature* pPartner = pKarazhan->findCreature(mPartnerEntry);
        if (pPartner != nullptr)
            mPartnerGuid = pPartner->GetNewGUID();

        return pPartner;
    }

    uint32_t mPartnerEntry;
    WoWGuid mPartnerGuid;
};

class ReturnToKarazhanGalindreAI : public ReturnToKarazhanWikketAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanGalindreAI(c); }
    explicit ReturnToKarazhanGalindreAI(Creature* pCreature) : ReturnToKarazhanWikketAI(pCreature, BOSS_ELFYRA) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GALINDRE_FLASHY_BOLT, 1000);
        scriptEvents.addEvent(EVENT_GALINDRE_WONDROUS_RADIANCE, 8000);
        scriptEvents.addEvent(EVENT_GALINDRE_MAGIC_MAGNIFICENT, 48000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GALINDRE_FLASHY_BOLT:
                castSpellOnVictim(SPELL_GALINDRE_FLASHY_BOLT);
                scriptEvents.addEvent(EVENT_GALINDRE_FLASHY_BOLT, 4000);
                break;
            case EVENT_GALINDRE_WONDROUS_RADIANCE:
                castSpellOnVictim(SPELL_GALINDRE_WONDROUS_RADIANCE);
                scriptEvents.addEvent(EVENT_GALINDRE_WONDROUS_RADIANCE, 10000);
                break;
            case EVENT_GALINDRE_MAGIC_MAGNIFICENT:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Galindre begins to cast Magic Magnificent!");
                castSpellOnSelf(SPELL_GALINDRE_MAGNIFICENT_SHIELD, true);
                castSpellAOE(SPELL_GALINDRE_MAGIC_MAGNIFICENT);
                scriptEvents.addEvent(EVENT_GALINDRE_SHIELD_FADES, 5000);
                scriptEvents.addEvent(EVENT_GALINDRE_MAGIC_MAGNIFICENT, 48000);
                break;
            case EVENT_GALINDRE_SHIELD_FADES:
                _removeAura(SPELL_GALINDRE_MAGNIFICENT_SHIELD);
                break;
            default:
                break;
        }
    }
};

class ReturnToKarazhanElfyraAI : public ReturnToKarazhanWikketAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanElfyraAI(c); }
    explicit ReturnToKarazhanElfyraAI(Creature* pCreature) : ReturnToKarazhanWikketAI(pCreature, BOSS_GALINDRE) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_ELFYRA_DREARY_BOLT, 4000);
        scriptEvents.addEvent(EVENT_ELFYRA_DEFY_GRAVITY, 11000);
        scriptEvents.addEvent(EVENT_ELFYRA_ASSISTANTS, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ELFYRA_DREARY_BOLT:
                castSpellOnVictim(SPELL_ELFYRA_DREARY_BOLT);
                scriptEvents.addEvent(EVENT_ELFYRA_DREARY_BOLT, 4000);
                break;
            case EVENT_ELFYRA_DEFY_GRAVITY:
                castSpellAOE(SPELL_ELFYRA_DEFY_GRAVITY);
                scriptEvents.addEvent(EVENT_ELFYRA_DEFY_GRAVITY, 19000);
                break;
            case EVENT_ELFYRA_ASSISTANTS:
                castSpellAOE(SPELL_ELFYRA_SUMMON_ASSISTANTS);
                for (uint8_t i = 0; i < 3; ++i)
                    callAdd(NPC_WINGED_ASSISTANT, aroundUnit(getCreature(), 5.0f, 10.0f));
                scriptEvents.addEvent(EVENT_ELFYRA_ASSISTANTS, 32000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Opera Hall: Westfall Story - the gang falls back at half health and the murlocs take over

class ReturnToKarazhanToeKneeAI : public ReturnToKarazhanBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanToeKneeAI(c); }
    explicit ReturnToKarazhanToeKneeAI(Creature* pCreature) : ReturnToKarazhanBossAI(pCreature, DATA_OPERA_HALL) {}

    void OnLoad() override { mStagePosition = getCreature()->GetPosition(); }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mBackstage)
        {
            *damage = 0;
            return;
        }

        // holds at half health until the murlocs took over once
        uint32_t const floor = getCreature()->getMaxHealth() / 2;
        uint32_t const health = getCreature()->getHealth();
        if (!mFellBack && health > floor && *damage >= health - floor)
        {
            *damage = health - floor;
            mFallBackPending = true;
        }
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_TOE_KNEE_RETURNS || !mBackstage || !_isInCombat())
            return;

        mBackstage = false;
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Toe Knee rejoins the fight!");
        startRotation();
    }

protected:
    void onEngage() override
    {
        mBackstage = false;
        mFellBack = false;
        mFallBackPending = false;
        startRotation();
    }

    void onReset() override
    {
        mBackstage = false;
        mFallBackPending = false;
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mFallBackPending)
            fallBack();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TOE_KNEE_LEG_SWEEP:
                castSpellAOE(SPELL_TOE_KNEE_BURNING_LEG_SWEEP);
                scriptEvents.addEvent(EVENT_TOE_KNEE_LEG_SWEEP, 20000);
                break;
            case EVENT_TOE_KNEE_FLAME_GALE:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Toe Knee prepares a Dashing Flame Gale!");
                castSpellAOE(SPELL_TOE_KNEE_DASHING_FLAME_GALE);
                scriptEvents.addEvent(EVENT_TOE_KNEE_FLAME_GALE_CHARGE, 100);
                scriptEvents.addEvent(EVENT_TOE_KNEE_FLAME_GALE, 30000);
                break;
            case EVENT_TOE_KNEE_FLAME_GALE_CHARGE:
                castOnRandomPlayer(SPELL_TOE_KNEE_FLAME_GALE_CHARGE);
                break;
            default:
                break;
        }
    }

private:
    void startRotation()
    {
        scriptEvents.addEvent(EVENT_TOE_KNEE_LEG_SWEEP, 10000);
        scriptEvents.addEvent(EVENT_TOE_KNEE_FLAME_GALE, 22000);
    }

    // backstage until Mrrgria is down to half
    void fallBack()
    {
        mFallBackPending = false;
        mFellBack = true;
        mBackstage = true;

        scriptEvents.resetEvents();
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_PASSIVE);
        attackStop();
        stopMovement();
        movePoint(1, mStagePosition);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Toe Knee calls a time out! The Fins take the stage!");

        if (ReturnToKarazhanInstanceScript* pKarazhan = getKarazhan())
        {
            pKarazhan->sendAction(BOSS_MRRGRIA, ACTION_OPERA_RESERVE_JOINS);
            pKarazhan->sendAction(NPC_SHORELINE_TIDESPEAKER, ACTION_OPERA_RESERVE_JOINS);
        }
    }

    LocationVector mStagePosition;
    bool mBackstage = false;
    bool mFellBack = false;
    bool mFallBackPending = false;
};

// shared waiting state of the murloc team
class ReturnToKarazhanFinsAI : public ReturnToKarazhanBossAI
{
public:
    explicit ReturnToKarazhanFinsAI(Creature* pCreature) : ReturnToKarazhanBossAI(pCreature, DATA_OPERA_HALL) {}

    void OnLoad() override { waitOffStage(); }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && mWaiting)
            *damage = 0;
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_OPERA_RESERVE_JOINS || !mWaiting)
            return;

        mWaiting = false;
        setCanEnterCombat(true);
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);

        if (_isInCombat())
            startRotation();
        else
            setZoneWideCombat(getCreature());
    }

protected:
    void onEngage() override
    {
        if (!mWaiting)
            startRotation();
    }

    void onReset() override { waitOffStage(); }

    virtual void startRotation() {}

    bool mWaiting = false;

private:
    void waitOffStage()
    {
        mWaiting = true;
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
    }
};

class ReturnToKarazhanMrrgriaAI : public ReturnToKarazhanFinsAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanMrrgriaAI(c); }
    explicit ReturnToKarazhanMrrgriaAI(Creature* pCreature) : ReturnToKarazhanFinsAI(pCreature) {}

    void DamageTaken(Unit* pAttacker, uint32_t* damage) override
    {
        ReturnToKarazhanFinsAI::DamageTaken(pAttacker, damage);
        if (damage == nullptr || mWaiting || mCalledBack)
            return;

        // Toe Knee comes back once she drops below half
        uint32_t const floor = getCreature()->getMaxHealth() / 2;
        if (getCreature()->getHealth() <= floor + *damage)
        {
            mCalledBack = true;
            notify(BOSS_TOE_KNEE, ACTION_TOE_KNEE_RETURNS);
        }
    }

protected:
    void startRotation() override
    {
        mCalledBack = false;
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Mrrgria and the Fins storm the stage!");
        scriptEvents.addEvent(EVENT_MRRGRIA_THUNDER_RITUAL, 8000);
        scriptEvents.addEvent(EVENT_MRRGRIA_WASH_AWAY, 16000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MRRGRIA_THUNDER_RITUAL:
                castSpellAOE(SPELL_MRRGRIA_THUNDER_RITUAL);
                scriptEvents.addEvent(EVENT_MRRGRIA_THUNDER_RITUAL, 18000);
                break;
            case EVENT_MRRGRIA_WASH_AWAY:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Mrrgria calls a wave to Wash Away the stage!");
                castSpellAOE(SPELL_MRRGRIA_WASH_AWAY);
                scriptEvents.addEvent(EVENT_MRRGRIA_WASH_AWAY, 23000);
                break;
            default:
                break;
        }
    }

private:
    bool mCalledBack = false;
};

// Shoreline Tidespeaker: rooted caster of the murloc team
class ReturnToKarazhanTidespeakerAI : public ReturnToKarazhanCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanTidespeakerAI(c); }
    explicit ReturnToKarazhanTidespeakerAI(Creature* pCreature) : ReturnToKarazhanCombatAI(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        waitOffStage();
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && mWaiting)
            *damage = 0;
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_OPERA_RESERVE_JOINS || !mWaiting)
            return;

        mWaiting = false;
        setCanEnterCombat(true);
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);

        if (_isInCombat())
            onEngage();
        else
            setZoneWideCombat(getCreature());
    }

protected:
    void onEngage() override
    {
        if (!mWaiting)
            scriptEvents.addEvent(EVENT_TIDESPEAKER_BUBBLE_BLAST, 5000);
    }

    void onReset() override { waitOffStage(); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_TIDESPEAKER_BUBBLE_BLAST)
            return;

        castOnRandomPlayer(SPELL_TIDESPEAKER_BUBBLE_BLAST, true);
        scriptEvents.addEvent(EVENT_TIDESPEAKER_BUBBLE_BLAST, 5000);
    }

private:
    void waitOffStage()
    {
        mWaiting = true;
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
    }

    bool mWaiting = false;
};

class ReturnToKarazhanGangRuffianAI : public ReturnToKarazhanCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanGangRuffianAI(c); }
    explicit ReturnToKarazhanGangRuffianAI(Creature* pCreature) : ReturnToKarazhanCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_RUFFIAN_SHANK, Util::getRandomUInt(5000, 8000));
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_RUFFIAN_SHANK)
            return;

        castOnRandomPlayer(SPELL_RUFFIAN_POISONOUS_SHANK);
        scriptEvents.addEvent(EVENT_RUFFIAN_SHANK, 10000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Opera Hall: Beautiful Beast - Coggleston serves until the staff is gone

class ReturnToKarazhanBeastCastAI : public ReturnToKarazhanBossAI
{
public:
    explicit ReturnToKarazhanBeastCastAI(Creature* pCreature) : ReturnToKarazhanBossAI(pCreature, DATA_OPERA_HALL) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_BEAST_CAST_FALLEN || !_isInCombat())
            return;

        castSpellOnSelf(SPELL_BEAST_EMINENCE, true);
        onCastFallen();
    }

protected:
    // the survivors grow stronger
    void onDefeat() override
    {
        if (ReturnToKarazhanInstanceScript* pKarazhan = getKarazhan())
            pKarazhan->sendActionToMembers(DATA_OPERA_HALL, ACTION_BEAST_CAST_FALLEN);
    }

    virtual void onCastFallen() {}
};

class ReturnToKarazhanCogglestonAI : public ReturnToKarazhanBeastCastAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanCogglestonAI(c); }
    explicit ReturnToKarazhanCogglestonAI(Creature* pCreature) : ReturnToKarazhanBeastCastAI(pCreature) {}

    void OnLoad() override { serve(); }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && mServing)
            *damage = 0;
    }

protected:
    void onEngage() override
    {
        if (mServing)
            castSpellOnSelf(SPELL_COGGLESTON_SPECTRAL_SERVICE, true);
    }

    void onReset() override { serve(); }

    void onCastFallen() override
    {
        if (!mServing)
            return;

        ReturnToKarazhanInstanceScript* pKarazhan = getKarazhan();
        if (pKarazhan == nullptr)
            return;

        for (Creature* pMember : pKarazhan->getMembers(DATA_OPERA_HALL))
        {
            if (pMember != getCreature() && pMember->isAlive())
                return;
        }

        stepIn();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_COGGLESTON_DENT_ARMOR:
                castSpellOnVictim(SPELL_COGGLESTON_DENT_ARMOR);
                scriptEvents.addEvent(EVENT_COGGLESTON_DENT_ARMOR, 9000);
                break;
            case EVENT_COGGLESTON_KARA_KAZHAM:
                castSpellAOE(SPELL_COGGLESTON_KARA_KAZHAM);
                scriptEvents.addEvent(EVENT_COGGLESTON_KARA_KAZHAM, 20000);
                break;
            case EVENT_COGGLESTON_DINNER_BELL:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Coggleston rings the Dinner Bell!");
                castSpellAOE(SPELL_COGGLESTON_DINNER_BELL);
                scriptEvents.addEvent(EVENT_COGGLESTON_DINNER_BELL, 20000);
                break;
            default:
                break;
        }
    }

private:
    void serve()
    {
        mServing = true;
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_COGGLESTON_SPECTRAL_SERVICE, true);
    }

    void stepIn()
    {
        mServing = false;
        _removeAura(SPELL_COGGLESTON_SPECTRAL_SERVICE);
        _setMeleeDisabled(false);
        setReactState(REACT_AGGRESSIVE);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Coggleston joins the fight!");

        scriptEvents.addEvent(EVENT_COGGLESTON_KARA_KAZHAM, 2000);
        scriptEvents.addEvent(EVENT_COGGLESTON_DENT_ARMOR, 9000);
        scriptEvents.addEvent(EVENT_COGGLESTON_DINNER_BELL, 12000);
    }

    bool mServing = true;
};

class ReturnToKarazhanLuminoreAI : public ReturnToKarazhanBeastCastAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanLuminoreAI(c); }
    explicit ReturnToKarazhanLuminoreAI(Creature* pCreature) : ReturnToKarazhanBeastCastAI(pCreature) {}

    void OnLoad() override { castSpellOnSelf(SPELL_LUMINORE_CANDLE_HAT, true); }

protected:
    void onEngage() override
    {
        mBlazeCount = 0;
        scriptEvents.addEvent(EVENT_LUMINORE_BURNING_BLAZE, 2000);
        scriptEvents.addEvent(EVENT_LUMINORE_HEAT_WAVE, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LUMINORE_BURNING_BLAZE:
                castOnRandomPlayer(SPELL_LUMINORE_BURNING_BLAZE);
                // three blazes, then a short pause
                if (++mBlazeCount >= 3)
                {
                    mBlazeCount = 0;
                    scriptEvents.addEvent(EVENT_LUMINORE_BURNING_BLAZE, 9000);
                }
                else
                {
                    scriptEvents.addEvent(EVENT_LUMINORE_BURNING_BLAZE, 3000);
                }
                break;
            case EVENT_LUMINORE_HEAT_WAVE:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Luminore begins to cast Heat Wave!");
                castSpellAOE(SPELL_LUMINORE_HEAT_WAVE);
                scriptEvents.addEvent(EVENT_LUMINORE_HEAT_WAVE, 30000);
                break;
            default:
                break;
        }
    }

private:
    uint8_t mBlazeCount = 0;
};

class ReturnToKarazhanBabbletAI : public ReturnToKarazhanBeastCastAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanBabbletAI(c); }
    explicit ReturnToKarazhanBabbletAI(Creature* pCreature) : ReturnToKarazhanBeastCastAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_BABBLET_SEVERE_DUSTING, true);
        scriptEvents.addEvent(EVENT_BABBLET_FIXATE, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_BABBLET_FIXATE)
            return;

        // chases one player at a time
        if (Unit* pTarget = randomPlayer())
        {
            castSpell(pTarget, SPELL_BABBLET_DUSTING_FIXATE, true);
            _clearHateList();
            addThreat(pTarget, 1000000.0f);
        }

        scriptEvents.addEvent(EVENT_BABBLET_FIXATE, 11000);
    }
};

class ReturnToKarazhanMrsCauldronsAI : public ReturnToKarazhanBeastCastAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanMrsCauldronsAI(c); }
    explicit ReturnToKarazhanMrsCauldronsAI(Creature* pCreature) : ReturnToKarazhanBeastCastAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_CAULDRONS_SOUP_SPRAY, 1000);
        scriptEvents.addEvent(EVENT_CAULDRONS_LEFTOVERS, 8000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CAULDRONS_SOUP_SPRAY:
                castOnRandomPlayer(SPELL_CAULDRONS_SOUP_SPRAY);
                scriptEvents.addEvent(EVENT_CAULDRONS_SOUP_SPRAY, 3000);
                break;
            case EVENT_CAULDRONS_LEFTOVERS:
                castSpellAOE(SPELL_CAULDRONS_LEFTOVERS);
                scriptEvents.addEvent(EVENT_CAULDRONS_LEFTOVERS, 18000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mana Devourer: 5 energy per 3s, Coalesce Power drains it into Loose Mana, full bar wipes

class ReturnToKarazhanManaDevourerAI : public ReturnToKarazhanBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanManaDevourerAI(c); }
    explicit ReturnToKarazhanManaDevourerAI(Creature* pCreature) : ReturnToKarazhanBossAI(pCreature, DATA_MANA_DEVOURER) {}

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        mDrainClock = 0;
        mCoalescing = false;

        scriptEvents.addEvent(EVENT_DEVOURER_ARCANE_BOMB, 8000);
        scriptEvents.addEvent(EVENT_DEVOURER_ENERGY_VOID, 15000);
        scriptEvents.addEvent(EVENT_DEVOURER_ENERGY_DISCHARGE, 22000);
        scriptEvents.addEvent(EVENT_DEVOURER_COALESCE_POWER, 30000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mCoalescing)
        {
            // 5 energy per 250ms leave as Loose Mana
            mDrainClock += diff;
            while (mDrainClock >= 250)
            {
                mDrainClock -= 250;
                if (mEnergy < 5)
                {
                    mCoalescing = false;
                    break;
                }

                mEnergy -= 5;
                castSpellOnSelf(SPELL_DEVOURER_LOOSE_MANA, true);
            }
            return;
        }

        mEnergyClock += diff;
        while (mEnergyClock >= 3000)
        {
            mEnergyClock -= 3000;
            mEnergy += 5;
        }

        if (mEnergy >= 100 && !_isCasting())
        {
            mEnergy = 0;
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "The Mana Devourer is overflowing with energy and casts Decimating Essence!");
            castSpellAOE(SPELL_DEVOURER_DECIMATING_ESSENCE);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DEVOURER_ARCANE_BOMB:
                castOnRandomPlayer(SPELL_DEVOURER_ARCANE_BOMB);
                scriptEvents.addEvent(EVENT_DEVOURER_ARCANE_BOMB, 14000);
                break;
            case EVENT_DEVOURER_ENERGY_VOID:
                castOnRandomPlayer(SPELL_DEVOURER_ENERGY_VOID);
                scriptEvents.addEvent(EVENT_DEVOURER_ENERGY_VOID, 22000);
                break;
            case EVENT_DEVOURER_ENERGY_DISCHARGE:
                castSpellOnSelf(SPELL_DEVOURER_ENERGY_DISCHARGE, true);
                scriptEvents.addEvent(EVENT_DEVOURER_ENERGY_DISCHARGE, 27000);
                break;
            case EVENT_DEVOURER_COALESCE_POWER:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "The Mana Devourer begins to Coalesce Power! Soak the Loose Mana!");
                castSpellOnSelf(SPELL_DEVOURER_COALESCE_POWER, true);
                mCoalescing = true;
                mDrainClock = 0;
                scriptEvents.addEvent(EVENT_DEVOURER_COALESCE_POWER, 30000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    uint32_t mDrainClock = 0;
    bool mCoalescing = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Attumen the Huntsman: Midnight carries him until half health, then he fights on foot

class ReturnToKarazhanMidnightAI : public ReturnToKarazhanBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanMidnightAI(c); }
    explicit ReturnToKarazhanMidnightAI(Creature* pCreature) : ReturnToKarazhanBossAI(pCreature, DATA_ATTUMEN) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mAttumenSlain)
            return;

        // cannot drop below half while the rider lives
        uint32_t const floor = getCreature()->getMaxHealth() / 2;
        uint32_t const health = getCreature()->getHealth();
        if (*damage >= health || health - *damage < floor)
        {
            *damage = health > floor ? health - floor : 0;
            if (!mRiderless)
                mRiderlessPending = true;
        }
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_ATTUMEN_SLAIN || mAttumenSlain || !_isInCombat())
            return;

        mAttumenSlain = true;
        if (mRiderless)
        {
            mRiderless = false;
            _removeAura(SPELL_MIDNIGHT_RIDERLESS);
            scriptEvents.removeEvent(EVENT_MIDNIGHT_MEZAIR);
        }

        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Midnight becomes enraged at the loss of his master!");
        castSpellOnSelf(SPELL_MIDNIGHT_ENRAGE, true);
        mountedRotation();
    }

protected:
    void onEngage() override
    {
        resetRide();
        mountedRotation();
    }

    void onReset() override { resetRide(); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mRiderlessPending)
            throwRider();
        else if (mRiderless && !mAttumenSlain && _getHealthPercent() >= 100)
            remount();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MIDNIGHT_INTANGIBLE_PRESENCE:
                castSpellAOE(SPELL_MIDNIGHT_INTANGIBLE_PRESENCE);
                scriptEvents.addEvent(EVENT_MIDNIGHT_INTANGIBLE_PRESENCE, 58000);
                break;
            case EVENT_MIDNIGHT_MIGHTY_STOMP:
                castSpellAOE(SPELL_MIDNIGHT_MIGHTY_STOMP);
                scriptEvents.addEvent(EVENT_MIDNIGHT_MIGHTY_STOMP, 14000);
                break;
            case EVENT_MIDNIGHT_MEZAIR:
                castOnRandomPlayer(SPELL_MIDNIGHT_MEZAIR);
                scriptEvents.addEvent(EVENT_MIDNIGHT_MEZAIR, 6000);
                break;
            default:
                break;
        }
    }

private:
    void resetRide()
    {
        mRiderless = false;
        mRiderlessPending = false;
        mAttumenSlain = false;
        mAttumenHealthPct = 100;
    }

    void mountedRotation()
    {
        scriptEvents.addEvent(EVENT_MIDNIGHT_INTANGIBLE_PRESENCE, 6000);
        scriptEvents.addEvent(EVENT_MIDNIGHT_MIGHTY_STOMP, 14000);
    }

    // Attumen takes the field with the health he left with
    void throwRider()
    {
        mRiderlessPending = false;
        mRiderless = true;

        scriptEvents.removeEvent(EVENT_MIDNIGHT_INTANGIBLE_PRESENCE);
        scriptEvents.removeEvent(EVENT_MIDNIGHT_MIGHTY_STOMP);
        castSpellOnSelf(SPELL_MIDNIGHT_RIDERLESS, true);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Attumen the Huntsman is thrown from Midnight!");

        if (Creature* pAttumen = callAdd(BOSS_ATTUMEN, nearSelf(3.0f, 3.0f)))
            pAttumen->setHealth(healthForShare(pAttumen, uint64_t(mAttumenHealthPct) * 10));

        scriptEvents.addEvent(EVENT_MIDNIGHT_MEZAIR, 3000);
    }

    void remount()
    {
        ReturnToKarazhanInstanceScript* pKarazhan = getKarazhan();
        Creature* pAttumen = pKarazhan != nullptr ? pKarazhan->findCreature(BOSS_ATTUMEN) : nullptr;
        if (pAttumen != nullptr && pAttumen->isAlive())
        {
            mAttumenHealthPct = static_cast<uint32_t>(healthShare(pAttumen) / 10);
            if (mAttumenHealthPct == 0)
                mAttumenHealthPct = 1;

            if (pAttumen->GetScript() != nullptr)
                pAttumen->GetScript()->DoAction(ACTION_ATTUMEN_REMOUNT);
        }

        mRiderless = false;
        _removeAura(SPELL_MIDNIGHT_RIDERLESS);
        scriptEvents.removeEvent(EVENT_MIDNIGHT_MEZAIR);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Attumen the Huntsman mounts Midnight again!");
        mountedRotation();
    }

    bool mRiderless = false;
    bool mRiderlessPending = false;
    bool mAttumenSlain = false;
    uint32_t mAttumenHealthPct = 100;
};

class ReturnToKarazhanAttumenAI : public ReturnToKarazhanBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanAttumenAI(c); }
    explicit ReturnToKarazhanAttumenAI(Creature* pCreature) : ReturnToKarazhanBossAI(pCreature, DATA_ATTUMEN) {}

    void OnCombatStop(Unit* pTarget) override
    {
        // leaving for the saddle is no wipe
        if (mRemounting)
        {
            scriptEvents.resetEvents();
            return;
        }

        ReturnToKarazhanBossAI::OnCombatStop(pTarget);
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_ATTUMEN_REMOUNT || mRemounting)
            return;

        mRemounting = true;
        scriptEvents.resetEvents();
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        despawn(100, 0);
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_ATTUMEN_MORTAL_STRIKE, 10000);
        scriptEvents.addEvent(EVENT_ATTUMEN_SHARED_SUFFERING, 18000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ATTUMEN_MORTAL_STRIKE:
                castSpellOnVictim(SPELL_ATTUMEN_MORTAL_STRIKE);
                scriptEvents.addEvent(EVENT_ATTUMEN_MORTAL_STRIKE, 16000);
                break;
            case EVENT_ATTUMEN_SHARED_SUFFERING:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Attumen the Huntsman casts Shared Suffering!");
                castSpellOnVictim(SPELL_ATTUMEN_SHARED_SUFFERING);
                scriptEvents.addEvent(EVENT_ATTUMEN_SHARED_SUFFERING, 18000);
                break;
            default:
                break;
        }
    }

    void onDefeat() override { notify(BOSS_MIDNIGHT, ACTION_ATTUMEN_SLAIN); }

private:
    bool mRemounting = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Moroes and his dinner guests

class ReturnToKarazhanMoroesAI : public ReturnToKarazhanBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanMoroesAI(c); }
    explicit ReturnToKarazhanMoroesAI(Creature* pCreature) : ReturnToKarazhanBossAI(pCreature, DATA_MOROES) {}

protected:
    void onEngage() override
    {
        mPurged = false;
        scriptEvents.addEvent(EVENT_MOROES_VANISH, 7000);
        scriptEvents.addEvent(EVENT_MOROES_COAT_CHECK, 30000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mPurged && _getHealthPercent() <= 50)
        {
            mPurged = true;
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Moroes casts Ghastly Purge!");
            castSpellOnSelf(SPELL_MOROES_GHASTLY_PURGE, true);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MOROES_VANISH:
                castSpellOnSelf(SPELL_MOROES_VANISH, true);
                scriptEvents.addEvent(EVENT_MOROES_GARROTE, 1500);
                scriptEvents.addEvent(EVENT_MOROES_VANISH, 19000);
                break;
            case EVENT_MOROES_GARROTE:
                castOnRandomPlayer(SPELL_MOROES_GARROTE, true);
                break;
            case EVENT_MOROES_COAT_CHECK:
                castSpellOnVictim(SPELL_MOROES_COAT_CHECK);
                scriptEvents.addEvent(EVENT_MOROES_COAT_CHECK, 30000);
                break;
            default:
                break;
        }
    }

private:
    bool mPurged = false;
};

class ReturnToKarazhanDinnerGuestAI : public ReturnToKarazhanCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanDinnerGuestAI(c); }
    explicit ReturnToKarazhanDinnerGuestAI(Creature* pCreature) : ReturnToKarazhanCombatAI(pCreature)
    {
        for (DinnerGuestAbility const& ability : DinnerGuestAbilities)
        {
            if (ability.entry == pCreature->getEntry() && mAbilities.size() < 2)
                mAbilities.push_back(&ability);
        }
    }

    void OnLoad() override { castSpellOnSelf(SPELL_GUEST_DINNER_PARTY, true); }

    void OnCombatStart(Unit* pTarget) override
    {
        // the host answers for his guests
        if (ReturnToKarazhanInstanceScript* pKarazhan = getKarazhan())
        {
            Creature* pMoroes = pKarazhan->findCreature(BOSS_MOROES);
            if (pMoroes != nullptr && pMoroes->isAlive() && !pMoroes->isInCombat())
                setZoneWideCombat(pMoroes);
        }

        ReturnToKarazhanCombatAI::OnCombatStart(pTarget);
    }

protected:
    void onEngage() override
    {
        for (size_t i = 0; i < mAbilities.size(); ++i)
            scriptEvents.addEvent(EVENT_GUEST_ABILITY_1 + static_cast<uint32_t>(i), mAbilities[i]->firstMs);
    }

    void onEvent(uint32_t eventId) override
    {
        size_t const slot = eventId - EVENT_GUEST_ABILITY_1;
        if (eventId < EVENT_GUEST_ABILITY_1 || slot >= mAbilities.size())
            return;

        DinnerGuestAbility const* pAbility = mAbilities[slot];
        switch (pAbility->targeting)
        {
            case GUEST_TARGET_VICTIM:
                castSpellOnVictim(pAbility->spellId);
                break;
            case GUEST_TARGET_RANDOM:
                castOnRandomPlayer(pAbility->spellId);
                break;
            case GUEST_TARGET_SELF:
                castSpellOnSelf(pAbility->spellId, true);
                break;
            default:
                castSpellAOE(pAbility->spellId);
                break;
        }

        scriptEvents.addEvent(eventId, pAbility->repeatMs);
    }

private:
    std::vector<DinnerGuestAbility const*> mAbilities;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Curator: mana drains in about 62s, Evocation refills it over 20s

class ReturnToKarazhanCuratorAI : public ReturnToKarazhanBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanCuratorAI(c); }
    explicit ReturnToKarazhanCuratorAI(Creature* pCreature) : ReturnToKarazhanBossAI(pCreature, DATA_CURATOR) {}

protected:
    void onEngage() override { drainRotation(); }

    void onReset() override { _setMeleeDisabled(false); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CURATOR_VOLATILE_ENERGY:
                callAdd(NPC_VOLATILE_ENERGY, aroundUnit(getCreature(), 8.0f, 15.0f));
                scriptEvents.addEvent(EVENT_CURATOR_VOLATILE_ENERGY, 10000);
                break;
            case EVENT_CURATOR_POWER_DISCHARGE:
                castSpellAOE(SPELL_CURATOR_POWER_DISCHARGE);
                scriptEvents.addEvent(EVENT_CURATOR_POWER_DISCHARGE, 14000);
                break;
            case EVENT_CURATOR_EVOCATION:
                scriptEvents.removeEvent(EVENT_CURATOR_VOLATILE_ENERGY);
                scriptEvents.removeEvent(EVENT_CURATOR_POWER_DISCHARGE);
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "The Curator is out of mana and begins to Evocate!");
                attackStop();
                _setMeleeDisabled(true);
                castSpellOnSelf(SPELL_CURATOR_EVOCATION, true);
                scriptEvents.addEvent(EVENT_CURATOR_OVERLOAD, 20000);
                break;
            case EVENT_CURATOR_OVERLOAD:
                _removeAura(SPELL_CURATOR_EVOCATION);
                _setMeleeDisabled(false);
                castSpellOnSelf(SPELL_CURATOR_OVERLOAD, true);
                drainRotation();
                break;
            default:
                break;
        }
    }

private:
    void drainRotation()
    {
        scriptEvents.addEvent(EVENT_CURATOR_VOLATILE_ENERGY, 6000);
        scriptEvents.addEvent(EVENT_CURATOR_POWER_DISCHARGE, 13000);
        scriptEvents.addEvent(EVENT_CURATOR_EVOCATION, 62000);
    }
};

class ReturnToKarazhanVolatileEnergyAI : public ReturnToKarazhanCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanVolatileEnergyAI(c); }
    explicit ReturnToKarazhanVolatileEnergyAI(Creature* pCreature) : ReturnToKarazhanCombatAI(pCreature) {}

    void OnLoad() override { castSpellOnSelf(SPELL_VOLATILE_ENERGY_PASSIVE, true); }

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_VOLATILE_STATIC_CHARGE, 7000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_VOLATILE_STATIC_CHARGE)
            return;

        castSpellAOE(SPELL_VOLATILE_ENERGY_STATIC_CHARGE, true);
        scriptEvents.addEvent(EVENT_VOLATILE_STATIC_CHARGE, 12000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shade of Medivh: mana fills in 3s steps of 4/3/3, a full bar casts one of three specials

class ReturnToKarazhanShadeOfMedivhAI : public ReturnToKarazhanBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanShadeOfMedivhAI(c); }
    explicit ReturnToKarazhanShadeOfMedivhAI(Creature* pCreature) : ReturnToKarazhanBossAI(pCreature, DATA_SHADE_OF_MEDIVH) {}

    void OnLoad() override { setRooted(true); }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && mSpecial == SHADE_SPECIAL_ARCANE)
            *damage = 0;
    }

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || pSummon->getEntry() != NPC_GUARDIANS_IMAGE || mSpecial != SHADE_SPECIAL_ARCANE)
            return;

        if (mImagesLeft > 0 && --mImagesLeft == 0)
            reform();
    }

protected:
    void onEngage() override
    {
        mMana = 0;
        mManaTicks = 0;
        mManaClock = 0;
        mSpecial = SHADE_SPECIAL_NONE;
        mSpecialCast = false;
        mImagesLeft = 0;
        scriptEvents.addEvent(EVENT_SHADE_BASIC_SPELL, 500);
    }

    void onReset() override
    {
        mSpecial = SHADE_SPECIAL_NONE;
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mSpecial == SHADE_SPECIAL_NONE)
        {
            mManaClock += diff;
            while (mManaClock >= 1000)
            {
                mManaClock -= 1000;
                mMana += (mManaTicks++ % 3 == 0) ? 4 : 3;
            }

            if (mMana >= 100 && !_isCasting())
                beginSpecial();
            return;
        }

        // frost and fire end with their cast
        if (mSpecial != SHADE_SPECIAL_ARCANE && mSpecialCast && !_isCasting())
            endSpecial();
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SHADE_BASIC_SPELL)
            return;

        switch (Util::getRandomUInt(2))
        {
            case 0:
                if (Unit* pTarget = randomNonTank())
                    castSpell(pTarget, SPELL_SHADE_FROSTBITE);
                break;
            case 1:
                if (Unit* pTarget = randomNonTank())
                    castSpell(pTarget, SPELL_SHADE_INFERNO_BOLT);
                break;
            default:
                castSpellOnVictim(SPELL_SHADE_PIERCING_MISSILES);
                break;
        }

        scriptEvents.addEvent(EVENT_SHADE_BASIC_SPELL, 4000);
    }

private:
    void beginSpecial()
    {
        mMana = 100;
        mSpecialCast = false;

        switch (Util::getRandomUInt(2))
        {
            case 0:
                mSpecial = SHADE_SPECIAL_FROST;
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "The Shade of Medivh begins to channel Ceaseless Winter! Keep moving!");
                castSpellAOE(SPELL_SHADE_CEASELESS_WINTER);
                mSpecialCast = true;
                break;
            case 1:
                mSpecial = SHADE_SPECIAL_FIRE;
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "The Shade of Medivh begins to cast Flame Wreath! Do not move!");
                castSpellAOE(SPELL_SHADE_FLAME_WREATH);
                mSpecialCast = true;
                break;
            default:
                dissolve();
                break;
        }
    }

    void endSpecial()
    {
        mSpecial = SHADE_SPECIAL_NONE;
        mSpecialCast = false;
        mMana = 0;
        mManaClock = 0;
    }

    // hides until three Guardian's Images are destroyed
    void dissolve()
    {
        mSpecial = SHADE_SPECIAL_ARCANE;
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "The Shade of Medivh dissolves into three Guardian's Images!");
        scriptEvents.removeEvent(EVENT_SHADE_BASIC_SPELL);
        castSpellOnSelf(SPELL_SHADE_DISSOLVE, true);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_PASSIVE);
        attackStop();

        mImagesLeft = 0;
        for (LocationVector const& spot : ShadeImageSpots)
        {
            if (callAdd(NPC_GUARDIANS_IMAGE, spot) != nullptr)
                ++mImagesLeft;
        }

        if (mImagesLeft == 0)
            reform();
    }

    void reform()
    {
        _removeAura(SPELL_SHADE_DISSOLVE);
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        endSpecial();
        scriptEvents.addEvent(EVENT_SHADE_BASIC_SPELL, 500);
    }

    uint32_t mMana = 0;
    uint32_t mManaTicks = 0;
    uint32_t mManaClock = 0;
    uint8_t mSpecial = SHADE_SPECIAL_NONE;
    bool mSpecialCast = false;
    uint32_t mImagesLeft = 0;
};

class ReturnToKarazhanGuardiansImageAI : public ReturnToKarazhanCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanGuardiansImageAI(c); }
    explicit ReturnToKarazhanGuardiansImageAI(Creature* pCreature) : ReturnToKarazhanCombatAI(pCreature) {}

    void OnLoad() override { setRooted(true); }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_IMAGE_SHIMMER, 2000);
        scriptEvents.addEvent(EVENT_IMAGE_ARCANE_BOLT, 2000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_IMAGE_SHIMMER:
                castSpellOnSelf(SPELL_IMAGE_SHIMMER, true);
                break;
            case EVENT_IMAGE_ARCANE_BOLT:
                castOnRandomPlayer(SPELL_IMAGE_ARCANE_BOLT);
                scriptEvents.addEvent(EVENT_IMAGE_ARCANE_BOLT, 2000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Viz'aduum the Watcher: three stages at 66% and 33%

class ReturnToKarazhanVizaduumAI : public ReturnToKarazhanBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanVizaduumAI(c); }
    explicit ReturnToKarazhanVizaduumAI(Creature* pCreature) : ReturnToKarazhanBossAI(pCreature, DATA_VIZADUUM) {}

protected:
    void onEngage() override
    {
        mStage = 1;
        scriptEvents.addEvent(EVENT_VIZADUUM_BURNING_BLAST, 2000);
        scriptEvents.addEvent(EVENT_VIZADUUM_FEL_BEAM, 6000);
        scriptEvents.addEvent(EVENT_VIZADUUM_DISINTEGRATE, 11000);
        scriptEvents.addEvent(EVENT_VIZADUUM_CHAOTIC_SHADOWS, 16000);
        scriptEvents.addEvent(EVENT_VIZADUUM_BOMBARDMENT, 26000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if ((mStage == 1 && _getHealthPercent() <= 66) || (mStage == 2 && _getHealthPercent() <= 33))
            shiftStage();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VIZADUUM_BURNING_BLAST:
                castSpellOnVictim(SPELL_VIZADUUM_BURNING_BLAST);
                scriptEvents.addEvent(EVENT_VIZADUUM_BURNING_BLAST, 5000);
                break;
            case EVENT_VIZADUUM_FEL_BEAM:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Viz'aduum commands a Fel Beam!");
                castSpellAOE(SPELL_VIZADUUM_COMMAND_FEL_BEAM);
                scriptEvents.addEvent(EVENT_VIZADUUM_FEL_BEAM, 40000);
                break;
            case EVENT_VIZADUUM_DISINTEGRATE:
                castOnRandomPlayer(SPELL_VIZADUUM_DISINTEGRATE);
                scriptEvents.addEvent(EVENT_VIZADUUM_DISINTEGRATE, 11000);
                break;
            case EVENT_VIZADUUM_CHAOTIC_SHADOWS:
                castSpellAOE(SPELL_VIZADUUM_CHAOTIC_SHADOWS);
                scriptEvents.addEvent(EVENT_VIZADUUM_CHAOTIC_SHADOWS, 30000);
                break;
            case EVENT_VIZADUUM_BOMBARDMENT:
                castSpellAOE(SPELL_VIZADUUM_COMMAND_BOMBARDMENT);
                scriptEvents.addEvent(EVENT_VIZADUUM_BOMBARDMENT, 25000);
                break;
            case EVENT_VIZADUUM_PHASE_SHIFT:
                startStage();
                break;
            case EVENT_VIZADUUM_STABILIZE_RIFT:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Viz'aduum begins to Stabilize the Rift! Interrupt him!");
                castSpellAOE(SPELL_VIZADUUM_STABILIZE_RIFT);
                break;
            case EVENT_VIZADUUM_FELGUARDS:
                for (uint8_t i = 0; i < 2; ++i)
                {
                    if (Creature* pSentry = callAdd(NPC_FELGUARD_SENTRY, aroundUnit(getCreature(), 10.0f, 15.0f)))
                        pSentry->castSpell(pSentry, SPELL_FELGUARD_LEGION_SPAWN, true);
                }
                scriptEvents.addEvent(EVENT_VIZADUUM_FELGUARDS, 30000);
                break;
            default:
                break;
        }
    }

private:
    // short pause through the portal before the next stage
    void shiftStage()
    {
        ++mStage;
        scriptEvents.resetEvents();
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Viz'aduum opens a Demonic Portal!");
        scriptEvents.addEvent(EVENT_VIZADUUM_PHASE_SHIFT, 4000);
    }

    void startStage()
    {
        scriptEvents.addEvent(EVENT_VIZADUUM_BURNING_BLAST, 2000);
        scriptEvents.addEvent(EVENT_VIZADUUM_DISINTEGRATE, 11000);
        scriptEvents.addEvent(EVENT_VIZADUUM_CHAOTIC_SHADOWS, 16000);

        if (mStage == 2)
        {
            scriptEvents.addEvent(EVENT_VIZADUUM_BOMBARDMENT, 26000);
            return;
        }

        scriptEvents.addEvent(EVENT_VIZADUUM_STABILIZE_RIFT, 2000);
        scriptEvents.addEvent(EVENT_VIZADUUM_FELGUARDS, 2000);
        for (uint8_t i = 0; i < 4; ++i)
            callAdd(NPC_SHADOW_SPITTER, aroundUnit(getCreature(), 15.0f, 25.0f));
    }

    uint8_t mStage = 1;
};

class ReturnToKarazhanShadowSpitterAI : public ReturnToKarazhanCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanShadowSpitterAI(c); }
    explicit ReturnToKarazhanShadowSpitterAI(Creature* pCreature) : ReturnToKarazhanCombatAI(pCreature) {}

    void OnLoad() override { setRooted(true); }

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_SPITTER_SHADOW_PHLEGM, 3000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SPITTER_SHADOW_PHLEGM)
            return;

        castOnRandomPlayer(SPELL_SPITTER_SHADOW_PHLEGM, true);
        scriptEvents.addEvent(EVENT_SPITTER_SHADOW_PHLEGM, 5000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nightbane: takes off at half health until Bonecurse falls

class ReturnToKarazhanNightbaneAI : public ReturnToKarazhanBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ReturnToKarazhanNightbaneAI(c); }
    explicit ReturnToKarazhanNightbaneAI(Creature* pCreature) : ReturnToKarazhanBossAI(pCreature, DATA_NIGHTBANE) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && mAirborne)
            *damage = 0;
    }

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon != nullptr && pSummon->getEntry() == NPC_BONECURSE && mAirborne)
            land();
    }

protected:
    void onEngage() override
    {
        mAirborne = false;
        mTookOff = false;
        groundRotation(false);
    }

    void onReset() override
    {
        mAirborne = false;
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        setFlyMode(false);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mTookOff && _getHealthPercent() <= 50)
            takeOff();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NIGHTBANE_CHARRED_EARTH:
                castSpellAOE(SPELL_NIGHTBANE_CHARRED_EARTH);
                break;
            case EVENT_NIGHTBANE_CINDER_BREATH:
                castSpellOnVictim(SPELL_NIGHTBANE_CINDER_BREATH);
                scriptEvents.addEvent(EVENT_NIGHTBANE_CINDER_BREATH, 20000);
                break;
            case EVENT_NIGHTBANE_TAIL_SWIPE:
                castSpellAOE(SPELL_NIGHTBANE_TAIL_SWIPE);
                scriptEvents.addEvent(EVENT_NIGHTBANE_TAIL_SWIPE, 12000);
                break;
            case EVENT_NIGHTBANE_REVERBERATING_SHADOWS:
                castSpellAOE(SPELL_NIGHTBANE_REVERBERATING_SHADOWS);
                scriptEvents.addEvent(EVENT_NIGHTBANE_REVERBERATING_SHADOWS, 13000);
                break;
            case EVENT_NIGHTBANE_BURNING_BONES:
                castOnRandomPlayer(SPELL_NIGHTBANE_BURNING_BONES);
                scriptEvents.addEvent(EVENT_NIGHTBANE_BURNING_BONES, 18000);
                break;
            case EVENT_NIGHTBANE_IGNITE_SOUL:
                if (Unit* pTarget = randomNonTank())
                {
                    sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Nightbane ignites a soul!");
                    castSpell(pTarget, SPELL_NIGHTBANE_IGNITE_SOUL);
                }
                scriptEvents.addEvent(EVENT_NIGHTBANE_IGNITE_SOUL, 25000);
                break;
            case EVENT_NIGHTBANE_CONCENTRATED_POWER:
                castSpellOnSelf(SPELL_NIGHTBANE_CONCENTRATED_POWER);
                break;
            case EVENT_NIGHTBANE_INFERNAL_POWER:
                castSpellOnSelf(SPELL_NIGHTBANE_INFERNAL_POWER);
                break;
            case EVENT_NIGHTBANE_BELLOWING_ROAR:
                castSpellAOE(SPELL_NIGHTBANE_BELLOWING_ROAR);
                scriptEvents.addEvent(EVENT_NIGHTBANE_BELLOWING_ROAR, 42000);
                break;
            case EVENT_NIGHTBANE_RAIN_OF_BONES:
                rainOfBones();
                break;
            default:
                break;
        }
    }

private:
    void groundRotation(bool landed)
    {
        scriptEvents.addEvent(EVENT_NIGHTBANE_CHARRED_EARTH, 3000);
        scriptEvents.addEvent(EVENT_NIGHTBANE_CINDER_BREATH, 8000);
        scriptEvents.addEvent(EVENT_NIGHTBANE_TAIL_SWIPE, 12000);
        scriptEvents.addEvent(EVENT_NIGHTBANE_REVERBERATING_SHADOWS, 17000);
        scriptEvents.addEvent(EVENT_NIGHTBANE_BURNING_BONES, 19000);
        scriptEvents.addEvent(EVENT_NIGHTBANE_IGNITE_SOUL, 20000);
        scriptEvents.addEvent(EVENT_NIGHTBANE_CONCENTRATED_POWER, 30000);
        scriptEvents.addEvent(EVENT_NIGHTBANE_INFERNAL_POWER, 53000);

        if (landed)
            scriptEvents.addEvent(EVENT_NIGHTBANE_BELLOWING_ROAR, 20000);
    }

    void takeOff()
    {
        mTookOff = true;
        mAirborne = true;
        mGroundZ = getCreature()->GetPositionZ();

        scriptEvents.resetEvents();
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Nightbane takes to the air!");
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_PASSIVE);
        attackStop();
        setFlyMode(true);
        movePoint(1, getCreature()->GetPositionX(), getCreature()->GetPositionY(), mGroundZ + 20.0f, false);
        scriptEvents.addEvent(EVENT_NIGHTBANE_RAIN_OF_BONES, 3000);
    }

    // stays up until Bonecurse dies
    void rainOfBones()
    {
        castSpellAOE(SPELL_NIGHTBANE_RAIN_OF_BONES);

        Unit* pTarget = randomPlayer();
        Creature* pBonecurse = pTarget != nullptr ? callAdd(NPC_BONECURSE, aroundUnit(pTarget, 3.0f, 6.0f)) : nullptr;
        if (pBonecurse == nullptr)
            land();
    }

    void land()
    {
        mAirborne = false;
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        setFlyMode(false);
        movePoint(2, getCreature()->GetPositionX(), getCreature()->GetPositionY(), mGroundZ, false);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Nightbane lands!");
        groundRotation(true);
    }

    bool mAirborne = false;
    bool mTookOff = false;
    float mGroundZ = 0.0f;
};

void SetupReturnToKarazhan(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_RETURN_TO_KARAZHAN, &ReturnToKarazhanInstanceScript::Create);

    mgr->register_creature_script(BOSS_MAIDEN_OF_VIRTUE, &ReturnToKarazhanMaidenOfVirtueAI::Create);

    mgr->register_creature_script(BOSS_GALINDRE, &ReturnToKarazhanGalindreAI::Create);
    mgr->register_creature_script(BOSS_ELFYRA, &ReturnToKarazhanElfyraAI::Create);
    mgr->register_creature_script(BOSS_TOE_KNEE, &ReturnToKarazhanToeKneeAI::Create);
    mgr->register_creature_script(BOSS_MRRGRIA, &ReturnToKarazhanMrrgriaAI::Create);
    mgr->register_creature_script(NPC_GANG_RUFFIAN, &ReturnToKarazhanGangRuffianAI::Create);
    mgr->register_creature_script(NPC_SHORELINE_TIDESPEAKER, &ReturnToKarazhanTidespeakerAI::Create);
    mgr->register_creature_script(BOSS_COGGLESTON, &ReturnToKarazhanCogglestonAI::Create);
    mgr->register_creature_script(BOSS_LUMINORE, &ReturnToKarazhanLuminoreAI::Create);
    mgr->register_creature_script(BOSS_BABBLET, &ReturnToKarazhanBabbletAI::Create);
    mgr->register_creature_script(BOSS_MRS_CAULDRONS, &ReturnToKarazhanMrsCauldronsAI::Create);

    mgr->register_creature_script(BOSS_MANA_DEVOURER, &ReturnToKarazhanManaDevourerAI::Create);

    mgr->register_creature_script(BOSS_MIDNIGHT, &ReturnToKarazhanMidnightAI::Create);
    mgr->register_creature_script(BOSS_ATTUMEN, &ReturnToKarazhanAttumenAI::Create);

    mgr->register_creature_script(BOSS_MOROES, &ReturnToKarazhanMoroesAI::Create);
    for (uint32_t guest : MoroesGuestEntries)
        mgr->register_creature_script(guest, &ReturnToKarazhanDinnerGuestAI::Create);

    mgr->register_creature_script(BOSS_CURATOR, &ReturnToKarazhanCuratorAI::Create);
    mgr->register_creature_script(NPC_VOLATILE_ENERGY, &ReturnToKarazhanVolatileEnergyAI::Create);

    mgr->register_creature_script(BOSS_SHADE_OF_MEDIVH, &ReturnToKarazhanShadeOfMedivhAI::Create);
    mgr->register_creature_script(NPC_GUARDIANS_IMAGE, &ReturnToKarazhanGuardiansImageAI::Create);

    mgr->register_creature_script(BOSS_VIZADUUM, &ReturnToKarazhanVizaduumAI::Create);
    mgr->register_creature_script(NPC_SHADOW_SPITTER, &ReturnToKarazhanShadowSpitterAI::Create);

    mgr->register_creature_script(BOSS_NIGHTBANE, &ReturnToKarazhanNightbaneAI::Create);
}
