/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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
#include <git_version.h>


bool ChatHandler::HandleRangeCheckCommand(const char* args, WorldSession* m_session)
{
    WorldPacket data;
    uint64 guid = m_session->GetPlayer()->GetSelection();
    m_session->SystemMessage("=== RANGE CHECK ===");
    if (guid == 0)
    {
        m_session->SystemMessage("No selection.");
        return true;
    }

    Unit* unit = m_session->GetPlayer()->GetMapMgr()->GetUnit(guid);
    if (!unit)
    {
        m_session->SystemMessage("Invalid selection.");
        return true;
    }
    float DistSq = unit->GetDistanceSq(m_session->GetPlayer());
    m_session->SystemMessage("GetDistanceSq  :   %u", float2int32(DistSq));
    LocationVector locvec(m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ());
    float DistReal = unit->CalcDistance(locvec);
    m_session->SystemMessage("CalcDistance   :   %u", float2int32(DistReal));
    float Dist2DSq = unit->GetDistance2dSq(m_session->GetPlayer());
    m_session->SystemMessage("GetDistance2dSq:   %u", float2int32(Dist2DSq));
    return true;
}

float CalculateDistance(float x1, float y1, float z1, float x2, float y2, float z2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    float dz = z1 - z2;
    return sqrt(dx * dx + dy * dy + dz * dz);
}

bool ChatHandler::HandleSimpleDistanceCommand(const char* args, WorldSession* m_session)
{
    float toX, toY, toZ;
    if (sscanf(args, "%f %f %f", &toX, &toY, &toZ) != 3)
        return false;

    if (toX >= _maxX || toX <= _minX || toY <= _minY || toY >= _maxY)
        return false;

    float distance = CalculateDistance(
        m_session->GetPlayer()->GetPositionX(),
        m_session->GetPlayer()->GetPositionY(),
        m_session->GetPlayer()->GetPositionZ(),
        toX, toY, toZ);

    m_session->SystemMessage("Your distance to location (%f, %f, %f) is %0.2f meters.", toX, toY, toZ, distance);

    return true;
}

bool ChatHandler::HandleSendFailed(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (plr == NULL)
        return false;

    uint32 fail = atol(args);
    if (SPELL_CANCAST_OK < fail)
    {
        RedSystemMessage(m_session, "Argument %u is out of range!", fail);
        return false;
    }
    plr->SendCastResult(1, (uint8)fail, 0, 0);
    return true;
}

bool ChatHandler::HandlePlayMovie(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (plr == NULL)
        return false;

    uint32 movie = atol(args);

    plr->SendTriggerMovie(movie);

    SystemMessage(m_session, "Movie started.");
    return true;
}

bool ChatHandler::HandleAuraUpdateAdd(const char* args, WorldSession* m_session)
{
    if (!args)
        return false;

    uint32 SpellID = 0;
    int Flags = 0;
    int StackCount = 0;
    if (sscanf(args, "%u 0x%X %i", &SpellID, &Flags, &StackCount) != 3 && sscanf(args, "%u %u %i", &SpellID, &Flags, &StackCount) != 3)
        return false;

    Player* Pl = m_session->GetPlayer();
    if (Aura* AuraPtr = Pl->FindAura(SpellID))
    {
        uint8 VisualSlot = AuraPtr->m_visualSlot;
        Pl->SendAuraUpdate(AuraPtr->m_auraSlot, false);
        SystemMessage(m_session, "SMSG_AURA_UPDATE (update): VisualSlot %u - SpellID %u - Flags %i (0x%04X) - StackCount %i", VisualSlot, SpellID, Flags, Flags, StackCount);
    }
    else
    {
        SpellEntry* Sp = dbcSpell.LookupEntryForced(SpellID);
        if (!Sp)
        {
            SystemMessage(m_session, "SpellID %u is invalid.", SpellID);
            return true;
        }
        Spell* SpellPtr = sSpellFactoryMgr.NewSpell(Pl, Sp, false, NULL);
        AuraPtr = sSpellFactoryMgr.NewAura(Sp, SpellPtr->GetDuration(), Pl, Pl);
        SystemMessage(m_session, "SMSG_AURA_UPDATE (add): VisualSlot %u - SpellID %u - Flags %i (0x%04X) - StackCount %i", AuraPtr->m_visualSlot, SpellID, Flags, Flags, StackCount);
        Pl->AddAura(AuraPtr);       // Serves purpose to just add the aura to our auraslots

        delete SpellPtr;
    }
    return true;
}

bool ChatHandler::HandleAuraUpdateRemove(const char* args, WorldSession* m_session)
{
    if (!args)
        return false;

    char* pArgs = strtok((char*)args, " ");
    if (!pArgs)
        return false;
    uint8 VisualSlot = (uint8)atoi(pArgs);
    Player* Pl = m_session->GetPlayer();
    Aura* AuraPtr = Pl->FindAura(Pl->m_auravisuals[VisualSlot]);
    if (!AuraPtr)
    {
        SystemMessage(m_session, "No auraid found in slot %u", VisualSlot);
        return true;
    }
    SystemMessage(m_session, "SMSG_AURA_UPDATE (remove): VisualSlot %u - SpellID 0", VisualSlot);
    AuraPtr->Remove();
    return true;
}
