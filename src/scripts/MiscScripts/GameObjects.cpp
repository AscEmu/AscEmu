/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

class TyraliusPrison : public GameObjectAIScript
{
public:

    TyraliusPrison(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TyraliusPrison(GO); }

    void OnActivate(Player* player)
    {
        LocationVector pos = player->GetPosition();
        Creature* creature = player->GetMapMgr()->CreateAndSpawnCreature(20787, pos.x, pos.y, pos.z, pos.o);
        if (creature != nullptr)
            creature->Despawn(2 * 60 * 1000, 0);
    }
};

class AndorhalTower1 : public GameObjectAIScript
{
public:

    AndorhalTower1(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new AndorhalTower1(GO); }

    void OnActivate(Player* player)
    {
        if (player->IsTeamHorde())
            player->AddQuestKill(5098, 0, 0);
        else
            player->AddQuestKill(5097, 0, 0);
    }
};

class AndorhalTower2 : public GameObjectAIScript
{
public:

    AndorhalTower2(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new AndorhalTower2(GO); }

    void OnActivate(Player* player)
    {
        if (player->IsTeamHorde())
            player->AddQuestKill(5098, 1, 0);
        else
            player->AddQuestKill(5097, 1, 0);
    }
};

class AndorhalTower3 : public GameObjectAIScript
{
public:

    AndorhalTower3(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new AndorhalTower3(GO); }

    void OnActivate(Player* player)
    {
        if (player->IsTeamHorde())
            player->AddQuestKill(5098, 2, 0);
        else
            player->AddQuestKill(5097, 2, 0);
    }
};

class AndorhalTower4 : public GameObjectAIScript
{
public:

    AndorhalTower4(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new AndorhalTower4(GO); }

    void OnActivate(Player* player)
    {
        if (player->IsTeamHorde())
            player->AddQuestKill(5098, 3, 0);
        else
            player->AddQuestKill(5097, 3, 0);
    }
};

class OrbOfCommand : public GameObjectAIScript
{
public:

    OrbOfCommand(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new OrbOfCommand(GO); }

    void OnActivate(Player* player)
    {
        if (player->HasFinishedQuest(7761) && player->getLevel() >= 58 && player->InGroup() == true)
        {
            player->SafeTeleport(469, 0, -7672.939941f, -1107.307617f, 396.649994f, 0.616532f);
        }
        else if (player->getLevel() <= 57 || player->HasFinishedQuest(7761) == false)
        {
            player->BroadcastMessage("You need to be level 58 and have completed the quest : Blackhand's Command");
        }
        else if (player->HasFinishedQuest(7761) == true && player->getLevel() >= 58 && player->InGroup() == false)
        {
            player->BroadcastMessage("You need to be in a raid group to be able to enter this instance");
        }
    }
};

class Blacksmithing_Plans_Use : public GameObjectAIScript
{
public:

    Blacksmithing_Plans_Use(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Blacksmithing_Plans_Use(GO); }

    void OnLootTaken(Player* player, ItemProperties const* /*itemProperties*/)
    {
        LocationVector pos = player->GetPosition();
        Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(11120, pos.x, pos.y, pos.z, pos.o, true, false, 0, 0);
        if (NewCreature != nullptr)
            NewCreature->Despawn(600000, 0);
    }
};

class GongOfBethekk : public GameObjectAIScript
{
public:

    GongOfBethekk(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new GongOfBethekk(GO); }

    void OnActivate(Player* player)
    {
        Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(14515, -11556.3f, -1628.32f, 41.299f, 4.1f, true, false, 0, 0);
        if (NewCreature != nullptr)
            NewCreature->Despawn(1200000, 0);
    }
};

class TerokksDownfall : public GameObjectAIScript
{
public:

    TerokksDownfall(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TerokksDownfall(GO); }

    void OnActivate(Player* player)
    {
        if (player->HasQuest(11073) && player->GetItemInterface()->GetItemCount(32720, 1))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(21838, pos.x, pos.y, pos.z, pos.o, true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
        else
        {
            player->BroadcastMessage("You need to have item : Time-Lost Offering and to have quest : Terokk's Downfall");
        }
    }
};

class VilebranchKidnapper : public GameObjectAIScript
{
public:

