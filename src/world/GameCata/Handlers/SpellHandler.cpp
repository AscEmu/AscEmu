/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "Management/Battleground/Battleground.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/SpellCastTargetFlags.h"
#include <Spell/Definitions/AuraInterruptFlags.h>
#include "Spell/Definitions/SpellRanged.h"
#include "Spell/Definitions/SpellState.h"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Units/Creatures/Pet.h"

void WorldSession::HandleCastSpellOpcode(WorldPacket& recvPacket)
{
    uint32_t spellId;
    uint8_t castCount;
    uint32_t glyphSlot;
    uint8_t missileflag;

    recvPacket >> castCount;
    recvPacket >> spellId;
    recvPacket >> glyphSlot;
    recvPacket >> missileflag;

    // check for spell id
    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);
    if (!spellInfo)
    {
        LogError("WORLD: unknown spell id %i", spellId);
        return;
    }

    if (!_player->isAlive() && _player->GetShapeShift() != FORM_SPIRITOFREDEMPTION && !(spellInfo->Attributes & ATTRIBUTES_DEAD_CASTABLE)) //They're dead, not in spirit of redemption and the spell can't be cast while dead.
        return;

    LogDetail("WORLD: got cast spell packet, spellId - %i (%s), data length = %i", spellId, spellInfo->Name.c_str(), recvPacket.size());

    // Check does player have the spell
    if (!GetPlayer()->HasSpell(spellId))
    {
        sCheatLog.writefromsession(this, "Cast spell %lu but doesn't have that spell.", spellId);
        LogDetail("WORLD: Spell isn't cast because player \'%s\' is cheating", GetPlayer()->GetName());
        return;
    }

    // Check is player trying to cast a passive spell
    if (spellInfo->IsPassive())
    {
        sCheatLog.writefromsession(this, "Cast passive spell %lu.", spellId);
        LogDetail("WORLD: Spell isn't cast because player \'%s\' is cheating", GetPlayer()->GetName());
        return;
    }

    // Check are we already casting this autorepeat spell
    if ((spellInfo->getAttributesExB() & ATTRIBUTESEXB_AUTOREPEAT) && _player->getCurrentSpell(CURRENT_AUTOREPEAT_SPELL) != nullptr
        && spellInfo == _player->getCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->GetSpellInfo())
    {
        return;
    }

    // TODO: move this check to new Spell::prepare() and clean it
    if (_player->isCastingNonMeleeSpell(false, true, true, spellInfo->getId() == 75))
    {
        _player->SendCastResult(spellId, SPELL_FAILED_SPELL_IN_PROGRESS, castCount, 0);
        return;
    }

    SpellCastTargets targets(recvPacket, GetPlayer()->GetGUID());

    // some anticheat stuff
    if (spellInfo->custom_self_cast_only)
    {
        if (targets.m_unitTarget && targets.m_unitTarget != _player->GetGUID())
        {
            // send the error message
            _player->SendCastResult(spellInfo->Id, SPELL_FAILED_BAD_TARGETS, castCount, 0);
            return;
        }
    }

    Spell* spell = sSpellFactoryMgr.NewSpell(GetPlayer(), spellInfo, false, nullptr);
    spell->extra_cast_number = castCount;
    spell->m_glyphslot = glyphSlot;
    spell->prepare(&targets);
}


void WorldSession::HandleSpellClick(WorldPacket& recvPacket)
{
    LOG_DETAIL("WORLD: got CMSG_SPELLCLICK packet, data length = %i", recvPacket.size());

    if (_player->getDeathState() == CORPSE)
        return;

    uint64_t unitGuid; // this will store the guid of the object we are going to use it's spell. There must be a dbc that indicates what spells a unit has

    recvPacket >> unitGuid;

    //we have only 1 example atm for entry : 28605
    Unit* unitTarget = _player->GetMapMgr()->GetUnit(unitGuid);

    if (!unitTarget)
        return;

    if (!_player->isInRange(unitTarget, MAX_INTERACTION_RANGE))
        return;

    if (unitTarget->IsVehicle())
    {
        if (unitTarget->GetVehicleComponent() != nullptr)
            unitTarget->GetVehicleComponent()->AddPassenger(_player);
        return;
    }

    uint32_t creature_id = unitTarget->GetEntry();
    uint32_t cast_spell_id = 0;

    if (unitTarget->RemoveAura(59907))
    {
        uint32 lightwellRenew[] =
        {
            //SPELL_HASH_LIGHTWELL_RENEW
            7001,
            27873,
            27874,
            28276,
            48084,
            48085,
            60123,
            0
        };

        if (!_player->hasAurasWithId(lightwellRenew))
        {
            SpellClickSpell const* sp = sMySQLStore.getSpellClickSpell(creature_id);
            if (sp == nullptr)
            {
                if (unitTarget->IsCreature())
                {
                    Creature* c = static_cast<Creature*>(unitTarget);

                    sChatHandler.BlueSystemMessage(this, "NPC Id %u (%s) has no spellclick spell associated with it.", c->GetCreatureProperties()->Id, c->GetCreatureProperties()->Name.c_str());
                    LOG_ERROR("Spellclick packet received for creature %u but there is no spell associated with it.", creature_id);
                    return;
                }
            }
            else
            {
                cast_spell_id = sp->SpellID;
                unitTarget->CastSpell(_player, cast_spell_id, true);
            }

            if (!unitTarget->HasAura(59907))
                static_cast<Creature*>(unitTarget)->Despawn(0, 0); //IsCreature() check is not needed, refer to r2387 and r3230

            return;
        }
    }

    SpellClickSpell const* sp = sMySQLStore.getSpellClickSpell(creature_id);
    if (sp == nullptr)
    {
        if (unitTarget->IsCreature())
        {
            Creature* c = static_cast< Creature* >(unitTarget);

            sChatHandler.BlueSystemMessage(this, "NPC Id %u (%s) has no spellclick spell associated with it.", c->GetCreatureProperties()->Id, c->GetCreatureProperties()->Name.c_str());
            LOG_ERROR("Spellclick packet received for creature %u but there is no spell associated with it.", creature_id);
            return;
        }
    }
    else
    {
        cast_spell_id = sp->SpellID;

        SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(cast_spell_id);
        if (spellInfo == nullptr)
            return;

        Spell* spell = sSpellFactoryMgr.NewSpell(_player, spellInfo, false, nullptr);
        SpellCastTargets targets(unitGuid);
        spell->prepare(&targets);
    }
}

