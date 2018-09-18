/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
    CmsgPetAction recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (GET_TYPE_FROM_GUID(recv_packet.guid.GetOldGuid()) == HIGHGUID_TYPE_UNIT)
    {
        const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLowPart());
        if (creature == nullptr)
            return;

        if (recv_packet.action == PET_ACTION_ACTION)
        {
            switch (recv_packet.misc)
            {
                case PET_ACTION_ATTACK:
                {
                    if (!sEventMgr.HasEvent(GetPlayer(), EVENT_PLAYER_CHARM_ATTACK))
                    {
                        uint32_t timer = creature->getBaseAttackTime(MELEE);
                        if (timer == 0)
                            timer = 2000;

                        sEventMgr.AddEvent(GetPlayer(), &Player::_EventCharmAttack, EVENT_PLAYER_CHARM_ATTACK, timer, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                        GetPlayer()->_EventCharmAttack();
                    }
                } break;
                default:
                break;
            }
        }
        return;
    }

    const auto pet = GetPlayer()->GetMapMgr()->GetPet(recv_packet.guid.getGuidLowPart());
    if (pet == nullptr)
        return;

    Unit* unitTarget = nullptr;
    if (recv_packet.action == PET_ACTION_SPELL || recv_packet.action == PET_ACTION_SPELL_1 || recv_packet.action == PET_ACTION_SPELL_2 || (recv_packet.action == PET_ACTION_ACTION && recv_packet.misc == PET_ACTION_ATTACK))
    {
        unitTarget = GetPlayer()->GetMapMgr()->GetUnit(recv_packet.targetguid);
        if (unitTarget == nullptr)
            unitTarget = pet;
    }

    std::list<Pet*> summons = GetPlayer()->GetSummons();
    bool alive_summon = false;
    for (auto itr = summons.begin(); itr != summons.end();)
    {
        const auto summonedPet = (*itr);
        ++itr;

        if (!summonedPet->isAlive())
            continue;

        alive_summon = true;
        const uint64_t summonedPetGuid = summonedPet->getGuid();
        switch (recv_packet.action)
        {
            case PET_ACTION_ACTION:
            {
                summonedPet->SetPetAction(recv_packet.misc);
                switch (recv_packet.misc)
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
                            summonedPet->GetAIInterface()->SetUnitToFollow(GetPlayer());

                        summonedPet->GetAIInterface()->setAiState(AI_STATE_ATTACKING);
                        summonedPet->GetAIInterface()->AttackReaction(unitTarget, 1, 0);
                    }
                    break;
                    case PET_ACTION_FOLLOW:
                    {
                        summonedPet->GetAIInterface()->WipeTargetList();
                        summonedPet->GetAIInterface()->WipeHateList();

                        summonedPet->GetAIInterface()->SetUnitToFollow(GetPlayer());
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
                const auto spellInfo = sSpellCustomizations.GetSpellInfo(recv_packet.misc);
                if (spellInfo == nullptr)
                    return;

                const auto aiSpell = summonedPet->GetAISpellForSpellId(spellInfo->getId());
                if (aiSpell != nullptr)
                {
                    if (aiSpell->cooldowntime &&Util::getMSTime() < aiSpell->cooldowntime)
                    {
                        summonedPet->SendCastFailed(recv_packet.misc, SPELL_FAILED_NOT_READY);
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
                                summonedPet->CastSpell(GetPlayer(), aiSpell->spell, false);
                            else
                                summonedPet->CastSpell(summonedPet, aiSpell->spell, false);
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
                if (recv_packet.misc == PET_ACTION_STAY) 
                {
                    summonedPet->GetAIInterface()->WipeTargetList();
                    summonedPet->GetAIInterface()->WipeHateList();
                    summonedPet->GetAIInterface()->SetUnitToFollow(GetPlayer());
                    summonedPet->GetAIInterface()->HandleEvent(EVENT_FOLLOWOWNER, summonedPet, 0);
                }
                summonedPet->SetPetState(recv_packet.misc);

            }
            break;
            default:
                LOG_DEBUG("WARNING: Unknown pet action received. Action = %u, Misc = %u", recv_packet.action, recv_packet.misc);
            break;
        }

        SendPacket(SmsgPetActionSound(summonedPetGuid, 1).serialise().get());
    }

    if (!alive_summon)
        pet->SendActionFeedback(PET_FEEDBACK_PET_DEAD);
}

void WorldSession::handlePetNameQuery(WorldPacket& recvPacket)
{
    CmsgPetNameQuery recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto pet = GetPlayer()->GetMapMgr()->GetPet(recv_packet.guid.getGuidLowPart());
    if (pet == nullptr)
        return;

    SendPacket(SmsgPetNameQuery(recv_packet.petNumber, pet->GetName(), pet->getUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP), 0).serialise().get());
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
    const auto pet = GetPlayer()->GetSummon();
    if (pet != nullptr && pet->IsSummonedPet())
        return;

    const auto playerPet = GetPlayer()->GetPlayerPet(GetPlayer()->GetUnstabledPetNumber());
    if (playerPet == nullptr)
        return;

    playerPet->stablestate = STABLE_STATE_PASSIVE;

    if (pet != nullptr)
        pet->Remove(true, true);

    SendPacket(SmsgStableResult(PetStableResult::StableSuccess).serialise().get());
}

