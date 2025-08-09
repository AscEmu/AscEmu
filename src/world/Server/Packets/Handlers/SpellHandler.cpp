/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatHandler.hpp"
#include "Logging/Logger.hpp"
#include "Management/Battleground/Battleground.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/SpellAura.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Creatures/Summons/Summon.hpp"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/Summons/SummonHandler.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Server/Packets/CmsgCastSpell.h"
#include "Server/Packets/CmsgPetCastSpell.h"
#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleSpellClick(WorldPacket& recvPacket)
{
    if (!_player->isAlive())
        return;

    // The guid of the unit we clicked
    uint64_t unitGuid;
    recvPacket >> unitGuid;

    Unit* unitTarget = _player->getWorldMap()->getUnit(unitGuid);
    if (!unitTarget || !unitTarget->IsInWorld() || !unitTarget->isCreature())
        return;

    if (!_player->isInRange(unitTarget, MAX_INTERACTION_RANGE))
        return;

    unitTarget->handleSpellClick(_player);

    // TODO: move this Lightwell 'script' to SpellScript or CreatureScript...
    // For future reference; seems like the Lightwell npc should actually cast spell 60123 on click
    // and this 60123 spell has Script Effect, where should be determined which rank of the Lightwell Renew needs to be casted (switch (GetCaster()->getCreatedBySpellId())...)

    // Commented this out for now, it's not even working -Appled
    /*const uint32_t lightWellCharges = 59907;
    if (creatureTarget->removeAllAurasById(lightWellCharges))
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
                sLogger.failure("Spellclick packet received for creature {} but there is no spell associated with it.", creatureTarget->getEntry());
                return;
            }

            if (!creatureTarget->hasAurasWithId(lightWellCharges))
            {
                creatureTarget->Despawn(0, 0);
            }
            return;
        }
    }*/
}

void WorldSession::handleCastSpellOpcode(WorldPacket& recvPacket)
{
    CmsgCastSpell srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto spellInfo = sSpellMgr.getSpellInfo(srlPacket.spell_id);
    if (spellInfo == nullptr)
    {
        sLogger.failure("Unknown spell id {} in handleCastSpellOpcode().", srlPacket.spell_id);
        return;
    }

    // Check does player have the spell
    if (!_player->hasSpell(srlPacket.spell_id))
    {
        sCheatLog.writefromsession(this, "WORLD: Player %u tried to cast spell %u but player does not have it.", _player->getGuidLow(), srlPacket.spell_id);
        sLogger.info("WORLD: Player {} tried to cast spell {} but player does not have it.", _player->getGuidLow(), srlPacket.spell_id);
        return;
    }

    // Check is player trying to cast a passive spell
    if (spellInfo->isPassive())
    {
        sCheatLog.writefromsession(this, "WORLD: Player %u tried to cast a passive spell %u, ignored", _player->getGuidLow(), srlPacket.spell_id);
        sLogger.info("WORLD: Player {} tried to cast a passive spell {}, ignored", _player->getGuidLow(), srlPacket.spell_id);
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

        LocationVector const spellDestination = targets.getDestination();
        LocationVector const spellSource = targets.getSource();
        float const deltaX = spellDestination.x - spellSource.x; // Calculate change of x position
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

    // You can't cancel negative auras
    if (spellAura->isNegative())
        return;

    spellAura->removeAura();
}

void WorldSession::handleCancelChannellingOpcode(WorldPacket& recvPacket)
{
    recvPacket.read_skip<uint32_t>(); // Spell Id, unused

    _player->interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL);
}

void WorldSession::handleCancelAutoRepeatSpellOpcode(WorldPacket& /*recvPacket*/)
{
    _player->interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
}

void WorldSession::handlePetCastSpell(WorldPacket& recvPacket)
{
    CmsgPetCastSpell srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto spellInfo = sSpellMgr.getSpellInfo(srlPacket.spellId);
    if (spellInfo == nullptr)
        return;

    if (_player->getPet() == nullptr && _player->getCharmGuid() == 0)
    {
        sLogger.failure("Received opcode but player {} has no pet.", _player->getGuidLow());
        return;
    }

    Unit* petUnit = _player->getWorldMap()->getUnit(srlPacket.petGuid);
    if (petUnit == nullptr)
    {
        sLogger.failure("Pet entity cannot be found for player {}.", _player->getGuidLow());
        return;
    }

    if (spellInfo->isPassive())
        return;

    // If pet is summoned by player
    if (_player->getPet() == petUnit)
    {
        // Check does the pet have the spell
        if (!dynamic_cast<Pet*>(petUnit)->HasSpell(srlPacket.spellId))
            return;
    }
    // If pet is charmed or possessed by player
    else if (_player->getCharmGuid() == srlPacket.petGuid)
    {
        bool found = false;
        for (const auto& aiSpell : petUnit->getAIInterface()->m_spells)
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
                if (const auto creatureSpellData = sCreatureSpellDataStore.lookupEntry(petCreature->GetCreatureProperties()->spelldataid))
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
        sLogger.failure("Pet doesn't belong to player {}", _player->getGuidLow());
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

        LocationVector const spellDestination = targets.getDestination();
        LocationVector const spellSource = targets.getSource();
        float const deltaX = spellDestination.x - spellSource.x; // Calculate change of x position
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
    uint8_t totemSlot;
    recvPacket >> totemSlot;

    // Clientside slot is zero indexed
    totemSlot += 1;
    if (totemSlot >= MAX_SUMMON_SLOT)
    {
        sLogger.failure("Player {} tried to cancel totem from out of range slot {}, ignored.", _player->getGuidLow(), totemSlot);
        return;
    }

    const auto summon = _player->getSummonInterface()->getSummonInSlot(static_cast<SummonSlot>(totemSlot));
    if (summon != nullptr)
        summon->unSummon();
}

void WorldSession::handleUpdateProjectilePosition(WorldPacket& recvPacket)
{
    uint64_t casterGuid;
    uint32_t spellId;
    uint8_t castCount;
    float x, y, z; // Projectile hit location

    recvPacket >> casterGuid >> spellId >> castCount >> x >> y >> z;

    Unit* caster = _player->getWorldMap()->getUnit(casterGuid);
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
    caster->sendMessageToSet(&data, true);
#endif
}
