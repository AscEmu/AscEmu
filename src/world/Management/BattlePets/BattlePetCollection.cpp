/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "BattlePetCollection.hpp"

#include "BattlePetMgr.hpp"
#include "LogonCommDefines.hpp"
#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include "Management/AchievementMgr.h"
#include "Map/Maps/WorldMap.hpp"
#include "Network/WorldPacket.hpp"
#include "Objects/Units/Creatures/Summons/Summon.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/Packets/SmsgBattlePetDeleted.h"
#include "Server/Packets/SmsgBattlePetError.h"
#include "Server/Packets/SmsgBattlePetJournal.h"
#include "Server/Packets/SmsgBattlePetJournalLockAcquired.h"
#include "Server/Packets/SmsgBattlePetUpdates.h"
#include "Server/Packets/SmsgPetBattleSlotUpdates.h"
#include "Server/WorldSession.h"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/utf8.hpp"

#include <algorithm>
#include <set>

using namespace AscEmu::Packets;
using Version::PlayerField;
using Version::UnitField;

BattlePetCollection::BattlePetCollection(WorldSession& session) : m_session(session)
{
}

void BattlePetCollection::requestLoad()
{
    if (LogonCommClientSocket* socket = sLogonCommHandler.getLogonServerSocket())
    {
        WorldPacket data(LRCMSG_BATTLE_PET_REQUEST, 4);
        data << m_session.GetAccountId();
        socket->SendPacket(&data, false);
    }
}

void BattlePetCollection::onLoaded(std::vector<LogonComm::BattlePetRecord> pets, std::vector<LogonComm::BattlePetSlotRecord> slots)
{
    std::lock_guard lock(m_mutex);

    m_pets.clear();
    for (auto& record : pets)
    {
        const uint32_t id = record.id;
        m_pets.emplace(id, sBattlePetMgr.makePet(std::move(record)));
    }

    // an account without stored slots starts with locked ones
    m_slots.clear();
    for (uint8_t index = 0; index < BattlePets::SlotCount; ++index)
    {
        LogonComm::BattlePetSlotRecord slot;
        slot.index = index;
        m_slots.push_back(slot);
    }

    for (const auto& slot : slots)
        if (slot.index < BattlePets::SlotCount)
            m_slots[slot.index] = slot;

    m_loaded = true;
}

void BattlePetCollection::onAddResult(uint8_t result, LogonComm::BattlePetRecord record)
{
    std::lock_guard lock(m_mutex);

    if (const auto pending = std::find(m_pendingSpecies.begin(), m_pendingSpecies.end(), record.species); pending != m_pendingSpecies.end())
        m_pendingSpecies.erase(pending);

    if (result != LogonComm::BattlePetAddOk)
    {
        const auto error = result == LogonComm::BattlePetAddTooManyPets ? BattlePets::Error::CantHaveMorePets : BattlePets::Error::CantHaveMorePetsOfType;
        BattlePets::BattlePet refused = sBattlePetMgr.makePet(std::move(record));
        m_refusedPets.emplace_back(static_cast<uint8_t>(error), refused.creatureId);
        return;
    }

    const uint32_t id = record.id;
    m_pets.insert_or_assign(id, sBattlePetMgr.makePet(std::move(record)));
    m_addedPets.push_back(id);
}

void BattlePetCollection::update()
{
    size_t const added = updateLocked();

    Player* player = m_session.GetPlayer();
    for (size_t i = 0; i < added && player != nullptr && player->IsInWorld(); ++i)
        player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARNED_NEW_PET);
}

size_t BattlePetCollection::updateLocked()
{
    std::lock_guard lock(m_mutex);
    if (m_loaded && m_journalWanted && !m_journalSent)
        sendJournalLocked();

    // a journal sent after the answer holds the pet already
    if (m_journalSent)
    {
        for (const uint32_t id : m_addedPets)
            if (const auto* pet = findKnown(id))
                sendUpdate(*pet, true);
    }
    size_t const added = m_addedPets.size();
    m_addedPets.clear();

    for (const auto& [error, creatureId] : m_refusedPets)
    {
        SmsgBattlePetError errorPacket(error, creatureId);
        m_session.sendManagedPacket(errorPacket);
    }
    m_refusedPets.clear();
    return added;
}