void WorldSession::HandleCancelCastOpcode(WorldPacket& recvPacket)
{
    uint32_t spellId;
    recvPacket.read_skip<uint8_t>();
    recvPacket >> spellId;

    if (_player->isCastingNonMeleeSpell(false))
    {
        _player->interruptSpell(spellId, false, false);
    }
}

void WorldSession::HandleCancelAuraOpcode(WorldPacket& recvPacket)
{
    uint32_t spellId;
    recvPacket >> spellId;

    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);
    if (spellInfo == nullptr)
    {
        return;
    }

    if (spellInfo->getAttributes() & ATTRIBUTES_CANT_CANCEL)
    {
        return;
    }

    if (spellInfo->getAttributesEx() & (ATTRIBUTESEX_CHANNELED_1 | ATTRIBUTESEX_CHANNELED_2))
    {
        if (_player->getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
        {
            if (_player->getCurrentSpell(CURRENT_CHANNELED_SPELL)->GetSpellInfo()->getId() == spellId)
                _player->interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL);
        }
        return;
    }

    if (spellInfo->IsPassive())
    {
        return;
    }

    Aura* spellAura = _player->getAuraWithId(spellId);
    if (spellAura == nullptr)
    {
        return;
    }

    if (!spellAura->IsPositive())
    {
        return;
    }

    if (spellInfo->getAttributes() & ATTRIBUTES_NEGATIVE)
    {
        return;
    }

    _player->removeAllAurasById(spellId);


    /*
    if (_player->m_currentSpell && _player->m_currentSpell->GetSpellInfo()->getId() == spellId)
        _player->m_currentSpell->cancel();
    else
    {
        SpellInfo* info = sSpellCustomizations.GetSpellInfo(spellId);
        if (info == nullptr)
            return;

        Aura* aura = _player->getAuraWithId(spellId);
        if (aura)
        {
            if (!aura->IsPositive())
                return;
            if (info->getAttributes() & ATTRIBUTES_NEGATIVE)
                return;
        }
        if (!(info->getAttributes() & static_cast<uint32>(ATTRIBUTES_CANT_CANCEL)))
        {
            _player->removeAllAurasById(spellId);
            LOG_DEBUG("Removing all auras with ID: %u", spellId);
        }
    }
    */
}

void WorldSession::HandleCancelChannellingOpcode(WorldPacket& recvPacket)
{
    uint32_t spellId;
    recvPacket >> spellId;

    _player->interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL);
}

void WorldSession::HandleCancelAutoRepeatSpellOpcode(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "Received CMSG_CANCEL_AUTO_REPEAT_SPELL message.");
    //on original we automatically enter combat when creature got close to us
    //	GetPlayer()->GetSession()->OutPacket(SMSG_CANCEL_COMBAT);
    _player->interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
}

void WorldSession::HandleCancelTotem(WorldPacket& recv_data)
{
    uint8_t slot;
    recv_data >> slot;

    if (slot >= UNIT_SUMMON_SLOTS)
    {
        LOG_ERROR("Player %u %s tried to cancel a summon at slot %u, slot number is out of range. (tried to crash the server?)", _player->GetLowGUID(), _player->GetName(), slot);
        return;
    }

    _player->summonhandler.RemoveSummonFromSlot(slot);
}

void WorldSession::HandleUpdateProjectilePosition(WorldPacket& recv_data)
{
    uint64_t casterGuid;          // guid of the caster
    uint32_t spellId;             // spell ID of casted spell
    uint8_t castCount;            // count how many times it is/was cast
    float x, y, z;                // missile hit position

    casterGuid = recv_data.unpackGUID();
    recv_data >> spellId;
    recv_data >> castCount;
    recv_data >> x;
    recv_data >> y;
    recv_data >> z;

    LogDebugFlag(LF_OPCODE, "Recieved spell: %u, count: %i, position: x(%f) y(%f) z(%f)", spellId, castCount, x, y, z);

    SpellInfo* spell = Spell::checkAndReturnSpellEntry(spellId);
    if (!spell || spell->ai_target_type == TARGET_FLAG_DEST_LOCATION)
        return;

    WorldPacket data(SMSG_SET_PROJECTILE_POSITION, 21);
    data << uint64_t(casterGuid);
    data << uint8_t(castCount);
    data << float(x);
    data << float(y);
    data << float(z);
    SendPacket(&data);
}
