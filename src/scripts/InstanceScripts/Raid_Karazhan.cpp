/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
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

 // \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Objects/Faction.h"
#include <Spell/Definitions/PowerType.h>

// Partially by Plexor (I used a spell before, but changed to his method)
class Berthold : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* Plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 11224);
        menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(428), 1);     // What is this place?
        menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(429), 2);     // Where is Medivh?
        menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(430), 3);     // How do you navigate the tower?

        //Killing the Shade of Aran makes a teleport to medivh's available from Berthold the Doorman.
        Unit* soa = pObject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-11165.2f, -1912.13f, 232.009f, 16524);
        if (!soa || !soa->isAlive())
            menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(431), 4); // Please teleport me to the Guardian's Library.

        menu.Send(Plr);
    }

    void OnSelectOption(Object* /*pObject*/, Player* Plr, uint32 Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 4:
                Plr->SafeTeleport(Plr->GetMapId(), Plr->GetInstanceID(), -11165.123f, -1911.13f, 232.009f, 2.3255f);
                break;
        }
        Arcemu::Gossip::Menu::Complete(Plr);
    }

};

/////////////////////////////////////////////////////////////////////
//Attumen the Huntsman (and Midnight)
const uint32 CN_MIDNIGHT = 16151;
const uint32 CN_ATTUMEN = 15550;
const uint32 ATTUMEN_SHADOW_CLEAVE = 29832;
const uint32 ATTUMEN_BERSERKER_CHARGE = 22886;
const uint32 ATTUMEN_INTANGIBLE_PRESENCE = 29833;

class AttumenTheHuntsmanAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AttumenTheHuntsmanAI);
    AttumenTheHuntsmanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //All phase spells
        addAISpell(ATTUMEN_SHADOW_CLEAVE, 15.0f, TARGET_ATTACKING, 0, 6, false, true);
        addAISpell(ATTUMEN_INTANGIBLE_PRESENCE, 15.0f, TARGET_ATTACKING, 0, 12, false, true);

        //Phase 2 spells
        auto berserkCharge = addAISpell(ATTUMEN_BERSERKER_CHARGE, 15.0f, TARGET_RANDOM_SINGLE, 0, 6, false, true);
        berserkCharge->setAvailableForScriptPhase({ 2 });
        berserkCharge->setMinMaxDistance(15.0f, 40.0f);

        //Emotes
        addEmoteForEvent(Event_OnCombatStart, 8816);
        addEmoteForEvent(Event_OnCombatStart, 8817);
        addEmoteForEvent(Event_OnTargetDied, 8818);
        addEmoteForEvent(Event_OnTargetDied, 8819);
        addEmoteForEvent(Event_OnTargetDied, 8820);
        addEmoteForEvent(Event_OnDied, 8821);
        addEmoteForEvent(Event_OnTaunt, 8822);
        addEmoteForEvent(Event_OnTaunt, 8823);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        despawn(10000);
    }

    void AIUpdate() override
    {
        if (isScriptPhase(1))
        {
            if (getLinkedCreatureAIScript() && getLinkedCreatureAIScript()->isAlive() && _getHealthPercent() <= 25 && !_isCasting())
            {
                setScriptPhase(2);
                _setMeleeDisabled(false);
                _setCastDisabled(true);
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 9168, "Come Midnight, let's disperse this petty rabble!");
                CreatureAIScript* midnight = static_cast<CreatureAIScript*>(getLinkedCreatureAIScript());
                midnight->setScriptPhase(2);
                midnight->moveToUnit(getCreature());
                midnight->_setMeleeDisabled(false);
            }
        }
        
    }
};

class MidnightAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MidnightAI);
    MidnightAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        _setMeleeDisabled(true);
        _setCastDisabled(false);
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        if (getLinkedCreatureAIScript() && getLinkedCreatureAIScript()->isAlive())
        {
            static_cast<CreatureAIScript*>(getLinkedCreatureAIScript())->sendChatMessage(CHAT_MSG_MONSTER_YELL, 9173, "Well done Midnight!");
        }
    }

    void AIUpdate() override
    {
        if (isScriptPhase(1))
        {
            if (getLinkedCreatureAIScript() == nullptr && _getHealthPercent() <= 95 && !_isCasting())
            {
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Midnight calls for her master!");
                CreatureAIScript* attumen = spawnCreatureAndGetAIScript(CN_ATTUMEN, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
                if (attumen != nullptr)
                {
                    setLinkedCreatureAIScript(attumen);
                    attumen->setLinkedCreatureAIScript(this);
                }
            }
            else if (getLinkedCreatureAIScript() && getLinkedCreatureAIScript()->isAlive() && _getHealthPercent() <= 25 && !_isCasting())
            {
                setScriptPhase(2);
                CreatureAIScript* attumen = static_cast<CreatureAIScript*>(getLinkedCreatureAIScript());
                moveToUnit(attumen->getCreature());
                _setMeleeDisabled(false);
                attumen->setScriptPhase(2);
                attumen->_setMeleeDisabled(false);
                attumen->_setCastDisabled(true);
                attumen->sendChatMessage(CHAT_MSG_MONSTER_YELL, 9168, "Come Midnight, let's disperse this petty rabble!");
            }
        }
        else if (isScriptPhase(2))
        {
            if (getLinkedCreatureAIScript() && getLinkedCreatureAIScript()->isAlive())
            {
                CreatureAIScript* attumen = static_cast<CreatureAIScript*>(getLinkedCreatureAIScript());
                if (attumen && getRangeToObject(attumen->getCreature()) <= 15)
                {
                    attumen->_regenerateHealth();
                    attumen->_setDisplayId(16040);
                    attumen->_clearHateList();
                    attumen->_setMeleeDisabled(true);
                    attumen->_setCastDisabled(false);
                    despawn();
                    return;
                }
                else
                    moveToUnit(attumen->getCreature());
            }
        }
        
    }
};

/////////////////////////////////////////////////////////////////////
//Moroes
const uint32 CN_MOROES = 15687;
const uint32 MOROES_GOUGE = 28456;
const uint32 MOROES_VANISH = 29448;
const uint32 MOROES_BLIND = 34654;
const uint32 MOROES_ENRAGE = 37023;
const uint32 MOROES_GARROTE = 37066;

class MoroesAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MoroesAI);
    MoroesAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //Initialize timers
        mVanishTimer = 0;
        mGarroteTimer = 0;

        //Phase 1 spells
        auto gouge = addAISpell(MOROES_GOUGE, 20.0f, TARGET_ATTACKING, 0, 10);
        gouge->setAvailableForScriptPhase({ 1 });

        auto blind = addAISpell(MOROES_BLIND, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        blind->setAvailableForScriptPhase({ 1 });

        mVanish = addAISpell(MOROES_VANISH, 0.0f, TARGET_SELF, 12, 0);
        mVanish->addEmote("Now, where was I? Oh yes...", CHAT_MSG_MONSTER_YELL, 9215);
        mVanish->addEmote("You rang?", CHAT_MSG_MONSTER_YELL, 9316);

        mEnrage = addAISpell(MOROES_ENRAGE, 40.0f, TARGET_SELF, 0, 60);
        mEnrage->setMinMaxPercentHp(0, 30);

        //Phase 2 spells
        mGarrote = addAISpell(MOROES_GARROTE, 0.0f, TARGET_RANDOM_SINGLE);

        //Emotes
        addEmoteForEvent(Event_OnCombatStart, 8824);
        addEmoteForEvent(Event_OnDied, 8825);
        addEmoteForEvent(Event_OnTargetDied, 8826);
        addEmoteForEvent(Event_OnTargetDied, 8827);
        addEmoteForEvent(Event_OnTargetDied, 8828);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mVanishTimer = _addTimer(35000);    //First vanish after 35sec
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        _removeAuraOnPlayers(MOROES_GARROTE);
    }

    void AIUpdate() override
    {
        if (isScriptPhase(1))
        {
            if (_isTimerFinished(mVanishTimer) && !_isCasting())
            {
                setScriptPhase(2);
            }
        }
        else if (isScriptPhase(2))
        {
            if (_isTimerFinished(mGarroteTimer) && !_isCasting())
            {
                setScriptPhase(1);
            }
        }
    }

    void OnScriptPhaseChange(uint32_t phaseId) override
    {
        switch (phaseId)
        {
            case 1:
            {
                if (_isTimerFinished(mGarroteTimer))
                {
                    _castAISpell(mGarrote);
                    _removeAura(MOROES_VANISH);
                    _removeTimer(mGarroteTimer);
                }

            } break;
            case 2:
                _castAISpell(mVanish);
                mGarroteTimer = _addTimer(12000);
                _resetTimer(mVanishTimer, 35000);
                break;
            default:
                break;
        }
    }

    CreatureAISpells* mVanish;
    CreatureAISpells* mGarrote;
    CreatureAISpells* mEnrage;
    uint32 mVanishTimer;
    uint32 mGarroteTimer;
};

/////////////////////////////////////////////////////////////////////
//Maiden of Virtue
const uint32 CN_MAIDENOFVIRTUE = 16457;
const uint32 MAIDENOFVIRTUE_REPENTANCE = 29511;
const uint32 MAIDENOFVIRTUE_HOLY_GROUND = 29512;
const uint32 MAIDENOFVIRTUE_HOLY_FIRE = 29522;
const uint32 MAIDENOFVIRTUE_HOLY_WRATH = 32445;

class MaidenOfVirtueAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MaidenOfVirtueAI);
    MaidenOfVirtueAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //Spells
        addAISpell(MAIDENOFVIRTUE_HOLY_GROUND, 100.0f, TARGET_SELF, 0, 3);
        addAISpell(MAIDENOFVIRTUE_HOLY_FIRE, 25.0f, TARGET_RANDOM_SINGLE, 1, 15);
        addAISpell(MAIDENOFVIRTUE_HOLY_WRATH, 25.0f, TARGET_RANDOM_SINGLE, 0, 20, 0);
        mRepentance = addAISpell(MAIDENOFVIRTUE_REPENTANCE,25.0f,  TARGET_SELF, 1, 30);
        mRepentance->addEmote("Cast out your corrupt thoughts.", CHAT_MSG_MONSTER_YELL, 9313);
        mRepentance->addEmote("Your impurity must be cleansed.", CHAT_MSG_MONSTER_YELL, 9208);

        //Emotes
        addEmoteForEvent(Event_OnCombatStart, 8829);
        addEmoteForEvent(Event_OnTargetDied, 8830);
        addEmoteForEvent(Event_OnTargetDied, 8831);
        addEmoteForEvent(Event_OnTargetDied, 8832);
        addEmoteForEvent(Event_OnDied, 8833);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mRepentance->setAttackStopTimer(0);    //No repentance at the beginning of the fight
    }

    CreatureAISpells* mRepentance;
};

/////////////////////////////////////////////////////////////////////
//The Big Bad Wolf
const uint32 CN_BIGBADWOLF = 17521;

const uint32 TERRIFYING_HOWL = 30752;
const uint32 WIDE_SWIPE = 6749;
// Combines display visual + buff
const uint32 REDRIDINGHOOD_DEBUFF = 30768;
const uint32 PBS_TAUNT = 30755;    // Picnic Basket Smell (taunt)

class BigBadWolfAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BigBadWolfAI);
    BigBadWolfAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_threattimer = 0;
        ThreatAdd = false;
        RTarget = NULL;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(1993);     // The better to own you with!

        ThreatAdd = false;
        m_threattimer = 20;

        RegisterAIUpdateEvent(1000);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        //\todo not in db
        getCreature()->PlaySoundToSet(9275);
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Aarrhhh.");

        GameObject* DoorLeftO = getNearestGameObject(-10917.1445f, -1774.05f, 90.478f, 184279);
        GameObject* DoorRightO = getNearestGameObject(-10872.195f, -1779.42f, 90.45f, 184278);

        if (DoorLeftO)
            DoorLeftO->SetState(GO_STATE_OPEN);

        if (DoorRightO)
            DoorRightO->SetState(GO_STATE_OPEN);
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(1994);     // Mmmm... delicious.
    }

protected:

    int m_threattimer;
    bool ThreatAdd;
    Unit* RTarget;
};


const uint32 THEBIGBADWOLF_TERRIFYING_HOWL = 30752;
const uint32 MORPH_LITTLE_RED_RIDING_HOOD = 30768;
const uint32 DEBUFF_LITTLE_RED_RIDING_HOOD = 30756;

class THEBIGBADWOLFAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(THEBIGBADWOLFAI);
    THEBIGBADWOLFAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(1993);     // The better to own you with!
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        GameObject* DoorLeft = getNearestGameObject(-10917.1445f, -1774.05f, 90.478f, 184279);
        GameObject* DoorRight = getNearestGameObject(-10872.195f, -1779.42f, 90.45f, 184278);
        GameObject* Curtain = getNearestGameObject(-10894.17f, -1774.218f, 90.477f, 183932);

        if (DoorLeft)
            DoorLeft->SetState(GO_STATE_CLOSED);

        if (DoorRight)
            DoorRight->SetState(GO_STATE_OPEN);

        if (Curtain)
            Curtain->SetState(GO_STATE_CLOSED);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        GameObject* DoorLeft = getNearestGameObject(-10917.1445f, -1774.05f, 90.478f, 184279);
        GameObject* DoorRight = getNearestGameObject(-10872.195f, -1779.42f, 90.45f, 184278);
        GameObject* Curtain = getNearestGameObject(-10894.17f, -1774.218f, 90.477f, 183932);

        if (DoorLeft)
            DoorLeft->SetState(GO_STATE_OPEN);

        if (DoorRight)
            DoorRight->SetState(GO_STATE_OPEN);

        // Make sure the curtain stays up
        if (Curtain)
            Curtain->SetState(GO_STATE_OPEN);

        getCreature()->PlaySoundToSet(9275);
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "AArrhhh.");
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        getCreature()->PlaySoundToSet(9277);
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Mmmm... delicious.");
    }
};


