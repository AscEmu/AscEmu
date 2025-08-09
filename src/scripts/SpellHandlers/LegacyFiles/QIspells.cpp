/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
#include "Management/ItemInterface.h"
#include "Management/QuestLogEntry.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Movement/MovementManager.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Master.h"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Utilities/Random.hpp"

enum
{
    // ShipBombing
    GO_FIRE = 183816
};

bool CleansingVial(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    if (pPlayer->hasQuestInQuestLog(9427) == false)
        return true;

    Creature* pAggonar = pPlayer->getWorldMap()->createAndSpawnCreature(17000, LocationVector(428.15f, 3461.73f, 63.40f));
    if (pAggonar != nullptr)
        pAggonar->Despawn(6 * 60 * 1000, 0);

    return true;
}

bool SummonCyclonian(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getUnitCaster() == nullptr)
        return true;

    Unit* pUnit = pSpell->getUnitCaster();

    LocationVector unitPos = pUnit->GetPosition();

    Creature* pCreature = pUnit->getWorldMap()->getInterface()->spawnCreature(6239, unitPos, true, false, 0, 0);
    if (pCreature != nullptr)
    {
        pCreature->Despawn(600000, 0);
    }

    return true;
}

bool ElementalPowerExtractor(uint32_t /*i*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();
    Unit* pUnit = pSpell->getUnitTarget();
    if (pUnit == nullptr || pUnit->isCreature() == false)
        return true;

    Creature* pTarget = static_cast<Creature*>(pUnit);
    if ((pTarget->getEntry() == 18881 || pTarget->getEntry() == 18865) && pTarget->isAlive())
    {
        pPlayer->getItemInterface()->AddItemById(28548, 1, 0);
    }

    return true;
}

bool SummonEkkorash(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();

    LocationVector pos = plr->GetPosition();

    plr->getWorldMap()->createAndSpawnCreature(19493, pos);
    return true;
}

bool CallRexxar(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    LocationVector pos = pPlayer->GetPosition();

    if (pPlayer->hasQuestInQuestLog(10742))
    {
        Creature* calax1 = pPlayer->getWorldMap()->createAndSpawnCreature(21984, pos);
        if (calax1 != nullptr)
            calax1->Despawn(2 * 60 * 1000, 0);

        Creature* calax2 = pPlayer->getWorldMap()->createAndSpawnCreature(20555, pos);
        if (calax2 != nullptr)
            calax2->Despawn(2 * 60 * 1000, 0);
    }

    return true;
}

bool LayWreath(uint8_t /*effectIndex*/, Spell* pSpell)  //Peace at Last quest
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    LocationVector pos = pPlayer->GetPosition();

    if (pPlayer->hasQuestInQuestLog(11152))
    {
        pPlayer->addQuestKill(11152, 0, 0);

        GameObject* pWreath = pPlayer->getWorldMap()->createAndSpawnGameObject(501541, pos, 1);
        if (pWreath != nullptr)
            pWreath->despawn(2 * 60 * 1000, 0);
    }

    return true;
}

bool ScrapReaver(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    LocationVector pos = pPlayer->GetPosition();

    Creature* pCreature = pPlayer->getWorldMap()->getInterface()->spawnCreature(19851, pos, true, false, 0, 0);
    if (pCreature != nullptr)
    {
        pCreature->Despawn(600000, 0);
    }

    return true;
}

bool RuuanokClaw(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster())
        return true;

    pSpell->getPlayerCaster()->getWorldMap()->getInterface()->spawnCreature(21767, LocationVector(3210.960693f, 5348.308594f, 144.537476f, 5.450696f), true, false, 0, 0);
    return true;
}

bool KarangsBanner(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    // Banner Aura
    pPlayer->castSpell(pPlayer, sSpellMgr.getSpellInfo(20746), true);

    pSpell->getPlayerCaster()->getWorldMap()->getInterface()->spawnCreature(12921, LocationVector(2231.710205f, -1543.603027f, 90.694946f, 4.700579f), true, false, 0, 0);
    pSpell->getPlayerCaster()->getWorldMap()->getInterface()->spawnCreature(12921, LocationVector(2232.534912f, -1556.983276f, 89.744415f, 1.527570f), true, false, 0, 0);

    pSpell->getPlayerCaster()->getWorldMap()->getInterface()->spawnCreature(12757, LocationVector(2239.357178f, -1546.649536f, 89.671097f, 3.530336f), true, false, 0, 0);

    return true;
}

bool ADireSituation(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() != nullptr)
    {
        pSpell->getPlayerCaster()->addQuestKill(10506, 0);
    }

    return true;
}

bool FuryoftheDreghoodElders(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->hasQuestInQuestLog(10369) == false)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    Creature* arzethpower = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 19354);
    if (arzethpower == nullptr)
        return true;

    LocationVector arzethPos = arzethpower->GetPosition();

    Creature* arzethless = pPlayer->getWorldMap()->createAndSpawnCreature(20680, arzethPos);
    if (arzethless != nullptr)
        arzethless->Despawn(5 * 60 * 1000, 0);

    arzethpower->Despawn(1, 6 * 60 * 1000);

    return true;
}

bool ASpiritAlly(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->hasQuestInQuestLog(9847) == false)
        return true;

    Creature* allyspirit = pPlayer->getWorldMap()->createAndSpawnCreature(18185, LocationVector(-353, 7255, 49.36f, 6.28f));
    if (allyspirit != nullptr)
        allyspirit->Despawn(6 * 60 * 1000, 0);

    return true;
}

bool BalanceMustBePreserved(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    if (apply == false)
        return true;

    if (pAura->getCaster()->isPlayer() == false)
        return true;

    Player* pPlayer = dynamic_cast<Player*>(pAura->getCaster());
    if (pPlayer == nullptr)
        return true;

    if (!pPlayer->hasQuestInQuestLog(9720))
        return true;

    GameObject* lake1 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-756, 5926, 19, 300076);
    GameObject* lake2 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-213, 6302, 21, 300076);
    GameObject* lake3 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(498, 8197, 21, 300076);
    GameObject* lake4 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(566, 6358, 23, 300076);

    if (lake1 != nullptr)
        pPlayer->addQuestKill(9720, 0, 0);

    if (lake2 != nullptr)
        pPlayer->addQuestKill(9720, 3, 0);

    if (lake3 != nullptr)
        pPlayer->addQuestKill(9720, 1, 0);

    if (lake4 != nullptr)
        pPlayer->addQuestKill(9720, 2, 0);

    return true;
}

bool BlessingofIncineratus(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (!pPlayer->hasQuestInQuestLog(9805))
        return true;

    GameObject* big = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-1972, 6276, 56, 300077);
    GameObject* east = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-1841, 6387, 52, 400050);
    GameObject* west = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-1920, 6361, 56, 400051);
    GameObject* south = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-1808, 6293, 59, 400052);

    if (big != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, big) < 30)
            pPlayer->addQuestKill(9805, 0, 0);
    }

    if (east != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, east) < 30)
            pPlayer->addQuestKill(9805, 1, 0);
    }

    if (south != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, south) < 30)
            pPlayer->addQuestKill(9805, 2, 0);
    }

    if (west != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, west) < 30)
            pPlayer->addQuestKill(9805, 3, 0);
    }

    return true;
}

bool TagMurloc(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    Object* Caster = pAura->getCaster();
    if (Caster->isPlayer() == false)
        return false;

    if (pAura->getOwner()->isCreature() == false)
        return false;

    if (apply == false)
        return true;

    Player* pPlayer = dynamic_cast<Player*>(Caster);

    if (!pPlayer->hasQuestInQuestLog(9629))
        return true;

    Creature* murloc = dynamic_cast<Creature*>(pAura->getOwner());
    if (murloc == nullptr)
        return true;

    const LocationVector murlocPos = pPlayer->GetPosition();

    Creature* tagged = pPlayer->getWorldMap()->createAndSpawnCreature(17654, murlocPos);
    if (tagged != nullptr)
        tagged->Despawn(5 * 60 * 1000, 0);

    murloc->Despawn(1, 6 * 60 * 1000);

    pPlayer->addQuestKill(9629, 0, 0);

    return true;
}

bool CookingPot(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (!pPlayer->hasQuestInQuestLog(11379))
        return true;

    pPlayer->getItemInterface()->RemoveItemAmt(31673, 1);
    pPlayer->getItemInterface()->RemoveItemAmt(31672, 2);
    pPlayer->getItemInterface()->AddItemById(33848, 1, 0);

    return true;
}

bool EvilDrawsNear(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->hasQuestInQuestLog(10923) == false)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    Creature* dragon = pPlayer->getWorldMap()->createAndSpawnCreature(22441, LocationVector(pos.x + 15, pos.y + 15, pos.z, pos.o));
    dragon->Despawn(6 * 60 * 1000, 0);

    return true;
}

bool UnyieldingBattleHorn(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Creature* creat = pSpell->getPlayerCaster()->getWorldMap()->getInterface()->spawnCreature(19862, LocationVector(-1190.856079f, 2261.246582f, 46.625797f, 1.705882f), true, false, 0, 0);
    creat->Despawn(300000, 0); // 5 mins delay

    return true;
}

bool MeasuringWarpEnergies(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (!pPlayer->hasQuestInQuestLog(10313))
        return true;

    GameObject* north = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(3216, 4045, 145, 300094);
    GameObject* east = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(2766, 3865, 145, 300094);
    GameObject* west = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(2958, 4318, 145, 300094);
    GameObject* south = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(2817, 4337, 145, 300094);

    if (north != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, north) < 30)
            pPlayer->addQuestKill(10313, 0, 0);
    }

    if (east != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, east) < 30)
            pPlayer->addQuestKill(10313, 1, 0);
    }

    if (south != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, south) < 30)
            pPlayer->addQuestKill(10313, 2, 0);
    }

    if (west != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, west) < 30)
            pPlayer->addQuestKill(10313, 3, 0);
    }

    return true;
}

bool YennikuRelease(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* qle = pPlayer->getQuestLogByQuestId(592);
    if (qle == nullptr)
        return true;

    Creature* yenniku = static_cast<Creature*>(pSpell->getUnitTarget());
    if (yenniku == nullptr)
        return true;

    yenniku->setFaction(29);
    yenniku->getThreatManager().clearAllThreat();
    yenniku->getThreatManager().removeMeFromThreatLists();
    yenniku->Despawn(30 * 1000, 60 * 1000);

    return true;
}

bool ScrollOfMyzrael(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (!pPlayer->hasQuestInQuestLog(656))
        return true;

    const float MyzraelPos[] = { -940.7374f, -3111.1953f, 48.9566f, 3.327f };

    Creature* myzrael = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(MyzraelPos[0], MyzraelPos[1], MyzraelPos[2], 2755);
    if (myzrael != nullptr)
    {
        if (!myzrael->isAlive())
            myzrael->Delete();
        else
            return true;
    }

    myzrael = pPlayer->getWorldMap()->createAndSpawnCreature(2755, LocationVector(MyzraelPos[0], MyzraelPos[1], MyzraelPos[2], MyzraelPos[3]));
    return true;
}

