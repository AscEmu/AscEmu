/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2009-2014 ArcEmu Team <http://www.arcemu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
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
#include "Instance_HallsOfReflection.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Halls of Reflection
class HallsOfReflectionScript : public MoonInstanceScript
{
    public:

        ADD_INSTANCE_FACTORY_FUNCTION(HallsOfReflectionScript)
        HallsOfReflectionScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
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

        void OnPlayerEnter(Player* pPlayer)
        {
            // Fixes a bug where you enter the instance and you run so far and teleports you out, changed DB coords still not working so this is a solution.
            pPlayer->SafeTeleport(MAP_HALLSOFREFLECTION, pPlayer->GetInstanceID(), 5260.970f, 1956.850f, 707.692f, 1.08f);
            if (!mSpawnsCreated)
            {
                if (pPlayer->GetTeam() == TEAM_ALLIANCE)
                {
                    PushCreature(CN_JAINA_PROUDMOORE, 5266.77f, 1953.52f, 707.69f, 0.74f, 35);
                    PushCreature(CN_ARCHMAGE_KORELN, 5264.26f, 1953.36f, 707.69f, 0.74f, 35);
                }
                else // TEAM_HORDE
                {
                    PushCreature(CN_SYLVANAS_WINDRUNNER, 5266.77f, 1953.52f, 707.69f, 0.74f, 35);
                }
                mSpawnsCreated = true;
            }
        }
};


//JainaAI
class JainaAI : public MoonScriptCreatureAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(JainaAI, MoonScriptCreatureAI);
        JainaAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {}

        void StartInstance()
        {
            MoveTo(5302.581f, 1994.510f, 707.694f);
            SpawnCreature(37225, 5307.37f, 2000.80f, 709.341f, 4.03f);
            SpawnCreature(37226, 5355.244f, 2052.96f, 707.695f, 3.94f);
            Emote("Frostmourne! the blade that destroyed our kingdom..", Text_Yell, 16633);
            RegisterAIUpdateEvent(172000);
            InstanceRealStart();
        }

        void InstanceRealStart()
        {
            Creature* Uther = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(5307.370f, 2000.80f, 709.341f, CN_UTHER);
            Creature* Lich = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(5355.244f, 2052.96f, 707.695f, CN_LICH);
            if (Uther && Lich)
            {
                Lich->SetDisplayId(11686); // HACK FIX makes invisible till needed
                Lich->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                // 8 second delay from first chat..
                _unit->SendTimedScriptTextChatMessage(SAY_JAINA_01, 8000);

                // Uther talks about 12 seconds.
                Uther->SendTimedScriptTextChatMessage(SAY_UTHER_01, 12500);

                // Jaina wait about 9-10 secs
                _unit->SendTimedScriptTextChatMessage(SAY_JAINA_05, 21000);
                // Uther wait about 9-11 secs
                Uther->SendTimedScriptTextChatMessage(SAY_UTHER_02, 27000);
                // Jaina wait about 9-10 sec.
                _unit->SendTimedScriptTextChatMessage(SAY_JAINA_06, 37000);
                // Uther wait about 6 seconds to chat.
                Uther->SendTimedScriptTextChatMessage(SAY_UTHER_03, 44000);
                // Jaina with a 15 sec delay.
                _unit->SendTimedScriptTextChatMessage(SAY_JAINA_07, 59000);
                // Uther with a 8.5 sec delay chat
                Uther->SendTimedScriptTextChatMessage(SAY_UTHER_04, 67500);
                // Jaina with a 16 sec delay
                _unit->SendTimedScriptTextChatMessage(SAY_JAINA_08, 83500);
                // Uther with a 7 sec delay
                Uther->SendTimedScriptTextChatMessage(SAY_UTHER_05, 90500);
                // Jaina delay with a 15 sec.
                _unit->SendTimedScriptTextChatMessage(SAY_JAINA_09, 105500);
                // Uther delay with a 17 sec delay.
                Uther->SendTimedScriptTextChatMessage(SAY_UTHER_06, 122500);
                // Uther with a 17 sec delay
                Uther->SendTimedScriptTextChatMessage(SAY_UTHER_07, 139500);
                // Jaina with a 7 sec delay.
                _unit->SendTimedScriptTextChatMessage(SAY_JAINA_10, 146500);
                // Uther with a 4 sec delay.
                Uther->SendChatMessage(CHAT_MSG_EMOTE, LANG_UNIVERSAL, "shakes his head.", 150500); // WHAT
                // Uther with a 4 sec delay
                Uther->SendTimedScriptTextChatMessage(SAY_UTHER_08, 154500);
                // Jaina wuith a 14 sec delay.
                _unit->SendTimedScriptTextChatMessage(SAY_JAINA_11, 168500);
                // Uther says nonono when lich king appear than despawns ;D 7 sec delay
                Uther->SendTimedScriptTextChatMessage(SAY_UTHER_09, 175500);
                Uther->Despawn(188000, 0); // Uther gets sent to the "frostmourne" when LK comes in and silences him ;P        
                // Lich King 11 sec delay
                Lich->SendTimedScriptTextChatMessage(SAY_LICH_01, 186500);
                // Lich King 8 sec delay
                Lich->SendTimedScriptTextChatMessage(SAY_LICH_02, 194500);
                // Lich King 10 sec delay also he should spawn falric & marwyn with this & start the waves..
                Lich->SendTimedScriptTextChatMessage(SAY_LICH_03, 204500);
            }

        }

        void AIUpdate()
        {
            Creature* Lich = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(5355.244f, 2052.96f, 707.695f, CN_LICH);
            if (!Lich)
                return;

            Lich->SetDisplayId(30721);
            Lich->GetAIInterface()->MoveTo(5312.09f, 2009.14f, 709.341f, 3.93f);
            RemoveAIUpdateEvent();
        }
};

