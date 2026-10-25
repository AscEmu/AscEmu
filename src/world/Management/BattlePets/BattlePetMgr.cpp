/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "BattlePetMgr.hpp"

#include "Logging/Logger.hpp"
#include "Management/ItemProperties.hpp"
#include "Objects/ItemDefines.hpp"
#include "Objects/Units/Creatures/CreatureDefines.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Random.hpp"

#include <cmath>

BattlePetMgr& BattlePetMgr::getInstance()
{
    static BattlePetMgr mInstance;
    return mInstance;
}

void BattlePetMgr::addState(StatStates& stats, uint32_t stateId, int32_t value)
{
    switch (stateId)
    {
        case BattlePets::STATE_STAT_POWER:      stats.power += value; break;
        case BattlePets::STATE_STAT_STAMINA:    stats.stamina += value; break;
        case BattlePets::STATE_STAT_SPEED:      stats.speed += value; break;
        default: break;
    }
}

void BattlePetMgr::initialize()
{
#if VERSION_STRING >= Mop
    for (uint32_t i = 0; i < sBattlePetBreedStateStore.getNumRows(); ++i)
        if (const auto* state = sBattlePetBreedStateStore.lookupEntry(i))
            addState(m_breedStates[state->breedId], state->stateId, state->value);

    for (uint32_t i = 0; i < sBattlePetSpeciesStateStore.getNumRows(); ++i)
        if (const auto* state = sBattlePetSpeciesStateStore.lookupEntry(i))
            addState(m_speciesStates[state->speciesId], state->stateId, state->value);

    for (uint32_t i = 0; i < sBattlePetBreedQualityStore.getNumRows(); ++i)
        if (const auto* quality = sBattlePetBreedQualityStore.lookupEntry(i))
            m_qualityMultipliers[static_cast<uint8_t>(quality->quality)] = quality->multiplier;

    for (uint32_t i = 0; i < sBattlePetSpeciesStore.getNumRows(); ++i)
    {
        const auto* species = sBattlePetSpeciesStore.lookupEntry(i);
        if (species == nullptr)
            continue;

        if (species->creatureId != 0)
            m_speciesByCreature[species->creatureId] = species;
        if (species->summonSpellId != 0)
            m_speciesBySpell[species->summonSpellId] = species;
    }

    // newer client data leaves most summon spells of the species empty, the summon spell of the creature names them
    for (const auto& [spellId, spellInfo] : *sSpellMgr.getSpellInfoMap())
    {
        for (uint8_t effect = 0; effect < MAX_SPELL_EFFECTS; ++effect)
        {
            if (spellInfo->getEffect(effect) != SPELL_EFFECT_SUMMON)
                continue;

            const auto creature = m_speciesByCreature.find(static_cast<uint32_t>(spellInfo->getEffectMiscValue(effect)));
            if (creature != m_speciesByCreature.end())
                m_speciesBySpell.try_emplace(spellId, creature->second);
        }
    }

    for (uint32_t i = 0; i < sItemToBattlePetStore.getNumRows(); ++i)
        if (const auto* itemEntry = sItemToBattlePetStore.lookupEntry(i))
            if (const auto* species = sBattlePetSpeciesStore.lookupEntry(itemEntry->speciesId))
                m_speciesByItem[itemEntry->itemId] = species;

    uint32_t breedCount = 0;
    if (auto result = WorldDatabase.query("SELECT species_id, breed_id FROM battle_pet_breeds"))
    {
        do
        {
            Field* fields = result->fetch();
            const uint32_t speciesId = fields[0].asUint32();
            if (sBattlePetSpeciesStore.lookupEntry(speciesId) == nullptr)
                continue;

            m_breedsBySpecies[speciesId].push_back(fields[1].asUint16());
            ++breedCount;
        }
        while (result->nextRow());
    }

    if (auto result = WorldDatabase.query("SELECT species_id, quality FROM battle_pet_quality"))
    {
        do
        {
            Field* fields = result->fetch();
            const uint32_t speciesId = fields[0].asUint32();
            const uint8_t quality = fields[1].asUint8();
            if (sBattlePetSpeciesStore.lookupEntry(speciesId) == nullptr || quality >= static_cast<uint8_t>(BattlePets::Quality::Count))
                continue;

            m_defaultQualities[speciesId] = quality;
        }
        while (result->nextRow());
    }

    sLogger.info("BattlePetMgr : {} species ({} summon spells), {} breeds and {} qualities loaded",
        m_speciesByCreature.size(), m_speciesBySpell.size(), breedCount, m_defaultQualities.size());
#endif
}

WDB::Structures::BattlePetSpeciesEntry const* BattlePetMgr::getSpecies(uint32_t speciesId) const
{
#if VERSION_STRING >= Mop
    return sBattlePetSpeciesStore.lookupEntry(speciesId);
#else
    (void)speciesId;
    return nullptr;
#endif
}

