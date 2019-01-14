/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "Management/Battleground/Battleground.h"
#include "Map/MapMgr.h"
#include "Server/MainServerDefines.h"
#include "Spell/Definitions/AuraInterruptFlags.h"
#include "Spell/Definitions/SpellCastTargetFlags.h"
#include "Spell/Definitions/SpellRanged.h"
#include "Spell/Definitions/SpellState.h"
#include "Spell/SpellMgr.h"
#include "Spell/SpellAuras.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Creatures/Pet.h"
#include "Objects/Faction.h"
#include "Data/WoWItem.h"
#include "Server/Packets/CmsgCastSpell.h"
#include "Server/Packets/CmsgPetCastSpell.h"

using namespace AscEmu::Packets;

void WorldSession::handleSpellClick(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    if (!_player->isAlive())
        return;

    // The guid of the unit we clicked
    uint64_t unitGuid;
    recvPacket >> unitGuid;

    Unit* unitTarget = _player->GetMapMgr()->GetUnit(unitGuid);
    if (!unitTarget || !unitTarget->IsInWorld() || !unitTarget->isCreature())
        return;

    auto creatureTarget = dynamic_cast<Creature*>(unitTarget);
    if (!_player->isInRange(creatureTarget, MAX_INTERACTION_RANGE))
        return;

    // TODO: investigate vehicles more, is this necessary? vehicle enter is handled in ::HandleEnterVehicle() anyway... -Appled
    if (creatureTarget->isVehicle())
    {
        if (creatureTarget->getVehicleComponent() != nullptr)
            creatureTarget->getVehicleComponent()->AddPassenger(_player);

        return;
    }

    // TODO: move this Lightwell 'script' to SpellScript or CreatureScript...
    // For future reference; seems like the Lightwell npc should actually cast spell 60123 on click
    // and this 60123 spell has Script Effect, where should be determined which rank of the Lightwell Renew needs to be casted (switch (GetCaster()->getCreatedBySpellId())...)

    // Commented this out for now, it's not even working -Appled
    /*const uint32_t lightWellCharges = 59907;
    if (creatureTarget->RemoveAura(lightWellCharges))
    {
        uint32_t lightWellRenew[] =
        {
            7001,
            27873,
            27874,
            28276,
            48084,
            48085,
            0
        };

        if (!_player->hasAurasWithId(lightWellRenew))
        {
            if (SpellClickSpell const* clickSpell = sMySQLStore.getSpellClickSpell(creatureTarget->getEntry()))
            {
                creatureTarget->castSpell(_player, clickSpell->SpellID, true);
            }
            else
            {
                sChatHandler.BlueSystemMessage(this, "NPC Id %u (%s) has no spellclick spell associated with it.", creatureTarget->GetCreatureProperties()->Id, creatureTarget->GetCreatureProperties()->Name.c_str());
                LogError("Spellclick packet received for creature %u but there is no spell associated with it.", creatureTarget->getEntry());
                return;
            }

            if (!creatureTarget->HasAura(lightWellCharges))
            {
                creatureTarget->Despawn(0, 0);
            }
            return;
        }
    }*/

    SpellClickSpell const* spellClickData = sMySQLStore.getSpellClickSpell(creatureTarget->getEntry());
    if (spellClickData != nullptr)
    {
        // TODO: there are spellclick spells which are friendly only, raid only and party only
        if (!isFriendly(_player, creatureTarget))
            return;

        const auto spellInfo = sSpellMgr.getSpellInfo(spellClickData->SpellID);
        if (spellInfo == nullptr)
        {
            LOG_ERROR("NPC ID %u has spell associated on SpellClick but spell id %u cannot be found.", creatureTarget->getEntry(), spellClickData->SpellID);
            return;
        }

        // TODO: there are spellclick spells which should be casted on player by npc (i.e. Lightwell spell) but also vice versa
        Spell* spell = sSpellMgr.newSpell(_player, spellInfo, false, nullptr);
        SpellCastTargets targets(unitGuid);
        spell->prepare(&targets);
    }
    else
    {
        sChatHandler.BlueSystemMessage(this, "NPC ID %u (%s) has no spellclick spell associated with it.", creatureTarget->GetCreatureProperties()->Id, creatureTarget->GetCreatureProperties()->Name.c_str());
        LOG_ERROR("SpellClick packet received for creature %u but there is no spell associated with it.", creatureTarget->getEntry());
        return;
    }
}