void WorldSession::handleUnstablePet(WorldPacket& recvPacket)
{
    CmsgUnstablePet recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto playerPet = GetPlayer()->GetPlayerPet(recv_packet.petNumber);
    if (playerPet == nullptr)
    {
        LOG_ERROR("PET SYSTEM: Player " I64FMT " tried to unstable non-existent pet %u", GetPlayer()->getGuid(), recv_packet.petNumber);
        return;
    }

    if (playerPet->alive)
        GetPlayer()->SpawnPet(recv_packet.petNumber);

    playerPet->stablestate = STABLE_STATE_ACTIVE;

    SendPacket(SmsgStableResult(PetStableResult::UnstableSuccess).serialise().get());
}

void WorldSession::handleStableSwapPet(WorldPacket& recvPacket)
{
    CmsgStableSwapPet recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto playerPet = GetPlayer()->GetPlayerPet(recv_packet.petNumber);
    if (playerPet == nullptr)
    {
        LOG_ERROR("PET SYSTEM: Player " I64FMT " tried to unstable non-existent pet %u", GetPlayer()->getGuid(), recv_packet.petNumber);
        return;
    }

    const auto pet = GetPlayer()->GetSummon();
    if (pet != nullptr && pet->IsSummonedPet())
        return;

    const auto playerPet2 = GetPlayer()->GetPlayerPet(GetPlayer()->GetUnstabledPetNumber());
    if (playerPet2 == nullptr)
        return;

    if (pet != nullptr)
        pet->Remove(true, true);

    playerPet2->stablestate = STABLE_STATE_PASSIVE;

    if (playerPet->alive)
        GetPlayer()->SpawnPet(recv_packet.petNumber);

    playerPet->stablestate = STABLE_STATE_ACTIVE;

    SendPacket(SmsgStableResult(PetStableResult::UnstableSuccess).serialise().get());
}

void WorldSession::handleBuyStableSlot(WorldPacket& /*recvPacket*/)
{
    uint32_t stable_cost = 0;

#if VERSION_STRING != Cata
    const auto stableSlotPrices = sStableSlotPricesStore.LookupEntry(GetPlayer()->GetStableSlotCount() + 1);

    if (stableSlotPrices != nullptr)
        stable_cost = stableSlotPrices->Price;
#endif

    if (!GetPlayer()->HasGold(stable_cost))
    {
        SendPacket(SmsgStableResult(PetStableResult::NotEnoughMoney).serialise().get());
        return;
    }

    GetPlayer()->ModGold(-static_cast<int32_t>(stable_cost));

    SendPacket(SmsgStableResult(PetStableResult::BuySuccess).serialise().get());

    GetPlayer()->m_StableSlotCount++;
}

void WorldSession::handlePetSetActionOpcode(WorldPacket& recvPacket)
{
    CmsgPetSetAction recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (!GetPlayer()->GetSummon())
        return;

    const auto pet = GetPlayer()->GetSummon();
    const auto spellInfo = sSpellCustomizations.GetSpellInfo(recv_packet.spell);
    if (spellInfo == nullptr)
        return;

    const auto petSpellMap = pet->GetSpells()->find(spellInfo);
    if (petSpellMap == pet->GetSpells()->end())
        return;

    pet->ActionBar[recv_packet.slot] = recv_packet.spell;
    pet->SetSpellState(recv_packet.spell, recv_packet.state);
}

void WorldSession::handlePetRename(WorldPacket& recvPacket)
{
    CmsgPetRename recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    Pet* pet = nullptr;
    std::list<Pet*> summons = GetPlayer()->GetSummons();
    for (auto summon : summons)
    {
        if (summon->getGuid() == recv_packet.guid.GetOldGuid())
        {
            pet = summon;
            break;
        }
    }

    if (pet == nullptr)
        return;

    const std::string newName = CharacterDatabase.EscapeString(recv_packet.name);

    pet->Rename(newName);

    pet->setSheathType(SHEATH_STATE_MELEE);
    pet->setPetFlags(PET_RENAME_NOT_ALLOWED);

    ARCEMU_ASSERT(pet->GetPetOwner() != nullptr);

    if (pet->GetPetOwner()->IsPvPFlagged())
        pet->SetPvPFlag();
    else
        pet->RemovePvPFlag();

    if (pet->GetPetOwner()->IsFFAPvPFlagged())
        pet->SetFFAPvPFlag();
    else
        pet->RemoveFFAPvPFlag();

    if (pet->GetPetOwner()->IsSanctuaryFlagged())
        pet->SetSanctuaryFlag();
    else
        pet->RemoveSanctuaryFlag();
}

