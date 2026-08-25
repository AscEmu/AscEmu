/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#define _CRT_SECURE_NO_DEPRECATE

#include "dbcfile.h"
#include <cstring>

DBCFile::DBCFile(mpqlib::MpqPatchChain& mpq, std::string const& fileName) :
    _mpq(mpq), _fileName(fileName), _data(nullptr), _stringTable(nullptr)
{
}

bool DBCFile::open()
{
    std::vector<uint8_t> raw;
    if (!_mpq.readFile(_fileName, raw))
        return false;

    if (raw.size() < 20)
        return false;

    if (raw[0] != 'W' || raw[1] != 'D' || raw[2] != 'B' || raw[3] != 'C')
        return false;

    unsigned int na, nb, es, ss;
    std::memcpy(&na, raw.data() + 4, 4);  // Number of records
    std::memcpy(&nb, raw.data() + 8, 4);  // Number of fields
    std::memcpy(&es, raw.data() + 12, 4); // Size of a record
    std::memcpy(&ss, raw.data() + 16, 4); // String size

    _recordSize = es;
    _recordCount = na;
    _fieldCount = nb;
    _stringSize = ss;
    if (_fieldCount * 4 != _recordSize)
        return false;

    const uint32_t data_size = static_cast<uint32_t>(_recordSize * _recordCount + _stringSize);
    if (raw.size() < 20 + data_size)
        return false;

    _data = new unsigned char[data_size];
    _stringTable = _data + _recordSize * _recordCount;
    std::memcpy(_data, raw.data() + 20, data_size);

    return true;
}

DBCFile::~DBCFile()
{
    delete[] _data;
}

DBCFile::Record DBCFile::getRecord(size_t id)
{
    assert(_data);
    return Record(*this, _data + id * _recordSize);
}

size_t DBCFile::getMaxId()
{
    assert(_data);

    size_t maxId = 0;
    for (size_t i = 0; i < getRecordCount(); ++i)
        if (maxId < getRecord(i).getUInt(0))
            maxId = getRecord(i).getUInt(0);

    return maxId;
}

DBCFile::Iterator DBCFile::begin()
{
    assert(_data);
    return Iterator(*this, _data);
}

DBCFile::Iterator DBCFile::end()
{
    assert(_data);
    return Iterator(*this, _stringTable);
}
