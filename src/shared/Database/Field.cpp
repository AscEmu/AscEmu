/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Field.hpp"
#include "Utilities/Narrow.hpp"
#include <cstdint>

bool Field::isSet() const { return m_value ? true : false; }
void Field::setValue(char* value) { m_value = value; }

const char* Field::asCString() const { return m_value; }

float Field::asFloat() const { return m_value ? Util::stringToFloat(m_value) : 0.0f; }
bool Field::asBool() const { return m_value ? Util::stringToBool(m_value) : false; }

uint8_t Field::asUint8(bool _silencedError) const { return m_value ? Util::stringToUint8(m_value, _silencedError) : 0U; }
int8_t Field::asInt8() const { return m_value ? Util::stringToInt8(m_value) : 0; }

uint16_t Field::asUint16(bool _silencedError) const { return m_value ? Util::stringToUint16(m_value, _silencedError) : 0U; }
int16_t Field::asInt16() const { return m_value ? Util::stringToInt16(m_value) : 0; }

uint32_t Field::asUint32(bool _silencedError) const { return m_value ? Util::stringToUint32(m_value, _silencedError) : 0U; }
int32_t Field::asInt32() const { return m_value ? Util::stringToInt32(m_value) : 0; }

uint64_t Field::asUint64(bool _silencedError) const { return m_value ? Util::stringToUint64(m_value, _silencedError) : 0U; }
int64_t Field::asInt64() const { return m_value ? Util::stringToInt64(m_value) : 0; }

