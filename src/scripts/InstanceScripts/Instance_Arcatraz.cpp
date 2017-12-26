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

#include "Setup.h"
#include "Instance_Arcatraz.h"
#include "Objects/Faction.h"


// VOID_ZONE 36119    // DBC: 36119; it's not fully functionl without additional core support (for dmg and random place targeting).

// Zereketh the UnboundAI
class ZerekethAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ZerekethAI);
        ZerekethAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto seedOfC = addAISpell(SEED_OF_C, 6.0f, TARGET_RANDOM_SINGLE, 2, 20);
            seedOfC->setMinMaxDistance(0.0f, 100.0f);

            SpeechTimer = 0;
            VoidTimer = 0;

            if (!_isHeroic())
                addAISpell(SHADOW_NOVA, 15.0f, TARGET_SELF, 2, 15);
            else
                addAISpell(SHADOW_NOVA_H, 15.0f, TARGET_SELF, 2, 15);

            addEmoteForEvent(Event_OnCombatStart, 5496);     // Life energy to... consume.
            addEmoteForEvent(Event_OnTargetDied, 5497);      // This vessel is empty.
            addEmoteForEvent(Event_OnTargetDied, 5498);      // No... more... life.
            addEmoteForEvent(Event_OnDied, 5501);            // The Void... beckons.
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            VoidTimer = _addTimer((Util::getRandomUInt(10) + 30) * 1000);
            SpeechTimer = _addTimer((Util::getRandomUInt(10) + 40) * 1000);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            //despawn voids
            for (const auto& itr : getCreature()->getInRangeObjectsSet())
            {
                if (itr)
                {
                    Object* obj = itr;
                    if (obj->IsCreature())
                    {
                        auto creature = static_cast<Creature*>(obj);

                        if (creature->GetCreatureProperties()->Id == 21101 && creature->isAlive())
                        {
                            creature->Despawn(0, 0);
                        }
                    }
                }
            }
        }

        void Speech()
        {
            switch (Util::getRandomUInt(1))
            {
                case 0:
                    sendDBChatMessage(SAY_ZEREKETH_01);
                    break;
                case 1:
                    sendDBChatMessage(SAY_ZEREKETH_02);
                    break;
            }
            _resetTimer(SpeechTimer, (Util::getRandomUInt(10) + 40) * 1000);
        }

        void VoidZoneArc()
        {
            _resetTimer(VoidTimer, (Util::getRandomUInt(10) + 30) * 1000);

            std::vector<Player*> TargetTable;
            for (const auto& itr : getCreature()->getInRangePlayersSet())
            {
                if (!itr || !itr->IsPlayer())
                    continue;

                Player* RandomTarget = static_cast<Player*>(itr);
                if (RandomTarget->isAlive() && isHostile(itr, getCreature()))
                    TargetTable.push_back(RandomTarget);
            }

            if (!TargetTable.size())
                return;

            auto random_index = Util::getRandomUInt(0, uint32(TargetTable.size() - 1));
            auto random_target = TargetTable[random_index];

            if (random_target == nullptr)
                return;

            float vzX = Util::getRandomUInt(5) * cos(Util::getRandomFloat(6.28f)) + random_target->GetPositionX();
            float vzY = Util::getRandomUInt(5) * cos(Util::getRandomFloat(6.28f)) + random_target->GetPositionY();
            float vzZ = random_target->GetPositionZ();

            Creature* VoidZone = spawnCreature(CN_VOIDZONEARC, vzX, vzY, vzZ, 0.0f);
            VoidZone->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
            VoidZone->m_noRespawn = true;
            if (!VoidZone->IsInWorld())
            {
                VoidZone->Despawn(0, 0);
                return;
            }
            random_target = NULL;
            VoidZone->Despawn(60000, 0);
        }

        void AIUpdate() override
        {
            if (_isTimerFinished(SpeechTimer))
                Speech();

            if (_isTimerFinished(VoidTimer))
                VoidZoneArc();
        }

    protected:

        int32 SpeechTimer;
        int32 VoidTimer;
};