WDB::Structures::BattlePetSpeciesEntry const* BattlePetMgr::getSpeciesBySpell(uint32_t spellId) const
{
    const auto itr = m_speciesBySpell.find(spellId);
    return itr != m_speciesBySpell.end() ? itr->second : nullptr;
}

WDB::Structures::BattlePetSpeciesEntry const* BattlePetMgr::getSpeciesByCreature(uint32_t creatureId) const
{
    const auto itr = m_speciesByCreature.find(creatureId);
    return itr != m_speciesByCreature.end() ? itr->second : nullptr;
}

WDB::Structures::BattlePetSpeciesEntry const* BattlePetMgr::getSpeciesByItem(ItemProperties const* itemProperties) const
{
    if (itemProperties == nullptr)
        return nullptr;

    if (const auto itr = m_speciesByItem.find(itemProperties->ItemId); itr != m_speciesByItem.end())
        return itr->second;

    for (const auto& itemSpell : itemProperties->Spells)
        if (itemSpell.Trigger == LEARNING)
            if (const auto* species = getSpeciesBySpell(itemSpell.Id))
                return species;

    return nullptr;
}

uint16_t BattlePetMgr::rollBreed(uint32_t speciesId) const
{
    const auto itr = m_breedsBySpecies.find(speciesId);
    if (itr == m_breedsBySpecies.end() || itr->second.empty())
        return BattlePets::DefaultBreed;

    return itr->second[Util::getRandomUInt(static_cast<uint32_t>(itr->second.size() - 1))];
}

BattlePets::Quality BattlePetMgr::getDefaultQuality(uint32_t speciesId) const
{
    const auto itr = m_defaultQualities.find(speciesId);
    return itr != m_defaultQualities.end() ? static_cast<BattlePets::Quality>(itr->second) : BattlePets::Quality::Poor;
}

uint32_t BattlePetMgr::selectDisplay(WDB::Structures::BattlePetSpeciesEntry const* species) const
{
#if VERSION_STRING >= Mop
    // a species with random displays picks them itself
    if (species == nullptr || (species->flags & BattlePets::SPECIES_FLAG_RANDOM_DISPLAY) != 0)
        return 0;

    if (const auto* properties = sMySQLStore.getCreatureProperties(species->creatureId))
        return properties->getRandomModelId();
#else
    (void)species;
#endif
    return 0;
}

uint32_t BattlePetMgr::realmDisplay(WDB::Structures::BattlePetSpeciesEntry const* species, uint32_t storedDisplay) const
{
#if VERSION_STRING >= Mop
    if (species == nullptr)
        return 0;

    if (storedDisplay != 0)
    {
        if (const auto* properties = sMySQLStore.getCreatureProperties(species->creatureId))
        {
            for (const uint32_t model : { properties->Male_DisplayID, properties->Male_DisplayID2, properties->Female_DisplayID, properties->Female_DisplayID2 })
                if (model == storedDisplay)
                    return storedDisplay;
        }
    }

    return selectDisplay(species);
#else
    (void)species;
    (void)storedDisplay;
    return 0;
#endif
}

void BattlePetMgr::calculateStats(BattlePets::BattlePet& pet) const
{
    const auto breed = m_breedStates.find(pet.record.breed);
    if (breed == m_breedStates.end())
        return;

    float power = static_cast<float>(breed->second.power);
    float stamina = static_cast<float>(breed->second.stamina);
    float speed = static_cast<float>(breed->second.speed);

    // not every species changes the breed values
    if (const auto species = m_speciesStates.find(pet.record.species); species != m_speciesStates.end())
    {
        power += static_cast<float>(species->second.power);
        stamina += static_cast<float>(species->second.stamina);
        speed += static_cast<float>(species->second.speed);
    }

    const auto quality = m_qualityMultipliers.find(pet.record.quality);
    const float multiplier = quality != m_qualityMultipliers.end() ? quality->second : 1.0f;
    const float level = static_cast<float>(pet.record.level);

    power *= multiplier * level;
    stamina *= multiplier * level;
    speed *= multiplier * level;

    pet.maxHealth = static_cast<uint32_t>(std::round(stamina / 20.0f) + 100.0f);
    pet.power = static_cast<uint32_t>(std::round(power / 100.0f));
    pet.speed = static_cast<uint32_t>(std::round(speed / 100.0f));
}

BattlePets::BattlePet BattlePetMgr::makePet(LogonComm::BattlePetRecord record) const
{
    BattlePets::BattlePet pet;
    pet.record = std::move(record);

#if VERSION_STRING >= Mop
    // pets of species this realm does not know stay in the journal, hidden and untouched
    if (const auto* species = getSpecies(pet.record.species))
    {
        pet.known = true;
        pet.creatureId = species->creatureId;
        pet.displayId = realmDisplay(species, pet.record.displayId);
        pet.noRename = (species->flags & BattlePets::SPECIES_FLAG_NO_RENAME) != 0;
    }
#endif

    calculateStats(pet);
    if (pet.record.health == 0 || pet.record.health > pet.maxHealth)
        pet.record.health = pet.maxHealth;

    return pet;
}
