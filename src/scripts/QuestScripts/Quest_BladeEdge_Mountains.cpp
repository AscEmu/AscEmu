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
#include "Spell/SpellAuras.h"

class WyrmcultBlackwhelp : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(WyrmcultBlackwhelp);
    WyrmcultBlackwhelp(Creature* c) : CreatureAIScript(c) {}

    void OnLoad()
    {
        RegisterAIUpdateEvent(1000);
    }

    void AIUpdate()
    {
        // Let's see if we are netted
        Aura* a = getCreature()->getAuraWithId(38177);
        if (a != nullptr)
        {
            Unit* Caster = a->GetUnitCaster();
            if (!Caster)
                return;

            if (Caster->IsPlayer())
            {

                QuestLogEntry* qle = static_cast<Player*>(Caster)->GetQuestLogForEntry(10747);
                if (qle != nullptr)
                {
                    // casting the spell that will create the item for the player
                    getCreature()->CastSpell(Caster, 38178, true);
                    getCreature()->Despawn(1000, 360000);
                }
            }
        }
    }
};

// The Bladespire Threat Quest
class BladespireQAI : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(BladespireQAI);
    BladespireQAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller)
    {
        if (mKiller->IsPlayer())
        {
            static_cast<Player*>(mKiller)->AddQuestKill(10503, 0, 0);
        }
    }
};

class IntotheSoulgrinder : public QuestScript
{
public:

    void OnQuestComplete(Player* mTarget, QuestLogEntry* qLogEntry)
    {
        Creature* qg = mTarget->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 22941);
        if (qg == nullptr)
            return;

        qg->GetMapMgr()->GetInterface()->SpawnCreature(23053, 2794.978271f, 5842.185547f, 35.911819f, 0, true, false, 0, 0);
    }
};

class MagnetoAura : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(MagnetoAura);
    MagnetoAura(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad()
    {
        getCreature()->CastSpell(getCreature(), 37136, true);
    }
};

class powerconv : public GameObjectAIScript
{
public:

    powerconv(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new powerconv(GO); }

    void OnActivate(Player* pPlayer)
    {
        QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(10584);
        if (qle == nullptr)
            return;

        Creature* magneto = pPlayer->GetMapMgr()->CreateAndSpawnCreature(21729, _gameobject->GetPositionX(), _gameobject->GetPositionY(), _gameobject->GetPositionZ(), 0);
        if (magneto != nullptr)
        {
            magneto->Despawn(5 * 60 * 1000, 0);
        }

        _gameobject->Despawn(300000, 0);
    }
};

class NetherEgg : public GameObjectAIScript
{
public:

    NetherEgg(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new NetherEgg(GO); }

    void OnActivate(Player* pPlayer)
    {
        QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(10609);
        if (qle == nullptr)
            return;

        Creature* whelp = pPlayer->GetMapMgr()->CreateAndSpawnCreature(20021, _gameobject->GetPositionX(), _gameobject->GetPositionY(), _gameobject->GetPositionZ(), 0);
        if (whelp != nullptr)
        {
            whelp->Despawn(5 * 60 * 1000, 0);
        }

        _gameobject->Despawn(300000, 0);
    }
};

class FunnyDragon : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(FunnyDragon);
    FunnyDragon(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        i = 0;      // rename this....
    }

    void OnLoad()
    {
        RegisterAIUpdateEvent(5000);
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        setAIAgent(AGENT_NULL);
        _setMeleeDisabled(true);
        getCreature()->SetEmoteState(EMOTE_ONESHOT_NONE);
        getCreature()->GetAIInterface()->m_canMove = false;
        i = 1;
    }

    void AIUpdate()
    {
        switch (i)
        {
            case 1:
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Muahahahahaha! You fool! you've released me from my banishment in the interstices between space and time!");
                break;
            case 2:
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "All of Draenor shall quake beneath my feet! i Will destroy this world and reshape it in my immage!");
                break;
            case 3:
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Where shall i Begin? i cannot bother myself with a worm such as yourself. Theres a World to be Conquered!");
                break;
            case 4:
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "No doubt the fools that banished me are long dead. i shall take the wing and survey my new demense, Pray to whatever gods you hold dear that we do not meet again.");
                getCreature()->Despawn(5000, 0);
                break;
        }

        ++i;
    }

    uint32 i;
};

class LegionObelisk : public GameObjectAIScript
{
public:

    LegionObelisk(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new LegionObelisk(GO); }