bool Showdown(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* p_caster = pSpell->getPlayerCaster();
    if (p_caster == nullptr)
    {
        return true;
    }

    if (p_caster->hasQuestInQuestLog(10742) && p_caster->hasQuestInQuestLog(10806) == false)
    {
        return true;
    }

    Creature* goc = p_caster->getWorldMap()->createAndSpawnCreature(20555, LocationVector(3739, 5365, -4, 3.5));
    goc->Despawn(6 * 60 * 1000, 0);

    return true;
}

bool TheBaitforLarkorwi1(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (!pPlayer->hasQuestInQuestLog(4292))
        return true;

    const LocationVector pos = pPlayer->GetPosition();

    GameObject* obj = pPlayer->getWorldMap()->createAndSpawnGameObject(169216, pos, 1);
    if (obj != nullptr)
        obj->despawn(1 * 60 * 1000, 0);

    return true;
}

bool TheBaitforLarkorwi2(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->hasQuestInQuestLog(4292) == false)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    Creature* larkowi = pPlayer->getWorldMap()->createAndSpawnCreature(9684, LocationVector(pos.x + 2, pos.y + 3, pos.z, pos.o));
    larkowi->Despawn(5 * 60 * 1000, 0);

    return true;
}

bool Fumping(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->hasQuestInQuestLog(10929) == false)
        return true;

    uint32_t chance = Util::getRandomUInt(1);
    uint32_t entry = 0;
    switch (chance)
    {
        case 0:
            entry = 22482;
            break;
        case 1:
            entry = 22483;
            break;
    }

    LocationVector pos = pPlayer->GetPosition();

    Creature* creat = pPlayer->getWorldMap()->createAndSpawnCreature(entry, pos);
    if (entry == 22483) //Sand Gnomes ;)
    {
        creat->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "YIEEEEEEEAA!");
    }
    creat->Despawn(5 * 60 * 1000, 0);

    return true;
}

bool TheBigBoneWorm(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->hasQuestInQuestLog(10930) == false)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    Creature* exarch = pPlayer->getWorldMap()->createAndSpawnCreature(22038, LocationVector(pos.x + 7, pos.y + 7, pos.z, pos.o));
    exarch->Despawn(6 * 60 * 1000, 0);

    return true;
}

bool Torgos(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->hasQuestInQuestLog(10035) == false)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    Creature* torgos = pPlayer->getWorldMap()->createAndSpawnCreature(18707, LocationVector(pos.x, pos.y - 10, pos.z, pos.o));
    if (torgos == nullptr)
        return true;

    torgos->Despawn(6 * 60 * 1000, 0);
    return true;
}

bool WelcomingtheWolfSpirit(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    Creature* spiritwolf = pPlayer->getWorldMap()->createAndSpawnCreature(19616, LocationVector(pos.x + 2, pos.y + 3, pos.z, pos.o));
    spiritwolf->Despawn(5 * 60 * 1000, 0);

    pPlayer->addQuestKill(10791, 0, 0);

    return true;
}

bool NaturalRemedies(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    if (pPlayer->hasQuestInQuestLog(10351) == false)
        return true;

    Creature* colos = pPlayer->getWorldMap()->createAndSpawnCreature(19305, pos);
    if (colos != nullptr)
        colos->Despawn(5 * 60 * 1000, 0);

    return true;
}

bool FloraoftheEcoDomes(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr || pSpell->getUnitTarget() == nullptr || pSpell->getUnitTarget()->isCreature() == false)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    Creature* normal = static_cast<Creature*>(pSpell->getUnitTarget());

    LocationVector normPos = normal->GetPosition();

    Creature* mutant = pPlayer->getWorldMap()->createAndSpawnCreature(20983, normPos);

    normal->Despawn(1, 6 * 60 * 1000);
    mutant->Despawn(5 * 60 * 1000, 0);

    mutant->getAIInterface()->Init(mutant);
    mutant->getThreatManager().tauntUpdate();

    pPlayer->addQuestKill(10426, 0, 0);

    return true;
}

bool TheCleansingMustBeStopped(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    if (pPlayer->hasQuestInQuestLog(9370) == false)
        return true;

    Creature* draenei1 = pPlayer->getWorldMap()->createAndSpawnCreature(16994, LocationVector(pos.x + Util::getRandomFloat(5.0f), pos.y + Util::getRandomFloat(5.0f), pos.z, pos.o));
    draenei1->Despawn(6 * 60 * 1000, 0);

    Creature* draenei2 = pPlayer->getWorldMap()->createAndSpawnCreature(16994, LocationVector(pos.x - Util::getRandomFloat(5.0f), pos.y + Util::getRandomFloat(5.0f), pos.z, pos.o));
    draenei2->Despawn(6 * 60 * 1000, 0);

    Creature* draenei3 = pPlayer->getWorldMap()->createAndSpawnCreature(16994, LocationVector(pos.x + Util::getRandomFloat(5.0f), pos.y - Util::getRandomFloat(5.0f), pos.z, pos.o));
    draenei3->Despawn(6 * 60 * 1000, 0);

    return true;
}

bool AdministreringtheSalve(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    Object* m_caster = pAura->getCaster();
    if (m_caster->isPlayer() == false)
        return true;

    if (pAura->getOwner()->isCreature() == false)
        return true;

    if (apply)
    {
        Player* pPlayer = dynamic_cast<Player*>(m_caster);

        if (!pPlayer->hasQuestInQuestLog(9447))
            return true;

        Creature* sick = dynamic_cast<Creature*>(pAura->getOwner());
        if (sick == nullptr)
            return true;

        const LocationVector sickPos = sick->GetPosition();

        Creature* healed = pPlayer->getWorldMap()->createAndSpawnCreature(16846, sickPos);
        sick->Despawn(1, 6 * 60 * 1000);
        healed->Despawn(3 * 60 * 1000, 0);

        pPlayer->addQuestKill(9447, 0, 0);
    }

    return true;
}

bool ZappedGiants(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->hasQuestInQuestLog(7003) == false && pPlayer->hasQuestInQuestLog(7725) == false)
        return true;

    Creature* creat = static_cast<Creature*>(pSpell->getUnitTarget());
    if (creat == nullptr)
        return true;

    LocationVector creatPos = creat->GetPosition();

    uint32_t cit = creat->getEntry();
    switch (cit)
    {
        case 5360:
        {
            creat->Despawn(1000, 6 * 60 * 1000);
            Creature* zappedcreat = pPlayer->getWorldMap()->createAndSpawnCreature(14639, creatPos);
            zappedcreat->Despawn(3 * 60 * 1000, 0);
        }
        break;
        case 5361:
        {
            creat->Despawn(1000, 6 * 60 * 1000);
            Creature* zappedcreat = pPlayer->getWorldMap()->createAndSpawnCreature(14638, creatPos);
            zappedcreat->Despawn(3 * 60 * 1000, 0);
        }
        break;
        case 5359:
        {
            creat->Despawn(1000, 6 * 60 * 1000);
            Creature* zappedcreat = pPlayer->getWorldMap()->createAndSpawnCreature(14603, creatPos);
            zappedcreat->Despawn(3 * 60 * 1000, 0);
        }
        break;
        case 5358:
        {
            creat->Despawn(1000, 6 * 60 * 1000);
            Creature* zappedcreat = pPlayer->getWorldMap()->createAndSpawnCreature(14640, creatPos);
            zappedcreat->Despawn(3 * 60 * 1000, 0);
        }
        break;
        case 5357:
        {
            creat->Despawn(1000, 6 * 60 * 1000);
            Creature* zappedcreat = pPlayer->getWorldMap()->createAndSpawnCreature(14604, creatPos);
            zappedcreat->Despawn(3 * 60 * 1000, 0);
        }
        break;
    }
    return true;
}

bool BuildingAPerimeter(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    GameObject* pEast = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(2257.0f, 2465.0f, 101.0f, 183947);
    if (pEast != nullptr && pPlayer->CalcDistance(pPlayer, pEast) < 30)
    {
        pPlayer->addQuestKill(10313, 0, 0);
        return true;
    }

    GameObject* pNorth = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(2375.0f, 2285.0f, 141.0f, 183947);
    if (pNorth != nullptr && pPlayer->CalcDistance(pPlayer, pNorth) < 30)
    {
        pPlayer->addQuestKill(10313, 1, 0);
        return true;
    }

    GameObject* pWest = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(2283.0f, 2181.0f, 95.0f, 183947);
    if (pWest != nullptr && pPlayer->CalcDistance(pPlayer, pWest) < 30)
    {
        pPlayer->addQuestKill(10313, 2, 0);
        return true;
    }

    return true;
}

bool RodofPurification(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (auto* questLog = pPlayer->getQuestLogByQuestId(10839))
    {
        GameObject* Darkstone = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-2512, 5418, 0, 185191);
        if (Darkstone != nullptr)
        {
            if (pPlayer->CalcDistance(pPlayer, Darkstone) < 15)
                questLog->sendQuestComplete();
        }
    }

    return true;
}

bool AnUnusualPatron(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->hasQuestInQuestLog(9457) == false)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    Creature* Naias = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 17207);
    if (Naias != nullptr)
        return true;

    Naias = pPlayer->getWorldMap()->createAndSpawnCreature(17207, pos);
    Naias->Despawn(10 * 60 * 1000, 0);
    return true;
}

bool MagnetoCollector(uint8_t /*effectIndex*/, Aura* pAura, bool /*apply*/)
{
    if (pAura->getCaster()->isPlayer() == false)
        return true;

    Player* pPlayer = static_cast<Player*>(pAura->getCaster());
    if (pPlayer->hasQuestInQuestLog(10584) == false)
        return true;

    Creature* magneto = static_cast<Creature*>(pAura->getOwner());
    if (magneto == nullptr)
        return true;

    LocationVector magnetoPos = magneto->GetPosition();

    Creature* auramagneto = pPlayer->getWorldMap()->createAndSpawnCreature(21731, magnetoPos);
    if (auramagneto != nullptr)
        auramagneto->Despawn(4 * 60 * 1000, 0);

    magneto->Despawn(1, 0);

    return true;
}

bool TemporalPhaseModulator(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    if (pPlayer->hasQuestInQuestLog(10609) == false)
        return true;

    Creature* whelp = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 20021);
    if (whelp != nullptr)
    {
        LocationVector whelpPos = whelp->GetPosition();
        if (Util::getRandomUInt(1) == 0)
        {
            Creature* adolescent = pPlayer->getWorldMap()->createAndSpawnCreature(21817, whelpPos);
            adolescent->Despawn(5 * 60 * 1000, 0);
        }
        else
        {
            Creature* proto = pPlayer->getWorldMap()->createAndSpawnCreature(21821, whelpPos);
            proto->Despawn(5 * 60 * 1000, 0);
        }
        whelp->Despawn(1, 0);
        return true;
    }

    whelp = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 21817);
    if (whelp != nullptr)
    {
        LocationVector whelpPos = whelp->GetPosition();
        if (Util::getRandomUInt(10) < 8)
        {
            Creature* mature = pPlayer->getWorldMap()->createAndSpawnCreature(21820, whelpPos);
            mature->Despawn(5 * 60 * 1000, 0);
        }
        else
        {
            Creature* funnyd = pPlayer->getWorldMap()->createAndSpawnCreature(21823, whelpPos);
            funnyd->Despawn(5 * 60 * 1000, 0);
        }
        whelp->Despawn(1, 0);
        return true;
    }

    whelp = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 21821);
    if (whelp != nullptr)
    {
        LocationVector whelpPos = whelp->GetPosition();
        if (Util::getRandomUInt(10) < 8)
        {
            Creature* mature = pPlayer->getWorldMap()->createAndSpawnCreature(21820, whelpPos);
            mature->Despawn(5 * 60 * 1000, 0);
        }
        else
        {
            Creature* funnyd = pPlayer->getWorldMap()->createAndSpawnCreature(21823, whelpPos);
            funnyd->Despawn(5 * 60 * 1000, 0);
        }
        whelp->Despawn(1, 0);
        return true;
    }
    whelp = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 21823);
    if (whelp != nullptr)
    {
        LocationVector whelpPos = whelp->GetPosition();
        if (Util::getRandomUInt(1) == 0)
        {
            Creature* adolescent = pPlayer->getWorldMap()->createAndSpawnCreature(21817, whelpPos);
            adolescent->Despawn(5 * 60 * 1000, 0);
        }
        else
        {
            Creature* proto = pPlayer->getWorldMap()->createAndSpawnCreature(21821, whelpPos);
            proto->Despawn(5 * 60 * 1000, 0);
        }
        whelp->Despawn(1, 0);
        return true;
    }

    return true;
}