class VoidZoneARC : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(VoidZoneARC);
        VoidZoneARC(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            stopMovement();
            setRooted(true);
            setCanEnterCombat(false);
            RegisterAIUpdateEvent(1000);
        }

        void AIUpdate() override
        {
            // M4ksiu: I'm not sure if it should be cast once, on start
            uint32 SpellId = CONSUMPTION;
            if (_isHeroic())
                SpellId = CONSUMPTION_H;

            getCreature()->CastSpell(getCreature(), SpellId, true);
            RemoveAIUpdateEvent();
        }
};


// Dalliah the DoomsayerAI
// sounds missing related to Wrath... (look on script below this one)
class DalliahTheDoomsayerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(DalliahTheDoomsayerAI);
        DalliahTheDoomsayerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(GIFT_OF_THE_DOOMSAYER, 8.0f, TARGET_ATTACKING);

            auto whirlTemp = addAISpell(WHIRLWIND, 15.0f, TARGET_SELF);
            whirlTemp->addEmote("Reap the Whirlwind!", CHAT_MSG_MONSTER_YELL, 11089);
            whirlTemp->addEmote("I'll cut you to peices!", CHAT_MSG_MONSTER_YELL, 11090);

            auto healTemp = addAISpell(HEAL, 8.0f, TARGET_SELF);
            healTemp->addEmote("That is much better.", CHAT_MSG_MONSTER_YELL, 11091);
            healTemp->addEmote("Ah, just what I needed.", CHAT_MSG_MONSTER_YELL, 11092);

            if (_isHeroic())
                addAISpell(SHADOW_WAVE, 8.0f, TARGET_ATTACKING);

            addEmoteForEvent(Event_OnCombatStart, 7368);    // It is unwise to anger me!
            addEmoteForEvent(Event_OnTargetDied, 7369);     // Completely ineffective.  Just like someone else I know.
            addEmoteForEvent(Event_OnTargetDied, 7370);     // You chose the wrong opponent.
            addEmoteForEvent(Event_OnDied, 7375);           // Now I'm really angry.
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            GameObject* door2 = getNearestGameObject(184319);
            if (door2 != NULL)
                door2->SetState(GO_STATE_OPEN);
        }
};

// Wrath-Scryer SoccothratesAI
// \todo Add moar sounds
// CHARGE_TARGETING 36038 ?
// There are more sounds connected with Dalliah and some spells, but I don't know situation in which they are used
// so haven't added them.
class WrathScryerSoccothratesAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(WrathScryerSoccothratesAI);
        WrathScryerSoccothratesAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(IMMOLATION, 10.0f, TARGET_SELF);
            addAISpell(FELFIRE_SHOCK, 8.0f, TARGET_ATTACKING);
            addAISpell(FELFIRE_LINE_UP, 8.0f, TARGET_SELF);
            addAISpell(KNOCK_AWAY, 6.0f, TARGET_RANDOM_DESTINATION);
            addAISpell(CHARGE, 4.0f, TARGET_ATTACKING);

            addEmoteForEvent(Event_OnCombatStart, 7365);    // At last, a target for my frustrations!
            addEmoteForEvent(Event_OnTargetDied, 7364);     // Yes, that was quite satisfying.
            addEmoteForEvent(Event_OnTargetDied, 8753);     // Ha! Much better!
            addEmoteForEvent(Event_OnDied, 7380);           // Knew this was... the only way out.
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            GameObject* door1 = getNearestGameObject(199.969f, 118.5837f, 22.379f, 184318);
            if (door1 != NULL)
                door1->SetState(GO_STATE_OPEN);
        }
};

