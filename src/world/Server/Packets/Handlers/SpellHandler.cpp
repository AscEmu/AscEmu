/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
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

void WorldSession::HandleUseItemOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    typedef std::list<Aura*> AuraList;

    Player* p_User = GetPlayer();
    LOG_DETAIL("WORLD: got use Item packet, data length = %i", recvPacket.size());
    int8 tmp1, slot;
    uint8 unk; //Alice : added in 3.0.2
    uint64 item_guid;
    uint8 cn;
    uint32 spellId = 0;
    uint32 glyphIndex;
    bool found = false;

    recvPacket >> tmp1;
    recvPacket >> slot;
    recvPacket >> cn;
    recvPacket >> spellId;
    recvPacket >> item_guid;
    recvPacket >> glyphIndex;
    recvPacket >> unk;

    Item* tmpItem = nullptr;
    tmpItem = p_User->GetItemInterface()->GetInventoryItem(tmp1, slot);
    if (!tmpItem)
        tmpItem = p_User->GetItemInterface()->GetInventoryItem(slot);

    if (!tmpItem)
        return;

    ItemProperties const* itemProto = tmpItem->GetItemProperties();

    // only some consumable items can be used in arenas
    if ((itemProto->Class == ITEM_CLASS_CONSUMABLE) &&
        !itemProto->HasFlag(ITEM_FLAG_USEABLE_IN_ARENA) &&
        (GetPlayer()->m_bg != NULL) &&
        isArena(GetPlayer()->m_bg->GetType()))
    {
        GetPlayer()->GetItemInterface()->BuildInventoryChangeError(tmpItem, NULL, INV_ERR_NOT_DURING_ARENA_MATCH);
        return;
    }

    if (tmpItem->IsSoulbound())     // SouldBind item will be used after SouldBind()
    {
        if (sScriptMgr.CallScriptedItem(tmpItem, _player))
            return;
    }

    if (_player->getDeathState() == CORPSE)
        return;

    if (itemProto->Bonding == ITEM_BIND_ON_USE)
        tmpItem->SoulBind();

    if (sScriptMgr.CallScriptedItem(tmpItem, _player))
        return;

    if (itemProto->InventoryType != 0 && !_player->GetItemInterface()->IsEquipped(itemProto->ItemId))  //Equipable items cannot be used before they're equipped. Prevents exploits
        return;//Prevents exploits such as keeping an on-use trinket in your bag and using WPE to use it from your bag in mid-combat.

    if (itemProto->QuestId)
    {
        // Item Starter
        QuestProperties const* qst = sMySQLStore.getQuestProperties(itemProto->QuestId);
        if (!qst)
            return;

        WorldPacket data;
        sQuestMgr.BuildQuestDetails(&data, qst, tmpItem, 0, language, _player);
        SendPacket(&data);
    }

    // Let's check if the item even has that spell
    for (uint8 i = 0; i < 5; ++i)
    {
        if (itemProto->Spells[i].Trigger == USE && itemProto->Spells[i].Id == spellId)
        {
            found = true;
            break;//found 1 already
        }
    }

    // Let's see if it is an onuse spellid
    if (tmpItem->HasOnUseSpellID(spellId))
        found = true;

    // We didn't find the spell, so the player is probably trying to cheat
    // with an edited itemcache.wdb
    //
    // Altough this could also happen after a DB update
    // if he/she didn't delete his/her cache.
    if (found == false)
    {

        this->Disconnect();
        Anticheat_Log->writefromsession(this, "Player tried to use an item with a spell that didn't match the spell in the database.");
        Anticheat_Log->writefromsession(this, "Possibly corrupted or intentionally altered itemcache.wdb");
        Anticheat_Log->writefromsession(this, "Itemid: %lu", itemProto->ItemId);
        Anticheat_Log->writefromsession(this, "Spellid: %lu", spellId);
        Anticheat_Log->writefromsession(this, "Player was disconnected");

        return;
    }

    SpellCastTargets targets(recvPacket, _player->GetGUID());

    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);
    if (spellInfo == NULL)
    {
        LOG_ERROR("WORLD: unknown spell id %i", spellId);
        return;
    }

    if (spellInfo->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP && !_player->IsSitting())
    {
        if (p_User->CombatStatus.IsInCombat() || p_User->IsMounted())
        {
            _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, NULL, INV_ERR_CANT_DO_IN_COMBAT);
            return;
        }
        else
            p_User->SetStandState(STANDSTATE_SIT);
        // loop through the auras and removing existing eating spells
    }
    /*else   // cebernic: why not stand up (because reasons...)
    {
        if (!p_User->CombatStatus.IsInCombat() && !p_User->IsMounted())
        {
            if (p_User->GetStandState())
            {
                p_User->SetStandState(STANDSTATE_STAND);
            }
        }
    }*/

    // cebernic: remove stealth on using item
    if (!(spellInfo->getAuraInterruptFlags() & ATTRIBUTESEX_NOT_BREAK_STEALTH))
    {
        if (p_User->IsStealth())
            p_User->RemoveAllAuraType(SPELL_AURA_MOD_STEALTH);
    }

    if (itemProto->RequiredLevel)
    {
        if (_player->getLevel() < itemProto->RequiredLevel)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, NULL, INV_ERR_ITEM_RANK_NOT_ENOUGH);
            return;
        }
    }

    if (itemProto->RequiredSkill)
    {
        if (!_player->_HasSkillLine(itemProto->RequiredSkill))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, NULL, INV_ERR_ITEM_RANK_NOT_ENOUGH);
            return;
        }

        if (itemProto->RequiredSkillRank)
        {
            if (_player->_GetSkillLineCurrent(itemProto->RequiredSkill, false) < itemProto->RequiredSkillRank)
            {
                _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, NULL, INV_ERR_ITEM_RANK_NOT_ENOUGH);
                return;
            }
        }
    }

    if ((itemProto->AllowableClass && !(_player->getClassMask() & itemProto->AllowableClass)) || (itemProto->AllowableRace && !(_player->getRaceMask() & itemProto->AllowableRace)))
    {
        _player->GetItemInterface()->BuildInventoryChangeError(tmpItem, NULL, INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM);
        return;
    }

    if (!_player->Cooldown_CanCast(spellInfo))
    {
        _player->SendCastResult(spellInfo->getId(), SPELL_FAILED_NOT_READY, cn, 0);
        return;
    }


    if (_player->m_currentSpell)
    {
        _player->SendCastResult(spellInfo->getId(), SPELL_FAILED_SPELL_IN_PROGRESS, cn, 0);
        return;
    }

    if (itemProto->ForcedPetId >= 0)
    {
        if (itemProto->ForcedPetId == 0)
        {
            if (_player->GetGUID() != targets.m_unitTarget)
            {
                _player->SendCastResult(spellInfo->getId(), SPELL_FAILED_BAD_TARGETS, cn, 0);
                return;
            }
        }
        else
        {

            if (!_player->GetSummon() || _player->GetSummon()->GetEntry() != (uint32)itemProto->ForcedPetId)
            {
                _player->SendCastResult(spellInfo->getId(), SPELL_FAILED_SPELL_IN_PROGRESS, cn, 0);
                return;
            }
        }
    }

    Spell* spell = sSpellFactoryMgr.NewSpell(_player, spellInfo, false, NULL);
    spell->extra_cast_number = cn;
    spell->i_caster = tmpItem;
    spell->m_glyphslot = glyphIndex;

    //GetPlayer()->setCurrentSpell(spell);
    spell->prepare(&targets);

