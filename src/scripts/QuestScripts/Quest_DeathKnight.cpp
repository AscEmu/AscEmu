/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Management/Gossip/GossipScript.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Movement/MovementManager.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Server/Script/QuestScript.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellScript.hpp"

enum 
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // 
    CN_INITIATE_1 = 29519,
    CN_INITIATE_2 = 29565,
    CN_INITIATE_3 = 29567,
    CN_INITIATE_4 = 29520,

    //////////////////////////////////////////////////////////////////////////////////////////
    // QuestID for Praparation for the Battle
    QUEST_PREPARATION = 12842,

    SPELL_RUNE_I = 53341, // Spell Rune of Cinderglacier
    SPELL_RUNE_II = 53343, // Spell Rune of Razorice
    SPELL_PREPERATION_FOR_BATTLE_CREDIT = 54586

    //
    //////////////////////////////////////////////////////////////////////////////////////////
};

class GossipScourgeGryphon : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        if (plr->hasQuestInQuestLog(12670) || plr->hasQuestFinished(12670))
        {
            if (uint32_t path = pObject->getEntry() == 29488 ? 1053 : 1054)
                plr->activateTaxiPathTo(path, pObject->ToCreature());
        }
    }
};

enum UnworthyInitiate
{
    SPELL_SOUL_PRISON_CHAINED       = 54613,
    SPELL_SOUL_PRISON_CHAIN         = 54612,
    SPELL_DK_INITIATE_VISUAL        = 51519,

    SPELL_ICY_TOUCH                 = 52372,
    SPELL_PLAGUE_STRIKE             = 52373,
    SPELL_BLOOD_STRIKE              = 52374,
    SPELL_DEATH_COIL                = 52375,

    SAY_EVENT_START1                = 3079,
    SAY_EVENT_START2                = 3080,
    SAY_EVENT_START3                = 3081,
    SAY_EVENT_START4                = 3082,
    SAY_EVENT_START5                = 3083,
    SAY_EVENT_START6                = 3084,
    SAY_EVENT_START7                = 3085,
    SAY_EVENT_START8                = 3086,
    SAY_EVENT_ATTACK1               = 3087,
    SAY_EVENT_ATTACK2               = 3088,
    SAY_EVENT_ATTACK3               = 3089,
    SAY_EVENT_ATTACK4               = 3090,
    SAY_EVENT_ATTACK5               = 3091,
    SAY_EVENT_ATTACK6               = 3092,
    SAY_EVENT_ATTACK7               = 3093,
    SAY_EVENT_ATTACK8               = 3094,
    SAY_EVENT_ATTACK9               = 3095,
    SET_PLAYER_DATA                 = 1,
    ACTION_START                    = 1,
};

enum UnworthyInitiatePhase
{
    PHASE_CHAINED                   = 0,
    PHASE_TO_EQUIP                  = 1,
    PHASE_EQUIPING                  = 2,
    PHASE_TO_ATTACK                 = 3,
    PHASE_ATTACKING                 = 4,
};

uint32_t acherus_soul_prison[12] =
{
    191577,
    191580,
    191581,
    191582,
    191583,
    191584,
    191585,
    191586,
    191587,
    191588,
    191589,
    191590
};

uint32_t acherus_unworthy_initiate[5] =
{
    29519,
    29520,
    29565,
    29566,
    29567
};

