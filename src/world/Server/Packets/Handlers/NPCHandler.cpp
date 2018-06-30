/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/ManagedPacket.h"
#include "Server/WorldSession.h"
#include "Server/Packets/MsgTabardvendorActivate.h"
#include "Server/Packets/CmsgBankerActivate.h"
#include "Server/Packets/SmsgShowBank.h"
#include "Server/Packets/MsgAuctionHello.h"
#include "Server/Packets/SmsgSpiritHealerConfirm.h"
#include "Server/Packets/CmsgTrainerBuySpell.h"
#include "Server/Packets/SmsgTrainerBuySucceeded.h"
#include "Server/Packets/SmsgPetitionShowlist.h"
#include "Server/Packets/CmsgPetitionShowlist.h"
#include "Units/Players/PlayerClasses.hpp"
#include "Units/Creatures/Creature.h"
#include "Map/MapMgr.h"
#include "Management/AuctionMgr.h"
#include "Server/MainServerDefines.h"
#include "Objects/ObjectMgr.h"

using namespace AscEmu::Packets;

void WorldSession::handleTabardVendorActivateOpcode(WorldPacket& recvPacket)
{
    MsgTabardvendorActivate recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLow());
    if (creature == nullptr)
        return;

    SendPacket(MsgTabardvendorActivate(recv_packet.guid).serialise().get());
}

//helper
void WorldSession::sendTabardHelp(Creature* creature)
{
    if (creature == nullptr)
        return;

    SendPacket(MsgTabardvendorActivate(creature->getGuid()).serialise().get());
}

void WorldSession::handleBankerActivateOpcode(WorldPacket& recvPacket)
{
    CmsgBankerActivate recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLow());
    if (creature == nullptr)
        return;

    SendPacket(SmsgShowBank(recv_packet.guid).serialise().get());
}

//helper
void WorldSession::sendBankerList(Creature* creature)
{
    if (creature == nullptr)
        return;

    SendPacket(SmsgShowBank(creature->getGuid()).serialise().get());
}

void WorldSession::handleAuctionHelloOpcode(WorldPacket& recvPacket)
{
    MsgAuctionHello recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLow());
    if (creature == nullptr)
        return;

    sendAuctionList(creature);
}

//helper
void WorldSession::sendAuctionList(Creature* creature)
{
    if (creature == nullptr)
        return;

    const auto auctionHouse = sAuctionMgr.GetAuctionHouse(creature->getEntry());
    if (auctionHouse == nullptr)
        return;

    SendPacket(MsgAuctionHello(creature->getGuid(), auctionHouse->GetID(), auctionHouse->enabled ? 1 : 0).serialise().get());
}

//helper
void WorldSession::sendSpiritHealerRequest(Creature* creature)
{
    SendPacket(SmsgSpiritHealerConfirm(creature->getGuid()).serialise().get());
}

void WorldSession::handleTrainerBuySpellOpcode(WorldPacket& recvPacket)
{
    CmsgTrainerBuySpell recv_packet;
    if (!recv_packet.deserialise((recvPacket)))
        return;

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLow());
    if (creature == nullptr)
        return;

    const auto trainer = creature->GetTrainer();
    if (trainer == nullptr)
        return;

    TrainerSpell* trainerSpell = nullptr;
#if VERSION_STRING == Cata
    for (auto itr : trainer->Spells)
    {
        if (itr.spell == recv_packet.spellId)
        {
            trainerSpell = &itr;
        }
    }
#else
    for (auto itr : trainer->Spells)
    {
        if ((itr.pCastSpell && itr.pCastSpell->getId() == recv_packet.spellId) ||
            (itr.pLearnSpell && itr.pLearnSpell->getId() == recv_packet.spellId))
        {
            trainerSpell = &itr;
        }
    }
#endif

    if (trainerSpell == nullptr)
    {
        sCheatLog.writefromsession(this, "%s tried to learn none-obtainable spell - Possibly using WPE", GetPlayer()->getName().c_str());
        Disconnect();
        return;
    }

#if VERSION_STRING == Cata
    if (TrainerGetSpellStatus(trainerSpell) == TRAINER_SPELL_RED || TRAINER_SPELL_GRAY)
        return;

    GetPlayer()->ModGold(-static_cast<int32_t>(trainerSpell->spellCost));

    if (trainerSpell->IsCastable())
    {
        GetPlayer()->CastSpell(GetPlayer(), trainerSpell->spell, true);
    }
    else
    {
        GetPlayer()->playSpellVisual(creature->getGuid(), 179);
        GetPlayer()->playSpellVisual(GetPlayer()->getGuid(), 362);

        GetPlayer()->addSpell(trainerSpell->spell);
    }
#else
    if (TrainerGetSpellStatus(trainerSpell) != TRAINER_STATUS_LEARNABLE)
        return;

    // teach the spell
    GetPlayer()->ModGold(-static_cast<int32>(trainerSpell->Cost));
    if (trainerSpell->pCastSpell)
    {
        GetPlayer()->CastSpell(GetPlayer(), trainerSpell->pCastSpell->getId(), true);
    }
    else
    {
        GetPlayer()->playSpellVisual(creature->getGuid(), 1459);
        GetPlayer()->playSpellVisual(GetPlayer()->getGuid(), 362);

        GetPlayer()->addSpell(trainerSpell->pLearnSpell->getId());
    }

    if (trainerSpell->DeleteSpell)
    {
        if (trainerSpell->pLearnSpell)
            GetPlayer()->removeSpell(trainerSpell->DeleteSpell, true, true, trainerSpell->pLearnSpell->getId());
        else if (trainerSpell->pCastSpell)
            GetPlayer()->removeSpell(trainerSpell->DeleteSpell, true, true, trainerSpell->pCastRealSpell->getId());
        else
            GetPlayer()->removeSpell(trainerSpell->DeleteSpell, true, false, 0);
    }
#endif
    GetPlayer()->_UpdateSkillFields();

    SendPacket(SmsgTrainerBuySucceeded(recv_packet.guid.GetOldGuid(), recv_packet.spellId).serialise().get());
}

void WorldSession::handleCharterShowListOpcode(WorldPacket& recvPacket)
{
    CmsgPetitionShowlist recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLow());
    if (creature == nullptr)
        return;

    sendCharterRequest(creature);
}

//helper
void WorldSession::sendCharterRequest(Creature* creature)
{
    if (creature == nullptr)
        return;

    SendPacket(SmsgPetitionShowlist(creature->getGuid(), creature->isTabardDesigner()).serialise().get());
}
