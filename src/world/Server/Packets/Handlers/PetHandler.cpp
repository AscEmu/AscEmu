/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Server/Packets/CmsgDismissCritter.h"
#include "Server/Packets/CmsgPetLearnTalent.h"
#include "Server/Packets/CmsgPetCancelAura.h"
#include "Server/Packets/CmsgPetSpellAutocast.h"
#include "Server/Packets/CmsgPetUnlearn.h"
#include "Server/Packets/CmsgPetRename.h"
#include "Server/Packets/CmsgPetSetAction.h"
#include "Server/Packets/CmsgStableSwapPet.h"
#include "Server/Packets/CmsgUnstablePet.h"
#include "Server/Packets/CmsgPetNameQuery.h"
#include "Server/Packets/CmsgPetAction.h"
#include "Server/Packets/SmsgStableResult.h"
#include "Server/Packets/SmsgPetNameQuery.h"
#include "Server/Packets/SmsgPetActionSound.h"
#include "Server/WorldSession.h"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Creatures/Summons/SummonHandler.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Movement/MovementDefines.h"
#include "Movement/MovementManager.h"
#include "Objects/Units/Creatures/Vehicle.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Spell/Definitions/SpellFailure.hpp"
#include "Server/Packets/SmsgPetLearnedSpell.h"
#include "Objects/Units/ThreatHandler.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/EventMgr.h"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/SpellTarget.h"
#include "Storage/WDB/WDBStructures.hpp"

using namespace AscEmu::Packets;