bool EmblazonRuneblade(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();
    pPlayer->sendChatMessageToPlayer(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, "Player check", pPlayer);

    if (!pPlayer->hasQuestInQuestLog(12619))
        return true;

    pPlayer->getItemInterface()->AddItemById(38631, 1, 0);
    pPlayer->getItemInterface()->RemoveItemAmt(38607, 1);

    return true;
}

bool WyrmcallersHorn(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();

    LocationVector pos = plr->GetPosition();

    Creature* pCreature = plr->getWorldMap()->createAndSpawnCreature(24019, pos);
    if (pCreature == nullptr)
    {
        return true;
    }

    pCreature->Despawn(5 * 60 * 1000, 0);
    return true;
}

bool RaeloraszSpark(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();
    if (plr == nullptr)
        return true;

    const LocationVector pos = plr->GetPosition();

    Creature* pCreature = plr->getWorldMap()->createAndSpawnCreature(26237, pos);
    pCreature->Despawn(5 * 60 * 1000, 0);

    if (auto* questLog = plr->getQuestLogByQuestId(11969))
        questLog->sendQuestComplete();

    return true;
}

bool RuneOfDistortion(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* plr = pSpell->getPlayerCaster();
    if (plr == nullptr)
        return true;

    LocationVector pos = plr->GetPosition();

    Creature* pCreature = plr->getWorldMap()->createAndSpawnCreature(32162, pos);
    pCreature->Despawn(5 * 60 * 1000, 0);

    if (plr->hasQuestInQuestLog(13312) == false && plr->hasQuestInQuestLog(13337) == false)
        return true;

    return true;
}

bool GoreBladder(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
    {
        return true;
    }

    Unit* target = pSpell->getUnitTarget();
    if (target == nullptr || target->getEntry() != 29392 || target->isDead() == false)
        return true;

    static_cast<Creature*>(target)->Despawn(500, 360000);

    Player* pPlayer = pSpell->getPlayerCaster();
    pPlayer->addQuestKill(12810, 0, 0);

    return true;
}

bool PlagueSpray(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Unit* target = pSpell->getUnitTarget();
    if (!target || target->getEntry() != 23652 || !target->isAlive())
        return true;
    else if (!target || target->getEntry() != 23652 || !target->hasAurasWithId(40467))
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    pPlayer->addQuestKill(11307, 0, 0);

    return true;
}

bool GoblinWeatherMachine(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    uint32_t Weather = 46736 + Util::getRandomUInt(4);

    pSpell->getPlayerCaster()->castSpell(pSpell->getPlayerCaster(), sSpellMgr.getSpellInfo(Weather), true);
    return true;
}

bool PurifiedAshes(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Unit* target = pSpell->getUnitTarget();
    if (!target || target->getEntry() != 26633 || !target->isDead())
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();
    int entry;

    if (pPlayer->isTeamHorde())
        entry = 12236;
    else
        entry = 12249;

    pPlayer->addQuestKill(entry, 0, 0);

    return true;
}

bool DISMEMBER(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster())
        return true;

    Unit* target = pSpell->getUnitTarget();
    if (!target || (target->getEntry() != 23657 && target->getEntry() != 23661 && target->getEntry() != 23662 && target->getEntry() != 23663 && target->getEntry() != 23664 && target->getEntry() != 23665 && target->getEntry() != 23666 && target->getEntry() != 23667 && target->getEntry() != 23668 && target->getEntry() != 23669 && target->getEntry() != 23670) || !target->isDead())
        return true;

    static_cast<Creature*>(target)->Despawn(500, 300000);

    Player* pPlayer = pSpell->getPlayerCaster();
    int entry;

    if (pPlayer->isTeamHorde())
    {
        entry = 11257;
    }
    else
    {
        entry = 11246;
    }

    pPlayer->addQuestKill(entry, 0, 0);

    return true;
}

bool CraftyBlaster(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
    {
        return true;
    }

    Unit* target = pSpell->getUnitTarget();
    if (!target || (target->getEntry() != 25432 && target->getEntry() != 25434) || !target->isAlive())
    {
        return true;
    }

    Player* pPlayer = pSpell->getPlayerCaster();

    pPlayer->addQuestKill(11653, 0, 0);

    return true;
}

bool RagefistTorch(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
    {
        return true;
    }

    Unit* target = pSpell->getUnitTarget();
    if (!target || (target->getEntry() != 25342 && target->getEntry() != 25343))
    {
        return true;
    }

    static_cast<Creature*>(target)->Despawn(500, 360000);

    Player* pPlayer = pSpell->getPlayerCaster();

    pPlayer->addQuestKill(11593, 0, 0);

    return true;
}

bool SummonShadra(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    LocationVector pos = pSpell->getPlayerCaster()->GetPosition();
    pSpell->getPlayerCaster()->getWorldMap()->getInterface()->spawnCreature(2707, pos, true, false, 0, 0);

    return true;
}

bool SummonEcheyakee(uint8_t effectIndex, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr || effectIndex != 1)  //Effect_1 = SEND_EVENT,Effect_2 = DUMMY
        return true;

    LocationVector pos = pSpell->getPlayerCaster()->GetPosition();
    pSpell->getPlayerCaster()->getWorldMap()->getInterface()->spawnCreature(3475, pos, true, false, 0, 0);

    return true;
}

bool HodirsHorn(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Unit* target = pSpell->getUnitTarget();
    if (!target || (target->getEntry() != 29974 && target->getEntry() != 30144 && target->getEntry() != 30135) || !target->isDead())
        return true;

    static_cast<Creature*>(target)->Despawn(500, 360000);

    Player* pPlayer = pSpell->getPlayerCaster();

    pPlayer->addQuestKill(12977, 0, 0);

    return true;
}

bool TelluricPoultice(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
    {
        return true;
    }

    Unit* target = pSpell->getUnitTarget();
    if (!target || target->getEntry() != 30035)
    {
        return true;
    }

    static_cast<Creature*>(target)->Despawn(500, 360000);

    Player* pPlayer = pSpell->getPlayerCaster();

    pPlayer->addQuestKill(12937, 0, 0);

    return true;
}

bool Screwdriver(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
    {
        return true;
    }

    Unit* target = pSpell->getUnitTarget();
    if (!target || target->getEntry() != 25753 || !target->isDead())
    {
        return true;
    }

    static_cast<Creature*>(target)->Despawn(500, 360000);

    Player* pPlayer = pSpell->getPlayerCaster();

    pPlayer->addQuestKill(11730, 0, 0);

    return true;
}

bool IncineratingOil(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
    {
        return true;
    }

    Unit* target = pSpell->getUnitTarget();
    if (!target || target->getEntry() != 28156)
    {
        return true;
    }

    static_cast<Creature*>(target)->Despawn(500, 360000);

    Player* pPlayer = pSpell->getPlayerCaster();

    pPlayer->addQuestKill(12568, 0, 0);

    return true;
}

bool SummonAquementas(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    LocationVector pos = pSpell->getPlayerCaster()->GetPosition();

    pSpell->getPlayerCaster()->getWorldMap()->getInterface()->spawnCreature(9453, pos, true, false, 0, 0);

    return true;
}

bool PrayerBeads(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Unit* target = pSpell->getUnitTarget();
    if (!target || target->getEntry() != 22431)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    pPlayer->addQuestKill(10935, 0, 0);

    return true;
}

bool CleansingVialDND(uint32_t /*i*/, Spell* s)
{
    if (auto* questLog = s->getPlayerCaster()->getQuestLogByQuestId(9427))
        questLog->sendQuestComplete();

    return true;
}

bool HunterTamingQuest(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* m_target = a->getOwner();
    Player* p_caster = a->GetPlayerCaster();

    if (p_caster == nullptr)
        return true;

    if (apply)
    {
        m_target->getAIInterface()->onHostileAction(a->GetUnitCaster());
        m_target->getThreatManager().addThreat(a->GetUnitCaster(), 10.f);
    }
    else
    {
        uint32_t TamingSpellid = a->getSpellInfo()->getEffectMiscValue(1);

        SpellInfo const* triggerspell = sSpellMgr.getSpellInfo(TamingSpellid);
        if (triggerspell == NULL)
        {
            DLLLogDetail("An Aura with spellid %u is calling HunterTamingQuest() with an invalid TamingSpellid: %u", a->getSpellId(), TamingSpellid);
            return true;
        }

        QuestProperties const* tamequest = sMySQLStore.getQuestProperties(triggerspell->getEffectMiscValue(1));
        if (tamequest == NULL)
        {
            DLLLogDetail("An Aura with spellid %u is calling HunterTamingQuest() with an invalid tamequest id: %u", a->getSpellId(), triggerspell->getEffectMiscValue(1));
            return true;
        }

        if (!p_caster->hasQuestInQuestLog(tamequest->id))
        {
            p_caster->sendCastFailedPacket(triggerspell->getId(), SPELL_FAILED_BAD_TARGETS, 0, 0);
        }
        else if (!a->getTimeLeft())
        {
            // Creates a 15 minute pet, if player has the quest that goes with the spell and if target corresponds to quest
            //\todo you can't do that here. SpellHandler load will fail on *nix systemy
            /*if (Rand(75.0f))    // 75% chance on success
            {

                if (m_target->isCreature())
                {
                    Creature* tamed = static_cast<Creature*>(m_target);
                    tamed->getAIInterface()->HandleEvent(EVENT_LEAVECOMBAT, getPlayerCaster(), 0);

                    Pet* pPet = sObjectMgr.CreatePet(tamed->getEntry());
                    if (!pPet->CreateAsSummon(tamed->getEntry(), tamed->GetCreatureProperties(), tamed, getPlayerCaster(), triggerspell, 2, 900000))
                    {
                        pPet->DeleteMe();//CreateAsSummon() returns false if an error occurred.
                        pPet = NULL;
                    }

                    tamed->Despawn(1, 0); //we despawn the tamed creature once we are out of Aura::Remove()

                    QuestLogEntry* qle = getPlayerCaster()->getQuestLogByQuestId(tamequest->id);
                    if (qle != nullptr)
                    {
                        qle->SetMobCount(0, 1);
                        qle->SendUpdateAddKill(1);
                        qle->UpdatePlayerFields();
                        qle->SendQuestComplete();
                    }
                }
            }
            else
            {
                getPlayerCaster()->sendCastFailedPacket(triggerspell->getId(), SPELL_FAILED_TRY_AGAIN, 0, 0);
            }*/
        }
    }

    return true;
}

