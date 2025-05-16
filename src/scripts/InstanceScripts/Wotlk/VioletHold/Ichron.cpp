/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Ichron.hpp"

#include "Movement/MovementManager.h"
#include "Movement/MovementGenerators/PointMovementGenerator.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "CommonTime.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
//  Ichron AI
IchronAI::IchronAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();

    // Spells
    addAISpell(Ichron::SPELL_WATER_BOLT_VOLLEY, 33.0f, 10, [this]() { return getCreature(); });
    addAISpell(Ichron::SPELL_WATER_BLAST, 33.0f, 8, [this]() { return getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 50.0f); });

    // Emotes
    addEmoteForEvent(Event_OnCombatStart, Ichron::SAY_AGGRO);
    addEmoteForEvent(Event_OnTargetDied, Ichron::SAY_SLAY1);
    addEmoteForEvent(Event_OnTargetDied, Ichron::SAY_SLAY2);
    addEmoteForEvent(Event_OnTargetDied, Ichron::SAY_SLAY3);
    addEmoteForEvent(Event_OnDied, Ichron::SAY_DEATH);
}

CreatureAIScript* IchronAI::Create(Creature* pCreature) { return new IchronAI(pCreature); }

void IchronAI::OnLoad()
{
    initialize();

    getCreature()->getMovementManager()->moveTargetedHome();
    getCreature()->getAIInterface()->setImmuneToNPC(true);
    getCreature()->getAIInterface()->setImmuneToPC(true);
    getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);

    /// for some reason ichoron can't walk back to it's water basin on evade
    getCreature()->addUnitStateFlag(UNIT_STATE_IGNORE_PATHFINDING);
}

void IchronAI::OnCombatStop(Unit* /*_target*/)
{
    initialize();
}

void IchronAI::initialize()
{
    mIsFrenzy = false;
    mDehydration = true;

    getCreature()->castSpell(getCreature(), Ichron::SPELL_SHRINK);
    getCreature()->castSpell(getCreature(), Ichron::SPELL_PROTECTIVE_BUBBLE);
}

void IchronAI::OnDied(Unit* /*_killer*/)
{
    if (mInstance)
        mInstance->setBossState(DATA_ICHORON, Performed);
}

void IchronAI::justReachedSpawn()
{
    // Reset Cell Door
    if (mInstance)
        mInstance->setLocalData(DATA_HANDLE_CELLS, DATA_ICHORON);
}

void IchronAI::DoAction(int32_t actionId)
{
    switch (actionId)
    {
        case Ichron::ACTION_WATER_GLOBULE_HIT:
        {
            if (!getCreature()->isAlive())
                break;

            getCreature()->modHealth(int32_t(getCreature()->getPctFromMaxHealth(3)));
            mDehydration = false;
        } break;
        case Ichron::ACTION_PROTECTIVE_BUBBLE_SHATTERED:
        {
            sendDBChatMessage(Ichron::SAY_SHATTER);

            getCreature()->castSpell(nullptr, Ichron::SPELL_SPLATTER, true);
            getCreature()->castSpell(nullptr, Ichron::SPELL_BURST, true);
            getCreature()->castSpell(getCreature(), Ichron::SPELL_DRAINED, true);

            uint32_t damage = getCreature()->getPctFromMaxHealth(30);
            getCreature()->modHealth(-std::min<int32_t>(damage, getCreature()->getHealth() - 1));

            for (auto spell : mCreatureAISpells)
            {
                spell->setCooldownTimer(spell->mCooldownTimer->getExpireTime() + 15 * TimeVarsMs::Second);
            }
            break;
        }
        case Ichron::ACTION_DRAINED:
        {
            if (getCreature()->getHealthPct() > 30)
            {
                sendDBChatMessage(Ichron::SAY_BUBBLE);
                getCreature()->castSpell(getCreature(), Ichron::SPELL_PROTECTIVE_BUBBLE, true);
            }
        } break;
        default:
            break;
    }
}

uint32_t IchronAI::GetCreatureData(uint32_t type) const
{
    if (type == Ichron::DATA_DEHYDRATION)
        return mDehydration ? 1 : 0;
    return 0;
}

