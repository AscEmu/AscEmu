/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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

    // Cheat Detection only if player and not from an item
    // this could fuck up things but meh it's needed ALOT of the newbs are using WPE now
    // WPE allows them to mod the outgoing packet and basically choose what ever spell they want :(

    if (!GetPlayer()->HasSpell(spellId))
    {
        sCheatLog.writefromsession(this, "Cast spell %lu but doesn't have that spell.", spellId);
        LogDetail("WORLD: Spell isn't cast because player \'%s\' is cheating", GetPlayer()->GetName());
        return;
    }
    if (spellInfo->IsPassive())
    {
        sCheatLog.writefromsession(this, "Cast passive spell %lu.", spellId);
        LogDetail("WORLD: Spell isn't cast because player \'%s\' is cheating", GetPlayer()->GetName());
        return;
    }

    if (GetPlayer()->GetOnMeleeSpell() != spellId)
    {
        //autoshot 75
        if ((spellInfo->AttributesExB & ATTRIBUTESEXB_ACTIVATE_AUTO_SHOT) /*spellInfo->Attributes == 327698*/)	// auto shot..
        {
            LogDebugFlag(LF_SPELL, "HandleCastSpellOpcode : Auto Shot-type spell cast (id %u, name %s)", spellInfo->Id, spellInfo->Name.c_str());
            Item* weapon = GetPlayer()->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
            if (!weapon)
                return;

            uint32_t spellid;
            switch (weapon->GetItemProperties()->SubClass)
            {
                case 2:			 // bows
                case 3:			 // guns
                case 18:		 // crossbow
                    spellid = SPELL_RANGED_GENERAL;
                    break;
                case 16:			// thrown
                    spellid = SPELL_RANGED_THROW;
                    break;
                case 19:			// wands
                    spellid = SPELL_RANGED_WAND;
                    break;
                default:
                    spellid = 0;
                    break;
            }

            if (!spellid)
                spellid = spellInfo->Id;

            if (!_player->m_onAutoShot)
            {
                _player->m_AutoShotTarget = _player->GetSelection();
                uint32_t duration = _player->GetBaseAttackTime(RANGED);
                SpellCastTargets targets(recvPacket, GetPlayer()->GetGUID());
                if (!targets.m_unitTarget)
                {
                    LogDebugFlag(LF_SPELL, "HandleCastSpellOpcode : Cancelling auto-shot cast because targets.m_unitTarget is null!");
                    return;
                }
                SpellInfo* sp = sSpellCustomizations.GetSpellInfo(spellid);

                _player->m_AutoShotSpell = sp;
                _player->m_AutoShotDuration = duration;
                //This will fix fast clicks
                if (_player->m_AutoShotAttackTimer < 500)
                    _player->m_AutoShotAttackTimer = 500;

                _player->m_onAutoShot = true;
            }
            
            return;
        }

        if (_player->m_currentSpell)
        {
            if (_player->m_currentSpell->getState() == SPELL_STATE_CASTING)
            {
                // cancel the existing channel spell, cast this one
                _player->m_currentSpell->cancel();
            }
            else
            {
                // send the error message
                _player->SendCastResult(spellInfo->Id, SPELL_FAILED_SPELL_IN_PROGRESS, castCount, 0);
                return;
            }
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

    if (GetPlayer()->m_currentSpell)
        GetPlayer()->m_currentSpell->cancel();
}

void WorldSession::HandleCancelAuraOpcode(WorldPacket& recvPacket)
{
    uint32_t spellId;
    recvPacket >> spellId;

    // do not cancel ghost auras
    if (spellId == 8326 || spellId == 9036)
        return;

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
}

void WorldSession::HandleCancelChannellingOpcode(WorldPacket& recvPacket)
{
    uint32_t spellId;
    recvPacket >> spellId;

    Player* plyr = GetPlayer();
    if (!plyr)
        return;
    if (plyr->m_currentSpell)
    {
        plyr->m_currentSpell->cancel();
    }
}

void WorldSession::HandleCancelAutoRepeatSpellOpcode(WorldPacket& recv_data)
{
    LogDebugFlag(LF_OPCODE, "Received CMSG_CANCEL_AUTO_REPEAT_SPELL message.");
    //on original we automatically enter combat when creature got close to us
    //	GetPlayer()->GetSession()->OutPacket(SMSG_CANCEL_COMBAT);
    GetPlayer()->m_onAutoShot = false;
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