bool ArcaneDisruption(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    if (apply == false)
        return true;

    if (pAura->GetPlayerCaster() == nullptr)
        return true;

    Player* plr = pAura->GetPlayerCaster();
    if (plr == nullptr)
        return true;

    if (auto* questLog = plr->getQuestLogByQuestId(13149))
    {
        GameObject* crate = plr->getWorldMap()->getInterface()->getGameObjectNearestCoords(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), 190094);
        if (crate != nullptr)
        {
            GameObject* go = plr->getWorldMap()->createGameObject(190095);
            go->create(190095, crate->getWorldMap(), 0, plr->GetPosition(), QuaternionData(), GO_STATE_CLOSED);
            go->PushToWorld(crate->getWorldMap());
            crate->despawn(0, 0);

            questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
            questLog->sendUpdateAddKill(0);
            questLog->updatePlayerFields();

            if (questLog->getMobCountByIndex(0) == 5)
            {
                //weee, Uther
                CreatureProperties const* cp = sMySQLStore.getCreatureProperties(26528);
                if (cp != nullptr)
                {
                    Creature* c = plr->getWorldMap()->createCreature(26528);
                    if (c != nullptr)
                    {
                        //position is guessed
                        c->Load(cp, 1759.4351f, 1265.3317f, 138.052f, 0.1902f);
                        c->PushToWorld(plr->getWorldMap());
                    }
                }
            }
        }
    }
    return true;
}

bool ToLegionHold(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    if (pAura == nullptr)
        return true;

    Player* pPlayer = pAura->GetPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    Creature* pJovaanCheck = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(-3310.743896f, 2951.929199f, 171.132538f, 21633);
    if (pJovaanCheck != nullptr)
        return true;

    if (apply)
    {

        pPlayer->setMoveRoot(true);
        Creature* pJovaan = pPlayer->getWorldMap()->createAndSpawnCreature(21633, LocationVector(-3310.743896f, 2951.929199f, 171.132538f, 5.054039f));    // Spawn Jovaan
        if (pJovaan != nullptr)
        {
            pJovaan->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
            if (pJovaan->getAIInterface() != nullptr)
            {
                pJovaan->getAIInterface()->setAllowedToEnterCombat(false);
            }
        }
        GameObject* pGameObject = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 184834);
        if (pGameObject != nullptr)
        {
            pGameObject->despawn(60000, 0);
            pPlayer->updateNearbyQuestGameObjects();
        }
    }
    else
    {
        if (pPlayer->isTeamAlliance())
            pPlayer->addQuestKill(10563, 2, 0);
        else
            pPlayer->addQuestKill(10596, 2, 0);

        pPlayer->setMoveRoot(false);
    }

    return true;
}

bool CenarionMoondust(uint8_t /*effectIndex*/, Spell* pSpell) // Body And Heart (Alliance)
{
    if (!pSpell->getPlayerCaster())
        return true;

    if (!pSpell->getPlayerCaster()->IsInWorld())
        return true;

    const float pos[] = { 6335.2329f, 144.0811f, 24.0068f, 5.701f }; // x, y, z, o
    Player* p_caster = pSpell->getPlayerCaster();

    //Moonkin Stone aura
    p_caster->getWorldMap()->createAndSpawnGameObject(177644, LocationVector(6331.01f, 88.245f, 22.6522f, 2.01455f), 1.0);

    // it dont delete lunaclaw if he is here
    Creature* lunaclaw = p_caster->getWorldMap()->createAndSpawnCreature(12138, LocationVector(pos[0], pos[1], pos[2], pos[3]));

    if (lunaclaw == nullptr)
        return true;

    //Waypoints
    lunaclaw->setMoveWalk(false);
    lunaclaw->getMovementManager()->movePoint(0, 6348.3833f, 132.5197f, 21.6042f, false, 4.19f);
    //make sure that player dont cheat speed or something
    if (lunaclaw->GetDistance2dSq(p_caster) < 200)   // can be more? - he can speed hack or teleport hack
    {
        LocationVector casterPos = p_caster->GetPosition();
        lunaclaw->setMoveWalk(false);
        lunaclaw->getMovementManager()->movePoint(1, casterPos.x, casterPos.y, casterPos.z, false, casterPos.o + 3);
    }
    else
    {
        lunaclaw->setMoveWalk(false);
        lunaclaw->getMovementManager()->movePoint(2, 5328.2148f, 94.5505f, 21.4547f, false, 4.2489f);
    }

    // Make sure that creature will attack player
    if (!lunaclaw->getCombatHandler().isInCombat())
    {
        lunaclaw->getAIInterface()->setCurrentTarget(p_caster);
    }

    return true;
}

bool CenarionLunardust(uint8_t /*effectIndex*/, Spell* pSpell)  // Body And Heart (Horde)
{
    if (!pSpell->getPlayerCaster())
        return true;

    if (!pSpell->getPlayerCaster()->IsInWorld())
        return true;

    const float pos[] = { -2443.9711f, -1642.8002f, 92.5129f, 1.71f }; // x, y, z, o
    Player* p_caster = pSpell->getPlayerCaster();

    //Moonkin Stone aura
    p_caster->getWorldMap()->createAndSpawnGameObject(177644, LocationVector(-2499.54f, -1633.03f, 91.8121f, 0.262894f), 1.0);

    Creature* lunaclaw = p_caster->getWorldMap()->createAndSpawnCreature(12138, LocationVector(pos[0], pos[1], pos[2], pos[3]));
    if (lunaclaw == nullptr)
        return true;

    // Waypoints
    lunaclaw->setMoveWalk(false);
    lunaclaw->getMovementManager()->movePoint(0, -2448.2253f, -1625.0148f, 91.89f, false, 1.913f);
    //make sure that player dont cheat speed or something
    if (lunaclaw->GetDistance2dSq(p_caster) < 200)   // can be more? - he can speed hack or teleport hack
    {
        LocationVector targetPos = p_caster->GetPosition();
        lunaclaw->setMoveWalk(false);
        lunaclaw->getMovementManager()->movePoint(1, targetPos.x, targetPos.y, targetPos.z, false, targetPos.o + 3);
    }
    else
    {
        lunaclaw->setMoveWalk(false);
        lunaclaw->getMovementManager()->movePoint(2, -2504.2641f, -1630.7354f, 91.93f, false, 3.2f);
    }

    // Make sure that creature will attack player
    if (!lunaclaw->getCombatHandler().isInCombat())
    {
        lunaclaw->getAIInterface()->setCurrentTarget(p_caster);
    }

    return true;
}

bool CurativeAnimalSalve(uint8_t /*effectIndex*/, Spell* pSpell) // Curing the Sick
{
    Player* caster = pSpell->getPlayerCaster();
    if (caster == NULL)
        return true;

    if (!pSpell->getUnitTarget()->isCreature())
        return true;

    Creature* target = static_cast<Creature*>(pSpell->getUnitTarget());

    LocationVector targetPos = target->GetPosition();

    uint32_t entry = target->getEntry();
    if (entry == 12296 || entry == 12298)
    {
        caster->addQuestKill(6129, 0, 0);
        caster->addQuestKill(6124, 0, 0);

        if (entry == 12298) // Sickly Deer
        {
            Creature* deer = caster->getWorldMap()->createAndSpawnCreature(12298, targetPos); // Cured Deer
            if (deer != nullptr)
                deer->Despawn(2 * 60 * 1000, 0);
        }
        else // Sickly Gazelle
        {
            Creature* gazelle = caster->getWorldMap()->createAndSpawnCreature(12297, targetPos); // Cured Gazelle
            if (gazelle != nullptr)
                gazelle->Despawn(2 * 60 * 1000, 0);
        }

        target->Despawn(0, 3 * 60 * 1000);

        return true;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Trial Of The Lake
bool TrialOfTheLake(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == NULL)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    pPlayer->addQuestKill(28, 0, 0);
    pPlayer->addQuestKill(29, 0, 0);

    return true;
}

bool SymbolOfLife(uint8_t /*effectIndex*/, Spell* pSpell) // Alliance ress. quests
{
    Player* plr = pSpell->getPlayerCaster();
    if (plr == nullptr)
        return true;

    WoWGuid wowGuid;
    wowGuid.Init(plr->getTargetGuid());

    Creature* target = plr->getWorldMap()->getCreature(wowGuid.getGuidLowPart());

    if (target == nullptr)
        return true;

    const uint32_t targets[] = { 17542, 6177, 6172 };
    const uint32_t quests[] = { 9600, 1783, 1786 };

    bool questOk = false;
    bool targetOk = false;

    for (uint8_t j = 0; j < 3; j++)
    {
        if (target->getEntry() == targets[j])
        {
            targetOk = true;
            break;
        }
    }

    if (!targetOk)
        return true;

    QuestLogEntry* questLog = nullptr;

    for (uint8_t j = 0; j < 3; j++)
    {
        if (plr->hasQuestInQuestLog(quests[j]))
        {
            questLog = plr->getQuestLogByQuestId(quests[j]);
            if (questLog != nullptr)
                questOk = true;

            break;
        }
    }

    if (!questOk)
        return true;

    target->setStandState(STANDSTATE_STAND);
    target->setDeathState(ALIVE);

    target->Despawn(10 * 1000, 1 * 60 * 1000);

    questLog->setMobCountForIndex(0, 1);
    questLog->sendUpdateAddKill(0);
    questLog->updatePlayerFields();

    return true;
}

bool FilledShimmeringVessel(uint8_t /*effectIndex*/, Spell* pSpell) // Blood Elf ress. quest
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();

    WoWGuid wowGuid;
    wowGuid.Init(plr->getTargetGuid());

    Creature* target = plr->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
    if (target == nullptr)
        return true;

    if (target->getEntry() != 17768)
        return true;

    if (auto* questLog = plr->getQuestLogByQuestId(9685))
    {
        target->setStandState(STANDSTATE_STAND);
        target->setDeathState(ALIVE);
        target->Despawn(30 * 1000, 1 * 60 * 1000);

        questLog->setMobCountForIndex(0, 1);
        questLog->sendUpdateAddKill(0);
        questLog->updatePlayerFields();
    }

    return true;
}

bool DouseEternalFlame(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();

    if (!plr->hasQuestInQuestLog(9737))
        return true;

    const LocationVector pos = plr->GetPosition();

    GameObject* Flame = plr->getWorldMap()->getInterface()->getGameObjectNearestCoords(3678, -3640, 139, 182068);
    if (Flame != nullptr)
    {
        if (plr->CalcDistance(plr, Flame) < 30)
        {
            plr->addQuestKill(9737, 0, 0);

            Creature* pCreature = plr->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 10917);
            if (pCreature != nullptr)
                pCreature->setFaction(11);
        }
    }
    return true;
}

bool Triage(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster() || pSpell->getUnitTarget() == nullptr)
        return true;

    pSpell->getPlayerCaster()->castSpell(pSpell->getUnitTarget(), sSpellMgr.getSpellInfo(746), true);

    pSpell->getPlayerCaster()->addQuestKill(6624, 0, 0);

    return true;
}

