/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Server/Script/QuestScript.hpp"
#include "Spell/SpellAura.hpp"

class WyrmcultBlackwhelp : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WyrmcultBlackwhelp(c); }
    explicit WyrmcultBlackwhelp(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        RegisterAIUpdateEvent(1000);
    }

    void AIUpdate() override
    {
        // Let's see if we are netted
        if (Aura* a = getCreature()->getAuraWithId(38177))
        {
            if (Unit* Caster = a->GetUnitCaster())
            {
                if (Caster->isPlayer())
                {
                    if (dynamic_cast<Player*>(Caster)->hasQuestInQuestLog(10747))
                    {
                        // casting the spell that will create the item for the player
                        getCreature()->castSpell(Caster, 38178, true);
                        getCreature()->Despawn(1000, 360000);
                    }
                }
            }
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Bladespire Threat Quest
class BladespireQAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BladespireQAI(c); }
    explicit BladespireQAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller) override
    {
        if (mKiller->isPlayer())
        {
            static_cast<Player*>(mKiller)->addQuestKill(10503, 0, 0);
        }
    }
};

class IntotheSoulgrinder : public QuestScript
{
public:
    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* qg = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 22941);
        if (qg == nullptr)
            return;

        qg->getWorldMap()->getInterface()->spawnCreature(23053, LocationVector(2794.978271f, 5842.185547f, 35.911819f), true, false, 0, 0);
    }
};

class MagnetoAura : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MagnetoAura(c); }
    explicit MagnetoAura(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->castSpell(getCreature(), 37136, true);
    }
};

class Powerconv : public GameObjectAIScript
{
public:
    explicit Powerconv(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Powerconv(GO); }

    void OnActivate(Player* pPlayer) override
    {
        if (pPlayer->hasQuestInQuestLog(10584))
        {
            Creature* magneto = pPlayer->getWorldMap()->createAndSpawnCreature(21729, _gameobject->GetPosition());
            if (magneto != nullptr)
            {
                magneto->Despawn(5 * 60 * 1000, 0);
            }

            _gameobject->despawn(300000, 0);
        }
    }
};

class NetherEgg : public GameObjectAIScript
{
public:
    explicit NetherEgg(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new NetherEgg(GO); }

    void OnActivate(Player* pPlayer) override
    {
        if (!pPlayer->hasQuestInQuestLog(10609))
        {
            Creature* whelp = pPlayer->getWorldMap()->createAndSpawnCreature(20021, _gameobject->GetPosition());
            if (whelp != nullptr)
            {
                whelp->Despawn(5 * 60 * 1000, 0);
            }

            _gameobject->despawn(300000, 0);
        }
    }
};

class FunnyDragon : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FunnyDragon(c); }
    explicit FunnyDragon(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        i = 0;      // rename this....
    }

    void OnLoad() override
    {
        RegisterAIUpdateEvent(5000);
        getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        setAIAgent(AGENT_NULL);
        _setMeleeDisabled(true);
        getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);
        getCreature()->setControlled(false, UNIT_STATE_ROOTED);
        i = 1;
    }

    void AIUpdate() override
    {
        switch (i)
        {
            case 1:
                getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Muahahahahaha! You fool! you've released me from my banishment in the interstices between space and time!");
                break;
            case 2:
                getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "All of Draenor shall quake beneath my feet! i Will destroy this world and reshape it in my immage!");
                break;
            case 3:
                getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Where shall i Begin? i cannot bother myself with a worm such as yourself. Theres a World to be Conquered!");
                break;
            case 4:
                getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "No doubt the fools that banished me are long dead. i shall take the wing and survey my new demense, Pray to whatever gods you hold dear that we do not meet again.");
                getCreature()->Despawn(5000, 0);
                break;
        }

        ++i;
    }

    uint32_t i;
};

class LegionObelisk : public GameObjectAIScript
{
public:
    explicit LegionObelisk(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new LegionObelisk(GO); }

