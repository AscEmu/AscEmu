/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Master.hpp"

int main()
{
    AscEmu::Battlenet::Master::getInstance().run();
    return 0;
}