void WorldSession::handlePetAbandon(WorldPacket& /*recvPacket*/)
{
    const auto pet = GetPlayer()->GetSummon();
    if (pet == nullptr)
        return;

    pet->Dismiss();
}

void WorldSession::handlePetUnlearn(WorldPacket& recvPacket)
{
    CmsgPetUnlearn recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto pet = GetPlayer()->GetSummon();
    if (pet == nullptr || pet->getGuid() != recv_packet.guid.GetOldGuid())
        return;

    const uint32_t untrainCost = pet->GetUntrainCost();
    if (!GetPlayer()->HasGold(untrainCost))
    {
        SendBuyFailed(GetPlayer()->getGuid(), 0, 2);
        return;
    }
    GetPlayer()->ModGold(-static_cast<int32_t>(untrainCost));

    pet->WipeTalents();
    pet->setPetTalentPoints(pet->GetTPsForLevel(pet->getLevel()));
    pet->SendTalentsToOwner();
}

void WorldSession::handlePetSpellAutocast(WorldPacket& recvPacket)
{
    CmsgPetSpellAutocast recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto spellInfo = sSpellCustomizations.GetSpellInfo(recv_packet.spellId);
    if (spellInfo == nullptr)
        return;

    std::list<Pet*> summons = GetPlayer()->GetSummons();
    for (auto summon : summons)
    {
        const auto petSpell = summon->GetSpells()->find(spellInfo);
        if (petSpell == summon->GetSpells()->end())
            continue;

        summon->SetSpellState(recv_packet.spellId, recv_packet.state > 0 ? AUTOCAST_SPELL_STATE : DEFAULT_SPELL_STATE);
    }
}
void WorldSession::handlePetCancelAura(WorldPacket& recvPacket)
{
    CmsgPetCancelAura recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto spellInfo = sSpellCustomizations.GetSpellInfo(recv_packet.spellId);
    if (spellInfo != nullptr && spellInfo->getAttributes() & static_cast<uint32_t>(ATTRIBUTES_CANT_CANCEL))
        return;

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLow());
    if (creature != nullptr && (creature->GetPlayerOwner() == GetPlayer() || GetPlayer()->GetCurrentVehicle() && GetPlayer()->GetCurrentVehicle()->IsControler(GetPlayer())))
        creature->RemoveAura(recv_packet.spellId);
}

#if VERSION_STRING != Cata
#if VERSION_STRING > TBC
void WorldSession::handlePetLearnTalent(WorldPacket& recvPacket)
{
    CmsgPetLearnTalent recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto pet = GetPlayer()->GetSummon();
    if (pet == nullptr)
        return;

    if (pet->getPetTalentPoints() < 1)
        return;

    const auto talent = sTalentStore.LookupEntry(recv_packet.talentId);
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

    if (recv_packet.talentCol > 0 && talent->RankID[recv_packet.talentCol - 1] != 0)
        pet->RemoveSpell(talent->RankID[recv_packet.talentCol - 1]);

    const auto spellInfo = sSpellCustomizations.GetSpellInfo(talent->RankID[recv_packet.talentCol]);
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
    CmsgPetLearnTalent recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto pet = GetPlayer()->GetSummon();
    if (pet == nullptr)
        return;

    if (recv_packet.guid != pet->getGuid())
        return;

    if (pet->getPetTalentPoints() < 1)
        return;

    const auto talentEntry = sTalentStore.LookupEntry(recv_packet.talentId);
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

    if (recv_packet.talentCol > 0 && talentEntry->RankID[recv_packet.talentCol - 1] != 0)
        pet->RemoveSpell(talentEntry->RankID[recv_packet.talentCol - 1]);

    const auto spellInfo = sSpellCustomizations.GetSpellInfo(talentEntry->RankID[recv_packet.talentCol]);
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
    CmsgDismissCritter recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (GetPlayer()->getCritterGuid() == 0)
    {
        LOG_ERROR("Player %u sent dismiss companion packet, but player has no companion", GetPlayer()->getGuidLow());
        return;
    }

    if (GetPlayer()->getCritterGuid() != recv_packet.guid.GetOldGuid())
    {
        LOG_ERROR("Player %u sent dismiss companion packet, but it doesn't match player's companion", GetPlayer()->getGuidLow());
        return;
    }

    const auto unit = GetPlayer()->GetMapMgr()->GetUnit(recv_packet.guid.GetOldGuid());
    if (unit != nullptr)
        unit->Delete();

    GetPlayer()->setCritterGuid(0);
#endif
}
