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

#ifndef _ATOMICFLOAT_H
#define _ATOMICFLOAT_H

namespace Arcemu
{
    namespace Threading
    {

        //////////////////////////////////////////////////////////////////////////////////////////
        /// class AtomicFloat
        /// Stores a Float atomically.
        /// Implemented using AtomicULong.
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
        class AtomicFloat
        {
            public:
                AtomicFloat() : Value(0) {}

                AtomicFloat(float InitialValue)
                {
                    unsigned long iv = *(reinterpret_cast< unsigned long* >(&InitialValue));
                    Value.SetVal(iv);
                }


                //////////////////////////////////////////////////////////////////////////////////////////
                /// float SetVal(float NewValue)
                /// lockless threadsafe set operation on the contained value
                ///
                /// \param float val  -  value to be set
                ///
                /// \return the initial value contained
                //////////////////////////////////////////////////////////////////////////////////////////
                float SetVal(float NewValue);


                //////////////////////////////////////////////////////////////////////////////////////////
                /// bool GetVal()
                /// non-threadsafe get operation on the contained value
                ///
                /// \param none
                ///
                /// \return the value contained
                //////////////////////////////////////////////////////////////////////////////////////////
                float GetVal();

            private:

                // Disabled copy constructor
                AtomicFloat(const AtomicFloat& other) {}

                // Disabled assignment operator
                AtomicFloat operator=(const AtomicFloat& other) { return *this; }

                AtomicULong Value;
        };
    }
}

#endif      //_ATOMICFLOAT_H
