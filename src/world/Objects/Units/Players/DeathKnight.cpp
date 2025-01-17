/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "Objects/Units/Players/PlayerClasses.hpp"
#include "Server/EventMgr.h"
#include "Server/WorldSession.h"
#include "Server/Packets/SmsgConvertRune.h"

void DeathKnight::SendRuneUpdate(uint8_t slot)
{
#if VERSION_STRING > TBC
    getSession()->SendPacket(AscEmu::Packets::SmsgConvertRune(slot, m_runes[slot].type).serialise().get());
#endif
}

uint8_t DeathKnight::GetBaseRuneType(uint8_t slot)
{
    return base_runes[slot];
}

uint8_t DeathKnight::GetRuneType(uint8_t slot)
{
    return m_runes[slot].type;
}

bool DeathKnight::GetRuneIsUsed(uint8_t slot)
{
    return m_runes[slot].is_used;
}

void DeathKnight::ConvertRune(uint8_t slot, uint8_t type)
{
    if (m_runes[slot].type == type)
        return;

    m_runes[slot].type = type;
    SendRuneUpdate(slot);
}

uint32_t DeathKnight::HasRunes(uint8_t type, uint32_t count)
{
    uint32_t found = 0;
    for (uint8_t i = 0; i < MAX_RUNES && count != found; ++i)
        if (m_runes[i].type == type && !m_runes[i].is_used)
            found++;

    return (count - found);
}

uint32_t DeathKnight::TakeRunes(uint8_t type, uint32_t count)
{
    uint8_t found = 0;
    for (uint8_t i = 0; i < MAX_RUNES && count != found; ++i)
    {
        if (m_runes[i].type == type && !m_runes[i].is_used)
        {
            m_runes[i].is_used = true;
            m_last_used_rune_slot = i;
            sEventMgr.AddEvent(this, &DeathKnight::ResetRune, i, EVENT_PLAYER_RUNE_REGEN + i, 10000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            found++;
        }
    }
    return (count - found);
}

void DeathKnight::ResetRune(uint8_t slot)
{
    m_runes[slot].is_used = false;
    sEventMgr.RemoveEvents(this, EVENT_PLAYER_RUNE_REGEN + slot);
}

uint8_t DeathKnight::GetRuneFlags()
{
    uint8_t result = 0;
    for (uint8_t k = 0; k < MAX_RUNES; k++)
        if (!m_runes[k].is_used)
            result |= (1 << k);

    return result;
}

bool DeathKnight::IsAllRunesOfTypeInUse(uint8_t type)
{
    for (uint8_t i = 0; i < MAX_RUNES; ++i)
        if (m_runes[i].type == type && !m_runes[i].is_used)
            return false;

    return true;
}
