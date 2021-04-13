/*
 *
 * SpellHandler Plugin
 * Copyright (c) 2007 Team Ascent
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * License. To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to Creative Commons,
 * 543 Howard Street, 5th Floor, San Francisco, California, 94105, USA.
 *
 * EXCEPT TO THE EXTENT REQUIRED BY APPLICABLE LAW, IN NO EVENT WILL LICENSOR BE LIABLE TO YOU
 * ON ANY LEGAL THEORY FOR ANY SPECIAL, INCIDENTAL, CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES
 * ARISING OUT OF THIS LICENSE OR THE USE OF THE WORK, EVEN IF LICENSOR HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 *
 */

#include "Setup.h"
#include "Management/QuestLogEntry.hpp"
#include "Management/Skill.h"
#include "Management/ItemInterface.h"
#include "../EventScripts/Setup.h"
#include "Objects/Faction.h"
#include "Spell/SpellAuras.h"

 /*
     How to add a new item spell to the dummy spell handler:

     1) Add a new function to handle the spell at the end of this file but before the
     SetupItemSpells_1() function. SetupItemSpells_1() must always be the last function.

     2) Register the dummy spell by adding a new line to the end of the list in the
     SetupItemSpells_1() function.

     Please look at how the other spells are handled and try to use the
     same variable names and formatting style in your new spell handler.
 */


bool BreathOfFire(uint8_t /*effectIndex*/, Spell* /*pSpell*/)
{
    /* No handler required */
    return true;
}

bool GnomishTransporter(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster())
        return true;

    pSpell->getPlayerCaster()->EventAttackStop();
    pSpell->getPlayerCaster()->SafeTeleport(1, 0, LocationVector(-7169.41f, -3838.63f, 8.72f));
    return true;
}

bool NoggenFoggerElixr(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster())
        return true;

    uint32_t chance = Util::getRandomUInt(2);

    switch (chance)
    {
        case 0:
            pSpell->getPlayerCaster()->castSpell(pSpell->getPlayerCaster(), sSpellMgr.getSpellInfo(16591), true);
            break;
        case 1:
            pSpell->getPlayerCaster()->castSpell(pSpell->getPlayerCaster(), sSpellMgr.getSpellInfo(16593), true);
            break;
        case 2:
            pSpell->getPlayerCaster()->castSpell(pSpell->getPlayerCaster(), sSpellMgr.getSpellInfo(16595), true);
            break;
    }
    return true;
}

bool HallowsEndCandy(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster())
        return true;

    int newspell = 24924 + Util::getRandomUInt(3);

    SpellInfo const* spInfo = sSpellMgr.getSpellInfo(newspell);
    if (!spInfo) return true;

    pSpell->getPlayerCaster()->castSpell(pSpell->getPlayerCaster(), spInfo, true);
    return true;
}

bool DeviateFish(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster())
        return true;

    int newspell = 8064 + Util::getRandomUInt(4);

    SpellInfo const* spInfo = sSpellMgr.getSpellInfo(newspell);
    if (!spInfo) return true;

    pSpell->getPlayerCaster()->castSpell(pSpell->getPlayerCaster(), spInfo, true);
    return true;
}

bool CookedDeviateFish(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster())
        return true;

    int chance = 0;
    int newspell = 0;

    chance = Util::getRandomUInt(1);

    switch (chance)
    {
        case 0:
            newspell = 8219; // Flip Out (60 min) (turns you into a ninja)
            break;
        case 1:
            newspell = 8221; // Yaaarrrr (60 min) (turns you into a pirate)
            break;
    }

    if (newspell)
    {
        SpellInfo const* spInfo = sSpellMgr.getSpellInfo(newspell);
        if (!spInfo) return true;

        pSpell->getPlayerCaster()->castSpell(pSpell->getPlayerCaster(), spInfo, true);
    }
    return true;
}

bool HolidayCheer(uint8_t effectIndex, Spell* pSpell)
{
    if (!pSpell->getCaster())
        return true;

    Unit* target;
    float dist = pSpell->GetRadius(effectIndex);

    for (const auto& itr : pSpell->getCaster()->getInRangeObjectsSet())
    {
        if (itr && itr->isCreatureOrPlayer())
            target = static_cast<Unit*>(itr);
        else
            continue;

        if (pSpell->getCaster()->CalcDistance(target) > dist || isAttackable(pSpell->getCaster(), target))
            continue;

        target->emote(EMOTE_ONESHOT_LAUGH);
    }
    return true;
}