bool NeutralizingTheCauldrons(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr || !pSpell->getPlayerCaster()->IsInWorld())
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    if (!pPlayer->hasQuestInQuestLog(11647))
        return true;

    GameObject* pCauldron = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 187690);
    if (pCauldron == nullptr)
        return true;

    float posX = pCauldron->GetPositionX();

    if (posX == 3747.07f)
        pSpell->getPlayerCaster()->addQuestKill(11647, 0, 0);

    if (posX == 4023.5f)
        pSpell->getPlayerCaster()->addQuestKill(11647, 1, 0);

    if (posX == 4126.12f)
        pSpell->getPlayerCaster()->addQuestKill(11647, 2, 0);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Stop the Plague
bool HighmessasCleansingSeeds(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr || !pSpell->getPlayerCaster()->IsInWorld())
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();
    pPlayer->addQuestKill(11677, 0, 0);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// There's Something Going On In Those Caves
bool BixiesInhibitingPowder(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr || !pSpell->getPlayerCaster()->IsInWorld())
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();
    pPlayer->addQuestKill(11694, 0, 0);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Leading the Ancestors Home
bool CompleteAncestorRitual(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr || !pSpell->getPlayerCaster()->IsInWorld())
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    if (!pPlayer->hasQuestInQuestLog(11610))
        return true;

    GameObject* pElderObj = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 191088);
    if (pElderObj != nullptr && pPlayer->GetDistance2dSq(pElderObj) < 8.0f)
        pPlayer->addQuestKill(11610, 0, 0);

    pElderObj = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 191089);
    if (pElderObj != nullptr && pPlayer->GetDistance2dSq(pElderObj) < 8.0f)
        pPlayer->addQuestKill(11610, 1, 0);

    pElderObj = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 191090);
    if (pElderObj != nullptr && pPlayer->GetDistance2dSq(pElderObj) < 8.0f)
        pPlayer->addQuestKill(11610, 2, 0);

    return true;
}

