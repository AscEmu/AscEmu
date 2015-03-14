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

#ifndef _ATOMICBOOLEAN_H
#define _ATOMICBOOLEAN_H

namespace Arcemu
{
    namespace Threading
    {

        //////////////////////////////////////////////////////////////////////////////////////////
        /// class AtomicBoolean
        /// Stores a Boolean atomically, using an AtomicULong
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
        class AtomicBoolean
        {
            public:

                AtomicBoolean() : Value(0) {}

                AtomicBoolean(bool val)
                {
                    if (val)
                        Value.SetVal(1);
                    else
                        Value.SetVal(0);
                }

                //////////////////////////////////////////////////////////////////////////////////////////
                /// bool SetVal(bool val)
                /// lockless threadsafe set operation on the contained value
                ///
                /// \param bool val  -  value to be set
                ///
                /// \return the initial value contained
                //////////////////////////////////////////////////////////////////////////////////////////
                bool SetVal(bool val);


                //////////////////////////////////////////////////////////////////////////////////////////
                ///bool GetVal()
                /// non-threadsafe get operation on the contained value
                ///
                /// \param none
                ///
                /// \return the value contained
                //////////////////////////////////////////////////////////////////////////////////////////
                bool GetVal()
                {
                    unsigned long val = 0;

                    val = Value.GetVal();

                    return (val & 1);
                }

            private:

                // Disabled copy constructor
                AtomicBoolean(const AtomicBoolean& other) {}

                // Disabled assignment operator
                AtomicBoolean operator=(const AtomicBoolean& other) { return *this; }

                AtomicULong Value;
        };
    }
}

#endif      //_ATOMICBOOLEAN_H
