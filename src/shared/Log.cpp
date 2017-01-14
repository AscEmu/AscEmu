/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Log.hpp"
#include "Util.hpp"

namespace AELog
{
    std::string GetFormattedFileName(std::string path_prefix, std::string file_prefix, bool use_date_time)
    {
        std::string full_name;

        std::string basic_path_name = path_prefix + "/" + file_prefix;

        if (use_date_time)
        {
            std::string current_date_time = Util::GetCurrentDateTimeString();
            //replace time seperator with valid character for file name
            std::replace(current_date_time.begin(), current_date_time.end(), ':', '-');

            full_name = basic_path_name + "-" + current_date_time + ".log";
        }
        else
        {
            full_name = basic_path_name + ".log";
        }

        return full_name;
    }
}

#ifndef _WIN32
void oLog::SetConsoleColor(const char* color)
{
    fputs(color, stdout);
}

#else
void oLog::SetConsoleColor(int color)
{
    SetConsoleTextAttribute(stdout_handle, (WORD)color);
}
#endif