void WorldSession::handlePetAction(WorldPacket& recvPacket)
{
    CmsgPetAction srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.guid.isUnit())
    {
        const auto creature = _player->getWorldMapCreature(srlPacket.guid.getGuidLowPart());
        if (creature == nullptr)
            return;

        if (srlPacket.buttonData.parts.state == PET_SPELL_STATE_SET_ACTION)
        {
            switch (srlPacket.buttonData.parts.spellId)
            {
                case PET_ACTION_ATTACK:
                {
                    if (!sEventMgr.HasEvent(_player, EVENT_PLAYER_CHARM_ATTACK))
                    {
                        uint32_t timer = creature->getBaseAttackTime(MELEE);
                        if (timer == 0)
                            timer = 2000;

                        sEventMgr.AddEvent(_player, &Player::eventCharmAttack, EVENT_PLAYER_CHARM_ATTACK, timer, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                        _player->eventCharmAttack();
                    }
                } break;
                default:
                break;
            }
        }
        return;
    }

    const auto pet = _player->getWorldMapPet(srlPacket.guid.getGuidLowPart());
    if (pet == nullptr)
        return;

    Unit* unitTarget = nullptr;
    if (srlPacket.buttonData.parts.state & PET_SPELL_STATE_DEFAULT || (srlPacket.buttonData.parts.state == PET_SPELL_STATE_SET_ACTION && srlPacket.buttonData.parts.spellId == PET_ACTION_ATTACK))
    {
        unitTarget = _player->getWorldMapUnit(srlPacket.targetguid);
        if (unitTarget == nullptr)
            unitTarget = pet;
    }

    // Intentionally create a copy of guardians to avoid invalid iterators later
    const std::vector<Summon*> summons = _player->getSummonInterface()->getSummons();
    bool alive_summon = false;
    for (auto itr = summons.cbegin(); itr != summons.cend();)
    {
        const auto guardian = (*itr);
        ++itr;

        if (!guardian->isPet())
            continue;

        const auto summonedPet = dynamic_cast<Pet*>(guardian);
        if (summonedPet == nullptr)
            continue;

        if (!summonedPet->isAlive())
            continue;

        alive_summon = true;
        const uint64_t summonedPetGuid = summonedPet->getGuid();
        switch (srlPacket.buttonData.parts.state)
        {
            case PET_SPELL_STATE_SET_ACTION:
            {
                summonedPet->setPetAction(static_cast<PetCommands>(srlPacket.buttonData.parts.spellId));
                switch (srlPacket.buttonData.parts.spellId)
                {
                    case PET_ACTION_ATTACK:
                    {
                        if (!summonedPet->getAIInterface()->canOwnerAttackUnit(unitTarget))
                        {
                            summonedPet->sendActionFeedback(PET_FEEDBACK_CANT_ATTACK_TARGET);
                            return;
                        }

                        summonedPet->getAIInterface()->setPetOwner(_player);
                        summonedPet->getMovementManager()->remove(FOLLOW_MOTION_TYPE);
                        summonedPet->getAIInterface()->attackStartUnsafe(unitTarget);
                    }
                    break;
                    case PET_ACTION_FOLLOW:
                    {
                        summonedPet->getAIInterface()->enterEvadeMode();
                    }
                    break;
                    case PET_ACTION_STAY:
                    {
                        summonedPet->getMovementManager()->remove(FOLLOW_MOTION_TYPE);
                    }
                    break;
                    case PET_ACTION_DISMISS:
                    {
                        summonedPet->abandonPet();
                    }
                    break;
                }
            }
            break;

            case PET_SPELL_STATE_PASSIVE:
            case PET_SPELL_STATE_DEFAULT:
            case PET_SPELL_STATE_AUTOCAST:
            {
                const auto spellInfo = sSpellMgr.getSpellInfo(srlPacket.buttonData.parts.spellId);
                if (spellInfo == nullptr || spellInfo->isPassive())
                    return;

                if (!summonedPet->hasSpell(spellInfo->getId()))
                    return;

                // If spell is autocasted check cooldown from ai spell data
                // TODO: pet cooldowns are not saved for non autocasted spells!
                auto* const aiSpell = summonedPet->getAIInterface()->getAISpell(spellInfo->getId());
                if (aiSpell != nullptr)
                {
                    if (!aiSpell->mCooldownTimer->isTimePassed())
                    {
                        summonedPet->sendPetCastFailed(srlPacket.buttonData.parts.spellId, SPELL_FAILED_NOT_READY);
                        return;
                    }
                }

                // Check target for hostile spells
                const auto spellTargetMask = spellInfo->getRequiredTargetMask(true);
                if (spellTargetMask & SPELL_TARGET_REQUIRE_ATTACKABLE && !(spellTargetMask & SPELL_TARGET_REQUIRE_FRIENDLY))
                {
                    if (!summonedPet->getAIInterface()->canOwnerAttackUnit(unitTarget))
                    {
                        summonedPet->sendActionFeedback(PET_FEEDBACK_CANT_ATTACK_TARGET);
                        return;
                    }
                }

                SpellCastResult castResult = SPELL_FAILED_ERROR;
                if (aiSpell != nullptr)
                {
                    // TODO: target cannot be changed?
                    castResult = summonedPet->getAIInterface()->castAISpell(aiSpell);
                }
                else
                {
                    castResult = summonedPet->castSpell(unitTarget, spellInfo);
                }

                if (castResult != SPELL_CAST_SUCCESS)
                {
                    summonedPet->sendPetCastFailed(spellInfo->getId(), castResult);
                    return;
                }

                if (spellTargetMask & SPELL_TARGET_REQUIRE_ATTACKABLE && !(spellTargetMask & SPELL_TARGET_REQUIRE_FRIENDLY))
                {
                    summonedPet->getAIInterface()->onHostileAction(unitTarget, nullptr, true);
                }
            }
            break;
            case PET_SPELL_STATE_SET_REACT:
            {
                // If set to passive pet should stop attacking and return to owner
                if (srlPacket.buttonData.parts.spellId == REACT_PASSIVE)
                    summonedPet->getAIInterface()->enterEvadeMode();

                summonedPet->getAIInterface()->setReactState(ReactStates(srlPacket.buttonData.parts.spellId));

            }
            break;
            default:
                sLogger.debug("WARNING: Unknown pet action received. State = {}, Spell = {}", static_cast<uint32_t>(srlPacket.buttonData.parts.state), static_cast<uint32_t>(srlPacket.buttonData.parts.spellId));
            break;
        }

        SendPacket(SmsgPetActionSound(summonedPetGuid, 1).serialise().get());
    }

    if (!alive_summon)
        pet->sendActionFeedback(PET_FEEDBACK_PET_DEAD);
}

void WorldSession::handlePetNameQuery(WorldPacket& recvPacket)
{
    CmsgPetNameQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto pet = _player->getWorldMapPet(srlPacket.guid.getGuidLowPart());
    if (pet == nullptr)
        return;

    SendPacket(SmsgPetNameQuery(srlPacket.petNumber, pet->getName(), pet->getPetNameTimestamp(), 0).serialise().get());
}