    void OnActivate(Player* pPlayer)
    {
        GameObject* obelisk1 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2898.92f, 4759.29f, 277.408f, 185198);
        GameObject* obelisk2 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2942.3f, 4752.28f, 285.553f, 185197);
        GameObject* obelisk3 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2834.39f, 4856.67f, 277.632f, 185196);
        GameObject* obelisk4 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2923.37f, 4840.36f, 278.45f, 185195);
        GameObject* obelisk5 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2965.75f, 4835.25f, 277.949f, 185193);

        if (obelisk1 && obelisk2 && obelisk3 && obelisk4 && obelisk5)
        {
            if (obelisk1->GetState() == 0 && obelisk2->GetState() == 0 && obelisk3->GetState() == 0 && obelisk4->GetState() == 0 && obelisk5->GetState() == 0)
            {
                Creature* ct = pPlayer->GetMapMgr()->CreateAndSpawnCreature(19963, 2943.59f, 4779.05f, 284.49f, 1.89f);
                if (ct != nullptr)
                    ct->Despawn(5 * 60 * 1000, 0);
            }
        }

#if VERSION_STRING > TBC
        if (obelisk1 != nullptr)
            sEventMgr.AddEvent(static_cast<Object*>(obelisk1), &Object::setByteValue, (uint16)GAMEOBJECT_BYTES_1, (uint8)GAMEOBJECT_BYTES_STATE, (uint8)1, EVENT_UNK, 10000, 0, 1);
        if (obelisk2 != nullptr)
            sEventMgr.AddEvent(static_cast<Object*>(obelisk2), &Object::setByteValue, (uint16)GAMEOBJECT_BYTES_1, (uint8)GAMEOBJECT_BYTES_STATE, (uint8)1, EVENT_UNK, 10000, 0, 1);
        if (obelisk3 != nullptr)
            sEventMgr.AddEvent(static_cast<Object*>(obelisk3), &Object::setByteValue, (uint16)GAMEOBJECT_BYTES_1, (uint8)GAMEOBJECT_BYTES_STATE, (uint8)1, EVENT_UNK, 10000, 0, 1);
        if (obelisk4 != nullptr)
            sEventMgr.AddEvent(static_cast<Object*>(obelisk4), &Object::setByteValue, (uint16)GAMEOBJECT_BYTES_1, (uint8)GAMEOBJECT_BYTES_STATE, (uint8)1, EVENT_UNK, 10000, 0, 1);
        if (obelisk5 != nullptr)
            sEventMgr.AddEvent(static_cast<Object*>(obelisk5), &Object::setByteValue, (uint16)GAMEOBJECT_BYTES_1, (uint8)GAMEOBJECT_BYTES_STATE, (uint8)1, EVENT_UNK, 10000, 0, 1);
#endif
    }

};

class BloodmaulQAI : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(BloodmaulQAI);
    BloodmaulQAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller)
    {
        if (!mKiller->IsPlayer())
            return;

        Player* pPlayer = static_cast<Player*>(mKiller);

        if (pPlayer->IsTeamHorde())
            pPlayer->AddQuestKill(10505, 0, 0);
        else
            pPlayer->AddQuestKill(10502, 0, 0);
    }
};

class Thuk_the_DefiantAI : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(Thuk_the_DefiantAI);

    Thuk_the_DefiantAI(Creature* pCreature) : CreatureAIScript(pCreature) {}
    void OnLoad()
    {
        getCreature()->setFloatValue(OBJECT_FIELD_SCALE_X, 0.4f);
    }

    void OnDied(Unit* mKiller)
    {
        RemoveAIUpdateEvent();
    }

    void OnTargetDied(Unit* mTarget)
    {
        getCreature()->SetFaction(35);
        getCreature()->setFloatValue(OBJECT_FIELD_SCALE_X, 0.4f);
    }
};

class Stasis_Chamber_Alpha : public GameObjectAIScript
{
public:

    Stasis_Chamber_Alpha(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO)
    {
        return new Stasis_Chamber_Alpha(GO);
    }

    void OnActivate(Player* pPlayer)
    {
        if (pPlayer->HasQuest(10974))
        {
            Creature* pCreature = nullptr;
            pCreature = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(3989.094482f, 6071.562500f, 266.416656f, 22920);
            if (pCreature != nullptr)
            {
                pCreature->SetFaction(14);
                pCreature->setFloatValue(OBJECT_FIELD_SCALE_X, 1);
                pCreature->GetAIInterface()->setNextTarget(pPlayer);
                pCreature->GetAIInterface()->AttackReaction(pPlayer, 1);
            }
        }
        else
        {
            pPlayer->BroadcastMessage("Missing required quest : Stasis Chambers of Bash'ir");
        }
    }
};



