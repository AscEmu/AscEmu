/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Spell/SpellTarget.h"
#include "Units/Creatures/Pet.h"
#include "Spell.h"
#include "Spell/Definitions/SpellInFrontStatus.h"
#include "Spell/Definitions/SpellCastTargetFlags.h"
#include "Spell/Definitions/SpellDamageType.h"
#include "Spell/Definitions/ProcFlags.h"
#include "Spell/Definitions/CastInterruptFlags.h"
#include "Spell/Definitions/AuraInterruptFlags.h"
#include "Spell/Definitions/SpellCustomFlags.h"
#include "Spell/Definitions/SpellGoFlags.h"
#include "Spell/Definitions/SpellTargetType.h"
#include "Spell/Definitions/SpellRanged.h"
#include "Spell/Definitions/SpellIsFlags.h"
#include "Spell/Definitions/DiminishingGroup.h"
#include "Spell/Definitions/SpellState.h"
#include "Spell/Definitions/SpellMechanics.h"
#include "Spell/Definitions/SpellEffectTarget.h"
#include "Spell/Definitions/PowerType.h"
#include "Spell/Definitions/SpellDidHitResult.h"
#include "Spell/SpellHelpers.h"
#include "StdAfx.h"
#include "VMapFactory.h"
#include "Management/Item.h"
#include "Objects/DynamicObject.h"
#include "Management/ItemInterface.h"
#include "Units/Stats.h"
#include "Management/Battleground/Battleground.h"
#include "Server/WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Players/PlayerClasses.hpp"
#include "Map/MapMgr.h"
#include "Map/MapScriptInterface.h"
#include "Objects/Faction.h"
#include "Spell/SpellMgr.h"
#include "Spell/SpellAuras.h"
#include "Map/WorldCreatorDefines.hpp"

void Spell::SendSpellStart()
{
    // no need to send this on passive spells
    if (!m_caster->IsInWorld() || hasAttribute(ATTRIBUTES_PASSIVE) || m_triggeredSpell)
        return;

    WorldPacket data(150);

    uint32 castFlags = 2;

    if (GetType() == SPELL_DMG_TYPE_RANGED)
        castFlags |= 0x20;

    // hacky yeaaaa
    if (GetSpellInfo()->getId() == 8326)   // death
        castFlags = 0x0F;

    data.SetOpcode(SMSG_SPELL_START);
    if (i_caster != nullptr)
    {
        data << i_caster->GetNewGUID();
        data << u_caster->GetNewGUID();
    }
    else
    {
        data << m_caster->GetNewGUID();
        data << m_caster->GetNewGUID();
    }

    data << extra_cast_number;
    data << GetSpellInfo()->getId();
    data << castFlags;
    data << uint32(m_timer);
    data << (uint32)m_castTime;

    m_targets.write(data);

    if (GetType() == SPELL_DMG_TYPE_RANGED)
    {
        ItemProperties const* ip = nullptr;
        if (GetSpellInfo()->getId() == SPELL_RANGED_THROW)   // throw
        {
            if (p_caster != nullptr)
            {
                auto item = p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                if (item != nullptr)
                {
                    ip = item->GetItemProperties();
                    /* Throwing Weapon Patch by Supalosa
                    p_caster->GetItemInterface()->RemoveItemAmt(it->GetEntry(),1);
                    (Supalosa: Instead of removing one from the stack, remove one from durability)
                    We don't need to check if the durability is 0, because you can't cast the Throw spell if the thrown weapon is broken, because it returns "Requires Throwing Weapon" or something.
                    */

                    // burlex - added a check here anyway (wpe suckers :P)
                    if (item->GetDurability() > 0)
                    {
                        item->SetDurability(item->GetDurability() - 1);
                        if (item->GetDurability() == 0)
                            p_caster->ApplyItemMods(item, EQUIPMENT_SLOT_RANGED, false, true);
                    }
                }
                else
                {
                    ip = sMySQLStore.getItemProperties(2512);	/*rough arrow*/
                }
            }
        }

        if (ip != nullptr)
        {
            data << ip->DisplayInfoID;
            data << ip->InventoryType;
        }
        else
        {
            data << uint32(0);
            data << uint32(0);
        }
    }

    m_caster->SendMessageToSet(&data, true);
}