void WorldSession::handleStablePet(WorldPacket& /*recvPacket*/)
{
    // Get current pet or first active pet
    std::optional<uint8_t> petId = std::nullopt;
    if (const auto* pet = _player->getPet())
    {
        if (pet->isHunterPet())
            petId = pet->getPetId();
    }
    else
    {
        for (const auto& [slot, id] : _player->getPetCachedSlotMap())
        {
            if (slot >= PET_SLOT_MAX_ACTIVE_SLOT)
                break;

            const auto petCache = _player->getPetCache(id);
            if (petCache == nullptr || petCache->type != PET_TYPE_HUNTER)
                continue;

            petId = id;
            break;
        }
    }

    if (!petId.has_value())
    {
        SendPacket(SmsgStableResult(PetStableResult::Error).serialise().get());
        return;
    }

    const auto foundSlot = _player->findFreeStablePetSlot();

    // Check if player has a free stable slot
    if (!foundSlot.has_value())
    {
        SendPacket(SmsgStableResult(PetStableResult::Error).serialise().get());
        return;
    }

    if (!_player->tryPutPetToSlot(petId.value(), foundSlot.value()))
        return;

    SendPacket(SmsgStableResult(PetStableResult::StableSuccess).serialise().get());
}

static void performStableSlotSwap(Player* player, uint8_t petNumber)
{
    std::optional<uint8_t> currentPetSlot = std::nullopt;
    for (const auto& [slot, petId] : player->getPetCachedSlotMap())
    {
        if (slot >= PET_SLOT_MAX_ACTIVE_SLOT)
            break;

        // Get current pet's slot or first active pet's slot
        if (const auto* currentPet = player->getPet())
        {
            if (petId == currentPet->getPetId() && currentPet->isHunterPet())
            {
                currentPetSlot = slot;
                break;
            }
        }
        else
        {
            const auto petCache = player->getPetCache(petId);
            if (petCache == nullptr || petCache->type != PET_TYPE_HUNTER)
                continue;

            currentPetSlot = slot;
            break;
        }
    }

    if (!currentPetSlot.has_value())
    {
        player->sendPacket(SmsgStableResult(PetStableResult::Error).serialise().get());
        return;
    }

    if (!player->tryPutPetToSlot(petNumber, currentPetSlot.value()))
        return;

    player->sendPacket(SmsgStableResult(PetStableResult::UnstableSuccess).serialise().get());
}

void WorldSession::handleUnstablePet(WorldPacket& recvPacket)
{
    CmsgUnstablePet srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    // Check if player is actually performing a swap
    const auto foundSlot = _player->findFreeActivePetSlot();
    if (!foundSlot.has_value())
    {
        // Client sends unstable packet instead of swap packet if current pet is dismissed and player starts from stable slot
        performStableSlotSwap(_player, srlPacket.petNumber);
        return;
    }
    else
    {
        if (!_player->tryPutPetToSlot(srlPacket.petNumber, foundSlot.value()))
            return;

        // If pet is taken from first stable slot, client automatically shifts next pet in stables to this first slot
        // Pre-cata pet slots do not actually exist but we must update our serverside pet slots
        for (uint8_t i = PET_SLOT_FIRST_STABLE_SLOT; i < PET_SLOT_LAST_STABLE_SLOT; ++i)
        {
            if (_player->getStableSlotCount() <= (i - PET_SLOT_FIRST_STABLE_SLOT))
                break;

            if (!_player->hasPetInSlot(i))
            {
                // Found empty slot, check if there is pet in next slot and move it to this slot
                const auto nextPetId = _player->getPetIdFromSlot(i + 1);
                if (nextPetId.has_value())
                    _player->tryPutPetToSlot(nextPetId.value(), i);
            }
        }
    }

    // Summon unstabled pet if pet is alive and player is able to summon it
    if (!_player->isPetRequiringTemporaryUnsummon())
    {
        if (const auto petCache = _player->getPetCache(srlPacket.petNumber))
        {
            if (petCache->alive)
                _player->_spawnPet(petCache);
        }
    }

    SendPacket(SmsgStableResult(PetStableResult::UnstableSuccess).serialise().get());
}

void WorldSession::handleStableSwapPet(WorldPacket& recvPacket)
{
    CmsgStableSwapPet srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    // Pet number in packet is always the pet that is in stables
    performStableSlotSwap(_player, srlPacket.petNumber);
}

void WorldSession::handleBuyStableSlot(WorldPacket& /*recvPacket*/)
{
    uint32_t stable_cost = 0;

#if VERSION_STRING < Cata
    const auto stableSlotPrices = sStableSlotPricesStore.lookupEntry(_player->getStableSlotCount() + 1);

    if (stableSlotPrices != nullptr)
        stable_cost = stableSlotPrices->Price;
#endif

    if (!_player->hasEnoughCoinage(stable_cost))
    {
        SendPacket(SmsgStableResult(PetStableResult::NotEnoughMoney).serialise().get());
        return;
    }

    _player->modCoinage(-static_cast<int32_t>(stable_cost));

    SendPacket(SmsgStableResult(PetStableResult::BuySuccess).serialise().get());

    _player->m_stableSlotCount++;
}

