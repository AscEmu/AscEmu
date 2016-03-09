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

//////////////////////////////////////////////////////////////////////////////////////////
//Tempest Keep: The Arcatraz
class InstanceTheArcatrazScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceTheArcatrazScript, MoonInstanceScript);
        InstanceTheArcatrazScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
        {
            // Way to select bosses
            BuildEncounterMap();
            if (mEncounters.size() == 0)
                return;

            for (EncounterMap::iterator Iter = mEncounters.begin(); Iter != mEncounters.end(); ++Iter)
            {
                if ((*Iter).second.mState != State_Finished)
                    continue;
            }
        }

        void OnGameObjectPushToWorld(GameObject* pGameObject) { }

        void SetInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = (EncounterState)pData;
        }

        uint32 GetInstanceData(uint32 pType, uint32 pIndex)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return 0;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return 0;

            return (*Iter).second.mState;
        }

        void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
        {
            EncounterMap::iterator Iter = mEncounters.find(pCreature->GetEntry());
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = State_Finished;

            return;
        }
};

// #define VOID_ZONE 36119    // DBC: 36119; it's not fully functionl without additional core support (for dmg and random place targeting).

// Zereketh the UnboundAI
class ZerekethAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(ZerekethAI, MoonScriptBossAI);
        ZerekethAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            AddSpell(SEED_OF_C, Target_RandomPlayer, 6.0f, 2, 20, 0, 100.0f);

            SpeechTimer = 0;
            VoidTimer = 0;

            if (!IsHeroic())
                AddSpell(SHADOW_NOVA, Target_Self, 15, 2, 15);
            else
                AddSpell(SHADOW_NOVA_H, Target_Self, 15, 2, 15);

        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(5496);     // Life energy to... consume.

            VoidTimer = AddTimer((RandomUInt(10) + 30) * 1000);
            SpeechTimer = AddTimer((RandomUInt(10) + 40) * 1000);

            ParentClass::OnCombatStart(mTarget);
        }

        void OnTargetDied(Unit* mKiller)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(5497);     // This vessel is empty.
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(5498);     // No... more... life.
                    break;
            }
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(5501);     // The Void... beckons.

            //despawn voids
            Creature* creature = NULL;
            for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd();)
            {
                Object* obj = *itr;
                ++itr;
                if (obj->IsCreature())
                {
                    creature = static_cast<Creature*>(obj);

                    if (creature->GetCreatureInfo()->Id == 21101 && creature->isAlive())
                    {
                        creature->Despawn(0, 0);
                    }
                }
            }

            ParentClass::OnDied(mKiller);
        }

        void Speech()
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_ZEREKETH_01);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_ZEREKETH_02);
                    break;
            }
            ResetTimer(SpeechTimer, (RandomUInt(10) + 40) * 1000);
        }

        void VoidZoneArc()
        {
            ResetTimer(VoidTimer, (RandomUInt(10) + 30) * 1000);

            std::vector<Player*> TargetTable;
            std::set< Object* >::iterator Itr = _unit->GetInRangePlayerSetBegin();
            for (; Itr != _unit->GetInRangePlayerSetEnd(); ++Itr)
            {
                Player* RandomTarget = NULL;
                if (!(*Itr)->IsPlayer())
                    continue;
                RandomTarget = static_cast< Player* >(*Itr);
                if (RandomTarget && RandomTarget->isAlive() && isHostile(*Itr, _unit))
                    TargetTable.push_back(RandomTarget);
            }

            if (!TargetTable.size())
                return;

            auto random_index = RandomUInt(0, TargetTable.size() - 1);
            auto random_target = TargetTable[random_index];

            if (random_target == nullptr)
                return;

            float vzX = RandomUInt(5) * cos(RandomFloat(6.28f)) + random_target->GetPositionX();
            float vzY = RandomUInt(5) * cos(RandomFloat(6.28f)) + random_target->GetPositionY();
            float vzZ = random_target->GetPositionZ();
            MoonScriptCreatureAI* VoidZone = SpawnCreature(CN_VOIDZONEARC, vzX, vzY, vzZ);
            VoidZone->GetUnit()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
            VoidZone->GetUnit()->m_noRespawn = true;
            if (!VoidZone->GetUnit()->IsInWorld())
            {
                VoidZone->Despawn();
                return;
            }
            random_target = NULL;
            VoidZone->Despawn(60000, 0);
        }

        void AIUpdate()
        {
            if (IsTimerFinished(SpeechTimer))
                Speech();

            if (IsTimerFinished(VoidTimer))
                VoidZoneArc();

            ParentClass::AIUpdate();
        }

    protected:

        int32 SpeechTimer;
        int32 VoidTimer;
};