bool PoweringOurDefenses(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();

    plr->addQuestKill(8490, 0, 0);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Testing the Antidote
bool TestingTheAntidote(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getUnitTarget() || !pSpell->getUnitTarget()->isCreature())
        return true;

    Creature* target = static_cast<Creature*>(pSpell->getUnitTarget());
    if (target == nullptr || target->getEntry() != 16880) // Hulking Helboar
        return true;

    LocationVector targetPos = target->GetPosition();

    Creature* spawned = target->getWorldMap()->getInterface()->spawnCreature(16992, targetPos, true, false, 0, 0);
    if (spawned == nullptr)
        return true;

    target->Despawn(0, 300000);

    spawned->getAIInterface()->setCurrentTarget(pSpell->getUnitCaster());

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Zeth'Gor Must Burn!
bool ZethGorMustBurnHorde(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (auto* questLog = pPlayer->getQuestLogByQuestId(10792))
    {
        // Barracks
        if (questLog->getMobCountByIndex(0) < questLog->getQuestProperties()->required_mob_or_go_count[0])
        {
            GameObject* pBarracks = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-1137.0f, 1970.0f, 74.0f, 300151);
            if (pBarracks != nullptr && pPlayer->CalcDistance(pPlayer, pBarracks) < 30)
            {
                pPlayer->addQuestKill(10792, 0, 0);

                GameObject* pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-1129.08f, 1921.77f, 94.0074f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-1135.00f, 1944.05f, 84.7084f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-1152.01f, 1945.00f, 102.901f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-1159.60f, 1958.76f, 83.0412f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-1126.17f, 1880.96f, 95.065f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-1185.79f, 1968.29f, 90.931f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                return true;
            }
        }

        // Eastern Hovel
        if (questLog->getMobCountByIndex(1) < questLog->getQuestProperties()->required_mob_or_go_count[1])
        {
            GameObject* pEasternHovel = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-940.0f, 1920.0f, 69.0f, 300151);
            if (pEasternHovel != nullptr && pPlayer->CalcDistance(pPlayer, pEasternHovel) < 30)
            {
                pPlayer->addQuestKill(10792, 1, 0);

                GameObject* pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-938.034f, 1924.153f, 73.590f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                return true;
            }
        }

        // Western Hovel
        if (questLog->getMobCountByIndex(2) < questLog->getQuestProperties()->required_mob_or_go_count[2])
        {
            GameObject* pWesternHovel = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-1155.0f, 2061.0f, 68.0f, 300151);
            if (pWesternHovel != nullptr && pPlayer->CalcDistance(pPlayer, pWesternHovel) < 30)
            {
                pPlayer->addQuestKill(10792, 2, 0);

                GameObject* pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-1152.10f, 2066.20f, 72.959f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                return true;
            }
        }

        // Stable
        if (questLog->getMobCountByIndex(3) < questLog->getQuestProperties()->required_mob_or_go_count[3])
        {
            GameObject* pStable = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-1052.0f, 2007.0f, 66.0f, 300151);
            if (pStable != nullptr && pPlayer->CalcDistance(pPlayer, pStable) < 30)
            {
                pPlayer->addQuestKill(10792, 3, 0);

                GameObject* pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-1058.85f, 2010.95f, 68.776f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                return true;
            }
        }
    }
    else
    {
        pPlayer->broadcastMessage("Missing required quest : Zeth'Gor Must Burn");
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Laying Waste to the Unwanted
bool LayingWasteToTheUnwantedAlliance(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    if (auto* questLog = pPlayer->getQuestLogByQuestId(10078))
    {
        // Eastern Thrower
        if (questLog->getMobCountByIndex(0) < questLog->getQuestProperties()->required_mob_or_go_count[0])
        {
            GameObject* pEasternTower = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-155.0f, 2517.0f, 43.0f, 300152);
            if (pEasternTower != nullptr && pPlayer->CalcDistance(pPlayer, pEasternTower) < 30)
            {
                pPlayer->addQuestKill(10078, 0, 0);

                GameObject* pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-157.916f, 2517.71f, 58.5508f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                return true;
            }
        }

        // Central Eastern Thrower
        if (questLog->getMobCountByIndex(1) < questLog->getQuestProperties()->required_mob_or_go_count[1])
        {
            GameObject* pCentralEasternTower = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-152.0f, 2661.0f, 44.0f, 300152);
            if (pCentralEasternTower != nullptr && pPlayer->CalcDistance(pPlayer, pCentralEasternTower) < 30)
            {
                pPlayer->addQuestKill(10078, 1, 0);

                GameObject* pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-152.527f, 2661.99f, 60.8123f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                return true;
            }
        }

        // Central Western Thrower
        if (questLog->getMobCountByIndex(2) < questLog->getQuestProperties()->required_mob_or_go_count[2])
        {
            GameObject* pCentralWesternTower = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-174.0f, 2772.0f, 32.0f, 300152);
            if (pCentralWesternTower != nullptr && pPlayer->CalcDistance(pPlayer, pCentralWesternTower) < 30)
            {
                pPlayer->addQuestKill(10078, 2, 0);

                GameObject* pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-177.916f, 2773.75f, 48.636f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                return true;
            }
        }

        // Western Thrower
        if (questLog->getMobCountByIndex(3) < questLog->getQuestProperties()->required_mob_or_go_count[3])
        {
            GameObject* pWesternTower = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-166.0f, 2818.0f, 29.0f, 300152);
            if (pWesternTower != nullptr && pPlayer->CalcDistance(pPlayer, pWesternTower) < 30)
            {
                pPlayer->addQuestKill(10078, 3, 0);

                GameObject* pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-166.0f, 2818.0f, 29.0f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                return true;
            }
        }
    }
    else
    {
        pPlayer->broadcastMessage("Missing required quest : Laying Waste to the Unwanted");
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Burn It Up... For the Horde!
bool BurnItUp(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    if (auto* questLog = pPlayer->getQuestLogByQuestId(10087))
    {
        if (questLog->getMobCountByIndex(0) < questLog->getQuestProperties()->required_mob_or_go_count[0])
        {
            GameObject* pEastern = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-300.0f, 2407.0f, 50.0f, 183122);
            if (pEastern == nullptr)
                pEastern = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-300.0f, 2407.0f, 50.0f, 185122);

            if (pEastern != nullptr && pPlayer->CalcDistance(pPlayer, pEastern) < 30)
            {
                pPlayer->addQuestKill(10087, 0, 0);

                GameObject* pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-300.0f, 2407.0f, 50.0f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                return true;
            }
        }

        if (questLog->getMobCountByIndex(1) < questLog->getQuestProperties()->required_mob_or_go_count[1])
        {
            GameObject* pWestern = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-350.0f, 2708.0f, 35.0f, 183122);
            if (pWestern == nullptr)
                pWestern = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-350.0f, 2708.0f, 35.0f, 185122);

            if (pWestern != nullptr && pPlayer->CalcDistance(pPlayer, pWestern) < 30)
            {
                pPlayer->addQuestKill(10087, 1, 0);

                GameObject* pGameobject = pPlayer->getWorldMap()->createAndSpawnGameObject(183816, LocationVector(-350.0f, 2708.0f, 35.0f, 0), 4);
                if (pGameobject != nullptr)
                    pGameobject->despawn(1 * 60 * 1000, 0);

                return true;
            }
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// The Seer's Relic
bool TheSeersRelic(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();
    QuestLogEntry* questLog = pPlayer->getQuestLogByQuestId(9545);
    if (questLog == nullptr || questLog->getMobCountByIndex(0) >= questLog->getQuestProperties()->required_mob_or_go_count[0])
        return true;

    WoWGuid wowGuid;
    wowGuid.Init(pPlayer->getTargetGuid());

    Creature* pTarget = pPlayer->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
    if (pTarget == nullptr)
        return true;

    if (pTarget->getEntry() != 16852)
        return true;

    pTarget->setStandState(STANDSTATE_STAND);
    pTarget->setDeathState(ALIVE);
    pTarget->Despawn(30 * 1000, 1 * 60 * 1000);

    pPlayer->addQuestKill(9545, 0, 0);

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Disrupt Their Reinforcements
bool DisruptTheirReinforcements(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();
    QuestLogEntry* questLogAliance = pPlayer->getQuestLogByQuestId(10144);
    QuestLogEntry* questLogHorde = pPlayer->getQuestLogByQuestId(10208);

    if (questLogAliance != nullptr)
    {
        bool SendMsg = false;
        if (questLogAliance->getMobCountByIndex(0) < questLogAliance->getQuestProperties()->required_mob_or_go_count[0])
        {
            GameObject* pGrimh = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-419.0f, 1847.0f, 80.0f, 184414);
            if (pGrimh != nullptr && pPlayer->CalcDistance(pPlayer, pGrimh) < 10)
                pPlayer->addQuestKill(10144, 0, 0);
            else
                SendMsg = true;
        }

        if (questLogAliance->getMobCountByIndex(1) < questLogAliance->getQuestProperties()->required_mob_or_go_count[1])
        {
            GameObject* pKaalez = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-548.0f, 1782.0f, 58.0f, 184415);
            if (pKaalez != nullptr && pPlayer->CalcDistance(pPlayer, pKaalez) < 10)
                pPlayer->addQuestKill(10144, 1, 0);
            else
                SendMsg = true;
        }

        if (SendMsg)
            pPlayer->broadcastMessage("Go to the Port of the Dark Legion!");
    }
    else if (questLogHorde != nullptr)
    {
        bool SendMsg = false;
        if (questLogHorde->getMobCountByIndex(0) < questLogHorde->getQuestProperties()->required_mob_or_go_count[0])
        {
            GameObject* pXilus = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-85.0f, 1880.0f, 74.0f, 184290);
            if (pXilus != nullptr && pPlayer->CalcDistance(pPlayer, pXilus) < 10)
                pPlayer->addQuestKill(10208, 0, 0);
            else
                SendMsg = true;
        }
        if (questLogHorde->getMobCountByIndex(1) < questLogHorde->getQuestProperties()->required_mob_or_go_count[1])
        {
            GameObject* pKruul = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(147.0f, 1717.0f, 38.0f, 184289);
            if (pKruul != nullptr && pPlayer->CalcDistance(pPlayer, pKruul) < 10)
                pPlayer->addQuestKill(10208, 1, 0);
            else
                SendMsg = true;
        }

        if (SendMsg)
            pPlayer->broadcastMessage("Go to the Port of the Dark Legion!");
    }
    else
    {
        pPlayer->broadcastMessage("Missing required quest : Disrupt Their Reinforcements");
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
//Arzeth's Demise
bool FuryOfTheDreghoodElders(uint32_t /*i*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    Unit* pUnit = pSpell->getUnitTarget();
    if (pUnit == nullptr || !pUnit->isCreature() || pUnit->getEntry() != 19354)
        return true;

    LocationVector targetPos = pUnit->GetPosition();

    Creature* elder = pPlayer->getWorldMap()->createAndSpawnCreature(20680, targetPos);
    if (elder != nullptr)
        elder->Despawn(5 * 60 * 1000, 0);

    static_cast<Creature*>(pUnit)->Despawn(0, 3 * 60 * 1000);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// War is Hell
bool WarIsHell(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();

    LocationVector pos = plr->GetPosition();

    Creature* target = plr->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 24008);
    if (target == nullptr)
        return true;

    if (!plr->hasQuestInQuestLog(11270))
        return true;

    plr->addQuestKill(11270, 0, 0);

    LocationVector targetPos = target->GetPosition();

    GameObject* obj = plr->getWorldMap()->createAndSpawnGameObject(183816, targetPos, 1);
    if (obj != nullptr)
        obj->despawn(1 * 60 * 1000, 0);

    target->Despawn(2000, 60 * 1000);
    plr->updateNearbyQuestGameObjects();

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// A Lesson in Fear
bool PlantForsakenBanner(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    if (!pPlayer->hasQuestInQuestLog(11282))
        return true;

    Creature* target = dynamic_cast<Creature*>(pSpell->getUnitTarget());
    if (target == nullptr || target->isAlive())
        return true;

    const uint32_t cit = target->getEntry();
    switch (cit)
    {
        case 24161:
        {
            pPlayer->addQuestKill(11282, 0, 0);
            target->Despawn(0, 3 * 60 * 1000);
        } break;
        case 24016:
        {
            pPlayer->addQuestKill(11282, 1, 0);
            target->Despawn(0, 3 * 60 * 1000);
        } break;
        case 24162:
        {
            pPlayer->addQuestKill(11282, 2, 0);
            target->Despawn(0, 3 * 60 * 1000);
        } break;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Erratic Behavior
bool ConvertingSentry(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pCaster = pSpell->getPlayerCaster();
    if (pCaster == nullptr)
        return true;

    Creature* pTarget = dynamic_cast<Creature*>(pSpell->getUnitTarget());
    if (pTarget == nullptr || pTarget->getEntry() != 24972 || pTarget->isAlive())   // Erratic Sentry: 24972
        return true;

    auto* questLog = pCaster->getQuestLogByQuestId(11525);
    if (questLog == nullptr)
    {
        questLog = pCaster->getQuestLogByQuestId(11524);
        if (questLog == nullptr)
            return true;
    }

    if (questLog->getMobCountByIndex(0) == questLog->getQuestProperties()->required_mob_or_go_count[0])
        return true;

    pCaster->addQuestKill(11525, 0, 0);
    pCaster->addQuestKill(11524, 0, 0);

    pTarget->Despawn(500, 2 * 60 * 1000);

    return true;
}

bool OrbOfMurlocControl(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    if (auto* questLog = pPlayer->getQuestLogByQuestId(11541))
    {
        Creature* pTarget;

        for (const auto& itr : pSpell->getCaster()->getInRangeObjectsSet())
        {
            if (itr && itr->isCreatureOrPlayer() && static_cast<Unit*>(itr)->isCreature())
                pTarget = static_cast<Creature*>(itr);
            else
                continue;

            if (pSpell->getCaster()->CalcDistance(pTarget) > 5)
                continue;

            if (pTarget->getEntry() == 25084)
            {
                const LocationVector targetPos = pTarget->GetPosition();

                if (questLog->getMobCountByIndex(0) < questLog->getQuestProperties()->required_mob_or_go_count[0])
                {
                    pPlayer->addQuestKill(11541, 0, 0);

                    Creature* FreedGreengill = pPlayer->getWorldMap()->createAndSpawnCreature(25085, targetPos);
                    if (FreedGreengill != nullptr)
                        FreedGreengill->Despawn(6 * 60 * 1000, 0);

                    pTarget->Despawn(0, 6 * 60 * 1000);

                    return true;
                }
            }
        }
    }
    return true;
}

bool ShipBombing(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* questLog = pPlayer->getQuestLogByQuestId(11542);
    if (questLog == nullptr)
    {
        questLog = pPlayer->getQuestLogByQuestId(11543);
        if (questLog == nullptr)
            return true;
    }

    GameObject* pSinloren = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(13200.232422f, -7049.176270f, 3.838517f, 550000);
    GameObject* pBloodoath = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(13319.419922f, -6988.779785f, 4.002993f, 550000);
    GameObject* pDawnchaser = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(13274.51625f, -7145.434570f, 4.770292f, 550000);

    GameObject* obj;

    if (pSinloren != nullptr)
    {
        if (questLog->getMobCountByIndex(0) < questLog->getQuestProperties()->required_mob_or_go_count[0])
        {
            if (pPlayer->CalcDistance(pPlayer, pSinloren) < 15)
            {
                pPlayer->addQuestKill(11542, 0, 0);
                pPlayer->addQuestKill(11543, 0, 0);

                obj = pPlayer->getWorldMap()->createAndSpawnGameObject(GO_FIRE, LocationVector(13214.3f, -7059.19f, 17.5717f, 1.58573f), 1);
                if (obj != nullptr)
                    obj->despawn(2 * 60 * 1000, 0);

                obj = pPlayer->getWorldMap()->createAndSpawnGameObject(GO_FIRE, LocationVector(13204.2f, -7059.38f, 17.5717f, 1.57787f), 1);
                if (obj != nullptr)
                    obj->despawn(2 * 60 * 1000, 0);
            }
        }
    }

    if (pBloodoath != nullptr)
    {
        if (questLog->getMobCountByIndex(1) < questLog->getQuestProperties()->required_mob_or_go_count[1])
        {
            if (pPlayer->CalcDistance(pPlayer, pBloodoath) < 15)
            {
                pPlayer->addQuestKill(11542, 1, 0);
                pPlayer->addQuestKill(11543, 1, 0);

                obj = pPlayer->getWorldMap()->createAndSpawnGameObject(GO_FIRE, LocationVector(13329.4f, -6994.70f, 14.5219f, 1.38938f), 1);
                if (obj != nullptr)
                    obj->despawn(2 * 60 * 1000, 0);

                obj = pPlayer->getWorldMap()->createAndSpawnGameObject(GO_FIRE, LocationVector(13315.4f, -6990.72f, 14.7647f, 1.25979f), 1);
                if (obj != nullptr)
                    obj->despawn(2 * 60 * 1000, 0);
            }
        }
    }

    if (pDawnchaser != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, pDawnchaser) < 15)
        {
            if (questLog->getMobCountByIndex(2) < questLog->getQuestProperties()->required_mob_or_go_count[2])
            {
                pPlayer->addQuestKill(11542, 2, 0);
                pPlayer->addQuestKill(11543, 2, 0);

                obj = pPlayer->getWorldMap()->createAndSpawnGameObject(GO_FIRE, LocationVector(13284.1f, -7152.65f, 15.9774f, 1.44828f), 1);
                if (obj != nullptr)
                    obj->despawn(2 * 60 * 1000, 0);

                obj = pPlayer->getWorldMap()->createAndSpawnGameObject(GO_FIRE, LocationVector(13273.0f, -7151.21f, 15.9774f, 1.39723f), 1);
                if (obj != nullptr)
                    obj->despawn(2 * 60 * 1000, 0);
            }
        }
    }

    return true;
}

bool ImpaleEmissary(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    const LocationVector pos = pPlayer->GetPosition();

    QuestLogEntry* questLog = pPlayer->getQuestLogByQuestId(11537);
    if (questLog == nullptr)
    {
        questLog = pPlayer->getQuestLogByQuestId(11538);
        if (questLog == nullptr)
            return true;
    }

    Creature* pEmissary = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 25003);
    if (pEmissary == nullptr)
        return true;

    if (questLog->getMobCountByIndex(0) < questLog->getQuestProperties()->required_mob_or_go_count[0])
    {
        pPlayer->addQuestKill(11537, 0, 0);
        pPlayer->addQuestKill(11538, 0, 0);

        pEmissary->Despawn(0, 3 * 60 * 1000);
    }

    return true;
}

bool LeyLine(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    const LocationVector pos = pPlayer->GetPosition();

    if (QuestLogEntry* questLog = pPlayer->getQuestLogByQuestId(11547))
    {
        uint32_t portals[] = { 25156, 25154, 25157 };

        for (uint8_t i = 0; i < 3; i++)
        {
            Object* portal = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, portals[i]);
            if (portal != nullptr && questLog->getMobCountByIndex(i) < questLog->getQuestProperties()->required_mob_or_go_count[i])
            {
                pPlayer->addQuestKill(11547, i, 0);
                break;
            }
        }
    }

    return true;
}

bool ManaRemnants(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return false;

    const LocationVector pos = pPlayer->GetPosition();

    Creature* Ward = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 40404);
    if (Ward == nullptr)
        return false;

    uint32_t quests[] = { 11496, 11523 };

    for (uint32_t questId : quests)
    {
        QuestLogEntry* questLog = pPlayer->getQuestLogByQuestId(questId);
        if (questLog != nullptr && questLog->getMobCountByIndex(0) < questLog->getQuestProperties()->required_mob_or_go_count[0])
        {
            pPlayer->castSpell(Ward, sSpellMgr.getSpellInfo(44981), false);
            pPlayer->setChannelObjectGuid(Ward->getGuid());
            pPlayer->setChannelSpellId(44981);

            pPlayer->addQuestKill(questId, 0, 0);
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Stopping the Spread
bool StoppingTheSpread(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();

    const LocationVector pos = plr->GetPosition();

    Creature* target = plr->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 18240);
    if (target == nullptr)
        return true;

    const LocationVector targetPos = plr->GetPosition();

    if (auto* questLog = plr->getQuestLogByQuestId(9874))
    {
        if (questLog->getMobCountByIndex(0) < questLog->getQuestProperties()->required_mob_or_go_count[0])
        {
            questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
            questLog->sendUpdateAddKill(0);

            GameObject* obj = plr->getWorldMap()->createAndSpawnGameObject(183816, targetPos, 1);
            if (obj != nullptr)
                obj->despawn(1 * 30 * 1000, 0);
        }

        target->Despawn(2000, 60 * 1000);
        plr->updateNearbyQuestGameObjects();
        questLog->updatePlayerFields();
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//Ruthless Cunning
bool RuthlessCunning(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();

    const LocationVector pos = plr->GetPosition();

    Creature* kilsorrow = plr->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z);
    if (kilsorrow == nullptr || kilsorrow->isAlive())
        return true;

    QuestLogEntry* questLog = plr->getQuestLogByQuestId(9927);
    if (questLog && questLog->getMobCountByIndex(0) < questLog->getQuestProperties()->required_mob_or_go_count[0])
    {
        kilsorrow->Despawn(0, 60000);
        plr->addQuestKill(9927, 0, 0);
    }

    return true;
}

bool FindingTheKeymaster(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();
    plr->addQuestKill(10256, 0, 0);

    return true;
}

bool TheFleshLies(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();

    const LocationVector pos = plr->GetPosition();

    Creature* target = plr->getWorldMap()->getInterface()->getCreatureNearestCoords(pos.x, pos.y, pos.z, 20561);
    if (target == nullptr)
        return true;

    const LocationVector tyrgetPos = target->GetPosition();

    if (auto* questLog = plr->getQuestLogByQuestId(10345))
    {
        if (questLog->getMobCountByIndex(0) < questLog->getQuestProperties()->required_mob_or_go_count[0])
        {
            plr->addQuestKill(10345, 0, 0);

            GameObject* obj = plr->getWorldMap()->createAndSpawnGameObject(183816, tyrgetPos, 1);
            if (obj != nullptr)
                obj->despawn(1 * 30 * 1000, 0);
        }
        target->Despawn(2000, 60 * 1000);
        plr->updateNearbyQuestGameObjects();
    }

    return true;
}

bool SurveyingtheRuins(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    if (auto* questLog = pPlayer->getQuestLogByQuestId(10335))
    {
        if (questLog->getMobCountByIndex(0) < questLog->getQuestProperties()->required_mob_or_go_count[0])
        {
            GameObject* mark1 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(4695.2f, 2603.39f, 209.878f, 184612);
            if (mark1 == nullptr)
                mark1 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(4695.28f, 2602.73f, 209.84f, 300095);

            if (mark1 != nullptr && pPlayer->CalcDistance(pPlayer, mark1) < 15)
            {
                pPlayer->addQuestKill(10335, 0, 0);
                return true;
            }
        }

        if (questLog->getMobCountByIndex(1) < questLog->getQuestProperties()->required_mob_or_go_count[1])
        {
            GameObject* mark2 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(4608.08f, 2442.02f, 195.71f, 184612);
            if (mark2 == nullptr)
                mark2 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(4607.71f, 2440.72f, 195.65f, 300095);

            if (mark2 != nullptr && pPlayer->CalcDistance(pPlayer, mark2) < 15)
            {
                pPlayer->addQuestKill(10335, 1, 0);
                return true;
            }
        }

        if (questLog->getMobCountByIndex(2) < questLog->getQuestProperties()->required_mob_or_go_count[2])
        {
            GameObject* mark3 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(4716.37f, 2371.59f, 198.168f, 184612);
            if (mark3 == nullptr)
                mark3 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(4716.77f, 2371.6f, 198.19f, 300095);

            if (mark3 != nullptr && pPlayer->CalcDistance(pPlayer, mark3) < 15)
            {
                pPlayer->addQuestKill(10335, 2, 0);
                return true;
            }
        }
    }

    return true;
}

bool CrystalOfDeepShadows(uint8_t /*effectIndex*/, Spell* pSpell) // Becoming a Shadoweave Tailor
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();

    if (auto* questLog = plr->getQuestLogByQuestId(10833))
    {
        questLog->setMobCountForIndex(0, 1);
        questLog->sendUpdateAddKill(0);
        questLog->updatePlayerFields();
    }

    return true;
}