uint32 WayStartBBW[1000000];
class BarnesGS : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* Plr) override
    {
        if (WayStartBBW[pObject->GetInstanceID()] == 5)
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 8975, 0);
            menu.Send(Plr);
        }
        else
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 8970, 0);
            menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(432), 1);     // I'm not an actor.
            menu.Send(Plr);
        }
    }

    void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 8971, 0);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(433), 2);     // Ok, I'll give it a try, then.
                menu.Send(Plr);
            }
            break;
            case 2:
            {
                Creature* pCreature = static_cast<Creature*>(pObject);
                pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Splendid. I'm going to get the audience ready. Break a leg!");
                pCreature->CastSpell(pCreature, 32616, false);
                pCreature->GetAIInterface()->StopMovement(0);
                pCreature->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                pCreature->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
                pCreature->GetAIInterface()->setWayPointToMove(0);
                pCreature->setUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
                pCreature->PlaySoundToSet(9357);
                WayStartBBW[pCreature->GetInstanceID()] = 2;
                Arcemu::Gossip::Menu::Complete(Plr);
            }
            break;
        }
    }
};

class GrandMother : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* Plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7245, 0);         // Don't get too close, $N. I'm liable to fumble and bash your brains open with the face of my hammer.
        menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(434), 1);         // What phat lewts you have Grandmother!
        menu.Send(Plr);
    }

    void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                static_cast<Creature*>(pObject)->Despawn(100, 0);
                Creature* pop = pObject->GetMapMgr()->GetInterface()->SpawnCreature(17521, pObject->GetPositionX(), pObject->GetPositionY(), pObject->GetPositionZ(), 0, true, true, 0, 0);
                if (pop)
                    pop->GetAIInterface()->AttackReaction(Plr, 1, 0);
                break;
            }
        }
    }
};

static Movement::Location Barnes[] =
{
    { },
    { -10873.91f, -1780.177f, 90.50f, 3.3f },
    { -10895.299805f, -1783.349976f, 90.50f, 4.5f },
    { -10873.91f, -1780.177f, 90.50f, 3.3f },
    { -10868.4f, -1781.63f, 90.50f, 1.24f }
};

class BarnesAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BarnesAI);
    BarnesAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        AddWaypoint(CreateWaypoint(1, 0, Movement::WP_MOVE_TYPE_WALK, Barnes[1]));
        AddWaypoint(CreateWaypoint(2, 43000, Movement::WP_MOVE_TYPE_WALK, Barnes[2]));
        AddWaypoint(CreateWaypoint(3, 0, Movement::WP_MOVE_TYPE_WALK, Barnes[3]));
        AddWaypoint(CreateWaypoint(4, 0, Movement::WP_MOVE_TYPE_WALK, Barnes[4]));

        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

        WayStartBBW[getCreature()->GetInstanceID()] = 1;

        eventRand = 0;
        switch (Util::getRandomUInt(2))
        {
            case 0:
                eventRand = 0;
                break;
            case 1:
                eventRand = 1;
                break;
            case 2:
                eventRand = 2;
                break;
        }
    }

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        switch (iWaypointId)
        {
            case 0:
            {
                getCreature()->GetAIInterface()->setWayPointToMove(1);
                WayStartBBW[getCreature()->GetInstanceID()] = 3;
            } break;
            case 2:
            {
                cleanStage();
                WayStartBBW[getCreature()->GetInstanceID()] = 4;
                Creature* spotlight = spawnCreature(19525, Barnes[2].x, Barnes[2].y, Barnes[2].z, Barnes[2].o);
                if (spotlight)
                    spotlight->Despawn(43000, 0);

                getCreature()->SetFacing(4.5f);
                switch (eventRand)
                {
                    case 0:
                        BarnesSpeakRed();
                        break;
                    case 1:
                        BarnesSpeakRJ();
                        break;
                    case 2:
                        BarnesSpeakWOZ();
                        break;
                }
            } break;
            case 3:
            {
                switch (eventRand)
                {
                    case 0:
                        EventRed();
                        break;
                    case 1:
                        EventRJ();
                        break;
                    case 2:
                        EventWOZ();
                        break;
                }
                getCreature()->setUInt32Value(UNIT_FIELD_FLAGS, 1);
                WayStartBBW[getCreature()->GetInstanceID()] = 5;
            } break;
            default:
                break;
        }
    }

    void cleanStage()
    {
        GameObject* DoorLeft = getNearestGameObject(-10917.1445f, -1774.05f, 90.478f, 184279);
        GameObject* DoorRight = getNearestGameObject(-10872.195f, -1779.42f, 90.45f, 184278);
        GameObject* Curtain = getNearestGameObject(-10894.17f, -1774.218f, 90.477f, 183932);

        if (DoorLeft)
            DoorLeft->SetState(GO_STATE_CLOSED);

        if (DoorRight)
            DoorRight->SetState(GO_STATE_OPEN);

        if (Curtain)
            Curtain->SetState(GO_STATE_CLOSED);

        Creature* Julianne = getNearestCreature(-10883.0f, -1751.81f, 90.4765f, 17534);
        Creature* Romulo = getNearestCreature(-10883.0f, -1751.81f, 90.4765f, 17533);
        Creature* BigBadWolf = getNearestCreature(-10883.0f, -1751.81f, 90.4765f, 17521);
        Creature* Grandma = getNearestCreature(-10883.0f, -1751.81f, 90.4765f, 17603);

        Creature* Dorothee = getNearestCreature(-10897.650f, -1755.8311f, 90.476f, 17535); //Dorothee
        Creature* Strawman = getNearestCreature(-10904.088f, -1754.8988f, 90.476f, 17543); //Strawman
        Creature* Roar = getNearestCreature(-10891.115f, -1756.4898f, 90.476f, 17546);//Roar
        Creature* Tinman = getNearestCreature(-10884.501f, -1757.3249f, 90.476f, 17547); //Tinman

        GameObject* House = getNearestGameObject(-10883.0f, -1751.81f, 90.4765f, 183493);
        GameObject* Tree = getNearestGameObject(-10877.7f, -1763.18f, 90.4771f, 183492);
        GameObject* Tree2 = getNearestGameObject(-10906.7f, -1750.01f, 90.4765f, 183492);
        GameObject* Tree3 = getNearestGameObject(-10909.5f, -1761.79f, 90.4773f, 183492);
        //GameObject* BackDrop = getNearestGameObject(-10890.9f, -1744.06f, 90.4765f, 183491);

        if (Julianne)
            Julianne->Despawn(0, 0);
        if (Romulo)
            Romulo->Despawn(0, 0);

        if (BigBadWolf)
            BigBadWolf->Despawn(0, 0);
        if (Grandma)
            Grandma->Despawn(0, 0);

        if (Dorothee)
            Dorothee->Despawn(0, 0);
        if (Strawman)
            Strawman->Despawn(0, 0);
        if (Roar)
            Roar->Despawn(0, 0);
        if (Tinman)
            Tinman->Despawn(0, 0);

        if (House)
            House->Despawn(0, 0);
        if (Tree)
            Tree->Despawn(0, 0);
        if (Tree2)
            Tree2->Despawn(0, 0);
        if (Tree3)
            Tree3->Despawn(0, 0);
        //if (BackDrop)
        //    BackDrop->GetMapMgr()->GetInterface()->DeleteGameObject(BackDrop);
    }

    void BarnesSpeakWOZ()
    {
        // Start text
        sendDBChatMessage(2011);     // Good evening, ladies and gentleman. Welcome to this evening's presentation!
        // Timed text 1
        getCreature()->SendTimedScriptTextChatMessage(2008, 7000);  // Tonight, we plumb the depths of the human soul as we join a lost, lonely girl trying desperately, with the help of her loyal companions, to find her way home.
        // Timed text 2
        getCreature()->SendTimedScriptTextChatMessage(2009, 23000); // But she is pursued by a wicked, malevolent crone!", 23000);
        // Timed text 3
        getCreature()->SendTimedScriptTextChatMessage(2010, 32000); // Will she survive? Will she prevail? Only time will tell. And now: On with the show!", 32000);
        // Applause
        sEventMgr.AddEvent(static_cast<Object*>(getCreature()), &Object::PlaySoundToSet, (uint32)9332, EVENT_UNK, 41000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }

    void EventWOZ()
    {
        GameObject* DoorLeft = getNearestGameObject(-10917.1445f, -1774.05f, 90.478f, 184279);
        GameObject* DoorRight = getNearestGameObject(-10872.195f, -1779.42f, 90.45f, 184278);
        GameObject* Curtain = getNearestGameObject(-10894.17f, -1774.218f, 90.477f, 183932);

        if (DoorLeft)
            DoorLeft->SetState(GO_STATE_CLOSED);

        if (DoorRight)
            DoorRight->SetState(GO_STATE_CLOSED);

        if (Curtain)
            Curtain->SetState(GO_STATE_OPEN);

        getCreature()->SetDisplayId(16616);

        spawnCreature(17535, -10897.650f, -1755.8311f, 90.476f, 4.61f); //Dorothee
        spawnCreature(17543, -10904.088f, -1754.8988f, 90.476f, 4.61f); //Strawman
        spawnCreature(17546, -10891.115f, -1756.4898f, 90.476f, 4.61f);//Roar
        spawnCreature(17547, -10884.501f, -1757.3249f, 90.476f, 4.61f); //Tinman
    }

    void BarnesSpeakRJ()
    {
        // Start text
        sendDBChatMessage(2011);                 // Good evening, ladies and gentleman. Welcome to this evening's presentation!
        // Timed text 1
        getCreature()->SendTimedScriptTextChatMessage(2016, 6000);      // Tonight we explore a tale of forbidden love!
        // Timed text 2
        getCreature()->SendTimedScriptTextChatMessage(2016, 19300);     // But beware, for not all love stories end happily. As you may find out, sometimes love pricks like a thorn.
        // Timed text 3
        getCreature()->SendTimedScriptTextChatMessage(2018, 32000);     // But don't take it from me. See for yourself what tragedy lies ahead when the paths of star-crossed lovers meet. And now: On with the show!
        // Applause
        sEventMgr.AddEvent(static_cast<Object*>(getCreature()), &Object::PlaySoundToSet, (uint32)9332, EVENT_UNK, 41000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }

    void EventRJ()
    {
        GameObject* DoorLeft = getNearestGameObject(-10917.1445f, -1774.05f, 90.478f, 184279);
        GameObject* DoorRight = getNearestGameObject(-10872.195f, -1779.42f, 90.45f, 184278);
        GameObject* Curtain = getNearestGameObject(-10894.17f, -1774.218f, 90.477f, 183932);

        if (DoorLeft)
            DoorLeft->SetState(GO_STATE_CLOSED);

        if (DoorRight)
            DoorRight->SetState(GO_STATE_CLOSED);

        if (Curtain)
            Curtain->SetState(GO_STATE_OPEN);

        getCreature()->SetDisplayId(16616);
        spawnCreature(17534, -10891.582f, -1755.5177f, 90.476f, 4.61f); //Spawn Julianne
    }

    void BarnesSpeakRed()
    {
        // Start text
        sendDBChatMessage(2011);                 // Good evening, ladies and gentleman. Welcome to this evening's presentation!
        // Timed text 1
        getCreature()->SendTimedScriptTextChatMessage(2012, 7000);      // Tonight things are not what they seems for tonight your eyes may not be trusted.
        // Timed text 2
        getCreature()->SendTimedScriptTextChatMessage(2013, 17000);     // Take for instance this quiet elderly woman waiting for a visit from her granddaughter. Surely there is nothing to fear from this sweet gray-haired old lady.
        // Timed text 3
        getCreature()->SendTimedScriptTextChatMessage(2014, 32000);     // But don't let me pull the wool over your eyes. See for yourself what lies beneath those covers. And now: On with the show!
        // Applause
        sEventMgr.AddEvent(static_cast<Object*>(getCreature()), &Object::PlaySoundToSet, (uint32)9332, EVENT_UNK, 41000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }

    void EventRed()
    {
        GameObject* DoorLeft = getNearestGameObject(-10917.1445f, -1774.05f, 90.478f, 184279);
        GameObject* DoorRight = getNearestGameObject(-10872.195f, -1779.42f, 90.45f, 184278);
        GameObject* Curtain = getNearestGameObject(-10894.17f, -1774.218f, 90.477f, 183932);

        if (DoorLeft)
            DoorLeft->SetState(GO_STATE_CLOSED);

        if (DoorRight)
            DoorRight->SetState(GO_STATE_CLOSED);

        if (Curtain)
            Curtain->SetState(GO_STATE_OPEN);

        getCreature()->SetDisplayId(16616);
        spawnCreature(17603, -10891.582f, -1755.5177f, 90.476f, 4.61f);

    }

protected:
    int eventRand;
};


class StageLight : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(StageLight);
    StageLight(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->setUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        _setMeleeDisabled(true);
        getCreature()->GetAIInterface()->m_canMove = false;
        getCreature()->m_noRespawn = true;
        getCreature()->CastSpell(getCreature(), 34126, true);
    }

};

// The Curator + Astral Flare
const uint32 CN_CURATOR = 15691;
const uint32 CN_ASTRALFLARE = 17096;

const uint32 HATEFUL_BOLT = 30383;
const uint32 EVOCATION = 30254;
const uint32 C_ENRAGE = 28747;
const uint32 BERSERK = 26662;

class CuratorAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CuratorAI);
    CuratorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        evocation = false;
        enrage = false;
        berserk = false;
        Timer = 0;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(2062);     // The Menagerie is for guests only.

        evocation = false;
        enrage = false;
        uint32 t = (uint32)time(NULL);
        Timer = t + 10;

        RegisterAIUpdateEvent(1000);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        sendDBChatMessage(2069);     // This Curator is no longer op... er... ation... al.
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        if (getCreature()->GetHealthPct() > 0)
        {
            switch (Util::getRandomUInt(1))
            {
                case 0:
                    sendDBChatMessage(2067);     // Do not touch the displays.
                    break;
                case 1:
                    sendDBChatMessage(2068);     // You are not a guest.
                    break;
            }
        }
    }

    void AstralSpawn()
    {
        std::vector<Player*> Target_List;
        for (std::set< Object* >::iterator itr = getCreature()->GetInRangePlayerSetBegin();
            itr != getCreature()->GetInRangePlayerSetEnd(); ++itr)
        {
            Player* RandomTarget = NULL;
            RandomTarget = static_cast<Player*>(*itr);
            if (RandomTarget && RandomTarget->isAlive() && isHostile(getCreature(), (*itr)))
                Target_List.push_back(RandomTarget);
            RandomTarget = NULL;
        }
        if (!Target_List.size())
            return;

        auto random_index = Util::getRandomUInt(0, uint32(Target_List.size() - 1));
        Unit* random_target = Target_List[random_index];

        if (random_target == nullptr)
            return;

        switch (Util::getRandomUInt(1))
        {
            case 0:
                sendDBChatMessage(2063);     // Gallery rules will be strictly enforced.
                break;
            case 1:
                sendDBChatMessage(2064);     // This curator is equipped for gallery protection.
                break;
        }

        getCreature()->setUInt32Value(UNIT_FIELD_POWER1, getCreature()->GetPower(POWER_TYPE_MANA) - (getCreature()->GetMaxPower(POWER_TYPE_MANA) / 10));
        float dX = getCreature()->GetPositionX();
        float dY = getCreature()->GetPositionY();
        Creature* AstralFlare = NULL;
        switch (Util::getRandomUInt(3))
        {
            case 0:
            {
                AstralFlare = spawnCreature(CN_ASTRALFLARE, dX + 3, dY + 3, getCreature()->GetPositionZ(), 0);
                AstralFlare->GetAIInterface()->AttackReaction(random_target, 1, 0);
                AstralFlare = NULL;
            }
            break;
            case 1:
            {
                AstralFlare = spawnCreature(CN_ASTRALFLARE, dX + 3, dY - 3, getCreature()->GetPositionZ(), 0);
                AstralFlare->GetAIInterface()->AttackReaction(random_target, 1, 0);
                AstralFlare = NULL;
            }
            break;
            case 2:
            {
                AstralFlare = spawnCreature(CN_ASTRALFLARE, dX - 3, dY - 3, getCreature()->GetPositionZ(), 0);
                AstralFlare->GetAIInterface()->AttackReaction(random_target, 1, 0);
                AstralFlare = NULL;
            }
            break;
            case 3:
            {
                AstralFlare = spawnCreature(CN_ASTRALFLARE, dX - 3, dY + 3, getCreature()->GetPositionZ(), 0);
                AstralFlare->GetAIInterface()->AttackReaction(random_target, 1, 0);
                AstralFlare = NULL;
            }
            break;
        }
        Target_List.clear();
    }