    VilebranchKidnapper(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new VilebranchKidnapper(GO); }

    void OnActivate(Player* player)
    {
        LocationVector pos = player->GetPosition();
        Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(14748, pos.x, pos.y + 1, pos.z, pos.o, true, false, 0, 0);
        if (NewCreature != nullptr)
            NewCreature->Despawn(600000, 0);

        NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(14748, pos.x, pos.y, pos.z, pos.o, true, false, 0, 0);
        if (NewCreature != nullptr)
            NewCreature->Despawn(600000, 0);

        NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(14748, pos.x - 1, pos.y, pos.z, pos.o, true, false, 0, 0);
        if (NewCreature != nullptr)
            NewCreature->Despawn(600000, 0);
    }
};

class GongOfZulFarrak : public GameObjectAIScript
{
public:

    GongOfZulFarrak(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new GongOfZulFarrak(GO); }

    void OnActivate(Player* player)
    {
        if (player->GetItemInterface()->GetItemCount(9240, 1))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(7273, pos.x, pos.y + 1, pos.z, pos.o, true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(1800000, 0);
        }
        else
        {
            player->BroadcastMessage("Missing required item : Mallet of Zul'Farrak");
        }
    }
};

class Obsidias_Egg : public GameObjectAIScript
{
public:

    Obsidias_Egg(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Obsidias_Egg(GO); }

    void OnActivate(Player* player)
    {
        if (player->HasQuest(11078))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(23282, pos.x, pos.y + 1, pos.z, pos.o, true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
        else
        {
            player->BroadcastMessage("Missing required quest : To Rule The Skies");
        }
    }
};

class Rivendarks_Egg : public GameObjectAIScript
{
public:

    Rivendarks_Egg(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Rivendarks_Egg(GO); }

    void OnActivate(Player* player)
    {
        if (player->HasQuest(11078))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(23061, pos.x, pos.y + 1, pos.z, pos.o, true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
        else
        {
            player->BroadcastMessage("Missing required quest : To Rule The Skies");
        }
    }
};

class Furywings_Egg : public GameObjectAIScript
{
public:

    Furywings_Egg(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Furywings_Egg(GO); }

    void OnActivate(Player* player)
    {
        if (player->HasQuest(11078))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(23261, pos.x, pos.y + 1, pos.z, pos.o, true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
        else
        {
            player->BroadcastMessage("Missing required quest : To Rule The Skies");
        }
    }
};

class Insidions_Egg : public GameObjectAIScript
{
public:

    Insidions_Egg(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Insidions_Egg(GO); }

    void OnActivate(Player* player)
    {
        if (player->HasQuest(11078))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(23281, pos.x, pos.y + 1, pos.z, pos.o, true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
        else
        {
            player->BroadcastMessage("Missing required quest : To Rule The Skies");
        }
    }
};

class Corrupt_Minor_Manifestation_Water_Object : public GameObjectAIScript
{
public:

    Corrupt_Minor_Manifestation_Water_Object(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Corrupt_Minor_Manifestation_Water_Object(GO); }

    void OnActivate(Player* player)
    {
        if (player->HasQuest(63))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(5894, pos.x, pos.y + 1, pos.z, pos.o, true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
        else
        {
            player->BroadcastMessage("Missing required quest : Call of Water");
        }
    }
};

class Telathion_the_Impure_Object : public GameObjectAIScript
{
public:

    Telathion_the_Impure_Object(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Telathion_the_Impure_Object(GO); }

    void OnActivate(Player* player)
    {
        if (player->HasQuest(9508))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(17359, pos.x, pos.y + 1, pos.z, pos.o, true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
        else
        {
            player->BroadcastMessage("Missing required quest : Call of Water");
        }
    }
};

class UlagTheCleaver : public GameObjectAIScript
{
public:

    UlagTheCleaver(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new UlagTheCleaver(GO); }

