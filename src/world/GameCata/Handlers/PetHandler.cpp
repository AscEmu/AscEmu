/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Objects/Faction.h"
#include "Server/WorldSession.h"
#include "Units/Creatures/Pet.h"
#include "../../../scripts/Battlegrounds/AlteracValley.h"
#include "Objects/ObjectMgr.h"

//\todo Rewrite for cata - after this all functions are copied from wotlk

void WorldSession::HandlePetAction(WorldPacket& recvData)
{
    //WorldPacket data;
    uint64 petGuid = 0;
    uint16 misc = 0;
    uint16 action = 0;
    uint64 targetguid = 0;

    recvData >> petGuid;
    recvData >> misc;
    recvData >> action;
    //recv_data.hexlike();

    //printf("Pet_Action: 0x%.4X 0x%.4X\n", misc, action);

    if (GET_TYPE_FROM_GUID(petGuid) == HIGHGUID_TYPE_UNIT)
    {
        Creature* pCharm = GetPlayer()->GetMapMgr()->GetCreature(GET_LOWGUID_PART(petGuid));
        if (!pCharm)
            return;

        // must be a mind controlled creature..
        if (action == PET_ACTION_ACTION)
        {
            recvData >> targetguid;
            switch (misc)
            {
            case PET_ACTION_ATTACK:
            {
                if (!sEventMgr.HasEvent(_player, EVENT_PLAYER_CHARM_ATTACK))
                {
                    uint32 timer = pCharm->GetBaseAttackTime(MELEE);
                    if (!timer) timer = 2000;

                    sEventMgr.AddEvent(_player, &Player::_EventCharmAttack, EVENT_PLAYER_CHARM_ATTACK, timer, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                    _player->_EventCharmAttack();
                }
            }
            break;
            }
        }
        return;
    }

    Pet* pPet = _player->GetMapMgr()->GetPet(GET_LOWGUID_PART(petGuid));

    if (!pPet)
        return;

    Unit* pTarget = nullptr;

    if (action == PET_ACTION_SPELL || action == PET_ACTION_SPELL_1 || action == PET_ACTION_SPELL_2 || (action == PET_ACTION_ACTION && misc == PET_ACTION_ATTACK))  // >> target
    {
        recvData >> targetguid;
        pTarget = _player->GetMapMgr()->GetUnit(targetguid);
        if (!pTarget) pTarget = pPet;    // target self
    }

    std::list<Pet*> summons = _player->GetSummons();
    bool alive_summon = false;
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end();)
    {
        pPet = (*itr);
        ++itr;
        if (!pPet->isAlive())
            continue;
        alive_summon = true;//we found a an alive summon
        uint64 GUID = pPet->GetGUID();
        switch (action)
        {
        case PET_ACTION_ACTION:
        {
            pPet->SetPetAction(misc);       // set current action
            switch (misc)
            {
            case PET_ACTION_ATTACK:
            {
                // make sure the target is attackable
                if (pTarget == pPet || !isAttackable(pPet, pTarget))
                {
                    pPet->SendActionFeedback(PET_FEEDBACK_CANT_ATTACK_TARGET);
                    return;
                }

                // Clear the threat
                pPet->GetAIInterface()->WipeTargetList();
                pPet->GetAIInterface()->WipeHateList();

                // Attack target with melee if the owner if we don't have spells - other wise cast. All done by AIInterface.
                if (pPet->GetAIInterface()->getUnitToFollow() == nullptr)
                    pPet->GetAIInterface()->SetUnitToFollow(_player);

                // EVENT_PET_ATTACK
                pPet->GetAIInterface()->setAiState(AI_STATE_ATTACKING);
                pPet->GetAIInterface()->AttackReaction(pTarget, 1, 0);
            }
            break;
            case PET_ACTION_FOLLOW:
            {
                // Clear the threat
                pPet->GetAIInterface()->WipeTargetList();
                pPet->GetAIInterface()->WipeHateList();

                // Follow the owner... run to him...
                pPet->GetAIInterface()->SetUnitToFollow(_player);
                pPet->GetAIInterface()->HandleEvent(EVENT_FOLLOWOWNER, pPet, 0);
            }
            break;
            case PET_ACTION_STAY:
            {
                // Clear the threat
                pPet->GetAIInterface()->WipeTargetList();
                pPet->GetAIInterface()->WipeHateList();

                // Stop following the owner, and sit.
                pPet->GetAIInterface()->ResetUnitToFollow();
            }
            break;
            case PET_ACTION_DISMISS:
            {
                // Bye byte...
                pPet->Dismiss();
            }
            break;
            }
        }
        break;

        case PET_ACTION_SPELL_2:
        case PET_ACTION_SPELL_1:
        case PET_ACTION_SPELL:
        {
            // misc == spellid
            SpellInfo* entry = sSpellCustomizations.GetSpellInfo(misc);
            if (entry == nullptr)
                return;

            AI_Spell* sp = pPet->GetAISpellForSpellId(entry->getId());
            if (sp != nullptr)
            {
                // Check the cooldown
                if (sp->cooldowntime &&Util::getMSTime() < sp->cooldowntime)
                {
                    pPet->SendCastFailed(misc, SPELL_FAILED_NOT_READY);
                    return;
                }
                else
                {
                    if (sp->spellType != STYPE_BUFF)
                    {
                        // make sure the target is attackable
                        if (pTarget == pPet || !isAttackable(pPet, pTarget))
                        {
                            pPet->SendActionFeedback(PET_FEEDBACK_CANT_ATTACK_TARGET);
                            return;
                        }
                    }

                    if (sp->autocast_type != AUTOCAST_EVENT_ATTACK)
                    {
                        if (sp->autocast_type == AUTOCAST_EVENT_OWNER_ATTACKED)
                            pPet->CastSpell(_player, sp->spell, false);
                        else
                            pPet->CastSpell(pPet, sp->spell, false);
                    }
                    else
                    {
                        // Clear the threat
                        pPet->GetAIInterface()->WipeTargetList();
                        pPet->GetAIInterface()->WipeHateList();

                        pPet->GetAIInterface()->AttackReaction(pTarget, 1, 0);
                        pPet->GetAIInterface()->SetNextSpell(sp);
                    }
                }
            }
        }
        break;
        case PET_ACTION_STATE:
        {
            if (misc == PET_ACTION_STAY)        // PET_STATE_PASSIVE
            {
                // stop attacking and run to owner
                pPet->GetAIInterface()->WipeTargetList();
                pPet->GetAIInterface()->WipeHateList();
                pPet->GetAIInterface()->SetUnitToFollow(_player);
                pPet->GetAIInterface()->HandleEvent(EVENT_FOLLOWOWNER, pPet, 0);
            }
            pPet->SetPetState(misc);

        }
        break;
        default:
        {
            LOG_DEBUG("WARNING: Unknown pet action received. Action = %.4X, Misc = %.4X", action, misc);
        }
        break;
        }

        // Send pet action sound - WHEE THEY TALK
        WorldPacket actionp(SMSG_PET_ACTION_SOUND, 12);
        actionp << GUID << uint32(1);                       //should we send only 1 sound for all the pets?
        SendPacket(&actionp);
    }
    if (!alive_summon)
    {
        pPet->SendActionFeedback(PET_FEEDBACK_PET_DEAD);
        return;
    }
}

