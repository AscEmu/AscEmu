/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/ItemInterface.h"
#include "Management/QuestLogEntry.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Utilities/Random.hpp"

class TyraliusPrison : public GameObjectAIScript
{
public:
    explicit TyraliusPrison(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TyraliusPrison(GO); }

    void OnActivate(Player* player) override
    {
        LocationVector pos = player->GetPosition();
        Creature* creature = player->getWorldMap()->createAndSpawnCreature(20787, pos);
        if (creature != nullptr)
            creature->Despawn(2 * 60 * 1000, 0);
    }
};

class AndorhalTower1 : public GameObjectAIScript
{
public:
    explicit AndorhalTower1(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new AndorhalTower1(GO); }

    void OnActivate(Player* player) override
    {
        if (player->isTeamHorde())
            player->addQuestKill(5098, 0, 0);
        else
            player->addQuestKill(5097, 0, 0);
    }
};

class AndorhalTower2 : public GameObjectAIScript
{
public:
    explicit AndorhalTower2(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new AndorhalTower2(GO); }

    void OnActivate(Player* player) override
    {
        if (player->isTeamHorde())
            player->addQuestKill(5098, 1, 0);
        else
            player->addQuestKill(5097, 1, 0);
    }
};

class AndorhalTower3 : public GameObjectAIScript
{
public:
    explicit AndorhalTower3(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new AndorhalTower3(GO); }

    void OnActivate(Player* player) override
    {
        if (player->isTeamHorde())
            player->addQuestKill(5098, 2, 0);
        else
            player->addQuestKill(5097, 2, 0);
    }
};

class AndorhalTower4 : public GameObjectAIScript
{
public:
    explicit AndorhalTower4(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new AndorhalTower4(GO); }

    void OnActivate(Player* player) override
    {
        if (player->isTeamHorde())
            player->addQuestKill(5098, 3, 0);
        else
            player->addQuestKill(5097, 3, 0);
    }
};

class OrbOfCommand : public GameObjectAIScript
{
public:
    explicit OrbOfCommand(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new OrbOfCommand(GO); }

    void OnActivate(Player* player) override
    {
        if (player->hasQuestFinished(7761) && player->getLevel() >= 58 && player->isInGroup())
        {
            player->safeTeleport(469, 0, LocationVector(-7672.939941f, -1107.307617f, 396.649994f, 0.616532f));
        }
        else if (player->getLevel() <= 57 || player->hasQuestFinished(7761) == false)
        {
            player->broadcastMessage("You need to be level 58 and have completed the quest : Blackhand's Command");
        }
        else if (player->hasQuestFinished(7761) == true && player->getLevel() >= 58 && !player->isInGroup())
        {
            player->broadcastMessage("You need to be in a raid group to be able to enter this instance");
        }
    }
};

class Blacksmithing_Plans_Use : public GameObjectAIScript
{
public:
    explicit Blacksmithing_Plans_Use(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Blacksmithing_Plans_Use(GO); }

    void OnLootTaken(Player* player, ItemProperties const* /*itemProperties*/) override
    {
        LocationVector pos = player->GetPosition();
        Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(11120, pos, true, false, 0, 0);
        if (NewCreature != nullptr)
            NewCreature->Despawn(600000, 0);
    }
};

class GongOfBethekk : public GameObjectAIScript
{
public:
    explicit GongOfBethekk(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new GongOfBethekk(GO); }

    void OnActivate(Player* player) override
    {
        Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(14515, LocationVector(-11556.3f, -1628.32f, 41.299f, 4.1f), true, false, 0, 0);
        if (NewCreature != nullptr)
            NewCreature->Despawn(1200000, 0);
    }
};

class TerokksDownfall : public GameObjectAIScript
{
public:
    explicit TerokksDownfall(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TerokksDownfall(GO); }

    void OnActivate(Player* player) override
    {
        if (player->hasQuestInQuestLog(11073) && player->getItemInterface()->GetItemCount(32720, 1))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(21838, pos, true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
        else
        {
            player->broadcastMessage("You need to have item : Time-Lost Offering and to have quest : Terokk's Downfall");
        }
    }
};

class VilebranchKidnapper : public GameObjectAIScript
{
public:
    explicit VilebranchKidnapper(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new VilebranchKidnapper(GO); }

