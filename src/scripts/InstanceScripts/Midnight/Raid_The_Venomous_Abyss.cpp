/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_The_Venomous_Abyss.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <list>
#include <utility>
#include <vector>

enum VenomousAbyssEvents
{
    // Sszorak
    EVENT_SSZORAK_CORRODING_VENOM = 1,
    EVENT_SSZORAK_APEX_PREDATOR,
    EVENT_SSZORAK_RAVAGE,
    EVENT_SSZORAK_MUTILATE,
    EVENT_SSZORAK_VENOMOUS_SURGE,
    EVENT_SSZORAK_RAGING_CROSSWINDS,

    // The Twin Fangs
    EVENT_VEXHUL_CONCENTRATED_SPITTLE,
    EVENT_VEXHUL_CAUSTIC_DELUGE,
    EVENT_VEXHUL_VENOMOUS_EMERGENCE,
    EVENT_VEXHUL_STIR_THE_DEPTHS,
    EVENT_VEXHUL_VILE_FLOOD,
    EVENT_ITHRAZ_STONE_BREAKER,
    EVENT_ITHRAZ_RAVENOUS_FEAST,
    EVENT_ITHRAZ_COILING_ICHOR,
    EVENT_ITHRAZ_ROUSE_THE_BROOD,
    EVENT_ITHRAZ_BLOOD_TORRENT,

    // The Coiled Altar
    EVENT_ZULJAN_VENOMFANG,
    EVENT_ZULJAN_SEVER,
    EVENT_ZULJAN_AXEGRINDER,
    EVENT_ZULJAN_TOXIC_DELUGE,
    EVENT_ZULJAN_GUILLOTINE,
    EVENT_ZULJAN_FANGS_OF_THE_ALTAR,
    EVENT_ZULJAN_DEFILEMENT,
    EVENT_ZULJAN_GRIM_GUILLOTINE,
    EVENT_ZULJAN_BLIGHTED_SEVER,
    EVENT_MALACRASS_DREAD_BOLT,
    EVENT_MALACRASS_SOUL_SEVER,
    EVENT_MALACRASS_DREADMARCH,
    EVENT_MALACRASS_GLOOMBOMB,
    EVENT_MALACRASS_SPIRITCACKLE,
    EVENT_MALACRASS_ETERNAL_NIGHTFALL,
    EVENT_MALACRASS_NIGHTFALL_DESCENDS,
    EVENT_MALACRASS_VESSEL_BOUND,

    // Entombed Sentinels
    EVENT_BREATH_EMPOWERING_SLAM,
    EVENT_BREATH_TOXIC_DROPLETS,
    EVENT_BREATH_LIVING_VENOM,
    EVENT_BREATH_VENOM_COAGULATION,
    EVENT_BLOOD_BLOODVENOM_INJECTION,
    EVENT_BLOOD_BLIGHTED_BLOOD,
    EVENT_BLOOD_BLOOD_VENOM,
    EVENT_BLOOD_UNSTABLE_MIASMA,

    // Vashnik the Malignant
    EVENT_VASHNIK_DRIPPING_FANGS,
    EVENT_VASHNIK_MALIGNANT_CATALYST,
    EVENT_VASHNIK_ADAPTIVE_INFECTION,
    EVENT_VASHNIK_PLAGUE_FROTH,
    EVENT_VASHNIK_EXPULSION,

    // Nek'zali the Soulcoiler
    EVENT_NEKZALI_HOLLOWING_STRIKES,
    EVENT_NEKZALI_POSSESSION_BARRAGE,
    EVENT_NEKZALI_ESSENCE_REND,
    EVENT_NEKZALI_LATENT_CULTIST,
    EVENT_NEKZALI_SOULCOIL_IGNITION,
    EVENT_NEKZALI_SOULCOIL_RITE,
    EVENT_NEKZALI_GRASPING_DEPTHS,
    EVENT_NEKZALI_INVOKE,

    // Ula'tek
    EVENT_ULATEK_CAUSTIC_WAVES,
    EVENT_ULATEK_CALL_OF_THE_SERPENT,
    EVENT_ULATEK_NECROTIC_VAPORS,
    EVENT_ULATEK_MOTHERS_WRATH,
    EVENT_ULATEK_RAGE_OF_THE_SHACKLED,
    EVENT_ULATEK_UNCHECKED_RAGE,
    EVENT_ULATEK_VIRULENT_SPIT,
    EVENT_ULATEK_BLIGHTSCALE_CLUTCH,
    EVENT_ULATEK_SPECTRAL_COILS,
    EVENT_ULATEK_ASCENSION,
    EVENT_ULATEK_SERPENTS_BITE,
    EVENT_ULATEK_CIRCLING_PREY,

    // The Lost Explorers
    EVENT_MORZAHI_EVIL_EYES,
    EVENT_MORZAHI_DARK_WHISPERS,
    EVENT_MORZAHI_MALEVOLENT_PRESENCE,
    EVENT_GEBBO_SMASHING_SHOVEL,
    EVENT_GEBBO_THROW_JUNK,
    EVENT_GEBBO_MUSHROOM_TOSS,
    EVENT_GEBBO_EXPLOSIVE_SURPRISE,
    EVENT_IKU_SHREDDING_SHARDS,
    EVENT_IKU_FROSTFIRE_VOLLEY,
    EVENT_IKU_ICEBOUND_FLAMES,
    EVENT_IKU_BLINK_NOVA,
    EVENT_NAMA_SHELL_SPIN,
    EVENT_NAMA_MIGHTY_THUD,
    EVENT_NAMA_UNITED_DEFENSE,

    // simple adds, one event per kit slot
    EVENT_ABYSS_KIT_MOVE_1,
    EVENT_ABYSS_KIT_MOVE_2,
    EVENT_ABYSS_KIT_MOVE_3
};

enum VenomousAbyssActions
{
    ACTION_ABYSS_UNCOILED_WRATH = 1,
    ACTION_ABYSS_MALACRASS_JOINS,
    ACTION_ABYSS_ZULJAN_RISES,
    ACTION_ABYSS_WIPE_RESET,
    ACTION_ABYSS_RUPTURE,
    ACTION_ABYSS_SENTINEL_STASIS,
    ACTION_ABYSS_SOUL_ENTERS_WELL,
    ACTION_ABYSS_EXPLORER_EMPOWER,
    ACTION_ABYSS_EXPLORER_FREED,
    ACTION_ABYSS_EXPLORER_RELEASE
};

namespace
{
    float const AbyssTwoPi = 6.28318531f;
    float const AbyssPlayerReach = 100.0f;

    uint32_t const AbyssBerserkMs = 600000;
    uint32_t const AbyssLongBerserkMs = 720000;
    uint32_t const AbyssGaugeFull = 100;

    // The Coiled Altar stages
    uint32_t const AltarStageZuljan = 1;
    uint32_t const AltarStageMalacrass = 2;
    uint32_t const AltarStageVessel = 3;
    uint32_t const AltarStageUnion = 4;

    int32_t const MalacrassVesselPct = 50;
    uint32_t const VesselBindingMs = 15000;
    uint32_t const NightfallDelayMs = 8000;
    float const SeverVenomReach = 12.0f;

    int32_t const NekzaliRitualPct = 50;
    uint32_t const NekzaliEchoCount = 2;

    int32_t const UlatekDoomscalePct = 70;
    int32_t const UlatekShatteringPct = 40;
    float const UlatekMeleeReach = 8.0f;

    // walkers that head for the room centre
    float const CrawlerArrivalRange = 4.0f;
    uint32_t const CrawlerRepathMs = 3000;
    uint32_t const CrawlerMoveId = 1;

    // GetCreatureData type of a tortollan freed from Mor'zahi
    uint32_t const ExplorerFreedData = 1;

    // creatures that fight one encounter together
    struct AbyssMember
    {
        uint32_t dataId;
        uint32_t entry;
        bool required;
    };

    AbyssMember const AbyssMembers[] =
    {
        { DATA_TWIN_FANGS,          BOSS_ITHRAZ,                    true },
        { DATA_TWIN_FANGS,          BOSS_VEXHUL,                    true },
        { DATA_COILED_ALTAR,        BOSS_ZULJAN,                    true },
        { DATA_COILED_ALTAR,        BOSS_ZULJAN_ALT,                true },
        { DATA_COILED_ALTAR,        BOSS_ZULJAN_ALT2,               true },
        { DATA_COILED_ALTAR,        BOSS_HEX_LORD_MALACRASS,        true },
        { DATA_COILED_ALTAR,        BOSS_HEX_LORD_MALACRASS_ALT,    true },
        { DATA_ENTOMBED_SENTINELS,  BOSS_BLOOD_OF_ULATEK,           true },
        { DATA_ENTOMBED_SENTINELS,  BOSS_BREATH_OF_ULATEK,          true },
        { DATA_LOST_EXPLORERS,      BOSS_MORZAHI,                   true },
        { DATA_LOST_EXPLORERS,      BOSS_FIRST_MATE_NAMA,           false },
        { DATA_LOST_EXPLORERS,      BOSS_TRADER_GEBBO,              false },
        { DATA_LOST_EXPLORERS,      BOSS_SCROLLSAGE_IKU,            false }
    };

    bool isZuljanEntry(uint32_t entry)
    {
        return entry == BOSS_ZULJAN || entry == BOSS_ZULJAN_ALT || entry == BOSS_ZULJAN_ALT2;
    }

    bool isMalacrassEntry(uint32_t entry)
    {
        return entry == BOSS_HEX_LORD_MALACRASS || entry == BOSS_HEX_LORD_MALACRASS_ALT;
    }

    uint32_t const EchoEntries[3] = { NPC_ECHO_OF_JAWAE, NPC_ECHO_OF_JAWAE_ALT, NPC_ECHO_OF_JAWAE_ALT2 };

    bool isEchoEntry(uint32_t entry)
    {
        for (uint32_t echo : EchoEntries)
        {
            if (echo == entry)
                return true;
        }

        return false;
    }

    // adds with a few timed spells
    enum class AbyssAim : uint8_t
    {
        Area,
        Victim,
        AnyPlayer,
        OtherPlayer
    };

    struct AbyssAddMove
    {
        uint32_t spellId;
        uint32_t firstMs;
        uint32_t repeatMs;      // 0 casts once
        AbyssAim aim;
        bool heroicOnly;
    };

    size_t const AbyssKitSlots = 3;

    struct AbyssAddKit
    {
        uint32_t entry;
        bool stationary;
        AbyssAddMove moves[AbyssKitSlots];
    };

    AbyssAddKit const AbyssAddKits[] =
    {
        // The Twin Fangs
        { NPC_SPAWN_OF_VEXHUL, false, { { SPELL_SPAWN_CORROSIVE_SPIT, 4000, 9000, AbyssAim::AnyPlayer, false }, {}, {} } },
        { NPC_BROODLING_OF_ITHRAZ, false, { { SPELL_BROODLING_VISCERAL_BURST, 8000, 12000, AbyssAim::Area, false }, {}, {} } },
        { NPC_BARBED_BULWARK, true, { {}, {}, {} } },

        // The Coiled Altar
        { NPC_SPITEFUL_SOULCOILER, false, { { SPELL_SOULCOILER_WAIL_OF_TERROR, 5000, 15000, AbyssAim::Area, false }, {}, {} } },

        // Nek'zali the Soulcoiler
        { NPC_LATENT_CULTIST, false, { { SPELL_CULTIST_ENTWINED_STEP, 6000, 14000, AbyssAim::AnyPlayer, false }, {}, {} } },
        { NPC_ECHO_OF_JAWAE, false, { { SPELL_ECHO_SOUL_TRANSFER, 1000, 0, AbyssAim::Area, false }, { SPELL_ECHO_HUNGERING_PYRE, 5000, 12000, AbyssAim::AnyPlayer, false }, {} } },

        // Ula'tek
        { NPC_GORE_RATTLE, false, { { SPELL_GORE_RATTLE_MEPHITIC_THRASH, 6000, 14000, AbyssAim::Victim, false }, { SPELL_GORE_RATTLE_RATTLER_SLAM, 10000, 20000, AbyssAim::Area, false }, { SPELL_ULATEK_SPECTRAL_COILS, 30000, 45000, AbyssAim::Area, false } } },
        { NPC_BLIGHTSCALE_VIPER, false, { { SPELL_VIPER_ACIDIC_BURST, 6000, 10000, AbyssAim::Area, false }, { SPELL_VIPER_PETRIFYING_STING, 8000, 16000, AbyssAim::AnyPlayer, true }, {} } },
        { NPC_BLIGHTSCALE_RAWLING, false, { { SPELL_RAWLING_POISONOUS_BITE, 4000, 8000, AbyssAim::Victim, true }, {}, {} } },
        { NPC_DOOMSCALE_WARDEN, false, { { SPELL_WARDEN_GRASPING_FANGS, 6000, 18000, AbyssAim::AnyPlayer, false }, { SPELL_WARDEN_SHADOW_MOLT, 10000, 20000, AbyssAim::Area, false }, { SPELL_WARDEN_WRITHING_GESTATION, 14000, 25000, AbyssAim::Area, false } } },
        { NPC_RAVENOUS_DOOMSCALE, false, { { SPELL_DOOMSCALE_DREAD_ROAR, 5000, 15000, AbyssAim::Area, false }, {}, {} } },
        { NPC_BLIGHTSCALE_SHRIEKER, false, { { SPELL_SHRIEKER_ACIDIC_EXPULSION, 6000, 12000, AbyssAim::AnyPlayer, false }, { SPELL_SHRIEKER_VICIOUS_ECHOES, 10000, 20000, AbyssAim::Area, false }, {} } }
    };

