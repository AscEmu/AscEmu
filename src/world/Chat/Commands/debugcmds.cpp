/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#include "VMapFactory.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/SpellCastTargetFlags.h"
#include "Spell/Customization/SpellCustomizations.hpp"

bool ChatHandler::HandleDebugDumpMovementCommand(const char* /*args*/, WorldSession* session)
{
    try
    {
        auto me = session->GetPlayerOrThrow();

        SystemMessage(session, "Position: [%f, %f, %f, %f]", me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
#if VERSION_STRING != Cata
        SystemMessage(session, "On transport: %s", me->obj_movement_info.transporter_info.guid != 0 ? "yes" : "no");
        SystemMessage(session, "Transport GUID: %lu", me->obj_movement_info.transporter_info.guid);
        SystemMessage(session, "Transport relative position: [%f, %f, %f, %f]", me->obj_movement_info.transporter_info.position.x, me->obj_movement_info.transporter_info.position.y, me->obj_movement_info.transporter_info.position.z, me->obj_movement_info.transporter_info.position.o);
#else
        SystemMessage(session, "On transport: %s", !me->obj_movement_info.getTransportGuid().IsEmpty() ? "yes" : "no");
        //SystemMessage(session, "Transport GUID: %lu", me->obj_movement_info.getTransportGuid());
        SystemMessage(session, "Transport relative position: [%f, %f, %f, %f]", me->obj_movement_info.getTransportPosition()->x, me->obj_movement_info.getTransportPosition()->y, me->obj_movement_info.getTransportPosition()->z, me->obj_movement_info.getTransportPosition()->o);
#endif

        return true;
    }
    catch(...)
    {
        return false;
    }
}

bool ChatHandler::HandleDebugInFrontCommand(const char* /*args*/, WorldSession* m_session)
{
    Object* obj;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->GetMapMgr()->GetUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
    {
        obj = m_session->GetPlayer();
    }

    char buf[256];
    snprintf((char*)buf, 256, "%d", m_session->GetPlayer()->isInFront((Unit*)obj));

    SystemMessage(m_session, buf);

    return true;
}

bool ChatHandler::HandleShowReactionCommand(const char* args, WorldSession* m_session)
{
    Object* obj = NULL;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        obj = m_session->GetPlayer()->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    }

    if (!obj)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }


    char* pReaction = strtok((char*)args, " ");
    if (!pReaction)
        return false;

    uint32 Reaction = atoi(pReaction);

    obj->SendAIReaction(Reaction);

    std::stringstream sstext;
    sstext << "Sent Reaction of " << Reaction << " to " << obj->GetUIdFromGUID() << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleDistanceCommand(const char* /*args*/, WorldSession* m_session)
{
    Object* obj;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->GetMapMgr()->GetUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
    {
        obj = m_session->GetPlayer();
    }

    float dist = m_session->GetPlayer()->CalcDistance(obj);
    std::stringstream sstext;
    sstext << "Distance is: " << dist << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}


bool ChatHandler::HandleAIMoveCommand(const char* args, WorldSession* m_session)
{
    Creature* creature = nullptr;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        creature = m_session->GetPlayer()->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    }

    if (creature == nullptr)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    //m_session->GetPlayer()->GetOrientation();

    uint32 Move = 1;
    uint32 Run = 0;
    uint32 Time = 0;
    uint32 Meth = 0;

    char* pMove = strtok((char*)args, " ");
    if (pMove)
        Move = atoi(pMove);

    char* pRun = strtok(NULL, " ");
    if (pRun)
        Run = atoi(pRun);

    char* pTime = strtok(NULL, " ");
    if (pTime)
        Time = atoi(pTime);

    char* pMeth = strtok(NULL, " ");
    if (pMeth)
        Meth = atoi(pMeth);

    float x = m_session->GetPlayer()->GetPositionX();
    float y = m_session->GetPlayer()->GetPositionY();
    float z = m_session->GetPlayer()->GetPositionZ();
    float o = m_session->GetPlayer()->GetOrientation();

    if (Run)
        creature->GetAIInterface()->setSplineRun();
    else
        creature->GetAIInterface()->setSplineWalk();

    float distance = creature->CalcDistance(x, y, z);
    if (Move == 1)
    {
        if (Meth == 1)
        {
            float q = distance - 0.5f;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 2)
        {
            float q = distance - 1;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 3)
        {
            float q = distance - 2;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 4)
        {
            float q = distance - 2.5f;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 5)
        {
            float q = distance - 3;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 6)
        {
            float q = distance - 3.5f;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else
        {
            float q = distance - 4;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        creature->GetAIInterface()->MoveTo(x, y, z, o);
    }
    else
    {
        creature->GetAIInterface()->MoveTo(x, y, z, o);
    }

    return true;
}

bool ChatHandler::HandleFaceCommand(const char* args, WorldSession* m_session)
{
    Object* obj = NULL;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        obj = m_session->GetPlayer()->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    }

    if (obj == NULL)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    uint32 Orentation = 0;
    char* pOrentation = strtok((char*)args, " ");
    if (pOrentation)
        Orentation = atoi(pOrentation);

    // Convert to Blizzards Format
    float theOrientation = Orentation / (180.0f / M_PI_FLOAT);

    obj->SetPosition(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), theOrientation, false);

    LOG_DEBUG("facing sent");
    return true;

}

