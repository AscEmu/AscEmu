/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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

#ifndef _ATOMICULONG_H
#define _ATOMICULONG_H

namespace Arcemu
{
    namespace Threading
    {

        //////////////////////////////////////////////////////////////////////////////////////////
        /// class AtomicULong
        /// Stores an unsigned long atomically.
        /// Base class for all Arcemu atomics.
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
        class AtomicULong
        {
            public:
                AtomicULong() { Value = 0; }

                AtomicULong(unsigned long InitialValue) { Value = InitialValue; }

                //////////////////////////////////////////////////////////////////////////////////////////
                /// unsigned long SetVal( unsigned long NewValue )
                /// lockless threadsafe set operation on the contained value
                ///
                /// \param unsigned long val  -  value to be set
                ///
                /// \return the initial value contained
                //////////////////////////////////////////////////////////////////////////////////////////
                unsigned long SetVal(unsigned long NewValue);


                //////////////////////////////////////////////////////////////////////////////////////////
                /// unsigned long GetVal()
                /// non-threadsafe get operation on the contained value
                ///
                /// \param none
                ///
                /// \return the value contained
                //////////////////////////////////////////////////////////////////////////////////////////
                unsigned long GetVal() { return Value; }


            private:

                // Disabled copy constructor
                AtomicULong(const AtomicULong& other) {}

                // Disabled assignment operator
                AtomicULong operator=(AtomicULong& other) { return *this; }


            protected:

#ifdef WIN32
                __declspec(align(4))  volatile unsigned long Value;
#else
                volatile unsigned long Value;
#endif
        };
    }
}

#endif      //_ATOMICULONG_H
