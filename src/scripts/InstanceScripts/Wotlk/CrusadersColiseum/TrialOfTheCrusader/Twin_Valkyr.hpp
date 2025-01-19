/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Raid_TrialOfTheCrusader.hpp"
#include "Spell/SpellScript.hpp"

namespace twins
{
    enum Texts
    {
        SAY_AGGRO = 0,
        SAY_NIGHT = 1,
        SAY_LIGHT = 2,
        EMOTE_VORTEX = 3,
        EMOTE_TWIN_PACT = 4,
        SAY_TWIN_PACT = 5,
        SAY_KILL_PLAYER = 6,
        SAY_BERSERK = 7,
        SAY_DEATH = 8
    };

    enum Misc
    {
        EQUIP_MAIN_1 = 9423,
        EQUIP_MAIN_2 = 37377,
        POINT_INITIAL_MOVEMENT = 1,
        SPLINE_INITIAL_MOVEMENT = 1,
        PHASE_EVENT = 1,
        PHASE_COMBAT = 2
    };

    enum Summons
    {
        NPC_BULLET_CONTROLLER = 34743,

        NPC_BULLET_DARK = 34628,
        NPC_BULLET_LIGHT = 34630
    };

    enum BossSpells
    {
        SPELL_LIGHT_TWIN_SPIKE = 66075,
        SPELL_LIGHT_SURGE = 65766,
        SPELL_LIGHT_SHIELD = 65858,
        SPELL_LIGHT_TWIN_PACT = 65876,
        SPELL_LIGHT_VORTEX = 66046,
        SPELL_LIGHT_VORTEX_DAMAGE = 66048,
        SPELL_LIGHT_TOUCH = 67297,
        SPELL_LIGHT_ESSENCE = 65686,
        SPELL_EMPOWERED_LIGHT = 65748,
        SPELL_TWIN_EMPATHY_LIGHT = 66133,
        SPELL_UNLEASHED_LIGHT = 65795,

        SPELL_DARK_TWIN_SPIKE = 66069,
        SPELL_DARK_SURGE = 65768,
        SPELL_DARK_SHIELD = 65874,
        SPELL_DARK_TWIN_PACT = 65875,
        SPELL_DARK_VORTEX = 66058,
        SPELL_DARK_VORTEX_DAMAGE = 66059,
        SPELL_DARK_TOUCH = 67282,
        SPELL_DARK_ESSENCE = 65684,
        SPELL_EMPOWERED_DARK = 65724,
        SPELL_TWIN_EMPATHY_DARK = 66132,
        SPELL_UNLEASHED_DARK = 65808,

        SPELL_CONTROLLER_PERIODIC = 66149,
        SPELL_POWER_TWINS = 65879,
        SPELL_BERSERK = 64238,
        SPELL_POWERING_UP = 67590,
        SPELL_SURGE_OF_SPEED = 65828,

        SPELL_SUMMON_PERIODIC_LIGHT = 66152,
        SPELL_SUMMON_PERIODIC_DARK = 66153
    };

    enum Events
    {
        EVENT_SPECIAL_ABILITY = 1
    };

    enum Stages
    {
        STAGE_DARK_VORTEX,
        STAGE_DARK_PACT,
        STAGE_LIGHT_VORTEX,
        STAGE_LIGHT_PACT,
        MAX_STAGES
    };

    enum Actions
    {
        ACTION_VORTEX,
        ACTION_PACT
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Twins Base AI
class TwinsAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit TwinsAI(Creature* pCreature);

    void InitOrReset() override;
    void OnLoad() override;
    void OnCombatStart(Unit* /*_target*/) override;
    void justReachedSpawn() override;
    void OnDied(Unit* /*_killer*/) override;
    void OnReachWP(uint32_t /*type*/, uint32_t /*id*/) override;
    void DoAction(int32_t action) override;

    void handleRemoveAuras();
    void enableDualWield(bool mode = true);

protected:
    void intro(CreatureAIFunc pThis);

    uint32_t AuraState;
    uint32_t Weapon;
    uint32_t MyEmphatySpellId;
    uint32_t OtherEssenceSpellId;
    uint32_t SurgeSpellId;
    uint32_t VortexSpellId;
    uint32_t ShieldSpellId;
    uint32_t TwinPactSpellId;
    uint32_t SpikeSpellId;
    uint32_t TouchSpellId;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Fjola Lightbane
class FjolaAI : public TwinsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit FjolaAI(Creature* pCreature);

    void InitOrReset() override;
    void OnCombatStart(Unit* _target) override;
    void OnCombatStop(Unit* _target) override;
    void justReachedSpawn() override;
    void AIUpdate(unsigned long time_passed) override;
    void onSummonedCreature(Creature* summon) override;

    void generateStageSequence();

private:
    uint8_t Stage[twins::MAX_STAGES];
    uint8_t CurrentStage;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Eydis Darkbane
class EydisAI : public TwinsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit EydisAI(Creature* pCreature);

    void InitOrReset() override;
    void onSummonedCreature(Creature* summon) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Essence
class EssenceGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override;

    uint32_t getData(Creature* pCreature, bool data) const;

    template<class T> inline
    const T& RAID_MODE(Creature* pCreature, const T& normal10, const T& normal25, const T& heroic10, const T& heroic25) const
    {
        if (pCreature->getWorldMap()->getInstance())
        {
            switch (pCreature->getWorldMap()->getDifficulty())
            {
            case InstanceDifficulty::RAID_10MAN_NORMAL:
                return normal10;
            case InstanceDifficulty::RAID_25MAN_NORMAL:
                return normal25;
            case InstanceDifficulty::RAID_10MAN_HEROIC:
                return heroic10;
            case InstanceDifficulty::RAID_25MAN_HEROIC:
                return heroic25;
            default:
                break;
            }
        }

        return normal10;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Unleashed Ball
class UnleashedBallAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit UnleashedBallAI(Creature* pCreature);

    void OnLoad() override;
    void OnReachWP(uint32_t type, uint32_t id) override;

protected:
    TrialOfTheCrusaderInstanceScript* mInstance;
    uint32_t RangeCheckTimer;
    std::vector<uint32_t> stalkerGUIDS;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Unleashed Dark
class UnleashedDarkAI : public UnleashedBallAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit UnleashedDarkAI(Creature* pCreature);

    void AIUpdate(unsigned long time_passed) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Unleashed Light
class UnleashedLightAI : public UnleashedBallAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit UnleashedLightAI(Creature* pCreature);

    void AIUpdate(unsigned long time_passed) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Bullet Controller
class BulletCotrollerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit BulletCotrollerAI(Creature* pCreature);

    void OnLoad() override;

    void onSummonedCreature(Creature* summon) override;
    void OnSummonDespawn(Creature* summon) override;
    void OnDespawn() override;

protected:
    uint32_t lightCounter = 0;
    uint32_t darkCounter = 0;

    TrialOfTheCrusaderInstanceScript* mInstance;
    std::vector<uint32_t> stalkerGUIDS;
};


//////////////////////////////////////////////////////////////////////////////////////////
/// PoweringUp
bool PoweringUpEffect(uint8_t /*effectIndex*/, Spell* pSpell);

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Dark Essence and Light Essence
class EssenceScript : public SpellScript
{
public:
    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Power Of the Twins
class PowerOfTwinsScript : public SpellScript
{
public:
    void onAuraApply(Aura* aur) override;
    void onAuraRemove(Aura* aur, AuraRemoveMode mode) override;
};
