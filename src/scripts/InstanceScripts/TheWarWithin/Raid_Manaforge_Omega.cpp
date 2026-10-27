/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Manaforge_Omega.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <map>
#include <vector>

enum ManaforgeOmegaEvents
{
    // The Soul Hunters
    EVENT_HUNTER_INTERMISSION_START = 1,
    EVENT_HUNTER_INTERMISSION_END,
    EVENT_ADARUS_DEVOURERS_IRE,
    EVENT_ADARUS_CONSUME,
    EVENT_ADARUS_VOIDSTEP,
    EVENT_ADARUS_HUNGERING_SLASH,
    EVENT_ADARUS_COLLAPSING_STAR,
    EVENT_VELARYN_EYE_BEAM,
    EVENT_VELARYN_FELBLADE,
    EVENT_VELARYN_THE_HUNT,
    EVENT_VELARYN_BLADE_DANCE,
    EVENT_VELARYN_FEL_INFERNO,
    EVENT_VELARYN_FEL_RUSH,
    EVENT_ILYSSA_FRACTURE,
    EVENT_ILYSSA_SPIRIT_BOMB,
    EVENT_ILYSSA_SIGIL_OF_CHAINS,
    EVENT_ILYSSA_IMMOLATION_AURA,
    EVENT_ILYSSA_INFERNAL_STRIKE,
    EVENT_ILYSSA_FEL_DEVASTATION,

    // Plexus Sentinel
    EVENT_PLEXUS_ARCANE_LIGHTNING,
    EVENT_PLEXUS_ARCANOCANNON,
    EVENT_PLEXUS_ERADICATING_SALVO,
    EVENT_PLEXUS_MANIFEST_MATRICES,
    EVENT_PLEXUS_PROTOCOL_PURGE,
    EVENT_PLEXUS_PURGING_LIGHTNING,
    EVENT_PLEXUS_ENERGY_CUTTER,
    EVENT_PLEXUS_EXPULSION_ZONE,
    EVENT_PLEXUS_PURGE_END,

    // Soulbinder Naazindhri
    EVENT_NAAZINDHRI_SOUL_CALLING,
    EVENT_NAAZINDHRI_MYSTIC_LASH,
    EVENT_NAAZINDHRI_SOULFIRE_CONVERGENCE,
    EVENT_NAAZINDHRI_SPELLBURN,
    EVENT_NAAZINDHRI_SOULFRAY_ANNIHILATION,
    EVENT_NAAZINDHRI_ARCANE_EXPULSION,

    // Loom'ithar
    EVENT_LOOMITHAR_LAIR_WEAVING,
    EVENT_LOOMITHAR_PIERCING_STRAND,
    EVENT_LOOMITHAR_INFUSION_TETHER,
    EVENT_LOOMITHAR_PRIMAL_SPELLSTORM,
    EVENT_LOOMITHAR_OVERINFUSION_BURST,
    EVENT_LOOMITHAR_WRITHING_WAVE,
    EVENT_LOOMITHAR_ARCANE_OVERFLOW,
    EVENT_LOOMITHAR_ARCANE_OUTRAGE,
    EVENT_LOOMITHAR_UNBOUND_RAGE,

    // Forgeweaver Araz
    EVENT_ARAZ_INVOKE_COLLECTOR,
    EVENT_ARAZ_OVERWHELMING_POWER,
    EVENT_ARAZ_SILENCING_TEMPEST,
    EVENT_ARAZ_ARCANE_OBLITERATION,
    EVENT_ARAZ_ARCANE_EXPULSION,
    EVENT_ARAZ_OPEN_IRIS,
    EVENT_ARAZ_ASTRAL_BURN,
    EVENT_ARAZ_MANA_SPLINTER,
    EVENT_ARAZ_RITUAL_CHECK,
    EVENT_ARAZ_UNSTABLE_SURGE,
    EVENT_ARAZ_CRUSHING_DARKNESS,
    EVENT_ARAZ_VOID_HARVEST,
    EVENT_COLLECTOR_ASTRAL_HARVEST,
    EVENT_COLLECTOR_ARCANE_SIPHON,
    EVENT_COLLECTOR_PHOTON_BLAST,
    EVENT_COLLECTOR_ARCANE_CONVERGENCE,

    // Fractillus
    EVENT_FRACTILLUS_SHOCKWAVE_SLAM,
    EVENT_FRACTILLUS_SLAM_SHOCKWAVE,
    EVENT_FRACTILLUS_CRYSTALLINE_SHOCKWAVE,
    EVENT_FRACTILLUS_REFRACTED_ENTROPY,
    EVENT_FRACTILLUS_SHATTERSHELL,
    EVENT_FRACTILLUS_SHATTERING_BACKHAND,
    EVENT_FRACTILLUS_NEXUS_SHRAPNEL,

    // Nexus-King Salhadaar
    EVENT_SALHADAAR_TYRANNY,
    EVENT_SALHADAAR_DECREE,
    EVENT_SALHADAAR_INVOKE_THE_OATH,
    EVENT_SALHADAAR_SUBJUGATION_RULE,
    EVENT_SALHADAAR_CONQUER,
    EVENT_SALHADAAR_VANQUISH,
    EVENT_SALHADAAR_BANISHMENT,
    EVENT_SALHADAAR_FRACTAL_IMAGES,
    EVENT_SALHADAAR_BEHEAD,
    EVENT_SALHADAAR_BESIEGE,
    EVENT_SALHADAAR_RALLY_CHECK,
    EVENT_SALHADAAR_RALLY_TIMEOUT,
    EVENT_SALHADAAR_NETHERBREAKER,
    EVENT_SALHADAAR_VOIDWING_CONSUMED,
    EVENT_SALHADAAR_TWILIGHT_FALLS,
    EVENT_SALHADAAR_GALACTIC_SMASH,
    EVENT_SALHADAAR_STARKILLER_SWING,

    // Dimensius, the All-Devouring
    EVENT_DIMENSIUS_DEVOUR,
    EVENT_DIMENSIUS_MASSIVE_SMASH,
    EVENT_DIMENSIUS_LIVING_MASS,
    EVENT_DIMENSIUS_DARK_MATTER,
    EVENT_DIMENSIUS_SHATTERED_SPACE,
    EVENT_DIMENSIUS_REVERSE_GRAVITY,
    EVENT_DIMENSIUS_ENDLESS_DARKNESS,
    EVENT_DIMENSIUS_BROKEN_WORLD,
    EVENT_DIMENSIUS_ASTROPHYSICAL_JET,
    EVENT_DIMENSIUS_BLACK_HOLE,
    EVENT_DIMENSIUS_STELLAR_CORE,
    EVENT_DIMENSIUS_HORIZON_END,
    EVENT_DIMENSIUS_GAMMA_BURST,
    EVENT_DIMENSIUS_GRAVITATIONAL_DISTORTION,
    EVENT_DIMENSIUS_EXTINCTION,
    EVENT_DIMENSIUS_LORDS_CHECK,
    EVENT_DIMENSIUS_LORDS_TIMEOUT,
    EVENT_DIMENSIUS_ACCRETION_DISK,
    EVENT_DIMENSIUS_COSMIC_COLLAPSE,
    EVENT_DIMENSIUS_EXTINGUISH_THE_STARS,
    EVENT_DIMENSIUS_DARKENED_SKY,
    EVENT_DIMENSIUS_SINGULARITY_DEVOUR,
    EVENT_DEVOURED_LORD_TOUCH_OF_OBLIVION,
    EVENT_DEVOURED_LORD_SIGNATURE,
    EVENT_DEVOURED_LORD_CONQUERORS_CROSS,

    // slots of the shared add kits
    EVENT_MANAFORGE_ADD_SLOT_1,
    EVENT_MANAFORGE_ADD_SLOT_2,
    EVENT_MANAFORGE_ADD_SLOT_3,
    EVENT_MANAFORGE_ADD_FINALE,
    EVENT_MANAFORGE_ADD_VANISH
};

namespace
{
    float const ManaforgeTwoPi = 6.28318531f;

    uint32_t const SoulHuntersBerserkMs = 540000;
    uint32_t const PlexusBerserkMs = 480000;
    uint32_t const NaazindhriBerserkMs = 540000;
    uint32_t const LoomitharBerserkMs = 480000;
    uint32_t const ArazBerserkMs = 540000;
    uint32_t const FractillusBerserkMs = 480000;
    uint32_t const SalhadaarBerserkMs = 600000;
    uint32_t const DimensiusBerserkMs = 600000;

    // every hunter leads one intermission per rotation
    uint32_t const SoulHuntersRotationMs = 270000;
    uint32_t const SoulHuntersIntermissionMs = 20000;

    // Powered Automaton fills the bar in this time
    uint32_t const PlexusEnergyFillMs = 90000;
    uint32_t const PlexusPurgeMs = 35000;

    int32_t const LoomitharUnboundPct = 50;

    uint8_t const ArazCollectorCount = 4;
    uint8_t const ArazRitualsToBreak = 2;
    float const ArazCollectorRange = 28.0f;
    uint32_t const ArazRitualMs = 45000;

    // nexuses on one prism before the overcharge
    uint8_t const FractillusOverchargeNexuses = 6;

    int32_t const SalhadaarDescentPct = 80;
    int32_t const SalhadaarHungerPct = 50;

    int32_t const DimensiusHorizonPct = 60;
    uint32_t const DimensiusHorizonMs = 40000;
    uint32_t const DimensiusLordsMs = 180000;

