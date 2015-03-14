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

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Instance_HallsOfReflection.h"

#define MAP_HALLSOFREFLECTION 668

class HallsOfReflectionScript : public MoonInstanceScript
{
    public:
        ADD_INSTANCE_FACTORY_FUNCTION(HallsOfReflectionScript)
            HallsOfReflectionScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr){}

        void OnPlayerEnter(Player* pPlayer)
        {
            pPlayer->CastSpell(pPlayer, pPlayer->GetTeam() == TEAM_ALLIANCE ? 55774 : 55773, true);
            // Fixes a bug where you enter the instance and you run so far and teleports you out, changed DB coords still not working so this is a solution.
            pPlayer->SafeTeleport(MAP_HALLSOFREFLECTION, pPlayer->GetInstanceID(), 5260.970f, 1956.850f, 707.692f, 1.08f);
        }
};

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
            Creature* Uther = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(5307.370f, 2000.80f, 709.341f, 37225);
            Creature* Lich = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(5355.244f, 2052.96f, 707.695f, 37226);
            if (Uther && Lich)
            {
                Lich->SetDisplayId(11686); // HACK FIX makes invisible till needed
                Lich->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                // 8 second delay from first chat..
                _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Stand back! Touch that blade and your soul will be scarred for all eternity! I must attempt to commune with the spirits locked away within Frostmourne. Give me space. Back up, please.", 8000);
                sEventMgr.AddEvent(TO_OBJECT(_unit), &Object::PlaySoundToSet, (uint32)16634, EVENT_UNK, 8000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Uther talks about 12 seconds.
                Uther->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Jaina! Could it truly be you?", 12500);
                sEventMgr.AddEvent(TO_OBJECT(Uther), &Object::PlaySoundToSet, (uint32)16666, EVENT_UNK, 12500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Jaina wait about 9-10 secs
                _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Uther! Dear Uther! I... I'm so sorry.", 21000);
                sEventMgr.AddEvent(TO_OBJECT(_unit), &Object::PlaySoundToSet, (uint32)16635, EVENT_UNK, 21000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Uther wait about 9-11 secs
                Uther->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Jaina, you haven't much time. The Lich King sees what the sword sees. He will be here shortly.", 27000);
                sEventMgr.AddEvent(TO_OBJECT(Uther), &Object::PlaySoundToSet, (uint32)16667, EVENT_UNK, 27000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Jaina wait about 9-10 sec.
                _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Arthas is here? Maybe I...", 37000);
                sEventMgr.AddEvent(TO_OBJECT(_unit), &Object::PlaySoundToSet, (uint32)16636, EVENT_UNK, 37000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Uther wait about 6 seconds to chat.
                Uther->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "No, girl. Arthas is not here. Arthas is merely a presence within the Lich King's mind. A dwindling presence...", 44000);
                sEventMgr.AddEvent(TO_OBJECT(Uther), &Object::PlaySoundToSet, (uint32)16668, EVENT_UNK, 44000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Jaina with a 15 sec delay.
                _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "But Uther, if there's any hope of reaching Arthas. I... I must try.", 59000);
                sEventMgr.AddEvent(TO_OBJECT(_unit), &Object::PlaySoundToSet, (uint32)16637, EVENT_UNK, 59000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Uther with a 8.5 sec delay chat
                Uther->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Jaina, listen to me. You must destroy the Lich King. You cannot reason with him. He will kill you and your allies and raise you all as powerful soldiers of the Scourge.", 67500);
                sEventMgr.AddEvent(TO_OBJECT(Uther), &Object::PlaySoundToSet, (uint32)16669, EVENT_UNK, 67500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Jaina with a 16 sec delay
                _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Tell me how, Uther? How do I destroy my prince? My...", 83500);
                sEventMgr.AddEvent(TO_OBJECT(_unit), &Object::PlaySoundToSet, (uint32)16638, EVENT_UNK, 83500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Uther with a 7 sec delay
                Uther->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Snap out of it, girl. You must destroy the Lich King at the place where he merged with Ner'zhul - atop the spire, at the Frozen Throne, It is the only way.", 90500);
                sEventMgr.AddEvent(TO_OBJECT(Uther), &Object::PlaySoundToSet, (uint32)16670, EVENT_UNK, 90500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Jaina delay with a 15 sec.
                _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "You're right, Uther. Forgive me. I... I don't know what got a hold of me. We will deliver this information to the King and the knights that battle the Scourge within Icecrown Citadel.", 105500);
                sEventMgr.AddEvent(TO_OBJECT(_unit), &Object::PlaySoundToSet, (uint32)16639, EVENT_UNK, 105500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Uther delay with a 17 sec delay.
                Uther->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "There is... something else that you should know about the Lich King. Control over the Scourge must never be lost. Even if you were to strike down the Lich King, another would have to take his place. For without the control of its master, the Scourge world run rampant across the world - destroying all living things.", 122500);
                sEventMgr.AddEvent(TO_OBJECT(Uther), &Object::PlaySoundToSet, (uint32)16671, EVENT_UNK, 122500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Uther with a 17 sec delay
                Uther->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "A grand sacrifice by a noble soul...", 139500);
                sEventMgr.AddEvent(TO_OBJECT(Uther), &Object::PlaySoundToSet, (uint32)16672, EVENT_UNK, 139500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Jaina with a 7 sec delay.
                _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Who could bear such a burden?", 146500);
                sEventMgr.AddEvent(TO_OBJECT(_unit), &Object::PlaySoundToSet, (uint32)16640, EVENT_UNK, 146500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Uther with a 4 sec delay.
                Uther->SendChatMessage(CHAT_MSG_EMOTE, LANG_UNIVERSAL, "shakes his head.", 150500);
                // Uther with a 4 sec delay
                Uther->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "I do not know, Jaina. I Suspect that the piece of Arthas that might be left inside the Lich King is all that holds the Scourge from annihilating Azeroth.", 154500);
                sEventMgr.AddEvent(TO_OBJECT(Uther), &Object::PlaySoundToSet, (uint32)16673, EVENT_UNK, 154500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Jaina wuith a 14 sec delay.
                _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Then maybe there is still hope...", 168500);
                sEventMgr.AddEvent(TO_OBJECT(_unit), &Object::PlaySoundToSet, (uint32)16641, EVENT_UNK, 168500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Uther says nonono when lich king appear than despawns ;D 7 sec delay
                Uther->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "No, Jaina! ARRRRRRGHHHH... He... He is coming. You... You must... ", 175500);
                sEventMgr.AddEvent(TO_OBJECT(Uther), &Object::PlaySoundToSet, (uint32)16674, EVENT_UNK, 175500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                Uther->Despawn(188000, 0); // Uther gets sent to the "frostmourne" when LK comes in and silences him ;P        
                // Lich King 11 sec delay
                Lich->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "SILENCE, PALADIN!", 186500);
                sEventMgr.AddEvent(TO_OBJECT(Lich), &Object::PlaySoundToSet, (uint32)17225, EVENT_UNK, 186500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Lich King 8 sec delay
                Lich->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "So? You wish to commune with the dead? You shall your wish.", 194500);
                sEventMgr.AddEvent(TO_OBJECT(Lich), &Object::PlaySoundToSet, (uint32)17226, EVENT_UNK, 194500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                // Lich King 10 sec delay also he should spawn falric & marwyn with this & start the waves..
                Lich->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Falric. Marwyn. Bring their corpses to my chamber when you are through.", 204500);
                sEventMgr.AddEvent(TO_OBJECT(Lich), &Object::PlaySoundToSet, (uint32)17227, EVENT_UNK, 204500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            }

        }

        void AIUpdate()
        {
            Creature* Lich = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(5355.244f, 2052.96f, 707.695f, 37226);
            Lich->SetDisplayId(30721);
            Lich->GetAIInterface()->MoveTo(5312.09f, 2009.14f, 709.341f, 3.93f);
            RemoveAIUpdateEvent();
        }
};

// Lady Jaina Proudmoore Gossip
class Jaina_Gossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr)
        {
            Arcemu::Gossip::Menu::SendQuickMenu(pObject->GetGUID(), 1, plr, 1, ICON_CHAT, "Can you remove the sword?");
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* Code)
        {
            if (Id == 1)
                if (JainaAI* pJaina = TO< JainaAI* >(TO_CREATURE(pObject)->GetScript()))
                    pJaina->StartInstance();
            pObject->SetUInt32Value(UNIT_NPC_FLAGS, 0);
            plr->Gossip_Complete();
        }
};

enum MarwynSpells
{
    NPC_MARWYN = 38113,
    // Normal Mode Spells
    N_SPELL_OBLITERATE = 72360,
    N_SPELL_WELL = 72362,
    N_SPELL_CORRUPTFLESH = 72363,
    N_SPELL_SHARED = 72368,
    // Heroic Mode Spells
    H_SPELL_OBLITERATE = 72434,
    H_SPELL_WELL = 72362,
    H_SPELL_CORRUPTFLESH = 72436,
    H_SPELL_SHARED = 72369
};

class Marwyn : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(Marwyn, MoonScriptBossAI);
        Marwyn(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            if (IsHeroic() == false) // NORMAL MODE
            {
                AddSpell(N_SPELL_OBLITERATE, Target_Current, 45, 0, 30); // Timer may be off on this.
                AddSpell(N_SPELL_WELL, Target_RandomPlayer, 60, 0, 13, 0, 0, false, "Your flesh has decayed before your very eyes!", Text_Yell, 16739);
                AddSpell(N_SPELL_CORRUPTFLESH, Target_Current, 40, 0, 20, 0, 0, false, "Waste away into nothingness!", Text_Yell, 16740);
                AddSpell(N_SPELL_SHARED, Target_RandomPlayer, 45, 0, 20);

                AddEmote(Event_OnCombatStart, "Death is all that you will find here!", Text_Yell, 16734);
                AddEmote(Event_OnTargetDied, "I saw the same look in his eyes when he died. Terenas could hardly believe it. Hahahaha!", Text_Yell, 16735);
                AddEmote(Event_OnDied, "Yes... Run... Run to meet your destiny... Its bitter, cold embrace, awaits you.", Text_Yell, 16737);
            }
            else // HEROIC MODE
            {
                AddSpell(H_SPELL_OBLITERATE, Target_Current, 45, 0, 30); // Timer may be off on this.
                AddSpell(H_SPELL_WELL, Target_RandomPlayer, 60, 0, 13, 0, 0, false, "Your flesh has decayed before your very eyes!", Text_Yell, 16739);
                AddSpell(H_SPELL_CORRUPTFLESH, Target_Current, 40, 0, 20, 0, 0, false, "Waste away into nothingness!", Text_Yell, 16740);
                AddSpell(H_SPELL_SHARED, Target_RandomPlayer, 45, 0, 20);

                AddEmote(Event_OnCombatStart, "Death is all that you will find here!", Text_Yell, 16734);
                AddEmote(Event_OnTargetDied, "I saw the same look in his eyes when he died. Terenas could hardly believe it. Hahahaha!", Text_Yell, 16735);
                AddEmote(Event_OnDied, "Yes... Run... Run to meet your destiny... Its bitter, cold embrace, awaits you.", Text_Yell, 16737);
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
            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_InProgress);
            ParentClass::OnCombatStart(pKiller);
        }

        void OnCombatStop(Unit* Target)
        {
            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Performed);
            ParentClass::OnCombatStop(Target);
        }

        void OnDied(Unit* pKiller)
        {
            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Finished);
            ParentClass::OnDied(pKiller);
        }
        MoonInstanceScript* mInstance;
};

enum FalricSpells
{
    NPC_FALRIC = 38112,
    // Normal Mode Spells
    N_SPELL_QSTRIKE = 72422,
    N_SPELL_IMPEND = 72426,
    N_SPELL_HORROR = 72435,
    // Heroic Mode Spells
    H_SPELL_QSTRIKE = 72453,
    H_SPELL_IMPEND = 72426,
    H_SPELL_HORROR = 72452,
};

class Falric : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(Falric, MoonScriptBossAI);
        Falric(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            if (IsHeroic() == false) // NORMAL MODE
            {
                AddSpell(N_SPELL_QSTRIKE, Target_Current, 45, 0, 23);
                AddSpell(N_SPELL_IMPEND, Target_Current, 60, 0, 9);
                AddSpell(N_SPELL_HORROR, Target_Current, 40, 0, 20);

                AddEmote(Event_OnCombatStart, "Men, women and children... None were spared the master's wrath. Your death will be no different.", Text_Yell, 16710);
                AddEmote(Event_OnTargetDied, "The children of Stratholme fought with more ferocity!", Text_Yell, 16712);
                AddEmote(Event_OnTargetDied, "Sniveling maggot!", Text_Yell, 16711);
                AddEmote(Event_OnDied, "Marwyn, finish them...", Text_Yell, 16713);
            }
            else // HEROIC MODE
            {
                AddSpell(H_SPELL_QSTRIKE, Target_Current, 45, 0, 23);
                AddSpell(H_SPELL_IMPEND, Target_Current, 60, 0, 9);
                AddSpell(H_SPELL_HORROR, Target_Current, 40, 0, 20);
                AddSpell(H_SPELL_SHARED, Target_RandomPlayer, 45, 0, 20);

                AddEmote(Event_OnCombatStart, "Men, women and children... None were spared the master's wrath. Your death will be no different.", Text_Yell, 16710);
                AddEmote(Event_OnTargetDied, "The children of Stratholme fought with more ferocity!", Text_Yell, 16712);
                AddEmote(Event_OnTargetDied, "Sniveling maggot!", Text_Yell, 16711);
                AddEmote(Event_OnDied, "Marwyn, finish them...", Text_Yell, 16713);
            }
        }

        void OnLoad()
        {
            if (IsHeroic() == true) // HEROIC MODE
            {
                _unit->SetMaxHealth(633607); // SET HP CAUSE ARCEMU DONT SUPPORT HEROIC MODES!
                _unit->SetHealth(633607); //SET HP CAUSE ARCEMU DONT SUPPORT HEROIC MODES!
            }
        }

        void OnCombatStart(Unit* pKiller)
        {
            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_InProgress);
            ParentClass::OnCombatStart(pKiller);
        }

        void OnCombatStop(Unit* Target)
        {
            if (mInstance)
                mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Performed);
            ParentClass::OnCombatStop(Target);
        }

        void OnDied(Unit* pKiller)
        {
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

void SetupHallsOfReflection(ScriptMgr * mgr)
{
    mgr->register_instance_script(MAP_HALLSOFREFLECTION, &HallsOfReflectionScript::Create);
    mgr->register_creature_script(37221, &JainaAI::Create);
    mgr->register_creature_gossip(37221, new Jaina_Gossip);
    mgr->register_creature_script(NPC_MARWYN, &Marwyn::Create);
    mgr->register_creature_script(NPC_FALRIC, &Falric::Create);
}
