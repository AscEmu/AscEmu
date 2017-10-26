/*
Copyright (c) 2017 AscEmu Team <http://www.ascemu.org/>
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
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Spell/SpellAuras.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Creatures/Pet.h"

void WorldSession::HandleUseItemOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    uint8_t containerIndex, inventorySlot, castCount, castFlags;
    uint32_t spellId, glyphIndex;
    uint64_t itemGuid;

    recvPacket >> containerIndex >> inventorySlot >> castCount >> spellId >> itemGuid >> glyphIndex >> castFlags;

    Item* tmpItem = _player->GetItemInterface()->GetInventoryItem(containerIndex, inventorySlot);
    if (tmpItem == nullptr)
    {
        tmpItem = _player->GetItemInterface()->GetInventoryItem(inventorySlot);
    }

    if (tmpItem == nullptr)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    if (tmpItem->GetGUID() != itemGuid)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    ItemProperties const* itemProto = tmpItem->GetItemProperties();
    if (!itemProto)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    // Equipable items needs to be equipped before use
    if (itemProto->InventoryType != INVTYPE_NON_EQUIP && !_player->GetItemInterface()->IsEquipped(itemProto->ItemId))
    {
        // todo: is this correct error msg?
        _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    if (!_player->isAlive())
    {
        _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_YOU_ARE_DEAD);
        return;
    }

    if (tmpItem->IsSoulbound() && tmpItem->GetOwnerGUID() != _player->GetGUID() && !tmpItem->IsAccountbound())
    {
        _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_DONT_OWN_THAT_ITEM);
        return;
    }

    if ((itemProto->Flags2 & ITEM_FLAG2_HORDE_ONLY) && _player->GetTeam() != TEAM_HORDE ||
        (itemProto->Flags2 & ITEM_FLAG2_ALLIANCE_ONLY) && _player->GetTeam() != TEAM_ALLIANCE)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM);
        return;
    }

    if ((itemProto->AllowableClass & _player->getClassMask()) == 0 || (itemProto->AllowableRace & _player->getRaceMask()) == 0)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM);
        return;
    }

    if (itemProto->RequiredSkill)
    {
        if (!_player->_HasSkillLine(itemProto->RequiredSkill))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_NO_REQUIRED_PROFICIENCY);
            return;
        }
        else if (_player->_GetSkillLineCurrent(itemProto->RequiredSkill) < itemProto->RequiredSkillRank)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_SKILL_ISNT_HIGH_ENOUGH);
            return;
        }
    }

    if (itemProto->RequiredSkillSubRank != 0 && !_player->HasSpell(itemProto->RequiredSkillSubRank))
    {
        _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_NO_REQUIRED_PROFICIENCY);
        return;
    }

    if (_player->getLevel() < itemProto->RequiredLevel)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_YOU_MUST_REACH_LEVEL_N);
        return;
    }

    // Learning spells (mounts, companion pets etc)
    if (itemProto->Spells[0].Id == 483 || itemProto->Spells[0].Id == 55884)
    {
        if (_player->HasSpell(itemProto->Spells[1].Id))
            // No error message, handled elsewhere
            return;
    }

    if (itemProto->RequiredFaction && uint32_t(_player->GetStanding(itemProto->RequiredFaction)) < itemProto->RequiredFactionStanding)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_ITEM_REPUTATION_NOT_ENOUGH);
        return;
    }

    // Arena cases
    if (_player->m_bg != nullptr && isArena(_player->m_bg->GetType()))
    {
        // Not all consumables are usable in arena
        if (itemProto->Class == ITEM_CLASS_CONSUMABLE && !itemProto->HasFlag(ITEM_FLAG_USEABLE_IN_ARENA))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_NOT_DURING_ARENA_MATCH);
            return;
        }

        // Not all items are usable in arena
        if (itemProto->HasFlag(ITEM_FLAG_NOT_USEABLE_IN_ARENA))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_NOT_DURING_ARENA_MATCH);
            return;
        }
    }

    if (itemProto->Bonding == ITEM_BIND_ON_USE || itemProto->Bonding == ITEM_BIND_ON_PICKUP || itemProto->Bonding == ITEM_BIND_QUEST)
    {
        if (!tmpItem->IsSoulbound())
            tmpItem->SoulBind();
    }

    // Combat check
    if (_player->getcombatstatus()->IsInCombat())
    {
        for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (SpellInfo const* spellInfo = sSpellCustomizations.GetSpellInfo(itemProto->Spells[i].Id))
            {
                if (spellInfo->getAttributes() & ATTRIBUTES_REQ_OOC)
                {
                    _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_CANT_DO_IN_COMBAT);
                    return;
                }
            }
        }
    }

    // Start quest
    if (itemProto->QuestId)
    {
        QuestProperties const* quest = sMySQLStore.getQuestProperties(itemProto->QuestId);
        if (!quest)
            return;

        // Create packet
        WorldPacket data;
        sQuestMgr.BuildQuestDetails(&data, quest, tmpItem, 0, language, _player);
        SendPacket(&data);
    }

    // Anticheat to prevent WDB editing
    bool found = false;
    for (uint8_t i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (itemProto->Spells[i].Trigger == USE && itemProto->Spells[i].Id == spellId)
        {
            found = true;
            break;
        }
    }

    if (tmpItem->HasOnUseSpellID(spellId))
        found = true;

    // Item doesn't have this spell, either player is cheating or player's itemcache.wdb doesn't match with database
    if (!found)
    {
        Disconnect();
        Anticheat_Log->writefromsession(this, "Player tried to use an item with a spell that didn't match the spell in the database.");
        Anticheat_Log->writefromsession(this, "Possibly corrupted or intentionally altered itemcache.wdb");
        Anticheat_Log->writefromsession(this, "Itemid: %lu", itemProto->ItemId);
        Anticheat_Log->writefromsession(this, "Spellid: %lu", spellId);
        Anticheat_Log->writefromsession(this, "Player was disconnected.");
        return;
    }

    // Call item scripts
    if (sScriptMgr.CallScriptedItem(tmpItem, _player))
        return;

    SpellCastTargets targets(recvPacket, _player->GetGUID());
    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);
    if (spellInfo == nullptr)
    {
        LogError("WORLD: Unknown spell id %i in ::HandleUseItemOpcode() from item id %i", spellId, itemProto->ItemId);
        return;
    }

    // TODO: remove this and get rid of 'ForcedPetId' hackfix
    // move the spells from MySQLDataStore.cpp to SpellScript
    if (itemProto->ForcedPetId >= 0)
    {
        if (itemProto->ForcedPetId == 0)
        {
            if (targets.m_unitTarget != _player->GetGUID())
            {
                _player->SendCastResult(spellInfo->getId(), SPELL_FAILED_BAD_TARGETS, castCount, 0);
                return;
            }
        }
        else
        {
            if (!_player->GetSummon() || _player->GetSummon()->GetEntry() != (uint32_t)itemProto->ForcedPetId)
            {
                _player->SendCastResult(spellInfo->getId(), SPELL_FAILED_BAD_TARGETS, castCount, 0);
                return;
            }
        }
    }

    // TODO: move this to rewritten canCast() and castMe()
    if (spellInfo->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP && !_player->IsSitting())
    {
        if (_player->CombatStatus.IsInCombat() || _player->IsMounted())
        {
            _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_CANT_DO_IN_COMBAT);
            return;
        }
        else
        {
            _player->SetStandState(STANDSTATE_SIT);
        }
    }

    Spell* spell = sSpellFactoryMgr.NewSpell(_player, spellInfo, false, nullptr);
    spell->extra_cast_number = castCount;
    spell->i_caster = tmpItem;
    spell->m_glyphslot = glyphIndex;

    // Some spell cast packets include more data
    if (castFlags & 0x02)
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
            recvPacket.SetOpcode(recvPacket.read<uint32_t>()); // MSG_MOVE_STOP
            HandleMovementOpcodes(recvPacket);
        }

        spell->m_missilePitch = projectilePitch;
        spell->m_missileTravelTime = travelTime;
    }

    spell->prepare(&targets);

#if VERSION_STRING > TBC
    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM, itemProto->ItemId, 0, 0);
#endif
}

#if VERSION_STRING != Cata
void WorldSession::HandleSpellClick(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    if (!_player->isAlive())
    {
        return;
    }

    // The guid of the unit we clicked
    uint64_t unitGuid;
    recvPacket >> unitGuid;

    Unit* unitTarget = _player->GetMapMgr()->GetUnit(unitGuid);
    if (!unitTarget || !unitTarget->IsInWorld() || !unitTarget->IsCreature())
    {
        return;
    }

    Creature* creatureTarget = static_cast<Creature*>(unitTarget);
    if (!_player->isInRange(creatureTarget, MAX_INTERACTION_RANGE))
    {
        return;
    }

    // TODO: investigate vehicles more, is this necessary? vehicle enter is handled in ::HandleEnterVehicle() anyway... -Appled
    if (creatureTarget->IsVehicle())
    {
        if (creatureTarget->GetVehicleComponent() != nullptr)
            creatureTarget->GetVehicleComponent()->AddPassenger(_player);
        return;
    }

    // TODO: move this Lightwell 'script' to SpellScript or CreatureScript...
    // For future reference; seems like the Lightwell npc should actually cast spell 60123 on click
    // and this 60123 spell has Script Effect, where should be determined which rank of the Lightwell Renew needs to be casted (switch (GetCaster()->GetCreatedBySpell())...)

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
            if (SpellClickSpell const* clickSpell = sMySQLStore.getSpellClickSpell(creatureTarget->GetEntry()))
            {
                creatureTarget->CastSpell(_player, clickSpell->SpellID, true);
            }
            else
            {
                sChatHandler.BlueSystemMessage(this, "NPC Id %u (%s) has no spellclick spell associated with it.", creatureTarget->GetCreatureProperties()->Id, creatureTarget->GetCreatureProperties()->Name.c_str());
                LogError("Spellclick packet received for creature %u but there is no spell associated with it.", creatureTarget->GetEntry());
                return;
            }

            if (!creatureTarget->HasAura(lightWellCharges))
            {
                creatureTarget->Despawn(0, 0);
            }
            return;
        }
    }*/

    SpellClickSpell const* spellClickData = sMySQLStore.getSpellClickSpell(creatureTarget->GetEntry());
    if (spellClickData != nullptr)
    {
        // TODO: there are spellclick spells which are friendly only, raid only and party only
        if (!isFriendly(_player, creatureTarget))
            return;

        SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellClickData->SpellID);
        if (spellInfo == nullptr)
        {
            LogError("NPC ID %u has spell associated on SpellClick but spell id %u cannot be found.", creatureTarget->GetEntry(), spellClickData->SpellID);
            return;
        }

        // TODO: there are spellclick spells which should be casted on player by npc (i.e. Lightwell spell) but also vice versa
        Spell* spell = sSpellFactoryMgr.NewSpell(_player, spellInfo, false, nullptr);
        SpellCastTargets targets(unitGuid);
        spell->prepare(&targets);
    }
    else
    {
        sChatHandler.BlueSystemMessage(this, "NPC ID %u (%s) has no spellclick spell associated with it.", creatureTarget->GetCreatureProperties()->Id, creatureTarget->GetCreatureProperties()->Name.c_str());
        LogError("SpellClick packet received for creature %u but there is no spell associated with it.", creatureTarget->GetEntry());
        return;
    }
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleCastSpellOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    uint32_t spellId;
    uint8_t castCount, castFlags;

    recvPacket >> castCount >> spellId >> castFlags;

    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);
    if (spellInfo == nullptr)
    {
        LogError("WORLD: Unknown spell id %u in HandleCastSpellOpcode().", spellId);
        return;
    }

    // Check does player have the spell
    if (!_player->HasSpell(spellId))
    {
        sCheatLog.writefromsession(this, "WORLD: Player %u tried to cast spell %u but player does not have it.", _player->GetLowGUID(), spellId);
        LogDetail("WORLD: Player %u tried to cast spell %u but player does not have it.", _player->GetLowGUID(), spellId);
        return;
    }

    // Check is player trying to cast a passive spell
    if (spellInfo->IsPassive())
    {
        sCheatLog.writefromsession(this, "WORLD: Player %u tried to cast a passive spell %u, ignored", _player->GetLowGUID(), spellId);
        LogDetail("WORLD: Player %u tried to cast a passive spell %u, ignored", _player->GetLowGUID(), spellId);
        return;
    }

    // TODO: Autorepeat spells (such as Auto Shot and Shoot (wand)) require full rework. maybe m_currentSpell needs to be rewritten as well...
    // leaving old Arcemu solution here for now, but I'll try to get back to this sooner or later in spell system rewrite -Appled
    if (GetPlayer()->GetOnMeleeSpell() != spellId)
    {
        if ((spellInfo->getAttributesExB() & ATTRIBUTESEXB_ACTIVATE_AUTO_SHOT))	// Auto Shot and Shoot (wand)
        {
            LogDebug("HandleCastSpellOpcode: Auto Shot-type spell cast (id %u, name %s)", spellInfo->getId(), spellInfo->getName().c_str());
            Item* rangedWep = GetPlayer()->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
            if (rangedWep == nullptr)
                return;

            uint32_t rangedSpellId;
            switch (rangedWep->GetItemProperties()->SubClass)
            {
                case 2:			 // Bows
                case 3:			 // Guns
                case 18:		 // Crossbows
                    rangedSpellId = SPELL_RANGED_GENERAL;
                    break;
                case 16:		 // Throw
                    rangedSpellId = SPELL_RANGED_THROW;
                    break;
                case 19:		 // Wands
                    rangedSpellId = SPELL_RANGED_WAND;
                    break;
                default:
                    rangedSpellId = spellInfo->getId();
                    break;
            }

            if (!_player->m_onAutoShot)
            {
                _player->m_AutoShotTarget = _player->GetSelection();
                uint32_t duration = _player->GetBaseAttackTime(RANGED);
                SpellCastTargets targets(recvPacket, GetPlayer()->GetGUID());
                if (!targets.m_unitTarget)
                {
                    LogDebug("HandleCastSpellOpcode: Cancelling auto-shot cast because targets.m_unitTarget is null!");
                    return;
                }

                SpellInfo* sp = sSpellCustomizations.GetSpellInfo(rangedSpellId);
                _player->m_AutoShotSpell = sp;
                _player->m_AutoShotDuration = duration;
                //This will fix fast clicks
                if (_player->m_AutoShotAttackTimer < 500)
                    _player->m_AutoShotAttackTimer = 500;
                _player->m_onAutoShot = true;
            }
            return;
        }

        // TODO: move this check to Spell::prepare() or smth
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
                _player->SendCastResult(spellInfo->getId(), SPELL_FAILED_SPELL_IN_PROGRESS, castCount, 0);
                return;
            }
        }

        SpellCastTargets targets(recvPacket, GetPlayer()->GetGUID());
        Spell* spell = sSpellFactoryMgr.NewSpell(GetPlayer(), spellInfo, false, nullptr);
        spell->extra_cast_number = castCount;

        // Some spell cast packets include more data
        if (castFlags & 0x02)
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
                recvPacket.SetOpcode(recvPacket.read<uint32_t>()); // MSG_MOVE_STOP
                HandleMovementOpcodes(recvPacket);
            }

            spell->m_missilePitch = projectilePitch;
            spell->m_missileTravelTime = travelTime;
        }

        spell->prepare(&targets);
    }
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleCancelCastOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    // Before this was only read in Cata but seems like it was already around in Wotlk
#if VERSION_STRING > TBC
    recvPacket.read_skip<uint8_t>(); // Increments with every HandleCancelCast packet, unused