void Spell::SendSpellGo()
{
    // Fill UniqueTargets
    std::vector<uint64_t>::iterator i, j;
    for (uint8 x = 0; x < 3; x++)
    {
        if (GetSpellInfo()->getEffect(x))
        {
            bool add = true;
            for (i = m_targetUnits[x].begin(); i != m_targetUnits[x].end(); ++i)
            {
                add = true;
                for (j = UniqueTargets.begin(); j != UniqueTargets.end(); ++j)
                {
                    if ((*j) == (*i))
                    {
                        add = false;
                        break;
                    }
                }

                if (add && (*i) != 0)
                    UniqueTargets.push_back((*i));
                //TargetsList::iterator itr = std::unique(m_targetUnits[x].begin(), m_targetUnits[x].end());
                //UniqueTargets.insert(UniqueTargets.begin(),));
                //UniqueTargets.insert(UniqueTargets.begin(), itr);
            }
        }
    }

    // no need to send this on passive spells
    if (!m_caster->IsInWorld() || hasAttribute(ATTRIBUTES_PASSIVE))
        return;

    // Start Spell
    WorldPacket data(200);
    data.SetOpcode(SMSG_SPELL_GO);
    uint32 flags = 0;

    if (m_missileTravelTime != 0)
        flags |= 0x20000;

    if (GetType() == SPELL_DMG_TYPE_RANGED)
        flags |= SPELL_GO_FLAGS_RANGED; // 0x20 RANGED

    if (i_caster != NULL)
        flags |= SPELL_GO_FLAGS_ITEM_CASTER; // 0x100 ITEM CASTER

    if (ModeratedTargets.size() > 0)
        flags |= SPELL_GO_FLAGS_EXTRA_MESSAGE; // 0x400 TARGET MISSES AND OTHER MESSAGES LIKE "Resist"

    if (p_caster != NULL && GetSpellInfo()->getPowerType() != POWER_TYPE_HEALTH)
        flags |= SPELL_GO_FLAGS_POWER_UPDATE;

    //experiments with rune updates
    uint8 cur_have_runes = 0;
    if (p_caster && p_caster->IsDeathKnight())   //send our rune updates ^^
    {
        if (GetSpellInfo()->getRuneCostID() && GetSpellInfo()->getPowerType() == POWER_TYPE_RUNES)
            flags |= SPELL_GO_FLAGS_ITEM_CASTER | SPELL_GO_FLAGS_RUNE_UPDATE | SPELL_GO_FLAGS_UNK40000;
        //see what we will have after cast
        cur_have_runes = static_cast<DeathKnight*>(p_caster)->GetRuneFlags();
        if (cur_have_runes != m_rune_avail_before)
            flags |= SPELL_GO_FLAGS_RUNE_UPDATE | SPELL_GO_FLAGS_UNK40000;
    }

    // hacky..
    if (GetSpellInfo()->getId() == 8326)   // death
        flags = SPELL_GO_FLAGS_ITEM_CASTER | 0x0D;

    if (i_caster != NULL && u_caster != NULL)   // this is needed for correct cooldown on items
    {
        data << i_caster->GetNewGUID();
        data << u_caster->GetNewGUID();
    }
    else
    {
        data << m_caster->GetNewGUID();
        data << m_caster->GetNewGUID();
    }

    data << extra_cast_number; //3.0.2
    data << GetSpellInfo()->getId();
    data << flags;
    data << Util::getMSTime();
    data << (uint8)(UniqueTargets.size()); //number of hits
    writeSpellGoTargets(&data);

    if (flags & SPELL_GO_FLAGS_EXTRA_MESSAGE)
    {
        data << (uint8)(ModeratedTargets.size()); //number if misses
        writeSpellMissedTargets(&data);
    }
    else
        data << uint8(0);   //moderated target size is 0 since we did not set the flag

    m_targets.write(data);   // this write is included the target flag

    if (flags & SPELL_GO_FLAGS_POWER_UPDATE)
        data << (uint32)p_caster->GetPower(static_cast<uint16_t>(GetSpellInfo()->getPowerType()));

    // er why handle it being null inside if if you can't get into if if its null
    if (GetType() == SPELL_DMG_TYPE_RANGED)
    {
        ItemProperties const* ip = nullptr;
        if (GetSpellInfo()->getId() == SPELL_RANGED_THROW)
        {
            if (p_caster != NULL)
            {
                Item* it = p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                if (it != nullptr)
                    ip = it->GetItemProperties();
            }
            else
                ip = sMySQLStore.getItemProperties(2512);	/*rough arrow*/
        }

        if (ip != nullptr)
        {
            data << ip->DisplayInfoID;
            data << ip->InventoryType;
        }
        else
        {
            data << uint32(0);
            data << uint32(0);
        }
    }

    //data order depending on flags : 0x800, 0x200000, 0x20000, 0x20, 0x80000, 0x40 (this is not spellgoflag but seems to be from spellentry or packet..)
    //.text:00401110                 mov     eax, [ecx+14h] -> them
    //.text:00401115                 cmp     eax, [ecx+10h] -> us
    if (flags & SPELL_GO_FLAGS_RUNE_UPDATE)
    {
        data << uint8(m_rune_avail_before);
        data << uint8(cur_have_runes);
        for (uint8 k = 0; k < MAX_RUNES; k++)
        {
            uint8 x = (1 << k);
            if ((x & m_rune_avail_before) != (x & cur_have_runes))
                data << uint8(0);   //values of the rune converted into byte. We just think it is 0 but maybe it is not :P
        }
    }

    /*
    float dx = targets.m_destX - targets.m_srcX;
    float dy = targets.m_destY - targets.m_srcY;
    if (missilepitch != M_PI / 4 && missilepitch != -M_PI / 4) //lets not divide by 0 lul
    traveltime = (sqrtf(dx * dx + dy * dy) / (cosf(missilepitch) * missilespeed)) * 1000;
    */

    if (flags & 0x20000)
    {
        data << float(m_missilePitch);
        data << uint32(m_missileTravelTime);
    }

    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        data << uint8(0);   //some spells require this ? not sure if it is last byte or before that.

    m_caster->SendMessageToSet(&data, true);

    // spell log execute is still send 2.08
    // as I see with this combination, need to test it more
    //if (flags != 0x120 && GetProto()->Attributes & 16) // not ranged and flag 5
    //SendLogExecute(0,m_targets.m_unitTarget);
}

void Spell::SendChannelStart(uint32 duration)
{
    if (!m_caster->IsGameObject())
    {
        // Send Channel Start
        WorldPacket data(MSG_CHANNEL_START, 22);
        data << WoWGuid(m_caster->GetNewGUID());
        data << uint32(m_spellInfo->getId());
        data << uint32(duration);
        data << uint8(0);
        data << uint8(0);
        m_caster->SendMessageToSet(&data, true);
    }

    m_castTime = m_timer = duration;

    if (u_caster != nullptr)
    {
        u_caster->SetChannelSpellId(GetSpellInfo()->getId());
        sEventMgr.AddEvent(u_caster, &Unit::EventStopChanneling, false, EVENT_STOP_CHANNELING, duration, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}