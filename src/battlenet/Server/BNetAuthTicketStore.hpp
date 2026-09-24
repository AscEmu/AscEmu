/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>

namespace AscEmu::Battlenet
{
    void storeWebAuthTicket(const std::string& ticket, const std::string& login);
    bool consumeWebAuthTicket(const std::string& ticket, std::string& login);
}