bool ChatHandler::HandleSetBytesCommand(const char* args, WorldSession* m_session)
{
    Object* obj;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->GetMapMgr()->GetUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
    {
        obj = m_session->GetPlayer();
    }

    char* pBytesIndex = strtok((char*)args, " ");
    if (!pBytesIndex)
        return false;

    uint16 BytesIndex = static_cast<uint16>(atoi(pBytesIndex));

    char* pValue1 = strtok(NULL, " ");
    if (!pValue1)
        return false;

    uint8 Value1 = static_cast<uint8>(atol(pValue1));

    char* pValue2 = strtok(NULL, " ");
    if (!pValue2)
        return false;

    uint8 Value2 = static_cast<uint8>(atol(pValue2));

    char* pValue3 = strtok(NULL, " ");
    if (!pValue3)
        return false;

    uint8 Value3 = static_cast<uint8>(atol(pValue3));

    char* pValue4 = strtok(NULL, " ");
    if (!pValue4)
        return false;

    uint8 Value4 = static_cast<uint8>(atol(pValue4));

    std::stringstream sstext;
    sstext << "Set Field " << BytesIndex
        << " bytes to " << uint16((uint8)Value1)
        << " " << uint16((uint8)Value2)
        << " " << uint16((uint8)Value3)
        << " " << uint16((uint8)Value4)
        << '\0';
    obj->setUInt32Value(BytesIndex, ((Value1) | (Value2 << 8) | (Value3 << 16) | (Value4 << 24)));
    SystemMessage(m_session, sstext.str().c_str());

    return true;
}

bool ChatHandler::HandleGetBytesCommand(const char* args, WorldSession* m_session)
{
    Object* obj;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->GetMapMgr()->GetUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
        obj = m_session->GetPlayer();

    char* pBytesIndex = strtok((char*)args, " ");
    if (!pBytesIndex)
        return false;

    uint16 BytesIndex = static_cast<uint16>(atoi(pBytesIndex));
    uint32 theBytes = obj->getUInt32Value(BytesIndex);

    std::stringstream sstext;
    sstext << "bytes for Field " << BytesIndex << " are " << uint16((uint8)theBytes & 0xFF) << " " << uint16((uint8)(theBytes >> 8) & 0xFF) << " ";
    sstext << uint16((uint8)(theBytes >> 16) & 0xFF) << " " << uint16((uint8)(theBytes >> 24) & 0xFF) << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}
bool ChatHandler::HandleDebugLandWalk(const char* /*args*/, WorldSession* m_session)
{
    Player* chr = GetSelectedPlayer(m_session, true, true);
    if (chr == nullptr)
        return true;

    char buf[256];

    chr->setMoveLandWalk();
    snprintf((char*)buf, 256, "Land Walk Test Ran.");
    SystemMessage(m_session, buf);

    return true;
}

bool ChatHandler::HandleAggroRangeCommand(const char* /*args*/, WorldSession* m_session)
{
    Unit* unit = GetSelectedUnit(m_session, true);
    if (unit == nullptr)
        return true;

    float aggroRange = unit->GetAIInterface()->_CalcAggroRange(m_session->GetPlayer());

    GreenSystemMessage(m_session, "Aggrorange is %f", aggroRange);

    return true;
}

