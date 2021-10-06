/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008 WEmu Team
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
#include "Server/Script/CreatureAIScript.h"

enum
{
    GO_MEAT = 181291,
    GO_BOTTLE = 2687,
    GO_BREAD = 2562
};

class StrFever : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 1, plr->GetSession()->language);
        if (plr->hasQuestInQuestLog(348) && plr->getItemInterface()->GetItemCount(2799, 0) && !plr->getItemInterface()->GetItemCount(2797, 0))
            menu.addItem(GOSSIP_ICON_CHAT, 494, 1);     // I'm ready, Summon Him!

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Creature* doctor = static_cast<Creature*>(pObject);

        plr->getItemInterface()->RemoveItemAmt(2799, 1);
        doctor->castSpell(doctor, sSpellMgr.getSpellInfo(12380), true);
        if (!plr->GetMapMgr() || !plr->GetMapMgr()->GetInterface())
            return;

        Creature* firstenemy = plr->GetMapMgr()->CreateAndSpawnCreature(1511, -13770.5f, -6.79f, 42.8f, 5.7f);
        if (firstenemy != nullptr)
        {
            firstenemy->GetAIInterface()->moveTo(-13727.8f, -26.2f, 46.15f);
            firstenemy->Despawn(10 * 60 * 1000, 0);
        }
    }
};

class Beka : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Beka(c); }
    explicit Beka(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller) override
    {
        if (mKiller->isPlayer())
        {
            Player* mPlayer = static_cast<Player*>(mKiller);
            Creature* beka1 =mPlayer->GetMapMgr()->CreateAndSpawnCreature(1516, -13770.5f, -6.79f, 42.8f, 5.7f);
            if (beka1 != nullptr)
            {
                beka1->GetAIInterface()->moveTo(-13727.8f, -26.2f, 46.15f);
                beka1->SetOrientation(4.07f);
                beka1->Despawn(10 * 60 * 1000, 0);
            }
        }
        else
        {
            Player* mPlayer = getCreature()->GetMapMgr()->GetInterface()->GetPlayerNearestCoords(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ());
            if (mPlayer)
            {
                Creature* beka1 = mPlayer->GetMapMgr()->CreateAndSpawnCreature(1516, -13770.5f, -6.79f, 42.8f, 5.7f);
                if (beka1 != nullptr)
                {
                    beka1->GetAIInterface()->moveTo(-13727.8f, -26.2f, 46.15f);
                    beka1->SetOrientation(4.07f);
                    beka1->Despawn(10 * 60 * 1000, 0);
                }
            }
        }
    }
};

class Beka1 : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Beka1(c); }
    explicit Beka1(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller) override
    {
        if (mKiller->isPlayer())
        {
            Player* mPlayer = static_cast<Player*>(mKiller);
            Creature* beka1 = mPlayer->GetMapMgr()->CreateAndSpawnCreature(1514, -13770.5f, -6.79f, 42.8f, 5.7f);
            if (beka1 != nullptr)
            {
                beka1->GetAIInterface()->moveTo(-13727.8f, -26.2f, 46.15f);
                beka1->SetOrientation(4.07f);
                beka1->Despawn(10 * 60 * 1000, 0);
            }
        }
        else
        {
            Player* mPlayer = getCreature()->GetMapMgr()->GetInterface()->GetPlayerNearestCoords(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ());
            if (mPlayer)
            {
                Creature* beka1 = mPlayer->GetMapMgr()->CreateAndSpawnCreature(1514, -13770.5f, -6.79f, 42.8f, 5.7f);
                if (beka1 != nullptr)
                {
                    beka1->GetAIInterface()->moveTo(-13727.8f, -26.2f, 46.15f);
                    beka1->SetOrientation(4.07f);
                    beka1->Despawn(10 * 60 * 1000, 0);
                }
            }
        }
    }
};

class Beka2 : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Beka2(c); }
    explicit Beka2(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* pPlayer) override
    {
        Creature* pCreature = pPlayer->MAP_GET_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 1449);
        if (pCreature)
        {
            pCreature->emote(EMOTE_ONESHOT_CHEER);
        }
    }
};

class BloodscalpClanHeads : public QuestScript
{
public:
    void OnQuestComplete(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        GameObject* skull1 = pPlayer->MAP_GAMEOBJECT_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 2551);
        if (skull1 == nullptr)
            return;