    void OnActivate(Player* player) override
    {
        LocationVector pos = player->GetPosition();
        Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(14748, LocationVector(pos.x, pos.y + 1, pos.z, pos.o), true, false, 0, 0);
        if (NewCreature != nullptr)
            NewCreature->Despawn(600000, 0);

        NewCreature = player->getWorldMap()->getInterface()->spawnCreature(14748, pos, true, false, 0, 0);
        if (NewCreature != nullptr)
            NewCreature->Despawn(600000, 0);

        NewCreature = player->getWorldMap()->getInterface()->spawnCreature(14748, LocationVector(pos.x - 1, pos.y, pos.z, pos.o), true, false, 0, 0);
        if (NewCreature != nullptr)
            NewCreature->Despawn(600000, 0);
    }
};

class GongOfZulFarrak : public GameObjectAIScript
{
public:
    explicit GongOfZulFarrak(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new GongOfZulFarrak(GO); }

    void OnActivate(Player* player) override
    {
        if (player->getItemInterface()->GetItemCount(9240, 1))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(7273, LocationVector(pos.x, pos.y + 1, pos.z, pos.o), true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(1800000, 0);
        }
        else
        {
            player->broadcastMessage("Missing required item : Mallet of Zul'Farrak");
        }
    }
};

class Obsidias_Egg : public GameObjectAIScript
{
public:
    explicit Obsidias_Egg(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Obsidias_Egg(GO); }

    void OnActivate(Player* player) override
    {
        if (player->hasQuestInQuestLog(11078))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(23282, LocationVector(pos.x, pos.y + 1, pos.z, pos.o), true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
        else
        {
            player->broadcastMessage("Missing required quest : To Rule The Skies");
        }
    }
};

class Rivendarks_Egg : public GameObjectAIScript
{
public:
    explicit Rivendarks_Egg(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Rivendarks_Egg(GO); }

    void OnActivate(Player* player) override
    {
        if (player->hasQuestInQuestLog(11078))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(23061, LocationVector(pos.x, pos.y + 1, pos.z, pos.o), true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
        else
        {
            player->broadcastMessage("Missing required quest : To Rule The Skies");
        }
    }
};

class Furywings_Egg : public GameObjectAIScript
{
public:
    explicit Furywings_Egg(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Furywings_Egg(GO); }

    void OnActivate(Player* player) override
    {
        if (player->hasQuestInQuestLog(11078))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(23261, LocationVector(pos.x, pos.y + 1, pos.z, pos.o), true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
        else
        {
            player->broadcastMessage("Missing required quest : To Rule The Skies");
        }
    }
};

class Insidions_Egg : public GameObjectAIScript
{
public:
    explicit Insidions_Egg(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Insidions_Egg(GO); }

    void OnActivate(Player* player) override
    {
        if (player->hasQuestInQuestLog(11078))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(23281, LocationVector(pos.x, pos.y + 1, pos.z, pos.o), true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
        else
        {
            player->broadcastMessage("Missing required quest : To Rule The Skies");
        }
    }
};

class Corrupt_Minor_Manifestation_Water_Object : public GameObjectAIScript
{
public:
    explicit Corrupt_Minor_Manifestation_Water_Object(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Corrupt_Minor_Manifestation_Water_Object(GO); }

    void OnActivate(Player* player) override
    {
        if (player->hasQuestInQuestLog(63))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(5894, LocationVector(pos.x, pos.y + 1, pos.z, pos.o), true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
        else
        {
            player->broadcastMessage("Missing required quest : Call of Water");
        }
    }
};

class Telathion_the_Impure_Object : public GameObjectAIScript
{
public:
    explicit Telathion_the_Impure_Object(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Telathion_the_Impure_Object(GO); }

    void OnActivate(Player* player) override
    {
        if (player->hasQuestInQuestLog(9508))
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(17359, LocationVector(pos.x, pos.y + 1, pos.z, pos.o), true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
        else
        {
            player->broadcastMessage("Missing required quest : Call of Water");
        }
    }
};

class UlagTheCleaver : public GameObjectAIScript
{
public:
    explicit UlagTheCleaver(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new UlagTheCleaver(GO); }