    void OnActivate(Player* player)
    {
        Creature* Ulag = player->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(2390.101807f, 336.676788f, 40.015614f, 6390);
        GameObject* pDoor = player->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2388.480029f, 338.3901f, 40.092899f, 176594);
        QuestLogEntry* en = player->GetQuestLogForEntry(1819);
        if (en == nullptr || pDoor == nullptr || Ulag == nullptr)
            return;

        Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(6390, 2390.101807f, 336.676788f, 40.015614f, 2.259590f, true, false, 0, 0);
        if (NewCreature != nullptr)
        {
            NewCreature->Despawn(180000, 0);
            NewCreature->GetAIInterface()->setNextTarget(player);
            NewCreature->GetAIInterface()->AttackReaction(player, 1);
            pDoor->SetFlags(33);
            pDoor->SetState(GO_STATE_OPEN);
        };
    }
};

class DustySpellbooks : public GameObjectAIScript
{
public:

    DustySpellbooks(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new DustySpellbooks(GO); }

    void OnLootTaken(Player* player, ItemProperties const* /*itemProperties*/)
    {
        QuestLogEntry* en = player->GetQuestLogForEntry(422);
        if (en == nullptr)
            return;

        LocationVector pos = player->GetPosition();
        Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(1770, pos.x, pos.y, pos.z, pos.o, true, false, 0, 0);
        if (NewCreature != nullptr)
            NewCreature->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "The Sons of Arugal will rise against all who challenge the power of the Moonrage!");
    }
};

class CatFigurine : public GameObjectAIScript
{
public:

    CatFigurine(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new CatFigurine(GO); }

    void OnActivate(Player* player)
    {
        uint32 Chance = Util::getRandomUInt(100);
        if (Chance <= 10)
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(3619, pos.x, pos.y + 1, pos.z, pos.o, true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
    }
};

class EthereumTransponderZeta : public GameObjectAIScript
{
public:

    EthereumTransponderZeta(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new EthereumTransponderZeta(GO); }

    void OnActivate(Player* player)
    {
        LocationVector pos = player->GetPosition();
        Creature* commander = player->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 20482);
        if (commander)
            return;

        if (player->HasQuest(10339))
        {
            float x = 4017.96f;
            float y = 2315.91f;
            float z = 116.418f;
            Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(20482, x, y, z, pos.o, true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(1 * 60 * 1000, 0);
        }
    }
};

class BringMetheEgg : public GameObjectAIScript
{
public:

    BringMetheEgg(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new BringMetheEgg(GO); }

    void OnActivate(Player* player)
    {
        QuestLogEntry* qle = player->GetQuestLogForEntry(10111);
        if (qle == nullptr)
            return;

        if (!player->HasAura(33382))
            player->CastSpell(player, 33382, true);
    }
};

class MysteriousEgg : public GameObjectAIScript
{
public:

    MysteriousEgg(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new MysteriousEgg(GO); }

    void OnActivate(Player* player)
    {
        player->AddQuestKill(10111, 0, 0);

        LocationVector pos = player->GetPosition();
        Creature* bird = player->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 19055);
        if (bird != nullptr)
            return;

        bird = player->GetMapMgr()->CreateAndSpawnCreature(19055, pos.x, pos.y, pos.z, pos.o);
        if (bird != nullptr)
            bird->Despawn(5 * 60 * 1000, 0);
    }
};

class AlterofTidalMastery : public GameObjectAIScript
{
public:

    AlterofTidalMastery(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new AlterofTidalMastery(GO); }

    void OnActivate(Player* player)
    {
        player->GetMapMgr()->GetInterface()->SpawnCreature(16292, 7934.343750f, -7637.020996f, 112.694130f, 3.098388f, true, false, 0, 0);
    }
};

class ShrineOfDathRemar : public GameObjectAIScript
{
public:

    ShrineOfDathRemar(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new ShrineOfDathRemar(GO); }

    void OnActivate(Player* player)
    {
        QuestLogEntry* qle = player->GetQuestLogForEntry(8345);
        if (qle != nullptr)
            qle->SendQuestComplete();
    }
};

class APlagueUponThee : public GameObjectAIScript
{
public:

    APlagueUponThee(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new APlagueUponThee(GO); }