#endif
    recvPacket.read_skip<uint32_t>(); // Spell Id, unused

    // TODO: rewrite me!
    if (_player->m_currentSpell)
    {
        _player->m_currentSpell->cancel();
    }
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleCancelAuraOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

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

    // You can't cancel a passive aura
    if (spellInfo->IsPassive())
    {
        return;
    }

    // You can't cancel an aura which is from a channeled spell, unless you are currently channeling it
    if (spellInfo->getAttributesEx() & (ATTRIBUTESEX_CHANNELED_1 | ATTRIBUTESEX_CHANNELED_2))
    {
        // TODO: rewrite me!
        if (_player->m_currentSpell != nullptr)
        {
            if (_player->m_currentSpell->GetSpellInfo()->getAttributesEx() & (ATTRIBUTESEX_CHANNELED_1 | ATTRIBUTESEX_CHANNELED_2))
            {
                if (_player->m_currentSpell->GetSpellInfo()->getId() == spellId)
                    _player->m_currentSpell->cancel();
            }
        }
        return;
    }

    Aura* spellAura = _player->getAuraWithId(spellId);
    if (spellAura == nullptr)
    {
        return;
    }

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
    {
        return;
    }

    if (spellInfo->getAttributes() & ATTRIBUTES_NEGATIVE)
    {
        return;
    }

    _player->removeAllAurasById(spellId);
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleCancelChannellingOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    recvPacket.read_skip<uint32_t>(); // Spell Id, unused

    // TODO: rewrite me!
    if (_player->m_currentSpell)
    {
        _player->m_currentSpell->cancel();
    }
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleCancelAutoRepeatSpellOpcode(WorldPacket& /*recvPacket*/)
{
    CHECK_INWORLD_RETURN

    // TODO: rewrite me!
    _player->m_onAutoShot = false;
}
#endif