    void OnActivate(Player* player) override
    {
        Creature* Ulag = player->getWorldMap()->getInterface()->getCreatureNearestCoords(2390.101807f, 336.676788f, 40.015614f, 6390);
        GameObject* pDoor = player->getWorldMap()->getInterface()->getGameObjectNearestCoords(2388.480029f, 338.3901f, 40.092899f, 176594);
        QuestLogEntry* en = player->getQuestLogByQuestId(1819);
        if (en == nullptr || pDoor == nullptr || Ulag == nullptr)
            return;

        Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(6390, LocationVector(2390.101807f, 336.676788f, 40.015614f, 2.259590f), true, false, 0, 0);
        if (NewCreature != nullptr)
        {
            NewCreature->Despawn(180000, 0);
            NewCreature->getAIInterface()->setCurrentTarget(player);
            NewCreature->getAIInterface()->onHostileAction(player);
            pDoor->setFlags(GO_FLAG_NONSELECTABLE | GO_FLAG_NEVER_DESPAWN);
            pDoor->setState(GO_STATE_OPEN);
        };
    }
};

class DustySpellbooks : public GameObjectAIScript
{
public:
    explicit DustySpellbooks(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new DustySpellbooks(GO); }

    void OnLootTaken(Player* player, ItemProperties const* /*itemProperties*/) override
    {
        QuestLogEntry* en = player->getQuestLogByQuestId(422);
        if (en == nullptr)
            return;

        LocationVector pos = player->GetPosition();
        Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(1770, pos, true, false, 0, 0);
        if (NewCreature != nullptr)
            NewCreature->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "The Sons of Arugal will rise against all who challenge the power of the Moonrage!");
    }
};

class CatFigurine : public GameObjectAIScript
{
public:
    explicit CatFigurine(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new CatFigurine(GO); }

    void OnActivate(Player* player) override
    {
        uint32_t Chance = Util::getRandomUInt(100);
        if (Chance <= 10)
        {
            LocationVector pos = player->GetPosition();
            Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(3619, LocationVector(pos.x, pos.y + 1, pos.z, pos.o), true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(600000, 0);
        }
    }
};

class EthereumTransponderZeta : public GameObjectAIScript
{
public:
    explicit EthereumTransponderZeta(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new EthereumTransponderZeta(GO); }

    void OnActivate(Player* player) override
    {
        LocationVector pos = player->GetPosition();
        Creature* commander = player->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 20482);
        if (commander)
            return;

        if (player->hasQuestInQuestLog(10339))
        {
            float x = 4017.96f;
            float y = 2315.91f;
            float z = 116.418f;
            Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(20482, LocationVector(x, y, z, pos.o), true, false, 0, 0);
            if (NewCreature != nullptr)
                NewCreature->Despawn(1 * 60 * 1000, 0);
        }
    }
};

class BringMetheEgg : public GameObjectAIScript
{
public:
    explicit BringMetheEgg(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new BringMetheEgg(GO); }

    void OnActivate(Player* player) override
    {
        QuestLogEntry* qle = player->getQuestLogByQuestId(10111);
        if (qle == nullptr)
            return;

        if (!player->hasAurasWithId(33382))
            player->castSpell(player, 33382, true);
    }
};

class MysteriousEgg : public GameObjectAIScript
{
public:
    explicit MysteriousEgg(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new MysteriousEgg(GO); }

    void OnActivate(Player* player) override
    {
        player->addQuestKill(10111, 0, 0);

        LocationVector pos = player->GetPosition();
        Creature* bird = player->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 19055);
        if (bird != nullptr)
            return;

        bird = player->getWorldMap()->createAndSpawnCreature(19055, pos);
        if (bird != nullptr)
            bird->Despawn(5 * 60 * 1000, 0);
    }
};

class AlterofTidalMastery : public GameObjectAIScript
{
public:
    explicit AlterofTidalMastery(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new AlterofTidalMastery(GO); }

    void OnActivate(Player* player) override
    {
        player->getWorldMap()->getInterface()->spawnCreature(16292, LocationVector(7934.343750f, -7637.020996f, 112.694130f, 3.098388f), true, false, 0, 0);
    }
};

class ShrineOfDathRemar : public GameObjectAIScript
{
public:
    explicit ShrineOfDathRemar(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new ShrineOfDathRemar(GO); }

    void OnActivate(Player* player) override
    {
        QuestLogEntry* qle = player->getQuestLogByQuestId(8345);
        if (qle != nullptr)
            qle->sendQuestComplete();
    }
};

class APlagueUponThee : public GameObjectAIScript
{
public:
    explicit APlagueUponThee(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new APlagueUponThee(GO); }