protected:
    bool evocation;
    bool enrage;
    bool berserk;
    uint32 Timer;
};

// Astral Flare
const uint32 ASTRAL_FLARE_PASSIVE = 30234;
const uint32 ASTRAL_FLARE_VISUAL = 30237;
const uint32 ARCING_SEAR = 30235;

class AstralFlareAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AstralFlareAI);
    AstralFlareAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ASTRAL_FLARE_PASSIVE, 100.0f, TARGET_SELF, 0, 3);
        addAISpell(ASTRAL_FLARE_VISUAL, 100.0f, TARGET_SELF, 0, 6);
        addAISpell(30235, 20.0f, TARGET_ATTACKING);
    }
};

// Shade of Aran
const uint32 SHADEOFARAN = 16524;

const uint32 FROSTBOLT = 29954;
const uint32 FIREBALL = 29953;
const uint32 ARCMISSLE = 29955;
const uint32 CHAINSOFICE = 29991;
const uint32 DRAGONSBREATH = 29964;
const uint32 AOE_COUNTERSPELL = 29961;

const uint32 BLIZZARD = 29952;
const uint32 FLAME_WREATH = 29946; // detonate -> 30004

const uint32 BLINK_CENTER = 29967;
const uint32 MAGNETIC_PULL = 38540;
const uint32 MASSSLOW = 30035;
const uint32 AEXPLOSION = 29973;

const uint32 MASS_POLYMORPH = 29963;
const uint32 CONJURE = 29975;
const uint32 DRINK = 30024;
const uint32 AOE_PYROBLAST = 29978;

const uint32 SUMMON_ELEMENTAL_1 = 37051;
const uint32 SUMMON_ELEMENTAL_2 = 37052;
const uint32 SUMMON_ELEMENTAL_3 = 37053;
const uint32 WATERELE = 17167;
const uint32 WATERBOLT = 31012;

const uint32 SHADOWOFARAN = 18254;
const uint32 SHADOWPYRO = 29978;

enum SUPERSPELL
{
    SUPER_FLAME = 0,
    SUPER_BLIZZARD = 1,
    SUPER_AOE = 2,
};

class ShadeofAranAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadeofAranAI);
    ShadeofAranAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        /*spells[0].info = sSpellCustomizations.GetSpellInfo(FROSTBOLT);
        spells[0].targettype = TARGET_RANDOM_SINGLE;
        spells[0].instant = false;
        spells[0].cooldown = 5;
        spells[0].casttime = 5;
        spells[0].attackstoptimer = 2000;

        spells[1].info = sSpellCustomizations.GetSpellInfo(FIREBALL);
        spells[1].targettype = TARGET_RANDOM_SINGLE;
        spells[1].instant = false;
        spells[1].cooldown = 5;
        spells[1].casttime = 5;
        spells[1].attackstoptimer = 2000;

        spells[2].info = sSpellCustomizations.GetSpellInfo(ARCMISSLE);
        spells[2].targettype = TARGET_RANDOM_SINGLE;
        spells[2].instant = false;
        spells[2].cooldown = 10;
        spells[2].casttime = 10;
        spells[2].attackstoptimer = 6000;

        spells[3].info = sSpellCustomizations.GetSpellInfo(CHAINSOFICE);
        spells[3].targettype = TARGET_RANDOM_SINGLE;
        spells[3].instant = true;
        spells[3].cooldown = Util::getRandomUInt(5) + 14;
        spells[3].casttime = Util::getRandomUInt(5) + 14;
        spells[3].attackstoptimer = 1000;

        spells[4].info = sSpellCustomizations.GetSpellInfo(DRAGONSBREATH);
        spells[4].targettype = TARGET_RANDOM_SINGLE;
        spells[4].instant = true;
        spells[4].cooldown = Util::getRandomUInt(5) + 16;
        spells[4].casttime = Util::getRandomUInt(5) + 16;
        spells[4].attackstoptimer = 1000;
        spells[4].maxdist2cast = 15;

        spells[5].info = sSpellCustomizations.GetSpellInfo(AOE_COUNTERSPELL);
        spells[5].targettype = TARGET_SELF;
        spells[5].instant = true;
        spells[5].cooldown = 13;
        spells[5].casttime = 13;
        spells[5].attackstoptimer = 1000;*/

        info_flame_wreath = sSpellCustomizations.GetSpellInfo(FLAME_WREATH);
        info_blink_center = sSpellCustomizations.GetSpellInfo(BLINK_CENTER);
        info_massslow = sSpellCustomizations.GetSpellInfo(MASSSLOW);
        info_magnetic_pull = sSpellCustomizations.GetSpellInfo(MAGNETIC_PULL);
        info_aexplosion = sSpellCustomizations.GetSpellInfo(AEXPLOSION);
        info_blizzard = sSpellCustomizations.GetSpellInfo(BLIZZARD);
        info_summon_elemental_1 = sSpellCustomizations.GetSpellInfo(SUMMON_ELEMENTAL_1);
        info_summon_elemental_2 = sSpellCustomizations.GetSpellInfo(SUMMON_ELEMENTAL_2);
        info_summon_elemental_3 = sSpellCustomizations.GetSpellInfo(SUMMON_ELEMENTAL_3);
        info_mass_polymorph = sSpellCustomizations.GetSpellInfo(MASS_POLYMORPH);
        info_conjure = sSpellCustomizations.GetSpellInfo(CONJURE);
        info_drink = sSpellCustomizations.GetSpellInfo(DRINK);
        info_pyroblast = sSpellCustomizations.GetSpellInfo(AOE_PYROBLAST);

        drinking = false;
        enraged = false;
        summoned = false;
        explode = false;
        slow = false;
        LastSuperSpell = 0;
        m_time_enrage = 0;
        m_time_special = 0;
        m_time_pyroblast = 0;
        m_time_conjure = 0;
        FlameWreathTimer = 0;
    }

    void OnCombatStart(Unit* mTarget) override
    {
        //Atiesh check
        bool HasAtiesh = false;
        if (mTarget->IsPlayer())
        {
            for (std::set< Object* >::iterator itr = getCreature()->GetInRangePlayerSetBegin(); itr != getCreature()->GetInRangePlayerSetEnd(); ++itr)
            {
                if (*itr)
                {
                    Player* plr = static_cast<Player*>(*itr);
                    if (plr->GetItemInterface()->GetItemCount(22589) > 0 ||
                        plr->GetItemInterface()->GetItemCount(22630) > 0 ||
                        plr->GetItemInterface()->GetItemCount(22631) > 0 ||
                        plr->GetItemInterface()->GetItemCount(22632) > 0)
                    {
                        HasAtiesh = true;
                        break;
                    }
                }
            }
        }

        if (HasAtiesh)
        {
            sendDBChatMessage(2046);     // Where did you get that?! Did HE send you?!
        }
        else
        {
            switch (Util::getRandomUInt(2))
            {
                case 0:
                    sendDBChatMessage(2031);     // Please, no more. My son... he's gone mad!
                    break;
                case 1:
                    sendDBChatMessage(2032);     // I'll not be tortured again!
                    break;
                case 2:
                    sendDBChatMessage(2033);     // Who are you? What do you want? Stay away from me!
                    break;
            }
        }

        setAIAgent(AGENT_SPELL);
        RegisterAIUpdateEvent(1000);
        m_time_enrage = 900;
        m_time_special = (uint32)Util::getRandomUInt(5) + 25;
        m_time_pyroblast = 0;
        drinking = false;
        enraged = false;
        summoned = false;
        explode = false;
        slow = false;
        LastSuperSpell = Util::getRandomUInt(100) % 3;
        // Door closing
        GameObject* SDoor = getNearestGameObject(-11190.012f, -1881.016f, 231.95f, 184517);
        if (SDoor)
        {
            SDoor->SetState(GO_STATE_CLOSED);
            SDoor->SetFlags(33);
        }
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->setUInt32Value(UNIT_FIELD_POWER1, getCreature()->GetMaxPower(POWER_TYPE_MANA));
        // Door opening
        GameObject* SDoor = getNearestGameObject(-11190.012f, -1881.016f, 231.95f, 184517);
        if (SDoor)
            SDoor->SetFlags(34);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        sendDBChatMessage(2045);     // At last... The nightmare is.. over...

        // Door opening
        GameObject* SDoor = getNearestGameObject(-11190.012f, -1881.016f, 231.95f, 184517);
        if (SDoor)
            SDoor->SetFlags(34);
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        switch (Util::getRandomUInt(1))
        {
            case 0:
                sendDBChatMessage(2042);     // I want this nightmare to be over!
                break;

            case 1:
                sendDBChatMessage(2043);     // Torment me no more!
                break;
        }
    }

    void AIUpdate() override
    {
        if (FlameWreathTimer)
        {
            FlameWreathTimer--;
            for (uint8 i = 0; i < 3; i++)
            {
                if (!FlameWreathTarget[i])
                    continue;

                Unit* pTarget = getCreature()->GetMapMgr()->GetUnit(FlameWreathTarget[i]);
                if (pTarget && pTarget->getDistanceSq(FWTargPosX[i], FWTargPosY[i], getCreature()->GetPositionZ()) > 3)
                {
                    pTarget->CastSpell(pTarget, 20476, true);
                    FlameWreathTarget[i] = 0;
                }
            }
        }

        if (!drinking)
        {
            if (explode)
            {
                if (slow)
                {
                    getCreature()->CastSpell(getCreature(), info_massslow, true);
                    slow = false;
                }
                else
                {
                    getCreature()->CastSpell(getCreature(), info_aexplosion, false);
                    explode = false;
                }
            }
            else if (summoned == false && getCreature()->GetHealthPct() <= 40)
            {
                getCreature()->CastSpell(getCreature(), info_summon_elemental_1, true);
                getCreature()->CastSpell(getCreature(), info_summon_elemental_2, true);
                getCreature()->CastSpell(getCreature(), info_summon_elemental_3, true);

                sendDBChatMessage(2041);     // I'm not finished yet! No, I have a few more tricks up me sleeve.
                summoned = true;
            }
            else if (getCreature()->GetManaPct() <= 20 && !getCreature()->isCastingNonMeleeSpell())
            {
                if (!m_time_pyroblast)
                {
                    getCreature()->GetAIInterface()->WipeHateList();
                    sendDBChatMessage(2040);     // Surely you would not deny an old man a replenishing drink? No, no I thought not.
                    m_time_pyroblast = 10;
                    getCreature()->CastSpell(getCreature(), info_mass_polymorph, true);
                    getCreature()->setAttackTimer(2000, false);
                }
                else if (getCreature()->GetStandState() != STANDSTATE_SIT)
                {
                    getCreature()->setAttackTimer(3000, false);
                    getCreature()->CastSpell(getCreature(), info_conjure, false);
                    getCreature()->SetStandState(STANDSTATE_SIT);
                    getCreature()->setEmoteState(EMOTE_ONESHOT_EAT);
                }
                else
                    getCreature()->CastSpell(getCreature(), info_drink, true);
            }
            else
                SpellTrigger();
        }
        else if (!getCreature()->isCastingNonMeleeSpell())
        {
            m_time_pyroblast--;
            if (!m_time_pyroblast)
            {
                getCreature()->SetStandState(STANDSTATE_STAND);
                getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);
                getCreature()->CastSpell(getCreature(), info_pyroblast, false);
                drinking = false;
            }

        }
    }

    void SpellTrigger()
    {
        m_time_enrage--;
        m_time_special--;
        if (!enraged && !m_time_enrage)
        {
            sendDBChatMessage(2044);     // You've wasted enough of my time. Let these games be finished!

            float ERX = 5 * cos(Util::getRandomFloat(6.28f)) + (getCreature()->GetPositionX());
            float ERY = 5 * sin(Util::getRandomFloat(6.28f)) + (getCreature()->GetPositionY());
            float ERZ = getCreature()->GetPositionZ();

            for (uint8 i = 0; i < 4; i++)
            {
                spawnCreature(SHADOWOFARAN, ERX, ERY, ERZ, 0);
            }
            ERX = 0;
            ERY = 0;
            ERZ = 0;
            enraged = true;
            return;
        }
        else if (!m_time_special)
        {
            CastSpecial();
            m_time_special = (uint32)Util::getRandomUInt(5) + 25;
            return;
        }
    }

    void FlameWreath()
    {
        switch (Util::getRandomUInt(1))
        {
            case 0:
                sendDBChatMessage(2034);     // I'll show you this beaten dog still has some teeth!
                break;
            case 1:
                sendDBChatMessage(2035);     // Burn you hellish fiends!
                break;
        }

        FlameWreathTimer = 20;
        FlameWreathTarget[0] = 0;
        FlameWreathTarget[1] = 0;
        FlameWreathTarget[2] = 0;

        std::vector<Player*> Targets;
        std::set< Object* >::iterator hostileItr = getCreature()->GetInRangePlayerSetBegin();
        for (; hostileItr != getCreature()->GetInRangePlayerSetEnd(); ++hostileItr)
        {
            Player* RandomTarget = NULL;
            RandomTarget = static_cast<Player*>(*hostileItr);

            if (RandomTarget && RandomTarget->isAlive() && getCreature()->GetAIInterface()->getThreatByPtr(RandomTarget) > 0)
                Targets.push_back(RandomTarget);
        }

        while (Targets.size() > 3)
            Targets.erase(Targets.begin() + Util::getRandomUInt(static_cast<uint32>(Targets.size())));

        uint32 i = 0;
        for (std::vector<Player*>::iterator itr = Targets.begin(); itr != Targets.end(); ++itr)
        {
            if (*itr)
            {
                FlameWreathTarget[i] = (*itr)->GetGUID();
                FWTargPosX[i] = (*itr)->GetPositionX();
                FWTargPosY[i] = (*itr)->GetPositionY();
                getCreature()->CastSpell((*itr), FLAME_WREATH, true);
            }
        }

    }

    void Blizzard()
    {
        switch (Util::getRandomUInt(1))
        {
            case 0:
                sendDBChatMessage(2036);     // I'll freeze you all!
                break;
            case 1:
                sendDBChatMessage(2037);     // Back to the cold dark with you!
                break;
        }

        getCreature()->CastSpell(getCreature(), info_blizzard, true);
    }

    void AoEExplosion()
    {
        switch (Util::getRandomUInt(1))
        {
            case 0:
                sendDBChatMessage(2038);     // Yes, yes, my son is quite powerful... but I have powers of my own!
                break;
            case 1:
                sendDBChatMessage(2039);     // I am not some simple jester! I am Nielas Aran!
                break;
        }

        getCreature()->CastSpell(getCreature(), info_blink_center, true);
        getCreature()->CastSpell(getCreature(), info_magnetic_pull, true);
        explode = true;
        slow = true;
    }

    void CastSpecial()
    {
        int Available[2];

        switch (LastSuperSpell)
        {
            case SUPER_BLIZZARD:
                Available[0] = SUPER_FLAME;
                Available[1] = SUPER_AOE;
                break;

            case SUPER_FLAME:
                Available[0] = SUPER_BLIZZARD;
                Available[1] = SUPER_AOE;
                break;

            case SUPER_AOE:
                Available[0] = SUPER_BLIZZARD;
                Available[1] = SUPER_FLAME;
                break;

            default:
                return;
        }

        LastSuperSpell = Available[Util::getRandomUInt(1)];

        switch (LastSuperSpell)
        {
            case SUPER_BLIZZARD:
                Blizzard();
                break;

            case SUPER_FLAME:
                FlameWreath();
                break;

            case SUPER_AOE:
                AoEExplosion();
                break;
        }
    }

