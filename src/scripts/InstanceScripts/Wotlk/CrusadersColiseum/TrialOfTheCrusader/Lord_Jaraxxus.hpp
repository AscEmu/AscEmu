/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "Server/Script/CreatureAIScript.hpp"

namespace jaraxxus
{
    enum Yells
    {
        SAY_INTRO                       = 0,
        SAY_AGGRO                       = 1,
        EMOTE_LEGION_FLAME              = 2,
        EMOTE_NETHER_PORTAL             = 3,
        SAY_MISTRESS_OF_PAIN            = 4,
        EMOTE_INCINERATE                = 5,
        SAY_INCINERATE                  = 6,
        EMOTE_INFERNAL_ERUPTION         = 7,
        SAY_INFERNAL_ERUPTION           = 8,
        SAY_KILL_PLAYER                 = 9,
        SAY_DEATH                       = 11,
        SAY_BERSERK                     = 10
    };

    enum Summons
    {
        NPC_LEGION_FLAME                = 34784,
        NPC_INFERNAL_VOLCANO            = 34813,
        NPC_FEL_INFERNAL                = 34815, // immune to all CC on Heroic (stuns, banish, interrupt, etc)
        NPC_NETHER_PORTAL               = 34825,
        NPC_MISTRESS_OF_PAIN            = 34826
    };

    enum BossSpells
    {
        SPELL_LEGION_FLAME              = 66197,
        SPELL_LEGION_FLAME_EFFECT       = 66201,
        SPELL_NETHER_POWER              = 66228,
        SPELL_FEL_LIGHTNING             = 66528,
        SPELL_FEL_FIREBALL              = 66532,
        SPELL_INCINERATE_FLESH          = 66237,
        SPELL_BURNING_INFERNO           = 66242,
        SPELL_INFERNAL_ERUPTION         = 66258,
        SPELL_INFERNAL_ERUPTION_EFFECT  = 66252,
        SPELL_NETHER_PORTAL             = 66269,
        SPELL_NETHER_PORTAL_EFFECT      = 66263,
        SPELL_NETHER_PORTAL_DAMAGE      = 66264,
        SPELL_LORD_JARAXXUS_HITTIN_YA   = 66327,
        SPELL_FEL_LIGHTNING_INTRO       = 67888,
        SPELL_BERSERK                   = 64238,

        // Mistress of Pain spells
        SPELL_SHIVAN_SLASH              = 66378,
        SPELL_SPINNING_SPIKE            = 66283,
        SPELL_MISTRESS_KISS             = 66336,
        SPELL_MISTRESS_KISS_DAMAGE_SILENCE = 66359,

        // Felflame Infernal
        SPELL_FEL_STREAK_VISUAL         = 66493
    };

    enum Misc
    {
        PHASE_INTRO                     = 1,
        PHASE_COMBAT                    = 2,
        SPLINE_INITIAL_MOVEMENT         = 1,
        POINT_SUMMONED                  = 1
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Jaraxxus
class JaraxxusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit JaraxxusAI(Creature* pCreature);

    void InitOrReset() override;
    void OnCombatStart(Unit* _target) override;
    void OnCombatStop(Unit* /*_target*/) override;
    void OnDied(Unit* /*_killer*/) override;
    void OnCastSpell(uint32_t spellId) override;
    void DoAction(int32_t action) override;

    void OnReachWP(uint32_t type, uint32_t id) override;

protected:
    void intro(CreatureAIFunc pThis);
    void taunt(CreatureAIFunc pThis);

    void killGnome(CreatureAIFunc pThis);
    void faceto(CreatureAIFunc pThis);
    void startCombat(CreatureAIFunc pThis);

    void summonPortal(CreatureAIFunc pThis);
    void summonInfernal(CreatureAIFunc pThis);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Wilfred Frizzlebang
class FrizzlebangAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit FrizzlebangAI(Creature* pCreature);

    void OnLoad() override;
    void OnReachWP(uint32_t /*type*/, uint32_t /*id*/) override;
    void OnHitBySpell(uint32_t /*_spellId*/, Unit* /*_caster*/) override;

protected:
    void startMove(CreatureAIFunc pThis);
    void oblivion(CreatureAIFunc pThis);
    void summonJaraxus(CreatureAIFunc pThis);
    void setTarget(CreatureAIFunc pThis);
    void lastTalk(CreatureAIFunc pThis);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Portal
class PortalAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit PortalAI(Creature* pCreature);

    void InitOrReset() override;
    void OnHitBySpell(uint32_t /*_spellId*/, Unit* /*_caster*/) override;

protected:
    void portalOpening(CreatureAIFunc pThis);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Legion Flame
class LegionFlameAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit LegionFlameAI(Creature* pCreature);

    void OnLoad() override;
    void OnCombatStop(Unit* /*_target*/) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Infernal Volcano
class InfernalVolcanoAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit InfernalVolcanoAI(Creature* pCreature);

    void OnLoad() override;
    void OnCombatStop(Unit* /*_target*/) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Fel Infernal
class FelInfernalAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit FelInfernalAI(Creature* pCreature);

    void OnLoad() override;
    void OnCombatStop(Unit* /*_target*/) override;

protected:
    void felStreak(CreatureAIFunc pThis);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Nether Portal
class NetherPortalAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit NetherPortalAI(Creature* pCreature);

    void OnLoad() override;
    void OnCombatStop(Unit* /*_target*/) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Mistress Of Pain
class MistressOfPainAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit MistressOfPainAI(Creature* pCreature);

    void OnLoad() override;
    void OnCombatStart(Unit* /*_target*/) override;
    void OnCombatStop(Unit* /*_target*/) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Felstreak
bool FelStreakEffect(uint8_t /*effectIndex*/, Spell* pSpell);
