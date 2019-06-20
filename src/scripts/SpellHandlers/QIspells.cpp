/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "Units/Creatures/AIInterface.h"
#include "Management/Item.h"
#include "Map/MapMgr.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include <Management/QuestLogEntry.hpp>
#include "Map/MapScriptInterface.h"
#include "Spell/SpellAuras.h"
#include <Units/Creatures/Pet.h>

enum
{
    // ShipBombing
    GO_FIRE = 183816
};

bool CleansingVial(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    if (pPlayer->HasQuest(9427) == false)
        return true;

    Creature* pAggonar = pPlayer->GetMapMgr()->CreateAndSpawnCreature(17000, 428.15f, 3461.73f, 63.40f, 0);
    if (pAggonar != nullptr)
        pAggonar->Despawn(6 * 60 * 1000, 0);

    return true;
}

bool SummonCyclonian(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->u_caster == nullptr)
        return true;

    Unit* pUnit = pSpell->u_caster;

    LocationVector unitPos = pUnit->GetPosition();

    Creature* pCreature = pUnit->GetMapMgr()->GetInterface()->SpawnCreature(6239, unitPos.x, unitPos.y, unitPos.z, unitPos.o, true, false, 0, 0);
    if (pCreature != nullptr)
    {
        pCreature->Despawn(600000, 0);
    }

    return true;
}

bool ElementalPowerExtractor(uint32 /*i*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;
    Unit* pUnit = pSpell->GetUnitTarget();
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
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;

    LocationVector pos = plr->GetPosition();

    plr->GetMapMgr()->CreateAndSpawnCreature(19493, pos.x, pos.y, pos.z, 0);
    return true;
}

bool CallRexxar(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    LocationVector pos = pPlayer->GetPosition();

    if (pPlayer->HasQuest(10742))
    {
        Creature* calax1 = pPlayer->GetMapMgr()->CreateAndSpawnCreature(21984, pos.x, pos.y, pos.z, pos.o);
        if (calax1 != nullptr)
            calax1->Despawn(2 * 60 * 1000, 0);

        Creature* calax2 = pPlayer->GetMapMgr()->CreateAndSpawnCreature(20555, pos.x, pos.y, pos.z, pos.o);
        if (calax2 != nullptr)
            calax2->Despawn(2 * 60 * 1000, 0);
    }

    return true;
}

bool LayWreath(uint8_t /*effectIndex*/, Spell* pSpell)  //Peace at Last quest
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    LocationVector pos = pPlayer->GetPosition();

    if (pPlayer->HasQuest(11152))
    {
        pPlayer->AddQuestKill(11152, 0, 0);

        GameObject* pWreath = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(501541, pos.x, pos.y, pos.z, pos.o, 1);
        if (pWreath != nullptr)
            pWreath->Despawn(2 * 60 * 1000, 0);
    }

    return true;
}

bool ScrapReaver(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    LocationVector pos = pPlayer->GetPosition();

    Creature* pCreature = pPlayer->GetMapMgr()->GetInterface()->SpawnCreature(19851, pos.x, pos.y, pos.z, pos.o, true, false, 0, 0);
    if (pCreature != nullptr)
    {
        pCreature->Despawn(600000, 0);
    }

    return true;
}

bool RuuanokClaw(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->p_caster)
        return true;

    pSpell->p_caster->GetMapMgr()->GetInterface()->SpawnCreature(21767, 3210.960693f, 5348.308594f, 144.537476f, 5.450696f, true, false, 0, 0);
    return true;
}

bool KarangsBanner(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    // Banner Aura
    pPlayer->castSpell(pPlayer, sSpellMgr.getSpellInfo(20746), true);

    pSpell->p_caster->GetMapMgr()->GetInterface()->SpawnCreature(12921, 2231.710205f, -1543.603027f, 90.694946f, 4.700579f, true, false, 0, 0);
    pSpell->p_caster->GetMapMgr()->GetInterface()->SpawnCreature(12921, 2232.534912f, -1556.983276f, 89.744415f, 1.527570f, true, false, 0, 0);

    pSpell->p_caster->GetMapMgr()->GetInterface()->SpawnCreature(12757, 2239.357178f, -1546.649536f, 89.671097f, 3.530336f, true, false, 0, 0);

    return true;
}

bool ADireSituation(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster != nullptr)
    {
        pSpell->p_caster->AddQuestKill(10506, 0);
    }

    return true;
}

bool FuryoftheDreghoodElders(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->HasQuest(10369) == false)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    Creature* arzethpower = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 19354);
    if (arzethpower == nullptr)
        return true;

    LocationVector arzethPos = arzethpower->GetPosition();

    Creature* arzethless = pPlayer->GetMapMgr()->CreateAndSpawnCreature(20680, arzethPos.x, arzethPos.y, arzethPos.z, arzethPos.o);
    if (arzethless != nullptr)
        arzethless->Despawn(5 * 60 * 1000, 0);

    arzethpower->Despawn(1, 6 * 60 * 1000);

    return true;
}

bool ASpiritAlly(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->HasQuest(9847) == false)
        return true;

    Creature* allyspirit = pPlayer->GetMapMgr()->CreateAndSpawnCreature(18185, -353, 7255, 49.36f, 6.28f);
    if (allyspirit != nullptr)
        allyspirit->Despawn(6 * 60 * 1000, 0);

    return true;
}

bool BalanceMustBePreserved(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    if (apply == false)
        return true;

    if (pAura->GetCaster()->isPlayer() == false)
        return true;

    Player* pPlayer = static_cast<Player*>(pAura->GetCaster());
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(9720);
    if (qle == nullptr)
        return true;

    GameObject* lake1 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-756, 5926, 19, 300076);
    GameObject* lake2 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-213, 6302, 21, 300076);
    GameObject* lake3 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(498, 8197, 21, 300076);
    GameObject* lake4 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(566, 6358, 23, 300076);

    if (lake1 != nullptr)
        pPlayer->AddQuestKill(9720, 0, 0);

    if (lake2 != nullptr)
        pPlayer->AddQuestKill(9720, 3, 0);

    if (lake3 != nullptr)
        pPlayer->AddQuestKill(9720, 1, 0);

    if (lake4 != nullptr)
        pPlayer->AddQuestKill(9720, 2, 0);

    return true;
}

bool BlessingofIncineratus(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(9805);
    if (qle == nullptr)
        return true;

    GameObject* big = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-1972, 6276, 56, 300077);
    GameObject* east = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-1841, 6387, 52, 400050);
    GameObject* west = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-1920, 6361, 56, 400051);
    GameObject* south = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-1808, 6293, 59, 400052);

    if (big != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, big) < 30)
        {
            pPlayer->AddQuestKill(9805, 0, 0);
        }
    }

    if (east != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, east) < 30)
        {
            pPlayer->AddQuestKill(9805, 1, 0);
        }
    }

    if (south != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, south) < 30)
        {
            pPlayer->AddQuestKill(9805, 2, 0);
        }
    }

    if (west != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, west) < 30)
        {
            pPlayer->AddQuestKill(9805, 3, 0);
        }
    }
    return true;
}

bool TagMurloc(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    Object* Caster = pAura->GetCaster();
    if (Caster->isPlayer() == false)
        return false;

    if (pAura->GetTarget()->isCreature() == false)
        return false;

    if (apply == false)
        return true;

    Player* pPlayer = static_cast<Player*>(Caster);

    QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(9629);
    if (qle == nullptr)
        return true;

    Creature* murloc = static_cast<Creature*>(pAura->GetTarget());
    if (murloc == nullptr)
        return true;

    LocationVector murlocPos = pPlayer->GetPosition();

    Creature* tagged = pPlayer->GetMapMgr()->CreateAndSpawnCreature(17654, murlocPos.x, murlocPos.y, murlocPos.z, 0);
    if (tagged != nullptr)
        tagged->Despawn(5 * 60 * 1000, 0);

    murloc->Despawn(1, 6 * 60 * 1000);

    pPlayer->AddQuestKill(9629, 0, 0);

    return true;
}

bool CookingPot(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(11379);
    if (qle == nullptr)
        return true;

    pPlayer->getItemInterface()->RemoveItemAmt(31673, 1);
    pPlayer->getItemInterface()->RemoveItemAmt(31672, 2);
    pPlayer->getItemInterface()->AddItemById(33848, 1, 0);

    return true;
}

bool EvilDrawsNear(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->HasQuest(10923) == false)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    Creature* dragon = pPlayer->GetMapMgr()->CreateAndSpawnCreature(22441, pos.x + 15, pos.y + 15, pos.z, pos.o);
    dragon->Despawn(6 * 60 * 1000, 0);

    return true;
}

bool UnyieldingBattleHorn(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Creature* creat = pSpell->p_caster->GetMapMgr()->GetInterface()->SpawnCreature(19862, -1190.856079f, 2261.246582f, 46.625797f, 1.705882f, true, false, 0, 0);
    creat->Despawn(300000, 0); // 5 mins delay

    return true;
}

bool MeasuringWarpEnergies(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(10313);
    if (qle == nullptr)
        return true;

    GameObject* north = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(3216, 4045, 145, 300094);
    GameObject* east = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2766, 3865, 145, 300094);
    GameObject* west = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2958, 4318, 145, 300094);
    GameObject* south = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2817, 4337, 145, 300094);

    if (north != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, north) < 30)
        {
            pPlayer->AddQuestKill(10313, 0, 0);
        }
    }

    if (east != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, east) < 30)
        {
            pPlayer->AddQuestKill(10313, 1, 0);
        }
    }

    if (south != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, south) < 30)
        {
            pPlayer->AddQuestKill(10313, 2, 0);
        }
    }

    if (west != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, west) < 30)
        {
            pPlayer->AddQuestKill(10313, 3, 0);
        }
    }

    return true;
}

bool YennikuRelease(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(592);
    if (qle == nullptr)
        return true;

    Creature* yenniku = static_cast<Creature*>(pSpell->GetUnitTarget());
    if (yenniku == nullptr)
        return true;

    yenniku->SetFaction(29);
    yenniku->GetAIInterface()->WipeTargetList();
    yenniku->Despawn(30 * 1000, 60 * 1000);

    return true;
}