    AbyssAddKit const* findAbyssAddKit(uint32_t entry)
    {
        // alternative templates use the kit of their main entry
        if (isEchoEntry(entry))
            entry = NPC_ECHO_OF_JAWAE;
        else if (entry == NPC_DOOMSCALE_WARDEN_ALT)
            entry = NPC_DOOMSCALE_WARDEN;

        for (AbyssAddKit const& kit : AbyssAddKits)
        {
            if (kit.entry == entry)
                return &kit;
        }

        return nullptr;
    }

    // eggs and clutches that hatch on a clock
    struct AbyssNestKit
    {
        uint32_t entry;
        uint32_t shellSpell;
        uint32_t hatchEntry;
        uint32_t hatchSpell;
        uint32_t firstMs;
        uint32_t repeatMs;      // 0 hatches once
        uint32_t brokenEntry;   // released when destroyed early
    };

    AbyssNestKit const AbyssNestKits[] =
    {
        { NPC_BLIGHTSCALE_SPAWN, SPELL_SPAWN_MALIGNANT_SHELL, NPC_BLIGHTSCALE_VIPER, SPELL_SPAWN_PUTRID_MEMBRANE, 15000, 0, NPC_BLIGHTSCALE_RAWLING },
        { NPC_BLIGHTSCALE_CLUTCH, 0, NPC_BLIGHTSCALE_RAWLING, 0, 12000, 15000, 0 },
        { NPC_DOOMSCALE_EGG, SPELL_DOOMSCALE_SHELL, NPC_RAVENOUS_DOOMSCALE, SPELL_DOOMSCALE_HATCHING_DOOM, 30000, 0, 0 }
    };

    AbyssNestKit const* findAbyssNestKit(uint32_t entry)
    {
        for (AbyssNestKit const& kit : AbyssNestKits)
        {
            if (kit.entry == entry)
                return &kit;
        }

        return nullptr;
    }

    uint32_t crawlerAura(uint32_t entry)
    {
        switch (entry)
        {
            case NPC_SHROUDED_VENOM:
                return SPELL_SHROUDED_MIASMIC_COATING;
            case NPC_BURNING_VENOM:
                return SPELL_BURNING_BURNING_PRESENCE;
            case NPC_RESTLESS_AMANI:
                return SPELL_AMANI_GRAVEBOUND_ADVANCE;
            default:
                return 0;
        }
    }

    // Vashnik's fountains: living venom, infusion and expulsion
    struct VashnikFountain
    {
        uint32_t venomEntry;
        uint32_t infusion;
        uint32_t expulsion;
    };

    VashnikFountain const VashnikFountains[3] =
    {
        { NPC_CLOTTING_VENOM, SPELL_VASHNIK_BLOOD_INFUSION, SPELL_VASHNIK_HEMO_EXPULSION },
        { NPC_SHROUDED_VENOM, SPELL_VASHNIK_SHADOW_INFUSION, SPELL_VASHNIK_GLOOM_EXPULSION },
        { NPC_BURNING_VENOM, SPELL_VASHNIK_FLAME_INFUSION, SPELL_VASHNIK_CONFLAGRATING_EXPULSION }
    };

    LocationVector abyssRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(AbyssTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // spawn point of a boss, its position when it has none
    LocationVector abyssHomeOf(Object* pObject)
    {
        if (pObject->GetSpawnX() == 0.0f && pObject->GetSpawnY() == 0.0f)
            return pObject->GetPosition();

        return LocationVector(pObject->GetSpawnX(), pObject->GetSpawnY(), pObject->GetSpawnZ());
    }

    uint32_t healthShare(Creature* pCreature)
    {
        uint32_t const maxHealth = pCreature->getMaxHealth();
        return maxHealth == 0 ? 0 : static_cast<uint32_t>(static_cast<uint64_t>(pCreature->getHealth()) * 100 / maxHealth);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class VenomousAbyssInstanceScript : public InstanceScript
{
public:
    explicit VenomousAbyssInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(VenomousAbyssEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new VenomousAbyssInstanceScript(pMapMgr); }

    // a fallen member must not end the encounter while a partner stands
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isEncounterDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (id == DATA_COILED_ALTAR && state != EncounterStates::InProgress)
            mAltarStage = AltarStageZuljan;

        if (state == EncounterStates::Failed)
        {
            // a wiped group fights again in full
            for (Creature* pMember : getMembers(id))
            {
                if (!pMember->isAlive())
                    pMember->despawn(1000, 5000);
                else if (pMember->GetScript() != nullptr)
                    pMember->GetScript()->DoAction(ACTION_ABYSS_WIPE_RESET);
            }
            return;
        }

        // the freed tortollans stay out of any further fight
        if (state == EncounterStates::Performed)
        {
            for (Creature* pMember : getMembers(id))
            {
                if (pMember->isAlive() && pMember->GetScript() != nullptr)
                    pMember->GetScript()->DoAction(ACTION_ABYSS_EXPLORER_RELEASE);
            }
        }
    }

    std::vector<Creature*> getMembers(uint32_t dataId)
    {
        std::vector<Creature*> members;
        for (AbyssMember const& member : AbyssMembers)
        {
            if (member.dataId != dataId)
                continue;

            for (Creature* pCreature : getCreatureSetForEntry(member.entry))
                members.push_back(pCreature);
        }

        return members;
    }

    // single boss encounters are down with their boss
    bool isEncounterDown(uint32_t dataId, Creature* pExcept)
    {
        for (AbyssMember const& member : AbyssMembers)
        {
            if (member.dataId != dataId || !member.required)
                continue;

            for (Creature* pCreature : getCreatureSetForEntry(member.entry))
            {
                if (pCreature != pExcept && pCreature->isAlive())
                    return false;
            }
        }

        return true;
    }

    bool isEncounterFighting(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getMembers(dataId))
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat())
                return true;
        }

        return false;
    }

    // pulling one member pulls the rest
    void rallyEncounter(uint32_t dataId, CreatureAIScript* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getMembers(dataId))
        {
            if (pMember == pPuller->getCreature() || !pMember->isAlive() || pMember->isInCombat())
                continue;

            pPuller->setZoneWideCombat(pMember);
        }
        mRallying = false;
    }

    uint32_t getAltarStage() const { return mAltarStage; }
    void setAltarStage(uint32_t stage) { mAltarStage = stage; }

private:
    uint32_t mAltarStage = AltarStageZuljan;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, berserk, energy gauge and target helpers

class VenomousAbyssCombatAI : public CreatureAIScript
{
public:
    explicit VenomousAbyssCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mBerserkLeftMs = 0;
        resetGauge();
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mBerserkLeftMs = 0;

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
        tickBerserk(diff);
        onCombatTick(diff);

        // due events run one after another unless a cast holds them
        while (!_isCasting())
        {
            uint32_t const eventId = scriptEvents.getFinishedEvent();
            if (eventId == 0)
                break;

            onEvent(eventId);
        }
    }