// Harbinger SkyrissAI
// Full event must be scripted for this guy.
// BLINK_VISUAL 36937 ?
// SIMPLE_TELEPORT 12980 ?
// Add sounds related to his dialog with mind controlled guy
class HarbringerSkyrissAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HarbringerSkyrissAI);
        HarbringerSkyrissAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(MIND_REND, 15.0f, TARGET_ATTACKING);

            auto fear = addAISpell(FEAR, 8.0f, TARGET_ATTACKING);
            fear->addEmote("Flee in terror!", CHAT_MSG_MONSTER_YELL, 11129);
            fear->addEmote("I will show you horrors undreamed of.", CHAT_MSG_MONSTER_YELL, 11130);

            auto domination = addAISpell(DOMINATION, 6.0f, TARGET_ATTACKING);
            domination->addEmote("You will do my bidding, weakling.", CHAT_MSG_MONSTER_YELL, 11127);
            domination->addEmote("Your will is no longer your own.", CHAT_MSG_MONSTER_YELL, 11128);

            Illusion66 = addAISpell(SUMMON_ILLUSION_66, 0.0f, TARGET_SELF);
            Illusion66->addEmote("", CHAT_MSG_MONSTER_YELL, 11131);

            Illusion33 = addAISpell(SUMMON_ILLUSION_33, 0.0f, TARGET_SELF);
            Illusion33->addEmote("", CHAT_MSG_MONSTER_YELL, 11131);

            IllusionCount = 0;

            addEmoteForEvent(Event_OnCombatStart, 5034);    // Bear witness to the agent of your demise!
            addEmoteForEvent(Event_OnTargetDied, 5035);     // Your fate is written.
            addEmoteForEvent(Event_OnTargetDied, 5036);     // The chaos I have sown here is but a taste....
            addEmoteForEvent(Event_OnDied, 5042);           // I am merely one of... infinite multitudes.
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            IllusionCount = 0;
        }

        void AIUpdate() override
        {
            if (_getHealthPercent() <= 66 && IllusionCount == 0)
            {
                IllusionCount = 1;
                _castAISpell(Illusion66);
            }
            else if (_getHealthPercent() <= 33 && IllusionCount == 1)
            {
                IllusionCount = 2;
                _castAISpell(Illusion33);
            }
        }

    protected:

        uint8 IllusionCount;
        CreatureAISpells* Illusion66;
        CreatureAISpells* Illusion33;
};