class UnworthyInitiateAI : CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UnworthyInitiateAI(c); }
    explicit UnworthyInitiateAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SPELL_ICY_TOUCH, 25.0f, TARGET_ATTACKING, 0, 1);
        addAISpell(SPELL_PLAGUE_STRIKE, 25.0f, TARGET_ATTACKING, 0, 3);
        addAISpell(SPELL_BLOOD_STRIKE, 25.0f, TARGET_ATTACKING, 0, 2);
        addAISpell(SPELL_DEATH_COIL, 25.0f, TARGET_ATTACKING, 0, 5);

        emoteVectorStart.clear();
        emoteVectorStart.push_back(SAY_EVENT_START1);
        emoteVectorStart.push_back(SAY_EVENT_START2);
        emoteVectorStart.push_back(SAY_EVENT_START3);
        emoteVectorStart.push_back(SAY_EVENT_START4);
        emoteVectorStart.push_back(SAY_EVENT_START5);
        emoteVectorStart.push_back(SAY_EVENT_START6);
        emoteVectorStart.push_back(SAY_EVENT_START7);
        emoteVectorStart.push_back(SAY_EVENT_START8);

        emoteVectorAttack.clear();
        emoteVectorAttack.push_back(SAY_EVENT_ATTACK1);
        emoteVectorAttack.push_back(SAY_EVENT_ATTACK2);
        emoteVectorAttack.push_back(SAY_EVENT_ATTACK3);
        emoteVectorAttack.push_back(SAY_EVENT_ATTACK4);
        emoteVectorAttack.push_back(SAY_EVENT_ATTACK5);
        emoteVectorAttack.push_back(SAY_EVENT_ATTACK6);
        emoteVectorAttack.push_back(SAY_EVENT_ATTACK7);
        emoteVectorAttack.push_back(SAY_EVENT_ATTACK8);
        emoteVectorAttack.push_back(SAY_EVENT_ATTACK9);
    }

    void SetCreatureData64(uint32_t type, uint64_t data)
    {
        switch (type)
        {
            case SET_PLAYER_DATA:
                playerGUID = data;
                break;
        }
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        Reset();
    }

    void OnDespawn() override
    {
        Reset();
    }

    void Reset()
    {
        anchorGUID = 0;
        playerGUID = 0;
        getCreature()->setFaction(7);
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->getAIInterface()->setImmuneToPC(true);
        getCreature()->getAIInterface()->setReactState(REACT_PASSIVE);
        setScriptPhase(PHASE_CHAINED);
        getCreature()->setStandState(STANDSTATE_KNEEL);
        getCreature()->setVirtualItemSlotId(MELEE, 0);
    }

    void DoAction(int32_t /*action*/) override
    {
        wait_timer = 5000;
        setScriptPhase(PHASE_TO_EQUIP);

        getCreature()->setStandState(STANDSTATE_STAND);
        getCreature()->removeAllAurasById(SPELL_SOUL_PRISON_CHAIN);

        float z;
        if (Creature* anchor = getCreature()->getWorldMapCreature(anchorGUID))
        {
            anchor->interruptSpell();
            anchor->getNearPoint(getCreature(), anchorX, anchorY, z, 1.0f, anchor->getAbsoluteAngle(getCreature()));
        }

        Player* player = getCreature()->getWorldMapPlayer(playerGUID);
        sendRandomDBChatMessage(emoteVectorStart, player);
    }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (iWaypointId == 1)
        {
            wait_timer = 5000;
            
            getCreature()->castSpell(getCreature(), SPELL_DK_INITIATE_VISUAL, true);

            if (Player* starter = getCreature()->getWorldMapPlayer(playerGUID))
                sendRandomDBChatMessage(emoteVectorAttack, starter);

            setScriptPhase(PHASE_TO_ATTACK);
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        switch (getScriptPhase())
        {
            case PHASE_CHAINED:
            {
                if (!anchorGUID)
                {
                    if (Creature* anchor = findNearestCreature(29521, 30.0f))
                    {
                        anchor->GetScript()->SetCreatureData64(0, getCreature()->getGuid());
                        anchor->castSpell(getCreature(), SPELL_SOUL_PRISON_CHAIN);
                        anchorGUID = anchor->getGuid();
                    }

                    GameObject* prison = nullptr;

                    for (uint8_t i = 0; i < 12; ++i)
                    {
                        if (GameObject* temp_prison = findNearestGameObject(acherus_soul_prison[i], 30))
                        {
                            if (getCreature()->isInDist(temp_prison, 99.0f))
                                prison = temp_prison;
                        }
                    }

                    if (prison)
                    {
                        prison->setState(GO_STATE_OPEN);
                        prison->removeFlags(GO_FLAG_NONSELECTABLE);
                    }
                }
            } break;
            case PHASE_TO_EQUIP:
            {
                if (wait_timer)
                {
                    if (wait_timer > time_passed)
                    {
                        wait_timer -= time_passed;
                    }
                    else
                    {
                        getCreature()->getMovementManager()->movePoint(1, anchorX, anchorY, getCreature()->GetPositionZ());
                        setScriptPhase(PHASE_EQUIPING);
                        wait_timer = 0;
                    }
                }
            } break;
            case PHASE_TO_ATTACK:
            {
                if (wait_timer)
                {
                    if (wait_timer > time_passed)
                    {
                        wait_timer -= time_passed;
                    }
                    else
                    {
                        getCreature()->setFaction(14);
                        getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
                        getCreature()->getAIInterface()->setImmuneToPC(false);
                        getCreature()->getAIInterface()->setReactState(REACT_AGGRESSIVE);
                        setScriptPhase(PHASE_ATTACKING);

                        if (Player* target = getCreature()->getWorldMapPlayer(playerGUID))
                        {
                            getCreature()->getAIInterface()->setCurrentTarget(target);
                            getCreature()->getAIInterface()->onHostileAction(target);
                        }

                        wait_timer = 0;
                    }
                }
            } break;
        }
    }