protected:
    virtual void onEngage() {}
    virtual void onReset() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    VenomousAbyssInstanceScript* getAbyss()
    {
        if (getCreature()->GetMapId() != MAP_THE_VENOMOUS_ABYSS)
            return nullptr;

        return static_cast<VenomousAbyssInstanceScript*>(getInstanceScript());
    }

    // distinct random players, the current target only as last resort
    std::vector<Unit*> rollPlayers(size_t count, bool spareTank)
    {
        TargetFilter const filter = spareTank ? TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent) : TargetFilter_Player;
        FilterArgs const args(filter);

        std::vector<Unit*> pool;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr || getCreature()->getDistance(pObject) > AbyssPlayerReach)
                continue;

            if (isValidUnitTarget(pObject, filter, args))
                pool.push_back(static_cast<Unit*>(pObject));
        }

        if (pool.empty() && spareTank)
            return rollPlayers(count, false);

        for (size_t i = 0; i < pool.size() && i < count; ++i)
            std::swap(pool[i], pool[i + Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1 - i))]);

        if (pool.size() > count)
            pool.resize(count);

        return pool;
    }

    Unit* rollPlayer(bool spareTank)
    {
        std::vector<Unit*> const picked = rollPlayers(1, spareTank);
        return picked.empty() ? nullptr : picked.front();
    }

    void castOnPlayers(uint32_t spellId, size_t count, bool spareTank)
    {
        for (Unit* pTarget : rollPlayers(count, spareTank))
            castSpell(pTarget, spellId, true);
    }

    Unit* castOnPlayer(uint32_t spellId, bool spareTank)
    {
        Unit* pTarget = rollPlayer(spareTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId);

        return pTarget;
    }

    // summoned helper that joins the fight at once, placeholders summon nothing
    Creature* bringAdd(uint32_t entry, LocationVector const& spot)
    {
        if (entry == 0)
            return nullptr;

        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // summoned helper that acts on its own: eggs, venoms, walkers
    Creature* placeAdd(uint32_t entry, LocationVector const& spot)
    {
        if (entry == 0)
            return nullptr;

        return summonCreature(entry, spot, CORPSE_DESPAWN);
    }

    LocationVector ringAroundSelf(float minRange, float maxRange)
    {
        return abyssRingPoint(getCreature()->GetPosition(), minRange, maxRange);
    }

    void announce(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void startBerserkClock(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

    // true once the gauge is full, it starts over then
    bool fillGauge(uint32_t diff, uint32_t perSecond)
    {
        mGaugeClockMs += diff;
        while (mGaugeClockMs >= 1000)
        {
            mGaugeClockMs -= 1000;
            mGauge += perSecond;
        }

        if (mGauge < AbyssGaugeFull || _isCasting())
            return false;

        mGauge = 0;
        return true;
    }

    void resetGauge()
    {
        mGauge = 0;
        mGaugeClockMs = 0;
    }

    // out of reach and passive while yielded or waiting
    void setUntouchable(bool apply)
    {
        if (apply)
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        else
            removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        setRooted(apply);
        _setMeleeDisabled(apply);
    }

private:
    void tickBerserk(uint32_t diff)
    {
        if (mBerserkLeftMs == 0)
            return;

        if (mBerserkLeftMs > diff)
        {
            mBerserkLeftMs -= diff;
            return;
        }

        mBerserkLeftMs = 0;
        castSpellOnSelf(SPELL_ABYSS_BERSERK, true);
    }

    uint32_t mBerserkLeftMs = 0;
    uint32_t mGauge = 0;
    uint32_t mGaugeClockMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class VenomousAbyssBossAI : public VenomousAbyssCombatAI
{
public:
    VenomousAbyssBossAI(Creature* pCreature, uint32_t dataId, bool credited = true) : VenomousAbyssCombatAI(pCreature), mDataId(dataId), mCredited(credited) {}

    void OnCombatStart(Unit* pTarget) override
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance != nullptr && pInstance->getBossState(mDataId) != EncounterStates::Performed)
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);

            if (VenomousAbyssInstanceScript* pAbyss = getAbyss())
                pAbyss->rallyEncounter(mDataId, this);
        }

        VenomousAbyssCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last partner has left the fight
                VenomousAbyssInstanceScript* pAbyss = getAbyss();
                if (pAbyss == nullptr || !pAbyss->isEncounterFighting(mDataId, getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        VenomousAbyssCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            VenomousAbyssInstanceScript* pAbyss = getAbyss();
            if (mCredited && (pAbyss == nullptr || pAbyss->isEncounterDown(mDataId, getCreature())))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    virtual void onDefeat() {}

    bool isEncounterRunning()
    {
        InstanceScript* pInstance = getInstanceScript();
        return pInstance != nullptr && pInstance->getBossState(mDataId) == EncounterStates::InProgress;
    }

    bool isEncounterDone()
    {
        InstanceScript* pInstance = getInstanceScript();
        return pInstance != nullptr && pInstance->getBossState(mDataId) == EncounterStates::Performed;
    }

    std::vector<Creature*> livingPartners()
    {
        std::vector<Creature*> partners;
        if (VenomousAbyssInstanceScript* pAbyss = getAbyss())
        {
            for (Creature* pMember : pAbyss->getMembers(mDataId))
            {
                if (pMember != getCreature() && pMember->isAlive())
                    partners.push_back(pMember);
            }
        }

        return partners;
    }

    void signalPartner(Creature* pPartner, int32_t action)
    {
        if (pPartner != nullptr && pPartner->GetScript() != nullptr)
            pPartner->GetScript()->DoAction(action);
    }

    uint32_t mDataId;
    bool mCredited;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Simple adds driven by their kit

class VenomousAbyssKitAI : public VenomousAbyssCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssKitAI(c); }
    explicit VenomousAbyssKitAI(Creature* pCreature) : VenomousAbyssCombatAI(pCreature), mKit(findAbyssAddKit(pCreature->getEntry())) {}

    void OnLoad() override
    {
        if (mKit == nullptr || !mKit->stationary)
            return;

        setRooted(true);
        _setMeleeDisabled(true);
    }

protected:
    void onEngage() override
    {
        if (mKit == nullptr)
            return;

        for (size_t slot = 0; slot < AbyssKitSlots; ++slot)
        {
            AbyssAddMove const& move = mKit->moves[slot];
            if (move.spellId == 0 || (move.heroicOnly && !isHeroic()))
                continue;

            scriptEvents.addEvent(EVENT_ABYSS_KIT_MOVE_1 + static_cast<uint32_t>(slot), move.firstMs);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        if (mKit == nullptr || eventId < EVENT_ABYSS_KIT_MOVE_1 || eventId >= EVENT_ABYSS_KIT_MOVE_1 + AbyssKitSlots)
            return;

        AbyssAddMove const& move = mKit->moves[eventId - EVENT_ABYSS_KIT_MOVE_1];
        switch (move.aim)
        {
            case AbyssAim::Victim:
                castSpellOnVictim(move.spellId);
                break;
            case AbyssAim::AnyPlayer:
                castOnPlayer(move.spellId, false);
                break;
            case AbyssAim::OtherPlayer:
                castOnPlayer(move.spellId, true);
                break;
            default:
                castSpellAOE(move.spellId);
                break;
        }

        if (move.repeatMs != 0)
            scriptEvents.addEvent(eventId, move.repeatMs);
    }

private:
    AbyssAddKit const* mKit;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Walkers: living venoms and restless spirits heading for the room centre

class VenomousAbyssCrawlerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssCrawlerAI(c); }
    explicit VenomousAbyssCrawlerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* pSummoner) override
    {
        if (pSummoner == nullptr)
            return;

        mMasterGuid = pSummoner->GetNewGUID();
        mGoal = abyssHomeOf(pSummoner);
        mWalking = true;
        mRepathMs = 0;

        if (uint32_t const aura = crawlerAura(getCreature()->getEntry()))
            castSpellOnSelf(aura, true);

        setReactState(REACT_PASSIVE);
        movePoint(CrawlerMoveId, mGoal);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mWalking || !isAlive())
            return;

        if (getCreature()->getExactDist2d(mGoal) <= CrawlerArrivalRange)
        {
            arrive();
            return;
        }

        // hits must not stop the walk
        mRepathMs += static_cast<uint32_t>(time_passed);
        if (mRepathMs >= CrawlerRepathMs)
        {
            mRepathMs = 0;
            movePoint(CrawlerMoveId, mGoal);
        }
    }

private:
    void arrive()
    {
        mWalking = false;

        if (getCreature()->getEntry() == NPC_RESTLESS_AMANI)
        {
            // the soul feeds the well
            InstanceScript* pInstance = getInstanceScript();
            Creature* pMaster = pInstance != nullptr ? pInstance->getCreatureByGuid(mMasterGuid) : nullptr;
            if (pMaster != nullptr && pMaster->isAlive() && pMaster->GetScript() != nullptr)
                pMaster->GetScript()->DoAction(ACTION_ABYSS_SOUL_ENTERS_WELL);
        }
        else
        {
            castSpellAOE(SPELL_VASHNIK_MALIGNANT_BURST, true);
        }

        despawn(300, 0);
    }

    WoWGuid mMasterGuid;
    LocationVector mGoal;
    bool mWalking = false;
    uint32_t mRepathMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nests: Blightscale Spawn, Blightscale Clutch and Doomscale Egg

class VenomousAbyssNestAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssNestAI(c); }
    explicit VenomousAbyssNestAI(Creature* pCreature) : CreatureAIScript(pCreature), mKit(findAbyssNestKit(pCreature->getEntry())) {}

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void OnSummon(Unit* pSummoner) override
    {
        if (mKit == nullptr)
            return;

        if (pSummoner != nullptr)
            mMotherGuid = pSummoner->GetNewGUID();

        mClockMs = 0;
        mNextHatchMs = mKit->firstMs;
        mActive = true;
        mBroken = false;

        if (mKit->shellSpell != 0)
            castSpellOnSelf(mKit->shellSpell, true);
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mKit == nullptr || mKit->brokenEntry == 0)
            return;

        if (mBroken)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        // cracked early, a weaker hatchling crawls out
        *damage = health > 1 ? health - 1 : 0;
        mBroken = true;
        mActive = false;
        releaseHatchling(mKit->brokenEntry);
        despawn(300, 0);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mActive || !isAlive())
            return;

        mClockMs += static_cast<uint32_t>(time_passed);
        if (mClockMs < mNextHatchMs)
            return;

        releaseHatchling(mKit->hatchEntry);
        if (mKit->hatchSpell != 0)
            castSpellAOE(mKit->hatchSpell, true);

        if (mKit->repeatMs == 0)
        {
            mActive = false;
            despawn(300, 0);
            return;
        }

        mNextHatchMs += mKit->repeatMs;
    }

private:
    // the brood belongs to the mother so it leaves with her fight
    void releaseHatchling(uint32_t entry)
    {
        InstanceScript* pInstance = getInstanceScript();
        Creature* pMother = pInstance != nullptr ? pInstance->getCreatureByGuid(mMotherGuid) : nullptr;

        Creature* pHatchling = nullptr;
        if (pMother != nullptr && pMother->isAlive())
            pHatchling = pMother->summonCreature(entry, getCreature()->GetPosition(), CORPSE_DESPAWN);
        else
            pHatchling = summonCreature(entry, getCreature()->GetPosition(), CORPSE_DESPAWN);

        if (pHatchling != nullptr)
            setZoneWideCombat(pHatchling);
    }

    AbyssNestKit const* mKit;
    WoWGuid mMotherGuid;
    uint32_t mClockMs = 0;
    uint32_t mNextHatchMs = 0;
    bool mActive = false;
    bool mBroken = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sszorak

class VenomousAbyssSszorakAI : public VenomousAbyssBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssSszorakAI(c); }
    explicit VenomousAbyssSszorakAI(Creature* pCreature) : VenomousAbyssBossAI(pCreature, DATA_SSZORAK) {}

protected:
    void onEngage() override
    {
        startBerserkClock(AbyssBerserkMs);
        scriptEvents.addEvent(EVENT_SSZORAK_CORRODING_VENOM, 6000);
        scriptEvents.addEvent(EVENT_SSZORAK_APEX_PREDATOR, 12000);
        scriptEvents.addEvent(EVENT_SSZORAK_VENOMOUS_SURGE, 18000);
        scriptEvents.addEvent(EVENT_SSZORAK_RAGING_CROSSWINDS, 24000);
    }

    // the altar winds build up to a maelstrom every 100 seconds
    void onCombatTick(uint32_t diff) override
    {
        if (!fillGauge(diff, 1))
            return;

        announce("The winds of the altar build into a Howling Maelstrom!");
        castSpellAOE(SPELL_SSZORAK_HOWLING_MAELSTROM, true);
        castSpellOnSelf(SPELL_SSZORAK_DIG_IN, true);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SSZORAK_CORRODING_VENOM:
                castSpellOnVictim(SPELL_SSZORAK_CORRODING_VENOM);
                scriptEvents.addEvent(EVENT_SSZORAK_CORRODING_VENOM, 12000);
                break;
            case EVENT_SSZORAK_APEX_PREDATOR:
                announce("Sszorak strikes as the Apex Predator!");
                castSpellOnSelf(SPELL_SSZORAK_APEX_PREDATOR, true);
                scriptEvents.addEvent(EVENT_SSZORAK_RAVAGE, 2000);
                scriptEvents.addEvent(EVENT_SSZORAK_MUTILATE, 5000);
                scriptEvents.addEvent(EVENT_SSZORAK_APEX_PREDATOR, 32000);
                break;
            case EVENT_SSZORAK_RAVAGE:
                castSpellOnVictim(SPELL_SSZORAK_RAVAGE);
                break;
            case EVENT_SSZORAK_MUTILATE:
                castSpellOnVictim(SPELL_SSZORAK_MUTILATE);
                break;
            case EVENT_SSZORAK_VENOMOUS_SURGE:
                castOnPlayer(SPELL_SSZORAK_VENOMOUS_SURGE, true);
                scriptEvents.addEvent(EVENT_SSZORAK_VENOMOUS_SURGE, 30000);
                break;
            case EVENT_SSZORAK_RAGING_CROSSWINDS:
                castOnPlayers(SPELL_SSZORAK_RAGING_CROSSWINDS, 2, false);
                scriptEvents.addEvent(EVENT_SSZORAK_RAGING_CROSSWINDS, 36000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Twin Fangs

// the surviving twin is overcome by wrath
class VenomousAbyssTwinAI : public VenomousAbyssBossAI
{
public:
    explicit VenomousAbyssTwinAI(Creature* pCreature) : VenomousAbyssBossAI(pCreature, DATA_TWIN_FANGS) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_ABYSS_UNCOILED_WRATH || !isAlive() || !_isInCombat())
            return;

        announce("The surviving Twin Fang is overcome by Uncoiled Wrath!");
        castSpellOnSelf(SPELL_TWINS_UNCOILED_WRATH, true);
    }

protected:
    void onDefeat() override
    {
        for (Creature* pPartner : livingPartners())
            signalPartner(pPartner, ACTION_ABYSS_UNCOILED_WRATH);
    }
};

class VenomousAbyssVexhulAI : public VenomousAbyssTwinAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssVexhulAI(c); }
    explicit VenomousAbyssVexhulAI(Creature* pCreature) : VenomousAbyssTwinAI(pCreature) {}