bool ChatHandler::HandleKnockBackCommand(const char* args, WorldSession* m_session)
{
    float f = args ? (float)atof(args) : 0.0f;
    if (f == 0.0f)
        f = 5.0f;

    float dx = sinf(m_session->GetPlayer()->GetOrientation());
    float dy = cosf(m_session->GetPlayer()->GetOrientation());

    float z = f * 0.66f;

    WorldPacket data(SMSG_MOVE_KNOCK_BACK, 50);
    data << m_session->GetPlayer()->GetNewGUID();
    data << Util::getMSTime();
    data << dy;
    data << dx;
    data << f;
    data << z;
    m_session->SendPacket(&data);
    return true;
}

bool ChatHandler::HandleFadeCommand(const char* args, WorldSession* m_session)
{
    Unit* target = m_session->GetPlayer()->GetMapMgr()->GetUnit(m_session->GetPlayer()->GetSelection());
    if (!target)
        target = m_session->GetPlayer();
    char* v = strtok((char*)args, " ");
    if (!v)
        return false;

    target->ModThreatModifyer(atoi(v));

    std::stringstream sstext;
    sstext << "threat is now reduced by: " << target->GetThreatModifyer() << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleThreatModCommand(const char* args, WorldSession* m_session)
{
    Unit* target = m_session->GetPlayer()->GetMapMgr()->GetUnit(m_session->GetPlayer()->GetSelection());
    if (!target)
        target = m_session->GetPlayer();
    char* v = strtok((char*)args, " ");
    if (!v)
        return false;

    target->ModGeneratedThreatModifyer(0, atoi(v));

    std::stringstream sstext;
    sstext << "new threat caused is now reduced by: " << target->GetGeneratedThreatModifyer(0) << "%" << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleCalcThreatCommand(const char* args, WorldSession* m_session)
{
    Unit* target = m_session->GetPlayer()->GetMapMgr()->GetUnit(m_session->GetPlayer()->GetSelection());
    if (!target)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }
    char* dmg = strtok((char*)args, " ");
    if (!dmg)
        return false;
    char* spellId = strtok(NULL, " ");
    if (!spellId)
        return false;

    uint32 threat = target->GetAIInterface()->_CalcThreat(atol(dmg), sSpellCustomizations.GetSpellInfo(atoi(spellId)), m_session->GetPlayer());

    std::stringstream sstext;
    sstext << "generated threat is: " << threat << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleThreatListCommand(const char* /*args*/, WorldSession* m_session)
{
    Unit* target = NULL;
    target = m_session->GetPlayer()->GetMapMgr()->GetUnit(m_session->GetPlayer()->GetSelection());
    if (!target)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    std::stringstream sstext;
    sstext << "threatlist of creature: " << Arcemu::Util::GUID_LOPART(m_session->GetPlayer()->GetSelection()) << " " << Arcemu::Util::GUID_HIPART(m_session->GetPlayer()->GetSelection()) << '\n';
    TargetMap::iterator itr;
    for (itr = target->GetAIInterface()->GetAITargets()->begin(); itr != target->GetAIInterface()->GetAITargets()->end();)
    {
        Unit* ai_t = target->GetMapMgr()->GetUnit(itr->first);
        if (!ai_t || !itr->second)
        {
            ++itr;
            continue;
        }
        sstext << "guid: " << itr->first << " | threat: " << itr->second << "| threat after mod: " << (itr->second + ai_t->GetThreatModifyer()) << "\n";
        ++itr;
    }

    SendMultilineMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleSendItemPushResult(const char* args, WorldSession* m_session)
{
    uint32 uint_args[7];
    char* arg = const_cast<char*>(args);
    char* token = strtok(arg, " ");

    uint8 i = 0;
    while (token != NULL && i < 7)
    {
        uint_args[i] = atol(token);
        token = strtok(NULL, " ");
        i++;
    }
    for (; i < 7; i++)
        uint_args[i] = 0;

    if (uint_args[0] == 0)   // null itemid
        return false;

    WorldPacket data;
    data.SetOpcode(SMSG_ITEM_PUSH_RESULT);
    data << m_session->GetPlayer()->GetGUID();    // recivee_guid
    data << uint_args[2];   // type
    data << uint32(1);      // unk
    data << uint_args[1];   // count
    data << uint8(0xFF);    // uint8 unk const 0xFF
    data << uint_args[3];   // unk1
    data << uint_args[0];   // item id
    data << uint_args[4];   // unk2
    data << uint_args[5];   // random prop
    data << uint_args[6];   // unk3
    m_session->SendPacket(&data);

    return true;
}

bool ChatHandler::HandleModifyBitCommand(const char* args, WorldSession* m_session)
{
    Object* obj;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->GetMapMgr()->GetUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
    {
        obj = m_session->GetPlayer();
    }

    char* pField = strtok((char*)args, " ");
    if (!pField)
        return false;

    char* pBit = strtok(NULL, " ");
    if (!pBit)
        return false;

    uint16 field = static_cast<uint16>(atoi(pField));
    uint32 bit = atoi(pBit);

    if (field < 1 || field >= PLAYER_END)
    {
        SystemMessage(m_session, "Incorrect values.");
        return true;
    }

    if (bit < 1 || bit > 32)
    {
        SystemMessage(m_session, "Incorrect values.");
        return true;
    }

    char buf[256];

    if (obj->HasFlag(field, (1 << (bit - 1))))
    {
        obj->RemoveFlag(field, (1 << (bit - 1)));
        snprintf((char*)buf, 256, "Removed bit %i in field %i.", (unsigned int)bit, (unsigned int)field);
    }
    else
    {
        obj->SetFlag(field, (1 << (bit - 1)));
        snprintf((char*)buf, 256, "Set bit %i in field %i.", (unsigned int)bit, (unsigned int)field);
    }

    SystemMessage(m_session, buf);
    return true;
}

bool ChatHandler::HandleModifyValueCommand(const char* args, WorldSession* m_session)
{
    Object* obj;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->GetMapMgr()->GetUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
        obj = m_session->GetPlayer();

    char* pField = strtok((char*)args, " ");
    if (!pField)
        return false;

    char* pValue = strtok(NULL, " ");
    if (!pValue)
        return false;

    uint16 field = static_cast<uint16>(atoi(pField));
    uint32 value = atoi(pValue);

    if (field < 1 || field >= PLAYER_END)
    {
        SystemMessage(m_session, "Incorrect Field.");
        return true;
    }

    char buf[256];
    uint32 oldValue = obj->getUInt32Value(field);
    obj->setUInt32Value(field, value);

    snprintf((char*)buf, 256, "Set Field %i from %i to %i.", (unsigned int)field, (unsigned int)oldValue, (unsigned int)value);

    if (obj->IsPlayer())
        static_cast< Player* >(obj)->UpdateChances();

    SystemMessage(m_session, buf);

    return true;
}

bool ChatHandler::HandleDebugDumpCoordsCommmand(const char* /*args*/, WorldSession* m_session)
{
    Player* p = m_session->GetPlayer();
    //char buffer[200] = {0};
    FILE* f = fopen("C:\\script_dump.txt", "a");
    if (!f)
        return false;

    fprintf(f, "mob.CreateWaypoint(%f, %f, %f, %f, 0, 0, 0);\n", p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), p->GetOrientation());
    fclose(f);

    return true;
}

bool ChatHandler::HandleDebugSpawnWarCommand(const char* args, WorldSession* m_session)
{
    uint32 count, npcid;
    uint32 health = 0;

    // takes 2 or 3 arguments: npcid, count, (health)
    if (sscanf(args, "%u %u %u", &npcid, &count, &health) != 3)
    {
        if (sscanf(args, "%u %u", &count, &npcid) != 2)
        {
            return false;
        }
    }

    if (!count || !npcid)
    {
        return false;
    }

    CreatureProperties const* cp = sMySQLStore.getCreatureProperties(npcid);
    if (cp == nullptr)
    {
        return false;
    }

    MapMgr* m = m_session->GetPlayer()->GetMapMgr();

    // if we have selected unit, use its position
    Unit* unit = m->GetUnit(m_session->GetPlayer()->GetSelection());
    if (unit == nullptr)
    {
        unit = m_session->GetPlayer(); // otherwise ours
    }

    float bx = unit->GetPositionX();
    float by = unit->GetPositionY();
    float x, y, z;

    float angle = 1;
    float r = 3; // starting radius
    for (; count > 0; --count)
    {
        // spawn in spiral
        x = r * sinf(angle);
        y = r * cosf(angle);
        z = m->GetLandHeight(bx + x, by + y, unit->GetPositionZ() + 2);

        Creature* c = m->CreateCreature(npcid);
        c->Load(cp, bx + x, by + y, z, 0.0f);
        if (health != 0)
        {
            c->setUInt32Value(UNIT_FIELD_MAXHEALTH, health);
            c->setUInt32Value(UNIT_FIELD_HEALTH, health);
        }
        c->setUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, (count % 2) ? 1 : 2);
        c->_setFaction();
        c->PushToWorld(m);

        r += 0.5;
        angle += 8 / r;
    }
    return true;
}

bool ChatHandler::HandleUpdateWorldStateCommand(const char *args, WorldSession* session)
{
    if (*args == '\0')
    {
        RedSystemMessage(session, "You need to specify the worldstate field and the new value.");
        return true;
    }

    uint32 field = 0;
    uint32 state = 0;

    std::stringstream ss(args);

    ss >> field;
    if (ss.fail())
    {
        RedSystemMessage(session, "You need to specify the worldstate field and the new value.");
        return true;
    }

    ss >> state;
    if (ss.fail())
    {
        RedSystemMessage(session, "You need to specify the worldstate field and the new value.");
        return true;
    }

    session->GetPlayer()->SendWorldStateUpdate(field, state);

    return true;
}

bool ChatHandler::HandleInitWorldStatesCommand(const char* /*args*/, WorldSession* session)
{
    Player* p = session->GetPlayer();

    uint32 zone = p->GetZoneId();
    if (zone == 0)
        zone = p->GetAreaID();

    BlueSystemMessage(session, "Sending initial worldstates for zone %u", zone);

    p->SendInitialWorldstates();

    return true;
}

bool ChatHandler::HandleClearWorldStatesCommand(const char* /*args*/, WorldSession* session)
{
    Player* p = session->GetPlayer();

    uint32 zone = p->GetZoneId();
    if (zone == 0)
        zone = p->GetAreaID();

    BlueSystemMessage(session, "Clearing worldstates for zone %u", zone);

    WorldPacket data(SMSG_INIT_WORLD_STATES, 16);

    data << uint32(p->GetMapId());
    data << uint32(p->GetZoneId());
    data << uint32(p->GetAreaID());
    data << uint16(0);

    p->SendPacket(&data);

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
    Aura* AuraPtr = Pl->getAuraWithId(Pl->m_auravisuals[VisualSlot]);
    if (!AuraPtr)
    {
        SystemMessage(m_session, "No auraid found in slot %u", VisualSlot);
        return true;
    }
    SystemMessage(m_session, "SMSG_AURA_UPDATE (remove): VisualSlot %u - SpellID 0", VisualSlot);
    AuraPtr->Remove();
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
    if (Aura* AuraPtr = Pl->getAuraWithId(SpellID))
    {
        uint8 VisualSlot = AuraPtr->m_visualSlot;
        Pl->SendAuraUpdate(AuraPtr->m_auraSlot, false);
        SystemMessage(m_session, "SMSG_AURA_UPDATE (update): VisualSlot %u - SpellID %u - Flags %i (0x%04X) - StackCount %i", VisualSlot, SpellID, Flags, Flags, StackCount);
    }
    else
    {
        SpellInfo* Sp = sSpellCustomizations.GetSpellInfo(SpellID);
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

bool ChatHandler::HandleRangeCheckCommand(const char* /*args*/, WorldSession* m_session)
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
    float DistSq = unit->getDistanceSq(m_session->GetPlayer());
    m_session->SystemMessage("getDistanceSq  :   %u", float2int32(DistSq));
    LocationVector locvec(m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ());
    float DistReal = unit->CalcDistance(locvec);
    m_session->SystemMessage("CalcDistance   :   %u", float2int32(DistReal));
    float Dist2DSq = unit->GetDistance2dSq(m_session->GetPlayer());
    m_session->SystemMessage("GetDistance2dSq:   %u", float2int32(Dist2DSq));
    return true;
}

bool ChatHandler::HandleCollisionTestIndoor(const char* /*args*/, WorldSession* m_session)
{
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        Player* plr = m_session->GetPlayer();
        const LocationVector & loc = plr->GetPosition();
        bool res = !MapManagement::AreaManagement::AreaStorage::IsOutdoor(plr->GetMapId(), loc.x, loc.y, loc.z + 2.0f);
        SystemMessage(m_session, "Result was: %s.", res ? "indoors" : "outside");
        return true;
    }
    else
    {
        SystemMessage(m_session, "Collision is not enabled.");
        return true;
    }
}

bool ChatHandler::HandleCollisionTestLOS(const char* /*args*/, WorldSession* m_session)
{
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        Object* pObj = NULL;
        Creature* pCreature = GetSelectedCreature(m_session, false);
        Player* pPlayer = GetSelectedPlayer(m_session, true, true);
        if (pCreature)
            pObj = pCreature;
        else if (pPlayer)
            pObj = pPlayer;

        if (pObj == NULL)
        {
            SystemMessage(m_session, "Invalid target.");
            return true;
        }

        VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
        const LocationVector & loc2 = pObj->GetPosition();
        const LocationVector & loc1 = m_session->GetPlayer()->GetPosition();
        bool res = mgr->isInLineOfSight(pObj->GetMapId(), loc1.x, loc1.y, loc1.z, loc2.x, loc2.y, loc2.z);
        bool res2 = mgr->isInLineOfSight(pObj->GetMapId(), loc1.x, loc1.y, loc1.z + 2.0f, loc2.x, loc2.y, loc2.z + 2.0f);
        bool res3 = mgr->isInLineOfSight(pObj->GetMapId(), loc1.x, loc1.y, loc1.z + 5.0f, loc2.x, loc2.y, loc2.z + 5.0f);
        SystemMessage(m_session, "Result was: %s %s %s.", res ? "in LOS" : "not in LOS", res2 ? "in LOS" : "not in LOS", res3 ? "in LOS" : "not in LOS");
        return true;
    }
    else
    {
        SystemMessage(m_session, "Collision is not enabled.");
        return true;
    }
}

