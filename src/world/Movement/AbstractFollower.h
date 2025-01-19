/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

class Unit;

struct AbstractFollower
{
public:
    AbstractFollower(Unit* target = nullptr) { setTarget(target); }
    ~AbstractFollower() { setTarget(nullptr); }

    void setTarget(Unit* unit);
    Unit* getTarget() const { return _target; }

private:
    Unit* _target = nullptr;
};