    void OnActivate(Player* player)
    {
        if (player->HasQuest(5902) || player->HasQuest(5904))
        {
            LocationVector pos = player->GetPosition();
            GameObject* go = player->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(pos.x, pos.y, pos.z, 177491);
            if (go == nullptr)
            {
                GameObject* barel = player->GetMapMgr()->CreateAndSpawnGameObject(177491, 2449.51f, -1662.32f, 104.38f, 1.0f, 1);
                if (barel != nullptr)
                    barel->Despawn(2 * 60 * 1000, 0);
            }
        }
    }
};

class SerpentStatue : public GameObjectAIScript
{
public:

    SerpentStatue(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new SerpentStatue(GO); }

    void OnActivate(Player* player)
    {
        QuestLogEntry* qle = player->GetQuestLogForEntry(6027);
        if (qle == nullptr)
            return;

        Creature* naga = player->GetMapMgr()->CreateAndSpawnCreature(12369, 246.741f, 2953.3f, 5.8631f, 1.078f);
        if (naga != nullptr)
            naga->Despawn(6 * 60 * 1000, 0);
    }
};

class CuregosGold : public GameObjectAIScript
{
public:

    CuregosGold(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new CuregosGold(GO); }

    void OnActivate(Player* player)
    {
        QuestLogEntry* qle = player->GetQuestLogForEntry(2882);
        if (qle == nullptr)
            return;

        LocationVector pos = player->GetPosition();

        Creature* pirate = player->GetMapMgr()->CreateAndSpawnCreature(7899, pos.x + Util::getRandomFloat(5.0f), pos.y + Util::getRandomFloat(5.0f), pos.z, pos.o);
        if (pirate != nullptr)
            pirate->Despawn(6 * 60 * 1000, 0);

        pirate = player->GetMapMgr()->CreateAndSpawnCreature(7899, pos.x - Util::getRandomFloat(5.0f), pos.y + Util::getRandomFloat(5.0f), pos.z, pos.o);
        if (pirate != nullptr)
            pirate->Despawn(6 * 60 * 1000, 0);

        pirate = player->GetMapMgr()->CreateAndSpawnCreature(7901, pos.x + Util::getRandomFloat(5.0f), pos.y - Util::getRandomFloat(5.0f), pos.z, pos.o);
        if (pirate != nullptr)
            pirate->Despawn(6 * 60 * 1000, 0);

        pirate = player->GetMapMgr()->CreateAndSpawnCreature(7901, pos.x + Util::getRandomFloat(5.0f), pos.y + Util::getRandomFloat(5.0f), pos.z, pos.o);
        if (pirate != nullptr)
            pirate->Despawn(6 * 60 * 1000, 0);

        pirate = player->GetMapMgr()->CreateAndSpawnCreature(7902, pos.x - Util::getRandomFloat(5.0f), pos.y - Util::getRandomFloat(5.0f), pos.z, pos.o);
        if (pirate != nullptr)
            pirate->Despawn(6 * 60 * 1000, 0);

        GameObject* gobj = player->GetMapMgr()->CreateAndSpawnGameObject(142194, pos.x + 5, pos.y, pos.z, pos.o, 1);
        if (gobj != nullptr)
            gobj->Despawn(10 * 60 * 1000, 0);
    }
};

class DreadmaulRock : public GameObjectAIScript
{
public:

    DreadmaulRock(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new DreadmaulRock(GO); }

    void OnActivate(Player* player)
    {
        QuestLogEntry* qle = player->GetQuestLogForEntry(3821);
        if (qle == nullptr)
            return;

        LocationVector pos = player->GetPosition();

        Creature* shaghost = player->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 9136);
        if (shaghost)
            return;

        Creature* shaghostspawn = player->GetMapMgr()->CreateAndSpawnCreature(9136, pos.x, pos.y, pos.z, pos.o);
        if (shaghostspawn != nullptr)
            shaghostspawn->Despawn(2 * 60 * 1000, 0);
    }
};

class HandofIruxos : public GameObjectAIScript
{
public:

    HandofIruxos(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new HandofIruxos(GO); }