void BattlePetCollection::sendJournal()
{
    std::lock_guard lock(m_mutex);
    m_journalWanted = true;
    m_journalSent = false;

    if (m_loaded)
        sendJournalLocked();
}

BattlePets::BattlePet* BattlePetCollection::findKnown(uint32_t petId)
{
    const auto itr = m_pets.find(petId);
    return itr != m_pets.end() && itr->second.known ? &itr->second : nullptr;
}

BattlePets::BattlePet const* BattlePetCollection::findKnown(uint32_t petId) const
{
    const auto itr = m_pets.find(petId);
    return itr != m_pets.end() && itr->second.known ? &itr->second : nullptr;
}

uint32_t BattlePetCollection::visibleSlotPet(LogonComm::BattlePetSlotRecord const& slot) const
{
    // a slot with a pet this realm does not know shows empty and keeps its pet
    return findKnown(slot.petId) != nullptr ? slot.petId : 0;
}

void BattlePetCollection::sendJournalLocked()
{
    std::vector<BattlePetSlotData> slots;
    for (const auto& slot : m_slots)
        slots.push_back({ slot.index, visibleSlotPet(slot), slot.locked });

    std::vector<BattlePetData> pets;
    pets.reserve(m_pets.size());
    for (const auto& [id, pet] : m_pets)
        if (pet.known)
            pets.push_back(toPacket(pet));

    SmsgBattlePetJournal journal(std::move(slots), std::move(pets), true);
    journal.maxPets = BattlePets::MaxPets;
    m_session.sendManagedPacket(journal);

    SmsgBattlePetJournalLockAcquired lockAcquired;
    m_session.sendManagedPacket(lockAcquired);

    m_journalSent = true;
}

BattlePetData BattlePetCollection::toPacket(BattlePets::BattlePet const& pet) const
{
    BattlePetData data;
    data.id = pet.record.id;
    data.species = pet.record.species;
    data.creatureId = pet.creatureId;
    data.displayId = pet.displayId;
    data.breed = pet.record.breed;
    data.level = pet.record.level;
    data.experience = pet.record.experience;
    data.flags = pet.record.flags;
    data.power = pet.power;
    data.health = pet.record.health;
    data.maxHealth = pet.maxHealth;
    data.speed = pet.speed;
    data.quality = pet.record.quality;
    data.name = pet.record.name;
    data.noRename = pet.noRename;
    return data;
}

uint8_t BattlePetCollection::canLearn(uint32_t speciesId) const
{
    std::lock_guard lock(m_mutex);
    return canLearnLocked(speciesId);
}

uint8_t BattlePetCollection::canLearnLocked(uint32_t speciesId) const
{
#if VERSION_STRING >= Mop
    // without the journal the limits are unknown
    const auto* species = sBattlePetMgr.getSpecies(speciesId);
    if (species == nullptr || !m_loaded)
        return static_cast<uint8_t>(BattlePets::Error::CantHaveMorePetsOfType);

    // the limits count the pets of every realm and the ones still on their way
    if (m_pets.size() + m_pendingSpecies.size() >= BattlePets::MaxPets)
        return static_cast<uint8_t>(BattlePets::Error::CantHaveMorePets);

    const auto stored = std::count_if(m_pets.begin(), m_pets.end(), [speciesId](auto const& entry) { return entry.second.record.species == speciesId; });
    const auto pending = std::count(m_pendingSpecies.begin(), m_pendingSpecies.end(), speciesId);
    const uint32_t maxCount = (species->flags & BattlePets::SPECIES_FLAG_LEGACY_ACCOUNT_UNIQUE) != 0 ? 1u : uint32_t(BattlePets::MaxPetsPerSpecies);
    if (static_cast<uint32_t>(stored + pending) >= maxCount)
        return static_cast<uint8_t>(BattlePets::Error::CantHaveMorePetsOfType);

    return 0;
#else
    (void)speciesId;
    return static_cast<uint8_t>(BattlePets::Error::CantHaveMorePetsOfType);
#endif
}