void IchronAI::onSummonedCreature(Creature* summon)
{
    summons.summon(summon);

    if (summon->getEntry() == NPC_ICHOR_GLOBULE)
        getCreature()->castSpell(summon, Ichron::SPELL_WATER_GLOBULE_VISUAL);
}

void IchronAI::OnSummonDespawn(Creature* /*summon*/)
{
    if (summons.empty())
        getCreature()->removeAllAurasById(Ichron::SPELL_DRAINED);
}

void IchronAI::AIUpdate(unsigned long /*time_passed*/)
{
    if (!mIsFrenzy && getCreature()->getHealthPct() < 25 && !getCreature()->hasAurasWithId(Ichron::SPELL_DRAINED))
    {
        sendDBChatMessage(Ichron::SAY_ENRAGE);
        getCreature()->castSpell(getCreature(), Ichron::SPELL_FRENZY, true);
        mIsFrenzy = true;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Ichron Globule AI
IchronGlobuleAI::IchronGlobuleAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();

    getCreature()->getAIInterface()->setReactState(REACT_PASSIVE);
}

CreatureAIScript* IchronGlobuleAI::Create(Creature* pCreature) { return new IchronGlobuleAI(pCreature); }

void IchronGlobuleAI::OnHitBySpell(uint32_t spellId, Unit* caster)
{
    Unit* unitCaster = caster->ToUnit();
    if (!unitCaster)
        return;

    if (spellId == Ichron::SPELL_WATER_GLOBULE_VISUAL)
    {
        getCreature()->castSpell(getCreature(), Ichron::SPELL_WATER_GLOBULE_TRANSFORM);
        getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->getMovementManager()->moveFollow(unitCaster, 0.0f, 0.0f);
    }
}

void IchronGlobuleAI::OnReachWP(uint32_t type, uint32_t /*pointId*/)
{
    if (type != FOLLOW_MOTION_TYPE)
        return;

    if (mInstance && mInstance->getCreatureFromData(DATA_ICHORON))
        return;

    getCreature()->castSpell(getCreature(), Ichron::SPELL_MERGE);
    despawn(1, 0);
}

void IchronGlobuleAI::DamageTaken(Unit* /*attacker*/, uint32_t* damage)
{
    if (mSplashTriggered)
        return;

    if (*damage >= getCreature()->getHealth())
    {
        // Dont Let us Take Dmg until we Casted our Splash
        *damage = 0;
        mSplashTriggered = true;
        getCreature()->castSpell(nullptr, Ichron::SPELL_SPLASH);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: 54269 - Merge
SpellScriptCheckDummy IchronMerge::onDummyOrScriptedEffect(Spell* spell, uint8_t /*effIndex*/)
{
    if (spell->getUnitTarget() == nullptr)
        return SpellScriptCheckDummy::DUMMY_OK;

    if (Creature* target =  spell->getUnitTarget()->ToCreature())
    {
        if (Aura* aura = target->getAuraWithId(Ichron::SPELL_SHRINK))
            aura->refreshOrModifyStack(false, -1);

        if (target->GetScript())
            target->GetScript()->DoAction(Ichron::ACTION_WATER_GLOBULE_HIT);
    }

    return SpellScriptCheckDummy::DUMMY_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: 54306 - Protective Bubble
SpellScriptCheckDummy IchronBubble::onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool /*apply*/)
{
    if (aur->getCharges() <= 1)
    {
        for (AreaAuraList::iterator itr = aur->targets.begin(); itr != aur->targets.end(); ++itr)
        {
            auto unit = aur->getOwner()->getWorldMap()->getUnit(*itr);
            if (unit == nullptr || !unit->ToCreature())
                return SpellScriptCheckDummy::DUMMY_OK;

            if (CreatureAIScript* targetAI = unit->ToCreature()->GetScript())
                targetAI->DoAction(Ichron::ACTION_PROTECTIVE_BUBBLE_SHATTERED);
        }
    }

    return SpellScriptCheckDummy::DUMMY_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Dehydration Achievement
bool achievement_Dehydration::canCompleteCriteria(uint32_t /*criteriaID*/, Player* /*pPlayer*/, Object* target)
{
    if (!target)
        return false;

    if (Creature* Ichoron = target->ToCreature())
        if (Ichoron->GetScript() && Ichoron->GetScript()->GetCreatureData(Ichron::DATA_DEHYDRATION))
            return true;

    return false;
}
