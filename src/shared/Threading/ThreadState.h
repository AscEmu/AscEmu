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