    void OnActivate(Player* player) override
    {
        if (player->hasQuestInQuestLog(5902) || player->hasQuestInQuestLog(5904))
        {
            GameObject* go = player->getWorldMap()->getInterface()->getGameObjectNearestCoords(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), 177491);
            if (go == nullptr)
            {
                GameObject* barel = player->getWorldMap()->createAndSpawnGameObject(177491, LocationVector(2449.51f, -1662.32f, 104.38f, 1.0f), 1);
                if (barel != nullptr)
                    barel->despawn(2 * 60 * 1000, 0);
            }
        }
    }
};

class SerpentStatue : public GameObjectAIScript
{
public:
    explicit SerpentStatue(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new SerpentStatue(GO); }

    void OnActivate(Player* player) override
    {
        QuestLogEntry* qle = player->getQuestLogByQuestId(6027);
        if (qle == nullptr)
            return;

        Creature* naga = player->getWorldMap()->createAndSpawnCreature(12369, LocationVector(246.741f, 2953.3f, 5.8631f, 1.078f));
        if (naga != nullptr)
            naga->Despawn(6 * 60 * 1000, 0);
    }
};

class CuregosGold : public GameObjectAIScript
{
public:
    explicit CuregosGold(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new CuregosGold(GO); }

    void OnActivate(Player* player) override
    {
        QuestLogEntry* qle = player->getQuestLogByQuestId(2882);
        if (qle == nullptr)
            return;

        LocationVector pos = player->GetPosition();

        Creature* pirate = player->getWorldMap()->createAndSpawnCreature(7899, LocationVector(pos.x + Util::getRandomFloat(5.0f), pos.y + Util::getRandomFloat(5.0f), pos.z, pos.o));
        if (pirate != nullptr)
            pirate->Despawn(6 * 60 * 1000, 0);

        pirate = player->getWorldMap()->createAndSpawnCreature(7899, LocationVector(pos.x - Util::getRandomFloat(5.0f), pos.y + Util::getRandomFloat(5.0f), pos.z, pos.o));
        if (pirate != nullptr)
            pirate->Despawn(6 * 60 * 1000, 0);

        pirate = player->getWorldMap()->createAndSpawnCreature(7901, LocationVector(pos.x + Util::getRandomFloat(5.0f), pos.y - Util::getRandomFloat(5.0f), pos.z, pos.o));
        if (pirate != nullptr)
            pirate->Despawn(6 * 60 * 1000, 0);

        pirate = player->getWorldMap()->createAndSpawnCreature(7901, LocationVector(pos.x + Util::getRandomFloat(5.0f), pos.y + Util::getRandomFloat(5.0f), pos.z, pos.o));
        if (pirate != nullptr)
            pirate->Despawn(6 * 60 * 1000, 0);

        pirate = player->getWorldMap()->createAndSpawnCreature(7902, LocationVector(pos.x - Util::getRandomFloat(5.0f), pos.y - Util::getRandomFloat(5.0f), pos.z, pos.o));
        if (pirate != nullptr)
            pirate->Despawn(6 * 60 * 1000, 0);

        GameObject* gobj = player->getWorldMap()->createAndSpawnGameObject(142194, LocationVector(pos.x + 5, pos.y, pos.z, pos.o), 1);
        if (gobj != nullptr)
            gobj->despawn(10 * 60 * 1000, 0);
    }
};

class DreadmaulRock : public GameObjectAIScript
{
public:
    explicit DreadmaulRock(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new DreadmaulRock(GO); }

    void OnActivate(Player* player) override
    {
        QuestLogEntry* qle = player->getQuestLogByQuestId(3821);
        if (qle == nullptr)
            return;

        LocationVector pos = player->GetPosition();

        Creature* shaghost = player->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 9136);
        if (shaghost)
            return;

        Creature* shaghostspawn = player->getWorldMap()->createAndSpawnCreature(9136, pos);
        if (shaghostspawn != nullptr)
            shaghostspawn->Despawn(2 * 60 * 1000, 0);
    }
};

class HandofIruxos : public GameObjectAIScript
{
public:
    explicit HandofIruxos(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new HandofIruxos(GO); }