bool ChatHandler::HandleCollisionGetHeight(const char* /*args*/, WorldSession* m_session)
{
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        Player* plr = m_session->GetPlayer();
        float radius = 5.0f;

        float posX = plr->GetPositionX();
        float posY = plr->GetPositionY();
        float posZ = plr->GetPositionZ();
        float ori = plr->GetOrientation();

        LocationVector src(posX, posY, posZ);

        LocationVector dest(posX + (radius * (cosf(ori))), posY + (radius * (sinf(ori))), posZ);
        //LocationVector destest(posX+(radius*(cosf(ori))),posY+(radius*(sinf(ori))),posZ);

        VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
        float z = mgr->getHeight(plr->GetMapId(), posX, posY, posZ + 2.0f, 10000.0f);
        float z2 = mgr->getHeight(plr->GetMapId(), posX, posY, posZ + 5.0f, 10000.0f);
        float z3 = mgr->getHeight(plr->GetMapId(), posX, posY, posZ, 10000.0f);
        float z4 = plr->GetMapMgr()->GetADTLandHeight(plr->GetPositionX(), plr->GetPositionY());
        bool fp = mgr->getObjectHitPos(plr->GetMapId(), src.x, src.y, src.z, dest.x, dest.y, dest.z, dest.x, dest.y, dest.z, -1.5f);

        SystemMessage(m_session, "Results were: %f(offset2.0f) | %f(offset5.0f) | %f(org) | landheight:%f | target radius5 FP:%d", z, z2, z3, z4, fp);
        return true;
    }
    else
    {
        SystemMessage(m_session, "Collision is not enabled.");
        return true;
    }
}