class VoidZoneARC : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(VoidZoneARC, MoonScriptCreatureAI);
        VoidZoneARC(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            StopMovement();
            SetCanMove(false);
            SetCanEnterCombat(false);
            RegisterAIUpdateEvent(1000);
        }

        void AIUpdate()
        {
            // M4ksiu: I'm not sure if it should be cast once, on start
            uint32 SpellId = CONSUMPTION;
            if (IsHeroic())
                SpellId = CONSUMPTION_H;

            _unit->CastSpell(_unit, SpellId, true);
            RemoveAIUpdateEvent();
        }
};


// Dalliah the DoomsayerAI
// sounds missing related to Wrath... (look on script below this one)
class DalliahTheDoomsayerAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(DalliahTheDoomsayerAI, MoonScriptBossAI);
        DalliahTheDoomsayerAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            AddSpell(GIFT_OF_THE_DOOMSAYER, Target_Current, 8.0f, 0.0f, -1);

            SpellDesc* WhirlTemp = AddSpell(WHIRLWIND, Target_Self, 15.0f, 0.0f, -1);
            WhirlTemp->AddEmote("Reap the Whirlwind!", Text_Yell, 11089);
            WhirlTemp->AddEmote("I'll cut you to peices!", Text_Yell, 11090);

            SpellDesc* HealTemp = AddSpell(HEAL, Target_Self, 8.0f, 0, -1);
            HealTemp->AddEmote("That is much better.", Text_Yell, 11091);
            HealTemp->AddEmote("Ah, just what I needed.", Text_Yell, 11092);

            if (IsHeroic())
                AddSpell(SHADOW_WAVE, Target_Current, 8.0f, 0, -1);

        }

        void OnEnterCombat(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(7368);     // It is unwise to anger me!
        }

        void OnTargetDied(Unit* mKiller)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(7369);     // Completely ineffective.  Just like someone else I know.
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(7370);     // You chose the wrong opponent.
                    break;
            }
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(7375);     // Now I'm really angry.

            GameObject* door2 = NULL;
            door2 = GetNearestGameObject(184319);
            if (door2 != NULL)
                door2->SetState(GO_STATE_OPEN);

            ParentClass::OnDied(mKiller);
        }

};

// Wrath-Scryer SoccothratesAI
// \todo Add moar sounds
// CHARGE_TARGETING 36038 ?
// There are more sounds connected with Dalliah and some spells, but I don't know situation in which they are used
// so haven't added them.
class WrathScryerSoccothratesAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(WrathScryerSoccothratesAI, MoonScriptBossAI);
        WrathScryerSoccothratesAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            AddSpell(IMMOLATION, Target_Self, 10.0f, 0, -1);
            AddSpell(FELFIRE_SHOCK, Target_Current, 8.0f, 0, -1);
            AddSpell(FELFIRE_LINE_UP, Target_Self, 8.0f, 0, -1);
            AddSpell(KNOCK_AWAY, Target_Destination, 6.0f, 0, -1);
            AddSpell(CHARGE, Target_Current, 4.0f, 0, -1);
        }

        void OnCombatStart(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(7365);     // At last, a target for my frustrations!
        }

        void OnTargetDied(Unit* mKiller)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(7364);     // Yes, that was quite satisfying.
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(8753);     // Ha! Much better!
                    break;
            }
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(7380);     // Knew this was... the only way out.

            GameObject* door1 = NULL;
            door1 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(199.969f, 118.5837f, 22.379f, 184318);
            if (door1 != NULL)
                door1->SetState(GO_STATE_OPEN);

            ParentClass::OnDied(mKiller);
        }

};