    LocationVector ringPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(ManaforgeTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // who an add kit spell is aimed at
    enum class ManaforgeAim : uint8_t
    {
        Victim,
        AnyPlayer,
        NotTank,
        Self,
        Area
    };

    struct ManaforgeAddCast
    {
        uint32_t spellId;
        ManaforgeAim aim;
        uint32_t firstMs;
        uint32_t repeatMs;
    };

    struct ManaforgeAddKit
    {
        bool rooted;
        uint32_t engageSpell;
        ManaforgeAddCast casts[3];
        // area spell after finaleMs, the add leaves afterwards
        uint32_t finaleSpell;
        uint32_t finaleMs;
        char const* finaleText;
    };

    ManaforgeAddCast const NoCast = { 0, ManaforgeAim::Area, 0, 0 };

    ManaforgeAddKit const VolatileManifestationKit =
    {
        false, 0, { NoCast, NoCast, NoCast },
        SPELL_VOLATILE_MANIFESTATION_ENERGY_OVERLOAD, 20000, nullptr
    };

    ManaforgeAddKit const ShadowguardMageKit =
    {
        false, 0,
        {
            { SPELL_SHADOWGUARD_MAGE_VOID_BURST, ManaforgeAim::Victim, 3000, 6000 },
            NoCast, NoCast
        },
        0, 0, nullptr
    };

    ManaforgeAddKit const ShadowguardAssassinKit =
    {
        false, 0,
        {
            { SPELL_SHADOWGUARD_ASSASSIN_VOIDBLADE_AMBUSH, ManaforgeAim::NotTank, 4000, 15000 },
            NoCast, NoCast
        },
        0, 0, nullptr
    };

    ManaforgeAddKit const ShadowguardPhasebladeKit =
    {
        false, 0,
        {
            { SPELL_SHADOWGUARD_PHASEBLADE_PHASE_BLADES, ManaforgeAim::Victim, 5000, 12000 },
            { SPELL_SHADOWGUARD_PHASEBLADE_VOID_RESONANCE, ManaforgeAim::Area, 10000, 20000 },
            NoCast
        },
        0, 0, nullptr
    };

    ManaforgeAddKit const ArcaneEchoKit =
    {
        true, SPELL_ARCANE_ECHO_ECHOING_INVOCATION,
        {
            { SPELL_ARCANE_ECHO_ECHOING_TEMPEST, ManaforgeAim::Area, 6000, 12000 },
            NoCast, NoCast
        },
        0, 0, nullptr
    };

    ManaforgeAddKit const ShieldedAttendantKit =
    {
        false, SPELL_SHIELDED_ATTENDANT_HARDENED_SHELL,
        {
            { SPELL_SHIELDED_ATTENDANT_RAMPING_POWER, ManaforgeAim::Self, 10000, 10000 },
            NoCast, NoCast
        },
        0, 0, nullptr
    };

    ManaforgeAddKit const RoyalVoidwingKit =
    {
        false, 0,
        {
            { SPELL_ROYAL_VOIDWING_DIMENSION_BREATH, ManaforgeAim::Victim, 8000, 20000 },
            { SPELL_ROYAL_VOIDWING_COSMIC_MAW, ManaforgeAim::Victim, 14000, 20000 },
            { SPELL_ROYAL_VOIDWING_BEHEAD, ManaforgeAim::NotTank, 20000, 30000 }
        },
        0, 0, nullptr
    };

    ManaforgeAddKit const ManaforgedTitanKit =
    {
        false, 0,
        {
            { SPELL_MANAFORGED_TITAN_DREAD_MORTAR, ManaforgeAim::AnyPlayer, 5000, 10000 },
            NoCast, NoCast
        },
        SPELL_MANAFORGED_TITAN_SELF_DESTRUCT, 45000, "A Manaforged Titan begins to Self-Destruct!"
    };

    ManaforgeAddKit const NexusPrinceKit =
    {
        false, SPELL_NEXUS_PRINCE_TWILIGHT_BARRIER,
        {
            { SPELL_NEXUS_PRINCE_NETHERBLAST, ManaforgeAim::Victim, 4000, 9000 },
            { SPELL_NEXUS_PRINCE_NEXUS_BEAMS, ManaforgeAim::AnyPlayer, 8000, 18000 },
            NoCast
        },
        0, 0, nullptr
    };

    ManaforgeAddKit const ShadowguardReaperKit =
    {
        false, 0,
        {
            { SPELL_SHADOWGUARD_REAPER_REAP, ManaforgeAim::NotTank, 6000, 14000 },
            { SPELL_SHADOWGUARD_REAPER_TWILIGHT_MASSACRE, ManaforgeAim::Area, 15000, 25000 },
            NoCast
        },
        0, 0, nullptr
    };

    ManaforgeAddKit const LivingMassKit =
    {
        false, 0,
        {
            { SPELL_LIVING_MASS_FISSION, ManaforgeAim::Self, 15000, 20000 },
            NoCast, NoCast
        },
        0, 0, nullptr
    };

    uint32_t const SoulHunterEntries[3] = { BOSS_ADARUS_DUSKBLAZE, BOSS_VELARYN_BLOODWRATH, BOSS_ILYSSA_DARKSORROW };

    // Salhadaar's rallied Shadowguard
    std::vector<uint32_t> const SalhadaarRallyEntries =
    {
        NPC_SALHADAAR_MANAFORGED_TITAN,
        NPC_SALHADAAR_NEXUS_PRINCE_XEVVOS,
        NPC_SALHADAAR_NEXUS_PRINCE_KYVOR,
        NPC_SALHADAAR_SHADOWGUARD_REAPER
    };

    std::vector<uint32_t> const DevouredLordEntries = { NPC_DIMENSIUS_ARTOSHION, NPC_DIMENSIUS_PARGOTH };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class ManaforgeOmegaInstanceScript : public InstanceScript
{
public:
    explicit ManaforgeOmegaInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(ManaforgeOmegaEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ManaforgeOmegaInstanceScript(pMapMgr); }

    // a fallen hunter must not end the hunt while another still stands
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isEncounterDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (uint32_t entry : SoulHunterEntries)
        {
            if (pCreature->getEntry() == entry)
            {
                mHunterGuids[entry] = pCreature->GetNewGUID();
                return;
            }
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (id != DATA_SOUL_HUNTERS || state != EncounterStates::Failed)
            return;

        // a wiped hunt starts again with all three
        for (Creature* pHunter : getMembers(id))
        {
            if (!pHunter->isAlive())
                pHunter->despawn(1000, 5000);
        }
    }

    std::vector<Creature*> getMembers(uint32_t dataId)
    {
        std::vector<Creature*> members;
        if (dataId != DATA_SOUL_HUNTERS)
            return members;

        for (auto const& hunter : mHunterGuids)
        {
            if (Creature* pHunter = getCreatureByGuid(hunter.second))
                members.push_back(pHunter);
        }

        return members;
    }

    // single boss encounters are down with their boss
    bool isEncounterDown(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getMembers(dataId))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
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

    // pulling one hunter pulls the others
    void rallyEncounter(uint32_t dataId, Creature* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getMembers(dataId))
        {
            if (pMember == pPuller || !pMember->isAlive() || pMember->isInCombat() || pMember->GetScript() == nullptr)
                continue;

            pMember->GetScript()->setZoneWideCombat();
        }
        mRallying = false;
    }

    uint32_t countAlive(std::vector<uint32_t> const& entries)
    {
        uint32_t alive = 0;
        for (uint32_t entry : entries)
        {
            for (Creature* pCreature : getCreatureSetForEntry(entry))
            {
                if (pCreature->isAlive())
                    ++alive;
            }
        }

        return alive;
    }

    void sendActionToEntry(uint32_t entry, int32_t action)
    {
        for (Creature* pCreature : getCreatureSetForEntry(entry))
        {
            if (pCreature->isAlive() && pCreature->GetScript() != nullptr)
                pCreature->GetScript()->DoAction(action);
        }
    }

private:
    std::map<uint32_t, WoWGuid> mHunterGuids;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, berserk, stages and target helpers

class ManaforgeOmegaCombatAI : public CreatureAIScript
{
public:
    explicit ManaforgeOmegaCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mStage = 1;
        mBerserkLeftMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mStage = 0;
        mBerserkLeftMs = 0;
        setShielded(false);
        onDisengage();
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

        // due events run until a cast holds the queue
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
    virtual void onDisengage() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    virtual void onBerserk() { castSpellOnSelf(SPELL_MANAFORGE_BERSERK, true); }

    ManaforgeOmegaInstanceScript* getManaforge()
    {
        if (getCreature()->GetMapId() != MAP_MANAFORGE_OMEGA || getInstanceScript() == nullptr)
            return nullptr;

        return static_cast<ManaforgeOmegaInstanceScript*>(getInstanceScript());
    }

    // distinct random players, the tank only when nobody else is left
    std::vector<Unit*> rollPlayers(size_t count, bool skipTank)
    {
        TargetFilter const filter = skipTank ? TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent) : TargetFilter_Player;
        FilterArgs const args(filter);

        std::vector<Unit*> pool;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject != nullptr && isValidUnitTarget(pObject, filter, args))
                pool.push_back(static_cast<Unit*>(pObject));
        }

        if (pool.empty() && skipTank)
            return rollPlayers(count, false);