protected:
    std::vector<uint32_t> emoteVectorStart;
    std::vector<uint32_t> emoteVectorAttack;
    uint64_t anchorGUID = 0;
    uint64_t playerGUID = 0;
    uint32_t wait_timer = 0;
    float anchorX, anchorY = 0.0f;
};

class UnworthyInitiateAnchorAI : CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UnworthyInitiateAnchorAI(c); }
    explicit UnworthyInitiateAnchorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }

    void SetCreatureData64(uint32_t /*type*/, uint64_t data)
    {
        prisonerGUID = data;
    }

    uint64_t GetCreatureData64(uint32_t /*type*/) const 
    { 
        return prisonerGUID;
    }

protected:
    uint64_t prisonerGUID = 0;
};

class AcherusSoulPrison : GameObjectAIScript
{
public:
    explicit AcherusSoulPrison(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO)
    {
        return new AcherusSoulPrison(GO);
    }

    void OnActivate(Player* pPlayer) override
    {
        if (Creature* anchor = _gameobject->getWorldMap()->getInterface()->findNearestCreature(_gameobject, 29521, 15.0f))
        {
            if (anchor->GetScript())
            {
                if (uint64_t prisonerGUID = anchor->GetScript()->GetCreatureData64(1))
                {
                    if (Creature* prisoner = pPlayer->getWorldMapCreature(prisonerGUID))
                    {
                        if (prisoner->GetScript())
                        {
                            prisoner->GetScript()->SetCreatureData64(SET_PLAYER_DATA, pPlayer->getGuid());
                            prisoner->GetScript()->DoAction(ACTION_START);
                        }
                    }
                }
            }
        }
    }
};