// Lady Jaina Proudmoore Gossip
/// \todo update this to new GossipHello
class Jaina_Gossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr)
        {
            Arcemu::Gossip::Menu::SendQuickMenu(pObject->GetGUID(), 1, plr, 1, GOSSIP_ICON_CHAT, "Can you remove the sword?");
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* Code)
        {
            if (Id == 1)
                if (JainaAI* pJaina = static_cast< JainaAI* >(static_cast<Creature*>(pObject)->GetScript()))
                    pJaina->StartInstance();
            pObject->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
            plr->Gossip_Complete();
        }
};


//Marwyn
class Marwyn : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(Marwyn, MoonScriptBossAI);
        Marwyn(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            mInstance = GetInstanceScript();

            if (IsHeroic() == false) // NORMAL MODE
            {
                AddSpell(N_SPELL_OBLITERATE, Target_Current, 45, 0, 30); // Timer may be off on this.
                AddSpell(N_SPELL_WELL, Target_RandomPlayer, 60, 0, 13, 0, 0, false, "Your flesh has decayed before your very eyes!", Text_Yell, 16739);
                AddSpell(N_SPELL_CORRUPTFLESH, Target_Current, 40, 0, 20, 0, 0, false, "Waste away into nothingness!", Text_Yell, 16740);
                AddSpell(N_SPELL_SHARED, Target_RandomPlayer, 45, 0, 20);
            }
            else // HEROIC MODE
            {
                AddSpell(H_SPELL_OBLITERATE, Target_Current, 45, 0, 30); // Timer may be off on this.
                AddSpell(H_SPELL_WELL, Target_RandomPlayer, 60, 0, 13, 0, 0, false, "Your flesh has decayed before your very eyes!", Text_Yell, 16739);
                AddSpell(H_SPELL_CORRUPTFLESH, Target_Current, 40, 0, 20, 0, 0, false, "Waste away into nothingness!", Text_Yell, 16740);
                AddSpell(H_SPELL_SHARED, Target_RandomPlayer, 45, 0, 20);
            }
        }

        void OnLoad()
        {
            if (IsHeroic() == true) // HEROIC MODE
            {
                _unit->SetMaxHealth(903227); // SET HP CAUSE ARCEMU DONT SUPPORT HEROIC MODES!
                _unit->SetHealth(903227); //SET HP CAUSE ARCEMU DONT SUPPORT HEROIC MODES!
                SetDisplayWeaponIds(51010, 51010); // Just incase DB doesn't have them correctly.
            }
        }

        void OnCombatStart(Unit* pKiller)
        {
            _unit->SendScriptTextChatMessage(4105);     // Death is all that you will find here!

            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_InProgress);
            ParentClass::OnCombatStart(pKiller);
        }

        void OnTargetDied(Unit* pTarget)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(5254);     // I saw the same look in his eyes when he died. Terenas could hardly believe it.
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(5255);     // Choke on your suffering!
                    break;
            }
        }

        void OnCombatStop(Unit* Target)
        {
            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Performed);
            ParentClass::OnCombatStop(Target);
        }

        void OnDied(Unit* pKiller)
        {
            _unit->SendScriptTextChatMessage(5256);      // Yes... Run... Run to meet your destiny... Its bitter, cold embrace, awaits you.

            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Finished);
            ParentClass::OnDied(pKiller);
        }
        MoonInstanceScript* mInstance;
};