void WorldSession::handleCastSpellOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgCastSpell srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto spellInfo = sSpellMgr.getSpellInfo(srlPacket.spell_id);
    if (spellInfo == nullptr)
    {
        LOG_ERROR("Unknown spell id %u in handleCastSpellOpcode().", srlPacket.spell_id);
        return;
    }

    // Check does player have the spell
    if (!_player->HasSpell(srlPacket.spell_id))
    {
        sCheatLog.writefromsession(this, "WORLD: Player %u tried to cast spell %u but player does not have it.", _player->getGuidLow(), srlPacket.spell_id);
        LogDetail("WORLD: Player %u tried to cast spell %u but player does not have it.", _player->getGuidLow(), srlPacket.spell_id);
        return;
    }

    // Check is player trying to cast a passive spell
    if (spellInfo->isPassive())
    {
        sCheatLog.writefromsession(this, "WORLD: Player %u tried to cast a passive spell %u, ignored", _player->getGuidLow(), srlPacket.spell_id);
        LogDetail("WORLD: Player %u tried to cast a passive spell %u, ignored", _player->getGuidLow(), srlPacket.spell_id);
        return;
    }

    // Check are we already casting this autorepeat spell
    if (spellInfo->isRangedAutoRepeat() && _player->getCurrentSpell(CURRENT_AUTOREPEAT_SPELL) != nullptr
       && spellInfo == _player->getCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->getSpellInfo())
    {
        return;
    }

    // TODO: move this check to new Spell::prepare() and clean it
    if (_player->isCastingSpell(true, true, spellInfo->getId() == 75))
    {
        _player->sendCastFailedPacket(srlPacket.spell_id, SPELL_FAILED_SPELL_IN_PROGRESS, srlPacket.cast_count, 0);
        return;
    }

    SpellCastTargets targets(recvPacket, _player->getGuid());
    Spell* spell = sSpellMgr.newSpell(_player, spellInfo, false, nullptr);
    spell->extra_cast_number = srlPacket.cast_count;

#if VERSION_STRING >= Cata
    spell->m_glyphslot = srlPacket.glyphSlot;
#endif

    // Some spell cast packets include more data
    if (srlPacket.flags & 0x02)
    {
        float projectilePitch, projectileSpeed;
        uint8_t hasMovementData; // 1 or 0
        recvPacket >> projectilePitch >> projectileSpeed >> hasMovementData;

        LocationVector const spellDestination = targets.destination();
        LocationVector const spellSource = targets.source();
        float const deltaX = spellDestination.x - spellSource.y; // Calculate change of x position
        float const deltaY = spellDestination.y - spellSource.y; // Calculate change of y position

        uint32_t travelTime = 0;
        if ((projectilePitch != M_PI / 4) && (projectilePitch != -M_PI / 4)) // No division by zero
        {
            // Calculate projectile's travel time by using Pythagorean theorem to get distance from delta X and delta Y, and divide that with the projectile's velocity
            travelTime = static_cast<uint32_t>((sqrtf(deltaX * deltaX + deltaY * deltaY) / (cosf(projectilePitch) * projectileSpeed)) * 1000);
        }

        if (hasMovementData)
        {
#if VERSION_STRING < Cata
            recvPacket.SetOpcode(recvPacket.read<uint16_t>()); // MSG_MOVE_STOP
            handleMovementOpcodes(recvPacket);
#else
            recvPacket.SetOpcode(recvPacket.read<uint32_t>()); // MSG_MOVE_STOP
            handleMovementOpcodes(recvPacket);
#endif
        }

        spell->m_missilePitch = projectilePitch;
        spell->m_missileTravelTime = travelTime;
    }

    spell->prepare(&targets);
}