bool BattlePetCollection::learnSpecies(uint32_t speciesId)
{
#if VERSION_STRING >= Mop
    std::lock_guard lock(m_mutex);
    if (canLearnLocked(speciesId) != 0)
        return false;

    LogonCommClientSocket* socket = sLogonCommHandler.getLogonServerSocket();
    if (socket == nullptr)
    {
        sLogger.failure("BattlePetCollection::learnSpecies: no logonserver, account {} can not learn species {}", m_session.GetAccountId(), speciesId);
        return false;
    }

    const auto* species = sBattlePetMgr.getSpecies(speciesId);

    LogonComm::BattlePetRecord record;
    record.species = speciesId;
    record.breed = sBattlePetMgr.rollBreed(speciesId);
    record.displayId = sBattlePetMgr.selectDisplay(species);
    record.level = 1;
    record.quality = static_cast<uint8_t>(sBattlePetMgr.getDefaultQuality(speciesId));
    record.health = sBattlePetMgr.makePet(record).maxHealth;

    const uint8_t maxPerSpecies = (species->flags & BattlePets::SPECIES_FLAG_LEGACY_ACCOUNT_UNIQUE) != 0 ? uint8_t(1) : BattlePets::MaxPetsPerSpecies;

    WorldPacket data(LRCMSG_BATTLE_PET_ADD, 64);
    data << m_session.GetAccountId() << maxPerSpecies << BattlePets::MaxPets;
    LogonComm::writeBattlePet(data, record);
    socket->SendPacket(&data, false);

    m_pendingSpecies.push_back(speciesId);
    return true;
#else
    (void)speciesId;
    return false;
#endif
}

void BattlePetCollection::sendUpdate(BattlePets::BattlePet const& pet, bool added)
{
    SmsgBattlePetUpdates updates({ toPacket(pet) }, added);
    m_session.sendManagedPacket(updates);
}

void BattlePetCollection::removePet(uint32_t petId)
{
    std::lock_guard lock(m_mutex);
    if (findKnown(petId) == nullptr)
        return;

    m_pets.erase(petId);
    if (m_summonedPet == petId)
        dismissLocked(m_session.GetPlayer());

    for (auto& slot : m_slots)
        if (slot.petId == petId)
            slot.petId = 0;

    SmsgBattlePetDeleted deleted(petId);
    m_session.sendManagedPacket(deleted);
    save({}, { petId });
}

void BattlePetCollection::rename(uint32_t petId, std::string const& name)
{
    std::lock_guard lock(m_mutex);
    auto* pet = findKnown(petId);
    if (pet == nullptr || pet->noRename)
        return;

    if (!AscEmu::Util::utf8::is_valid(name) || AscEmu::Util::utf8::length(name) > BattlePets::MaxNameLength)
        return;

    pet->record.name = name;
    pet->record.nameTimestamp = static_cast<uint64_t>(UNIXTIME);

    // the new time makes the clients ask for the name again
    if (m_summonedPet == petId)
        if (Player* player = m_session.GetPlayer())
            if (Unit* critter = player->getWorldMapUnit(player->getCritterGuid()))
                critter->setField<uint32_t>(UnitField::BattlePetCompanionNameTimestamp, static_cast<uint32_t>(pet->record.nameTimestamp));

    save({ petId }, {});
}

void BattlePetCollection::setFlags(uint32_t petId, uint32_t flags, bool apply)
{
    std::lock_guard lock(m_mutex);
    auto* pet = findKnown(petId);
    if (pet == nullptr)
        return;

    const uint16_t clientFlags = static_cast<uint16_t>(flags & BattlePets::PET_FLAGS_CLIENT);
    auto& record = pet->record;
    const uint16_t newFlags = apply ? static_cast<uint16_t>(record.flags | clientFlags) : static_cast<uint16_t>(record.flags & ~clientFlags);
    if (newFlags == record.flags)
        return;

    record.flags = newFlags;
    save({ petId }, {});
}