void WorldSession::HandlePetInfo(WorldPacket& /*recv_data*/)
{
    // nothing
    LOG_DEBUG("HandlePetInfo is called");

    // uint32_t petNumber = 0;
    // uint64_t petGuid = 0;

    // recv_data >> petNumber;
    // recv_data >> petGuid;
}

void WorldSession::HandlePetNameQuery(WorldPacket& recv_data)
{
    uint32_t petNumber = 0;
    uint64_t petGuid = 0;

    recv_data >> petNumber;
    recv_data >> petGuid;

    Pet* pPet = _player->GetMapMgr()->GetPet(GET_LOWGUID_PART(petGuid));
    if (!pPet)
        return;

    WorldPacket data(SMSG_PET_NAME_QUERY_RESPONSE, 10 + pPet->GetName().size());
    data << uint32_t(petNumber);
    data << pPet->GetName().c_str();
    data << pPet->getUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP);
    data << uint8_t(0);
    SendPacket(&data);
}

void WorldSession::HandleStablePet(WorldPacket& /*recv_data*/)
{
    // remove pet from world and association with player
    Pet* pPet = _player->GetSummon();
    if (pPet != nullptr && pPet->IsSummonedPet())
        return;

    PlayerPet* pet = _player->GetPlayerPet(_player->GetUnstabledPetNumber());
    if (!pet)
        return;
    pet->stablestate = STABLE_STATE_PASSIVE;

    if (pPet != nullptr)       //if pPet is NULL here then the pet is dead and we relogged.
        pPet->Remove(true, true);

    WorldPacket data(1);
    data.SetOpcode(SMSG_STABLE_RESULT);
    data << uint8(0x8);     // success
    SendPacket(&data);
}