protected:
    bool drinking;
    bool enraged;
    bool summoned;
    bool explode;
    bool slow;
    float FWTargPosX[3];
    float FWTargPosY[3];
    int LastSuperSpell;
    uint32 m_time_enrage;
    uint32 m_time_special;
    uint32 m_time_pyroblast;
    uint32 m_time_conjure;
    uint32 FlameWreathTimer;
    uint64 FlameWreathTarget[3];
    SpellInfo* info_flame_wreath;
    SpellInfo* info_aexplosion;
    SpellInfo* info_blizzard;
    SpellInfo* info_magnetic_pull;
    SpellInfo* info_blink_center;
    SpellInfo* info_massslow;
    SpellInfo* info_mass_polymorph;
    SpellInfo* info_conjure;
    SpellInfo* info_drink;
    SpellInfo* info_pyroblast;
    SpellInfo* info_summon_elemental_1;
    SpellInfo* info_summon_elemental_2;
    SpellInfo* info_summon_elemental_3;
};

class WaterEleAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(WaterEleAI);
    WaterEleAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        WaterBolt = 0;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        WaterBolt = (Util::getRandomUInt(3) + 5);
        RegisterAIUpdateEvent(1250);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getCreature()->Despawn(20000, 0);
    }

    void AIUpdate() override
    {
        WaterBolt--;
        if (!WaterBolt)
        {
            getCreature()->setAttackTimer(2000, false);
            Unit* target = getCreature()->GetAIInterface()->getNextTarget();
            if (target)
                getCreature()->CastSpell(target, WATERBOLT, true);
        }
    }

protected:
    int WaterBolt;
};

class ShadowofAranAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowofAranAI);
    ShadowofAranAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        ShadowPyro = 0;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        ShadowPyro = (Util::getRandomUInt(2) + 4);
        RegisterAIUpdateEvent(1250);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->Despawn(10000, 0);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getCreature()->Despawn(5000, 0);
    }

    void AIUpdate() override
    {
        ShadowPyro--;
        if (!ShadowPyro)
        {
            Unit* target = getCreature()->GetAIInterface()->getNextTarget();
            if (target != NULL)
                getCreature()->CastSpell(target, SHADOWPYRO, true);
        }
    }

protected:
    int ShadowPyro;
};

// Terestian Illhoof
const uint32 CN_ILLHOOF = 15688;

const uint32 SHADOW_BOLT = 19729;
// const uint32 S_DEMONCHAINS = 30120;
const uint32 S_KILREK = 30066;
// const uint32 F_PORTAL1 = 30179;
// const uint32 F_PORTAL2 = 30171;
const uint32 I_ENRAGE = 32964;
const uint32 SACRIFICE = 30115;

// Kil'Rek
const uint32 CN_KILREK = 17229;
const uint32 AMPLIFY_FLAMES = 30053;
const uint32 BROKEN_PACT = 30065;

// Imps
const uint32 CN_FIENDISH_IMP = 17267;
const uint32 FIREBALL_IMP = 31620;

// Demon Chains
const uint32 CN_DEMONCHAINS = 17248;
const uint32 CHAINS_VISUAL = 30206;

const uint32 CN_FPORTAL = 17265;

class IllhoofAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IllhoofAI);
    IllhoofAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        /*spells[0].info = sSpellCustomizations.GetSpellInfo(SHADOW_BOLT);
        spells[0].targettype = TARGET_ATTACKING;
        spells[0].instant = true;
        spells[0].cooldown = 5;
        spells[0].perctrigger = 50.0f;
        spells[0].attackstoptimer = 1000;

        spells[1].info = sSpellCustomizations.GetSpellInfo(I_ENRAGE);
        spells[1].targettype = TARGET_SELF;
        spells[1].instant = true;
        spells[1].cooldown = 600;
        spells[1].perctrigger = 0.0f;
        spells[1].attackstoptimer = 1000;*/

        ReSummon = false;
        ImpTimer = 0;
        ReKilrek = 0;
        DemonChain = 0;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(2050);     // Ah, you're just in time. The rituals are about to begin.

        uint32 t = (uint32)time(NULL);

        DemonChain = t + 45;
        ImpTimer = t + 10;
        ReSummon = false;

        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        clean();
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        clean();
        sendDBChatMessage(2049);     // My life, is yours. Oh great one.
    }

    void clean()
    {
        Creature* portal = getNearestCreature(-11249.51f, -1702.182f, 179.237f, CN_FPORTAL);
        Creature* portal2 = getNearestCreature(-11239.534f, -1715.338f, 179.237f, CN_FPORTAL);

        if (portal != NULL)
            portal->Despawn(0, 0);
        if (portal2 != NULL)
            portal2->Despawn(0, 0);
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        switch (Util::getRandomUInt(1))
        {
            case 0:
                sendDBChatMessage(2047);     // Your blood will anoint my circle.
                break;
            case 1:
                sendDBChatMessage(2048);     // The great one will be pleased.
                break;
        }
    }

    void AIUpdate() override
    {
        uint32 t = (uint32)time(NULL);
        if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
        {
            if (t > ImpTimer)
            {
                spawnSummoningPortals();
                ImpTimer = 0;
            }
            if (t > DemonChain)
            {
                DemonChain = t + 45;
                PlrSacrifice();
            }
            if (getCreature()->HasAura(BROKEN_PACT) && !ReSummon)
            {
                ReSummon = true;
                ReKilrek = t + 45;
            }
            else if (ReSummon && t > ReKilrek)
            {
                ReSummon = false;
                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(S_KILREK), true);
                getCreature()->RemoveAura(BROKEN_PACT);
            }
        }
    }

    void spawnSummoningPortals()
    {
        switch (Util::getRandomUInt(1))
        {
            case 0:
                sendDBChatMessage(2053);     // Come, you dwellers in the dark. Rally to my call!
                break;
            case 1:
                sendDBChatMessage(2054);     // Gather, my pets. There is plenty for all.
                break;
        }

        spawnCreature(CN_FPORTAL, -11249.51f, -1702.182f, 179.237f, 0);
        spawnCreature(CN_FPORTAL, -11239.534f, -1715.338f, 179.237f, 0);
    }

    void PlrSacrifice()
    {
        switch (Util::getRandomUInt(1))
        {
            case 0:
                sendDBChatMessage(2051);     // Please, accept this humble offering, oh great one.
                break;
            case 1:
                sendDBChatMessage(2052);     // Let the sacrifice serve his testament to my fealty.
                break;
        }

        std::vector<Player* > TargetTable;
        std::set< Object* >::iterator itr = getCreature()->GetInRangePlayerSetBegin();

        for (; itr != getCreature()->GetInRangePlayerSetEnd(); ++itr)
        {
            if (isHostile(getCreature(), (*itr)))
            {
                Player* RandomTarget = NULL;
                RandomTarget = static_cast<Player*>(*itr);
                if (RandomTarget && RandomTarget->isAlive() && isHostile(getCreature(), RandomTarget))
                    TargetTable.push_back(RandomTarget);
            }
        }
        if (!TargetTable.size())
            return;

        auto random_index = Util::getRandomUInt(0, uint32(TargetTable.size() - 1));
        auto random_target = TargetTable[random_index];

        if (random_target == nullptr)
            return;

        getCreature()->CastSpell(random_target, sSpellCustomizations.GetSpellInfo(SACRIFICE), false);

        TargetTable.clear();

        float dcX = -11234.7f;
        float dcY = -1698.73f;
        float dcZ = 179.24f;
        spawnCreature(CN_DEMONCHAINS, dcX, dcY, dcZ, 0);
    }

protected:

    bool ReSummon;
    uint32 ImpTimer;
    uint32 ReKilrek;
    uint32 DemonChain;
};

// Kil'Rek
class KilrekAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KilrekAI);
    KilrekAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        /*spells[0].info = sSpellCustomizations.GetSpellInfo(AMPLIFY_FLAMES);
        spells[0].targettype = TARGET_RANDOM_SINGLE;
        spells[0].instant = true;
        spells[0].cooldown = 5;
        spells[0].perctrigger = 50.0f;
        spells[0].attackstoptimer = 1000;

        spells[1].info = sSpellCustomizations.GetSpellInfo(BROKEN_PACT);
        spells[1].targettype = TARGET_ATTACKING;
        spells[1].instant = true;*/
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        //spells[0].casttime = (uint32)time(NULL) + spells[0].cooldown;

        RegisterAIUpdateEvent(1000);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        /*Unit* Illhoof = getNearestCreature(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), 15688);
        if (Illhoof != NULL && Illhoof->isAlive())
            Illhoof->CastSpell(Illhoof, spells[1].info, spells[1].instant);*/
    }
};

