/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
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
#include "Units/Creatures/Pet.h"
#include "Map/MapMgr.h"
#include "Server/MainServerDefines.h"
#include "Units/Creatures/Vehicle.h"
#include "Objects/Faction.h"
#include "Spell/Definitions/SpellFailure.h"

using namespace AscEmu::Packets;

void WorldSession::handlePetAction(WorldPacket& recvPacket)
{
    CmsgPetAction srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.guid.isUnit())
    {
        const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
        if (creature == nullptr)
            return;

        if (srlPacket.action == PET_ACTION_ACTION)
        {
            switch (srlPacket.misc)
            {
                case PET_ACTION_ATTACK:
                {
                    if (!sEventMgr.HasEvent(_player, EVENT_PLAYER_CHARM_ATTACK))
                    {
                        uint32_t timer = creature->getBaseAttackTime(MELEE);
                        if (timer == 0)
                            timer = 2000;

                        sEventMgr.AddEvent(_player, &Player::_EventCharmAttack, EVENT_PLAYER_CHARM_ATTACK, timer, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                        _player->_EventCharmAttack();
                    }
                } break;
                default:
                break;
            }
        }
        return;
    }

    const auto pet = _player->GetMapMgr()->GetPet(srlPacket.guid.getGuidLowPart());
    if (pet == nullptr)
        return;

    Unit* unitTarget = nullptr;
    if (srlPacket.action == PET_ACTION_SPELL || srlPacket.action == PET_ACTION_SPELL_1 || srlPacket.action == PET_ACTION_SPELL_2 || (srlPacket.action == PET_ACTION_ACTION && srlPacket.misc == PET_ACTION_ATTACK))
    {
        unitTarget = _player->GetMapMgr()->GetUnit(srlPacket.targetguid);
        if (unitTarget == nullptr)
            unitTarget = pet;
    }

    std::list<Pet*> summons = _player->GetSummons();
    bool alive_summon = false;
    for (auto itr = summons.begin(); itr != summons.end();)
    {
        const auto summonedPet = (*itr);
        ++itr;

        if (!summonedPet->isAlive())
            continue;

        alive_summon = true;
        const uint64_t summonedPetGuid = summonedPet->getGuid();
        switch (srlPacket.action)
        {
            case PET_ACTION_ACTION:
            {
                summonedPet->SetPetAction(srlPacket.misc);
                switch (srlPacket.misc)
                {
                    case PET_ACTION_ATTACK:
                    {
                        if (unitTarget == summonedPet || !isAttackable(summonedPet, unitTarget))
                        {
                            summonedPet->SendActionFeedback(PET_FEEDBACK_CANT_ATTACK_TARGET);
                            return;
                        }

                        summonedPet->GetAIInterface()->WipeTargetList();
                        summonedPet->GetAIInterface()->WipeHateList();

                        if (summonedPet->GetAIInterface()->getUnitToFollow() == nullptr)
                            summonedPet->GetAIInterface()->SetUnitToFollow(_player);

                        summonedPet->GetAIInterface()->setAiState(AI_STATE_ATTACKING);
                        summonedPet->GetAIInterface()->AttackReaction(unitTarget, 1, 0);
                    }
                    break;
                    case PET_ACTION_FOLLOW:
                    {
                        summonedPet->GetAIInterface()->WipeTargetList();
                        summonedPet->GetAIInterface()->WipeHateList();

                        summonedPet->GetAIInterface()->SetUnitToFollow(_player);
                        summonedPet->GetAIInterface()->HandleEvent(EVENT_FOLLOWOWNER, summonedPet, 0);
                    }
                    break;
                    case PET_ACTION_STAY:
                    {
                        summonedPet->GetAIInterface()->WipeTargetList();
                        summonedPet->GetAIInterface()->WipeHateList();

                        summonedPet->GetAIInterface()->ResetUnitToFollow();
                    }
                    break;
                    case PET_ACTION_DISMISS:
                    {
                        summonedPet->Dismiss();
                    }
                    break;
                }
            }
            break;

            case PET_ACTION_SPELL_2:
            case PET_ACTION_SPELL_1:
            case PET_ACTION_SPELL:
            {
                const auto spellInfo = sSpellMgr.getSpellInfo(srlPacket.misc);
                if (spellInfo == nullptr)
                    return;

                const auto aiSpell = summonedPet->GetAISpellForSpellId(spellInfo->getId());
                if (aiSpell != nullptr)
                {
                    if (aiSpell->cooldowntime &&Util::getMSTime() < aiSpell->cooldowntime)
                    {
                        summonedPet->SendCastFailed(srlPacket.misc, SPELL_FAILED_NOT_READY);
                        return;
                    }
                    else
                    {
                        if (aiSpell->spellType != STYPE_BUFF)
                        {
                            if (unitTarget == summonedPet || !isAttackable(summonedPet, unitTarget))
                            {
                                summonedPet->SendActionFeedback(PET_FEEDBACK_CANT_ATTACK_TARGET);
                                return;
                            }
                        }

                        if (aiSpell->autocast_type != AUTOCAST_EVENT_ATTACK)
                        {
                            if (aiSpell->autocast_type == AUTOCAST_EVENT_OWNER_ATTACKED)
                                summonedPet->castSpell(_player, aiSpell->spell, false);
                            else
                                summonedPet->castSpell(summonedPet, aiSpell->spell, false);
                        }
                        else
                        {
                            summonedPet->GetAIInterface()->WipeTargetList();
                            summonedPet->GetAIInterface()->WipeHateList();

                            summonedPet->GetAIInterface()->AttackReaction(unitTarget, 1, 0);
                            summonedPet->GetAIInterface()->SetNextSpell(aiSpell);
                        }
                    }
                }
            }
            break;
            case PET_ACTION_STATE:
            {
                if (srlPacket.misc == PET_ACTION_STAY) 
                {
                    summonedPet->GetAIInterface()->WipeTargetList();
                    summonedPet->GetAIInterface()->WipeHateList();
                    summonedPet->GetAIInterface()->SetUnitToFollow(_player);
                    summonedPet->GetAIInterface()->HandleEvent(EVENT_FOLLOWOWNER, summonedPet, 0);
                }
                summonedPet->SetPetState(srlPacket.misc);

            }
            break;
            default:
                LOG_DEBUG("WARNING: Unknown pet action received. Action = %u, Misc = %u", srlPacket.action, srlPacket.misc);
            break;
        }

        SendPacket(SmsgPetActionSound(summonedPetGuid, 1).serialise().get());
    }

    if (!alive_summon)
        pet->SendActionFeedback(PET_FEEDBACK_PET_DEAD);
}