    void OnActivate(Player* player) override
    {
        QuestLogEntry* qle = player->getQuestLogByQuestId(5381);
        if (qle == nullptr)
            return;

        Creature* demon = player->getWorldMap()->createAndSpawnCreature(11876, LocationVector(-348.231f, 1763.85f, 138.371f, 4.42728f));
        if (demon != nullptr)
            demon->Despawn(6 * 60 * 1000, 0);
    }
};

class LegionPortals : public GameObjectAIScript
{
public:
    explicit LegionPortals(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new LegionPortals(GO); }

    void OnActivate(Player* player) override
    {
        player->addQuestKill(5581, 0, 0);
    }
};

class ProphecyofAkida : public GameObjectAIScript
{
public:
    explicit ProphecyofAkida(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new ProphecyofAkida(GO); }

    void OnActivate(Player* player) override
    {
        player->addQuestKill(9544, 0, 0);

        LocationVector pos = player->GetPosition();

        Creature* prisoner = player->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 17375);
        if (prisoner != nullptr)
            prisoner->Despawn(1, 6 * 60 * 1000);
    }
};

class Razormaw : public GameObjectAIScript
{
public:
    explicit Razormaw(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new Razormaw(GO); }

    void OnActivate(Player* player) override
    {
        QuestLogEntry* qle = player->getQuestLogByQuestId(9689);
        if (qle == nullptr)
            return;

        Creature* razormaw = player->getWorldMap()->createAndSpawnCreature(17592, LocationVector(-1203.8f, -12424.7f, 95.36f, 4.7f));
        if (razormaw != nullptr)
            razormaw->Despawn(6 * 60 * 1000, 0);
    }
};

class TabletoftheSeven : public GameObjectAIScript
{
public:
    explicit TabletoftheSeven(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TabletoftheSeven(GO); }

    void OnActivate(Player* player) override
    {
        QuestLogEntry* qle = player->getQuestLogByQuestId(4296);
        if (qle == nullptr)
            return;

        if (player->getItemInterface()->GetItemCount(11470, 0) < 1)
            player->getItemInterface()->AddItemById(11470, 1, 0);
    }
};

class TestofEndurance : public GameObjectAIScript
{
public:
    explicit TestofEndurance(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TestofEndurance(GO); }

    void OnActivate(Player* player) override
    {
        QuestLogEntry* qle = player->getQuestLogByQuestId(1150);
        if (qle == nullptr)
            return;

        LocationVector pos = player->GetPosition();

        Creature* grenka = player->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 4490);
        if (grenka != nullptr)
        {
            if (!grenka->isAlive())
                grenka->Despawn(5000, 120000);
            else
                return;
        }

        Creature* grenkaspawn = player->getWorldMap()->createAndSpawnCreature(4490, pos);
        if (grenkaspawn != nullptr)
            grenkaspawn->Despawn(6 * 60 * 1000, 0);
    }
};

class TheFallenExarch : public GameObjectAIScript
{
public:
    explicit TheFallenExarch(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TheFallenExarch(GO); }

    void OnActivate(Player* player) override
    {
        QuestLogEntry* qle = player->getQuestLogByQuestId(10915);
        if (qle == nullptr)
            return;

        Creature* exarch = player->getWorldMap()->createAndSpawnCreature(22452, LocationVector(-3365.9f, 5143.19f, -9.00132f, 3.05f));
        if (exarch != nullptr)
            exarch->Despawn(6 * 60 * 1000, 0);
    }
};

class TheFinalCode : public GameObjectAIScript
{
public:
    explicit TheFinalCode(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TheFinalCode(GO); }

    void OnActivate(Player* player) override
    {
        player->addQuestKill(10447, 1, 0);
    }
};

class TheRootofAllEvil : public GameObjectAIScript
{
public:
    explicit TheRootofAllEvil(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TheRootofAllEvil(GO); }

    void OnActivate(Player* player) override
    {
        QuestLogEntry* qle = player->getQuestLogByQuestId(8481);
        if (qle == nullptr)
            return;

        LocationVector pos = player->GetPosition();

        Creature* xandivious = player->getWorldMap()->createAndSpawnCreature(15623, LocationVector(pos.x + 5, pos.y, pos.z, pos.o));
        if (xandivious != nullptr)
            xandivious->Despawn(6 * 60 * 1000, 0);
    }
};

class TheThunderspike : public GameObjectAIScript
{
public:
    explicit TheThunderspike(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TheThunderspike(GO); }