bool ScrollOfMyzrael(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(656);
    if (qle == nullptr)
        return true;

    const float MyzraelPos[] = { -940.7374f, -3111.1953f, 48.9566f, 3.327f };

    Creature* myzrael = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(MyzraelPos[0], MyzraelPos[1], MyzraelPos[2], 2755);
    if (myzrael != nullptr)
    {
        if (!myzrael->isAlive())
            myzrael->Delete();
        else
            return true;
    }

    myzrael = pPlayer->GetMapMgr()->CreateAndSpawnCreature(2755, MyzraelPos[0], MyzraelPos[1], MyzraelPos[2], MyzraelPos[3]);
    return true;
}

bool Showdown(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* p_caster = pSpell->p_caster;
    if (p_caster == nullptr)
    {
        return true;
    }

    if (p_caster->HasQuest(10742) && p_caster->HasQuest(10806) == false)
    {
        return true;
    }

    Creature* goc = p_caster->GetMapMgr()->CreateAndSpawnCreature(20555, 3739, 5365, -4, 3.5);
    goc->Despawn(6 * 60 * 1000, 0);

    return true;
}

bool TheBaitforLarkorwi1(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(4292);
    if (qle == nullptr)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    GameObject* obj = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(169216, pos.x, pos.y, pos.z, pos.o, 1);
    if (obj != nullptr)
        obj->Despawn(1 * 60 * 1000, 0);

    return true;
}

bool TheBaitforLarkorwi2(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->HasQuest(4292) == false)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    Creature* larkowi = pPlayer->GetMapMgr()->CreateAndSpawnCreature(9684, pos.x + 2, pos.y + 3, pos.z, pos.o);
    larkowi->Despawn(5 * 60 * 1000, 0);

    return true;
}

bool Fumping(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->HasQuest(10929) == false)
        return true;

    uint32 chance = Util::getRandomUInt(1);
    uint32 entry = 0;
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

    Creature* creat = pPlayer->GetMapMgr()->CreateAndSpawnCreature(entry, pos.x, pos.y, pos.z, 0);
    if (entry == 22483) //Sand Gnomes ;)
    {
        creat->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "YIEEEEEEEAA!");
    }
    creat->Despawn(5 * 60 * 1000, 0);

    return true;
}

bool TheBigBoneWorm(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->HasQuest(10930) == false)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    Creature* exarch = pPlayer->GetMapMgr()->CreateAndSpawnCreature(22038, pos.x + 7, pos.y + 7, pos.z, pos.o);
    exarch->Despawn(6 * 60 * 1000, 0);

    return true;
}

bool Torgos(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->HasQuest(10035) == false)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    Creature* torgos = pPlayer->GetMapMgr()->CreateAndSpawnCreature(18707, pos.x, pos.y - 10, pos.z, pos.o);
    if (torgos == nullptr)
        return true;

    torgos->Despawn(6 * 60 * 1000, 0);
    return true;
}

bool WelcomingtheWolfSpirit(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    Creature* spiritwolf = pPlayer->GetMapMgr()->CreateAndSpawnCreature(19616, pos.x + 2, pos.y + 3, pos.z, pos.o);
    spiritwolf->Despawn(5 * 60 * 1000, 0);

    pPlayer->AddQuestKill(10791, 0, 0);

    return true;
}

bool NaturalRemedies(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    if (pPlayer->HasQuest(10351) == false)
        return true;

    Creature* colos = pPlayer->GetMapMgr()->CreateAndSpawnCreature(19305, pos.x, pos.y, pos.z, 0);
    if (colos != nullptr)
        colos->Despawn(5 * 60 * 1000, 0);

    return true;
}

bool FloraoftheEcoDomes(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr || pSpell->GetUnitTarget() == nullptr || pSpell->GetUnitTarget()->isCreature() == false)
        return true;

    Player* pPlayer = pSpell->p_caster;

    Creature* normal = static_cast<Creature*>(pSpell->GetUnitTarget());

    LocationVector normPos = normal->GetPosition();

    Creature* mutant = pPlayer->GetMapMgr()->CreateAndSpawnCreature(20983, normPos.x, normPos.y, normPos.z, 0);

    normal->Despawn(1, 6 * 60 * 1000);
    mutant->Despawn(5 * 60 * 1000, 0);

    mutant->GetAIInterface()->Init(mutant, AI_SCRIPT_AGRO, Movement::WP_MOVEMENT_SCRIPT_NONE);
    mutant->GetAIInterface()->taunt(pPlayer, true);

    pPlayer->AddQuestKill(10426, 0, 0);

    return true;
}

bool TheCleansingMustBeStopped(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    if (pPlayer->HasQuest(9370) == false)
        return true;

    Creature* draenei1 = pPlayer->GetMapMgr()->CreateAndSpawnCreature(16994, pos.x + Util::getRandomFloat(5.0f), pos.y + Util::getRandomFloat(5.0f), pos.z, pos.o);
    draenei1->Despawn(6 * 60 * 1000, 0);

    Creature* draenei2 = pPlayer->GetMapMgr()->CreateAndSpawnCreature(16994, pos.x - Util::getRandomFloat(5.0f), pos.y + Util::getRandomFloat(5.0f), pos.z, pos.o);
    draenei2->Despawn(6 * 60 * 1000, 0);

    Creature* draenei3 = pPlayer->GetMapMgr()->CreateAndSpawnCreature(16994, pos.x + Util::getRandomFloat(5.0f), pos.y - Util::getRandomFloat(5.0f), pos.z, pos.o);
    draenei3->Despawn(6 * 60 * 1000, 0);

    return true;
}

bool AdministreringtheSalve(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    Object* m_caster = pAura->GetCaster();
    if (m_caster->isPlayer() == false)
        return true;

    if (pAura->GetTarget()->isCreature() == false)
        return true;

    if (apply)
    {
        Player* pPlayer = static_cast<Player*>(m_caster);

        QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(9447);
        if (qle == nullptr)
            return true;

        Creature* sick = static_cast<Creature*>(pAura->GetTarget());
        if (sick == nullptr)
            return true;

        LocationVector sickPos = sick->GetPosition();

        Creature* healed = pPlayer->GetMapMgr()->CreateAndSpawnCreature(16846, sickPos.x, sickPos.y, sickPos.z, 0);
        sick->Despawn(1, 6 * 60 * 1000);
        healed->Despawn(3 * 60 * 1000, 0);

        pPlayer->AddQuestKill(9447, 0, 0);
    }

    return true;
}

bool ZappedGiants(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->HasQuest(7003) == false && pPlayer->HasQuest(7725) == false)
        return true;

    Creature* creat = static_cast<Creature*>(pSpell->GetUnitTarget());
    if (creat == nullptr)
        return true;

    LocationVector creatPos = creat->GetPosition();

    uint32 cit = creat->getEntry();
    switch (cit)
    {
        case 5360:
        {
            creat->Despawn(1000, 6 * 60 * 1000);
            Creature* zappedcreat = pPlayer->GetMapMgr()->CreateAndSpawnCreature(14639, creatPos.x, creatPos.y, creatPos.z, 0);
            zappedcreat->Despawn(3 * 60 * 1000, 0);
        }
        break;
        case 5361:
        {
            creat->Despawn(1000, 6 * 60 * 1000);
            Creature* zappedcreat = pPlayer->GetMapMgr()->CreateAndSpawnCreature(14638, creatPos.x, creatPos.y, creatPos.z, 0);
            zappedcreat->Despawn(3 * 60 * 1000, 0);
        }
        break;
        case 5359:
        {
            creat->Despawn(1000, 6 * 60 * 1000);
            Creature* zappedcreat = pPlayer->GetMapMgr()->CreateAndSpawnCreature(14603, creatPos.x, creatPos.y, creatPos.z, 0);
            zappedcreat->Despawn(3 * 60 * 1000, 0);
        }
        break;
        case 5358:
        {
            creat->Despawn(1000, 6 * 60 * 1000);
            Creature* zappedcreat = pPlayer->GetMapMgr()->CreateAndSpawnCreature(14640, creatPos.x, creatPos.y, creatPos.z, 0);
            zappedcreat->Despawn(3 * 60 * 1000, 0);
        }
        break;
        case 5357:
        {
            creat->Despawn(1000, 6 * 60 * 1000);
            Creature* zappedcreat = pPlayer->GetMapMgr()->CreateAndSpawnCreature(14604, creatPos.x, creatPos.y, creatPos.z, 0);
            zappedcreat->Despawn(3 * 60 * 1000, 0);
        }
        break;
    }
    return true;
}

bool BuildingAPerimeter(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    GameObject* pEast = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2257.0f, 2465.0f, 101.0f, 183947);
    if (pEast != nullptr && pPlayer->CalcDistance(pPlayer, pEast) < 30)
    {
        pPlayer->AddQuestKill(10313, 0, 0);
        return true;
    }

    GameObject* pNorth = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2375.0f, 2285.0f, 141.0f, 183947);
    if (pNorth != nullptr && pPlayer->CalcDistance(pPlayer, pNorth) < 30)
    {
        pPlayer->AddQuestKill(10313, 1, 0);
        return true;
    }

    GameObject* pWest = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2283.0f, 2181.0f, 95.0f, 183947);
    if (pWest != nullptr && pPlayer->CalcDistance(pPlayer, pWest) < 30)
    {
        pPlayer->AddQuestKill(10313, 2, 0);
        return true;
    }

    return true;
}

bool RodofPurification(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(10839);
    if (qle == nullptr)
        return true;

    GameObject* Darkstone = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-2512, 5418, 0, 185191);
    if (Darkstone != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, Darkstone) < 15)
        {
            qle->SendQuestComplete();
        }
    }

    return true;
}

bool AnUnusualPatron(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    if (pPlayer->HasQuest(9457) == false)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    Creature* Naias = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 17207);
    if (Naias != nullptr)
        return true;

    Naias = pPlayer->GetMapMgr()->CreateAndSpawnCreature(17207, pos.x, pos.y, pos.z, pos.o);
    Naias->Despawn(10 * 60 * 1000, 0);
    return true;
}

bool MagnetoCollector(uint8_t /*effectIndex*/, Aura* pAura, bool /*apply*/)
{
    if (pAura->GetCaster()->isPlayer() == false)
        return true;

    Player* pPlayer = static_cast<Player*>(pAura->GetCaster());
    if (pPlayer->HasQuest(10584) == false)
        return true;

    Creature* magneto = static_cast<Creature*>(pAura->GetTarget());
    if (magneto == nullptr)
        return true;

    LocationVector magnetoPos = magneto->GetPosition();

    Creature* auramagneto = pPlayer->GetMapMgr()->CreateAndSpawnCreature(21731, magnetoPos.x, magnetoPos.y, magnetoPos.z, magnetoPos.o);
    if (auramagneto != nullptr)
        auramagneto->Despawn(4 * 60 * 1000, 0);

    magneto->Despawn(1, 0);

    return true;
}