    void OnActivate(Player* pPlayer) override
    {
        GameObject* obelisk1 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(2898.92f, 4759.29f, 277.408f, 185198);
        GameObject* obelisk2 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(2942.3f, 4752.28f, 285.553f, 185197);
        GameObject* obelisk3 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(2834.39f, 4856.67f, 277.632f, 185196);
        GameObject* obelisk4 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(2923.37f, 4840.36f, 278.45f, 185195);
        GameObject* obelisk5 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(2965.75f, 4835.25f, 277.949f, 185193);

        if (obelisk1 && obelisk2 && obelisk3 && obelisk4 && obelisk5)
        {
            if (obelisk1->getState() == 0 && obelisk2->getState() == 0 && obelisk3->getState() == 0 && obelisk4->getState() == 0 && obelisk5->getState() == 0)
            {
                Creature* ct = pPlayer->getWorldMap()->createAndSpawnCreature(19963, LocationVector(2943.59f, 4779.05f, 284.49f, 1.89f));
                if (ct != nullptr)
                    ct->Despawn(5 * 60 * 1000, 0);
            }
        }

        if (obelisk1 != nullptr)
            sEventMgr.AddEvent(obelisk1, &GameObject::setState, (uint8_t)1, EVENT_UNK, 10000, 0, 1);
        if (obelisk2 != nullptr)
            sEventMgr.AddEvent(obelisk2, &GameObject::setState, (uint8_t)1, EVENT_UNK, 10000, 0, 1);
        if (obelisk3 != nullptr)
            sEventMgr.AddEvent(obelisk3, &GameObject::setState, (uint8_t)1, EVENT_UNK, 10000, 0, 1);
        if (obelisk4 != nullptr)
            sEventMgr.AddEvent(obelisk4, &GameObject::setState, (uint8_t)1, EVENT_UNK, 10000, 0, 1);
        if (obelisk5 != nullptr)
            sEventMgr.AddEvent(obelisk5, &GameObject::setState, (uint8_t)1, EVENT_UNK, 10000, 0, 1);
    }

};

class BloodmaulQAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BloodmaulQAI(c); }
    explicit BloodmaulQAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller) override
    {
        if (!mKiller->isPlayer())
            return;

        Player* pPlayer = static_cast<Player*>(mKiller);

        if (pPlayer->isTeamHorde())
            pPlayer->addQuestKill(10505, 0, 0);
        else
            pPlayer->addQuestKill(10502, 0, 0);
    }
};

class Thuk_the_DefiantAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Thuk_the_DefiantAI(c); }
    explicit Thuk_the_DefiantAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setScale(0.4f);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        RemoveAIUpdateEvent();
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        getCreature()->setFaction(35);
        getCreature()->setScale(0.4f);
    }
};

class Stasis_Chamber_Alpha : public GameObjectAIScript
{
public:
    explicit Stasis_Chamber_Alpha(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO)
    {
        return new Stasis_Chamber_Alpha(GO);
    }

    void OnActivate(Player* pPlayer) override
    {
        if (pPlayer->hasQuestInQuestLog(10974))
        {
            Creature* pCreature = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(3989.094482f, 6071.562500f, 266.416656f, 22920);
            if (pCreature != nullptr)
            {
                pCreature->setFaction(14);
                pCreature->setScale(1.0f);
                pCreature->getAIInterface()->setCurrentTarget(pPlayer);
                pCreature->getAIInterface()->onHostileAction(pPlayer);
            }
        }
        else
        {
            pPlayer->broadcastMessage("Missing required quest : Stasis Chambers of Bash'ir");
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Bladespire entry
enum CreatureEntry
{
    CN_BLADESPIRE_OGRE_1 = 19995,
    CN_BLADESPIRE_OGRE_2 = 19998,
    CN_BLADESPIRE_OGRE_3 = 20756,
    CN_BLOODMAUL_BRUTEBANE_STOUT_TRIGGER = 21241
};

//////////////////////////////////////////////////////////////////////////////////////////
// Bloodmaul Brutebane Stout Trigger
class BrutebaneStoutTriggerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BrutebaneStoutTriggerAI(c); }
    explicit BrutebaneStoutTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->setFaction(35);

        setRooted(true);
        NdGo = nullptr;

        plr = getCreature()->getWorldMap()->getInterface()->getPlayerNearestCoords(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ());
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

    void AIUpdate() override
    {
        if (Ogre == nullptr)
            return;

        if (getRangeToObject(Ogre->getCreature()) <= 5)
        {
            Ogre->_setDisplayWeaponIds(28562, 0);
            Ogre->getCreature()->setEmoteState(EMOTE_ONESHOT_EAT_NOSHEATHE);
            Ogre->getCreature()->setFaction(35);
            Ogre->getCreature()->setStandState(STANDSTATE_SIT);

            NdGo = getNearestGameObject(184315);
            if (NdGo == nullptr)
                return;

            NdGo->despawn(0, 0);
            Ogre->despawn(60 * 1000, 3 * 60 * 1000);
            if (plr == nullptr)
                return;

            plr->addQuestKill(10512, 0, 0);

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
    mgr->register_gameobject_script(184906, &Powerconv::Create);
    mgr->register_gameobject_script(185198, &LegionObelisk::Create);
    mgr->register_gameobject_script(185197, &LegionObelisk::Create);
    mgr->register_gameobject_script(185196, &LegionObelisk::Create);
    mgr->register_gameobject_script(185195, &LegionObelisk::Create);
    mgr->register_gameobject_script(185193, &LegionObelisk::Create);
    mgr->register_gameobject_script(185512, &Stasis_Chamber_Alpha::Create);

    mgr->register_creature_script(21387, &WyrmcultBlackwhelp::Create);
}
