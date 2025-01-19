/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "Server/Script/CreatureAIScript.hpp"

const float VazrudenMiddle[3] = { -1406.5f, 1746.5f, 81.2f };

const float VazrudenRing[2][3] =
{
    { -1430.0f, 1705.0f, 112.0f },
    { -1377.0f, 1760.0f, 112.0f }
};

enum NazanSpells
{
    // All
    SPELL_FIREBALL              = 34653,
    SPELL_CONE_OF_FIRE          = 30926,
    SPELL_FIRE_NOVA_VISUAL      = 19823,

    // Normal
    SPELL_REVENGE               = 19130,
    SPELL_SUMMON_LIQUID_FIRE    = 23971,

    // Heroic
    SPELL_BELLOWING_ROAR        = 39427,
    SPELL_REVENGE_H             = 40392,
    SPELL_SUMMON_LIQUID_FIRE_H  = 30928
};

enum VazrudenNazanSay
{
    // Vazruden Herald
    VAZRUDEN_INTRO              = 5385,

    // Vazruden
    VAZRUDEN_WIPE               = 4869,
    VAZRUDEN_AGGRO1             = 4868,
    VAZRUDEN_AGGRO2             = 4867,
    VAZRUDEN_AGGRO3             = 4866,
    VAZRUDEN_KILL1              = 4865,
    VAZRUDEN_KILL2              = 4864,
    VAZRUDEN_DIE                = 4863,

    // Nazan
    NAZAN_EMOTE                 = 5386
};

enum Phases
{
    NONE                    = 0,
    CombatStart             = 1,

    // Nazan
    AIR_PHASE               = 2,
    GROUND_PHASE            = 3,

    // Vazruden Herald
    SUMMON_PHASE            = 2
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Nazan
class NazanAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit NazanAI(Creature* pCreature);

    void AIUpdate(unsigned long time_passed) override;

    void OnSpellHitTarget(Object* /*target*/, SpellInfo const* /*info*/) override;

    void OnReachWP(uint32_t /*type*/, uint32_t /*id*/) override;

    void OnSummon(Unit* /*summoner*/) override;
    void onSummonedCreature(Creature* /*summon*/) override;

protected:
    uint32_t Fly_Timer = 45000;
    uint32_t VazrudenGUID = 0;

    CreatureAISpells* m_FireballSpell;
    CreatureAISpells* m_ConeOfFireSpell;
    CreatureAISpells* m_BelowingRoarSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Vazruden
class VazrudenAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit VazrudenAI(Creature* pCreature);

    void AIUpdate(unsigned long time_passed) override;

protected:
    bool wipeSaid = false;
    uint32_t targetCheck = 2000;

    CreatureAISpells* m_RenevgeSpell;
    CreatureAISpells* m_ConeOfFireSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Vazruden The Herald
class VazrudenTheHeraldAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit VazrudenTheHeraldAI(Creature* pCreature);

    void OnCombatStop(Unit* /*pTarget*/) override;
    void AIUpdate(unsigned long time_passed) override;

    void onSummonedCreature(Creature* /*summon*/) override;

    void OnReachWP(uint32_t /*type*/, uint32_t /*id*/) override;

    void DoAction(int32_t /*action*/) override;

    void summonAdds();
    void despawnAdds();

protected:
    bool summoned = false;
    uint32_t nazanGUID = 0;
    uint32_t vazrudenGUID = 0;
};