        std::vector<Unit*> picked;
        while (!pool.empty() && picked.size() < count)
        {
            size_t const slot = Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1));
            picked.push_back(pool[slot]);
            pool[slot] = pool.back();
            pool.pop_back();
        }

        return picked;
    }

    Unit* rollPlayer(bool skipTank)
    {
        std::vector<Unit*> const picked = rollPlayers(1, skipTank);
        return picked.empty() ? nullptr : picked.front();
    }

    Unit* castAtPlayer(uint32_t spellId, bool skipTank, bool triggered = false)
    {
        Unit* pTarget = rollPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    void castAtPlayers(uint32_t spellId, size_t count, bool skipTank)
    {
        for (Unit* pTarget : rollPlayers(count, skipTank))
            castSpell(pTarget, spellId, true);
    }

    void castAimed(uint32_t spellId, ManaforgeAim aim)
    {
        switch (aim)
        {
            case ManaforgeAim::Victim:
                castSpellOnVictim(spellId);
                break;
            case ManaforgeAim::AnyPlayer:
                castAtPlayer(spellId, false);
                break;
            case ManaforgeAim::NotTank:
                castAtPlayer(spellId, true);
                break;
            case ManaforgeAim::Self:
                castSpellOnSelf(spellId);
                break;
            default:
                castSpellAOE(spellId);
                break;
        }
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t durationMs = 0)
    {
        Creature* pAdd = summonCreature(entry, spot, despawnType, durationMs);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return ringPoint(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void armBerserk(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

    // a new stage drops every pending event
    void enterStage(uint32_t stage)
    {
        mStage = stage;
        scriptEvents.resetEvents();
    }

    // out of reach and in place while a ritual or intermission runs
    void setShielded(bool shielded)
    {
        if (mShielded == shielded)
            return;

        mShielded = shielded;
        if (shielded)
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        else
            removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);

        setRooted(shielded);
        _setMeleeDisabled(shielded);
    }

    uint32_t mStage = 0;

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
        onBerserk();
    }

    uint32_t mBerserkLeftMs = 0;
    bool mShielded = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class ManaforgeOmegaBossAI : public ManaforgeOmegaCombatAI
{
public:
    ManaforgeOmegaBossAI(Creature* pCreature, uint32_t dataId) : ManaforgeOmegaCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (ManaforgeOmegaInstanceScript* pManaforge = getManaforge())
        {
            pManaforge->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pManaforge->setBossState(mDataId, EncounterStates::InProgress);
            pManaforge->rallyEncounter(mDataId, getCreature());
        }

        ManaforgeOmegaCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (ManaforgeOmegaInstanceScript* pManaforge = getManaforge())
            {
                pManaforge->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last member has left the fight
                if (!pManaforge->isEncounterFighting(mDataId, getCreature()))
                    pManaforge->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        ManaforgeOmegaCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (ManaforgeOmegaInstanceScript* pManaforge = getManaforge())
        {
            pManaforge->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            if (pManaforge->isEncounterDown(mDataId, getCreature()))
                pManaforge->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Adds built from a kit of up to three timed spells

class ManaforgeOmegaAddAI : public ManaforgeOmegaCombatAI
{
public:
    static CreatureAIScript* CreateVolatileManifestation(Creature* c) { return new ManaforgeOmegaAddAI(c, VolatileManifestationKit); }
    static CreatureAIScript* CreateShadowguardMage(Creature* c) { return new ManaforgeOmegaAddAI(c, ShadowguardMageKit); }
    static CreatureAIScript* CreateShadowguardAssassin(Creature* c) { return new ManaforgeOmegaAddAI(c, ShadowguardAssassinKit); }
    static CreatureAIScript* CreateShadowguardPhaseblade(Creature* c) { return new ManaforgeOmegaAddAI(c, ShadowguardPhasebladeKit); }
    static CreatureAIScript* CreateArcaneEcho(Creature* c) { return new ManaforgeOmegaAddAI(c, ArcaneEchoKit); }
    static CreatureAIScript* CreateShieldedAttendant(Creature* c) { return new ManaforgeOmegaAddAI(c, ShieldedAttendantKit); }
    static CreatureAIScript* CreateRoyalVoidwing(Creature* c) { return new ManaforgeOmegaAddAI(c, RoyalVoidwingKit); }
    static CreatureAIScript* CreateManaforgedTitan(Creature* c) { return new ManaforgeOmegaAddAI(c, ManaforgedTitanKit); }
    static CreatureAIScript* CreateNexusPrince(Creature* c) { return new ManaforgeOmegaAddAI(c, NexusPrinceKit); }
    static CreatureAIScript* CreateShadowguardReaper(Creature* c) { return new ManaforgeOmegaAddAI(c, ShadowguardReaperKit); }
    static CreatureAIScript* CreateLivingMass(Creature* c) { return new ManaforgeOmegaAddAI(c, LivingMassKit); }

    ManaforgeOmegaAddAI(Creature* pCreature, ManaforgeAddKit const& kit) : ManaforgeOmegaCombatAI(pCreature), mKit(kit) {}

protected:
    void onEngage() override
    {
        if (mKit.rooted)
        {
            setRooted(true);
            _setMeleeDisabled(true);
        }

        if (mKit.engageSpell != 0)
            castSpellOnSelf(mKit.engageSpell, true);

        for (uint32_t slot = 0; slot < 3; ++slot)
        {
            if (mKit.casts[slot].spellId != 0)
                scriptEvents.addEvent(EVENT_MANAFORGE_ADD_SLOT_1 + slot, mKit.casts[slot].firstMs);
        }

        if (mKit.finaleSpell != 0)
            scriptEvents.addEvent(EVENT_MANAFORGE_ADD_FINALE, mKit.finaleMs);
    }

    void onDisengage() override
    {
        if (mKit.rooted)
        {
            setRooted(false);
            _setMeleeDisabled(false);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MANAFORGE_ADD_FINALE:
                if (mKit.finaleText != nullptr)
                    warn(mKit.finaleText);
                castSpellAOE(mKit.finaleSpell);
                scriptEvents.addEvent(EVENT_MANAFORGE_ADD_VANISH, 3000);
                break;
            case EVENT_MANAFORGE_ADD_VANISH:
                despawn(500, 0);
                break;
            case EVENT_MANAFORGE_ADD_SLOT_1:
            case EVENT_MANAFORGE_ADD_SLOT_2:
            case EVENT_MANAFORGE_ADD_SLOT_3:
            {
                ManaforgeAddCast const& cast = mKit.casts[eventId - EVENT_MANAFORGE_ADD_SLOT_1];
                castAimed(cast.spellId, cast.aim);
                scriptEvents.addEvent(eventId, cast.repeatMs);
                break;
            }
            default:
                break;
        }
    }

private:
    ManaforgeAddKit const& mKit;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Motes that walk to a goal and burst once they arrive

class ManaforgeOmegaDrifterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* CreateArcaneManifestation(Creature* c) { return new ManaforgeOmegaDrifterAI(c, NPC_ARAZ_ARCANE_COLLECTOR, SPELL_ARCANE_MANIFESTATION_ASTRAL_SURGE); }
    static CreatureAIScript* CreateVoidManifestation(Creature* c) { return new ManaforgeOmegaDrifterAI(c, BOSS_FORGEWEAVER_ARAZ, SPELL_VOID_MANIFESTATION_VOID_SURGE); }

    ManaforgeOmegaDrifterAI(Creature* pCreature, uint32_t goalEntry, uint32_t burstSpellId)
        : CreatureAIScript(pCreature), mGoalEntry(goalEntry), mBurstSpellId(burstSpellId) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mArrived || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mStepMs > diff)
        {
            mStepMs -= diff;
            return;
        }

        mStepMs = 1000;
        Creature* pGoal = findNearestCreature(mGoalEntry, 200.0f);
        if (pGoal == nullptr || !pGoal->isAlive())
            return;

        if (getCreature()->getDistance(pGoal) > 3.0f)
        {
            moveTo(pGoal->GetPositionX(), pGoal->GetPositionY(), pGoal->GetPositionZ(), false);
            return;
        }

        mArrived = true;
        castSpellAOE(mBurstSpellId, true);
        despawn(1000, 0);
    }

private:
    uint32_t mGoalEntry;
    uint32_t mBurstSpellId;
    uint32_t mStepMs = 1000;
    bool mArrived = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Soul Hunters

// each hunter leads its own intermission once per rotation
class ManaforgeOmegaSoulHunterAI : public ManaforgeOmegaBossAI
{
public:
    ManaforgeOmegaSoulHunterAI(Creature* pCreature, uint32_t firstIntermissionMs, char const* intermissionText)
        : ManaforgeOmegaBossAI(pCreature, DATA_SOUL_HUNTERS), mFirstIntermissionMs(firstIntermissionMs), mIntermissionText(intermissionText) {}

protected:
    void onEngage() override
    {
        armBerserk(SoulHuntersBerserkMs);
        resumeHunt(mFirstIntermissionMs);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HUNTER_INTERMISSION_START:
                beginIntermission();
                break;
            case EVENT_HUNTER_INTERMISSION_END:
                resumeHunt(SoulHuntersRotationMs - SoulHuntersIntermissionMs);
                break;
            default:
                onHunterEvent(eventId);
                break;
        }
    }

    virtual void scheduleHunt() = 0;
    virtual void scheduleIntermission() = 0;
    virtual void onHunterEvent(uint32_t eventId) = 0;

private:
    void resumeHunt(uint32_t nextIntermissionMs)
    {
        enterStage(1);
        scheduleHunt();
        scriptEvents.addEvent(EVENT_HUNTER_INTERMISSION_START, nextIntermissionMs);
    }

    void beginIntermission()
    {
        enterStage(2);
        warn(mIntermissionText);
        castSpellOnSelf(SPELL_SOUL_HUNTERS_SOUL_TETHER, true);
        scheduleIntermission();
        scriptEvents.addEvent(EVENT_HUNTER_INTERMISSION_END, SoulHuntersIntermissionMs);
    }

    uint32_t mFirstIntermissionMs;
    char const* mIntermissionText;
};

class ManaforgeOmegaAdarusAI : public ManaforgeOmegaSoulHunterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ManaforgeOmegaAdarusAI(c); }
    explicit ManaforgeOmegaAdarusAI(Creature* pCreature)
        : ManaforgeOmegaSoulHunterAI(pCreature, 90000, "Adarus Duskblaze enshrouds the arena in void!") {}

protected:
    void scheduleHunt() override
    {
        scriptEvents.addEvent(EVENT_ADARUS_DEVOURERS_IRE, 8000);
        scriptEvents.addEvent(EVENT_ADARUS_CONSUME, 14000);
        scriptEvents.addEvent(EVENT_ADARUS_VOIDSTEP, 30000);
    }

    void scheduleIntermission() override
    {
        scriptEvents.addEvent(EVENT_ADARUS_COLLAPSING_STAR, 2000);
    }

    void onHunterEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ADARUS_DEVOURERS_IRE:
                castAtPlayers(SPELL_ADARUS_DEVOURERS_IRE, 2, true);
                scriptEvents.addEvent(EVENT_ADARUS_DEVOURERS_IRE, 40000);
                break;
            case EVENT_ADARUS_CONSUME:
                if (Unit* pPrey = findBranded())
                    castSpell(pPrey, SPELL_ADARUS_CONSUME);
                scriptEvents.addEvent(EVENT_ADARUS_CONSUME, 8000);
                break;
            case EVENT_ADARUS_VOIDSTEP:
                castAtPlayer(SPELL_ADARUS_VOIDSTEP, true);
                scriptEvents.addEvent(EVENT_ADARUS_HUNGERING_SLASH, 2500);
                scriptEvents.addEvent(EVENT_ADARUS_VOIDSTEP, 35000);
                break;
            case EVENT_ADARUS_HUNGERING_SLASH:
                castSpellOnVictim(SPELL_ADARUS_HUNGERING_SLASH);
                break;
            case EVENT_ADARUS_COLLAPSING_STAR:
                castSpellAOE(SPELL_ADARUS_COLLAPSING_STAR);
                scriptEvents.addEvent(EVENT_ADARUS_COLLAPSING_STAR, 8000);
                break;
            default:
                break;
        }
    }

private:
    // a player carrying Devourer's Ire, anyone else when nobody is branded
    Unit* findBranded()
    {
        std::vector<Unit*> branded;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr || !pObject->isPlayer())
                continue;

            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer->isAlive() && pPlayer->hasAurasWithId(SPELL_ADARUS_DEVOURERS_IRE))
                branded.push_back(pPlayer);
        }

        if (branded.empty())
            return rollPlayer(true);

        return branded[Util::getRandomUInt(static_cast<uint32_t>(branded.size() - 1))];
    }
};

class ManaforgeOmegaVelarynAI : public ManaforgeOmegaSoulHunterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ManaforgeOmegaVelarynAI(c); }
    explicit ManaforgeOmegaVelarynAI(Creature* pCreature)
        : ManaforgeOmegaSoulHunterAI(pCreature, 180000, "Velaryn Bloodwrath scours the arena with Fel Rush!") {}

protected:
    void scheduleHunt() override
    {
        scriptEvents.addEvent(EVENT_VELARYN_EYE_BEAM, 10000);
        scriptEvents.addEvent(EVENT_VELARYN_BLADE_DANCE, 18000);
        scriptEvents.addEvent(EVENT_VELARYN_THE_HUNT, 25000);
        scriptEvents.addEvent(EVENT_VELARYN_FEL_INFERNO, 35000);
    }

    void scheduleIntermission() override
    {
        scriptEvents.addEvent(EVENT_VELARYN_FEL_RUSH, 2000);
    }

    void onHunterEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VELARYN_EYE_BEAM:
                castSpellOnVictim(SPELL_VELARYN_EYE_BEAM);
                scriptEvents.addEvent(EVENT_VELARYN_FELBLADE, 4000);
                scriptEvents.addEvent(EVENT_VELARYN_EYE_BEAM, 30000);
                break;
            case EVENT_VELARYN_FELBLADE:
                castSpellOnVictim(SPELL_VELARYN_FELBLADE);
                break;
            case EVENT_VELARYN_THE_HUNT:
                if (castAtPlayer(SPELL_VELARYN_THE_HUNT, true) != nullptr)
                    warn("Velaryn Bloodwrath begins The Hunt!");
                scriptEvents.addEvent(EVENT_VELARYN_THE_HUNT, 45000);
                break;
            case EVENT_VELARYN_BLADE_DANCE:
                castSpellAOE(SPELL_VELARYN_BLADE_DANCE);
                scriptEvents.addEvent(EVENT_VELARYN_BLADE_DANCE, 22000);
                break;
            case EVENT_VELARYN_FEL_INFERNO:
                castSpellAOE(SPELL_VELARYN_FEL_INFERNO);
                scriptEvents.addEvent(EVENT_VELARYN_FEL_INFERNO, 40000);
                break;
            case EVENT_VELARYN_FEL_RUSH:
                castAtPlayer(SPELL_VELARYN_FEL_RUSH, false);
                scriptEvents.addEvent(EVENT_VELARYN_FEL_RUSH, 5000);
                break;
            default:
                break;
        }
    }
};