//////////////////////////////////////////////////////////////////////////
// Bladespire entry
enum CreatureEntry
{
    CN_BLADESPIRE_OGRE_1 = 19995,
    CN_BLADESPIRE_OGRE_2 = 19998,
    CN_BLADESPIRE_OGRE_3 = 20756,
    CN_BLOODMAUL_BRUTEBANE_STOUT_TRIGGER = 21241
};

//////////////////////////////////////////////////////////////////////////
/////// Bloodmaul Brutebane Stout Trigger
class BrutebaneStoutTriggerAI : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(BrutebaneStoutTriggerAI);
    BrutebaneStoutTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->SetFaction(35);

        setRooted(true);
        NdGo = nullptr;

        plr = getCreature()->GetMapMgr()->GetInterface()->GetPlayerNearestCoords(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ());
        Ogre = getNearestCreatureAI(CN_BLADESPIRE_OGRE_1);
        if (Ogre == nullptr)
        {
            Ogre = getNearestCreatureAI(CN_BLADESPIRE_OGRE_2);
            if (Ogre == nullptr)
            {
                Ogre = getNearestCreatureAI(CN_BLADESPIRE_OGRE_3);
                if (Ogre == nullptr)
                {
                    return;
                }
            }
        }
        Ogre->moveToUnit(getCreature());
        RegisterAIUpdateEvent(1000);
    }

    void AIUpdate()
    {
        if (Ogre == nullptr)
            return;

        if (getRangeToObject(Ogre->getCreature()) <= 5)
        {
            Ogre->_setDisplayWeaponIds(28562, 0);
            Ogre->getCreature()->SetEmoteState(EMOTE_ONESHOT_EAT_NOSHEATHE);
            Ogre->getCreature()->SetFaction(35);
            Ogre->getCreature()->SetStandState(STANDSTATE_SIT);

            NdGo = getNearestGameObject(184315);
            if (NdGo == nullptr)
                return;

            NdGo->Despawn(0, 0);
            Ogre->despawn(60 * 1000, 3 * 60 * 1000);
            if (plr == nullptr)
                return;

            plr->AddQuestKill(10512, 0, 0);

            despawn(0, 0);
            return;
        }
        
    }

    Player* plr;
    GameObject* NdGo;
    CreatureAIScript* Ogre;
};


void SetupBladeEdgeMountains(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_BLADESPIRE_OGRE_1, &BladespireQAI::Create);
    mgr->register_creature_script(21296, &BladespireQAI::Create);
    mgr->register_creature_script(CN_BLADESPIRE_OGRE_3, &BladespireQAI::Create);
    mgr->register_creature_script(20766, &BladespireQAI::Create);
    mgr->register_creature_script(CN_BLADESPIRE_OGRE_2, &BladespireQAI::Create);
    mgr->register_creature_script(21731, &MagnetoAura::Create);
    mgr->register_creature_script(21823, &FunnyDragon::Create);
    mgr->register_creature_script(19957, &BloodmaulQAI::Create);
    mgr->register_creature_script(19991, &BloodmaulQAI::Create);
    mgr->register_creature_script(21238, &BloodmaulQAI::Create);
    mgr->register_creature_script(19952, &BloodmaulQAI::Create);
    mgr->register_creature_script(21294, &BloodmaulQAI::Create);
    mgr->register_creature_script(19956, &BloodmaulQAI::Create);
    mgr->register_creature_script(19993, &BloodmaulQAI::Create);
    mgr->register_creature_script(19992, &BloodmaulQAI::Create);
    mgr->register_creature_script(19948, &BloodmaulQAI::Create);
    mgr->register_creature_script(22384, &BloodmaulQAI::Create);
    mgr->register_creature_script(22160, &BloodmaulQAI::Create);
    mgr->register_creature_script(19994, &BloodmaulQAI::Create);
    mgr->register_creature_script(22920, &Thuk_the_DefiantAI::Create);

    mgr->register_creature_script(CN_BLOODMAUL_BRUTEBANE_STOUT_TRIGGER, &BrutebaneStoutTriggerAI::Create);

    mgr->register_quest_script(11000, new IntotheSoulgrinder());

    mgr->register_gameobject_script(184867, &NetherEgg::Create);
    mgr->register_gameobject_script(184906, &powerconv::Create);
    mgr->register_gameobject_script(185198, &LegionObelisk::Create);
    mgr->register_gameobject_script(185197, &LegionObelisk::Create);
    mgr->register_gameobject_script(185196, &LegionObelisk::Create);
    mgr->register_gameobject_script(185195, &LegionObelisk::Create);
    mgr->register_gameobject_script(185193, &LegionObelisk::Create);
    mgr->register_gameobject_script(185512, &Stasis_Chamber_Alpha::Create);

    mgr->register_creature_script(21387, &WyrmcultBlackwhelp::Create);
}
