/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WorldSocket.hpp"

#ifndef AE_WORLD_PROFILE_FOREVER
#define AE_WORLD_PROFILE_FOREVER 0
#endif

#ifndef AE_WORLD_PROFILE_WOD
#define AE_WORLD_PROFILE_WOD 0
#endif

#ifndef AE_WORLD_PROFILE_LEGION
#define AE_WORLD_PROFILE_LEGION 0
#endif

#if !AE_WORLD_PROFILE_FOREVER && !AE_WORLD_PROFILE_WOD && !AE_WORLD_PROFILE_LEGION
bool WorldSocket::initializeVersionedConnection()
{
    return false;
}

bool WorldSocket::processVersionedRead()
{
    return false;
}

bool WorldSocket::sendVersionedPacket(WorldPacket*)
{
    return false;
}

#endif