void WorldSession::handlePetSetActionOpcode(WorldPacket& recvPacket)
{
    CmsgPetSetAction srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.srcSlot >= PET_MAX_ACTION_BAR_SLOT || srlPacket.dstSlot >= PET_MAX_ACTION_BAR_SLOT)
        return;

    auto* const pet = _player->getPet();
    if (pet == nullptr)
        return;

    // Prevent removing action or react button from bar
    if (srlPacket.srcButton.parts.state & PET_SPELL_STATE_NOT_SPELL && srlPacket.dstButton.raw == 0)
        return;

    const auto handleButton = [pet](uint32_t slot, PetActionButtonData const& button) -> void
    {
        if (button.parts.state == PET_SPELL_STATE_PASSIVE || button.parts.state == PET_SPELL_STATE_DEFAULT || button.parts.state == PET_SPELL_STATE_AUTOCAST)
        {
            if (button.parts.spellId == 0)
            {
                // Emptied a slot
                pet->setActionBarSlot(slot, 0, 0);
                return;
            }

            if (!pet->hasSpell(button.parts.spellId))
                return;

            // Updates actionbar as well
            pet->setSpellState(button.parts.spellId, button.parts.state, slot);
        }
        else
        {
            // Client seems to send sometimes weird data or packet structure is still not correct
            if (button.parts.state != PET_SPELL_STATE_SET_REACT && button.parts.state != PET_SPELL_STATE_SET_ACTION)
                pet->setActionBarSlot(slot, 0, 0);
            else
                pet->setActionBarSlot(slot, button.parts.spellId, button.parts.state);
        }
    };

    handleButton(srlPacket.srcSlot, srlPacket.srcButton);
    // Handle swap case
    if (srlPacket.dstButton.raw != 0)
        handleButton(srlPacket.dstSlot, srlPacket.dstButton);
}

void WorldSession::handlePetRename(WorldPacket& recvPacket)
{
    CmsgPetRename srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto pet = _player->getPet();
    if (pet == nullptr || pet->getGuid() != srlPacket.guid.getRawGuid())
        return;

    const std::string newName = CharacterDatabase.EscapeString(srlPacket.name);

    pet->rename(newName);

    pet->setSheathType(SHEATH_STATE_MELEE);
#if VERSION_STRING == Classic
    pet->removeUnitFlags(UNIT_FLAG_PET_CAN_BE_RENAMED);
#else
    pet->removePetFlags(PET_FLAG_CAN_BE_RENAMED);
#endif

    if (pet->getPlayerOwner() != nullptr)
    {
        if (pet->getPlayerOwner()->isPvpFlagSet())
            pet->setPvpFlag();
        else
            pet->removePvpFlag();

        if (pet->getPlayerOwner()->isFfaPvpFlagSet())
            pet->setFfaPvpFlag();
        else
            pet->removeFfaPvpFlag();

        if (pet->getPlayerOwner()->isSanctuaryFlagSet())
            pet->setSanctuaryFlag();
        else
            pet->removeSanctuaryFlag();
    }
}

void WorldSession::handlePetAbandon(WorldPacket& /*recvPacket*/)
{
    const auto pet = _player->getPet();
    if (pet == nullptr)
        return;

    pet->abandonPet();
}

void WorldSession::handlePetUnlearn(WorldPacket& recvPacket)
{
    CmsgPetUnlearn srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto pet = _player->getPet();
    if (pet == nullptr || pet->getGuid() != srlPacket.guid.getRawGuid())
        return;

#if VERSION_STRING < Mop
    const uint32_t untrainCost = pet->getUntrainCost();
    if (!_player->hasEnoughCoinage(untrainCost))
    {
        sendBuyFailed(_player->getGuid(), 0, 2);
        return;
    }
    _player->modCoinage(-static_cast<int32_t>(untrainCost));
#endif

    pet->WipeTalents();
#if VERSION_STRING == WotLK || VERSION_STRING == Cata
    pet->setPetTalentPoints(pet->getTotalTalentPoints());
    pet->SendTalentsToOwner();
#endif
}