void WorldSession::HandleUnstablePet(WorldPacket& recvData)
{
    uint64 npcguid = 0;
    uint32 petnumber = 0;

    recvData >> npcguid;
    recvData >> petnumber;

    PlayerPet* pet = _player->GetPlayerPet(petnumber);
    if (!pet)
    {
        LOG_ERROR("PET SYSTEM: Player " I64FMT " tried to unstable non-existent pet %d", _player->GetGUID(), petnumber);
        return;
    }
    //unstable selected pet but spawn it only if it's alive
    if (pet->alive)
        _player->SpawnPet(petnumber);
    pet->stablestate = STABLE_STATE_ACTIVE;

    WorldPacket data(1);
    data.SetOpcode(SMSG_STABLE_RESULT);
    data << uint8(0x9);     // success?
    SendPacket(&data);
}

void WorldSession::HandleStableSwapPet(WorldPacket& recvData)
{
    uint64 npcguid = 0;
    uint32 petnumber = 0;

    recvData >> npcguid;
    recvData >> petnumber;

    PlayerPet* pet = _player->GetPlayerPet(petnumber);
    if (!pet)
    {
        LOG_ERROR("PET SYSTEM: Player " I64FMT " tried to unstable non-existent pet %d", _player->GetGUID(), petnumber);
        return;
    }
    Pet* pPet = _player->GetSummon();
    if (pPet != nullptr && pPet->IsSummonedPet())
        return;

    //stable current pet
    PlayerPet* pet2 = _player->GetPlayerPet(_player->GetUnstabledPetNumber());
    if (!pet2)
        return;

    if (pPet != nullptr)       //if pPet is NULL here then the pet is dead and we relogged.
        pPet->Remove(true, true);
    pet2->stablestate = STABLE_STATE_PASSIVE;

    //unstable selected pet but spawn it only if it's alive
    if (pet->alive)
        _player->SpawnPet(petnumber);
    pet->stablestate = STABLE_STATE_ACTIVE;

    WorldPacket data;
    data.SetOpcode(SMSG_STABLE_RESULT);
    data << uint8(0x09);
    SendPacket(&data);
}

void WorldSession::HandleStabledPetList(WorldPacket& recvData)
{
    uint64 npcguid = 0;
    recvData >> npcguid;

    if (_player->getClass() != HUNTER)
    {
        Arcemu::Gossip::Menu::SendSimpleMenu(npcguid, 13584, _player);
        return;
    }

    SendStabledPetList(npcguid);
}

void WorldSession::HandleBuyStableSlot(WorldPacket& /*recv_data*/)
{
    int32 stableCost = 0;

    WorldPacket data(SMSG_STABLE_RESULT, 1);

    if (!_player->HasGold(stableCost))
    {
        data << uint8(1);       // not enough money
        SendPacket(&data);
        return;
    }
    _player->ModGold(-stableCost);

    data << uint8(0x0A);
    SendPacket(&data);

    _player->m_StableSlotCount++;
}