class ManaforgeOmegaIlyssaAI : public ManaforgeOmegaSoulHunterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ManaforgeOmegaIlyssaAI(c); }
    explicit ManaforgeOmegaIlyssaAI(Creature* pCreature)
        : ManaforgeOmegaSoulHunterAI(pCreature, 270000, "Ilyssa Darksorrow rampages about the arena!") {}

protected:
    void scheduleHunt() override
    {
        scriptEvents.addEvent(EVENT_ILYSSA_FRACTURE, 8000);
        scriptEvents.addEvent(EVENT_ILYSSA_IMMOLATION_AURA, 15000);
        scriptEvents.addEvent(EVENT_ILYSSA_SIGIL_OF_CHAINS, 20000);
    }

    void scheduleIntermission() override
    {
        scriptEvents.addEvent(EVENT_ILYSSA_INFERNAL_STRIKE, 1000);
        scriptEvents.addEvent(EVENT_ILYSSA_FEL_DEVASTATION, 12000);
    }

    void onHunterEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ILYSSA_FRACTURE:
                castSpellOnVictim(SPELL_ILYSSA_FRACTURE);
                scriptEvents.addEvent(EVENT_ILYSSA_SPIRIT_BOMB, 6000);
                scriptEvents.addEvent(EVENT_ILYSSA_FRACTURE, 25000);
                break;
            case EVENT_ILYSSA_SPIRIT_BOMB:
                castSpellAOE(SPELL_ILYSSA_SPIRIT_BOMB);
                break;
            case EVENT_ILYSSA_SIGIL_OF_CHAINS:
                castAtPlayer(SPELL_ILYSSA_SIGIL_OF_CHAINS, true);
                scriptEvents.addEvent(EVENT_ILYSSA_SIGIL_OF_CHAINS, 35000);
                break;
            case EVENT_ILYSSA_IMMOLATION_AURA:
                castSpellOnSelf(SPELL_ILYSSA_IMMOLATION_AURA);
                scriptEvents.addEvent(EVENT_ILYSSA_IMMOLATION_AURA, 45000);
                break;
            case EVENT_ILYSSA_INFERNAL_STRIKE:
                castAtPlayer(SPELL_ILYSSA_INFERNAL_STRIKE, false);
                scriptEvents.addEvent(EVENT_ILYSSA_INFERNAL_STRIKE, 4000);
                break;
            case EVENT_ILYSSA_FEL_DEVASTATION:
                warn("Ilyssa Darksorrow channels Fel Devastation!");
                castSpellOnVictim(SPELL_ILYSSA_FEL_DEVASTATION);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Plexus Sentinel

class ManaforgeOmegaPlexusSentinelAI : public ManaforgeOmegaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ManaforgeOmegaPlexusSentinelAI(c); }
    explicit ManaforgeOmegaPlexusSentinelAI(Creature* pCreature) : ManaforgeOmegaBossAI(pCreature, DATA_PLEXUS_SENTINEL) {}

protected:
    void onEngage() override
    {
        armBerserk(PlexusBerserkMs);
        castSpellOnSelf(SPELL_PLEXUS_POWERED_AUTOMATON, true);
        beginSweep();
    }

    void onBerserk() override
    {
        warn("The Plexus Sentinel cleanses the chamber!");
        castSpellAOE(SPELL_PLEXUS_CLEANSE_THE_CHAMBER, true);
        ManaforgeOmegaBossAI::onBerserk();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_PLEXUS_ARCANE_LIGHTNING:
                castAtPlayer(SPELL_PLEXUS_ARCANE_LIGHTNING, false);
                scriptEvents.addEvent(EVENT_PLEXUS_ARCANE_LIGHTNING, 12000);
                break;
            case EVENT_PLEXUS_ARCANOCANNON:
                warn("The Plexus Sentinel aims its Obliteration Arcanocannon!");
                castSpellOnVictim(SPELL_PLEXUS_OBLITERATION_ARCANOCANNON);
                scriptEvents.addEvent(EVENT_PLEXUS_ARCANOCANNON, 35000);
                break;
            case EVENT_PLEXUS_ERADICATING_SALVO:
                castAtPlayer(SPELL_PLEXUS_ERADICATING_SALVO, true);
                scriptEvents.addEvent(EVENT_PLEXUS_ERADICATING_SALVO, 30000);
                break;
            case EVENT_PLEXUS_MANIFEST_MATRICES:
                castSpellAOE(SPELL_PLEXUS_MANIFEST_MATRICES);
                scriptEvents.addEvent(EVENT_PLEXUS_MANIFEST_MATRICES, 40000);
                break;
            case EVENT_PLEXUS_PROTOCOL_PURGE:
                beginPurge();
                break;
            case EVENT_PLEXUS_PURGING_LIGHTNING:
                castSpellAOE(SPELL_PLEXUS_PURGING_LIGHTNING, true);
                scriptEvents.addEvent(EVENT_PLEXUS_PURGING_LIGHTNING, 4000);
                break;
            case EVENT_PLEXUS_ENERGY_CUTTER:
                castAtPlayer(SPELL_PLEXUS_ENERGY_CUTTER, false);
                scriptEvents.addEvent(EVENT_PLEXUS_ENERGY_CUTTER, 12000);
                break;
            case EVENT_PLEXUS_EXPULSION_ZONE:
                castAtPlayer(SPELL_PLEXUS_EXPULSION_ZONE, true);
                scriptEvents.addEvent(EVENT_PLEXUS_EXPULSION_ZONE, 15000);
                break;
            case EVENT_PLEXUS_PURGE_END:
                beginSweep();
                break;
            default:
                break;
        }
    }

private:
    // stage one until the energy bar is full again
    void beginSweep()
    {
        enterStage(1);
        scriptEvents.addEvent(EVENT_PLEXUS_ARCANE_LIGHTNING, 6000);
        scriptEvents.addEvent(EVENT_PLEXUS_ARCANOCANNON, 12000);
        scriptEvents.addEvent(EVENT_PLEXUS_ERADICATING_SALVO, 20000);
        scriptEvents.addEvent(EVENT_PLEXUS_MANIFEST_MATRICES, 28000);
        scriptEvents.addEvent(EVENT_PLEXUS_PROTOCOL_PURGE, PlexusEnergyFillMs);
    }

    void beginPurge()
    {
        enterStage(2);
        warn("The Plexus Sentinel initiates Protocol: Purge!");
        castSpellOnSelf(SPELL_PLEXUS_PROTOCOL_PURGE);

        for (uint8_t i = 0; i < 3; ++i)
            callAdd(NPC_PLEXUS_VOLATILE_MANIFESTATION, aroundSelf(10.0f, 20.0f));

        scriptEvents.addEvent(EVENT_PLEXUS_PURGING_LIGHTNING, 3000);
        scriptEvents.addEvent(EVENT_PLEXUS_ENERGY_CUTTER, 6000);
        scriptEvents.addEvent(EVENT_PLEXUS_EXPULSION_ZONE, 10000);
        scriptEvents.addEvent(EVENT_PLEXUS_PURGE_END, PlexusPurgeMs);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Soulbinder Naazindhri

class ManaforgeOmegaNaazindhriAI : public ManaforgeOmegaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ManaforgeOmegaNaazindhriAI(c); }
    explicit ManaforgeOmegaNaazindhriAI(Creature* pCreature) : ManaforgeOmegaBossAI(pCreature, DATA_SOULBINDER_NAAZINDHRI) {}

protected:
    void onEngage() override
    {
        armBerserk(NaazindhriBerserkMs);
        scriptEvents.addEvent(EVENT_NAAZINDHRI_SOUL_CALLING, 5000);
        scriptEvents.addEvent(EVENT_NAAZINDHRI_MYSTIC_LASH, 8000);
        scriptEvents.addEvent(EVENT_NAAZINDHRI_SOULFIRE_CONVERGENCE, 15000);
        scriptEvents.addEvent(EVENT_NAAZINDHRI_SPELLBURN, 22000);
        scriptEvents.addEvent(EVENT_NAAZINDHRI_SOULFRAY_ANNIHILATION, 40000);
        scriptEvents.addEvent(EVENT_NAAZINDHRI_ARCANE_EXPULSION, 50000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NAAZINDHRI_SOUL_CALLING:
                castSpellAOE(SPELL_NAAZINDHRI_SOUL_CALLING);
                callShadowguard();
                scriptEvents.addEvent(EVENT_NAAZINDHRI_SOUL_CALLING, 60000);
                break;
            case EVENT_NAAZINDHRI_MYSTIC_LASH:
                castSpellOnVictim(SPELL_NAAZINDHRI_MYSTIC_LASH);
                scriptEvents.addEvent(EVENT_NAAZINDHRI_MYSTIC_LASH, 18000);
                break;
            case EVENT_NAAZINDHRI_SOULFIRE_CONVERGENCE:
                castAtPlayers(SPELL_NAAZINDHRI_SOULFIRE_CONVERGENCE, 3, true);
                scriptEvents.addEvent(EVENT_NAAZINDHRI_SOULFIRE_CONVERGENCE, 35000);
                break;
            case EVENT_NAAZINDHRI_SPELLBURN:
                castSpellAOE(SPELL_NAAZINDHRI_SPELLBURN);
                scriptEvents.addEvent(EVENT_NAAZINDHRI_SPELLBURN, 30000);
                break;
            case EVENT_NAAZINDHRI_SOULFRAY_ANNIHILATION:
                if (castAtPlayer(SPELL_NAAZINDHRI_SOULFRAY_ANNIHILATION, false) != nullptr)
                    warn("Soulbinder Naazindhri unleashes Soulfray Annihilation!");
                scriptEvents.addEvent(EVENT_NAAZINDHRI_SOULFRAY_ANNIHILATION, 45000);
                break;
            case EVENT_NAAZINDHRI_ARCANE_EXPULSION:
                castSpellAOE(SPELL_NAAZINDHRI_ARCANE_EXPULSION);
                scriptEvents.addEvent(EVENT_NAAZINDHRI_ARCANE_EXPULSION, 55000);
                break;
            default:
                break;
        }
    }

private:
    // the binding chambers release a wave of Shadowguard
    void callShadowguard()
    {
        callAdd(NPC_NAAZINDHRI_SHADOWGUARD_MAGE, aroundSelf(15.0f, 25.0f));
        callAdd(NPC_NAAZINDHRI_SHADOWGUARD_ASSASSIN, aroundSelf(15.0f, 25.0f));
        callAdd(NPC_NAAZINDHRI_SHADOWGUARD_PHASEBLADE, aroundSelf(15.0f, 25.0f));

        if (isHeroic())
            callAdd(NPC_NAAZINDHRI_SHADOWGUARD_ASSASSIN, aroundSelf(15.0f, 25.0f));
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Loom'ithar

class ManaforgeOmegaLoomitharAI : public ManaforgeOmegaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ManaforgeOmegaLoomitharAI(c); }
    explicit ManaforgeOmegaLoomitharAI(Creature* pCreature) : ManaforgeOmegaBossAI(pCreature, DATA_LOOMITHAR) {}

