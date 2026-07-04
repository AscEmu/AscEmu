/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "MovementTypes.hpp"
#include <span>

extern std::span<MovementStep const> const UnknownDescriptor;

std::span<MovementStep const> getClassicMovementDescriptor(uint16_t opcode, bool read);
std::span<MovementStep const> getTbcMovementDescriptor(uint16_t opcode, bool read);
std::span<MovementStep const> getWotlkMovementDescriptor(uint16_t opcode, bool read);
std::span<MovementStep const> getCataMovementDescriptor(uint16_t opcode, bool read);
std::span<MovementStep const> getMopMovementDescriptor(uint16_t opcode, bool read);