protected:
    void onEngage() override
    {
        startBerserkClock(AbyssBerserkMs);
        scriptEvents.addEvent(EVENT_VEXHUL_CONCENTRATED_SPITTLE, 6000);
        scriptEvents.addEvent(EVENT_VEXHUL_CAUSTIC_DELUGE, 12000);
        scriptEvents.addEvent(EVENT_VEXHUL_VENOMOUS_EMERGENCE, 20000);
        scriptEvents.addEvent(EVENT_VEXHUL_STIR_THE_DEPTHS, 30000);
        scriptEvents.addEvent(EVENT_VEXHUL_VILE_FLOOD, 38000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VEXHUL_CONCENTRATED_SPITTLE:
                castSpellOnVictim(SPELL_VEXHUL_CONCENTRATED_SPITTLE);
                scriptEvents.addEvent(EVENT_VEXHUL_CONCENTRATED_SPITTLE, 15000);
                break;
            case EVENT_VEXHUL_CAUSTIC_DELUGE:
                castOnPlayer(SPELL_VEXHUL_CAUSTIC_DELUGE, true);
                scriptEvents.addEvent(EVENT_VEXHUL_CAUSTIC_DELUGE, 28000);
                break;
            case EVENT_VEXHUL_VENOMOUS_EMERGENCE:
                announce("Vexhul calls her progeny with Venomous Emergence!");
                castSpellAOE(SPELL_VEXHUL_VENOMOUS_EMERGENCE);
                for (uint8_t i = 0; i < 3; ++i)
                    bringAdd(NPC_SPAWN_OF_VEXHUL, ringAroundSelf(6.0f, 14.0f));
                scriptEvents.addEvent(EVENT_VEXHUL_VENOMOUS_EMERGENCE, 45000);
                break;
            case EVENT_VEXHUL_STIR_THE_DEPTHS:
                castSpellAOE(SPELL_VEXHUL_STIR_THE_DEPTHS);
                scriptEvents.addEvent(EVENT_VEXHUL_STIR_THE_DEPTHS, 40000);
                break;
            case EVENT_VEXHUL_VILE_FLOOD:
                castSpellAOE(SPELL_VEXHUL_VILE_FLOOD);
                scriptEvents.addEvent(EVENT_VEXHUL_VILE_FLOOD, 50000);
                break;
            default:
                break;
        }
    }
};

class VenomousAbyssIthrazAI : public VenomousAbyssTwinAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssIthrazAI(c); }
    explicit VenomousAbyssIthrazAI(Creature* pCreature) : VenomousAbyssTwinAI(pCreature) {}

protected:
    void onEngage() override
    {
        startBerserkClock(AbyssBerserkMs);
        scriptEvents.addEvent(EVENT_ITHRAZ_STONE_BREAKER, 8000);
        scriptEvents.addEvent(EVENT_ITHRAZ_RAVENOUS_FEAST, 15000);
        scriptEvents.addEvent(EVENT_ITHRAZ_COILING_ICHOR, 22000);
        scriptEvents.addEvent(EVENT_ITHRAZ_ROUSE_THE_BROOD, 28000);
        scriptEvents.addEvent(EVENT_ITHRAZ_BLOOD_TORRENT, 40000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ITHRAZ_STONE_BREAKER:
                castSpellOnVictim(SPELL_ITHRAZ_STONE_BREAKER);
                scriptEvents.addEvent(EVENT_ITHRAZ_STONE_BREAKER, 16000);
                break;
            case EVENT_ITHRAZ_RAVENOUS_FEAST:
                announce("Ithraz begins a Ravenous Feast!");
                castOnPlayer(SPELL_ITHRAZ_RAVENOUS_FEAST, false);
                scriptEvents.addEvent(EVENT_ITHRAZ_RAVENOUS_FEAST, 30000);
                break;
            case EVENT_ITHRAZ_COILING_ICHOR:
                castOnPlayers(SPELL_ITHRAZ_COILING_ICHOR, 2, true);
                scriptEvents.addEvent(EVENT_ITHRAZ_COILING_ICHOR, 30000);
                break;
            case EVENT_ITHRAZ_ROUSE_THE_BROOD:
                castSpellAOE(SPELL_ITHRAZ_ROUSE_THE_BROOD);
                for (uint8_t i = 0; i < 4; ++i)
                    bringAdd(NPC_BROODLING_OF_ITHRAZ, ringAroundSelf(5.0f, 12.0f));
                scriptEvents.addEvent(EVENT_ITHRAZ_ROUSE_THE_BROOD, 50000);
                break;
            case EVENT_ITHRAZ_BLOOD_TORRENT:
                castSpellAOE(SPELL_ITHRAZ_BLOOD_TORRENT);
                bringAdd(NPC_BARBED_BULWARK, ringAroundSelf(8.0f, 14.0f));
                scriptEvents.addEvent(EVENT_ITHRAZ_BLOOD_TORRENT, 55000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Coiled Altar: Zul'jan yields, Malacrass fights, then both together

class VenomousAbyssZuljanAI : public VenomousAbyssBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssZuljanAI(c); }
    explicit VenomousAbyssZuljanAI(Creature* pCreature) : VenomousAbyssBossAI(pCreature, DATA_COILED_ALTAR) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mFallen)
        {
            *damage = 0;
            return;
        }

        // only stage one ends with Zul'jan on his knees
        VenomousAbyssInstanceScript* pAbyss = getAbyss();
        if (pAbyss == nullptr || pAbyss->getAltarStage() != AltarStageZuljan || !_isInCombat())
            return;

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        mFallPending = true;
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_ABYSS_ZULJAN_RISES)
            rise();
        else if (action == ACTION_ABYSS_WIPE_RESET && !_isInCombat())
            standUp();
    }

protected:
    void onEngage() override
    {
        mFallPending = false;

        VenomousAbyssInstanceScript* pAbyss = getAbyss();
        if (pAbyss != nullptr && pAbyss->getAltarStage() == AltarStageUnion)
        {
            startUnion();
            return;
        }

        mFallen = false;
        startBerserkClock(AbyssLongBerserkMs);
        scriptEvents.addEvent(EVENT_ZULJAN_VENOMFANG, 6000);
        scriptEvents.addEvent(EVENT_ZULJAN_SEVER, 10000);
        scriptEvents.addEvent(EVENT_ZULJAN_TOXIC_DELUGE, 15000);
        scriptEvents.addEvent(EVENT_ZULJAN_AXEGRINDER, 18000);
        scriptEvents.addEvent(EVENT_ZULJAN_GUILLOTINE, 25000);
        scriptEvents.addEvent(EVENT_ZULJAN_FANGS_OF_THE_ALTAR, 32000);
    }

    void onReset() override
    {
        mFallPending = false;

        // fallen while Malacrass fights on
        if (mFallen && isEncounterRunning())
            return;

        standUp();
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mFallPending || _isCasting())
            return;

        mFallPending = false;
        collapse();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ZULJAN_VENOMFANG:
                castOnPlayer(SPELL_ZULJAN_VENOMFANG, false);
                scriptEvents.addEvent(EVENT_ZULJAN_VENOMFANG, 14000);
                break;
            case EVENT_ZULJAN_SEVER:
                castSpellOnVictim(SPELL_ZULJAN_SEVER);
                ruptureNearbyVenom();
                scriptEvents.addEvent(EVENT_ZULJAN_SEVER, 20000);
                break;
            case EVENT_ZULJAN_TOXIC_DELUGE:
                castSpellAOE(SPELL_ZULJAN_TOXIC_DELUGE);
                for (uint8_t i = 0; i < 3; ++i)
                    placeAdd(NPC_COALESCED_VENOM, ringAroundSelf(10.0f, 25.0f));
                scriptEvents.addEvent(EVENT_ZULJAN_TOXIC_DELUGE, 40000);
                break;
            case EVENT_ZULJAN_AXEGRINDER:
                castSpellAOE(SPELL_ZULJAN_AXEGRINDER);
                scriptEvents.addEvent(EVENT_ZULJAN_AXEGRINDER, 26000);
                break;
            case EVENT_ZULJAN_GUILLOTINE:
                if (castOnPlayer(SPELL_ZULJAN_GUILLOTINE, true) != nullptr)
                    announce("Zul'jan raises his axes for the Guillotine!");
                scriptEvents.addEvent(EVENT_ZULJAN_GUILLOTINE, 35000);
                break;
            case EVENT_ZULJAN_FANGS_OF_THE_ALTAR:
                castSpellAOE(SPELL_ZULJAN_FANGS_OF_THE_COILED_ALTAR);
                scriptEvents.addEvent(EVENT_ZULJAN_FANGS_OF_THE_ALTAR, 45000);
                break;
            case EVENT_ZULJAN_DEFILEMENT:
                castSpellAOE(SPELL_ZULJAN_DEFILEMENT);
                scriptEvents.addEvent(EVENT_ZULJAN_DEFILEMENT, 40000);
                break;
            case EVENT_ZULJAN_GRIM_GUILLOTINE:
                if (castOnPlayer(SPELL_ZULJAN_GRIM_GUILLOTINE, true) != nullptr)
                    announce("Zul'jan prepares a Grim Guillotine!");
                scriptEvents.addEvent(EVENT_ZULJAN_GRIM_GUILLOTINE, 35000);
                break;
            case EVENT_ZULJAN_BLIGHTED_SEVER:
                castSpellOnVictim(SPELL_ZULJAN_BLIGHTED_SEVER);
                ruptureNearbyVenom();
                scriptEvents.addEvent(EVENT_ZULJAN_BLIGHTED_SEVER, 18000);
                break;
            default:
                break;
        }
    }

private:
    void collapse()
    {
        std::vector<Creature*> masters;
        for (Creature* pPartner : livingPartners())
        {
            if (isMalacrassEntry(pPartner->getEntry()))
                masters.push_back(pPartner);
        }

        VenomousAbyssInstanceScript* pAbyss = getAbyss();

        // without Malacrass the next blow ends Zul'jan
        if (masters.empty())
        {
            if (pAbyss != nullptr)
                pAbyss->setAltarStage(AltarStageUnion);
            return;
        }

        mFallen = true;
        scriptEvents.resetEvents();
        setUntouchable(true);
        announce("Zul'jan collapses! Hex Lord Malacrass joins the fray!");

        if (pAbyss != nullptr)
            pAbyss->setAltarStage(AltarStageMalacrass);

        for (Creature* pMaster : masters)
            signalPartner(pMaster, ACTION_ABYSS_MALACRASS_JOINS);
    }

    // bound to Malacrass, Zul'jan fights on
    void rise()
    {
        if (!mFallen || !isAlive())
            return;

        mFallen = false;
        setUntouchable(false);
        getCreature()->setHealth(getCreature()->getMaxHealth() / 2);
        castSpellOnSelf(SPELL_ZULJAN_SOULBOUND, true);
        announce("Zul'jan rises again, soulbound to Hex Lord Malacrass!");

        if (_isInCombat())
            startUnion();
        else
            setZoneWideCombat();
    }

    void standUp()
    {
        mFallen = false;
        setUntouchable(false);
    }

    void startUnion()
    {
        scriptEvents.resetEvents();
        scriptEvents.addEvent(EVENT_ZULJAN_BLIGHTED_SEVER, 8000);
        scriptEvents.addEvent(EVENT_ZULJAN_DEFILEMENT, 12000);
        scriptEvents.addEvent(EVENT_ZULJAN_TOXIC_DELUGE, 18000);
        scriptEvents.addEvent(EVENT_ZULJAN_GRIM_GUILLOTINE, 25000);
    }

    // the axes tear through venom near the tank
    void ruptureNearbyVenom()
    {
        std::list<Creature*> venoms;
        GetCreatureListWithEntryInGrid(venoms, NPC_COALESCED_VENOM, SeverVenomReach);
        for (Creature* pVenom : venoms)
        {
            if (pVenom->isAlive() && pVenom->GetScript() != nullptr)
                pVenom->GetScript()->DoAction(ACTION_ABYSS_RUPTURE);
        }
    }

    bool mFallen = false;
    bool mFallPending = false;
};

class VenomousAbyssMalacrassAI : public VenomousAbyssBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssMalacrassAI(c); }
    explicit VenomousAbyssMalacrassAI(Creature* pCreature) : VenomousAbyssBossAI(pCreature, DATA_COILED_ALTAR) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_ABYSS_MALACRASS_JOINS)
        {
            // out of the fight he is pulled in, the stage makes him join
            if (!_isInCombat())
                setZoneWideCombat();
            else if (mWaiting)
                join();
        }
        else if (action == ACTION_ABYSS_WIPE_RESET && !_isInCombat())
        {
            mWaiting = false;
            setUntouchable(false);
        }
    }