        Creature* pCreature = pPlayer->MAP_GET_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 2519);
        if (pCreature == nullptr)
        {
            return;
        }

        pCreature->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Ah. Good $N. Now let us see what tale these heads tell...");
        pCreature->castSpell(pCreature, sSpellMgr.getSpellInfo(3644), false);
        skull1->Despawn(5000, 0);
        GameObject* skull2 = pPlayer->MAP_GAMEOBJECT_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 2551);
        if (skull2)
            skull2->Despawn(5000, 0);

        pCreature->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "There, $N. You may now speak to the Bloodscalp chief and his witchdoctor.", 500);
    }
};

class BacktoBootyBay : public QuestScript
{
public:
    void OnQuestComplete(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* pCreature = pPlayer->MAP_GET_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 2498);
        if (pCreature)
        {
            pCreature->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Hm... if you're looking to adle wits. $N, then the secret behind Zanzil's zombies might just do the trick!");
        }
    }
};

class VoodooDues : public QuestScript
{
public:
    void OnQuestComplete(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* pCreature = pPlayer->MAP_GET_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 2501);
        if (pCreature)
        {
            pCreature->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Bah! $N, this foot won't budge!");
        }
    }
};

static MovementNew::Location MeatSpawnPoints[] =
{
    { -14655.1f, 148.229f, 3.01744f, 3.45635f},
    { -14655.6f, 146.111f, 2.29463f, 1.43766f},
    { -14655.1f, 147.721f, 2.64111f, 3.99435f},
    { -14654.3f, 147.015f, 2.21427f, 2.44821f},
    { -14655.8f, 147.092f, 2.36460f, 0.66155f},
    { -14654.3f, 147.866f, 2.90964f, 0.05732f},
    { -14653.5f, 147.004f, 2.36253f, 2.80556f},
    { -14652.2f, 146.926f, 3.63756f, 6.06693f},
    { -14653.0f, 145.274f, 2.76439f, 6.06279f}
};
static MovementNew::Location BottleSpawnPoints[] =
{
    { -14653.5f, 145.711f, 2.01005f, 1.14750f},
    { -14656.7f, 147.404f, 3.05695f, 1.44181f},
    { -14657.1f, 147.068f, 2.94368f, 1.40036f},
    { -14657.5f, 147.567f, 2.83560f, 2.14234f},
    { -14655.9f, 148.848f, 3.93732f, 2.79728f}
};
static MovementNew::Location BreadSpawnPoints[] =
{
    { -14654.6f, 146.299f, 2.04134f, 5.47387f},
    { -14656.5f, 148.372f, 3.50805f, 5.76817f},
    { -14652.6f, 146.079f, 3.35855f, 2.89231f}
};

class FacingNegolash : public QuestScript
{
    void OnQuestComplete(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        GameObject* obj = nullptr;

        for (uint8_t i = 0; i < 9; ++i)
        {
            obj = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(GO_MEAT, MeatSpawnPoints[i].x, MeatSpawnPoints[i].y, MeatSpawnPoints[i].z, MeatSpawnPoints[i].orientation, 1);
            if (obj != nullptr)
                obj->Despawn(2 * 60 * 1000, 0);
        }

        for (uint8_t i = 0; i < 5; ++i)
        {
            obj = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(GO_BOTTLE, BottleSpawnPoints[i].x, BottleSpawnPoints[i].y, BottleSpawnPoints[i].z, BottleSpawnPoints[i].orientation, 1);
            if (obj != nullptr)
                obj->Despawn(2 * 60 * 1000, 0);
        }

        for (uint8_t i = 0; i < 3; ++i)
        {
            obj = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(GO_BREAD, BreadSpawnPoints[i].x, BreadSpawnPoints[i].y, BreadSpawnPoints[i].z, BreadSpawnPoints[i].orientation, 1);
            if (obj != nullptr)
                obj->Despawn(2 * 60 * 1000, 0);
        }

        Creature* Negolash = pPlayer->GetMapMgr()->CreateAndSpawnCreature(1494, -14657.400391f, 155.115997f, 4.081050f, 0.353429f);
        if (Negolash != nullptr)
        {
            Negolash->GetAIInterface()->moveTo(-14647.526367f, 143.710052f, 1.164550f);
        }
    }
};

class NegolashAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NegolashAI(c); }
    explicit NegolashAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* /*mKiller*/) override
    {
        getCreature()->Despawn(180000, 0);
        RemoveAIUpdateEvent();
    }
};

void SetupStranglethornVale(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(1449, new StrFever());

    mgr->register_creature_script(1511, &Beka::Create);
    mgr->register_creature_script(1516, &Beka1::Create);
    mgr->register_quest_script(584, new BloodscalpClanHeads());
    mgr->register_quest_script(1118, new BacktoBootyBay());
    mgr->register_quest_script(609, new VoodooDues());
}