protected:
    void onEngage() override
    {
        armBerserk(LoomitharBerserkMs);
        scriptEvents.addEvent(EVENT_LOOMITHAR_LAIR_WEAVING, 1500);
        scriptEvents.addEvent(EVENT_LOOMITHAR_PIERCING_STRAND, 10000);
        scriptEvents.addEvent(EVENT_LOOMITHAR_INFUSION_TETHER, 16000);
        scriptEvents.addEvent(EVENT_LOOMITHAR_PRIMAL_SPELLSTORM, 25000);
        scriptEvents.addEvent(EVENT_LOOMITHAR_OVERINFUSION_BURST, 45000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == 1 && _getHealthPercent() <= LoomitharUnboundPct)
            beginUnbound();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LOOMITHAR_LAIR_WEAVING:
                castSpellAOE(SPELL_LOOMITHAR_LAIR_WEAVING);
                break;
            case EVENT_LOOMITHAR_PIERCING_STRAND:
                castSpellOnVictim(SPELL_LOOMITHAR_PIERCING_STRAND);
                scriptEvents.addEvent(EVENT_LOOMITHAR_PIERCING_STRAND, 20000);
                break;
            case EVENT_LOOMITHAR_INFUSION_TETHER:
                castAtPlayers(SPELL_LOOMITHAR_INFUSION_TETHER, 3, true);
                scriptEvents.addEvent(EVENT_LOOMITHAR_INFUSION_TETHER, 35000);
                break;
            case EVENT_LOOMITHAR_PRIMAL_SPELLSTORM:
                castSpellAOE(SPELL_LOOMITHAR_PRIMAL_SPELLSTORM);
                scriptEvents.addEvent(EVENT_LOOMITHAR_PRIMAL_SPELLSTORM, 30000);
                break;
            case EVENT_LOOMITHAR_OVERINFUSION_BURST:
                warn("Loom'ithar charges an Overinfusion Burst!");
                castSpellAOE(SPELL_LOOMITHAR_OVERINFUSION_BURST);
                scriptEvents.addEvent(EVENT_LOOMITHAR_OVERINFUSION_BURST, 60000);
                break;
            case EVENT_LOOMITHAR_WRITHING_WAVE:
                castSpellOnVictim(SPELL_LOOMITHAR_WRITHING_WAVE);
                scriptEvents.addEvent(EVENT_LOOMITHAR_WRITHING_WAVE, 20000);
                break;
            case EVENT_LOOMITHAR_ARCANE_OVERFLOW:
                castSpellAOE(SPELL_LOOMITHAR_ARCANE_OVERFLOW);
                scriptEvents.addEvent(EVENT_LOOMITHAR_ARCANE_OVERFLOW, 25000);
                break;
            case EVENT_LOOMITHAR_ARCANE_OUTRAGE:
                castSpellAOE(SPELL_LOOMITHAR_ARCANE_OUTRAGE);
                scriptEvents.addEvent(EVENT_LOOMITHAR_ARCANE_OUTRAGE, 35000);
                break;
            case EVENT_LOOMITHAR_UNBOUND_RAGE:
                warn("Loom'ithar is consumed by Unbound Rage!");
                castSpellOnSelf(SPELL_LOOMITHAR_UNBOUND_RAGE);
                scriptEvents.addEvent(EVENT_LOOMITHAR_UNBOUND_RAGE, 50000);
                break;
            default:
                break;
        }
    }

private:
    // torn in two at half health, it fights in the open
    void beginUnbound()
    {
        enterStage(2);
        warn("Loom'ithar tears itself free of the loom!");
        castSpellOnSelf(SPELL_LOOMITHAR_UNBOUND_RAGE, true);

        scriptEvents.addEvent(EVENT_LOOMITHAR_WRITHING_WAVE, 8000);
        scriptEvents.addEvent(EVENT_LOOMITHAR_ARCANE_OVERFLOW, 12000);
        scriptEvents.addEvent(EVENT_LOOMITHAR_ARCANE_OUTRAGE, 15000);
        scriptEvents.addEvent(EVENT_LOOMITHAR_PRIMAL_SPELLSTORM, 20000);
        scriptEvents.addEvent(EVENT_LOOMITHAR_UNBOUND_RAGE, 45000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Forgeweaver Araz

class ManaforgeOmegaArazAI : public ManaforgeOmegaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ManaforgeOmegaArazAI(c); }
    explicit ManaforgeOmegaArazAI(Creature* pCreature) : ManaforgeOmegaBossAI(pCreature, DATA_FORGEWEAVER_ARAZ) {}

protected:
    void onEngage() override
    {
        armBerserk(ArazBerserkMs);
        mForgeCenter = getCreature()->GetPosition();
        mRitualsBroken = 0;
        beginPriming();
    }

    void onDisengage() override { _removeAura(SPELL_ARAZ_FOCUSING_IRIS); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ARAZ_INVOKE_COLLECTOR:
                invokeCollector();
                if (mCollectorsCalled < ArazCollectorCount)
                    scriptEvents.addEvent(EVENT_ARAZ_INVOKE_COLLECTOR, 25000);
                else
                    scriptEvents.addEvent(EVENT_ARAZ_OPEN_IRIS, 10000);
                break;
            case EVENT_ARAZ_OVERWHELMING_POWER:
                castSpellOnVictim(SPELL_ARAZ_OVERWHELMING_POWER);
                scriptEvents.addEvent(EVENT_ARAZ_OVERWHELMING_POWER, 12000);
                break;
            case EVENT_ARAZ_SILENCING_TEMPEST:
                castSpellAOE(SPELL_ARAZ_SILENCING_TEMPEST);
                scriptEvents.addEvent(EVENT_ARAZ_SILENCING_TEMPEST, 25000);
                break;
            case EVENT_ARAZ_ARCANE_OBLITERATION:
                if (castAtPlayer(SPELL_ARAZ_ARCANE_OBLITERATION, true) != nullptr)
                    warn("Forgeweaver Araz calls down Arcane Obliteration!");
                // every second obliteration leaves an echo behind
                if (++mObliterations % 2 == 0)
                    callAdd(NPC_ARAZ_ARCANE_ECHO, aroundSelf(6.0f, 10.0f), TIMED_DESPAWN, 40000);
                scriptEvents.addEvent(EVENT_ARAZ_ARCANE_OBLITERATION, 40000);
                break;
            case EVENT_ARAZ_ARCANE_EXPULSION:
                castSpellAOE(SPELL_ARAZ_ARCANE_EXPULSION);
                scriptEvents.addEvent(EVENT_ARAZ_ARCANE_EXPULSION, 45000);
                break;
            case EVENT_ARAZ_OPEN_IRIS:
                openIris();
                break;
            case EVENT_ARAZ_ASTRAL_BURN:
                castSpellAOE(SPELL_ARAZ_ASTRAL_BURN, true);
                scriptEvents.addEvent(EVENT_ARAZ_ASTRAL_BURN, 5000);
                break;
            case EVENT_ARAZ_MANA_SPLINTER:
                castAtPlayer(SPELL_ARAZ_MANA_SPLINTER, false, true);
                scriptEvents.addEvent(EVENT_ARAZ_MANA_SPLINTER, 8000);
                break;
            case EVENT_ARAZ_RITUAL_CHECK:
                if (collectorsLeft() == 0)
                    closeIris(true);
                else
                    scriptEvents.addEvent(EVENT_ARAZ_RITUAL_CHECK, 1000);
                break;
            case EVENT_ARAZ_UNSTABLE_SURGE:
                warn("The Manaforge erupts in an Unstable Surge!");
                castSpellAOE(SPELL_ARAZ_UNSTABLE_SURGE, true);
                closeIris(false);
                break;
            case EVENT_ARAZ_CRUSHING_DARKNESS:
                castSpellAOE(SPELL_ARAZ_CRUSHING_DARKNESS, true);
                scriptEvents.addEvent(EVENT_ARAZ_CRUSHING_DARKNESS, 10000);
                break;
            case EVENT_ARAZ_VOID_HARVEST:
                castSpellAOE(SPELL_ARAZ_VOID_HARVEST);
                for (uint8_t i = 0; i < 3; ++i)
                    summonCreature(NPC_ARAZ_VOID_MANIFESTATION, ringPoint(mForgeCenter, 25.0f, 35.0f), TIMED_DESPAWN, 40000);
                scriptEvents.addEvent(EVENT_ARAZ_VOID_HARVEST, 25000);
                break;
            default:
                break;
        }
    }

private:
    // stage one: the collectors are primed one after another
    void beginPriming()
    {
        enterStage(1);
        mCollectorsCalled = 0;
        mObliterations = 0;
        scriptEvents.addEvent(EVENT_ARAZ_INVOKE_COLLECTOR, 5000);
        scriptEvents.addEvent(EVENT_ARAZ_OVERWHELMING_POWER, 6000);
        scriptEvents.addEvent(EVENT_ARAZ_SILENCING_TEMPEST, 18000);
        scriptEvents.addEvent(EVENT_ARAZ_ARCANE_OBLITERATION, 30000);
        scriptEvents.addEvent(EVENT_ARAZ_ARCANE_EXPULSION, 40000);
    }

    void invokeCollector()
    {
        float const angle = ManaforgeTwoPi * static_cast<float>(mCollectorsCalled) / static_cast<float>(ArazCollectorCount);
        LocationVector const spot(mForgeCenter.x + ArazCollectorRange * std::cos(angle), mForgeCenter.y + ArazCollectorRange * std::sin(angle), mForgeCenter.z, angle);

        ++mCollectorsCalled;
        castSpellOnSelf(SPELL_ARAZ_INVOKE_COLLECTOR, true);
        callAdd(NPC_ARAZ_ARCANE_COLLECTOR, spot);
    }

    // intermission: the ritual runs until every collector is broken
    void openIris()
    {
        enterStage(2);
        warn("Forgeweaver Araz pushes the Manaforge beyond its limits!");
        setShielded(true);
        castSpellOnSelf(SPELL_ARAZ_FOCUSING_IRIS, true);

        if (ManaforgeOmegaInstanceScript* pManaforge = getManaforge())
            pManaforge->sendActionToEntry(NPC_ARAZ_ARCANE_COLLECTOR, ACTION_ARAZ_EXPOSE_COLLECTOR);

        for (uint8_t i = 0; i < 2; ++i)
            callAdd(NPC_ARAZ_SHIELDED_ATTENDANT, ringPoint(mForgeCenter, 8.0f, 12.0f));

        scriptEvents.addEvent(EVENT_ARAZ_ASTRAL_BURN, 3000);
        scriptEvents.addEvent(EVENT_ARAZ_MANA_SPLINTER, 4000);
        scriptEvents.addEvent(EVENT_ARAZ_RITUAL_CHECK, 5000);
        scriptEvents.addEvent(EVENT_ARAZ_UNSTABLE_SURGE, ArazRitualMs);
    }

    void closeIris(bool disrupted)
    {
        _removeAura(SPELL_ARAZ_FOCUSING_IRIS);
        setShielded(false);
        summons.despawnEntry(NPC_ARAZ_ARCANE_COLLECTOR);

        if (disrupted)
        {
            ++mRitualsBroken;
            warn("The ritual of Forgeweaver Araz is disrupted!");
        }

        if (mRitualsBroken >= ArazRitualsToBreak)
            beginDarkness();
        else
            beginPriming();
    }

    // stage two: Araz feeds himself to the void
    void beginDarkness()
    {
        enterStage(3);
        warn("Forgeweaver Araz opens a door to the void itself!");
        castSpellOnSelf(SPELL_ARAZ_DARK_SINGULARITY, true);

        scriptEvents.addEvent(EVENT_ARAZ_CRUSHING_DARKNESS, 4000);
        scriptEvents.addEvent(EVENT_ARAZ_OVERWHELMING_POWER, 6000);
        scriptEvents.addEvent(EVENT_ARAZ_VOID_HARVEST, 10000);
        scriptEvents.addEvent(EVENT_ARAZ_SILENCING_TEMPEST, 15000);
    }

    uint32_t collectorsLeft()
    {
        ManaforgeOmegaInstanceScript* pManaforge = getManaforge();
        return pManaforge != nullptr ? pManaforge->countAlive({ NPC_ARAZ_ARCANE_COLLECTOR }) : 0;
    }

    LocationVector mForgeCenter;
    uint8_t mCollectorsCalled = 0;
    uint8_t mRitualsBroken = 0;
    uint32_t mObliterations = 0;
};