bool ChatHandler::HandleGetDeathState(const char* /*args*/, WorldSession* m_session)
{
    Player* SelectedPlayer = GetSelectedPlayer(m_session, true, true);
    if (!SelectedPlayer)
        return true;

    SystemMessage(m_session, "Death State: %d", SelectedPlayer->getDeathState());
    return true;
}

struct spell_thingo
{
    uint32 type;
    uint32 target;
};

std::list<SpellInfo*> aiagent_spells;
std::map<uint32, spell_thingo> aiagent_extra;

SpellCastTargets SetTargets(SpellInfo* /*sp*/, uint32 /*type*/, uint32 targettype, Unit* dst, Creature* src)
{
    SpellCastTargets targets;
    targets.m_unitTarget = 0;
    targets.m_itemTarget = 0;
    targets.setSource(LocationVector(0, 0, 0));
    targets.setDestination(LocationVector(0, 0, 0));

    if (targettype == TTYPE_SINGLETARGET)
    {
        targets.m_targetMask = TARGET_FLAG_UNIT;
        targets.m_unitTarget = dst->GetGUID();
    }
    else if (targettype == TTYPE_SOURCE)
    {
        targets.m_targetMask = TARGET_FLAG_SOURCE_LOCATION;
        targets.setSource(src->GetPosition());
    }
    else if (targettype == TTYPE_DESTINATION)
    {
        targets.m_targetMask = TARGET_FLAG_DEST_LOCATION;
        targets.setDestination(dst->GetPosition());
    }

    return targets;
};