#if VERSION_STRING > TBC
    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM, itemProto->ItemId, 0, 0);
#endif
}

#if VERSION_STRING != Cata
void WorldSession::HandleSpellClick(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

        LOG_DETAIL("WORLD: got CMSG_SPELLCLICK packet, data length = %i", recvPacket.size());

    if (_player->getDeathState() == CORPSE)
        return;

    uint64 target_guid; // this will store the guid of the object we are going to use it's spell. There must be a dbc that indicates what spells a unit has

    recvPacket >> target_guid;

    //we have only 1 example atm for entry : 28605
    Unit* target_unit = _player->GetMapMgr()->GetUnit(target_guid);

    if (!target_unit)
        return;

    if (!_player->isInRange(target_unit, MAX_INTERACTION_RANGE))
        return;

    if (target_unit->IsVehicle())
    {
        if (target_unit->GetVehicleComponent() != NULL)
            target_unit->GetVehicleComponent()->AddPassenger(_player);
        return;
    }

    uint32 creature_id = target_unit->GetEntry();
    uint32 cast_spell_id = 0;

    if (target_unit->RemoveAura(59907))
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
                if (target_unit->IsCreature())
                {
                    Creature* c = static_cast<Creature*>(target_unit);

                    sChatHandler.BlueSystemMessage(this, "NPC Id %u (%s) has no spellclick spell associated with it.", c->GetCreatureProperties()->Id, c->GetCreatureProperties()->Name.c_str());
                    LOG_ERROR("Spellclick packet received for creature %u but there is no spell associated with it.", creature_id);
                    return;
                }
            }
            else
            {
                cast_spell_id = sp->SpellID;
                target_unit->CastSpell(_player, cast_spell_id, true);
            }

            if (!target_unit->HasAura(59907))
                static_cast<Creature*>(target_unit)->Despawn(0, 0); //IsCreature() check is not needed, refer to r2387 and r3230

            return;
        }
    }

    SpellClickSpell const* sp = sMySQLStore.getSpellClickSpell(creature_id);
    if (sp == nullptr)
    {
        if (target_unit->IsCreature())
        {
            Creature* c = static_cast< Creature* >(target_unit);

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

        Spell* spell = sSpellFactoryMgr.NewSpell(_player, spellInfo, false, NULL);
        SpellCastTargets targets(target_guid);
        spell->prepare(&targets);
    }
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleCastSpellOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    uint32 spellId;
    uint8 cn, unk; //Alice : Added to 3.0.2

    recvPacket >> cn >> spellId >> unk;
    // check for spell id
    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);

    if (!spellInfo)
    {
        LogError("WORLD: unknown spell id %i", spellId);
        return;
    }

    if (!_player->isAlive() && _player->GetShapeShift() != FORM_SPIRITOFREDEMPTION && !(spellInfo->getAttributes() & ATTRIBUTES_DEAD_CASTABLE)) //They're dead, not in spirit of redemption and the spell can't be cast while dead.
        return;

    LogDetail("WORLD: got cast spell packet, spellId - %i (%s), data length = %i", spellId, spellInfo->getName().c_str(), recvPacket.size());

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
        if ((spellInfo->getAttributesExB() & ATTRIBUTESEXB_ACTIVATE_AUTO_SHOT) /*spellInfo->Attributes == 327698*/)	// auto shot..
        {
            LogDebugFlag(LF_SPELL, "HandleCastSpellOpcode : Auto Shot-type spell cast (id %u, name %s)" , spellInfo->getId(), spellInfo->getName().c_str());
            Item* weapon = GetPlayer()->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
            if (!weapon)
                return;

            uint32 spellid;
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
                spellid = spellInfo->getId();

            if (!_player->m_onAutoShot)
            {
                _player->m_AutoShotTarget = _player->GetSelection();
                uint32 duration = _player->GetBaseAttackTime(RANGED);
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
                _player->SendCastResult(spellInfo->getId(), SPELL_FAILED_SPELL_IN_PROGRESS, cn, 0);
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
                _player->SendCastResult(spellInfo->getId(), SPELL_FAILED_BAD_TARGETS, cn, 0);
                return;
            }
        }

        Spell* spell = sSpellFactoryMgr.NewSpell(GetPlayer(), spellInfo, false, NULL);
        spell->extra_cast_number = cn;
        spell->prepare(&targets);
    }
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleCancelCastOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    uint32 spellId;
    recvPacket >> spellId;

    if (GetPlayer()->m_currentSpell)
        GetPlayer()->m_currentSpell->cancel();
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleCancelAuraOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    uint32 spellId;
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
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleCancelChannellingOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    uint32 spellId;
    recvPacket >> spellId;

    Player* plyr = GetPlayer();
    if (!plyr)
        return;
    if (plyr->m_currentSpell)
    {
        plyr->m_currentSpell->cancel();
    }
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleCancelAutoRepeatSpellOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    LogDebugFlag(LF_OPCODE, "Received CMSG_CANCEL_AUTO_REPEAT_SPELL message.");
    //on original we automatically enter combat when creature got close to us
    //	GetPlayer()->GetSession()->OutPacket(SMSG_CANCEL_COMBAT);
    GetPlayer()->m_onAutoShot = false;
}
#endif