// shielded by its barrier until Araz opens the iris
class ManaforgeOmegaArcaneCollectorAI : public ManaforgeOmegaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ManaforgeOmegaArcaneCollectorAI(c); }
    explicit ManaforgeOmegaArcaneCollectorAI(Creature* pCreature) : ManaforgeOmegaCombatAI(pCreature) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_ARAZ_EXPOSE_COLLECTOR || mExposed || !_isInCombat())
            return;

        mExposed = true;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        _removeAura(SPELL_ARCANE_COLLECTOR_ARCANE_BARRIER);

        scriptEvents.resetEvents();
        scriptEvents.addEvent(EVENT_COLLECTOR_PHOTON_BLAST, 3000);
        scriptEvents.addEvent(EVENT_COLLECTOR_ARCANE_CONVERGENCE, 8000);
    }

protected:
    void onEngage() override
    {
        mExposed = false;
        setRooted(true);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        castSpellOnSelf(SPELL_ARCANE_COLLECTOR_ARCANE_BARRIER, true);
        castSpellOnSelf(SPELL_ARCANE_COLLECTOR_PRIME_SEQUENCE, true);

        scriptEvents.addEvent(EVENT_COLLECTOR_ASTRAL_HARVEST, 6000);
        scriptEvents.addEvent(EVENT_COLLECTOR_ARCANE_SIPHON, 12000);
    }

    void onDisengage() override
    {
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_COLLECTOR_ASTRAL_HARVEST:
                if (Unit* pTarget = rollPlayer(false))
                {
                    castSpell(pTarget, SPELL_ARCANE_COLLECTOR_ASTRAL_HARVEST, true);
                    summonCreature(NPC_ARAZ_ARCANE_MANIFESTATION, ringPoint(pTarget->GetPosition(), 2.0f, 5.0f), TIMED_DESPAWN, 30000);
                }
                scriptEvents.addEvent(EVENT_COLLECTOR_ASTRAL_HARVEST, 20000);
                break;
            case EVENT_COLLECTOR_ARCANE_SIPHON:
                castSpellOnSelf(SPELL_ARCANE_COLLECTOR_ARCANE_SIPHON);
                scriptEvents.addEvent(EVENT_COLLECTOR_ARCANE_SIPHON, 15000);
                break;
            case EVENT_COLLECTOR_PHOTON_BLAST:
                castAtPlayer(SPELL_ARCANE_COLLECTOR_PHOTON_BLAST, false);
                scriptEvents.addEvent(EVENT_COLLECTOR_PHOTON_BLAST, 6000);
                break;
            case EVENT_COLLECTOR_ARCANE_CONVERGENCE:
                castSpellAOE(SPELL_ARCANE_COLLECTOR_ARCANE_CONVERGENCE);
                scriptEvents.addEvent(EVENT_COLLECTOR_ARCANE_CONVERGENCE, 15000);
                break;
            default:
                break;
        }
    }

private:
    bool mExposed = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Fractillus

class ManaforgeOmegaFractillusAI : public ManaforgeOmegaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ManaforgeOmegaFractillusAI(c); }
    explicit ManaforgeOmegaFractillusAI(Creature* pCreature) : ManaforgeOmegaBossAI(pCreature, DATA_FRACTILLUS) {}

protected:
    void onEngage() override
    {
        armBerserk(FractillusBerserkMs);
        mNexuses = 0;
        castSpellOnSelf(SPELL_FRACTILLUS_NETHER_PRISM, true);

        scriptEvents.addEvent(EVENT_FRACTILLUS_SHOCKWAVE_SLAM, 12000);
        scriptEvents.addEvent(EVENT_FRACTILLUS_CRYSTALLINE_SHOCKWAVE, 20000);
        scriptEvents.addEvent(EVENT_FRACTILLUS_REFRACTED_ENTROPY, 28000);
        scriptEvents.addEvent(EVENT_FRACTILLUS_SHATTERSHELL, 35000);
        scriptEvents.addEvent(EVENT_FRACTILLUS_SHATTERING_BACKHAND, 75000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_FRACTILLUS_SHOCKWAVE_SLAM:
                castSpellOnVictim(SPELL_FRACTILLUS_SHOCKWAVE_SLAM);
                scriptEvents.addEvent(EVENT_FRACTILLUS_SLAM_SHOCKWAVE, 2000);
                scriptEvents.addEvent(EVENT_FRACTILLUS_SHOCKWAVE_SLAM, 25000);
                break;
            case EVENT_FRACTILLUS_SLAM_SHOCKWAVE:
                castSpellOnVictim(SPELL_FRACTILLUS_CRYSTALLINE_SHOCKWAVE, true);
                growNexuses(1);
                break;
            case EVENT_FRACTILLUS_CRYSTALLINE_SHOCKWAVE:
                castAtPlayers(SPELL_FRACTILLUS_CRYSTALLINE_SHOCKWAVE, 2, true);
                growNexuses(2);
                scriptEvents.addEvent(EVENT_FRACTILLUS_CRYSTALLINE_SHOCKWAVE, 30000);
                break;
            case EVENT_FRACTILLUS_REFRACTED_ENTROPY:
                castSpellAOE(SPELL_FRACTILLUS_REFRACTED_ENTROPY);
                scriptEvents.addEvent(EVENT_FRACTILLUS_REFRACTED_ENTROPY, 40000);
                break;
            case EVENT_FRACTILLUS_SHATTERSHELL:
                castAtPlayers(SPELL_FRACTILLUS_SHATTERSHELL, 2, true);
                scriptEvents.addEvent(EVENT_FRACTILLUS_SHATTERSHELL, 45000);
                break;
            case EVENT_FRACTILLUS_SHATTERING_BACKHAND:
                warn("Fractillus winds up a Shattering Backhand!");
                castSpellAOE(SPELL_FRACTILLUS_SHATTERING_BACKHAND);
                scriptEvents.addEvent(EVENT_FRACTILLUS_NEXUS_SHRAPNEL, 2500);
                scriptEvents.addEvent(EVENT_FRACTILLUS_SHATTERING_BACKHAND, 75000);
                break;
            case EVENT_FRACTILLUS_NEXUS_SHRAPNEL:
                // the backhand shatters every standing nexus
                if (mNexuses > 0)
                    castSpellAOE(SPELL_FRACTILLUS_NEXUS_SHRAPNEL, true);
                mNexuses = 0;
                break;
            default:
                break;
        }
    }

private:
    void growNexuses(uint8_t count)
    {
        mNexuses += count;
        if (mNexuses < FractillusOverchargeNexuses)
            return;

        mNexuses = 0;
        warn("Fractillus is empowered by Crystaline Overcharge!");
        castSpellOnSelf(SPELL_FRACTILLUS_CRYSTALINE_OVERCHARGE, true);
    }

    uint8_t mNexuses = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nexus-King Salhadaar

class ManaforgeOmegaSalhadaarAI : public ManaforgeOmegaBossAI
{
    enum SalhadaarStage : uint32_t
    {
        StageOathBreakers = 1,
        StageNexusDescent,
        StageRiderOfTheDark,
        StageKingsHunger,
        StageWorldInTwilight
    };

public:
    static CreatureAIScript* Create(Creature* c) { return new ManaforgeOmegaSalhadaarAI(c); }
    explicit ManaforgeOmegaSalhadaarAI(Creature* pCreature) : ManaforgeOmegaBossAI(pCreature, DATA_NEXUS_KING_SALHADAAR) {}

protected:
    void onEngage() override
    {
        armBerserk(SalhadaarBerserkMs);
        beginOathBreakers();
    }

    void onDisengage() override { _removeAura(SPELL_SALHADAAR_ROYAL_WARD); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        int32_t const health = _getHealthPercent();
        if (mStage == StageOathBreakers && health <= SalhadaarDescentPct)
            beginNexusDescent();
        else if (mStage == StageRiderOfTheDark && health <= SalhadaarHungerPct)
            beginKingsHunger();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SALHADAAR_TYRANNY:
                castSpellAOE(SPELL_SALHADAAR_TYRANNY);
                scriptEvents.addEvent(EVENT_SALHADAAR_TYRANNY, 30000);
                break;
            case EVENT_SALHADAAR_DECREE:
                castSpellAOE(SPELL_SALHADAAR_DECREE_OATH_BOUND);
                scriptEvents.addEvent(EVENT_SALHADAAR_INVOKE_THE_OATH, 12000);
                scriptEvents.addEvent(EVENT_SALHADAAR_DECREE, 60000);
                break;
            case EVENT_SALHADAAR_INVOKE_THE_OATH:
                warn("Nexus-King Salhadaar invokes the oath!");
                castSpellAOE(SPELL_SALHADAAR_INVOKE_THE_OATH);
                break;
            case EVENT_SALHADAAR_SUBJUGATION_RULE:
                castSpellOnVictim(SPELL_SALHADAAR_SUBJUGATION_RULE);
                scriptEvents.addEvent(EVENT_SALHADAAR_CONQUER, 2500);
                scriptEvents.addEvent(EVENT_SALHADAAR_VANQUISH, 5000);
                scriptEvents.addEvent(EVENT_SALHADAAR_SUBJUGATION_RULE, 35000);
                break;
            case EVENT_SALHADAAR_CONQUER:
                castSpellOnVictim(SPELL_SALHADAAR_CONQUER);
                break;
            case EVENT_SALHADAAR_VANQUISH:
                castSpellOnVictim(SPELL_SALHADAAR_VANQUISH);
                break;
            case EVENT_SALHADAAR_BANISHMENT:
                castAtPlayer(SPELL_SALHADAAR_BANISHMENT, true);
                scriptEvents.addEvent(EVENT_SALHADAAR_BANISHMENT, 25000);
                break;
            case EVENT_SALHADAAR_FRACTAL_IMAGES:
                castSpellOnSelf(SPELL_ROYAL_VOIDWING_FRACTAL_IMAGES, true);
                scriptEvents.addEvent(EVENT_SALHADAAR_BEHEAD, 3000);
                scriptEvents.addEvent(EVENT_SALHADAAR_FRACTAL_IMAGES, 45000);
                break;
            case EVENT_SALHADAAR_BEHEAD:
                castAtPlayer(SPELL_ROYAL_VOIDWING_BEHEAD, true, true);
                break;
            case EVENT_SALHADAAR_BESIEGE:
                castAtPlayer(SPELL_ROYAL_VOIDWING_BESIEGE, false, true);
                scriptEvents.addEvent(EVENT_SALHADAAR_BESIEGE, 45000);
                break;
            case EVENT_SALHADAAR_RALLY_CHECK:
                if (rallyLeft() == 0)
                    beginRiderOfTheDark();
                else
                    scriptEvents.addEvent(EVENT_SALHADAAR_RALLY_CHECK, 2000);
                break;
            case EVENT_SALHADAAR_RALLY_TIMEOUT:
                beginRiderOfTheDark();
                break;
            case EVENT_SALHADAAR_NETHERBREAKER:
                castAtPlayer(SPELL_SALHADAAR_NETHERBREAKER, false);
                scriptEvents.addEvent(EVENT_SALHADAAR_NETHERBREAKER, 25000);
                break;
            case EVENT_SALHADAAR_VOIDWING_CONSUMED:
                summons.despawnEntry(NPC_SALHADAAR_ROYAL_VOIDWING);
                break;
            case EVENT_SALHADAAR_TWILIGHT_FALLS:
                beginWorldInTwilight();
                break;
            case EVENT_SALHADAAR_GALACTIC_SMASH:
                warn("Nexus-King Salhadaar prepares a Galactic Smash!");
                castSpellAOE(SPELL_SALHADAAR_GALACTIC_SMASH);
                scriptEvents.addEvent(EVENT_SALHADAAR_GALACTIC_SMASH, 50000);
                break;
            case EVENT_SALHADAAR_STARKILLER_SWING:
                castSpellOnVictim(SPELL_SALHADAAR_STARKILLER_SWING);
                scriptEvents.addEvent(EVENT_SALHADAAR_STARKILLER_SWING, 30000);
                break;
            default:
                break;
        }
    }

