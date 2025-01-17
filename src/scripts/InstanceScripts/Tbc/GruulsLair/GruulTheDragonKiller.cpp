/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "GruulTheDragonKiller.hpp"
#include "Raid_GruulsLair.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Spell/Spell.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Gruul the Dragonkiller
GruulTheDragonkillerAI::GruulTheDragonkillerAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    mGrowth = addAISpell(SPELL_GROWTH, 100.0f, TARGET_SELF, 0, 30);
    mGrowth->setMaxStackCount(30);
    mGrowth->addDBEmote(GRUUL_EMOTE_GROW);

    mCaveIn = addAISpell(SPELL_CAVE_IN, 7.0f, TARGET_RANDOM_DESTINATION, 0, 25);

    mGroundSlam = addAISpell(SPELL_GROUND_SLAM, 8.0f, TARGET_SELF, 1, 35);
    mGroundSlam->addDBEmote(GRUUL_SAY_SLAM_01);
    mGroundSlam->addDBEmote(GRUUL_SAY_SLAM_02);

    mReverberation = addAISpell(SPELL_REVERBERATION, TARGET_SELF, 4, 0, 30);

    mShatter = addAISpell(SPELL_SHATTER, 0.0f, TARGET_SELF, 0, 1, 0);
    mShatter->addDBEmote(GRUUL_SAY_SHATTER_01);
    mShatter->addDBEmote(GRUUL_SAY_SHATTER_02);

    mHurtfulStrike = addAISpell(SPELL_HURTFUL_STRIKE, 0.0f, TARGET_ATTACKING);

    addEmoteForEvent(CreatureAIScript::Event_OnCombatStart, GRUUL_SAY_AGGRO);
    addEmoteForEvent(Event_OnTargetDied, GRUUL_SAY_SLAY_01);
    addEmoteForEvent(Event_OnTargetDied, GRUUL_SAY_SLAY_02);
    addEmoteForEvent(Event_OnTargetDied, GRUUL_SAY_SLAY_03);
    addEmoteForEvent(Event_OnDied, GRUUL_SAY_DEATH);
}

CreatureAIScript* GruulTheDragonkillerAI::Create(Creature* pCreature) { return new GruulTheDragonkillerAI(pCreature); }

void GruulTheDragonkillerAI::OnCombatStart(Unit* /*pTarget*/)
{
    scriptEvents.addEvent(EVENT_HURTFUL_STRIKE, 8000);
}

void GruulTheDragonkillerAI::OnCastSpell(uint32_t spellId)
{
    if (spellId == SPELL_GROUND_SLAM)
        scriptEvents.addEvent(EVENT_SHATTER, 5000);
}

void GruulTheDragonkillerAI::AIUpdate(unsigned long time_passed)
{
    if (_isCasting())
        return;

    scriptEvents.updateEvents(time_passed, getScriptPhase());

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case EVENT_SHATTER:
                _castAISpell(mShatter);
                break;
            case EVENT_HURTFUL_STRIKE:
            {
                Unit* pCurrentTarget = getCreature()->getAIInterface()->getCurrentTarget();
                if (pCurrentTarget != nullptr)
                {
                    Unit* pTarget = pCurrentTarget;
                    for (const auto& itr : getCreature()->getInRangePlayersSet())
                    {
                        Player* pPlayer = static_cast<Player*>(itr);
                        if (!pPlayer || !pPlayer->isAlive())
                            continue;
                        if (pPlayer->hasUnitFlags(UNIT_FLAG_FEIGN_DEATH))
                            continue;
                        if (getRangeToObject(pPlayer) > 8.0f)
                            continue;
                        if (getCreature()->getThreatManager().getThreat(pPlayer) >= getCreature()->getThreatManager().getThreat(pCurrentTarget))
                            continue;

                        pTarget = static_cast<Unit*>(pPlayer);
                    }

                    if (pTarget == pCurrentTarget)
                        _castAISpell(mHurtfulStrike);
                    else
                        getCreature()->castSpell(pTarget, SPELL_HURTFUL_STRIKE, true);
                }
                scriptEvents.addEvent(EVENT_HURTFUL_STRIKE, 8000);
            }
                break;
            default:
                break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Effect: Ground Slam
bool GroundSlamEffect(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* target = pSpell->getUnitTarget();

    if (!target || !target->isPlayer())
        return true;

    target->handleKnockback(target, Util::getRandomFloat(10.0f, 15.0f), Util::getRandomFloat(10.0f, 15.0f));

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Effect: Shatter
bool ShatterEffect(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* target = pSpell->getUnitTarget();

    if (!target)
        return true;

    target->removeAllAurasById(SPELL_STONED);
    target->castSpell(target, SPELL_SHATTER_EFFECT, true);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Shatter Damage
SpellScriptEffectDamage ShatterDamage::doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg)
{
    if (effIndex != EFF_INDEX_0 || spell->getUnitTarget() == nullptr)
        return SpellScriptEffectDamage::DAMAGE_DEFAULT;

    float radius = spell->getEffectRadius(EFF_INDEX_0);
    auto distance = spell->getUnitTarget()->GetDistance2dSq(spell->getCaster());

    if (distance < 1.0f)
        distance = 1.0f;

    *dmg = Util::float2int32(*dmg * ((radius - distance) / radius));

    return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
}
