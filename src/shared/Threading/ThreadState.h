/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace AscEmu { namespace Threading
{
    enum class ThreadState
    {
        Terminated,
        Paused,
        Sleeping,
        Busy,
        Awaiting
    };
}}
