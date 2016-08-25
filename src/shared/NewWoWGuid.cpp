/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "NewWoWGuid.hpp"

#include "../world/Server/Master.h"

#include <sstream>

char const* NewWoWGuid::GetHighGuidTypeName(HighGuidType high)
{
    switch (high)
    {
        case HIGHGUID_TYPE_ITEM:
            return "Item";
        case HIGHGUID_TYPE_PLAYER:
            return "Player";
        case HIGHGUID_TYPE_GAMEOBJECT:
            return "Gameobject";
        case HIGHGUID_TYPE_TRANSPORTER:
            return "Transporter";
        case HIGHGUID_TYPE_UNIT:
            return "Creature";
        case HIGHGUID_TYPE_PET:
            return "Pet";
        case HIGHGUID_TYPE_VEHICLE:
            return "Vehicle";
        case HIGHGUID_TYPE_DYNAMICOBJECT:
            return "DynObject";
        case HIGHGUID_TYPE_CORPSE:
            return "Corpse";
        case HIGHGUID_TYPE_MO_TRANSPORT:
            return "MoTransport";
        case HIGHGUID_TYPE_INSTANCE:
            return "InstanceID";
        case HIGHGUID_TYPE_GROUP:
            return "Group";
        case HIGHGUID_TYPE_BATTLEGROUND:
            return "Battleground";
        default:
            return "<unknown>";
    }
}

std::string NewWoWGuid::GetObjectString() const
{
    std::ostringstream str;
    str << GetHighGuidTypeName();

    if (!IsPlayer())
    {
        str << " (";
        if (HasEntry())
            str << (IsPet() ? "Petnumber: " : "Entry: ") << GetEntry() << " ";
        str << "Guid: " << GetCounter() << ")";
    }
    /*else
    {
        PlayerInfo* player = objmgr.GetPlayerInfo(GetEntry());
        if (player != nullptr)
            str << " " << player->name;
    }*/

    return str.str();
}

template<HighGuidType high>
uint32 NewWoWGuidGenerator<high>::Generate()
{
    if (m_nextGuid >= NewWoWGuid::GetMaxCounter(high) - 1)
    {
        sLog.outError("%s guid overflow!! Can't continue, shutting down server. ", NewWoWGuid::GetHighGuidTypeName(high));
        //World::StopNow(ERROR_EXIT_CODE);
    }
    return m_nextGuid++;
}

NewByteBuffer& operator<< (NewByteBuffer& buf, NewWoWGuid const& guid)
{
    buf << uint64(guid.GetRawValue());
    return buf;
}

NewByteBuffer& operator >> (NewByteBuffer& buf, NewWoWGuid& guid)
{
    guid.Set(buf.read<uint64>());
    return buf;
}

NewByteBuffer& operator<< (NewByteBuffer& buf, PackedWoWGuid const& guid)
{
    buf.append(guid.m_packedGuid);
    return buf;
}

NewByteBuffer& operator >> (NewByteBuffer& buf, PackedWoWGuidReader const& guid)
{
    guid.m_guidPtr->Set(buf.readPackGUID());
    return buf;
}

template uint32 NewWoWGuidGenerator<HIGHGUID_TYPE_ITEM>::Generate();
template uint32 NewWoWGuidGenerator<HIGHGUID_TYPE_PLAYER>::Generate();
template uint32 NewWoWGuidGenerator<HIGHGUID_TYPE_GAMEOBJECT>::Generate();
template uint32 NewWoWGuidGenerator<HIGHGUID_TYPE_TRANSPORTER>::Generate();
template uint32 NewWoWGuidGenerator<HIGHGUID_TYPE_UNIT>::Generate();
template uint32 NewWoWGuidGenerator<HIGHGUID_TYPE_PET>::Generate();
template uint32 NewWoWGuidGenerator<HIGHGUID_TYPE_VEHICLE>::Generate();
template uint32 NewWoWGuidGenerator<HIGHGUID_TYPE_DYNAMICOBJECT>::Generate();
template uint32 NewWoWGuidGenerator<HIGHGUID_TYPE_CORPSE>::Generate();
template uint32 NewWoWGuidGenerator<HIGHGUID_TYPE_INSTANCE>::Generate();
template uint32 NewWoWGuidGenerator<HIGHGUID_TYPE_GROUP>::Generate();