void WorldSession::handlePetSpellAutocast(WorldPacket& recvPacket)
{
    CmsgPetSpellAutocast srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto pet = _player->getPet();
    if (pet == nullptr)
        return;

    const auto spellInfo = sSpellMgr.getSpellInfo(srlPacket.spellId);
    if (spellInfo == nullptr)
        return;

    if (!pet->hasSpell(spellInfo->getId()))
        return;

    pet->setSpellState(srlPacket.spellId, srlPacket.state > 0 ? PET_SPELL_STATE_AUTOCAST : PET_SPELL_STATE_DEFAULT);
}

void WorldSession::handlePetCancelAura(WorldPacket& recvPacket)
{
    CmsgPetCancelAura srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto spellInfo = sSpellMgr.getSpellInfo(srlPacket.spellId);
    if (spellInfo != nullptr && spellInfo->getAttributes() & static_cast<uint32_t>(ATTRIBUTES_CANT_CANCEL))
        return;

    const auto creature = _player->getWorldMapCreature(srlPacket.guid.getGuidLow());
#ifdef FT_VEHICLES
    if (creature != nullptr && (creature->getPlayerOwner() == _player  || _player->getVehicleKit() && _player->getVehicleKit()->isControler(_player)))
        creature->removeAllAurasById(srlPacket.spellId);
#else
    if (creature != nullptr && (creature->getPlayerOwner() == _player))
        creature->removeAllAurasById(srlPacket.spellId);
#endif
}

void WorldSession::handlePetLearnTalent(WorldPacket& recvPacket)
{
#if VERSION_STRING < Cata
#if VERSION_STRING > TBC
    CmsgPetLearnTalent srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto pet = _player->getPet();
    if (pet == nullptr)
        return;

    if (pet->getPetTalentPoints() < 1)
        return;

    const auto talent = sTalentStore.lookupEntry(srlPacket.talentId);
    if (talent == nullptr)
        return;

    if (talent->DependsOn > 0)
    {
        const auto depends_talent = sTalentStore.lookupEntry(talent->DependsOn);
        if (depends_talent == nullptr)
            return;

        bool req_ok = false;
        for (unsigned int i : depends_talent->RankID)
        {
            if (i != 0)
            {
                if (pet->hasSpell(i))
                {
                    req_ok = true;
                    break;
                }
            }
        }
        if (!req_ok)
            return;
    }

    if (pet->getSpentTalentPoints() < (talent->Row * 3))
        return;

    const auto spellInfo = sSpellMgr.getSpellInfo(talent->RankID[srlPacket.talentCol]);
    if (spellInfo != nullptr)
    {
        pet->addSpell(spellInfo);
        pet->setPetTalentPoints(pet->getPetTalentPoints() - 1);
    }

    pet->SendTalentsToOwner();

#endif
#else

#if VERSION_STRING < Mop
    CmsgPetLearnTalent srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto pet = _player->getPet();
    if (pet == nullptr)
        return;

    if (srlPacket.guid != pet->getGuid())
        return;

    if (pet->getPetTalentPoints() < 1)
        return;

    const auto talentEntry = sTalentStore.lookupEntry(srlPacket.talentId);
    if (talentEntry == nullptr)
        return;

    if (talentEntry->DependsOn > 0)
    {
        WDB::Structures::TalentEntry const* depends_talent = sTalentStore.lookupEntry(talentEntry->DependsOn);
        if (depends_talent == nullptr)
            return;

        bool req_ok = false;
        for (unsigned int i : depends_talent->RankID)
        {
            if (i != 0)
            {
                if (pet->hasSpell(i))
                {
                    req_ok = true;
                    break;
                }
            }
        }
        if (!req_ok)
            return;
    }

    if (pet->getSpentTalentPoints() < (talentEntry->Row * 3))
        return;

    const auto spellInfo = sSpellMgr.getSpellInfo(talentEntry->RankID[srlPacket.talentCol]);
    if (spellInfo != nullptr)
    {
        pet->addSpell(spellInfo);
        pet->setPetTalentPoints(pet->getPetTalentPoints() - 1);
    }

    pet->SendTalentsToOwner();
#endif
#endif
}

void WorldSession::handleDismissCritter(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    CmsgDismissCritter srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (_player->getCritterGuid() == 0)
    {
        sLogger.failure("Player {} sent dismiss companion packet, but player has no companion", _player->getGuidLow());
        return;
    }

    if (_player->getCritterGuid() != srlPacket.guid.getRawGuid())
    {
        sLogger.failure("Player {} sent dismiss companion packet, but it doesn't match player's companion", _player->getGuidLow());
        return;
    }

    const auto unit = _player->getWorldMapUnit(srlPacket.guid.getRawGuid());
    if (unit != nullptr)
        unit->Delete();

    _player->setCritterGuid(0);
#endif
}