bool ChatHandler::HandleAIAgentDebugSkip(const char* args, WorldSession* m_session)
{
    uint32 count = atoi(args);
    if (!count) return false;

    for (uint32 i = 0; i < count; ++i)
    {
        if (!aiagent_spells.size())
            break;

        aiagent_spells.erase(aiagent_spells.begin());
    }
    BlueSystemMessage(m_session, "Erased %u spells.", count);
    return true;
}

bool ChatHandler::HandleAIAgentDebugContinue(const char* args, WorldSession* m_session)
{
    uint32 count = atoi(args);
    if (!count)
        return false;

    Creature* pCreature = GetSelectedCreature(m_session, true);
    if (!pCreature)
        return true;

    Player* pPlayer = m_session->GetPlayer();

    for (uint32 i = 0; i < count; ++i)
    {
        if (!aiagent_spells.size())
            break;

        SpellInfo* sp = *aiagent_spells.begin();
        aiagent_spells.erase(aiagent_spells.begin());
        BlueSystemMessage(m_session, "Casting %u, " MSG_COLOR_SUBWHITE "%u remaining.", sp->getId(), aiagent_spells.size());

        std::map<uint32, spell_thingo>::iterator it = aiagent_extra.find(sp->getId());
        ARCEMU_ASSERT(it != aiagent_extra.end());

        SpellCastTargets targets;
        if (it->second.type == STYPE_BUFF)
            targets = SetTargets(sp, it->second.type, it->second.type, pCreature, pCreature);
        else
            targets = SetTargets(sp, it->second.type, it->second.type, pPlayer, pCreature);

        pCreature->GetAIInterface()->CastSpell(pCreature, sp, targets);
    }

    if (!aiagent_spells.size())
        RedSystemMessage(m_session, "Finished.");
    /*else
    BlueSystemMessage(m_session, "Got %u remaining.", aiagent_spells.size());*/
    return true;
}

