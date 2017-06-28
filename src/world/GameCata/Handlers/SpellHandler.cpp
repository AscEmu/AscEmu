/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Item.h"
#include "Spell/SpellNameHashes.h"
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
    uint8_t cn;
    uint32_t glyphSlot;
    uint8_t missileflag;

    recvPacket >> cn;
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
                _player->SendCastResult(spellInfo->Id, SPELL_FAILED_SPELL_IN_PROGRESS, cn, 0);
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
                _player->SendCastResult(spellInfo->Id, SPELL_FAILED_BAD_TARGETS, cn, 0);
                return;
            }
        }

        Spell* spell = sSpellFactoryMgr.NewSpell(GetPlayer(), spellInfo, false, NULL);
        spell->extra_cast_number = cn;
        spell->m_glyphslot = glyphSlot;
        spell->prepare(&targets);
    }
}
