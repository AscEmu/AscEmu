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

bool ChatHandler::HandleDebugDumpMovementCommand(const char* args, WorldSession* session)
{
    try
    {
        auto me = session->GetPlayerOrThrow();

        SystemMessage(session, "Position: [%f, %f, %f, %f]", me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
        SystemMessage(session, "On transport: %s", me->obj_movement_info.transporter_info.guid != 0 ? "yes" : "no");
        SystemMessage(session, "Transport GUID: %lu", me->obj_movement_info.transporter_info.guid);
        SystemMessage(session, "Transport relative position: [%f, %f, %f, %f]", me->obj_movement_info.transporter_info.position.x, me->obj_movement_info.transporter_info.position.y, me->obj_movement_info.transporter_info.position.z, me->obj_movement_info.transporter_info.position.o);

        return true;
    }
    catch(...)
    {
        return false;
    }
}

bool ChatHandler::HandleDebugInFrontCommand(const char* args, WorldSession* m_session)
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

bool ChatHandler::HandleDistanceCommand(const char* args, WorldSession* m_session)
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
        static_cast< Creature* >(obj)->GetAIInterface()->SetRun();
    else
        static_cast< Creature* >(obj)->GetAIInterface()->SetWalk();

    float distance = static_cast< Creature* >(obj)->CalcDistance(x, y, z);
    if (Move == 1)
    {
        if (Meth == 1)
        {
            float q = distance - 0.5f;
            x = (static_cast< Creature* >(obj)->GetPositionX() + x * q) / (1 + q);
            y = (static_cast< Creature* >(obj)->GetPositionY() + y * q) / (1 + q);
            z = (static_cast< Creature* >(obj)->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 2)
        {
            float q = distance - 1;
            x = (static_cast< Creature* >(obj)->GetPositionX() + x * q) / (1 + q);
            y = (static_cast< Creature* >(obj)->GetPositionY() + y * q) / (1 + q);
            z = (static_cast< Creature* >(obj)->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 3)
        {
            float q = distance - 2;
            x = (static_cast< Creature* >(obj)->GetPositionX() + x * q) / (1 + q);
            y = (static_cast< Creature* >(obj)->GetPositionY() + y * q) / (1 + q);
            z = (static_cast< Creature* >(obj)->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 4)
        {
            float q = distance - 2.5f;
            x = (static_cast< Creature* >(obj)->GetPositionX() + x * q) / (1 + q);
            y = (static_cast< Creature* >(obj)->GetPositionY() + y * q) / (1 + q);
            z = (static_cast< Creature* >(obj)->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 5)
        {
            float q = distance - 3;
            x = (static_cast< Creature* >(obj)->GetPositionX() + x * q) / (1 + q);
            y = (static_cast< Creature* >(obj)->GetPositionY() + y * q) / (1 + q);
            z = (static_cast< Creature* >(obj)->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 6)
        {
            float q = distance - 3.5f;
            x = (static_cast< Creature* >(obj)->GetPositionX() + x * q) / (1 + q);
            y = (static_cast< Creature* >(obj)->GetPositionY() + y * q) / (1 + q);
            z = (static_cast< Creature* >(obj)->GetPositionZ() + z * q) / (1 + q);
        }
        else
        {
            float q = distance - 4;
            x = (static_cast< Creature* >(obj)->GetPositionX() + x * q) / (1 + q);
            y = (static_cast< Creature* >(obj)->GetPositionY() + y * q) / (1 + q);
            z = (static_cast< Creature* >(obj)->GetPositionZ() + z * q) / (1 + q);
        }
        static_cast< Creature* >(obj)->GetAIInterface()->MoveTo(x, y, z, 0);
    }
    else
    {
        static_cast<Creature*>(obj)->GetAIInterface()->MoveTo(x, y, z, o);
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

    uint32 BytesIndex = atoi(pBytesIndex);

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
    obj->SetUInt32Value(BytesIndex, ((Value1) | (Value2 << 8) | (Value3 << 16) | (Value4 << 24)));
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

    uint32 BytesIndex = atoi(pBytesIndex);
    uint32 theBytes = obj->GetUInt32Value(BytesIndex);

    std::stringstream sstext;
    sstext << "bytes for Field " << BytesIndex << " are " << uint16((uint8)theBytes & 0xFF) << " " << uint16((uint8)(theBytes >> 8) & 0xFF) << " ";
    sstext << uint16((uint8)(theBytes >> 16) & 0xFF) << " " << uint16((uint8)(theBytes >> 24) & 0xFF) << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}
bool ChatHandler::HandleDebugLandWalk(const char* args, WorldSession* m_session)
{
    Player* chr = GetSelectedPlayer(m_session, true, true);
    if (chr == nullptr)
        return true;

    char buf[256];

    chr->SetMovement(MOVE_LAND_WALK, 8);
    snprintf((char*)buf, 256, "Land Walk Test Ran.");
    SystemMessage(m_session, buf);

    return true;
}

bool ChatHandler::HandleDebugWaterWalk(const char* args, WorldSession* m_session)
{
    Player* chr = GetSelectedPlayer(m_session, true, true);
    if (chr == nullptr)
        return true;

    char buf[256];

    chr->SetMovement(MOVE_WATER_WALK, 4);
    snprintf((char*)buf, 256, "Water Walk Test Ran.");
    SystemMessage(m_session, buf);

    return true;
}

bool ChatHandler::HandleAggroRangeCommand(const char* args, WorldSession* m_session)
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
    data << getMSTime();
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

    uint32 threat = target->GetAIInterface()->_CalcThreat(atol(dmg), dbcSpell.LookupEntry(atoi(spellId)), m_session->GetPlayer());

    std::stringstream sstext;
    sstext << "generated threat is: " << threat << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleThreatListCommand(const char* args, WorldSession* m_session)
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
    uint32 oldValue = obj->GetUInt32Value(field);
    obj->SetUInt32Value(field, value);

    snprintf((char*)buf, 256, "Set Field %i from %i to %i.", (unsigned int)field, (unsigned int)oldValue, (unsigned int)value);

    if (obj->IsPlayer())
        static_cast< Player* >(obj)->UpdateChances();

    SystemMessage(m_session, buf);

    return true;
}

bool ChatHandler::HandleDebugDumpCoordsCommmand(const char* args, WorldSession* m_session)
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

//As requested by WaRxHeAd for database development.
//This should really only be available in special cases and NEVER on real servers... -DGM

//#define _ONLY_FOOLS_TRY_THIS_

bool ChatHandler::HandleSQLQueryCommand(const char* args, WorldSession* m_session)
{
#ifdef _ONLY_FOOLS_TRY_THIS_
    if (!*args)
    {
        RedSystemMessage(m_session, "No query given.");
        return false;
    }

    bool isok = false;
    //SQL query lenght is seems to be limited to 16384 characters, thus the check
    if (strlen(args) > 16384)
    {
        RedSystemMessage(m_session, "Query is longer than 16384 chars!");
        //Now let the user now what are we talking about
        GreenSystemMessage(m_session, args);
    }
    else
    {
        //First send back what we got. Feedback to the user of the command.
        GreenSystemMessage(m_session, args);
        //Sending the query, but this time getting the result back
        QueryResult* qResult = WorldDatabase.Query(args);
        if (qResult != NULL)
        {
            Field* field;
            //Creating the line (instancing)
            std::string line = "";
            do
            {
                field = qResult->Fetch();
                for (uint32 i = 0; i < (qResult->GetFieldCount()); i++)
                {
                    //Constructing the line
                    line += field[i].GetString();
                }
                //Sending the line as ingame message
                BlueSystemMessage(m_session, line.data());
                //Clear the line
                line.clear();
            }
            while (qResult->NextRow());
            delete field;
        }
        else
        {
            RedSystemMessage(m_session, "Invalid query, or the Databse might be busy.");
            isok = false;
        }
        //delete qResult anyway, not to cause some leak!
        delete qResult;
        isok = true;
    }

    if (isok)
        GreenSystemMessage(m_session, "Query was executed successfully.");
    else
        RedSystemMessage(m_session, "Query failed to execute.");

#endif

    return true;
}

bool ChatHandler::HandleSendpacket(const char* args, WorldSession* m_session)
{
#ifdef _ONLY_FOOLS_TRY_THIS_

    uint32 arg_len = strlen(args);
    char* xstring = new char[arg_len];
    memcpy(xstring, args, arg_len);

    for (uint32 i = 0; i < arg_len; i++)
    {
        if (xstring[i] == ' ')
        {
            xstring[i] = '\0';
        }
    }

    // we receive our packet as hex, that means we get it like ff ff ff ff
    // the opcode consists of 2 bytes

    if (!xstring)
    {
        LOG_DEBUG("[Debug][Sendpacket] Packet is invalid");
        return false;
    }

    WorldPacket data(arg_len);

    uint32 loop = 0;
    uint16 opcodex = 0;
    uint16 opcodez = 0;

    // get the opcode
    sscanf(xstring, "%x", &opcodex);

    // should be around here
    sscanf(&xstring[3], "%x", &opcodez);

    opcodex = opcodex << 8;
    opcodex |= opcodez;
    data.Initialize(opcodex);


    int j = 3;
    int x = 0;
    do
    {
        if (xstring[j] == '\0')
        {
            uint32 HexValue;
            sscanf(&xstring[j + 1], "%x", &HexValue);
            if (HexValue > 0xFF)
            {
                LOG_DEBUG("[Debug][Sendpacket] Packet is invalid");
                return false;
            }
            data << uint8(HexValue);
            //j++;
        }
        j++;
    }
    while (j < arg_len);

    data.hexlike();

    m_session->SendPacket(&data);

    LOG_DEBUG("[Debug][Sendpacket] Packet was send");
#endif
    return true;
}

bool ChatHandler::HandleDebugSpawnWarCommand(const char* args, WorldSession* m_session)
{
    uint32 count, npcid;
    uint32 health = 0;

    // takes 2 or 3 arguments: npcid, count, (health)
    if (sscanf(args, "%u %u %u", &npcid, &count, &health) != 3)
        if (sscanf(args, "%u %u", &count, &npcid) != 2)
            return false;

    if (!count || !npcid)
        return false;

    CreatureProperties const* cp = sMySQLStore.GetCreatureProperties(npcid);
    if (cp == nullptr)
        return false;

    MapMgr* m = m_session->GetPlayer()->GetMapMgr();

    // if we have selected unit, use its position
    Unit* unit = m->GetUnit(m_session->GetPlayer()->GetSelection());
    if (unit == NULL)
        unit = m_session->GetPlayer(); // otherwise ours

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
            c->SetUInt32Value(UNIT_FIELD_MAXHEALTH, health);
            c->SetUInt32Value(UNIT_FIELD_HEALTH, health);
        }
        c->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, (count % 2) ? 1 : 2);
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

bool ChatHandler::HandleInitWorldStatesCommand(const char* args, WorldSession* session)
{
    Player* p = session->GetPlayer();

    uint32 zone = p->GetZoneId();
    if (zone == 0)
        zone = p->GetAreaID();

    BlueSystemMessage(session, "Sending initial worldstates for zone %u", zone);

    p->SendInitialWorldstates();

    return true;
}

bool ChatHandler::HandleClearWorldStatesCommand(const char* args, WorldSession* session)
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
