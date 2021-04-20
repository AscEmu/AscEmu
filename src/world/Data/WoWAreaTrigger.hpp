/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

/*
 * THIS IS A WOW DATA STRUCT FILE
 *
 * DO NOT ADD ADDITIONAL MEMBERS TO IT
 *
 * DO NOT EDIT IT UNLESS YOU KNOW EXACTLY WHAT YOU ARE DOING
 */

#pragma once
#include "WoWObject.hpp"
#pragma pack(push, 1)

#if VERSION_STRING < Mop
struct WoWAreaTrigger : WoWObject
{
    uint32_t spell_id;
    uint32_t spell_visual_id;
    uint32_t duration;
    float pos_x;
    float pos_y;
    float pos_z;
};
#else
struct WoWAreaTrigger : WoWObject
{
    guid_union caster_guid;
    uint32_t duration;
    uint32_t spell_id;
    uint32_t spell_visual_id;
    float scale;
};
#endif

#pragma pack(pop)
