/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "MovementTypes.hpp"
#include <span>

extern std::span<MovementStep const> const UnknownDescriptor;

std::span<MovementStep const> getClassicMovementDescriptor(uint32_t opcode, bool read);
std::span<MovementStep const> getTbcMovementDescriptor(uint32_t opcode, bool read);
std::span<MovementStep const> getWotlkMovementDescriptor(uint32_t opcode, bool read);
std::span<MovementStep const> getCataMovementDescriptor(uint32_t opcode, bool read);
std::span<MovementStep const> getMopMovementDescriptor(uint32_t opcode, bool read);
std::span<MovementStep const> getWoDMovementDescriptor(uint32_t opcode, bool read);
std::span<MovementStep const> getLegionMovementDescriptor(uint32_t opcode, bool read);
std::span<MovementStep const> getBfAMovementDescriptor(uint32_t opcode, bool read);
std::span<MovementStep const> getShadowlandsMovementDescriptor(uint32_t opcode, bool read);
std::span<MovementStep const> getDragonflightMovementDescriptor(uint32_t opcode, bool read);
std::span<MovementStep const> getTheWarWithinMovementDescriptor(uint32_t opcode, bool read);
std::span<MovementStep const> getMidnightMovementDescriptor(uint32_t opcode, bool read);