// Harbinger SkyrissAI
// Full event must be scripted for this guy.
// BLINK_VISUAL 36937 ?
// SIMPLE_TELEPORT 12980 ?
// Add sounds related to his dialog with mind controlled guy
class HarbringerSkyrissAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(HarbringerSkyrissAI, MoonScriptBossAI);
        HarbringerSkyrissAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            AddSpell(MIND_REND, Target_Current, 15.0f, 0, -1);

            SpellDesc* Fear = AddSpell(FEAR, Target_Current, 8.0f, 0, -1);
            Fear->AddEmote("Flee in terror!", Text_Yell, 11129);
            Fear->AddEmote("I will show you horrors undreamed of.", Text_Yell, 11130);

            SpellDesc* Domination = AddSpell(DOMINATION, Target_Current, 6.0f, 0, -1);
            Domination->AddEmote("You will do my bidding, weakling.", Text_Yell, 11127);
            Domination->AddEmote("Your will is no longer your own.", Text_Yell, 11128);

            Illusion66 = AddSpell(SUMMON_ILLUSION_66, Target_Self, 0, 0, -1, 0, 0, false, "", Text_Yell, 11131);
            Illusion66->mEnabled = false;

            Illusion33 = AddSpell(SUMMON_ILLUSION_33, Target_Self, 0, 0, -1, 0, 0, false, "", Text_Yell, 11131);
            Illusion33->mEnabled = false;

            IllusionCount = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(5034);     // Bear witness to the agent of your demise!

            IllusionCount = 0;
            ParentClass::OnCombatStart(mTarget);
        }

        void OnTargetDied(Unit* mKiller)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(5035);     // Your fate is written.
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(5036);     // The chaos I have sown here is but a taste....
                    break;
            }
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(5042);     // I am merely one of... infinite multitudes.
        }

        void AIUpdate()
        {
            if (GetHealthPercent() <= 66 && IllusionCount == 0)
            {
                IllusionCount = 1;
                CastSpell(Illusion66);
            }
            else if (GetHealthPercent() <= 33 && IllusionCount == 1)
            {
                IllusionCount = 2;
                CastSpell(Illusion33);
            }
            ParentClass::AIUpdate();
        }

    protected:

        uint8 IllusionCount;
        SpellDesc* Illusion66;
        SpellDesc* Illusion33;
};