// Fiendish Imp
class FiendishImpAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FiendishImpAI);
    FiendishImpAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        /*spells[0].info = sSpellCustomizations.GetSpellInfo(FIREBALL_IMP);
        spells[0].targettype = TARGET_ATTACKING;
        spells[0].instant = false;
        spells[0].cooldown = 0;
        spells[0].perctrigger = 100.0f;
        spells[0].attackstoptimer = 1000;
        spells[0].casttime = 0;*/
    }

    void OnCombatStart(Unit* mTarget) override
    {
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);

        if (getCreature()->GetDistance2dSq(mTarget) <= 1225.0f)
        {
            setAIAgent(AGENT_SPELL);
        }

        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->Despawn(1, 0);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getCreature()->Despawn(1, 0);
    }

    void AIUpdate() override
    {
        setAIAgent(AGENT_NULL);
        /*if (getCreature()->GetAIInterface()->getNextTarget() && getCreature()->GetDistance2dSq(getCreature()->GetAIInterface()->getNextTarget()) <= 1225.0f)
        {
            setAIAgent(AGENT_SPELL);
            if (!getCreature()->isCastingNonMeleeSpell() && Util::getRandomUInt(10) > 2)
            {
                getCreature()->setAttackTimer(spells[0].attackstoptimer, false);

                Unit* target = NULL;
                target = getCreature()->GetAIInterface()->getNextTarget();

                getCreature()->CastSpell(target, spells[0].info, spells[0].instant);
                return;
            }
        }*/
    }
};

class DemonChains : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DemonChains);
    DemonChains(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(CHAINS_VISUAL), true);
        getCreature()->setMoveRoot(true);
        getCreature()->DisableAI();
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        if (getCreature()->GetHealthPct() > 0)
            getCreature()->Despawn(10000, 0);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        Unit* uIllhoof = getNearestCreature(getCreature()->GetPositionX(), getCreature()->GetPositionY(),
            getCreature()->GetPositionZ(), CN_ILLHOOF);
        if (uIllhoof != NULL && uIllhoof->isAlive())
            uIllhoof->RemoveAura(SACRIFICE);

        getCreature()->Despawn(10000, 0);
    }
};

class FiendPortal : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FiendPortal);
    FiendPortal(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->setMoveRoot(true);

        getCreature()->setUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
        _setMeleeDisabled(true);
        getCreature()->GetAIInterface()->m_canMove = false;
        getCreature()->m_noRespawn = true;

        RegisterAIUpdateEvent(10000);
    }

    void AIUpdate() override
    {
        spawnCreature(CN_FIENDISH_IMP, getCreature()->GetPosition());
    }

};

// Prince Malchezaar
const uint32 CN_MALCHEZAAR = 15690;
const uint32 CN_INFERNAL = 17646;
const uint32 CN_AXES = 17650;
const uint32 CN_DUMMY = 17644;

const uint32 ENFEEBLE = 30843;
const uint32 SHADOWNOVA = 30852;
const uint32 SW_PAIN = 30854;
const uint32 THRASH_AURA = 12787;
const uint32 SUNDER_ARMOR = 25225;
const uint32 AMPLIFY_DMG = 39095; // old 12738
const uint32 SUMMON_AXES = 30891;
const uint32 WIELD_AXES = 30857;

// Extra creature info
const uint32 INF_RAIN = 33814;
const uint32 HELLFIRE = 39131;
const uint32 DEMONIC_FRENZY = 32851;

// Item model info
const uint32 AXE_ITEM_MODEL = 40066;
const uint32 AXE_ITEM_INFO = 33488898;
const uint32 AXE_ITEM_SLOT = 768;
/* Emotes:
SPECIAL? - 9223 - 9320
AXETOSS2? - 9317
*/

class MalchezaarAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MalchezaarAI);
    MalchezaarAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_phase = 1;
       
        memset(Enfeeble_Targets, 0, sizeof(Enfeeble_Targets));
        memset(Enfeeble_Health, 0, sizeof(Enfeeble_Health));

        /*spells[0].info = sSpellCustomizations.GetSpellInfo(SW_PAIN);
        spells[0].targettype = TARGET_ATTACKING;
        spells[0].instant = true;
        spells[0].cooldown = 15;
        spells[0].perctrigger = 50.0f;
        spells[0].attackstoptimer = 1000;

        spells[1].info = sSpellCustomizations.GetSpellInfo(ENFEEBLE);
        spells[1].targettype = TARGET_VARIOUS;
        spells[1].instant = true;
        spells[1].cooldown = 25;
        spells[1].perctrigger = 0.0f;
        spells[1].attackstoptimer = 1000;

        spells[2].info = sSpellCustomizations.GetSpellInfo(INF_RAIN);
        spells[2].targettype = TARGET_DESTINATION;
        spells[2].instant = true;
        spells[2].cooldown = 43;
        spells[2].perctrigger = 0.0f;
        spells[2].attackstoptimer = 1000;

        spells[3].info = sSpellCustomizations.GetSpellInfo(SUNDER_ARMOR);
        spells[3].targettype = TARGET_ATTACKING;
        spells[3].instant = true;
        spells[3].cooldown = 15;
        spells[3].perctrigger = 0.0f;
        spells[3].attackstoptimer = 1000;

        spells[4].info = sSpellCustomizations.GetSpellInfo(AMPLIFY_DMG);
        spells[4].targettype = TARGET_RANDOM_SINGLE;
        spells[4].instant = true;
        spells[4].cooldown = 20;
        spells[4].perctrigger = 0.0f;
        spells[4].attackstoptimer = 1000;
        spells[4].mindist2cast = 0.0f;

        spells[5].info = sSpellCustomizations.GetSpellInfo(SHADOWNOVA);
        spells[5].targettype = TARGET_VARIOUS;
        spells[5].instant = false;
        spells[5].cooldown = 4;
        spells[5].perctrigger = 0.0f;
        spells[5].attackstoptimer = 2000;

        spells[6].info = sSpellCustomizations.GetSpellInfo(THRASH_AURA);
        spells[6].targettype = TARGET_SELF;
        spells[6].instant = true;
        spells[6].cooldown = 1;
        spells[6].perctrigger = 0.0f;
        spells[6].attackstoptimer = 1000;

        spells[7].info = sSpellCustomizations.GetSpellInfo(WIELD_AXES);
        spells[7].targettype = TARGET_SELF;
        spells[7].instant = true;
        spells[7].attackstoptimer = 1000;

        spells[8].info = sSpellCustomizations.GetSpellInfo(SUMMON_AXES);
        spells[8].targettype = TARGET_SELF;
        spells[8].instant = true;
        spells[8].attackstoptimer = 1000;*/

        // Dummy initialization
        float dumX = -10938.56f;
        float dumY = -2041.26f;
        float dumZ = 305.132f;

        CreatureAIScript* infernalDummy = spawnCreatureAndGetAIScript(CN_DUMMY, dumX, dumY, dumZ, 0);
        if (infernalDummy != nullptr)
        {
            setLinkedCreatureAIScript(infernalDummy);
            infernalDummy->setLinkedCreatureAIScript(this);
        }

        ranX = 0;
        ranY = 0;
        m_infernal = false;
        m_enfeebleoff = 0;
        m_spawn_infernal = 0;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(2019);     // Madness has brought you here to me. I shall be your undoing.

        m_spawn_infernal = 0;
        m_infernal = false;

        RegisterAIUpdateEvent(1000);

        GameObject* MDoor = getNearestGameObject(-11018.5f, -1967.92f, 276.652f, 185134);
        if (MDoor != NULL)
        {
            MDoor->SetState(GO_STATE_CLOSED);
            MDoor->SetFlags(33);
        }
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        // Reset weapon
        getCreature()->SetEquippedItem(MELEE, 0);

        // Off hand weapon
        getCreature()->SetEquippedItem(OFFHAND, 0);

        CreatureProperties const* cp = sMySQLStore.getCreatureProperties(CN_MALCHEZAAR);
        if (cp == nullptr)
            return;

        getCreature()->SetMinDamage(cp->MinDamage);
        getCreature()->SetMaxDamage(cp->MaxDamage);

        for (uint8 i = 0; i < 5; ++i)
            Enfeeble_Targets[i] = 0;

        if (getLinkedCreatureAIScript() != NULL)
            getLinkedCreatureAIScript()->getCreature()->Despawn(10000, 0);

        GameObject* MDoor = getNearestGameObject(-11018.5f, -1967.92f, 276.652f, 185134);
        // Open door
        if (MDoor != NULL)
            MDoor->SetState(GO_STATE_OPEN);

        Creature* MAxes = NULL;
        MAxes = getNearestCreature(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), CN_AXES);
        if (MAxes != NULL)
            MAxes->Despawn(1000, 0);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        sendDBChatMessage(2030);     // I refuse to concede defeat. I am a prince of the Eredar! I am...

        if (getLinkedCreatureAIScript() != NULL)
            getLinkedCreatureAIScript()->getCreature()->Despawn(10000, 0);

        Creature* MAxes = NULL;
        MAxes = getNearestCreature(getCreature()->GetPositionX(), getCreature()->GetPositionY(),
            getCreature()->GetPositionZ(), CN_AXES);
        if (MAxes)
            MAxes->Despawn(1000, 0);

        GameObject* MDoor = getNearestGameObject(-11018.5f, -1967.92f, 276.652f, 185134);
        // Open door
        if (MDoor)
            MDoor->SetState(GO_STATE_OPEN);
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        switch (Util::getRandomUInt(2))
        {
            case 0:
                sendDBChatMessage(2027);     // You are, but a plaything, unfit even to amuse.
                break;
            case 1:
                sendDBChatMessage(2026);     // Your greed, your foolishness has brought you to this end.
                break;
            case 2:
                sendDBChatMessage(2025);     // Surely you did not think you could win.
                break;
        }
    }

    void AIUpdate() override
    {
        switch (m_phase)
        {
            case 1:
                if (getCreature()->GetHealthPct() <= 60)
                    PhaseTwo();
                break;
            case 2:
                PhaseTwo();
                if (getCreature()->GetHealthPct() <= 30)
                    PhaseThree();
                break;
            case 3:
                PhaseThree();
                break;
            default:
                m_phase = 1;
                break;
        }
        /*if (t > spells[1].casttime && getCreature()->GetAIInterface()->getNextTarget() && !getCreature()->isCastingNonMeleeSpell())
        {
            Enfeebler();
            spells[1].casttime = t + spells[1].cooldown;
            spells[5].casttime = t + spells[5].cooldown;
        }
        else if (t > m_spawn_infernal && m_infernal == true && getCreature()->GetAIInterface()->getNextTarget())
        {
            spawnCreature(CN_INFERNAL, ranX, ranY, 276.0f, 0);
            m_spawn_infernal = 0;
            m_infernal = false;
        }
        else if (t > spells[5].casttime && getCreature()->GetAIInterface()->getNextTarget() && !getCreature()->isCastingNonMeleeSpell())
        {
            spells[5].casttime = 0;
            getCreature()->CastSpell(getCreature(), spells[5].info, spells[5].instant);
            m_enfeebleoff = t + 3;
        }
        else if (t > m_enfeebleoff)
        {
            EnfeebleOff();
            m_enfeebleoff = 0;
        }
        else if (t > spells[2].casttime)
        {
            SummonInfernal();
            if (m_phase == 3)
                spells[2].casttime = t + 20;
            else
                spells[2].casttime = t + spells[2].cooldown;
        }
        else
        {
            float val = Util::getRandomFloat(100.0f);
            SpellCast(val);
        }*/
    }

    void PhaseTwo()
    {
        if (getCreature()->GetHealthPct() <= 60 && m_phase == 1)
        {
            sendDBChatMessage(2020);     // Time is the fire in which you'll burn!");

            /*uint32 t = (uint32)time(NULL);
            spells[0].casttime = 0;
            spells[3].casttime = t + spells[3].cooldown;
            spells[3].perctrigger = 50.0f;

            getCreature()->CastSpell(getCreature(), spells[6].info, spells[6].instant);
            getCreature()->CastSpell(getCreature(), spells[7].info, spells[6].instant);*/

            // Main hand weapon
            getCreature()->SetEquippedItem(MELEE, AXE_ITEM_MODEL);

            //Off Hand
            getCreature()->SetEquippedItem(OFFHAND, AXE_ITEM_MODEL);

            CreatureProperties const* cp = sMySQLStore.getCreatureProperties(CN_MALCHEZAAR);
            if (cp == nullptr)
                return;

            getCreature()->SetMinDamage(1.5f * cp->MinDamage);
            getCreature()->SetMaxDamage(1.5f * cp->MaxDamage);

            m_phase = 2;
        }
    }

    void PhaseThree()
    {
        if (getCreature()->GetHealthPct() <= 30 && m_phase == 2)
        {
            sendDBChatMessage(2024);     // How can you hope to withstand against such overwhelming power?

            /*uint32 t = (uint32)time(NULL);

            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].casttime = t + spells[0].cooldown;

            spells[1].casttime = 0;
            spells[1].perctrigger = 0.0f;

            spells[4].casttime = t + spells[4].cooldown;
            spells[4].perctrigger = 50.0f;

            getCreature()->CastSpell(getCreature(), spells[8].info, spells[8].instant);
*/
            getCreature()->RemoveAura(THRASH_AURA);
            getCreature()->RemoveAura(WIELD_AXES);

            // Main hand weapon
            getCreature()->SetEquippedItem(MELEE, 0);

            //Off Hand
            getCreature()->SetEquippedItem(OFFHAND, 0);

            CreatureProperties const* cp = sMySQLStore.getCreatureProperties(CN_MALCHEZAAR);
            if (cp == nullptr)
                return;

            getCreature()->SetMinDamage(cp->MinDamage);
            getCreature()->SetMaxDamage(cp->MaxDamage);
            m_phase = 3;
        }
    }

    void SummonInfernal()
    {
        switch (Util::getRandomUInt(1))
        {
            case 0:
                sendDBChatMessage(2029);     // You face not Malchezaar alone, but the legions I command!
                break;
            case 1:
                sendDBChatMessage(2028);     // All realities, all dimensions are open to me!
                break;
        }

        ranX = Util::getRandomFloat(113.47f) - 11019.37f;
        ranY = Util::getRandomFloat(36.951f) - 2011.549f;
        //if (getLinkedCreatureAIScript() != NULL)
        //{
        //    getLinkedCreatureAIScript()->getCreature()->CastSpellAoF(LocationVector(ranX, ranY, 275.0f), spells[2].info, spells[2].instant); // Shoots the missile
        //    float dist = getLinkedCreatureAIScript()->getCreature()->CalcDistance(ranX, ranY, 275.0f);
        //    uint32 dtime = (uint32)(dist / spells[2].info->getSpeed());
        //    m_spawn_infernal = (uint32)time(NULL) + dtime + 1;
        //    m_infernal = true;
        //}
    }

    void Enfeebler()
    {
        std::vector<Player*> Targets;
        std::set< Object* >::iterator Itr = getCreature()->GetInRangePlayerSetBegin();

        for (; Itr != getCreature()->GetInRangePlayerSetEnd(); ++Itr)
        {
            if (isHostile(getCreature(), (*Itr)))
            {
                Player* RandomTarget = static_cast<Player*>(*Itr);

                if (RandomTarget->isAlive())
                    Targets.push_back(RandomTarget);
            }
        }

        while (Targets.size() > 5)
            Targets.erase(Targets.begin() + Util::getRandomUInt(static_cast<uint32>(Targets.size())));

        /*for (std::vector<Player*>::iterator E_Itr = Targets.begin(); E_Itr != Targets.end(); ++E_Itr)
        {
            if ((*E_Itr)->GetGUID() != getCreature()->GetAIInterface()->GetMostHated()->GetGUID())
            {
                Enfeeble_Targets[i] = (*E_Itr)->GetGUID();
                Enfeeble_Health[i] = (*E_Itr)->getUInt32Value(UNIT_FIELD_HEALTH);

                getCreature()->CastSpell((*E_Itr), spells[1].info, spells[1].instant);
                (*E_Itr)->SetHealth(1);
                i++;
            }
        }*/
    }

    void EnfeebleOff()
    {
        for (uint8 i = 0; i < 5; ++i)
        {
            Unit* ETarget = getCreature()->GetMapMgr()->GetUnit(Enfeeble_Targets[i]);
            if (ETarget && ETarget->isAlive())
                ETarget->setUInt64Value(UNIT_FIELD_HEALTH, Enfeeble_Health[i]);
            Enfeeble_Targets[i] = 0;
            Enfeeble_Health[i] = 0;
        }
    }

