/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Definitions/SpellFailure.h"

#include "CommonTypes.hpp"

class Spell;
class SERVER_DECL SpellScript
{
public:

    SpellScript() {}
    virtual ~SpellScript() {}

    virtual SpellCastResult onCanCast(Spell* /*spell*/, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/) { return SPELL_CANCAST_OK; }
};