// Warden MellicharAI
class WardenMellicharAI : public MoonScriptBossAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(WardenMellicharAI, MoonScriptBossAI);
        WardenMellicharAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            SetCanMove(false);
            Phase_Timer = -1;
            SetPhase(0);
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
        }

        void OnCombatStart(Unit* mTarget)
        {
            SetPhase(0);
            Phasepart = 0;
            SetCanMove(false);
            Phase_Timer = AddTimer(55000);

            SetCanEnterCombat(false);
            _unit->SetEmoteState(EMOTE_ONESHOT_READY1H); // to be replaced for the standstate

            shield = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(445.786f, -169.263f, 43.0466f, 184802);
            if (shield)
                shield->SetState(GO_STATE_CLOSED);

            _unit->SendScriptTextChatMessage(SAY_MELLICHAR_01);
            _unit->SendTimedScriptTextChatMessage(SAY_MELLICHAR_02, 27000);

            ParentClass::OnCombatStart(mTarget);
        }

        void OnCombatStop(Unit* mTarget)
        {
            Reset_Event();
            ParentClass::OnCombatStop(mTarget);
        }

        void AIUpdate()
        {
            SetCanEnterCombat(false);
            SetCanMove(false);
            SetAllowMelee(false);
            SetAllowSpell(false);

            // ORB ONE
            if (IsTimerFinished(Phase_Timer) && GetPhase() == 0)
            {
                if (Phasepart == 0)
                {
                    Spawncounter = 0;
                    orb1 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(482.929f, -151.114f, 43.654f, 183961);
                    if (orb1)
                        orb1->SetState(GO_STATE_OPEN);

                    switch (RandomUInt(1))
                    {
                        NPC_ID_Spawn = 0;
                        case 0:
                            NPC_ID_Spawn = CN_BLAZING_TRICKSTER;
                            break;
                        case 1:
                            NPC_ID_Spawn = CN_WARP_STALKER;
                            break;
                    }
                    ResetTimer(Phase_Timer, 8000);
                    Phasepart = 1;
                    return;
                }

                else if (Phasepart == 1)
                {
                    if (!NPC_orb1 && NPC_ID_Spawn != 0 && Spawncounter == 0)
                    {
                        ++Spawncounter;
                        NPC_orb1 = SpawnCreature(NPC_ID_Spawn, 475.672f, -147.086f, 42.567f, 3.184015f);
                        return;
                    }
                    else if (NPC_orb1 && !NPC_orb1->IsAlive())
                    {
                        _unit->SendScriptTextChatMessage(SAY_MELLICHAR_03);
                        SetPhase(1);
                        Phasepart = 0;
                        ResetTimer(Phase_Timer, 6000);
                        return;
                    }
                    else
                    {
                        return;
                    }
                    return;
                }
                //return;
            }

            // ORB TWO
            else if (IsTimerFinished(Phase_Timer) && GetPhase() == 1)
            {
                if (Phasepart == 0)
                {
                    Spawncounter = 0;
                    orb2 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(409.062f, -152.161f, 43.653f, 183963);
                    if (orb2)
                        orb2->SetState(GO_STATE_OPEN);

                    ResetTimer(Phase_Timer, 8000);
                    Phasepart = 1;
                    return;
                }

                else if (Phasepart == 1)
                {
                    if (!NPC_orb2 && Spawncounter == 0)
                    {
                        ++Spawncounter;
                        NPC_orb2 = SpawnCreature(CN_MILLHOUSE_MANASTORM, 413.192f, -148.586f, 42.569f, 0.024347f);
                        return;
                    }
                    else if (NPC_orb2 && NPC_orb2->IsAlive())
                    {
                        Creature* millhouse = static_cast<Creature*>(ForceCreatureFind(CN_MILLHOUSE_MANASTORM));
                        if (millhouse)
                        {
                            millhouse->SendTimedScriptTextChatMessage(SAY_MILLHOUS_01, 2000);

                            _unit->SendTimedScriptTextChatMessage(SAY_MELLICHAR_04, 13000);

                            millhouse->SendTimedScriptTextChatMessage(SAY_MILLHOUS_02, 22000);
                        }
                        SetPhase(2);
                        Phasepart = 0;
                        ResetTimer(Phase_Timer, 25000);
                        return;
                    }
                    else
                    {
                        return;
                    }
                    return;

                }
                //return;
            }

            // ORB THREE
            else if (IsTimerFinished(Phase_Timer) && GetPhase() == 2)
            {
                if (Phasepart == 0)
                {
                    Spawncounter = 0;
                    orb3 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(415.167f, -174.338f, 43.654f, 183964);
                    if (orb3)
                        orb3->SetState(GO_STATE_OPEN);

                    switch (RandomUInt(1))
                    {
                        NPC_ID_Spawn = 0;
                        case 0:
                            NPC_ID_Spawn = CN_SULFURON_MAGMA_THROWER;
                            break;
                        case 1:
                            NPC_ID_Spawn = CN_AKKIRIS_LIGHTNING_WAKER;
                            break;
                    }
                    ResetTimer(Phase_Timer, 8000);
                    Phasepart = 1;
                    return;
                }

                else if (Phasepart == 1)
                {
                    if (!NPC_orb3 && NPC_ID_Spawn != 0 && Spawncounter == 0)
                    {
                        /// \todo investigate.... saying "1"... really?
                        _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "1");
                        ++Spawncounter;
                        NPC_orb3 = SpawnCreature(NPC_ID_Spawn, 420.050f, -173.500f, 42.580f, 6.110f);
                        return;
                    }
                    else if (!NPC_orb3)
                    {
                        /// \todo investigate.... saying "2"... really?
                        _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "2");
                        NPC_orb3 = GetNearestCreature(NPC_ID_Spawn);
                    }
                    else if (NPC_orb3 && !NPC_orb3->IsAlive())
                    {
                        _unit->SendScriptTextChatMessage(SAY_MELLICHAR_05);
                        SetPhase(3);
                        Phasepart = 0;
                        ResetTimer(Phase_Timer, 8000);
                        return;
                    }
                    else
                    {
                        return;
                    }
                    return;
                }
                //return;
            }

            // ORB FOUR
            else if (IsTimerFinished(Phase_Timer) && GetPhase() == 3)
            {
                if (Phasepart == 0)
                {
                    Spawncounter = 0;
                    orb4 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(476.422f, -174.517f, 42.748f, 183962);
                    if (orb4)
                        orb4->SetState(GO_STATE_OPEN);

                    switch (RandomUInt(1))
                    {
                        NPC_ID_Spawn = 0;
                        case 0:
                            NPC_ID_Spawn = CN_TWILIGHT_DRAKONAAR;
                            break;
                        case 1:
                            NPC_ID_Spawn = CN_BLACKWING_DRAKONAAR;
                            break;
                    }
                    ResetTimer(Phase_Timer, 8000);
                    Phasepart = 1;
                    return;
                }

                else if (Phasepart == 1)
                {
                    if (!NPC_orb4 && NPC_ID_Spawn != 0 && Spawncounter == 0)
                    {
                        ++Spawncounter;
                        NPC_orb4 = SpawnCreature(NPC_ID_Spawn, 471.153f, -174.715f, 42.589f, 3.097f);
                        return;
                    }
                    else if (!NPC_orb4)
                    {
                        NPC_orb4 = GetNearestCreature(NPC_ID_Spawn);
                    }
                    else if (NPC_orb4 && !NPC_orb4->IsAlive())
                    {
                        _unit->SendScriptTextChatMessage(SAY_MELLICHAR_06);
                        SetPhase(4);
                        Phasepart = 0;
                        ResetTimer(Phase_Timer, 6000);
                        return;
                    }
                    else
                    {
                        return;
                    }
                    return;
                }
                //return;
            }

            else if (IsTimerFinished(Phase_Timer) && GetPhase() == 4)
            {}

            ParentClass::AIUpdate();
            SetCanMove(false);
            SetAllowMelee(false);
            SetAllowSpell(false);
        }

        void Reset_Event()
        {
            SetCanMove(true);
            SetAllowMelee(true);
            SetAllowSpell(true);
            _unit->SetEmoteState(8); // to be replaced for the standstate

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
                NPC_orb1->Despawn(0);
                NPC_orb1 = NULL;
            }

            if (NPC_orb2)
            {
                NPC_orb2->Despawn(0);
                NPC_orb2 = NULL;
            }

            if (NPC_orb3)
            {
                NPC_orb3->Despawn(0);
                NPC_orb3 = NULL;
            }

            if (NPC_orb4)
            {
                NPC_orb4->Despawn(0);
                NPC_orb4 = NULL;
            }

            if (NPC_orb5)
            {
                NPC_orb5->Despawn(0);
                NPC_orb5 = NULL;
            }

        }

    protected:

        uint8 Phasepart;
        uint32 NPC_ID_Spawn;
        uint32 Spawncounter;
        int32 Phase_Timer;

        MoonScriptCreatureAI* NPC_orb1;
        MoonScriptCreatureAI* NPC_orb2;
        MoonScriptCreatureAI* NPC_orb3;
        MoonScriptCreatureAI* NPC_orb4;
        MoonScriptCreatureAI* NPC_orb5;
        GameObject* shield;
        GameObject* orb1;
        GameObject* orb2;
        GameObject* orb3;
        GameObject* orb4;
};

void SetupArcatraz(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_TK_THE_ARCATRAZ, &InstanceTheArcatrazScript::Create);

    mgr->register_creature_script(CN_ZEREKETH, &ZerekethAI::Create);
    mgr->register_creature_script(CN_VOIDZONEARC, &VoidZoneARC::Create);

    mgr->register_creature_script(CN_DALLIAH_THE_DOOMSAYER, &DalliahTheDoomsayerAI::Create);
    mgr->register_creature_script(CN_WRATH_SCRYER_SOCCOTHRATES, &WrathScryerSoccothratesAI::Create);
    mgr->register_creature_script(CN_HARBRINGER_SKYRISS, &HarbringerSkyrissAI::Create);
    //mgr->register_creature_script(CN_WARDEN_MELLICHAR, &WardenMellicharAI::Create);
}