void WorldSession::handleCancelCastOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    uint32_t spellId;
#if VERSION_STRING > TBC
    recvPacket.read_skip<uint8_t>(); // Increments with every HandleCancelCast packet, unused
#endif
    recvPacket >> spellId;

    if (_player->isCastingSpell())
    {
        _player->interruptSpell(spellId, false);
    }
}

void WorldSession::handleCancelAuraOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    uint32_t spellId;
    recvPacket >> spellId;

    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo == nullptr)
        return;

    if (spellInfo->getAttributes() & ATTRIBUTES_CANT_CANCEL)
        return;

    // You can't cancel a passive aura
    if (spellInfo->isPassive())
        return;

    // You can't cancel an aura which is from a channeled spell, unless you are currently channeling it
    if (spellInfo->isChanneled())
    {
        if (_player->getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
        {
            if (_player->getCurrentSpell(CURRENT_CHANNELED_SPELL)->getSpellInfo()->getId() == spellId)
                _player->interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL);
        }

        return;
    }

    Aura* spellAura = _player->getAuraWithId(spellId);
    if (spellAura == nullptr)
        return;

    // You can't cancel non-positive auras
    // TODO: find better solution for this, currently in SpellAuras some auras are forced to be positive or negative, regardless of what their SpellInfo says
    // maybe new function under SpellInfo class, somewhat like this:
    /*{
        int const NEGATIVE = 1;
        int const POSITIVE = 0;
        int val = spellInfo->getAttributes() & ATTRIBUTES_NEGATIVE;
        // custom fixes to set spell/aura either positive or negative
        switch (spellInfo->getId())
        {
            case 5: // Instakill
                val = NEGATIVE;
                break;
            case 1784: // Stealth
                val = POSITIVE;
                break;
            ...
        }
        return val;
    }*/

    if (!spellAura->IsPositive())
        return;

    if (spellInfo->getAttributes() & ATTRIBUTES_NEGATIVE)
        return;

    _player->removeAllAurasById(spellId);
}

void WorldSession::handleCancelChannellingOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    recvPacket.read_skip<uint32_t>(); // Spell Id, unused

    _player->interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL);
}

void WorldSession::handleCancelAutoRepeatSpellOpcode(WorldPacket& /*recvPacket*/)
{
    CHECK_INWORLD_RETURN

    _player->interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
}