bool NetOMatic(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* target = pSpell->GetUnitTarget();
    if (!pSpell->getPlayerCaster() || !target)
        return true;

    SpellInfo const* spInfo = sSpellMgr.getSpellInfo(13099);
    if (!spInfo)
        return true;

    int chance = Util::getRandomUInt(99) + 1;

    if (chance < 51) // nets target: 50%
        pSpell->getPlayerCaster()->castSpell(target, spInfo, true);

    else if (chance < 76) // nets you instead: 25%
        pSpell->getPlayerCaster()->castSpell(pSpell->getPlayerCaster(), spInfo, true);

    else // nets you and target: 25%
    {
        pSpell->getPlayerCaster()->castSpell(pSpell->getPlayerCaster(), spInfo, true);
        pSpell->getPlayerCaster()->castSpell(target, spInfo, true);
    }
    return true;
}

bool BanishExile(uint8_t effectIndex, Spell* pSpell)
{
    Unit* target = pSpell->GetUnitTarget();
    if (!pSpell->getPlayerCaster() || !target)
        return true;

    pSpell->getPlayerCaster()->doSpellDamage(target, pSpell->getSpellInfo()->getId(), static_cast<float_t>(target->getHealth()), effectIndex);
    return true;
}

bool ForemansBlackjack(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* target = pSpell->GetUnitTarget();
    if (!pSpell->getPlayerCaster() || !target || !target->isCreature())
        return true;

    // check to see that we have the correct creature
    Creature* c_target = static_cast<Creature*>(target);
    if (c_target->getEntry() != 10556 || !c_target->HasAura(17743))
        return true;

    // Start moving again
    if (target->GetAIInterface())
        target->GetAIInterface()->StopMovement(0);

    // Remove Zzz aura
    c_target->RemoveAllAuras();

    pSpell->getPlayerCaster()->sendPlayObjectSoundPacket(c_target->getGuid(), 6197);

    // send chat message
    char msg[100];
    sprintf(msg, "Ow! Ok, I'll get back to work, %s", pSpell->getPlayerCaster()->getName().c_str());
    target->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg);

    c_target->emote(EMOTE_STATE_WORK_CHOPWOOD);

    // Add timed event to return lazy peon to Zzz after 5-10 minutes (spell 17743)
    SpellInfo const* pSpellEntry = sSpellMgr.getSpellInfo(17743);
    sEventMgr.AddEvent(target, &Unit::eventCastSpell, target, pSpellEntry, EVENT_UNK, 300000 + Util::getRandomUInt(300000), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    return true;
}

bool NetherWraithBeacon(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster())
        return true;

    float SSX = pSpell->getPlayerCaster()->GetPositionX();
    float SSY = pSpell->getPlayerCaster()->GetPositionY();
    float SSZ = pSpell->getPlayerCaster()->GetPositionZ();
    float SSO = pSpell->getPlayerCaster()->GetOrientation();

    pSpell->getPlayerCaster()->GetMapMgr()->GetInterface()->SpawnCreature(22408, SSX, SSY, SSZ, SSO, true, false, 0, 0);
    return true;
}

bool NighInvulnBelt(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster())
        return true;

    int chance = Util::getRandomUInt(99) + 1;

    if (chance > 10)    // Buff - Nigh-Invulnerability - 30456
        pSpell->getPlayerCaster()->castSpell(pSpell->getPlayerCaster(), sSpellMgr.getSpellInfo(30456), true);
    else                // Malfunction - Complete Vulnerability - 30457
        pSpell->getPlayerCaster()->castSpell(pSpell->getPlayerCaster(), sSpellMgr.getSpellInfo(30457), true);

    return true;
}

bool ReindeerTransformation(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster())
        return true;

    if (pSpell->getPlayerCaster()->getMountDisplayId() != 0)
    {
        /*Zyres: This is not correct!
        if (pSpell->getPlayerCaster()->m_setflycheat)
            pSpell->getPlayerCaster()->setMountDisplayId(22724);
        else*/
        pSpell->getPlayerCaster()->setMountDisplayId(15902);
    }
    return true;
}