void WorldSession::handlePetNameQuery(WorldPacket& recvPacket)
{
    CmsgPetNameQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto pet = _player->GetMapMgr()->GetPet(srlPacket.guid.getGuidLowPart());
    if (pet == nullptr)
        return;

    SendPacket(SmsgPetNameQuery(srlPacket.petNumber, pet->GetName(), pet->getPetNameTimestamp(), 0).serialise().get());
}

namespace PetStableResult
{
    enum
    {
        NotEnoughMoney = 1,
        Error = 6,
        StableSuccess = 8,
        UnstableSuccess = 9,
        BuySuccess = 10
    };
}

void WorldSession::handleStablePet(WorldPacket& /*recvPacket*/)
{
    const auto pet = _player->GetSummon();
    if (pet != nullptr && pet->IsSummonedPet())
        return;

    const auto playerPet = _player->GetPlayerPet(_player->GetUnstabledPetNumber());
    if (playerPet == nullptr)
        return;

    playerPet->stablestate = STABLE_STATE_PASSIVE;

    if (pet != nullptr)
        pet->Remove(true, true);

    SendPacket(SmsgStableResult(PetStableResult::StableSuccess).serialise().get());
}

void WorldSession::handleUnstablePet(WorldPacket& recvPacket)
{
    CmsgUnstablePet srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto playerPet = _player->GetPlayerPet(srlPacket.petNumber);
    if (playerPet == nullptr)
    {
        LOG_ERROR("PET SYSTEM: Player " I64FMT " tried to unstable non-existent pet %u", _player->getGuid(), srlPacket.petNumber);
        return;
    }

    if (playerPet->alive)
        _player->SpawnPet(srlPacket.petNumber);

    playerPet->stablestate = STABLE_STATE_ACTIVE;

    SendPacket(SmsgStableResult(PetStableResult::UnstableSuccess).serialise().get());
}