void WorldSession::handlePetCastSpell(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgPetCastSpell srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto spellInfo = sSpellMgr.getSpellInfo(srlPacket.spellId);
    if (spellInfo == nullptr)
        return;

    if (_player->GetSummon() == nullptr && _player->m_CurrentCharm == 0 && _player->getCharmGuid() == 0)
    {
        LOG_ERROR("Received opcode but player %u has no pet.", _player->getGuidLow());
        return;
    }

    Unit* petUnit = _player->GetMapMgr()->GetUnit(srlPacket.petGuid);
    if (petUnit == nullptr)
    {
        LOG_ERROR("Pet entity cannot be found for player %u.", _player->getGuidLow());
        return;
    }

    if (spellInfo->isPassive())
        return;

    // If pet is summoned by player
    if (_player->GetSummon() == petUnit)
    {
        // Check does the pet have the spell
        if (!dynamic_cast<Pet*>(petUnit)->HasSpell(srlPacket.spellId))
            return;
    }
    // If pet is charmed or possessed by player
    else if (_player->m_CurrentCharm == srlPacket.petGuid || _player->getCharmGuid() == srlPacket.petGuid)
    {
        bool found = false;
        for (auto aiSpell : petUnit->GetAIInterface()->m_spells)
        {
            if (aiSpell->spell->getId() == srlPacket.spellId)
            {
                found = true;
                break;
            }
        }

        if (!found && petUnit->isCreature())
        {
            Creature* petCreature = dynamic_cast<Creature*>(petUnit);
            if (petCreature->GetCreatureProperties()->spelldataid != 0)
            {
                if (const auto creatureSpellData = sCreatureSpellDataStore.LookupEntry(petCreature->GetCreatureProperties()->spelldataid))
                {
                    for (uint8_t i = 0; i < 3; ++i)
                    {
                        if (creatureSpellData->Spells[i] == srlPacket.spellId)
                        {
                            found = true;
                            break;
                        }
                    }
                }
            }

            if (!found)
            {
                for (uint8_t i = 0; i < 4; ++i)
                {
                    if (petCreature->GetCreatureProperties()->AISpells[i] == srlPacket.spellId)
                    {
                        found = true;
                        break;
                    }
                }
            }
        }

        if (!found)
            return;
    }
    else
    {
        LOG_ERROR("Pet doesn't belong to player %u", _player->getGuidLow());
        return;
    }

    SpellCastTargets targets(recvPacket, srlPacket.petGuid);
    Spell* spell = sSpellMgr.newSpell(petUnit, spellInfo, false, nullptr);
    spell->extra_cast_number = srlPacket.castCount;

    // Some spell cast packets include more data
    if (srlPacket.castFlags & 0x02)
    {
        float projectilePitch, projectileSpeed;
        uint8_t hasMovementData; // 1 or 0
        recvPacket >> projectilePitch >> projectileSpeed >> hasMovementData;

        LocationVector const spellDestination = targets.destination();
        LocationVector const spellSource = targets.source();
        float const deltaX = spellDestination.x - spellSource.y; // Calculate change of x position
        float const deltaY = spellDestination.y - spellSource.y; // Calculate change of y position

        uint32_t travelTime = 0;
        if ((projectilePitch != M_PI / 4) && (projectilePitch != -M_PI / 4)) // No division by zero
        {
            // Calculate projectile's travel time by using Pythagorean theorem to get distance from delta X and delta Y, and divide that with the projectile's velocity
            travelTime = static_cast<uint32_t>((sqrtf(deltaX * deltaX + deltaY * deltaY) / (cosf(projectilePitch) * projectileSpeed)) * 1000);
        }

        if (hasMovementData)
        {
            recvPacket.SetOpcode(recvPacket.read<uint16_t>()); // MSG_MOVE_STOP
            handleMovementOpcodes(recvPacket);
        }

        spell->m_missilePitch = projectilePitch;
        spell->m_missileTravelTime = travelTime;
    }

    spell->prepare(&targets);
}

void WorldSession::handleCancelTotem(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    uint8_t totemSlot;
    recvPacket >> totemSlot;

    if (totemSlot >= UNIT_SUMMON_SLOTS)
    {
        LOG_ERROR("Player %u tried to cancel summon from out of range slot %u, ignored.", _player->getGuidLow(), totemSlot);
        return;
    }

    _player->summonhandler.RemoveSummonFromSlot(totemSlot);
}

void WorldSession::handleUpdateProjectilePosition(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    uint64_t casterGuid;
    uint32_t spellId;
    uint8_t castCount;
    float x, y, z; // Projectile hit location

    recvPacket >> casterGuid >> spellId >> castCount >> x >> y >> z;

    Unit* caster = _player->GetMapMgr()->GetUnit(casterGuid);
    if (caster == nullptr)
        return;

    Spell* curSpell = _player->findCurrentCastedSpellBySpellId(spellId);
    if (curSpell == nullptr || !curSpell->m_targets.hasDestination())
        return;

    // Relocate spell
    curSpell->m_targets.setDestination(LocationVector(x, y, z));

#if VERSION_STRING > TBC
    WorldPacket data(SMSG_SET_PROJECTILE_POSITION, 21);
    data << uint64_t(casterGuid);
    data << uint8_t(castCount);
    data << float(x);
    data << float(y);
    data << float(z);
    caster->SendMessageToSet(&data, true);
#endif
}
