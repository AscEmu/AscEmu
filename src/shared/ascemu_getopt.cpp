/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "Common.h"
#include "ascemu_getopt.h"

int arg_counter = 1;
char arcemu_optarg[514];
int arcemu_getopt_long_only(int ___argc, char* const* ___argv, const char* __shortopts, const struct arcemu_option* __longopts, int* __longind)
{
    ///\todo handle the shortops, at the moment it only works with longopts.

    if(___argc == 1 || arg_counter == ___argc)            // No arguments (apart from filename)
        return -1;

    const char* opt = ___argv[arg_counter];
//    int return_val = 0;

    // if we're not an option, return an error.
    if(strnicmp(opt, "--", 2) != 0)
        return 1;
    else
        opt += 2;


    // parse argument list
    int i = 0;
    for(; __longopts[i].name != 0; ++i)
    {
        if(!strnicmp(__longopts[i].name, opt, strlen(__longopts[i].name)))
        {
            // woot, found a valid argument =)
            char* par = 0;
            if((arg_counter + 1) != ___argc)
            {
                // grab the parameter from the next argument (if its not another argument)
                if(strnicmp(___argv[arg_counter + 1], "--", 2) != 0)
                {
                    arg_counter++;        // Trash this next argument, we won't be needing it.
                    par = ___argv[arg_counter];
                }
            }

            // increment the argument for next time
            arg_counter++;

            // determine action based on type
            if(__longopts[i].has_arg == arcemu_required_argument)
            {
                if(!par)
                    return 1;

                // parameter missing and its a required parameter option
                if(__longopts[i].flag)
                {
                    *__longopts[i].flag = atoi(par);
                    return 0;
                }
            }

            // store argument in optarg
            if (par)
            {
                auto result = snprintf(arcemu_optarg, sizeof(arcemu_optarg), "%s", par);
                /* If an error has occurred or the buffer was not large enough for this argument */
                if (result < 0 || result > sizeof(arcemu_optarg))
                    return 1;
            }

            if(__longopts[i].flag != 0)
            {
                // this is a variable, we have to set it if this argument is found.
                *__longopts[i].flag = 1;
                return 0;
            }
            else
            {
                if(__longopts[i].val == -1 || par == 0)
                    return 1;

                return __longopts[i].val;
            }
            break;
        }
    }

    // return 1 (invalid argument)
    return 1;
}