void BattlePetCollection::clearFanfare(uint32_t petId)
{
    std::lock_guard lock(m_mutex);
    auto* pet = findKnown(petId);
    if (pet == nullptr || (pet->record.flags & BattlePets::PET_FLAG_FANFARE_NEEDED) == 0)
        return;

    pet->record.flags = static_cast<uint16_t>(pet->record.flags & ~BattlePets::PET_FLAG_FANFARE_NEEDED);
    save({ petId }, {});
}

void BattlePetCollection::setSlot(uint32_t petId, uint8_t slot)
{
    std::lock_guard lock(m_mutex);
    if (slot >= m_slots.size() || m_slots[slot].locked || findKnown(petId) == nullptr)
        return;

    std::vector<uint8_t> changed{ slot };

    // a pet keeps one slot, the slot it leaves takes the pet it replaces
    for (auto& other : m_slots)
    {
        if (other.index != slot && other.petId == petId)
        {
            other.petId = m_slots[slot].petId;
            changed.push_back(other.index);
        }
    }
    m_slots[slot].petId = petId;

    std::vector<BattlePetSlotData> slots;
    for (const uint8_t index : changed)
        slots.push_back({ m_slots[index].index, visibleSlotPet(m_slots[index]), m_slots[index].locked });

    // 5.x takes one slot per packet
    if (m_session.getClientProtocol().isMop())
    {
        for (const auto& entry : slots)
        {
            SmsgPetBattleSlotUpdates updates({ entry }, false, false);
            m_session.sendManagedPacket(updates);
        }
    }
    else
    {
        SmsgPetBattleSlotUpdates updates(std::move(slots), false, false);
        m_session.sendManagedPacket(updates);
    }

    save({}, {});
}

void BattlePetCollection::summon(uint32_t petId)
{
#if VERSION_STRING >= Mop
    Player* player = m_session.GetPlayer();
    if (player == nullptr || !player->IsInWorld())
        return;

    std::lock_guard lock(m_mutex);
    const auto* pet = findKnown(petId);
    if (pet == nullptr)
        return;

    const bool wasSummoned = m_summonedPet == petId && player->getCritterGuid() != 0;
    dismissLocked(player);
    if (wasSummoned)
        return;

    const auto* species = sBattlePetMgr.getSpecies(pet->record.species);
    if (species == nullptr || species->creatureId == 0)
        return;

    // the summon properties of the companion come from the summon spell of the species
    const uint32_t spellId = species->summonSpellId != 0 ? species->summonSpellId : BattlePets::SpellSummonBattlePet;
    WDB::Structures::SummonPropertiesEntry const* properties = nullptr;
    for (const uint32_t candidate : { spellId, BattlePets::SpellSummonBattlePet })
    {
        if (const auto* spellInfo = sSpellMgr.getSpellInfo(candidate))
            for (uint8_t effect = 0; effect < MAX_SPELL_EFFECTS && properties == nullptr; ++effect)
                if (spellInfo->getEffect(effect) == SPELL_EFFECT_SUMMON)
                    properties = sSummonPropertiesStore.lookupEntry(static_cast<uint32_t>(spellInfo->getEffectMiscValueB(effect)));

        if (properties != nullptr)
            break;
    }

    if (properties == nullptr)
    {
        sLogger.failure("BattlePetCollection::summon: no summon properties for species {} (spell {})", pet->record.species, spellId);
        return;
    }

    Summon* critter = player->getWorldMap()->summonCreature(species->creatureId, player->GetPosition(), properties, 0, player, spellId);
    if (critter == nullptr)
    {
        sLogger.failure("BattlePetCollection::summon: creature {} of species {} could not be summoned", species->creatureId, pet->record.species);
        return;
    }

    if (pet->displayId != 0)
    {
        critter->setDisplayId(pet->displayId);
        critter->setNativeDisplayId(pet->displayId);
    }

    critter->setField<uint64_t>(UnitField::BattlePetCompanionGuid, BattlePets::petGuid(petId));
    critter->setField<uint32_t>(UnitField::BattlePetCompanionNameTimestamp, static_cast<uint32_t>(pet->record.nameTimestamp));
    critter->setField<uint32_t>(UnitField::WildBattlePetLevel, pet->record.level);

    player->setCritterGuid(critter->getGuid());
    player->setField<uint64_t>(PlayerField::SummonedBattlePetGuid, BattlePets::petGuid(petId));
    player->setField<uint8_t>(PlayerField::CurrentBattlePetBreedQuality, pet->record.quality);
    m_summonedPet = petId;
#else
    (void)petId;
#endif
}