bool TemporalPhaseModulator(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    if (pPlayer->HasQuest(10609) == false)
        return true;

    Creature* whelp = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 20021);
    if (whelp != nullptr)
    {
        LocationVector whelpPos = whelp->GetPosition();
        if (Util::getRandomUInt(1) == 0)
        {
            Creature* adolescent = pPlayer->GetMapMgr()->CreateAndSpawnCreature(21817, whelpPos.x, whelpPos.y, whelpPos.z, whelpPos.o);
            adolescent->Despawn(5 * 60 * 1000, 0);
        }
        else
        {
            Creature* proto = pPlayer->GetMapMgr()->CreateAndSpawnCreature(21821, whelpPos.x, whelpPos.y, whelpPos.z, whelpPos.o);
            proto->Despawn(5 * 60 * 1000, 0);
        }
        whelp->Despawn(1, 0);
        return true;
    }

    whelp = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 21817);
    if (whelp != nullptr)
    {
        LocationVector whelpPos = whelp->GetPosition();
        if (Util::getRandomUInt(10) < 8)
        {
            Creature* mature = pPlayer->GetMapMgr()->CreateAndSpawnCreature(21820, whelpPos.x, whelpPos.y, whelpPos.z, whelpPos.o);
            mature->Despawn(5 * 60 * 1000, 0);
        }
        else
        {
            Creature* funnyd = pPlayer->GetMapMgr()->CreateAndSpawnCreature(21823, whelpPos.x, whelpPos.y, whelpPos.z, whelpPos.o);
            funnyd->Despawn(5 * 60 * 1000, 0);
        }
        whelp->Despawn(1, 0);
        return true;
    }

    whelp = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 21821);
    if (whelp != nullptr)
    {
        LocationVector whelpPos = whelp->GetPosition();
        if (Util::getRandomUInt(10) < 8)
        {
            Creature* mature = pPlayer->GetMapMgr()->CreateAndSpawnCreature(21820, whelpPos.x, whelpPos.y, whelpPos.z, whelpPos.o);
            mature->Despawn(5 * 60 * 1000, 0);
        }
        else
        {
            Creature* funnyd = pPlayer->GetMapMgr()->CreateAndSpawnCreature(21823, whelpPos.x, whelpPos.y, whelpPos.z, whelpPos.o);
            funnyd->Despawn(5 * 60 * 1000, 0);
        }
        whelp->Despawn(1, 0);
        return true;
    }
    whelp = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 21823);
    if (whelp != nullptr)
    {
        LocationVector whelpPos = whelp->GetPosition();
        if (Util::getRandomUInt(1) == 0)
        {
            Creature* adolescent = pPlayer->GetMapMgr()->CreateAndSpawnCreature(21817, whelpPos.x, whelpPos.y, whelpPos.z, whelpPos.o);
            adolescent->Despawn(5 * 60 * 1000, 0);
        }
        else
        {
            Creature* proto = pPlayer->GetMapMgr()->CreateAndSpawnCreature(21821, whelpPos.x, whelpPos.y, whelpPos.z, whelpPos.o);
            proto->Despawn(5 * 60 * 1000, 0);
        }
        whelp->Despawn(1, 0);
        return true;
    }

    return true;
}

bool EmblazonRuneblade(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
    {
        return true;
    }

    Player* pPlayer = pSpell->p_caster;
    pPlayer->SendChatMessageToPlayer(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, "Player check", pPlayer);

    QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(12619);
    if (qle == nullptr)
    {
        return true;
    }

    pPlayer->getItemInterface()->AddItemById(38631, 1, 0);
    pPlayer->getItemInterface()->RemoveItemAmt(38607, 1);
    return true;
}

bool WyrmcallersHorn(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;

    LocationVector pos = plr->GetPosition();

    Creature* pCreature = plr->GetMapMgr()->CreateAndSpawnCreature(24019, pos.x, pos.y, pos.z, 0);
    if (pCreature == nullptr)
    {
        return true;
    }

    pCreature->Despawn(5 * 60 * 1000, 0);
    return true;
}

bool RaeloraszSpark(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;
    if (plr == nullptr)
        return true;

    LocationVector pos = plr->GetPosition();

    Creature* pCreature = plr->GetMapMgr()->CreateAndSpawnCreature(26237, pos.x, pos.y, pos.z, 0);
    pCreature->Despawn(5 * 60 * 1000, 0);

    QuestLogEntry* qle = plr->GetQuestLogForEntry(11969);
    if (qle == nullptr)
        return true;

    qle->SendQuestComplete();
    return true;
}

bool RuneOfDistortion(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* plr = pSpell->p_caster;
    if (plr == nullptr)
        return true;

    LocationVector pos = plr->GetPosition();

    Creature* pCreature = plr->GetMapMgr()->CreateAndSpawnCreature(32162, pos.x, pos.y, pos.z, 0);
    pCreature->Despawn(5 * 60 * 1000, 0);

    if (plr->HasQuest(13312) == false && plr->HasQuest(13337) == false)
        return true;

    return true;
}

bool GoreBladder(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
    {
        return true;
    }

    Unit* target = pSpell->GetUnitTarget();
    if (target == nullptr || target->getEntry() != 29392 || target->isDead() == false)
        return true;

    static_cast<Creature*>(target)->Despawn(500, 360000);

    Player* pPlayer = pSpell->p_caster;
    pPlayer->AddQuestKill(12810, 0, 0);

    return true;
}

bool PlagueSpray(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Unit* target = pSpell->GetUnitTarget();
    if (!target || target->getEntry() != 23652 || !target->isAlive())
        return true;
    else if (!target || target->getEntry() != 23652 || !target->HasAura(40467))
        return true;

    Player* pPlayer = pSpell->p_caster;

    pPlayer->AddQuestKill(11307, 0, 0);

    return true;
}

bool GoblinWeatherMachine(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    uint32 Weather = 46736 + Util::getRandomUInt(4);

    pSpell->p_caster->castSpell(pSpell->p_caster, sSpellMgr.getSpellInfo(Weather), true);
    return true;
}

bool PurifiedAshes(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Unit* target = pSpell->GetUnitTarget();
    if (!target || target->getEntry() != 26633 || !target->isDead())
        return true;

    Player* pPlayer = pSpell->p_caster;
    int entry;

    if (pPlayer->isTeamHorde())
        entry = 12236;
    else
        entry = 12249;

    pPlayer->AddQuestKill(entry, 0, 0);

    return true;
}

bool DISMEMBER(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->p_caster)
        return true;

    Unit* target = pSpell->GetUnitTarget();
    if (!target || (target->getEntry() != 23657 && target->getEntry() != 23661 && target->getEntry() != 23662 && target->getEntry() != 23663 && target->getEntry() != 23664 && target->getEntry() != 23665 && target->getEntry() != 23666 && target->getEntry() != 23667 && target->getEntry() != 23668 && target->getEntry() != 23669 && target->getEntry() != 23670) || !target->isDead())
        return true;

    static_cast<Creature*>(target)->Despawn(500, 300000);

    Player* pPlayer = pSpell->p_caster;
    int entry;

    if (pPlayer->isTeamHorde())
    {
        entry = 11257;
    }
    else
    {
        entry = 11246;
    }

    pPlayer->AddQuestKill(entry, 0, 0);

    return true;
}

bool CraftyBlaster(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
    {
        return true;
    }

    Unit* target = pSpell->GetUnitTarget();
    if (!target || (target->getEntry() != 25432 && target->getEntry() != 25434) || !target->isAlive())
    {
        return true;
    }

    Player* pPlayer = pSpell->p_caster;

    pPlayer->AddQuestKill(11653, 0, 0);

    return true;
}

bool RagefistTorch(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
    {
        return true;
    }

    Unit* target = pSpell->GetUnitTarget();
    if (!target || (target->getEntry() != 25342 && target->getEntry() != 25343))
    {
        return true;
    }

    static_cast<Creature*>(target)->Despawn(500, 360000);

    Player* pPlayer = pSpell->p_caster;

    pPlayer->AddQuestKill(11593, 0, 0);

    return true;
}

bool SummonShadra(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    LocationVector pos = pSpell->p_caster->GetPosition();
    pSpell->p_caster->GetMapMgr()->GetInterface()->SpawnCreature(2707, pos.x, pos.y, pos.z, pos.o, true, false, 0, 0);

    return true;
}

bool SummonEcheyakee(uint8_t effectIndex, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr || effectIndex != 1)  //Effect_1 = SEND_EVENT,Effect_2 = DUMMY
        return true;

    LocationVector pos = pSpell->p_caster->GetPosition();
    pSpell->p_caster->GetMapMgr()->GetInterface()->SpawnCreature(3475, pos.x, pos.y, pos.z, pos.o, true, false, 0, 0);

    return true;
}

bool HodirsHorn(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Unit* target = pSpell->GetUnitTarget();
    if (!target || (target->getEntry() != 29974 && target->getEntry() != 30144 && target->getEntry() != 30135) || !target->isDead())
        return true;

    static_cast<Creature*>(target)->Despawn(500, 360000);

    Player* pPlayer = pSpell->p_caster;

    pPlayer->AddQuestKill(12977, 0, 0);

    return true;
}

bool TelluricPoultice(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
    {
        return true;
    }

    Unit* target = pSpell->GetUnitTarget();
    if (!target || target->getEntry() != 30035)
    {
        return true;
    }

    static_cast<Creature*>(target)->Despawn(500, 360000);

    Player* pPlayer = pSpell->p_caster;

    pPlayer->AddQuestKill(12937, 0, 0);

    return true;
}

bool Screwdriver(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
    {
        return true;
    }

    Unit* target = pSpell->GetUnitTarget();
    if (!target || target->getEntry() != 25753 || !target->isDead())
    {
        return true;
    }

    static_cast<Creature*>(target)->Despawn(500, 360000);

    Player* pPlayer = pSpell->p_caster;

    pPlayer->AddQuestKill(11730, 0, 0);

    return true;
}

bool IncineratingOil(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
    {
        return true;
    }

    Unit* target = pSpell->GetUnitTarget();
    if (!target || target->getEntry() != 28156)
    {
        return true;
    }

    static_cast<Creature*>(target)->Despawn(500, 360000);

    Player* pPlayer = pSpell->p_caster;

    pPlayer->AddQuestKill(12568, 0, 0);

    return true;
}

