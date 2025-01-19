/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AbstractFollower.h"
#include "Objects/Units/Unit.hpp"

void AbstractFollower::setTarget(Unit* unit)
{
    if (unit == _target)
        return;

    if (_target)
        _target->followerRemoved(this);
    _target = unit;
    if (_target)
        _target->followerAdded(this);
}
