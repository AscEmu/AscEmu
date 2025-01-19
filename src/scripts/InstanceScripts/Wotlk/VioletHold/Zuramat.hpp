/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Script/AchievementScript.hpp"

namespace Zuramat
{
    enum Spells
    {
        SPELL_SHROUD_OF_DARKNESS            = 54524,
        SPELL_SUMMON_VOID_SENTRY            = 54369,
        SPELL_VOID_SHIFT                    = 54361,
        SPELL_VOID_SHIFTED                  = 54343,
        SPELL_ZURAMAT_ADD                   = 54341,
        SPELL_ZURAMAT_ADD_2                 = 54342,
        SPELL_ZURAMAT_ADD_DUMMY             = 54351,
        SPELL_SUMMON_VOID_SENTRY_BALL       = 58650
    };

    enum Yells
    {
        SAY_AGGRO                           = 4551,
        SAY_SLAY1                           = 4552,
        SAY_SLAY2                           = 4553,
        SAY_SLAY3                           = 4554,
        SAY_DEATH                           = 4555,
        SAY_SPAWN                           = 4556,
        SAY_SHIELD                          = 4557,
        SAY_WHISPER                         = 4558
    };

    enum Misc
    {
        ACTION_DESPAWN_VOID_SENTRY_BALL     = 1,
        DATA_VOID_DANCE                     = 2153
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Zuramat AI
class ZuramatAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit ZuramatAI(Creature* pCreature);

    void OnLoad() override;
    void OnCombatStop(Unit* /*_target*/) override;
    void OnDied(Unit* /*_killer*/) override;
    void OnSummonDies(Creature* /*summon*/, Unit* /*killer*/) override;
    void OnSummonDespawn(Creature* /*summon*/) override;
    void justReachedSpawn() override;

    uint32_t GetCreatureData(uint32_t /*type*/) const override;

protected:
    InstanceScript* mInstance;

    bool mVoidDance = true;
};

//////////////////////////////////////////////////////////////////////////////////////////
//  Void Sentry AI
class VoidSentryAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit VoidSentryAI(Creature* pCreature);

    void DoAction(int32_t /*action*/) override;
    void OnSummon(Unit* /*summoner*/) override;
    void onSummonedCreature(Creature* /*summon*/) override;
    void OnDied(Unit* /*_killer*/) override;
    void OnSummonDespawn(Creature* /*summon*/) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
//  Void Sentry Achievement
class achievement_void_dance : public AchievementCriteriaScript
{
public:
    bool canCompleteCriteria(uint32_t criteriaID, Player* /*pPlayer*/, Object* target) override;
};