void WorldSession::HandlePetCastSpell(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    uint64 guid = 0;
    uint8  castCount = 0;
    uint32 spellid = 0;
    uint8  castflags = 0;
    uint32 targetmask = 0;

    recvPacket >> guid;
    recvPacket >> castCount;
    recvPacket >> spellid;
    recvPacket >> castflags;

    SpellInfo* sp = sSpellCustomizations.GetSpellInfo(spellid);
    if (sp == NULL)
        return;
    // Summoned Elemental's Freeze
    if (spellid == 33395)
    {
        if (!_player->GetSummon())
            return;
    }
    else if (guid != _player->m_CurrentCharm)
    {
        if (_player->GetCharmedUnitGUID() != guid)
            return;
    }

    SpellCastTargets targets;
    targets.read(recvPacket, guid);

    float missilepitch = 0.0f;
    float missilespeed = 0;
    uint32 traveltime = 0;

    if (castflags & 2)
    {
        recvPacket >> missilepitch;
        recvPacket >> missilespeed;

        auto destination = targets.destination();
        auto source = targets.source();
        auto dx = destination.x - source.x;
        auto dy = destination.y - source.y;

        if ((missilepitch != M_PI / 4) && (missilepitch != -M_PI / 4)) //lets not divide by 0 lul
            traveltime = static_cast<uint32>((sqrtf(dx * dx + dy * dy) / (cosf(missilepitch) * missilespeed)) * 1000);
    }

    if (spellid == 33395)	// Summoned Water Elemental's freeze
    {
        Spell* pSpell = sSpellFactoryMgr.NewSpell(_player->GetSummon(), sp, false, 0);
        pSpell->prepare(&targets);
    }
    else			// trinket?
    {
        uint64 charmguid = _player->m_CurrentCharm;
        if (charmguid == 0)
            charmguid = _player->GetCharmedUnitGUID();

        Unit* nc = _player->GetMapMgr()->GetUnit(charmguid);
        if (nc)
        {
            bool check = false;
            for (std::list<AI_Spell*>::iterator itr = nc->GetAIInterface()->m_spells.begin(); itr != nc->GetAIInterface()->m_spells.end(); ++itr)//.......meh. this is a crappy way of doing this, I bet.
            {
                if ((*itr)->spell->getId() == spellid)
                {
                    check = true;
                    break;
                }
            }
            
            if (nc->IsCreature())
            {
                Creature* c = static_cast< Creature* >(nc);

                if (c->GetCreatureProperties()->spelldataid != 0)
                {
                    auto creature_spell_data = sCreatureSpellDataStore.LookupEntry(c->GetCreatureProperties()->spelldataid);

                    if (creature_spell_data != nullptr)
                    {
                        for (uint8 i = 0; i < 3; i++)
                        {
                            if (creature_spell_data->Spells[i] == spellid)
                            {
                                check = true;
                                break;
                            }
                        }
                    }
                }

                for (uint8 i = 0; i < 4; ++i)
                {
                    if (c->GetCreatureProperties()->AISpells[i] == spellid)
                    {
                        check = true;
                        break;
                    }
                }
            }

            if (!check)
                return;

            Spell* pSpell = sSpellFactoryMgr.NewSpell(nc, sp, false, 0);
            pSpell->m_missilePitch = missilepitch;
            pSpell->m_missileTravelTime = traveltime;

            pSpell->prepare(&targets);
        }
    }
}

#if VERSION_STRING != Cata
void WorldSession::HandleCancelTotem(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint8 slot;
    recv_data >> slot;

    if (slot >= UNIT_SUMMON_SLOTS)
    {
        LOG_ERROR("Player %u %s tried to cancel a summon at slot %u, slot number is out of range. (tried to crash the server?)", _player->GetLowGUID(), _player->GetName(), slot);
        return;
    }

    _player->summonhandler.RemoveSummonFromSlot(slot);
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleUpdateProjectilePosition(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 casterGuid;          // guid of the caster
    uint32 spellId;             // spell ID of casted spell
    uint8 castCount;            // count how many times it is/was cast
    float x, y, z;              // missile hit position

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

#if VERSION_STRING > TBC
    WorldPacket data(SMSG_SET_PROJECTILE_POSITION, 21);
    data << uint64(casterGuid);
    data << uint8(castCount);
    data << float(x);
    data << float(y);
    data << float(z);
    SendPacket(&data);
#endif
}
#endif