bool Carcass(uint8_t /*effectIndex*/, Spell* pSpell) // Becoming a Shadoweave Tailor
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    QuestLogEntry* pQuest = pPlayer->getQuestLogByQuestId(10804);
    Creature* NetherDrake = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 21648);
    GameObject* FlayerCarcass = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 185155);

    if (FlayerCarcass == nullptr)
    {
        FlayerCarcass = pPlayer->getWorldMap()->createAndSpawnGameObject(185155, pPlayer->GetPosition(), 1);
        FlayerCarcass->despawn(60000, 0);
    }
    if (NetherDrake == nullptr)
        return true;

    if (NetherDrake->hasAurasWithId(38502))
        return true;

    if (pQuest != nullptr && pQuest->getMobCountByIndex(0) < pQuest->getQuestProperties()->required_mob_or_go_count[0])
    {
        NetherDrake->castSpell(NetherDrake, sSpellMgr.getSpellInfo(38502), true);
        NetherDrake->getMovementManager()->moveTakeoff(0, pPlayer->GetPosition());
        pPlayer->addQuestKill(10804, 0, 0);
    }
    return true;
}

bool ForceofNeltharakuSpell(uint8_t /*effectIndex*/, Spell* pSpell) // Becoming a Shadoweave Tailor
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    WoWGuid wowGuid;
    wowGuid.Init(pPlayer->getTargetGuid());
    Creature* pTarget = pPlayer->getWorldMap()->getCreature(wowGuid.getGuidLowPart());

    if (pTarget == nullptr)
        return true;

    QuestLogEntry* pQuest = pPlayer->getQuestLogByQuestId(10854);
    if (pQuest == nullptr)
        return true;

    if (pTarget->getEntry() == 21722 && pPlayer->CalcDistance(pTarget) < 30)
    {
        if (pQuest->getMobCountByIndex(0) < pQuest->getQuestProperties()->required_mob_or_go_count[0])
        {
            pTarget->castSpell(pPlayer, sSpellMgr.getSpellInfo(38775), true);

            pPlayer->addQuestKill(10854, 0, 0);
            pTarget->setMoveRoot(false);
        }
    }
    return true;
}

bool UnlockKarynakuChains(uint32_t /*i*/, Spell* pSpell) // Becoming a Shadoweave Tailor
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    pPlayer->addQuestKill(10872, 0, 0);

    return true;
}


bool ShatariTorch(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();
    WoWGuid wowGuid;
    wowGuid.Init(plr->getTargetGuid());
    Creature* target = plr->getWorldMap()->getCreature(wowGuid.getGuidLowPart());

    if (target == nullptr)
        return true;

    LocationVector pos = target->GetPosition();

    if (plr->CalcDistance(pos.x, pos.y, pos.z, plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ()) > 5)
        return true;

    QuestLogEntry* qle = plr->getQuestLogByQuestId(10913);
    if (qle == nullptr)
        return true;

    GameObject* obj = nullptr;

    if (target->getEntry() == 21859)
    {
        if (qle->getMobCountByIndex(0) == qle->getQuestProperties()->required_mob_or_go_count[0])
            return true;

        plr->addQuestKill(10913, 0, 0);

        obj = plr->getWorldMap()->createAndSpawnGameObject(183816, pos, 1);
        if (obj != nullptr)
            obj->despawn(1 * 60 * 1000, 0);
    }
    else if (target->getEntry() == 21846)
    {
        if (qle->getMobCountByIndex(1) == qle->getQuestProperties()->required_mob_or_go_count[1])
            return true;

        plr->addQuestKill(10913, 1, 0);

        obj = plr->getWorldMap()->createAndSpawnGameObject(183816, pos, 1);
        if (obj != nullptr)
            obj->despawn(1 * 60 * 1000, 0);
    }
    else
        return true;

    target->Despawn(0, 1 * 60 * 1000);

    plr->updateNearbyQuestGameObjects();

    return true;
}

// Lost!

bool SpragglesCanteen(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();
    WoWGuid wowGuid;
    wowGuid.Init(plr->getTargetGuid());

    Creature* target = plr->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
    if (target == nullptr)
        return true;

    if (target->getEntry() != 9999)
        return true;

    QuestLogEntry* qle = plr->getQuestLogByQuestId(4492);
    if (qle == nullptr)
        return true;

    target->setStandState(STANDSTATE_STAND);
    target->setDeathState(ALIVE);

    target->Despawn(30 * 1000, 1 * 60 * 1000);

    qle->setMobCountForIndex(0, 1);
    qle->sendUpdateAddKill(0);
    qle->updatePlayerFields();

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Finding the Source
bool FindingTheSource(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->getPlayerCaster();
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* qle = pPlayer->getQuestLogByQuestId(974);
    if (qle == nullptr)
        return true;

    GameObject* place1 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-7163, -1149, -264, 148503);
    GameObject* place2 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-7281, -1244, -248, 148503);
    GameObject* place3 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-7140, -1465, -242, 148503);
    GameObject* place4 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-7328, -1461, -242, 148503);
    GameObject* place5 = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(-7092, -1305, -187, 148503);

    if (place1 != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, place1) < 11)
            pPlayer->castSpell(pPlayer, 14797, true);
    }
    if (place2 != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, place2) < 11)
            pPlayer->castSpell(pPlayer, 14797, true);
    }
    if (place3 != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, place3) < 11)
            pPlayer->castSpell(pPlayer, 14797, true);
    }
    if (place4 != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, place4) < 11)
            pPlayer->castSpell(pPlayer, 14797, true);
    }
    if (place5 != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, place5) < 11)
        {
            if (qle->getMobCountByIndex(0) < qle->getQuestProperties()->required_mob_or_go_count[0])
            {
                pPlayer->addQuestKill(974, 0, 0);
            }
        }
    }
    return true;
}