bool ChatHandler::HandleAIAgentDebugBegin(const char* /*args*/, WorldSession* m_session)
{
    QueryResult* result = WorldDatabase.Query("SELECT DISTINCT spell FROM ai_agents");
    if (!result) return false;

    do
    {
        SpellInfo* se = sSpellCustomizations.GetSpellInfo(result->Fetch()[0].GetUInt32());
        if (se)
            aiagent_spells.push_back(se);
    } while (result->NextRow());
    delete result;

    for (std::list<SpellInfo*>::iterator itr = aiagent_spells.begin(); itr != aiagent_spells.end(); ++itr)
    {
        result = WorldDatabase.Query("SELECT * FROM ai_agents WHERE spell = %u", (*itr)->getId());
        ARCEMU_ASSERT(result != NULL);
        spell_thingo t;
        t.type = result->Fetch()[6].GetUInt32();
        t.target = result->Fetch()[7].GetUInt32();
        delete result;
        aiagent_extra[(*itr)->getId()] = t;
    }

    GreenSystemMessage(m_session, "Loaded %u spells for testing.", aiagent_spells.size());
    return true;
}

bool ChatHandler::HandleCastSpellCommand(const char* args, WorldSession* m_session)
{
    Unit* caster = m_session->GetPlayer();
    Unit* target = GetSelectedPlayer(m_session, true, true);
    if (!target)
        target = GetSelectedCreature(m_session, false);
    if (!target)
    {
        RedSystemMessage(m_session, "Must select a char or creature.");
        return false;
    }

    uint32 spellid = atol(args);
    SpellInfo* spellentry = sSpellCustomizations.GetSpellInfo(spellid);
    if (!spellentry)
    {
        RedSystemMessage(m_session, "Invalid spell id!");
        return false;
    }

    Spell* sp = sSpellFactoryMgr.NewSpell(caster, spellentry, false, NULL);

    BlueSystemMessage(m_session, "Casting spell %d on target.", spellid);
    SpellCastTargets targets;
    targets.m_unitTarget = target->GetGUID();
    sp->prepare(&targets);

    switch (target->GetTypeId())
    {
        case TYPEID_PLAYER:
            if (caster != target)
                sGMLog.writefromsession(m_session, "cast spell %d on PLAYER %s", spellid, static_cast< Player* >(target)->GetName());
            break;
        case TYPEID_UNIT:
            sGMLog.writefromsession(m_session, "cast spell %d on CREATURE %u [%s], sqlid %u", spellid, static_cast< Creature* >(target)->GetEntry(), static_cast< Creature* >(target)->GetCreatureProperties()->Name.c_str(), static_cast< Creature* >(target)->GetSQL_id());
            break;
    }

    return true;
}