void WorldSession::handleStableSwapPet(WorldPacket& recvPacket)
{
    CmsgStableSwapPet srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto playerPet = _player->GetPlayerPet(srlPacket.petNumber);
    if (playerPet == nullptr)
    {
        LOG_ERROR("PET SYSTEM: Player " I64FMT " tried to unstable non-existent pet %u", _player->getGuid(), srlPacket.petNumber);
        return;
    }

    const auto pet = _player->GetSummon();
    if (pet != nullptr && pet->IsSummonedPet())
        return;

    const auto playerPet2 = _player->GetPlayerPet(_player->GetUnstabledPetNumber());
    if (playerPet2 == nullptr)
        return;

    if (pet != nullptr)
        pet->Remove(true, true);

    playerPet2->stablestate = STABLE_STATE_PASSIVE;

    if (playerPet->alive)
        _player->SpawnPet(srlPacket.petNumber);

    playerPet->stablestate = STABLE_STATE_ACTIVE;

    SendPacket(SmsgStableResult(PetStableResult::UnstableSuccess).serialise().get());
}

void WorldSession::handleBuyStableSlot(WorldPacket& /*recvPacket*/)
{
    uint32_t stable_cost = 0;

#if VERSION_STRING < Cata
    const auto stableSlotPrices = sStableSlotPricesStore.LookupEntry(_player->GetStableSlotCount() + 1);

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

    _player->m_StableSlotCount++;
}

void WorldSession::handlePetSetActionOpcode(WorldPacket& recvPacket)
{
    CmsgPetSetAction srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (!_player->GetSummon())
        return;

    const auto pet = _player->GetSummon();
    const auto spellInfo = sSpellMgr.getSpellInfo(srlPacket.spell);
    if (spellInfo == nullptr)
        return;

    const auto petSpellMap = pet->GetSpells()->find(spellInfo);
    if (petSpellMap == pet->GetSpells()->end())
        return;

    pet->ActionBar[srlPacket.slot] = srlPacket.spell;
    pet->SetSpellState(srlPacket.spell, srlPacket.state);
}

void WorldSession::handlePetRename(WorldPacket& recvPacket)
{
    CmsgPetRename srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    Pet* pet = nullptr;
    std::list<Pet*> summons = _player->GetSummons();
    for (auto summon : summons)
    {
        if (summon->getGuid() == srlPacket.guid.GetOldGuid())
        {
            pet = summon;
            break;
        }
    }

    if (pet == nullptr)
        return;

    const std::string newName = CharacterDatabase.EscapeString(srlPacket.name);

    pet->Rename(newName);

    pet->setSheathType(SHEATH_STATE_MELEE);
    pet->setPetFlags(PET_RENAME_NOT_ALLOWED);

    ARCEMU_ASSERT(pet->getPlayerOwner() != nullptr);

    if (dynamic_cast<Player*>(pet->getPlayerOwner())->isPvpFlagSet())
        pet->setPvpFlag();
    else
        pet->removePvpFlag();

    if (dynamic_cast<Player*>(pet->getPlayerOwner())->isFfaPvpFlagSet())
        pet->setFfaPvpFlag();
    else
        pet->removeFfaPvpFlag();

    if (dynamic_cast<Player*>(pet->getPlayerOwner())->isSanctuaryFlagSet())
        pet->setSanctuaryFlag();
    else
        pet->removeSanctuaryFlag();
}

void WorldSession::handlePetAbandon(WorldPacket& /*recvPacket*/)
{
    const auto pet = _player->GetSummon();
    if (pet == nullptr)
        return;

    pet->Dismiss();
}

void WorldSession::handlePetUnlearn(WorldPacket& recvPacket)
{
    CmsgPetUnlearn srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto pet = _player->GetSummon();
    if (pet == nullptr || pet->getGuid() != srlPacket.guid.GetOldGuid())
        return;

    const uint32_t untrainCost = pet->GetUntrainCost();
    if (!_player->hasEnoughCoinage(untrainCost))
    {
        sendBuyFailed(_player->getGuid(), 0, 2);
        return;
    }
    _player->modCoinage(-static_cast<int32_t>(untrainCost));

    pet->WipeTalents();
    pet->setPetTalentPoints(pet->GetTPsForLevel(pet->getLevel()));
    pet->SendTalentsToOwner();
}

void WorldSession::handlePetSpellAutocast(WorldPacket& recvPacket)
{
    CmsgPetSpellAutocast srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto spellInfo = sSpellMgr.getSpellInfo(srlPacket.spellId);
    if (spellInfo == nullptr)
        return;

    std::list<Pet*> summons = _player->GetSummons();
    for (auto summon : summons)
    {
        const auto petSpell = summon->GetSpells()->find(spellInfo);
        if (petSpell == summon->GetSpells()->end())
            continue;

        summon->SetSpellState(srlPacket.spellId, srlPacket.state > 0 ? AUTOCAST_SPELL_STATE : DEFAULT_SPELL_STATE);
    }
}
void WorldSession::handlePetCancelAura(WorldPacket& recvPacket)
{
    CmsgPetCancelAura srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto spellInfo = sSpellMgr.getSpellInfo(srlPacket.spellId);
    if (spellInfo != nullptr && spellInfo->getAttributes() & static_cast<uint32_t>(ATTRIBUTES_CANT_CANCEL))
        return;

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLow());
    if (creature != nullptr && (creature->getPlayerOwner() == _player || _player->getCurrentVehicle() && _player->getCurrentVehicle()->IsControler(_player)))
        creature->RemoveAura(srlPacket.spellId);
}

