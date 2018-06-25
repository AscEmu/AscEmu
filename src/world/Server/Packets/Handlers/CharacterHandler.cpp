/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "git_version.h"
#include "AuthCodes.h"
#include "Management/WordFilter.h"
#include "Management/ArenaTeam.h"
#include "Management/Battleground/Battleground.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Players/PlayerClasses.hpp"
#include "Server/MainServerDefines.h"
#include "Config/Config.h"
#include "Map/MapMgr.h"
#include "Map/WorldCreator.h"
#include "Spell/Definitions/PowerType.h"
#include "Data/WoWPlayer.h"
#include "Server/Packets/SmsgCharEnum.h"
#if VERSION_STRING == Cata
#include "GameCata/Management/GuildMgr.h"
#endif

void WorldSession::HandleChangePlayerNameOpcode(WorldPacket& recv_data)
{
    //\todo check utf8 and cyrillic chars
    // check declined names

    CHECK_PACKET_SIZE(recv_data,8+1);

    uint64_t guid;
    std::string newname;

    recv_data >> guid;
    recv_data >> newname;

    WorldPacket data(SMSG_CHAR_RENAME, 1);
    data << uint8_t(E_CHAR_CREATE_ERROR);
    SendPacket(&data);
}

void WorldSession::HandleChangePlayerNameOpcodeCallBack(WorldPacket& recv_data)
{
    //\todo check utf8 and cyrillic chars
    // check declined names

    CHECK_PACKET_SIZE(recv_data,8+1);

    uint64_t guid;

    recv_data >> guid;

    std::string newname;

    WorldPacket data(SMSG_CHAR_RENAME, 1+8+(newname.size()+1));
    data << uint8_t(E_RESPONSE_SUCCESS);
    data << uint64_t(guid);
    data << newname;
    SendPacket(&data);
}

void WorldSession::HandleDeclinedPlayerNameOpcode(WorldPacket& recv_data)
{
    //\todo check utf8 and cyrillic chars

    uint64_t guid;
    uint32_t error = 0;

    recv_data >> guid;

    std::string declinedname[MAX_DECLINED_NAME_CASES];

    for(int i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
    {
        recv_data >> declinedname[i];

        if(declinedname[i].empty())
            error = 1;

        CharacterDatabase.EscapeString(declinedname[i]);
    }
    for(int i = 1; i < MAX_DECLINED_NAME_CASES; ++i)
    {
        if( declinedname[i].size() < declinedname[0].size() )
            error = 1;
    } 
    if(!error) 
        CharacterDatabase.Query("REPLACE INTO declinedname_character (guid, genitive, dative, accusative, instrumental, prepositional) VALUES ('%u','%s','%s','%s','%s','%s')", Arcemu::Util::GUID_LOPART(guid), declinedname[1].c_str(),declinedname[2].c_str(),declinedname[3].c_str(),declinedname[4].c_str(),declinedname[5].c_str()); 

    WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
    data << uint32_t(0);
    data << uint64_t(guid);
    SendPacket(&data);
}