protected:
    float ranX;
    float ranY;
    int m_phase;
    bool m_infernal;
    uint32 m_enfeebleoff;
    uint32 m_spawn_infernal;
    uint64 Enfeeble_Targets[5];
    uint64 Enfeeble_Health[5];
};

class NetherInfernalAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(NetherInfernalAI);
    NetherInfernalAI(Creature* pCreature) : CreatureAIScript(pCreature) {};

    void OnLoad() override
    {
        setRooted(true);
        setCanEnterCombat(false);
        getCreature()->setUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->m_noRespawn = true;
        RegisterAIUpdateEvent(6000);
        despawn(175000, 0);
        getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(HELLFIRE), true);
    }

    void AIUpdate() override
    {
        getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(HELLFIRE), true);
    }

};

class InfernalDummyAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(InfernalDummyAI);
    InfernalDummyAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);

        Movement::Location loc;
        loc.x = -10938.56f;
        loc.y = -2041.26f;
        loc.z = 305.132f;
        loc.o = 0;

        AddWaypoint(CreateWaypoint(1, 0, Movement::WP_MOVE_TYPE_FLY, loc));
    }
};

class MAxesAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MAxesAI);
    MAxesAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->setUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);

        /*spells[0].info = sSpellCustomizations.GetSpellInfo(DEMONIC_FRENZY);
        spells[0].targettype = TARGET_SELF;
        spells[0].instant = true;
        spells[0].cooldown = 1;*/
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(6000);

        //spells[0].casttime = (uint32)time(NULL) + spells[0].cooldown;

        std::vector<Unit* > TargetTable;
        for (std::set< Object* >::iterator itr = getCreature()->GetInRangePlayerSetBegin(); itr != getCreature()->GetInRangePlayerSetEnd(); ++itr)
        {
            if (isHostile(getCreature(), (*itr)) && (static_cast<Player*>(*itr))->isAlive())
            {
                Player* RandomTarget = NULL;
                RandomTarget = static_cast<Player*>(*itr);

                if (RandomTarget && RandomTarget->isAlive() && isHostile(getCreature(), RandomTarget))
                    TargetTable.push_back(RandomTarget);
            }
        }

        if (!TargetTable.size())
            return;

        auto random_index = Util::getRandomUInt(0, uint32(TargetTable.size() - 1));
        auto random_target = TargetTable[random_index];

        if (random_target == nullptr)
            return;

        getCreature()->GetAIInterface()->taunt(random_target, true);
    }

    void AIUpdate() override
    {
        /*uint32 t = (uint32)time(NULL);
        if (t > spells[0].casttime)
        {
            getCreature()->CastSpell(getCreature(), spells[0].info, spells[0].instant);
            spells[0].casttime = t + spells[0].cooldown;
        }*/
    }
};

// Netherspite
const uint32 CN_NETHERSPITE = 15689;
const uint32 CN_VOIDZONE = 17470;

// const uint32 NETHERBURN = 30523; //not aura
// const uint32 VOIDZONE = 28863;
const uint32 CONSUMPTION = 32251; // used by void zone
const uint32 NETHERBREATH = 38524; // old 36631
const uint32 N_BERSERK = 38688;
const uint32 NETHERBURN = 30522;

class NetherspiteAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(NetherspiteAI);
    NetherspiteAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        /*spells[0].info = sSpellCustomizations.GetSpellInfo(NETHERBREATH);
        spells[0].targettype = TARGET_ATTACKING;
        spells[0].instant = false;
        spells[0].cooldown = Util::getRandomUInt(5) + 30;
        spells[0].perctrigger = 50.0f;
        spells[0].attackstoptimer = 1000;

        spells[1].info = sSpellCustomizations.GetSpellInfo(N_BERSERK);
        spells[1].targettype = TARGET_SELF;
        spells[1].instant = true;
        spells[1].cooldown = 540;
        spells[1].perctrigger = 0.0f;
        spells[1].attackstoptimer = 1000;

        spells[2].info = sSpellCustomizations.GetSpellInfo(NETHERBURN);
        spells[2].targettype = TARGET_SELF;
        spells[2].instant = true;*/
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        /*for (uint8 i = 0; i < nrspells; i++)
            spells[i].casttime = spells[i].cooldown;

        uint32 t = (uint32)time(NULL);
        VoidTimer = t + 25;
        getCreature()->CastSpell(getCreature(), spells[2].info, spells[2].instant);*/

        RegisterAIUpdateEvent(1000);

        GameObject* NDoor = getNearestGameObject(-11186.2f, -1665.14f, 281.398f, 185521);
        if (NDoor)
        {
            NDoor->SetState(GO_STATE_CLOSED);
            NDoor->SetFlags(33);
        }
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->RemoveAura(NETHERBURN);

        GameObject* NDoor = getNearestGameObject(-11186.2f, -1665.14f, 281.398f, 185521);
        if (NDoor)
            NDoor->SetState(GO_STATE_OPEN);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        GameObject* NDoor = getNearestGameObject(-11186.2f, -1665.14f, 281.398f, 185521);
        if (NDoor)
            NDoor->SetState(GO_STATE_OPEN);
    }

    void AIUpdate() override
    {
        /*uint32 t = (uint32)time(NULL);
        if (t > VoidTimer && getCreature()->GetAIInterface()->getNextTarget())
        {
            VoidTimer = t + 20;
            std::vector<Unit* > TargetTable;
            for (std::set< Object* >::iterator itr = getCreature()->GetInRangePlayerSetBegin(); itr != getCreature()->GetInRangePlayerSetEnd(); ++itr)
            {
                Unit* RandomTarget = NULL;
                RandomTarget = static_cast<Unit*>(*itr);

                if (RandomTarget && RandomTarget->isAlive() && isHostile(getCreature(), (*itr)))
                    TargetTable.push_back(RandomTarget);
            }

            if (!TargetTable.size())
                return;

            auto random_index = Util::getRandomUInt(0, uint32(TargetTable.size() - 1));
            auto random_target = TargetTable[random_index];

            if (random_target == nullptr)
                return;

            float vzX = 5 * cos(Util::getRandomFloat(6.28f)) + random_target->GetPositionX();
            float vzY = 5 * cos(Util::getRandomFloat(6.28f)) + random_target->GetPositionY();
            float vzZ = random_target->GetPositionZ();
            spawnCreature(CN_VOIDZONE, vzX, vzY, vzZ, 0);
            TargetTable.clear();
        }*/
    }
};

class VoidZoneAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(VoidZoneAI);
    VoidZoneAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->setMoveRoot(true);
        getCreature()->DisableAI();
        getCreature()->setUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
        _setMeleeDisabled(true);
        getCreature()->GetAIInterface()->m_canMove = false;
        getCreature()->m_noRespawn = true;
        getCreature()->Despawn(30000, 0);

        RegisterAIUpdateEvent(2000);

        /*spells[0].info = sSpellCustomizations.GetSpellInfo(CONSUMPTION);
        spells[0].instant = true;
        spells[0].cooldown = 2;
        spells[0].casttime = (uint32)time(NULL) + spells[0].cooldown;

        getCreature()->CastSpell(getCreature(), spells[0].info, spells[0].instant);*/
    }

    void AIUpdate() override
    {
        /*uint32 t = (uint32)time(NULL);
        if (t > spells[0].casttime)
        {
            getCreature()->CastSpell(getCreature(), spells[0].casttime, spells[0].instant);
            spells[0].casttime = t + spells[0].cooldown;
        }*/
    }
};

//------------------------------------
//    -= Nightbane =-
//------------------------------------

/* \todo
 - Rain of Bones on one random player/pet
 - Summons five Restless Skeletons.
*/

const uint32 CN_NIGHTBANE = 17225;
const uint32 CN_RESTLESS_SKELETON = 17261; // not needed if spell works


// ground spells
const uint32 BELLOWING_ROAR = 36922;
const uint32 CHARRED_EARTH = 30129; //Also 30209 (Target Charred Earth) triggers this
#undef CLEAVE
const uint32 CLEAVE = 31043; // fixme: correct spell?!
const uint32 SMOLDERING_BREATH = 39385;
const uint32 TAIL_SWEEP = 25653;//\todo  how to use this spell???
const uint32 DISTRACTING_ASH = 30280;

// air spells
const uint32 DISTRACTING_ASH_FLY = 30130; // all guides say ground spell but animation is flying?!
const uint32 RAIN_OF_BONES = 37091; // Spell bugged: should debuff with 37098
const uint32 SMOKING_BLAST = 37057;
const uint32 FIREBALL_BARRAGE = 30282;
const uint32 SUMMON_BONE_SKELETONS = 30170;

static Movement::Location coords[] =
{
    { 0, 0, 0, 0 },
    { -11173.719727f, -1863.993164f, 130.390396f, 5.343079f }, // casting point
    { -11125.542969f, -1926.884644f, 139.349365f, 3.982360f },
    { -11166.404297f, -1950.729736f, 114.714726f, 1.537812f },
    { -11167.497070f, -1922.315918f, 91.473755f, 1.390549f } // landing point
};

class NightbaneAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(NightbaneAI);
    NightbaneAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        ////ground phase spells
        //spells[0].info = sSpellCustomizations.GetSpellInfo(BELLOWING_ROAR);
        //spells[0].targettype = TARGET_VARIOUS;
        //spells[0].instant = false;
        //spells[0].cooldown = 30; //confirmed
        //spells[0].perctrigger = 0.0f;
        //spells[0].attackstoptimer = 1500;

        //spells[1].info = sSpellCustomizations.GetSpellInfo(CHARRED_EARTH);
        //spells[1].targettype = TARGET_RANDOM_SINGLE;
        //spells[1].instant = false;
        //spells[1].cooldown = 15;
        //spells[1].perctrigger = 0.0f;
        //spells[1].attackstoptimer = 1000;

        //spells[2].info = sSpellCustomizations.GetSpellInfo(CLEAVE);
        //spells[2].targettype = TARGET_ATTACKING;
        //spells[2].instant = false;
        //spells[2].cooldown = 7;
        //spells[2].perctrigger = 0.0f;
        //spells[2].attackstoptimer = 1000;

        //spells[3].info = sSpellCustomizations.GetSpellInfo(DISTRACTING_ASH);
        //spells[3].targettype = TARGET_RANDOM_SINGLE;
        //spells[3].instant = false;
        //spells[3].cooldown = 60;
        //spells[3].perctrigger = 0.0f;
        //spells[3].attackstoptimer = 1000;

        //spells[4].info = sSpellCustomizations.GetSpellInfo(SMOLDERING_BREATH);
        //spells[4].targettype = TARGET_ATTACKING;
        //spells[4].instant = false;
        //spells[4].cooldown = 20;
        //spells[4].perctrigger = 0.0f;
        //spells[4].attackstoptimer = 1000;

        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);

        for (uint8 i = 1; i < 5; i++)
        {
            AddWaypoint(CreateWaypoint(i, 0, Movement::WP_MOVE_TYPE_FLY, coords[i]));
        }

        m_phase = 0;
        m_currentWP = 0;
        mTailSweepTimer = 0;
        m_FlyPhaseTimer = 0;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        m_phase = 0;
        m_currentWP = 4;
        mTailSweepTimer = 25;
        //not sure about this: _unit->PlaySoundToSet(9456);
        ///\todo  "An ancient being awakens in the distance..."
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
        getCreature()->GetAIInterface()->unsetSplineFlying();
        getCreature()->GetAIInterface()->m_canMove = true;
    }

    void AIUpdate() override
    {
        switch (m_phase)
        {
            case 1:
            case 3:
            case 5:
                FlyPhase();
                break;

            case 0:
            case 2:
            case 4:
                GroundPhase();
                break;
        }
    }

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        switch (iWaypointId)
        {
            case 1: //casting point
            {
                getCreature()->GetAIInterface()->m_canMove = false;
                getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
                getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                m_currentWP = 1;
            }
            break;
            case 4: //ground point
            {
                getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
                getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                Land();
                m_currentWP = 4;
            }
            break;
            default:
            {
                //move to the next waypoint
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
            }
            break;
        }
    }

    void FlyPhase()
    {
        if (m_currentWP != 1)
            return;

        m_FlyPhaseTimer--;
        if (!m_FlyPhaseTimer)
        {
            if (getCreature()->isCastingNonMeleeSpell())
                getCreature()->interruptSpell();

            getCreature()->GetAIInterface()->m_canMove = true;
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
            getCreature()->GetAIInterface()->StopMovement(0);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
            getCreature()->GetAIInterface()->setWayPointToMove(2);
            m_phase++;
            return;
        }

        if (m_FlyPhaseTimer > 15)
            return;

        Unit* target = NULL;

        //first cast
        if (m_FlyPhaseTimer == 15)
        {
            //Casts Rain of Bones on one random player/pet
            //CastSpellOnRandomTarget(5, 0, 40);
            //summon 3 skeletons
            //_unit->CastSpellAoF(target->GetPositionX(),target->GetPositionY(),target->GetPositionZ(), sSpellCustomizations.GetSpellInfo(SUMMON_BONE_SKELETONS), true);
            return;
        }

        //Shoots powerful Smoking Blast every second for approximately 15 seconds.
        if (getCreature()->GetAIInterface()->getNextTarget() != NULL)
        {
            target = getCreature()->GetAIInterface()->getNextTarget();
            getCreature()->CastSpell(target, sSpellCustomizations.GetSpellInfo(SMOKING_BLAST), true);
        }

        target = NULL;
        //fireball barrage check
        for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
        {
            if ((*itr)->IsPlayer())
            {
                target = static_cast<Unit*>(*itr);

                if (getCreature()->GetDistance2dSq(target) > 2025) //45 yards
                {
                    getCreature()->CastSpellAoF(target->GetPosition(), sSpellCustomizations.GetSpellInfo(FIREBALL_BARRAGE), true);
                    break; //stop
                }
            }
        }
    }

    void GroundPhase()
    {
        if (m_currentWP != 4)
            return;

        //Switch if needed
        if ((m_phase == 0 && getCreature()->GetHealthPct() <= 75)
            || (m_phase == 2 && getCreature()->GetHealthPct() <= 50)
            || (m_phase == 4 && getCreature()->GetHealthPct() <= 25))
        {
            if (getCreature()->isCastingNonMeleeSpell())
                getCreature()->interruptSpell();

            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
            getCreature()->GetAIInterface()->StopMovement(0);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
            getCreature()->GetAIInterface()->setWayPointToMove(1);
            Fly();
            m_FlyPhaseTimer = 17;
            m_phase++;
            return;
        }

        //Tail Sweep
        mTailSweepTimer--;
        if (!mTailSweepTimer)
        {
            Unit* target = NULL;
            for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
            {
                if ((*itr)->IsPlayer())
                {
                    target = static_cast<Unit*>(*itr);

                    //cone behind the boss
                    if (target->isAlive() && target->isInBack(getCreature()))
                        getCreature()->CastSpell(target, sSpellCustomizations.GetSpellInfo(TAIL_SWEEP), true);
                }
            }
            mTailSweepTimer = 25;
        }
    }

    void Fly()
    {
        getCreature()->Emote(EMOTE_ONESHOT_LIFTOFF);

        getCreature()->setMoveHover(true);

        getCreature()->GetAIInterface()->setSplineFlying();
    }

    void Land()
    {
        getCreature()->Emote(EMOTE_ONESHOT_LAND);

        getCreature()->setMoveHover(false);

        getCreature()->GetAIInterface()->unsetSplineFlying();
    }

protected:
    uint32 m_phase;
    uint32 m_FlyPhaseTimer;
    uint32 m_currentWP;
    uint32 mTailSweepTimer;
};

//Opera Event
//Wizzard of Oz
const uint32 CN_DOROTHEE = 17535;

const uint32 SP_AOE_FEAR = 29321;
const uint32 SP_WATER_BOLT = 31012;
const uint32 SP_SUMMON_TITO = 31014;

class DorotheeAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DorotheeAI);
    uint32 summontito;

    DorotheeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //spells[0].info = sSpellCustomizations.GetSpellInfo(SP_AOE_FEAR);
        //spells[0].targettype = TARGET_ATTACKING;
        //spells[0].instant = true;
        //spells[0].cooldown = 30;  //correct cooldown?
        //spells[0].perctrigger = 0.0f;
        //spells[0].attackstoptimer = 1000;

        //spells[1].info = sSpellCustomizations.GetSpellInfo(SP_WATER_BOLT);
        //spells[1].targettype = TARGET_RANDOM_SINGLE;
        //spells[1].instant = false;
        //spells[1].perctrigger = 100.0f;
        //spells[1].attackstoptimer = 1000;

        summontito = 0;
        tito = NULL;
        titoSpawned = false;
        titoDeadSpeech = false;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(1978);     // Oh Tito, we simply must find a way home! The old wizard could be our only hope! Strawman, Roar, Tinhead, will you - wait... oh golly, look we have visitors!

        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->Despawn(1, 0);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        sendDBChatMessage(1975);     // Oh at last, at last I can go home!

        //Check to see if we can spawn The Crone now
        Creature* Dorothee = getNearestCreature(-10897.650f, -1755.8311f, 90.476f, 17535); //Dorothee
        Creature* Strawman = getNearestCreature(-10904.088f, -1754.8988f, 90.476f, 17543); //Strawman
        Creature* Roar = getNearestCreature(-10891.115f, -1756.4898f, 90.476f, 17546);//Roar
        Creature* Tinman = getNearestCreature(-10884.501f, -1757.3249f, 90.476f, 17547); //Tinman

        if ((Dorothee == NULL || Dorothee->IsDead()) && (Strawman == NULL || Strawman->IsDead()) && (Roar == NULL || Roar->IsDead()) && (Tinman == NULL || Tinman->IsDead()))
        {
            spawnCreature(18168, -10884.501f, -1757.3249f, 90.476f, 0.0f);
        }
    }

    void SpawnTito()    // Lacking in collision checks!
    {
        float xchange = Util::getRandomFloat(15.0f);
        float distance = 15.0f;

        float ychange = sqrt(distance * distance - xchange * xchange);

        if (Util::getRandomUInt(1) == 1)
            xchange *= -1;
        if (Util::getRandomUInt(1) == 1)
            ychange *= -1;

        float newposx = getCreature()->GetPositionX() + xchange;
        float newposy = getCreature()->GetPositionY() + ychange;

        tito = spawnCreature(17548, newposx, newposy, getCreature()->GetPositionZ() + 0.5f, 2.177125f);
    }

    void AIUpdate() override
    {
        if (titoSpawned && !tito && titoDeadSpeech)
        {
            sendDBChatMessage(1977);     // Tito! Oh Tito, no!

            titoDeadSpeech = false;
        }

        if (summontito > 20 && !titoSpawned)
        {
            sendDBChatMessage(1976);     // Don't let them hurt us Tito! Oh, you won't, will you?

            SpawnTito();
            titoSpawned = true;
            titoDeadSpeech = true;
            return;
        }
        summontito++;
    }

protected:
    Unit* tito;
    bool titoSpawned;
    bool titoDeadSpeech;
};

const uint32 CN_TITO = 17548;

const uint32 SP_ANNOYING_YIPPING = 31015;

//No kill sound

class TitoAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TitoAI);
    TitoAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        /*spells[0].info = sSpellCustomizations.GetSpellInfo(SP_ANNOYING_YIPPING);
        spells[0].targettype = TARGET_ATTACKING;
        spells[0].instant = true;
        spells[0].perctrigger = 15.0f;
        spells[0].attackstoptimer = 1000;*/

        getCreature()->m_noRespawn = true;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->Despawn(1, 0);
    }
};

const uint32 CN_STRAWMAN = 17543;

const uint32 SP_BURNING_STRAW = 31075;
const uint32 SP_BRAIN_BASH = 31046;

class StrawmanAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(StrawmanAI);
    StrawmanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //spells[0].info = sSpellCustomizations.GetSpellInfo(SP_BURNING_STRAW);//  NEEDS TO BE SO IT ONLY AFFECTS HIM WHEN HE IS HIT BY FIRE DMG!
        //spells[0].targettype = TARGET_SELF;
        //spells[0].instant = true;
        //spells[0].perctrigger = 0.0f;
        //spells[0].attackstoptimer = 1000;

        //spells[1].info = sSpellCustomizations.GetSpellInfo(SP_BRAIN_BASH);
        //spells[1].targettype = TARGET_RANDOM_SINGLE;
        //spells[1].instant = true;
        //spells[1].cooldown = 8; //not sure about this
        //spells[1].perctrigger = 0.0f;
        //spells[1].attackstoptimer = 1000;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(1982);     // Now what should I do with you? I simply can't make up my mind.

        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->Despawn(1, 0);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        sendDBChatMessage(1983);     // Don't let them make... a mattress outta' me.

        //Check to see if we can spawn The Crone now
        Creature* Dorothee = getNearestCreature(-10897.650f, -1755.8311f, 90.476f, 17535);    //Dorothee
        Creature* Strawman = getNearestCreature(-10904.088f, -1754.8988f, 90.476f, 17543);    //Strawman
        Creature* Roar = getNearestCreature(-10891.115f, -1756.4898f, 90.476f, 17546);    //Roar
        Creature* Tinman = getNearestCreature(-10884.501f, -1757.3249f, 90.476f, 17547);    //Tinman

        if ((Dorothee == NULL || Dorothee->IsDead()) && (Strawman == NULL || Strawman->IsDead()) && (Roar == NULL || Roar->IsDead()) && (Tinman == NULL || Tinman->IsDead()))
        {
            spawnCreature(18168, -10884.501f, -1757.3249f, 90.476f, 0.0f);
        }
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(1984);     // I guess I'm not a failure after all!
    }
};

const uint32 CN_TINHEAD = 17547;

const uint32 SP_CLEAVE = 15284;
const uint32 SP_RUST = 31086;
//dont bother.. spell does not work.. needs fix

class TinheadAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TinheadAI);
    TinheadAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        /*spells[0].info = sSpellCustomizations.GetSpellInfo(SP_CLEAVE);
        spells[0].targettype = TARGET_ATTACKING;
        spells[0].instant = true;
        spells[0].perctrigger = 0.0f;
        spells[0].attackstoptimer = 1000;

        spells[1].info = sSpellCustomizations.GetSpellInfo(SP_RUST);
        spells[1].targettype = TARGET_SELF;
        spells[1].instant = true;
        spells[1].cooldown = 60;
        spells[1].perctrigger = 0.0f;
        spells[1].attackstoptimer = 1000;*/
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(1985);     // I could really use a heart. Say, can I have yours?

        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->Despawn(1, 0);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        sendDBChatMessage(1986);     // Back to being an old rust bucket.

        //Check to see if we can spawn The Crone now
        Creature* Dorothee = getNearestCreature(-10897.650f, -1755.8311f, 90.476f, 17535);    //Dorothee
        Creature* Strawman = getNearestCreature(-10904.088f, -1754.8988f, 90.476f, 17543);    //Strawman
        Creature* Roar = getNearestCreature(-10891.115f, -1756.4898f, 90.476f, 17546);    //Roar
        Creature* Tinman = getNearestCreature(-10884.501f, -1757.3249f, 90.476f, 17547);    //Tinman

        if ((Dorothee == NULL || Dorothee->IsDead()) && (Strawman == NULL || Strawman->IsDead()) && (Roar == NULL || Roar->IsDead()) && (Tinman == NULL || Tinman->IsDead()))
        {
            spawnCreature(18168, -10884.501f, -1757.3249f, 90.476f, 0.0f);
        }
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(1987);     // Guess I'm not so rusty after all.
    }
};

const uint32 CN_ROAR = 17546;