protected:
    void onEngage() override
    {
        mWaiting = false;
        mVesselClaimed = false;
        startBerserkClock(AbyssLongBerserkMs);

        VenomousAbyssInstanceScript* pAbyss = getAbyss();
        uint32_t const stage = pAbyss != nullptr ? pAbyss->getAltarStage() : AltarStageMalacrass;

        // stays aside while Zul'jan stands
        if (stage == AltarStageZuljan && findZuljan() != nullptr)
        {
            mWaiting = true;
            setUntouchable(true);
            return;
        }

        if (stage == AltarStageUnion)
        {
            mVesselClaimed = true;
            startUnion();
            return;
        }

        join();
    }

    void onReset() override
    {
        mWaiting = false;
        mVesselClaimed = false;
        _removeAura(SPELL_MALACRASS_VEIL_OF_TWILIGHT);
        setUntouchable(false);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mWaiting || mVesselClaimed || _isCasting())
            return;

        if (_getHealthPercent() <= MalacrassVesselPct)
            claimVessel();
    }

    // a lying Zul'jan gets up to finish the fight
    void onDefeat() override
    {
        if (VenomousAbyssInstanceScript* pAbyss = getAbyss())
            pAbyss->setAltarStage(AltarStageUnion);

        signalPartner(findZuljan(), ACTION_ABYSS_ZULJAN_RISES);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MALACRASS_DREAD_BOLT:
                castOnPlayer(SPELL_MALACRASS_DREAD_BOLT, false);
                scriptEvents.addEvent(EVENT_MALACRASS_DREAD_BOLT, 7000);
                break;
            case EVENT_MALACRASS_SOUL_SEVER:
                castSpellOnVictim(SPELL_MALACRASS_SOUL_SEVER);
                scriptEvents.addEvent(EVENT_MALACRASS_SOUL_SEVER, 18000);
                break;
            case EVENT_MALACRASS_DREADMARCH:
                if (Unit* pTarget = castOnPlayer(SPELL_MALACRASS_DREADMARCH, true))
                    bringAdd(NPC_MANIFESTATION_OF_DREAD, abyssRingPoint(pTarget->GetPosition(), 3.0f, 6.0f));
                scriptEvents.addEvent(EVENT_MALACRASS_DREADMARCH, 38000);
                break;
            case EVENT_MALACRASS_GLOOMBOMB:
                castOnPlayer(SPELL_MALACRASS_GLOOMBOMB, false);
                scriptEvents.addEvent(EVENT_MALACRASS_GLOOMBOMB, 28000);
                break;
            case EVENT_MALACRASS_SPIRITCACKLE:
                castSpellAOE(SPELL_MALACRASS_SPIRITCACKLE);
                bringAdd(NPC_SPITEFUL_SOULCOILER, ringAroundSelf(6.0f, 12.0f));
                scriptEvents.addEvent(EVENT_MALACRASS_SPIRITCACKLE, 48000);
                break;
            case EVENT_MALACRASS_ETERNAL_NIGHTFALL:
                announce("Hex Lord Malacrass shrouds himself in a Veil of Twilight! Break it before the Eternal Nightfall!");
                castSpellOnSelf(SPELL_MALACRASS_VEIL_OF_TWILIGHT, true);
                scriptEvents.addEvent(EVENT_MALACRASS_NIGHTFALL_DESCENDS, NightfallDelayMs);
                scriptEvents.addEvent(EVENT_MALACRASS_ETERNAL_NIGHTFALL, 60000);
                break;
            case EVENT_MALACRASS_NIGHTFALL_DESCENDS:
                // an unbroken veil lets the night fall
                if (hasAura(SPELL_MALACRASS_VEIL_OF_TWILIGHT))
                {
                    _removeAura(SPELL_MALACRASS_VEIL_OF_TWILIGHT);
                    castSpellAOE(SPELL_MALACRASS_ETERNAL_NIGHTFALL, true);
                }
                break;
            case EVENT_MALACRASS_VESSEL_BOUND:
                bindVessel();
                break;
            default:
                break;
        }
    }

private:
    Creature* findZuljan()
    {
        for (Creature* pPartner : livingPartners())
        {
            if (isZuljanEntry(pPartner->getEntry()))
                return pPartner;
        }

        return nullptr;
    }

    // stage two
    void join()
    {
        mWaiting = false;
        setUntouchable(false);

        VenomousAbyssInstanceScript* pAbyss = getAbyss();
        if (pAbyss != nullptr && pAbyss->getAltarStage() < AltarStageMalacrass)
            pAbyss->setAltarStage(AltarStageMalacrass);

        scriptEvents.resetEvents();
        scriptEvents.addEvent(EVENT_MALACRASS_DREAD_BOLT, 3000);
        scriptEvents.addEvent(EVENT_MALACRASS_SOUL_SEVER, 8000);
        scriptEvents.addEvent(EVENT_MALACRASS_DREADMARCH, 14000);
        scriptEvents.addEvent(EVENT_MALACRASS_GLOOMBOMB, 20000);
        scriptEvents.addEvent(EVENT_MALACRASS_SPIRITCACKLE, 26000);
        scriptEvents.addEvent(EVENT_MALACRASS_ETERNAL_NIGHTFALL, 40000);
    }

    // intermission: Malacrass binds himself to Zul'jan
    void claimVessel()
    {
        mVesselClaimed = true;
        scriptEvents.resetEvents();
        _removeAura(SPELL_MALACRASS_VEIL_OF_TWILIGHT);
        announce("Hex Lord Malacrass claims Zul'jan as his vessel!");

        if (VenomousAbyssInstanceScript* pAbyss = getAbyss())
            pAbyss->setAltarStage(AltarStageVessel);

        if (Creature* pZuljan = findZuljan())
            castSpell(pZuljan, SPELL_MALACRASS_SOULBINDING, true);

        for (uint8_t i = 0; i < 4; ++i)
            bringAdd(NPC_FRAGMENT_OF_MALACRASS_PLACEHOLDER, ringAroundSelf(15.0f, 25.0f));

        scriptEvents.addEvent(EVENT_MALACRASS_DREAD_BOLT, 4000);
        scriptEvents.addEvent(EVENT_MALACRASS_VESSEL_BOUND, VesselBindingMs);
    }

    // stage three: Coiled Union
    void bindVessel()
    {
        if (VenomousAbyssInstanceScript* pAbyss = getAbyss())
            pAbyss->setAltarStage(AltarStageUnion);

        if (Creature* pZuljan = findZuljan())
        {
            castSpell(pZuljan, SPELL_MALACRASS_GHASTLY_REGENERATION, true);
            signalPartner(pZuljan, ACTION_ABYSS_ZULJAN_RISES);
        }

        startUnion();
    }

    void startUnion()
    {
        scriptEvents.resetEvents();
        scriptEvents.addEvent(EVENT_MALACRASS_DREAD_BOLT, 3000);
        scriptEvents.addEvent(EVENT_MALACRASS_DREADMARCH, 10000);
        scriptEvents.addEvent(EVENT_MALACRASS_GLOOMBOMB, 16000);
        scriptEvents.addEvent(EVENT_MALACRASS_SPIRITCACKLE, 24000);
        scriptEvents.addEvent(EVENT_MALACRASS_ETERNAL_NIGHTFALL, 35000);
    }

    bool mWaiting = false;
    bool mVesselClaimed = false;
};

// stationary venom pool, bursts when destroyed or cut by Sever
class VenomousAbyssCoalescedVenomAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssCoalescedVenomAI(c); }
    explicit VenomousAbyssCoalescedVenomAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mRuptured)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        rupture();
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_ABYSS_RUPTURE)
            rupture();
    }

private:
    void rupture()
    {
        if (mRuptured || !isAlive())
            return;

        mRuptured = true;
        castSpellAOE(SPELL_COALESCED_VENOM_RUPTURE, true);
        despawn(500, 0);
    }

    bool mRuptured = false;
};

// hunts a single player
class VenomousAbyssDreadAI : public VenomousAbyssCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssDreadAI(c); }
    explicit VenomousAbyssDreadAI(Creature* pCreature) : VenomousAbyssCombatAI(pCreature) {}

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        if (_isInCombat())
            fixate();
    }

protected:
    void onEngage() override { fixate(); }

private:
    void fixate()
    {
        Unit* pPrey = rollPlayer(false);
        if (pPrey == nullptr)
            return;

        castSpell(pPrey, SPELL_DREAD_UNNERVING_FIXATION, true);
        addThreat(pPrey, 1000000.0f);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Entombed Sentinels

class VenomousAbyssBreathAI : public VenomousAbyssBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssBreathAI(c); }
    explicit VenomousAbyssBreathAI(Creature* pCreature) : VenomousAbyssBossAI(pCreature, DATA_ENTOMBED_SENTINELS) {}

protected:
    void onEngage() override
    {
        startBerserkClock(AbyssBerserkMs);
        scriptEvents.addEvent(EVENT_BREATH_EMPOWERING_SLAM, 7000);
        scriptEvents.addEvent(EVENT_BREATH_TOXIC_DROPLETS, 12000);
        scriptEvents.addEvent(EVENT_BREATH_LIVING_VENOM, 18000);
        scriptEvents.addEvent(EVENT_BREATH_VENOM_COAGULATION, 30000);
    }

    // both Sentinels reach full energy together every 50 seconds
    void onCombatTick(uint32_t diff) override
    {
        if (fillGauge(diff, 2))
            vitriolicHomeostasis();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BREATH_EMPOWERING_SLAM:
                castSpellOnVictim(SPELL_BREATH_EMPOWERING_SLAM);
                scriptEvents.addEvent(EVENT_BREATH_EMPOWERING_SLAM, 16000);
                break;
            case EVENT_BREATH_TOXIC_DROPLETS:
                castSpellAOE(SPELL_BREATH_TOXIC_DROPLETS);
                scriptEvents.addEvent(EVENT_BREATH_TOXIC_DROPLETS, 22000);
                break;
            case EVENT_BREATH_LIVING_VENOM:
                castOnPlayer(SPELL_BREATH_LIVING_VENOM, true);
                scriptEvents.addEvent(EVENT_BREATH_LIVING_VENOM, 30000);
                break;
            case EVENT_BREATH_VENOM_COAGULATION:
                castSpellAOE(SPELL_BREATH_VENOM_COAGULATION);
                scriptEvents.addEvent(EVENT_BREATH_VENOM_COAGULATION, 45000);
                break;
            default:
                break;
        }
    }

private:
    // the weaker Sentinel recovers to the health share of the stronger one
    void vitriolicHomeostasis()
    {
        Creature* pBlood = nullptr;
        for (Creature* pPartner : livingPartners())
        {
            if (pPartner->getEntry() == BOSS_BLOOD_OF_ULATEK)
                pBlood = pPartner;
        }

        if (pBlood == nullptr)
            return;

        announce("The Entombed Sentinels come together in Vitriolic Homeostasis!");
        castSpellOnSelf(SPELL_SENTINEL_VITRIOLIC_STASIS, true);
        signalPartner(pBlood, ACTION_ABYSS_SENTINEL_STASIS);

        Creature* pSelf = getCreature();
        uint32_t const ownShare = healthShare(pSelf);
        uint32_t const bloodShare = healthShare(pBlood);
        Creature* pWeaker = ownShare < bloodShare ? pSelf : pBlood;
        uint32_t const share = ownShare < bloodShare ? bloodShare : ownShare;
        pWeaker->setHealth(static_cast<uint32_t>(static_cast<uint64_t>(pWeaker->getMaxHealth()) * share / 100));

        castOnPlayers(SPELL_SENTINEL_HELICAL_TOXINS, 3, false);
    }
};

class VenomousAbyssBloodAI : public VenomousAbyssBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssBloodAI(c); }
    explicit VenomousAbyssBloodAI(Creature* pCreature) : VenomousAbyssBossAI(pCreature, DATA_ENTOMBED_SENTINELS) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_ABYSS_SENTINEL_STASIS && isAlive() && _isInCombat())
            castSpellOnSelf(SPELL_SENTINEL_VITRIOLIC_STASIS, true);
    }