void WorldSession::HandlePetCastSpell(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    uint64_t petGuid;
    uint32_t spellId;
    uint8_t castCount, castFlags;

    recvPacket >> petGuid >> castCount >> spellId >> castFlags;

    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);
    if (spellInfo == nullptr)
    {
        return;
    }

    if (_player->GetSummon() == nullptr && _player->m_CurrentCharm == 0 && _player->GetCharmedUnitGUID() == 0)
    {
        LogError("HandlePetCastSpell: Received opcode but player %u has no pet.", _player->GetLowGUID());
        return;
    }

    Unit* petUnit = _player->GetMapMgr()->GetUnit(petGuid);
    if (petUnit == nullptr)
    {
        LogError("HandlePetCastSpell: Pet entity cannot be found for player %u.", _player->GetLowGUID());
        return;
    }

    if (spellInfo->IsPassive())
    {
        return;
    }

    // If pet is summoned by player
    if (_player->GetSummon() == petUnit)
    {
        // Check does the pet have the spell
        if (!static_cast<Pet*>(petUnit)->HasSpell(spellId))
        {
            return;
        }
    }
    // If pet is charmed or possessed by player
    else if (_player->m_CurrentCharm == petGuid || _player->GetCharmedUnitGUID() == petGuid)
    {
        // TODO: find less uglier way for this... using Arcemu's solution for now
        bool found = false;
        for (std::list<AI_Spell*>::iterator itr = petUnit->GetAIInterface()->m_spells.begin(); itr != petUnit->GetAIInterface()->m_spells.end(); ++itr)
        {
            if ((*itr)->spell->getId() == spellId)
            {
                found = true;
                break;
            }
        }

        if (!found && petUnit->IsCreature())
        {
            Creature* petCreature = static_cast<Creature*>(petUnit);
            if (petCreature->GetCreatureProperties()->spelldataid != 0)
            {
                if (auto creatureSpellData = sCreatureSpellDataStore.LookupEntry(petCreature->GetCreatureProperties()->spelldataid))
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        if (creatureSpellData->Spells[i] == spellId)
                        {
                            found = true;
                            break;
                        }
                    }
                }
            }

            if (!found)
            {
                for (int i = 0; i < 4; ++i)
                {
                    if (petCreature->GetCreatureProperties()->AISpells[i] == spellId)
                    {
                        found = true;
                        break;
                    }
                }
            }
        }

        if (!found)
        {
            return;
        }
    }
    else
    {
        LogError("HandlePetCastSpell: Pet doesn't belong to player %u", _player->GetLowGUID());
        return;
    }

    SpellCastTargets targets(recvPacket, petGuid);
    Spell* spell = sSpellFactoryMgr.NewSpell(petUnit, spellInfo, false, nullptr);
    spell->extra_cast_number = castCount;

    // Some spell cast packets include more data
    if (castFlags & 0x02)
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
            recvPacket.SetOpcode(recvPacket.read<uint32_t>()); // MSG_MOVE_STOP
            HandleMovementOpcodes(recvPacket);
        }

        spell->m_missilePitch = projectilePitch;
        spell->m_missileTravelTime = travelTime;
    }

    spell->prepare(&targets);
}

#if VERSION_STRING != Cata
void WorldSession::HandleCancelTotem(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    uint8_t totemSlot;
    recvPacket >> totemSlot;

    if (totemSlot >= UNIT_SUMMON_SLOTS)
    {
        LogError("HandleCancelTotem: Player %u tried to cancel summon from out of range slot %u, ignored.", _player->GetLowGUID(), totemSlot);
        return;
    }

    _player->summonhandler.RemoveSummonFromSlot(totemSlot);
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleUpdateProjectilePosition(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    uint64_t casterGuid;
    uint32_t spellId;
    uint8_t castCount;
    float x, y, z; // Projectile hit location

    recvPacket >> casterGuid >> spellId >> castCount >> x >> y >> z;

    Unit* caster = _player->GetMapMgr()->GetUnit(casterGuid);
    if (caster == nullptr)
    {
        return;
    }

    if (caster->m_currentSpell == nullptr || caster->m_currentSpell->GetSpellInfo()->getId() != spellId)
    {
        return;
    }

    if (!caster->m_currentSpell->m_targets.hasDestination())
    {
        return;
    }

    // Relocate spell
    caster->m_currentSpell->m_targets.setDestination(LocationVector(x, y, z));

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
#endif