class RoarAI : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(RoarAI);

    RoarAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(1979);     // I'm not afraid a' you! Do you wanna' fight? Huh, do ya'? C'mon! I'll fight ya' with both paws behind my back!
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->Despawn(1, 0);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        sendDBChatMessage(1980);     // You didn't have to go and do that!

        //Check to see if we can spawn The Crone now
        Creature* Dorothee = getNearestCreature(-10897.650f, -1755.8311f, 90.476f, 17535); //Dorothee
        Creature* Strawman = getNearestCreature(-10904.088f, -1754.8988f, 90.476f, 17543); //Strawman
        Creature* Roar = getNearestCreature(-10891.115f, -1756.4898f, 90.476f, 17546);//Roar
        Creature* Tinman = getNearestCreature(-10884.501f, -1757.3249f, 90.476f, 17547); //Tinman

        if ((Dorothee == NULL || Dorothee->IsDead()) && (Strawman == NULL || Strawman->IsDead()) && (Roar == NULL || Roar->IsDead()) && (Tinman == NULL || Tinman->IsDead()))
        {
            spawnCreature(18168, -10884.501f, -1757.3249f, 90.476f, 0.0f);
        }
    }
};

const uint32 CN_CRONE = 18168;

const uint32 SP_SUMMON_CYCLONE = 38337;
const uint32 SP_CHAIN_LIGHTNING = 32337;

class CroneAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CroneAI);
    CroneAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        /*spells[0].info = sSpellCustomizations.GetSpellInfo(SP_SUMMON_CYCLONE);
        spells[0].targettype = TARGET_DESTINATION;
        spells[0].instant = true;
        spells[0].perctrigger = 5.0f;
        spells[0].attackstoptimer = 1000;

        spells[1].info = sSpellCustomizations.GetSpellInfo(SP_CHAIN_LIGHTNING);
        spells[1].targettype = TARGET_ATTACKING;
        spells[1].instant = true;
        spells[1].perctrigger = 10.0f;
        spells[1].attackstoptimer = 1000;*/
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(1989);     // Woe to each and every one of you, my pretties!

        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        if (getCreature()->GetHealthPct() > 0)
        {
            GameObject* DoorLeft = getNearestGameObject(-10917.1445f, -1774.05f, 90.478f, 184279);
            GameObject* DoorRight = getNearestGameObject(-10872.195f, -1779.42f, 90.45f, 184278);
            GameObject* Curtain = getNearestGameObject(-10894.17f, -1774.218f, 90.477f, 183932);

            if (DoorLeft)
                DoorLeft->SetState(GO_STATE_CLOSED);

            if (DoorRight)
                DoorRight->SetState(GO_STATE_OPEN);

            if (Curtain)
                Curtain->SetState(GO_STATE_CLOSED);
        }

        getCreature()->Despawn(1, 0);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        sendDBChatMessage(1991);     // How could you? What a cruel, cruel world...

        GameObject* DoorLeft = getNearestGameObject(-10917.1445f, -1774.05f, 90.478f, 184279);
        GameObject* DoorRight = getNearestGameObject(-10872.195f, -1779.42f, 90.45f, 184278);
        GameObject* Curtain = getNearestGameObject(-10894.17f, -1774.218f, 90.477f, 183932);

        if (DoorLeft)
            DoorLeft->SetState(GO_STATE_OPEN);

        if (DoorRight)
            DoorRight->SetState(GO_STATE_OPEN);

        // Make sure the curtain stays up
        if (Curtain)
            Curtain->SetState(GO_STATE_OPEN);
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(1992);     // Fixed you, didn't I?
    }
};

const uint32 CN_CYCLONEOZ = 22104;
const uint32 CYCLONE_VISUAL = 32332;
const uint32 CYCLONE_KNOCK = 38517;

class CycloneOZ : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CycloneOZ);
    CycloneOZ(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(CYCLONE_VISUAL), true);
        getCreature()->setUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
        _setMeleeDisabled(true);
        getCreature()->GetAIInterface()->m_canMove = false;
        getCreature()->m_noRespawn = true;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
    }

    void AIUpdate() override
    {
        getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(CYCLONE_KNOCK), true);
    }
};

//Romulo & Julianne
const uint32 CN_ROMULO = 17533;

const uint32 SP_BACKWARD_LUNGE = 30815;
const uint32 SP_DEADLY_SWATHE = 30817;
const uint32 SP_POISONED_THRUST = 30822;
const uint32 SP_DARING = 30841;

//\todo play sound on resurection
//sendDBChatMessage(2005);     // Thou detestable maw, thou womb of death; I enforce thy rotten jaws to open!

class RomuloAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(RomuloAI);
    RomuloAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        /*spells[0].info = sSpellCustomizations.GetSpellInfo(SP_BACKWARD_LUNGE);
        spells[0].targettype = TARGET_ATTACKING;
        spells[0].instant = false;
        spells[0].cooldown = 12;
        spells[0].perctrigger = 20.0f;
        spells[0].attackstoptimer = 1000;

        spells[1].info = sSpellCustomizations.GetSpellInfo(SP_DEADLY_SWATHE);
        spells[1].targettype = TARGET_ATTACKING;
        spells[1].instant = false;
        spells[1].cooldown = 0;
        spells[1].perctrigger = 20.0f;
        spells[1].attackstoptimer = 1000;

        spells[2].info = sSpellCustomizations.GetSpellInfo(SP_POISONED_THRUST);
        spells[2].targettype = TARGET_ATTACKING;
        spells[2].instant = false;
        spells[2].cooldown = 0;
        spells[2].perctrigger = 20.0f;
        spells[2].attackstoptimer = 1000;

        spells[3].info = sSpellCustomizations.GetSpellInfo(SP_DARING);
        spells[3].targettype = TARGET_SELF;
        spells[3].instant = false;
        spells[3].cooldown = 0;
        spells[3].perctrigger = 20.0f;
        spells[3].attackstoptimer = 1000;*/
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(2002);     // Wilt thou provoke me? Then have at thee, boy!

        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        if (getCreature()->GetHealthPct() > 0)
        {
            GameObject* DoorLeft = getNearestGameObject(-10917.1445f, -1774.05f, 90.478f, 184279);
            GameObject* DoorRight = getNearestGameObject(-10872.195f, -1779.42f, 90.45f, 184278);
            GameObject* Curtain = getNearestGameObject(-10894.17f, -1774.218f, 90.477f, 183932);

            if (DoorLeft)
                DoorLeft->SetState(GO_STATE_CLOSED);

            if (DoorRight)
                DoorRight->SetState(GO_STATE_OPEN);

            if (Curtain)
                Curtain->SetState(GO_STATE_CLOSED);
        }

        getCreature()->Despawn(1, 0);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        switch (Util::getRandomUInt(1))
        {
            case 0:
                sendDBChatMessage(2003);     // Thou smilest... upon the stroke that... murders me.
                break;
            case 1:
                sendDBChatMessage(2004);     // This day's black fate on more days doth depend. This but begins the woe. Others must end.
                break;
        }

        GameObject* DoorLeft = getNearestGameObject(-10917.1445f, -1774.05f, 90.478f, 184279);
        GameObject* DoorRight = getNearestGameObject(-10872.195f, -1779.42f, 90.45f, 184278);
        GameObject* Curtain = getNearestGameObject(-10894.17f, -1774.218f, 90.477f, 183932);

        if (DoorLeft)
            DoorLeft->SetState(GO_STATE_OPEN);

        if (DoorRight)
            DoorRight->SetState(GO_STATE_OPEN);

        // Make sure the curtain stays up
        if (Curtain)
            Curtain->SetState(GO_STATE_OPEN);

    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(2006);     // How well my comfort is revived by this!
    }
};

const uint32 CN_JULIANNE = 17534;

const uint32 SP_ETERNAL_AFFECTION = 30878;
const uint32 SP_POWERFUL_ATTRACTION = 30889;
const uint32 SP_BINDING_PASSION = 30890;
const uint32 SP_DEVOTION = 30887;

//\todo play sound on resurrection
//sendDBChatMessage(2000);     // Come, gentle night; and give me back my Romulo!

class JulianneAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(JulianneAI);
    JulianneAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        /*spells[0].info = sSpellCustomizations.GetSpellInfo(SP_ETERNAL_AFFECTION);
        spells[0].targettype = TARGET_SELF;
        spells[0].instant = false;
        spells[0].cooldown = 12;
        spells[0].perctrigger = 5.0f;
        spells[0].attackstoptimer = 1000;

        spells[1].info = sSpellCustomizations.GetSpellInfo(SP_POWERFUL_ATTRACTION);
        spells[1].targettype = TARGET_ATTACKING;
        spells[1].instant = false;
        spells[1].cooldown = 0;
        spells[1].perctrigger = 5.0f;
        spells[1].attackstoptimer = 1000;

        spells[2].info = sSpellCustomizations.GetSpellInfo(SP_BINDING_PASSION);
        spells[2].targettype = TARGET_ATTACKING;
        spells[2].instant = false;
        spells[2].cooldown = 0;
        spells[2].perctrigger = 5.0f;
        spells[2].attackstoptimer = 1000;

        spells[3].info = sSpellCustomizations.GetSpellInfo(SP_DEVOTION);
        spells[3].targettype = TARGET_SELF;
        spells[3].instant = false;
        spells[3].cooldown = 0;
        spells[3].perctrigger = 5.0f;
        spells[3].attackstoptimer = 1000;*/
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(1996);     // What devil art thou, that dost torment me thus?

        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        if (getCreature()->GetHealthPct() > 0)
        {
            GameObject* DoorLeft = getNearestGameObject(-10917.1445f, -1774.05f, 90.478f, 184279);
            GameObject* DoorRight = getNearestGameObject(-10872.195f, -1779.42f, 90.45f, 184278);
            GameObject* Curtain = getNearestGameObject(-10894.17f, -1774.218f, 90.477f, 183932);

            if (DoorLeft)
                DoorLeft->SetState(GO_STATE_CLOSED);

            if (DoorRight)
                DoorRight->SetState(GO_STATE_OPEN);

            if (Curtain)
                Curtain->SetState(GO_STATE_CLOSED);
        }

        getCreature()->Despawn(1, 0);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        switch (Util::getRandomUInt(1))
        {
            case 0:
                sendDBChatMessage(1998);     // Romulo, I come! Oh... this do I drink to thee!
                break;
            case 1:
                sendDBChatMessage(1997);     // Where is my lord? Where is my Romulo?
                break;
        }

        //_unit->RemoveAllAuras();
        //_unit->setEmoteState(EMOTE_ONESHOT_EAT);
        //_unit->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        spawnCreature(17533, -10891.582f, -1755.5177f, 90.476f, 4.61f);
        //_unit->setEmoteState(EMOTE_STATE_DEAD);
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(2001);     // Parting is such sweet sorrow.
    }
};

void SetupKarazhan(ScriptMgr* mgr)
{
    Arcemu::Gossip::Script* KBerthold = new Berthold();
    mgr->register_creature_gossip(16153, KBerthold);

    mgr->register_creature_script(CN_ATTUMEN, &AttumenTheHuntsmanAI::Create);
    mgr->register_creature_script(CN_MIDNIGHT, &MidnightAI::Create);
    mgr->register_creature_script(CN_MOROES, &MoroesAI::Create);
    mgr->register_creature_script(CN_MAIDENOFVIRTUE, &MaidenOfVirtueAI::Create);

    // Opera event related
    mgr->register_creature_script(CN_BIGBADWOLF, &BigBadWolfAI::Create);
    mgr->register_creature_script(CN_ROMULO, &RomuloAI::Create);
    mgr->register_creature_script(CN_JULIANNE, &JulianneAI::Create);
    mgr->register_creature_script(19525, &StageLight::Create);

    Arcemu::Gossip::Script* KGrandMother = new GrandMother;
    Arcemu::Gossip::Script* KBarnes = new BarnesGS;
    mgr->register_creature_gossip(16812, KBarnes);
    mgr->register_creature_gossip(17603, KGrandMother);

    mgr->register_creature_script(16812, &BarnesAI::Create);

    //WoOz here... commented yet to be implemented - kamyn
    mgr->register_creature_script(CN_DOROTHEE, &DorotheeAI::Create);
    mgr->register_creature_script(CN_STRAWMAN, &StrawmanAI::Create);
    mgr->register_creature_script(CN_TINHEAD, &TinheadAI::Create);
    mgr->register_creature_script(CN_ROAR, &RoarAI::Create);
    mgr->register_creature_script(CN_TITO, &TitoAI::Create);
    mgr->register_creature_script(CN_CRONE, &CroneAI::Create);
    mgr->register_creature_script(CN_CYCLONEOZ, &CycloneOZ::Create);

    mgr->register_creature_script(CN_CURATOR, &CuratorAI::Create);
    mgr->register_creature_script(CN_ASTRALFLARE, &AstralFlareAI::Create);

    mgr->register_creature_script(CN_ILLHOOF, &IllhoofAI::Create);
    mgr->register_creature_script(CN_KILREK, &KilrekAI::Create);
    mgr->register_creature_script(CN_FIENDISH_IMP, &FiendishImpAI::Create);
    mgr->register_creature_script(CN_DEMONCHAINS, &DemonChains::Create);
    mgr->register_creature_script(CN_FPORTAL, &FiendPortal::Create);

    mgr->register_creature_script(SHADEOFARAN, &ShadeofAranAI::Create);
    mgr->register_creature_script(WATERELE, &WaterEleAI::Create);
    mgr->register_creature_script(SHADOWOFARAN, &ShadowofAranAI::Create);

    mgr->register_creature_script(CN_NETHERSPITE, &NetherspiteAI::Create);
    mgr->register_creature_script(CN_VOIDZONE, &VoidZoneAI::Create);

    mgr->register_creature_script(CN_MALCHEZAAR, &MalchezaarAI::Create);
    mgr->register_creature_script(CN_INFERNAL, &NetherInfernalAI::Create);
    mgr->register_creature_script(CN_DUMMY, &InfernalDummyAI::Create);
    mgr->register_creature_script(CN_AXES, &MAxesAI::Create);

    mgr->register_creature_script(CN_NIGHTBANE, &NightbaneAI::Create);
}