// quest 5163 - Are We There, Yeti?
bool ReleaseUmisYeti(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr || pSpell->getUnitTarget() == nullptr || !pSpell->getUnitTarget()->isCreature())
        return true;

    QuestLogEntry* qLogEntry = pSpell->getPlayerCaster()->getQuestLogByQuestId(5163);
    if (qLogEntry == nullptr)
        return true;

    Creature* target = static_cast<Creature*>(pSpell->getUnitTarget());
    static const uint32_t friends[] = { 10978, 7583, 10977 };
    for (uint8_t j = 0; j < sizeof(friends) / sizeof(uint32_t); j++)
    {
        if (target->getEntry() == friends[j] && qLogEntry->getMobCountByIndex(j) < qLogEntry->getQuestProperties()->required_mob_or_go_count[j])
        {
            pSpell->getPlayerCaster()->addQuestKill(5163, j, 0);
            return true;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Healing The Lake
bool HealingTheLake(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    pPlayer->addQuestKill(9294, 0, 0);

    return true;
}

// Protecting Our Own
bool ProtectingOurOwn(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* plr = pSpell->getPlayerCaster();

    plr->addQuestKill(10488, 0, 0);

    return true;
}

//////////////////////////////////////////////////////////////////
//Cast Fishing Net dummy handler
//( SpellId 29688 )
//
//Precondition(s)
//  Casted by Player
//  Player has quest 9452 "Red Snapper - Very Tasty"
//  Player is near a school of Red Snapper fish
//
//Effect(s)
//     Despawns the nearest fish school and gives the player one Red Snapper (item id: 23614)
//     Has a chance of spawning an Angry Murloc.
//
/////////////////////////////////////////////////////////////////
bool CastFishingNet(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr || pSpell->getGameObjectTarget() == nullptr)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();
    QuestLogEntry* pQuest = pPlayer->getQuestLogByQuestId(9452); //Red Snapper - Very Tasty
    if (pQuest == nullptr)
        return true;

    pSpell->getGameObjectTarget()->despawn(600, 20000);

    LocationVector pos = pPlayer->GetPosition();

    if (Util::getRandomUInt(10) <= 3)
    {
        Creature* pNewCreature = pPlayer->getWorldMap()->getInterface()->spawnCreature(17102, pos, true, false, 0, 0);
        if (pNewCreature != nullptr)
        {
            pNewCreature->pauseMovement(500);
            pNewCreature->setAttackTimer(MELEE, 1000);
            pNewCreature->m_noRespawn = true;
        }
    }

    if (pPlayer->getItemInterface()->GetItemCount(pQuest->getQuestProperties()->required_item[0], true) < pQuest->getQuestProperties()->required_itemcount[0])
        pPlayer->getItemInterface()->AddItemById(23614, 1, 0); //Red Snapper.

    return true;
}

uint32_t const pathSize = 22;
G3D::Vector3 const InducingVisionPath[pathSize] =
{
    { -2240.52f, -407.11f, -9.42f },
    { -2225.76f, -419.25f, -9.36f },
    { -2200.88f, -441.00f, -5.61f },
    { -2143.71f, -468.07f, -9.40f },
    { -2100.81f, -420.98f, -5.32f },
    { -2079.47f, -392.47f, -10.26f },
    { -2043.70f, -343.80f, -6.97f },
    { -2001.86f, -242.53f, -10.76f },
    { -1924.75f, -119.97f, -11.77f },
    { -1794.8f, -7.92f, -9.33f },
    { -1755.21f, 72.43f, 1.12f },
    { -1734.55f, 116.84f, -4.34f },
    { -1720.04f, 125.93f, -2.33f },
    { -1704.41f, 183.6f, 12.07f },
    { -1674.32f, 201.6f, 11.24f },
    { -1624.07f, 223.56f, 2.07f },
    { -1572.86f, 234.71f, 2.31f },
    { -1542.87f, 277.9f, 20.54f },
    { -1541.81f, 316.42f, 49.91f },
    { -1526.98f, 329.66f, 61.84f },
    { -1524.17f, 335.24f, 63.33f },
    { -1513.97f, 355.76f, 63.06f }
};

bool InducingVision(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == nullptr)
        return true;

    Player* mTarget = pSpell->getPlayerCaster();
    if (mTarget == nullptr || mTarget->getWorldMap() == nullptr || mTarget->getWorldMap()->getInterface() == nullptr)
        return true;

    Creature* creature = mTarget->getWorldMap()->getInterface()->spawnCreature(2983, LocationVector(-2238.994873f, -408.009552f, -9.424423f, 5.753043f), true, false, 0, 0);

    MovementMgr::PointsArray path(InducingVisionPath, InducingVisionPath + pathSize);
    MovementMgr::MoveSplineInit init(creature);
    init.MovebyPath(path, 0);
    init.SetWalk(true);
    creature->getMovementManager()->launchMoveSpline(std::move(init), 0, MOTION_PRIORITY_NORMAL, POINT_MOTION_TYPE);

    return true;
}

void SetupLegacyQuestItems(ScriptMgr* mgr)
{
    mgr->register_dummy_spell(3607, &YennikuRelease);
    mgr->register_dummy_spell(4141, &ScrollOfMyzrael);
    mgr->register_dummy_spell(8606, &SummonCyclonian);

    mgr->register_script_effect(15118, &TheBaitforLarkorwi1);
    mgr->register_script_effect(15119, &TheBaitforLarkorwi2);
    mgr->register_script_effect(20737, &KarangsBanner);
    mgr->register_dummy_spell(23359, &ZappedGiants);
    mgr->register_script_effect(29279, &TheCleansingMustBeStopped);

    mgr->register_dummy_aura(29314, &AdministreringtheSalve);

    mgr->register_script_effect(30015, &AnUnusualPatron);
    mgr->register_dummy_aura(30877, &TagMurloc);

    mgr->register_dummy_aura(31736, &BalanceMustBePreserved);
    mgr->register_dummy_spell(31927, &BlessingofIncineratus);
    mgr->register_script_effect(32037, &ASpiritAlly);
    mgr->register_script_effect(34622, &UnyieldingBattleHorn);
    mgr->register_script_effect(34630, &ScrapReaver);
    mgr->register_dummy_spell(34646, &BuildingAPerimeter);
    mgr->register_script_effect(34992, &SummonEkkorash);
    mgr->register_dummy_spell(35113, &MeasuringWarpEnergies);
    mgr->register_script_effect(35413, &NaturalRemedies);
    mgr->register_dummy_spell(35460, &FuryoftheDreghoodElders);
    mgr->register_dummy_spell(37573, &TemporalPhaseModulator);
    mgr->register_dummy_spell(35772, &FloraoftheEcoDomes);
    mgr->register_script_effect(36310, &ADireSituation);
    mgr->register_script_effect(37065, &Torgos);
    mgr->register_dummy_aura(37136, &MagnetoCollector);
    mgr->register_script_effect(37426, &RuuanokClaw);
    mgr->register_script_effect(38336, &WelcomingtheWolfSpirit);
    mgr->register_script_effect(38729, &RodofPurification);
    mgr->register_script_effect(39223, &CallRexxar);
    mgr->register_script_effect(39224, &Showdown);
    mgr->register_script_effect(39239, &EvilDrawsNear);
    mgr->register_dummy_spell(39238, &Fumping);
    mgr->register_dummy_spell(39246, &TheBigBoneWorm);
    mgr->register_script_effect(42390, &LayWreath);
    mgr->register_dummy_spell(43723, &CookingPot);
    mgr->register_script_effect(51770, &EmblazonRuneblade);
    mgr->register_dummy_spell(42817, &WyrmcallersHorn);
    mgr->register_script_effect(60036, &RuneOfDistortion);
    mgr->register_dummy_spell(62272, &RaeloraszSpark);
    mgr->register_dummy_spell(6509, &GoreBladder); // http://www.wowhead.com/?item=40551
    mgr->register_dummy_spell(43385, &PlagueSpray); // http://www.wowhead.com/?item=33621
    mgr->register_script_effect(46203, &GoblinWeatherMachine); // Goblin Weather machine
    mgr->register_dummy_spell(48549, &PurifiedAshes); // http://www.wowhead.com/?item=37307
    mgr->register_dummy_spell(43036, &DISMEMBER); //http://www.wowhead.com/?item=33342
    mgr->register_script_effect(45668, &CraftyBlaster); //http://www.wowhead.com/?item=34812
    mgr->register_dummy_spell(51276, &IncineratingOil); //http://www.wowhead.com/?item=38556
    mgr->register_dummy_spell(46023, &Screwdriver); //http://www.wowhead.com/?item=35116
    mgr->register_dummy_spell(55804, &TelluricPoultice); //http://www.wowhead.com/?item=41988
    mgr->register_dummy_spell(55983, &HodirsHorn); //http://www.wowhead.com/?item=42164
    mgr->register_dummy_spell(12189, &SummonEcheyakee);
    mgr->register_dummy_spell(11548, &SummonShadra);
    mgr->register_dummy_spell(45474, &RagefistTorch);
    mgr->register_script_effect(13978, &SummonAquementas); //http://www.wowhead.com/?quest=4005
    mgr->register_dummy_spell(39371, &PrayerBeads); //http://www.wowhead.com/?quest=10935

    mgr->register_script_effect(29297, &CleansingVial);

    uint32_t huntertamingquestspellids[] =
    {
        19548,
        19674,
        19687,
        19688,
        19689,
        19692,
        19693,
        19694,
        19696,
        19697,
        19699,
        19700,
        30099,
        30102,
        30105,
        30646,
        30653,
        30654,
        0
    };
    mgr->register_dummy_aura(huntertamingquestspellids, &HunterTamingQuest);

    mgr->register_dummy_aura(49590, &ArcaneDisruption);

    mgr->register_dummy_aura(37097, &ToLegionHold);

    mgr->register_script_effect(19138, &CenarionLunardust);
    mgr->register_script_effect(18974, &CenarionMoondust);
    mgr->register_dummy_spell(19512, &CurativeAnimalSalve);
    mgr->register_script_effect(19719, &TrialOfTheLake);

    mgr->register_dummy_spell(8593, &SymbolOfLife);
    mgr->register_dummy_spell(31225, &FilledShimmeringVessel);
    mgr->register_script_effect(31497, &DouseEternalFlame);

    mgr->register_dummy_spell(20804, &Triage);

    mgr->register_dummy_spell(45653, &NeutralizingTheCauldrons);
    // Stop the Plague
    mgr->register_dummy_spell(45834, &HighmessasCleansingSeeds);
    // There's Something Going On In Those Caves
    mgr->register_dummy_spell(45835, &BixiesInhibitingPowder);
    // Leading the Ancestors Home
    mgr->register_dummy_spell(45536, &CompleteAncestorRitual);

    mgr->register_dummy_spell(28247, &PoweringOurDefenses); // need to script event

    mgr->register_dummy_spell(34665, &TestingTheAntidote);
    mgr->register_dummy_spell(35724, &ZethGorMustBurnHorde);
    mgr->register_dummy_spell(32979, &LayingWasteToTheUnwantedAlliance);
    mgr->register_dummy_spell(33067, &BurnItUp);
    mgr->register_script_effect(30489, &TheSeersRelic);
    mgr->register_dummy_spell(34387, &DisruptTheirReinforcements);
    mgr->register_dummy_spell(42793, &WarIsHell);
    mgr->register_dummy_spell(43178, &PlantForsakenBanner);

    mgr->register_dummy_spell(45109, &OrbOfMurlocControl);
    mgr->register_dummy_spell(44997, &ConvertingSentry);
    mgr->register_dummy_spell(45115, &ShipBombing);
    mgr->register_dummy_spell(45030, &ImpaleEmissary);
    mgr->register_dummy_spell(45191, &LeyLine);
    mgr->register_dummy_spell(44969, &ManaRemnants);

    mgr->register_dummy_spell(4981, &InducingVision);

    mgr->register_dummy_spell(32146, &StoppingTheSpread);
    mgr->register_script_effect(32307, &RuthlessCunning);

    mgr->register_script_effect(34717, &FindingTheKeymaster);
    mgr->register_dummy_spell(35372, &TheFleshLies);
    mgr->register_dummy_spell(35246, &SurveyingtheRuins);

    mgr->register_script_effect(39094, &CrystalOfDeepShadows);
    mgr->register_dummy_spell(38439, &Carcass);
    mgr->register_dummy_spell(38762, &ForceofNeltharakuSpell);

    mgr->register_dummy_spell(39189, &ShatariTorch);

    mgr->register_dummy_spell(15591, &SpragglesCanteen);
    mgr->register_dummy_spell(16378, &FindingTheSource);

    mgr->register_dummy_spell(17166, &ReleaseUmisYeti);

    mgr->register_script_effect(28700, &HealingTheLake);

    mgr->register_script_effect(32578, &ProtectingOurOwn);

    mgr->register_dummy_spell(29866, &CastFishingNet);      // Draenei Fishing Net
}
