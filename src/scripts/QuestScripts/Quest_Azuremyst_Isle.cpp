/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2008 WEmu Team
 * Copyright (C) 2007-2008 Moon++ Team
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

typedef std::pair<uint64, Creature*> QuestDefinition;
typedef std::vector<QuestDefinition> QuestCreature;

/*class TotemofCoo : public QuestScript
{
    public:

        void OnQuestStart(Player* pPlayer, QuestLogEntry* pQuest)
        {
            Creature* pAkida = sEAS.SpawnCreature(pPlayer, 17379, -4183.043457f, -12511.419922f, 44.361786f, 6.05629f, 0);
            if (pAkida == nullptr)
                return;

            pAkida->m_escorter = pPlayer;
            pAkida->GetAIInterface()->SetWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_QUEST);
            pAkida->GetAIInterface()->StopMovement(1000);
            pAkida->GetAIInterface()->SetAllowedToEnterCombat(false);
            pAkida->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Follow me I shall result you on a place!");
            pAkida->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
            pAkida->CastSpell(pAkida, 25035, true);   // Apparition Effect

            sEAS.CreateCustomWaypointMap(pAkida);
            sEAS.WaypointCreate(pAkida, -4174.025879f, -12512.800781f, 44.361458f, 2.827430f, 0, Movement::WP_MOVE_TYPE_RUN, 16995);
            sEAS.WaypointCreate(pAkida, -4078.135986f, -12535.500977f, 43.066765f, 5.949394f, 0, Movement::WP_MOVE_TYPE_RUN, 16995);
            sEAS.WaypointCreate(pAkida, -4040.495361f, -12565.537109f, 43.698250f, 5.592041f, 0, Movement::WP_MOVE_TYPE_RUN, 16995);
            sEAS.WaypointCreate(pAkida, -4009.526367f, -12598.929688f, 53.168480f, 5.434962f, 0, Movement::WP_MOVE_TYPE_RUN, 16995);
            sEAS.WaypointCreate(pAkida, -3981.581543f, -12635.541602f, 63.896046f, 5.332861f, 0, Movement::WP_MOVE_TYPE_RUN, 16995);
            sEAS.WaypointCreate(pAkida, -3953.170410f, -12680.391602f, 75.433006f, 5.218981f, 0, Movement::WP_MOVE_TYPE_RUN, 16995);
            sEAS.WaypointCreate(pAkida, -3924.324951f, -12741.846680f, 95.187035f, 5.124734f, 0, Movement::WP_MOVE_TYPE_RUN, 16995);
            sEAS.WaypointCreate(pAkida, -3920.791260f, -12746.218750f, 96.887978f, 3.271200f, 0, Movement::WP_MOVE_TYPE_RUN, 16995);
            sEAS.EnableWaypoints(pAkida);
            mAkidas.push_back(std::make_pair(pPlayer->GetGUID(), pAkida));
        }

        void OnQuestComplete(Player* pPlayer, QuestLogEntry* pQuest)
        {
            uint64 PlayerGuid = pPlayer->GetGUID();
            for (QuestCreature::iterator itr = mAkidas.begin(); itr != mAkidas.end(); ++itr)
            {
                if (itr->first == PlayerGuid)
                {
                    Creature* pAkida = itr->second;
                    if (pAkida != nullptr)
                    {
                        pAkida->CastSpell(pAkida, 30428, true);    // Disparition Effect
                        pAkida->Despawn(5000, 0);
                    }

                    itr = mAkidas.erase(itr);
                }
            }
        }

        void OnQuestCancel(Player* pPlayer)
        {
            uint64 PlayerGuid = pPlayer->GetGUID();
            for (QuestCreature::iterator itr = mAkidas.begin(); itr != mAkidas.end(); ++itr)
            {
                if (itr->first == PlayerGuid)
                {
                    Creature* pAkida = itr->second;
                    if (pAkida != nullptr)
                    {
                        pAkida->CastSpell(pAkida, 30428, true);    // Disparition Effect
                        pAkida->Despawn(5000, 0);
                    }

                    itr = mAkidas.erase(itr);
                }
            }
        }

        QuestCreature    mAkidas;
};

class TotemofTikti : public QuestScript
{
    public:

        void OnQuestStart(Player* mTarget, QuestLogEntry* qLogEntry)
        {
            if (!mTarget || !mTarget->GetMapMgr() || !mTarget->GetMapMgr()->GetInterface())
                return;

            Coo = sEAS.SpawnCreature(mTarget, 17391, -3926.974365f, -12752.285156f, 97.672722f, 4.926801f, 0);
            if (!Coo)
                return;

            Coo->CastSpell(Coo, 25035, true);  // Apparition Effect

            Coo->m_escorter = mTarget;
            Coo->GetAIInterface()->SetWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_QUEST);
            Coo->GetAIInterface()->StopMovement(3000);
            Coo->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Follow me!");
            Coo->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);

            sEAS.CreateCustomWaypointMap(Coo);
            sEAS.WaypointCreate(Coo, -3926.076660f, -12755.158203f, 99.080429f, 5.031188f, 0, Movement::WP_MOVE_TYPE_RUN, 16993);
            sEAS.WaypointCreate(Coo, -3924.019043f, -12763.895508f, 101.547874f, 5.212689f, 0, Movement::WP_MOVE_TYPE_RUN, 16993);
            sEAS.EnableWaypoints(Coo);


            Unit* Totem = static_cast<Unit*>(Coo);
            Unit* Plr = static_cast<Unit*>(mTarget);

            std::string msg = "Ritk kin'chikx azul azure summit...";
            Coo->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg.c_str(), 6000);

            if (mTarget->CalcDistance(Coo, mTarget) <= 10)
                sEventMgr.AddEvent(static_cast<Unit*>(Coo), &Unit::EventCastSpell, Plr, dbcSpell.LookupEntry(30424), EVENT_CREATURE_UPDATE, 8750, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

            std::string msg2 = "Coo xip fly... Jump ilos river. Find Tikti.";
            Coo->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg2.c_str(), 9000);

            sEventMgr.AddEvent(static_cast<Unit*>(Coo), &Unit::EventCastSpell, Totem, dbcSpell.LookupEntry(30473), EVENT_CREATURE_UPDATE, 12750, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            Coo->Despawn(15200, 0);
        }

    private:

        Creature* Coo;
};

class TotemofYor : public QuestScript
{
    public:

        void OnQuestStart(Player* mTarget, QuestLogEntry* qLogEntry)
        {
            if (!mTarget || !mTarget->GetMapMgr() || !mTarget->GetMapMgr()->GetInterface())
                return;

            Tikti = sEAS.SpawnCreature(mTarget, 17392, -3875.430664f, -13125.011719f, 6.822148f, 2.020735f, 0);
            if (!Tikti)
                return;

            mTarget->CastSpell(Tikti, 25035, true);  // Apparition Effect

            Tikti->m_escorter = mTarget;
            Tikti->GetAIInterface()->SetWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_QUEST);
            Tikti->GetAIInterface()->StopMovement(3000);
            Tikti->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Follow me!");
            Tikti->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);

            sEAS.CreateCustomWaypointMap(Tikti);
            sEAS.WaypointCreate(Tikti, -3881.700928f, -13111.898438f, 5.814010f, 1.855801f, 0, Movement::WP_MOVE_TYPE_RUN, 16999);
            sEAS.WaypointCreate(Tikti, -3886.341553f, -13098.914063f, 3.964841f, 1.855801f, 0, Movement::WP_MOVE_TYPE_RUN, 16999);
            sEAS.EnableWaypoints(Tikti);

            Unit* Totem = static_cast<Unit*>(Tikti);
            Unit* Plr = static_cast<Unit*>(mTarget);

            std::string msg = "[Furbolg] Far you mixik tak come. Gaze upon the kitch'kal river. Follow. Ilog to Yor.";
            Tikti->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg.c_str(), 15000);

            if (mTarget->CalcDistance(Tikti, mTarget) <= 10)
                sEventMgr.AddEvent(static_cast<Unit*>(Tikti), &Unit::EventCastSpell, Plr, dbcSpell.LookupEntry(30430), EVENT_CREATURE_UPDATE, 18000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

            std::string msg2 = "[Furbolg] Go... Search yitix'kil bottom river. South!";
            Tikti->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg2.c_str(), 20000);

            sEventMgr.AddEvent(static_cast<Unit*>(Tikti), &Unit::EventCastSpell, Totem, dbcSpell.LookupEntry(30431), EVENT_GMSCRIPT_EVENT, 20000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

            Tikti->Despawn(24000, 0);
        }

    private:

        Creature* Tikti;
};

class TotemofVark : public QuestScript
{
    public:

        void OnQuestStart(Player* mTarget, QuestLogEntry* qLogEntry)
        {
            if (!mTarget || !mTarget->GetMapMgr() || !mTarget->GetMapMgr()->GetInterface())
                return;

            Yor = sEAS.SpawnCreature(mTarget, 17393, -4634.246582f, -13071.686523f, -14.755350f, 1.569997f, 0);
            if (!Yor)
                return;

            Yor->CastSpell(Yor, 25035, true);   // Apparition Effect

            Yor->m_escorter = mTarget;
            Yor->GetAIInterface()->StopMovement(1000);
            Yor->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
            char msg[256];
            snprintf((char*)msg, 256, "Come, %s . Let us leave the water together, purified.", mTarget->GetName());
            Yor->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg);

            sEAS.CreateCustomWaypointMap(Yor);
            sEAS.WaypointCreate(Yor, -4650.081055f, -13016.692383f, 1.776296f, 2.021601f, 0, Movement::WP_MOVE_TYPE_RUN, 16393);
            sEAS.WaypointCreate(Yor, -3886.341553f, -13098.914063f, 3.964841f, 1.855801f, 1000, Movement::WP_MOVE_TYPE_RUN, 16393);
            sEAS.WaypointCreate(Yor, -4677.421387f, -12983.874023f, 0.833827f, 2.335760f, 0, Movement::WP_MOVE_TYPE_RUN, 16393);   // Should look player
            sEAS.EnableWaypoints(Yor);

            //We have to set up these pointers first to resolve ambiguity in the event manager template
            Unit* Totem = static_cast<Unit*>(Yor);
            Unit* Plr = static_cast<Unit*>(mTarget);

            // Change to Stillpine form
            sEventMgr.AddEvent(Totem, &Unit::EventCastSpell, Totem, dbcSpell.LookupEntry(30446), EVENT_CREATURE_UPDATE, 15500, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            std::string msg2 = "[Furbolg] We go now, together. We will seek Vark.";
            Yor->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg2.c_str(), 26500);

            // Change to nightsaber form
            sEventMgr.AddEvent(Totem, &Unit::EventCastSpell, Totem, dbcSpell.LookupEntry(30448), EVENT_CREATURE_UPDATE, 30000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            std::string msg3 = "[Furbolg] Follow me to Vark.";
            Yor->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg3.c_str(), 31000);

            if (mTarget->CalcDistance(Totem, Plr) <= 10)
                sEventMgr.AddEvent(Totem, &Unit::EventCastSpell, Plr, dbcSpell.LookupEntry(30448), EVENT_CREATURE_UPDATE, 31000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }

    // NEED TO MAKE THE PATH TO THE TOTEM OF VARK
    private:

        Creature* Yor;
};*/

// Chieftain Oomooroo
class ChieftainOomoorooQAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ChieftainOomoorooQAI);
        ChieftainOomoorooQAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

        void OnDied(Unit* mKiller)
        {
            if (mKiller->IsPlayer())
            {
                QuestLogEntry* pQuest = static_cast<Player*>(mKiller)->GetQuestLogForEntry(9573);
                if (pQuest != nullptr && pQuest->GetMobCount(1) < pQuest->GetQuest()->required_mob_or_go_count[1])
                {
                    pQuest->SetMobCount(1, pQuest->GetMobCount(1) + 1);
                    pQuest->SendUpdateAddKill(1);
                    pQuest->UpdatePlayerFields();
                }
            }
        }
};


void SetupAzuremystIsle(ScriptMgr* mgr)
{
    /*mgr->register_quest_script( 9539, new TotemofCoo() );
    mgr->register_quest_script( 9540, new TotemofTikti());
    mgr->register_quest_script( 9541, new TotemofYor() );
    mgr->register_quest_script( 9542, new TotemofVark() );*/
    mgr->register_creature_script(17189, &ChieftainOomoorooQAI::Create);
}