bool WinterWondervolt(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* target = pSpell->GetUnitTarget();

    if (target == NULL || !target->isPlayer())
        return true;

    uint32_t outfitspells[] =
    {
        26157, // green male
        26272, // green female
        26273, // red male
        26274  // red female
    };

    uint32_t team = target->GetTeam();
    uint32_t gender = target->getGender();
    uint32_t spellid = 0;

    if (team == TEAM_HORDE)
        spellid = outfitspells[2 + gender];
    else
        spellid = outfitspells[gender];


    target->castSpell(target, spellid, true);

    return true;
}

bool ScryingCrystal(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* player = pSpell->getPlayerCaster();
    LocationVector pos = player->GetPosition();
    if (player->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(pos.x, pos.y, pos.z, 300078))
    {
        player->AddQuestKill(9824, 0, 0);
        return false;
    }
    else if (player->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(pos.x, pos.y, pos.z, 300142))
    {
        player->AddQuestKill(9824, 1, 0);
        return false;
    }
    return true;
}

bool MinionsOfGurok(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* target = pSpell->GetUnitTarget();
    if (!pSpell->getPlayerCaster() || !target || !target->isCreature() || target->getEntry() != 17157)
        return true;

    static_cast<Creature*>(target)->Despawn(500, 360000);

    float SSX = target->GetPositionX();
    float SSY = target->GetPositionY();
    float SSZ = target->GetPositionZ();
    float SSO = target->GetOrientation();

    pSpell->getPlayerCaster()->GetMapMgr()->GetInterface()->SpawnCreature(18181, SSX + Util::getRandomUInt(8) - 4, SSY + Util::getRandomUInt(8) - 4, SSZ, SSO, true, false, 0, 0);
    pSpell->getPlayerCaster()->GetMapMgr()->GetInterface()->SpawnCreature(18181, SSX + Util::getRandomUInt(8) - 4, SSY + Util::getRandomUInt(8) - 4, SSZ, SSO, true, false, 0, 0);
    pSpell->getPlayerCaster()->GetMapMgr()->GetInterface()->SpawnCreature(18181, SSX + Util::getRandomUInt(8) - 4, SSY + Util::getRandomUInt(8) - 4, SSZ, SSO, true, false, 0, 0);

    return true;
}

bool PurifyBoarMeat(uint8_t /*effectIndex*/, Spell* pSpell)
{
    uint32_t bormeat = Util::getRandomUInt(2);
    switch (bormeat)
    {
        case 0:
            pSpell->getPlayerCaster()->castSpell(pSpell->getPlayerCaster(), 29277, true);
            break;
        case 1:
            pSpell->getPlayerCaster()->castSpell(pSpell->getPlayerCaster(), 29278, true);
            break;
    }

    return true;
}

bool WarpRiftGenerator(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster())
        return true;

    float SSX = pSpell->getPlayerCaster()->GetPositionX();
    float SSY = pSpell->getPlayerCaster()->GetPositionY();
    float SSZ = pSpell->getPlayerCaster()->GetPositionZ();
    float SSO = pSpell->getPlayerCaster()->GetOrientation();

    pSpell->getPlayerCaster()->GetMapMgr()->GetInterface()->SpawnCreature(16939, SSX, SSY, SSZ, SSO, true, false, 0, 0);

    return true;
}

bool OrbOfTheSindorei(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    Unit* target = pAura->getOwner();
    if (!target->isPlayer())
        return true;
    if (apply)
    {
        uint32_t spellid = 0;

        if (target->getGender() == 0)
            spellid = 46355;
        else
            spellid = 46356;

        target->castSpell(target, spellid, true);
    }

    return true;
}

bool ScalingMountDummyAura(uint32_t /*i*/, Aura* pAura, bool /*apply*/)
{
    // Remove dummy aura on application, dummy effect will occur directly after
    pAura->removeAura();
    return true;
}

bool BigBlizzardBear(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (Player* plr = pSpell->GetPlayerTarget())
    {
        uint32_t newspell = 58997;
        if (plr->_GetSkillLineCurrent(SKILL_RIDING, true) >= 150)
            plr->castSpell(plr, newspell, true);
    }

    return true;
}