bool SummonAquementas(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    LocationVector pos = pSpell->p_caster->GetPosition();

    pSpell->p_caster->GetMapMgr()->GetInterface()->SpawnCreature(9453, pos.x, pos.y, pos.z, pos.o, true, false, 0, 0);

    return true;
}

bool PrayerBeads(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Unit* target = pSpell->GetUnitTarget();
    if (!target || target->getEntry() != 22431)
        return true;

    Player* pPlayer = pSpell->p_caster;

    pPlayer->AddQuestKill(10935, 0, 0);

    return true;
}

bool CleansingVialDND(uint32 /*i*/, Spell* s)
{
    QuestLogEntry* en = s->p_caster->GetQuestLogForEntry(9427);
    if (en == nullptr)
        return true;

    en->SendQuestComplete();

    return true;
}

bool HunterTamingQuest(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* m_target = a->GetTarget();
    Player* p_caster = a->GetPlayerCaster();

    if (p_caster == nullptr)
        return true;

    if (apply)
    {
        m_target->GetAIInterface()->AttackReaction(a->GetUnitCaster(), 10, 0);
    }
    else
    {
        uint32 TamingSpellid = a->GetSpellInfo()->getEffectMiscValue(1);

        SpellInfo const* triggerspell = sSpellMgr.getSpellInfo(TamingSpellid);
        if (triggerspell == NULL)
        {
            DLLLogDetail("An Aura with spellid %u is calling HunterTamingQuest() with an invalid TamingSpellid: %u", a->GetSpellId(), TamingSpellid);
            return true;
        }

        QuestProperties const* tamequest = sMySQLStore.getQuestProperties(triggerspell->getEffectMiscValue(1));
        if (tamequest == NULL)
        {
            DLLLogDetail("An Aura with spellid %u is calling HunterTamingQuest() with an invalid tamequest id: %u", a->GetSpellId(), triggerspell->getEffectMiscValue(1));
            return true;
        }

        if (!p_caster->HasQuest(tamequest->id))
        {
            p_caster->sendCastFailedPacket(triggerspell->getId(), SPELL_FAILED_BAD_TARGETS, 0, 0);
        }
        else if (!a->GetTimeLeft())
        {
            // Creates a 15 minute pet, if player has the quest that goes with the spell and if target corresponds to quest
            //\todo you can't do that here. SpellHandler load will fail on *nix systemy
            /*if (Rand(75.0f))    // 75% chance on success
            {

                if (m_target->isCreature())
                {
                    Creature* tamed = static_cast<Creature*>(m_target);
                    tamed->GetAIInterface()->HandleEvent(EVENT_LEAVECOMBAT, p_caster, 0);

                    Pet* pPet = objmgr.CreatePet(tamed->getEntry());
                    if (!pPet->CreateAsSummon(tamed->getEntry(), tamed->GetCreatureProperties(), tamed, p_caster, triggerspell, 2, 900000))
                    {
                        pPet->DeleteMe();//CreateAsSummon() returns false if an error occurred.
                        pPet = NULL;
                    }

                    tamed->Despawn(1, 0); //we despawn the tamed creature once we are out of Aura::Remove()

                    QuestLogEntry* qle = p_caster->GetQuestLogForEntry(tamequest->id);
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
                p_caster->sendCastFailedPacket(triggerspell->getId(), SPELL_FAILED_TRY_AGAIN, 0, 0);
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

    LocationVector pos = plr->GetPosition();

    QuestLogEntry* pQuest = plr->GetQuestLogForEntry(13149);
    if (pQuest == nullptr)
        return true;

    GameObject* crate = plr->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(pos.x, pos.y, pos.z, 190094);
    if (crate != nullptr)
    {
        GameObject* go = plr->GetMapMgr()->CreateGameObject(190095);
        go->CreateFromProto(190095, crate->GetMapMgr()->GetMapId(), pos.x, pos.y, pos.z, pos.o, 0.0f, 0.0f, 0.0f, 0.0f);
        go->PushToWorld(crate->GetMapMgr());
        crate->Despawn(0, 0);
        pQuest->SetMobCount(0, pQuest->GetMobCount(0) + 1);
        pQuest->SendUpdateAddKill(0);
        pQuest->UpdatePlayerFields();

        if (pQuest->GetMobCount(0) == 5)
        {
            //weee, Uther
            CreatureProperties const* cp = sMySQLStore.getCreatureProperties(26528);
            if (cp != nullptr)
            {
                Creature* c = plr->GetMapMgr()->CreateCreature(26528);
                if (c != nullptr)
                {
                    //position is guessed
                    c->Load(cp, 1759.4351f, 1265.3317f, 138.052f, 0.1902f);
                    c->PushToWorld(plr->GetMapMgr());
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

    LocationVector pos = pPlayer->GetPosition();

    Creature* pJovaanCheck = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-3310.743896f, 2951.929199f, 171.132538f, 21633);
    if (pJovaanCheck != nullptr)
        return true;

    if (apply)
    {

        pPlayer->setMoveRoot(true);
        Creature* pJovaan = pPlayer->GetMapMgr()->CreateAndSpawnCreature(21633, -3310.743896f, 2951.929199f, 171.132538f, 5.054039f);    // Spawn Jovaan
        if (pJovaan != nullptr)
        {
            pJovaan->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
            if (pJovaan->GetAIInterface() != nullptr)
            {
                pJovaan->GetAIInterface()->SetAllowedToEnterCombat(false);
            }
        }
        GameObject* pGameObject = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(pos.x, pos.y, pos.z, 184834);
        if (pGameObject != nullptr)
        {
            pGameObject->Despawn(60000, 0);
            pPlayer->UpdateNearbyGameObjects();
        }
    }
    else
    {
        if (pPlayer->isTeamAlliance())
            pPlayer->AddQuestKill(10563, 2, 0);
        else
            pPlayer->AddQuestKill(10596, 2, 0);

        pPlayer->setMoveRoot(false);
    }

    return true;
}

bool CenarionMoondust(uint8_t /*effectIndex*/, Spell* pSpell) // Body And Heart (Alliance)
{
    if (!pSpell->p_caster)
        return true;

    if (!pSpell->p_caster->IsInWorld())
        return true;

    const float pos[] = { 6335.2329f, 144.0811f, 24.0068f, 5.701f }; // x, y, z, o
    Player* p_caster = pSpell->p_caster;

    //Moonkin Stone aura
    p_caster->GetMapMgr()->CreateAndSpawnGameObject(177644, 6331.01f, 88.245f, 22.6522f, 2.01455f, 1.0);

    // it dont delete lunaclaw if he is here
    Creature* lunaclaw = p_caster->GetMapMgr()->CreateAndSpawnCreature(12138, pos[0], pos[1], pos[2], pos[3]);

    if (lunaclaw == nullptr)
        return true;

    lunaclaw->CreateCustomWaypointMap();
    uint32 md = lunaclaw->getDisplayId();

    //Waypoints
    lunaclaw->LoadCustomWaypoint(6348.3833f, 132.5197f, 21.6042f, 4.19f, 200, Movement::WP_MOVE_TYPE_RUN, false, 0, false, 0, md, md);
    //make sure that player dont cheat speed or something
    if (lunaclaw->GetDistance2dSq(p_caster) < 200)   // can be more? - he can speed hack or teleport hack
    {
        LocationVector casterPos = p_caster->GetPosition();
        lunaclaw->LoadCustomWaypoint(casterPos.x, casterPos.y, casterPos.z, casterPos.o + 3, 200, Movement::WP_MOVE_TYPE_RUN, false, 0, false, 0, md, md);
    }
    else
    {
        lunaclaw->LoadCustomWaypoint(5328.2148f, 94.5505f, 21.4547f, 4.2489f, 200, Movement::WP_MOVE_TYPE_RUN, false, 0, false, 0, md, md);
    }

    lunaclaw->SwitchToCustomWaypoints();

    // Make sure that creature will attack player
    if (!lunaclaw->CombatStatus.IsInCombat())
    {
        lunaclaw->GetAIInterface()->setNextTarget(p_caster);
    }

    return true;
}

bool CenarionLunardust(uint8_t /*effectIndex*/, Spell* pSpell)  // Body And Heart (Horde)
{
    if (!pSpell->p_caster)
        return true;

    if (!pSpell->p_caster->IsInWorld())
        return true;

    const float pos[] = { -2443.9711f, -1642.8002f, 92.5129f, 1.71f }; // x, y, z, o
    Player* p_caster = pSpell->p_caster;

    //Moonkin Stone aura
    p_caster->GetMapMgr()->CreateAndSpawnGameObject(177644, -2499.54f, -1633.03f, 91.8121f, 0.262894f, 1.0);

    Creature* lunaclaw = p_caster->GetMapMgr()->CreateAndSpawnCreature(12138, pos[0], pos[1], pos[2], pos[3]);
    if (lunaclaw == nullptr)
        return true;

    lunaclaw->CreateCustomWaypointMap();
    uint32 md = lunaclaw->getDisplayId();

    // Waypoints
    lunaclaw->LoadCustomWaypoint(-2448.2253f, -1625.0148f, 91.89f, 1.913f, 200, Movement::WP_MOVE_TYPE_RUN, false, 0, false, 0, md, md);
    //make sure that player dont cheat speed or something
    if (lunaclaw->GetDistance2dSq(p_caster) < 200)   // can be more? - he can speed hack or teleport hack
    {
        LocationVector targetPos = p_caster->GetPosition();
        lunaclaw->LoadCustomWaypoint(targetPos.x, targetPos.y, targetPos.z, targetPos.o + 3, 200, Movement::WP_MOVE_TYPE_RUN, false, 0, false, 0, md, md);
    }
    else
    {
        lunaclaw->LoadCustomWaypoint(-2504.2641f, -1630.7354f, 91.93f, 3.2f, 200, Movement::WP_MOVE_TYPE_RUN, false, 0, false, 0, md, md);
    }

    lunaclaw->SwitchToCustomWaypoints();

    // Make sure that creature will attack player
    if (!lunaclaw->CombatStatus.IsInCombat())
    {
        lunaclaw->GetAIInterface()->setNextTarget(p_caster);
    }

    return true;
}

bool CurativeAnimalSalve(uint8_t /*effectIndex*/, Spell* pSpell) // Curing the Sick
{
    Player* caster = pSpell->p_caster;
    if (caster == NULL)
        return true;

    if (!pSpell->GetUnitTarget()->isCreature())
        return true;

    Creature* target = static_cast<Creature*>(pSpell->GetUnitTarget());

    LocationVector targetPos = target->GetPosition();

    uint32 entry = target->getEntry();
    if (entry == 12296 || entry == 12298)
    {
        caster->AddQuestKill(6129, 0, 0);
        caster->AddQuestKill(6124, 0, 0);

        if (entry == 12298) // Sickly Deer
        {
            Creature* deer = caster->GetMapMgr()->CreateAndSpawnCreature(12298, targetPos.x, targetPos.y, targetPos.z, targetPos.o); // Cured Deer
            if (deer != nullptr)
                deer->Despawn(2 * 60 * 1000, 0);
        }
        else // Sickly Gazelle
        {
            Creature* gazelle = caster->GetMapMgr()->CreateAndSpawnCreature(12297, targetPos.x, targetPos.y, targetPos.z, targetPos.o); // Cured Gazelle
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
    if (pSpell->p_caster == NULL)
        return true;

    Player* pPlayer = pSpell->p_caster;

    pPlayer->AddQuestKill(28, 0, 0);
    pPlayer->AddQuestKill(29, 0, 0);

    return true;
}

bool SymbolOfLife(uint8_t /*effectIndex*/, Spell* pSpell) // Alliance ress. quests
{
    Player* plr = pSpell->p_caster;
    if (plr == nullptr)
        return true;

    WoWGuid wowGuid;
    wowGuid.Init(plr->GetSelection());

    Creature* target = plr->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());

    if (target == nullptr)
        return true;

    const uint32 targets[] = { 17542, 6177, 6172 };
    const uint32 quests[] = { 9600, 1783, 1786 };

    bool questOk = false;
    bool targetOk = false;

    for (uint8 j = 0; j < 3; j++)
    {
        if (target->getEntry() == targets[j])
        {
            targetOk = true;
            break;
        }
    }

    if (!targetOk)
        return true;

    QuestLogEntry* quest_entry;

    for (uint8 j = 0; j < 3; j++)
    {
        if (plr->HasQuest(quests[j]))
        {
            quest_entry = plr->GetQuestLogForEntry(quests[j]);
            if (quest_entry != nullptr)
                questOk = true;

            break;
        }
    }

    if (!questOk)
        return true;

    target->setStandState(STANDSTATE_STAND);
    target->setDeathState(ALIVE);

    target->Despawn(10 * 1000, 1 * 60 * 1000);

    quest_entry->SetMobCount(0, 1);
    quest_entry->SendUpdateAddKill(0);
    quest_entry->UpdatePlayerFields();

    return true;
}

bool FilledShimmeringVessel(uint8_t /*effectIndex*/, Spell* pSpell) // Blood Elf ress. quest
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;

    WoWGuid wowGuid;
    wowGuid.Init(plr->GetSelection());

    Creature* target = plr->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
    if (target == nullptr)
        return true;

    if (target->getEntry() != 17768)
        return true;

    QuestLogEntry* qle = plr->GetQuestLogForEntry(9685);
    if (qle == nullptr)
        return true;

    target->setStandState(STANDSTATE_STAND);
    target->setDeathState(ALIVE);

    target->Despawn(30 * 1000, 1 * 60 * 1000);

    qle->SetMobCount(0, 1);
    qle->SendUpdateAddKill(0);
    qle->UpdatePlayerFields();

    return true;
}

bool DouseEternalFlame(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;

    QuestLogEntry* qle = plr->GetQuestLogForEntry(9737);
    if (qle == nullptr)
        return true;

    LocationVector pos = plr->GetPosition();

    GameObject* Flame = plr->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(3678, -3640, 139, 182068);
    if (Flame != nullptr)
    {
        if (plr->CalcDistance(plr, Flame) < 30)
        {
            plr->AddQuestKill(9737, 0, 0);

            Creature* pCreature = plr->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 10917);
            if (pCreature != nullptr)
            {
                pCreature->SetFaction(11);
            }
        }
    }
    return true;
}

bool Triage(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->p_caster || pSpell->GetUnitTarget() == nullptr)
        return true;

    pSpell->p_caster->castSpell(pSpell->GetUnitTarget(), sSpellMgr.getSpellInfo(746), true);

    pSpell->p_caster->AddQuestKill(6624, 0, 0);

    return true;
}

