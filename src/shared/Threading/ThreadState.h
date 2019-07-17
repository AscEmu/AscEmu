/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace AscEmu::Threading
{
    enum class ThreadState
    {
        Terminated,
        Paused,
        Sleeping,
        Busy,
        Awaiting
    };
}
