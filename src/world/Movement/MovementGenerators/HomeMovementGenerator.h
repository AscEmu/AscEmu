/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Movement/MovementGenerator.h"

template <class T>
class HomeMovementGenerator : public MovementGeneratorMedium< T, HomeMovementGenerator<T> >
{
public:
    explicit HomeMovementGenerator();

    MovementGeneratorType getMovementGeneratorType() const override;

    void doInitialize(T*);
    void doReset(T*);
    bool doUpdate(T*, uint32_t);
    void doDeactivate(T*);
    void doFinalize(T*, bool, bool);

private:
    void setTargetLocation(T*);
};