bool NeutralizingTheCauldrons(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr || !pSpell->p_caster->IsInWorld())
        return true;

    Player* pPlayer = pSpell->p_caster;

    LocationVector pos = pPlayer->GetPosition();

    QuestLogEntry* pQuest = pPlayer->GetQuestLogForEntry(11647);
    if (pQuest == nullptr)
        return true;

    GameObject* pCauldron = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(pos.x, pos.y, pos.z, 187690);
    if (pCauldron == nullptr)
        return true;

    float posX = pCauldron->GetPositionX();

    if (posX == 3747.07f)
    {
        pSpell->p_caster->AddQuestKill(11647, 0, 0);
    }

    if (posX == 4023.5f)
    {
        pSpell->p_caster->AddQuestKill(11647, 1, 0);
    }

    if (posX == 4126.12f)
    {
        pSpell->p_caster->AddQuestKill(11647, 2, 0);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Stop the Plague
bool HighmessasCleansingSeeds(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr || !pSpell->p_caster->IsInWorld())
        return true;

    Player* pPlayer = pSpell->p_caster;

    pPlayer->AddQuestKill(11677, 0, 0);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// There's Something Going On In Those Caves
bool BixiesInhibitingPowder(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr || !pSpell->p_caster->IsInWorld())
        return true;

    Player* pPlayer = pSpell->p_caster;

    pPlayer->AddQuestKill(11694, 0, 0);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Leading the Ancestors Home
bool CompleteAncestorRitual(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr || !pSpell->p_caster->IsInWorld())
        return true;

    Player* pPlayer = pSpell->p_caster;
    QuestLogEntry* pQuest = pPlayer->GetQuestLogForEntry(11610);
    if (!pQuest)
        return true;

    LocationVector pos = pPlayer->GetPosition();

    GameObject* pElderObj = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(pos.x, pos.y, pos.z, 191088);
    if (pElderObj != nullptr && pPlayer->GetDistance2dSq(pElderObj) < 8.0f)
    {
        pPlayer->AddQuestKill(11610, 0, 0);
    }

    pElderObj = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(pos.x, pos.y, pos.z, 191089);
    if (pElderObj != nullptr && pPlayer->GetDistance2dSq(pElderObj) < 8.0f)
    {
        pPlayer->AddQuestKill(11610, 1, 0);
    }

    pElderObj = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(pos.x, pos.y, pos.z, 191090);
    if (pElderObj != nullptr && pPlayer->GetDistance2dSq(pElderObj) < 8.0f)
    {
        pPlayer->AddQuestKill(11610, 2, 0);
    }

    return true;
}