    void OnActivate(Player* player)
    {
        QuestLogEntry* qle = player->GetQuestLogForEntry(5381);
        if (qle == nullptr)
            return;

        Creature* demon = player->GetMapMgr()->CreateAndSpawnCreature(11876, -348.231f, 1763.85f, 138.371f, 4.42728f);
        if (demon != nullptr)
            demon->Despawn(6 * 60 * 1000, 0);
    }
};

class LegionPortals : public GameObjectAIScript
{
public:

    LegionPortals(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new LegionPortals(GO); }

    void OnActivate(Player* player)
    {
        player->AddQuestKill(5581, 0, 0);
    }
};

class ProphecyofAkida : public GameObjectAIScript
{
public:

    ProphecyofAkida(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new ProphecyofAkida(GO); }

    void OnActivate(Player* player)
    {
        player->AddQuestKill(9544, 0, 0);

        LocationVector pos = player->GetPosition();

        Creature* prisoner = player->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 17375);
        if (prisoner != nullptr)
            prisoner->Despawn(1, 6 * 60 * 1000);
    }
};

class Razormaw : public GameObjectAIScript
{
public:

    Razormaw(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Razormaw(GO); }

    void OnActivate(Player* player)
    {
        QuestLogEntry* qle = player->GetQuestLogForEntry(9689);
        if (qle == nullptr)
            return;

        Creature* razormaw = player->GetMapMgr()->CreateAndSpawnCreature(17592, -1203.8f, -12424.7f, 95.36f, 4.7f);
        if (razormaw != nullptr)
            razormaw->Despawn(6 * 60 * 1000, 0);
    }
};

class TabletoftheSeven : public GameObjectAIScript
{
public:

    TabletoftheSeven(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TabletoftheSeven(GO); }

    void OnActivate(Player* player)
    {
        QuestLogEntry* qle = player->GetQuestLogForEntry(4296);
        if (qle == nullptr)
            return;

        if (player->GetItemInterface()->GetItemCount(11470, 0) < 1)
            player->GetItemInterface()->AddItemById(11470, 1, 0);
    }
};

class TestofEndurance : public GameObjectAIScript
{
public:

    TestofEndurance(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TestofEndurance(GO); }

    void OnActivate(Player* player)
    {
        QuestLogEntry* qle = player->GetQuestLogForEntry(1150);
        if (qle == nullptr)
            return;

        LocationVector pos = player->GetPosition();

        Creature* grenka = player->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 4490);
        if (grenka != nullptr)
        {
            if (!grenka->isAlive())
                grenka->Despawn(5000, 120000);
            else
                return;
        }

        Creature* grenkaspawn = player->GetMapMgr()->CreateAndSpawnCreature(4490, pos.x, pos.y, pos.z, pos.o);
        if (grenkaspawn != nullptr)
            grenkaspawn->Despawn(6 * 60 * 1000, 0);
    }
};

class TheFallenExarch : public GameObjectAIScript
{
public:

    TheFallenExarch(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TheFallenExarch(GO); }

    void OnActivate(Player* player)
    {
        QuestLogEntry* qle = player->GetQuestLogForEntry(10915);
        if (qle == nullptr)
            return;

        Creature* exarch = player->GetMapMgr()->CreateAndSpawnCreature(22452, -3365.9f, 5143.19f, -9.00132f, 3.05f);
        if (exarch != nullptr)
            exarch->Despawn(6 * 60 * 1000, 0);
    }
};

class TheFinalCode : public GameObjectAIScript
{
public:

    TheFinalCode(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TheFinalCode(GO); }

    void OnActivate(Player* player)
    {
        player->AddQuestKill(10447, 1, 0);
    }
};

class TheRootofAllEvil : public GameObjectAIScript
{
public:

    TheRootofAllEvil(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TheRootofAllEvil(GO); }

    void OnActivate(Player* player)
    {
        QuestLogEntry* qle = player->GetQuestLogForEntry(8481);
        if (qle == nullptr)
            return;

        LocationVector pos = player->GetPosition();

        Creature* xandivious = player->GetMapMgr()->CreateAndSpawnCreature(15623, pos.x + 5, pos.y, pos.z, pos.o);
        if (xandivious != nullptr)
            xandivious->Despawn(6 * 60 * 1000, 0);
    }
};

class TheThunderspike : public GameObjectAIScript
{
public:
    TheThunderspike(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TheThunderspike(GO); }