bool ChatHandler::HandleCastSpellNECommand(const char* args, WorldSession* m_session)
{
    Unit* caster = m_session->GetPlayer();
    Unit* target = GetSelectedPlayer(m_session, true, true);
    if (!target)
        target = GetSelectedCreature(m_session, false);
    if (!target)
    {
        RedSystemMessage(m_session, "Must select a char or creature.");
        return false;
    }

    uint32 spellId = atol(args);
    SpellInfo* spellentry = sSpellCustomizations.GetSpellInfo(spellId);
    if (!spellentry)
    {
        RedSystemMessage(m_session, "Invalid spell id!");
        return false;
    }
    BlueSystemMessage(m_session, "Casting spell %d on target.", spellId);

    WorldPacket data;

    data.Initialize(SMSG_SPELL_START);
    data << caster->GetNewGUID();
    data << caster->GetNewGUID();
    data << spellId;
    data << uint8(0);
    data << uint16(0);
    data << uint32(0);
    data << uint16(2);
    data << target->GetGUID();
    //        WPARCEMU_ASSERT(  data.size() == 36);
    m_session->SendPacket(&data);

    data.Initialize(SMSG_SPELL_GO);
    data << caster->GetNewGUID();
    data << caster->GetNewGUID();
    data << spellId;
    data << uint8(0) << uint8(1) << uint8(1);
    data << target->GetGUID();
    data << uint8(0);
    data << uint16(2);
    data << target->GetGUID();
    //        WPARCEMU_ASSERT(  data.size() == 42);
    m_session->SendPacket(&data);

    switch (target->GetTypeId())
    {
        case TYPEID_PLAYER:
            if (caster != target)
                sGMLog.writefromsession(m_session, "cast spell %d on PLAYER %s", spellId, static_cast< Player* >(target)->GetName());
            break;
        case TYPEID_UNIT:
            sGMLog.writefromsession(m_session, "cast spell %d on CREATURE %u [%s], sqlid %u", spellId, static_cast< Creature* >(target)->GetEntry(), static_cast< Creature* >(target)->GetCreatureProperties()->Name.c_str(), static_cast< Creature* >(target)->GetSQL_id());
            break;
    }

    return true;
}

bool ChatHandler::HandleCastSelfCommand(const char* args, WorldSession* m_session)
{
    Unit* target = GetSelectedPlayer(m_session, true, true);
    if (!target)
        target = GetSelectedCreature(m_session, false);
    if (!target)
    {
        RedSystemMessage(m_session, "Must select a char or creature.");
        return false;
    }

    uint32 spellid = atol(args);
    SpellInfo* spellentry = sSpellCustomizations.GetSpellInfo(spellid);
    if (!spellentry)
    {
        RedSystemMessage(m_session, "Invalid spell id!");
        return false;
    }

    Spell* sp = sSpellFactoryMgr.NewSpell(target, spellentry, false, NULL);

    BlueSystemMessage(m_session, "Target is casting spell %d on himself.", spellid);
    SpellCastTargets targets;
    targets.m_unitTarget = target->GetGUID();
    sp->prepare(&targets);

    switch (target->GetTypeId())
    {
        case TYPEID_PLAYER:
            if (m_session->GetPlayer() != target)
                sGMLog.writefromsession(m_session, "used castself with spell %d on PLAYER %s", spellid, static_cast< Player* >(target)->GetName());
            break;
        case TYPEID_UNIT:
            sGMLog.writefromsession(m_session, "used castself with spell %d on CREATURE %u [%s], sqlid %u", spellid, static_cast< Creature* >(target)->GetEntry(), static_cast< Creature* >(target)->GetCreatureProperties()->Name.c_str(), static_cast< Creature* >(target)->GetSQL_id());
            break;
    }

    return true;
}