bool PoweringOurDefenses(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;

    plr->AddQuestKill(8490, 0, 0);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Testing the Antidote
bool TestingTheAntidote(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->GetUnitTarget() || !pSpell->GetUnitTarget()->isCreature())
        return true;

    Creature* target = static_cast<Creature*>(pSpell->GetUnitTarget());
    if (target == nullptr || target->getEntry() != 16880) // Hulking Helboar
        return true;

    LocationVector targetPos = target->GetPosition();

    Creature* spawned = target->GetMapMgr()->GetInterface()->SpawnCreature(16992, targetPos.x, targetPos.y, targetPos.z, targetPos.o, true, false, 0, 0);
    if (spawned == nullptr)
        return true;

    target->Despawn(0, 300000);

    spawned->GetAIInterface()->setNextTarget(pSpell->u_caster);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Zeth'Gor Must Burn!
bool ZethGorMustBurnHorde(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* pQuest = pPlayer->GetQuestLogForEntry(10792);
    if (pQuest != nullptr)
    {
        // Barracks
        if (pQuest->GetMobCount(0) < pQuest->GetQuest()->required_mob_or_go_count[0])
        {
            GameObject* pBarracks = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-1137.0f, 1970.0f, 74.0f, 300151);
            if (pBarracks != nullptr && pPlayer->CalcDistance(pPlayer, pBarracks) < 30)
            {
                pPlayer->AddQuestKill(10792, 0, 0);

                GameObject* pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -1129.08f, 1921.77f, 94.0074f, 0, 4);
                if (pGameobject != nullptr)
                    pGameobject->Despawn(1 * 60 * 1000, 0);

                pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -1135.00f, 1944.05f, 84.7084f, 0, 4);
                if (pGameobject != nullptr)
                    pGameobject->Despawn(1 * 60 * 1000, 0);

                pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -1152.01f, 1945.00f, 102.901f, 0, 4);
                if (pGameobject != nullptr)
                    pGameobject->Despawn(1 * 60 * 1000, 0);

                pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -1159.60f, 1958.76f, 83.0412f, 0, 4);
                if (pGameobject != nullptr)
                    pGameobject->Despawn(1 * 60 * 1000, 0);

                pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -1126.17f, 1880.96f, 95.065f, 0, 4);
                if (pGameobject != nullptr)
                    pGameobject->Despawn(1 * 60 * 1000, 0);

                pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -1185.79f, 1968.29f, 90.931f, 0, 4);
                if (pGameobject != nullptr)
                    pGameobject->Despawn(1 * 60 * 1000, 0);

                return true;
            }
        }

        // Eastern Hovel
        if (pQuest->GetMobCount(1) < pQuest->GetQuest()->required_mob_or_go_count[1])
        {
            GameObject* pEasternHovel = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-940.0f, 1920.0f, 69.0f, 300151);
            if (pEasternHovel != nullptr && pPlayer->CalcDistance(pPlayer, pEasternHovel) < 30)
            {
                pPlayer->AddQuestKill(10792, 1, 0);

                GameObject* pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -938.034f, 1924.153f, 73.590f, 0, 4);
                if (pGameobject != nullptr)
                    pGameobject->Despawn(1 * 60 * 1000, 0);

                return true;
            }
        }

        // Western Hovel
        if (pQuest->GetMobCount(2) < pQuest->GetQuest()->required_mob_or_go_count[2])
        {
            GameObject* pWesternHovel = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-1155.0f, 2061.0f, 68.0f, 300151);
            if (pWesternHovel != nullptr && pPlayer->CalcDistance(pPlayer, pWesternHovel) < 30)
            {
                pPlayer->AddQuestKill(10792, 2, 0);

                GameObject* pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -1152.10f, 2066.20f, 72.959f, 0, 4);
                if (pGameobject != nullptr)
                    pGameobject->Despawn(1 * 60 * 1000, 0);

                return true;
            }
        }

        // Stable
        if (pQuest->GetMobCount(3) < pQuest->GetQuest()->required_mob_or_go_count[3])
        {
            GameObject* pStable = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-1052.0f, 2007.0f, 66.0f, 300151);
            if (pStable != nullptr && pPlayer->CalcDistance(pPlayer, pStable) < 30)
            {
                pPlayer->AddQuestKill(10792, 3, 0);

                GameObject* pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -1058.85f, 2010.95f, 68.776f, 0, 4);
                if (pGameobject != nullptr)
                    pGameobject->Despawn(1 * 60 * 1000, 0);

                return true;
            }
        }
    }
    else
    {
        pPlayer->BroadcastMessage("Missing required quest : Zeth'Gor Must Burn");
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Laying Waste to the Unwanted
bool LayingWasteToTheUnwantedAlliance(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* pQuest = pPlayer->GetQuestLogForEntry(10078);
    if (pQuest != nullptr)
    {
        // Eastern Thrower
        if (pQuest->GetMobCount(0) < pQuest->GetQuest()->required_mob_or_go_count[0])
        {
            GameObject* pEasternTower = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-155.0f, 2517.0f, 43.0f, 300152);
            if (pEasternTower != nullptr && pPlayer->CalcDistance(pPlayer, pEasternTower) < 30)
            {
                pPlayer->AddQuestKill(10078, 0, 0);

                GameObject* pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -157.916f, 2517.71f, 58.5508f, 0, 4);
                if (pGameobject != nullptr)
                    pGameobject->Despawn(1 * 60 * 1000, 0);

                return true;
            }
        }

        // Central Eastern Thrower
        if (pQuest->GetMobCount(1) < pQuest->GetQuest()->required_mob_or_go_count[1])
        {
            GameObject* pCentralEasternTower = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-152.0f, 2661.0f, 44.0f, 300152);
            if (pCentralEasternTower != nullptr && pPlayer->CalcDistance(pPlayer, pCentralEasternTower) < 30)
            {
                pPlayer->AddQuestKill(10078, 1, 0);

                GameObject* pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -152.527f, 2661.99f, 60.8123f, 0, 4);
                if (pGameobject != nullptr)
                    pGameobject->Despawn(1 * 60 * 1000, 0);

                return true;
            }
        }

        // Central Western Thrower
        if (pQuest->GetMobCount(2) < pQuest->GetQuest()->required_mob_or_go_count[2])
        {
            GameObject* pCentralWesternTower = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-174.0f, 2772.0f, 32.0f, 300152);
            if (pCentralWesternTower != nullptr && pPlayer->CalcDistance(pPlayer, pCentralWesternTower) < 30)
            {
                pPlayer->AddQuestKill(10078, 2, 0);

                GameObject* pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -177.916f, 2773.75f, 48.636f, 0, 4);
                if (pGameobject != nullptr)
                    pGameobject->Despawn(1 * 60 * 1000, 0);

                return true;
            }
        }

        // Western Thrower
        if (pQuest->GetMobCount(3) < pQuest->GetQuest()->required_mob_or_go_count[3])
        {
            GameObject* pWesternTower = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-166.0f, 2818.0f, 29.0f, 300152);
            if (pWesternTower != nullptr && pPlayer->CalcDistance(pPlayer, pWesternTower) < 30)
            {
                pPlayer->AddQuestKill(10078, 3, 0);

                GameObject* pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -166.0f, 2818.0f, 29.0f, 0, 4);
                if (pGameobject != nullptr)
                    pGameobject->Despawn(1 * 60 * 1000, 0);

                return true;
            }
        }
    }
    else
    {
        pPlayer->BroadcastMessage("Missing required quest : Laying Waste to the Unwanted");
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Burn It Up... For the Horde!
bool BurnItUp(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    QuestLogEntry* pQuest = pPlayer->GetQuestLogForEntry(10087);
    if (pQuest == nullptr)
        return true;

    if (pQuest->GetMobCount(0) < pQuest->GetQuest()->required_mob_or_go_count[0])
    {
        GameObject* pEastern = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-300.0f, 2407.0f, 50.0f, 183122);
        if (pEastern == nullptr)
            pEastern = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-300.0f, 2407.0f, 50.0f, 185122);

        if (pEastern != nullptr && pPlayer->CalcDistance(pPlayer, pEastern) < 30)
        {
            pPlayer->AddQuestKill(10087, 0, 0);

            GameObject* pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -300.0f, 2407.0f, 50.0f, 0, 4);
            if (pGameobject != nullptr)
                pGameobject->Despawn(1 * 60 * 1000, 0);

            return true;
        }
    }

    if (pQuest->GetMobCount(1) < pQuest->GetQuest()->required_mob_or_go_count[1])
    {
        GameObject* pWestern = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-350.0f, 2708.0f, 35.0f, 183122);
        if (pWestern == nullptr)
            pWestern = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-350.0f, 2708.0f, 35.0f, 185122);

        if (pWestern != nullptr && pPlayer->CalcDistance(pPlayer, pWestern) < 30)
        {
            pPlayer->AddQuestKill(10087, 1, 0);

            GameObject* pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -350.0f, 2708.0f, 35.0f, 0, 4);
            if (pGameobject != nullptr)
                pGameobject->Despawn(1 * 60 * 1000, 0);

            return true;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// The Seer's Relic