protected:
    void onEngage() override
    {
        startBerserkClock(AbyssBerserkMs);
        scriptEvents.addEvent(EVENT_BLOOD_BLOODVENOM_INJECTION, 7000);
        scriptEvents.addEvent(EVENT_BLOOD_BLIGHTED_BLOOD, 12000);
        scriptEvents.addEvent(EVENT_BLOOD_BLOOD_VENOM, 18000);
        scriptEvents.addEvent(EVENT_BLOOD_UNSTABLE_MIASMA, 26000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BLOOD_BLOODVENOM_INJECTION:
                castSpellOnVictim(SPELL_BLOOD_BLOODVENOM_INJECTION);
                scriptEvents.addEvent(EVENT_BLOOD_BLOODVENOM_INJECTION, 16000);
                break;
            case EVENT_BLOOD_BLIGHTED_BLOOD:
                castOnPlayers(SPELL_BLOOD_BLIGHTED_BLOOD, 2, true);
                scriptEvents.addEvent(EVENT_BLOOD_BLIGHTED_BLOOD, 24000);
                break;
            case EVENT_BLOOD_BLOOD_VENOM:
                castOnPlayer(SPELL_BLOOD_BLOOD_VENOM, false);
                scriptEvents.addEvent(EVENT_BLOOD_BLOOD_VENOM, 30000);
                break;
            case EVENT_BLOOD_UNSTABLE_MIASMA:
                castSpellAOE(SPELL_BLOOD_UNSTABLE_MIASMA);
                scriptEvents.addEvent(EVENT_BLOOD_UNSTABLE_MIASMA, 40000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Vashnik the Malignant

class VenomousAbyssVashnikAI : public VenomousAbyssBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssVashnikAI(c); }
    explicit VenomousAbyssVashnikAI(Creature* pCreature) : VenomousAbyssBossAI(pCreature, DATA_VASHNIK) {}

protected:
    void onEngage() override
    {
        mExpelling = false;
        mFortifiedClot = false;
        mExpulsionTurn = 0;
        mAffinity[0] = 0;
        mAffinity[1] = 1;

        startBerserkClock(AbyssBerserkMs);
        scriptEvents.addEvent(EVENT_VASHNIK_DRIPPING_FANGS, 6000);
        scriptEvents.addEvent(EVENT_VASHNIK_MALIGNANT_CATALYST, 12000);
        scriptEvents.addEvent(EVENT_VASHNIK_ADAPTIVE_INFECTION, 18000);
        scriptEvents.addEvent(EVENT_VASHNIK_PLAGUE_FROTH, 24000);
    }

    void onReset() override
    {
        for (VashnikFountain const& fountain : VashnikFountains)
            _removeAura(fountain.infusion);
    }

    // Imbibe at full energy every 50 seconds
    void onCombatTick(uint32_t diff) override
    {
        if (fillGauge(diff, 2))
            imbibe();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VASHNIK_DRIPPING_FANGS:
                castSpellOnVictim(SPELL_VASHNIK_DRIPPING_FANGS);
                scriptEvents.addEvent(EVENT_VASHNIK_DRIPPING_FANGS, 14000);
                break;
            case EVENT_VASHNIK_MALIGNANT_CATALYST:
                castOnPlayers(SPELL_VASHNIK_MALIGNANT_CATALYST, 2, true);
                scriptEvents.addEvent(EVENT_VASHNIK_MALIGNANT_CATALYST, 25000);
                break;
            case EVENT_VASHNIK_ADAPTIVE_INFECTION:
                castOnPlayers(SPELL_VASHNIK_ADAPTIVE_INFECTION, 2, true);
                scriptEvents.addEvent(EVENT_VASHNIK_ADAPTIVE_INFECTION, 30000);
                break;
            case EVENT_VASHNIK_PLAGUE_FROTH:
                castSpellAOE(SPELL_VASHNIK_PLAGUE_FROTH);
                scriptEvents.addEvent(EVENT_VASHNIK_PLAGUE_FROTH, 35000);
                break;
            case EVENT_VASHNIK_EXPULSION:
                // the two fountains he drank from take turns
                castSpellAOE(VashnikFountains[mAffinity[mExpulsionTurn % 2]].expulsion);
                ++mExpulsionTurn;
                scriptEvents.addEvent(EVENT_VASHNIK_EXPULSION, 15000);
                break;
            default:
                break;
        }
    }

private:
    void imbibe()
    {
        uint32_t const first = Util::getRandomUInt(2);
        uint32_t const second = (first + 1 + Util::getRandomUInt(1)) % 3;

        announce("Vashnik Imbibes from two venomous fountains!");
        castSpellAOE(SPELL_VASHNIK_IMBIBE);
        castSpellAOE(SPELL_VASHNIK_TOXIC_VAPOR, true);

        for (uint32_t i = 0; i < 3; ++i)
        {
            if (i != first && i != second)
                _removeAura(VashnikFountains[i].infusion);
        }

        castSpellOnSelf(VashnikFountains[first].infusion, true);
        castSpellOnSelf(VashnikFountains[second].infusion, true);
        mAffinity[0] = first;
        mAffinity[1] = second;

        releaseVenoms(VashnikFountains[first].venomEntry);
        releaseVenoms(VashnikFountains[second].venomEntry);

        if (!mExpelling)
        {
            mExpelling = true;
            scriptEvents.addEvent(EVENT_VASHNIK_EXPULSION, 8000);
        }
    }

    // two living venoms per fountain crawl toward the cavity
    void releaseVenoms(uint32_t entry)
    {
        LocationVector const cavity = abyssHomeOf(getCreature());
        for (uint8_t i = 0; i < 2; ++i)
        {
            Creature* pVenom = placeAdd(entry, abyssRingPoint(cavity, 30.0f, 38.0f));
            if (pVenom == nullptr)
                continue;

            // only the first Clotting Venom is protected
            if (entry == NPC_CLOTTING_VENOM && !mFortifiedClot)
            {
                mFortifiedClot = true;
                pVenom->castSpell(pVenom, SPELL_CLOTTING_SANGUINEOUS_FORTITUDE, true);
            }
        }
    }

    bool mExpelling = false;
    bool mFortifiedClot = false;
    uint32_t mExpulsionTurn = 0;
    uint32_t mAffinity[2] = { 0, 1 };
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nek'zali the Soulcoiler

class VenomousAbyssNekzaliAI : public VenomousAbyssBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssNekzaliAI(c); }
    explicit VenomousAbyssNekzaliAI(Creature* pCreature) : VenomousAbyssBossAI(pCreature, DATA_NEKZALI) {}

    // the echoes shield her during the ritual
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && mWarded)
            *damage = 0;
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_ABYSS_SOUL_ENTERS_WELL || !isAlive() || !_isInCombat())
            return;

        announce("A restless soul reaches the Soulcoil Well!");
        castSpellOnSelf(SPELL_NEKZALI_UNCOILED_RAGE, true);
    }

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || !mWarded || !isEchoEntry(pSummon->getEntry()))
            return;

        for (uint32_t entry : EchoEntries)
        {
            std::list<Creature*> echoes;
            GetCreatureListWithEntryInGrid(echoes, entry, AbyssPlayerReach);
            for (Creature* pEcho : echoes)
            {
                if (pEcho != pSummon && pEcho->isAlive())
                    return;
            }
        }

        uncoil();
    }

protected:
    void onEngage() override
    {
        mRitualDone = false;
        mWarded = false;
        mCultistSpots.clear();

        startBerserkClock(AbyssBerserkMs);
        scriptEvents.addEvent(EVENT_NEKZALI_HOLLOWING_STRIKES, 6000);
        scriptEvents.addEvent(EVENT_NEKZALI_POSSESSION_BARRAGE, 11000);
        scriptEvents.addEvent(EVENT_NEKZALI_ESSENCE_REND, 16000);
        scriptEvents.addEvent(EVENT_NEKZALI_SOULCOIL_IGNITION, 24000);
        scriptEvents.addEvent(EVENT_NEKZALI_SOULCOIL_RITE, 30000);
    }

    void onReset() override
    {
        mWarded = false;
        mCultistSpots.clear();
        setRooted(false);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mRitualDone && !_isCasting() && _getHealthPercent() <= NekzaliRitualPct)
            beginRitual();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NEKZALI_HOLLOWING_STRIKES:
                castSpellOnVictim(SPELL_NEKZALI_HOLLOWING_STRIKES);
                scriptEvents.addEvent(EVENT_NEKZALI_HOLLOWING_STRIKES, 14000);
                break;
            case EVENT_NEKZALI_POSSESSION_BARRAGE:
                castSpellOnVictim(SPELL_NEKZALI_POSSESSION_BARRAGE);
                scriptEvents.addEvent(EVENT_NEKZALI_POSSESSION_BARRAGE, 20000);
                break;
            case EVENT_NEKZALI_ESSENCE_REND:
                // a cultist rises where the rend expires
                if (Unit* pTarget = castOnPlayer(SPELL_NEKZALI_ESSENCE_REND, true))
                {
                    mCultistSpots.push_back(pTarget->GetPosition());
                    scriptEvents.addEvent(EVENT_NEKZALI_LATENT_CULTIST, 10000);
                }
                scriptEvents.addEvent(EVENT_NEKZALI_ESSENCE_REND, 30000);
                break;
            case EVENT_NEKZALI_LATENT_CULTIST:
                if (!mCultistSpots.empty())
                {
                    bringAdd(NPC_LATENT_CULTIST, mCultistSpots.front());
                    mCultistSpots.erase(mCultistSpots.begin());
                }
                break;
            case EVENT_NEKZALI_SOULCOIL_IGNITION:
                castSpellAOE(SPELL_NEKZALI_SOULCOIL_IGNITION);
                scriptEvents.addEvent(EVENT_NEKZALI_SOULCOIL_IGNITION, 32000);
                break;
            case EVENT_NEKZALI_SOULCOIL_RITE:
                soulcoilRite();
                scriptEvents.addEvent(EVENT_NEKZALI_SOULCOIL_RITE, mWarded ? 25000 : 45000);
                break;
            case EVENT_NEKZALI_GRASPING_DEPTHS:
                castOnPlayer(SPELL_NEKZALI_GRASPING_DEPTHS, false);
                scriptEvents.addEvent(EVENT_NEKZALI_GRASPING_DEPTHS, 15000);
                break;
            case EVENT_NEKZALI_INVOKE:
                castSpellAOE(SPELL_NEKZALI_INVOKE);
                for (uint8_t i = 0; i < 2; ++i)
                    bringAdd(NPC_LATENT_CULTIST, ringAroundSelf(6.0f, 12.0f));
                soulcoilRite();
                scriptEvents.addEvent(EVENT_NEKZALI_INVOKE, 40000);
                break;
            default:
                break;
        }
    }

private:
    // restless spirits rise at the edge and walk to the well
    void soulcoilRite()
    {
        announce("Nek'zali invokes the Soulcoil Rite! The Restless Amani awaken!");
        castSpellAOE(SPELL_NEKZALI_SOULCOIL_RITE);
        if (isHeroic())
            castOnPlayers(SPELL_NEKZALI_RITUAL_BURN, 3, false);

        LocationVector const well = abyssHomeOf(getCreature());
        for (uint8_t i = 0; i < 3; ++i)
            placeAdd(NPC_RESTLESS_AMANI, abyssRingPoint(well, 26.0f, 34.0f));
    }

    // intermission: Echoes of Jawae shield her
    void beginRitual()
    {
        mRitualDone = true;
        mWarded = true;
        scriptEvents.resetEvents();
        setRooted(true);
        announce("Nek'zali retreats to the Soulcoil Well and begins the Ritual of Awakening!");

        uint32_t echoes = 0;
        for (uint32_t i = 0; i < NekzaliEchoCount; ++i)
        {
            Creature* pEcho = bringAdd(NPC_ECHO_OF_JAWAE, ringAroundSelf(10.0f, 16.0f));
            if (pEcho == nullptr)
                continue;

            pEcho->castSpell(getCreature(), SPELL_ECHO_TETHER_OF_AWAKENING, true);
            ++echoes;
        }

        if (echoes == 0)
        {
            uncoil();
            return;
        }

        scriptEvents.addEvent(EVENT_NEKZALI_GRASPING_DEPTHS, 6000);
        scriptEvents.addEvent(EVENT_NEKZALI_SOULCOIL_RITE, 10000);
    }

    // stage two
    void uncoil()
    {
        mWarded = false;
        setRooted(false);
        scriptEvents.resetEvents();
        announce("Nek'zali uncoils the Soulcoil Well!");
        castSpellOnSelf(SPELL_NEKZALI_UNCOILING, true);

        scriptEvents.addEvent(EVENT_NEKZALI_HOLLOWING_STRIKES, 5000);
        scriptEvents.addEvent(EVENT_NEKZALI_POSSESSION_BARRAGE, 10000);
        scriptEvents.addEvent(EVENT_NEKZALI_ESSENCE_REND, 15000);
        scriptEvents.addEvent(EVENT_NEKZALI_INVOKE, 20000);
    }

    bool mRitualDone = false;
    bool mWarded = false;
    std::vector<LocationVector> mCultistSpots;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ula'tek

