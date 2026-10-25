/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "BattlePetDefines.hpp"

#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    struct BattlePetData;
}

class Player;
class WorldSession;

// account wide battle pet journal of the logonserver, this realm shows and changes only pets of species it knows
class BattlePetCollection
{
public:
    explicit BattlePetCollection(WorldSession& session);

    void requestLoad();
    void onLoaded(std::vector<LogonComm::BattlePetRecord> pets, std::vector<LogonComm::BattlePetSlotRecord> slots);
    void onAddResult(uint8_t result, LogonComm::BattlePetRecord record);

    // sends the journal once it is loaded and the pets the logonserver added
    void update();
    void sendJournal();

    // the error the client shows when the species can not be learned, 0 when it can
    uint8_t canLearn(uint32_t speciesId) const;

    // asks the logonserver for the new pet, the client gets it with its answer
    bool learnSpecies(uint32_t speciesId);

    void removePet(uint32_t petId);
    void rename(uint32_t petId, std::string const& name);
    void setFlags(uint32_t petId, uint32_t flags, bool apply);
    void clearFanfare(uint32_t petId);
    void setSlot(uint32_t petId, uint8_t slot);

    // the same pet a second time dismisses it
    void summon(uint32_t petId);
    void dismiss();

    // name and name time of a pet for the name query of other clients
    bool getPetName(uint32_t petId, std::string& name, uint64_t& timestamp, uint32_t& creatureId) const;

private:
    BattlePets::BattlePet* findKnown(uint32_t petId);
    BattlePets::BattlePet const* findKnown(uint32_t petId) const;
    AscEmu::Packets::BattlePetData toPacket(BattlePets::BattlePet const& pet) const;
    uint32_t visibleSlotPet(LogonComm::BattlePetSlotRecord const& slot) const;
    uint8_t canLearnLocked(uint32_t speciesId) const;
    void sendJournalLocked();
    void sendUpdate(BattlePets::BattlePet const& pet, bool added);
    void save(std::vector<uint32_t> const& changed, std::vector<uint32_t> const& removed);
    void dismissLocked(Player* player);

    WorldSession& m_session;

    mutable std::mutex m_mutex;
    std::map<uint32_t, BattlePets::BattlePet> m_pets;   // every pet of the account, also those this realm does not know
    std::vector<LogonComm::BattlePetSlotRecord> m_slots;
    std::vector<uint32_t> m_pendingSpecies;             // species asked for, the logonserver did not answer yet
    std::vector<uint32_t> m_addedPets;                  // pets the logonserver added, the client did not get yet
    std::vector<std::pair<uint8_t, uint32_t>> m_refusedPets; // error and creature of refused pets
    bool m_loaded = false;
    bool m_journalWanted = false;
    bool m_journalSent = false;
    uint32_t m_summonedPet = 0;
};
