/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef LOG_HPP
#define LOG_HPP

#include "Log.Legacy.h"

namespace AELog
{
    /*! \brief Returns formatted file name based on input */
    std::string GetFormattedFileName(std::string path_prefix, std::string file_prefix, bool use_date_time);

    /*! \brief Returns cons char* (linux) or int (windows) color definition for console */
#ifndef _WIN32
    const char* GetColorForDebugFlag(LogFlags log_flags);
#else
    int GetColorForDebugFlag(LogFlags log_flags);
#endif
}

#endif  // LOG_HPP