class QuestInServiceOfLichKing : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        // Play first sound
        mTarget->sendPlaySoundPacket(14734);

        // Play second sound after 22.5 seconds
        sEventMgr.AddEvent(mTarget, &Player::sendPlaySoundPacket, (uint32_t)14735, EVENT_UNK, 22500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        // Play third sound after 48.5 seconds
        sEventMgr.AddEvent(mTarget, &Player::sendPlaySoundPacket, (uint32_t)14736, EVENT_UNK, 48500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
};

bool PreparationForBattleEffect(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pCaster = pSpell->getPlayerCaster();
    if (pCaster == nullptr)
        return false;

    // Apply spell if caster has quest and still heven't completed it yet
    if (pCaster->hasQuestInQuestLog(QUEST_PREPARATION) && !pCaster->hasQuestFinished(QUEST_PREPARATION))
        pCaster->castSpell(pCaster, SPELL_PREPERATION_FOR_BATTLE_CREDIT, true);

    return true;
}

class DK_INITIATE_VISUAL : public SpellScript
{
public:
    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t /*effIndex*/) override
    {
        if (!spell->getCaster())
            return SpellScriptCheckDummy::DUMMY_NOT_HANDLED;

        if (spell->getCaster()->ToCreature())
        {
            uint32_t spellId;
            switch (spell->getCaster()->ToCreature()->getDisplayId())
            {
                case 25369:
                    spellId = 51552;
                    break; // bloodelf female
                case 25373:
                    spellId = 51551;
                    break; // bloodelf male
                case 25363:
                    spellId = 51542;
                    break; // draenei female
                case 25357:
                    spellId = 51541;
                    break; // draenei male
                case 25361:
                    spellId = 51537;
                    break; // dwarf female
                case 25356:
                    spellId = 51538;
                    break; // dwarf male
                case 25372:
                    spellId = 51550;
                    break; // forsaken female
                case 25367:
                    spellId = 51549;
                    break; // forsaken male
                case 25362:
                    spellId = 51540;
                    break; // gnome female
                case 25359:
                    spellId = 51539;
                    break; // gnome male
                case 25355:
                    spellId = 51534;
                    break; // human female
                case 25354:
                    spellId = 51520;
                    break; // human male
                case 25360:
                    spellId = 51536;
                    break; // nightelf female
                case 25358:
                    spellId = 51535;
                    break; // nightelf male
                case 25368:
                    spellId = 51544;
                    break; // orc female
                case 25364:
                    spellId = 51543;
                    break; // orc male
                case 25371:
                    spellId = 51548;
                    break; // tauren female
                case 25366:
                    spellId = 51547;
                    break; // tauren male
                case 25370:
                    spellId = 51545;
                    break; // troll female
                case 25365:
                    spellId = 51546;
                    break; // troll male
                default:
                    return SpellScriptCheckDummy::DUMMY_NOT_HANDLED;
            }

            spell->getCaster()->ToCreature()->castSpell(nullptr, spellId, true);
            spell->getCaster()->ToCreature()->setVirtualItemSlotId(MELEE, 38707);
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Quest Death Comes From On High
class EyeofAcherusControl : public GameObjectAIScript
{
public:
    explicit EyeofAcherusControl(GameObject* gameobject) : GameObjectAIScript(gameobject) {}
    static GameObjectAIScript* Create(GameObject* gameobject_ai) { return new EyeofAcherusControl(gameobject_ai); }

    void OnActivate(Player* player) override
    {
        if (!player->hasQuestInQuestLog(12641))
            return;

        if (player->hasAurasWithId(51852))
            return;

        player->castSpell(player, 51888, true);

        _gameobject->setState(GO_STATE_CLOSED);
    }
};

void SetupDeathKnight(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(29488, new GossipScourgeGryphon());
    mgr->register_creature_gossip(29501, new GossipScourgeGryphon());

    mgr->register_dummy_spell(SPELL_RUNE_I, &PreparationForBattleEffect);
    mgr->register_dummy_spell(SPELL_RUNE_II, &PreparationForBattleEffect);
    mgr->register_quest_script(12593, new QuestInServiceOfLichKing);

    mgr->register_gameobject_script(acherus_soul_prison, &AcherusSoulPrison::Create);
    mgr->register_creature_script(acherus_unworthy_initiate, UnworthyInitiateAI::Create);
    mgr->register_creature_script(29521, UnworthyInitiateAnchorAI::Create);
    mgr->register_spell_script(SPELL_DK_INITIATE_VISUAL, new DK_INITIATE_VISUAL);

    mgr->register_gameobject_script(191609, &EyeofAcherusControl::Create);
}
