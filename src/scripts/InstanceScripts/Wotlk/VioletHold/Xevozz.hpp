/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Script/CreatureAIScript.hpp"
#include "Utilities/Random.hpp"

namespace Xevozz
{
    static uint32_t const EtherealSphereCount = 3;
    static uint32_t const EtherealSphereSummonSpells[EtherealSphereCount] = { 54102, 54137, 54138 };
    static uint32_t const EtherealSphereHeroicSummonSpells[EtherealSphereCount] = { 54102, 54137, 54138 };

    enum Spells
    {
        SPELL_ARCANE_BARRAGE_VOLLEY         = 54202,
        SPELL_ARCANE_BUFFET                 = 54226,
        SPELL_SUMMON_TARGET_VISUAL          = 54111
    };

    enum NPCs
    {
        NPC_ETHEREAL_SPHERE                 = 29271,
        NPC_ETHEREAL_SPHERE2                = 32582,
        NPC_ETHEREAL_SUMMON_TARGET          = 29276
    };

    enum CreatureSpells
    {
        SPELL_ARCANE_POWER                  = 54160,
        H_SPELL_ARCANE_POWER                = 59474,
        SPELL_MAGIC_PULL                    = 50770,
        SPELL_SUMMON_PLAYERS                = 54164,
        SPELL_POWER_BALL_VISUAL             = 54141,
        SPELL_POWER_BALL_DAMAGE_TRIGGER     = 54207
    };

    enum Yells
    {
        // Xevozz
        SAY_AGGRO                           = 4541,
        SAY_SLAY1                           = 4542,
        SAY_SLAY2                           = 4543,
        SAY_SLAY3                           = 4544,
        SAY_DEATH                           = 4545,
        SAY_SPAWN                           = 4546,
        SAY_CHARGED                         = 4547,
        SAY_REPEAT_SUMMON1                  = 4548,
        SAY_REPEAT_SUMMON2                  = 4549,
        SAY_SUMMON_ENERGY                   = 4550,
    };

    enum SphereActions
    {
        ACTION_SUMMON                       = 1,
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Xevozz AI
class XevozzAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit XevozzAI(Creature* pCreature);

    void OnLoad() override;
    void OnCombatStart(Unit* /*_target*/) override;
    void AIUpdate(unsigned long /*time_passed*/) override;
    void OnDied(Unit* /*_killer*/) override;
    void justReachedSpawn() override;
    void onSummonedCreature(Creature* /*summon*/) override;
    void OnSpellHitTarget(Object* /*target*/, SpellInfo const* /*info*/) override;

    template <class C> typename C::value_type const& SelectRandomContainerElement(C const& container)
    {
        typename C::const_iterator it = container.begin();
        std::advance(it, Util::getRandomUInt(0, static_cast<uint32_t>(container.size() - 1)));
        return *it;
    }

protected:
    InstanceScript* mInstance;

    std::list<uint8_t> summonSpells = { 0, 1, 2 };
};

//////////////////////////////////////////////////////////////////////////////////////////
//  Ethereal Sphere AI
class EtherealSphereAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit EtherealSphereAI(Creature* pCreature);

    void OnLoad() override;

    void AIUpdate(unsigned long /*time_passed*/) override;

protected:
    InstanceScript* mInstance;
};
