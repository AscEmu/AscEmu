/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
 */

#ifndef _CONDITION_VARIABLE_H
#define _CONDITION_VARIABLE_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace Arcemu
{
    namespace Threading
    {
        //////////////////////////////////////////////////////////////////////////////////////////
        //class ConditionVariable
        //  Class implementing a platform independent condition variable
        //
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        class ConditionVariable
        {
            public:

                ConditionVariable();
                ~ConditionVariable();

                //////////////////////////////////////////////////////////////////////////////////////////
                /// void Signal()
                /// Signals the condition variable, allowing the blocked thread to proceed
                ///
                /// \param none
                ///
                /// \retun none
                ///
                //////////////////////////////////////////////////////////////////////////////////////////
                void Signal();


                //////////////////////////////////////////////////////////////////////////////////////////
                /// void Wait(unsigned long timems)
                /// Blocks execution of the calling thread until signaled or until the timer runs out.
                ///
                /// \param unsigned long timems  -  Maximum time to block in milliseconds
                ///
                /// \retun none
                ///
                //////////////////////////////////////////////////////////////////////////////////////////
                void Wait(unsigned long timems);

            private:

#ifdef WIN32
                HANDLE hEvent;
#else
                pthread_cond_t cond;
                pthread_mutex_t mutex;
#endif
        };
    }
}

#endif  //_CONDITION_VARIABLE_H
