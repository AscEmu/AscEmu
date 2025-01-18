/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "Server/Script/CreatureAIScript.hpp"
#include "Spell/SpellScript.hpp"

namespace Beasts
{
    namespace Gormok
    {
        enum Spells
        {
            // Gormok
            SPELL_IMPALE                = 66331,
            SPELL_STAGGERING_STOMP      = 66330,
            SPELL_TANKING_GORMOK        = 66415,

            // Snobold
            SPELL_RISING_ANGER          = 66636,
            SPELL_SNOBOLLED             = 66406,
            SPELL_BATTER                = 66408,
            SPELL_FIRE_BOMB             = 66313,
            SPELL_HEAD_CRACK            = 66407,
            SPELL_JUMP_TO_HAND          = 66342,
            SPELL_RIDE_PLAYER           = 66245,
            SPELL_FIRE_BOMB_AURA        = 66318
        };

        enum Yells
        {
            EMOTE_SNOBOLLED             = 0
        };
    }

    namespace Dreadscale_Acidmaw
    {
        enum Spells
        {
            // Acidmaw & Dreadscale Generic
            SPELL_SWEEP                 = 66794,
            SUMMON_SLIME_POOL           = 66883,
            SPELL_EMERGE                = 66947,
            SPELL_SUBMERGE              = 66948,
            SPELL_SUBMERGE_2            = 66936,
            SPELL_ENRAGE                = 68335,
            SPELL_GROUND_VISUAL_0       = 66969,
            SPELL_GROUND_VISUAL_1       = 68302,
            SPELL_HATE_TO_ZERO          = 63984,
            // Acidmaw
            SPELL_ACID_SPIT             = 66880,
            SPELL_PARALYTIC_SPRAY       = 66901,
            SPELL_PARALYTIC_BITE        = 66824,
            SPELL_ACID_SPEW             = 66818,
            SPELL_PARALYSIS             = 66830,
            SPELL_PARALYTIC_TOXIN       = 66823,
            // Dreadscale
            SPELL_BURNING_BITE          = 66879,
            SPELL_MOLTEN_SPEW           = 66821,
            SPELL_FIRE_SPIT             = 66796,
            SPELL_BURNING_SPRAY         = 66902,
            SPELL_BURNING_BILE          = 66869,
            SPELL_BURNING_BILE2         = 66870,

            // Slime Pool
            SPELL_SLIME_POOL_VISUAL     = 66881,
            SPELL_SLIME_POOL_EFFECT     = 66882,
            SPELL_PACIFY_SELF           = 19951
        };

        enum Yells
        {
            EMOTE_ENRAGE                = 0,
            SAY_SPECIAL                 = 1
        };

        enum Model
        {
            MODEL_DREADSCALE_STATIONARY = 26935,
            MODEL_DREADSCALE_MOBILE     = 24564,
            MODEL_ACIDMAW_STATIONARY    = 29815,
            MODEL_ACIDMAW_MOBILE        = 29816
        };
    }

    namespace Icehowl
    {
        enum Spells
        {
            SPELL_FEROCIOUS_BUTT        = 66770,
            SPELL_MASSIVE_CRASH         = 66683,
            SPELL_WHIRL                 = 67345,
            SPELL_ARCTIC_BREATH         = 66688, // Scripted effect
            SPELL_TRAMPLE               = 66734,
            SPELL_FROTHING_RAGE         = 66759,
            SPELL_STAGGERED_DAZE        = 66758,
            SPELL_FURIOUS_CHARGE_SUMMON = 66729,
            SPELL_ROAR                  = 66736,
            SPELL_JUMP_BACK             = 66733,
            SPELL_SURGE_OF_ADRENALINE   = 68667,
            SPELL_BERSERK               = 26662
        };

        enum Yells
        {
            EMOTE_TRAMPLE_ROAR          = 0,
            EMOTE_TRAMPLE_FAIL          = 1,
            EMOTE_TRAMPLE_ENRAGE        = 2
        };
    }

    enum Actions
    {
        ACTION_ENABLE_FIRE_BOMB         = 1,
        ACTION_DISABLE_FIRE_BOMB,
        ACTION_ACTIVE_SNOBOLD,
        ACTION_ENRAGE,
        ACTION_TRAMPLE_FAIL,
        ACTION_GORMOK_DEAD,
        ACTION_JORMUNGARS_DEAD
    };

    enum Phases
    {
        PHASE_EVENT                     = 1,
        PHASE_COMBAT,
        PHASE_MOBILE,
        PHASE_STATIONARY,
        PHASE_SUBMERGED,
        PHASE_CHARGE
    };

    enum NorthrendBeastsPoint
    {
        POINT_INITIAL_MOVEMENT          = 1,
        POINT_MIDDLE,
        POINT_ICEHOWL_CHARGE
    };

    enum Misc
    {
        DATA_NEW_TARGET                 = 1,
        GORMOK_HAND_SEAT                = 4,
        MAX_SNOBOLDS                    = 4,
        SPLINE_INITIAL_MOVEMENT         = 1
    };
}

LocationVector const CombatStalkerPosition = { 563.8941f, 137.3333f, 405.8467f };

//////////////////////////////////////////////////////////////////////////////////////////
/// Northrend Beasts Combat Stalker
class CombatStalkerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit CombatStalkerAI(Creature* pCreature);

    void OnLoad() override;

    void DoAction(int32_t action) override;

protected:
    CreatureAIFunc startJormungars;
    CreatureAIFunc startIcehowl;

    void Berserk(CreatureAIFunc pThis);
    void StartJormungars(CreatureAIFunc pThis);
    void StartIcehowl(CreatureAIFunc pThis);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Northrend Beasts
class NorthrendBeastsAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit NorthrendBeastsAI(Creature* pCreature);

    void InitOrReset() override;

    void OnCombatStart(Unit* _target) override;
    void OnCombatStop(Unit* _target) override;
    void OnDied(Unit* /*_killer*/) override;

    void handleInitialMovement();
    void handleEncounterProgress();
    void handleWithHeroicEvents();

protected:
    void initialMove(CreatureAIFunc pThis);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Gormok
class GormokAI : public NorthrendBeastsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit GormokAI(Creature* pCreature);

    void OnCombatStart(Unit* _target) override;

    void OnReachWP(uint32_t type, uint32_t id) override;
    void OnAddPassenger(Unit* _passenger, int8_t _seatId) override;

protected:
    void Engage(CreatureAIFunc pThis);
    void Throw(CreatureAIFunc pThis);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Snobold
class SnoboldAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit SnoboldAI(Creature* pCreature);

    void InitOrReset() override;
    void OnLoad() override;
    void OnDied(Unit* /*_killer*/) override;

    void SetCreatureData64(uint32_t type, uint64_t data) override;

    bool canAttackTarget(Unit* target) override;

    void DoAction(int32_t action) override;

    void mountOnBoss();

    void CheckMount(CreatureAIFunc pThis);
    void Batter(CreatureAIFunc pThis);
    void HeadCrack(CreatureAIFunc pThis);

protected:
    uint64_t targetGUID;
    bool mountedOnPlayer;
    bool gormokDead;

    CreatureAIFunc func_CheckMount;
    CreatureAIFunc func_FireBomb;
    CreatureAIFunc func_Batter;
    CreatureAIFunc func_HeadCrack;
    CreatureAIFunc func_Snowballed;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Firebomb
class FireBombAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit FireBombAI(Creature* pCreature);

    void InitOrReset() override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Ride Player
class RidePlayer : public SpellScript
{
public:
    void onAuraCreate(Aura* aur) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Snobolled
class Snobolled : public SpellScript
{
public:
    SpellScriptExecuteState onAuraPeriodicTick(Aura* aur, AuraEffectModifier* aurEff, float_t* damage) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Firebomb
class Firebomb : public SpellScript
{
public:
    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effIndex) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Dreadscale
class DreadscaleAI : public NorthrendBeastsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit DreadscaleAI(Creature* pCreature);

    void InitOrReset() override;
    void OnCombatStart(Unit* _target) override;
    void OnDied(Unit* /*_killer*/) override;

    void OnReachWP(uint32_t type, uint32_t id) override;
    void DoAction(int32_t action) override;

    void addTasks();

protected:
    void Engage(CreatureAIFunc pThis);

    void submerge(CreatureAIFunc pThis);
    void emerge(CreatureAIFunc pThis);

    bool wasMobile = false;

    CreatureAIFunc func_Emerge;
    CreatureAIFunc func_Submerge;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Acidmaw
class AcidmawAI : public NorthrendBeastsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit AcidmawAI(Creature* pCreature);

    void InitOrReset() override;
    void OnCombatStart(Unit* _target) override;
    void OnDied(Unit* /*_killer*/) override;

    void DoAction(int32_t action) override;

    void addTasks();

protected:
    void Engage(CreatureAIFunc pThis);

    void submerge(CreatureAIFunc pThis);
    void emerge(CreatureAIFunc pThis);

    bool wasMobile = false;

    CreatureAIFunc func_Emerge;
    CreatureAIFunc func_Submerge;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Slime Pool
class SlimePoolAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit SlimePoolAI(Creature* pCreature);

    void OnLoad() override;

protected:
    void slimeEffect(CreatureAIFunc pThis);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Paralytic Spray
class ParalyticSpray : public SpellScript
{
public:
    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effIndex) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Paralytic Toxin
class ParalyticToxin : public SpellScript
{
public:
    void onAuraApply(Aura* aur) override;
    void onAuraRemove(Aura* aur, AuraRemoveMode mode) override;
    void onAuraRefreshOrGainNewStack(Aura* aur, uint32_t newStackCount, uint32_t oldStackCount) override;

protected:
    int32_t counter = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Burning Spray
class BurningSpray : public SpellScript
{
public:
    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effIndex) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Burning Bile
class BurningBile : public SpellScript
{
public:
    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effectIndex) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Slime Pool
class SlimePool : public SpellScript
{
public:
    SpellScriptExecuteState onAuraPeriodicTick(Aura* aur, AuraEffectModifier* aurEff, float_t* damage) override;

protected:
    int32_t stackCounter = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell paralysis
class Paralysis : public SpellScript
{
public:
    void onAuraApply(Aura* aur) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Icehowl
class IcehowlAI : public NorthrendBeastsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit IcehowlAI(Creature* pCreature);

    void OnCombatStart(Unit* _target) override;
    void OnReachWP(uint32_t type, uint32_t id) override;
    void DoAction(int32_t action) override;

    void addTasks();

protected:
    void Engage(CreatureAIFunc pThis);

    void Charge(CreatureAIFunc pThis);
    void Roar(CreatureAIFunc pThis);
    void JumpBack(CreatureAIFunc pThis);
    void Crash(CreatureAIFunc pThis);
    void Trample(CreatureAIFunc pThis);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Arctic Breath
bool ArcticBreathEffect(uint8_t /*effectIndex*/, Spell* pSpell);

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Trample
class Trample : public SpellScript
{
public:
    void filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Massive Crash
class MassiceCrash : public SpellScript
{
public:
    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effIndex) override;
};