bool TheSeersRelic(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;
    QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(9545);
    if (qle == nullptr || qle->GetMobCount(0) >= qle->GetQuest()->required_mob_or_go_count[0])
        return true;

    WoWGuid wowGuid;
    wowGuid.Init(pPlayer->GetSelection());

    Creature* pTarget = pPlayer->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
    if (pTarget == nullptr)
        return true;

    if (pTarget->getEntry() != 16852)
        return true;

    pTarget->setStandState(STANDSTATE_STAND);
    pTarget->setDeathState(ALIVE);
    pTarget->Despawn(30 * 1000, 1 * 60 * 1000);

    pPlayer->AddQuestKill(9545, 0, 0);

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Disrupt Their Reinforcements
bool DisruptTheirReinforcements(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;
    QuestLogEntry* pQuestA = pPlayer->GetQuestLogForEntry(10144);
    QuestLogEntry* pQuestH = pPlayer->GetQuestLogForEntry(10208);

    if (pQuestA != nullptr)
    {
        bool SendMsg = false;
        if (pQuestA->GetMobCount(0) < pQuestA->GetQuest()->required_mob_or_go_count[0])
        {
            GameObject* pGrimh = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-419.0f, 1847.0f, 80.0f, 184414);
            if (pGrimh != NULL && pPlayer->CalcDistance(pPlayer, pGrimh) < 10)
            {
                pPlayer->AddQuestKill(10144, 0, 0);
            }
            else
            {
                SendMsg = true;
            }
        }

        if (pQuestA->GetMobCount(1) < pQuestA->GetQuest()->required_mob_or_go_count[1])
        {
            GameObject* pKaalez = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-548.0f, 1782.0f, 58.0f, 184415);
            if (pKaalez != nullptr && pPlayer->CalcDistance(pPlayer, pKaalez) < 10)
            {
                pPlayer->AddQuestKill(10144, 1, 0);
            }
            else
            {
                SendMsg = true;
            }
        }

        if (SendMsg)
        {
            pPlayer->BroadcastMessage("Go to the Port of the Dark Legion!");
        }
    }
    else if (pQuestH != nullptr)
    {
        bool SendMsg = false;
        if (pQuestH->GetMobCount(0) < pQuestH->GetQuest()->required_mob_or_go_count[0])
        {
            GameObject* pXilus = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-85.0f, 1880.0f, 74.0f, 184290);
            if (pXilus != nullptr && pPlayer->CalcDistance(pPlayer, pXilus) < 10)
            {
                pPlayer->AddQuestKill(10208, 0, 0);
            }
            else
            {
                SendMsg = true;
            }
        }
        if (pQuestH->GetMobCount(1) < pQuestH->GetQuest()->required_mob_or_go_count[1])
        {
            GameObject* pKruul = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(147.0f, 1717.0f, 38.0f, 184289);
            if (pKruul != nullptr && pPlayer->CalcDistance(pPlayer, pKruul) < 10)
            {
                pPlayer->AddQuestKill(10208, 1, 0);
            }
            else
            {
                SendMsg = true;
            }
        }

        if (SendMsg)
        {
            pPlayer->BroadcastMessage("Go to the Port of the Dark Legion!");
        }
    }
    else
    {
        pPlayer->BroadcastMessage("Missing required quest : Disrupt Their Reinforcements");
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
//Arzeth's Demise
bool FuryOfTheDreghoodElders(uint32 /*i*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    Unit* pUnit = pSpell->GetUnitTarget();
    if (pUnit == nullptr || !pUnit->isCreature() || pUnit->getEntry() != 19354)
        return true;

    LocationVector targetPos = pUnit->GetPosition();

    Creature* elder = pPlayer->GetMapMgr()->CreateAndSpawnCreature(20680, targetPos.x, targetPos.y, targetPos.z, targetPos.o);
    if (elder != nullptr)
        elder->Despawn(5 * 60 * 1000, 0);

    static_cast<Creature*>(pUnit)->Despawn(0, 3 * 60 * 1000);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// War is Hell
bool WarIsHell(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;

    LocationVector pos = plr->GetPosition();

    Creature* target = plr->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 24008);
    if (target == nullptr)
        return true;

    QuestLogEntry* qle = plr->GetQuestLogForEntry(11270);
    if (qle == nullptr)
        return true;

    plr->AddQuestKill(11270, 0, 0);

    LocationVector targetPos = target->GetPosition();

    GameObject* obj = plr->GetMapMgr()->CreateAndSpawnGameObject(183816, targetPos.x, targetPos.y, targetPos.z, targetPos.o, 1);
    if (obj != nullptr)
        obj->Despawn(1 * 60 * 1000, 0);

    target->Despawn(2000, 60 * 1000);
    plr->UpdateNearbyGameObjects();

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// A Lesson in Fear
bool PlantForsakenBanner(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    QuestLogEntry* pQuest = pPlayer->GetQuestLogForEntry(11282);
    if (pQuest == nullptr)
        return true;

    Creature* target = static_cast<Creature*>(pSpell->GetUnitTarget());
    if (target == nullptr || target->isAlive())
        return true;

    uint32 cit = target->getEntry();
    switch (cit)
    {
        case 24161:
        {
            pPlayer->AddQuestKill(11282, 0, 0);

            target->Despawn(0, 3 * 60 * 1000);
        } break;
        case 24016:
        {
            pPlayer->AddQuestKill(11282, 1, 0);

            target->Despawn(0, 3 * 60 * 1000);
        } break;
        case 24162:
        {
            pPlayer->AddQuestKill(11282, 2, 0);

            target->Despawn(0, 3 * 60 * 1000);
        } break;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Erratic Behavior
bool ConvertingSentry(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pCaster = pSpell->p_caster;
    if (pCaster == nullptr)
        return true;

    Creature* pTarget = static_cast<Creature*>(pSpell->GetUnitTarget());
    if (pTarget == nullptr || pTarget->getEntry() != 24972 || pTarget->isAlive())   // Erratic Sentry: 24972
        return true;

    QuestLogEntry* qle = pCaster->GetQuestLogForEntry(11525);
    if (qle == nullptr)
    {
        qle = pCaster->GetQuestLogForEntry(11524);
        if (qle == nullptr)
            return true;
    }

    if (qle->GetMobCount(0) == qle->GetQuest()->required_mob_or_go_count[0])
        return true;

    pCaster->AddQuestKill(11525, 0, 0);
    pCaster->AddQuestKill(11524, 0, 0);

    pTarget->Despawn(500, 2 * 60 * 1000);

    return true;
}

bool OrbOfMurlocControl(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    QuestLogEntry* pQuest = pPlayer->GetQuestLogForEntry(11541);
    if (pQuest == nullptr)
        return true;

    Creature* pTarget;

    for (const auto& itr : pSpell->m_caster->getInRangeObjectsSet())
    {
        if (itr && itr->isCreatureOrPlayer() && static_cast<Unit*>(itr)->isCreature())
            pTarget = static_cast<Creature*>(itr);
        else
            continue;

        if (pSpell->m_caster->CalcDistance(pTarget) > 5)
            continue;

        if (pTarget->getEntry() == 25084)
        {
            LocationVector targetPos = pTarget->GetPosition();

            if (pQuest->GetMobCount(0) < pQuest->GetQuest()->required_mob_or_go_count[0])
            {
                pPlayer->AddQuestKill(11541, 0, 0);

                Creature* FreedGreengill = pPlayer->GetMapMgr()->CreateAndSpawnCreature(25085, targetPos.x, targetPos.y, targetPos.z, targetPos.o);
                if (FreedGreengill != nullptr)
                    FreedGreengill->Despawn(6 * 60 * 1000, 0);
                pTarget->Despawn(0, 6 * 60 * 1000);

                return true;
            }
        }
    }
    return true;
}

bool ShipBombing(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(11542);
    if (qle == nullptr)
    {
        qle = pPlayer->GetQuestLogForEntry(11543);
        if (qle == nullptr)
        {
            return true;
        }
    }

    GameObject* pSinloren = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(13200.232422f, -7049.176270f, 3.838517f, 550000);
    GameObject* pBloodoath = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(13319.419922f, -6988.779785f, 4.002993f, 550000);
    GameObject* pDawnchaser = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(13274.51625f, -7145.434570f, 4.770292f, 550000);

    GameObject* obj = nullptr;

    if (pSinloren != nullptr)
    {
        if (qle->GetMobCount(0) < qle->GetQuest()->required_mob_or_go_count[0])
        {
            if (pPlayer->CalcDistance(pPlayer, pSinloren) < 15)
            {
                pPlayer->AddQuestKill(11542, 0, 0);
                pPlayer->AddQuestKill(11543, 0, 0);

                obj = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(GO_FIRE, 13214.3f, -7059.19f, 17.5717f, 1.58573f, 1);
                if (obj != nullptr)
                    obj->Despawn(2 * 60 * 1000, 0);

                obj = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(GO_FIRE, 13204.2f, -7059.38f, 17.5717f, 1.57787f, 1);
                if (obj != nullptr)
                    obj->Despawn(2 * 60 * 1000, 0);

            }
        }
    }
    if (pBloodoath != nullptr)
    {
        if (qle->GetMobCount(1) < qle->GetQuest()->required_mob_or_go_count[1])
        {
            if (pPlayer->CalcDistance(pPlayer, pBloodoath) < 15)
            {
                pPlayer->AddQuestKill(11542, 1, 0);
                pPlayer->AddQuestKill(11543, 1, 0);

                obj = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(GO_FIRE, 13329.4f, -6994.70f, 14.5219f, 1.38938f, 1);
                if (obj != nullptr)
                    obj->Despawn(2 * 60 * 1000, 0);

                obj = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(GO_FIRE, 13315.4f, -6990.72f, 14.7647f, 1.25979f, 1);
                if (obj != nullptr)
                    obj->Despawn(2 * 60 * 1000, 0);

            }
        }
    }
    if (pDawnchaser != nullptr)
    {
        if (pPlayer->CalcDistance(pPlayer, pDawnchaser) < 15)
        {
            if (qle->GetMobCount(2) < qle->GetQuest()->required_mob_or_go_count[2])
            {
                pPlayer->AddQuestKill(11542, 2, 0);
                pPlayer->AddQuestKill(11543, 2, 0);

                obj = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(GO_FIRE, 13284.1f, -7152.65f, 15.9774f, 1.44828f, 1);
                if (obj != nullptr)
                    obj->Despawn(2 * 60 * 1000, 0);

                obj = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(GO_FIRE, 13273.0f, -7151.21f, 15.9774f, 1.39723f, 1);
                if (obj != nullptr)
                    obj->Despawn(2 * 60 * 1000, 0);
            }
        }
    }
    return true;
}

bool ImpaleEmissary(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    LocationVector pos = pPlayer->GetPosition();

    QuestLogEntry* pQuest = pPlayer->GetQuestLogForEntry(11537);
    if (pQuest == nullptr)
    {
        pQuest = pPlayer->GetQuestLogForEntry(11538);
        if (pQuest == nullptr)
            return true;
    }

    Creature* pEmissary = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 25003);
    if (pEmissary == nullptr)
        return true;

    if (pQuest->GetMobCount(0) < pQuest->GetQuest()->required_mob_or_go_count[0])
    {
        pPlayer->AddQuestKill(11537, 0, 0);
        pPlayer->AddQuestKill(11538, 0, 0);

        pEmissary->Despawn(0, 3 * 60 * 1000);
    }
    return true;
}

bool LeyLine(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    LocationVector pos = pPlayer->GetPosition();

    QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(11547);
    if (qle == nullptr)
        return true;

    uint32 portals[] = { 25156, 25154, 25157 };

    for (uint8 i = 0; i < 3; i++)
    {
        Object*  portal = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, portals[i]);
        if (portal != nullptr && qle->GetMobCount(i) < qle->GetQuest()->required_mob_or_go_count[i])
        {
            pPlayer->AddQuestKill(11547, i, 0);

            break;
        }
    }

    return true;
}

bool ManaRemnants(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return false;

    LocationVector pos = pPlayer->GetPosition();

    Creature* Ward = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 40404);
    if (Ward == nullptr)
        return false;

    uint32 quests[] = { 11496, 11523 };
    for (uint8 i = 0; i < 2; i++)
    {
        QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(quests[i]);
        if (qle != nullptr && qle->GetMobCount(0) < qle->GetQuest()->required_mob_or_go_count[0])
        {
            pPlayer->castSpell(Ward, sSpellMgr.getSpellInfo(44981), false);
            pPlayer->setChannelObjectGuid(Ward->getGuid());
            pPlayer->setChannelSpellId(44981);

            pPlayer->AddQuestKill(quests[i], 0, 0);
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Stopping the Spread
bool StoppingTheSpread(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;

    LocationVector pos = plr->GetPosition();

    Creature* target = plr->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 18240);
    if (target == nullptr)
        return true;

    LocationVector targetPos = plr->GetPosition();

    QuestLogEntry* qle = plr->GetQuestLogForEntry(9874);
    if (qle == nullptr)
        return true;

    if (qle && qle->GetMobCount(0) < qle->GetQuest()->required_mob_or_go_count[0])
    {
        qle->SetMobCount(0, qle->GetMobCount(0) + 1);
        qle->SendUpdateAddKill(0);

        GameObject* obj = plr->GetMapMgr()->CreateAndSpawnGameObject(183816, targetPos.x, targetPos.y, targetPos.z, targetPos.o, 1);
        if (obj != nullptr)
            obj->Despawn(1 * 30 * 1000, 0);
    }

    target->Despawn(2000, 60 * 1000);
    plr->UpdateNearbyGameObjects();
    qle->UpdatePlayerFields();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//Ruthless Cunning
bool RuthlessCunning(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;

    LocationVector pos = plr->GetPosition();

    Creature* kilsorrow = plr->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z);
    if (kilsorrow == nullptr || kilsorrow->isAlive())
        return true;

    QuestLogEntry* qle = plr->GetQuestLogForEntry(9927);
    if (qle && qle->GetMobCount(0) < qle->GetQuest()->required_mob_or_go_count[0])
    {
        kilsorrow->Despawn(0, 60000);
        plr->AddQuestKill(9927, 0, 0);
    };

    return true;
}

bool FindingTheKeymaster(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;

    plr->AddQuestKill(10256, 0, 0);

    return true;
}

bool TheFleshLies(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;

    LocationVector pos = plr->GetPosition();

    Creature* target = plr->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 20561);
    if (target == nullptr)
        return true;

    LocationVector tyrgetPos = target->GetPosition();

    QuestLogEntry* qle = plr->GetQuestLogForEntry(10345);
    if (qle == nullptr)
        return true;

    if (qle && qle->GetMobCount(0) < qle->GetQuest()->required_mob_or_go_count[0])
    {
        plr->AddQuestKill(10345, 0, 0);

        GameObject* obj = plr->GetMapMgr()->CreateAndSpawnGameObject(183816, tyrgetPos.x, tyrgetPos.y, tyrgetPos.z, tyrgetPos.o, 1);
        if (obj != nullptr)
            obj->Despawn(1 * 30 * 1000, 0);
    }
    target->Despawn(2000, 60 * 1000);
    plr->UpdateNearbyGameObjects();

    return true;
}

bool SurveyingtheRuins(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    QuestLogEntry* pQuest = pPlayer->GetQuestLogForEntry(10335);
    if (pQuest == nullptr)
        return true;

    if (pQuest->GetMobCount(0) < pQuest->GetQuest()->required_mob_or_go_count[0])
    {
        GameObject* mark1 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(4695.2f, 2603.39f, 209.878f, 184612);
        if (mark1 == nullptr)
            mark1 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(4695.28f, 2602.73f, 209.84f, 300095);

        if (mark1 != nullptr && pPlayer->CalcDistance(pPlayer, mark1) < 15)
        {
            pPlayer->AddQuestKill(10335, 0, 0);

            return true;
        }
    }

    if (pQuest->GetMobCount(1) < pQuest->GetQuest()->required_mob_or_go_count[1])
    {
        GameObject* mark2 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(4608.08f, 2442.02f, 195.71f, 184612);
        if (mark2 == nullptr)
            mark2 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(4607.71f, 2440.72f, 195.65f, 300095);

        if (mark2 != NULL && pPlayer->CalcDistance(pPlayer, mark2) < 15)
        {
            pPlayer->AddQuestKill(10335, 1, 0);

            return true;
        }
    }

    if (pQuest->GetMobCount(2) < pQuest->GetQuest()->required_mob_or_go_count[2])
    {
        GameObject* mark3 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(4716.37f, 2371.59f, 198.168f, 184612);
        if (mark3 == nullptr)
            mark3 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(4716.77f, 2371.6f, 198.19f, 300095);

        if (mark3 != nullptr && pPlayer->CalcDistance(pPlayer, mark3) < 15)
        {
            pPlayer->AddQuestKill(10335, 2, 0);

            return true;
        }
    }

    return true;
}

bool CrystalOfDeepShadows(uint8_t /*effectIndex*/, Spell* pSpell) // Becoming a Shadoweave Tailor
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;
    QuestLogEntry* qle = plr->GetQuestLogForEntry(10833);

    if (qle == nullptr)
        return true;

    qle->SetMobCount(0, 1);
    qle->SendUpdateAddKill(0);
    qle->UpdatePlayerFields();

    return true;
}