void BattlePetCollection::dismiss()
{
    std::lock_guard lock(m_mutex);
    dismissLocked(m_session.GetPlayer());
}

void BattlePetCollection::dismissLocked(Player* player)
{
    m_summonedPet = 0;
    if (player == nullptr)
        return;

#if VERSION_STRING > TBC
    if (const uint64_t critterGuid = player->getCritterGuid())
    {
        player->setCritterGuid(0);
        if (Unit* critter = player->getWorldMapUnit(critterGuid))
            critter->destroy();
    }
#endif

    player->setField<uint64_t>(PlayerField::SummonedBattlePetGuid, uint64_t(0));
    player->setField<uint8_t>(PlayerField::CurrentBattlePetBreedQuality, uint8_t(0));
}

bool BattlePetCollection::getPetName(uint32_t petId, std::string& name, uint64_t& timestamp, uint32_t& creatureId) const
{
    std::lock_guard lock(m_mutex);
    const auto* pet = findKnown(petId);
    if (pet == nullptr)
        return false;

    name = pet->record.name;
    timestamp = pet->record.nameTimestamp;
    creatureId = pet->creatureId;
    return true;
}

uint32_t BattlePetCollection::getUniqueSpeciesCount() const
{
    std::lock_guard lock(m_mutex);
    std::set<uint32_t> species;
    for (const auto& [id, pet] : m_pets)
        if (pet.known)
            species.insert(pet.record.species);
    return static_cast<uint32_t>(species.size());
}

uint32_t BattlePetCollection::getPetCountAtLevel(uint16_t level) const
{
    std::lock_guard lock(m_mutex);
    return static_cast<uint32_t>(std::count_if(m_pets.begin(), m_pets.end(), [level](auto const& entry) { return entry.second.known && entry.second.record.level >= level; }));
}

uint16_t BattlePetCollection::getLowestSlotLevel() const
{
    std::lock_guard lock(m_mutex);
    uint16_t lowest = BattlePets::MaxLevel;
    for (const auto& slot : m_slots)
    {
        // an empty slot has no level
        const auto* pet = findKnown(slot.petId);
        lowest = std::min<uint16_t>(lowest, pet != nullptr ? pet->record.level : 0);
    }
    return lowest;
}

void BattlePetCollection::save(std::vector<uint32_t> const& changed, std::vector<uint32_t> const& removed)
{
    LogonCommClientSocket* socket = sLogonCommHandler.getLogonServerSocket();
    if (socket == nullptr)
    {
        sLogger.failure("BattlePetCollection::save: no logonserver, the journal of account {} is not saved", m_session.GetAccountId());
        return;
    }

    WorldPacket data(LRCMSG_BATTLE_PET_SAVE, 16 + changed.size() * 48 + removed.size() * 4);
    data << m_session.GetAccountId() << uint32_t(changed.size());
    for (const uint32_t id : changed)
        LogonComm::writeBattlePet(data, m_pets.at(id).record);

    data << uint32_t(removed.size());
    for (const uint32_t id : removed)
        data << id;

    // the stored slots, also those with pets this realm does not show
    LogonComm::writeBattlePetSlots(data, m_slots);
    socket->SendPacket(&data, false);
}
