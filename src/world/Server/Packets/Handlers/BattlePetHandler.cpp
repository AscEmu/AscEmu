/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Management/BattlePets/BattlePetCollection.hpp"
#include "Management/BattlePets/BattlePetMgr.hpp"
#include "Management/ItemInterface.h"
#include "Objects/Item.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Packets/CmsgBattlePetGuid.h"
#include "Server/Packets/CmsgBattlePetModifyName.h"
#include "Server/Packets/CmsgBattlePetQueryName.h"
#include "Server/Packets/CmsgBattlePetSetBattleSlot.h"
#include "Server/Packets/CmsgBattlePetSetFlags.h"
#include "Server/Packets/CmsgUseCritterItem.h"
#include "Server/Packets/SmsgBattlePetError.h"
#include "Server/Packets/SmsgBattlePetJournalLockAcquired.h"
#include "Server/Packets/SmsgQueryBattlePetNameResponse.h"
#include "Server/WorldSession.h"
#include "Storage/WDB/WDBStructures.hpp"
#include "Version/VersionRegistry.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleBattlePetRequestJournal(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING >= Mop
    m_battlePets->sendJournal();
#endif
}

void WorldSession::handleBattlePetRequestJournalLock(WorldPacket& /*recvPacket*/)
{
    // one session per account, the lock is always ours
    SmsgBattlePetJournalLockAcquired lockAcquired;
    sendManagedPacket(lockAcquired);
}

void WorldSession::handleBattlePetDeletePet([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Mop
    CmsgBattlePetGuid srlPacket(CMSG_BATTLE_PET_DELETE_PET);
    if (!parsePacket(recvPacket, srlPacket))
        return;

    m_battlePets->removePet(srlPacket.petId);
#endif
}

void WorldSession::handleBattlePetSummon([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Mop
    CmsgBattlePetGuid srlPacket(static_cast<uint16_t>(Version::opcodeIdForHex(recvPacket.getOpcode(), getClientProtocol())));
    if (!parsePacket(recvPacket, srlPacket))
        return;

    m_battlePets->summon(srlPacket.petId);
#endif
}

void WorldSession::handleBattlePetSetBattleSlot([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Mop
    CmsgBattlePetSetBattleSlot srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    m_battlePets->setSlot(srlPacket.petId, srlPacket.slot);
#endif
}

void WorldSession::handleBattlePetModifyName([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Mop
    CmsgBattlePetModifyName srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    m_battlePets->rename(srlPacket.petId, srlPacket.name);
#endif
}

void WorldSession::handleBattlePetQueryName([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Mop
    CmsgBattlePetQueryName srlPacket;
    if (!parsePacket(recvPacket, srlPacket) || _player == nullptr)
        return;

    // the pet belongs to the journal of the player who summoned the companion
    Unit* critter = _player->getWorldMapUnit(srlPacket.unitGuid);
    Player* owner = critter != nullptr ? critter->getPlayerOwner() : nullptr;
    if (owner == nullptr || owner->getSession() == nullptr)
        return;

    std::string name;
    uint64_t timestamp = 0;
    uint32_t creatureId = 0;
    if (!owner->getSession()->getBattlePets().getPetName(srlPacket.petId, name, timestamp, creatureId))
        return;

    const bool allow = !name.empty();
    SmsgQueryBattlePetNameResponse response(srlPacket.petId, creatureId, static_cast<time_t>(timestamp), std::move(name), allow);
    sendManagedPacket(response);
#endif
}

void WorldSession::handleBattlePetSetFlags([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Mop
    CmsgBattlePetSetFlags srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    m_battlePets->setFlags(srlPacket.petId, srlPacket.flags, srlPacket.apply);
#endif
}

void WorldSession::handleBattlePetClearFanfare([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Mop
    CmsgBattlePetGuid srlPacket(CMSG_BATTLE_PET_CLEAR_FANFARE);
    if (!parsePacket(recvPacket, srlPacket))
        return;

    m_battlePets->clearFanfare(srlPacket.petId);
#endif
}

void WorldSession::handleUseCritterItem([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Mop
    CmsgUseCritterItem srlPacket;
    if (!parsePacket(recvPacket, srlPacket) || _player == nullptr)
        return;

    Item* item = _player->getItemInterface()->GetItemByGUID(srlPacket.itemGuid);
    if (item == nullptr)
        return;

    const auto* species = sBattlePetMgr.getSpeciesByItem(item->getItemProperties());
    if (species == nullptr)
        return;

    if (const uint8_t error = m_battlePets->canLearn(species->id))
    {
        SmsgBattlePetError errorPacket(error, species->creatureId);
        sendManagedPacket(errorPacket);
        return;
    }

    if (m_battlePets->learnSpecies(species->id))
        _player->getItemInterface()->RemoveItemAmtByGuid(srlPacket.itemGuid, 1);
#endif
}