    void OnActivate(Player* player) override
    {
        if (!player->hasQuestInQuestLog(10526))
            return;

        // Wth is that ? To remove ?
        GameObject* gobj = player->getWorldMap()->getInterface()->getGameObjectNearestCoords(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), 184729);
        if (gobj != nullptr)
            gobj->despawn(6 * 60 * 1000, 0);

        Creature* spike = player->getWorldMap()->createAndSpawnCreature(21319, LocationVector(1315.54f, 6688.33f, -18, 0.001f));
        if (spike != nullptr)
            spike->Despawn(5 * 60 * 1000, 0);
    }
};

class StrengthofOne : public GameObjectAIScript
{
public:
    explicit StrengthofOne(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new StrengthofOne(GO); }

    void OnActivate(Player* player) override
    {
        QuestLogEntry* qle = player->getQuestLogByQuestId(9582);
        if (qle == nullptr)
            return;

        LocationVector pos = player->GetPosition();

        // What is this ? :O To remove ?
        Creature* reaver = player->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 17556);
        if (reaver)
        {
            LocationVector pos2 = reaver->GetPosition();
            reaver->Despawn(1, 5 * 60 * 1000);

            Creature* reaver2 = player->getWorldMap()->getInterface()->spawnCreature(17556, pos2, true, false, 0, 0);
            if (reaver2 != nullptr)
                reaver2->Despawn(5 * 60 * 1000, 0);
        }
    }
};

class HealingTheLake : public GameObjectAIScript
{
public:
    explicit HealingTheLake(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new HealingTheLake(GO); }

    void OnActivate(Player* player) override
    {
        player->addQuestKill(181433, 0, 0);
    }
};

class TheRavensClaw : public GameObjectAIScript
{
public:
    explicit TheRavensClaw(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new TheRavensClaw(GO); }

    void OnActivate(Player* player) override
    {
        if (player->getWorldMap()->getDifficulty() == InstanceDifficulty::DUNGEON_HEROIC)
            player->getWorldMap()->createAndSpawnCreature(23035, LocationVector(-87.3546f, 288.006f, 26.4832f, 0));
    }
};

class DeathGate1 : public GameObjectAIScript
{
public:
    explicit DeathGate1(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new DeathGate1(GO); }

    void OnSpawn() override
    {
        RegisterAIUpdateEvent(500);
    }

    void AIUpdate() override
    {
        LocationVector pos = _gameobject->GetPosition();
        Player* player = _gameobject->getWorldMap()->getInterface()->getPlayerNearestCoords(pos.x, pos.y, pos.z);
        if (player == nullptr)
            return;

        if (_gameobject->CalcDistance(_gameobject, player) <= 1.5f && player->GetMapId() == 609)
            player->castSpell(player, 54699, true);
        else if (_gameobject->CalcDistance(_gameobject, player) <= 1.5f && player->GetMapId() == 0)
            player->castSpell(player, 54744, true);
    }
};

class DeathGate2 : public GameObjectAIScript
{
public:
    explicit DeathGate2(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new DeathGate2(GO); }

    void OnSpawn() override
    {
        RegisterAIUpdateEvent(500);
    }

    void AIUpdate() override
    {
        LocationVector pos = _gameobject->GetPosition();
        Player* player = _gameobject->getWorldMap()->getInterface()->getPlayerNearestCoords(pos.x, pos.y, pos.z);
        if (player == nullptr)
            return;

        if (_gameobject->CalcDistance(_gameobject, player) <= 1.5f && player->GetMapId() == 609)
            player->castSpell(player, 54725, true);
        else if (_gameobject->CalcDistance(_gameobject, player) <= 1.5f && player->GetMapId() == 0)
            player->castSpell(player, 54746, true);
    }
};

class DeathGate3 : public GameObjectAIScript
{
public:
    explicit DeathGate3(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new DeathGate3(GO); }

    void OnActivate(Player* player) override
    {
        if (player->getClass() == 6)
        {
            if (player->GetMapId() == 609)
                player->castSpell(player, 53098, true);
            else
                player->castSpell(player, 53822, true);
        }
    }
};

class SacredFireofLife : public GameObjectAIScript
{
public:
    explicit SacredFireofLife(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new SacredFireofLife(GO); }

    void OnActivate(Player* player) override
    {
        LocationVector pos = player->GetPosition();
        Creature* NewCreature = player->getWorldMap()->getInterface()->spawnCreature(10882, pos, true, false, 0, 0);
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