bool WingedSteed(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (Player* plr = pSpell->GetPlayerTarget())
    {
        uint32_t newspell = 54726;
        if (plr->_GetSkillLineCurrent(SKILL_RIDING, true) == 300)
            newspell = 54727;
        plr->castSpell(plr, newspell, true);
    }

    return true;
}

bool HeadlessHorsemanMount(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (Player* plr = pSpell->GetPlayerTarget())
    {
        uint32_t newspell = 51621;
        auto pArea = plr->GetArea();
        if (pArea && (plr->_GetSkillLineCurrent(SKILL_RIDING, true) >= 225 && ((pArea->flags & 1024 && plr->GetMapId() != 571) ||
            (pArea->flags & 1024 && plr->GetMapId() == 571 && plr->HasSpell(54197)))))

        {
            if (plr->_GetSkillLineCurrent(SKILL_RIDING, true) == 300)
                newspell = 48023;
            else
                newspell = 51617;
        }
        else if (plr->_GetSkillLineCurrent(SKILL_RIDING, true) >= 150)
            newspell = 48024;
        plr->castSpell(plr, newspell, true);
    }

    return true;
}

bool MagicBroomMount(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (Player* plr = pSpell->GetPlayerTarget())
    {
        uint32_t newspell = 42680;
        auto pArea = plr->GetArea();
        if (pArea && (plr->_GetSkillLineCurrent(SKILL_RIDING, true) >= 225 &&
            ((pArea->flags & 1024 && plr->GetMapId() != 571) ||
            (pArea->flags & 1024 && plr->GetMapId() == 571 && plr->HasSpell(54197)))))
        {
            if (plr->_GetSkillLineCurrent(SKILL_RIDING, true) == 300)
                newspell = 42668;
            else
                newspell = 42667;
        }
        else if (plr->_GetSkillLineCurrent(SKILL_RIDING, true) >= 150)
            newspell = 42683;
        plr->castSpell(plr, newspell, true);
    }

    return true;
}

bool MagicRoosterMount(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (Player* plr = pSpell->GetPlayerTarget())
        plr->castSpell(plr, 66122, true);

    return true;
}

bool Invincible(uint8_t /*effectIndex*/, Spell* pSpell)
{
    // Apply the new aura in the 3rd effect call
    if (Player* plr = pSpell->GetPlayerTarget())
    {
        uint32_t newspell = 72281;
        auto pArea = plr->GetArea();
        if (pArea && (plr->_GetSkillLineCurrent(SKILL_RIDING, true) >= 225 && ((pArea->flags & 1024 && plr->GetMapId() != 571) ||
            (pArea->flags & 1024 && plr->GetMapId() == 571 && plr->HasSpell(54197)))))
        {
            if (plr->_GetSkillLineCurrent(SKILL_RIDING, true) == 300)
                newspell = 72284;
            else
                newspell = 72283;
        }
        else if (plr->_GetSkillLineCurrent(SKILL_RIDING, true) >= 150)
            newspell = 72282;

        plr->castSpell(plr, newspell, true);
    }

    return true;
}

bool Poultryizer(uint8_t /*effectIndex*/, Spell* s)
{
    Unit* unitTarget = s->GetUnitTarget();

    if (!unitTarget || !unitTarget->isAlive())
        return false;

    s->getUnitCaster()->castSpell(unitTarget->getGuid(), 30501, true);

    return true;
}

bool SixDemonBag(uint8_t /*effectIndex*/, Spell* s)
{
    Unit* unitTarget = s->GetUnitTarget();

    if (!unitTarget || !unitTarget->isAlive())
        return false;

    uint32_t ClearSpellId[6] = { 8401, 8408, 930, 118, 1680, 10159 };
    uint32_t randid = Util::getRandomUInt(5);
    uint32_t spelltocast = ClearSpellId[randid];

    s->getUnitCaster()->castSpell(unitTarget, spelltocast, true);

    return true;
}

