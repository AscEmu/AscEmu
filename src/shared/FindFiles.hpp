/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

#pragma once

#include "FindFilesResult.hpp"

namespace Arcemu
{
    //////////////////////////////////////////////////////////////////////////////////////////
    ///bool FindFiles(const char* where, const char* filename, FindFilesResult& r)
    ///  Searches for files with the name of *filename*, and
    ///  puts the result(s) into a FindFileResult object.
    ///
    /// \param const char *where     -  directory to search
    /// \param const char *filename  -  filename/mask to search for
    /// \param FindFilesResult &r    -  reference to the result object
    ///
    /// \return true if at least 1 file was found, false if there were no files found.
    ///
    //////////////////////////////////////////////////////////////////////////////////////////
    bool FindFiles(const char* where, const char* filename, FindFilesResult& r);
}