void WorldSession::HandlePetSetActionOpcode(WorldPacket& recvData)
{
    uint64 guid;
    uint32 slot;
    uint16 spell;
    uint16 state;

    recvData >> guid;
    recvData >> slot;
    recvData >> spell;
    recvData >> state;

    if (!_player->GetSummon())
        return;

    Pet* pet = _player->GetSummon();
    SpellInfo* spe = sSpellCustomizations.GetSpellInfo(spell);
    if (spe == nullptr)
        return;

    // do we have the spell? if not don't set it (exploit fix)
    PetSpellMap::iterator itr = pet->GetSpells()->find(spe);
    if (itr == pet->GetSpells()->end())
        return;

    pet->ActionBar[slot] = spell;
    pet->SetSpellState(spell, state);
}

void WorldSession::HandlePetRename(WorldPacket& recvData)
{
    uint64 guid;
    std::string name;

    recvData >> guid;
    recvData >> name;

    Pet* pet = nullptr;
    std::list<Pet*> summons = _player->GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        if ((*itr)->GetGUID() == guid)
        {
            pet = (*itr);
            break;
        }
    }
    if (pet == nullptr)
    {
        sChatHandler.SystemMessage(this, "That pet is not your current pet, or you do not have a pet.");
        return;
    }
    name = CharacterDatabase.EscapeString(name);

    pet->Rename(name);

    // Disable pet rename.
    pet->setUInt32Value(UNIT_FIELD_BYTES_2, 1 | /* (0x28 << 8) | */ (PET_RENAME_NOT_ALLOWED << 16));

    ARCEMU_ASSERT(pet->GetPetOwner() != NULL);

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

void WorldSession::HandlePetAbandon(WorldPacket& /*recv_data*/)
{
    Pet* pet = _player->GetSummon();
    if (!pet) return;

    pet->Dismiss();
}

void WorldSession::HandlePetUnlearn(WorldPacket& recv_data)
{
    uint64 guid;
    recv_data >> guid;

    Pet* pPet = _player->GetSummon();
    if (pPet == nullptr || pPet->GetGUID() != guid)
    {
        sChatHandler.SystemMessage(this, "That pet is not your current pet, or you do not have a pet.");
        return;
    }

    int32 cost = pPet->GetUntrainCost();
    if (!_player->HasGold(cost))
    {
        WorldPacket data(SMSG_BUY_FAILED, 12);
        data << uint64(_player->GetGUID());
        data << uint32(0);
        data << uint8(2);        //not enough money
        SendPacket(&data);
        return;
    }
    _player->ModGold(-cost);

    pPet->WipeTalents();
    pPet->SetTPs(pPet->GetTPsForLevel(pPet->getLevel()));
    pPet->SendTalentsToOwner();
}

void WorldSession::HandlePetSpellAutocast(WorldPacket& recvPacket)
{
    // handles toggle autocast from spellbook
    uint64 guid;
    uint16 spellid;
    uint16 unk;
    uint8  state;
    recvPacket >> guid >> spellid >> unk >> state;

    SpellInfo* spe = sSpellCustomizations.GetSpellInfo(spellid);
    if (spe == nullptr)
        return;

    std::list<Pet*> summons = _player->GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        // do we have the spell? if not don't set it (exploit fix)
        PetSpellMap::iterator itr2 = (*itr)->GetSpells()->find(spe);
        if (itr2 == (*itr)->GetSpells()->end())
            continue;

        (*itr)->SetSpellState(spellid, state > 0 ? AUTOCAST_SPELL_STATE : DEFAULT_SPELL_STATE);
    }
}
void WorldSession::HandlePetCancelAura(WorldPacket& recvPacket)
{
    uint64 guid;
    uint32 spellid;

    recvPacket >> guid >> spellid;

    SpellInfo* info = sSpellCustomizations.GetSpellInfo(spellid);
    if (info != nullptr && info->getAttributes() & static_cast<uint32>(ATTRIBUTES_CANT_CANCEL))
        return;
    Creature* pet = _player->GetMapMgr()->GetCreature(static_cast<uint32>(guid));
    if (pet != nullptr && (pet->GetPlayerOwner() == _player || _player->GetCurrentVehicle() && _player->GetCurrentVehicle()->IsControler(_player)))
        pet->RemoveAura(spellid);
}