private:
    void beginOathBreakers()
    {
        enterStage(StageOathBreakers);
        scriptEvents.addEvent(EVENT_SALHADAAR_DECREE, 10000);
        scriptEvents.addEvent(EVENT_SALHADAAR_TYRANNY, 12000);
        scriptEvents.addEvent(EVENT_SALHADAAR_BANISHMENT, 15000);
        scriptEvents.addEvent(EVENT_SALHADAAR_SUBJUGATION_RULE, 20000);
        scriptEvents.addEvent(EVENT_SALHADAAR_FRACTAL_IMAGES, 30000);
        scriptEvents.addEvent(EVENT_SALHADAAR_BESIEGE, 40000);
    }

    // warded while his elite Shadowguard hold the line
    void beginNexusDescent()
    {
        enterStage(StageNexusDescent);
        warn("Nexus-King Salhadaar rallies the Shadowguard!");
        setShielded(true);
        castSpellOnSelf(SPELL_SALHADAAR_ROYAL_WARD, true);
        castSpellAOE(SPELL_SALHADAAR_SEAL_THE_FORGE, true);
        castSpellAOE(SPELL_SALHADAAR_RALLY_THE_SHADOWGUARD, true);

        callAdd(NPC_SALHADAAR_MANAFORGED_TITAN, aroundSelf(12.0f, 18.0f));
        callAdd(NPC_SALHADAAR_NEXUS_PRINCE_XEVVOS, aroundSelf(12.0f, 18.0f));
        callAdd(NPC_SALHADAAR_NEXUS_PRINCE_KYVOR, aroundSelf(12.0f, 18.0f));
        for (uint8_t i = 0; i < 2; ++i)
            callAdd(NPC_SALHADAAR_SHADOWGUARD_REAPER, aroundSelf(12.0f, 18.0f));

        scriptEvents.addEvent(EVENT_SALHADAAR_RALLY_CHECK, 5000);
        scriptEvents.addEvent(EVENT_SALHADAAR_TYRANNY, 10000);
        scriptEvents.addEvent(EVENT_SALHADAAR_RALLY_TIMEOUT, 90000);
    }

    // the images converge back into the Royal Voidwing
    void beginRiderOfTheDark()
    {
        enterStage(StageRiderOfTheDark);
        _removeAura(SPELL_SALHADAAR_ROYAL_WARD);
        setShielded(false);
        warn("Nexus-King Salhadaar takes his seat atop the Royal Voidwing!");
        castSpellOnSelf(SPELL_SALHADAAR_COALESCE_VOIDWING, true);
        callAdd(NPC_SALHADAAR_ROYAL_VOIDWING, aroundSelf(4.0f, 6.0f));

        scriptEvents.addEvent(EVENT_SALHADAAR_TYRANNY, 10000);
        scriptEvents.addEvent(EVENT_SALHADAAR_NETHERBREAKER, 12000);
    }

    // he devours the Voidwing's energy
    void beginKingsHunger()
    {
        enterStage(StageKingsHunger);
        warn("Nexus-King Salhadaar consumes the power of the Royal Voidwing!");
        setShielded(true);
        castSpellOnSelf(SPELL_SALHADAAR_ROYAL_WARD, true);
        castSpellOnSelf(SPELL_SALHADAAR_KINGS_HUNGER);

        scriptEvents.addEvent(EVENT_SALHADAAR_VOIDWING_CONSUMED, 8000);
        scriptEvents.addEvent(EVENT_SALHADAAR_TWILIGHT_FALLS, 15000);
    }

    void beginWorldInTwilight()
    {
        enterStage(StageWorldInTwilight);
        _removeAura(SPELL_SALHADAAR_ROYAL_WARD);
        setShielded(false);
        warn("Nexus-King Salhadaar stands alone as the final champion of the void!");
        castSpellAOE(SPELL_SALHADAAR_WORLD_IN_TWILIGHT, true);

        scriptEvents.addEvent(EVENT_SALHADAAR_TYRANNY, 10000);
        scriptEvents.addEvent(EVENT_SALHADAAR_GALACTIC_SMASH, 15000);
        scriptEvents.addEvent(EVENT_SALHADAAR_STARKILLER_SWING, 25000);
    }

    uint32_t rallyLeft()
    {
        ManaforgeOmegaInstanceScript* pManaforge = getManaforge();
        return pManaforge != nullptr ? pManaforge->countAlive(SalhadaarRallyEntries) : 0;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Dimensius, the All-Devouring

class ManaforgeOmegaDimensiusAI : public ManaforgeOmegaBossAI
{
    enum DimensiusStage : uint32_t
    {
        StageCriticalMass = 1,
        StageEventHorizon,
        StageDarkHeart,
        StageSingularity
    };

public:
    static CreatureAIScript* Create(Creature* c) { return new ManaforgeOmegaDimensiusAI(c); }
    explicit ManaforgeOmegaDimensiusAI(Creature* pCreature) : ManaforgeOmegaBossAI(pCreature, DATA_DIMENSIUS) {}

protected:
    void onEngage() override
    {
        armBerserk(DimensiusBerserkMs);
        castSpellOnSelf(SPELL_DIMENSIUS_FISTS_OF_THE_VOIDLORD, true);
        castSpellAOE(SPELL_DIMENSIUS_COSMIC_RADIATION, true);
        beginCriticalMass();
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == StageCriticalMass && _getHealthPercent() <= DimensiusHorizonPct)
            beginEventHorizon();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DIMENSIUS_DEVOUR:
                warn("Dimensius attempts to Devour you!");
                castSpellAOE(SPELL_DIMENSIUS_DEVOUR);
                castSpellOnSelf(SPELL_DIMENSIUS_GROWING_HUNGER, true);
                scriptEvents.addEvent(EVENT_DIMENSIUS_DEVOUR, 90000);
                break;
            case EVENT_DIMENSIUS_MASSIVE_SMASH:
                castSpellOnVictim(SPELL_DIMENSIUS_MASSIVE_SMASH);
                scriptEvents.addEvent(EVENT_DIMENSIUS_LIVING_MASS, 2000);
                scriptEvents.addEvent(EVENT_DIMENSIUS_MASSIVE_SMASH, 30000);
                break;
            case EVENT_DIMENSIUS_LIVING_MASS:
            {
                uint8_t const masses = isHeroic() ? 3 : 2;
                for (uint8_t i = 0; i < masses; ++i)
                    callAdd(NPC_DIMENSIUS_LIVING_MASS, aroundSelf(5.0f, 10.0f));
                break;
            }
            case EVENT_DIMENSIUS_DARK_MATTER:
                castAtPlayers(SPELL_DIMENSIUS_DARK_MATTER, 3, false);
                scriptEvents.addEvent(EVENT_DIMENSIUS_DARK_MATTER, 25000);
                break;
            case EVENT_DIMENSIUS_SHATTERED_SPACE:
                castSpellAOE(SPELL_DIMENSIUS_SHATTERED_SPACE);
                scriptEvents.addEvent(EVENT_DIMENSIUS_SHATTERED_SPACE, 40000);
                break;
            case EVENT_DIMENSIUS_REVERSE_GRAVITY:
                castAtPlayers(SPELL_DIMENSIUS_REVERSE_GRAVITY, 3, true);
                scriptEvents.addEvent(EVENT_DIMENSIUS_REVERSE_GRAVITY, 50000);
                break;
            case EVENT_DIMENSIUS_ENDLESS_DARKNESS:
                castSpellAOE(SPELL_DIMENSIUS_ENDLESS_DARKNESS, true);
                break;
            case EVENT_DIMENSIUS_BROKEN_WORLD:
                castSpellAOE(SPELL_DIMENSIUS_BROKEN_WORLD, true);
                break;
            case EVENT_DIMENSIUS_ASTROPHYSICAL_JET:
                castAtPlayer(SPELL_DIMENSIUS_ASTROPHYSICAL_JET, false, true);
                scriptEvents.addEvent(EVENT_DIMENSIUS_ASTROPHYSICAL_JET, 6000);
                break;
            case EVENT_DIMENSIUS_BLACK_HOLE:
                castAtPlayer(SPELL_DIMENSIUS_BLACK_HOLE, false, true);
                scriptEvents.addEvent(EVENT_DIMENSIUS_BLACK_HOLE, 12000);
                break;
            case EVENT_DIMENSIUS_STELLAR_CORE:
                castSpellAOE(SPELL_DIMENSIUS_STELLAR_CORE, true);
                scriptEvents.addEvent(EVENT_DIMENSIUS_STELLAR_CORE, 15000);
                break;
            case EVENT_DIMENSIUS_HORIZON_END:
                beginDarkHeart();
                break;
            case EVENT_DIMENSIUS_GAMMA_BURST:
                castSpellAOE(SPELL_DIMENSIUS_GAMMA_BURST, true);
                scriptEvents.addEvent(EVENT_DIMENSIUS_GAMMA_BURST, 22000);
                break;
            case EVENT_DIMENSIUS_GRAVITATIONAL_DISTORTION:
                castAtPlayer(SPELL_DIMENSIUS_GRAVITATIONAL_DISTORTION, false);
                scriptEvents.addEvent(EVENT_DIMENSIUS_GRAVITATIONAL_DISTORTION, 30000);
                break;
            case EVENT_DIMENSIUS_EXTINCTION:
                warn("Dimensius calls for Extinction!");
                castSpellAOE(SPELL_DIMENSIUS_EXTINCTION, true);
                scriptEvents.addEvent(EVENT_DIMENSIUS_EXTINCTION, 40000);
                break;
            case EVENT_DIMENSIUS_LORDS_CHECK:
                if (lordsLeft() == 0)
                    beginSingularity();
                else
                    scriptEvents.addEvent(EVENT_DIMENSIUS_LORDS_CHECK, 2000);
                break;
            case EVENT_DIMENSIUS_LORDS_TIMEOUT:
                beginSingularity();
                break;
            case EVENT_DIMENSIUS_ACCRETION_DISK:
                castSpellAOE(SPELL_DIMENSIUS_ACCRETION_DISK);
                scriptEvents.addEvent(EVENT_DIMENSIUS_ACCRETION_DISK, 30000);
                break;
            case EVENT_DIMENSIUS_COSMIC_COLLAPSE:
                castSpellOnVictim(SPELL_DIMENSIUS_COSMIC_COLLAPSE);
                scriptEvents.addEvent(EVENT_DIMENSIUS_COSMIC_COLLAPSE, 25000);
                break;
            case EVENT_DIMENSIUS_EXTINGUISH_THE_STARS:
                warn("Dimensius begins to Extinguish the Stars!");
                castSpellAOE(SPELL_DIMENSIUS_EXTINGUISH_THE_STARS);
                scriptEvents.addEvent(EVENT_DIMENSIUS_EXTINGUISH_THE_STARS, 45000);
                break;
            case EVENT_DIMENSIUS_DARKENED_SKY:
                castSpellAOE(SPELL_DIMENSIUS_DARKENED_SKY);
                scriptEvents.addEvent(EVENT_DIMENSIUS_DARKENED_SKY, 40000);
                break;
            case EVENT_DIMENSIUS_SINGULARITY_DEVOUR:
                warn("Dimensius attempts to Devour you!");
                castSpellAOE(SPELL_DIMENSIUS_SINGULARITY_DEVOUR);
                scriptEvents.addEvent(EVENT_DIMENSIUS_SINGULARITY_DEVOUR, 60000);
                break;
            default:
                break;
        }
    }