bool ExtractGas(uint8_t /*effectIndex*/, Spell* s)
{
    bool check = false;
    uint32_t cloudtype = 0;
    Creature* creature = nullptr;

    if (!s->getPlayerCaster())
        return false;

    for (const auto& itr : s->getPlayerCaster()->getInRangeObjectsSet())
    {
        if (itr && itr->isCreature())
        {
            creature = static_cast<Creature*>(itr);
            cloudtype = creature->getEntry();

            if (cloudtype == 24222 || cloudtype == 17408 || cloudtype == 17407 || cloudtype == 17378)
            {
                if (s->getPlayerCaster()->GetDistance2dSq(itr) < 400)
                {
                    s->getPlayerCaster()->setTargetGuid(creature->getGuid());
                    check = true;
                    break;
                }
            }
        }
    }

    if (!check)
        return false;

    uint32_t item = 0;
    uint32_t count = 0;

    count = 3 + (Util::getRandomUInt(3));

    if (cloudtype == 24222)
        item = 22572; //-air
    if (cloudtype == 17408)
        item = 22576; //-mana
    if (cloudtype == 17407)
        item = 22577; //-shadow
    if (cloudtype == 17378)
        item = 22578; //-water

    s->getPlayerCaster()->getItemInterface()->AddItemById(item, count, 0);
    creature->Despawn(3500, creature->GetCreatureProperties()->RespawnTime);

    return true;
}

bool BrittleArmor(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getUnitCaster() == NULL)
        return false;

    for (uint8_t j = 0; j < 20; j++)
        s->getUnitCaster()->castSpell(s->getUnitCaster(), 24575, true);

    return true;
}

