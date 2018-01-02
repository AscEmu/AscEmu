/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
 */

#ifndef WORLDCREATOR_DEFINES_HPP
#define WORLDCREATOR_DEFINES_HPP

enum INSTANCE_TYPE
{
    INSTANCE_NULL,
    INSTANCE_RAID,
    INSTANCE_NONRAID,
    INSTANCE_BATTLEGROUND,
    INSTANCE_MULTIMODE,
};

enum INSTANCE_ABORT_ERROR
{
    INSTANCE_OK                                 = 0x00,
    INSTANCE_ABORT_ERROR_ERROR                  = 0x01,
    INSTANCE_ABORT_FULL                         = 0x02,
    INSTANCE_ABORT_NOT_FOUND                    = 0x03,
    INSTANCE_ABORT_TOO_MANY                     = 0x04,
    INSTANCE_ABORT_ENCOUNTER                    = 0x06,
    INSTANCE_ABORT_NON_CLIENT_TYPE              = 0x07,
    INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE    = 0x08,
    INSTANCE_ABORT_UNIQUE_MESSAGE               = 0x09,
    INSTANCE_ABORT_TOO_MANY_REALM_INSTANCES     = 0x0A,
    INSTANCE_ABORT_NOT_IN_RAID_GROUP            = 0x0B,
    INSTANCE_ABORT_REALM_ONLY                   = 0x0F,
    INSTANCE_ABORT_MAP_NOT_ALLOWED              = 0x10
};

enum INSTANCE_RESET_ERROR
{
    INSTANCE_RESET_ERROR_PLAYERS_INSIDE         = 0x00,
    INSTANCE_RESET_ERROR_MEMBERS_OFFLINE        = 0x01,
    INSTANCE_RESET_ERROR_PLAYERS_ENTERING       = 0x02
};

#endif      //WORLDCREATOR_DEFINES_HPP