    void OnActivate(Player* player)
    {
        if (!player->HasQuest(10526))
            return;

        LocationVector pos = player->GetPosition();

        // Wth is that ? To remove ?
        GameObject* gobj = player->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(pos.x, pos.y, pos.z, 184729);
        if (gobj != nullptr)
            gobj->Despawn(6 * 60 * 1000, 0);

        Creature* spike = player->GetMapMgr()->CreateAndSpawnCreature(21319, 1315.54f, 6688.33f, -18, 0.001f);
        if (spike != nullptr)
            spike->Despawn(5 * 60 * 1000, 0);
    }
};

class StrengthofOne : public GameObjectAIScript
{
public:

    StrengthofOne(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new StrengthofOne(GO); }

    void OnActivate(Player* player)
    {
        QuestLogEntry* qle = player->GetQuestLogForEntry(9582);
        if (qle == nullptr)
            return;

        LocationVector pos = player->GetPosition();

        // What is this ? :O To remove ?
        Creature* reaver = player->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 17556);
        if (reaver)
        {
            LocationVector pos2 = reaver->GetPosition();
            reaver->Despawn(1, 5 * 60 * 1000);

            Creature* reaver2 = player->GetMapMgr()->GetInterface()->SpawnCreature(17556, pos2.x, pos2.y, pos2.z, pos2.o, true, false, 0, 0);
            if (reaver2 != nullptr)
                reaver2->Despawn(5 * 60 * 1000, 0);
        }
    }
};

class HealingTheLake : public GameObjectAIScript
{
public:

    HealingTheLake(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new HealingTheLake(GO); }

    void OnActivate(Player* player)
    {
        player->AddQuestKill(181433, 0, 0);
    }
};

class TheRavensClaw : public GameObjectAIScript
{
public:

    TheRavensClaw(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TheRavensClaw(GO); }

    void OnActivate(Player* player)
    {
        if (player->GetMapMgr()->iInstanceMode == MODE_HEROIC)
            player->GetMapMgr()->CreateAndSpawnCreature(23035, -87.3546f, 288.006f, 26.4832f, 0);
    }
};

class DeathGate1 : public GameObjectAIScript
{
public:

    DeathGate1(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new DeathGate1(GO); }

    void OnSpawn()
    {
        RegisterAIUpdateEvent(500);
    }

    void AIUpdate()
    {
        LocationVector pos = _gameobject->GetPosition();
        Player* player = _gameobject->GetMapMgr()->GetInterface()->GetPlayerNearestCoords(pos.x, pos.y, pos.z);
        if (player == nullptr)
            return;

        if (_gameobject->CalcDistance(_gameobject, player) <= 1.5f && player->GetMapId() == 609)
            player->CastSpell(player, 54699, true);
        else if (_gameobject->CalcDistance(_gameobject, player) <= 1.5f && player->GetMapId() == 0)
            player->CastSpell(player, 54744, true);
    }
};

class DeathGate2 : public GameObjectAIScript
{
public:
    DeathGate2(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new DeathGate2(GO); }

    void OnSpawn()
    {
        RegisterAIUpdateEvent(500);
    }

    void AIUpdate()
    {
        LocationVector pos = _gameobject->GetPosition();
        Player* player = _gameobject->GetMapMgr()->GetInterface()->GetPlayerNearestCoords(pos.x, pos.y, pos.z);
        if (player == nullptr)
            return;

        if (_gameobject->CalcDistance(_gameobject, player) <= 1.5f && player->GetMapId() == 609)
            player->CastSpell(player, 54725, true);
        else if (_gameobject->CalcDistance(_gameobject, player) <= 1.5f && player->GetMapId() == 0)
            player->CastSpell(player, 54746, true);
    }
};

class DeathGate3 : public GameObjectAIScript
{
public:

    DeathGate3(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new DeathGate3(GO); }

    void OnActivate(Player* player)
    {
        if (player->getClass() == 6)
        {
            if (player->GetMapId() == 609)
                player->CastSpell(player, 53098, true);
            else
                player->CastSpell(player, 53822, true);
        }
    }
};

class SacredFireofLife : public GameObjectAIScript
{
public:

    SacredFireofLife(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new SacredFireofLife(GO); }

    void OnActivate(Player* player)
    {
        LocationVector pos = player->GetPosition();
        Creature* NewCreature = player->GetMapMgr()->GetInterface()->SpawnCreature(10882, pos.x, pos.y, pos.y, pos.o, true, false, 0, 0);
        if (NewCreature != nullptr)
            NewCreature->Despawn(600000, 0);
    }
};


void SetupGoHandlers(ScriptMgr* mgr)
{
    mgr->register_gameobject_script(179879, &OrbOfCommand::Create);
    mgr->register_gameobject_script(173232, &Blacksmithing_Plans_Use::Create);
    mgr->register_gameobject_script(180526, &GongOfBethekk::Create);
    mgr->register_gameobject_script(185928, &TerokksDownfall::Create);
    mgr->register_gameobject_script(179910, &VilebranchKidnapper::Create);
    mgr->register_gameobject_script(141832, &GongOfZulFarrak::Create);
    mgr->register_gameobject_script(185932, &Obsidias_Egg::Create);
    mgr->register_gameobject_script(185936, &Rivendarks_Egg::Create);
    mgr->register_gameobject_script(185937, &Furywings_Egg::Create);
    mgr->register_gameobject_script(185938, &Insidions_Egg::Create);
    mgr->register_gameobject_script(113791, &Corrupt_Minor_Manifestation_Water_Object::Create);
    mgr->register_gameobject_script(181699, &Telathion_the_Impure_Object::Create);
    mgr->register_gameobject_script(104593, &UlagTheCleaver::Create);
    mgr->register_gameobject_script(1571, &DustySpellbooks::Create);
    mgr->register_gameobject_script(13873, &CatFigurine::Create);
    mgr->register_gameobject_script(184383, &EthereumTransponderZeta::Create);
    mgr->register_gameobject_script(183146, &BringMetheEgg::Create);
    mgr->register_gameobject_script(183147, &MysteriousEgg::Create);
    mgr->register_gameobject_script(180516, &ShrineOfDathRemar::Create);
    mgr->register_gameobject_script(177490, &APlagueUponThee::Create);
    mgr->register_gameobject_script(177673, &SerpentStatue::Create);
    mgr->register_gameobject_script(142189, &CuregosGold::Create);
    mgr->register_gameobject_script(160445, &DreadmaulRock::Create);
    mgr->register_gameobject_script(176581, &HandofIruxos::Create);
    mgr->register_gameobject_script(177400, &LegionPortals::Create);
    mgr->register_gameobject_script(181730, &ProphecyofAkida::Create);
    mgr->register_gameobject_script(181988, &Razormaw::Create);
    mgr->register_gameobject_script(169294, &TabletoftheSeven::Create);
    mgr->register_gameobject_script(20447, &TestofEndurance::Create);
    mgr->register_gameobject_script(184999, &TheFallenExarch::Create);
    mgr->register_gameobject_script(184725, &TheFinalCode::Create);
    mgr->register_gameobject_script(180672, &TheRootofAllEvil::Create);
    mgr->register_gameobject_script(184729, &TheThunderspike::Create);
    mgr->register_gameobject_script(181849, &StrengthofOne::Create);
    mgr->register_gameobject_script(181433, &HealingTheLake::Create);
    mgr->register_gameobject_script(185554, &TheRavensClaw::Create);
    mgr->register_gameobject_script(191538, &DeathGate1::Create);
    mgr->register_gameobject_script(191539, &DeathGate2::Create);
    mgr->register_gameobject_script(190942, &DeathGate3::Create);
    mgr->register_gameobject_script(175944, &SacredFireofLife::Create);

    mgr->register_gameobject_script(310030, &AndorhalTower1::Create);
    mgr->register_gameobject_script(310031, &AndorhalTower2::Create);
    mgr->register_gameobject_script(310032, &AndorhalTower3::Create);
    mgr->register_gameobject_script(310033, &AndorhalTower4::Create);

    mgr->register_gameobject_script(184588, &TyraliusPrison::Create);
}