bool RequiresNoAmmo(uint8_t effectIndex, Aura* a, bool apply)
{
    auto aurEff = a->getAuraEffect(effectIndex);
    a->SpellAuraConsumeNoAmmo(&aurEff, apply);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//Nitro Boosts dummy effect handler
//
//Precondition(s)
//  Casted by Player.
//  Engineering skill of at least 400 points.
//
//Effect(s)
//  Casts another spell that increases run speed for 5 seconds
//
//////////////////////////////////////////////////////////////////////////////////////////
bool NitroBoosts(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getPlayerCaster() == NULL)
        return true;

    uint32_t engineeringskill = s->getPlayerCaster()->_GetSkillLineCurrent(SKILL_ENGINEERING);

    if (engineeringskill >= 400)
        s->getPlayerCaster()->castSpell(s->getPlayerCaster(), 54861, true);

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
//Gnomish Shrink Ray dummy effect handler
//
//Preconditions
//  Casted by Player
//
//Effect
//  Normally casts spell 13010 on the targeted unit. Shrinking it, and
//  making it weaker
//
//  On malfunction it either shrinks, or enlarges, the the caster,
//  the caster's party, all enemies, or enlarges the target
//
//
//////////////////////////////////////////////////////////////////////////////////////////
bool ShrinkRay(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getPlayerCaster() == NULL)
        return true;

    uint32_t spellids[] =
    {
        13004, // grow
        13010  // shrink
    };

    uint32_t chance = Util::getRandomUInt(5);
    bool malfunction = false;

    if (chance == 5)
        malfunction = true;

    if (!malfunction)
    {

        s->getPlayerCaster()->castSpell(s->GetUnitTarget(), spellids[1], true);

    }
    else
    {
        uint32_t spellindex = Util::getRandomUInt(1);
        uint32_t who = Util::getRandomUInt(3);

        switch (who)
        {

            case 0:  // us
            {
                s->getPlayerCaster()->castSpell(s->getPlayerCaster(), spellids[spellindex], true);
            }
            break;

            case 1:  // them
            {
                // if it's a malfunction it will only grow the target, since shrinking is normal
                s->getPlayerCaster()->castSpell(s->GetUnitTarget(), spellids[0], true);
            }
            break;

            case 2:  // our party
            {
                for (const auto& itr : s->getPlayerCaster()->getInRangePlayersSet())
                {
                    if (!itr)
                        continue;

                    Player* p = static_cast<Player*>(itr);
                    if ((p->GetPhase() & s->getPlayerCaster()->GetPhase()) == 0)
                        continue;

                    if (p->getGroup()->GetID() != s->getPlayerCaster()->getGroup()->GetID())
                        continue;

                    s->getPlayerCaster()->castSpell(p, spellids[spellindex], true);
                }
            }
            break;

            case 3:  // every attacking enemy
            {
                for (const auto& itr : s->getPlayerCaster()->getInRangeOppositeFactionSet())
                {
                    if(!itr)
                        continue;

                    Object* o = itr;

                    if ((o->GetPhase() & s->getPlayerCaster()->GetPhase()) == 0)
                        continue;

                    if (!o->isCreature())
                        continue;

                    Unit* u = static_cast<Unit*>(o);

                    if (u->getTargetGuid() != s->getPlayerCaster()->getGuid())
                        continue;

                    if (!isAttackable(s->getPlayerCaster(), u))
                        continue;

                    s->getPlayerCaster()->castSpell(u, spellids[spellindex], true);
                }
            }
            break;
        }
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
//Championing Tabards' dummy aura handler
//
//Precondition(s)
//  Casted by Player.
//
//Effect
//  We will get reputation for the faction the tabard represents,
//  in lvl80 dungeons, heroics and raids.
//
//////////////////////////////////////////////////////////////////////////////////////////
bool ChampioningTabards(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Player* p_caster = a->GetPlayerCaster();

    if (p_caster == NULL)
        return true;

    uint32_t Faction = a->getSpellInfo()->getEffectMiscValue(0);

    if (apply)
        p_caster->SetChampioningFaction(Faction);
    else
        p_caster->SetChampioningFaction(0);

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
//Spinning dummy effect handler
//
//Precondition(s)
//  Casted by Player
//
//Effect(s)
//  Spins the target around, setting a random orientation
//
//
//////////////////////////////////////////////////////////////////////////////////////////
bool Spinning(uint8_t /*effectIndex*/, Spell* s)
{
    Player* p_caster = s->getPlayerCaster();

    if (p_caster == NULL)
        return true;

    float neworientation = Util::getRandomFloat(M_PI_FLOAT * 2);

    float X = p_caster->GetPositionX();
    float Y = p_caster->GetPositionY();
    float Z = p_caster->GetPositionZ();
    uint32_t mapid = p_caster->GetMapId();
    uint32_t instanceid = p_caster->GetInstanceID();

    p_caster->SafeTeleport(mapid, instanceid, X, Y, Z, neworientation);

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
//Listening to Music periodically triggered dummy aura
//( SpellId 50493 )
//
//Precondition(s)
//  Casted by Player
//
//Effect(s)
//  Makes the player dance
//
//
//////////////////////////////////////////////////////////////////////////////////////////
bool ListeningToMusic(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* m_target = a->getOwner();
    Player* p_target = NULL;

    if (m_target->isPlayer())
        p_target = static_cast<Player*>(m_target);
    else
        return true;

    if (apply)
    {
        p_target->setEmoteState(EMOTE_STATE_DANCE);
    }
    else
    {
        p_target->setEmoteState(EMOTE_STATE_NONE);
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
//Periodic Drink Dummy Aura handler
//
//
//Precondition(s)
//  None
//
//Effect(s)
//  Restores x Mana Points every second
//
//
//////////////////////////////////////////////////////////////////////////////////////////
bool DrinkDummyAura(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    if (!apply)
        return true;

    float famount = 2.2f * (static_cast<float>(a->getSpellInfo()->getEffectBasePoints(1)) / 5.0f);
    int32_t amount = static_cast<int32_t>(std::round(famount));

    a->EventPeriodicDrink(amount);

    return true;
}

bool X53Mount(uint8_t /*effectIndex*/, Aura *a, bool apply)
{
    if (a->getOwner() == NULL)
        return true;

    if (!a->getOwner()->isPlayer())
        return true;

    if (apply)
    {
        uint32_t newspell = 0;
        if (Player* p = dynamic_cast<Player*>(a->getOwner()))
        {
            if (auto area = p->GetArea())
            {
                uint32_t skill = p->_GetSkillLineCurrent(SKILL_RIDING, true);

                if (skill >= 225 && (((area->flags & 1024) && p->GetMapId() != 571) ||
                    ((area->flags & 1024) && p->GetMapId() == 571 && p->HasSpell(54197))))
                {
                    if (skill == 300)
                    {
                        if (p->HasSpellWithAuraNameAndBasePoints(SPELL_AURA_ENABLE_FLIGHT2, 310))
                            newspell = 76154;
                        else
                            newspell = 75972;
                    }
                    else
                        newspell = 75957;
                }
                a->getOwner()->castSpell(a->getOwner(), newspell, true);
            }
        }
    }
    return true;
}

bool SchoolsOfArcaneMagicMastery(uint8_t /*effectIndex*/, Spell* s)
{
    if (auto player = s->GetPlayerTarget())
    {
        auto spell = player->getAreaId() == 4637 ? 59316 : 59314;
        player->castSpell(player, spell, true);
    }

    return true;
}

void SetupLegacyItemSpells_1(ScriptMgr* mgr)
{
    mgr->register_dummy_spell(59317, &SchoolsOfArcaneMagicMastery); // The Schools of Arcane Magic - Mastery
    mgr->register_dummy_spell(29403, &BreathOfFire);                // Fiery Festival Brew
    mgr->register_dummy_spell(23453, &GnomishTransporter);          // Gnomish Transporter
    mgr->register_dummy_spell(16589, &NoggenFoggerElixr);           // Noggenfogger
    mgr->register_dummy_spell(24930, &HallowsEndCandy);             // Hallows End Candy
    mgr->register_dummy_spell(8063, &DeviateFish);                  // Deviate Fish
    mgr->register_dummy_spell(8213, &CookedDeviateFish);            // Savory Deviate Delight
    mgr->register_dummy_spell(26074, &HolidayCheer);                // Holiday Cheer
    mgr->register_dummy_spell(13120, &NetOMatic);                   // Net-o-Matic
    uint32_t BanishExileIds[] = { 4130, 4131, 4132, 0 };
    mgr->register_dummy_spell(BanishExileIds, &BanishExile);        // Essence of the Exile Quest
    mgr->register_dummy_spell(19938, &ForemansBlackjack);           // Lazy Peons Quest
    mgr->register_dummy_spell(39105, &NetherWraithBeacon);          // Spellfire Tailor Quest
    mgr->register_dummy_spell(30458, &NighInvulnBelt);              // Nigh Invulnerability Belt

    mgr->register_dummy_spell(25860, &ReindeerTransformation);      // Fresh Holly & Preserved Holly

    mgr->register_script_effect(26275, &WinterWondervolt);          // PX-238 Winter Wondervolt Trap

    mgr->register_dummy_spell(32042, &ScryingCrystal);              // Violet Scrying Crystal (Quest)
    mgr->register_dummy_spell(32001, &MinionsOfGurok);              // Minions of gurok
    mgr->register_dummy_spell(29200, &PurifyBoarMeat);              // Purify Boar meat spell
    mgr->register_script_effect(35036, &WarpRiftGenerator);         // Summon a Warp Rift in Void Ridge
    mgr->register_dummy_aura(46354, &OrbOfTheSindorei);             // Orb of the Sin'dorei

    mgr->register_dummy_spell(65917, &MagicRoosterMount);           // Magic Rooster Mount
    mgr->register_dummy_spell(58983, &BigBlizzardBear);             // Big Blizzard Bear mount
    mgr->register_dummy_spell(54729, &WingedSteed);                 // DK flying mount
    mgr->register_dummy_spell(48025, &HeadlessHorsemanMount);       // Headless Horseman Mount
    mgr->register_dummy_spell(47977, &MagicBroomMount);             // Magic Broom Mount
    mgr->register_dummy_spell(72286, &Invincible);                  // Invincible

    mgr->register_dummy_spell(30507, &Poultryizer);
    mgr->register_dummy_spell(14537, &SixDemonBag);
    mgr->register_dummy_spell(30427, &ExtractGas);

    mgr->register_script_effect(24590, &BrittleArmor);

    mgr->register_dummy_aura(46699, &RequiresNoAmmo);

    mgr->register_dummy_spell(55004, &NitroBoosts);

    mgr->register_dummy_spell(13006, &ShrinkRay);

    uint32_t championingspellids[] =
    {
        57819,
        57820,
        57821,
        57822,
        0
    };
    mgr->register_dummy_aura(championingspellids, &ChampioningTabards);

    mgr->register_dummy_spell(64385, &Spinning);

    mgr->register_dummy_aura(50493, &ListeningToMusic);

    uint32_t DrinkDummySpellIDs[] =
    {
        430,
        431,
        432,
        1133,
        1135,
        1137,
        10250,
        22734,
        27089,
        34291,
        43182,
        43183,
        46755,
        57073,
        61830,
        72623,
        0
    };
    mgr->register_dummy_aura(DrinkDummySpellIDs, &DrinkDummyAura);
    mgr->register_dummy_aura(75973, &X53Mount);
}
