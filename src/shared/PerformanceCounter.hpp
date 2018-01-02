/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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


#pragma once

namespace Arcemu
{
    //////////////////////////////////////////////////////////////////////////////////////////
    ///class PerformanceCounter
    /// A simple performance counter class, that samples
    /// resource usages and tells the usage in percentages
    ///
    //////////////////////////////////////////////////////////////////////////////////////////
    class PerformanceCounter
    {
        public:

            PerformanceCounter();
            ~PerformanceCounter(){}

            //////////////////////////////////////////////////////////////////////////////////////////
            ///float GetCurrentCPUUsage()
            /// Returns the CPU usage in percentage since the last sample
            ///
            /// \param none
            ///
            /// \return the CPU usage in percentage
            ///
            //////////////////////////////////////////////////////////////////////////////////////////
            float GetCurrentCPUUsage();

            //////////////////////////////////////////////////////////////////////////////////////////
            ///float GetCurrentRAMUsage()
            /// Tells the current RAM usage in bytes
            ///
            /// \param none
            ///
            /// \return the current RAM usage in bytes
            ///
            //////////////////////////////////////////////////////////////////////////////////////////
            float GetCurrentRAMUsage();
    
        private:

            unsigned long cpu_count;            // The number of CPUs in this system  ( CPU cores count as separate CPUs )
            unsigned long long last_update;     // The time the last sample was created
            unsigned long long last_cpu_usage;  // The last sample of CPU usage
    };

}
