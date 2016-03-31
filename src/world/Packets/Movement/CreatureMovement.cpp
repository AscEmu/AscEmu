/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

namespace Packets
{
    namespace Movement
    {
        void SendMoveToPacket(Unit* pUnit)
        {
            ::Packets::Movement::SmsgMonsterMove MovePacket;
            MovePacket.m_Guid = pUnit->GetNewGUID();
            MovePacket.m_unk0 = uint8(0);
            MovePacket.m_point = ::Movement::Point{ 0.0f, 0.0f, 0.0f };
        }
    }
} 