class VenomousAbyssUlatekAI : public VenomousAbyssBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssUlatekAI(c); }
    explicit VenomousAbyssUlatekAI(Creature* pCreature) : VenomousAbyssBossAI(pCreature, DATA_ULATEK) {}

protected:
    void onEngage() override
    {
        mStage = 1;
        startBerserkClock(AbyssLongBerserkMs);
        bringTail();

        scriptEvents.addEvent(EVENT_ULATEK_UNCHECKED_RAGE, 4000);
        scriptEvents.addEvent(EVENT_ULATEK_CAUSTIC_WAVES, 8000);
        scriptEvents.addEvent(EVENT_ULATEK_CALL_OF_THE_SERPENT, 12000);
        scriptEvents.addEvent(EVENT_ULATEK_NECROTIC_VAPORS, 18000);
        scriptEvents.addEvent(EVENT_ULATEK_MOTHERS_WRATH, 26000);
        scriptEvents.addEvent(EVENT_ULATEK_RAGE_OF_THE_SHACKLED, 40000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (_isCasting())
            return;

        int32_t const health = _getHealthPercent();
        if (mStage == 1 && health <= UlatekDoomscalePct)
            childrenOfTheDoomscale();
        else if (mStage == 2 && health <= UlatekShatteringPct)
            theShattering();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ULATEK_UNCHECKED_RAGE:
                uncheckedRage();
                scriptEvents.addEvent(EVENT_ULATEK_UNCHECKED_RAGE, 4000);
                break;
            case EVENT_ULATEK_CAUSTIC_WAVES:
                castSpellAOE(SPELL_ULATEK_CAUSTIC_WAVES);
                scriptEvents.addEvent(EVENT_ULATEK_CAUSTIC_WAVES, 25000);
                break;
            case EVENT_ULATEK_CALL_OF_THE_SERPENT:
                callOfTheSerpent();
                scriptEvents.addEvent(EVENT_ULATEK_CALL_OF_THE_SERPENT, mStage == 4 ? 40000 : 35000);
                break;
            case EVENT_ULATEK_NECROTIC_VAPORS:
                castSpellAOE(SPELL_ULATEK_NECROTIC_VAPORS);
                scriptEvents.addEvent(EVENT_ULATEK_NECROTIC_VAPORS, 30000);
                break;
            case EVENT_ULATEK_MOTHERS_WRATH:
                castOnPlayer(SPELL_ULATEK_MOTHERS_WRATH, true);
                scriptEvents.addEvent(EVENT_ULATEK_MOTHERS_WRATH, 40000);
                break;
            case EVENT_ULATEK_RAGE_OF_THE_SHACKLED:
                announce("Ula'tek rages against her shackles!");
                castSpellAOE(SPELL_ULATEK_RAGE_OF_THE_SHACKLED);
                scriptEvents.addEvent(EVENT_ULATEK_RAGE_OF_THE_SHACKLED, 50000);
                break;
            case EVENT_ULATEK_VIRULENT_SPIT:
                castOnPlayer(SPELL_ULATEK_VIRULENT_SPIT, true);
                scriptEvents.addEvent(EVENT_ULATEK_VIRULENT_SPIT, 16000);
                break;
            case EVENT_ULATEK_BLIGHTSCALE_CLUTCH:
                castSpellAOE(SPELL_ULATEK_BLIGHTSCALE_CLUTCH);
                placeAdd(NPC_BLIGHTSCALE_CLUTCH, ringAroundSelf(12.0f, 22.0f));
                scriptEvents.addEvent(EVENT_ULATEK_BLIGHTSCALE_CLUTCH, 40000);
                break;
            case EVENT_ULATEK_SPECTRAL_COILS:
                castSpellAOE(SPELL_ULATEK_SPECTRAL_COILS, true);
                break;
            case EVENT_ULATEK_ASCENSION:
                ascension();
                break;
            case EVENT_ULATEK_SERPENTS_BITE:
                castSpellOnVictim(SPELL_ULATEK_SERPENTS_BITE);
                scriptEvents.addEvent(EVENT_ULATEK_SERPENTS_BITE, 18000);
                break;
            case EVENT_ULATEK_CIRCLING_PREY:
                castOnPlayer(SPELL_ULATEK_CIRCLING_PREY, true);
                scriptEvents.addEvent(EVENT_ULATEK_CIRCLING_PREY, 30000);
                break;
            default:
                break;
        }
    }

private:
    // a placed tail is used, otherwise it is called in
    void bringTail()
    {
        Creature* pTail = findNearestCreature(NPC_GORE_RATTLE, 60.0f);
        if (pTail != nullptr && pTail->isAlive())
        {
            if (!pTail->isInCombat())
                setZoneWideCombat(pTail);
            return;
        }

        bringAdd(NPC_GORE_RATTLE, ringAroundSelf(6.0f, 10.0f));
    }

    // without a target in melee reach she lashes out at everyone
    void uncheckedRage()
    {
        Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
        if (pVictim != nullptr && getCreature()->getDistance(pVictim) <= UlatekMeleeReach)
            return;

        castSpellAOE(SPELL_ULATEK_UNCHECKED_RAGE);
    }

    void callOfTheSerpent()
    {
        bool const risen = mStage == 4;
        castSpellAOE(risen ? SPELL_ULATEK_CALL_OF_THE_SERPENT_RISEN : SPELL_ULATEK_CALL_OF_THE_SERPENT);

        uint32_t const eggs = risen ? 3 : 4;
        for (uint32_t i = 0; i < eggs; ++i)
            placeAdd(NPC_BLIGHTSCALE_SPAWN, ringAroundSelf(10.0f, 25.0f));

        if (risen)
            bringAdd(NPC_BLIGHTSCALE_SHRIEKER, ringAroundSelf(15.0f, 25.0f));
    }

    // stage two
    void childrenOfTheDoomscale()
    {
        mStage = 2;
        announce("The Doomscale brood answers Ula'tek's call!");

        scriptEvents.removeEvent(EVENT_ULATEK_CAUSTIC_WAVES);
        scriptEvents.removeEvent(EVENT_ULATEK_CALL_OF_THE_SERPENT);
        scriptEvents.removeEvent(EVENT_ULATEK_MOTHERS_WRATH);

        for (uint8_t i = 0; i < 2; ++i)
            bringAdd(NPC_DOOMSCALE_WARDEN, ringAroundSelf(12.0f, 20.0f));
        placeAdd(NPC_DOOMSCALE_EGG, ringAroundSelf(15.0f, 22.0f));

        scriptEvents.addEvent(EVENT_ULATEK_VIRULENT_SPIT, 6000);
        scriptEvents.addEvent(EVENT_ULATEK_BLIGHTSCALE_CLUTCH, 12000);
    }

    // intermission
    void theShattering()
    {
        mStage = 3;
        scriptEvents.resetEvents();
        announce("Ula'tek shatters her prison with her Spectral Coils!");
        castSpellAOE(SPELL_ULATEK_SPECTRAL_COILS);

        scriptEvents.addEvent(EVENT_ULATEK_SPECTRAL_COILS, 5000);
        scriptEvents.addEvent(EVENT_ULATEK_ASCENSION, 10000);
    }

    // stage three
    void ascension()
    {
        mStage = 4;
        announce("Ula'tek ascends, her fury unleashed!");
        castSpellOnSelf(SPELL_ULATEK_FURY_UNLEASHED, true);

        scriptEvents.addEvent(EVENT_ULATEK_UNCHECKED_RAGE, 4000);
        scriptEvents.addEvent(EVENT_ULATEK_SERPENTS_BITE, 6000);
        scriptEvents.addEvent(EVENT_ULATEK_CAUSTIC_WAVES, 10000);
        scriptEvents.addEvent(EVENT_ULATEK_CIRCLING_PREY, 14000);
        scriptEvents.addEvent(EVENT_ULATEK_CALL_OF_THE_SERPENT, 18000);
        scriptEvents.addEvent(EVENT_ULATEK_MOTHERS_WRATH, 26000);
        scriptEvents.addEvent(EVENT_ULATEK_NECROTIC_VAPORS, 32000);
    }

    uint32_t mStage = 1;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Lost Explorers: Mor'zahi and the possessed tortollans

class VenomousAbyssMorzahiAI : public VenomousAbyssBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssMorzahiAI(c); }
    explicit VenomousAbyssMorzahiAI(Creature* pCreature) : VenomousAbyssBossAI(pCreature, DATA_LOST_EXPLORERS) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_ABYSS_EXPLORER_FREED || mAscended || !_isInCombat())
            return;

        if (possessedExplorers().empty())
            ascend();
    }

protected:
    void onEngage() override
    {
        mAscended = false;
        startBerserkClock(AbyssBerserkMs);
        scriptEvents.addEvent(EVENT_MORZAHI_EVIL_EYES, 6000);
        scriptEvents.addEvent(EVENT_MORZAHI_DARK_WHISPERS, 10000);
        scriptEvents.addEvent(EVENT_MORZAHI_MALEVOLENT_PRESENCE, 15000);
    }

    // Mor'zahi's Command at full energy every 50 seconds
    void onCombatTick(uint32_t diff) override
    {
        if (!mAscended && fillGauge(diff, 2))
            command();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MORZAHI_EVIL_EYES:
                castOnPlayers(SPELL_MORZAHI_EVIL_EYES, 2, true);
                scriptEvents.addEvent(EVENT_MORZAHI_EVIL_EYES, mAscended ? 12000 : 18000);
                break;
            case EVENT_MORZAHI_DARK_WHISPERS:
                if (Creature* pExplorer = pickPossessed())
                    castSpell(pExplorer, SPELL_MORZAHI_DARK_WHISPERS, true);
                scriptEvents.addEvent(EVENT_MORZAHI_DARK_WHISPERS, 30000);
                break;
            case EVENT_MORZAHI_MALEVOLENT_PRESENCE:
                castSpellAOE(SPELL_MORZAHI_MALEVOLENT_PRESENCE);
                scriptEvents.addEvent(EVENT_MORZAHI_MALEVOLENT_PRESENCE, 25000);
                break;
            default:
                break;
        }
    }

private:
    std::vector<Creature*> possessedExplorers()
    {
        std::vector<Creature*> possessed;
        for (Creature* pPartner : livingPartners())
        {
            if (pPartner->getEntry() == BOSS_MORZAHI || pPartner->GetScript() == nullptr)
                continue;

            if (pPartner->GetScript()->GetCreatureData(ExplorerFreedData) == 0)
                possessed.push_back(pPartner);
        }

        return possessed;
    }

    Creature* pickPossessed()
    {
        std::vector<Creature*> const possessed = possessedExplorers();
        if (possessed.empty())
            return nullptr;

        return possessed[Util::getRandomUInt(static_cast<uint32_t>(possessed.size() - 1))];
    }

    // spends the energy to drive one puppet harder
    void command()
    {
        Creature* pExplorer = pickPossessed();
        if (pExplorer == nullptr)
            return;

        announce("Mor'zahi's Command empowers a possessed explorer!");
        castSpell(pExplorer, SPELL_MORZAHI_COMMAND, true);
        signalPartner(pExplorer, ACTION_ABYSS_EXPLORER_EMPOWER);
    }

    void ascend()
    {
        mAscended = true;
        announce("Robbed of his puppets, Mor'zahi begins his Final Ascension!");
        castSpellOnSelf(SPELL_MORZAHI_FINAL_ASCENSION, true);
    }

    bool mAscended = false;
};

// broken free at the brink of death instead of dying
class VenomousAbyssExplorerAI : public VenomousAbyssBossAI
{
public:
    explicit VenomousAbyssExplorerAI(Creature* pCreature) : VenomousAbyssBossAI(pCreature, DATA_LOST_EXPLORERS, false) {}

    uint32_t GetCreatureData(uint32_t type) const override
    {
        return type == ExplorerFreedData && mFreed ? 1 : 0;
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mFreed)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        mFreePending = true;
    }

    void DoAction(int32_t action) override
    {
        switch (action)
        {
            case ACTION_ABYSS_EXPLORER_EMPOWER:
                if (!mFreed && _isInCombat())
                    onEmpowered();
                break;
            case ACTION_ABYSS_EXPLORER_RELEASE:
                if (!mFreed)
                    standDown();
                break;
            case ACTION_ABYSS_WIPE_RESET:
                if (!_isInCombat())
                    takeUp();
                break;
            default:
                break;
        }
    }