//Falric
class Falric : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(Falric, MoonScriptBossAI);
        Falric(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            mInstance = GetInstanceScript();

            if (IsHeroic() == false) // NORMAL MODE
            {
                AddSpell(N_SPELL_QSTRIKE, Target_Current, 45, 0, 23);
                AddSpell(N_SPELL_IMPEND, Target_Current, 60, 0, 9);
                AddSpell(N_SPELL_HORROR, Target_Current, 40, 0, 20);
            }
            else // HEROIC MODE
            {
                AddSpell(H_SPELL_QSTRIKE, Target_Current, 45, 0, 23);
                AddSpell(H_SPELL_IMPEND, Target_Current, 60, 0, 9);
                AddSpell(H_SPELL_HORROR, Target_Current, 40, 0, 20);
                AddSpell(H_SPELL_SHARED, Target_RandomPlayer, 45, 0, 20);
            }
        }

        void OnCombatStart(Unit* pKiller)
        {
            _unit->SendScriptTextChatMessage(4084);      // Men, women, and children... None were spared the master's wrath. Your death will be no different.

            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_InProgress);
            ParentClass::OnCombatStart(pKiller);
        }

        void OnTargetDied(Unit* pTarget)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(4086);     // The children of Stratholme fought with more ferocity!
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(4085);     // Sniveling maggot!
                    break;
            }
        }

        void OnCombatStop(Unit* Target)
        {
            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Performed);
            ParentClass::OnCombatStop(Target);
        }

        void OnDied(Unit* pKiller)
        {
            _unit->SendScriptTextChatMessage(4087);     // Marwyn, finish them...

            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Finished);
            ParentClass::OnDied(pKiller);
        }

        void AIUpdate(Player* Plr)
        {
            if (GetPhase() == 1 && GetHealthPercent() <= 66)
            {
                _unit->CastSpell(Plr, 72395, true);
                SetPhase(2);
            }

            if (GetPhase() == 2 && GetHealthPercent() <= 33)
            {
                _unit->CastSpell(Plr, 72396, true);
                SetPhase(3);
            }

            if (GetPhase() == 3 && GetHealthPercent() <= 11)
            {
                _unit->CastSpell(Plr, 72397, true);
                SetPhase(4);
            }

        }
        MoonInstanceScript* mInstance;
};

void SetupHallsOfReflection(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_HALLSOFREFLECTION, &HallsOfReflectionScript::Create);

    //Bosses
    mgr->register_creature_script(CN_JAINA, &JainaAI::Create);
    mgr->register_creature_gossip(CN_JAINA, new Jaina_Gossip);
    mgr->register_creature_script(CN_MARWYN, &Marwyn::Create);
    mgr->register_creature_script(CN_FALRIC, &Falric::Create);
}