bool Carcass(uint8_t /*effectIndex*/, Spell* pSpell) // Becoming a Shadoweave Tailor
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    LocationVector pos = pPlayer->GetPosition();

    QuestLogEntry* pQuest = pPlayer->GetQuestLogForEntry(10804);
    Creature* NetherDrake = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pos.x, pos.y, pos.z, 21648);
    GameObject* FlayerCarcass = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(pos.x, pos.y, pos.z, 185155);

    if (FlayerCarcass == nullptr)
    {
        FlayerCarcass = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(185155, pos.x, pos.y, pos.z, 0, 1);
        FlayerCarcass->Despawn(60000, 0);
    }
    if (NetherDrake == nullptr)
        return true;

    if (NetherDrake->HasAura(38502))
        return true;

    if (pQuest != nullptr && pQuest->GetMobCount(0) < pQuest->GetQuest()->required_mob_or_go_count[0])
    {
        NetherDrake->castSpell(NetherDrake, sSpellMgr.getSpellInfo(38502), true);
        NetherDrake->GetAIInterface()->setSplineFlying();
        NetherDrake->GetAIInterface()->MoveTo(pos.x, pos.y + 2, pos.z);

        pPlayer->AddQuestKill(10804, 0, 0);
    }
    return true;
}

bool ForceofNeltharakuSpell(uint8_t /*effectIndex*/, Spell* pSpell) // Becoming a Shadoweave Tailor
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    WoWGuid wowGuid;
    wowGuid.Init(pPlayer->GetSelection());
    Creature* pTarget = pPlayer->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());

    if (pTarget == nullptr)
        return true;

    QuestLogEntry* pQuest = pPlayer->GetQuestLogForEntry(10854);
    if (pQuest == nullptr)
        return true;

    if (pTarget->getEntry() == 21722 && pPlayer->CalcDistance(pTarget) < 30)
    {
        if (pQuest->GetMobCount(0) < pQuest->GetQuest()->required_mob_or_go_count[0])
        {
            pTarget->castSpell(pPlayer, sSpellMgr.getSpellInfo(38775), true);

            pPlayer->AddQuestKill(10854, 0, 0);
            pTarget->setMoveRoot(false);
            pTarget->GetAIInterface()->setWayPointToMove(0);
        }
    }
    return true;
}

bool UnlockKarynakuChains(uint32 /*i*/, Spell* pSpell) // Becoming a Shadoweave Tailor
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    pPlayer->AddQuestKill(10872, 0, 0);

    return true;
}


bool ShatariTorch(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;
    WoWGuid wowGuid;
    wowGuid.Init(plr->GetSelection());
    Creature* target = plr->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());

    if (target == nullptr)
        return true;

    LocationVector pos = target->GetPosition();

    if (plr->CalcDistance(pos.x, pos.y, pos.z, plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ()) > 5)
        return true;

    QuestLogEntry* qle = plr->GetQuestLogForEntry(10913);
    if (qle == nullptr)
        return true;

    GameObject* obj = nullptr;

    if (target->getEntry() == 21859)
    {
        if (qle->GetMobCount(0) == qle->GetQuest()->required_mob_or_go_count[0])
            return true;

        plr->AddQuestKill(10913, 0, 0);

        obj = plr->GetMapMgr()->CreateAndSpawnGameObject(183816, pos.x, pos.y, pos.z, pos.o, 1);
        if (obj != nullptr)
            obj->Despawn(1 * 60 * 1000, 0);
    }
    else if (target->getEntry() == 21846)
    {
        if (qle->GetMobCount(1) == qle->GetQuest()->required_mob_or_go_count[1])
            return true;

        plr->AddQuestKill(10913, 1, 0);

        obj = plr->GetMapMgr()->CreateAndSpawnGameObject(183816, pos.x, pos.y, pos.z, pos.o, 1);
        if (obj != nullptr)
            obj->Despawn(1 * 60 * 1000, 0);
    }
    else
        return true;

    target->Despawn(0, 1 * 60 * 1000);

    plr->UpdateNearbyGameObjects();

    return true;
}

// Lost!

bool SpragglesCanteen(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;
    WoWGuid wowGuid;
    wowGuid.Init(plr->GetSelection());

    Creature* target = plr->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
    if (target == nullptr)
        return true;

    if (target->getEntry() != 9999)
        return true;

    QuestLogEntry* qle = plr->GetQuestLogForEntry(4492);
    if (qle == nullptr)
        return true;

    target->setStandState(STANDSTATE_STAND);
    target->setDeathState(ALIVE);

    target->Despawn(30 * 1000, 1 * 60 * 1000);

    qle->SetMobCount(0, 1);
    qle->SendUpdateAddKill(0);
    qle->UpdatePlayerFields();

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Finding the Source
bool FindingTheSource(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Player* pPlayer = pSpell->p_caster;
    if (pPlayer == nullptr)
        return true;

    QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(974);
    if (qle == nullptr)
        return true;

    GameObject* place1 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-7163, -1149, -264, 148503);
    GameObject* place2 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-7281, -1244, -248, 148503);
    GameObject* place3 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-7140, -1465, -242, 148503);
    GameObject* place4 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-7328, -1461, -242, 148503);
    GameObject* place5 = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-7092, -1305, -187, 148503);

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
            if (qle->GetMobCount(0) < qle->GetQuest()->required_mob_or_go_count[0])
            {
                pPlayer->AddQuestKill(974, 0, 0);
            }
        }
    }
    return true;
}

// quest 5163 - Are We There, Yeti?
bool ReleaseUmisYeti(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr || pSpell->GetUnitTarget() == nullptr || !pSpell->GetUnitTarget()->isCreature())
        return true;

    QuestLogEntry* qLogEntry = pSpell->p_caster->GetQuestLogForEntry(5163);
    if (qLogEntry == nullptr)
        return true;

    Creature* target = static_cast<Creature*>(pSpell->GetUnitTarget());
    static const uint32 friends[] = { 10978, 7583, 10977 };
    for (uint8 j = 0; j < sizeof(friends) / sizeof(uint32); j++)
    {
        if (target->getEntry() == friends[j] && qLogEntry->GetMobCount(j) < qLogEntry->GetQuest()->required_mob_or_go_count[j])
        {
            pSpell->p_caster->AddQuestKill(5163, j, 0);
            return true;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Healing The Lake
bool HealingTheLake(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;

    pPlayer->AddQuestKill(9294, 0, 0);

    return true;
}

// Protecting Our Own
bool ProtectingOurOwn(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* plr = pSpell->p_caster;

    plr->AddQuestKill(10488, 0, 0);

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
    if (pSpell->p_caster == nullptr || pSpell->GetGameObjectTarget() == nullptr)
        return true;

    Player* pPlayer = pSpell->p_caster;
    QuestLogEntry* pQuest = pPlayer->GetQuestLogForEntry(9452); //Red Snapper - Very Tasty
    if (pQuest == nullptr)
        return true;

    pSpell->GetGameObjectTarget()->Despawn(600, 20000);

    LocationVector pos = pPlayer->GetPosition();

    if (Util::getRandomUInt(10) <= 3)
    {
        Creature* pNewCreature = pPlayer->GetMapMgr()->GetInterface()->SpawnCreature(17102, pos.x, pos.y, pos.z, pos.o, true, false, 0, 0);
        if (pNewCreature != nullptr)
        {
            pNewCreature->GetAIInterface()->StopMovement(500);
            pNewCreature->setAttackTimer(MELEE, 1000);
            pNewCreature->m_noRespawn = true;
        }
    }

    if (pPlayer->getItemInterface()->GetItemCount(pQuest->GetQuest()->required_item[0], true) < pQuest->GetQuest()->required_itemcount[0])
        pPlayer->getItemInterface()->AddItemById(23614, 1, 0); //Red Snapper.

    return true;
}

bool InducingVision(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == nullptr)
        return true;

    Player* mTarget = pSpell->p_caster;
    if (mTarget == nullptr || mTarget->GetMapMgr() == nullptr || mTarget->GetMapMgr()->GetInterface() == nullptr)
        return true;

    Creature* creature = mTarget->GetMapMgr()->GetInterface()->SpawnCreature(2983, -2238.994873f, -408.009552f, -9.424423f, 5.753043f, true, false, 0, 0);
    creature->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);

    creature->LoadWaypointGroup(17);
    creature->SwitchToCustomWaypoints();

    return true;
}

void SetupQuestItems(ScriptMgr* mgr)
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

    uint32 huntertamingquestspellids[] =
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