protected:
    void onEngage() override
    {
        mFreePending = false;
        if (isEncounterDone())
        {
            standDown();
            return;
        }

        mFreed = false;
        beginRoutine();
    }

    void onReset() override
    {
        mFreePending = false;
        if (isEncounterDone())
            standDown();
        else
            takeUp();
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mFreePending || mFreed || _isCasting())
            return;

        mFreePending = false;
        standDown();
        announce("A tortollan explorer breaks free of Mor'zahi's control!");

        for (Creature* pPartner : livingPartners())
        {
            if (pPartner->getEntry() == BOSS_MORZAHI)
                signalPartner(pPartner, ACTION_ABYSS_EXPLORER_FREED);
        }
    }

    virtual void beginRoutine() {}
    virtual void onEmpowered() {}

private:
    void standDown()
    {
        mFreed = true;
        scriptEvents.resetEvents();
        setUntouchable(true);
    }

    void takeUp()
    {
        mFreed = false;
        setUntouchable(false);
    }

    bool mFreed = false;
    bool mFreePending = false;
};

class VenomousAbyssGebboAI : public VenomousAbyssExplorerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssGebboAI(c); }
    explicit VenomousAbyssGebboAI(Creature* pCreature) : VenomousAbyssExplorerAI(pCreature) {}

protected:
    void beginRoutine() override
    {
        scriptEvents.addEvent(EVENT_GEBBO_SMASHING_SHOVEL, 6000);
        scriptEvents.addEvent(EVENT_GEBBO_THROW_JUNK, 10000);
        scriptEvents.addEvent(EVENT_GEBBO_MUSHROOM_TOSS, 16000);
        scriptEvents.addEvent(EVENT_GEBBO_EXPLOSIVE_SURPRISE, 24000);
    }

    void onEmpowered() override { castSpellAOE(SPELL_GEBBO_EXPLOSIVE_SURPRISE, true); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GEBBO_SMASHING_SHOVEL:
                castSpellOnVictim(SPELL_GEBBO_SMASHING_SHOVEL);
                scriptEvents.addEvent(EVENT_GEBBO_SMASHING_SHOVEL, 14000);
                break;
            case EVENT_GEBBO_THROW_JUNK:
                castOnPlayer(SPELL_GEBBO_THROW_JUNK, false);
                scriptEvents.addEvent(EVENT_GEBBO_THROW_JUNK, 20000);
                break;
            case EVENT_GEBBO_MUSHROOM_TOSS:
                castOnPlayer(SPELL_GEBBO_MUSHROOM_TOSS, true);
                scriptEvents.addEvent(EVENT_GEBBO_MUSHROOM_TOSS, 30000);
                break;
            case EVENT_GEBBO_EXPLOSIVE_SURPRISE:
                castSpellAOE(SPELL_GEBBO_EXPLOSIVE_SURPRISE);
                scriptEvents.addEvent(EVENT_GEBBO_EXPLOSIVE_SURPRISE, 40000);
                break;
            default:
                break;
        }
    }
};

class VenomousAbyssIkuAI : public VenomousAbyssExplorerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssIkuAI(c); }
    explicit VenomousAbyssIkuAI(Creature* pCreature) : VenomousAbyssExplorerAI(pCreature) {}

protected:
    void beginRoutine() override
    {
        scriptEvents.addEvent(EVENT_IKU_SHREDDING_SHARDS, 5000);
        scriptEvents.addEvent(EVENT_IKU_FROSTFIRE_VOLLEY, 9000);
        scriptEvents.addEvent(EVENT_IKU_ICEBOUND_FLAMES, 14000);
        scriptEvents.addEvent(EVENT_IKU_BLINK_NOVA, 20000);
    }

    void onEmpowered() override
    {
        announce("Scrollsage Iku begins a Cataclysmic Invocation!");
        castSpellAOE(SPELL_IKU_CATACLYSMIC_INVOCATION);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_IKU_SHREDDING_SHARDS:
                castSpellOnVictim(SPELL_IKU_SHREDDING_SHARDS);
                scriptEvents.addEvent(EVENT_IKU_SHREDDING_SHARDS, 12000);
                break;
            case EVENT_IKU_FROSTFIRE_VOLLEY:
                castSpellAOE(SPELL_IKU_FROSTFIRE_VOLLEY);
                scriptEvents.addEvent(EVENT_IKU_FROSTFIRE_VOLLEY, 22000);
                break;
            case EVENT_IKU_ICEBOUND_FLAMES:
                castOnPlayer(SPELL_IKU_ICEBOUND_FLAMES, true);
                scriptEvents.addEvent(EVENT_IKU_ICEBOUND_FLAMES, 24000);
                break;
            case EVENT_IKU_BLINK_NOVA:
                castOnPlayer(SPELL_IKU_BLINK_NOVA, false);
                scriptEvents.addEvent(EVENT_IKU_BLINK_NOVA, 30000);
                break;
            default:
                break;
        }
    }
};

class VenomousAbyssNamaAI : public VenomousAbyssExplorerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VenomousAbyssNamaAI(c); }
    explicit VenomousAbyssNamaAI(Creature* pCreature) : VenomousAbyssExplorerAI(pCreature) {}

protected:
    void beginRoutine() override
    {
        castSpellOnSelf(SPELL_NAMA_STEADY_STRIKES, true);
        scriptEvents.addEvent(EVENT_NAMA_SHELL_SPIN, 10000);
        scriptEvents.addEvent(EVENT_NAMA_MIGHTY_THUD, 16000);
        scriptEvents.addEvent(EVENT_NAMA_UNITED_DEFENSE, 25000);
    }

    void onEmpowered() override { castSpellOnSelf(SPELL_NAMA_RELENTLESS_ESCALATION, true); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NAMA_SHELL_SPIN:
                castSpellAOE(SPELL_NAMA_SHELL_SPIN);
                scriptEvents.addEvent(EVENT_NAMA_SHELL_SPIN, 24000);
                break;
            case EVENT_NAMA_MIGHTY_THUD:
                announce("First Mate Nama prepares a Mighty Thud!");
                castSpellAOE(SPELL_NAMA_MIGHTY_THUD);
                scriptEvents.addEvent(EVENT_NAMA_MIGHTY_THUD, 35000);
                break;
            case EVENT_NAMA_UNITED_DEFENSE:
                castSpellAOE(SPELL_NAMA_UNITED_DEFENSE);
                scriptEvents.addEvent(EVENT_NAMA_UNITED_DEFENSE, 45000);
                break;
            default:
                break;
        }
    }
};

void SetupTheVenomousAbyss(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_VENOMOUS_ABYSS, &VenomousAbyssInstanceScript::Create);

    // Sszorak
    mgr->register_creature_script(BOSS_SSZORAK, &VenomousAbyssSszorakAI::Create);

    // The Twin Fangs
    mgr->register_creature_script(BOSS_VEXHUL, &VenomousAbyssVexhulAI::Create);
    mgr->register_creature_script(BOSS_ITHRAZ, &VenomousAbyssIthrazAI::Create);
    mgr->register_creature_script(NPC_SPAWN_OF_VEXHUL, &VenomousAbyssKitAI::Create);
    mgr->register_creature_script(NPC_BROODLING_OF_ITHRAZ, &VenomousAbyssKitAI::Create);
    mgr->register_creature_script(NPC_BARBED_BULWARK, &VenomousAbyssKitAI::Create);

    // The Coiled Altar
    mgr->register_creature_script(BOSS_ZULJAN, &VenomousAbyssZuljanAI::Create);
    mgr->register_creature_script(BOSS_ZULJAN_ALT, &VenomousAbyssZuljanAI::Create);
    mgr->register_creature_script(BOSS_ZULJAN_ALT2, &VenomousAbyssZuljanAI::Create);
    mgr->register_creature_script(BOSS_HEX_LORD_MALACRASS, &VenomousAbyssMalacrassAI::Create);
    mgr->register_creature_script(BOSS_HEX_LORD_MALACRASS_ALT, &VenomousAbyssMalacrassAI::Create);
    mgr->register_creature_script(NPC_COALESCED_VENOM, &VenomousAbyssCoalescedVenomAI::Create);
    mgr->register_creature_script(NPC_MANIFESTATION_OF_DREAD, &VenomousAbyssDreadAI::Create);
    mgr->register_creature_script(NPC_SPITEFUL_SOULCOILER, &VenomousAbyssKitAI::Create);

    // Entombed Sentinels
    mgr->register_creature_script(BOSS_BREATH_OF_ULATEK, &VenomousAbyssBreathAI::Create);
    mgr->register_creature_script(BOSS_BLOOD_OF_ULATEK, &VenomousAbyssBloodAI::Create);

    // Vashnik the Malignant
    mgr->register_creature_script(BOSS_VASHNIK, &VenomousAbyssVashnikAI::Create);
    mgr->register_creature_script(BOSS_VASHNIK_ALT, &VenomousAbyssVashnikAI::Create);
    mgr->register_creature_script(NPC_CLOTTING_VENOM, &VenomousAbyssCrawlerAI::Create);
    mgr->register_creature_script(NPC_SHROUDED_VENOM, &VenomousAbyssCrawlerAI::Create);
    mgr->register_creature_script(NPC_BURNING_VENOM, &VenomousAbyssCrawlerAI::Create);

    // Nek'zali the Soulcoiler
    mgr->register_creature_script(BOSS_NEKZALI, &VenomousAbyssNekzaliAI::Create);
    mgr->register_creature_script(NPC_RESTLESS_AMANI, &VenomousAbyssCrawlerAI::Create);
    mgr->register_creature_script(NPC_LATENT_CULTIST, &VenomousAbyssKitAI::Create);
    mgr->register_creature_script(NPC_ECHO_OF_JAWAE, &VenomousAbyssKitAI::Create);
    mgr->register_creature_script(NPC_ECHO_OF_JAWAE_ALT, &VenomousAbyssKitAI::Create);
    mgr->register_creature_script(NPC_ECHO_OF_JAWAE_ALT2, &VenomousAbyssKitAI::Create);

    // Ula'tek
    mgr->register_creature_script(BOSS_ULATEK, &VenomousAbyssUlatekAI::Create);
    mgr->register_creature_script(BOSS_ULATEK_ALT, &VenomousAbyssUlatekAI::Create);
    mgr->register_creature_script(BOSS_ULATEK_ALT2, &VenomousAbyssUlatekAI::Create);
    mgr->register_creature_script(BOSS_ULATEK_ALT3, &VenomousAbyssUlatekAI::Create);
    mgr->register_creature_script(NPC_GORE_RATTLE, &VenomousAbyssKitAI::Create);
    mgr->register_creature_script(NPC_BLIGHTSCALE_VIPER, &VenomousAbyssKitAI::Create);
    mgr->register_creature_script(NPC_BLIGHTSCALE_RAWLING, &VenomousAbyssKitAI::Create);
    mgr->register_creature_script(NPC_BLIGHTSCALE_SHRIEKER, &VenomousAbyssKitAI::Create);
    mgr->register_creature_script(NPC_DOOMSCALE_WARDEN, &VenomousAbyssKitAI::Create);
    mgr->register_creature_script(NPC_DOOMSCALE_WARDEN_ALT, &VenomousAbyssKitAI::Create);
    mgr->register_creature_script(NPC_RAVENOUS_DOOMSCALE, &VenomousAbyssKitAI::Create);
    mgr->register_creature_script(NPC_BLIGHTSCALE_SPAWN, &VenomousAbyssNestAI::Create);
    mgr->register_creature_script(NPC_BLIGHTSCALE_CLUTCH, &VenomousAbyssNestAI::Create);
    mgr->register_creature_script(NPC_DOOMSCALE_EGG, &VenomousAbyssNestAI::Create);

    // The Lost Explorers
    mgr->register_creature_script(BOSS_MORZAHI, &VenomousAbyssMorzahiAI::Create);
    mgr->register_creature_script(BOSS_FIRST_MATE_NAMA, &VenomousAbyssNamaAI::Create);
    mgr->register_creature_script(BOSS_TRADER_GEBBO, &VenomousAbyssGebboAI::Create);
    mgr->register_creature_script(BOSS_SCROLLSAGE_IKU, &VenomousAbyssIkuAI::Create);
}