// Warden MellicharAI
class WardenMellicharAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(WardenMellicharAI);
        WardenMellicharAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            setRooted(true);
            Phase_Timer = -1;
            Spawncounter = 0;
            NPC_orb1 = NULL;
            NPC_orb2 = NULL;
            NPC_orb3 = NULL;
            NPC_orb4 = NULL;
            NPC_orb5 = NULL;
            shield = NULL;
            orb1 = NULL;
            orb2 = NULL;
            orb3 = NULL;
            orb4 = NULL;
            Phasepart = 0;
            NPC_ID_Spawn = 0;

            addEmoteForEvent(Event_OnCombatStart, SAY_MELLICHAR_01);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            Phasepart = 0;
            setRooted(true);
            Phase_Timer = _addTimer(55000);

            setCanEnterCombat(false);
            getCreature()->SetEmoteState(EMOTE_ONESHOT_READY1H); // to be replaced for the standstate

            shield = getNearestGameObject(445.786f, -169.263f, 43.0466f, 184802);
            if (shield)
                shield->SetState(GO_STATE_CLOSED);

            getCreature()->SendTimedScriptTextChatMessage(SAY_MELLICHAR_02, 27000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            Reset_Event();
        }

        void AIUpdate() override
        {
            setCanEnterCombat(false);
            setRooted(true);
            _setMeleeDisabled(true);
            _setCastDisabled(true);

            // ORB ONE
            if (_isTimerFinished(Phase_Timer) && isScriptPhase(1))
            {
                if (Phasepart == 0)
                {
                    Spawncounter = 0;
                    orb1 = getNearestGameObject(482.929f, -151.114f, 43.654f, 183961);
                    if (orb1)
                        orb1->SetState(GO_STATE_OPEN);

                    switch (Util::getRandomUInt(1))
                    {
                        NPC_ID_Spawn = 0;
                        case 0:
                            NPC_ID_Spawn = CN_BLAZING_TRICKSTER;
                            break;
                        case 1:
                            NPC_ID_Spawn = CN_WARP_STALKER;
                            break;
                    }
                    _resetTimer(Phase_Timer, 8000);
                    Phasepart = 1;
                    return;
                }

                if (Phasepart == 1)
                {
                    if (!NPC_orb1 && NPC_ID_Spawn != 0 && Spawncounter == 0)
                    {
                        ++Spawncounter;
                        NPC_orb1 = spawnCreature(NPC_ID_Spawn, 475.672f, -147.086f, 42.567f, 3.184015f);
                        return;
                    }

                    if (NPC_orb1 && !NPC_orb1->isAlive())
                    {
                        sendDBChatMessage(SAY_MELLICHAR_03);
                        setScriptPhase(2);
                        Phasepart = 0;
                        _resetTimer(Phase_Timer, 6000);
                        return;
                    }
                }
            }

            // ORB TWO
            else if (_isTimerFinished(Phase_Timer) && isScriptPhase(2))
            {
                if (Phasepart == 0)
                {
                    Spawncounter = 0;
                    orb2 = getNearestGameObject(409.062f, -152.161f, 43.653f, 183963);
                    if (orb2)
                        orb2->SetState(GO_STATE_OPEN);

                    _resetTimer(Phase_Timer, 8000);
                    Phasepart = 1;
                    return;
                }

                if (Phasepart == 1)
                {
                    if (!NPC_orb2 && Spawncounter == 0)
                    {
                        ++Spawncounter;
                        NPC_orb2 = spawnCreature(CN_MILLHOUSE_MANASTORM, 413.192f, -148.586f, 42.569f, 0.024347f);
                        return;
                    }

                    if (NPC_orb2 && NPC_orb2->isAlive())
                    {
                        Creature* millhouse = getNearestCreature(CN_MILLHOUSE_MANASTORM);
                        if (millhouse)
                        {
                            millhouse->SendTimedScriptTextChatMessage(SAY_MILLHOUS_01, 2000);

                            getCreature()->SendTimedScriptTextChatMessage(SAY_MELLICHAR_04, 13000);

                            millhouse->SendTimedScriptTextChatMessage(SAY_MILLHOUS_02, 22000);
                        }
                        setScriptPhase(3);
                        Phasepart = 0;
                        _resetTimer(Phase_Timer, 25000);
                        return;
                    }
                }
            }

            // ORB THREE
            else if (_isTimerFinished(Phase_Timer) && isScriptPhase(3))
            {
                if (Phasepart == 0)
                {
                    Spawncounter = 0;
                    orb3 = getNearestGameObject(415.167f, -174.338f, 43.654f, 183964);
                    if (orb3)
                        orb3->SetState(GO_STATE_OPEN);

                    switch (Util::getRandomUInt(1))
                    {
                        NPC_ID_Spawn = 0;
                        case 0:
                            NPC_ID_Spawn = CN_SULFURON_MAGMA_THROWER;
                            break;
                        case 1:
                            NPC_ID_Spawn = CN_AKKIRIS_LIGHTNING_WAKER;
                            break;
                    }
                    _resetTimer(Phase_Timer, 8000);
                    Phasepart = 1;
                    return;
                }

                if (Phasepart == 1)
                {
                    if (!NPC_orb3 && NPC_ID_Spawn != 0 && Spawncounter == 0)
                    {
                        /// \todo investigate.... saying "1"... really?
                        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "1");
                        ++Spawncounter;
                        NPC_orb3 = spawnCreature(NPC_ID_Spawn, 420.050f, -173.500f, 42.580f, 6.110f);
                        return;
                    }

                    if (!NPC_orb3)
                    {
                        /// \todo investigate.... saying "2"... really?
                        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "2");
                        NPC_orb3 = getNearestCreature(NPC_ID_Spawn);
                    }
                    else if (NPC_orb3 && !NPC_orb3->isAlive())
                    {
                        sendDBChatMessage(SAY_MELLICHAR_05);
                        setScriptPhase(4);
                        Phasepart = 0;
                        _resetTimer(Phase_Timer, 8000);
                        return;
                    }
                    else
                    {
                        return;
                    }
                }
            }

            // ORB FOUR
            else if (_isTimerFinished(Phase_Timer) && isScriptPhase(4))
            {
                if (Phasepart == 0)
                {
                    Spawncounter = 0;
                    orb4 = getNearestGameObject(476.422f, -174.517f, 42.748f, 183962);
                    if (orb4)
                        orb4->SetState(GO_STATE_OPEN);

                    switch (Util::getRandomUInt(1))
                    {
                        NPC_ID_Spawn = 0;
                        case 0:
                            NPC_ID_Spawn = CN_TWILIGHT_DRAKONAAR;
                            break;
                        case 1:
                            NPC_ID_Spawn = CN_BLACKWING_DRAKONAAR;
                            break;
                    }
                    _resetTimer(Phase_Timer, 8000);
                    Phasepart = 1;
                    return;
                }

                if (Phasepart == 1)
                {
                    if (!NPC_orb4 && NPC_ID_Spawn != 0 && Spawncounter == 0)
                    {
                        ++Spawncounter;
                        NPC_orb4 = spawnCreature(NPC_ID_Spawn, 471.153f, -174.715f, 42.589f, 3.097f);
                        return;
                    }

                    if (!NPC_orb4)
                    {
                        NPC_orb4 = getNearestCreature(NPC_ID_Spawn);
                    }
                    else if (NPC_orb4 && !NPC_orb4->isAlive())
                    {
                        sendDBChatMessage(SAY_MELLICHAR_06);
                        setScriptPhase(5);
                        Phasepart = 0;
                        _resetTimer(Phase_Timer, 6000);
                        return;
                    }
                    else
                    {
                        return;
                    }
                }
            }

            setRooted(true);
            _setMeleeDisabled(true);
            _setCastDisabled(true);
        }

        void Reset_Event()
        {
            setRooted(false);
            _setMeleeDisabled(false);
            _setCastDisabled(false);
            getCreature()->SetStandState(STANDSTATE_KNEEL);

            if (shield)
                shield->SetState(GO_STATE_OPEN);

            if (orb1)
                orb1->SetState(GO_STATE_CLOSED);

            if (orb2)
                orb2->SetState(GO_STATE_CLOSED);

            if (orb3)
                orb3->SetState(GO_STATE_CLOSED);

            if (orb4)
                orb4->SetState(GO_STATE_CLOSED);

            if (NPC_orb1)
            {
                NPC_orb1->Despawn(0, 0);
                NPC_orb1 = NULL;
            }

            if (NPC_orb2)
            {
                NPC_orb2->Despawn(0, 0);
                NPC_orb2 = NULL;
            }

            if (NPC_orb3)
            {
                NPC_orb3->Despawn(0, 0);
                NPC_orb3 = NULL;
            }

            if (NPC_orb4)
            {
                NPC_orb4->Despawn(0, 0);
                NPC_orb4 = NULL;
            }

            if (NPC_orb5)
            {
                NPC_orb5->Despawn(0, 0);
                NPC_orb5 = NULL;
            }
        }

    protected:

        uint8 Phasepart;
        uint32 NPC_ID_Spawn;
        uint32 Spawncounter;
        int32 Phase_Timer;

        Creature* NPC_orb1;
        Creature* NPC_orb2;
        Creature* NPC_orb3;
        Creature* NPC_orb4;
        Creature* NPC_orb5;
        GameObject* shield;
        GameObject* orb1;
        GameObject* orb2;
        GameObject* orb3;
        GameObject* orb4;
};

void SetupArcatraz(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_ZEREKETH, &ZerekethAI::Create);
    mgr->register_creature_script(CN_VOIDZONEARC, &VoidZoneARC::Create);

    mgr->register_creature_script(CN_DALLIAH_THE_DOOMSAYER, &DalliahTheDoomsayerAI::Create);
    mgr->register_creature_script(CN_WRATH_SCRYER_SOCCOTHRATES, &WrathScryerSoccothratesAI::Create);
    mgr->register_creature_script(CN_HARBRINGER_SKYRISS, &HarbringerSkyrissAI::Create);
    //mgr->register_creature_script(CN_WARDEN_MELLICHAR, &WardenMellicharAI::Create);
}