#if VERSION_STRING < Cata
#if VERSION_STRING > TBC
void WorldSession::handlePetLearnTalent(WorldPacket& recvPacket)
{
    CmsgPetLearnTalent srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto pet = _player->GetSummon();
    if (pet == nullptr)
        return;

    if (pet->getPetTalentPoints() < 1)
        return;

    const auto talent = sTalentStore.LookupEntry(srlPacket.talentId);
    if (talent == nullptr)
        return;

    if (talent->DependsOn > 0)
    {
        const auto depends_talent = sTalentStore.LookupEntry(talent->DependsOn);
        if (depends_talent == nullptr)
            return;

        bool req_ok = false;
        for (unsigned int i : depends_talent->RankID)
        {
            if (i != 0)
            {
                if (pet->HasSpell(i))
                {
                    req_ok = true;
                    break;
                }
            }
        }
        if (!req_ok)
            return;
    }

    if (pet->GetSpentTPs() < (talent->Row * 3))
        return;

    if (srlPacket.talentCol > 0 && talent->RankID[srlPacket.talentCol - 1] != 0)
        pet->RemoveSpell(talent->RankID[srlPacket.talentCol - 1]);

    const auto spellInfo = sSpellMgr.getSpellInfo(talent->RankID[srlPacket.talentCol]);
    if (spellInfo != nullptr)
    {
        pet->AddSpell(spellInfo, true);
        pet->setPetTalentPoints(pet->getPetTalentPoints() - 1);
        auto id = spellInfo->getId();
        OutPacket(SMSG_PET_LEARNED_SPELL, 4, &id);
    }

    pet->SendTalentsToOwner();
}
#endif
#else
void WorldSession::handlePetLearnTalent(WorldPacket& recvPacket)
{
    CmsgPetLearnTalent srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto pet = _player->GetSummon();
    if (pet == nullptr)
        return;

    if (srlPacket.guid != pet->getGuid())
        return;

    if (pet->getPetTalentPoints() < 1)
        return;

    const auto talentEntry = sTalentStore.LookupEntry(srlPacket.talentId);
    if (talentEntry == nullptr)
        return;

    if (talentEntry->DependsOn > 0)
    {
        DBC::Structures::TalentEntry const* depends_talent = sTalentStore.LookupEntry(talentEntry->DependsOn);
        if (depends_talent == nullptr)
            return;

        bool req_ok = false;
        for (unsigned int i : depends_talent->RankID)
        {
            if (i != 0)
            {
                if (pet->HasSpell(i))
                {
                    req_ok = true;
                    break;
                }
            }
        }
        if (!req_ok)
            return;
    }

    if (pet->GetSpentTPs() < (talentEntry->Row * 3))
        return;

    if (srlPacket.talentCol > 0 && talentEntry->RankID[srlPacket.talentCol - 1] != 0)
        pet->RemoveSpell(talentEntry->RankID[srlPacket.talentCol - 1]);

    const auto spellInfo = sSpellMgr.getSpellInfo(talentEntry->RankID[srlPacket.talentCol]);
    if (spellInfo != nullptr)
    {
        pet->AddSpell(spellInfo, true);
        pet->setPetTalentPoints(pet->getPetTalentPoints() - 1);
    }

    pet->SendTalentsToOwner();
}
#endif

void WorldSession::handleDismissCritter(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    CmsgDismissCritter srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (_player->getCritterGuid() == 0)
    {
        LOG_ERROR("Player %u sent dismiss companion packet, but player has no companion", _player->getGuidLow());
        return;
    }

    if (_player->getCritterGuid() != srlPacket.guid.GetOldGuid())
    {
        LOG_ERROR("Player %u sent dismiss companion packet, but it doesn't match player's companion", _player->getGuidLow());
        return;
    }

    const auto unit = _player->GetMapMgr()->GetUnit(srlPacket.guid.GetOldGuid());
    if (unit != nullptr)
        unit->Delete();

    _player->setCritterGuid(0);
#endif
}
