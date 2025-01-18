/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "Server/Script/CreatureAIScript.hpp"
#include "Spell/SpellScript.hpp"

enum MagtheridonEvents
{
    // Magtheridon
    EVENT_BERSERK = 1,
    EVENT_CLEAVE,
    EVENT_BLAZE,
    EVENT_BLAST_NOVA,
    EVENT_QUAKE,
    EVENT_START_FIGHT,
    EVENT_RELEASED,
    EVENT_COLLAPSE,
    EVENT_DEBRIS_KNOCKDOWN,
    EVENT_DEBRIS,
    EVENT_NEARLY_EMOTE,
    EVENT_TAUNT
};

enum Phases
{
    PHASE_BANISH = 1,
    PHASE_1,
    PHASE_2,
    PHASE_3
};

enum Yells
{
    SAY_TAUNT01                         = 8740,
    SAY_TAUNT02                         = 8741,
    SAY_TAUNT03                         = 8742,
    SAY_TAUNT04                         = 8743,
    SAY_TAUNT05                         = 8743,
    SAY_TAUNT06                         = 8745,
    SAY_FREE                            = 8748,
    SAY_SLAY                            = 8751,
    SAY_BANISHED                        = 8749,
    SAY_COLLAPSE                        = 8752,
    SAY_DEATH                           = 8750
    //EMOTE_WEAKEN                        = not in database "%s's bonds begin to weaken!",
    //EMOTE_NEARLY_FREE                   = not in database "%s is nearly free of his bonds!",
    //EMOTE_BREAKS_FREE                   = not in database "%s breaks free!",
    //EMOTE_BLAST_NOVA                    = not in database "%s begins to cast Blast Nova!"
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Magtheridon
class MagtheridonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit MagtheridonAI(Creature* pCreature);

    void OnLoad() override;
    void CombatStart();
    void OnCombatStop(Unit* /*mTarget*/) override;
    void OnDied(Unit* /*killer*/) override;
    void OnTargetDied(Unit* target) override;
    void AIUpdate(unsigned long time_passed) override;
    void DoAction(int32_t action) override;
    void OnHitBySpell(uint32_t spellId, Unit* /*caster*/) override;
    void OnDamageTaken(Unit* /*_attacker*/, uint32_t /*_amount*/) override;
    void Reset();

protected:
    CreatureAISpells* shadowCage;
    CreatureAISpells* cleave;
    CreatureAISpells* blaseTarget;
    CreatureAISpells* quake;
    CreatureAISpells* blastNova;
    CreatureAISpells* berserk;
    CreatureAISpells* shake;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Soul Transfer
class SoulTransfer : public SpellScript
{
public:
    void filterEffectTargets(Spell* spell, uint8_t /*effectIndex*/, std::vector<uint64_t>* effectTargets) override;
};