private:
    // stage one: Devour on every full energy bar
    void beginCriticalMass()
    {
        enterStage(StageCriticalMass);
        scriptEvents.addEvent(EVENT_DIMENSIUS_DEVOUR, 6000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_MASSIVE_SMASH, 15000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_DARK_MATTER, 20000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_SHATTERED_SPACE, 30000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_REVERSE_GRAVITY, 45000);
    }

    // intermission cast from where he stands, the flight is left out
    void beginEventHorizon()
    {
        enterStage(StageEventHorizon);
        warn("Dimensius pulls you to the edge of the Event Horizon!");
        setShielded(true);

        scriptEvents.addEvent(EVENT_DIMENSIUS_ENDLESS_DARKNESS, 2000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_ASTROPHYSICAL_JET, 5000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_BROKEN_WORLD, 8000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_BLACK_HOLE, 10000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_STELLAR_CORE, 14000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_HORIZON_END, DimensiusHorizonMs);
    }

    // stage two: the Devoured Lords hold the players back
    void beginDarkHeart()
    {
        enterStage(StageDarkHeart);
        warn("Dimensius releases the Devoured Lords!");
        castSpellAOE(SPELL_DIMENSIUS_ECLIPSE, true);
        callAdd(NPC_DIMENSIUS_ARTOSHION, aroundSelf(15.0f, 20.0f));
        callAdd(NPC_DIMENSIUS_PARGOTH, aroundSelf(15.0f, 20.0f));

        scriptEvents.addEvent(EVENT_DIMENSIUS_LORDS_CHECK, 5000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_GAMMA_BURST, 10000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_GRAVITATIONAL_DISTORTION, 15000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_EXTINCTION, 20000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_LORDS_TIMEOUT, DimensiusLordsMs);
    }

    void beginSingularity()
    {
        enterStage(StageSingularity);
        setShielded(false);
        warn("Dimensius collapses into a Singularity!");
        castSpellOnSelf(SPELL_DIMENSIUS_DESTABILIZED, true);
        castSpellAOE(SPELL_DIMENSIUS_SINGULARITY_RADIATION, true);

        scriptEvents.addEvent(EVENT_DIMENSIUS_ACCRETION_DISK, 8000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_COSMIC_COLLAPSE, 12000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_EXTINGUISH_THE_STARS, 20000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_GRAVITATIONAL_DISTORTION, 25000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_DARKENED_SKY, 30000);
        scriptEvents.addEvent(EVENT_DIMENSIUS_SINGULARITY_DEVOUR, 40000);
    }

    uint32_t lordsLeft()
    {
        ManaforgeOmegaInstanceScript* pManaforge = getManaforge();
        return pManaforge != nullptr ? pManaforge->countAlive(DevouredLordEntries) : 0;
    }
};

// Artoshion and Pargoth, each with its own signature spell
class ManaforgeOmegaDevouredLordAI : public ManaforgeOmegaCombatAI
{
public:
    static CreatureAIScript* CreateArtoshion(Creature* c) { return new ManaforgeOmegaDevouredLordAI(c, SPELL_ARTOSHION_MASS_EJECTION, ManaforgeAim::Victim); }
    static CreatureAIScript* CreatePargoth(Creature* c) { return new ManaforgeOmegaDevouredLordAI(c, SPELL_PARGOTH_STARDUST_NOVA, ManaforgeAim::Area); }

    ManaforgeOmegaDevouredLordAI(Creature* pCreature, uint32_t signatureSpellId, ManaforgeAim signatureAim)
        : ManaforgeOmegaCombatAI(pCreature), mSignatureSpellId(signatureSpellId), mSignatureAim(signatureAim) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_DEVOURED_LORD_TOUCH_OF_OBLIVION, 6000);
        scriptEvents.addEvent(EVENT_DEVOURED_LORD_SIGNATURE, 10000);
        scriptEvents.addEvent(EVENT_DEVOURED_LORD_CONQUERORS_CROSS, 15000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DEVOURED_LORD_TOUCH_OF_OBLIVION:
                castSpellOnVictim(SPELL_DEVOURED_LORD_TOUCH_OF_OBLIVION);
                scriptEvents.addEvent(EVENT_DEVOURED_LORD_TOUCH_OF_OBLIVION, 12000);
                break;
            case EVENT_DEVOURED_LORD_SIGNATURE:
                castAimed(mSignatureSpellId, mSignatureAim);
                scriptEvents.addEvent(EVENT_DEVOURED_LORD_SIGNATURE, 20000);
                break;
            case EVENT_DEVOURED_LORD_CONQUERORS_CROSS:
                castSpellAOE(SPELL_DEVOURED_LORD_CONQUERORS_CROSS);
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_DIMENSIUS_VOIDWARDEN, aroundSelf(4.0f, 8.0f));
                if (isHeroic())
                    callAdd(NPC_DIMENSIUS_NULLBINDER, aroundSelf(4.0f, 8.0f));
                scriptEvents.addEvent(EVENT_DEVOURED_LORD_CONQUERORS_CROSS, 40000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mSignatureSpellId;
    ManaforgeAim mSignatureAim;
};

void SetupManaforgeOmega(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MANAFORGE_OMEGA, &ManaforgeOmegaInstanceScript::Create);

    // The Soul Hunters
    mgr->register_creature_script(BOSS_ADARUS_DUSKBLAZE, &ManaforgeOmegaAdarusAI::Create);
    mgr->register_creature_script(BOSS_VELARYN_BLOODWRATH, &ManaforgeOmegaVelarynAI::Create);
    mgr->register_creature_script(BOSS_ILYSSA_DARKSORROW, &ManaforgeOmegaIlyssaAI::Create);

    // Plexus Sentinel
    mgr->register_creature_script(BOSS_PLEXUS_SENTINEL, &ManaforgeOmegaPlexusSentinelAI::Create);
    mgr->register_creature_script(NPC_PLEXUS_VOLATILE_MANIFESTATION, &ManaforgeOmegaAddAI::CreateVolatileManifestation);

    // Soulbinder Naazindhri
    mgr->register_creature_script(BOSS_SOULBINDER_NAAZINDHRI, &ManaforgeOmegaNaazindhriAI::Create);
    mgr->register_creature_script(NPC_NAAZINDHRI_SHADOWGUARD_MAGE, &ManaforgeOmegaAddAI::CreateShadowguardMage);
    mgr->register_creature_script(NPC_NAAZINDHRI_SHADOWGUARD_ASSASSIN, &ManaforgeOmegaAddAI::CreateShadowguardAssassin);
    mgr->register_creature_script(NPC_NAAZINDHRI_SHADOWGUARD_PHASEBLADE, &ManaforgeOmegaAddAI::CreateShadowguardPhaseblade);

    // Loom'ithar
    mgr->register_creature_script(BOSS_LOOMITHAR, &ManaforgeOmegaLoomitharAI::Create);

    // Forgeweaver Araz
    mgr->register_creature_script(BOSS_FORGEWEAVER_ARAZ, &ManaforgeOmegaArazAI::Create);
    mgr->register_creature_script(NPC_ARAZ_ARCANE_COLLECTOR, &ManaforgeOmegaArcaneCollectorAI::Create);
    mgr->register_creature_script(NPC_ARAZ_ARCANE_COLLECTOR_ALT, &ManaforgeOmegaArcaneCollectorAI::Create);
    mgr->register_creature_script(NPC_ARAZ_ARCANE_MANIFESTATION, &ManaforgeOmegaDrifterAI::CreateArcaneManifestation);
    mgr->register_creature_script(NPC_ARAZ_ARCANE_ECHO, &ManaforgeOmegaAddAI::CreateArcaneEcho);
    mgr->register_creature_script(NPC_ARAZ_SHIELDED_ATTENDANT, &ManaforgeOmegaAddAI::CreateShieldedAttendant);
    mgr->register_creature_script(NPC_ARAZ_VOID_MANIFESTATION, &ManaforgeOmegaDrifterAI::CreateVoidManifestation);

    // Fractillus
    mgr->register_creature_script(BOSS_FRACTILLUS, &ManaforgeOmegaFractillusAI::Create);

    // Nexus-King Salhadaar
    mgr->register_creature_script(BOSS_NEXUS_KING_SALHADAAR, &ManaforgeOmegaSalhadaarAI::Create);
    mgr->register_creature_script(NPC_SALHADAAR_ROYAL_VOIDWING, &ManaforgeOmegaAddAI::CreateRoyalVoidwing);
    mgr->register_creature_script(NPC_SALHADAAR_MANAFORGED_TITAN, &ManaforgeOmegaAddAI::CreateManaforgedTitan);
    mgr->register_creature_script(NPC_SALHADAAR_NEXUS_PRINCE_XEVVOS, &ManaforgeOmegaAddAI::CreateNexusPrince);
    mgr->register_creature_script(NPC_SALHADAAR_NEXUS_PRINCE_KYVOR, &ManaforgeOmegaAddAI::CreateNexusPrince);
    mgr->register_creature_script(NPC_SALHADAAR_SHADOWGUARD_REAPER, &ManaforgeOmegaAddAI::CreateShadowguardReaper);

    // Dimensius, the All-Devouring
    mgr->register_creature_script(BOSS_DIMENSIUS, &ManaforgeOmegaDimensiusAI::Create);
    mgr->register_creature_script(NPC_DIMENSIUS_LIVING_MASS, &ManaforgeOmegaAddAI::CreateLivingMass);
    mgr->register_creature_script(NPC_DIMENSIUS_ARTOSHION, &ManaforgeOmegaDevouredLordAI::CreateArtoshion);
    mgr->register_creature_script(NPC_DIMENSIUS_PARGOTH, &ManaforgeOmegaDevouredLordAI::CreatePargoth);
}
